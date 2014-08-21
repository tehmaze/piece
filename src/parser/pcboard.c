#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "font.h"
#include "list.h"
#include "parser.h"
#include "parser/pcboard.h"
#include "screen.h"
#include "palette.h"
#include "util.h"

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

screen *pcboard_parser_read(FILE *fd, const char *filename)
{
    screen *display = NULL;
    sauce *record = NULL;
    int x = 0;
    int y = 0;
    unsigned char ch;
    unsigned char *buffer;
    pcboard_parser_state state = PCBOARD_STATE_TEXT;
    size_t fsize;

    record = sauce_read(fd);
    if (record != NULL) {
        fprintf(stderr, "%s: found SAUCE record\n", filename);
        fsize = record->file_size;
        rewind(fd);
    } else {
        fprintf(stderr, "%s: no SAUCE record\n", filename);
        record = allocate(sizeof(sauce));
        record->flags.flag_ls = SAUCE_LS_8PIXEL;
        record->flags.flag_b = 0;
        fseek(fd, 0, SEEK_END);
        fsize = ftell(fd);
        rewind(fd);
    }

    display = screen_create(80, 25, record);
    if (display == NULL) {
        fprintf(stderr, "%s: could not allocate 80x25 character buffer\n",
                        filename);
        fclose(fd);
        free(record);
        return NULL;
    }
    display->palette = palette_by_name("pcb");

    /* Slurp */
    buffer = allocate(sizeof(unsigned char) * fsize);
    fread(buffer, fsize, 1, fd);
    rewind(fd);
    fclose(fd);

    size_t fpos = 0;
    while (fpos < fsize &&
           state != PCBOARD_STATE_EXIT) {

        ch = buffer[fpos++];
        switch (state) {
            case PCBOARD_STATE_TEXT:
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
                        state = PCBOARD_STATE_EXIT;
                        break;

                    case '@':   // Operator
                        state = PCBOARD_STATE_OPERATOR;
                        break;

                    default:    // Text
                        screen_putchar(display, ch, &x, &y, true);
                        break;
                }
                break;

            case PCBOARD_STATE_OPERATOR:
                if (ch == 'X') {
                    display->current->bg = fromhex(buffer[fpos++]);
                    display->current->fg = fromhex(buffer[fpos++]);
                } else {
                    if (ch == 'C' && !memcmp(buffer + fpos, "LS", 2)) {
                        screen_reset(display);
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
                        screen_putchar(display, '@', &x, &y, true);
                        screen_putchar(display, ch, &x, &y, true);
                    }
                }
                state = PCBOARD_STATE_TEXT;
                break;

            default:
                state = PCBOARD_STATE_EXIT;
                break;
        }
    }

    return display;
}

static char *pcboard_extensions[] = {
    "pcb",
    NULL
};
static parser pcboard_parser = {
    "pcboard",
    "PCBoard",
    NULL,
    pcboard_parser_read,
    pcboard_extensions,
    "cp437_8x16"
};

void pcboard_parser_init(void) {
    parser_register(&pcboard_parser);
}
