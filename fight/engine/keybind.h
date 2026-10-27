#ifndef MUGEN_KEYBIND_H
#define MUGEN_KEYBIND_H

/*
 * Key bindings of the debug console ("keybind" commands, mugen.keybind*):
 * a key with modifiers bound to a console command line (0x41E480-0x41E875).
 */

/* KeyBind.mods: SDL 1.2 KMOD_* bits of the "$", "^" and "~" prefixes. */
#define KEYBIND_SHIFT   0x003
#define KEYBIND_CTRL    0x0C0
#define KEYBIND_ALT     0x300

typedef struct KeyBind {
    unsigned int mods;          /* 0x00 KEYBIND_* */
    int key;                    /* 0x04 SDL key code */
    unsigned short unicode;     /* 0x08 character; matched alone when set */
    unsigned short pad_0A;
    int valid;                  /* 0x0C parsed */
} KeyBind;
MUGEN_ASSERT_SIZE(KeyBind, 0x10);

typedef struct KeybindEntry {
    KeyBind kb;                 /* 0x00 */
    char *cmd;                  /* 0x10 malloc'ed command line */
} KeybindEntry;
MUGEN_ASSERT_SIZE(KeybindEntry, 0x14);

/* An entry of g_keybindNames: a key name and its SDL key code (NULL name ends it). */
typedef struct KeybindName {
    const char *name;           /* 0x00 */
    int key;                    /* 0x04 */
} KeybindName;
MUGEN_ASSERT_SIZE(KeybindName, 0x08);

typedef struct KeybindList {
    DynArray *entries;          /* 0x00 KeybindEntry */
} KeybindList;
MUGEN_ASSERT_SIZE(KeybindList, 0x04);

/*
 * The functions take a KeyBind by value; functions.txt cannot describe that
 * for register conventions, so they are declared with its four dwords.
 */
#define KEYBIND_ARGS(kb) (kb).mods, (kb).key, *(const int *)&(kb).unicode, (kb).valid

#endif /* MUGEN_KEYBIND_H */
