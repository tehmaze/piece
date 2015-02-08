#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef DEBUG
#include <assert.h>
#endif

#include "piece/font.h"
#include "piece/list.h"
#include "piece/palette.h"
#include "piece/parser.h"
#include "piece/parser/xbin.h"
#include "piece/screen.h"
#include "piece/options.h"
#include "piece/palette.h"
#include "piece/util.h"

static void piece_screen_putchar_xbin(piece_screen *display, unsigned char ch,
                                      unsigned char attribute, int *x, int *y)
{
    display->current->bg = (attribute & 0xf0) >> 4;
    display->current->fg = (attribute & 0x0f);
    piece_screen_putchar(display, ch, x, y, true);
}

bool xbin_parser_probe(FILE *fd, const char *UNUSED(filename))
{
    xbin_header *header;
    bool score = false;

    header = piece_allocate(sizeof(xbin_header));
    if (fread(&header->id, 4, 1, fd) == 0 ||
        fread(&header->eof_char, 1, 1, fd) == 0 ||
        fread(&header->width, 2, 1, fd) == 0 ||
        fread(&header->height, 2, 1, fd) == 0 ||
        fread(&header->font_size, 1, 1, fd) == 0 ||
        fread(&header->flags, 1, 1, fd) == 0) {
        score = false;
        goto return_probe_free;
    }

    score = (
        !strncmp(header->id, XBIN_ID, XBIN_ID_LEN) &&
        header->width > 0 &&
        header->width < 161 &&
        header->font_size > 0 &&
        header->font_size < 33
    );

return_probe_free:
    free(header);
    return score;
}

