#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "font.h"
#include "sauce.h"

#define SAUCE_READ_STRING(fd, record, field) do {                   \
    fread(record->field, sizeof(record->field), 1, fd);             \
    record->field[sizeof(record->field) - 1] = 0x00;                \
    if (ferror(fd)) {                                               \
        fprintf(stderr,                                             \
        "error %d reading SAUCE field %s\n", ferror(fd), #field);   \
        free(record);                                               \
        return NULL;                                                \
    }                                                               \
    } while(0)

#define SAUCE_READ_VALUE(fd, record, field) do {                    \
    fread(&(record->field), sizeof(record->field), 1, fd);          \
    if (ferror(fd)) {                                               \
        fprintf(stderr,                                             \
        "error %d reading SAUCE field %s\n", ferror(fd), #field);   \
        free(record);                                               \
        return NULL;                                                \
    }                                                               \
    } while(0)


typedef struct sauce_font_alias_s {
    const char *sauce_name; /* Font name in the SAUCE 00.5 spec */
    const char *piece_name; /* Font name known by us */
    int        bits;
    font       *font;
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

sauce *sauce_read(FILE *fd)
{
    sauce *record = malloc(sizeof(sauce));
    if (record == NULL) {
        return NULL;
    }
    memset(record, 0, sizeof(sauce));

    if (fseek(fd, -SAUCE_RECORD_SIZE, SEEK_END) != 0) {
        fprintf(stderr, "sauce: fseek() failed\n");
        free(record);
        return NULL;
    }
    SAUCE_READ_VALUE(fd, record, id);
    if (memcmp(record->id, SAUCE_ID, sizeof(record->id) - 1)) {
        free(record);
        return NULL;
    }

    SAUCE_READ_VALUE(fd, record, version);
    SAUCE_READ_STRING(fd, record, title);
    SAUCE_READ_STRING(fd, record, author);
    SAUCE_READ_STRING(fd, record, group);
    SAUCE_READ_VALUE(fd, record, date);
    SAUCE_READ_VALUE(fd, record, file_size);
    SAUCE_READ_VALUE(fd, record, data_type);
    SAUCE_READ_VALUE(fd, record, file_type);
    SAUCE_READ_VALUE(fd, record, tinfo);
    SAUCE_READ_VALUE(fd, record, comments);
    SAUCE_READ_VALUE(fd, record, flags);
    SAUCE_READ_STRING(fd, record, filler);

    if (record->comments > 0) {
        record->comment = malloc(record->comments * sizeof(*record->comment));
        if (record->comment == NULL) {
            free(record);
            return NULL;
        }
        sauce_read_comments(fd, record->comment, record->comments);
    } else {
        record->comment = NULL;
    }

    return record;
}

void sauce_read_comments(FILE *fd, char **comment, int32_t comments)
{
    int32_t i;
    char id[6];

    long offset = SAUCE_RECORD_SIZE + 5 + SAUCE_COMMENT_SIZE * comments;
    if (!fseek(fd, 0 - offset, SEEK_END)) {
        free(comment);
        return;
    }

    fread(id, sizeof(id) - 1, 1 , fd);
    id[sizeof(id) - 1] = 0x00;

    if (strcmp(id, SAUCE_COMMENT_ID)) {
        free(comment);
        return;
    }

    for (i = 0; i < comments; ++i) {
        char buf[SAUCE_COMMENT_SIZE + 1];
        memset(buf, 0, sizeof(buf));
        fread(buf, SAUCE_COMMENT_SIZE, 1, fd);
        buf[SAUCE_COMMENT_SIZE] = 0x00;

        if (!ferror(fd)) {
            comment[i] = strdup(buf);
            if (comment[i] == NULL) {
                free(comment);
                return;
            }
        } else {
            free(comment);
            return;
        }
    }
}

void sauce_free(sauce *record)
{
    //free(record->comment);
    free(record);
}

bool sauce_flag_non_blink(sauce *record) {
    return (bool) (record != NULL && record->flags.flag_b);
}

uint8_t sauce_flag_letter_spacing(sauce *record) {
    return (record != NULL && record->flags.flag_ls == SAUCE_LS_8PIXEL) ? 8 : 9;
}

font *sauce_font(sauce *record)
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

size_t sauce_size(sauce *record)
{
    size_t size = 0;
    if (record != NULL) {
        size += SAUCE_RECORD_SIZE;
        if (record->comments) {
            size += 5 + SAUCE_COMMENT_SIZE * record->comments;
        }
    }
    return size;
}

void sauce_init(void)
{
    for (uint16_t i = 0; sauce_font_map[i].sauce_name != NULL; ++i) {
        sauce_font_map[i].font = font_by_name(sauce_font_map[i].piece_name);
        if (sauce_font_map[i].font == NULL) {
            fprintf(stderr, "sauce: font \"%s\" not found (known as \"%s\")\n",
                            sauce_font_map[i].sauce_name,
                            sauce_font_map[i].piece_name);
        }
    }
}
