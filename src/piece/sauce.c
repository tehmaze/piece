#include <stdio.h>
#include <string.h>

#include "piece.h"
#include "piece/font.h"
#include "sauce.h"

typedef struct sauce_font_alias_s {
    const char *sauce_name; /* Font name in the SAUCE 00.5 spec */
    const char *piece_name; /* Font name known by us */
    int        bits;
    piece_font *font;
} sauce_font_alias;

sauce_font_alias sauce_font_map[] = {
    {"IBM VGA",               "cp437_8x16",      9, NULL},
    {"IBM VGA50",             "cp437_8x8",       9, NULL},
    {"IBM VGA25G",            "cp437_8x16",      8, NULL},
    {"IBM EGA",               "cp437_8x14",      8, NULL},
    {"IBM EGA43",             "cp437_8x8",       9, NULL},
    {"IBM VGA 437",           "cp437_8x16",      9, NULL},
    {"IBM VGA50 437",         "cp437_8x8",       9, NULL},
    {"IBM VGA 737",           "cp437_8x16",      9, NULL},
    {"IBM VGA25G 737",        "cp437_8x16",      8, NULL},
    {"IBM VGA 775",           "cp437_8x16",      9, NULL},
    {"IBM VGA25G 775",        "cp437_8x16",      8, NULL},
    {"IBM VGA 850",           "cp850_8x16",      9, NULL},
    {"IBM VGA25G 850",        "cp850_8x16",      8, NULL},
    {"IBM VGA 852",           "cp852_8x16",      9, NULL},
    {"IBM VGA25G 852",        "cp852_8x16",      8, NULL},
    {"IBM VGA 855",           "cp855_8x16",      9, NULL},
    {"IBM VGA25G 855",        "cp855_8x16",      8, NULL},
    {"IBM VGA 857",           "cp857_8x16",      9, NULL},
    {"IBM VGA25G 857",        "cp857_8x16",      8, NULL},
    {"IBM VGA 860",           "cp860_8x16",      9, NULL},
    {"IBM VGA25G 860",        "cp860_8x16",      8, NULL},
    {"IBM VGA 861",           "cp861_8x16",      9, NULL},
    {"IBM VGA25G 861",        "cp861_8x16",      8, NULL},
    {"IBM VGA 862",           "cp862_8x16",      9, NULL},
    {"IBM VGA25G 862",        "cp862_8x16",      8, NULL},
    {"IBM VGA 863",           "cp863_8x16",      9, NULL},
    {"IBM VGA25G 863",        "cp863_8x16",      8, NULL},
    {"IBM VGA 865",           "cp865_8x16",      9, NULL},
    {"IBM VGA25G 865",        "cp865_8x16",      8, NULL},
    {"IBM VGA 866",           "cp866_8x16",      9, NULL},
    {"IBM VGA25G 866",        "cp866_8x16",      8, NULL},
    {"IBM VGA 869",           "cp869_8x16",      9, NULL},
    {"IBM VGA25G 869",        "cp869_8x16",      8, NULL},
    {"Amiga Topaz 1",         "topaz_a500",      8, NULL},
    {"Amiga Topaz 1+",        "topazplus_a500",  8, NULL},
    {"Amiga Topaz 2",         "topaz_a1200",     8, NULL},
    {"Amiga Topaz 2+",        "topazplus_a1200", 8, NULL},
    {"Amiga P0T-NOoDLE",      "p0t_noodle",      8, NULL},
    {"Amiga MicroKnight",     "microknight",     8, NULL},
    {"Amiga MicroKnight+",    "microknightplus", 8, NULL},
    {"Amiga mOsOul",          "mo_soul",         8, NULL},
    {"Atari ATASCII",         "atari",           8, NULL},
    {"C64 PETSCII unshifted", "petscii",         8, NULL},
    {"C64 PETSCII shifted",   "petscii_shifted", 8, NULL},
    {NULL,                    NULL,              0, NULL}
};

piece_font *sauce_font(sauce *record)
{
    if (record == NULL) {
        return NULL;
    }
    for (uint16_t i = 0; sauce_font_map[i].sauce_name != NULL; ++i) {
        if (!strcmp(record->filler, sauce_font_map[i].sauce_name)) {
            return sauce_font_map[i].font;
        }
    }
    return NULL;
}

void sauce_init(void)
{
    for (uint16_t i = 0; sauce_font_map[i].sauce_name != NULL; ++i) {
        sauce_font_map[i].font = piece_font_by_name(sauce_font_map[i].piece_name);
        if (sauce_font_map[i].font == NULL) {
            fprintf(stderr, "sauce: font \"%s\" not found (known as \"%s\")\n",
                            sauce_font_map[i].sauce_name,
                            sauce_font_map[i].piece_name);
        }
    }
}
