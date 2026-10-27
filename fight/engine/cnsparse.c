/*
 * CNS parse helpers shared by the state controller parser and the trigger
 * parser (original module 0x458C30-0x45B2CF; its string literals follow the
 * trigger module's in .rdata, from "intro" to "Out of memory error parsing
 * format string.").
 *
 * - Cns_CompileInt/Float: a parameter of an Ini group with a default.
 * - Cns_ParseValues / Cns_ParseExprList / Cns_ParseArgs: comma-separated
 *   values in a format ("ii", "ff", "nnn:)"...) compiled to Exprs or appended
 *   as postfix tokens.
 * - postype, AssertSpecial flag names, F/S anim and sound references,
 *   velocities with "n" for none, hitdefattr, hitflag/guardflag, gethitvar
 *   names and values, command labels, DisplayToClipboard texts.
 *
 * Quirks kept from the original:
 * - The value parsers check a value's terminator against the source text
 *   instead of the format's terminator list (they scan the text from the
 *   argument count on until they meet the character), so any terminator is
 *   accepted for the last value; Cns_ParseExprList shrinks its array with
 *   realloc but keeps the old pointer (with fewer than three values the
 *   low fragmentation heap moves the block, so the array returned is freed
 *   memory, which the cond() trigger then frees again); failed values are
 *   not freed by it.
 * - HitDefAttr_Parse reads a lone "A" as 0x920 (the value of "H": HA+HT+HP)
 *   instead of AA+AT+AP.
 * - GetHitVar_Get returns xveladd/yveladd and the others as ints read from
 *   the player; zoff is always 0.0 (accepted only for old characters).
 * - AssertSpecial_ParseFlag copies the name to a 64-byte buffer without a
 *   length check (stack overflow in both for longer names).
 */
#include "mugen.h"

/* isspace() as the original inlines it: only for 1..127. */
static int is_blank(int c)
{
    return (unsigned int)(c - 1) <= 0x7E && isspace(c);
}

/* tolower() as the original inlines it: only for 1..127. */
static int lower(int c)
{
    return (unsigned int)(c - 1) <= 0x7E ? tolower(c) : c;
}

/* The EXPR_WANT_* of a format character; any other character is fatal. */
static int want_of(char c)
{
    if (c == 'f') {
        return EXPR_WANT_FLOAT;
    }
    if (c == 'i') {
        return EXPR_WANT_INT;
    }
    if (c != 'n') {
        Error_Exit();
    }
    return EXPR_WANT_ANY;
}

/* Values in fmt: its 'n', 'i' and 'f' characters, which must come first (then ':' or the end). */
static int format_count(const char *fmt)
{
    int count = 0;
    int i;

    for (i = 0; i < (int)strlen(fmt); i++) {
        if (fmt[i] == 'n' || fmt[i] == 'i' || fmt[i] == 'f') {
            count++;
        }
    }
    return count;
}

/* ------------------------------------------------------------ parameters */

/* 0x00458C30: e = def, then the key's value as an int expression if present. */
int Cns_CompileInt(Ini *ini, const char *key, Expr *e, CharData *owner, int def)
{
    const char *value = Ini_GetValue(ini, key);

    Expr_SetInt(e, def);
    if (value != NULL) {
        return Expr_Compile(value, e, owner, NULL, EXPR_WANT_INT);
    }
    return 1;
}

/* 0x00458C90: e = def, then the key's value as a float expression if present. */
int Cns_CompileFloat(Ini *ini, const char *key, Expr *e, CharData *owner, float def)
{
    const char *value = Ini_GetValue(ini, key);

    Expr_SetFloat(e, def);
    if (value != NULL) {
        return Expr_Compile(value, e, owner, NULL, EXPR_WANT_FLOAT);
    }
    return 1;
}

/*
 * 0x00458CE0: compiles the comma-separated values of s into the Expr *
 * arguments (NULL skips a value, which is then leaked), one per 'n'/'i'/'f'
 * of fmt. Returns the number of values read; *end is set after the trailing
 * blanks. A value ending in something else than ',' or the end of the text is
 * accepted only for the last one.
 */
