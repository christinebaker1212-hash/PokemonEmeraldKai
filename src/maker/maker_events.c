// Moemon Maker, M2: events (docs/MOEMON_MAKER.md "Events", docs/MAKER_HANDOFF.md "Build M2a"). An event is a person's,
// sign's or trigger's script written in the Maker, kept in the map's edit file as a block:
//
//   event NAME on=person:KEY            (KEY as the person / sign / trigger lines have it; or on=add:N for an added
//     page trigger=talk flag=FLAG_X      person, sign or trigger; on=map for an Autorun-only event)
//       text "Hello!"
//       yesno "Want a POTION?"
//       if yes
//         giveitem ITEM_POTION
//       else
//         text "Suit yourself."
//       end
//     page trigger=talk self=A
//       text "Have a nice day."
//
// Pages work as in RPG Maker: the last page whose conditions all hold is the one that runs. Conditions on the page
// line: flag= noflag= (a game FLAG_ or a name of the user's own), self=A..D / noself=, var=VAR_X>=N (== != < > <= >=),
// item=ITEM_X[*N], badges=N (at least), time=morning|day|evening|night. trigger=talk|step|autorun.
//
// The commands (one a line; "if" blocks close with "end"):
//   text "..."  yesno "..."  choice "A" "B" ...  (then: if yes / if no / if choice N / if result N)
//   if flag X / if !flag X / if self A / if var VAR_X >= N / if item ITEM_X [N] / if money N / else / end
//   setflag X  clearflag X  self A on|off  setvar VAR_X N  addvar VAR_X N  subvar VAR_X N
//   giveitem ITEM_X [N]  takeitem ITEM_X [N]  givemon SPECIES_X LEVEL [ITEM_X]  money +N / money -N  heal
//   battle TRAINER_X "intro" "defeat"   (the trainer's own battle: the lines after it run once they're beaten)
//   wildbattle SPECIES_X LEVEL  music MUS_X  fanfare MUS_X  sound SE_X  wait FRAMES  fade out|in
//   move player|this|LOCALID_X up down left right face_up... jump excl wait  (then waits for the walk)
//   remove this|LOCALID_X  warp MAP_X X Y  special NAME  lock  release  face  end (stops the script here)
// Text: "\n" a new line, "\p" a new box (after A), "\l" scrolls, {PLAYER} the player's name; é as written.
//
// They're compiled at run time into the game's own script commands (MAKER_OP_*, the numbers from
// data/script_cmd_table.inc by tools/maker/gen_names.py), in buffers of their own with native pointers to their own
// text: the game's script engine runs them like any script (ScriptReadPointer reads 8 bytes on this 64-bit port).
// Self switches are Maker flags ("SELF:MAP:EVENT:A") and the user's own flag names Maker flags too, in "<save>.maker".
// A compiled event is never freed while the game runs (a running script may still point into it: decision 16, a
// running script finishes first); an edit compiles a new one.
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "event_data.h"
#include "menu.h"
#include "rtc.h"
#include "script.h"
#include "script_menu.h"
#include "strings.h"
#include "text.h"
#include "constants/event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/vars.h"
#include "maker.h"
#include "maker_api.h"
#include "constants/maker_ops.h"

void MPNet_Log(const char *fmt, ...);
void MP_AsciiToGame(u8 *dst, const char *src, int maxLen);

// ---- Output

struct Fix { u32 at; u16 label; };
struct Out
{
    u8 *b;
    u32 n, cap;
    s32 *labels;          // offset of each label (-1: not placed yet)
    u16 labelCount, labelCap;
    struct Fix *fix;
    u32 fixCount, fixCap;
    char error[160];
    int line;
};

static void Grow(struct Out *o, u32 more)
{
    if (o->n + more > o->cap)
    {
        o->cap = (o->n + more) * 2 + 64;
        o->b = realloc(o->b, o->cap);
    }
}
static void B1(struct Out *o, u8 v) { Grow(o, 1); o->b[o->n++] = v; }
static void B2(struct Out *o, u16 v) { B1(o, v & 0xFF); B1(o, v >> 8); }
static void BW(struct Out *o, u32 v) { B2(o, v & 0xFFFF); B2(o, v >> 16); }
static void BP(struct Out *o, uintptr_t v)
{
    u32 i;
    Grow(o, sizeof(uintptr_t));
    for (i = 0; i < sizeof(uintptr_t); i++)
        o->b[o->n++] = (v >> (8 * i)) & 0xFF;
}

static u16 NewLabel(struct Out *o)
{
    if (o->labelCount >= o->labelCap)
    {
        o->labelCap = o->labelCap * 2 + 16;
        o->labels = realloc(o->labels, o->labelCap * sizeof(s32));
    }
    o->labels[o->labelCount] = -1;
    return o->labelCount++;
}
static void Place(struct Out *o, u16 label) { o->labels[label] = o->n; }
static void BLabel(struct Out *o, u16 label)   // a pointer to a label, patched once the code is final
{
    if (o->fixCount >= o->fixCap)
    {
        o->fixCap = o->fixCap * 2 + 16;
        o->fix = realloc(o->fix, o->fixCap * sizeof(struct Fix));
    }
    o->fix[o->fixCount].at = o->n;
    o->fix[o->fixCount++].label = label;
    BP(o, 0);
}

static void Error(struct Out *o, const char *fmt, const char *arg)
{
    if (!o->error[0])
    {
        char msg[120];
        snprintf(msg, sizeof(msg), fmt, arg ? arg : "");
        snprintf(o->error, sizeof(o->error), "line %d: %s", o->line, msg);
    }
}

// ---- The game's text, kept for good (the script points at it)

static const u8 *GameText(const char *src)
{
    char ascii[1200];
    u8 *out;
    int n = 0, i;
    for (i = 0; src[i] && n < (int)sizeof(ascii) - 2; i++)
    {
        if (src[i] == '\\' && src[i + 1])
        {
            char c = src[++i];
            ascii[n++] = c == 'n' ? '\n' : c == 'p' ? '\f' : c == 'l' ? '\v' : c;
        }
        else
            ascii[n++] = src[i];
    }
    ascii[n] = 0;
    out = malloc(n * 2 + 16);
    {
        // {PLAYER} and \l (scroll) aren't MP_AsciiToGame's: done around it
        u8 *d = out;
        char *p = ascii, *q;
        while (*p)
        {
            char part[1200];
            int len;
            q = p;
            while (*q && *q != '\v' && strncmp(q, "{PLAYER}", 8) != 0)
                q++;
            len = q - p;
            memcpy(part, p, len);
            part[len] = 0;
            MP_AsciiToGame(d, part, len * 2 + 4);
            while (*d != EOS)
                d++;
            if (*q == '\v')
                *d++ = CHAR_PROMPT_SCROLL, q++;
            else if (*q)
                *d++ = PLACEHOLDER_BEGIN, *d++ = PLACEHOLDER_ID_PLAYER, q += 8;
            p = q;
        }
        *d = EOS;
    }
    return out;
}

// ---- Names

static bool8 NamedValue(int table, const char *name, u32 *value)
{
    unsigned v;
    if (!MakerApi_ValueOf(table, name, &v))
        return FALSE;
    *value = v;
    return TRUE;
}

