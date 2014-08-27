#ifndef PIECE_XS_H
#define PIECE_XS_H

#include "piece.h"
#include "piece/font.h"
#include "piece/screen.h"
#include "piece/parser.h"
#include "piece/writer/image.h"
#include "sauce.h"

#define XS_STATE(type, x)                               \
    INT2PTR(type, SvROK(x) ? SvIV(SvRV(x)) : SvIV(x))

#define XS_STRUCT2OBJ(sv, class, object) do {           \
    if (object == NULL) {                               \
        sv_setsv(sv, &PL_sv_undef);                     \
    } else {                                            \
        sv_setref_pv(sv, class, (void *) object);       \
    }                                                   \
} while(0)

typedef enum {
    PIECE_IMAGE_BMP,
    PIECE_IMAGE_GD,
    PIECE_IMAGE_GIF,
    PIECE_IMAGE_JPEG,
    PIECE_IMAGE_PNG,
} piece_image_type;

typedef struct {
    char         *filename;
    char         *parsername;
    piece_screen *display;
    piece_parser *parser;
    piece_font   *font;
} piece_t;

#endif /* PIECE_XS_H */
