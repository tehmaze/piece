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
#include "options.h"
#include "parser.h"
#include "parser/binary.h"
#include "screen.h"
#include "palette.h"
#include "util.h"

screen *binary_parser_read(FILE *fd, const char *filename)
{
    struct stat st;
    unsigned char *p, *s;
    screen *display = NULL;
    sauce *record = NULL;
    int32_t fsize = 0, width = 160, height = 25;
    unsigned char ch, attribute;
    int x = 0;
    int y = 0;

    fseek(fd, 0, SEEK_SET);
    if (fstat(fileno(fd), &st) < 0) {
        fprintf(stderr, "%s: stat() failed\n", filename);
        goto binary_read_free;
    }

    p = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fileno(fd), 0);
    if (p == MAP_FAILED) {
        fprintf(stderr, "%s: mmap() failed\n", filename);
        goto binary_read_free;
    }
    s = p;

    record = sauce_read(fd);
    if (record != NULL) {
        if (record->data_type != SAUCE_DATA_TYPE_BINARYTEXT) {
            fprintf(stderr, "%s: not binary text (according to SAUCE)\n", filename);
            goto binary_read_munmap;
        }
        fsize = st.st_size - sauce_size(record) - 1;
        if (record->file_type > 1) {
            width = record->file_type * 2;
            height = fsize / (width * 2);
            dprintf("%s: using size %dx%d from SAUCE\n", filename, width, height);
        }
    } else {
        record = allocate(sizeof(sauce));
        record->flags.flag_ls = SAUCE_LS_8PIXEL;
        fsize = st.st_size;
    }

    display = screen_create(width, height, record);
    if (display == NULL) {
        fprintf(stderr, "%s: could not allocate %dx%d screen buffer\n",
                        filename, width, height);
        goto binary_read_free;
    }
    display->palette = palette_by_name("bin");

    if (fsize > st.st_size) {
        fsize = st.st_size;
    }
    while (fsize >= 2) {
        ch = *p++;
        attribute = *p++;
        display->current->bg = (attribute & 0xf0) >> 4;
        display->current->fg = (attribute & 0x0f);
        if (display->current->bg > 8 && !record->flags.flag_b) {
            display->current->bg -= 8;
        }
        screen_putchar(display, ch, &x, &y, true);
        fsize -= 2;
    }
    dprintf(".. at %lu of %lu (after graphics)\n", p - s, st.st_size);

binary_read_munmap:
    munmap(p, st.st_size);

binary_read_free:
    if (display == NULL) {
        free(record);
        record = NULL;
    }

    return display;
}

static char *binary_extensions[] = {
    "bin",
    NULL
};

static parser binary_parser = {
    "bin",
    "Binary",
    NULL,
    binary_parser_read,
    binary_extensions,
    "cp437_8x16"
};

void binary_parser_init(void) {
    parser_register(&binary_parser);
}
