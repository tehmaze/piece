#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "piece/options.h"
#include "piece/screen.h"
#include "piece/util.h"

piece_screen *piece_screen_new(int32_t width, int32_t height, sauce *record)
{
    piece_screen *display;

    display = piece_allocate(sizeof(piece_screen));
    if (record != NULL && (
        record->data_type == SAUCE_DATA_TYPE_CHARACTER &&
        record->file_type == SAUCE_FILE_TYPE_RIPSCRIPT
        ) || (
        record->data_type == SAUCE_DATA_TYPE_BITMAP
        ) || (
        record->data_type == SAUCE_DATA_TYPE_VECTOR
        )) {

        /* We don't use tiles */
        display->tiles = 0;
        display->tile = NULL;

        if (record->data_type == SAUCE_DATA_TYPE_BITMAP) {
            display->buffer = calloc(
                record->tinfo[0] * record->tinfo[1],    /* width x height */
                sizeof(uint32_t)                        /* 32 bit color */
            );
        }

    } else {
        display->tiles = width * height;
        display->tile = calloc(display->tiles, sizeof(piece_screen_tile));
        if (display->tile == NULL) {
            fprintf(stderr, "out of memory trying to allocate %d tiles (%lub)\n",
                            display->tiles, display->tiles * sizeof(piece_screen_tile));
            return NULL;
        }
        display->size.width = width;
        display->size.height = height;
        display->current = piece_allocate(sizeof(piece_screen_tile));
        piece_screen_tile_reset(display->current);
        display->record = record;
        display->palette = NULL;
        display->font = NULL;

        dprintf("creating %dx%d screen with %d tiles\n", width, height,
                                                         display->tiles);

        piece_screen_tile *tile = display->tile;
        for (int32_t i = 0; i < display->tiles; ++i) {
            piece_screen_tile_reset(tile++);
        }
    }

    return display;
}

void piece_screen_free(piece_screen *display)
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

    if (display->palette != NULL) {
        free(display->palette->color);
        free(display->palette);
        display->palette = NULL;
    }

    if (display->font != NULL) {
        free(display->font->glyphs);
        free(display->font);
        display->font = NULL;
    }

    if (display->record != NULL) {
        sauce_free(display->record);
        display->record = NULL;
    }

    free(display);
}

void piece_screen_putchar(piece_screen *display, unsigned char ch,
                          int32_t *x, int32_t *y,
                          bool update_cursor)
{
    uint8_t tmp;
    display->cursor = (display->size.width * (*y)) + (*x);
    while (display->cursor >= display->tiles)
    {
        if (!piece_screen_tile_append_many(display, display->size.width))
            return;
    }

    piece_screen_tile *current = &display->tile[display->cursor];
    current->fg = display->current->fg;
    current->bg = display->current->bg;
    current->ch = ch;
    current->attrib = display->current->attrib;

    if (current->attrib & PIECE_ATTRIB_BOLD) {
        current->fg += 8;
    }
    if (current->attrib & PIECE_ATTRIB_BLINK) {
        current->bg += 8;
    }
    if (current->attrib & PIECE_ATTRIB_NEGATIVE) {
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

void piece_screen_insert_line(piece_screen *display, int32_t y)
{
    int64_t offset = (display->size.width * y), i;
    if (!piece_screen_tile_append_many(display, display->size.width))
        return;
    /* Possibly a memmove is more efficient? */
    memcpy(display->tile, display->tile + offset, display->tiles - offset);
    for (i = 0; i < display->size.width; ++i) {
        piece_screen_tile_reset(&display->tile[offset + i]);
    }
}

bool piece_screen_reduce(piece_screen *display, int32_t width, int32_t height)
{
    uint32_t total = width * height;
    dprintf("screen: reducing from %d to %d tiles\n", display->tiles, total);
    piece_screen_tile *tiles = realloc(display->tile,
                                       sizeof(piece_screen_tile) * total);
    if (tiles == NULL) {
        fprintf(stderr, "out of memory trying to resize from %d to %d tiles\n",
                        display->tiles, total);
        return false;
    }
    display->tile = tiles;
    return true;
}

void piece_screen_reset(piece_screen *display)
{
    for (int32_t i = 0; i < display->tiles; ++i) {
        piece_screen_tile_reset(&display->tile[i]);
    }
}

bool piece_screen_tile_reset(piece_screen_tile *tile)
{
    tile->fg = PIECE_TILE_DEFAULT_FG;
    tile->bg = PIECE_TILE_DEFAULT_BG;
    tile->ch = PIECE_TILE_DEFAULT_CH;
    tile->attrib = PIECE_TILE_DEFAULT_ATTRIB;
    /*
    memcpy(tile, &screen_tile_defaults, 4);
    */
    return true;
}

piece_screen_tile *screen_tile_append(piece_screen *display)
{
    piece_screen_tile *current = piece_allocate(sizeof(piece_screen_tile));
    current->fg = PIECE_TILE_DEFAULT_FG;
    current->bg = PIECE_TILE_DEFAULT_BG;
    current->ch = PIECE_TILE_DEFAULT_CH;
    current->attrib = PIECE_TILE_DEFAULT_ATTRIB;

    // Update screen height
    display->tiles++;
    display->size.height = 1 + (display->tiles - 1) / display->size.width;

    return current;
}

bool piece_screen_tile_append_many(piece_screen *display, size_t n)
{
    uint32_t total = display->tiles + n;
    if (piece_options->verbose > 1) {
        printf("screen: expanding from %d to %d tiles\n", display->tiles, total);
    }
    piece_screen_tile *tiles = realloc(display->tile,
                                       sizeof(piece_screen_tile) * total);
    if (tiles == NULL) {
        fprintf(stderr, "out of memory trying to resize from %d to %d tiles\n",
                        display->tiles, total);
        return false;
    }
    display->tile = tiles;

    piece_screen_tile *tile = display->tile;
    tile += display->tiles;
    for (uint64_t i = display->tiles; i < total; ++i) {
        piece_screen_tile_reset(tile++);
    }
    display->tiles = total;
    display->size.height = 1 + (display->tiles - 1) / display->size.width;
    return true;
}
