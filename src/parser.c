#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "list.h"
#include "parser.h"
#include "util.h"

static list *parsers;

void parser_init(void)
{
    parsers = allocate(sizeof(list));
    list_new(parsers, NULL);
}

void parser_free(void)
{
    list_free(parsers);
}

void parser_register(parser *details)
{
    list_append(parsers, details);
}

void parser_iter(list_iterator iterator)
{
    list_foreach(parsers, iterator);
}

parser *parser_for_type(const char *typename)
{
    list_node *node = parsers->head;
    while (node != NULL) {
        if (!strcmp(((parser *) node->data)->name, typename)) {
            return (parser *) node->data;
        }
        node = node->next;
    }

    return NULL;
}

parser *parser_for(const char *filename)
{
    char *extension = get_extension(filename);
    char *parser_name = "";
    parser *found = NULL;

    if (!strcmp(extension, "")) {
        found = parser_for_type("ansi");
        parser_name = "ansi";

    } else {
        /* Iterate over all known parsers and their extensions */
        list_node *node = parsers->head;
        while (node != NULL) {
            parser *current = (parser *) node->data;
            for (int i = 0; current->extensions[i] != NULL; ++i) {
                printf("%s <> %s\n", current->extensions[i], extension);
                if (!strcmp(current->extensions[i], extension)) {
                    parser_name = current->name;
                    found = current;
                    node = NULL;
                    break;
                }
            }

            if (node != NULL)
                node = node->next;
        }
    }

    free(extension);
    printf("found parser %s for %s\n", parser_name, filename);
    return found;
}
