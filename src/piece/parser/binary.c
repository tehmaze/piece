#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "piece/font.h"
#include "piece/list.h"
#include "piece/options.h"
#include "piece/parser.h"
#include "piece/parser/binary.h"
#include "piece/screen.h"
#include "piece/palette.h"
#include "piece/util.h"

static void piece_screen_putchar_binary(piece_screen *display, unsigned char ch,
                                        unsigned char attribute, int *x, int *y)
{
    display->current->attrib = 0;
    display->current->bg = (attribute & 0xf0) >> 4;
    display->current->fg = (attribute & 0x0f);
    piece_screen_putchar(display, ch, x, y, true);
}

piece_screen *binary_parser_read(FILE *fd, const char *filename)
{
    struct stat st;
    unsigned char *p, *s;
    piece_screen *display = NULL;
    sauce *record = NULL;
    int32_t fsize = 0, width = 160, height = 25;
    unsigned char ch, attribute;
    int x = 0;
    int y = 0;

    fseek(fd, 0, SEEK_SET);
    if (fstat(fileno(fd), &st) < 0) {
        fprintf(stderr, "%s: stat() failed\n", filename);
        goto return_bail;
    }

    p = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fileno(fd), 0);
    if (p == MAP_FAILED) {
        fprintf(stderr, "%s: mmap() failed\n", filename);
        goto return_bail;
    }
    s = p;

    record = sauce_read(fd);
    if (record != NULL) {
        if (record->data_type == SAUCE_DATA_TYPE_BINARYTEXT) {
            fsize = st.st_size - sauce_size(record) - 1;
        }
        if (record->data_type == SAUCE_DATA_TYPE_BINARYTEXT &&
            record->file_type > 1) {
            width = record->file_type * 2;
            height = fsize / (width * 2);
            dprintf("%s: using size %dx%d from SAUCE\n", filename, width, height);
        }
    } else {
        record = piece_allocate(sizeof(sauce));
        record->flags.flag_ls = SAUCE_LS_8PIXEL;
        width = 160;
        height = st.st_size / (width * 2);
        dprintf("%s: using size %dx%d\n", filename, width, height);
    }

    if (fsize == 0) {
        fsize = st.st_size;
    }

    display = piece_screen_new(width, height, record, NULL);
    if (display == NULL) {
        fprintf(stderr, "%s: could not piece_allocate %dx%d screen buffer\n",
                        filename, width, height);
        goto return_free;
    }
    display->palette = piece_palette_by_name("bin");

    if (fsize > st.st_size) {
        fsize = st.st_size;
    }
    while (fsize >= 2) {
        ch = *p++;
        attribute = *p++;
        piece_screen_putchar_binary(display, ch, attribute, &x, &y);
        fsize -= 2;
    }
    dprintf(".. at %lu of %lu (after graphics)\n", p - s, st.st_size);


return_free:
    munmap(p, st.st_size);

return_bail:

    return display;
}

static char *binary_extensions[] = {
    "bin",
    NULL
};

static piece_parser binary_parser = {
    "bin",
    "Binary",
    NULL,
    binary_parser_read,
    binary_extensions,
    NULL,
    "cp437_8x16"
};

void piece_binary_parser_init(void) {
    piece_parser_register(&binary_parser);
}
