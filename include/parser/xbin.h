#ifndef __PARSER_XBIN_H
#define __PARSER_XBIN_H

#include "list.h"

typedef struct xbin_flags_s {
    uint8_t     flag_palette   : 1;
    uint8_t     flag_font      : 1;
    uint8_t     flag_compress  : 1;
    uint8_t     flag_non_blink : 1;
    uint8_t     flag_512_chars : 1;
    uint8_t     unused         : 3;
} xbin_flags;

typedef struct xbin_header_s {
    char        id[4];
    uint8_t     eof_char;
    uint16_t    width;
    uint16_t    height;
    uint8_t     font_size;
    xbin_flags  flags;
} xbin_header;


// Function prototypes

void xbin_parser_init();

#endif // __PARSER_XBIN_H
