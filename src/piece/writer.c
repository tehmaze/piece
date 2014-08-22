#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "piece/list.h"
#include "piece/writer.h"
#include "piece/writer/image.h"
#include "piece/writer/text.h"
#include "piece/util.h"

static piece_list *piece_writers;

void piece_writer_init(void)
{
    piece_writers = piece_allocate(sizeof(piece_list));
    piece_list_new(piece_writers, NULL);

    piece_image_writer_init();
    piece_text_writer_init();
}

void piece_writer_free(void)
{
    piece_list_free(piece_writers);
}

void piece_writer_register(piece_writer *details)
{
    piece_list_append(piece_writers, details);
}

void piece_writer_iter(piece_list_iterator iterator)
{
    piece_list_foreach(piece_writers, iterator);
}

piece_writer *piece_writer_for_type(const char *typename)
{
    piece_list_node *node = piece_writers->head;
    while (node != NULL) {
        if (!strcmp(((piece_writer *) node->data)->name, typename)) {
            return (piece_writer *) node->data;
        }
        node = node->next;
    }

    return NULL;
}
