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
                          uint32_t bits, unsigned char ch, int fg)
{
    int32_t offset = ch * font->h,
            offset_x = ch * bits,
            offset_y = 0;

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

#ifdef GD_BMP
    if (!strcmp(extension, "bmp")) {
        free(extension);
        fd = fopen(filename, "wb");
        if (ferror(fd)) {
            goto return_error;
        }
        gdImageBmp(image, fd, 0);
        goto return_done;
        return;
    }
#endif
#ifdef GD_JPEG
    if (!strcmp(extension, "jpg") ||
        !strcmp(extension, "jpeg")) {
        free(extension);
        fd = fopen(filename, "wb");
        if (ferror(fd)) {
            goto return_error;
        }
        gdImageJpeg(image, fd, 100);
        goto return_done;
        return;
    }
#endif
#ifdef GD_GIF
    if (!strcmp(extension, "gif")) {
        free(extension);
        if (piece_options->target->image->animate) {
            fd = piece_options->target->fd;
        } else {
            fd = fopen(filename, "wb");
            if (ferror(fd)) {
                goto return_error;
            }
            gdImageGif(image, fd);
        }
        goto return_done;
        return;
    }
#endif
#ifdef GD_PNG
    do {
        if (strcmp(extension, "png")) {
            fprintf(stderr, "warning: defaulting to PNG writer\n");
        }
        free(extension);
        fd = fopen(filename, "wb");
        if (ferror(fd)) {
            goto return_error;
        }
        gdImagePng(image, fd);
        goto return_done;
    } while(0);
#endif

return_error:
    fprintf(stderr, "%s: error writing to file\n", filename);
return_done:
    fclose(fd);
}

