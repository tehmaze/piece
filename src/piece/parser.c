#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "piece/list.h"
#include "piece/options.h"
#include "piece/parser.h"
#include "piece/parser/ansi.h"
#include "piece/parser/artworx.h"
#include "piece/parser/ascii.h"
#include "piece/parser/ascii7.h"
#include "piece/parser/binary.h"
#include "piece/parser/icedraw.h"
#include "piece/parser/irc.h"
#include "piece/parser/pcboard.h"
#include "piece/parser/tundradraw.h"
#include "piece/parser/xbin.h"
#include "piece/util.h"

static piece_list *piece_parsers;

void piece_parser_init(void)
{
    piece_parsers = piece_allocate(sizeof(piece_list));
    piece_list_new(piece_parsers, NULL);

    piece_ansi_parser_init();
    piece_artworx_parser_init();
    piece_ascii_parser_init();
    piece_ascii7_parser_init();
    piece_binary_parser_init();
    piece_icedraw_parser_init();
    piece_irc_parser_init();
    piece_pcboard_parser_init();
    piece_tundradraw_parser_init();
    piece_xbin_parser_init();
}

void piece_parser_free(void)
{
    piece_list_free(piece_parsers);
}

void piece_parser_register(piece_parser *details)
{
    piece_list_append(piece_parsers, details);
}

void piece_parser_iter(piece_list_iterator iterator)
{
    piece_list_foreach(piece_parsers, iterator);
}

piece_parser *piece_parser_for_type(const char *typename)
{
    piece_list_node *node = piece_parsers->head;
    while (node != NULL) {
        if (!strcmp(((piece_parser *) node->data)->name, typename)) {
            return (piece_parser *) node->data;
        }
        node = node->next;
    }

    return NULL;
}

piece_parser *piece_parser_for(FILE *fd, const char *filename)
{
    char *extension;
    piece_parser *found = NULL;
    piece_list_node *node;
    piece_parser *current;

    // Strategy 1: use the parser probes
    for (node = piece_parsers->head; node != NULL; node = node->next) {
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
        extension = piece_get_extension(filename);
        if (!strcmp(extension, "")) {

        } else {
            /* Iterate over all known piece_parsers and their extensions */
            for (node = piece_parsers->head; node != NULL; node = node->next) {
                current = node->data;
                if (current->extensions == NULL) {
                    continue;
                }
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

        //free(extension);
    }

    if (found == NULL) {
        fprintf(stderr, "%s: no suitable parser found, using ANSi parser\n",
                        filename);
        found = piece_parser_for_type("ansi");
    } else if (piece_options->verbose) {
        printf("%s: detected %s format\n", filename, found->name);
    }

    return found;
}
