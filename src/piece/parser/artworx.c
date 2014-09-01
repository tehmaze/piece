#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "piece/font.h"
#include "piece/list.h"
#include "piece/parser.h"
#include "piece/parser/artworx.h"
#include "piece/screen.h"
#include "piece/palette.h"
#include "piece/util.h"

static unsigned char artworx_piece_palette_slots[ARTWORX_PALETTE_DST_COLORS] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
    0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
};

piece_screen *artworx_parser_read(FILE *fd, const char *filename)
{
    piece_screen *display = NULL;
    sauce *record = NULL;
    int x = 0;
    int y = 0;
    unsigned char version, ch, attribute;
    piece_palette *artworx_palette;
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

    fread(&version, 1, 1, fd);
    fprintf(stderr, "%s: Artworx version %d\n", filename, version);

    display = piece_screen_create(80, 1, record);
    if (display == NULL) {
        fprintf(stderr, "%s: could not piece_allocate 80 character buffer\n",
                        filename);
        goto return_free;
    }

    // Artworx palettes are a bit perculiar, first we parse all 64 indexed RGB
    // colors, and then we make a sub selection of 16 particular colors (!?)
    artworx_palette = piece_palette_new("from file", 0);
    piece_rgb_color rgb;
    for (uint8_t i = 0; i < ARTWORX_PALETTE_SRC_COLORS; ++i) {
        ch = fgetc(fd);
        rgb.r = (ch << 2) | (ch >> 4);
        ch = fgetc(fd);
        rgb.g = (ch << 2) | (ch >> 4);
        ch = fgetc(fd);
        rgb.b = (ch << 2) | (ch >> 4);
        piece_palette_add_color(artworx_palette, &rgb);
    }

    display->palette = piece_palette_new("from file", 0);
    for (uint8_t i = 0; i < ARTWORX_PALETTE_DST_COLORS; ++i) {
        piece_palette_add_color(
            display->palette,
            &artworx_palette->color[artworx_piece_palette_slots[i]]
        );
    }
    free(artworx_palette);

    display->font = piece_allocate(sizeof(piece_font));
    display->font->name = "from file";
    display->font->w = 9;
    display->font->h = 16;
    display->font->l = 256;
    fprintf(stderr, "%s: reading 4096 bytes of glyphs\n", filename);
    display->font->glyphs = piece_allocate(4096);
    fread((char *) display->font->glyphs, 4096, 1, fd);
    if (ferror(fd)) {
        fprintf(stderr, "%s: read error %d\n", filename, ferror(fd));
        free(record);
        piece_screen_free(display);
        display = NULL;
        goto return_free;
    }

    while (!feof(fd) && ftell(fd) < fsize) {
        if (x == 80) {
            x = 0;
            y++;
        }
        ch = fgetc(fd);
        attribute = fgetc(fd);
        display->current->bg = (attribute & 0xf0) >> 4;
        display->current->fg = (attribute & 0x0f);
        piece_screen_putchar(display, ch, &x, &y, false);
        x++;
    }

return_free:

    return display;
}

static char *artworx_extensions[] = {
    "adf",
    NULL
};
static piece_parser artworx_parser = {
    "artworx",
    "Artworx",
    NULL,
    artworx_parser_read,
    artworx_extensions,
    "cp437_8x16"
};

void piece_artworx_parser_init(void) {
    piece_parser_register(&artworx_parser);
}
