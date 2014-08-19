#ifndef __LIST_H__
#define __LIST_H__

#include <stdbool.h>

typedef void (*list_free_function)(void *);
typedef bool (*list_iterator)(void *);

typedef struct list_node_s {
    void               *data;
    struct list_node_s *next;
} list_node;

typedef struct {
    int                length;
    int                item_size;
    list_node          *head;
    list_node          *tail;
    list_free_function free_fn;
} list;

void    list_new(list *, int, list_free_function);
void    list_free(list *);
void    list_prepend(list *, void *);
void    list_append(list *, void *);
void    list_foreach(list *, list_iterator);
int     list_size(list *);

#endif // __LIST_H
