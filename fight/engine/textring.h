#ifndef MUGEN_TEXTRING_H
#define MUGEN_TEXTRING_H

/*
 * Fixed-size ring of text lines. Backs the error text, the debug clipboard and
 * the copy of mugen.log shown in the error dialog ("Clipboard tail").
 */
typedef struct TextRing {
    char **lines; /* 0x00 count pointers into buf */
    char *buf;    /* 0x04 count * width bytes */
    int width;    /* 0x08 bytes per line, including the terminator */
    int count;    /* 0x0C number of lines */
    int head;     /* 0x10 oldest line */
    int cur;      /* 0x14 line being written */
} TextRing;
MUGEN_ASSERT_SIZE(TextRing, 0x18);

#endif /* MUGEN_TEXTRING_H */
