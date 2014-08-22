#ifndef __PIECE_PARSER_PCBOARD_H__
#define __PIECE_PARSER_PCBOARD_H__

#include <stdint.h>

typedef enum {
    PIECE_PCBOARD_STATE_TEXT,
    PIECE_PCBOARD_STATE_OPERATOR,
    PIECE_PCBOARD_STATE_EXIT
} piece_pcboard_parser_state;

// Function prototypes

void piece_pcboard_parser_init();

#endif // __PIECE_PARSER_PCBOARD_H__
