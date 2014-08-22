#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>

#include "font.h"
#include "list.h"
#include "options.h"
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
#include "util.h"
#include "writer/image.h"
#include "writer/text.h"

option_flags *options = NULL;

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

void print_usage(FILE *stream, bool long_help)
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
}

int main(int argc, char *argv[])
{
    FILE *fd = NULL;
    int next_option, status = 0;
    source_option_flags *source;
    target_option_flags *target;
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

    options = allocate(sizeof(option_flags));
    options->source = source = allocate(sizeof(source_option_flags));
    options->source->parsername = "auto";
    options->target = target = allocate(sizeof(target_option_flags));
    options->target->filename = NULL;
    options->target->fontname = "cp437_8x16";
    options->target->writername = "image";
    options->target->image = allocate(sizeof(image_option_flags));
    options->target->image->transparent = false;
    options->target->image->palette = "vga";
    options->verbose = 0;

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
            print_usage(stdout, false);
            goto exit_free;

        case 'H':
            print_usage(stdout, true);
            goto exit_free;

        case 'v':
            options->verbose++;
            break;

        case 't':
            source->parsername = optarg;
            if (!strcmp(optarg, "list") ||
                !strcmp(optarg, "help")) {
                print_type_list();
                exit(0);
            }
            break;

        case 'f':
            target->fontname = optarg;
            if (!strcmp(optarg, "list") ||
                !strcmp(optarg, "help")) {
                print_font_list();
                exit(0);
            }
            break;

        case 'o':
            target->filename = optarg;
            break;

        case 'p':
            target->image->palette = optarg;
            if (!strcmp(optarg, "list") ||
                !strcmp(optarg, "help")) {
                print_palette_list();
                goto exit_free;
            }
            break;

        case 'w':
            target->writername = optarg;
            if (!strcmp(optarg, "list") ||
                !strcmp(optarg, "help")) {
                print_writer_list();
                goto exit_free;
            }
            break;

        case -1:
            break;

        default:
            print_usage(stderr, false);
            status = 1;
            goto exit_free;
        }
    } while (next_option != -1);

    for (int i = optind; i < argc; ++i) {
        if (source->filename == NULL) {
            source->filename = argv[i];
        } else {
            fprintf(stderr, "invalid number of arguments\n");
            fprintf(stderr, "remainder: \"%s\"\n", argv[i]);
            print_usage(stderr, false);
            status = 1;
            goto exit_free;
        }
    }

    if (target->filename == NULL ||
        source->filename == NULL ||
        !strcmp(target->filename, source->filename)) {
        fprintf(stderr, "both input and output file name required\n");
        status = 1;
        goto exit_free;
    }

    dprintf("source: %s\n", source->filename);
    dprintf("target: %s\n", source->filename);

    if ((fd = fopen(source->filename, "rb")) == NULL) {
        fprintf(stderr, "%s: error opening input file\n",
                        source->filename);
        status = 1;
        goto exit_free;
    }

    source->parser = NULL;
    if (!strcmp(source->parsername, "auto")) {
        source->parser = parser_for(fd, source->filename);
    } else {
        source->parser = parser_for_type(source->parsername);
    }

    if (source->parser == NULL) {
        fprintf(stderr, "%s: no suitable parser found\n",
                        source->filename);
        status = 1;
        goto exit_close;
    } else {
        dprintf("%s: using %s parser\n", source->filename,
                                         source->parser->description);
    }

    target->writer = writer_for_type(target->writername);
    if (target->writer == NULL) {
        fprintf(stderr, "%s: no suitable writer found\n",
                        target->filename);
        status = 1;
        goto exit_close;
    }

    target->font = font_by_name(target->fontname);
    if (target->font == NULL) {
        fprintf(stderr, "%s: unknown font \"%s\"\n",
                        target->filename,
                        target->fontname);
        status = 1;
        goto exit_close;
    }

    target->display = source->parser->read(fd, source->filename);
    if (target->display == NULL || target->display->tiles == 0) {
        fprintf(stderr, "%s: parser failed\n", source->filename);
        status = 1;
        goto exit_free;
    }
    dprintf("%s: read %d tiles\n", source->filename, target->display->tiles);
    if (target->display->font != NULL) {
        target->font = target->display->font;
    }
    dprintf("%s: using %s writer\n", target->filename,
                                     target->writer->name);
    target->writer->write(target->display, target->filename, target->font);

exit_close:
    rewind(fd);
    fclose(fd);

exit_free:
    screen_free(target->display);
    palette_free();
    font_free();
    writer_free();
    parser_free();
    free(options->target->image);
    free(options->target);
    free(options->source);
    free(options);

    return status;
}
