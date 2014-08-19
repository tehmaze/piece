#ifndef __SCREEN_H__
#define __SCREEN_H__

#include <stdbool.h>
#include <stdint.h>
#include "list.h"
#include "sauce.h"

#define TILE_DEFAULT_FG     0x07
#define TILE_DEFAULT_BG     0x00
#define TILE_DEFAULT_CH     0x20

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

typedef struct tile_s {
    unsigned char fg;       /* Foreground color palette index */
    unsigned char bg;       /* Background color palette index */
    unsigned char ch;
    struct tile_s *next;
} tile;

typedef struct screen_s {
    list          *tiles;
    uint8_t       *buffer;
    uint32_t      width;
    uint32_t      height;
    int32_t       cursor;
    uint64_t      attrib;
    tile          *current;
    sauce         *record;
} screen;


// Function prototypes

screen *screen_create(uint32_t, uint32_t, sauce *);
void    screen_free(screen *);
void    screen_putchar(screen *, unsigned char);
void    screen_reset(screen *);

bool    screen_tile_reset(void *);
tile   *screen_tile_append(screen *);

#endif // __SCREEN_H__
