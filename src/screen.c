#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "options.h"
#include "screen.h"
#include "util.h"

static screen_tile screen_tile_defaults = TILE_DEFAULT;

screen *screen_create(int32_t width, int32_t height, sauce *record)
{
    screen *display;

    display = allocate(sizeof(screen));
    display->tiles = width * height;
    //display->tile = allocate(sizeof(screen_tile) * display->tiles);
    display->tile = calloc(display->tiles, sizeof(screen_tile));
    if (display->tile == NULL) {
        fprintf(stderr, "out of memory trying to allocate %d tiles (%lub)\n",
                        display->tiles, display->tiles * sizeof(screen_tile));
        return NULL;
    }
    display->size.width = width;
    display->size.height = height;
    display->current = allocate(sizeof(screen_tile));
    screen_tile_reset(display->current);
    display->record = record;
    display->palette = NULL;
    display->font = NULL;

    dprintf("creating %dx%d screen with %d tiles\n", width, height,
                                                     display->tiles);

    screen_tile *tile = display->tile;
    for (int32_t i = 0; i < display->tiles; ++i) {
        screen_tile_reset(tile++);
    }

    return display;
}

void screen_free(screen *display)
{
    if (display == NULL)
        return;

    if (display->current != NULL) {
        free(display->current);
        display->current = NULL;
    }

    if (display->tiles > 0) {
        free(display->tile);
        display->tiles = 0;
    }

    if (display->record != NULL) {
        sauce_free(display->record);
        display->record = NULL;
    }

    free(display);
}

void screen_putchar(screen *display, unsigned char ch, int32_t *x, int32_t *y,
                    bool update_cursor)
{
    uint8_t tmp;
    display->cursor = (display->size.width * (*y)) + (*x);
    while (display->cursor >= display->tiles)
    {
        if (!screen_tile_append_many(display, display->size.width))
            return;
    }

    screen_tile *current = &display->tile[display->cursor];
    current->fg = display->current->fg;
    current->bg = display->current->bg;
    current->ch = ch;
    current->attrib = display->current->attrib;

    if (current->attrib & ATTRIB_BOLD) {
        current->fg += 8;
    }
    if (current->attrib & ATTRIB_BLINK) {
        current->bg += 8;
    }
    if (current->attrib & ATTRIB_NEGATIVE) {
        tmp = current->fg;
        current->fg = current->bg;
        current->bg = tmp;
    }

    if (update_cursor) {
        display->cursor++;
        (*x)++;
        if (*x == (int32_t) display->size.width) {
            *x = 0;
            (*y)++;
        }
    }
}

void screen_insert_line(screen *display, int32_t y)
{
    int64_t offset = (display->size.width * y), i;
    if (!screen_tile_append_many(display, display->size.width))
        return;
    /* Possibly a memmove is more efficient? */
    memcpy(display->tile, display->tile + offset, display->tiles - offset);
    for (i = 0; i < display->size.width; ++i) {
        screen_tile_reset(&display->tile[offset + i]);
    }
}

void screen_reduce(screen *display, int32_t width, int32_t height)
{
    /*
    int64_t limit = width * height,
            total = list_size(display->tiles),
            i;

    if (total > limit) {
        list_node *node = display->tiles->head, *next = NULL;
        for (i = 0; i < limit; ++i) {
            node = node->next;
        }
        display->tiles->tail = node;
        for (i = limit; i < total; ++i) {
            next = node->next;
            node->next = NULL;
            free(node);
        }
        display->tiles->tail->next = NULL;
    }
    */
}

void screen_reset(screen *display)
{
    for (int32_t i = 0; i < display->tiles; ++i) {
        screen_tile_reset(&display->tile[i]);
    }
}

bool screen_tile_reset(screen_tile *tile)
{
    tile->fg = TILE_DEFAULT_FG;
    tile->bg = TILE_DEFAULT_BG;
    tile->ch = TILE_DEFAULT_CH;
    tile->attrib = TILE_DEFAULT_ATTRIB;
    /*
    memcpy(tile, &screen_tile_defaults, 4);
    */
    return true;
}

screen_tile *screen_tile_append(screen *display)
{
    screen_tile *current = allocate(sizeof(screen_tile));
    current->fg = TILE_DEFAULT_FG;
    current->bg = TILE_DEFAULT_BG;
    current->ch = TILE_DEFAULT_CH;
    current->attrib = TILE_DEFAULT_ATTRIB;

    // Update screen height
    display->tiles++;
    display->size.height = 1 + (display->tiles - 1) / display->size.width;

    return current;
}

bool screen_tile_append_many(screen *display, size_t n)
{
    uint32_t total = display->tiles + n;
    dprintf("screen: expanding from %d to %d tiles\n", display->tiles, total);
    screen_tile *tiles = realloc(display->tile, sizeof(screen_tile) * total);
    if (tiles == NULL) {
        fprintf(stderr, "out of memory trying to resize from %d to %d tiles\n",
                        display->tiles, total);
        return false;
    }
    display->tile = tiles;

    screen_tile *tile = display->tile;
    tile += display->tiles;
    for (uint64_t i = display->tiles; i < total; ++i) {
        screen_tile_reset(tile++);
    }
    display->tiles = total;
    display->size.height = 1 + (display->tiles - 1) / display->size.width;
    return true;
}
