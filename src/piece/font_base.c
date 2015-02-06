#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "piece/font.h"
#include "piece/options.h"

piece_list *piece_fonts;

static piece_font_alias piece_font_map[] = {
    {"default",         "cp437_8x16",                 8, true,  NULL},
    {"msdos",           "cp437_8x16",                 9, true,  NULL},
    {"pc",              "cp437_8x16",                 9, true,  NULL},
    {"greek",           "cp737_8x16",                 9, true,  NULL},
    {"baltic",          "cp775_8x16",                 9, true,  NULL},
    {"latin",           "cp850_8x16",                 9, true,  NULL},
    {"latin1",          "cp850_8x16",                 9, true,  NULL},
    {"latin2",          "cp852_8x16",                 9, true,  NULL},
    {"cyrillic",        "cp855_8x16",                 9, true,  NULL},
    {"turkish",         "cp857_8x16",                 9, true,  NULL},
    {"latin2",          "cp852_8x16",                 9, true,  NULL},
    {"brazillian",      "cp860_8x16",                 9, true,  NULL},
    {"portuguese",      "cp860_8x16",                 9, true,  NULL},
    {"icelandic",       "cp861_8x16",                 9, true,  NULL},
    {"hebrew",          "cp862_8x16",                 9, true,  NULL},
    {"canadian",        "cp863_8x16",                 9, true,  NULL},
    {"nordic",          "cp865_8x16",                 9, true,  NULL},
    {"russian",         "cp866_8x16",                 9, true,  NULL},

    {"amiga",           "topaz_a1200",                8, true,  NULL},
    {"a500",            "topaz_a500",                 8, true,  NULL},
    {"a1200",           "topaz_a1200",                8, true,  NULL},
    {"topaz",           "topaz_a1200",                8, true,  NULL},
    {"topaz+",          "topazplus_a1200",            8, true,  NULL},

    {"atari",           "atascii_international_8x16", 8, true,  NULL},
    {"atascii",         "atascii_international_8x16", 8, true,  NULL},

    {"c64",             "petscii_8x8",                8, true,  NULL},
    {"petscii",         "petscii_8x8",                8, true,  NULL},

    {NULL,              NULL,                         0, false, NULL}
};

void piece_font_init_alias(void)
{
    for (uint32_t i = 0; piece_font_map[i].alias != NULL; ++i) {
        piece_font_alias_add(&piece_font_map[i]);
    }
}

void piece_font_iter(piece_list_iterator iterator)
{
    piece_list_foreach(piece_fonts, iterator);
}

void piece_font_free_item(void *element)
{
    piece_font *font = element;
    if (font->aliases) {
        free(font->alias);
    }
}

void piece_font_free(void)
{
    piece_list_free(piece_fonts);
}

piece_font /*@null@*/ *piece_font_by_name(const char *name)
{
    piece_list_node *node;
    for (node = piece_fonts->head; node != NULL; node = node->next) {
        piece_font *curr = node->data;
        if (!strcmp(curr->name, name)) {
            return curr;
        }
    }

    return NULL;
}

void piece_font_alias_add(piece_font_alias *alias)
{
    piece_list_node *node;
    piece_font_alias **tmp;
    dprintf("piece: adding alias %s -> %s\n", alias->alias, alias->font_name);
    for (node = piece_fonts->head; node != NULL; node = node->next) {
        piece_font *font = node->data;
        if (!strcmp(font->name, alias->font_name)) {
            tmp = realloc(
                font->alias,
                (font->aliases + 1) * sizeof(piece_font_alias *)
            );
            if (tmp == NULL) {
                fprintf(stderr, "piece: out of memory adding font alias\n");
                return;
            }
            font->alias = tmp;
            font->alias[font->aliases++] = alias;
            return;
        }
    }
    fprintf(stderr, "piece: no font named %s (for alias %s)\n",
                    alias->font_name,
                    alias->alias);
}
