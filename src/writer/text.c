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
    list_node *node = display->tiles->head;

    printf("%s: writing %d tiles\n", filename, list_size(display->tiles));
    printf("%s: %dx%d\n", filename, display->width, display->height);

    uint64_t i = 0;
    while (node != NULL) {
        tile *current = (tile *) node->data;
        fputc((char) current->ch, fd);
        if (ferror(fd)) {
            fprintf(stderr, "%s: write error %d\n", filename, ferror(fd));
            exit(1);
        }
        fflush(fd);
        if (++i % display->width == 0) {
            fputs("\r\n", fd);
        }

        node = node->next;
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
    writer_register(text_writer);
}
