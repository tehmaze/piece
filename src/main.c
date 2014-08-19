#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>

#include "list.h"
#include "font.h"
#include "parser.h"
#include "parser/ansi.h"
#include "parser/xbin.h"
#include "writer.h"
#include "writer/image.h"
#include "writer/text.h"

const char* program_name;
const char* fontname_default = "cp437_8x16";

bool print_font_list_item(void *item)
{
    font *current = (font *)item;
    printf("  %s", current->name);
    if (!strcmp(current->name, fontname_default)) {
        printf(" (default)");
    }
    printf("\n");
    return true;
}

void print_font_list(void)
{
    printf("supported fonts:\n");
    font_iter(print_font_list_item);
}

bool print_writer_list_item(void *item)
{
    writer *current = (writer *) item;
    printf("  %s\n", current->name);
    return true;
}

void print_writer_list(void)
{
    printf("supported writers:\n");
    writer_iter(print_writer_list_item);
}

bool print_type_list_item(void *item)
{
    parser *current = (parser *)item;
    printf("  %-8s: %s\n", current->name, current->description);
    return true;
}

void print_type_list(void)
{
    printf("supported file types:\n");
    parser_iter(print_type_list_item);
}

void print_usage(FILE *stream, int exit_code)
{
    fprintf(stream, "%s [<options>] input\n", program_name);
    fprintf(
        stream,
        "  -h --help                Display this usage inwriterion\n"
        "\n"
        "  -t --type <type>         Input file type, use \"list\" for a list\n"
        "                                            use \"auto\" to auto detect\n"
        "\n"
        "  -f --font <font>         Output font name, use \"list\" for a list\n"
        "  -o --output <filename>   Output file name\n"
        "  -w --writer <type>       Output writer, use \"list\" for a list\n"
    );
    exit(exit_code);
}

int main(int argc, char *argv[])
{
    const char* const short_options = "hvt:f:o:w:";
    const struct option long_options[] = {
        {"help",    no_argument,       NULL, 'h'},
        {"verbose", no_argument,       NULL, 'v'},
        /* Input options */
        {"type",    required_argument, NULL, 't'},
        /* Output options */
        {"font",    required_argument, NULL, 'f'},
        {"output",  required_argument, NULL, 'o'},
        {"writer",  required_argument, NULL, 'w'},
        {0, 0, 0, 0} /* sentinel */
    };

    int next_option, verbose = 0;
    const char *source_filename = NULL;
    const char *source_typename = "auto";
    const char *target_filename = NULL;
    const char *target_fontname = "cp437_8x16";
    const char *target_typename = "png";

    /* Initialize parsers */
    parser_init();
    ansi_parser_init();
    xbin_parser_init();

    /* Initialize writers */
    writer_init();
    image_writer_init();
    text_writer_init();

    /* Initialize fonts */
    font_init();

    /* Parser command line arguments */

    program_name = argv[0];

    do {
        next_option = getopt_long(argc, argv,
                                  short_options,
                                  long_options,
                                  NULL);

        switch (next_option) {
            case 'h':
                print_usage(stdout, 0);
                break;

            case 'v':
                verbose = 1;
                break;

            case 't':
                source_typename = optarg;
                if (!strcmp(source_typename, "list") ||
                    !strcmp(source_typename, "help")) {
                    print_type_list();
                    exit(0);
                }
                break;

            case 'f':
                target_fontname = optarg;
                if (!strcmp(target_fontname, "list") ||
                    !strcmp(target_fontname, "help")) {
                    print_font_list();
                    exit(0);
                }
                break;

            case 'o':
                target_filename = optarg;
                break;

            case 'w':
                target_typename = optarg;
                if (!strcmp(target_typename, "list") ||
                    !strcmp(target_typename, "help")) {
                    print_writer_list();
                    exit(0);
                }
                break;

            case '?':
                fprintf(stderr, "?: invalid argument\n");
                print_usage(stderr, 1);

            case -1:
                break;

            default:
                abort();
        }
    } while (next_option != -1);

    for (int i = optind; i < argc; ++i) {
        if (source_filename == NULL) {
            source_filename = argv[i];
        } else {
            fprintf(stderr, "invalid number of arguments\n");
            fprintf(stderr, "remainder: \"%s\"\n", argv[i]);
            print_usage(stderr, 1);
        }
    }

    if (target_filename == NULL || source_filename == NULL) {
        fprintf(stderr, "both input and output file name required\n");
        print_usage(stderr, 1);
    }

    if (verbose) {
        printf("source: %s\n", source_filename);
        printf("target: %s\n", target_filename);
    }

    parser *source_parser = NULL;
    if (!strcmp(source_typename, "auto")) {
        source_parser = parser_for(source_filename);
    } else {
        source_parser = parser_for_type(source_typename);
    }

    if (source_parser == NULL) {
        fprintf(stderr, "%s: no suitable parser found\n", source_filename);
        exit(1);
    } else {
        printf("%s: using %s parser\n", source_filename, source_parser->name);
    }

    writer *target_writer = writer_for_type(target_typename);
    if (target_writer == NULL) {
        free(source_parser);
        fprintf(stderr, "%s: no suitable writer found\n", target_filename);
        exit(1);
    } else {
        printf("%s: using %s writer\n", target_filename, target_writer->name);
    }

    font *target_font = font_by_name(target_fontname);
    if (target_font == NULL) {
        free(source_parser);
        free(target_writer);
        fprintf(stderr, "%s: unknown font \"%s\"\n", source_filename,
                                                     target_fontname);
        exit(1);
    }

    screen *display = source_parser->read(source_filename);
    if (display == NULL || display->tiles == NULL) {
        free(source_parser);
        free(target_writer);
        fprintf(stderr, "%s: parser failed\n", source_filename);
        exit(1);
    }
    printf("read %d tiles\n", list_size(display->tiles));
    target_writer->write(display, target_filename, target_font);

    free(source_parser);
    free(target_writer);
    screen_free(display);

    return 0;
}
