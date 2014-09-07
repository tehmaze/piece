#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "piece/font.h"
#include "piece/list.h"
#include "piece/parser.h"
#include "piece/parser/ascii.h"
#include "piece/screen.h"
#include "piece/palette.h"
#include "piece/util.h"

static piece_screen *ascii_parser_read(FILE *fd, const char *filename)
{
    piece_screen *display = NULL;
    sauce *record = NULL;
    int x = 0;
    int y = 0;
    uint8_t ch;
    off_t fsize;

    record = sauce_read(fd);
    display = piece_screen_new(80, 1, record);
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
            piece_screen_putchar(display, ch, &x, &y, true);
            break;
        }
    }

return_free:

    return display;
}

static char *ascii_extensions[] = {
    "asc",
    "c",
    "cc",
    "cpp",
    "diz",
    "h",
    "hh",
    "hpp",
    "nfo",
    "txt",
    NULL
};

static piece_parser_sauce ascii_sauce[] = {
    {SAUCE_DATA_TYPE_CHARACTER, SAUCE_FILE_TYPE_ASCII},
    {0, 0}
};

static piece_parser ascii_parser = {
    "ascii",
    "Extended ASCII (8 bit)",
    NULL,
    ascii_parser_read,
    ascii_extensions,
    ascii_sauce,
    "cp437_8x16"
};

void piece_ascii_parser_init(void) {
    piece_parser_register(&ascii_parser);
}
