#ifndef __PIECE_PARSER_ANSI_H__
#define __PIECE_PARSER_ANSI_H__

#include "piece/list.h"
#include "piece/screen.h"

#define ANSI_WRAP    80
#define ANSI_TABSTOP 8

typedef enum {
    ANSI_STATE_TEXT,
    ANSI_STATE_CHECK_BRACE,
    ANSI_STATE_WAIT_LITERAL,
    ANSI_STATE_EXIT
} piece_ansi_parser_state;

typedef struct piece_ansi_sequence_s {
    unsigned char                   sequence[256];
    struct piece_ansi_sequence_s    *next;
} piece_ansi_sequence;

#define ANSI_SEQ(seq, i)    (((piece_ansi_sequence *) piece_list_get(seq, i))->sequence)
#define ANSI_SEQ_CC(seq, i) ((const char *) ANSI_SEQ(seq, i))

// Function prototypes

void piece_ansi_parser_init();

#endif // __PIECE_PARSER_ANSI_H__
