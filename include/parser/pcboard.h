#ifndef __PARSER_PCBOARD_H
#define __PARSER_PCBOARD_H

#include <stdint.h>

typedef enum {
    PCBOARD_STATE_TEXT,
    PCBOARD_STATE_OPERATOR,
    PCBOARD_STATE_EXIT
} pcboard_parser_state;

// Function prototypes

void pcboard_parser_init();

#endif // __PARSER_PCBOARD_H
