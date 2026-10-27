/*
 * The debug console (original module 0x4259F0-0x42873F, strings "List of
 * commands:" to "Too many arguments: %s"): UTF-8 text buffers and cursors,
 * the output pane, the line editor with its history, the commands
 * ("clear" to "screenshot", looked up in a hash table that is not
 * decompiled), the key bindings that run commands, drawing and the key
 * handler.
 */
#include <SDL_ttf.h>

#include "mugen.h"

/* ------------------------------------------------------------------ helpers */

/* strdup with the CRT's malloc (inlined by the original). */
static char *Con_StrDup(const char *s)
{
    size_t n = strlen(s) + 1;
    char *d = malloc(n);

    if (d != NULL) {
        memcpy(d, s, n);
    }
    return d;
}

/* Frees a text (its buffer when it owns it). */
static void Con_FreeText(ConText *t)
{
    if (t->buf != NULL && t->owned != 0) {
        free(t->buf);
    }
    free(t);
}

/* Counts the characters (UTF-8 lead bytes) and bytes of t->buf. */
static void Con_Measure(ConText *t)
{
    const unsigned char *p = (const unsigned char *)t->buf;
    int n = 0;

    t->len = (int)strlen(t->buf);
    for (; *p != 0; p++) {
        if ((*p & 0xC0) != 0x80) {
            n++;
        }
    }
    t->chars = n;
}

/* The cursor at the end of t (at its start while it is empty). */
static void Con_EndCursor(ConCursor *c, ConText *t)
{
    c->text = t;
    if (t->chars > 0) {
        c->chars = t->chars;
        c->bytes = t->len;
    } else {
        c->chars = 0;
        c->bytes = 0;
    }
}

/* Grows a pointer array (doubling it) when full; 0 when that fails. */
static int Con_Reserve(DynArray *a)
{
    if (a->count >= a->capacity) {
        void *d = realloc(a->data, a->elem_size * a->capacity * 2);
        if (d == NULL) {
            return 0;
        }
        a->data = d;
        a->capacity *= 2;
    }
    return 1;
}

/* ------------------------------------------------------------------ pointer arrays */

/*
 * 0x004259F0: drops the first line of the output (freed; the original
 * frees the first slot even when the array is empty). 0 when a line was
 * removed, 1 when the array was empty.
 */
int ConLines_RemoveFirst(DynArray *a)
{
    ConText *t = ((ConText **)a->data)[0];

    if (t != NULL) {
        Con_FreeText(t);
    }
    if (a->count > 0) {
        memmove(a->data, (char *)a->data + a->elem_size, (a->count - 1) * a->elem_size);
        a->count--;
        return 0;
    }
    return 1;
}

/*
 * 0x00425A50: appends elem (elem_size bytes of the argument); NULL when the
 * array cannot grow. The original returns the address of its argument on
 * the stack (only tested against NULL); the new element here.
 */
void *PtrArray_Push(DynArray *a, void *elem)
{
    char *p;

    if (!Con_Reserve(a)) {
        return NULL;
    }
    p = (char *)a->data + a->count * a->elem_size;
    memcpy(p, &elem, a->elem_size);
    a->count++;
    return p;
}

/* 0x00425AA0: removes element `index` (not freed). 0, or 1 for an index out of range. */
int PtrArray_RemoveAt(DynArray *a, int index)
{
    char *p;

    if (index < 0 || index >= a->count) {
        return 1;
    }
    p = (char *)a->data + index * a->elem_size;
    memmove(p, p + a->elem_size, (a->count - index - 1) * a->elem_size);
    a->count--;
    return 0;
}

/* 0x00426020: frees every element of a pointer array and empties it. */
void PtrArray_FreeAll(DynArray *a)
{
    int i;

    for (i = 0; i < a->count; i++) {
        void *p = ((void **)a->data)[i];
        if (p != NULL) {
            free(p);
        }
    }
    a->count = 0;
}

/* ------------------------------------------------------------------ texts */

/* 0x00425AE0: an empty text of at most cap bytes, or NULL. */
ConText *ConText_Create(int cap)
{
    ConText *t = calloc(1, sizeof(ConText));

    if (t == NULL) {
        g_outOfMemory = 1;
        return NULL;
    }
    t->cap = cap;
    t->buf = malloc(cap + 1);
    if (t->buf == NULL) {
        /* the structure is leaked */
        return NULL;
    }
    t->owned = 1;
    t->buf[0] = '\0';
    t->buf[cap] = '\0';
    return t;
}

/* 0x00425B30: dst becomes a copy of src (cut to dst's capacity). */
void ConText_Copy(ConText *dst, const ConText *src)
{
    dst->buf[0] = '\0';
    if ((unsigned int)(dst->cap + 1) > 1) {
        strncat(dst->buf, src->buf, dst->cap);
    }
    if (dst->cap > src->len) {
        dst->len = src->len;
        dst->chars = src->chars;
        return;
    }
    Con_Measure(dst);
}

/* 0x00425BA0: t becomes s (cut to its capacity, possibly in the middle of a character). */
void ConText_Set(ConText *t, const char *s)
{
    t->buf[0] = '\0';
    if ((unsigned int)(t->cap + 1) > 1) {
        strncat(t->buf, s, t->cap);
    }
    Con_Measure(t);
}

/* 0x00425C00: inserts src at the end of dst. Returns the cursor position (characters) it was inserted at. */
int ConText_Append(ConText *dst, const ConText *src)
{
    ConCursor c;

    Con_EndCursor(&c, dst);
    return ConCursor_Insert(&c, src);
}

/* 0x00425C40: appends the C string s to dst (through a temporary text). */
void ConText_AppendStr(const char *s, ConText *dst)
{
    int n = (int)strlen(s);
    ConText *t = calloc(1, sizeof(ConText));
    ConCursor c;

    if (t == NULL) {
        g_outOfMemory = 1;
        return;
    }
    t->cap = n;
    t->buf = malloc(n + 1);
    if (t->buf == NULL) {
        return;
    }
    t->owned = 1;
    t->buf[0] = '\0';
    t->buf[n] = '\0';
    ConText_Set(t, s);
    Con_EndCursor(&c, dst);
    ConCursor_Insert(&c, t);
    Con_FreeText(t);
}

/*
 * 0x00425D10: t becomes the UTF-16 string s encoded as UTF-8 (up to the
 * first character that does not fit).
 */