static bool8 Number(const char *s, s32 *v)
{
    char *end;
    if (!s || !*s)
        return FALSE;
    *v = strtol(s, &end, 0);
    return *end == 0;
}

// A flag: the game's FLAG_ by name, else a name of the user's own (a Maker flag)
static u16 FlagOf(struct Out *o, const char *name)
{
    u32 v;
    if (!name || !*name)
    {
        Error(o, "a flag's name is missing", NULL);
        return 0;
    }
    if (strncmp(name, "FLAG_", 5) == 0)
    {
        if (NamedValue(MAKER_NAMES_FLAG, name, &v))
            return v;
        Error(o, "no flag %s", name);
        return 0;
    }
    return Maker_FlagId(name);
}

static u16 VarOf(struct Out *o, const char *name)
{
    u32 v;
    if (name && NamedValue(MAKER_NAMES_VAR, name, &v))
        return v;
    Error(o, "no variable %s (the game's VAR_ names)", name);
    return VAR_TEMP_0;
}

static u32 ValueOf(struct Out *o, int table, const char *name, const char *what)
{
    u32 v;
    s32 n;
    if (name && NamedValue(table, name, &v))
        return v;
    if (name && Number(name, &n))
        return n;
    Error(o, what, name);
    return 0;
}

// ---- The words of a line ("quoted" words keep their spaces)

#define MAX_WORDS 16
static int Words(char *line, char *w[MAX_WORDS])
{
    int n = 0;
    char *p = line;
    while (*p && n < MAX_WORDS)
    {
        while (*p == ' ' || *p == '\t')
            p++;
        if (!*p || *p == '#')
            break;
        if (*p == '"')
        {
            char *d = ++p;
            w[n++] = d;
            while (*p && *p != '"')
            {
                if (*p == '\\' && p[1] == '"')
                    p++;
                *d++ = *p++;
            }
            if (*p)
                p++;
            *d = 0;
            continue;
        }
        w[n++] = p;
        while (*p && *p != ' ' && *p != '\t')
            p++;
        if (*p)
            *p++ = 0;
    }
    return n;
}

// ---- Comparisons: "VAR_X>=3" or "VAR_X >= 3"

enum { CMP_LT, CMP_EQ, CMP_GT, CMP_LE, CMP_GE, CMP_NE };   // the script engine's conditions (goto_if)
static int Op(const char *s)
{
    static const char *const sOps[] = {"<", "==", ">", "<=", ">=", "!="};
    int i;
    if (strcmp(s, "=") == 0)
        return CMP_EQ;
    for (i = 0; i < 6; i++)
        if (strcmp(s, sOps[i]) == 0)
            return i;
    return -1;
}
static int Negate(int op)
{
    static const u8 sNot[] = {CMP_GE, CMP_NE, CMP_LE, CMP_GT, CMP_LT, CMP_EQ};
    return sNot[op];
}

// ---- Context of the event being compiled

struct Ctx
{
    struct Out o;
    const char *map, *event;
    u8 kind;           // MAKER_EVENT_ON_*: how the body starts and ends (lock / faceplayer for people)
    u16 ifStack[16][2];   // per open "if": its else label, its end label
    u8 ifDepth;
    bool8 hasElse[16];
    // M4b: `label NAME` lines (a decompiled script's own places): name, label, where it ended up
    struct { char name[64]; u16 id; s32 at; } locals[128];
    int localCount;
    u32 *ptrAt;           // where the raw lines put pointers (for the proof: compared by what they point at)
    int ptrCount, ptrCap;
    u32 codeLen;          // the last Build's length
};

static void RecordPointer(struct Ctx *c)
{
    if (c->ptrCount >= c->ptrCap)
    {
        c->ptrCap = c->ptrCap * 2 + 32;
        c->ptrAt = realloc(c->ptrAt, c->ptrCap * sizeof(u32));
    }
    c->ptrAt[c->ptrCount++] = c->o.n;
}

// ---- M4b: `raw` lines (the game's own script macros: maker_raw.c) and `label` lines
static int LocalLabel(struct Ctx *c, const char *name)
{
    int i;
    for (i = 0; i < c->localCount; i++)
        if (strcmp(c->locals[i].name, name) == 0)
            return i;
    if (c->localCount >= (int)ARRAY_COUNT(c->locals))
        return -1;
    snprintf(c->locals[c->localCount].name, sizeof(c->locals[0].name), "%s", name);
    c->locals[c->localCount].id = NewLabel(&c->o);
    c->locals[c->localCount].at = -1;
    return c->localCount++;
}
static int FindLocal(struct Ctx *c, const char *name)
{
    int i;
    for (i = 0; i < c->localCount; i++)
        if (strcmp(c->locals[i].name, name) == 0)
            return i;
    return -1;
}
static void RawByte(void *x, u8 v) { B1(&((struct Ctx *)x)->o, v); }
static void RawHalf(void *x, u16 v) { B2(&((struct Ctx *)x)->o, v); }
static void RawWord(void *x, u32 v) { BW(&((struct Ctx *)x)->o, v); }
static u32 RawHere(void *x) { return ((struct Ctx *)x)->o.n; }
static bool8 RawIsLabel(void *x, const char *name) { return FindLocal((struct Ctx *)x, name) >= 0; }
static void RawError(void *x, const char *msg) { Error(&((struct Ctx *)x)->o, "%s", msg); }
// ptrvalue ARG: a label of this event, a "text", a label of the game's scripts, or a number / constant
static bool8 RawPointer(void *x, const char *arg)
{
    struct Ctx *c = x;
    char name[600];
    const u8 *at;
    s32 v;
    int k;
    size_t n;
    snprintf(name, sizeof(name), "%s", arg);
    n = strlen(name);
    while (n && (name[n - 1] == ' ' || name[n - 1] == '\t'))
        name[--n] = 0;
    RecordPointer(c);
    if (name[0] == '"' && n >= 2 && name[n - 1] == '"')
    {
        name[n - 1] = 0;
        BP(&c->o, (uintptr_t)GameText(name + 1));
        return TRUE;
    }
    if ((k = FindLocal(c, name)) >= 0)
    {
        BLabel(&c->o, c->locals[k].id);
        return TRUE;
    }
    if ((at = Maker_RawLabel(name)) != NULL)
    {
        BP(&c->o, (uintptr_t)at);
        return TRUE;
    }
    if (Maker_RawSymbol(name, &v) || Number(name, &v))
    {
        BP(&c->o, (uintptr_t)(intptr_t)v);
        return TRUE;
    }
    // (a label of this event not seen yet: placed later)
    if ((k = LocalLabel(c, name)) >= 0 && (isalpha((unsigned char)name[0]) || name[0] == '_'))
    {
        BLabel(&c->o, c->locals[k].id);
        return TRUE;
    }
    Error(&c->o, "nothing called %s to point at", name);
    return FALSE;
}

static u16 SelfFlag(struct Ctx *c, const char *sw)
{
    char name[160];
    char letter = sw && *sw ? toupper((unsigned char)*sw) : 'A';
    if (letter < 'A' || letter > 'D')
    {
        Error(&c->o, "self switches are A, B, C and D", NULL);
        letter = 'A';
    }
    snprintf(name, sizeof(name), "SELF:%s:%s:%c", c->map, c->event, letter);
    return Maker_FlagId(name);
}

