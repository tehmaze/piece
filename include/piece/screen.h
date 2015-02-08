#ifndef __PIECE_SCREEN_H__
#define __PIECE_SCREEN_H__

#include <stdbool.h>
#include <stdint.h>

#include "piece/list.h"
#include "piece/font.h"
#include "piece/palette.h"
#include "sauce.h"

#define PIECE_TILE_DEFAULT_FG     0x07
#define PIECE_TILE_DEFAULT_BG     0x00
#define PIECE_TILE_DEFAULT_CH     0x20
#define PIECE_TILE_DEFAULT_ATTRIB 0x00
#define PIECE_TILE_DEFAULT        {PIECE_TILE_DEFAULT_FG, PIECE_TILE_DEFAULT_BG, PIECE_TILE_DEFAULT_CH, PIECE_TILE_DEFAULT_ATTRIB}

#define PIECE_ATTRIB_DEFAULT      0U
#define PIECE_ATTRIB_BOLD         (1U << 1)
#define PIECE_ATTRIB_DIM          (1U << 2)
#define PIECE_ATTRIB_ITALICS      (1U << 3)
#define PIECE_ATTRIB_UNDERLINE    (1U << 4)
#define PIECE_ATTRIB_BLINK        (1U << 5)
#define PIECE_ATTRIB_NEGATIVE     (1U << 7)
#define PIECE_ATTRIB_CONCEAL      (1U << 8)
#define PIECE_ATTRIB_CROSSED_OUT  (1U << 9)
#define PIECE_ATTRIB_FG_256       (1U << 10)
#define PIECE_ATTRIB_BG_256       (1U << 11)

#define PIECE_ATTRIB_SET(x, attrib)   do { x |= attrib; } while(0)
#define PIECE_ATTRIB_UNSET(x, attrib) do { x &= ~attrib; } while(0)

typedef struct piece_screen_tile_s {
    int      fg;
    int      bg;
    uint8_t  ch;
    uint64_t attrib;
} piece_screen_tile;

typedef struct piece_screen_size_s {
    int32_t width;
    int32_t height;
} piece_screen_size;

typedef struct piece_screen_s {
    piece_screen_tile     *tile;
    int32_t               tiles;
    uint8_t               *buffer;
    piece_screen_size     size;
    int64_t               cursor;
    piece_screen_tile     *initial;
    piece_screen_tile     *current;
    sauce                 *record;
    piece_palette         *palette;
    const char            *palette_name;
    piece_font            *font;
    const char            *font_name;
} piece_screen;


// Function prototypes

piece_screen        *piece_screen_new(int32_t, int32_t, sauce *,
                                      piece_screen_tile *);
void                piece_screen_free(piece_screen *);
void                piece_screen_putchar(piece_screen *, uint8_t ch,
                                         int32_t *x, int32_t *y, bool);
void                piece_screen_insert_line(piece_screen *display, int32_t y);
bool                piece_screen_reduce(piece_screen *display,
                                        int32_t width, int32_t height);
void                piece_screen_reset(piece_screen *display);

bool                piece_screen_tile_reset(piece_screen *, piece_screen_tile *);
piece_screen_tile   *piece_screen_tile_append(piece_screen *display);
bool                piece_screen_tile_append_many(piece_screen *, size_t);

#endif // __SCREEN_H__
