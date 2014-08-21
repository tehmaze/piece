#ifndef __SCREEN_H__
#define __SCREEN_H__

#include <stdbool.h>
#include <stdint.h>
#include "list.h"
#include "font.h"
#include "sauce.h"
#include "palette.h"

#define TILE_DEFAULT_FG     0x07
#define TILE_DEFAULT_BG     0x00
#define TILE_DEFAULT_CH     0x20
#define TILE_DEFAULT_ATTRIB 0x00
#define TILE_DEFAULT        {TILE_DEFAULT_FG, TILE_DEFAULT_BG, TILE_DEFAULT_CH, TILE_DEFAULT_ATTRIB}

#define ATTRIB_DEFAULT      0U
#define ATTRIB_BOLD         (1U << 1)
#define ATTRIB_DIM          (1U << 2)
#define ATTRIB_ITALICS      (1U << 3)
#define ATTRIB_UNDERLINE    (1U << 4)
#define ATTRIB_BLINK        (1U << 5)
#define ATTRIB_NEGATIVE     (1U << 7)
#define ATTRIB_CONCEAL      (1U << 8)
#define ATTRIB_CROSSED_OUT  (1U << 9)
#define ATTRIB_FG_256       (1U << 10)
#define ATTRIB_BG_256       (1U << 11)

#define ATTRIB_SET(x, attrib)   do { x |= attrib; } while(0)
#define ATTRIB_UNSET(x, attrib) do { x &= ~attrib; } while(0)

typedef struct screen_tile_s {
    uint8_t         fg;       /* Foreground color palette index */
    uint8_t         bg;       /* Background color palette index */
    uint8_t         ch;
    uint64_t        attrib;
} screen_tile;

typedef struct screen_s {
    screen_tile     *tile;
    int32_t         tiles;
    uint8_t         *buffer;
    int32_t         width;
    int32_t         height;
    int64_t         cursor;
    screen_tile     *current;
    sauce           *record;
    palette         *palette;
    font            *font;
} screen;


// Function prototypes

screen      *screen_create(int32_t width, int32_t height, sauce *);
void        screen_free(screen *display);
void        screen_putchar(screen *display, uint8_t ch, int32_t *x, int32_t *y, bool);
void        screen_insert_line(screen *display, int32_t y);
void        screen_reduce(screen *display, int32_t width, int32_t height);
void        screen_reset(screen *display);

bool        screen_tile_reset(screen_tile *);
screen_tile *screen_tile_append(screen *display);
bool        screen_tile_append_many(screen *, size_t);

#endif // __SCREEN_H__
