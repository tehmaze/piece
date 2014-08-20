#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "screen.h"
#include "util.h"

static void screen_tile_free(void *element)
{
    free(element);
}

screen *screen_create(int32_t width, int32_t height, sauce *record)
{
    screen *display;
    list *tiles;
    tile *current;

    display = allocate(sizeof(screen));
    tiles = allocate(sizeof(list));
    list_new(tiles, screen_tile_free);
    current = allocate(sizeof(tile));

    printf("setting up %dx%d screen\n", width, height);

    display->tiles = tiles;
    display->width = width;
    display->height = height;
    display->current = current;
    display->current->fg = TILE_DEFAULT_FG;
    display->current->bg = TILE_DEFAULT_BG;
    display->current->ch = TILE_DEFAULT_CH;
    display->current->attrib = TILE_DEFAULT_ATTRIB;
    display->current->next = NULL;
    display->record = record;
    display->palette = NULL;
    display->font = NULL;

    for(int32_t y = 0; y < height; ++y) {
        for (int32_t x = 0; x < width; ++x) {
            if (screen_tile_append(display) == NULL) {
                exit(0);
            }
        }
    }

    return display;
}

void screen_free(screen *display)
{
    //free(display->buffer);
    //display->buffer = NULL;
    free(display->current);
    display->current = NULL;
    list_free(display->tiles);
    display->tiles = NULL;
    sauce_free(display->record);
    display->record = NULL;
    free(display);
}

void screen_putchar(screen *display, unsigned char ch, int32_t *x, int32_t *y)
{
    int64_t i;
    uint8_t tmp;
    display->cursor = (display->width * (*y)) + (*x);
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

    display->cursor++;
    (*x)++;
    if (*x == (int32_t) display->width) {
        *x = 0;
        (*y)++;
    }
}

void screen_insert_line(screen *display, int32_t y)
{
    int64_t offset = (display->width * y), i;
    list_node *node = display->tiles->head;
    for (; offset > 0; offset--) {
        node = node->next;
    }

    // Insert order doesn't really matter, so we're adding in front
    for (i = 0; i < display->width; i++) {
        tile *current = allocate(sizeof(tile));
        current->next = node->next->data;
        current->fg = ((tile *) node->data)->fg;
        current->bg = ((tile *) node->data)->bg;
        current->ch = TILE_DEFAULT_CH;
        current->attrib = ((tile *) node->data)->attrib;

        list_node *new = allocate(sizeof(list_node));
        memcpy(new->data, current, sizeof(tile));
        new->next = node->next;
        node->next = new;
    }
}

void screen_reduce(screen *display, int32_t width, int32_t height)
{
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
    current->attrib = TILE_DEFAULT_ATTRIB;
    printf("reset %p ", item);
    return true;
}

tile *screen_tile_append(screen *display)
{
    tile *current = allocate(sizeof(tile));
    current->next = NULL;
    current->fg = TILE_DEFAULT_FG;
    current->bg = TILE_DEFAULT_BG;
    current->ch = TILE_DEFAULT_CH;
    current->attrib = TILE_DEFAULT_ATTRIB;

    // Advance to last tile
    list_append(display->tiles, current);

    // Update screen height
    display->height = 1 + ((list_size(display->tiles) - 1) / display->width);

    return (tile *) display->tiles->tail;
}
