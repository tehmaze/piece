#include <stdlib.h>
#include <stdio.h>

#include "piece/font.h"
#include "piece/list.h"
#include "piece/screen.h"
#include "piece/util.h"
#include "piece/writer.h"
#include "piece/writer/text.h"

void text_writer_write(piece_screen *display, const char *filename)
{
    FILE *fd = fopen(filename, "wb");
    if (fd == NULL) {
        fprintf(stderr, "%s: open fail\n", filename);
        exit(1);
    }

    printf("%s: writing %d tiles\n", filename, display->tiles);
    printf("%s: %dx%d\n", filename, display->size.width, display->size.height);

    for (int32_t i = 0; i < display->tiles; ++i) {
        if (i > 0 && i % display->size.width == 0) {
            fputs("\r\n", fd);
        }
        piece_screen_tile *current = &display->tile[i];
        fputc((char) current->ch, fd);
        if (ferror(fd)) {
            fprintf(stderr, "%s: write error %d\n", filename, ferror(fd));
            exit(1);
        }
        fflush(fd);
    }

    fclose(fd);
}

static piece_writer text_writer = {
    "text",
    "txt",
    "Plain text, without ANSi formatting",
    text_writer_write
};

void piece_text_writer_init(void) {
    piece_writer_register(&text_writer);
}
