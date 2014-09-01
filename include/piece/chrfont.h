#ifndef PIECE_CHRFONT_H
#define PIECE_CHRFONT_H

#define CHRFONT_ID      {0x50, 0x4b, 0x08, 0x08, 0x42, 0x47, 0x49, 0x20}
#define CHRFONT_OP_EOD  0b00
#define CHRFONT_OP_SCAN 0b01
#define CHRFONT_OP_MOVE 0b10
#define CHRFONT_OP_DRAW 0b11

typedef struct piece_chrfont_version_s {
    uint8_t                 major;
    uint8_t                 minor;
} piece_chrfont_version;

typedef struct piece_chrfont_header_s {
    char                    id[8];
    char                    *description;
    uint16_t                header_size;
    char                    font_name_internal[5];
    uint16_t                font_file_size;
    piece_chrfont_version   font_driver_version;
    uint8_t                 signature;
    uint16_t                characters;
    uint8_t                 first_char;
    uint16_t                stroke_offset;
    uint8_t                 scan_flag;
    uint8_t                 top_of_capital_offset;
    uint8_t                 baseline_offset;
    uint8_t                 bottom_descender_offset;
    char                    font_name[5];
    uint16_t                *character_offsets;
    uint8_t                 *character_widths;
} piece_chrfont_header;

typedef struct piece_chrfont_character_operation_s {
    uint8_t                 x   : 7;
    uint8_t                 op1 : 1;
    uint8_t                 y   : 7;
    uint8_t                 op2 : 1;
} piece_chrfont_character_operation;

typedef struct piece_chrfont_character_s {
    piece_chrfont_character_operation *operation;
    uint16_t                          operations;
} piece_chrfont_character;

typedef struct piece_chrfont_s {
    piece_chrfont_header    *header;
    piece_chrfont_character *character;
} piece_chrfont;


// Function prototypes

piece_chrfont   *piece_chrfont_read(FILE *, const char *);
void            piece_chrfont_free(piece_chrfont *);

#endif /* PIECE_CHRFONT_H */
