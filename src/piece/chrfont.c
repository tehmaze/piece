#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "piece/chrfont.h"
#include "piece/util.h"

static const char chrfont_id[8] = CHRFONT_ID;

piece_chrfont *
piece_chrfont_read(FILE *fd, const char *filename)
{
    piece_chrfont *font = piece_allocate(sizeof(piece_chrfont));
    piece_chrfont_header *header;
    char *description = NULL, ch;

    header = font->header = piece_allocate(sizeof(piece_chrfont_header));
    header->description = piece_allocate(sizeof(uint8_t) * 128);
    description = header->description;

    rewind(fd);
    fread(&header->id, 8, 1, fd);
    if (strncmp(header->id, chrfont_id, 8)) {
        fprintf(stderr, "%s: not a Borland CHR font\n", filename);
        fprintf(stderr, "%s: header identifier reads \"", filename);
        for (uint8_t i = 0; i < 8; i++) {
            fprintf(stderr, "0x%02x ", header->id[i]);
        }
        fprintf(stderr, "\"\n");
        free(header->description);
        free(header);
        return NULL;
    }

    for (uint16_t i = 0; i < 128; i++) {
        ch = fgetc(fd);
        if (ch == 0x1a) {
            *description = 0;
            break;
        } else {
            *description++ = ch;
        }
    }
    header->header_size = piece_fget16i(fd);
    fread(&header->font_name_internal, 4, 1, fd);
    header->font_file_size = piece_fget16i(fd);
    fread(&header->font_driver_version, 2, 1, fd);

    fseek(fd, 0x80, SEEK_SET);
    header->signature = fgetc(fd);
    header->characters = piece_fget16i(fd);
    header->first_char = fgetc(fd);
    header->stroke_offset = piece_fget16i(fd);
    header->scan_flag = fgetc(fd);
    header->top_of_capital_offset = fgetc(fd);
    header->baseline_offset = fgetc(fd);
    header->bottom_descender_offset = fgetc(fd);
    fread(&header->font_name, 4, 1, fd);

    header->character_offsets = piece_allocate(sizeof(uint16_t) *
                                               header->characters);
    header->character_widths = piece_allocate(sizeof(uint8_t) *
                                              header->characters);
    font->character = piece_allocate(sizeof(piece_chrfont_character) *
                                     header->characters);


    for (uint16_t i = 0; i < header->characters; ++i) {
        header->character_offsets[i] = piece_fget16i(fd);
    }
    for (uint16_t i = 0; i < header->characters; ++i) {
        header->character_widths[i] = fgetc(fd);
    }

    return font;
}

void piece_chrfont_free(piece_chrfont *font)
{
    if (font->header != NULL) {
        free(font->header->description);
        free(font->header);
        font->header = NULL;
    }
    if (font != NULL) {
        free(font);
        font = NULL;
    }
}
