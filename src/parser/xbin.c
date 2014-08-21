#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "font.h"
#include "list.h"
#include "parser.h"
#include "parser/xbin.h"
#include "screen.h"
#include "palette.h"
#include "util.h"

static void screen_putchar_xbin(screen *display, unsigned char ch,
                                unsigned char attribute, int *x, int *y)
{
    printf("\r%d x %d     ", (*x), (*y));
    display->current->bg = (attribute & 0xf0) >> 4;
    display->current->fg = (attribute & 0x0f);
    screen_putchar(display, ch, x, y, true);
}

bool xbin_parser_probe(FILE *fd, const char *UNUSED(filename))
{
    xbin_header *header;
    bool score = false;

    header = allocate(sizeof(xbin_header));
    fread(&header->id, 4, 1, fd);
    fread(&header->eof_char, 1, 1, fd);
    fread(&header->width, 2, 1, fd);
    fread(&header->height, 2, 1, fd);
    fread(&header->font_size, 1, 1, fd);
    fread(&header->flags, 1, 1, fd);

    score = (
        !strncmp(header->id, XBIN_ID, XBIN_ID_LEN) &&
        header->width > 0 &&
        header->width < 161 &&
        header->font_size > 0 &&
        header->font_size < 33
    );
    free(header);
    return score;
}