int Cns_ParseValues(const char *s, const char *fmt, CharData *owner, const char **end, ...)
{
    const char *cur = s;
    const char *e;
    Expr tmp;
    Expr *out;
    va_list ap;
    int count, i, k, want;

    memset(&tmp, 0, sizeof(tmp));
    if (s == NULL) {
        return 0;
    }
    count = format_count(fmt);
    if (fmt[count] != ':' && fmt[count] != '\0') {
        return 0;
    }
    va_start(ap, end);
    for (i = 0; i < count; i++) {
        want = want_of(fmt[i]);
        out = va_arg(ap, Expr *);
        if (!Expr_Compile(cur, &tmp, owner, &e, want)) {
            break;
        }
        if (*e == ',') {
            if (i < count - 1) {
                e++;
            }
        } else if (*e != '\0') {
            if (i != count - 1) {
                Expr_Free(&tmp);
                break;
            }
            /* meant to check the terminator against fmt; s always holds it (see the top) */
            for (k = count + 1; s[k] != *e; k++) {
            }
            if (s[k] == '\0') {
                Expr_Free(&tmp);
                break;
            }
        }
        cur = e;
        if (out != NULL) {
            Expr_Free(out);
            *out = tmp;
        }
        memset(&tmp, 0, sizeof(tmp));
    }
    va_end(ap);
    while (is_blank(*cur)) {
        cur++;
    }
    if (end != NULL) {
        *end = cur;
    }
    return i;
}

/*
 * 0x00458EA0: Cns_ParseValues with the format "nnn:)" (folded in by LTCG; the
 * cond() trigger is its only user) into a new array of Exprs. Returns the
 * number of values and the array in *out, or 0.
 */
int Cns_ParseExprList(const char *s, CharData *owner, const char **end, Expr **out)
{
    static const char fmt[] = "nnn:)";
    const char *cur = s;
    const char *e;
    Expr *exprs;
    int count, i, k;

    if (s == NULL) {
        return 0;
    }
    count = format_count(fmt);
    if (fmt[count] != ':' && fmt[count] != '\0') {
        return 0;
    }
    exprs = calloc(count, sizeof(Expr));
    if (exprs == NULL) {
        g_outOfMemory = 1;
        return 0;
    }
    for (i = 0; i < count; i++) {
        if (!Expr_Compile(cur, &exprs[i], owner, &e, want_of(fmt[i]))) {
            break;
        }
        if (*e == ',') {
            if (i < count - 1) {
                e++;
            }
        } else if (*e != '\0') {
            /* the value just compiled is not freed */
            if (i != count - 1) {
                break;
            }
            for (k = count + 1; s[k] != *e; k++) {
            }
            if (s[k] == '\0') {
                break;
            }
        }
        cur = e;
    }
    while (is_blank(*cur)) {
        cur++;
    }
    if (end != NULL) {
        *end = cur;
    }
    if (i < count) {
        if (i == 0) {
            free(exprs);
            return 0;
        }
        /* the result is ignored: the block keeps its address when it shrinks */
        realloc(exprs, i * sizeof(Expr));
    }
    *out = exprs;
    return i;
}

/*
 * 0x00459050: like Cns_ParseValues, but appends each value as postfix tokens
 * to tokens/types (at most max in all) for a function trigger's arguments.
 * Returns the number of values; *count gets the number of tokens.
 */
int Cns_ParseArgs(const char *s, const char *fmt, CharData *owner, const char **end, ExprValue *tokens, char *types,
                  int *count, int max)
{
    const char *cur = s;
    const char *e;
    int nargs, i, k, n;
    int ntok = 0;

    if (s == NULL) {
        return 0;
    }
    nargs = format_count(fmt);
    if (fmt[nargs] != ':' && fmt[nargs] != '\0') {
        return 0;
    }
    for (i = 0; i < nargs; i++) {
        n = Expr_Parse(cur, tokens + ntok, types + ntok, owner, max - ntok, &e, want_of(fmt[i]));
        if (n == 0) {
            break;
        }
        ntok += n;
        if (*e == ',') {
            if (i < nargs - 1) {
                e++;
            }
        } else if (*e != '\0') {
            /* the tokens are dropped (trigger tokens leak) */
            if (i != nargs - 1) {
                ntok -= n;
                break;
            }
            for (k = nargs; s[k] != *e; k++) {
            }
            if (s[k] == '\0') {
                ntok -= n;
                break;
            }
        }
        cur = e;
    }
    while (is_blank(*cur)) {
        cur++;
    }
    if (end != NULL) {
        *end = cur;
    }
    *count = ntok;
    return i;
}

