#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "piece/font.h"
#include "piece/list.h"
#include "piece/parser.h"
#include "piece/parser/pcboard.h"
#include "piece/screen.h"
#include "piece/palette.h"
#include "piece/util.h"

static unsigned char fromhex(unsigned char ch)
{
    if (ch >= '0' && ch <= '9') {
        return ch - '0';
    }
    else if (ch >= 'A' && ch <= 'F') {
        return ch - 'A' + 10;
    }
    else if (ch >= 'a' && ch <= 'f') {
        return ch - 'a' + 10;
    }
    else {
        return 0;
    }
}

piece_screen *pcboard_parser_read(FILE *fd, const char *filename)
{
    piece_screen *display = NULL;
    sauce *record = NULL;
    int x = 0;
    int y = 0;
    unsigned char ch;
    unsigned char *buffer;
    piece_pcboard_parser_state state = PIECE_PCBOARD_STATE_TEXT;
    size_t fsize;

    record = sauce_read(fd);
    if (record != NULL) {
        fprintf(stderr, "%s: found SAUCE record\n", filename);
        fsize = record->file_size;
        rewind(fd);
    } else {
        fprintf(stderr, "%s: no SAUCE record\n", filename);
        record = piece_allocate(sizeof(sauce));
        record->flags.flag_ls = SAUCE_LS_8PIXEL;
        record->flags.flag_b = 0;
        fseek(fd, 0, SEEK_END);
        fsize = ftell(fd);
        rewind(fd);
    }

    display = piece_screen_new(80, 25, record, NULL);
    if (display == NULL) {
        fprintf(stderr, "%s: could not piece_allocate 80x25 character buffer\n",
                        filename);
        goto return_bail;
    }
    display->palette = piece_palette_by_name("pcb");

    /* Slurp */
    buffer = piece_allocate(sizeof(unsigned char) * fsize);
    if (fread(buffer, fsize, 1, fd) == -1) {
        fprintf(stderr, "%s: error %d reading file\n", filename, ferror(fd));
        piece_screen_free(display);
        free(buffer);
        goto return_bail;
    }

    size_t fpos = 0;
    while (fpos < fsize &&
           state != PIECE_PCBOARD_STATE_EXIT) {

        ch = buffer[fpos++];
        switch (state) {
            case PIECE_PCBOARD_STATE_TEXT:
                switch (ch) {
                    case 0x09:  // HT
                        x += 8;
                        break;

                    case 0x0a:  // LF
                        y++;
                        x = 0;
                        break;

                    case 0x0d:  // CR (ignored)
                        break;

                    case 0x1a:  // SUB
                        state = PIECE_PCBOARD_STATE_EXIT;
                        break;

                    case '@':   // Operator
                        state = PIECE_PCBOARD_STATE_OPERATOR;
                        break;

                    default:    // Text
                        piece_screen_putchar(display, ch, &x, &y, true);
                        break;
                }
                break;

            case PIECE_PCBOARD_STATE_OPERATOR:
                if (ch == 'X') {
                    display->current->bg = fromhex(buffer[fpos++]);
                    display->current->fg = fromhex(buffer[fpos++]);
                } else {
                    if (ch == 'C' && !memcmp(buffer + fpos, "LS", 2)) {
                        piece_screen_reset(display);
                        fpos += 3;
                    }
                    else if (ch == 'P' && !memcmp(buffer + fpos, "OS:", 3)) {
                        fpos += 3;
                        if (buffer[fpos + 1] == '@') {     // Cursor position
                            x = buffer[fpos] - '0' - 1;
                            fpos++;
                        } else {
                            x = buffer[fpos] - '0' * 10 +
                                buffer[fpos + 1] - '0' - 1;
                            fpos += 2;
                        }
                    }
                    else {
                        piece_screen_putchar(display, '@', &x, &y, true);
                        piece_screen_putchar(display, ch, &x, &y, true);
                    }
                }
                state = PIECE_PCBOARD_STATE_TEXT;
                break;

            default:
                state = PIECE_PCBOARD_STATE_EXIT;
                break;
        }
    }

return_bail:

    return display;
}

static char *pcboard_extensions[] = {
    "pcb",
    NULL
};

static piece_parser_sauce pcboard_sauce[] = {
    {SAUCE_DATA_TYPE_CHARACTER, SAUCE_FILE_TYPE_PCBOARD},
    {0, 0}
};

static piece_parser pcboard_parser = {
    "pcboard",
    "PCBoard",
    NULL,
    pcboard_parser_read,
    pcboard_extensions,
    pcboard_sauce,
    "cp437_8x16"
};

void piece_pcboard_parser_init(void) {
    piece_parser_register(&pcboard_parser);
}
