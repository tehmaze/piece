#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>

#include "piece.h"
#include "piece/banner.h"
#include "piece/version.h"

piece_option_flags *piece_options = NULL;

const char* program_name;
const char* fontname_default = "cp437_8x16";
const char* palette_default = "auto";

bool print_font_piece_list_item(void *item)
{
    piece_font *font = item;
    char **alias = NULL;
    uint32_t aliases = 0, i;
    printf("  %s", font->name);
    if (font->aliases) {
        for (i = 0; i < font->aliases; ++i) {
            if (font->alias[i]->listed) {
                alias = realloc(alias, (aliases + 1) * sizeof(char *));
                alias[aliases++] = (char *) font->alias[i]->alias;
            }
        }
    }
    if (aliases > 0) {
        printf(" (");
        for (i = 0; i < aliases; ++i) {
            printf("%s", alias[i]);
            if (i != aliases - 1) {
                printf(", ");
            }
        }
        printf(")");
        free(alias);
    }
    printf("\n");
    return true;
}

void print_font_list(void)
{
    printf("supported fonts:\n");
    piece_font_iter(print_font_piece_list_item);
}

bool print_palette_piece_list_item(void *item)
{
    piece_palette *current = item;
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
    piece_palette_iter(print_palette_piece_list_item);
}

bool print_writer_piece_list_item(void *item)
{
    piece_writer *current = item;
    printf("  %-8s: %s\n", current->name, current->description);
    return true;
}

void print_writer_list(void)
{
    printf("supported writers:\n");
    piece_writer_iter(print_writer_piece_list_item);
}

bool print_type_piece_list_item(void *item)
{
    piece_parser *current = item;
    printf("  \x1b[1;31m%-10s\x1b[0m description: %s\n", current->name, current->description);
    if (current->extensions != NULL) {
        printf("             extensions:  \x1b[1m");
        for (int i = 0; ; i++) {
            if (current->extensions[i] == NULL) {
                printf("\n");
                break;
            }
            printf("*.%s ", current->extensions[i]);
        }
        printf("\x1b[0m");
    }
    return true;
}

void print_type_list(void)
{
    printf("supported file types:\n");
    piece_parser_iter(print_type_piece_list_item);
}

