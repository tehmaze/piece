#ifndef __WRITER_H
#define __WRITER_H

#include "font.h"
#include "screen.h"

typedef void (*writer_write_func)(screen *, const char *, font *);

typedef struct writer_s {
    char                *name;
    char                *extension;
    char                *description;
    writer_write_func   write;
} writer;

void    writer_init(void);
void    writer_iter(list_iterator iterator);
writer *writer_for_type(const char *typename);
void    writer_register(writer);

#endif // __WRITER_H