// The registry of choice lists (choice "A" "B"...): the native reads VAR_0x8004 for which
struct ChoiceList { struct MenuAction actions[8]; u8 count; };
static struct ChoiceList *sChoices;
static u16 sChoiceCount, sChoiceCap;

static void Native_Choice(void)
{
    u16 i = VarGet(VAR_0x8004);
    if (i >= sChoiceCount)
        return;
    gSpecialVar_Result = 0xFF;
    ScriptMenu_MultichoiceList(20, 2, sChoices[i].actions, sChoices[i].count, FALSE);
}

static void Native_Badges(void)
{
    u16 n = 0, f;
    for (f = FLAG_BADGE01_GET; f <= FLAG_BADGE08_GET; f++)
        n += FlagGet(f);
    gSpecialVar_Result = n;
}

static void Native_TimeOfDay(void)   // 0 morning (4-9), 1 day (10-16), 2 evening (17-19), 3 night
{
    u8 h;
    RtcCalcLocalTime();
    h = gLocalTime.hours;
    gSpecialVar_Result = h >= 4 && h < 10 ? 0 : h >= 10 && h < 17 ? 1 : h >= 17 && h < 20 ? 2 : 3;
}

static void Native_Log(void)   // tests: which page and line ran (VAR_0x8005: the line)
{
    MPNet_Log("maker event: line %d ran", VarGet(VAR_0x8005));
}

static void Callnative(struct Out *o, void (*fn)(void))
{
    B1(o, MAKER_OP_CALLNATIVE);
    BP(o, (uintptr_t)fn);
}

static void GotoIf(struct Out *o, u8 cond, u16 label)
{
    B1(o, MAKER_OP_GOTO_IF);
    B1(o, cond);
    BLabel(o, label);
}
static void Goto(struct Out *o, u16 label)
{
    B1(o, MAKER_OP_GOTO);
    BLabel(o, label);
}
static void CompareValue(struct Out *o, u16 var, u16 value)
{
    B1(o, MAKER_OP_COMPARE_VAR_TO_VALUE);
    B2(o, var);
    B2(o, value);
}
static void SetVar(struct Out *o, u16 var, u16 value)
{
    B1(o, MAKER_OP_SETVAR);
    B2(o, var);
    B2(o, value);
}

// A condition: jumps to FAIL when it doesn't hold. KEY=VALUE as a page has them
static void Condition(struct Ctx *c, const char *key, const char *value, u16 fail)
{
    struct Out *o = &c->o;
    if (strcmp(key, "flag") == 0 || strcmp(key, "noflag") == 0)
    {
        B1(o, MAKER_OP_CHECKFLAG);
        B2(o, FlagOf(o, value));
        GotoIf(o, key[0] == 'n' ? CMP_EQ : CMP_LT, fail);   // (checkflag: 1 set, 0 not)
    }
    else if (strcmp(key, "self") == 0 || strcmp(key, "noself") == 0)
    {
        B1(o, MAKER_OP_CHECKFLAG);
        B2(o, SelfFlag(c, value));
        GotoIf(o, key[0] == 'n' ? CMP_EQ : CMP_LT, fail);
    }
    else if (strcmp(key, "var") == 0)
    {
        char name[64], ops[3] = {0};
        const char *p = value;
        int n = 0, k = 0, op;
        s32 v;
        while (*p && *p != '<' && *p != '>' && *p != '=' && *p != '!' && n < 63)
            name[n++] = *p++;
        name[n] = 0;
        while ((*p == '<' || *p == '>' || *p == '=' || *p == '!') && k < 2)
            ops[k++] = *p++;
        op = Op(ops);
        if (op < 0 || !Number(p, &v))
        {
            Error(o, "var= wants VAR_X>=N (or == != < > <=)", NULL);
            return;
        }
        CompareValue(o, VarOf(o, name), v);
        GotoIf(o, Negate(op), fail);
    }
    else if (strcmp(key, "item") == 0)
    {
        char name[64];
        const char *star = strchr(value, '*');
        s32 count = 1;
        snprintf(name, sizeof(name), "%.*s", star ? (int)(star - value) : 63, value);
        if (star)
            Number(star + 1, &count);
        B1(o, MAKER_OP_CHECKITEM);
        B2(o, ValueOf(o, MAKER_NAMES_ITEM, name, "no item %s"));
        B2(o, count);
        CompareValue(o, VAR_RESULT, TRUE);
        GotoIf(o, CMP_NE, fail);
    }
    else if (strcmp(key, "badges") == 0)
    {
        s32 n = 0;
        if (!Number(value, &n))
            Error(o, "badges= wants a number", NULL);
        Callnative(o, Native_Badges);
        CompareValue(o, VAR_RESULT, n);
        GotoIf(o, CMP_LT, fail);
    }
    else if (strcmp(key, "time") == 0)
    {
        static const char *const sTimes[] = {"morning", "day", "evening", "night"};
        int t;
        for (t = 0; t < 4 && strcmp(value, sTimes[t]) != 0; t++)
            ;
        if (t == 4)
        {
            Error(o, "time= is morning, day, evening or night", NULL);
            return;
        }
        Callnative(o, Native_TimeOfDay);
        CompareValue(o, VAR_RESULT, t);
        GotoIf(o, CMP_NE, fail);
    }
    else if (strcmp(key, "trigger") != 0)
        Error(o, "a page doesn't know %s=", key);
}

// ---- Movement routes (move WHO steps...)

static u8 *MovementOf(struct Out *o, char **w, int n)
{
    static const char *const sDirs[] = {"down", "up", "left", "right"};
    u8 *m = malloc(n * 2 + 2), *d = m;
    int i, k;
    for (i = 0; i < n; i++)
    {
        const char *s = w[i];
        bool8 fast = strncmp(s, "fast_", 5) == 0, face = strncmp(s, "face_", 5) == 0;
        const char *dir = fast || face ? s + 5 : s;
        for (k = 0; k < 4 && strcmp(dir, sDirs[k]) != 0; k++)
            ;
        if (k < 4)
            *d++ = face ? MOVEMENT_ACTION_FACE_DOWN + k : fast ? MOVEMENT_ACTION_WALK_FAST_DOWN + k
                                                          : MOVEMENT_ACTION_WALK_NORMAL_DOWN + k;
        else if (strcmp(s, "jump") == 0)
            *d++ = MOVEMENT_ACTION_JUMP_IN_PLACE_DOWN;
        else if (strcmp(s, "excl") == 0)
            *d++ = MOVEMENT_ACTION_EMOTE_EXCLAMATION_MARK;
        else if (strcmp(s, "wait") == 0)
            *d++ = MOVEMENT_ACTION_DELAY_16;
        else
            Error(o, "a walk doesn't know %s", s);
    }
    *d = MOVEMENT_ACTION_STEP_END;
    return m;
}

static const char *sMapName;   // (the map of the event being compiled)

