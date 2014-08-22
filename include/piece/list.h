#ifndef __PIECE_LIST_H
#define __PIECE_LIST_H

#include <stdbool.h>
#include <stdint.h>

typedef void (*piece_list_free_function)(void *);
typedef bool (*piece_list_iterator)(void *);

typedef struct piece_list_node_s {
    void               *data;
    struct piece_list_node_s *next;
} piece_list_node;

typedef struct {
    int                length;
    piece_list_node          *head;
    piece_list_node          *tail;
    piece_list_free_function free_fn;
} piece_list;

void                piece_list_new(piece_list *, piece_list_free_function);
void                piece_list_free(piece_list *);
void /*@null@*/    *piece_list_get(piece_list *, uint32_t);
void                piece_list_prepend(piece_list *, void *);
void                piece_list_append(piece_list *, void *);
void                piece_list_foreach(piece_list *, piece_list_iterator);
int                 piece_list_size(piece_list *);

#endif // __PIECE_LIST_H
