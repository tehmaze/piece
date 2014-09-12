#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "piece/psf.h"
#include "piece/util.h"

static const char psf1_magic[] = PSF1_MAGIC;
static const char psf2_magic[] = PSF2_MAGIC;

static uint16_t v16(uint8_t *ptr)
{
    uint16_t v = 0;
    v |= (*ptr++);
    v |= (*ptr++) << 8;
    return v;
}

static uint32_t v32(uint8_t *ptr)
{
    uint32_t v = 0;
    v |= (*ptr++);
    v |= (*ptr++) << 8;
    v |= (*ptr++) << 16;
    v |= (*ptr++) << 24;
    return v;
}

/* Read an UTF-8 character as uint32_t */
static piece_psf_unicode vutf8(uint8_t **begptr, int n, bool *err)
{
    uint8_t *ptr = *begptr;
    int16_t bit = 0x80, need = 0x00;
    bool bad = false;
    piece_psf_unicode uc, uc2;

    uc = *ptr++;
    while (uc & bit) {
        need++;
        bit >>= 1;
    }
    uc &= (bit - 1);
    if (n && n < need) {
        *err = true;
        return 0;
    }

    if (need == 1) {
        bad = true;
    } else if (need) {
        while (--need) {
            uc2 = *ptr++;
            if ((uc2 & 0xc0) != 0x80) {
                bad = true;
                break;
            }
            uc = ((uc << 16) | (uc2 & 0x3f));
        }
    }

    if (bad) {
        *err = true;
        return 0;
    } else {
        *err = false;
        *begptr = ptr;
        return uc;
    }
}

static bool autf8(uint8_t **ptr, int n, piece_psf_unicode *out)
{
    bool err;
    uint64_t uc = vutf8(ptr, n, &err);
    if (err) {
        return false;
    } else {
        *out = uc;
        return true;
    }
}

static bool aucs2(uint8_t **ptr, int n, piece_psf_unicode *out)
{
    piece_psf_unicode u1, u2;

    if (n < 2) {
        return false;
    }

    u1 = *(*ptr++);
    u2 = *(*ptr++);
    *out = (u1 | (u2 << 8));
    return true;
}

static void psf_unicode_add_pair(piece_psf_ul *up, piece_psf_unicode uc)
{
    piece_psf_ul *ul;
    piece_psf_us *us;

    ul = piece_allocate(sizeof(piece_psf_ul));
    ul->seq = us;
    ul->prev = up->prev;
    ul->prev->next = ul;
    ul->next = NULL;

    us = piece_allocate(sizeof(piece_psf_us));
    us->uc = uc;
    us->prev = us;
    us->next = NULL;

    up->prev = ul;
}

static void psf_unicode_add_seq(piece_psf_ul *up, piece_psf_unicode uc)
{
    piece_psf_ul *ul;
    piece_psf_us *us;

    ul = up->prev;
    us = piece_allocate(sizeof(piece_psf_us));
    us->uc = uc;
    us->prev = ul->seq->prev;
    us->prev->next = us;
    us->next = NULL;
    ul->seq->prev = us;
}

static void psf_unicode_del_entry(piece_psf_ul *up)
{
    up->next = NULL;
    up->seq = NULL;
    up->prev = up;
}

static bool psf_unicode_get_entry(begptr, endptr, up, utf8)
    uint8_t **begptr;
    uint8_t **endptr;
    piece_psf_ul *up;
    bool utf8;
{
    uint8_t c;
    uint8_t inseq = 0;
    uint8_t **ptr = begptr;
    piece_psf_unicode uc;

    while(true) {
        if (*endptr == *begptr) {
            return false;
        }

        if (utf8) {
            uc = *(*ptr)++;
            if (uc == PSF2_SEPARATOR)
                break;
            if (uc == PSF2_STARTSEQ) {
                inseq = 1;
                continue;
            }
            --(*ptr);
            if (!autf8(begptr, *endptr - *begptr, &uc)) {
                return false;
            }

        } else {
            if (!aucs2(begptr, *endptr - *begptr, &uc)) {
                return false;
            }
            if (uc == PSF1_SEPARATOR) {
                break;
            }
            if (uc == PSF1_STARTSEQ) {
                inseq = 1;
                continue;
            }
        }

        if (inseq < 2) {
            psf_unicode_add_pair(up, uc);
        } else {
            psf_unicode_add_seq(up, uc);
        }
        if (inseq) {
            inseq++;
        }
    }
}

