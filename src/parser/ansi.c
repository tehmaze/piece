#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "parser.h"
#include "parser/ansi.h"
#include "sauce.h"
#include "screen.h"
#include "util.h"


typedef enum {
    STATE_TEXT,
    STATE_CHECK_BRACE,
    STATE_WAIT_LITERAL,
    STATE_EXIT
} ansi_parser_state;


// Function prototype

void ansi_parser_parse_sgr(screen *display, uint32_t sgr);

screen *ansi_parser_read(const char *filename)
{
    FILE *fd = NULL;
    screen *display = NULL;
    sauce *record = NULL;
    unsigned int width = 80, height = 25;
    size_t fsize, rsize;
    ansi_parser_state state = STATE_TEXT;
    //unsigned char *buffer = NULL;

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
    }

    display = screen_create(width, height, record);
    if (display == NULL) {
        fprintf(stderr, "%s: could not allocate %dx%d screen display->buffer\n",
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

    display->buffer = malloc(sizeof(unsigned char) * fsize);
    if (display->buffer == NULL) {
        fprintf(stderr, "%s: error allocating %lu bytes\n",
                        filename,
                        sizeof(unsigned char) * fsize);
        sauce_free(display->record);
        return NULL;
    }

    rsize = fread(display->buffer, 1, fsize, fd);
    if (rsize != fsize) {
        free(display->buffer);
        fprintf(stderr, "%s: error in fread()\n", filename);
        sauce_free(display->record);
        return NULL;
    }

    // We're done here
    rewind(fd);
    fclose(fd);

    size_t position = 0;
    int32_t x = 0, y = 0, x_max = 0, y_max = 0, x_saved = 0, y_saved = 0;
    unsigned char curr_char, next_char, ansi_char;
    uint32_t ansi_sequence_position, sequence_count, sequence_position, i;
    char *sequence_bytes;
    char **sequence_array;

    while (state != STATE_EXIT && position < fsize) {
        curr_char = display->buffer[position];
        next_char = display->buffer[position + 1];

        if (x == 80) {
            y++;
            x = 0;
        }

        switch (curr_char) {
            case 0x0d:                      // CR
                if (next_char == 0x0a) {    // LF
                    y++;
                    x = 0;
                    position++;
                }
                break;

            case 0x0a:                      // LF
                y++;
                x = 0;
                break;
        
            case 0x09:                      // HT
                x += 8;
                break;

            case 0x1a:                      // SUB
                position = fsize - 1;
                break;

            case 0x1b:                      // ESC
                if (next_char == '[') {     // - ANSI Sequence
                    for (ansi_sequence_position = 0;
                         ansi_sequence_position < 12;
                         ansi_sequence_position++) {
                         
                         ansi_char = display->buffer[position + 2 +
                                            ansi_sequence_position];
                         
                         if (ansi_char == 'H' ||    // CUP
                             ansi_char == 'f') {    // 
                            sequence_bytes = substr((char *)display->buffer,
                                                    position + 2,
                                                    ansi_sequence_position);
                            sequence_count = split(&sequence_array, ';',
                                                    sequence_bytes);
                            
                            if (sequence_count > 0) {
                                y = atoi(sequence_array[0]) - 1;
                                x = atoi(sequence_array[1]) - 1;
                            } else {
                                y = 0;
                                x = 0;
                            }

                            position += ansi_sequence_position + 2;
                            for (i = 0; i < sequence_count; ++i) {
                                free(sequence_array[i]);
                            }
                            free(sequence_array);
                            free(sequence_bytes);
                            break;
                         }

                         if (ansi_char == 'A') {    // CUU (Cursor Up)
                            sequence_bytes = substr((char *)display->buffer,
                                                    position + 2,
                                                    ansi_sequence_position);
                            int32_t line = atoi(sequence_bytes);
                            if (line == 0) {
                                line = 1;
                            }

                            y -= line;
                            position += ansi_sequence_position + 2;
                            free(sequence_bytes);
                            break;
                         }

                         if (ansi_char == 'B') {    // CUD (Cursor Down)
                            sequence_bytes = substr((char *)display->buffer,
                                                    position + 2,
                                                    ansi_sequence_position);
                            int32_t line = atoi(sequence_bytes);
                            if (line == 0) {
                                line = 1;
                            }

                            y += line;
                            position += ansi_sequence_position + 2;
                            free(sequence_bytes);
                            break;
                         }

                         if (ansi_char == 'C') {    // CUF (Cursor Forward)
                            sequence_bytes = substr((char *)display->buffer,
                                                    position + 2,
                                                    ansi_sequence_position);
                            int32_t col = atoi(sequence_bytes);
                            if (col == 0) {
                                col = 1;
                            }

                            x += col;
                            if (x > 80) {
                                x = 80;
                            }
                            position += ansi_sequence_position + 2;
                            free(sequence_bytes);
                            break;
                         }

                         if (ansi_char == 'D') {    // CUB (Cursor Backward)
                            sequence_bytes = substr((char *)display->buffer,
                                                    position + 2,
                                                    ansi_sequence_position);
                            int32_t col = atoi(sequence_bytes);
                            if (col == 0) {
                                col = 1;
                            }

                            x -= col;
                            if (x < 0) {
                                x = 0;
                            }
                            position += ansi_sequence_position + 2;
                            free(sequence_bytes);
                            break;
                         }

                         if (ansi_char == 's') {    // SCP (Save Cursor Pos.)
                            y_saved = y;
                            x_saved = x;
                            position += ansi_sequence_position + 2;
                            break;
                         }

                         if (ansi_char == 'u') {    // RCP (Restore Cursor Pos.)
                            y = y_saved;
                            x = x_saved;
                            position += ansi_sequence_position + 2;
                            break;
                         }

                         if (ansi_char == 'J') {    // ED (Erase Display)
                            sequence_bytes = substr((char *)display->buffer,
                                                    position + 2,
                                                    ansi_sequence_position);
                            uint32_t n = atoi(sequence_bytes);

                            switch (n) {
                                case 2:
                                    y = 0;
                                    x = 0;
                                    y_max = 0;
                                    x_max = 0;

                                    // Reset the entire display->buffer
                                    screen_reset(display);
                                    display->cursor = 0;
                                    break;
                            }

                            position += ansi_sequence_position + 2;
                            free(sequence_bytes);
                            break;
                         }

                         if (ansi_char == 'm') {    // SGR (set Graphics Rend.)
                            sequence_bytes = substr((char *)display->buffer,
                                                    position + 2,
                                                    ansi_sequence_position);
                            sequence_count = split(&sequence_array, ';',
                                                    sequence_bytes);
                            for (sequence_position = 0;
                                 sequence_position < sequence_count;
                                 sequence_position++) {

                                 uint32_t n = atoi(sequence_array[sequence_position]);
                                 ansi_parser_parse_sgr(display, n);
                            }

                            position += ansi_sequence_position + 2;
                            free(sequence_array);
                            free(sequence_bytes);
                            break;
                         }

                         if (ansi_char == 'p') {    // Amiga hide cursor
                            position += ansi_sequence_position + 2;
                            break;
                         }

                         if (ansi_char == 'h' ||    // Hide cursor
                             ansi_char == 'l') {    // Show cursor
                            position += ansi_sequence_position + 2;
                            break;
                         }
                    }
                }
                break;

            default:
                // Time to put a character on the screen
                y_max = max32(y, y_max);
                x_max = max32(x, x_max);

                if (curr_char != 0xc) {
                    display->cursor = (display->width * y) + x;
                    screen_putchar(display, curr_char);
                    x++;
                }
        }
        position++;
    }

    return display;
}

void ansi_parser_parse_sgr(screen *display, uint32_t sgr) {
     switch (sgr) {
        case 0:
            display->current->bg = TILE_DEFAULT_BG;
            display->current->fg = TILE_DEFAULT_FG;
            display->attrib = ATTRIB_DEFAULT;
            break;

        case 1:
            ATTRIB_SET(display->attrib, ATTRIB_BOLD);
            break;

        case 3:
            ATTRIB_SET(display->attrib, ATTRIB_ITALICS);
            break;

        case 4:
            ATTRIB_SET(display->attrib, ATTRIB_UNDERLINE);
            break;

        case 5:
        case 6:
            ATTRIB_SET(display->attrib, ATTRIB_BLINK);
            break;

        case 7:
            ATTRIB_SET(display->attrib, ATTRIB_NEGATIVE);
            break;

        case 8:
            ATTRIB_SET(display->attrib, ATTRIB_CONCEAL);
            break;

        case 9:
            ATTRIB_SET(display->attrib, ATTRIB_CROSSED_OUT);
            break;

        case 21:
            ATTRIB_UNSET(display->attrib, ATTRIB_BOLD);
            break;

        case 22:
            ATTRIB_UNSET(display->attrib, ATTRIB_BOLD);
            break;

        case 24:
            ATTRIB_UNSET(display->attrib, ATTRIB_UNDERLINE);
            break;

        case 25:
            ATTRIB_UNSET(display->attrib, ATTRIB_BLINK);
            break;

        case 27:
            ATTRIB_UNSET(display->attrib, ATTRIB_NEGATIVE);
            break;

        case 28:
            ATTRIB_UNSET(display->attrib, ATTRIB_CONCEAL);
            break;

        case 29:
            ATTRIB_UNSET(display->attrib, ATTRIB_CROSSED_OUT);
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
            ATTRIB_SET(display->attrib, ATTRIB_FG_256);
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
            ATTRIB_SET(display->attrib, ATTRIB_BG_256);
            break;

        case 49:
            display->current->bg = TILE_DEFAULT_BG;
            break;

        default:
            // Ignore
            break;
     }
}

void ansi_parser_render(screen *display, unsigned int output_type)
{
    printf("cursor %d, type %d", display->cursor, output_type);
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
    parser_register(ansi_parser);
}
