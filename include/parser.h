#ifndef __PARSER_H
#define __PARSER_H

#include <stdio.h>

#include "list.h"
#include "screen.h"

typedef screen *(*parser_read_func)(const char *);
typedef void (*parser_render_func)(screen *, unsigned int);

typedef struct parser_extensions_s {
    char                **extensions;
} parser_extensions;

typedef struct parser_s {
    char                *name;
    char                *description;
    parser_read_func    read;
    parser_render_func  render;
    char                **extensions;
} parser;

// Function prototypes

void    parser_init(void);
void    parser_register(parser);
void    parser_iter(list_iterator);
parser *parser_for_type(const char *);
parser *parser_for(const char *);

#endif // __PARSER_H
