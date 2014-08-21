#include <stdlib.h>
#include <stdio.h>
#include "font.h"
#include "list.h"
#include "screen.h"
#include "util.h"
#include "writer.h"
#include "writer/text.h"

void text_writer_write(screen *display, const char *filename, font *UNUSED(font))
{
    FILE *fd = fopen(filename, "wb");
    if (fd == NULL) {
        fprintf(stderr, "%s: open fail\n", filename);
        exit(1);
    }

    printf("%s: writing %d tiles\n", filename, display->tiles);
    printf("%s: %dx%d\n", filename, display->width, display->height);

    for (int32_t i = 0; i < display->tiles; ++i) {
        if (i > 0 && i % display->width == 0) {
            fputs("\r\n", fd);
        }
        screen_tile *current = &display->tile[i];
        fputc((char) current->ch, fd);
        if (ferror(fd)) {
            fprintf(stderr, "%s: write error %d\n", filename, ferror(fd));
            exit(1);
        }
        fflush(fd);
    }

    fclose(fd);
}

static writer text_writer = {
    "text",
    "txt",
    "Plain text, without ANSi formatting",
    text_writer_write
};

void text_writer_init(void) {
    writer_register(&text_writer);
}
