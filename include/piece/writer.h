#ifndef __PIECE_WRITER_H__
#define __PIECE_WRITER_H__

#include "piece/screen.h"

typedef void (*piece_writer_write_func)(piece_screen *, const char *);

typedef struct piece_writer_s {
    char                    *name;
    char                    *extension;
    char                    *description;
    piece_writer_write_func write;
} piece_writer;

void            piece_writer_init(void);
void            piece_writer_free(void);
void            piece_writer_iter(piece_list_iterator iterator);
piece_writer    *piece_writer_for_type(const char *);
void            piece_writer_register(piece_writer *);

#endif // __PIECE_WRITER_H__
