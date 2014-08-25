#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <gd.h>

#include "piece/font.h"
#include "piece/options.h"
#include "piece/screen.h"
#include "piece/writer.h"
#include "piece/writer/image.h"
#include "piece/palette.h"
#include "piece/util.h"

static int spinners = 8;
static char *spinner[] = {
    "\xe2\xa3\xbe",
    "\xe2\xa3\xbd",
    "\xe2\xa3\xbb",
    "\xe2\xa2\xbf",
    "\xe2\xa1\xbf",
    "\xe2\xa3\x9f",
    "\xe2\xa3\xaf",
    "\xe2\xa3\xb7"
};

/* Copy a font glyph to the target image, the resulting image will have all
 * glyphs in all possible palette colors like so:
 *
 *   00|01|02|03|04|.. in color 0
 *   00|01|02|03|04|.. in color 1
 */
static void image_fontcpy(gdImagePtr image, piece_font *font,
                          uint32_t bits,
                          int32_t fg, unsigned char ch)
{
    int32_t offset = ch * font->h,
            offset_x = ch * bits,
            offset_y = fg * font->h;

    for (uint32_t row = 0; row < font->h; ++row) {
        unsigned char glyph = font->glyphs[offset + row];
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
    char *extension = piece_get_extension(filename);

    for (int i = strlen(extension) - 1; i > -1; i--) {
        extension[i] = tolower(extension[i]);
    }

    dprintf("%s: writing %s image\n", filename, extension);

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

gdImagePtr piece_image_writer_parse(piece_screen *display, const char *filename,
                                    piece_font *font)
{
    int32_t colors[256], i, canvas_back;
    uint16_t bits = sauce_flag_letter_spacing(display->record), s, t;
    gdImagePtr result;
    piece_image_writer_buffers *image = piece_allocate(sizeof(piece_image_writer_buffers));
    struct timeval start, now;
    gettimeofday(&start, NULL);

    if (display->palette == NULL) {
        fprintf(stderr, "%s: empty palette selected\n", filename);
        exit(1);
    }
    if (font == NULL) {
        font = piece_font_by_name("cp437_8x16");
    }

    dprintf("%s: using %d bit glyphs from font %s\n", filename, bits, font->name);

    image->back = gdImageCreate(
        9 * display->palette->colors,
        16
    );
    image->font = gdImageCreate(
        font->w * 256,
        font->h * display->palette->colors
    );
    image->ansi = gdImageCreate(bits * display->size.width,
                               font->h * display->size.height);

    if (image->back == NULL ||
        image->font == NULL ||
        image->ansi == NULL) {
        fprintf(stderr, "%s: out of memory trying to create %dx%d image\n",
                        filename,
                        bits * display->size.width,
                        font->h * display->size.height);
        exit(1);
    }

    // Colors for font (and back)
    dprintf("%s: setting up %d color palette %s\n", filename,
                                                    display->palette->colors,
                                                    display->palette->name);
    for (i = 0; i < display->palette->colors; ++i) {
        colors[i] = gdImageColorAllocate(
            image->font,
            display->palette->color[i].r,
            display->palette->color[i].g,
            display->palette->color[i].b
        );
    }
    colors[255] = gdImageColorAllocate(image->font, 200, 220, 169);
    gdImageColorTransparent(image->font, 255);
    gdImagePaletteCopy(image->back, image->font);
    canvas_back = gdImageColorAllocate(image->ansi, 0, 0, 0);
    dprintf("%s: .. it took %.03fs\n", filename, piece_seconds(start));

    // Font background is transparent
    gdImageFilledRectangle(
        image->font,
        0, 0,
        font->w * 256, font->h * display->palette->colors,
        255
    );

    // Colors for underline
    for (i = 0; i < display->palette->colors; ++i) {
        int32_t r = gdImageRed(image->back, i),
                g = gdImageGreen(image->back, i),
                b = gdImageBlue(image->back, i);
        colors[i] = gdImageColorAllocate(image->ansi, r, g, b);
    }

    // Render font bitmaps
    dprintf("%s: setting up %d character %dx%d font\n", filename,
                                                        font->l,
                                                        font->w,
                                                        font->h);
    gettimeofday(&now, NULL);
    for (int32_t fg = 0; fg < display->palette->colors; ++fg) {
        for (uint32_t ch = 0; ch < 256; ++ch) {
            image_fontcpy(image->font, font, bits, fg, ch);
        }
    }

    // Render back bitmaps
    for (i = 0; i < display->palette->colors; ++i) {
        gdImageFilledRectangle(
            image->back,
            i * 9, 0,
            i * 9 + 9, 16,
            i
        );
    }
    dprintf("%s: .. it took %.03fs\n", filename, piece_seconds(now));

    // Print piece onto canvas
    dprintf("%s: rendering %d tiles on canvas\n", filename, display->tiles);
    gettimeofday(&now, NULL);
    i = 0;
    s = 0;
    t = 0;
    for (i = 0; i < display->tiles; ++i) {
        int32_t dst_x = (i % display->size.width) * bits,
                dst_y = (i / display->size.width) * font->h;

        piece_screen_tile *current = &display->tile[i];

        int16_t src_x = current->ch * bits,
                src_y = current->fg * font->h;

        if (current->bg != 0) {
            gdImageCopy(
                image->ansi,
                image->back,
                dst_x, dst_y,
                current->bg * 9, 0,
                bits, font->h
            );
        }

        if (current->attrib & PIECE_ATTRIB_ITALICS) {
            gdImageCopy(
                image->ansi,
                image->font,
                dst_x + 3, dst_y,
                src_x, src_y,
                bits, 2
            );
            gdImageCopy(
                image->ansi,
                image->font,
                dst_x + 2, dst_y + 2,
                src_x, src_y + 2,
                bits, 4
            );
            gdImageCopy(
                image->ansi,
                image->font,
                dst_x + 1, dst_y + 6,
                src_x, src_y + 6,
                bits, 4
            );
            gdImageCopy(
                image->ansi,
                image->font,
                dst_x, dst_y + 10,
                src_x, src_y + 10,
                bits, 4
            );
            gdImageCopy(
                image->ansi,
                image->font,
                dst_x - 1, dst_y + 14,
                src_x, src_y + 14,
                bits, 2
            );

        } else {
            gdImageCopy(
                image->ansi,
                image->font,
                dst_x, dst_y,
                src_x, src_y,
                bits, font->h
            );
        }

        if (current->attrib & PIECE_ATTRIB_UNDERLINE) {
            
        }

        if (piece_options->verbose && !(i % 80)) {
            t = (int) (piece_seconds(now) * 4);
            if (t > s) {
                double perc = (i / (double) display->tiles) * 100.0;
                printf("\r%s: .. still busy, at %.02f%% %s", filename,
                                                             perc,
                                                             spinner[t % spinners]);
                fflush(stdout);
                s = t;
            }
        }
    }
    if (t != 0) {
        printf("\r");
    }
    dprintf("%s: .. it took %.03fs           \n", filename, piece_seconds(now));

    gdImageDestroy(image->font);
    gdImageDestroy(image->back);
    result = image->ansi;
    free(image);

    return result;
}

void image_writer_write(piece_screen *display, const char *filename,
                        piece_font *font)
{
    gdImagePtr image = piece_image_writer_parse(display, filename, font);
    if (image != NULL) {
        image_save(image, filename);
    }
    gdImageDestroy(image);
}

static piece_writer image_writer = {
    "image",
    "png",
    "Image",
    image_writer_write
};

void piece_image_writer_init(void) {
    piece_writer_register(&image_writer);
}