void ConText_SetUtf16(ConText *t, const unsigned short *s)
{
    ConText_Set(t, "");
    for (; *s != 0; s++) {
        unsigned int c = *s;
        unsigned char *p;
        if (c < 0x80) {
            if (t->len + 1 > t->cap) {
                break;
            }
            t->buf[t->len] = (char)c;
            t->len += 1;
        } else if (c < 0x800) {
            if (t->len + 2 > t->cap) {
                break;
            }
            p = (unsigned char *)t->buf + t->len;
            p[0] = (unsigned char)((c >> 6) | 0xC0);
            p[1] = (unsigned char)((c & 0x3F) | 0x80);
            t->len += 2;
        } else {
            if (t->len + 3 > t->cap) {
                break;
            }
            p = (unsigned char *)t->buf + t->len;
            p[0] = (unsigned char)((c >> 12) | 0xE0);
            p[1] = (unsigned char)(((c >> 6) & 0x3F) | 0x80);
            p[2] = (unsigned char)((c & 0x3F) | 0x80);
            t->len += 3;
        }
        t->chars++;
    }
    t->buf[t->len] = '\0';
}

/* ------------------------------------------------------------------ cursors */

/* 0x00425E00: moves the cursor n characters (back when negative), within the text. Returns its character index. */
int ConCursor_Move(ConCursor *c, int n)
{
    ConText *t = c->text;
    int dir = 1;

    if (n < 0) {
        n = -n;
        dir = -1;
    }
    while (n > 0) {
        if (dir == 1) {
            if (c->bytes >= t->len) {
                break;
            }
        } else if (dir == -1) {
            if (c->bytes <= 0) {
                break;
            }
        }
        c->bytes += dir;
        c->chars += dir;
        while (((unsigned char)t->buf[c->bytes] & 0xC0) == 0x80) {
            c->bytes += dir;
        }
        n--;
    }
    return c->chars;
}

/*
 * 0x00425FC0: moves the cursor to the last character boundary at or
 * before byte `bytes` (stepping forward past it, then back). Returns its
 * byte index.
 */
int ConCursor_SeekBytes(ConCursor *c, int bytes)
{
    ConText *t = c->text;

    if (bytes <= 0) {
        c->bytes = 0;
        c->chars = 0;
        bytes = 0;
    } else if (bytes >= t->len) {
        c->chars = t->chars;
        c->bytes = t->len;
    } else {
        while (c->bytes < bytes) {
            ConCursor_Move(c, 1);
        }
    }
    while (c->bytes > bytes) {
        ConCursor_Move(c, -1);
    }
    return c->bytes;
}

/* 0x00425F80: moves the cursor to character `chars` (clamped to the text). Returns its character index. */
int ConCursor_SetChars(ConCursor *c, int chars)
{
    ConText *t = c->text;

    if (chars <= 0) {
        c->bytes = 0;
        c->chars = 0;
        return 0;
    }
    if (chars >= t->chars) {
        c->chars = t->chars;
        c->bytes = t->len;
        return t->chars;
    }
    ConCursor_Move(c, chars - c->chars);
    return c->chars;
}

/*
 * 0x00425E80: inserts src at the cursor (as much of it as fits, whole
 * characters); the cursor does not move. Returns its character index.
 *
 * Quirk kept: the tail is moved when the cursor's character index (not its
 * byte index) is below the text's byte length.
 */
int ConCursor_Insert(ConCursor *c, const ConText *src)
{
    ConText *t = c->text;
    ConCursor s;
    int n;

    s.text = (ConText *)src;
    s.chars = 0;
    s.bytes = 0;
    n = ConCursor_SeekBytes(&s, t->cap - t->len);
    if (n > 0) {
        char *p = t->buf + c->bytes;
        if (c->chars < t->len) {
            memmove(p + n, p, t->len - c->bytes);
        }
        memcpy(p, src->buf, n);
        t->len += n;
        t->chars += s.chars;
        t->buf[t->len] = '\0';
    }
    return c->chars;
}

/* 0x00425F00: deletes the character at the cursor. Returns the bytes removed. */
int ConCursor_Delete(ConCursor *c)
{
    ConText *t = c->text;
    ConCursor next = *c;
    int n, chars;

    ConCursor_Move(&next, 1);
    n = next.bytes - c->bytes;
    chars = next.chars - c->chars;
    if (n <= 0) {
        return 0;
    }
    memmove(t->buf + c->bytes, t->buf + c->bytes + n, t->len - c->bytes - n);
    t->len -= n;
    t->chars -= chars;
    t->buf[t->len] = '\0';
    return n;
}

/* ------------------------------------------------------------------ output */

/* 0x00428530: empties the output pane. */
void Console_ClearLines(void)
{
    DynArray *a = g_console->view.lines;
    int i;

    for (i = 0; i < a->count; i++) {
        ConText *t = ((ConText **)a->data)[i];
        if (t != NULL) {
            Con_FreeText(t);
        }
    }
    a->count = 0;
}

/*
 * 0x004283B0: adds a line to the output pane (a copy of t, or t itself),
 * dropping the oldest when it is full. 1, or 0 when it fails.
 */
int Console_AddLine(ConText *t, int copy)
{
    DynArray *a = g_console->view.lines;
    ConText *line = t;

    if (copy) {
        line = ConText_Create(t->len);
        if (line == NULL) {
            return 0;
        }
        ConText_Copy(line, t);
    }
    if (a->count == a->capacity) {
        ConLines_RemoveFirst(a);
    }
    if (!Con_Reserve(a)) {
        return 0;
    }
    memcpy((char *)a->data + a->count * a->elem_size, &line, a->elem_size);
    a->count++;
    return 1;
}

/* 0x00428460: prints s as a line of the output pane. */
int Console_Print(const char *s)
{
    int n = (int)strlen(s);
    ConText *t = calloc(1, sizeof(ConText));

    if (t == NULL) {
        g_outOfMemory = 1;
        return 0;
    }
    t->cap = n;
    t->buf = malloc(n + 1);
    if (t->buf == NULL) {
        return 0;
    }
    t->owned = 1;
    t->buf[0] = '\0';
    t->buf[n] = '\0';
    ConText_Set(t, s);
    return Console_AddLine(t, 0);
}

/* 0x004284E0: printf into the output pane (at most 0x105F characters). */
void Console_Printf(const char *fmt, ...)
{
    char text[0x1060];
    va_list ap;

    va_start(ap, fmt);
    _vsnprintf(text, 0x105F, fmt, ap);
    va_end(ap);
    text[0x105F] = '\0';
    Console_Print(text);
}

/* ------------------------------------------------------------------ arguments */

