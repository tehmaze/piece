#ifndef __PALETTE_H
#define __PALETTE_H

#include <stdint.h>
#include "list.h"

typedef struct rgb_color_s {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rgb_color;

typedef struct rgb_palette_s {
    char      *name;
    rgb_color color[256];
    uint16_t  colors;
} palette;

extern palette *EGA, *VGA;
extern list *palettes;

void        palette_init(void);
void        palette_free(void);
void        palette_iter(list_iterator);
palette    *palette_by_name(const char *);

#endif // __PALETTE_H
