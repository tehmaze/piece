#ifndef __PARSER_ANSI_H
#define __PARSER_ANSI_H

#include "list.h"
#include "screen.h"

#define ANSI_WRAP    80
#define ANSI_TABSTOP 8

typedef enum {
    ANSI_STATE_TEXT,
    ANSI_STATE_CHECK_BRACE,
    ANSI_STATE_WAIT_LITERAL,
    ANSI_STATE_EXIT
} ansi_parser_state;

typedef struct ansi_sequence_s {
    unsigned char           sequence[256];
    struct ansi_sequence_s *next;
} ansi_sequence;

#define ANSI_SEQ(seq, i)    (((ansi_sequence *) list_get(seq, i))->sequence)
#define ANSI_SEQ_CC(seq, i) ((const char *) ANSI_SEQ(seq, i))

// Function prototypes

void ansi_parser_init();

#endif // __PARSER_ANSI_H
