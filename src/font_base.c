#include <stdlib.h>
#include <string.h>
#include "font.h"

list *fonts;

void font_iter(list_iterator iterator)
{
    list_foreach(fonts, iterator);
}

void font_free(void)
{
    list_free(fonts);
}

font /*@null@*/ *font_by_name(const char *name)
{
    list_node *node;
    for (node = fonts->head; node != NULL; node = node->next) {
        font *curr = node->data;
        if (!strcmp(curr->name, name)) {
            return curr;
        }
    }

    return NULL;
}
