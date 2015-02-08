#include <gd.h>
#include <stdlib.h>
#include <string.h>

#include "piece/options.h"
#include "piece/palette.h"
#include "piece/util.h"

static piece_rgba_color ANSI_RGB[16] = {
    PIECE_RGB(0x00, 0x00, 0x00),     /* 0x00 Black */
    PIECE_RGB(0xaa, 0x00, 0x00),     /* 0x01 Red */
    PIECE_RGB(0x00, 0xaa, 0x00),     /* 0x02 Green */
    PIECE_RGB(0xaa, 0x55, 0x00),     /* 0x03 Brown */
    PIECE_RGB(0x00, 0x00, 0xaa),     /* 0x04 Blue */
    PIECE_RGB(0xaa, 0x00, 0xaa),     /* 0x05 Magenta */
    PIECE_RGB(0x00, 0xaa, 0xaa),     /* 0x06 Cyan */
    PIECE_RGB(0xaa, 0xaa, 0xaa),     /* 0x07 White */
    PIECE_RGB(0x55, 0x55, 0x55),     /* 0x08 Black (dark) */
    PIECE_RGB(0xff, 0x55, 0x55),     /* 0x09 Red */
    PIECE_RGB(0x55, 0xff, 0x55),     /* 0x0a green */
    PIECE_RGB(0xff, 0xff, 0x55),     /* 0x0b yellow */
    PIECE_RGB(0x55, 0x55, 0xff),     /* 0x0c blue */
    PIECE_RGB(0xff, 0x55, 0xff),     /* 0x0d magenta */
    PIECE_RGB(0x55, 0xff, 0xff),     /* 0x0e cyan */
    PIECE_RGB(0xff, 0xff, 0xff),     /* 0x0f white */
};

static piece_rgba_color BIN_RGB[16] = {
    PIECE_RGB(0x00, 0x00, 0x00),
    PIECE_RGB(0x00, 0x00, 0xaa),
    PIECE_RGB(0x00, 0xaa, 0x00),
    PIECE_RGB(0x00, 0xaa, 0xaa),
    PIECE_RGB(0xaa, 0x00, 0x00),
    PIECE_RGB(0xaa, 0x00, 0xaa),
    PIECE_RGB(0xaa, 0x55, 0x00),
    PIECE_RGB(0xaa, 0xaa, 0xaa),
    PIECE_RGB(0x55, 0x55, 0x55),
    PIECE_RGB(0x55, 0x55, 0xff),
    PIECE_RGB(0x55, 0xff, 0x55),
    PIECE_RGB(0x55, 0xff, 0xff),
    PIECE_RGB(0xff, 0x55, 0x55),
    PIECE_RGB(0xff, 0x55, 0xff),
    PIECE_RGB(0xff, 0xff, 0x55),
    PIECE_RGB(0xff, 0xff, 0xff),
};

/* http://www.c64-wiki.com/index.php/Color */
static piece_rgba_color C64_RGB[16] = {
    PIECE_RGB(0x00, 0x00, 0x00),
    PIECE_RGB(0xff, 0xff, 0xff),
    PIECE_RGB(0x88, 0x00, 0x00),
    PIECE_RGB(0xaa, 0xff, 0xee),
    PIECE_RGB(0xcc, 0x44, 0xcc),
    PIECE_RGB(0x00, 0xcc, 0x55),
    PIECE_RGB(0x00, 0x00, 0xaa),
    PIECE_RGB(0xee, 0xee, 0x77),
    PIECE_RGB(0xdd, 0x88, 0x55),
    PIECE_RGB(0x66, 0x44, 0x00),
    PIECE_RGB(0xff, 0x77, 0x77),
    PIECE_RGB(0x33, 0x33, 0x33),
    PIECE_RGB(0x77, 0x77, 0x77),
    PIECE_RGB(0xaa, 0xff, 0x66),
    PIECE_RGB(0x00, 0x88, 0xff),
    PIECE_RGB(0xbb, 0xbb, 0xbb)
};

/* mIRC colors */
static piece_rgba_color IRC_RGB[16] = {
    PIECE_RGB(0x00, 0x00, 0x00),
    PIECE_RGB(0x7f, 0x00, 0x00),
    PIECE_RGB(0x00, 0x93, 0x00),
    PIECE_RGB(0xfc, 0x7f, 0x00),
    PIECE_RGB(0x00, 0x00, 0x7f),
    PIECE_RGB(0x9c, 0x00, 0x9c),
    PIECE_RGB(0x00, 0x93, 0x93),
    PIECE_RGB(0xd2, 0xd2, 0xd2),
    PIECE_RGB(0x7f, 0x7f, 0x7f),
    PIECE_RGB(0xff, 0x00, 0x00),
    PIECE_RGB(0x00, 0xfc, 0x00),
    PIECE_RGB(0xff, 0xff, 0x00),
    PIECE_RGB(0x00, 0x00, 0xfc),
    PIECE_RGB(0xff, 0x00, 0xff),
    PIECE_RGB(0x00, 0xff, 0xff),
    PIECE_RGB(0xff, 0xff, 0xff)
};

static piece_rgba_color PCB_RGB[16] = {
    PIECE_RGB(0x00, 0x00, 0x00),
    PIECE_RGB(0x00, 0x00, 0xaa),
    PIECE_RGB(0x00, 0xaa, 0x00),
    PIECE_RGB(0x00, 0xaa, 0xaa),
    PIECE_RGB(0xaa, 0x00, 0x00),
    PIECE_RGB(0xaa, 0x00, 0xaa),
    PIECE_RGB(0xaa, 0x55, 0x00),
    PIECE_RGB(0xaa, 0xaa, 0xaa),
    PIECE_RGB(0x55, 0x55, 0x55),
    PIECE_RGB(0x55, 0x55, 0xff),
    PIECE_RGB(0x55, 0xff, 0x55),
    PIECE_RGB(0x55, 0xff, 0xff),
    PIECE_RGB(0xff, 0x55, 0x55),
    PIECE_RGB(0xff, 0x55, 0xff),
    PIECE_RGB(0xff, 0xff, 0x55),
    PIECE_RGB(0xff, 0xff, 0xff)
};

