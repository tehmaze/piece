#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sauce.h"

#define SAUCE_READ_STRING(fd, record, field) do {                       \
    if (fread(record->field, sizeof(record->field), 1, fd) == 0 ||      \
        ferror(fd)) {                                                   \
        fprintf(stderr,                                                 \
        "error %d reading SAUCE field %s\n", ferror(fd), #field);       \
        free(record);                                                   \
        return NULL;                                                    \
    }                                                                   \
    record->field[sizeof(record->field) - 1] = 0x00;                    \
} while(0)

#define SAUCE_READ_VALUE(fd, record, field) do {                        \
    if (fread(&(record->field), sizeof(record->field), 1, fd) == 0 ||   \
        ferror(fd)) {                                                   \
        fprintf(stderr,                                                 \
        "error %d reading SAUCE field %s\n", ferror(fd), #field);       \
        free(record);                                                   \
        return NULL;                                                    \
    }                                                                   \
} while(0)

sauce *sauce_read(FILE *fd)
{
    sauce *record = malloc(sizeof(sauce));
    if (record == NULL) {
        return NULL;
    }
    memset(record, 0, sizeof(sauce));

    if (fseek(fd, -SAUCE_RECORD_SIZE, SEEK_END) != 0) {
        fprintf(stderr, "sauce: fseek() failed\n");
        free(record);
        return NULL;
    }
    SAUCE_READ_VALUE(fd, record, id);
    if (memcmp(record->id, SAUCE_ID, sizeof(record->id) - 1)) {
        free(record);
        return NULL;
    }

    SAUCE_READ_VALUE(fd, record, version);
    SAUCE_READ_STRING(fd, record, title);
    SAUCE_READ_STRING(fd, record, author);
    SAUCE_READ_STRING(fd, record, group);
    SAUCE_READ_VALUE(fd, record, date);
    SAUCE_READ_VALUE(fd, record, file_size);
    SAUCE_READ_VALUE(fd, record, data_type);
    SAUCE_READ_VALUE(fd, record, file_type);
    SAUCE_READ_VALUE(fd, record, tinfo);
    SAUCE_READ_VALUE(fd, record, comments);
    SAUCE_READ_VALUE(fd, record, flags);
    SAUCE_READ_STRING(fd, record, filler);

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

    if (fread(id, sizeof(id) - 1, 1 , fd) == 0) {
        return;
    }
    id[sizeof(id) - 1] = 0x00;

    if (strcmp(id, SAUCE_COMMENT_ID)) {
        free(comment);
        return;
    }

    for (i = 0; i < comments; ++i) {
        char buf[SAUCE_COMMENT_SIZE + 1];
        memset(buf, 0, sizeof(buf));
        if (fread(buf, SAUCE_COMMENT_SIZE, 1, fd) == 0) {
            free(comment);
            return;
        }
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
    return (record != NULL && record->flags.flag_ls == SAUCE_LS_9PIXEL) ? 9 : 8;
}

size_t sauce_size(sauce *record)
{
    size_t size = 0;
    if (record != NULL) {
        size += SAUCE_RECORD_SIZE;
        if (record->comments) {
            size += 5 + SAUCE_COMMENT_SIZE * record->comments;
        }
    }
    return size;
}