/* ------------------------------------------------------------ keywords */

/* 0x004591F0: postype: p1, p2, front, back, left, right, none (first letter, "p" then '2'); -2 if invalid. */
int Cns_ParsePosType(const char *s)
{
    switch (lower(s[0])) {
    case 'p':
        return lower(s[1]) == '2' ? POSTYPE_P2 : POSTYPE_P1;
    case 'f':
        return POSTYPE_FRONT;
    case 'b':
        return POSTYPE_BACK;
    case 'l':
        return POSTYPE_LEFT;
    case 'r':
        return POSTYPE_RIGHT;
    case 'n':
        return POSTYPE_NONE;
    default:
        return -2;
    }
}

typedef struct AssertFlag {
    const char *name;
    int kind;
    int index;
} AssertFlag;

static const AssertFlag assert_flags[] = {
    { "intro", ASSERT_GLOBAL, 0 },
    { "roundnotover", ASSERT_GLOBAL, 1 },
    { "noko", ASSERT_GLOBAL, 2 },
    { "nokosnd", ASSERT_GLOBAL, 3 },
    { "nokoslow", ASSERT_GLOBAL, 4 },
    { "nomusic", ASSERT_GLOBAL, 5 },
    { "globalnoshadow", ASSERT_GLOBAL, 6 },
    { "timerfreeze", ASSERT_GLOBAL, 7 },
    { "nobardisplay", ASSERT_GLOBAL, 8 },
    { "nobg", ASSERT_GLOBAL, 9 },
    { "nofg", ASSERT_GLOBAL, 10 },
    { "nostandguard", ASSERT_PLAYER, 0 },
    { "nocrouchguard", ASSERT_PLAYER, 1 },
    { "noairguard", ASSERT_PLAYER, 2 },
    { "noautoturn", ASSERT_PLAYER, 3 },
    { "noshadow", ASSERT_PLAYER, 4 },
    { "nojugglecheck", ASSERT_PLAYER, 5 },
    { "nowalk", ASSERT_PLAYER, 6 },
    { "unguardable", ASSERT_PLAYER, 7 },
    { "invisible", ASSERT_PLAYER, 8 },
};

/* 0x004592A0: an AssertSpecial flag name (any case): ASSERT_* kind and index. Returns 0 if unknown. */
int AssertSpecial_ParseFlag(const char *name, int *kind, int *index)
{
    char buf[0x40];
    int i;

    strcpy(buf, name);
    _strlwr(buf);
    for (i = 0; i < (int)(sizeof(assert_flags) / sizeof(assert_flags[0])); i++) {
        if (strcmp(buf, assert_flags[i].name) == 0) {
            *kind = assert_flags[i].kind;
            *index = assert_flags[i].index;
            return 1;
        }
    }
    return 0;
}

/*
 * 0x00459830: an animation number with an optional F (fight.def, *fight = 1)
 * or S (own, *fight = 0) prefix; *fight is left alone without one. A missing
 * value (NULL) is fine.
 */
int Cns_ParseAnimRef(const char *s, int *fight, Expr *e, CharData *owner)
{
    const char *end;
    int c;

    if (s == NULL) {
        return 1;
    }
    c = lower(*s);
    if (c == 'f') {
        *fight = 1;
        s++;
    } else if (c == 's') {
        *fight = 0;
        s++;
    }
    if (!Cns_ParseValues(s, "i", owner, &end, e)) {
        return 0;
    }
    return *end == '\0';
}

