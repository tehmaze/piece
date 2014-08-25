#include <Python.h>
#include "bytearrayobject.h"
#include "stringobject.h"
#include <gd.h>

#include <piece/font.h>
#include <piece/screen.h>
#include <piece/writer/image.h>

#include "python/piece.h"

static PyObject *piece_PieceError;
typedef enum {
    PIECE_IMAGE_BMP,
    PIECE_IMAGE_GD,
    PIECE_IMAGE_GIF,
    PIECE_IMAGE_JPEG,
    PIECE_IMAGE_PNG,
} piece_image_type;

static int
Piece_traverse(Piece *self, visitproc visit, void *arg)
{
    Py_VISIT(self->filename);
    Py_VISIT(self->parsername);
    return 0;
}

static int
Piece_clear(Piece *self)
{
    PyObject *tmp;

    tmp = self->filename;
    self->filename = NULL;
    Py_XDECREF(tmp);

    tmp = self->parsername;
    self->parsername = NULL;
    Py_XDECREF(tmp);

    return 0;
}

static void
Piece_dealloc(Piece *self)
{
    Piece_clear(self);
    self->ob_type->tp_free((PyObject *) self);
}

static PyObject *
Piece_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    Piece *self;

    self = (Piece *) type->tp_alloc(type, 0);
    if (self != NULL) {
        self->filename = PyString_FromString("");
        if (self->filename == NULL) {
            Py_DECREF(self);
            return NULL;
        }
        self->parsername = PyString_FromString("");
        if (self->parsername == NULL) {
            Py_DECREF(self);
            return NULL;
        }
    }

    return (PyObject *) self;
}

static int
Piece_init(Piece *self, PyObject *args, PyObject *kwargs)
{
    PyObject *filename = NULL,
             *parsername = NULL,
             *tmp;

    static char *kwlist[] = {"filename", "parsername", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "S|S", kwlist,
                                     &filename, &parsername)) {
        return -1;
    }

    if (filename) {
        tmp = self->filename;
        Py_INCREF(filename);
        self->filename = filename;
        Py_XDECREF(tmp);
    }
    if (parsername) {
        tmp = self->parsername;
        Py_INCREF(parsername);
        self->parsername = parsername;
        Py_XDECREF(tmp);
    }

    return 0;
}

static PyObject *
Piece_parse(Piece *self, PyObject *args, PyObject *kwargs)
{
    FILE *fd;
    char message[768];
    PyObject *parsername = NULL,
             *tmp;

    if (self->filename == NULL) {
        PyErr_SetString(PyExc_AttributeError, "filename");
        return NULL;
    }

    if ((fd = fopen(PyString_AS_STRING(self->filename), "rb")) == NULL) {
        sprintf(message, "Error opening %s for reading",
                          PyString_AS_STRING(self->filename));
        PyErr_SetString(PyExc_IOError, message);
        return NULL;
    }

    self->parser = piece_parser_for(fd, PyString_AS_STRING(self->filename));
    if (self->parser == NULL) {
        sprintf(message, "Error finding parser for %s",
                          PyString_AS_STRING(self->filename));
        PyErr_SetString(PyExc_TypeError, message);
        return NULL;
    }

    /* Store name of the parser */
    tmp = self->parsername;
    parsername = PyString_FromString(self->parser->name);
    Py_INCREF(parsername);
    self->parsername = parsername;
    Py_XDECREF(tmp);

    /* Do the actual parsing */
    self->display = self->parser->read(fd, PyString_AS_STRING(self->filename));
    if (self->display == NULL || self->display->tiles == 0) {
        tmp = self->parsername;
        parsername = PyString_FromString("");
        Py_INCREF(parsername);
        self->parsername = parsername;
        Py_XDECREF(tmp);

        sprintf(message, "Error parsing %s",
                         PyString_AS_STRING(self->filename));
        PyErr_SetString(PyExc_ValueError, message);
        return NULL;
    }

    /*
    image = piece_image_writer_parse(
        self->display,
        PyString_AS_STRING(self->filename),
        self->font
    );
    image_data = gdImagePngPtr(image, &image_size);
    gdImageDestroy(image);

    bytes = PyByteArray_FromStringAndSize(
        (const char *) image_data,
        (Py_ssize_t) image_size
    );
    */

    rewind(fd);
    fclose(fd);

    return Py_True;
}

static PyObject *
_Piece_image(Piece *self, piece_image_type type, const char *filename)
{
    FILE *fd;
    char message[768];
    gdImagePtr image;

    if ((fd = fopen(filename, "wb")) == NULL) {
        sprintf(message, "%s: error opening for writing", filename);
        PyErr_SetString(PyExc_IOError, message);
        return NULL;
    }

    image = piece_image_writer_parse(
        self->display,
        PyString_AS_STRING(self->filename),
        self->font
    );
    if (image == NULL) {
        sprintf(message, "%s: error creating image buffer",
                         PyString_AS_STRING(self->filename));
        PyErr_SetString(PyExc_MemoryError, message);
        return NULL;
    }

    switch (type) {
    case PIECE_IMAGE_BMP:
        gdImageBmp(image, fd, 0);
        break;

    case PIECE_IMAGE_GD:
        gdImageGd(image, fd);
        break;

    case PIECE_IMAGE_GIF:
        gdImageGif(image, fd);
        break;

    case PIECE_IMAGE_JPEG:
        gdImageJpeg(image, fd, 100);
        break;

    case PIECE_IMAGE_PNG:
    default:
        gdImagePng(image, fd);
        break;
    }

    gdImageDestroy(image);
    return Py_True;
}