screen *xbin_parser_read(FILE *fd, const char *filename)
{
    struct stat st;
    unsigned char *p, *s;
    screen *display = NULL;
    sauce *record = NULL;
    xbin_header *header;
    palette *xbin_palette;
    font *xbin_font;
    int16_t xbin_font_chars = 256;
    int64_t i, xbin_font_total, fsize, fpos = 0;
    unsigned char ch, attribute, counts;
    unsigned char *xbin_data, *tmp;
    int16_t repeat, method;
    int x = 0;
    int y = 0;

    fseek(fd, 0, SEEK_SET);
    if (fstat(fileno(fd), &st) < 0) {
        fprintf(stderr, "%s: stat() failed\n", filename);
        return NULL;
    }

    p = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fileno(fd), 0);
    if (p == MAP_FAILED) {
        fprintf(stderr, "%s: mmap() failed\n", filename);
        return NULL;
    }
    s = p;

    record = sauce_read(fd);
    if (record != NULL) {
        if (record->data_type != SAUCE_DATA_TYPE_XBIN) {
            fprintf(stderr, "%s: not an XBIN (according to SAUCE)\n", filename);
            fclose(fd);
            free(record);
            munmap(p, st.st_size);
            return NULL;
        }
        fsize = record->file_size;
    } else {
        record = allocate(sizeof(sauce));
        record->flags.flag_ls = SAUCE_LS_8PIXEL;
        fseek(fd, 0, SEEK_END);
        fsize = ftell(fd);
    }

    rewind(fd);
    header = (xbin_header *) p;
    p += sizeof(xbin_header);
    if (ferror(fd) || memcmp(&header->id, XBIN_ID, sizeof(header->id))) {
        fprintf(stderr, "%s: not an XBIN (got %s)\n", filename, header->id);
        fclose(fd);
        free(record);
        free(header);
        munmap(p, st.st_size);
        return NULL;
    }

    // XBIN overrules SAUCE
    record->flags.flag_b = header->flags.flag_non_blink;

    printf("width: %d\nheight: %d\n", header->width, header->height);
    printf("font_size: %d\n", header->font_size);
    printf("flag palette: %d\n", header->flags.flag_palette);
    printf("flag font: %d\n", header->flags.flag_font);
    printf("flag compress: %d\n", header->flags.flag_compress);
    printf("flag non_blink: %d\n", header->flags.flag_non_blink);
    printf("flag 512_chars: %d\n", header->flags.flag_512_chars);
    printf(".. at %lu\n", p - s);

    if (header->flags.flag_palette) {
        printf("%s: parsing palette\n", filename);
        xbin_palette = allocate(sizeof(palette));
        xbin_palette->name = "from file";
        xbin_palette->colors = 16;
        i = sizeof(rgb_color) * 16;
        xbin_palette->color = allocate(i);
        memcpy(xbin_palette->color, p, i);
        p += i;
        for (i = 0; i < 16; ++i) {
            xbin_palette->color[i].r <<= 4;
            xbin_palette->color[i].g <<= 4;
            xbin_palette->color[i].b <<= 4;
        }
    } else {
        printf("%s: using ega palette\n", filename);
        xbin_palette = palette_by_name("ega");
    }
    printf(".. at %lu (after palette)\n", p - s);

    if (header->flags.flag_font) {
        xbin_font_chars = header->flags.flag_512_chars ? 512 : 256;
        xbin_font_total = xbin_font_chars * 16;
        xbin_font = allocate(sizeof(font));
        xbin_font->name = "xbin";
        xbin_font->w = 9;
        xbin_font->h = header->font_size;
        xbin_font->l = xbin_font_chars;
        xbin_font->glyphs = allocate(xbin_font_total);
        memcpy((unsigned char *) xbin_font->glyphs, p, xbin_font_total);
        p += xbin_font_total;
    } else {
        xbin_font = NULL;
    }
    printf(".. at %lu (after font)\n", p - s);

    display = screen_create(header->width, header->height, record);
    if (display == NULL) {
        fprintf(stderr, "%s: could not allocate %dx%d screen buffer\n",
                        filename, header->width, header->height);
        fclose(fd);
        free(record);
        free(header);
        return NULL;
    }
    display->palette = xbin_palette;
    display->font = xbin_font;

    fsize = st.st_size - (p - s);
    printf(".. remains %lu of %lu\n", fsize, st.st_size);
    unsigned long gsize = header->width * header->height * 2;
    printf(".. remains %lu of %lu\n", gsize, st.st_size);
    xbin_data = tmp = allocate(fsize);
    if (header->flags.flag_compress) {
        while (fsize && (p - s) < st.st_size) {
            repeat = *p++;
            method = (repeat & 0xc0);
            repeat = (repeat & 0x3f);

            switch (method) {
                case XBIN_COMP_NONE:
                    for (i = 0; i < repeat + 1; i++) {
                        *xbin_data++ = *p++;
                        *xbin_data++ = *p++;
                        assert(fsize >= 2);
                        fsize -= 2;
                    }
                    break;

                case XBIN_COMP_CHAR:
                    ch = *p++;
                    for (i = 0; i < repeat + 1; i++) {
                        *xbin_data++ = ch;
                        *xbin_data++ = *p++;
                        assert(fsize >= 2);
                        fsize -= 2;
                    }
                    break;

                case XBIN_COMP_ATTR:
                    attribute = *p++;
                    for (i = 0; i < repeat; ++i) {
                        *xbin_data++ = *p++;
                        *xbin_data++ = attribute;
                        assert(fsize >= 2);
                        fsize -= 2;
                    }
                    break;

                case XBIN_COMP_BOTH:
                    ch = *p++;
                    attribute = *p++;
                    for (i = 0; i < repeat; ++i) {
                        *xbin_data++ = ch;
                        *xbin_data++ = attribute;
                        assert(fsize >= 2);
                        fsize -= 2;
                    }
                    break;

                default:
                    assert(0);
                    break;
            }
        }
        fsize = header->width * header->height * 2;
        xbin_data = tmp;
    } else {
        xbin_data = p;
    }

    if (fsize % 2) {
        fprintf(stderr, "uneven number of blocks!? [%d/%d]\n", fpos, fsize);
        assert(0);
    }
    while (fsize) {
        ch = *xbin_data++;
        attribute = *xbin_data++;
        screen_putchar_xbin(display, ch, attribute, &x, &y);
        fsize -= 2;
    }
    printf(".. at %lu of %lu (after graphics)\n", p - s, st.st_size);

    munmap(p, st.st_size);

    rewind(fd);
    fclose(fd);

    free(header);

    return display;
}

static char *xbin_extensions[] = {
    "xb",
    "xbin",
    NULL
};

static parser xbin_parser = {
    "xbin",
    "eXtended Binary",
    xbin_parser_probe,
    xbin_parser_read,
    xbin_extensions,
    "cp437_8x16"
};

void xbin_parser_init(void) {
    parser_register(&xbin_parser);
}
