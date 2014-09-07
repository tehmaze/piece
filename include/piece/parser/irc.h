#ifndef PIECE_PARSER_IRC_H
#define PIECE_PARSER_IRC_H

#include "piece/list.h"
#include "piece/screen.h"

#define PIECE_IRC_BOLD          0x02    /* ^B */
#define PIECE_IRC_COLOR         0x03    /* ^C */
#define PIECE_IRC_COLOR_RGB     0x04
#define PIECE_IRC_FIXED         0x0f    /* ^O */
#define PIECE_IRC_REVERSE       0x12
#define PIECE_IRC_INVERSE       0x16    /* ^R */
#define PIECE_IRC_ITALIC        0x1d    /* ^N */
#define PIECE_IRC_UNDERLINE     0x1f    /* ^U */

#define PIECE_IRC_PROBE_MAX     0x400   /* First 1kB */
#define PIECE_IRC_PROBE_COLORS  4

// Function prototypes

void piece_irc_parser_init();

#endif // PIECE_PARSER_IRC_H