/* 0x004285C0: s is a decimal number (digits, at most one '.', with a digit after it). */
int Str_IsNumber(const char *s)
{
    int dots = 0, digits = 0;

    if (*s == '\0') {
        return 0;
    }
    for (; *s != '\0'; s++) {
        char c = *s;
        if (c < 0) {
            return 0;
        }
        if (c == '.') {
            digits = 0;
            dots++;
        } else {
            if (!isdigit(c)) {
                return 0;
            }
            digits++;
        }
        if (dots > 1) {
            return 0;
        }
    }
    if (dots == 1 && digits == 0) {
        return 0;
    }
    return 1;
}

/* 0x00428620: the next argument as an integer ("%i"). 0, -1 when missing, -3 when not a number. */
int Con_ArgInt(char **args, int *out)
{
    char *tok = strtok(*args, " \r\n");

    if (tok == NULL) {
        Console_Printf("Missing argument.\n");
        return -1;
    }
    if (sscanf(tok, "%i", out) != 1) {
        Console_Printf("Expected integer value: %s\n", tok);
        return -3;
    }
    *args = NULL;
    return 0;
}

/* 0x00428690: the next argument as a number. 0, -1 when missing, -3 / -2 when not a number. */
int Con_ArgNumber(char **args, double *out)
{
    char *tok = strtok(*args, " \r\n");

    if (tok == NULL) {
        Console_Printf("Missing argument.\n");
        return -1;
    }
    if (!Str_IsNumber(tok)) {
        Console_Printf("Expected numeric value: %s\n", tok);
        return -3;
    }
    if (sscanf(tok, "%lf", out) != 1) {
        Console_Printf("Expected numeric value: %s\n", tok);
        return -2;
    }
    *args = NULL;
    return 0;
}

/* 0x00428710: 0 when there is no argument left, else -4 (printed). */
int Con_NoMoreArgs(char **args)
{
    char *tok = strtok(*args, " \r\n");

    if (tok == NULL) {
        return 0;
    }
    *args = NULL;
    Console_Printf("Too many arguments: %s\n", tok);
    return -4;
}

/* "Too many arguments" for the commands without arguments: 0, or -4 (inlined by every command). */
static int Con_NoArgs(char *args)
{
    char *tok = strtok(args, " \r\n");

    if (tok != NULL) {
        Console_Printf("Too many arguments: %s\n", tok);
        return -4;
    }
    return 0;
}

/* The debug commands are allowed: [Debug] Debug while the match was loaded, or AllowDebugKeys. */
static int Con_DebugAllowed(void)
{
    GameVars *g = g_game;

    return g->debug_player != 0 || g->allow_debug_keys != 0;
}

/* ------------------------------------------------------------------ the console */

/* 0x00428590: drops the queued keyboard, pad and input codes and resets the input state. */
void Console_ResetInput(void)
{
    g_keyboardQueueWrite = 0;
    g_keyboardQueueRead = 0;
    g_padQueueWrite = 0;
    g_padQueueRead = 0;
    Input_ResetState();
    g_keyQueue.write = 0;
    g_keyQueue.read = 0;
}

/*
 * 0x00427810: opens or closes the console: key repeat on while open, the
 * update and player input of the screen loop held; closing drops the
 * queued input. The cursor blink restarts.
 */
void Console_Toggle(void)
{
    Console *c = g_console;
    GameVars *g;

    c->open = c->open == 0;
    if (c->open != 0) {
        SDL_EnableKeyRepeat(0x140, 0x32);
        g = g_game;
        g->input_blocked = 1;
        g->console_open = 1;
        c = g_console;
        c->ticks[2] = 0;
        c->ticks[1] = 0;
        return;
    }
    SDL_EnableKeyRepeat(0, 0);
    g = g_game;
    g->input_blocked = 0;
    g->console_open = 0;
    g_keyboardQueueWrite = 0;
    g_keyboardQueueRead = 0;
    g_padQueueWrite = 0;
    g_padQueueRead = 0;
    Input_ResetState();
    c = g_console;
    g_keyQueue.write = 0;
    g_keyQueue.read = 0;
    c->ticks[2] = 0;
    c->ticks[1] = 0;
}

/*
 * 0x00427650: the output pane: its font (the console font, else the
 * medium font of the game), a 10-pixel margin, 300 pixels high, room for
 * 64 lines. 0 when the line array cannot be allocated (what was allocated
 * is leaked).
 */
int ConsoleView_Init(ConsoleView *v)
{
    DynArray *a;
    void *d;

    v->font = Font_Load("font/mssansserif-tt36.def", 0xC);
    if (v->font == NULL) {
        v->font = g_game->font_medium;
    } else {
        v->font_owned = 1;
    }
    v->x = 10.0f;
    v->y = 10.0f;
    v->line_gap = 0.0f;
    v->w = (float)((double)(g_videoWidth - 10) - 10.0);
    v->h = 300.0f;
    a = calloc(1, sizeof(DynArray));
    if (a == NULL) {
        g_outOfMemory = 1;
    }
    d = calloc(0x40, sizeof(ConText *));   /* Moemon Fight: pointers (4) */
    if (d == NULL) {
        g_outOfMemory = 1;
    }
    if (a != NULL && d != NULL) {
        a->data = d;
        a->capacity = 0x40;
        a->elem_size = sizeof(ConText *);   /* Moemon Fight: pointers (4 in the original) */
        v->lines = a;
        return 1;
    }
    v->lines = NULL;
    return 0;
}

/* 0x00427720: frees the lines of the pane and its font when it loaded one. */
void ConsoleView_Free(ConsoleView *v)
{
    if (v->lines != NULL) {
        DynArray *a;
        Console_ClearLines();
        a = v->lines;
        free(a->data);
        free(a);
        v->lines = NULL;
    }
    if (v->font_owned != 0 && v->font != NULL) {
        Font *f = v->font;
        if (f->sprites != NULL) {
            SpriteSet_Free(f->sprites);
        }
        if (f->ttf != NULL) {
            TTF_CloseFont(f->ttf);
        }
        free(f);
        v->font = NULL;
        v->font_owned = 0;
    }
}

