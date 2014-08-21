#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "font.h"
#include "list.h"
#include "parser.h"
#include "parser/tundradraw.h"
#include "screen.h"
#include "palette.h"
#include "util.h"

static uint16_t fgetrgb(palette *palette, FILE *fd)
{
    int index = fget32(fd);
    rgb_color rgb = {
        (index >> 16) & 0xff,
        (index >> 8) & 0xff,
        index & 0xff
    };
    return palette_add_color(palette, &rgb);
}

bool tundradraw_parser_probe(FILE *fd, const char *UNUSED(filename))
{
    tundradraw_header *header;
    bool score = false;

    header = allocate(sizeof(tundradraw_header));
    fread(&header->version, 1, 1, fd);
    fread(&header->header, 8, 1, fd);

    score = (header->version == 24 &&
             !strncmp(header->header, TUNDRADRAW_HEADER, 8));
    free(header);
    return score;
}

screen *tundradraw_parser_read(FILE *fd, const char *filename)
{
    screen *display = NULL;
    sauce *record = NULL;
    tundradraw_header *header;
    int x = 0;
    int y = 0;
    unsigned char ch;
    int32_t fsize = 0;

    record = sauce_read(fd);
    if (record != NULL) {
        fprintf(stderr, "%s: found SAUCE record\n", filename);
        if (record->data_type != SAUCE_DATA_TYPE_CHARACTER ||
            record->file_type != SAUCE_FILE_TYPE_TUNDRADRAW) {
            fprintf(stderr, "%s: not a ThundaDraw file (according to SAUCE)\n",
                            filename);
            fclose(fd);
            free(record);
            return NULL;
        }
        fsize = record->file_size;
        rewind(fd);
    } else {
        fprintf(stderr, "%s: no SAUCE record\n", filename);
        record = allocate(sizeof(sauce));
        record->flags.flag_ls = SAUCE_LS_8PIXEL;
        record->flags.flag_b = 0;
        fseek(fd, 0, SEEK_END);
        fsize = ftell(fd);
        rewind(fd);
    }

    header = allocate(sizeof(tundradraw_header));
    fread(&header->version, 1, 1, fd);
    fread(&header->header, 8, 1, fd);
    if (ferror(fd)) {
        fprintf(stderr, "%s: read error %d\n", filename, ferror(fd));
        fclose(fd);
        free(record);
        free(header);
        return NULL;
    }
    if (header->version != 24 ||
        strncmp(header->header, TUNDRADRAW_HEADER, 8)) {
        fprintf(stderr, "%s: not a TundraDraw file (header mismatch)\n",
                        filename);
        fclose(fd);
        free(record);
        free(header);
        return NULL;
    }

    display = screen_create(TUNDRADRAW_COLS, 1, record);
    if (display == NULL) {
        fprintf(stderr, "%s: could not allocate 80 character buffer\n",
                        filename);
        fclose(fd);
        free(record);
        free(header);
        return NULL;
    }

    display->font = NULL;
    if (header->version == 24) {
        display->palette = palette_new("from file", 0);
    } else {
        display->palette = palette_by_name("tnd");
    }

    rgb_color rgb = {0, 0, 0};
    palette_add_color(display->palette, &rgb);

    while (!feof(fd) && ftell(fd) < fsize) {
        if (x == TUNDRADRAW_COLS) {
            x = 0;
            y++;
        }

        ch = fgetc(fd);
        if (record != NULL && ch == 0x1a) {
            break;
        }
        if (ch == 0x01) {       // Cursor position
            y = fget32(fd);
            x = fget32(fd);
            if (y > TUNDRADRAW_ROWS || x > TUNDRADRAW_COLS) {
                fprintf(stderr, "%s: jump to out of bounds location %dx%d\n",
                                filename, x, y);
                free(record);
                free(header);
                screen_free(display);
                return NULL;
            }
        }
        if (ch == 0x02) {   // Forgeground color
            ch = fgetc(fd);
            display->current->fg = fgetrgb(display->palette, fd);
        }
        if (ch == 0x04) {   // Background color
            ch = fgetc(fd);
            display->current->bg = fgetrgb(display->palette, fd);
        }
        if (ch == 0x06) {   // Both colors
            ch = fgetc(fd);
            display->current->fg = fgetrgb(display->palette, fd);
            display->current->bg = fgetrgb(display->palette, fd);
        }
        if (!(ch == 0x01 || ch == 0x02 || ch == 0x04 || ch == 0x06)) {
            screen_putchar(display, ch, &x, &y, false);
            x++;
        }
    }

    rewind(fd);
    fclose(fd);
    free(header);

    return display;
}

static char *tundradraw_extensions[] = {
    "tnd",
    NULL
};
static parser tundradraw_parser = {
    "tundradraw",
    "Thundra Draw",
    tundradraw_parser_probe,
    tundradraw_parser_read,
    tundradraw_extensions,
    "cp437_8x16"
};

void tundradraw_parser_init(void) {
    parser_register(&tundradraw_parser);
}
