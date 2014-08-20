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

font *font_by_name(const char *name)
{
    list_node *node = fonts->head;
    while (node != NULL) {
        if (!strcmp(((font *) node->data)->name, name)) {
            return (font *) node->data;
        }
        node = node->next;
    }

    return NULL;
}
