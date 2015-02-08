#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "piece/font.h"
#include "piece/list.h"
#include "piece/parser.h"
#include "piece/parser/irc.h"
#include "piece/screen.h"
#include "piece/palette.h"
#include "piece/util.h"

typedef enum {
    IRC_PARSER_TEXT,
    IRC_PARSER_EXIT,
    IRC_PARSER_COLOR_FG,
    IRC_PARSER_COLOR_BG
} irc_parser_state;

static uint8_t irc_color_map[16] = {
    0x0f, /* white */
    0x00, /* black */
    0x01, /* blue */
    0x03, /* green */
    0x09, /* red */
    0x01, /* maroon */
    0x05, /* purple */
    0x03, /* orange */
    0x0b, /* yellow */
    0x0a, /* light green */
    0x06, /* teal */
    0x0e, /* cyan */
    0x0d, /* fuchsia */
    0x08, /* dark grey */
    0x07  /* light grey */
};

static int get2(uint8_t *data, int len, int *value)
{
    if (isdigit(data[0])) {
        *value = data[0];
        if (len > 1 && isdigit(data[1])) {
            *value *= 10;
            *value += (data[1] - '0');
            return 2;
        }
        return 1;
    }
    return 0;
}

static bool irc_parser_probe(FILE *fd, const char *UNUSED(filename))
{
    uint8_t probe[PIECE_IRC_PROBE_MAX], colors = 0;

    if (fread(probe, PIECE_IRC_PROBE_MAX, 1, fd) > 0) {
        for (size_t i = 0; i < (PIECE_IRC_PROBE_MAX - 1); i++) {
            if (probe[i] == PIECE_IRC_COLOR && isdigit(probe[i + 1])) {
                colors++;
            }
        }
    }

    return (colors >= PIECE_IRC_PROBE_COLORS);
}

static piece_screen *irc_parser_read(FILE *fd, const char *filename)
{
    piece_screen *display = NULL;
    sauce *record = NULL;
    int x = 0;
    int y = 0;
    uint8_t ch;
    uint8_t color[7], color_ptr = 0;
    off_t fsize;
    irc_parser_state state = IRC_PARSER_TEXT;

    record = sauce_read(fd);
    display = piece_screen_new(80, 1, record, NULL);
    if (display == NULL) {
        fprintf(stderr, "%s: could not piece_allocate 80 character buffer\n",
                        filename);
        goto return_free;
    }
    display->palette = piece_palette_by_name("mirc");

    fseeko(fd, 0, SEEK_END);
    fsize = ftello(fd) - sauce_size(record);
    fseeko(fd, 0, SEEK_SET);

    piece_screen_tile *current = display->current;
    while (!feof(fd) && ftello(fd) < fsize) {
        ch = fgetc(fd);

reinterpret_char:
        switch (state) {
        case IRC_PARSER_TEXT:
            switch (ch) {
            case PIECE_IRC_COLOR:
                state = IRC_PARSER_COLOR_FG;
                memset(color, 0x00, 6);
                color_ptr = 0;
                break;

            case PIECE_IRC_BOLD:
                /* not supported because we have a 16 color palette
                current->attrib ^= PIECE_ATTRIB_BOLD;
                */
                break;

            case PIECE_IRC_FIXED:
                current->fg = display->palette->color[PIECE_TILE_DEFAULT_FG];
                current->bg = display->palette->color[PIECE_TILE_DEFAULT_BG];
                current->attrib = PIECE_TILE_DEFAULT_ATTRIB;
                break;

            case PIECE_IRC_REVERSE:
                current->fg = current->fg ^ current->bg;
                current->bg = current->fg ^ current->bg;
                current->fg = current->fg ^ current->bg;
                break;

            case PIECE_IRC_INVERSE:
                current->attrib ^= PIECE_ATTRIB_NEGATIVE;
                break;

            case PIECE_IRC_ITALIC:
                current->attrib ^= PIECE_ATTRIB_ITALICS;
                break;

            case PIECE_IRC_UNDERLINE:
                current->attrib ^= PIECE_ATTRIB_UNDERLINE;
                break;

            case '\n':
                x = 0;
                y++;
                break;

            case '\r':
                break;

            case '\t':
                x += 8;
                break;

            default:
                piece_screen_putchar(display, ch, &x, &y, true);
                break;
            }
            break;

        case IRC_PARSER_COLOR_FG:
            if ((ch == ',' || isdigit(ch)) && color_ptr < 3) {
                color[color_ptr++] = ch;
                break;

            } else if (color_ptr == 0) {
                /* Empty ^C, reset colors */
                current->fg = display->palette->color[PIECE_TILE_DEFAULT_FG];
                current->bg = display->palette->color[PIECE_TILE_DEFAULT_BG];

            } else {
                color[color_ptr] = 0x00;
                uint8_t *data = color;
                int fg = -1, bg = -1, len = color_ptr, n;

                n = get2(data, len, &fg);
                if (n > 0) {
                    len -= n;
                    data += n;
                    if (len && *data == ',') {
                        len--;
                        data++;
                        n = get2(data, len, &bg);
                        data += n;
                        len -= n;
                    }
                }

                if (fg != -1) {
                    if (fg != 99) {
                        current->fg = display->palette->color[irc_color_map[fg % 16]];
                    }
                    if (bg != -1 && bg != 99) {
                        current->bg = display->palette->color[irc_color_map[bg % 16]];
                    }
                }

            }

            state = IRC_PARSER_TEXT;
            goto reinterpret_char;
            break;

        case IRC_PARSER_EXIT:
        default:
            break;
        }
    }


return_free:

    return display;
}

static char *irc_extensions[] = {
    "irc",
    "log",
    NULL
};

static piece_parser irc_parser = {
    "irc",
    "IRC log",
    irc_parser_probe,
    irc_parser_read,
    irc_extensions,
    NULL,
    "cp437_8x16"
};

void piece_irc_parser_init(void) {
    piece_parser_register(&irc_parser);
}
