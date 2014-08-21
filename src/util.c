#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

void *allocate(size_t size)
{
    void *allocated = malloc(size);
    if (allocated == NULL) {
        fprintf(stderr, "out of memory trying to allocate %lu bytes\n", size);
        exit(1);
    }
    memset(allocated, 0, size);
    return allocated;
}

int16_t fget16(FILE *fd)
{
    return (fgetc(fd) << 8 | fgetc(fd));
}

int32_t fget32(FILE *fd)
{
    return (fgetc(fd) << 24 | fgetc(fd) << 16 | fgetc(fd) << 8 | fgetc(fd));
}

char *get_extension(const char *filename)
{
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename) return "";
    return strdup(dot + 1);
}

int32_t max32(int32_t a, int32_t b)
{
    return (a > b) ? a : b;
}

int32_t min32(int32_t a, int32_t b)
{
    return (a < b) ? a : b;
}

int32_t split(char ***array, char delimiter, char *s)
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

char *substr(char *s, size_t start, size_t length)
{
    if (s == 0 || strlen(s) == 0)
        return 0;

    return strndup(s + start, length);
}
