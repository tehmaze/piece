#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "font.h"
#include "list.h"
#include "parser.h"
#include "parser/icedraw.h"
#include "screen.h"
#include "palette.h"
#include "util.h"

static const unsigned char idf_magic[] = {
    0x04, 0x31, 0x2e, 0x34, 0x00, 0x00, 0x00, 0x00, 0x4f, 0x00, 0x15, 0x00
};

bool icedraw_parser_probe(FILE *fd, const char *UNUSED(filename))
{
    unsigned char *header;
    bool score = false;

    header = allocate(sizeof(idf_magic));
    fread(header, sizeof(idf_magic), 1, fd);

    score = (memcmp(header, idf_magic, sizeof(idf_magic)) == 0);
    free(header);
    return score;
}

screen *icedraw_parser_read(FILE *fd, const char *filename)
{
    screen *display = NULL;
    sauce *record = NULL;
    int x = 0;
    int y = 0;
    unsigned char ch, attribute;
    unsigned char *buffer;
    int32_t fsize = 0;
    int16_t width = 0;

    rewind(fd);
    buffer = allocate(sizeof(idf_magic));
    fread(buffer, sizeof(idf_magic), 1, fd);
    if (memcmp(buffer, idf_magic, sizeof(idf_magic))) {
        fprintf(stderr, "%s: IDF magic mismatch\n", filename);
        free(buffer);
        return NULL;
    } else {
        free(buffer);
    }

    record = sauce_read(fd);
    if (record != NULL) {
        fprintf(stderr, "%s: found SAUCE record\n", filename);
        fsize = record->file_size;
    } else {
        fprintf(stderr, "%s: no SAUCE record\n", filename);
        record = allocate(sizeof(sauce));
        record->flags.flag_ls = SAUCE_LS_8PIXEL;
        record->flags.flag_b = 0;
        fseek(fd, 0, SEEK_END);
        fsize = ftell(fd);
    }

    fseek(fd, 8, SEEK_SET);
    width = (fgetc(fd) | (fgetc(fd) << 4)) + 1;
    display = screen_create(width, 1, record);
    if (display == NULL) {
        fprintf(stderr, "%s: could not allocate %d character buffer\n",
                        filename, width);
        fclose(fd);
        free(record);
        return NULL;
    }

    fseek(fd, fsize - 4144, SEEK_SET);
    display->font = allocate(sizeof(font));
    display->font->name = "from file";
    display->font->w = 9;
    display->font->h = 16;
    display->font->l = 256;
    display->font->glyphs = allocate(4096);
    fread((char *) display->font->glyphs, 4096, 1, fd);

    display->palette = palette_new("from file", 0);
    rgb_color rgb;
    for (uint8_t j = 0; j < 16; ++j) {
        ch = fgetc(fd);
        rgb.r = (ch << 2) | (ch >> 4);
        ch = fgetc(fd);
        rgb.g = (ch << 2) | (ch >> 4);
        ch = fgetc(fd);
        rgb.b = (ch << 2) | (ch >> 4);
        palette_add_color(display->palette, &rgb);
    }

    fseek(fd, 12, SEEK_SET);
    while (ftell(fd) < fsize - 4144) {
        ch = fgetc(fd);
        attribute = fgetc(fd);
        if (ch == 0x01 && attribute == 0x00) {        // RLE compressed data
            uint8_t repeat = fgetc(fd);
            fgetc(fd);
            ch = fgetc(fd);
            attribute = fgetc(fd);
            while (repeat-- > 0) {
                display->current->bg = (attribute & 0xf0) >> 4;
                display->current->fg = (attribute & 0x0f);
                screen_putchar(display, ch, &x, &y, false);
                if (++x == width) {
                    x = 0;
                    y++;
                }
            }
        } else {
            display->current->bg = (attribute & 0xf0) >> 4;
            display->current->fg = (attribute & 0x0f);
            screen_putchar(display, ch, &x, &y, false);
            if (++x == width) {
                x = 0;
                y++;
            }
        }
    }

    rewind(fd);
    fclose(fd);

    return display;
}

static char *icedraw_extensions[] = {
    "idf",
    NULL
};
static parser icedraw_parser = {
    "icedraw",
    "iCE Draw",
    icedraw_parser_probe,
    icedraw_parser_read,
    icedraw_extensions,
    "cp437_8x16"
};

void icedraw_parser_init(void) {
    parser_register(&icedraw_parser);
}
