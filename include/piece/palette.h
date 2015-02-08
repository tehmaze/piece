#ifndef __PIECE_PALETTE_H__
#define __PIECE_PALETTE_H__

#include <stdint.h>
#include <gd.h>

#include "piece/list.h"
#include "piece/palette.h"

#define PIECE_ALPHA_OPAQUE          gdAlphaOpaque
#define PIECE_ALPHA_TRANSPARENT     gdAlphaTransparent
#define PIECE_RGBA(r,g,b,a)         \
    ((((r) & 0xff) << 24) | (((g) & 0xff) << 16) | (((b) & 0xff) << 8) | ((a) &0xff))
#define PIECE_RGB(r,g,b)            PIECE_RGBA((r),(g),(b),PIECE_ALPHA_OPAQUE)
#define PIECE_RGBA_R(color)         ((color >> 24) & 0xff)
#define PIECE_RGBA_G(color)         ((color >> 16) & 0xff)
#define PIECE_RGBA_B(color)         ((color >>  8) & 0xff)
#define PIECE_RGBA_A(color)         ((color      ) & 0xff)

typedef uint32_t piece_rgba_color;

typedef struct piece_rgba_palette_s {
    char             *name;
    piece_rgba_color *color;
    uint16_t         colors;
} piece_palette;

extern piece_list *piece_palettes;

piece_palette   *piece_palette_new(char *name, uint16_t colors);
void            piece_palette_init(void);
void            piece_palette_free(void);
uint16_t        piece_palette_add_color(piece_palette *piece_palette, piece_rgba_color *);
void            piece_palette_iter(piece_list_iterator);
piece_palette   *piece_palette_by_name(const char *);

#endif // __PIECE_PALETTE_H__
