#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "list.h"
#include "parser.h"
#include "util.h"

static list *parsers;

void parser_init()
{
    parsers = malloc(sizeof(list));
    if (parsers == NULL) {
        fprintf(stderr, "out of memory");
        exit(1);
    }
    list_new(parsers, sizeof(parser), NULL);
}

void parser_register(parser details)
{
    fprintf(stderr, "parser: registering %s\n", details.name);
    list_append(parsers, &details);
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
        free(extension);
        return parser_for_type("ansi");

    } else {
        free(extension);

        /* Iterate over all known parsers and their extensions */
        list_node *node = parsers->head;
        while (node != NULL) {
            parser *current = (parser *) node->data;
            for (int i = 0; current->extensions[i] != NULL; ++i) {
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

    printf("found parser %s for %s\n", parser_name, filename);
    return found;
}
