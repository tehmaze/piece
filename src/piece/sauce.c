#include <stdio.h>
#include <string.h>

#include "piece.h"
#include "piece/font.h"
#include "sauce.h"

piece_font_alias sauce_font_map[] = {
    {"IBM VGA",               "ibm_dos_cp437_8x16",                 9, false, NULL},
    {"IBM VGA50",             "ibm_dos_cp437_8x8",                  9, false, NULL},
    {"IBM VGA25G",            "ibm_dos_cp437_8x16",                 8, false, NULL},
    {"IBM EGA",               "ibm_dos_cp437_8x14",                 8, false, NULL},
    {"IBM EGA43",             "ibm_dos_cp437_8x8",                  9, false, NULL},
    {"IBM VGA 437",           "ibm_dos_cp437_8x16",                 9, false, NULL},
    {"IBM VGA50 437",         "ibm_dos_cp437_8x8",                  9, false, NULL},
    {"IBM VGA 737",           "ibm_dos_cp437_8x16",                 9, false, NULL},
    {"IBM VGA25G 737",        "ibm_dos_cp437_8x16",                 8, false, NULL},
    {"IBM VGA 775",           "ibm_dos_cp437_8x16",                 9, false, NULL},
    {"IBM VGA25G 775",        "ibm_dos_cp437_8x16",                 8, false, NULL},
    {"IBM VGA 850",           "ibm_dos_cp850_8x16",                 9, false, NULL},
    {"IBM VGA25G 850",        "ibm_dos_cp850_8x16",                 8, false, NULL},
    {"IBM VGA 852",           "ibm_dos_cp852_8x16",                 9, false, NULL},
    {"IBM VGA25G 852",        "ibm_dos_cp852_8x16",                 8, false, NULL},
    {"IBM VGA 855",           "ibm_dos_cp855_8x16",                 9, false, NULL},
    {"IBM VGA25G 855",        "ibm_dos_cp855_8x16",                 8, false, NULL},
    {"IBM VGA 857",           "ibm_dos_cp857_8x16",                 9, false, NULL},
    {"IBM VGA25G 857",        "ibm_dos_cp857_8x16",                 8, false, NULL},
    {"IBM VGA 860",           "ibm_dos_cp860_8x16",                 9, false, NULL},
    {"IBM VGA25G 860",        "ibm_dos_cp860_8x16",                 8, false, NULL},
    {"IBM VGA 861",           "ibm_dos_cp861_8x16",                 9, false, NULL},
    {"IBM VGA25G 861",        "ibm_dos_cp861_8x16",                 8, false, NULL},
    {"IBM VGA 862",           "ibm_dos_cp862_8x16",                 9, false, NULL},
    {"IBM VGA25G 862",        "ibm_dos_cp862_8x16",                 8, false, NULL},
    {"IBM VGA 863",           "ibm_dos_cp863_8x16",                 9, false, NULL},
    {"IBM VGA25G 863",        "ibm_dos_cp863_8x16",                 8, false, NULL},
    {"IBM VGA 865",           "ibm_dos_cp865_8x16",                 9, false, NULL},
    {"IBM VGA25G 865",        "ibm_dos_cp865_8x16",                 8, false, NULL},
    {"IBM VGA 866",           "ibm_dos_cp866_8x16",                 9, false, NULL},
    {"IBM VGA25G 866",        "ibm_dos_cp866_8x16",                 8, false, NULL},
    {"IBM VGA 869",           "ibm_dos_cp869_8x16",                 9, false, NULL},
    {"IBM VGA25G 869",        "ibm_dos_cp869_8x16",                 8, false, NULL},
    {"Amiga Topaz 1",         "amiga_topaz_a500_8x16",            8, false, NULL},
    {"Amiga Topaz 1+",        "amiga_topazplus_a500_8x16",        8, false, NULL},
    {"Amiga Topaz 2",         "amiga_topaz_a1200_8x16",           8, false, NULL},
    {"Amiga Topaz 2+",        "amiga_topazplus_a1200_8x16",       8, false, NULL},
    {"Amiga P0T-NOoDLE",      "amiga_p0t_noodle_8x16",            8, false, NULL},
    {"Amiga MicroKnight",     "amiga_microknight_8x16",           8, false, NULL},
    {"Amiga MicroKnight+",    "amiga_microknightplus_8x16",       8, false, NULL},
    {"Amiga mOsOul",          "amiga_mo_soul_8x16",               8, false, NULL},
    {"Atari ATASCII",         "atascii_international_8x16", 8, false, NULL},
    {"C64 PETSCII unshifted", "pet_1_8x8",                  8, false, NULL},
    {"C64 PETSCII shifted",   "pet_2_8x8",                  8, false, NULL},
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
