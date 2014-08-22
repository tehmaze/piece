#include <gd.h>
#include <stdlib.h>
#include <string.h>

#include "piece/palette.h"
#include "piece/util.h"

static piece_rgb_color ANSI_RGB[16] = {
    {0x00, 0x00, 0x00},     /* Black */
    {0xaa, 0x00, 0x00},     /* Red */
    {0x00, 0xaa, 0x00},     /* Green */
    {0xaa, 0x55, 0x00},     /* Brown */
    {0x00, 0x00, 0xaa},     /* Blue */
    {0xaa, 0x00, 0xaa},     /* Magenta */
    {0x00, 0xaa, 0xaa},     /* Cyan */
    {0xaa, 0xaa, 0xaa},     /* White */
    {0x55, 0x55, 0x55},     /* Black (dark) */
    {0xff, 0x55, 0x55},     /* Red */
    {0x55, 0xff, 0x55},     /* green */
    {0xff, 0xff, 0x55},     /* yellow */
    {0x55, 0x55, 0xff},     /* blue */
    {0xff, 0x55, 0xff},     /* magenta */
    {0x55, 0xff, 0xff},     /* cyan */
    {0xff, 0xff, 0xff},     /* white */
};

static piece_rgb_color BIN_RGB[16] = {
    {0x00, 0x00, 0x00},
    {0x00, 0x00, 0xaa},
    {0x00, 0xaa, 0x00},
    {0x00, 0xaa, 0xaa},
    {0xaa, 0x00, 0x00},
    {0xaa, 0x00, 0xaa},
    {0xaa, 0x55, 0x00},
    {0xaa, 0xaa, 0xaa},
    {0x55, 0x55, 0x55},
    {0x55, 0x55, 0xff},
    {0x55, 0xff, 0x55},
    {0x55, 0xff, 0xff},
    {0xff, 0x55, 0x55},
    {0xff, 0x55, 0xff},
    {0xff, 0xff, 0x55},
    {0xff, 0xff, 0xff},
};

static piece_rgb_color PCB_RGB[16] = {
    {0x00, 0x00, 0x00},
    {0x00, 0x00, 0xaa},
    {0x00, 0xaa, 0x00},
    {0x00, 0xaa, 0xaa},
    {0xaa, 0x00, 0x00},
    {0xaa, 0x00, 0xaa},
    {0xaa, 0x55, 0x00},
    {0xaa, 0xaa, 0xaa},
    {0x55, 0x55, 0x55},
    {0x55, 0x55, 0xff},
    {0x55, 0xff, 0x55},
    {0x55, 0xff, 0xff},
    {0xff, 0x55, 0x55},
    {0xff, 0x55, 0xff},
    {0xff, 0xff, 0x55},
    {0xff, 0xff, 0xff},
};

static piece_rgb_color TND_RGB[16] = {
    {0x00, 0x00, 0x00},
    {0xad, 0x00, 0x00},
    {0x00, 0xaa, 0x00},
    {0xad, 0x55, 0x00},
    {0x00, 0x00, 0xad},
    {0xad, 0x00, 0xad},
    {0x00, 0xaa, 0xad},
    {0xad, 0xaa, 0xad},
    {0x52, 0x55, 0x52},
    {0xff, 0x52, 0x55},
    {0x52, 0xff, 0x52},
    {0xff, 0xff, 0x52},
    {0x52, 0x55, 0xff},
    {0xff, 0x55, 0xff},
    {0x52, 0xff, 0xff},
    {0xff, 0xff, 0xff},
};

piece_palette *AUTO,
    *BIN_PALETTE,
    *EGA_PALETTE,
    *PCB_PALETTE,
    *TND_PALETTE,
    *VGA_PALETTE;
piece_list *piece_palettes;

piece_palette *piece_palette_new(char *name, uint16_t colors)
{
    piece_palette *current = piece_allocate(sizeof(piece_palette));
    current->name = name;
    current->color = piece_allocate(sizeof(piece_rgb_color) * 256);
    current->colors = colors;
    memset(current->color, 0, 768 * sizeof(uint8_t));
    return current;
}

uint16_t piece_palette_add_color(piece_palette *palette, piece_rgb_color *color)
{
    for (uint16_t i = 0; i < palette->colors; ++i) {
        piece_rgb_color *other = &palette->color[i];
        if (other->r == color->r &&
            other->g == color->g &&
            other->b == color->b) {
            return i;
        }
    }

    palette->color[palette->colors++] = *color;
    return palette->colors - 1;
}

static void piece_palette_free_item(void *element)
{
    piece_palette *palette = element;
    free(palette->color);
    free(palette);
}

void piece_palette_init(void)
{
    int i;
    piece_palettes = piece_allocate(sizeof(piece_list));
    piece_list_new(piece_palettes, piece_palette_free_item);

    AUTO = piece_palette_new("auto", 0);
    BIN_PALETTE = piece_palette_new("bin", 16);
    EGA_PALETTE = piece_palette_new("ega", 16);
    PCB_PALETTE = piece_palette_new("pcb", 16);
    TND_PALETTE = piece_palette_new("tnd", 16);
    VGA_PALETTE = piece_palette_new("vga", 256);

    for (i = 0; i < 16; i++) {
        BIN_PALETTE->color[i] = BIN_RGB[i];
        EGA_PALETTE->color[i] = ANSI_RGB[i];
        PCB_PALETTE->color[i] = PCB_RGB[i];
        TND_PALETTE->color[i] = TND_RGB[i];
        VGA_PALETTE->color[i] = ANSI_RGB[i];
    }

    // 6x6x6 color cube
    for (int r = 0; r < 6; ++r) {
        for (int g = 0; g < 6; ++g) {
            for (int b = 0; b < 6; ++b) {
                VGA_PALETTE->color[i++] = (piece_rgb_color) {
                    55 + (r * 40),
                    55 + (g * 40),
                    55 + (b * 40)
                };
            }
        }
    }

    // gray scale ramp
    for (int i = 0; i < 24; ++i) {
        int g = 10 * i + 8;
        VGA_PALETTE->color[232 + i] = (piece_rgb_color) {g, g, g};
    }

    piece_list_append(piece_palettes, AUTO);
    piece_list_append(piece_palettes, BIN_PALETTE);
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