static u16 ObjectOf(struct Out *o, const char *who)
{
    s32 v;
    if (!who || strcmp(who, "this") == 0)
        return VAR_LAST_TALKED;   // (applymovement / removeobject read a variable here)
    if (strcmp(who, "player") == 0)
        return OBJ_EVENT_ID_PLAYER;
    if (Number(who, &v))
        return v;
    if (strncmp(who, "LOCALID_", 8) == 0)
    {
        // the map's own people by their LOCALID_
        int map = MakerApi_FindMap(sMapName), i;
        for (i = 0; map >= 0 && i < MakerApi_ObjectCount(map); i++)
        {
            struct MakerObjectInfo info;
            if (MakerApi_ObjectInfo(map, i, &info) && info.localIdName && strcmp(info.localIdName, who) == 0)
                return info.localId;
        }
    }
    Error(o, "no one called %s here (player, this, or a person's LOCALID_)", who);
    return VAR_LAST_TALKED;
}

// ---- One command

static void Command(struct Ctx *c, char *line)
{
    struct Out *o = &c->o;
    char *w[MAX_WORDS];
    int n;
    const char *cmd;
    s32 v;
    {
        const char *t = line;
        while (*t == ' ' || *t == '\t')
            t++;
        if (strncmp(t, "raw ", 4) == 0 || strncmp(t, "raw\t", 4) == 0)   // the game's own macros, as they are
        {
            struct MakerRawSink sink = {c, RawByte, RawHalf, RawWord, RawPointer, RawHere, RawError, RawIsLabel};
            if (!Maker_RawLine(&sink, t + 4) && !o->error[0])
                Error(o, "raw: %s isn't understood", t + 4);
            return;
        }
        if (strncmp(t, "label ", 6) == 0)   // a place in this event the raw lines can point at
        {
            char name[64];
            int k;
            snprintf(name, sizeof(name), "%s", t + 6);
            for (k = (int)strlen(name); k > 0 && (name[k - 1] == ' ' || name[k - 1] == '\t'); k--)
                name[k - 1] = 0;
            if ((k = LocalLabel(c, name)) < 0)
                Error(o, "too many labels", NULL);
            else if (o->labels[c->locals[k].id] >= 0)
                Error(o, "the label %s is there twice", name);
            else
                Place(o, c->locals[k].id);
            return;
        }
    }
    n = Words(line, w);
    if (n == 0)
        return;
    cmd = w[0];
    if (strcmp(cmd, "text") == 0 || strcmp(cmd, "yesno") == 0)
    {
        if (n < 2)
        {
            Error(o, "%s wants its words in quotes", cmd);
            return;
        }
        B1(o, MAKER_OP_LOAD_WORD);
        B1(o, 0);
        BP(o, (uintptr_t)GameText(w[1]));
        B1(o, MAKER_OP_CALL_STD);
        B1(o, cmd[0] == 'y' ? 5 : 4);   // MSGBOX_YESNO / MSGBOX_DEFAULT
    }
    else if (strcmp(cmd, "choice") == 0)
    {
        struct ChoiceList *l;
        int i;
        if (n < 3 || n > 9)
        {
            Error(o, "choice wants 2 to 8 answers in quotes", NULL);
            return;
        }
        if (sChoiceCount >= sChoiceCap)
        {
            sChoiceCap = sChoiceCap * 2 + 8;
            sChoices = realloc(sChoices, sChoiceCap * sizeof(*sChoices));
        }
        l = &sChoices[sChoiceCount];
        memset(l, 0, sizeof(*l));
        for (i = 1; i < n; i++)
            l->actions[i - 1].text = GameText(w[i]);
        l->count = n - 1;
        SetVar(o, VAR_0x8004, sChoiceCount++);
        Callnative(o, Native_Choice);
        B1(o, MAKER_OP_WAITSTATE);
    }
    else if (strcmp(cmd, "if") == 0)
    {
        u16 elseL = NewLabel(o), endL = NewLabel(o);
        const char *what = n > 1 ? w[1] : "";
        if (c->ifDepth >= 16)
        {
            Error(o, "too many ifs inside each other", NULL);
            return;
        }
        if (strcmp(what, "yes") == 0 || strcmp(what, "no") == 0)
        {
            CompareValue(o, VAR_RESULT, what[0] == 'y');
            GotoIf(o, CMP_NE, elseL);
        }
        else if ((strcmp(what, "choice") == 0 || strcmp(what, "result") == 0) && n > 2 && Number(w[2], &v))
        {
            CompareValue(o, VAR_RESULT, strcmp(what, "choice") == 0 ? v - 1 : v);   // (choice 1 is the first)
            GotoIf(o, CMP_NE, elseL);
        }
        else if ((strcmp(what, "flag") == 0 || strcmp(what, "!flag") == 0) && n > 2)
            Condition(c, what[0] == '!' ? "noflag" : "flag", w[2], elseL);
        else if ((strcmp(what, "self") == 0 || strcmp(what, "!self") == 0) && n > 2)
            Condition(c, what[0] == '!' ? "noself" : "self", w[2], elseL);
        else if (strcmp(what, "var") == 0 && n > 4 && Op(w[3]) >= 0 && Number(w[4], &v))
        {
            CompareValue(o, VarOf(o, w[2]), v);
            GotoIf(o, Negate(Op(w[3])), elseL);
        }
        else if (strcmp(what, "item") == 0 && n > 2)
        {
            char cond[80];
            snprintf(cond, sizeof(cond), "%s*%s", w[2], n > 3 ? w[3] : "1");
            Condition(c, "item", cond, elseL);
        }
        else if (strcmp(what, "money") == 0 && n > 2 && Number(w[2], &v))
        {
            B1(o, MAKER_OP_CHECKMONEY);
            BP(o, v);
            B1(o, 0);
            CompareValue(o, VAR_RESULT, TRUE);
            GotoIf(o, CMP_NE, elseL);
        }
        else
            Error(o, "if wants yes, no, choice N, flag X, !flag X, self A, var VAR_X >= N, item ITEM_X or money N", NULL);
        c->ifStack[c->ifDepth][0] = elseL;
        c->ifStack[c->ifDepth][1] = endL;
        c->hasElse[c->ifDepth] = FALSE;
        c->ifDepth++;
    }
    else if (strcmp(cmd, "else") == 0)
    {
        if (!c->ifDepth || c->hasElse[c->ifDepth - 1])
        {
            Error(o, "else without its if", NULL);
            return;
        }
        Goto(o, c->ifStack[c->ifDepth - 1][1]);
        Place(o, c->ifStack[c->ifDepth - 1][0]);
        c->hasElse[c->ifDepth - 1] = TRUE;
    }
    else if (strcmp(cmd, "end") == 0 && c->ifDepth)
    {
        c->ifDepth--;
        if (!c->hasElse[c->ifDepth])
            Place(o, c->ifStack[c->ifDepth][0]);
        Place(o, c->ifStack[c->ifDepth][1]);
    }
    else if (strcmp(cmd, "end") == 0 || strcmp(cmd, "stop") == 0)
    {
        B1(o, c->kind == MAKER_EVENT_ON_PERSON ? MAKER_OP_RELEASE : MAKER_OP_RELEASEALL);
        B1(o, MAKER_OP_END);
    }
    else if ((strcmp(cmd, "setflag") == 0 || strcmp(cmd, "clearflag") == 0) && n > 1)
    {
        B1(o, cmd[0] == 's' ? MAKER_OP_SETFLAG : MAKER_OP_CLEARFLAG);
        B2(o, FlagOf(o, w[1]));
    }
    else if (strcmp(cmd, "self") == 0 && n > 2)
    {
        B1(o, strcmp(w[2], "off") == 0 ? MAKER_OP_CLEARFLAG : MAKER_OP_SETFLAG);
        B2(o, SelfFlag(c, w[1]));
    }
    else if ((strcmp(cmd, "setvar") == 0 || strcmp(cmd, "addvar") == 0 || strcmp(cmd, "subvar") == 0) && n > 2
          && Number(w[2], &v))
    {
        B1(o, cmd[0] == 's' && cmd[1] == 'e' ? MAKER_OP_SETVAR : cmd[0] == 'a' ? MAKER_OP_ADDVAR : MAKER_OP_SUBVAR);
        B2(o, VarOf(o, w[1]));
        B2(o, v);
    }
    else if (strcmp(cmd, "giveitem") == 0 && n > 1)
    {
        s32 count = 1;
        if (n > 2)
            Number(w[2], &count);
        B1(o, MAKER_OP_SETORCOPYVAR);
        B2(o, VAR_0x8000);
        B2(o, ValueOf(o, MAKER_NAMES_ITEM, w[1], "no item %s"));
        B1(o, MAKER_OP_SETORCOPYVAR);
        B2(o, VAR_0x8001);
        B2(o, count);
        B1(o, MAKER_OP_CALL_STD);
        B1(o, 0);   // STD_OBTAIN_ITEM
    }
    else if (strcmp(cmd, "takeitem") == 0 && n > 1)
    {
        s32 count = 1;
        if (n > 2)
            Number(w[2], &count);
        B1(o, MAKER_OP_REMOVEITEM);
        B2(o, ValueOf(o, MAKER_NAMES_ITEM, w[1], "no item %s"));
        B2(o, count);
    }
    else if (strcmp(cmd, "givemon") == 0 && n > 2 && Number(w[2], &v))
    {
        B1(o, MAKER_OP_GIVEMON);
        B2(o, ValueOf(o, MAKER_NAMES_SPECIES, w[1], "no species %s"));
        B1(o, v);
        B2(o, n > 3 ? ValueOf(o, MAKER_NAMES_ITEM, w[3], "no item %s") : 0);
        BW(o, 0);
        BW(o, 0);
        B1(o, 0);
    }
    else if (strcmp(cmd, "money") == 0 && n > 1 && Number(w[1] + (w[1][0] == '+'), &v))
    {
        B1(o, v < 0 ? MAKER_OP_REMOVEMONEY : MAKER_OP_ADDMONEY);
        BP(o, v < 0 ? -v : v);
        B1(o, 0);
    }
    else if (strcmp(cmd, "heal") == 0)
    {
        int s = Maker_SpecialIndex("HealPlayerParty", NULL);
        B1(o, MAKER_OP_SPECIAL);
        B2(o, s < 0 ? 0 : s);
    }
    else if (strcmp(cmd, "special") == 0 && n > 1)
    {
        bool8 waits = FALSE;
        int s = Maker_SpecialIndex(w[1], &waits);
        if (s < 0)
        {
            Error(o, "no special %s", w[1]);
            return;
        }
        B1(o, MAKER_OP_SPECIAL);
        B2(o, s);
        if (waits)
            B1(o, MAKER_OP_WAITSTATE);
    }
    else if (strcmp(cmd, "battle") == 0 && n > 3)
    {
        B1(o, MAKER_OP_TRAINERBATTLE);
        B1(o, 0);   // TRAINER_BATTLE_SINGLE
        B2(o, ValueOf(o, MAKER_NAMES_TRAINER, w[1], "no trainer %s"));
        B2(o, LOCALID_NONE);
        BP(o, (uintptr_t)GameText(w[2]));
        BP(o, (uintptr_t)GameText(w[3]));
    }
    else if (strcmp(cmd, "wildbattle") == 0 && n > 2 && Number(w[2], &v))
    {
        B1(o, MAKER_OP_SETWILDBATTLE);
        B2(o, ValueOf(o, MAKER_NAMES_SPECIES, w[1], "no species %s"));
        B1(o, v);
        B2(o, 0);
        B1(o, MAKER_OP_DOWILDBATTLE);
    }
    else if ((strcmp(cmd, "music") == 0 || strcmp(cmd, "fanfare") == 0 || strcmp(cmd, "sound") == 0) && n > 1)
    {
        int song = MakerApi_SongId(w[1]);
        if (song < 0)
        {
            Error(o, "no song %s", w[1]);
            return;
        }
        if (cmd[0] == 'm')
        {
            B1(o, MAKER_OP_PLAYBGM);
            B2(o, song);
            B1(o, 0);
        }
        else if (cmd[0] == 'f')
        {
            B1(o, MAKER_OP_PLAYFANFARE);
            B2(o, song);
            B1(o, MAKER_OP_WAITFANFARE);
        }
        else
        {
            B1(o, MAKER_OP_PLAYSE);
            B2(o, song);
        }
    }
    else if (strcmp(cmd, "wait") == 0 && n > 1 && Number(w[1], &v))
    {
        B1(o, MAKER_OP_DELAY);
        B2(o, v);
    }
    else if (strcmp(cmd, "fade") == 0 && n > 1)
    {
        B1(o, MAKER_OP_FADESCREEN);
        B1(o, strcmp(w[1], "out") == 0 ? 1 : 0);   // FADE_TO_BLACK / FADE_FROM_BLACK
    }
    else if (strcmp(cmd, "move") == 0 && n > 2)
    {
        u16 who = ObjectOf(o, w[1]);
        B1(o, MAKER_OP_APPLYMOVEMENT);
        B2(o, who);
        BP(o, (uintptr_t)MovementOf(o, w + 2, n - 2));
        B1(o, MAKER_OP_WAITMOVEMENT);
        B2(o, 0);
    }
    else if (strcmp(cmd, "remove") == 0)
    {
        B1(o, MAKER_OP_REMOVEOBJECT);
        B2(o, ObjectOf(o, n > 1 ? w[1] : "this"));
    }
    else if (strcmp(cmd, "warp") == 0 && n > 3)
    {
        int map = MakerApi_FindMap(w[1]);
        struct MakerMapInfo info;
        s32 x, y;
        if (map < 0 || !MakerApi_MapInfo(map, &info) || !Number(w[2], &x) || !Number(w[3], &y))
        {
            Error(o, "warp wants MAP_X X Y", NULL);
            return;
        }
        B1(o, MAKER_OP_WARP);
        B1(o, info.mapGroup);
        B1(o, info.mapNum);
        B1(o, 0xFF);   // WARP_ID_NONE
        B2(o, x);
        B2(o, y);
        B1(o, MAKER_OP_WAITSTATE);
    }
    else if (strcmp(cmd, "lock") == 0)
        B1(o, c->kind == MAKER_EVENT_ON_PERSON ? MAKER_OP_LOCK : MAKER_OP_LOCKALL);
    else if (strcmp(cmd, "release") == 0)
        B1(o, c->kind == MAKER_EVENT_ON_PERSON ? MAKER_OP_RELEASE : MAKER_OP_RELEASEALL);
    else if (strcmp(cmd, "face") == 0)
        B1(o, MAKER_OP_FACEPLAYER);
    else if (strcmp(cmd, "log") == 0 && n > 1 && Number(w[1], &v))   // (tests)
    {
        SetVar(o, VAR_0x8005, v);
        Callnative(o, Native_Log);
    }
    else
        Error(o, "don't know \"%s\" (or something it needs is missing)", cmd);
}

