#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "piece/util.h"

void *piece_allocate(size_t size)
{
    void *allocated = malloc(size);
    if (allocated == NULL) {
        fprintf(stderr, "out of memory trying to allocate %lu bytes\n", size);
        exit(1);
    }
    memset(allocated, 0, size);
    return allocated;
}

/* Get a 16 bit word, network byte order */
int16_t piece_fget16(FILE *fd)
{
    return (fgetc(fd) << 8 | fgetc(fd));
}

/* Get a 16 bit word, Intel byte order */
int16_t piece_fget16i(FILE *fd)
{
    return (fgetc(fd) | fgetc(fd) << 8);
}

/* Get a 32 bit short, network byte order */
int32_t piece_fget32(FILE *fd)
{
    return (fgetc(fd) << 24 | fgetc(fd) << 16 | fgetc(fd) << 8 | fgetc(fd));
}

/* Get a 32 bit short, Intel byte order */
int32_t piece_fget32i(FILE *fd)
{
    return (fgetc(fd) | fgetc(fd) << 8 | fgetc(fd) << 16 | fgetc(fd) << 24);
}

char *piece_get_extension(const char *filename)
{
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename) return "";
    return strdup(dot + 1);
}

void piece_hexdump(FILE *stream, void *data, off_t offset, size_t size)
{
    uint8_t *src = data;
    off_t i, j;
    bool curr_zeros = false, prev_zeros = false, skip_zeros = false;

    for (i = 0; i < (off_t) size; i += 16) {
        curr_zeros = true;
        for (j = 0; j < 16; j++) {
            if (src[i + j] != 0x00) {
                curr_zeros = false;
            }
        }

        if (!curr_zeros && skip_zeros) {
            skip_zeros = false;
        }

        if (!skip_zeros && !(curr_zeros && !prev_zeros)) {
            fprintf(stream, "0x%08jx", offset + i);
            for (j = 0; j < 16; j++) {
                if (i + j > (off_t) size) break;
                if (j % 8 == 0) {
                    fprintf(stream, " ");
                }
                fprintf(stream, "%02x ", src[i + j]);
            }
            fprintf(stream, " |");
            for (j = 0; j < 16; j++) {
                if (isprint(src[i + j])) {
                    fprintf(stream, "%c", src[i + j]);
                } else {
                    fprintf(stream, ".");
                }
            }
            fprintf(stream, "|\n");
        }

        if (curr_zeros && prev_zeros) {
            if (!skip_zeros) {
                fprintf(stream, "*\n");
                skip_zeros = true;
            }
        }
        prev_zeros = curr_zeros;
    }

    if (size % 16) {
        fprintf(stream, "\n");
    }
}

int32_t piece_max32(int32_t a, int32_t b)
{
    return (a > b) ? a : b;
}

int32_t piece_min32(int32_t a, int32_t b)
{
    return (a < b) ? a : b;
}

double piece_seconds(struct timeval then)
{
    struct timeval curr;
    gettimeofday(&curr, NULL);

    return
        (curr.tv_sec + ((double) curr.tv_usec / 1000000.0)) -
        (then.tv_sec + ((double) then.tv_usec / 1000000.0));
}

int32_t piece_split(char ***array, char delimiter, char *s)
{
    char *src = s, *end, *dst;
    char **buf;
    int32_t size = 1, i;

    while ((end = strchr(src, delimiter)) != NULL) {
        size++;
        src = end + 1;
    }

    buf = malloc(size * sizeof(char *) + (strlen(s) + 1) * sizeof(char));
    if (buf == NULL) {
        return 0;
    }

    src = s;
    dst = (char *) buf + size * sizeof(char *);
    for (i = 0; i < size; ++i) {
        if ((end = strchr(src, delimiter)) == NULL) {
            end = src + strlen(src);
        }
        buf[i] = dst;
        strncpy(dst, src, end - src);
        dst[end - src] = 0x00;
        dst += end - src + 1;
        src = end + 1;
    }

    *array = buf;
    return size;
}

char *piece_substr(char *s, size_t start, size_t length)
{
    if (s == 0 || strlen(s) == 0)
        return 0;

    return strndup(s + start, length);
}
