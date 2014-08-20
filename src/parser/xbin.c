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

screen *xbin_parser_read(const char *filename)
{
    FILE *fd = NULL;
    screen *display = NULL;
    sauce *record = NULL;
    xbin_header *header;
    palette *xbin_palette;
    font *xbin_font;
    unsigned char *xbin_font_glyphs;
    int16_t xbin_font_chars = 256;
    int64_t i, xbin_font_total;
    unsigned char ch, attribute;
    int16_t repeat, method;
    int x = 0;
    int y = 0;

    fd = fopen(filename, "rb");
    if (fd == NULL) {
        fprintf(stderr, "%s: error opening\n", filename);
        return NULL;
    }

    record = sauce_read(fd);
    if (record != NULL) {
        if (record->data_type != SAUCE_DATA_TYPE_XBIN) {
            fprintf(stderr, "%s: not an XBIN (according to SAUCE)\n", filename);
            fclose(fd);
            free(record);
            return NULL;
        }
    } else {
        record = allocate(sizeof(sauce));
        record->flags.flag_ls = SAUCE_LS_8PIXEL;
    }

    rewind(fd);
    header = allocate(sizeof(xbin_header));
    fread(&header->id, 4, 1, fd);
    fread(&header->eof_char, 1, 1, fd);
    fread(&header->width, 2, 1, fd);
    fread(&header->height, 2, 1, fd);
    fread(&header->font_size, 1, 1, fd);
    fread(&header->flags, 1, 1, fd);
    if (ferror(fd) || strncmp((const char *) header->id, "XBIN", 4)) {
        fprintf(stderr, "%s: not an XBIN (got %s)\n", filename, header->id);
        fclose(fd);
        free(record);
        free(header);
        return NULL;
    }

    // XBIN overrules SAUCE
    record->flags.flag_b = header->flags.flag_non_blink;

    printf("hi mom\n");
    printf("width: %d\nheight: %d\n", header->width, header->height);
    printf("font_size: %d\n", header->font_size);
    printf("flag palette: %d\n", header->flags.flag_palette);
    printf("flag font: %d\n", header->flags.flag_font);
    printf("flag compress: %d\n", header->flags.flag_compress);
    printf("flag non_blink: %d\n", header->flags.flag_non_blink);
    printf("flag 512_chars: %d\n", header->flags.flag_512_chars);
    printf(".. at %lu\n", ftell(fd));

    if (header->flags.flag_palette) {
        printf("%s: parsing palette\n", filename);
        xbin_palette = allocate(sizeof(palette));
        xbin_palette->colors = 16;
        for (i = 0; i < xbin_palette->colors; ++i) {
            rgb_color c;
            fread(&c, 3, 1, fd);
            printf("color %02x %02x %02x\n", c.r << 2, c.g << 2, c.b << 2);
            xbin_palette->color[i] = (rgb_color) {
                c.r << 2,
                c.g << 2,
                c.b << 2
            };
        }
    } else {
        printf("%s: using ega palette\n", filename);
        xbin_palette = palette_by_name("ega");
    }
    printf(".. at %lu\n", ftell(fd));

    if (header->flags.flag_font) {
        xbin_font_chars = header->flags.flag_512_chars ? 512 : 256;
        xbin_font_total = xbin_font_chars * header->font_size;
        printf("%s: parsing %d pixel font", filename, header->font_size);
        printf(" with %d glyphs\n", xbin_font_chars);
        printf("%s: allocating %d bytes\n", filename, xbin_font_total);
        xbin_font_glyphs = allocate(xbin_font_total);
        for (i = 0; i < xbin_font_total; ++i) {
            fread(&xbin_font_glyphs[i], 1, 1, fd);
        }
        xbin_font = allocate(sizeof(font));
        xbin_font->name = "xbin";
        xbin_font->w = 9;
        xbin_font->h = header->font_size;
        xbin_font->l = xbin_font_chars;
        xbin_font->glyphs = xbin_font_glyphs;
    } else {
        xbin_font = NULL;
    }
    printf(".. at %lu\n", ftell(fd));

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

    if (header->flags.flag_compress) {
        while (!feof(fd)) {
            ch = fgetc(fd);
            method = ch & 0xc0;
            repeat = ch & 0x3f + 1;
            printf("repeat %d blocks compression %d [0x%02x]\n", repeat, method, ch);
            printf("method == 0xc0, %d\n", (method == 0xc0));

            bool read = false;
            while (repeat--) {
                if (method == 0x00) {       // No compression
                    ch = fgetc(fd);
                    attribute = fgetc(fd);
                    printf("none %02x %02x\n", ch, attribute);
                }
                else if (method == 0x40) {  // Character compression
                    if (!read) {
                        ch = fgetc(fd);
                        read = true;
                    }
                    attribute = fgetc(fd);
                    printf("char %02x %02x\n", ch, attribute);
                }
                else if (method == 0x80) {  // Attribute compression
                    if (!read) {
                        attribute = fgetc(fd);
                        read = true;
                    }
                    ch = fgetc(fd);
                    printf("attr %02x %02x\n", ch, attribute);
                }
                else {                      // Character and attribute compression
                    if (!read) {
                        ch = fgetc(fd);
                        attribute = fgetc(fd);
                        read = true;
                    }
                    printf("both %02x %02x\n", ch, attribute);
                }

                //printf("%d x %d [%d]\n", x, y, ftell(fd));
                display->current->bg = (attribute & 0xf0) >> 4;
                display->current->fg = (attribute & 0x0f);
                screen_putchar(display, ch, &x, &y);
            }
        }
    } else {
        while (!feof(fd)) {
            ch = fgetc(fd);
            if (ch == header->eof_char)
                break;
            attribute = fgetc(fd);
            display->current->bg = (attribute & 0xf0) >> 4;
            display->current->fg = (attribute & 0x0f);
            screen_putchar(display, ch, &x, &y);
        }
    }
    printf(".. at %lu\n", ftell(fd));

    rewind(fd);
    fclose(fd);

    return display;
}

void xbin_parser_render(screen *display, unsigned int output_type)
{
    
}

static char *xbin_extensions[] = {
    "xb",
    "xbin",
    NULL
};
static parser xbin_parser = {
    "xbin",
    "eXtended Binary",
    xbin_parser_read,
    xbin_parser_render,
    xbin_extensions
};

void xbin_parser_init(void) {
    parser_register(&xbin_parser);
}