// ---- A whole event

struct Page
{
    char *line;        // the page line
    char **cmds;
    int cmdCount, firstLine;
    u8 trigger;
};

enum { TRIGGER_TALK, TRIGGER_STEP, TRIGGER_AUTORUN };

static u8 TriggerOf(const char *pageLine)
{
    const char *t = strstr(pageLine, "trigger=");
    if (!t)
        return TRIGGER_TALK;
    t += 8;
    return strncmp(t, "autorun", 7) == 0 ? TRIGGER_AUTORUN : strncmp(t, "step", 4) == 0 ? TRIGGER_STEP : TRIGGER_TALK;
}

struct Compiled
{
    struct Compiled *next;
    char map[48], event[48];
    char *source;
    u8 kind, pageCount;
    const u8 *script;       // the entry: the last page whose conditions hold, then its commands
    const u8 *selector;     // autorun: run at once, leaves in VAR_RESULT the autorun page to start (0xFF: none)
    const u8 *bodies[32];   // each page's commands
    char error[160];
    const u8 **lineAt;      // (M4's debugger) where each command line's code starts, and its line in the event's text
    u16 *lineNo;
    int lineCount;
    char (*labelName)[64];  // (M4b) the event's `label` lines and where each is in script
    const u8 **labelAt;
    int labelCount;
    u32 *ptrAt;             // (M4b) where its raw lines put pointers, and the script's length
    int ptrCount;
    u32 scriptLen;
};
static struct Compiled *sCompiled;

