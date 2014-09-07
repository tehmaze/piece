#ifndef __PIECE_PARSER_H__
#define __PIECE_PARSER_H__

#include <stdio.h>

#include "piece/list.h"
#include "piece/screen.h"

typedef piece_screen *(*piece_parser_read_func)(FILE *, const char *);
typedef bool (*piece_parser_probe_func)(FILE *, const char *);

typedef struct piece_parser_extensions_s {
    char                    **extensions;
} piece_parser_extensions;

typedef struct piece_parser_sauce_s {
    uint8_t                 data_type;
    uint8_t                 file_type;
} piece_parser_sauce;

typedef struct piece_parser_s {
    char                    *name;
    char                    *description;
    piece_parser_probe_func probe;
    piece_parser_read_func  read;
    char                    **extensions;
    piece_parser_sauce      *saucetypes;
    char                    *font_name;
} piece_parser;

// Function prototypes

void            piece_parser_init(void);
void            piece_parser_free(void);
void            piece_parser_register(piece_parser *);
void            piece_parser_iter(piece_list_iterator);
piece_parser    *piece_parser_for_type(const char *);
piece_parser    *piece_parser_for(FILE *, const char *);

#endif // __PIECE_PARSER_H__
