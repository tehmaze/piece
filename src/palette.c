#include <gd.h>
#include <stdlib.h>
#include <string.h>

#include "palette.h"
#include "util.h"

static rgb_color ANSI[16] = {
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

palette *AUTO, *EGA, *VGA;
list *palettes;

static palette *palette_new(char *name, uint16_t colors)
{
    palette *current = allocate(sizeof(palette));
    current->name = name;
    current->colors = colors;
    memset(current->color, 0, 768 * sizeof(uint8_t));
    return current;
}

static void palette_free_item(void *element)
{
    free(element);
}

void palette_init(void)
{
    int i;
    palettes = allocate(sizeof(list));
    list_new(palettes, palette_free_item);

    AUTO = palette_new("auto", 0);
    EGA  = palette_new("ega", 16);
    VGA  = palette_new("vga", 256);

    for (i = 0; i < 16; i++) {
        EGA->color[i] = ANSI[i];
        VGA->color[i] = ANSI[i];
    }

    // 6x6x6 color cube
    for (int r = 0; r < 6; ++r) {
        for (int g = 0; g < 6; ++g) {
            for (int b = 0; b < 6; ++b) {
                VGA->color[i++] = (rgb_color) {
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
        /*
        VGA->color[232 + i].r = g;
        VGA->color[232 + i].g = g;
        VGA->color[232 + i].b = g;
        */
        VGA->color[232 + i] = (rgb_color) {g, g, g};
    }

    list_append(palettes, AUTO);
    list_append(palettes, EGA);
    list_append(palettes, VGA);
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
