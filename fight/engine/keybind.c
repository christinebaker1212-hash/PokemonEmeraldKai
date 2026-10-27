/*
 * Key bindings of the debug console (original functions 0x41E480-0x41E875):
 * a list of keys with modifiers ("^F1", "$~a", ...) bound to command lines.
 */
#include "mugen.h"

/*
 * 0x0041E480: an empty binding list (room for 10). NULL when out of memory;
 * if only one of the inner allocations fails, the other one is leaked.
 */
KeybindList *KeybindList_Create(void)
{
    KeybindList *list = calloc(1, sizeof(KeybindList));
    DynArray *a;
    void *data;

    if (list == NULL) {
        g_outOfMemory = 1;
        return NULL;
    }
    a = calloc(1, sizeof(DynArray));
    if (a == NULL) {
        g_outOfMemory = 1;
    }
    data = calloc(10, sizeof(KeybindEntry));
    if (data == NULL) {
        g_outOfMemory = 1;
    }
    if (a != NULL && data != NULL) {
        a->data = data;
        a->capacity = 10;
        a->elem_size = sizeof(KeybindEntry);
    } else {
        a = NULL;
    }
    list->entries = a;
    if (a == NULL) {
        free(list);
        return NULL;
    }
    return list;
}

/*
 * 0x0041E510: binds cmd (copied) to the key, replacing an earlier binding
 * of it. Returns 0 when the list cannot grow. The copy is not checked.
 */
int Keybind_Add(KeybindList *list, const char *cmd, unsigned int kb_mods, int kb_key, int kb_unicode, int kb_valid)
{
    DynArray *a = list->entries;
    KeybindEntry e;
    char *copy;

    Keybind_Remove(list, kb_mods, kb_key, kb_unicode, kb_valid);
    copy = malloc(strlen(cmd) + 1);
    strcpy(copy, cmd);
    e.cmd = copy;
    e.kb.mods = kb_mods;
    e.kb.key = kb_key;
    *(int *)&e.kb.unicode = kb_unicode;
    e.kb.valid = kb_valid;
    if (a->count >= a->capacity) {
        void *p = realloc(a->data, a->elem_size * a->capacity * 2);
        if (p == NULL) {
            return 0;
        }
        a->data = p;
        a->capacity *= 2;
    }
    memcpy((char *)a->data + a->elem_size * a->count, &e, a->elem_size);
    a->count++;
    return 1;
}

/* 0x0041E7F0: e is the key of kb: same character, or same key with the same kinds of modifiers. */
int Keybind_Match(const KeyBind *e, const KeyBind *kb)
{
    if (kb->valid != e->valid) {
        return 0;
    }
    if (e->unicode != 0 && kb->unicode == e->unicode) {
        return 1;
    }
    if (((e->mods & KEYBIND_ALT) == 0) != ((kb->mods & KEYBIND_ALT) == 0)) {
        return 0;
    }
    if (((e->mods & KEYBIND_CTRL) == 0) != ((kb->mods & KEYBIND_CTRL) == 0)) {
        return 0;
    }
    if (((e->mods & KEYBIND_SHIFT) == 0) != ((kb->mods & KEYBIND_SHIFT) == 0)) {
        return 0;
    }
    return kb->key == e->key;
}

/* 0x0041E600: removes the binding of the key. Returns 1 if there was one. */
int Keybind_Remove(KeybindList *list, unsigned int kb_mods, int kb_key, int kb_unicode, int kb_valid)
{
    DynArray *a = list->entries;
    KeyBind kb;
    KeybindEntry *e = a->data;
    int i;

    kb.mods = kb_mods;
    kb.key = kb_key;
    *(int *)&kb.unicode = kb_unicode;
    kb.valid = kb_valid;
    for (i = 0; i < a->count; i++, e++) {
        if (e != NULL && Keybind_Match(&e->kb, &kb)) {
            break;
        }
    }
    if (i >= a->count) {
        return 0;
    }
    free(e->cmd);
    memmove((char *)a->data + a->elem_size * i, (char *)a->data + a->elem_size * (i + 1),
            (a->count - i - 1) * a->elem_size);
    a->count--;
    return 1;
}

/* 0x0041E690: removes every binding (the array keeps its size). */
void KeybindList_Clear(KeybindList *list)
{
    DynArray *a = list->entries;
    int i;

    for (i = 0; i < a->count; i++) {
        KeybindEntry *e = (KeybindEntry *)((char *)a->data + i * sizeof(KeybindEntry));
        if (e != NULL && e->cmd != NULL) {
            free(e->cmd);
            e->cmd = NULL;
        }
    }
    a->count = 0;
}

/* 0x0041E6E0: the command bound to the key, or NULL. */
const char *Keybind_Find(KeybindList *list, unsigned int kb_mods, int kb_key, int kb_unicode, int kb_valid)
{
    DynArray *a = list->entries;
    int n = a->count;
    KeybindEntry *data = a->data;
    KeyBind kb;
    int i;

    kb.mods = kb_mods;
    kb.key = kb_key;
    *(int *)&kb.unicode = kb_unicode;
    kb.valid = kb_valid;
    for (i = 0; i < n; i++) {
        if (&data[i] != NULL && Keybind_Match(&data[i].kb, &kb)) {
            return data[i].cmd;
        }
    }
    return NULL;
}

/*
 * 0x0041E730: parses "[$^~]*name" (shift, ctrl, alt; name from
 * g_keybindNames, case-insensitive) after leading white space into *out.
 * Returns 0 for anything else.
 */
int Keybind_Parse(const char *str, KeyBind *out)
{
    const KeybindName *k;

    memset(out, 0, sizeof(*out));
    while ((unsigned int)(*str - 1) <= 0x7E && isspace(*str)) {
        str++;
    }
    if (!isalnum(*str)) {
        do {
            switch (*str) {
            case '$':
                out->mods |= KEYBIND_SHIFT;
                break;
            case '^':
                out->mods |= KEYBIND_CTRL;
                break;
            case '~':
                out->mods |= KEYBIND_ALT;
                break;
            default:
                return 0;
            }
            str++;
        } while (!isalnum(*str));
    }
    for (k = g_keybindNames; k->name != NULL; k++) {
        if (_stricmp(str, k->name) == 0) {
            out->valid = 1;
            out->key = k->key;
            return 1;
        }
    }
    return 0;
}
