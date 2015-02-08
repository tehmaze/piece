#ifndef __PIECE_WRITER_IMAGE_H__
#define __PIECE_WRITER_IMAGE_H__

#include <gd.h>

typedef struct {
    gdImagePtr ansi;
    gdImagePtr font;
} piece_image_writer_buffers;

void piece_image_writer_init(void);
gdImagePtr piece_image_writer_parse(piece_screen *display,
                                    const char *filename);

#endif // __PIECE_WRITER_IMAGE_H__