/* 0x004598B0: "group, index" of a sound with an optional F/S prefix, as Cns_ParseAnimRef. */
int Cns_ParseSoundRef(const char *s, int *fight, Expr *group, Expr *index, CharData *owner)
{
    const char *end;
    int c;

    if (s == NULL) {
        return 1;
    }
    c = lower(*s);
    if (c == 'f') {
        *fight = 1;
        s++;
    } else if (c == 's') {
        *fight = 0;
        s++;
    }
    if (!Cns_ParseValues(s, "ii", owner, &end, group, index)) {
        return 0;
    }
    return *end == '\0';
}

/*
 * 0x00459930: "x, y" float velocities where "n" stands for "none" (-FLT_MAX);
 * the y value is optional. The first "n" may be followed by a blank, ',' or
 * the end, the second only by a blank or the end. A missing value (NULL) is
 * fine.
 */
int Cns_ParseVelocity(const char *s, Expr *x, Expr *y, CharData *owner)
{
    const char *e;

    if (s == NULL) {
        return 1;
    }
    if (lower(*s) == 'n' && (is_blank(s[1]) || s[1] == ',' || s[1] == '\0')) {
        Expr_SetFloat(x, -FLT_MAX);
        e = s + 1;
    } else if (!Expr_Compile(s, x, owner, &e, EXPR_WANT_FLOAT)) {
        return 0;
    }
    if (*e != ',') {
        return *e == '\0';
    }
    do {
        e++;
    } while (is_blank(*e));
    if (lower(*e) == 'n' && (is_blank(e[1]) || e[1] == '\0')) {
        Expr_SetFloat(y, -FLT_MAX);
        e++;
    } else if (!Expr_Compile(e, y, owner, &e, EXPR_WANT_FLOAT)) {
        return 0;
    }
    while (is_blank(*e)) {
        e++;
    }
    return *e == '\0';
}

/*
 * 0x00459AB0: a hit attribute "SCA, NA, ST, HP..." (also the "= value" of the
 * HitDefAttr trigger): the state letters, then a comma-separated list of
 * attack classes. Returns the bits, or -1; *end is set after the last item
 * read. The list stops at an invalid item, or when an item is followed by a
 * letter, digit or '_' (that item is not counted).
 */
int HitDefAttr_Parse(const char *s, const char **end)
{
    unsigned int state = 0, flags = 0;
    int i = 0, count = 0, last = 0;
    int c;

    if (s == NULL) {
        return -1;
    }
    if (isalpha(s[0])) {
        do {
            c = lower(s[i]);
            if (c == 'a') {
                state |= HITDEFATTR_A;
            } else if (c == 'c') {
                state |= HITDEFATTR_C;
            } else if (c == 's') {
                state |= HITDEFATTR_S;
            } else {
                return -1;
            }
            i++;
        } while (isalpha(s[i]));
    }
    while (is_blank(s[i])) {
        i++;
    }
    if (s[i] == '\0') {
        if (end != NULL) {
            *end = s + i;
        }
        return (int)state;
    }
    for (;;) {
        unsigned int all, a, t, p;

        while (is_blank(s[i])) {
            i++;
        }
        count++;
        flags |= state;
        last = i - 1;
        if (s[i] != ',') {
            break;
        }
        do {
            i++;
        } while (is_blank(s[i]));
        state = 0;
        switch (lower(s[i])) {
        case 'n':
            all = 0x248, a = 0x008, t = 0x040, p = 0x200;
            break;
        case 's':
            all = 0x490, a = 0x010, t = 0x080, p = 0x400;
            break;
        case 'h':
            all = 0x920, a = 0x020, t = 0x100, p = 0x800;
            break;
        case 'a':
            /* original bug: a lone "A" gives the value of "H" */
            all = 0x920, a = 0x038, t = 0x1C0, p = 0xE00;
            break;
        default:
            goto finish;
        }
        switch (lower(s[i + 1])) {
        case '\0':
        case '\t':
        case ' ':
        case ',':
            state = all;
            break;
        case 'a':
            state = a;
            i++;
            break;
        case 't':
            state = t;
            i++;
            break;
        case 'p':
            state = p;
            i++;
            break;
        default:
            goto finish;
        }
        c = s[i + 1];
        if (isalnum(c) || c == '_') {
            break;
        }
        i++;
    }
finish:
    if (count < 1) {
        return -1;
    }
    if (end != NULL) {
        *end = s + last + 1;
    }
    return (int)flags;
}

