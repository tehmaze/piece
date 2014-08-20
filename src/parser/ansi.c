#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "parser.h"
#include "parser/ansi.h"
#include "palette.h"
#include "sauce.h"
#include "screen.h"
#include "util.h"

// Function prototype

void ansi_parser_parse_sgr(screen *display, uint32_t sgr);
void ansi_parser_parse_sgr256(screen *display, uint32_t sgr, uint32_t c);

static ansi_sequence *sequence_new(void) {
    ansi_sequence *sequence = allocate(sizeof(ansi_sequence));
    memset(sequence, 0, sizeof(ansi_sequence));
    return sequence;
}

screen *ansi_parser_read(const char *filename)
{
    FILE *fd = NULL;
    screen *display = NULL;
    sauce *record = NULL;
    unsigned int width = 80, height = 25;
    long fsize;
    ansi_parser_state state = STATE_TEXT;
    unsigned char ch;
    list *sequences = NULL;
    list_node *node = NULL;
    unsigned char argbuf[256];
    int arg_index = 0;
    int count = 0;
    int x = 0;
    int y = 0;
    int x_saved = 0;
    int y_saved = 0;
    int64_t i, cursor;
    char *feature;

    char *extension = get_extension(filename);
    if (!strcmp(extension, "diz") ||
        !strcmp(extension, "ion")) {
        width = 45;
    }
    free(extension);

    fd = fopen(filename, "r");
    if (fd == NULL) {
        fprintf(stderr, "%s: error opening\n", filename);
        return NULL;
    }

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
        record = allocate(sizeof(sauce));
        record->flags.flag_ls = SAUCE_LS_9PIXEL;
    }

    display = screen_create(width, height, record);
    display->palette = palette_by_name("vga");
    if (display == NULL) {
        fprintf(stderr, "%s: could not allocate %dx%d screen buffer\n",
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

    sequences = allocate(sizeof(list));
    list_new(sequences, NULL);
    while (state != STATE_EXIT &&
           (ch = fgetc(fd)) != 0x00 &&
           !feof(fd) &&
           ftell(fd) <= fsize
           ) {

        switch (state) {
            case STATE_TEXT:
                switch(ch) {
                    case '\x1a':
                        state = STATE_EXIT;
                        break;

                    case '\x1b':
                        state = STATE_CHECK_BRACE;
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
                                screen_putchar(display, ' ', &x, &y);
                            }
                        }
                        break;

                    default:
                        screen_putchar(display, ch, &x, &y);
                        break;
                }
                break;

            case STATE_CHECK_BRACE:
                if (ch == '[') {
                    state = STATE_WAIT_LITERAL;
                } else {
                    screen_putchar(display, '\x1b', &x, &y);
                    screen_putchar(display, ch, &x, &y);
                }
                break;

            case STATE_WAIT_LITERAL:
                if (isalpha(ch) || ch == ';') {
                    argbuf[arg_index] = '\x00';
                    ansi_sequence *sequence = sequence_new();
                    memcpy(sequence->sequence, argbuf, 256);
                    list_append(sequences, sequence);
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
                                if (list_size(sequences) == 3 &&
                                    (i == 38 || i == 48) &&
                                    atoi(ANSI_SEQ_CC(sequences, 1)) == 5) {
                                    ansi_parser_parse_sgr256(
                                        display,
                                        i,
                                        atoi(ANSI_SEQ_CC(sequences, 2))
                                    );
                                } else {
                                    node = sequences->head;
                                    while (node != NULL) {
                                        ansi_sequence *curr = (ansi_sequence *) node->data;
                                        uint32_t sgr = atoi((const char *) curr->sequence);
                                        ansi_parser_parse_sgr(display, sgr);
                                        node = node->next;
                                    }
                                }
                            }
                            break;

                        case 'H':   // CUP
                        case 'f':
                            switch (list_size(sequences)) {
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
                            list_free(sequences);
                            sequences = NULL;
                            break;

                        case 'A':   // CUU (CUrsor Up)
                            i = (list_size(sequences) > 0)
                                ? atoi(ANSI_SEQ_CC(sequences, 0))
                                : 1;
                            y -= i;
                            if (y < 0) y = 0;
                            break;

                        case 'B':   // CUD (CUrsor Down)
                            i = (list_size(sequences) > 0)
                                ? atoi(ANSI_SEQ_CC(sequences, 0))
                                : 1;
                            y += i;
                            break;

                        case 'C':   // CUF (CUrsor Forward)
                            i = (list_size(sequences) > 0)
                                ? atoi(ANSI_SEQ_CC(sequences, 0))
                                : 1;
                            x += i;
                            break;

                        case 'D':   // CUB (CUrsor Back)
                            i = (list_size(sequences) > 0)
                                ? atoi(ANSI_SEQ_CC(sequences, 0))
                                : 1;
                            x -= i;
                            if (x < 0) x = 0;
                            break;

                        case 'E':   // CNL (Cursor Next Line)
                            i = (list_size(sequences) > 0)
                                ? atoi(ANSI_SEQ_CC(sequences, 0))
                                : 1;
                            x = 0;
                            y += i;
                            break;

                        case 'F':   // CPL (Cursor Previous Line)
                            i = (list_size(sequences) > 0)
                                ? atoi(ANSI_SEQ_CC(sequences, 0))
                                : 1;
                            x = 0;
                            y -= i;
                            if (y < 0) y = 0;
                            break;

                        case 'G':   // CHA (Cursor Horizontal Absolute)
                            i = (list_size(sequences) > 0)
                                ? atoi(ANSI_SEQ_CC(sequences, 0))
                                : 1;
                            x = i - 1;
                            if (x < 0) x = 0;
                            break;

                        case 'J':   // ED (Erase Display)
                            i = (list_size(sequences) > 0)
                                ? atoi(ANSI_SEQ_CC(sequences, 0))
                                : 0;

                            switch (i) {
                                case 0:     // From cursor to EOF
                                    node = display->tiles->head;
                                    cursor = (display->width * y) + x;
                                    for (i = 0; i < cursor; i++) {
                                        node = node->next;
                                    }
                                    while (node != NULL) {
                                        screen_tile_reset(node->data);
                                    }
                                    break;

                                case 1:     // From cursor to start
                                    node = display->tiles->head;
                                    cursor = (display->width * y) + x;
                                    for (i = 0; i < cursor; ++i) {
                                        screen_tile_reset(node->data);
                                    }
                                    break;

                                case 2:     // Entire screen
                                default:
                                    node = display->tiles->head;
                                    while (node != NULL) {
                                        screen_tile_reset(node->data);
                                        node = node->next;
                                    }
                                    screen_reduce(display, width, height);
                                    y = 0;
                                    x = 0;
                            }
                            break;

                        case 'K':   // EL (Erase in Line)
                            i = (list_size(sequences) > 0)
                                ? atoi(ANSI_SEQ_CC(sequences, 0))
                                : 0;

                            int64_t start, end;
                            switch (i) {
                                case 0:     // To EOL
                                    start = (display->width * y) + x;
                                    end = (display->width * (y + 1)) - 1;
                                    break;
                                case 1:     // To BOL
                                    start = (display->width * (y - 1)) + 1;
                                    end = (display->width * y) + x;
                                    break;
                                case 2:     // Entire line
                                    start = (display->width * (y - 1)) + 1;
                                    end = (display->width * (y + 1)) - 1;
                                    break;
                            }

                            if (start < 0) {
                                start = 0;
                            }
                            if (end > list_size(display->tiles)) {
                                end = list_size(display->tiles) - 1;
                            }

                            node = display->tiles->head;
                            for (i = 0; i < start; ++i) {
                                node = node->next;
                            }
                            for (i = start; i < end; ++i) {
                                screen_tile_reset(node);
                                node = node->next;
                            }

                            break;

                        case 'L':   // IL (Insert Line)
                            i = (list_size(sequences) > 0)
                                ? atoi(ANSI_SEQ_CC(sequences, 0))
                                : 1;
                            for (; i > 0; i--) {
                                screen_insert_line(display, y);
                            }
                            break;

                        case 'S':   // SU (Scroll Up) FIXME
                            i = (list_size(sequences) > 0)
                                ? atoi(ANSI_SEQ_CC(sequences, 0))
                                : 1;
                            break;

                        case 'T':   // SD (Scroll Down) FIXME
                            i = (list_size(sequences) > 0)
                                ? atoi(ANSI_SEQ_CC(sequences, 0))
                                : 1;
                            break;

                        case 'h':   // Feature on
                            feature = (list_size(sequences) > 0)
                                ? (char *) ANSI_SEQ_CC(sequences, 0)
                                : "";

                            if (!strcmp(feature, "?33")) {  // Blink mode
                                display->record->flags.flag_b = 1;
                            }
                            break;

                        case 'i':   // AUX (serial port toggle)
                            break;  // ignored

                        case 'l':   // Feature off
                            feature = (list_size(sequences) > 0)
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
                                    ansi_sequence *curr = (ansi_sequence *) node->data;
                                    fprintf(stderr, "%s", curr->sequence);
                                    if (node->next != NULL) {
                                        fprintf(stderr, ";");
                                    }
                                    node = node->next;
                                }
                                fprintf(stderr, "%c\n", ch);
                            }
                    }

                    if (list_size(sequences) > 0) {
                        list_free(sequences);
                        sequences = allocate(sizeof(list));
                        list_new(sequences, NULL);
                    }

                    state = STATE_TEXT;
                    break;
                } // if (isalpha(ch))

                argbuf[arg_index] = ch;
                arg_index++;
                break;

            case STATE_EXIT:        // We're done here
            default:
                break;
        }
    }

    list_free(sequences);

    // We're done here
    rewind(fd);
    fclose(fd);

    return display;
}

