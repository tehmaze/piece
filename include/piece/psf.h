#ifndef PIECE_PSF_H
#define PIECE_PSF_H

#include <stdint.h>

#define PSF1_MAGIC0     0x36
#define PSF1_MAGIC1     0x04
#define PSF1_MAGIC      {PSF1_MAGIC0, PSF1_MAGIC1}

#define PSF1_MODENONE   0x00
#define PSF1_MODE512    0x01
#define PSF1_MODEHASTAB 0x02
#define PSF1_MODEHASSEQ 0x04
#define PSF1_MAXMODE    0x05

#define PSF1_SEPARATOR  0xFFFF
#define PSF1_STARTSEQ   0xFFFE

#define PSF2_MAGIC0     0x72
#define PSF2_MAGIC1     0xb5
#define PSF2_MAGIC2     0x4a
#define PSF2_MAGIC3     0x86
#define PSF2_MAGIC      {PSF2_MAGIC0, PSF2_MAGIC1, PSF2_MAGIC2, PSF2_MAGIC3}

/* bits used in flags */
#define PSF2_HAS_UNICODE_TABLE 0x01

/* max version recognized so far */
#define PSF2_MAXVERSION 0

/* UTF8 separators */
#define PSF2_SEPARATOR  0xFF
#define PSF2_STARTSEQ   0xFE

typedef struct psf1_header_s {
    uint8_t magic[2];     /* Magic number */
    uint8_t mode;         /* PSF font mode */
    uint8_t charsize;     /* Character size */
} psf1_header;

typedef struct psf2_header_s {
    uint8_t  magic[4];
    uint32_t version;
    uint32_t headersize;    /* offset of bitmaps in file */
    uint32_t flags;
    uint32_t length;        /* number of glyphs */
    uint32_t charsize;      /* number of bytes for each character */
    uint32_t height, width; /* max dimensions of glyphs */
    /* charsize = height * ((width + 7) / 8) */
} psf2_header;

typedef uint32_t piece_psf_unicode;

/* Sequence of unicode characters */
typedef struct piece_psf_us_s {
    struct piece_psf_us_s *next;
    struct piece_psf_us_s *prev;
    piece_psf_unicode     uc;
} piece_psf_us;

/* List of unicode sequences */
typedef struct piece_psf_ul_s {
    struct piece_psf_ul_s *next;
    struct piece_psf_ul_s *prev;
    struct piece_psf_us_s *seq;
} piece_psf_ul;

typedef struct piece_psf_s {
    psf1_header  *psf1;
    psf2_header  *psf2;
    uint8_t      char_width;
    uint8_t      char_height;
    uint32_t     char_size;
    off_t        glyph_offset;
    uint8_t      *glyph;
    off_t        glyph_size;
    uint16_t     glyphs;
    uint8_t      *buffer;
    off_t        buffer_size;
    bool         utf8;
    bool         has_unicode;
    piece_psf_ul *uclistheads;
} piece_psf;

#endif /* PIECE_PSF_H */