// SELECTOR: only which page (VAR_RESULT; autorun pages only, 0xFF for none) and no commands; else the entry and every
// page's commands (BODIES: where each starts)
// (M4's debugger: each command line's code offset and its line, while a script is built)
static u32 *sLineOff;
static u16 *sLineNum;
static int sLineCount, sLineCap;

static const u8 *Build(struct Ctx *c, struct Page *pages, int pageCount, bool8 selector, const u8 **bodiesOut)
{
    struct Out *o = &c->o;
    u16 *bodies = calloc(pageCount + 1, sizeof(u16));
    u16 none = NewLabel(o);
    int p, k;
    u8 *code;
    for (p = 0; p < pageCount; p++)
        bodies[p] = NewLabel(o);
    // which page: the last one whose conditions hold
    for (p = pageCount - 1; p >= 0; p--)
    {
        u16 next = NewLabel(o);
        char line[400], *w[MAX_WORDS];
        int n, i;
        snprintf(line, sizeof(line), "%s", pages[p].line);
        n = Words(line, w);
        o->line = pages[p].firstLine;
        for (i = 1; i < n; i++)
        {
            char *eq = strchr(w[i], '=');
            if (!eq)
            {
                Error(o, "page conditions are KEY=VALUE (%s)", w[i]);
                continue;
            }
            *eq = 0;
            Condition(c, w[i], eq + 1, next);
        }
        if (selector)
        {
            SetVar(o, VAR_RESULT, pages[p].trigger == TRIGGER_AUTORUN ? p : 0xFF);
            B1(o, MAKER_OP_END);
        }
        else   // (an autorun page isn't talked to: talking finds nothing to do)
            Goto(o, pages[p].trigger == TRIGGER_AUTORUN ? none : bodies[p]);
        Place(o, next);
    }
    Place(o, none);
    if (selector)
        SetVar(o, VAR_RESULT, 0xFF);
    B1(o, MAKER_OP_END);
    // the pages' commands
    for (p = 0; p < pageCount && !selector; p++)
    {
        bool8 person = c->kind == MAKER_EVENT_ON_PERSON && pages[p].trigger == TRIGGER_TALK;
        // a page that starts with a label is a script as the game has it (decompiled): no lock / release added
        bool8 rawPage = pages[p].cmdCount && strncmp(pages[p].cmds[0], "label ", 6) == 0;
        Place(o, bodies[p]);
        if (!rawPage)
            B1(o, person ? MAKER_OP_LOCK : MAKER_OP_LOCKALL);
        if (person && !rawPage)
            B1(o, MAKER_OP_FACEPLAYER);
        c->ifDepth = 0;
        for (k = 0; k < pages[p].cmdCount; k++)
        {
            char line[600];
            o->line = pages[p].firstLine + 1 + k;
            snprintf(line, sizeof(line), "%s", pages[p].cmds[k]);
            if (sLineCount == sLineCap)
            {
                sLineCap = sLineCap * 2 + 32;
                sLineOff = realloc(sLineOff, sLineCap * sizeof(*sLineOff));
                sLineNum = realloc(sLineNum, sLineCap * sizeof(*sLineNum));
            }
            sLineOff[sLineCount] = o->n;
            sLineNum[sLineCount++] = o->line;
            Command(c, line);
        }
        if (c->ifDepth)
            Error(o, "an if has no end", NULL);
        if (!rawPage)
        {
            B1(o, person ? MAKER_OP_RELEASE : MAKER_OP_RELEASEALL);
            B1(o, MAKER_OP_END);
        }
    }
    // final: the labels' pointers (a name never placed first: it says which, before the jump that has nowhere to go)
    for (k = 0; k < c->localCount; k++)
    {
        c->locals[k].at = o->labels[c->locals[k].id];
        if (c->locals[k].at < 0 && !selector)
            Error(o, "no label or constant %s", c->locals[k].name);
    }
    c->codeLen = o->n;
    code = malloc(o->n);
    memcpy(code, o->b, o->n);
    for (k = 0; k < (int)o->fixCount; k++)
    {
        uintptr_t at = o->labels[o->fix[k].label] < 0 ? 0 : (uintptr_t)(code + o->labels[o->fix[k].label]);
        u32 i;
        if (!at)
            Error(o, "internal: a jump with nowhere to go", NULL);
        for (i = 0; i < sizeof(uintptr_t); i++)
            code[o->fix[k].at + i] = (at >> (8 * i)) & 0xFF;
    }
    if (bodiesOut)
        for (p = 0; p < pageCount; p++)
            bodiesOut[p] = code + o->labels[bodies[p]];
    free(bodies);
    free(o->b);
    free(o->labels);
    free(o->fix);
    o->b = NULL, o->labels = NULL, o->fix = NULL;
    o->n = o->cap = o->fixCount = o->fixCap = 0;
    o->labelCount = o->labelCap = 0;
    return code;
}

