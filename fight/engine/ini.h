#ifndef MUGEN_INI_H
#define MUGEN_INI_H

/* Ini_Parse modes. */
enum {
    INI_MODE_READ = 0,      /* parsed lines only */
    INI_MODE_TRACK = 1,     /* also map parsed lines to source lines */
    INI_MODE_EDIT = 2       /* also keep the source lines, for Ini_SetValuef/Ini_Save */
};

/* Source text kept for editing (INI_MODE_TRACK / INI_MODE_EDIT). */
typedef struct IniSource {
    int *line_map;          /* 0x00 parsed line index -> source line index */
    char **src_lines;       /* 0x04 INI_MODE_EDIT: source lines without '\n' */
    int src_count;          /* 0x08 */
    int src_cap;            /* 0x0C */
    char bom[4];            /* 0x10 UTF-8 byte order mark of the source, if any */
} IniSource;
MUGEN_ASSERT_SIZE(IniSource, 0x14);

/*
 * A parsed MUGEN text file (.def, .cfg, .cns, .air, ...): "[Group]" headers
 * and "key = value" lines. Comments (';') and blank lines are dropped and the
 * remaining lines are stored trimmed in one text buffer ending in "[TPEnd]".
 * A cursor walks the lines of the selected group.
 */
typedef struct Ini {
    int *refs;              /* 0x00 shared by copies; buffers are freed with the last one */
    char *text;             /* 0x04 */
    long text_size;         /* 0x08 size of the source file */
    int group_line;         /* 0x0C first line of the selected group */
    int group_offset;       /* 0x10 offset of that line in text */
    int cursor;             /* 0x14 next line to read */
    int group;              /* 0x18 selected group */
    int at_end;             /* 0x1C cursor is past the selected group */
    char lang[4];           /* 0x20 language prefix for "[lang.Group]" lookups */
    char **lines;           /* 0x24 */
    int line_count;         /* 0x28 */
    int *group_lines;       /* 0x2C line index of each group header */
    int group_count;        /* 0x30 */
    IniSource *src;         /* 0x34 NULL in INI_MODE_READ */
} Ini;
MUGEN_ASSERT_SIZE(Ini, 0x38);

#endif /* MUGEN_INI_H */
