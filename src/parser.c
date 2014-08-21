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

parser *parser_for(FILE *fd, const char *filename)
{
    char *extension;
    parser *found = NULL;
    list_node *node;
    parser *current;

    // Strategy 1: use the parser probes
    for (node = parsers->head; node != NULL; node = node->next) {
        current = node->data;
        if (current->probe != NULL) {
            if (fseek(fd, 0, SEEK_SET)) {
                fprintf(stderr, "%s: could not rewind\n", filename);
                break;
            }
            else if (current->probe(fd, filename)) {
                found = current;
                break;
            }
        }
    }


    // Strategy 2: use the file extension
    if (found == NULL) {
        extension = get_extension(filename);
        if (!strcmp(extension, "")) {

        } else {
            /* Iterate over all known parsers and their extensions */
            for (node = parsers->head; node != NULL; node = node->next) {
                current = node->data;
                for (int i = 0; current->extensions[i] != NULL; ++i) {
                    if (!strcmp(current->extensions[i], extension)) {
                        found = current;
                        node = NULL;
                        break;
                    }
                }
                if (node == NULL)
                    break;
            }
        }

        free(extension);
    }

    if (found == NULL) {
        fprintf(stderr, "%s: no suitable parser found, using ANSi parser\n",
                        filename);
        found = parser_for_type("ansi");
    }

    return found;
}
