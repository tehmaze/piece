#ifndef __PARSER_TUNDRADRAW_H
#define __PARSER_TUNDRADRAW_H

#include <stdint.h>
#include "list.h"

#define TUNDRADRAW_HEADER "TUNDRA24"
#define TUNDRADRAW_ROWS   5000
#define TUNDRADRAW_COLS   80

typedef struct tundradraw_header_s {
    char version;
    char header[8];
} tundradraw_header;

// Function prototypes

void tundradraw_parser_init();

#endif // __PARSER_TUNDRADRAW_H
