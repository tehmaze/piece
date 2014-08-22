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
    char                *palette;
} piece_image_option_flags;

typedef struct piece_target_option_flags_s {
    char                        *filename;
    piece_screen                *display;
    char                        *fontname;
    piece_font                  *font;
    char                        *writername;
    piece_writer                *writer;
    piece_image_option_flags    *image;
} piece_target_option_flags;

typedef struct piece_option_flags_s {
    char                    *program;
    int                         verbose;
    piece_source_option_flags   *source;
    piece_target_option_flags   *target;
} piece_option_flags;

extern piece_option_flags *piece_options;

#endif // __OPTIONS_H
