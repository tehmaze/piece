#ifndef __PIECE_UTIL_H__
#define __PIECE_UTIL_H__

#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>

#ifdef UNUSED
#elif defined(__GNUC__)
# define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#elif defined(__LCLINT__)
# define UNUSED(x) /*@unused@*/ x
#else
# define UNUSED(x) x
#endif

void       *piece_allocate(size_t);
int16_t     piece_fget16(FILE *fd);
int16_t     piece_fget16i(FILE *fd);
int32_t     piece_fget32(FILE *fd);
int32_t     piece_fget32i(FILE *fd);
char       *piece_get_extension(const char *filename);
void        piece_hexdump(FILE *stream, void *data, off_t offset, size_t size);
int32_t     piece_max32(int32_t a, int32_t b);
int32_t     piece_min32(int32_t a, int32_t b);
double      piece_seconds(struct timeval then);
int32_t     piece_split(char ***array, char delimiter, char *s);
char       *piece_substr(char *s, size_t start, size_t length);

#endif // __PIECE_UTIL_H__
