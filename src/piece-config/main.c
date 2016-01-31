#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "piece/config.h"
#include "piece/version.h"


typedef struct {
    const char *option;
    const char *value;
} config_option_string;

typedef struct {
    const char *option;
    bool        value;
} config_option_bool;

config_option_string config_strings[] = {
    {"--prefix",          INSTALL_PREFIX},
    {"--bin-prefix",      INSTALL_BIN_PREFIX},
    {"--lib-prefix",      INSTALL_LIB_PREFIX},
    {"--includes",        WITH_PIECE_INCLUDES},
    {"--libs",            WITH_PIECE_LIBS},
    {"--version",         PIECE_VERSION},
    {NULL,                NULL}
};

config_option_bool config_bools[] = {
    {"--with-libpiece",   (bool) WITH_LIBPIECE},
    {"--with-libsauce",   (bool) WITH_LIBSAUCE},
    {NULL,                false}
};

static void
usage(char **argv)
{
    int32_t i = 0;
    printf("Usage: %s [OPTIONS]\n", argv[0]);
    printf("Options:\n");
    for (i = 0; config_strings[i].option != NULL; ++i) {
        printf("  %-20s[%s]\n", config_strings[i].option,
                                config_strings[i].value);
    }
    for (i = 0; config_bools[i].option != NULL; ++i) {
        printf("  %-20s[%s]\n", config_bools[i].option,
                                config_bools[i].value ? "yes" : "no");
    }
}

int main(int argc, char **argv)
{
    int32_t i = 0;
    if (argc > 1) {
        for (i = 0; config_strings[i].option != NULL; ++i) {
            if (!strcmp(config_strings[i].option, argv[1])) {
                printf("%s\n", config_strings[i].value);
                return 0;
            }
        }
        for (i = 0; config_bools[i].option != NULL; ++i) {
            if (!strcmp(config_strings[i].option, argv[1])) {
                printf("%s\n", config_strings[i].value ? "yes" : "no");
                return 0;
            }
        }
    }

    usage(argv);
    return 1;
}