/* 0x00427790: the line being edited (0x200 bytes) and its cursor. 0 when it cannot be allocated. */
int ConsoleInput_Init(ConsoleInput *in)
{
    ConText *t;

    in->max = 0x200;
    t = calloc(1, sizeof(ConText));
    if (t == NULL) {
        g_outOfMemory = 1;
    } else {
        t->cap = 0x200;
        t->buf = malloc(0x201);
        if (t->buf == NULL) {
            /* the structure is leaked */
            t = NULL;
        } else {
            t->owned = 1;
            t->buf[0] = '\0';
            t->buf[0x200] = '\0';
        }
    }
    in->text = t;
    if (t == NULL) {
        return 0;
    }
    in->cursor.text = t;
    in->cursor.chars = 0;
    in->cursor.bytes = 0;
    return 1;
}
/* ------------------------------------------------------------------ history and commands */

/*
 * 0x00428090: puts a copy of line in the history after the line being
 * edited (entry 0), dropping the oldest when it is full, then empties
 * entry 0 and goes back to it.
 */
void Console_AddHistory(const char *line)
{
    DynArray *h = g_console->history;
    char *s;

    if (h == NULL) {
        return;
    }
    s = Con_StrDup(line);
    if (s == NULL) {
        return;
    }
    if (h->count == h->capacity) {
        char *last = ((char **)h->data)[h->count - 1];
        if (last != NULL) {
            free(last);
        }
        PtrArray_RemoveAt(h, h->count - 1);
    }
    if (h->count < 1 || !Con_Reserve(h)) {
        free(s);
    } else {
        char *p = (char *)h->data + h->elem_size;
        memmove(p + h->elem_size, p, (h->count - 1) * h->elem_size);
        memcpy(p, &s, h->elem_size);
        h->count++;
    }
    g_console->history_pos = 0;
    if (((char **)h->data)[0] != NULL) {
        ((char **)h->data)[0][0] = '\0';
    }
}

/*
 * 0x004281A0: moves through the history by dir entries (clamped): the
 * line being edited replaces the entry left, the entry reached becomes
 * the line, with the cursor at its end.
 *
 * Quirk kept: the entry left is freed before its copy is made; when that
 * copy cannot be allocated the freed pointer stays in the history.
 */
void Console_HistoryMove(int dir)
{
    Console *c = g_console;
    DynArray *h = c->history;
    char *old, *s;

    if (c->history_pos + dir >= h->count) {
        dir = h->count - c->history_pos - 1;
    }
    if (c->history_pos + dir < 0) {
        dir = -c->history_pos;
    }
    if (dir == 0) {
        return;
    }
    old = ((char **)h->data)[c->history_pos];
    if (old != NULL) {
        free(old);
    }
    s = Con_StrDup(c->input.text->buf);
    if (s == NULL) {
        return;
    }
    if (c->history_pos >= 0 && c->history_pos < h->count) {
        memcpy((char *)h->data + h->elem_size * c->history_pos, &s, h->elem_size);
    }
    c->history_pos += dir;
    ConText_Set(c->input.text, ((char **)h->data)[c->history_pos]);
    ConCursor_SetChars(&c->input.cursor, c->input.text->chars);
}

/*
 * 0x004282C0: queues a command for Console_Tick; in Lua mode the lines
 * other than "lmode" and "lexec ..." become "lexec <line>".
 */
void Console_QueueCommand(const char *cmd)
{
    static const char lexec[] = "lexec ";
    Console *c = g_console;
    char *s;

    if (c->lua_mode != 0 && _stricmp(cmd, "lmode") != 0) {
        int i;
        for (i = 0; i < 6; i++) {
            if (cmd[i] != lexec[i] || cmd[i] == '\0' || lexec[i] == '\0') {
                ConText_Set(g_console->scratch, lexec);
                ConText_AppendStr(cmd, g_console->scratch);
                cmd = g_console->scratch->buf;
                break;
            }
        }
    }
    s = Con_StrDup(cmd);
    if (s != NULL && PtrArray_Push(g_console->pending, s) == NULL) {
        free(s);
    }
}

/* 0x00427C50: runs the queued commands and empties the queue. 0 when a command failed. */
int Console_Tick(void)
{
    DynArray *a = g_console->pending;
    int ok = 1;
    int i;

    for (i = 0; i < a->count; i++) {
        char *s = ((char **)a->data)[i];
        if (s != NULL && !CmdTable_Exec(s, g_console->commands)) {
            ok = 0;
        }
    }
    PtrArray_FreeAll(a);
    return ok;
}

/* ------------------------------------------------------------------ drawing and keys */

/* The text parameters of the console lines (white, unscaled). */
static void Con_TextParams(DrawParams *dp, float x, float y)
{
    memset(dp, 0, sizeof(*dp));
    dp->width_ratio = 1.0f;
    dp->width_anchor = 0.5f;
    dp->x = x;
    dp->y = y;
    dp->scale_x = 1.0f;
    dp->scale_y = 1.0f;
    dp->scale2_x = 1.0f;
    dp->scale2_y = 1.0f;
    dp->top_stretch = 1.0f;
    dp->bottom_stretch = 1.0f;
    dp->masked = 1;
    dp->alpha_src = 1.0f;
    dp->flags = 0xFFFF;
}

/*
 * 0x004278B0: the open console: a translucent dark blue pane, the prompt
 * and the line being edited at its bottom with a blinking cursor (half a
 * second on, half off), the output above it, newest first, as far as it
 * fits. The clip rectangle is reset to the whole target.
 */
void Console_Draw(void)
{
    Console *c = g_console;
    ConsoleView *v = &c->view;
    DynArray *lines = v->lines;
    ConText *scratch = c->scratch;
    const char *prompt = c->lua_mode != 0 ? "$> " : "> ";
    DrawParams dp;
    ConCursor end;
    float y_off, step;
    int i;

    if (c->open == 0) {
        return;
    }
    Gfx_SetClipRect(Ftol(v->x), Ftol(v->y), Ftol((double)v->w + v->x), Ftol((double)v->h + v->y));
    g_gfxBlendRect(Ftol(v->x), Ftol(v->y), Ftol((double)v->w + v->x), Ftol((double)v->h + v->y),
                   Video_MapRGB(0, 0, 0x20), 0xC0, 0x40);
    y_off = (float)((double)v->h - 1.0 - v->font->spacing_y);
    step = (float)((double)(v->font->size_h + v->font->spacing_y) + v->line_gap);
    ConText_Set(scratch, prompt);
    Con_EndCursor(&end, scratch);
    ConCursor_Insert(&end, c->input.text);
    Con_TextParams(&dp, (float)(v->x + 0.0), (float)(v->y + y_off));
    Font_DrawTextEx(scratch->buf, v->font, &dp, 1, 0, 0xFF, 0xFF, 0xFF);
    if (g_console->ticks[2] / 30 % 2 == 0) {
        int pos = c->input.cursor.bytes + (int)strlen(prompt);
        char saved = scratch->buf[pos];
        double y;
        int w, x;
        unsigned int color;
        scratch->buf[pos] = '\0';
        w = Font_TextWidth(scratch->buf, v->font);
        scratch->buf[pos] = saved;
        y = (double)v->y + y_off;
        x = Ftol((double)v->x + 0.0 + w);
        color = Video_MapRGB(0xFF, 0xFF, 0xFF);
        g_gfxFillRect(x, Ftol(y), x, Ftol(y - v->font->size_h + 1.0), color);
    }
    y_off = (float)(y_off - step);
    for (i = lines->count - 1; i >= 0; i--) {
        if (y_off < 0.0f) {
            break;
        }
        Con_TextParams(&dp, (float)(v->x + 0.0), (float)(v->y + y_off));
        Font_DrawTextEx(((ConText **)lines->data)[i]->buf, v->font, &dp, 1, 0, 0xFF, 0xFF, 0xFF);
        y_off = (float)(y_off - step);
    }
    Gfx_SetClipRect(0, 0, g_targetWidth, g_targetHeight);
}

