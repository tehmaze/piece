#ifndef __SAUCE_H
#define __SAUCE_H

#include <stdint.h>

#define SAUCE_RECORD_SIZE           128
#define SAUCE_COMMENT_SIZE          64
#define SAUCE_ID                    "SAUCE"
#define SAUCE_COMMENT_ID            "COMNT"

#define SAUCE_DATA_TYPE_NONE        0
#define SAUCE_DATA_TYPE_CHARACTER   1
#define SAUCE_DATA_TYPE_BITMAP      2
#define SAUCE_DATA_TYPE_VECTOR      3
#define SAUCE_DATA_TYPE_AUDIO       4
#define SAUCE_DATA_TYPE_BINARYTEXT  5
#define SAUCE_DATA_TYPE_XBIN        6
#define SAUCE_DATA_TYPE_ARCHIVE     7
#define SAUCE_DATA_TYPE_EXECUTABLE  8

#define SAUCE_FILE_TYPE_NONE        0

#define SAUCE_FILE_TYPE_ASCII       0
#define SAUCE_FILE_TYPE_ANSI        1
#define SAUCE_FILE_TYPE_ANSIMATION  2
#define SAUCE_FILE_TYPE_RIPSCRIPT   3
#define SAUCE_FILE_TYPE_PCBOARD     4
#define SAUCE_FILE_TYPE_AVATAR      5
#define SAUCE_FILE_TYPE_HTML        6
#define SAUCE_FILE_TYPE_SOURCE      7
#define SAUCE_FILE_TYPE_TUNDRADRAW  8

#define SAUCE_FILE_TYPE_XBIN        0


typedef struct sauce_flags_s {
    uint8_t     flag_b        : 1;
    uint8_t     flag_ls       : 2;
    uint8_t     flag_ar       : 2;
    uint8_t     flag_reserved : 3;
} sauce_flags;

typedef struct sauce_s {
    char        id[6];
    char        version[3];
    char        title[36];
    char        author[21];
    char        group[21];
    char        date[9];
    int32_t     file_size;
    uint8_t     data_type;
    uint8_t     file_type;
    uint16_t    tinfo[4];
    uint8_t     comments;
    sauce_flags flags;
    char        filler[23];
    char        **comment;
} sauce;

typedef enum sauce_ls_e {
    SAUCE_LS_LEGACY,
    SAUCE_LS_8PIXEL,
    SAUCE_LS_9PIXEL,
    SAUCE_LS_INVALID
} sauce_ls_t;

typedef enum sauce_ar_e {
    SAUCE_AR_LEGACY,
    SAUCE_AR_STRETCH,
    SAUCE_AR_SQUARE,
    SAUCE_AR_INVALID
} sauce_ar_t;

sauce  *sauce_read(FILE *fd);
void    sauce_read_comments(FILE *fd, char **comment, int32_t comments);
void    sauce_free(sauce *record);
bool    sauce_flag_non_blink(sauce *record);
uint8_t sauce_flag_letter_spacing(sauce *record);

#endif // __SAUCE_H
