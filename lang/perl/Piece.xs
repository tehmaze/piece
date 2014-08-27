#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <gd.h>
#include "piece_xs.h"

typedef piece_t * Image__Piece;

static void
_to_image(ctx, filename, type)
    piece_t *ctx;
    char *filename;
    piece_image_type type;
{
    FILE *fd;
    gdImagePtr image = NULL;

    if (ctx->display == NULL) {
        croak("%s: not yet parsed", ctx->filename);
    }

    if ((fd = fopen(filename, "wb")) == NULL) {
        croak("%s: error opening for writing", filename);
    }
    image = piece_image_writer_parse(
        ctx->display,
        ctx->filename,
        ctx->font
    );
    if (image == NULL) {
        croak("%s: error creating image buffer", filename);
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
    rewind(fd);
    fclose(fd);
}

static SV *
_to_image_buffer(ctx, type)
    piece_t *ctx;
    piece_image_type type;
{
    gdImagePtr image = NULL;
    void *output = NULL;
    SV *string = newSVpv("", 0);
    int size = 0;

    if (ctx->display == NULL) {
        croak("%s: not yet parsed", ctx->filename);
    }

    image = piece_image_writer_parse(
        ctx->display,
        ctx->filename,
        ctx->font
    );
    if (image == NULL) {
        croak("%s: error creating image buffer", ctx->filename);
    }

    switch (type) {
    case PIECE_IMAGE_BMP:
        output = gdImageBmpPtr(image, &size, 0);
        break;

    case PIECE_IMAGE_GD:
        output = gdImageGdPtr(image, &size);
        break;

    case PIECE_IMAGE_GIF:
        output = gdImageGifPtr(image, &size);
        break;

    case PIECE_IMAGE_JPEG:
        output = gdImageJpegPtr(image, &size, 100);
        break;

    case PIECE_IMAGE_PNG:
    default:
        output = gdImagePngPtr(image, &size);
        break;
    }

    gdImageDestroy(image);

    sv_catpvn(string, output, size);
    return string;
}

MODULE = Image::Piece       PACKAGE = Image::Piece

PROTOTYPES: ENABLE

piece_t*
Image::Piece::new(filename, parsername = "")
    char *filename;
    char *parsername = NO_INIT;
PREINIT:
    piece_t *ctx;
CODE:
    Newx(ctx, 1, piece_t);
    memset(ctx, 0, sizeof(piece_t));
    ctx->filename = strdup(filename);
    if (parsername != NULL) {
        print("got parsername %s\n", parsername);
        ctx->parsername = strdup(parsername);
    } else {
        ctx->parsername = NULL;
    }
    ctx->display = NULL;
    ctx->font = NULL;

    RETVAL = ctx;
OUTPUT:
    RETVAL

int
parse(ctx)
    piece_t *ctx;
PREINIT:
    FILE *fd = NULL;
CODE:
    if (ctx->filename == NULL) {
        Perl_croak(aTHX_ "No filename was passed to Image::Piece");
    }

    if ((fd = fopen(ctx->filename, "rb")) == NULL) {
        Perl_croak(aTHX_ "%s: failed to open", ctx->filename);
    }

    if (ctx->parsername != NULL && strlen(ctx->parsername) == 0) {
        ctx->parser = piece_parser_for_type(ctx->parsername);
    } else {
        ctx->parser = piece_parser_for(fd, ctx->filename);
    }

    if (ctx->parser == NULL) {
        Perl_croak(aTHX_ "%s: no suitable parser found or specified",
                         ctx->filename);
    }

    /* Store the name of the parser */
    if (ctx->parsername == NULL ||
        strcmp(ctx->parsername, ctx->parser->name)) {
        free(ctx->parsername);
        ctx->parsername = strdup(ctx->parser->name);
    }

    /* Do the actual parsing */
    ctx->display = ctx->parser->read(fd, ctx->filename);
    if (ctx->display == NULL || ctx->display->tiles == 0) {
        free(ctx->parsername);
        ctx->parsername = strdup("");
        Perl_croak(aTHX_ "%s: error parsing piece", ctx->filename);
    }

    rewind(fd);
    fclose(fd);

    RETVAL = 1;
OUTPUT:
    RETVAL


void
DESTROY(ctx)
    piece_t *ctx;
CODE:
    piece_screen_free(ctx->display);
    free(ctx->parsername);
    free(ctx->filename);
    Safefree(ctx);

void
to_bmp(ctx, filename)
    piece_t *ctx;
    char *filename;
CODE:
    _to_image(ctx, filename, PIECE_IMAGE_BMP);

void
to_gd(ctx, filename)
    piece_t *ctx;
    char *filename;
CODE:
    _to_image(ctx, filename, PIECE_IMAGE_GD);

void
to_gif(ctx, filename)
    piece_t *ctx;
    char *filename;
CODE:
    _to_image(ctx, filename, PIECE_IMAGE_GIF);

void
to_jpg(ctx, filename)
    piece_t *ctx;
    char *filename;
CODE:
    _to_image(ctx, filename, PIECE_IMAGE_JPEG);

void
to_png(ctx, filename)
    piece_t *ctx;
    char *filename;
CODE:
    _to_image(ctx, filename, PIECE_IMAGE_PNG);

SV *
to_png_data(ctx)
    piece_t *ctx;
CODE:
    RETVAL = _to_image_buffer(ctx, PIECE_IMAGE_PNG);
OUTPUT:
    RETVAL

BOOT:
    Newx(piece_options, 1, piece_option_flags);
    memset(piece_options, 0, sizeof(piece_option_flags));
    piece_options->verbose = 0;

    piece_parser_init();
    piece_font_init();
    sauce_init();
    piece_palette_init();
