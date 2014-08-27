#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "sauce.h"
#include "piece/version.h"

const char *sauce_data_type[] = {
    "(none)",
    "character",
    "bitmap",
    "vector",
    "audio",
    "binary text",
    "extended binary",
    "archive",
    "executable"
};

int sauce_file_type_count[] = {0, 8, 14, 4, 24, 0, 0, 10, 0, 0};
const char *sauce_file_type_character[] = {
    "ASCII", "ANSi", "ANSiMation", "RIP script", "PCBoard", "Avatar", "HTML",
    "Source", "TundraDraw"
};
const char *sauce_file_type_bitmap[] = {
    "GIF", "PCX", "LBM", "TGA", "FLI", "FLC", "BMP", "GL", "DL", "WPG", "PNG",
    "JPG", "MPG", "AVI"
};
const char *sauce_file_type_vector[] = {
    "DXF", "DWG", "WPG", "3DS"
};
const char *sauce_file_type_audio[] = {
    "MOD", "669", "STM", "S3M", "MTM", "FAR", "ULT", "AMF", "DMF", "OKT",
    "ROL", "CMF", "MID", "SADT", "VOC", "WAV", "SMP8", "SMP8S", "SMP16",
    "SMP16S", "PATCH8", "PATCH16", "XM", "HSC", "IT"
};
const char *sauce_file_type_archive[] = {
    "ZIP", "ARJ", "LZH", "ARC", "TAR", "ZOO", "RAR", "UC2", "PAK", "SQZ"
};
const char **sauce_file_type[] = {
    (void *) NULL,
    (void *) sauce_file_type_character,
    (void *) sauce_file_type_bitmap,
    (void *) sauce_file_type_vector,
    (void *) sauce_file_type_audio,
    (void *) NULL,
    (void *) NULL,
    (void *) sauce_file_type_archive,
    (void *) NULL
};

void sauce_show(const char *filename)
{
    FILE *fd = NULL;
    struct stat st;
    char id[6] = "     ", date[9] = "        ";
    int16_t version = 0;
    sauce *record = NULL;

    if ((fd = fopen(filename, "rb")) == NULL) {
        fprintf(stderr, "%s: could not open file\n", filename);
        return;
    }
    if (fstat(fileno(fd), &st) < 0) {
        fprintf(stderr, "%s: stat() failed\n", filename);
        goto return_close;
    }

    if ((record = sauce_read(fd)) == NULL) {
        fprintf(stderr, "%s: no SAUCE record\n", filename);
        goto return_close;
    }

    memcpy(&id, record->id, 5);
    version = (record->version[0] - '0') * 10 + (record->version[1] - '0');
    memcpy(&date, record->date, 8);

    printf("filename.: %s\n", filename);
    printf("sauce id.: %s\n", id);
    printf("version..: %02x\n", version);
    printf("title....: %s\n", record->title);
    printf("author...: %s\n", record->author);
    printf("group....: %s\n", record->group);
    printf("date.....: %s\n", date);
    printf("file size: %d (in record)\n", record->file_size);
    printf(".........: %lu (calculated)\n", st.st_size - sauce_size(record));
    printf("data type: %d (", record->data_type);
    if (record->data_type < 9) {
        printf("%s)\n", sauce_data_type[record->data_type]);
        if (record->data_type == SAUCE_DATA_TYPE_BINARYTEXT &&
            record->file_type > 1) {
            printf("width....: %d\n", record->file_type * 2);
        } else {
            printf("file type: %d (", record->file_type);
            if (sauce_file_type_count[record->file_type] > 0 &&
                sauce_file_type[record->data_type] != NULL) {
                printf("%s)\n", sauce_file_type[record->data_type][record->file_type]);
            } else {
                printf("-)\n");
            }
        }
    } else {
        printf("unkonwn)\n");
    }

    char *tinfos[4] = {
        "tinfo",
        "tinfo",
        "tinfo",
        "tinfo",
    };

    switch (record->data_type) {
    case SAUCE_DATA_TYPE_CHARACTER:
        switch (record->file_type) {
        case SAUCE_FILE_TYPE_ASCII:
        case SAUCE_FILE_TYPE_ANSI:
        case SAUCE_FILE_TYPE_PCBOARD:
        case SAUCE_FILE_TYPE_AVATAR:
        case SAUCE_FILE_TYPE_TUNDRADRAW:
            tinfos[0] = "width";
            tinfos[1] = "lines";
            break;
        case SAUCE_FILE_TYPE_ANSIMATION:
            tinfos[0] = "width";
            tinfos[1] = "height";
            break;
        case SAUCE_FILE_TYPE_RIPSCRIPT:
            tinfos[0] = "width";
            tinfos[1] = "height";
            break;
        }
        break;
    case SAUCE_DATA_TYPE_BITMAP:
        tinfos[0] = "width";
        tinfos[1] = "height";
        tinfos[2] = "pix depth";
        break;
    case SAUCE_DATA_TYPE_AUDIO:
        if (record->file_type >= 16 && record->file_type <= 19) {
            tinfos[0] = "samp rate";
        }
        break;
    case SAUCE_DATA_TYPE_XBIN:
        tinfos[0] = "width";
        tinfos[1] = "lines";
        break;
    }

    for (int i = 0; i < 4; ++i) {
        printf("%-9s: %d\n", tinfos[i], record->tinfo[i]);
    }

    free(record);
return_close:
    fclose(fd);
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "%s [-V] <filename>[ .. <filename>]\n", argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-V")) {
            printf("piece version %s (c) Wijnand Modderman-Lenstra, https://maze.io/\n",
                   PIECE_VERSION);
            return 0;
        }
    }

    for (int i = 1; i < argc; i++) {
        sauce_show(argv[i]);
        if (i != argc - 1) {
            printf("\n");
        }
    }

    return 0;
}
