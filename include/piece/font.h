#ifndef __PIECE_FONT_H
#define __PIECE_FONT_H

#include <stdlib.h>

#include "piece/list.h"

typedef struct piece_font_s {
    const char *name;
    unsigned int w;
    unsigned int h;
    unsigned int l;
    const unsigned char *glyphs;
} piece_font;

extern piece_list *piece_fonts;

void                    piece_font_init(void);
void                    piece_font_free(void);
void                    piece_font_iter(piece_list_iterator);
piece_font /*@null@*/   *piece_font_by_name(const char *);

#endif // __PIECE_FONT_H
