#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gd.h>
#include "font.h"
#include "screen.h"
#include "writer.h"
#include "writer/image.h"
#include "palette.h"
#include "util.h"

/* Copy a font glyph to the target image, the resulting image will have all
 * glyphs in all possible palette colors like so:
 *
 *   00|01|02|03|04|.. in color 0
 *   00|01|02|03|04|.. in color 1
 */
static void image_fontcpy(gdImagePtr image, font *font,
                          uint32_t bits,
                          int32_t fg, unsigned char ch)
{
    int32_t offset = ch * font->h,
            offset_x = ch * bits,
            offset_y = fg * font->h;

    for (uint32_t row = 0; row < font->h; ++row) {
        unsigned char glyph = font->glyphs[offset + row];
        //printf("0x%02x ", glyph);
        for (uint32_t col = 0; col < bits; ++col) {
            if (((0x80 >> col) & glyph) != 0) {
                gdImageSetPixel(
                    image,
                    offset_x + col,
                    offset_y + row,
                    fg
                );

                if (bits == 9 && col == 7 && ch > 0xbf && ch < 0xe0) {
                    // Copy the last row
                    gdImageSetPixel(
                        image,
                        offset_x + 8,
                        offset_y + row,
                        fg
                    );
                }
            }
        }
    }
}

static void image_save(gdImagePtr image, const char *filename)
{
    FILE *fd;
    char *extension = get_extension(filename);
    for (int i = strlen(extension) - 1; i > -1; i--) {
        extension[i] = tolower(extension[i]);
    }

    printf("saving %s (type %s)\n", filename, extension);

    if (!strcmp(extension, "bmp")) {
        free(extension);
        fd = fopen(filename, "wb");
        if (!ferror(fd)) {
            gdImageBmp(image, fd, 0);
            fclose(fd);
        }
        return;
    }
    else if (!strcmp(extension, "jpg") ||
        !strcmp(extension, "jpeg")) {
        free(extension);
        fd = fopen(filename, "wb");
        if (!ferror(fd)) {
            gdImageJpeg(image, fd, 100);
            fclose(fd);
        }
        return;
    }
    else if (!strcmp(extension, "gif")) {
        free(extension);
        fd = fopen(filename, "wb");
        if (!ferror(fd)) {
            gdImageGif(image, fd);
            fclose(fd);
        }
        return;
    }
    else {
        free(extension);
        fd = fopen(filename, "wb");
        if (!ferror(fd)) {
            gdImagePng(image, fd);
            fclose(fd);
        }
        return;
    }
}

void image_writer_write(screen *display, const char *filename, font *font)
{
    int32_t colors[21], i, j, canvas_back;
    uint16_t bits = sauce_flag_letter_spacing(display->record);
    gdImagePtr image_ansi, image_font, image_back;

    image_back = gdImageCreate(9 * 16, 16);
    image_font = gdImageCreate(font->w * 256, font->h * 16);
    image_ansi = gdImageCreate(bits * display->width,
                               font->h * display->height);

    if (image_back == NULL ||
        image_font == NULL ||
        image_ansi == NULL) {
        fprintf(stderr, "%s: out of memory trying to create %dx%d image",
                        filename,
                        bits * display->width,
                        font->h * display->height);
        exit(1);
    }

    // Colors for font (and back)
    for (i = 0; i < 16; ++i) {
        colors[i] = gdImageColorAllocate(image_font, EGA[i].r,
                                                     EGA[i].g,
                                                     EGA[i].b);
    }
    colors[20] = gdImageColorAllocate(image_font, 200, 220, 169);
    gdImageColorTransparent(image_font, 20);
    gdImagePaletteCopy(image_back, image_font);
    canvas_back = gdImageColorAllocate(image_ansi, 0, 0, 0);

    // Font background is transparent
    gdImageFilledRectangle(image_font, 0, 0, 256 * font->w, 16 * font->h, 20);

    // Colors for underline
    for (i = 0; i < 16; ++i) {
        int32_t r = gdImageRed(image_back, i),
                g = gdImageGreen(image_back, i),
                b = gdImageBlue(image_back, i);
        colors[i] = gdImageColorAllocate(image_ansi, r, g, b);
    }

    // Render font bitmaps
    for (int32_t fg = 0; fg < 16; ++fg) {
        for (uint32_t ch = 0; ch < 256; ++ch) {
            image_fontcpy(image_font, font, bits, fg, ch);
        }
    }

    // Render back bitmaps
    for (i = 0; i < 16; ++i) {
        gdImageFilledRectangle(image_back, i * 9, 0, i * 9 + 9, 16, i);
    }

    // Print piece onto canvas
    i = 0;
    for (list_node *node = display->tiles->head;
         node->next != NULL;
         node = node->next) {

         int32_t x = (i % display->width) * bits,
                 y = (i / display->width) * font->h;

         tile *current = (tile *) node->data;
         //printf("render %c [%u] at %dx%d\n", current->ch, current->ch, x, y);

         gdImageCopy(
            image_ansi,
            image_back,
            x, y,
            current->bg * 9,
            0,
            bits,
            font->h
         );
         gdImageCopy(
            image_ansi,
            image_font,
            x, y,
            current->ch * bits,
            current->fg * font->h,
            bits,
            font->h
         );

         i++;
    }
    /*
    for (i = 0; i < 256; ++i) {
        uint32_t x = (i % 80) * bits,
                 y = (i / 80) * font->h,
                 b = (i % 8),
                 f = ((i + 4) % 16);

        gdImageCopy(
            image_ansi,
            image_back,
            x, y,
            b * 9,
            0,
            bits,
            font->h
        );
        gdImageCopy(
            image_ansi,
            image_font,
            x, y,
            (unsigned char) i * font->w,
            f * font->h,
            bits,
            font->h
        );
    }
    */

    //gdImagePng(image_ansi, fd);
    //gdImagePng(image_font, fd);
    image_save(image_ansi, filename);

    gdImageDestroy(image_font);
    gdImageDestroy(image_back);
    gdImageDestroy(image_ansi);
}

static writer image_writer = {
    "image",
    "png",
    "Image",
    image_writer_write
};

void image_writer_init(void) {
    writer_register(image_writer);
}
