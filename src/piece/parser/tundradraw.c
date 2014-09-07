#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "piece/font.h"
#include "piece/list.h"
#include "piece/parser.h"
#include "piece/parser/tundradraw.h"
#include "piece/screen.h"
#include "piece/palette.h"
#include "piece/util.h"

static uint16_t fgetrgb(piece_palette *palette, FILE *fd)
{
    int index = piece_fget32(fd);
    piece_rgb_color rgb = {
        (index >> 16) & 0xff,
        (index >> 8) & 0xff,
        index & 0xff
    };
    return piece_palette_add_color(palette, &rgb);
}

bool tundradraw_parser_probe(FILE *fd, const char *UNUSED(filename))
{
    piece_tundradraw_header *header;
    bool score = false;

    header = piece_allocate(sizeof(piece_tundradraw_header));
    if (fread(&header->version, 1, 1, fd) == 0 ||
        fread(&header->header, 8, 1, fd) == 0) {
        goto return_probe_free;
    }

    score = (header->version == 24 &&
             !strncmp(header->header, TUNDRADRAW_HEADER, 8));

return_probe_free:
    free(header);
    return score;
}

piece_screen *tundradraw_parser_read(FILE *fd, const char *filename)
{
    piece_screen *display = NULL;
    sauce *record = NULL;
    piece_tundradraw_header *header;
    int x = 0;
    int y = 0;
    unsigned char ch;
    int32_t fsize = 0;

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

    header = piece_allocate(sizeof(piece_tundradraw_header));
    if (fread(&header->version, 1, 1, fd) == 0 ||
        fread(&header->header, 8, 1, fd) == 0 ||
        ferror(fd)) {
        fprintf(stderr, "%s: read error %d\n", filename, ferror(fd));
        free(record);
        goto return_free;
    }
    if (header->version != 24 ||
        strncmp(header->header, TUNDRADRAW_HEADER, 8)) {
        fprintf(stderr, "%s: not a TundraDraw file (header mismatch)\n",
                        filename);
        free(record);
        goto return_free;
    }

    display = piece_screen_new(TUNDRADRAW_COLS, 1, record);
    if (display == NULL) {
        fprintf(stderr, "%s: could not piece_allocate 80 character buffer\n",
                        filename);
        free(record);
        goto return_free;
    }

    display->font = NULL;
    if (header->version == 24) {
        display->palette = piece_palette_new("from file", 0);
    } else {
        display->palette = piece_palette_by_name("tnd");
    }

    piece_rgb_color rgb = {0, 0, 0};
    piece_palette_add_color(display->palette, &rgb);

    while (!feof(fd) && ftell(fd) < fsize) {
        if (x == TUNDRADRAW_COLS) {
            x = 0;
            y++;
        }

        ch = fgetc(fd);
        if (record != NULL && ch == 0x1a) {
            break;
        }
        if (ch == 0x01) {       // Cursor position
            y = piece_fget32(fd);
            x = piece_fget32(fd);
            if (y > TUNDRADRAW_ROWS || x > TUNDRADRAW_COLS) {
                fprintf(stderr, "%s: jump to out of bounds location %dx%d\n",
                                filename, x, y);
                free(record);
                free(header);
                piece_screen_free(display);
                return NULL;
            }
        }
        if (ch == 0x02) {   // Forgeground color
            ch = fgetc(fd);
            display->current->fg = fgetrgb(display->palette, fd);
        }
        if (ch == 0x04) {   // Background color
            ch = fgetc(fd);
            display->current->bg = fgetrgb(display->palette, fd);
        }
        if (ch == 0x06) {   // Both colors
            ch = fgetc(fd);
            display->current->fg = fgetrgb(display->palette, fd);
            display->current->bg = fgetrgb(display->palette, fd);
        }
        if (!(ch == 0x01 || ch == 0x02 || ch == 0x04 || ch == 0x06)) {
            piece_screen_putchar(display, ch, &x, &y, false);
            x++;
        }
    }

return_free:

    free(header);

    return display;
}

static char *tundradraw_extensions[] = {
    "tnd",
    NULL
};

static piece_parser_sauce tundradraw_sauce[] = {
    {SAUCE_DATA_TYPE_CHARACTER, SAUCE_FILE_TYPE_PCBOARD},
    {0, 0}
};

static piece_parser tundradraw_parser = {
    "tundradraw",
    "Thundra Draw",
    tundradraw_parser_probe,
    tundradraw_parser_read,
    tundradraw_extensions,
    tundradraw_sauce,
    "cp437_8x16"
};

void piece_tundradraw_parser_init(void) {
    piece_parser_register(&tundradraw_parser);
}
