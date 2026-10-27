#ifndef MUGEN_CONSOLE_H
#define MUGEN_CONSOLE_H

/*
 * The debug console (console.c; Ctrl+Shift+D / the "consoletoggle" key
 * binding): a line editor with a history, an output pane, commands looked
 * up by name in a command table (CmdTable, hashtable.c) and key bindings
 * that run commands.
 */

/* A text buffer: UTF-8, at most cap bytes (0x14 bytes). */
typedef struct ConText {
    char *buf;                  /* 0x00 cap + 1 bytes */
    int owned;                  /* 0x04 buf is freed with the text */
    int chars;                  /* 0x08 characters (UTF-8 lead bytes) */
    int len;                    /* 0x0C bytes */
    int cap;                    /* 0x10 */
} ConText;
MUGEN_ASSERT_SIZE(ConText, 0x14);

/* A position in a ConText (0x0C bytes). */
typedef struct ConCursor {
    ConText *text;              /* 0x00 */
    int chars;                  /* 0x04 character index */
    int bytes;                  /* 0x08 byte index */
} ConCursor;
MUGEN_ASSERT_SIZE(ConCursor, 0x0C);

/*
 * The commands are a CmdTable (hashtable.h: CmdTableEntry values with
 * CmdHandlerFn handlers and help texts) and the key bindings a KeybindList
 * (keybind.h). The help listing walks the command table's HashTable with:
 */
typedef struct CmdHashIter {
    HashTable *hash;            /* 0x00 */
    int bucket;                 /* 0x04 */
    HashEntry *node;            /* 0x08 */
    int visited;                /* 0x0C entries visited */
} CmdHashIter;
MUGEN_ASSERT_SIZE(CmdHashIter, 0x10);

/* The output pane: position and size, font and lines. */
typedef struct ConsoleView {
    float x;                    /* 0x00 10 */
    float y;                    /* 0x04 10 */
    float w;                    /* 0x08 video width - 20 */
    float h;                    /* 0x0C 300 */
    float line_gap;             /* 0x10 0 */
    Font *font;                 /* 0x14 font/mssansserif-tt36.def, else GameVars.font_medium */
    int font_owned;             /* 0x18 */
    int pad_1c;                 /* 0x1C never accessed by the game */
    DynArray *lines;            /* 0x20 ConText * of the output, 64 at most (the oldest go) */
} ConsoleView;
MUGEN_ASSERT_SIZE(ConsoleView, 0x24);

/* The line being edited and its cursor. */
typedef struct ConsoleInput {
    ConText *text;              /* 0x00 */
    ConCursor cursor;           /* 0x04 */
    int max;                    /* 0x10 0x200 */
} ConsoleInput;
MUGEN_ASSERT_SIZE(ConsoleInput, 0x14);

typedef struct Console {
    int open;                   /* 0x00 shown, takes the keys */
    int unused_04;              /* 0x04 1 once created, never read */
    DynArray *history;          /* 0x08 char * entered lines, [0] the line being edited */
    int history_max;            /* 0x0C 64 */
    int history_pos;            /* 0x10 */
    int lua_mode;               /* 0x14 the lines are Lua ("lexec " is put before them), "$> " prompt */
    ConsoleView view;           /* 0x18 */
    ConsoleInput input;         /* 0x3C */
    CmdTable *commands;         /* 0x50 */
    KeybindList *bindings;      /* 0x54 */
    int ticks[3];               /* 0x58 counted by Game_Tick while open; [1], [2] (cursor blink) reset on toggle and keys */
    DynArray *pending;          /* 0x64 char * commands run by Console_Tick */
    ConText *scratch;           /* 0x68 */
} Console;
MUGEN_ASSERT_OFFSET(Console, lua_mode, 0x14);
MUGEN_ASSERT_OFFSET(Console, input, 0x3C);
MUGEN_ASSERT_OFFSET(Console, commands, 0x50);
MUGEN_ASSERT_OFFSET(Console, pending, 0x64);
MUGEN_ASSERT_SIZE(Console, 0x6C);

#endif /* MUGEN_CONSOLE_H */