/* 0x00459FD0: hitflag/guardflag letters (H, L, A, M, F, D, P, +, -) to bits, from 2 ("-" clears it). */
unsigned int HitFlags_Parse(const char *s)
{
    unsigned int flags = 2;

    for (; *s != '\0'; s++) {
        switch (lower(*s)) {
        case '+':
            flags |= 0x100;
            break;
        case '-':
            flags &= ~2u;
            break;
        case 'a':
            flags |= 0x10;
            break;
        case 'd':
            flags |= 0x40;
            break;
        case 'f':
            flags |= 0x20;
            break;
        case 'h':
            flags |= 0x04;
            break;
        case 'l':
            flags |= 0x08;
            break;
        case 'm':
            flags |= 0x0C;
            break;
        case 'p':
            flags |= 0x80;
            break;
        }
    }
    return flags;
}

/* ------------------------------------------------------------ gethitvar */

typedef struct GetHitVarName {
    const char *name;
    int is_float;
} GetHitVarName;

/* By GetHitVar index. */
static const GetHitVarName gethitvar_names[GETHITVAR_COUNT] = {
    { "xveladd", 0 },
    { "yveladd", 0 },
    { "type", 0 },
    { "animtype", 0 },
    { "airtype", 0 },
    { "groundtype", 0 },
    { "damage", 0 },
    { "hitcount", 0 },
    { "fallcount", 0 },
    { "hitshaketime", 0 },
    { "hittime", 0 },
    { "slidetime", 0 },
    { "ctrltime", 0 },
    { "recovertime", 0 },
    { "xoff", 1 },
    { "yoff", 1 },
    { "zoff", 1 },
    { "xvel", 1 },
    { "yvel", 1 },
    { "yaccel", 1 },
    { "chainid", 0 },
    { "guarded", 0 },
    { "isbound", 0 },
    { "fall", 0 },
    { "fall.damage", 0 },
    { "fall.xvel", 1 },
    { "fall.yvel", 1 },
    { "fall.recover", 0 },
    { "fall.time", 0 },
    { "fall.recovertime", 0 },
    { "fall.kill", 0 },
    { "fall.envshake.time", 0 },
    { "fall.envshake.dir", 0 },
    { "fall.envshake.ampl", 0 },
    { "fall.envshake.freq", 1 },
    { "fall.envshake.phase", 1 },
};

/* 0x0045A0C0: index and type of a gethitvar name (lower case); "zoff" only when old_version. */
int GetHitVar_Lookup(const char *name, int old_version, int *index, int *is_float)
{
    int i;

    for (i = 0; i < GETHITVAR_COUNT; i++) {
        if (i == GETHITVAR_ZOFF && !old_version) {
            continue;
        }
        if (strcmp(name, gethitvar_names[i].name) == 0) {
            *index = i;
            *is_float = gethitvar_names[i].is_float;
            return 1;
        }
    }
    return 0;
}

/*
 * 0x0045AA90: the value of gethitvar index for p: an int in *ival or a float
 * in *fval (*is_float set); *scaled is set for values in the character's
 * coordinate units. Returns 0 for a bad index.
 */
