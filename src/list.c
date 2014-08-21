#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include "list.h"
#include "util.h"

void list_new(list *list, list_free_function free_fn)
{
    list->length = 0;
    list->head = NULL;
    list->tail = NULL;
    list->free_fn = free_fn;
}

void list_free(list *list)
{
    if (list == NULL)
        return;

    list_node *current = NULL;
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

void /*@null@*/ *list_get(list *list, uint32_t index)
{
    if (list == NULL || list->head == NULL)
        return NULL;

    list_node *node = list->head;
    for (uint32_t count = 0; count < index; ++count) {
        if (node->next == NULL) {
            return NULL;
        }
        node = node->next;
    }
    return node->data;
}

void list_prepend(list *list, void *element)
{
    list_node *node = malloc(sizeof(list_node));
    node->next = list->head;
    node->data = element;
    list->head = node;

    if (list->tail == NULL) {
        list->tail = list->head;
    }

    list->length++;
}

void list_append(list *list, void *element)
{
    list_node *node = allocate(sizeof(list_node));
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

void list_foreach(list *list, list_iterator iterator)
{
    assert(iterator != NULL);

    list_node *node = list->head;
    bool result = true;

    while (node != NULL && result) {
        result = iterator(node->data);
        node = node->next;
    }
}

int list_size(list *list)
{
    return list->length;
}
