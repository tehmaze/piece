#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "piece/font.h"
#include "piece/list.h"
#include "piece/parser.h"
#include "piece/parser/ascii7.h"
#include "piece/screen.h"
#include "piece/palette.h"
#include "piece/util.h"

static piece_screen *ascii7_parser_read(FILE *fd, const char *filename)
{
    piece_screen *display = NULL;
    sauce *record = NULL;
    int x = 0;
    int y = 0;
    uint8_t ch;
    off_t fsize;

    record = sauce_read(fd);
    display = piece_screen_new(80, 1, record, NULL);
    if (display == NULL) {
        fprintf(stderr, "%s: could not piece_allocate 80 character buffer\n",
                        filename);
        goto return_free;
    }
    display->palette_name = "ega";

    fseeko(fd, 0, SEEK_END);
    fsize = ftello(fd) - sauce_size(record);
    fseeko(fd, 0, SEEK_SET);

    while (!feof(fd) && ftello(fd) < fsize) {
        ch = fgetc(fd);

        switch (ch) {
        case 0x04:
            goto return_free;

        case 0x08: /* Backspace */
            x = (x ? x - 1 : 0);
            break;

        case 0x09: /* Tab */
            x += 8;
            break;

        case 0x0a: /* Newline */
            x = 0;
            y++;
            break;

        case 0x0d: /* Carriage return */
            break;

        default:
            /* Ignore all other control characters */
            if (ch >= 0x20) {
                ch = isascii(ch) ? ch : ' ';
                piece_screen_putchar(display, ch, &x, &y, true);
            }
            break;
        }
    }

return_free:

    return display;
}

static piece_parser ascii7_parser = {
    "ascii7",
    "US-ASCII (7 bit)",
    NULL,
    ascii7_parser_read,
    NULL,
    NULL,
    "cp437_8x16"
};

void piece_ascii7_parser_init(void) {
    piece_parser_register(&ascii7_parser);
}