gdImagePtr piece_image_writer_parse(piece_screen *display, const char *filename)
{
    int32_t colors[256] __attribute__((unused));
    int32_t i, canvas_back;
    uint16_t bits = sauce_flag_letter_spacing(display->record), s, t;
    uint8_t frames = 1;
    gdImagePtr result, prev, first = 0;
    piece_image_writer_buffers *image = piece_allocate(sizeof(piece_image_writer_buffers));
    piece_palette *palette = display->palette;
    piece_font *font = display->font;
    int font_fg, font_bg;
    struct timeval start, now;
    gettimeofday(&start, NULL);

    if (piece_options->target->image->animate) {
        frames++;
    }

    /* Command line specified palette takes presedence */
    if (piece_options->target->image->palette != NULL) {
        palette = piece_options->target->image->palette;
    }
    else if (palette == NULL) {
        if (display->palette_name == NULL) {
            fprintf(stderr, "%s: empty palette selected\n", filename);
            exit(1);
        } else {
            palette = piece_palette_by_name(display->palette_name);
        }
    }

    if (palette == NULL) {
        fprintf(stderr, "%s: could not find suitable palette\n", filename);
        exit(1);
    }

    /* Command line specified font takes presedence */
    if (piece_options->target->font) {
        font = piece_options->target->font;
    }
    else if (font == NULL) {
        if (display->font_name == NULL) {
            dprintf("%s: using default font cp437_8x16\n", filename);
            font = piece_font_by_name("cp437_8x16");
        } else {
            font = piece_font_by_name(display->font_name);
        }
    }

    dprintf("%s: using %d bit glyphs from font %s\n", filename, bits, font->name);

    image->font = gdImageCreate(
        font->w * 256,
        font->h
    );
    image->ansi = gdImageCreate(
        bits * display->size.width,
        font->h * display->size.height
    );

    if (image->font == NULL ||
        image->ansi == NULL) {
        fprintf(stderr, "%s: out of memory trying to create %dx%d image\n",
                        filename,
                        bits * display->size.width,
                        font->h * display->size.height);
        exit(1);
    }

    // Colors for font (and back)
    dprintf("%s: setting up %d color palette %s\n", filename,
                                                    palette->colors,
                                                    palette->name);
    font_bg = gdImageColorAllocate(image->font, 0x00, 0x00, 0x00);
    font_fg = gdImageColorAllocate(image->font, 0xff, 0xff, 0xff);
    canvas_back = gdImageColorAllocate(image->ansi, 0, 0, 0);
    dprintf("%s: .. it took %.03fs\n", filename, piece_seconds(start));

    // Font background is transparent
    gdImageFilledRectangle(
        image->font,
        0, 0,
        font->w * 256, font->h,
        font_bg
    );

    // Render font bitmaps
    dprintf("%s: setting up %d character %dx%d font\n", filename,
                                                        font->l / font->h,
                                                        font->w,
                                                        font->h);
    gettimeofday(&now, NULL);
    for (uint32_t ch = 0; ch < 256; ++ch) {
        image_fontcpy(image->font, font, bits, ch, font_fg);
    }

    dprintf("%s: .. it took %.03fs\n", filename, piece_seconds(now));

    // Colors for indexed colors
    for (i = 0; i < palette->colors; ++i) {
        colors[i] = gdImageColorAllocate(
            image->ansi,
            PIECE_RGBA_R(palette->color[i]),
            PIECE_RGBA_G(palette->color[i]),
            PIECE_RGBA_B(palette->color[i])
        );
    }

    if (piece_options->target->image->animate &&
        piece_options->target->fd != NULL) {
        first = image->ansi;
        prev = 0;
        gdImageGifAnimBegin(image->ansi, piece_options->target->fd, 1, 0);
    }

    // Print piece onto canvas
    dprintf("%s: rendering %d tiles on canvas\n", filename, display->tiles * frames);
    gettimeofday(&now, NULL);
    i = 0;
    s = 0;
    t = 0;
    for (uint8_t frame = 0; frame < frames; frame++) {
        if (piece_options->target->image->animate) {
            image->ansi = gdImageCreate(
                bits * display->size.width,
                font->h * display->size.height
            );
            // Colors for indexed colors
            gdImagePaletteCopy(image->ansi, first);
        }

        for (i = 0; i < display->tiles; ++i) {
            int32_t dst_x = (i % display->size.width) * bits,
                    dst_y = (i / display->size.width) * font->h;

            piece_screen_tile *current = &display->tile[i];

            int16_t src_x = current->ch * bits,
                    src_y = 0;

            // Fill background color
            gdImageFilledRectangle(
                image->ansi,
                dst_x,
                dst_y,
                dst_x + bits,
                dst_y + font->h,
                colors[current->bg]
            );

            // If we're in animation mode and iCE colors are disabled, skip
            // drawing the glyph if the glyph is blinking.
            if (piece_options->target->image->animate &&
                frame > 0 &&
                current->attrib & PIECE_ATTRIB_BLINK) {
                continue;
            }

            for (int fy = 0; fy < (int)font->h; fy++) {
                for (int fx = 0; fx < (int)bits; fx++) {
                    int pixel = gdImageGetPixel(image->font, fx + src_x, fy + src_y);
                    if (gdImageRed(image->font, pixel) > 0x80) {
                        gdImageSetPixel(
                            image->ansi,
                            dst_x + fx,
                            dst_y + fy,
                            colors[current->fg]
                        );
                    }
                }
            }

            if (current->attrib & PIECE_ATTRIB_UNDERLINE) {
                
            }

            if (piece_options->verbose && !(i % 80)) {
                t = (int) (piece_seconds(now) * 4);
                if (t > s) {
                    double perc = (i / (double) display->tiles) * 100.0;
                    printf("\r%s: .. still busy, at %.02f%% %s pass %d/%d",
                        filename,
                        perc,
                        spinner[t % spinners],
                        frame + 1,
                        frames
                    );
                    fflush(stdout);
                    s = t;
                }
            }
        }
        
        if (piece_options->target->image->animate) {
            gdImageGifAnimAdd(
                image->ansi,
                piece_options->target->fd,
                0,
                0,
                0,
                40,
                gdDisposalNone,
                prev
            );
            if (prev) {
                gdImageDestroy(prev);
            }
            prev = image->ansi;
        }
        if (t != 0) {
            printf("\r\n");
        }
    }
    dprintf("%s: .. it took %.03fs           \n", filename, piece_seconds(now));

    if (piece_options->target->image->animate) {
        gdImageDestroy(prev);
        gdImageGifAnimEnd(piece_options->target->fd);
        fclose(piece_options->target->fd);
        gdImageDestroy(first);
        gdImageDestroy(image->font);
        free(image);
        return NULL;
    } else if (piece_options->target->image->transparent) {
        gdImageColorTransparent(image->ansi, canvas_back);
    }

    gdImageDestroy(image->font);
    result = image->ansi;
    free(image);

    return result;
}

void image_writer_write(piece_screen *display, const char *filename)
{
    char *extension = piece_get_extension(filename);

    for (int i = strlen(extension) - 1; i > -1; i--) {
        extension[i] = tolower(extension[i]);
    }

    if (piece_options->target->image->animate && strcmp(extension, "gif")) {
        piece_options->target->image->animate = false;
        fprintf(stderr, "notice: animation not supported by output file type\n");
        fprintf(stderr, "notice: animation disabled\n");
    }

    free(extension);

    if (piece_options->target->image->animate && piece_options->target->image->ice_colors) {
        piece_options->target->image->animate = false;
        fprintf(stderr, "notice: animation disabled, iCE colors requested\n");
    }

    if (piece_options->target->image->animate) {
        piece_options->target->fd = fopen(filename, "wb");
        if (ferror(piece_options->target->fd)) {
            fprintf(stderr, "%s: error opening for writing\n", filename);
            return;
        }
    }

    gdImagePtr image = piece_image_writer_parse(display, filename);
    if (image != NULL) {
        image_save(image, filename);
        if (image != NULL) {
            gdImageDestroy(image);
        }
    }
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