void print_usage(FILE *stream, bool long_help)
{
    fputs(piece_banner, stream);
    fprintf(stream, "%s [<options>] input\n", program_name);
    fprintf(
        stream,
        "  -V --version             Display version information\n"
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
        "  -i --ice-colors          Output using iCE colors\n"
        "  -a --animate             Output animation for blink\n"
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
    piece_source_option_flags *source;
    piece_target_option_flags *target;
    const char* const short_options = "hHqvVt:f:o:p:w:ia";
    const struct option long_options[] = {
        {"help",       no_argument,       NULL, 'h'},
        {"long-help",  no_argument,       NULL, 'H'},
        {"quiet",      no_argument,       NULL, 'q'},
        {"verbose",    no_argument,       NULL, 'v'},
        {"version",    no_argument,       NULL, 'V'},
        /* Input options */
        {"type",       required_argument, NULL, 't'},
        /* Output options */
        {"font",       required_argument, NULL, 'f'},
        {"output",     required_argument, NULL, 'o'},
        {"palette",    required_argument, NULL, 'p'},
        {"writer",     required_argument, NULL, 'w'},
        {"ice-colors", no_argument,       NULL, 'i'},
        {"animate",    no_argument,       NULL, 'a'},
        {0, 0, 0, 0} /* sentinel */
    };

    piece_options = piece_allocate(sizeof(piece_option_flags));
    piece_options->source = source = piece_allocate(sizeof(piece_source_option_flags));
    piece_options->source->parsername = "auto";
    piece_options->target = target = piece_allocate(sizeof(piece_target_option_flags));
    piece_options->target->fd = NULL;
    piece_options->target->filename = NULL;
    piece_options->target->font_name = NULL;         /* Let parser decide */
    piece_options->target->writer_name = "image";
    piece_options->target->image = piece_allocate(sizeof(piece_image_option_flags));
    piece_options->target->image->transparent = false;
    piece_options->target->image->palette = NULL;   /* Let parser decide */
    piece_options->target->image->ice_colors = 0;   /* Let parser decide */
    piece_options->target->image->animate = false;
    piece_options->verbose = 0;

    /* Initialize parsers */
    piece_parser_init();

    /* Initialize writers */
    piece_writer_init();

    /* Initialize fonts */
    piece_font_init();
    sauce_init();

    /* Initialize palettes */
    piece_palette_init();

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
            piece_options->verbose++;
            piece_options->quiet = false;
            break;

        case 'q':
            piece_options->quiet = true;
            piece_options->verbose = 0;
            break;

        case 'V':
            printf("piece version %s (c) Wijnand Modderman-Lenstra, https://maze.io/\n",
                   PIECE_VERSION);
            goto exit_free;

        case 't':
            source->parsername = optarg;
            if (!strcmp(optarg, "list") ||
                !strcmp(optarg, "help")) {
                print_type_list();
                exit(0);
            }
            break;

        case 'f':
            target->font_name = optarg;
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
            target->image->palette_name = optarg;
            if (!strcmp(optarg, "list") ||
                !strcmp(optarg, "help")) {
                print_palette_list();
                goto exit_free;
            }
            break;

        case 'w':
            target->writer_name = optarg;
            if (!strcmp(optarg, "list") ||
                !strcmp(optarg, "help")) {
                print_writer_list();
                goto exit_free;
            }
            break;

        case 'i':
            target->image->ice_colors = 2;
            break;

        case 'a':
            target->image->animate = true;
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
    dprintf("target: %s\n", target->filename);

    if (!strcmp(source->filename, target->filename)) {
        fprintf(stderr, "%s: would overwrite\n", source->filename);
        status = 1;
        goto exit_free;
    }

    if ((fd = fopen(source->filename, "rb")) == NULL) {
        fprintf(stderr, "%s: error opening input file\n",
                        source->filename);
        status = 1;
        goto exit_free;
    }

    source->parser = NULL;
    if (!strcmp(source->parsername, "auto")) {
        source->parser = piece_parser_for(fd, source->filename);
    } else {
        source->parser = piece_parser_for_type(source->parsername);
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

    target->writer = piece_writer_for_type(target->writer_name);
    if (target->writer == NULL) {
        fprintf(stderr, "%s: no suitable writer found\n",
                        target->filename);
        status = 1;
        goto exit_close;
    }

    if (target->image->palette_name != NULL) {
        target->image->palette = piece_palette_by_name(target->image->palette_name);
        if (target->image->palette == NULL) {
            fprintf(stderr, "%s: unknown palette \"%s\"\n",
                            target->filename,
                            target->image->palette_name);
            status = 1;
            goto exit_close;
        }
    }

    if (target->font_name != NULL) {
        target->font = piece_font_by_name(target->font_name);
        if (target->font == NULL) {
            fprintf(stderr, "%s: unknown font \"%s\"\n",
                            target->filename,
                            target->font_name);
            status = 1;
            goto exit_close;
        }
    }

    /* Parse the file */
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

    /* Display some info about the file */
    if (!piece_options->quiet) {
        printf("source: %s\n", source->filename);
        printf("target: %s\n", target->filename);
        if (target->display->record &&
            !strncmp(target->display->record->id, SAUCE_ID, 5)) {
            printf(" title: %.30s\n", target->display->record->title);
            printf("author: %.20s\n", target->display->record->author);
            printf(" group: %.20s\n", target->display->record->group);
            printf("  date: %.8s\n", target->display->record->date);
        } else {
            fprintf(stderr, "%s: no SAUCE record\n", source->filename);
        }
    }

    /* Finally, write out using the selected writer */
    target->writer->write(target->display, target->filename);

    if (!piece_options->quiet) {
        printf("\ndone ;]\n");
    }

exit_close:
    rewind(fd);
    fclose(fd);

exit_free:
    piece_screen_free(target->display);
    piece_palette_free();
    piece_font_free();
    piece_writer_free();
    piece_parser_free();
    free(piece_options->target->image);
    free(piece_options->target);
    free(piece_options->source);
    free(piece_options);

    return status;
}
