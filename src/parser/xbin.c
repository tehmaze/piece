#include <stdio.h>

#include "list.h"
#include "parser.h"
#include "parser/xbin.h"
#include "screen.h"

screen *xbin_parser_read(const char *filename)
{
    return NULL;
}

void xbin_parser_render(screen *display, unsigned int output_type)
{
}

static char *xbin_extensions[] = {
    "xb",
    "xbin",
    NULL
};
static parser xbin_parser = {
    "xbin",
    "eXtended Binary",
    xbin_parser_read,
    xbin_parser_render,
    xbin_extensions
};

void xbin_parser_init(void) {
    parser_register(xbin_parser);
}