static void *
_Piece_image_buffer(Piece *self, piece_image_type type, int *size)
{
    char message[768];
    gdImagePtr image;
    void *output;

    image = piece_image_writer_parse(
        self->display,
        PyString_AS_STRING(self->filename),
        self->font
    );
    if (image == NULL) {
        sprintf(message, "%s: error creating image buffer",
                         PyString_AS_STRING(self->filename));
        PyErr_SetString(PyExc_MemoryError, message);
        return NULL;
    }

    switch (type) {
    case PIECE_IMAGE_BMP:
        output = gdImageBmpPtr(image, size, 0);
        break;

    case PIECE_IMAGE_GD:
        output = gdImageGdPtr(image, size);
        break;

    case PIECE_IMAGE_GIF:
        output = gdImageGifPtr(image, size);
        break;

    case PIECE_IMAGE_JPEG:
        output = gdImageJpegPtr(image, size, 100);
        break;

    case PIECE_IMAGE_PNG:
    default:
        output = gdImagePngPtr(image, size);
        break;
    }

    gdImageDestroy(image);
    return output;
}

static PyObject *
_Piece_to_any(Piece *self, PyObject *args, PyObject *kwargs,
              piece_image_type type)
{
    PyObject *filename = NULL;
    char message[768];

    static char *kwlist[] = {"filename", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "S", kwlist, &filename)) {
        return NULL;
    }

    if (self->display == NULL) {
        sprintf(message, "%s: not yet parsed",
                         PyString_AS_STRING(self->filename));
        PyErr_SetString(PyExc_ValueError, message);
        return NULL;
    }

    return _Piece_image(self, type, PyString_AS_STRING(filename));
}

static PyObject *
_Piece_to_any_buffer(Piece *self, PyObject *args, PyObject *kwargs,
                     piece_image_type type)
{
    char message[768];
    int size;
    void *buffer = NULL;

    if (self->display == NULL) {
        sprintf(message, "%s: not yet parsed",
                         PyString_AS_STRING(self->filename));
        PyErr_SetString(PyExc_ValueError, message);
        return NULL;
    }

    if ((buffer = _Piece_image_buffer(self, type, &size)) == NULL) {
        return NULL;
    }

    if (size <= 0) {
        sprintf(message, "%s: returned empty buffer",
                         PyString_AS_STRING(self->filename));
        PyErr_SetString(PyExc_ValueError, message);
        return NULL;
    }

    return PyByteArray_FromStringAndSize((const char *) buffer, size);
}

static PyObject *
Piece_to_bmp(Piece *self, PyObject *args, PyObject *kwargs)
{
    return _Piece_to_any(self, args, kwargs, PIECE_IMAGE_BMP);
}

static PyObject *
Piece_to_bmp_buffer(Piece *self, PyObject *args, PyObject *kwargs)
{
    return _Piece_to_any_buffer(self, args, kwargs, PIECE_IMAGE_BMP);
}

static PyObject *
Piece_to_gd(Piece *self, PyObject *args, PyObject *kwargs)
{
    return _Piece_to_any(self, args, kwargs, PIECE_IMAGE_GD);
}

static PyObject *
Piece_to_gd_buffer(Piece *self, PyObject *args, PyObject *kwargs)
{
    return _Piece_to_any_buffer(self, args, kwargs, PIECE_IMAGE_GD);
}

static PyObject *
Piece_to_gif(Piece *self, PyObject *args, PyObject *kwargs)
{
    return _Piece_to_any(self, args, kwargs, PIECE_IMAGE_GIF);
}

static PyObject *
Piece_to_gif_buffer(Piece *self, PyObject *args, PyObject *kwargs)
{
    return _Piece_to_any_buffer(self, args, kwargs, PIECE_IMAGE_GIF);
}

static PyObject *
Piece_to_jpg(Piece *self, PyObject *args, PyObject *kwargs)
{
    return _Piece_to_any(self, args, kwargs, PIECE_IMAGE_JPEG);
}

static PyObject *
Piece_to_jpg_buffer(Piece *self, PyObject *args, PyObject *kwargs)
{
    return _Piece_to_any_buffer(self, args, kwargs, PIECE_IMAGE_JPEG);
}

static PyObject *
Piece_to_png(Piece *self, PyObject *args, PyObject *kwargs)
{
    return _Piece_to_any(self, args, kwargs, PIECE_IMAGE_PNG);
}

static PyObject *
Piece_to_png_buffer(Piece *self, PyObject *args, PyObject *kwargs)
{
    return _Piece_to_any_buffer(self, args, kwargs, PIECE_IMAGE_PNG);
}