static bool psf1_parse(piece_psf *psf)
{
    uint8_t *ptr = psf->buffer + sizeof(psf1_magic);

    psf->psf1->mode = *ptr++;
    psf->psf1->charsize = *ptr++;

    psf->char_width = 8; /* constant */
    psf->char_height = psf->psf1->charsize;
    psf->char_size = psf->psf1->charsize;
    psf->glyph_offset = sizeof(psf1_header) - sizeof(psf1_magic);
    psf->glyphs = (psf->psf1->mode & PSF1_MODE512) ? 512 : 256;
    psf->has_unicode = (psf->psf1->mode & (PSF1_MODEHASTAB | PSF1_MODEHASSEQ));
    psf->utf8 = false;

    return true;
}

static bool psf2_parse(piece_psf *psf)
{
    uint8_t *ptr = psf->buffer + sizeof(psf2_magic);

    psf->psf2->version = v32(ptr);
    psf->psf2->headersize = v32(ptr);
    psf->psf2->flags = v32(ptr);
    psf->psf2->length = v32(ptr);
    psf->psf2->charsize = v32(ptr);
    psf->psf2->height = v32(ptr);
    psf->psf2->width = v32(ptr);

    psf->char_width = (uint8_t) (psf->psf2->width & 0xff);
    psf->char_height = (uint8_t) (psf->psf2->height & 0xff);
    psf->char_size = psf->psf2->charsize;
    psf->glyph_offset = sizeof(psf2_header);
    psf->glyphs = psf->psf2->length;
    psf->has_unicode = (psf->psf2->flags & PSF2_HAS_UNICODE_TABLE);
    psf->utf8 = true;
}

piece_psf *piece_psf_parse(const char *filename)
{
    FILE *fp;
    struct stat st;
    uint8_t ch;
    uint8_t magic[4];
    piece_psf *psf = piece_allocate(sizeof(piece_psf));

    if ((fp = fopen(filename, "rb")) == NULL) {
        return NULL;
    }

    /* Slurp file contents into buffer */
    fstat(fileno(fp), &st);
    psf->buffer = piece_allocate(st.st_size);
    psf->buffer_size = st.st_size;
    fread(psf->buffer, st.st_size, 1, fp);

    /* Magic check */
    if (!strncmp(psf->buffer, psf1_magic, 2)) {
        psf->psf1 = piece_allocate(sizeof(psf1_header));
        strncpy(psf->psf1->magic, psf1_magic, 2);
        if (!psf1_parse(psf)) {
            goto return_bail;
        }
    } else if (!strncmp(psf->buffer, psf2_magic, 4)) {
        psf->psf2 = piece_allocate(sizeof(psf2_header));
        strncpy(psf->psf2->magic, psf2_magic, 4);
        if (!psf2_parse(psf)) {
            goto return_bail;
        }
    } else {
        goto return_bail;
    }

    psf->glyph = psf->buffer + psf->glyph_offset;
    psf->glyph_size = psf->glyphs * psf->char_size;
    psf->uclistheads = piece_allocate(psf->glyphs * sizeof(piece_psf_ul));

    for (int i = 0; i < psf->glyphs; ++i) {
        psf_unicode_del_entry(&psf->uclistheads[i]);
    }
    if (psf->has_unicode) {
        uint8_t *begptr = psf->buffer + psf->glyph_offset
            + psf->glyphs *  psf->char_size;
        uint8_t *endptr = psf->buffer + st.st_size;

        for (int i = 0; i < psf->glyphs; ++i) {
            if (!psf_unicode_get_entry(&begptr, &endptr, &psf->uclistheads[i],
                                       psf->utf8)) {
                goto return_bail;
            }
        }
    }

return_bail:
    if (psf != NULL) {
        if (psf->psf1 != NULL) {
            free(psf->psf1);
        }
        if (psf->psf2 != NULL) {
            free(psf->psf2);
        }
        if (psf->buffer != NULL) {
            free(psf->buffer);
        }
        free(psf);
        psf = NULL;
    }

return_font:

    return psf;
}
