#include <gd.h>
#include <stdlib.h>
#include <string.h>

#include "palette.h"
#include "util.h"

static rgb_color ANSI_RGB[16] = {
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

static rgb_color BIN_RGB[16] = {
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

static rgb_color PCB_RGB[16] = {
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

static rgb_color TND_RGB[16] = {
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

palette *AUTO,
    *BIN_PALETTE,
    *EGA_PALETTE,
    *PCB_PALETTE,
    *TND_PALETTE,
    *VGA_PALETTE;
list *palettes;

palette *palette_new(char *name, uint16_t colors)
{
    palette *current = allocate(sizeof(palette));
    current->name = name;
    current->color = allocate(sizeof(rgb_color) * 256);
    current->colors = colors;
    memset(current->color, 0, 768 * sizeof(uint8_t));
    return current;
}

uint16_t palette_add_color(palette *palette, rgb_color *color)
{
    for (uint16_t i = 0; i < palette->colors; ++i) {
        rgb_color *other = &palette->color[i];
        if (other->r == color->r &&
            other->g == color->g &&
            other->b == color->b) {
            return i;
        }
    }

    palette->color[palette->colors++] = *color;
    return palette->colors - 1;
}

static void palette_free_item(void *element)
{
    palette *palette = element;
    free(palette->color);
    free(palette);
}

void palette_init(void)
{
    int i;
    palettes = allocate(sizeof(list));
    list_new(palettes, palette_free_item);

    AUTO = palette_new("auto", 0);
    BIN_PALETTE = palette_new("bin", 16);
    EGA_PALETTE = palette_new("ega", 16);
    PCB_PALETTE = palette_new("pcb", 16);
    TND_PALETTE = palette_new("tnd", 16);
    VGA_PALETTE = palette_new("vga", 256);

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
                VGA_PALETTE->color[i++] = (rgb_color) {
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
        VGA_PALETTE->color[232 + i] = (rgb_color) {g, g, g};
    }

    list_append(palettes, AUTO);
    list_append(palettes, BIN_PALETTE);
    list_append(palettes, EGA_PALETTE);
    list_append(palettes, PCB_PALETTE);
    list_append(palettes, TND_PALETTE);
    list_append(palettes, VGA_PALETTE);
}

void palette_free(void)
{
    list_free(palettes);
}

void palette_iter(list_iterator iterator)
{
    list_foreach(palettes, iterator);
}

palette *palette_by_name(const char *name)
{
    list_node *node = palettes->head;
    while (node != NULL) {
        if (!strcmp(((palette *) node->data)->name, name)) {
            return node->data;
        }
        node = node->next;
    }
    return NULL;
}