/*
 * 0x00427CD0: the key handler (Keyboard_SetEventHandler): a bound key
 * queues its command (unless the console is open and the key types a
 * printable character); "consoletoggle" drops the queued input. While open:
 * Escape clears the line (closes the console when it is empty), Delete,
 * Up / Down (history), Left / Right, Home / End; Backspace, Enter (the
 * line is echoed, queued and added to the history) and printable
 * characters by their unicode value.
 *
 * Quirk kept: Delete deletes the character at the cursor, then inserts its
 * unicode value (0x7F).
 */
void Console_HandleKey(unsigned int unicode, int key, int pressed)
{
    Console *c = g_console;
    ConsoleInput *in = &c->input;
    unsigned short ch = (unsigned short)unicode;
    const char *cmd;

    cmd = Keybind_Find(c->bindings, g_keyModState, key, ch, pressed);
    if (cmd != NULL) {
        if (c->open == 0 || ch < 0x20 || ch > 0x7E) {
            Console_QueueCommand(cmd);
        }
        if (_stricmp(cmd, "consoletoggle") == 0) {
            g_keyboardQueueWrite = 0;
            g_keyboardQueueRead = 0;
            g_padQueueWrite = 0;
            g_padQueueRead = 0;
            Input_ResetState();
            g_keyQueue.write = 0;
            g_keyQueue.read = 0;
            return;
        }
        c = g_console;
    }
    if (c->open == 0) {
        return;
    }
    if (pressed != 0) {
        c->ticks[2] = 0;
        switch (key) {
        case SDLK_ESCAPE:
            if (in->text->chars > 0) {
                ConText_Set(in->text, "");
                in->cursor.text = in->text;
                in->cursor.chars = 0;
                in->cursor.bytes = 0;
                Console_ResetInput();
                return;
            }
            Console_Toggle();
            return;
        case SDLK_DELETE:
            ConCursor_Delete(&in->cursor);
            c = g_console;
            break;
        case SDLK_UP:
            Console_HistoryMove(1);
            return;
        case SDLK_DOWN:
            Console_HistoryMove(-1);
            return;
        case SDLK_RIGHT:
            ConCursor_Move(&in->cursor, 1);
            return;
        case SDLK_LEFT:
            ConCursor_Move(&in->cursor, -1);
            return;
        case SDLK_HOME:
            in->cursor.bytes = 0;
            in->cursor.chars = 0;
            return;
        case SDLK_END:
            ConCursor_SetChars(&in->cursor, in->cursor.text->chars);
            return;
        default:
            break;
        }
    }
    if (ch == 8) {
        if (in->cursor.chars > 0) {
            ConCursor_Move(&in->cursor, -1);
            ConCursor_Delete(&in->cursor);
        }
        return;
    }
    if (ch == 0xD) {
        char *line = in->text->buf;
        ConText_Set(c->scratch, c->lua_mode != 0 ? "$> " : "> ");
        ConText_Append(g_console->scratch, in->text);
        Console_AddLine(g_console->scratch, 1);
        Console_QueueCommand(line);
        Console_AddHistory(line);
        ConText_Set(in->text, "");
        in->cursor.text = in->text;
        in->cursor.chars = 0;
        in->cursor.bytes = 0;
        return;
    }
    if (ch >= 0x20) {
        unsigned short s[2];
        s[0] = ch;
        s[1] = 0;
        ConText_SetUtf16(c->scratch, s);
        ConCursor_Insert(&in->cursor, g_console->scratch);
        ConCursor_Move(&in->cursor, 1);
    }
}

/* ------------------------------------------------------------------ the commands */

/* 0x00426130: prints the commands that have a help text. */
void Console_ListCommands(void)
{
    HashTable *h = g_console->commands->table;
    CmdHashIter *it = calloc(1, sizeof(CmdHashIter));

    if (it == NULL) {
        g_outOfMemory = 1;
        return;
    }
    it->hash = h;
    Console_Printf("List of commands:\n");
    while (it->visited != it->hash->count) {
        HashEntry *node;
        CmdTableEntry *e;
        if (it->visited == 0) {
            node = it->hash->buckets[it->bucket];
        } else {
            node = it->node->next;
        }
        while (node == NULL) {
            it->bucket++;
            node = it->hash->buckets[it->bucket];
        }
        it->visited++;
        it->node = node;
        e = node->value;
        if (e != NULL && e->name[0] != '\0' && e->help != NULL) {
            Console_Printf("  %s - %s\n", e->name, e->help);
        }
    }
    free(it);
}

/* 0x00426060: "clear". */
int Con_CmdClear(CmdTable *ctx, char *name, char *args)
{
    int r = Con_NoArgs(args);

    if (r != 0) {
        return r;
    }
    Console_ClearLines();
    return 0;
}

/* 0x004260A0: "consoletoggle". */
int Con_CmdConsoleToggle(CmdTable *ctx, char *name, char *args)
{
    int r = Con_NoArgs(args);

    if (r != 0) {
        return r;
    }
    Console_Toggle();
    return 0;
}

/* 0x004260E0: "framestep": one frame while paused. */
int Con_CmdFrameStep(CmdTable *ctx, char *name, char *args)
{
    int r = Con_NoArgs(args);
    GameVars *g;

    if (r != 0) {
        return r;
    }
    g = g_game;
    if (g->defocus_paused != 0) {
        g->frame_step = 1;
        g->defocus_paused = 0;
    }
    return 0;
}

