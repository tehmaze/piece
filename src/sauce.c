#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sauce.h"

#define SAUCE_READ_STRING(fd, record, field) do {                   \
    fread(record->field, sizeof(record->field) - 1, 1, fd);         \
    record->field[sizeof(record->field) - 1] = 0x00;                \
    if (ferror(fd)) {                                               \
        fprintf(stderr,                                             \
        "error %d reading SAUCE field %s\n", ferror(fd), #field);   \
    }                                                               \
    } while(0)

#define SAUCE_READ_VALUE(fd, record, field) do {                    \
    fread(&(record->field), sizeof(record->field), 1, fd);          \
    if (ferror(fd)) {                                               \
        fprintf(stderr,                                             \
        "error %d reading SAUCE field %s\n", ferror(fd), #field);   \
    }                                                               \
    } while(0)

sauce *sauce_read(FILE *fd)
{
    sauce *record = malloc(sizeof(sauce));
    if (record == NULL) {
        return NULL;
    }
    memset(record, 0, sizeof(sauce));

    size_t read = fread(record->id, sizeof(record->id) - 1, 1, fd);
    record->id[sizeof(record->id) - 1] = 0x00;

    if (read != 1 || !strcmp(record->id, SAUCE_ID)) {
        free(record);
        return NULL;
    }

    SAUCE_READ_STRING(fd, record, version);
    SAUCE_READ_STRING(fd, record, title);
    SAUCE_READ_STRING(fd, record, author);
    SAUCE_READ_STRING(fd, record, group);
    SAUCE_READ_STRING(fd, record, date);
    SAUCE_READ_VALUE(fd, record, file_size);
    SAUCE_READ_VALUE(fd, record, data_type);
    SAUCE_READ_VALUE(fd, record, file_type);
    SAUCE_READ_VALUE(fd, record, tinfo);
    SAUCE_READ_VALUE(fd, record, comments);
    SAUCE_READ_VALUE(fd, record, flags);
    SAUCE_READ_STRING(fd, record, filler);

    if (ferror(fd)) {
        fprintf(stderr, "error %d reading SAUCE record\n", ferror(fd));
        free(record);
        return NULL;
    }

    if (record->comments > 0) {
        record->comment = malloc(record->comments * sizeof(*record->comment));
        if (record->comment == NULL) {
            free(record);
            return NULL;
        }
        sauce_read_comments(fd, record->comment, record->comments);
    } else {
        record->comment = NULL;
    }

    return record;
}

void sauce_read_comments(FILE *fd, char **comment, int32_t comments)
{
    int32_t i;
    char id[6];

    long offset = SAUCE_RECORD_SIZE + 5 + SAUCE_COMMENT_SIZE * comments;
    if (!fseek(fd, 0 - offset, SEEK_END)) {
        free(comment);
        return;
    }

    fread(id, sizeof(id) - 1, 1 , fd);
    id[sizeof(id) - 1] = 0x00;

    if (strcmp(id, SAUCE_COMMENT_ID)) {
        free(comment);
        return;
    }

    for (i = 0; i < comments; ++i) {
        char buf[SAUCE_COMMENT_SIZE + 1];
        memset(buf, 0, sizeof(buf));
        fread(buf, SAUCE_COMMENT_SIZE, 1, fd);
        buf[SAUCE_COMMENT_SIZE] = 0x00;

        if (!ferror(fd)) {
            comment[i] = strdup(buf);
            if (comment[i] == NULL) {
                free(comment);
                return;
            }
        } else {
            free(comment);
            return;
        }
    }
}


void sauce_free(sauce *record)
{
    //free(record->comment);
    free(record);
}

bool sauce_flag_non_blink(sauce *record) {
    return (bool) (record != NULL && record->flags.flag_b);
}

uint8_t sauce_flag_letter_spacing(sauce *record) {
    return (record != NULL && record->flags.flag_ls == SAUCE_LS_8PIXEL) ? 8 : 9;
}