int GetHitVar_Get(Player *p, int index, int *ival, float *fval, int *is_float, int *scaled)
{
    int iv = 0;
    float fv = 0.0f;
    int fl = 0, sc = 0;

    switch (index) {
    case 0:
        iv = p->gethit_xveladd;
        sc = 1;
        break;
    case 1:
        iv = p->gethit_yveladd;
        sc = 1;
        break;
    case 2:
        iv = p->gethit_type;
        break;
    case 3:
        iv = p->gethit_animtype;
        break;
    case 4:
        iv = p->gethit_airtype;
        break;
    case 5:
        iv = p->gethit_groundtype;
        break;
    case 6:
        iv = p->gethit_damage;
        break;
    case 7:
        iv = p->gethit_hitcount;
        break;
    case 8:
        iv = p->gethit_fallcount;
        break;
    case 9:
        iv = p->gethit_shaketime;
        break;
    case 10:
        iv = p->gethit_hittime;
        break;
    case 11:
        iv = p->gethit_slidetime;
        break;
    case 12:
        iv = p->gethit_ctrltime;
        break;
    case 13:
        iv = p->gethit_recovertime;
        break;
    case 14:
        fv = p->gethit_xoff;
        fl = sc = 1;
        break;
    case 15:
        fv = p->gethit_yoff;
        fl = sc = 1;
        break;
    case 16:
        fv = 0.0f;
        fl = sc = 1;
        break;
    case 17:
        fv = p->gethit_vel_x;
        fl = sc = 1;
        break;
    case 18:
        fv = p->gethit_vel_y;
        fl = sc = 1;
        break;
    case 19:
        fv = p->gethit_yaccel;
        fl = sc = 1;
        break;
    case 20:
        iv = p->gethit_id;
        break;
    case 21:
        iv = p->gethit_guarded;
        break;
    case 22:
        iv = p->gethit_isbound != 0;
        break;
    case 23:
        iv = p->gethit_fall;
        break;
    case 24:
        iv = p->gethit_fall_damage;
        break;
    case 25:
        fv = p->gethit_fall_xvel;
        fl = sc = 1;
        break;
    case 26:
        fv = p->gethit_fall_yvel;
        fl = sc = 1;
        break;
    case 27:
        iv = p->gethit_fall_recover;
        break;
    case 28:
        iv = p->gethit_fall_time;
        break;
    case 29:
        iv = p->gethit_fall_recovertime;
        break;
    case 30:
        iv = p->gethit_fall_kill;
        break;
    case 31:
        iv = p->gethit_fall_envshake_time;
        break;
    case 32:
        iv = p->gethit_fall_envshake_dir;
        break;
    case 33:
        iv = (int)p->gethit_fall_envshake_ampl;
        sc = 1;
        break;
    case 34:
        fv = p->gethit_fall_envshake_freq;
        fl = 1;
        break;
    case 35:
        fv = p->gethit_fall_envshake_phase;
        fl = 1;
        break;
    default:
        return 0;
    }
    if (fl) {
        *fval = fv;
    } else {
        *ival = iv;
    }
    *is_float = fl;
    *scaled = sc;
    return 1;
}

/* ------------------------------------------------------------ commands */

/*
 * 0x0045ACF0: the label of the command named at s (up to a '"', at most 63
 * characters) in owner's .cmd. *end is set after the name. Returns 0 if
 * there is no such command.
 */
int Cmd_FindLabel(const char *s, int *label, CharData *owner, const char **end)
{
    char name[0x40];
    int n = 0;
    int i;

    if (s[0] == '\0') {
        return 0;
    }
    while (s[n] != '"' && n < 0x3F) {
        name[n] = s[n];
        n++;
        if (s[n] == '\0') {
            break;
        }
    }
    name[n] = '\0';
    for (i = 0; i < owner->cmds.count; i++) {
        if (strcmp(owner->cmds.names[i], name) == 0) {
            *label = owner->cmds.entries[i].label;
            *end = s + strlen(owner->cmds.names[i]);
            return 1;
        }
    }
    return 0;
}

/* ------------------------------------------------------------ texts */

/*
 * 0x0045ADD0: the printf specifier at s ("%" [#+-]* digits* [. digits*]
 * conversion). Returns 1 for d/i, 2 for e/f/g/E/F/G, 3 for a plain "%%", 0
 * otherwise; *len gets its length (0 when s is not a '%').
 */
