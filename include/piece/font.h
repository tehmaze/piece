#ifndef __PIECE_FONT_H
#define __PIECE_FONT_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "piece/list.h"

typedef struct piece_font_s piece_font;
typedef struct piece_font_alias_s piece_font_alias;

struct piece_font_s {
    const char          *name;
    uint32_t            w;
    uint32_t            h;
    uint32_t            l;
    const unsigned char *glyphs;
    uint32_t            aliases;
    piece_font_alias    **alias;
};

struct piece_font_alias_s {
    const char *alias;
    const char *font_name;
    int        bits;
    bool       listed;
    piece_font *font;
};

extern piece_list *piece_fonts;

void                    piece_font_init(void);
void                    piece_font_init_alias(void);
void                    piece_font_free_item(void *);
void                    piece_font_free(void);
void                    piece_font_iter(piece_list_iterator);
piece_font /*@null@*/   *piece_font_by_name(const char *);
void                    piece_font_alias_add(piece_font_alias *);

#endif // __PIECE_FONT_H
