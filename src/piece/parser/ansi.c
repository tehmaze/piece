#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "piece/list.h"
#include "piece/options.h"
#include "piece/parser.h"
#include "piece/parser/ansi.h"
#include "piece/palette.h"
#include "piece/screen.h"
#include "piece/util.h"
#include "sauce.h"

// Function prototype

void piece_ansi_parser_parse_sgr(piece_screen *display, uint32_t sgr);
void piece_ansi_parser_parse_sgr256(piece_screen *display, uint32_t sgr, uint32_t c);

static piece_ansi_sequence *sequence_new(void) {
    piece_ansi_sequence *sequence = piece_allocate(sizeof(piece_ansi_sequence));
    memset(sequence, 0, sizeof(piece_ansi_sequence));
    return sequence;
}

static void sequence_free(void *sequence) {
    free(sequence);
}

piece_screen *piece_ansi_parser_read(FILE *fd, const char *filename)
{
    piece_screen *display = NULL;
    sauce *record = NULL;
    unsigned int width = 80, height = 25;
    long fsize;
    piece_ansi_parser_state state = ANSI_STATE_TEXT;
    unsigned char ch;
    piece_list *sequences = NULL;
    piece_list_node *node = NULL;
    unsigned char argbuf[256];
    int arg_index = 0;
    int count = 0;
    int x = 0;
    int y = 0;
    int x_saved = 0;
    int y_saved = 0;
    int i;
    char *feature;

    char *extension = piece_get_extension(filename);
    if (!strcmp(extension, "diz") ||
        !strcmp(extension, "ion")) {
        width = 45;
    }
    free(extension);

    record = sauce_read(fd);
    if (record != NULL) {
        if (record->data_type == SAUCE_DATA_TYPE_CHARACTER && (
            record->file_type == SAUCE_FILE_TYPE_ASCII      ||
            record->file_type == SAUCE_FILE_TYPE_ANSI       ||
            record->file_type == SAUCE_FILE_TYPE_ANSIMATION
            )) {
            width = (unsigned int) record->tinfo[0];
            height = (unsigned int) record->tinfo[1];
        }
    } else {
        record = piece_allocate(sizeof(sauce));
        record->flags.flag_ls = SAUCE_LS_9PIXEL;
    }

    display = piece_screen_create(width, height, record);
    display->palette = piece_palette_by_name("ega");
    if (display == NULL) {
        fprintf(stderr, "%s: could not piece_allocate %dx%d piece_screen buffer\n",
                        filename, width, height);
        sauce_free(display->record);
        return NULL;
    }

    if (fseek(fd, 0, SEEK_END)) {
        fprintf(stderr, "%s: error seeking to end of file\n", filename);
        sauce_free(display->record);
        return NULL;
    }

    fsize = ftell(fd);
    rewind(fd);

    sequences = piece_allocate(sizeof(piece_list));
    piece_list_new(sequences, sequence_free);
    while (state != ANSI_STATE_EXIT &&
           (ch = fgetc(fd)) != 0x00 &&
           !feof(fd) &&
           ftell(fd) <= fsize
           ) {

        switch (state) {
            case ANSI_STATE_TEXT:
                switch(ch) {
                    case '\x1a':
                        state = ANSI_STATE_EXIT;
                        break;

                    case '\x1b':
                        state = ANSI_STATE_CHECK_BRACE;
                        break;

                    case '\n':
                        x = 0;
                        y++;

                    case '\r':
                        break;

                    case '\t':
                        count = (x + 1) % ANSI_TABSTOP;
                        if (count) {
                            count = ANSI_TABSTOP - count;
                            for (i = 0; i < count; ++i) {
                                piece_screen_putchar(display, ' ', &x, &y, true);
                            }
                        }
                        break;

                    default:
                        piece_screen_putchar(display, ch, &x, &y, true);
                        break;
                }
                break;

            case ANSI_STATE_CHECK_BRACE:
                if (ch == '[') {
                    state = ANSI_STATE_WAIT_LITERAL;
                } else {
                    piece_screen_putchar(display, '\x1b', &x, &y, true);
                    piece_screen_putchar(display, ch, &x, &y, true);
                }
                break;

            case ANSI_STATE_WAIT_LITERAL:
                if (isalpha(ch) || ch == ';') {
                    argbuf[arg_index] = '\x00';
                    piece_ansi_sequence *sequence = sequence_new();
                    memcpy(sequence->sequence, argbuf, 256);
                    piece_list_append(sequences, sequence);
                    arg_index = 0;
                    if (ch == ';') {
                        break;
                    }
                }

                if (isalpha(ch)) {
                    switch (ch) {
                        case 'm':
                            if (sequences != NULL) {
                                i = atoi(ANSI_SEQ_CC(sequences, 0));
                                if (piece_list_size(sequences) == 3 &&
                                    (i == 38 || i == 48) &&
                                    atoi(ANSI_SEQ_CC(sequences, 1)) == 5) {
                                    piece_ansi_parser_parse_sgr256(
                                        display,
                                        i,
                                        atoi(ANSI_SEQ_CC(sequences, 2))
                                    );
                                } else {
                                    node = sequences->head;
                                    while (node != NULL) {
                                        piece_ansi_sequence *curr = (piece_ansi_sequence *) node->data;
                                        uint32_t sgr = atoi((const char *) curr->sequence);
                                        piece_ansi_parser_parse_sgr(display, sgr);
                                        node = node->next;
                                    }
                                }
                            }
                            break;

                        case 'H':   // CUP
                        case 'f':
                            switch (piece_list_size(sequences)) {
                                case 2:
                                    y = atoi(ANSI_SEQ_CC(sequences, 0)) - 1;
                                    x = atoi(ANSI_SEQ_CC(sequences, 1)) - 1;
                                    break;
                                case 1:
                                    y = 0;
                                    x = atoi(ANSI_SEQ_CC(sequences, 0)) - 1;
                                    break;
                                default:
                                    y = 0;
                                    x = 0;
                                    break;
                            }
                            piece_list_free(sequences);
                            sequences = NULL;
                            break;

                        case 'A':   // CUU (CUrsor Up)
                            i = (piece_list_size(sequences) > 0)
                                ? atoi(ANSI_SEQ_CC(sequences, 0))
                                : 1;
                            y -= i;
                            if (y < 0) y = 0;
                            break;

                        case 'B':   // CUD (CUrsor Down)
                            i = (piece_list_size(sequences) > 0)
                                ? atoi(ANSI_SEQ_CC(sequences, 0))
                                : 1;
                            y += i;
                            break;

                        case 'C':   // CUF (CUrsor Forward)
                            i = (piece_list_size(sequences) > 0)
                                ? atoi(ANSI_SEQ_CC(sequences, 0))
                                : 1;
                            x += i;
                            break;

                        case 'D':   // CUB (CUrsor Back)
                            i = (piece_list_size(sequences) > 0)
                                ? atoi(ANSI_SEQ_CC(sequences, 0))
                                : 1;
                            x -= i;
                            if (x < 0) x = 0;
                            break;

                        case 'E':   // CNL (Cursor Next Line)
                            i = (piece_list_size(sequences) > 0)
                                ? atoi(ANSI_SEQ_CC(sequences, 0))
                                : 1;
                            x = 0;
                            y += i;
                            break;

                        case 'F':   // CPL (Cursor Previous Line)
                            i = (piece_list_size(sequences) > 0)
                                ? atoi(ANSI_SEQ_CC(sequences, 0))
                                : 1;
                            x = 0;
                            y -= i;
                            if (y < 0) y = 0;
                            break;

                        case 'G':   // CHA (Cursor Horizontal Absolute)
                            i = (piece_list_size(sequences) > 0)
                                ? atoi(ANSI_SEQ_CC(sequences, 0))
                                : 1;
                            x = i - 1;
                            if (x < 0) x = 0;
                            break;

                        case 'J':   // ED (Erase Display)
                            i = (piece_list_size(sequences) > 0)
                                ? atoi(ANSI_SEQ_CC(sequences, 0))
                                : 0;

                            switch (i) {
                                case 0:     // From cursor to EOF
                                    for (i = (display->size.width * y) + x;
                                         i < display->tiles; ++i) {
                                         piece_screen_tile_reset(&display->tile[i]);
                                    }
                                    break;

                                case 1:     // From cursor to start
                                    for (i = (display->size.width * y) + x;
                                         i >= 0; --i) {
                                         piece_screen_tile_reset(&display->tile[i]);
                                    }
                                    break;

                                case 2:     // Entire piece_screen
                                default:
                                    piece_screen_reset(display);
                                    piece_screen_reduce(display, width, height);
                                    y = 0;
                                    x = 0;
                            }
                            break;

                        case 'K':   // EL (Erase in Line)
                            i = (piece_list_size(sequences) > 0)
                                ? atoi(ANSI_SEQ_CC(sequences, 0))
                                : 0;

                            int64_t start, end;
                            switch (i) {
                                case 0:     // To EOL
                                    start = (display->size.width * y) + x;
                                    end = (display->size.width * (y + 1)) - 1;
                                    break;
                                case 1:     // To BOL
                                    start = (display->size.width * (y - 1)) + 1;
                                    end = (display->size.width * y) + x;
                                    break;
                                case 2:     // Entire line
                                    start = (display->size.width * (y - 1)) + 1;
                                    end = (display->size.width * (y + 1)) - 1;
                                    break;
                            }

                            if (start < 0) {
                                start = 0;
                            }
                            if (end > display->tiles) {
                                end = display->tiles - 1;
                            }

                            for (i = start; i < end; ++i) {
                                piece_screen_tile_reset(&display->tile[i]);
                            }

                            break;

                        case 'L':   // IL (Insert Line)
                            i = (piece_list_size(sequences) > 0)
                                ? atoi(ANSI_SEQ_CC(sequences, 0))
                                : 1;
                            for (; i > 0; i--) {
                                piece_screen_insert_line(display, y);
                            }
                            break;

                        case 'S':   // SU (Scroll Up) FIXME
                            i = (piece_list_size(sequences) > 0)
                                ? atoi(ANSI_SEQ_CC(sequences, 0))
                                : 1;
                            break;

                        case 'T':   // SD (Scroll Down) FIXME
                            i = (piece_list_size(sequences) > 0)
                                ? atoi(ANSI_SEQ_CC(sequences, 0))
                                : 1;
                            break;

                        case 'h':   // Feature on
                            feature = (piece_list_size(sequences) > 0)
                                ? (char *) ANSI_SEQ_CC(sequences, 0)
                                : "";

                            if (!strcmp(feature, "?33")) {  // Blink mode
                                display->record->flags.flag_b = 1;
                            }
                            break;

                        case 'i':   // AUX (serial port toggle)
                            break;  // ignored

                        case 'l':   // Feature off
                            feature = (piece_list_size(sequences) > 0)
                                ? (char *) ANSI_SEQ_CC(sequences, 0)
                                : "";

                            if (!strcmp(feature, "?33")) {  // Blink mode
                                display->record->flags.flag_b = 0;
                            }
                            break;

                        case 's':   // SCP (Save Cursor Position)
                            y_saved = y;
                            x_saved = x;
                            break;

                        case 'u':   // RCP (Restore Cursor Position)
                            y = y_saved;
                            x = x_saved;
                            break;

                        default:
                            fprintf(stderr, "%s: invalid sequence ", filename);
                            fprintf(stderr, "<ESC>[");
                            if (sequences != NULL) {
                                node = sequences->head;
                                while (node != NULL) {
                                    piece_ansi_sequence *curr = (piece_ansi_sequence *) node->data;
                                    fprintf(stderr, "%s", curr->sequence);
                                    if (node->next != NULL) {
                                        fprintf(stderr, ";");
                                    }
                                    node = node->next;
                                }
                                fprintf(stderr, "%c\n", ch);
                            }
                    }

                    if (sequences == NULL) {
                        sequences = piece_allocate(sizeof(piece_list));
                        piece_list_new(sequences, sequence_free);
                    }
                    else if (piece_list_size(sequences) > 0) {
                        piece_list_free(sequences);
                        sequences = piece_allocate(sizeof(piece_list));
                        piece_list_new(sequences, sequence_free);
                    }

                    state = ANSI_STATE_TEXT;
                    break;
                } // if (isalpha(ch))

                argbuf[arg_index] = ch;
                arg_index++;
                break;

            case ANSI_STATE_EXIT:        // We're done here
            default:
                break;
        }
    }

    // We're done here
    piece_list_free(sequences);

    return display;
}