int CnsText_ParseFormat(const char *s, int *len)
{
    int flags = 0, width = 0, precision = 0;
    int i;

    if (s == NULL || s[0] != '%') {
        *len = 0;
        return 0;
    }
    i = 1;
    while (s[i] == '#' || s[i] == '+' || s[i] == '-') {
        i++;
        flags = 1;
    }
    while ((unsigned int)(s[i] - 1) <= 0x7E && isdigit(s[i])) {
        i++;
        width = 1;
    }
    if (s[i] == '.') {
        i++;
        precision = 1;
        if (s[i] == '*') {
            return 0;
        }
        while ((unsigned int)(s[i] - 1) <= 0x7E && isdigit(s[i])) {
            i++;
        }
    }
    switch (s[i]) {
    case 'd':
    case 'i':
        *len = i + 1;
        return 1;
    case 'e':
    case 'f':
    case 'g':
    case 'E':
    case 'F':
    case 'G':
        *len = i + 1;
        return 2;
    case '%':
        if (flags || width || precision) {
            return 0;
        }
        *len = i + 1;
        return 3;
    default:
        return 0;
    }
}

/*
 * 0x0045AF40: the quoted text of DisplayToClipboard/AppendToClipboard with up
 * to six int/float specifiers and the escapes \" \\ \n \t (others become a
 * NUL character). Returns a new CnsText, or NULL with an error message.
 * A '\' at the very end makes both passes read past the terminator, as in
 * the original.
 */
CnsText *CnsText_Parse(const char *s)
{
    int pos[CNSTEXT_MAX_PARAMS];
    int lens[CNSTEXT_MAX_PARAMS];
    char types[CNSTEXT_MAX_PARAMS];
    const int *lp;
    CnsText *t;
    int n, len, i, o, r, l;
    int flen = 0;

    if (s[0] != '"') {
        Error_Printf("Message text not quote-delimited.\n");
        return NULL;
    }
    n = 0;
    len = 0;
    i = 1;
    if (s[1] != '\0') {
        do {
            char c = s[i];
            if (c == '"') {
                break;
            }
            if (c == '%') {
                r = CnsText_ParseFormat(s + i, &flen);
                if (r == 0) {
                    Error_Printf("Invalid format specifier in %s\n", s);
                    return NULL;
                }
                if (r == 1 || r == 2) {
                    if (n >= CNSTEXT_MAX_PARAMS) {
                        Error_Printf("More than 6 parameters in %s\n", s);
                        return NULL;
                    }
                    pos[n] = len;
                    types[n] = (char)(r != 1 ? 2 : 1);
                    lens[n] = flen;
                    n++;
                    i += flen;
                    len += flen;
                } else if (r == 3) {
                    i += 2;
                    len++;
                }
            } else if (c == '\\') {
                i += 2;
                len++;
            } else {
                i++;
                len++;
            }
        } while (s[i] != '\0');
    }
    if (s[i] == '\0') {
        Error_Printf("Message text not quote-delimited: %s\n", s);
        return NULL;
    }
    if (s[i + 1] != '\0') {
        Error_Printf("Trailing non-delimited text detected: %s\n", s);
        return NULL;
    }

    t = calloc(1, len + offsetof(CnsText, text) + 1);
    if (t == NULL) {
        g_outOfMemory = 1;
        Error_Printf("Out of memory error parsing format string.\n");
        return NULL;
    }
    memcpy(t->len, lens, n * sizeof(int));
    memcpy(t->pos, pos, n * sizeof(int));
    t->count = (short)n;
    memcpy(t->types, types, n);
    o = 0;
    i = 1;
    lp = lens;
    if (s[1] != '\0') {
        do {
            char c = s[i];
            if (c == '"') {
                break;
            }
            if (c == '%') {
                if (s[i + 1] == '%') {
                    t->text[o] = '%';
                    o++;
                    i += 2;
                } else {
                    l = *lp++;
                    memcpy(t->text + o, s + i, l);
                    o += l;
                    i += l;
                }
            } else if (c == '\\') {
                switch (s[i + 1]) {
                case '"':
                    t->text[o] = '"';
                    break;
                case 'n':
                    t->text[o] = '\n';
                    break;
                case 't':
                    t->text[o] = '\t';
                    break;
                case '\\':
                    t->text[o] = '\\';
                    break;
                }
                i += 2;
                o++;
            } else {
                t->text[o] = c;
                i++;
                o++;
            }
        } while (s[i] != '\0');
    }
    t->text[o] = '\0';
    return t;
}