static piece_rgba_color TND_RGB[16] = {
    PIECE_RGB(0x00, 0x00, 0x00),
    PIECE_RGB(0xad, 0x00, 0x00),
    PIECE_RGB(0x00, 0xaa, 0x00),
    PIECE_RGB(0xad, 0x55, 0x00),
    PIECE_RGB(0x00, 0x00, 0xad),
    PIECE_RGB(0xad, 0x00, 0xad),
    PIECE_RGB(0x00, 0xaa, 0xad),
    PIECE_RGB(0xad, 0xaa, 0xad),
    PIECE_RGB(0x52, 0x55, 0x52),
    PIECE_RGB(0xff, 0x52, 0x55),
    PIECE_RGB(0x52, 0xff, 0x52),
    PIECE_RGB(0xff, 0xff, 0x52),
    PIECE_RGB(0x52, 0x55, 0xff),
    PIECE_RGB(0xff, 0x55, 0xff),
    PIECE_RGB(0x52, 0xff, 0xff),
    PIECE_RGB(0xff, 0xff, 0xff),
};

piece_palette *AUTO,
    *BIN_PALETTE,
    *C64_PALETTE,
    *EGA_PALETTE,
    *IRC_PALETTE,
    *PCB_PALETTE,
    *TND_PALETTE,
    *VGA_PALETTE;
piece_list *piece_palettes;

piece_palette *piece_palette_new(char *name, uint16_t colors)
{
    piece_palette *current = piece_allocate(sizeof(piece_palette));
    current->name = name;
    current->color = piece_allocate(sizeof(piece_rgba_color) * 256);
    current->colors = colors;
    memset(current->color, 0, 768 * sizeof(uint8_t));
    return current;
}

uint16_t piece_palette_add_color(piece_palette *palette, piece_rgba_color *color)
{
    for (uint16_t i = 0; i < palette->colors; ++i) {
        piece_rgba_color *other = &palette->color[i];
        if (*other == *color) {
            return i;
        }
    }

    palette->color[palette->colors++] = *color;
    if (piece_options->verbose) {
        printf("palette: added %08x, now %d colors\n", *color, palette->colors);
    }
    return palette->colors - 1;
}

static void piece_palette_free_item(void *element)
{
    piece_palette *palette = element;
    if (palette != NULL) {
        if (palette->color != NULL) {
            free(palette->color);
            palette->color = NULL;
        }
        free(palette);
        palette = NULL;
    }
}

void piece_palette_init(void)
{
    int i;
    piece_palettes = piece_allocate(sizeof(piece_list));
    piece_list_new(piece_palettes, piece_palette_free_item);

    AUTO = piece_palette_new("auto", 0);
    BIN_PALETTE = piece_palette_new("bin", 16);
    C64_PALETTE = piece_palette_new("c64", 16);
    EGA_PALETTE = piece_palette_new("ega", 16);
    IRC_PALETTE = piece_palette_new("irc", 16);
    PCB_PALETTE = piece_palette_new("pcb", 16);
    TND_PALETTE = piece_palette_new("tnd", 16);
    VGA_PALETTE = piece_palette_new("vga", 256);

    for (i = 0; i < 16; i++) {
        BIN_PALETTE->color[i] = BIN_RGB[i];
        C64_PALETTE->color[i] = C64_RGB[i];
        EGA_PALETTE->color[i] = ANSI_RGB[i];
        IRC_PALETTE->color[i] = IRC_RGB[i];
        PCB_PALETTE->color[i] = PCB_RGB[i];
        TND_PALETTE->color[i] = TND_RGB[i];
        VGA_PALETTE->color[i] = ANSI_RGB[i];
    }

    // 6x6x6 color cube
    for (int r = 0; r < 6; ++r) {
        for (int g = 0; g < 6; ++g) {
            for (int b = 0; b < 6; ++b) {
                VGA_PALETTE->color[i++] = PIECE_RGB(
                    55 + (r * 40),
                    55 + (g * 40),
                    55 + (b * 40)
                );
            }
        }
    }

    // gray scale ramp
    for (int i = 0; i < 24; ++i) {
        int g = 10 * i + 8;
        VGA_PALETTE->color[232 + i] = PIECE_RGB(g, g, g);
    }

    piece_list_append(piece_palettes, AUTO);
    piece_list_append(piece_palettes, BIN_PALETTE);
    piece_list_append(piece_palettes, C64_PALETTE);
    piece_list_append(piece_palettes, EGA_PALETTE);
    piece_list_append(piece_palettes, PCB_PALETTE);
    piece_list_append(piece_palettes, TND_PALETTE);
    piece_list_append(piece_palettes, VGA_PALETTE);
}

void piece_palette_free(void)
{
    piece_list_free(piece_palettes);
}

void piece_palette_iter(piece_list_iterator iterator)
{
    piece_list_foreach(piece_palettes, iterator);
}

piece_palette *piece_palette_by_name(const char *name)
{
    piece_list_node *node = piece_palettes->head;
    while (node != NULL) {
        if (!strcmp(((piece_palette *) node->data)->name, name)) {
            return node->data;
        }
        node = node->next;
    }
    return NULL;
}
