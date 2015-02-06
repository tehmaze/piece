#ifndef __OPTIONS_H
#define __OPTIONS_H

#include <stdio.h>

#include "piece/font.h"
#include "piece/parser.h"
#include "piece/screen.h"
#include "piece/writer.h"

#define dprintf(fmt, ...) do {                  \
    if (piece_options->verbose) {               \
        fprintf(stdout, fmt, __VA_ARGS__);      \
    }                                           \
} while(0)

#define dfprintf(stream, fmt, ...) do {         \
    if (piece_options->verbose) {               \
        fprintf(stream, fmt, __VA_ARGS__);      \
    }                                           \
} while(0)

typedef struct piece_source_option_flags_s {
    char                *filename;
    char                *parsername;
    piece_parser        *parser;
} piece_source_option_flags;

typedef struct piece_image_option_flags_s {
    bool                transparent;
    piece_palette       *palette;
    char                *palette_name;
    unsigned char       ice_colors;
    bool                animate;
} piece_image_option_flags;

typedef struct piece_target_option_flags_s {
    char                        *filename;
    piece_screen                *display;
    piece_font                  *font;
    char                        *font_name;
    piece_writer                *writer;
    char                        *writer_name;
    FILE                        *fd;
    piece_image_option_flags    *image;
} piece_target_option_flags;

typedef struct piece_option_flags_s {
    char                        *program;
    int                         verbose;
    bool                        quiet;
    piece_source_option_flags   *source;
    piece_target_option_flags   *target;
} piece_option_flags;

extern piece_option_flags *piece_options;

#endif // __OPTIONS_H