/* 0x004261F0: "help [<command>]" (only "pause" has a help text). */
int Con_CmdHelp(CmdTable *ctx, char *name, char *args)
{
    char *tok = strtok(args, " \r\n");
    char *p;

    if (tok == NULL) {
        Console_ListCommands();
        Console_Printf("For specific help, type: help <command>\n");
        return 0;
    }
    for (p = tok; *p != '\0'; p++) {
        *p = (char)tolower(*p);
    }
    if (strcmp(tok, "pause") == 0) {
        Console_Printf("pause [<status>]\n");
        Console_Printf("<status> 0 to unpause, 1 to pause.\n");
        Console_Printf("If <status> is omitted, toggles pause.\n");
    } else {
        Console_Printf("No help for %s\n", tok);
    }
    tok = strtok(NULL, " \r\n");
    if (tok != NULL) {
        Console_Printf("Too many arguments: %s\n", tok);
        return -4;
    }
    return 0;
}

/* 0x004262E0: "pause [<status>]". */
int Con_CmdPause(CmdTable *ctx, char *name, char *args)
{
    double v;
    int r;
    char *tok;

    if (args == NULL) {
        g_game->defocus_paused = g_game->defocus_paused == 0;
        return 0;
    }
    r = Con_ArgNumber(&args, &v);
    if (r != 0) {
        return r;
    }
    tok = strtok(args, " \r\n");
    if (tok != NULL) {
        Console_Printf("Too many arguments: %s\n", tok);
        return -4;
    }
    g_game->defocus_paused = v != 0.0 ? 1 : 0;
    return 0;
}

/* 0x00426390: "quit". */
int Con_CmdQuit(CmdTable *ctx, char *name, char *args)
{
    int r = Con_NoArgs(args);

    if (r != 0) {
        return r;
    }
    Console_Toggle();
    g_quitRequested = 1;
    return 0;
}

/* 0x004263D0: "lifereset" (debug). */
int Con_CmdLifeReset(CmdTable *ctx, char *name, char *args)
{
    int allowed = Con_DebugAllowed();
    int r = Con_NoArgs(args);

    if (r != 0) {
        return r;
    }
    if (allowed) {
        Debug_ResetLifePowerTime();
    }
    return 0;
}

/* 0x00426430: "roundreset" (debug, fights only): the screen loop resets the round. */
int Con_CmdRoundReset(CmdTable *ctx, char *name, char *args)
{
    int allowed = Con_DebugAllowed();
    int r = Con_NoArgs(args);

    if (r != 0) {
        return r;
    }
    if (allowed) {
        Log_Printf("Requesting round reset\n");
        if (g_game->screen_kind == 4) {
            g_game->round_reset_pending = 1;
        }
    }
    return 0;
}

/* 0x004264B0: "matchreset" (debug, fights only): the match is reloaded. */
int Con_CmdMatchReset(CmdTable *ctx, char *name, char *args)
{
    int allowed = Con_DebugAllowed();
    int r = Con_NoArgs(args);

    if (r != 0) {
        return r;
    }
    if (allowed) {
        Log_Printf("Requesting match reload\n");
        if (g_game->screen_kind == 4) {
            g_game->match_reload = 1;
        }
    }
    return 0;
}

/* 0x00426530: "clearclipboard". */
int Con_CmdClearClipboard(CmdTable *ctx, char *name, char *args)
{
    int r = Con_NoArgs(args);

    if (r != 0) {
        return r;
    }
    Debug_ClearClipboards();
    return 0;
}

/* 0x00426570: "setroundtime <ticks>" (debug): clamped to the round's start time. */
int Con_CmdSetRoundTime(CmdTable *ctx, char *name, char *args)
{
    int t, r, max;
    char *tok;

    if (!Con_DebugAllowed()) {
        return 0;
    }
    r = Con_ArgInt(&args, &t);
    if (r != 0) {
        return r;
    }
    tok = strtok(args, " \r\n");
    if (tok != NULL) {
        Console_Printf("Too many arguments: %s\n", tok);
        return -4;
    }
    if (t < 0) {
        t = 0;
    }
    max = Round_GetStartTime();
    if (t > max) {
        t = max;
    }
    g_game->round_time = t;
    return 0;
}

/* 0x00426600: "maxpower" (debug): players 1 and 2 to their maximum power. */
int Con_CmdMaxPower(CmdTable *ctx, char *name, char *args)
{
    int allowed = Con_DebugAllowed();
    int r = Con_NoArgs(args);
    GameVars *g;

    if (r != 0) {
        return r;
    }
    if (allowed) {
        g = g_game;
        Player_SetPower(g->players[1], *g->players[1]->power_max_ref);
        Player_SetPower(g->players[2], *g->players[2]->power_max_ref);
    }
    return 0;
}

/* 0x00426690: "maxpowermode" (debug): the power stays full. */
int Con_CmdMaxPowerMode(CmdTable *ctx, char *name, char *args)
{
    int allowed = Con_DebugAllowed();
    int r = Con_NoArgs(args);

    if (r != 0) {
        return r;
    }
    if (allowed) {
        GameVars *g = g_game;
        int v = g->debug_full_power == 0;
        g->debug_full_power = v;
        Log_Printf("Max power mode set to %i\n", v);
    }
    return 0;
}

/* 0x00426710: "togglelifebars" (debug). */
int Con_CmdToggleLifebars(CmdTable *ctx, char *name, char *args)
{
    int allowed = Con_DebugAllowed();
    int r = Con_NoArgs(args);

    if (r != 0) {
        return r;
    }
    if (allowed) {
        g_game->hud_enabled = g_game->hud_enabled == 0;
    }
    return 0;
}

/* 0x00426780: "showclsn" (debug): no boxes, boxes, filled boxes. */
int Con_CmdShowClsn(CmdTable *ctx, char *name, char *args)
{
    int allowed = Con_DebugAllowed();
    int r = Con_NoArgs(args);

    if (r != 0) {
        return r;
    }
    if (allowed) {
        GameVars *g = g_game;
        g->clsn_draw_mode++;
        if (g->clsn_draw_mode > 2) {
            g->clsn_draw_mode = 0;
        }
    }
    return 0;
}

/* 0x00426800: "fps". */
int Con_CmdFps(CmdTable *ctx, char *name, char *args)
{
    int r = Con_NoArgs(args);

    if (r != 0) {
        return r;
    }
    g_game->show_fps = g_game->show_fps == 0;
    return 0;
}