static struct Compiled *Compile(const char *map, const char *event, u8 kind, const char *source)
{
    struct Compiled *ev = calloc(1, sizeof(*ev));
    struct Ctx c;
    struct Page pages[32];
    int pageCount = 0, lineNo = 0, p;
    char *text = strdup(source), *line, *save = NULL;
    bool8 anyAutorun = FALSE;
    snprintf(ev->map, sizeof(ev->map), "%s", map);
    snprintf(ev->event, sizeof(ev->event), "%s", event);
    ev->source = strdup(source);
    ev->kind = kind;
    memset(&c, 0, sizeof(c));
    memset(pages, 0, sizeof(pages));
    c.map = map, c.event = event, c.kind = kind;
    sMapName = map;
    for (line = strtok_r(text, "\n", &save); line; line = strtok_r(NULL, "\n", &save))
    {
        char *t = line;
        lineNo++;
        while (*t == ' ' || *t == '\t')
            t++;
        if (!*t || *t == '#')
            continue;
        if (strncmp(t, "page", 4) == 0 && (t[4] == 0 || t[4] == ' '))
        {
            if (pageCount >= 32)
                break;
            pages[pageCount].line = t;
            pages[pageCount].firstLine = lineNo;
            pages[pageCount].trigger = TriggerOf(t);
            anyAutorun |= pages[pageCount].trigger == TRIGGER_AUTORUN;
            pageCount++;
            continue;
        }
        if (!pageCount)   // (commands before any page: a page of their own, talk)
        {
            pages[0].line = (char *)"page";
            pages[0].firstLine = lineNo - 1;
            pageCount = 1;
        }
        {
            struct Page *pg = &pages[pageCount - 1];
            pg->cmds = realloc(pg->cmds, (pg->cmdCount + 1) * sizeof(char *));
            pg->cmds[pg->cmdCount++] = t;
        }
    }
    ev->pageCount = pageCount;
    if (pageCount)
    {
        int i;
        sLineCount = 0;
        Maker_RawReset();
        ev->script = Build(&c, pages, pageCount, FALSE, ev->bodies);
        ev->scriptLen = c.codeLen;
        ev->ptrAt = c.ptrAt, ev->ptrCount = c.ptrCount;
        c.ptrAt = NULL;
        ev->labelCount = c.localCount;
        ev->labelName = malloc(sizeof(*ev->labelName) * (c.localCount + 1));
        ev->labelAt = malloc(sizeof(*ev->labelAt) * (c.localCount + 1));
        for (i = 0; i < c.localCount; i++)
        {
            memcpy(ev->labelName[i], c.locals[i].name, sizeof(ev->labelName[i]));
            ev->labelAt[i] = c.locals[i].at >= 0 ? ev->script + c.locals[i].at : NULL;
        }
        ev->lineAt = malloc(sizeof(*ev->lineAt) * (sLineCount + 1));
        ev->lineNo = malloc(sizeof(*ev->lineNo) * (sLineCount + 1));
        for (i = 0; i < sLineCount; i++)
            ev->lineAt[i] = ev->script + sLineOff[i], ev->lineNo[i] = sLineNum[i];
        ev->lineCount = sLineCount;
        if (anyAutorun && !c.o.error[0])
            ev->selector = Build(&c, pages, pageCount, TRUE, NULL);
    }
    snprintf(ev->error, sizeof(ev->error), "%s", c.o.error);
    for (p = 0; p < pageCount; p++)
        free(pages[p].cmds);
    free(text);
    if (ev->error[0])
        MPNet_Log("maker event: %s %s: %s", map, event, ev->error);
    else
        MPNet_Log("maker event: %s %s compiled (%d page%s)", map, event, pageCount, pageCount == 1 ? "" : "s");
    ev->next = sCompiled;
    sCompiled = ev;
    return ev;
}

static struct Compiled *Find(const char *map, const char *event, u8 kind, const char *source)
{
    struct Compiled *ev;
    for (ev = sCompiled; ev; ev = ev->next)
        if (strcmp(ev->map, map) == 0 && strcmp(ev->event, event) == 0 && ev->kind == kind
         && strcmp(ev->source, source) == 0)
            return ev;
    return Compile(map, event, kind, source);
}

// (M4b) the proof of the decompiler (decision 19): every block of PATH ("@ LABEL" then its lines, from
// tools/maker/decompile.py --proof) compiled as an event and walked beside the game's own script at LABEL: the same
// bytes, a pointer into the event the same place as the game's label of that name, any other pointer the same address
int Maker_RawProof(const char *path)
{
    FILE *f = fopen(path, "rb");
    char *all, *p, *name = NULL, *body = NULL;
    long size;
    int blocks = 0, same = 0, shown = 0;
    if (!f)
    {
        MPNet_Log("maker proof: can't read %s", path);
        return 0;
    }
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    all = malloc(size + 2);
    size = (long)fread(all, 1, size, f);
    fclose(f);
    all[size] = 0;
    all[size + 1] = 0;
    for (p = all; ; )
    {
        char *next = strstr(p, "\n@ "), *nl;
        if (strncmp(p, "@ ", 2) != 0)
        {
            if (!next)
                break;
            p = next + 1;
            continue;
        }
        if (next)
            *next = 0;
        nl = strchr(p, '\n');
        name = p + 2;
        body = nl ? nl + 1 : p + strlen(p);
        if (nl)
            *nl = 0;
        {
            char *src = malloc(strlen(body) + strlen(name) + 16);
            struct Compiled *ev;
            const u8 *orig = Maker_RawLabel(name), *code;
            const char *why = NULL;
            u32 i, k, len, at = 0;
            sprintf(src, "label %s\n%s", name, body);
            ev = Compile("PROOF", name, MAKER_EVENT_ON_MAP, src);
            blocks++;
            code = ev->labelCount ? ev->labelAt[0] : NULL;
            if (ev->error[0])
                why = ev->error;
            else if (!orig || !code)
                why = "no such label in the game";
            else
            {
                u32 start = (u32)(code - ev->script);
                len = ev->scriptLen - start;
                for (i = 0; i < len && !why; )
                {
                    bool8 isPtr = FALSE;
                    for (k = 0; k < (u32)ev->ptrCount; k++)
                        if (ev->ptrAt[k] == start + i)
                            isPtr = TRUE;
                    if (isPtr)
                    {
                        uintptr_t pc = 0, po = 0;
                        int b;
                        for (b = 0; b < (int)sizeof(uintptr_t); b++)
                            pc |= (uintptr_t)code[i + b] << (8 * b), po |= (uintptr_t)orig[i + b] << (8 * b);
                        if (pc >= (uintptr_t)ev->script && pc < (uintptr_t)ev->script + ev->scriptLen)
                        {
                            int j;
                            for (j = 0; j < ev->labelCount; j++)
                                if ((uintptr_t)ev->labelAt[j] == pc)
                                    break;
                            if (j == ev->labelCount || (uintptr_t)Maker_RawLabel(ev->labelName[j]) != po)
                                why = "a pointer into the event isn't the game's", at = i;
                        }
                        else if (pc != po)
                            why = "a pointer differs", at = i;
                        i += sizeof(uintptr_t);
                    }
                    else
                    {
                        if (code[i] != orig[i])
                            why = "a byte differs", at = i;
                        i++;
                    }
                }
            }
            if (!why)
                same++;
            else if (shown++ < 40)
                MPNet_Log("maker proof: %s: %s (at %u: %02X vs %02X)", name, why, at, code ? code[at] : 0, orig ? orig[at] : 0);
            free(src);
        }
        if (!next)
            break;
        p = next + 1;
    }
    MPNet_Log("maker proof: %d blocks, %d the same, %d differ", blocks, same, blocks - same);
    free(all);
    return blocks - same;
}

