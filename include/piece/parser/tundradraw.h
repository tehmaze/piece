#ifndef __PIECE_PARSER_TUNDRADRAW_H__
#define __PIECE_PARSER_TUNDRADRAW_H__

#include <stdint.h>

#include "piece/list.h"

#define TUNDRADRAW_HEADER "TUNDRA24"
#define TUNDRADRAW_ROWS   5000
#define TUNDRADRAW_COLS   80

typedef struct piece_tundradraw_header_s {
    char version;
    char header[8];
} piece_tundradraw_header;

// Function prototypes

void piece_tundradraw_parser_init();

#endif // __PIECE_PARSER_TUNDRADRAW_H__