/* 0x00426850: "setlife <player> <life>" (debug): the player and its partner (player + 2). */
int Con_CmdSetLife(CmdTable *ctx, char *name, char *args)
{
    int n, life, r;
    Player *p;

    if (!Con_DebugAllowed()) {
        return 0;
    }
    r = Con_ArgInt(&args, &n);
    if (r != 0) {
        return r;
    }
    r = Con_ArgInt(&args, &life);
    if (r != 0) {
        return r;
    }
    r = Con_NoMoreArgs(&args);
    if (r != 0) {
        return r;
    }
    p = Player_GetActive(n);
    if (p == NULL) {
        Console_Printf("Invalid player specified.  Valid values are 1 or 2.");
        return 0;
    }
    Player_SetLife(p, life);
    p = Player_GetActive(n + 2);
    if (p != NULL) {
        Player_SetLife(p, life);
    }
    return 0;
}

/* 0x004268F0: "toggleai <player>" (debug; the argument is checked either way). */
int Con_CmdToggleAi(CmdTable *ctx, char *name, char *args)
{
    int allowed = Con_DebugAllowed();
    int n, r;
    char *tok;

    r = Con_ArgInt(&args, &n);
    if (r != 0) {
        return r;
    }
    tok = strtok(args, " \r\n");
    if (tok != NULL) {
        Console_Printf("Too many arguments: %s\n", tok);
        return -4;
    }
    if (allowed) {
        Debug_ToggleAi(n);
    }
    return 0;
}

/* 0x00426970: "toggleplayer <player>" (debug; the argument is checked either way). */
int Con_CmdTogglePlayer(CmdTable *ctx, char *name, char *args)
{
    int allowed = Con_DebugAllowed();
    int n, r;
    char *tok;

    r = Con_ArgInt(&args, &n);
    if (r != 0) {
        return r;
    }
    tok = strtok(args, " \r\n");
    if (tok != NULL) {
        Console_Printf("Too many arguments: %s\n", tok);
        return -4;
    }
    if (allowed) {
        Debug_TogglePlayer(n);
    }
    return 0;
}

/*
 * 0x004269F0: "forcestand": every character (not helper) back to its own
 * states and commands, state 0 with control and no velocity.
 *
 * Quirks kept: it works without the debug permission too (the argument is
 * only checked with it), and the velocity's pad_24 is stack garbage in the
 * original (0 here).
 */
int Con_CmdForceStand(CmdTable *ctx, char *name, char *args)
{
    GameVars *g = g_game;
    int i;

    if (g->debug_player != 0 || g->allow_debug_keys != 0) {
        char *tok = strtok(args, " \r\n");
        if (tok != NULL) {
            Console_Printf("Too many arguments: %s\n", tok);
            return -4;
        }
    }
    Console_Printf("Forcing players to stand state...");
    Log_Printf("Forcing players to stand state...\n");
    g = g_game;
    for (i = g->players_first; i <= g->players_last; i++) {
        Player *p = g->players[i];
        Coord v;
        if (p == NULL || !p->active || p->helper != 0) {
            continue;
        }
        if (p->cmds == p->custom_cmds) {
            memset(p->cmd_active, 0, sizeof(p->cmd_active));
            p->cmds = &p->data->cmds;
        }
        p->custom_states = NULL;
        p->custom_anim = NULL;
        p->custom_cmds = NULL;
        p->prev_state_no = p->state_no;
        p->state_no = 0;
        p->state_time = 0;
        p->custom_owner = -1;
        g->players[i]->ctrl = 1;
        v.x = 0.0;
        v.y = 0.0;
        v.z = 0.0;
        v.unit = 1.0;
        v.type = COORD_TYPE_2D;
        v.pad_24 = 0;
        g->players[i]->vel = v;
        g = g_game;
    }
    return 0;
}

/* 0x00426B70: "changestate <player> <state>" (debug). */
int Con_CmdChangeState(CmdTable *ctx, char *name, char *args)
{
    int n, st, r;

    if (!Con_DebugAllowed()) {
        return 0;
    }
    r = Con_ArgInt(&args, &n);
    if (r != 0) {
        return r;
    }
    r = Con_ArgInt(&args, &st);
    if (r != 0) {
        return r;
    }
    r = Con_NoMoreArgs(&args);
    if (r != 0) {
        return r;
    }
    if (Player_GetActive(n) == NULL) {
        Console_Printf("Invalid player specified");
        return 0;
    }
    Debug_SetPlayerState(n, st);
    return 0;
}

/* 0x00426C00: "screenshot": taken by the next Screen_Present. */
int Con_CmdScreenshot(CmdTable *ctx, char *name, char *args)
{
    int r = Con_NoArgs(args);

    if (r != 0) {
        return r;
    }
    g_game->screenshot_requested = 1;
    g_game->skip_draw = 0;
    return 0;
}

/* 0x00426C50: the handler of unknown commands (registered under ""). */
int Con_CmdInvalid(CmdTable *ctx, char *name, char *args)
{
    Console_Printf("Invalid command: %s\n", name);
    return 0;
}

/* ------------------------------------------------------------------ creating and freeing */

typedef struct ConKey {
    int mod;
    int key;
    const char *command;
} ConKey;

/* The default key bindings (Ctrl = 0xC0, Shift = 3, Ctrl+Alt = 0x3C0). */
static const ConKey con_keys[] = {
    { 0xC0, '`', "consoletoggle" }, { 0, ' ', "lifereset" }, { 0, SDLK_F1, "setlife 2 0" },
    { 0xC0, SDLK_F1, "setlife 1 0" }, { 0, SDLK_F2, "setlife 2 1" }, { 0xC0, SDLK_F2, "setlife 1 1" },
    { 0, SDLK_F3, "maxpower" }, { 0xC3, SDLK_F3, "maxpowermode" }, { 0, SDLK_F4, "roundreset" },
    { 3, SDLK_F4, "matchreset" }, { 0, SDLK_F5, "setroundtime 1" }, { 0, SDLK_F8, "clearclipboard" },
    { 0xC0, 'c', "showclsn" }, { 0xC0, 'l', "togglelifebars" }, { 0xC0, 'i', "forcestand" },
    { 0xC0, 't', "toggletrainingmode" }, { 0xC0, '1', "toggleai 1" }, { 0xC0, '2', "toggleai 2" },
    { 0xC0, '3', "toggleai 3" }, { 0xC0, '4', "toggleai 4" }, { 0x3C0, '1', "toggleplayer 1" },
    { 0x3C0, '2', "toggleplayer 2" }, { 0x3C0, '3', "toggleplayer 3" }, { 0x3C0, '4', "toggleplayer 4" },
};

