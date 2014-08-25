#ifndef __PYTHON_PIECE_H__
#define __PYTHON_PIECE_H__

#include <Python.h>
#include "structmember.h"

#include <piece.h>
#include <sauce.h>

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
    PyObject *filename;
    PyObject *parsername;

    /* Non-pythonic stuffs */
    piece_screen *display;
    piece_parser *parser;
    piece_font *font;
} Piece;

typedef struct {
    PyObject_HEAD
} Screen; 

#endif // __PYTHON_PIECE_H__
