#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "screen.h"

screen *screen_create(unsigned int width, unsigned int height, sauce *record)
{
    unsigned int x, y;
    screen *display;
    list *tiles;
    tile *current;

    display = malloc(sizeof(screen));
    if (display == NULL) {
        fprintf(stderr, "out of memory\n");
        return NULL;
    }
    tiles = malloc(sizeof(list));
    if (tiles == NULL) {
        free(display);
        fprintf(stderr, "out of memory\n");
        return NULL;
    }
    list_new(tiles, sizeof(tile), NULL);
    current = malloc(sizeof(tile));
    if (current == NULL) {
        free(tiles);
        free(display);
        fprintf(stderr, "out of memory\n");
        return NULL;
    }

    printf("setting up %dx%d screen\n", width, height);

    display->tiles = tiles;
    display->width = width;
    display->height = height;
    display->attrib = ATTRIB_DEFAULT;
    display->current = current;
    display->current->fg = TILE_DEFAULT_FG;
    display->current->bg = TILE_DEFAULT_BG;
    display->current->ch = TILE_DEFAULT_CH;
    display->current->next = NULL;
    display->record = record;

    for(y = 0; y < height; ++y) {
        for (x = 0; x < width; ++x) {
            if (screen_tile_append(display) == NULL) {
                exit(0);
            }
        }
    }

    return display;
}

void screen_free(screen *display)
{
    free(display->buffer);
    display->buffer = NULL;
    free(display->current);
    display->current = NULL;
    list_free(display->tiles);
    display->tiles = NULL;
    sauce_free(display->record);
    display->record = NULL;
    free(display);
}

void screen_putchar(screen *display, unsigned char ch)
{
    int64_t i;
    while (display->cursor >= list_size(display->tiles))
    {
        for (i = 0; i < display->width; ++i) {
            screen_tile_append(display);
        }
    }

    list_node *node = display->tiles->head;
    for (i = 0; i < display->cursor; ++i) {
        node = node->next;
    }
    tile *current = node->data;

    if (display->attrib & ATTRIB_NEGATIVE) {
        current->fg = display->current->bg;
        current->bg = display->current->fg;
    } else {
        current->fg = display->current->fg;
        current->bg = display->current->bg;
    }
    if (display->attrib & ATTRIB_BOLD &&
      !(display->attrib & ATTRIB_FG_256)) {
        current->fg += 8;
    }
    if (display->attrib & ATTRIB_BLINK &&
      !(display->attrib & ATTRIB_BG_256)) {
        if (sauce_flag_non_blink(display->record)) {
            current->bg += 8;
        }
    }
    current->ch = ch;
    display->cursor++;
}

void screen_reset(screen *display)
{
    list_foreach(display->tiles, screen_tile_reset);
}

bool screen_tile_reset(void *item)
{
    tile *current = (tile *) item;
    current->fg = TILE_DEFAULT_FG;
    current->bg = TILE_DEFAULT_BG;
    current->ch = TILE_DEFAULT_CH;
    return true;
}

tile *screen_tile_append(screen *display)
{
    tile *current = malloc(sizeof(tile));
    if (current == NULL) {
        fprintf(stderr, "out of memory\n");
        return NULL;
    }

    current->next = NULL;
    current->fg = TILE_DEFAULT_FG;
    current->bg = TILE_DEFAULT_BG;
    current->ch = TILE_DEFAULT_CH;

    // Advance to last tile
    list_append(display->tiles, &current);

    // Update screen height
    display->height = 1 + ((list_size(display->tiles) - 1) / display->width);

    return (tile *) display->tiles->tail;
}
