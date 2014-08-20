#ifndef __FONT_H
#define __FONT_H

#include <stdlib.h>
#include "list.h"

typedef struct font_s {
    const char *name;
    unsigned int w;
    unsigned int h;
    unsigned int l;
    const unsigned char *glyphs;
} font;

extern list *fonts;

void    font_init(void);
void    font_free(void);
void    font_iter(list_iterator);
font   *font_by_name(const char *);

#endif // __FONT_H
