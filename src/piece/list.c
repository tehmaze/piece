#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include "piece/list.h"
#include "piece/util.h"

void piece_list_new(piece_list *list, piece_list_free_function free_fn)
{
    list->length = 0;
    list->head = NULL;
    list->tail = NULL;
    list->free_fn = free_fn;
}

void piece_list_free(piece_list *list)
{
    if (list == NULL)
        return;

    piece_list_node *current = NULL;
    while (list->head != NULL) {
        current = list->head;
        list->head = current->next;

        if (list->free_fn != NULL && current->data != NULL) {
            list->free_fn(current->data);
        }
        free(current);
    }

    free(list);
}

void /*@null@*/ *piece_list_get(piece_list *list, uint32_t index)
{
    if (list == NULL || list->head == NULL)
        return NULL;

    piece_list_node *node = list->head;
    for (uint32_t count = 0; count < index; ++count) {
        if (node->next == NULL) {
            return NULL;
        }
        node = node->next;
    }
    return node->data;
}

void piece_list_prepend(piece_list *list, void *element)
{
    piece_list_node *node = malloc(sizeof(piece_list_node));
    node->next = list->head;
    node->data = element;
    list->head = node;

    if (list->tail == NULL) {
        list->tail = list->head;
    }

    list->length++;
}

void piece_list_append(piece_list *list, void *element)
{
    piece_list_node *node = piece_allocate(sizeof(piece_list_node));
    node->data = element;
    node->next = NULL;

    if (list->length == 0) {
        list->head = list->tail = node;
    } else {
        list->tail->next = node;
        list->tail = node;
    }

    list->length++;
}

void piece_list_foreach(piece_list *list, piece_list_iterator iterator)
{
    assert(iterator != NULL);

    piece_list_node *node = list->head;
    bool result = true;

    while (node != NULL && result) {
        result = iterator(node->data);
        node = node->next;
    }
}

int piece_list_size(piece_list *list)
{
    return list->length;
}