static PyMemberDef Piece_members[] = {
    {"filename", T_OBJECT_EX, offsetof(Piece, filename), 0, "file name"},
    {"parsername", T_OBJECT_EX, offsetof(Piece, parsername), 0, "parser"},
    {NULL}                                      /* Sentinel */
};

static PyMethodDef Piece_methods[] = {
    {"parse", (PyCFunction) Piece_parse, METH_NOARGS,
        "Parse the file, returns a bytearray with image data"},
    {"to_bmp", (PyCFunction) Piece_to_bmp, METH_VARARGS,
        "Save the parsed piece to a BMP image file"},
    {"to_bmp_buffer", (PyCFunction) Piece_to_bmp_buffer, METH_VARARGS,
        "Return the parsed piece as a BMP image buffer"},
    {"to_gd", (PyCFunction) Piece_to_gd, METH_VARARGS,
        "Save the parsed piece to a GD image file"},
    {"to_gd_buffer", (PyCFunction) Piece_to_gd_buffer, METH_VARARGS,
        "Return the parsed piece as a GD image buffer"},
    {"to_gif", (PyCFunction) Piece_to_gif, METH_VARARGS,
        "Save the parsed piece to a GIF image file"},
    {"to_gif_buffer", (PyCFunction) Piece_to_gif_buffer, METH_VARARGS,
        "Return the parsed piece as a GIF image buffer"},
    {"to_jpg", (PyCFunction) Piece_to_jpg, METH_VARARGS,
        "Save the parsed piece to a JPEG image file"},
    {"to_jpg_buffer", (PyCFunction) Piece_to_jpg_buffer, METH_VARARGS,
        "Return the parsed piece as a JPEG image buffer"},
    {"to_png", (PyCFunction) Piece_to_png, METH_VARARGS,
        "Save the parsed piece to a PNG image file"},
    {"to_png_buffer", (PyCFunction) Piece_to_png_buffer, METH_VARARGS,
        "Return the parsed piece as a PNG image buffer"},
    {NULL}                                      /* Sentinel */
};

static PyTypeObject piece_PieceType = {
    PyObject_HEAD_INIT(NULL)
    0,                                          /* ob_size*/
    "piece.Piece",                              /* tp_name*/
    sizeof(Piece),                              /* tp_basicsize*/
    0,                                          /* tp_itemsize*/
    (destructor) Piece_dealloc,                 /* tp_dealloc*/
    0,                                          /* tp_print*/
    0,                                          /* tp_getattr*/
    0,                                          /* tp_setattr*/
    0,                                          /* tp_compare*/
    0,                                          /* tp_repr*/
    0,                                          /* tp_as_number*/
    0,                                          /* tp_as_sequence*/
    0,                                          /* tp_as_mapping*/
    0,                                          /* tp_hash */
    0,                                          /* tp_call*/
    0,                                          /* tp_str*/
    0,                                          /* tp_getattro*/
    0,                                          /* tp_setattro*/
    0,                                          /* tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC, /* tp_flags*/
    "Piece objects",                            /* tp_doc */
    (traverseproc) Piece_traverse,              /* tp_traverse */
    (inquiry) Piece_clear,                      /* tp_clear */
    0,                                          /* tp_richcompare */
    0,                                          /* tp_weaklistoffset */
    0,                                          /* tp_iter */
    0,                                          /* tp_iternext */
    Piece_methods,                              /* tp_methods */
    Piece_members,                              /* tp_members */
    0,                                          /* tp_getset */
    0,                                          /* tp_base */
    0,                                          /* tp_dict */
    0,                                          /* tp_descr_get */
    0,                                          /* tp_descr_set */
    0,                                          /* tp_dictoffset */
    (initproc) Piece_init,                      /* tp_init */
    0,                                          /* tp_alloc */
    Piece_new,                                  /* tp_new */
    0,                                          /* tp_free */
    0,                                          /* tp_free */
    0,                                          /* tp_bases */
    0,                                          /* tp_mro */
    0,                                          /* tp_cache */
    0,                                          /* tp_subclasses */
    0,                                          /* tp_weaklist */
    0,                                          /* tp_del */
    0,                                          /* tp_version_tag */
};


static PyMethodDef module_methods[] = {
    {NULL}                                      /* Sentinel */
};

PyMODINIT_FUNC
init_piece(void)
{
    PyObject *m;

    piece_PieceType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&piece_PieceType) < 0)
        return;

    piece_options = malloc(sizeof(piece_option_flags));
    if (piece_options == NULL) {
        return;
    }
    memset(piece_options, 0, sizeof(piece_option_flags));
    piece_options->verbose = 0;

    piece_parser_init();
    piece_font_init();
    sauce_init();
    piece_palette_init();

    if ((m = Py_InitModule3("_piece", module_methods, "Piece artscene parser")) == NULL) {
        return;
    }

    piece_PieceError = PyErr_NewException("piece.error", NULL, NULL);
    Py_INCREF(piece_PieceError);
    PyModule_AddObject(m, "Error", piece_PieceError);

    Py_INCREF(&piece_PieceType);
    PyModule_AddObject(m, "Piece", (PyObject *) &piece_PieceType);
}
