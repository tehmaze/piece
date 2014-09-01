#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "piece/chrfont.h"

int main(int argc, const char **argv)
{
    int status = 0;
    piece_chrfont *font = NULL;
    piece_chrfont_header *header = NULL;
    FILE *fd = NULL;

    if (argc != 2) {
        fprintf(stderr, "%s <filename>\n", argv[0]);
        return 1;
    }

    if ((fd = fopen(argv[1], "rb")) == NULL) {
        fprintf(stderr, "%s: could not open for reading\n", argv[1]);
        return 2;
    }
    font = piece_chrfont_read(fd, argv[1]);
    if (font == NULL) {
        fprintf(stderr, "%s: parsing failed\n", argv[1]);
        status = 3;
        goto exit_close;
    }

    header = font->header;
    printf("pointer at.: %ld (0x%04lx)\n", ftell(fd), ftell(fd));
    printf("file name..: %s\n", argv[1]);
    printf("description: ");
    for (int i = 0, l = strlen(header->description); i < l; i++) {
        if (header->description[i] == '\n') {
            printf("\n             ");
        }
        else if (header->description[i] != '\r') {
            printf("%c", header->description[i]);
        }
    }
    printf("\n");
    printf("font name..: %s (internal)\n", header->font_name_internal);
    printf("font name..: %s\n", (strlen(header->font_name) > 0)
                                ? header->font_name
                                : "(undefined)");
    printf("header size: %d\n", header->header_size);
    printf("file size..: %d\n", header->font_file_size);
    printf("signature..: 0x%02x", header->signature);
    if (header->signature == '+') {
        printf(" (stroke font)\n");
    } else {
        printf("\n");
    }
    printf("characters.: %d (0x%02x%02x)\n", header->characters,
                                             header->characters >> 8,
                                             header->characters & 0xff);
    printf("first char.: 0x%02x\n", header->first_char);

    for (uint16_t i = 0; i < header->characters; ++i) {
        printf("glyph %04x: offset %04x, width %d\n",
               i,
               header->character_offsets[i],
               header->character_widths[i]);
    }

    piece_chrfont_free(font);

exit_close:
    fclose(fd);

    return status;
}
