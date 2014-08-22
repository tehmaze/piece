#ifndef __PIECE_PARSER_XBIN_H__
#define __PIECE_PARSER_XBIN_H__

#include "piece/list.h"

#define XBIN_ID         "XBIN"
#define XBIN_ID_LEN     4

#define XBIN_COMP_NONE  0x00
#define XBIN_COMP_CHAR  0x40
#define XBIN_COMP_ATTR  0x80
#define XBIN_COMP_BOTH  0xc0

typedef struct xbin_flags_s {
    uint8_t     flag_palette   : 1;
    uint8_t     flag_font      : 1;
    uint8_t     flag_compress  : 1;
    uint8_t     flag_non_blink : 1;
    uint8_t     flag_512_chars : 1;
    uint8_t     unused         : 3;
} __attribute__((packed)) xbin_flags;

typedef struct xbin_header_s {
    char        id[4];
    uint8_t     eof_char;
    uint16_t    width;
    uint16_t    height;
    uint8_t     font_size;
    xbin_flags  flags;
} __attribute__((packed)) xbin_header;


// Function prototypes

void piece_xbin_parser_init();

#endif // __PIECE_PARSER_XBIN_H__
