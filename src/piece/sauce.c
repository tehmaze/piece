#include <stdio.h>
#include <string.h>

#include "piece.h"
#include "piece/font.h"
#include "sauce.h"

piece_font_alias sauce_font_map[] = {
    {"IBM VGA",               "cp437_8x16",                 9, false, NULL},
    {"IBM VGA50",             "cp437_8x8",                  9, false, NULL},
    {"IBM VGA25G",            "cp437_8x16",                 8, false, NULL},
    {"IBM EGA",               "cp437_8x14",                 8, false, NULL},
    {"IBM EGA43",             "cp437_8x8",                  9, false, NULL},
    {"IBM VGA 437",           "cp437_8x16",                 9, false, NULL},
    {"IBM VGA50 437",         "cp437_8x8",                  9, false, NULL},
    {"IBM VGA 737",           "cp437_8x16",                 9, false, NULL},
    {"IBM VGA25G 737",        "cp437_8x16",                 8, false, NULL},
    {"IBM VGA 775",           "cp437_8x16",                 9, false, NULL},
    {"IBM VGA25G 775",        "cp437_8x16",                 8, false, NULL},
    {"IBM VGA 850",           "cp850_8x16",                 9, false, NULL},
    {"IBM VGA25G 850",        "cp850_8x16",                 8, false, NULL},
    {"IBM VGA 852",           "cp852_8x16",                 9, false, NULL},
    {"IBM VGA25G 852",        "cp852_8x16",                 8, false, NULL},
    {"IBM VGA 855",           "cp855_8x16",                 9, false, NULL},
    {"IBM VGA25G 855",        "cp855_8x16",                 8, false, NULL},
    {"IBM VGA 857",           "cp857_8x16",                 9, false, NULL},
    {"IBM VGA25G 857",        "cp857_8x16",                 8, false, NULL},
    {"IBM VGA 860",           "cp860_8x16",                 9, false, NULL},
    {"IBM VGA25G 860",        "cp860_8x16",                 8, false, NULL},
    {"IBM VGA 861",           "cp861_8x16",                 9, false, NULL},
    {"IBM VGA25G 861",        "cp861_8x16",                 8, false, NULL},
    {"IBM VGA 862",           "cp862_8x16",                 9, false, NULL},
    {"IBM VGA25G 862",        "cp862_8x16",                 8, false, NULL},
    {"IBM VGA 863",           "cp863_8x16",                 9, false, NULL},
    {"IBM VGA25G 863",        "cp863_8x16",                 8, false, NULL},
    {"IBM VGA 865",           "cp865_8x16",                 9, false, NULL},
    {"IBM VGA25G 865",        "cp865_8x16",                 8, false, NULL},
    {"IBM VGA 866",           "cp866_8x16",                 9, false, NULL},
    {"IBM VGA25G 866",        "cp866_8x16",                 8, false, NULL},
    {"IBM VGA 869",           "cp869_8x16",                 9, false, NULL},
    {"IBM VGA25G 869",        "cp869_8x16",                 8, false, NULL},
    {"Amiga Topaz 1",         "topaz_a500",                 8, false, NULL},
    {"Amiga Topaz 1+",        "topazplus_a500",             8, false, NULL},
    {"Amiga Topaz 2",         "topaz_a1200",                8, false, NULL},
    {"Amiga Topaz 2+",        "topazplus_a1200",            8, false, NULL},
    {"Amiga P0T-NOoDLE",      "p0t_noodle",                 8, false, NULL},
    {"Amiga MicroKnight",     "microknight",                8, false, NULL},
    {"Amiga MicroKnight+",    "microknightplus",            8, false, NULL},
    {"Amiga mOsOul",          "mo_soul",                    8, false, NULL},
    {"Atari ATASCII",         "atascii_international_8x16", 8, false, NULL},
    {"C64 PETSCII unshifted", "petscii_8x8",                8, false, NULL},
    {"C64 PETSCII shifted",   "petscii_8x8",                8, false, NULL},
    {NULL,                    NULL,                         0, false, NULL}
};

piece_font *sauce_font(sauce *record)
{
    if (record == NULL) {
        return NULL;
    }
    return piece_font_by_name(record->filler);
}

void sauce_init(void)
{
    for (uint32_t i = 0; sauce_font_map[i].alias != NULL; ++i) {
        piece_font_alias_add(&sauce_font_map[i]);
    }
}
