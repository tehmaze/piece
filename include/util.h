#ifndef __UTIL_H
#define __UTIL_H

#ifdef UNUSED
#elif defined(__GNUC__)
# define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#elif defined(__LCLINT__)
# define UNUSED(x) /*@unused@*/ x
#else
# define UNUSED(x) x
#endif

void       *allocate(size_t);
char       *get_extension(const char *filename);
int32_t     max32(int32_t a, int32_t b);
int32_t     min32(int32_t a, int32_t b);
int32_t     split(char ***array, char delimiter, char *s);
char       *substr(char *s, size_t start, size_t length);

#endif // __UTIL_H