/*
 * 0x00426C70: creates the console (once): the commands, the output pane,
 * the line editor, the command queue, the history (with an empty entry 0)
 * and the key bindings; the key handler is installed and Lua mode is on.
 * 0 when something cannot be allocated (what was created is freed).
 *
 * Quirk kept: the unicode value of the bindings is 0 in its low half and
 * stack garbage in its high half in the original (0 here); only the low
 * half is compared.
 */
int Console_Create(void)
{
    Console *c;
    CmdTable *t;
    DynArray *h;
    char *s;
    int i;

    if (g_console != NULL) {
        return 1;
    }
    c = calloc(1, sizeof(Console));
    if (c == NULL) {
        g_outOfMemory = 1;
    }
    g_console = c;
    if (c == NULL) {
        return 0;
    }
    c->commands = CmdTable_Create();
    if (g_console->commands == NULL) {
        goto fail;
    }
    g_console->scratch = ConText_Create(0x202);
    if (g_console->scratch == NULL) {
        goto fail;
    }
    t = g_console->commands;
    CmdTable_Add("clear", Con_CmdClear, "Clears the console output", t);
    CmdTable_Add("consoletoggle", Con_CmdConsoleToggle, "Toggles display of the console", t);
    CmdTable_Add("setlife", Con_CmdSetLife, "Sets a player's life to the specified value.", t);
    CmdTable_Add("maxpower", Con_CmdMaxPower, "Sets team power to max.", t);
    CmdTable_Add("maxpowermode", Con_CmdMaxPowerMode, "Toggles max (unlimited) power mode.", t);
    CmdTable_Add("framestep", Con_CmdFrameStep, "Framesteps the application if paused", t);
    CmdTable_Add("setroundtime", Con_CmdSetRoundTime, "Resets round time", t);
    CmdTable_Add("clearclipboard", Con_CmdClearClipboard, "Clears the debug output", t);
    CmdTable_Add("help", Con_CmdHelp, "Displays a list of commands", t);
    CmdTable_Add("pause", Con_CmdPause, "Pause/unpause the application", t);
    CmdTable_Add("quit", Con_CmdQuit, "Exits the application", t);
    CmdTable_Add("lifereset", Con_CmdLifeReset, "Resets life, power and time", t);
    CmdTable_Add("roundreset", Con_CmdRoundReset, "Resets the current round", t);
    CmdTable_Add("matchreset", Con_CmdMatchReset, "Reloads and resets the current match", t);
    CmdTable_Add("togglelifebars", Con_CmdToggleLifebars, "Toggles display of lifebars", t);
    CmdTable_Add("showclsn", Con_CmdShowClsn, "Cycles through display of collision boxes", t);
    CmdTable_Add("fps", Con_CmdFps, "Shows/Hides FPS counter.", t);
    CmdTable_Add("toggleai", Con_CmdToggleAi, "Turns player AI on or off", t);
    CmdTable_Add("toggleplayer", Con_CmdTogglePlayer, "Enables/Disables a specific player", t);
    CmdTable_Add("forcestand", Con_CmdForceStand, "Forces players into stand state", t);
    CmdTable_Add("changestate", Con_CmdChangeState, "Forces player n to specified state number if it exists.", t);
    CmdTable_Add("screenshot", Con_CmdScreenshot, "Takes a screenshot of the current frame.", t);
    CmdTable_Add("", Con_CmdInvalid, "", t);
    if (!ConsoleView_Init(&g_console->view)) {
        goto fail;
    }
    if (!ConsoleInput_Init(&g_console->input)) {
        goto fail;
    }
    g_console->pending = DynArray_Create(0x10, sizeof(void *));   /* Moemon Fight: pointers (4) */
    if (g_console->pending == NULL) {
        goto fail;
    }
    g_console->history_max = 0x40;
    h = DynArray_Create(0x40, sizeof(char *));   /* Moemon Fight: pointers (4) */
    g_console->history = h;
    if (h != NULL && (s = Mem_Calloc(1, 1)) != NULL) {
        /* entry 0: the line being edited */
        if (h->count < 0 || !Con_Reserve(h)) {
            free(s);
        } else {
            memmove((char *)h->data + h->elem_size, h->data, h->count * h->elem_size);
            memcpy(h->data, &s, h->elem_size);
            h->count++;
        }
    }
    g_console->bindings = KeybindList_Create();
    if (g_console->bindings == NULL) {
        goto fail;
    }
    for (i = 0; i < (int)(sizeof(con_keys) / sizeof(con_keys[0])); i++) {
        Keybind_Add(g_console->bindings, con_keys[i].command, con_keys[i].mod, con_keys[i].key, 0, 1);
    }
    Keyboard_SetEventHandler(Console_HandleKey);
    c = g_console;
    c->lua_mode = 1;
    c->unused_04 = 1;
    return 1;
fail:
    Console_Free();
    return 0;
}

/* 0x00427510: frees the console (the key handler is removed). */
void Console_Free(void)
{
    Console *c = g_console;
    KeybindList *kb;
    CmdTable *t;

    if (c == NULL) {
        return;
    }
    kb = c->bindings;
    g_keyEventHandler = NULL;
    if (kb != NULL) {
        if (kb->entries != NULL) {
            DynArray *a;
            KeybindList_Clear(kb);
            a = kb->entries;
            free(a->data);
            free(a);
            c = g_console;
            kb->entries = NULL;
        }
        free(kb);
    }
    if (c->history != NULL) {
        DynArray *a;
        PtrArray_FreeAll(c->history);
        c = g_console;
        a = c->history;
        free(a->data);
        free(a);
    }
    if (c->pending != NULL) {
        DynArray *a;
        PtrArray_FreeAll(c->pending);
        c = g_console;
        a = c->pending;
        free(a->data);
        free(a);
    }
    if (c->input.text != NULL) {
        Con_FreeText(c->input.text);
    }
    c->input.text = NULL;
    ConsoleView_Free(&c->view);
    c = g_console;
    t = c->commands;
    if (t != NULL) {
        if (t->table != NULL) {
            HashTable_Free(t->table);
            c = g_console;
            t->table = NULL;
        }
        free(t);
    }
    if (c->scratch != NULL) {
        Con_FreeText(c->scratch);
    }
    free(c);
    g_console = NULL;
}