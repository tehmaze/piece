#ifndef __PIECE_PALETTE_H__
#define __PIECE_PALETTE_H__

#include <stdint.h>

#include "piece/list.h"

typedef struct piece_rgb_color_s {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} piece_rgb_color;

typedef struct piece_rgb_palette_s {
    char            *name;
    piece_rgb_color *color;
    uint16_t        colors;
} piece_palette;

extern piece_list *piece_palettes;

piece_palette   *piece_palette_new(char *name, uint16_t colors);
void            piece_palette_init(void);
void            piece_palette_free(void);
uint16_t        piece_palette_add_color(piece_palette *piece_palette, piece_rgb_color *);
void            piece_palette_iter(piece_list_iterator);
piece_palette   *piece_palette_by_name(const char *);

#endif // __PIECE_PALETTE_H__
