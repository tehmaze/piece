#ifndef __OPTIONS_H
#define __OPTIONS_H

#include <stdio.h>

#include "font.h"
#include "parser.h"
#include "screen.h"
#include "writer.h"

#define dprintf(fmt, ...) do {                      \
        if (options->verbose) {                     \
            fprintf(stdout, fmt, __VA_ARGS__);      \
        }                                           \
    } while(0)

#define dfprintf(stream, fmt, ...) do {             \
        if (options->verbose) {                     \
            fprintf(stream, fmt, __VA_ARGS__);      \
        }                                           \
    } while(0)

typedef struct source_option_flags_s {
    char                *filename;
    char                *parsername;
    parser              *parser;
} source_option_flags;

typedef struct image_option_flags_s {
    bool                transparent;
    char                *palette;
} image_option_flags;

typedef struct target_option_flags_s {
    char                *filename;
    screen              *display;
    char                *fontname;
    font                *font;
    char                *writername;
    writer              *writer;
    image_option_flags  *image;
} target_option_flags;

typedef struct option_flags_s {
    char                *program;
    int                 verbose;
    source_option_flags *source;
    target_option_flags *target;
} option_flags;

extern option_flags *options;

#endif // __OPTIONS_H