void ansi_parser_parse_sgr(screen *display, uint32_t sgr) {
    switch (sgr) {
        case 0:
            display->current->bg = TILE_DEFAULT_BG;
            display->current->fg = TILE_DEFAULT_FG;
            display->current->attrib = ATTRIB_DEFAULT;
            break;

        case 1:
            ATTRIB_SET(display->current->attrib, ATTRIB_BOLD);
            break;

        case 3:
            ATTRIB_SET(display->current->attrib, ATTRIB_ITALICS);
            break;

        case 4:
            ATTRIB_SET(display->current->attrib, ATTRIB_UNDERLINE);
            break;

        case 5:
        case 6:
            ATTRIB_SET(display->current->attrib, ATTRIB_BLINK);
            break;

        case 7:
            ATTRIB_SET(display->current->attrib, ATTRIB_NEGATIVE);
            break;

        case 8:
            ATTRIB_SET(display->current->attrib, ATTRIB_CONCEAL);
            break;

        case 9:
            ATTRIB_SET(display->current->attrib, ATTRIB_CROSSED_OUT);
            break;

        case 21:
            ATTRIB_UNSET(display->current->attrib, ATTRIB_BOLD);
            break;

        case 22:
            ATTRIB_UNSET(display->current->attrib, ATTRIB_BOLD);
            break;

        case 24:
            ATTRIB_UNSET(display->current->attrib, ATTRIB_UNDERLINE);
            break;

        case 25:
            ATTRIB_UNSET(display->current->attrib, ATTRIB_BLINK);
            break;

        case 27:
            ATTRIB_UNSET(display->current->attrib, ATTRIB_NEGATIVE);
            break;

        case 28:
            ATTRIB_UNSET(display->current->attrib, ATTRIB_CONCEAL);
            break;

        case 29:
            ATTRIB_UNSET(display->current->attrib, ATTRIB_CROSSED_OUT);
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
            ATTRIB_SET(display->current->attrib, ATTRIB_FG_256);
            break;

        case 39:
            display->current->fg = TILE_DEFAULT_FG;
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
            ATTRIB_SET(display->current->attrib, ATTRIB_BG_256);
            break;

        case 49:
            display->current->bg = TILE_DEFAULT_BG;
            break;

        default:
            // Ignore
            break;
    }
}

void ansi_parser_parse_sgr256(screen *display, uint32_t sgr, uint32_t c)
{
    switch (sgr) {
        case 38:
            ATTRIB_SET(display->current->attrib, ATTRIB_FG_256);
            display->current->fg = c;
            break;
        case 48:
            ATTRIB_SET(display->current->attrib, ATTRIB_BG_256);
            display->current->bg = c;
            break;
    }
}

void ansi_parser_render(screen *display, unsigned int output_type)
{
    printf("cursor %lu, type %d", display->cursor, output_type);
}


static char *ansi_extensions[] = {
    "ans",
    NULL
};
static parser ansi_parser = {
    "ansi",
    "ANSi coloring codes and cursor positioning",
    ansi_parser_read,
    ansi_parser_render,
    ansi_extensions
};

void ansi_parser_init() {
    parser_register(&ansi_parser);
}
