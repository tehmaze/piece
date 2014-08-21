#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>

#include "font.h"
#include "list.h"
#include "palette.h"
#include "parser.h"
#include "parser/ansi.h"
#include "parser/artworx.h"
#include "parser/binary.h"
#include "parser/icedraw.h"
#include "parser/pcboard.h"
#include "parser/tundradraw.h"
#include "parser/xbin.h"
#include "sauce.h"
#include "writer.h"
#include "writer/image.h"
#include "writer/text.h"

const char* program_name;
const char* fontname_default = "cp437_8x16";
const char* palette_default = "auto";

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

bool print_palette_list_item(void *item)
{
    palette *current = (palette *) item;
    printf("  %s", current->name);
    if (!strcmp(current->name, palette_default)) {
        printf(" (default)");
    }
    printf("\n");
    return true;
}

void print_palette_list(void)
{
    printf("supported palettes:\n");
    palette_iter(print_palette_list_item);
}

bool print_writer_list_item(void *item)
{
    writer *current = (writer *) item;
    printf("  %-8s: %s\n", current->name, current->description);
    return true;
}

void print_writer_list(void)
{
    printf("supported writers:\n");
    writer_iter(print_writer_list_item);
}

bool print_type_list_item(void *item)
{
    parser *current = item;
    printf("  \x1b[1;31m%-10s\x1b[0m description: %s\n", current->name, current->description);
    printf("             extensions:  \x1b[1m");
    for (int i = 0; ; i++) {
        if (current->extensions[i] == NULL) {
            printf("\n");
            break;
        }
        printf("*.%s ", current->extensions[i]);
    }
    printf("\x1b[0m");
    return true;
}

void print_type_list(void)
{
    printf("supported file types:\n");
    parser_iter(print_type_list_item);
}

void print_usage(FILE *stream, int exit_code, bool long_help)
{
    fprintf(stream, "%s [<options>] input\n", program_name);
    fprintf(
        stream,
        "  -h --help                Display usage information\n"
        "  -H --long-helo           Display usage information with all options\n"
        "\n"
        "  -t --type <type>         Input file type, use \"list\" for a list\n"
        "                                            use \"auto\" to auto detect\n"
        "\n"
        "  -f --font <font>         Output font name, use \"list\" for a list\n"
        "  -p --palette <palette>   Output palette name, use \"list\" for a list\n"
        "  -o --output <filename>   Output file name\n"
        "  -w --writer <type>       Output writer, use \"list\" for a list\n"
    );

    if (long_help) {
        fprintf(stream, "\n");
        print_type_list();
        fprintf(stream, "\n");
        print_font_list();
        fprintf(stream, "\n");
        print_palette_list();
        fprintf(stream, "\n");
        print_writer_list();
    }

    exit(exit_code);
}

int main(int argc, char *argv[])
{
    FILE *fd = NULL;
    const char* const short_options = "hHvt:f:o:p:w:";
    const struct option long_options[] = {
        {"help",      no_argument,       NULL, 'h'},
        {"long-help", no_argument,       NULL, 'H'},
        {"verbose",   no_argument,       NULL, 'v'},
        /* Input options */
        {"type",      required_argument, NULL, 't'},
        /* Output options */
        {"font",      required_argument, NULL, 'f'},
        {"output",    required_argument, NULL, 'o'},
        {"palette",   required_argument, NULL, 'p'},
        {"writer",    required_argument, NULL, 'w'},
        {0, 0, 0, 0} /* sentinel */
    };

    int next_option, verbose = 0;
    const char *source_filename = NULL;
    const char *source_typename = "auto";
    const char *target_filename = NULL;
    const char *target_fontname = "cp437_8x16";
    const char *target_typename = "image";
    const char *target_palette  = "vga";

    /* Initialize parsers */
    parser_init();
    ansi_parser_init();
    artworx_parser_init();
    binary_parser_init();
    icedraw_parser_init();
    pcboard_parser_init();
    tundradraw_parser_init();
    xbin_parser_init();

    /* Initialize writers */
    writer_init();
    image_writer_init();
    text_writer_init();

    /* Initialize fonts */
    font_init();
    sauce_init();

    /* Initialize palettes */
    palette_init();

    /* Parser command line arguments */

    program_name = argv[0];

    do {
        next_option = getopt_long(argc, argv,
                                  short_options,
                                  long_options,
                                  NULL);

        switch (next_option) {
            case 'h':
                print_usage(stdout, 0, false);
                break;

            case 'H':
                print_usage(stdout, 0, true);
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

            case 'p':
                target_palette = optarg;
                if (!strcmp(target_palette, "list") ||
                    !strcmp(target_palette, "help")) {
                    print_palette_list();
                    exit(0);
                }
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
                print_usage(stderr, 1, false);

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
            print_usage(stderr, 1, false);
        }
    }

    if (target_filename == NULL || source_filename == NULL) {
        fprintf(stderr, "both input and output file name required\n");
        print_usage(stderr, 1, false);
    }

    if (verbose) {
        printf("source: %s\n", source_filename);
        printf("target: %s\n", target_filename);
    }

    if ((fd = fopen(source_filename, "rb")) == NULL) {
        fprintf(stderr, "%s: error opening input file\n", source_filename);
        return 1;
    }

    parser *source_parser = NULL;
    if (!strcmp(source_typename, "auto")) {
        source_parser = parser_for(fd, source_filename);
    } else {
        source_parser = parser_for_type(source_typename);
    }

    if (source_parser == NULL) {
        fprintf(stderr, "%s: no suitable parser found\n", source_filename);
        exit(1);
    } else {
        printf("%s: using %s parser\n", source_filename,
                                        source_parser->description);
    }

    writer *target_writer = writer_for_type(target_typename);
    if (target_writer == NULL) {
        free(source_parser);
        fprintf(stderr, "%s: no suitable writer found\n", target_filename);
        exit(1);
    }

    font *target_font = font_by_name(target_fontname);
    if (target_font == NULL) {
        free(source_parser);
        free(target_writer);
        fprintf(stderr, "%s: unknown font \"%s\"\n", source_filename,
                                                     target_fontname);
        exit(1);
    }

    screen *display = source_parser->read(fd, source_filename);
    if (display == NULL || display->tiles == 0) {
        free(source_parser);
        free(target_writer);
        fprintf(stderr, "%s: parser failed\n", source_filename);
        exit(1);
    }
    printf("%s: read %d tiles\n", source_filename, display->tiles);
    if (display->font != NULL) {
        target_font = display->font;
    }
    printf("%s: using %s writer\n", target_filename, target_writer->name);
    target_writer->write(display, target_filename, target_font);

    screen_free(display);
    parser_free();
    writer_free();
    font_free();
    palette_free();

    return 0;
}
