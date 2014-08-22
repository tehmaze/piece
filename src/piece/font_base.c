#include <stdlib.h>
#include <string.h>

#include "piece/font.h"

piece_list *piece_fonts;

void piece_font_iter(piece_list_iterator iterator)
{
    piece_list_foreach(piece_fonts, iterator);
}

void piece_font_free(void)
{
    piece_list_free(piece_fonts);
}

piece_font /*@null@*/ *piece_font_by_name(const char *name)
{
    piece_list_node *node;
    for (node = piece_fonts->head; node != NULL; node = node->next) {
        piece_font *curr = node->data;
        if (!strcmp(curr->name, name)) {
            return curr;
        }
    }

    return NULL;
}
