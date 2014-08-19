#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "list.h"
#include "writer.h"

static list *writers;

void writer_init()
{
    writers = (list *) malloc(sizeof(list));
    if (writers == NULL) {
        fprintf(stderr, "out of memory");
        exit(1);
    }
    list_new(writers, sizeof(writer), NULL);
}

void writer_register(writer details)
{
    fprintf(stderr, "writer: registering %s\n", details.name);
    list_append(writers, &details);
}

void writer_iter(list_iterator iterator)
{
    list_foreach(writers, iterator);
}

writer *writer_for_type(const char *typename)
{
    list_node *node = writers->head;
    while (node != NULL) {
        if (!strcmp(((writer *) node->data)->name, typename)) {
            return (writer *) node->data;
        }
        node = node->next;
    }

    return NULL;
}