// (M4b) where a `label` of an event ended up in its compiled script (NULL: no such label, or it didn't compile)
const u8 *Maker_EventLabel(const char *map, const char *event, u8 kind, const char *source, const char *label)
{
    struct Compiled *ev = Find(map, event, kind, source);
    int i;
    if (ev->error[0])
        return NULL;
    for (i = 0; i < ev->labelCount; i++)
        if (strcmp(ev->labelName[i], label) == 0)
            return ev->labelAt[i];
    return NULL;
}

// The script an event runs (talk / step), compiled once for this text; a script that does nothing if it can't be
const u8 *Maker_EventScript(const char *map, const char *event, u8 kind, const char *source)
{
    extern const u8 Maker_EventScript_Nothing[];
    struct Compiled *ev = Find(map, event, kind, source);
    return ev->error[0] || !ev->script ? Maker_EventScript_Nothing : ev->script;
}

// An event's autorun page that should start now, or NULL (its conditions checked at once by its selector)
const u8 *Maker_EventAutorun(const char *map, const char *event, const char *source)
{
    struct Compiled *ev = Find(map, event, MAKER_EVENT_ON_MAP, source);
    u16 saved = gSpecialVar_Result, page;
    if (ev->error[0] || !ev->selector)
        return NULL;
    RunScriptImmediately(ev->selector);
    page = gSpecialVar_Result;
    gSpecialVar_Result = saved;
    return page < ev->pageCount ? ev->bodies[page] : NULL;
}

// The editor's check of an event's text: "" when it compiles, else what's wrong (the first problem)
const char *MakerApi_CheckEvent(const char *map, const char *event, int kind, const char *source)
{
    return Find(map, event, kind, source)->error;
}

// The checker (M2c): how wide a line of text is in the message box's font ({PLAYER} as 7 letters)
int MakerApi_TextWidth(const char *utf8)
{
    const u8 *t = GameText(utf8);
    int w = GetStringWidth(FONT_NORMAL, t, 0);
    free((void *)t);
    return w;
}

// ---------------------------------------------------------------------------------------------------------------------
// M4: the debugger (the editor's DEBUGGER window). Breakpoints on a Maker event's lines hold the script there (script.c
// asks before each command of the field's script, never an immediate one); STEP runs one line and holds at the next,
// CONTINUE runs on to the next breakpoint. WATCH reads flags and variables by name while a test game plays.

bool8 gMakerDebugOn;
struct BreakPoint { char map[48], event[48]; int line; };
static struct BreakPoint sBreaks[64];
static int sBreakCount;
static struct
{
    bool8 paused, go, stepping;
    const u8 *at;
    char map[48], event[48];
    int line;
} sDbg;

static void DebugUpdate(void)
{
    gMakerDebugOn = sBreakCount > 0 || sDbg.paused || sDbg.stepping;
}

int MakerApi_DebugSetBreak(const char *map, const char *event, int line, int on)
{
    int i;
    for (i = 0; i < sBreakCount; i++)
        if (strcmp(sBreaks[i].map, map) == 0 && strcmp(sBreaks[i].event, event) == 0 && sBreaks[i].line == line)
        {
            if (!on)
                sBreaks[i] = sBreaks[--sBreakCount];
            DebugUpdate();
            return 1;
        }
    if (on && sBreakCount < (int)ARRAY_COUNT(sBreaks))
    {
        snprintf(sBreaks[sBreakCount].map, sizeof(sBreaks[0].map), "%s", map);
        snprintf(sBreaks[sBreakCount].event, sizeof(sBreaks[0].event), "%s", event);
        sBreaks[sBreakCount++].line = line;
    }
    DebugUpdate();
    return on;
}

int MakerApi_DebugHasBreak(const char *map, const char *event, int line)
{
    int i;
    for (i = 0; i < sBreakCount; i++)
        if (strcmp(sBreaks[i].map, map) == 0 && strcmp(sBreaks[i].event, event) == 0 && sBreaks[i].line == line)
            return 1;
    return 0;
}

void MakerApi_DebugClearBreaks(void)
{
    sBreakCount = 0;
    DebugUpdate();
}

int MakerApi_DebugPaused(char *map, int mapSize, char *event, int eventSize, int *line)
{
    if (!sDbg.paused)
        return 0;
    if (map)
        snprintf(map, mapSize, "%s", sDbg.map);
    if (event)
        snprintf(event, eventSize, "%s", sDbg.event);
    if (line)
        *line = sDbg.line;
    return 1;
}

void MakerApi_DebugContinue(void)
{
    if (sDbg.paused)
        sDbg.go = TRUE;
    sDbg.stepping = FALSE;
    DebugUpdate();
}

void MakerApi_DebugStep(void)
{
    if (sDbg.paused)
        sDbg.go = TRUE;
    sDbg.stepping = TRUE;
    DebugUpdate();
}

// The test game stopped: nothing held
void Maker_DebugReset(void)
{
    memset(&sDbg, 0, sizeof(sDbg));
    DebugUpdate();
}

// script.c, before each command of the field's script: TRUE holds it here (the command isn't run this frame)
bool8 Maker_DebugHold(const u8 *ptr)
{
    struct Compiled *ev;
    int i;
    if (sDbg.paused)
    {
        if (!sDbg.go || ptr != sDbg.at)
            return TRUE;
        sDbg.paused = sDbg.go = FALSE;
        DebugUpdate();
        return FALSE;
    }
    for (ev = sCompiled; ev; ev = ev->next)
        for (i = 0; i < ev->lineCount; i++)
            if (ev->lineAt[i] == ptr)
            {
                if (!sDbg.stepping && !MakerApi_DebugHasBreak(ev->map, ev->event, ev->lineNo[i]))
                    return FALSE;
                sDbg.paused = TRUE;
                sDbg.stepping = FALSE;
                sDbg.at = ptr;
                snprintf(sDbg.map, sizeof(sDbg.map), "%s", ev->map);
                snprintf(sDbg.event, sizeof(sDbg.event), "%s", ev->event);
                sDbg.line = ev->lineNo[i];
                MPNet_Log("maker debugger: held at %s %s line %d", ev->map, ev->event, ev->lineNo[i]);
                DebugUpdate();
                return TRUE;
            }
    return FALSE;
}

// WATCH: a flag (FLAG_..., or one of the user's own by name), a variable (VAR_...) or a self switch
// ("SELF:MAP:EVENT:A"); 0 if there's no such name
int MakerApi_DebugValue(const char *name, int *value)
{
    u32 v;
    u16 id;
    if (strncmp(name, "FLAG_", 5) == 0 && NamedValue(MAKER_NAMES_FLAG, name, &v))
        return *value = FlagGet(v), 1;
    if (strncmp(name, "VAR_", 4) == 0 && NamedValue(MAKER_NAMES_VAR, name, &v))
        return *value = VarGet(v), 1;
    if ((id = Maker_FlagFind(name)) != 0)
        return *value = FlagGet(id), 1;
    return 0;
}
