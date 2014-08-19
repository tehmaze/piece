#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include "list.h"

#define LIST_LOOP_LIMIT 65535  // fair dice roll


void list_new(list *list, int item_size, list_free_function free_fn)
{
    assert(item_size > 0);
    list->length = 0;
    list->item_size = item_size;
    list->head = list->tail = NULL;
    list->free_fn = free_fn;
}

void list_free(list *list)
{
    list_node *current;

    while (list->head != NULL) {
        current = list->head;
        list->head = current->next;

        if (list->free_fn != NULL) {
            list->free_fn(current->data);
        }

        free(current->data);
        free(current);
    }

    free(list);
}

void list_prepend(list *list, void *element)
{
    list_node *node = malloc(sizeof(list_node));
    node->data = malloc(list->item_size);
    memcpy(node->data, element, list->item_size);

    node->next = list->head;
    list->head = node;

    if (list->tail == NULL) {
        list->tail = list->head;
    }

    list->length++;
}

void list_append(list *list, void *element)
{
    list_node *node = malloc(sizeof(list_node));
    node->data = malloc(list->item_size);
    node->next = NULL;
    memcpy(node->data, element, list->item_size);

    if (list->length == 0) {
        list->head = list->tail = node;
    } else {
        list->tail->next = node;
        list->tail = node;
    }

    list->length++;
}

void list_foreach(list *list, list_iterator iterator)
{
    assert(iterator != NULL);

    unsigned int l = 0;
    list_node *node = list->head;
    bool result = true;
    while (node != NULL && result) {
        result = iterator(node->data);
        node = node->next;
        if (++l == LIST_LOOP_LIMIT) {
            fprintf(stderr, "list loop limit reached\n");
            exit(1);
        }
    }
}

int list_size(list *list)
{
    return list->length;
}
