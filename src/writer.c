#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "list.h"
#include "writer.h"
#include "util.h"

static list *writers;

void writer_init(void)
{
    writers = allocate(sizeof(list));
    list_new(writers, NULL);
}

void writer_free(void)
{
    list_free(writers);
}

void writer_register(writer *details)
{
    list_append(writers, details);
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
