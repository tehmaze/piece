#ifndef __PARSER_H
#define __PARSER_H

#include <stdio.h>

#include "list.h"
#include "screen.h"

typedef screen *(*parser_read_func)(FILE *, const char *);
typedef bool (*parser_probe_func)(FILE *, const char *);

typedef struct parser_extensions_s {
    char                **extensions;
} parser_extensions;

typedef struct parser_s {
    char                *name;
    char                *description;
    parser_probe_func   probe;
    parser_read_func    read;
    char                **extensions;
    char                *font_name;
} parser;

// Function prototypes

void    parser_init(void);
void    parser_free(void);
void    parser_register(parser *);
void    parser_iter(list_iterator);
parser *parser_for_type(const char *);
parser *parser_for(FILE *, const char *);

#endif // __PARSER_H