void piece_ansi_parser_parse_sgr(piece_screen *display, uint32_t sgr) {
    switch (sgr) {
        case 0:
            display->current->bg = PIECE_TILE_DEFAULT_BG;
            display->current->fg = PIECE_TILE_DEFAULT_FG;
            display->current->attrib = PIECE_ATTRIB_DEFAULT;
            break;

        case 1:
            PIECE_ATTRIB_SET(display->current->attrib, PIECE_ATTRIB_BOLD);
            break;

        case 3:
            PIECE_ATTRIB_SET(display->current->attrib, PIECE_ATTRIB_ITALICS);
            break;

        case 4:
            PIECE_ATTRIB_SET(display->current->attrib, PIECE_ATTRIB_UNDERLINE);
            break;

        case 5:
        case 6:
            PIECE_ATTRIB_SET(display->current->attrib, PIECE_ATTRIB_BLINK);
            break;

        case 7:
            PIECE_ATTRIB_SET(display->current->attrib, PIECE_ATTRIB_NEGATIVE);
            break;

        case 8:
            PIECE_ATTRIB_SET(display->current->attrib, PIECE_ATTRIB_CONCEAL);
            break;

        case 9:
            PIECE_ATTRIB_SET(display->current->attrib, PIECE_ATTRIB_CROSSED_OUT);
            break;

        case 21:
            PIECE_ATTRIB_UNSET(display->current->attrib, PIECE_ATTRIB_BOLD);
            break;

        case 22:
            PIECE_ATTRIB_UNSET(display->current->attrib, PIECE_ATTRIB_BOLD);
            break;

        case 24:
            PIECE_ATTRIB_UNSET(display->current->attrib, PIECE_ATTRIB_UNDERLINE);
            break;

        case 25:
            PIECE_ATTRIB_UNSET(display->current->attrib, PIECE_ATTRIB_BLINK);
            break;

        case 27:
            PIECE_ATTRIB_UNSET(display->current->attrib, PIECE_ATTRIB_NEGATIVE);
            break;

        case 28:
            PIECE_ATTRIB_UNSET(display->current->attrib, PIECE_ATTRIB_CONCEAL);
            break;

        case 29:
            PIECE_ATTRIB_UNSET(display->current->attrib, PIECE_ATTRIB_CROSSED_OUT);
            break;

        case 30:
        case 31:
        case 32:
        case 33:
        case 34:
        case 35:
        case 36:
        case 37:
            display->current->fg = sgr - 30;
            break;

        case 38:
            PIECE_ATTRIB_SET(display->current->attrib, PIECE_ATTRIB_FG_256);
            break;

        case 39:
            display->current->fg = PIECE_TILE_DEFAULT_FG;
            break;

        case 40:
        case 41:
        case 42:
        case 43:
        case 44:
        case 45:
        case 46:
        case 47:
            display->current->bg = sgr - 40;
            break;

        case 48:
            PIECE_ATTRIB_SET(display->current->attrib, PIECE_ATTRIB_BG_256);
            break;

        case 49:
            display->current->bg = PIECE_TILE_DEFAULT_BG;
            break;

        default:
            // Ignore
            break;
    }
}

void piece_ansi_parser_parse_sgr256(piece_screen *display, uint32_t sgr, uint32_t c)
{
    switch (sgr) {
        case 38:
            PIECE_ATTRIB_SET(display->current->attrib, PIECE_ATTRIB_FG_256);
            display->current->fg = c;
            break;
        case 48:
            PIECE_ATTRIB_SET(display->current->attrib, PIECE_ATTRIB_BG_256);
            display->current->bg = c;
            break;
    }
}

static char *piece_ansi_extensions[] = {
    "ans",
    NULL
};

static piece_parser piece_ansi_parser = {
    "ansi",
    "ANSi coloring codes and cursor positioning",
    NULL,
    piece_ansi_parser_read,
    piece_ansi_extensions,
    "cp437_8x16"
};

void piece_ansi_parser_init() {
    piece_parser_register(&piece_ansi_parser);
}