piece_screen *xbin_parser_read(FILE *fd, const char *filename)
{
    struct stat st;
    unsigned char *p, *s;
    piece_screen *display = NULL;
    sauce *record = NULL;
    xbin_header *header = NULL;
    piece_palette *xbin_palette = NULL;
    piece_font *xbin_font = NULL;
    int16_t xbin_font_chars = 256;
    int64_t i, r, xbin_font_total, fsize;
    unsigned char ch, attribute;
    unsigned char *xbin_data = NULL, *tmp = NULL;
    int16_t repeat, method;
    int x = 0;
    int y = 0;
    uint8_t rgb[16][3];

    fseek(fd, 0, SEEK_SET);
    if (fstat(fileno(fd), &st) < 0) {
        fprintf(stderr, "%s: stat() failed\n", filename);
        return NULL;
    }

    p = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fileno(fd), 0);
    if (p == MAP_FAILED) {
        fprintf(stderr, "%s: mmap() failed\n", filename);
        goto return_bail;
    }
    s = p;

    record = sauce_read(fd);

    rewind(fd);
    header = (xbin_header *) p;
    p += sizeof(xbin_header);
    if (ferror(fd) || memcmp(&header->id, XBIN_ID, sizeof(header->id))) {
        fprintf(stderr, "%s: not an XBIN (got %s)\n", filename, header->id);
        goto return_free;
    }

    dprintf("width: %d\nheight: %d\n", header->width, header->height);
    dprintf("font_size: %d\n", header->font_size);
    dprintf("flag palette: %d\n", header->flags.flag_palette);
    dprintf("flag font: %d\n", header->flags.flag_font);
    dprintf("flag compress: %d\n", header->flags.flag_compress);
    dprintf("flag non_blink: %d\n", header->flags.flag_non_blink);
    dprintf("flag 512_chars: %d\n", header->flags.flag_512_chars);
    dprintf(".. at %lu\n", p - s);

    if (header->flags.flag_palette) {
        dprintf("%s: parsing palette\n", filename);
        xbin_palette = piece_allocate(sizeof(piece_palette));
        xbin_palette->name = "from file";
        xbin_palette->colors = 16;
        i = sizeof(piece_rgba_color) * 16;
        xbin_palette->color = piece_allocate(i);
        memcpy(rgb, p, 48);
        for (i = 0; i < 16; ++i) {
            xbin_palette->color[i] = PIECE_RGB(
                rgb[i][0] << 4,
                rgb[i][1] << 4,
                rgb[i][2] << 4
            );
        }
        p += 48;
    } else {
        dprintf("%s: using ega palette\n", filename);
    }
    dprintf(".. at %lu (after palette)\n", p - s);

    if (header->flags.flag_font) {
        dprintf("%s: parsing font\n", filename);
        xbin_font_chars = header->flags.flag_512_chars ? 512 : 256;
        xbin_font_total = xbin_font_chars * header->font_size;
        xbin_font = piece_allocate(sizeof(piece_font));
        xbin_font->name = "xbin";
        xbin_font->w = 9;
        xbin_font->h = header->font_size;
        xbin_font->l = xbin_font_chars;
        xbin_font->glyphs = piece_allocate(xbin_font_total);
        memcpy((unsigned char *) xbin_font->glyphs, p, xbin_font_total);
        p += xbin_font_total;
    } else {
        dprintf("%s: using default font\n", filename);
        xbin_font = NULL;
    }
    dprintf(".. at %lu (after font)\n", p - s);

    // XBIN overrules SAUCE
    if (record == NULL) {
        record = piece_allocate(sizeof(sauce));
        record->flags.flag_ls = SAUCE_LS_8PIXEL;
        fsize = st.st_size;
        dprintf("fsize: %lu (based on file size)\n", fsize);
    } else {
        fseek(fd, 0, SEEK_END);
        fsize = st.st_size - sauce_size(record);
        dprintf("fsize: %lu (based on SAUCE)\n", fsize);
    }
    record->flags.flag_b = header->flags.flag_non_blink;

    display = piece_screen_new(header->width, header->height, record, NULL);
    if (display == NULL) {
        fprintf(stderr, "%s: could not piece_allocate %dx%d screen buffer\n",
                        filename, header->width, header->height);
        goto return_free;
    }
    if (xbin_palette != NULL) {
        display->palette = xbin_palette;
    } else {
        display->palette = piece_palette_by_name("ega");
    }
    if (xbin_font != NULL) {
        display->font = xbin_font;
    }

    long gsize = header->width * header->height * 2;
    if (header->flags.flag_compress) {
        xbin_data = tmp = piece_allocate(gsize);
        dprintf("%s: parsing %lu bytes of compressed binary data\n", filename,
                                                                     fsize);
        while (i < gsize) {
            repeat = *p++;
            method = (repeat >> 6);
            repeat = (repeat & 0x3f) + 1;

            switch (method) {
                case XBIN_COMP_NONE:
                    for (r = 0; r < repeat; r++) {
                        xbin_data[i++] = *p++;
                        xbin_data[i++] = *p++;
                    }
                    break;

                case XBIN_COMP_CHAR:
                    ch = *p++;
                    for (r = 0; r < repeat; r++) {
                        xbin_data[i++] = ch;
                        xbin_data[i++] = *p++;
                    }
                    break;

                case XBIN_COMP_ATTR:
                    attribute = *p++;
                    for (r = 0; r < repeat; r++) {
                        xbin_data[i++] = *p++;
                        xbin_data[i++] = attribute;
                    }
                    break;

                case XBIN_COMP_BOTH:
                    ch = *p++;
                    attribute = *p++;
                    for (r = 0; r < repeat; r++) {
                        xbin_data[i++] = ch;
                        xbin_data[i++] = attribute;
                    }
                    break;

                default:
#ifdef DEBUG
                    assert(0);
#endif
                    break;
            }
        }
        dprintf("%s: got %ld bytes in XBIN data buffer\n", filename,
                                                           (xbin_data - tmp));
        xbin_data = tmp;    /* Rewind pointer */
    } else {
        dprintf("%s: parsing uncompressed binary data\n", filename);
        xbin_data = p;      /* Read from mmap()ed area */
    }

    fsize = header->width * header->height * 2;
    while (fsize >= 2) {
        ch = *xbin_data++;
        attribute = *xbin_data++;
        piece_screen_putchar_xbin(display, ch, attribute, &x, &y);
        fsize -= 2;
    }
    dprintf(".. at %lu of %lu (after graphics)\n", p - s, st.st_size);

return_free:
    if (tmp != NULL) {
        free(tmp);
    }
    munmap(p, st.st_size);

return_bail:

    return display;
}

static char *xbin_extensions[] = {
    "xb",
    "xbin",
    NULL
};

static piece_parser_sauce xbin_sauce[] = {
    {SAUCE_DATA_TYPE_XBIN, SAUCE_FILE_TYPE_XBIN},
    {0, 0}
};

static piece_parser xbin_parser = {
    "xbin",
    "eXtended Binary",
    xbin_parser_probe,
    xbin_parser_read,
    xbin_extensions,
    xbin_sauce,
    "cp437_8x16"
};

void piece_xbin_parser_init(void) {
    piece_parser_register(&xbin_parser);
}
