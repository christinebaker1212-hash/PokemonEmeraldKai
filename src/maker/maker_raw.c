// Moemon Maker, M4b: the game's own script macros assembled at run time (the `raw` command of an event), the way the
// assembler builds data/event_scripts.s, so an event decompiled from a map's scripts.inc (tools/maker/decompile.py)
// is the same bytes as the built script. The macros, constants and global labels come from tools/maker/gen_macros.py
// (include/constants/maker_macros.h, maker_labels.h: generated when the game is built, never committed).
//
// A macro's body is read line by line as gas would: .byte / .short / .2byte / .int / .4byte values, ptrvalue (a
// pointer: the sink's), .if / .elseif / .else / .endif with expressions, .ifb / .ifnb on an argument, .set (symbols
// of this assembly, `.` the current offset), .warning (ignored), and calls of other macros with the arguments put in
// for \name (as text, like gas). Expressions: numbers, the constants, the symbols .set here, `.`, unary - ~ !,
// * / % + - << >> < <= > >= == != <> & ^ | && || and parentheses.
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "maker.h"
#include "constants/maker_macros.h"

struct MakerLabel { const char *name; const u8 *at; };
extern const struct MakerLabel gMakerLabels[];   // (src/maker/maker_labeltab.c)
extern const int gMakerLabelCount;

#define MAX_ARGS 12
#define MAX_DEPTH 8

static struct { char name[48]; s32 value; } sSets[16];   // .set NAME, value (this assembly's)
static int sSetCount;

static int CmpMacro(const void *k, const void *e) { return strcmp((const char *)k, ((const struct MakerMacro *)e)->name); }
static int CmpSym(const void *k, const void *e) { return strcmp((const char *)k, ((const struct MakerSym *)e)->name); }
static int CmpFunc(const void *k, const void *e) { return strcmp((const char *)k, ((const struct MakerFunc *)e)->name); }
static int CmpLabel(const void *k, const void *e) { return strcmp((const char *)k, ((const struct MakerLabel *)e)->name); }

bool8 Maker_RawSymbol(const char *name, s32 *value)
{
    const struct MakerSym *s;
    int i;
    for (i = 0; i < sSetCount; i++)
        if (strcmp(sSets[i].name, name) == 0)
        {
            *value = sSets[i].value;
            return TRUE;
        }
    s = bsearch(name, sMakerSyms, ARRAY_COUNT(sMakerSyms), sizeof(sMakerSyms[0]), CmpSym);
    if (!s)
        return FALSE;
    *value = s->value;
    return TRUE;
}

const u8 *Maker_RawLabel(const char *name)
{
    const struct MakerLabel *l = bsearch(name, gMakerLabels, gMakerLabelCount, sizeof(gMakerLabels[0]), CmpLabel);
    return l ? l->at : NULL;
}

bool8 Maker_RawIsMacro(const char *name)
{
    return bsearch(name, sMakerMacros, ARRAY_COUNT(sMakerMacros), sizeof(sMakerMacros[0]), CmpMacro) != NULL;
}

// ---- expressions
struct Expr { const char *p; struct MakerRawSink *s; bool8 bad; };

static void Skip(struct Expr *e) { while (*e->p == ' ' || *e->p == '\t') e->p++; }
static s32 Or(struct Expr *e);

static s32 Atom(struct Expr *e)
{
    s32 v = 0;
    Skip(e);
    if (*e->p == '(')
    {
        e->p++;
        v = Or(e);
        Skip(e);
        if (*e->p == ')')
            e->p++;
        else
            e->bad = TRUE;
        return v;
    }
    if (*e->p == '-') { e->p++; return -Atom(e); }
    if (*e->p == '+') { e->p++; return Atom(e); }
    if (*e->p == '~') { e->p++; return ~Atom(e); }
    if (*e->p == '!') { e->p++; return !Atom(e); }
    if (isdigit((unsigned char)*e->p))
    {
        char *end;
        v = (s32)strtol(e->p, &end, 0);
        e->p = end;
        return v;
    }
    if (*e->p == '.' && !isalnum((unsigned char)e->p[1]) && e->p[1] != '_')
    {
        e->p++;
        return (s32)e->s->here(e->s->ctx);
    }
    if (isalpha((unsigned char)*e->p) || *e->p == '_')
    {
        char name[128];
        int n = 0;
        while ((isalnum((unsigned char)*e->p) || *e->p == '_') && n < 127)
            name[n++] = *e->p++;
        name[n] = 0;
        Skip(e);
        if (*e->p == '(')   // a C macro of one parameter (MAP_NUM(map)): its body with the argument put in
        {
            const struct MakerFunc *f = bsearch(name, sMakerFuncs, ARRAY_COUNT(sMakerFuncs), sizeof(sMakerFuncs[0]), CmpFunc);
            if (f)
            {
                char arg[256], body[512];
                const char *b = f->body;
                int depth = 0, a = 0, o = 0, plen = strlen(f->param);
                struct Expr sub;
                e->p++;
                while (*e->p && (depth || *e->p != ')') && a < (int)sizeof(arg) - 1)
                {
                    depth += (*e->p == '(') - (*e->p == ')');
                    arg[a++] = *e->p++;
                }
                arg[a] = 0;
                if (*e->p == ')')
                    e->p++;
                while (*b && o < (int)sizeof(body) - a - 3)
                {
                    if (strncmp(b, f->param, plen) == 0 && !(isalnum((unsigned char)b[plen]) || b[plen] == '_')
                        && (b == f->body || !(isalnum((unsigned char)b[-1]) || b[-1] == '_')))
                    {
                        o += snprintf(body + o, sizeof(body) - o, "(%s)", arg);
                        b += plen;
                    }
                    else
                        body[o++] = *b++;
                }
                body[o] = 0;
                sub.p = body, sub.s = e->s, sub.bad = FALSE;
                v = Or(&sub);
                if (sub.bad)
                    e->bad = TRUE;
                return v;
            }
        }
        if (!Maker_RawSymbol(name, &v) && (Maker_RawLabel(name) || (e->s->isLabel && e->s->isLabel(e->s->ctx, name))))
            return 1;   // (a label: an address, never 0: ".if \event_script == FALSE")
        if (!Maker_RawSymbol(name, &v))
        {
            char msg[96];
            snprintf(msg, sizeof(msg), "no constant named %s", name);
            e->s->error(e->s->ctx, msg);
            e->bad = TRUE;
        }
        return v;
    }
    e->bad = TRUE;
    return 0;
}

static s32 Mul(struct Expr *e)
{
    s32 v = Atom(e), r;
    for (;;)
    {
        Skip(e);
        if (*e->p == '*') { e->p++; v *= Atom(e); }
        else if (*e->p == '/') { e->p++; r = Atom(e); v = r ? v / r : 0; }
        else if (*e->p == '%') { e->p++; r = Atom(e); v = r ? v % r : 0; }
        else return v;
    }
}
static s32 Add(struct Expr *e)
{
    s32 v = Mul(e);
    for (;;)
    {
        Skip(e);
        if (*e->p == '+') { e->p++; v += Mul(e); }
        else if (*e->p == '-') { e->p++; v -= Mul(e); }
        else return v;
    }
}
static s32 Shift(struct Expr *e)
{
    s32 v = Add(e);
    for (;;)
    {
        Skip(e);
        if (e->p[0] == '<' && e->p[1] == '<') { e->p += 2; v = (s32)((u32)v << Add(e)); }
        else if (e->p[0] == '>' && e->p[1] == '>') { e->p += 2; v >>= Add(e); }
        else return v;
    }
}
static s32 Rel(struct Expr *e)
{
    s32 v = Shift(e);
    for (;;)
    {
        Skip(e);
        if (e->p[0] == '<' && e->p[1] == '=') { e->p += 2; v = v <= Shift(e); }
        else if (e->p[0] == '>' && e->p[1] == '=') { e->p += 2; v = v >= Shift(e); }
        else if (e->p[0] == '<' && e->p[1] == '>') { e->p += 2; v = v != Shift(e); }
        else if (e->p[0] == '<' && e->p[1] != '<') { e->p++; v = v < Shift(e); }
        else if (e->p[0] == '>' && e->p[1] != '>') { e->p++; v = v > Shift(e); }
        else return v;
    }
}
static s32 Eq(struct Expr *e)
{
    s32 v = Rel(e);
    for (;;)
    {
        Skip(e);
        if (e->p[0] == '=' && e->p[1] == '=') { e->p += 2; v = v == Rel(e); }
        else if (e->p[0] == '!' && e->p[1] == '=') { e->p += 2; v = v != Rel(e); }
        else return v;
    }
}
static s32 BitAnd(struct Expr *e)
{
    s32 v = Eq(e);
    for (;;) { Skip(e); if (e->p[0] == '&' && e->p[1] != '&') { e->p++; v &= Eq(e); } else return v; }
}
static s32 BitXor(struct Expr *e)
{
    s32 v = BitAnd(e);
    for (;;) { Skip(e); if (e->p[0] == '^') { e->p++; v ^= BitAnd(e); } else return v; }
}
static s32 BitOr(struct Expr *e)
{
    s32 v = BitXor(e);
    for (;;) { Skip(e); if (e->p[0] == '|' && e->p[1] != '|') { e->p++; v |= BitXor(e); } else return v; }
}
static s32 And(struct Expr *e)
{
    s32 v = BitOr(e);
    for (;;) { Skip(e); if (e->p[0] == '&' && e->p[1] == '&') { e->p += 2; v = BitOr(e) && v; } else return v; }
}
static s32 Or(struct Expr *e)
{
    s32 v = And(e);
    for (;;) { Skip(e); if (e->p[0] == '|' && e->p[1] == '|') { e->p += 2; v = And(e) || v; } else return v; }
}

static bool8 Eval(struct MakerRawSink *s, const char *text, s32 *v)
{
    struct Expr e = {text, s, FALSE};
    *v = Or(&e);
    Skip(&e);
    if (*e.p)
        e.bad = TRUE;
    return !e.bad;
}

// ---- arguments: split at top-level commas (quotes and parentheses kept whole), trimmed
static int SplitArgs(char *text, char **out, int max)
{
    int n = 0, depth = 0;
    bool8 q = FALSE;
    char *start = text, *p;
    while (*start == ' ' || *start == '\t')
        start++;
    if (!*start)
        return 0;
    for (p = start;; p++)
    {
        if (*p == '"' && (p == start || p[-1] != '\\'))
            q = !q;
        else if (!q && *p == '(')
            depth++;
        else if (!q && *p == ')')
            depth--;
        if (*p == 0 || (!q && depth == 0 && *p == ','))
        {
            char end = *p, *t = p;
            *p = 0;
            while (t > start && (t[-1] == ' ' || t[-1] == '\t'))
                *--t = 0;
            if (n < max)
                out[n++] = start;
            if (!end)
                break;
            start = p + 1;
            while (*start == ' ' || *start == '\t')
                start++;
        }
    }
    if (n == 1 && strpbrk(out[0], " \t"))   // gas also takes names and numbers apart by spaces ("frontier_set A B")
    {
        char *w = out[0], *q2;
        bool8 plain = TRUE;
        for (q2 = w; *q2; q2++)
            if (!(isalnum((unsigned char)*q2) || *q2 == '_' || *q2 == ' ' || *q2 == '\t'))
                plain = FALSE;
        if (plain)
        {
            n = 0;
            while (*w && n < max)
            {
                out[n++] = w;
                while (*w && *w != ' ' && *w != '\t')
                    w++;
                if (*w)
                    *w++ = 0;
                while (*w == ' ' || *w == '\t')
                    w++;
            }
        }
    }
    return n;
}

static bool8 RunMacro(struct MakerRawSink *s, const struct MakerMacro *m, char **args, int nargs, int depth);

// one line of a macro body (or of the event: a directive or a macro call), \params already put in
static bool8 Line(struct MakerRawSink *s, char *line, int depth)
{
    char *w = line, *rest, *args[MAX_ARGS];
    int n, i;
    s32 v;
    while (*w == ' ' || *w == '\t')
        w++;
    if (!*w)
        return TRUE;
    for (rest = w; *rest && *rest != ' ' && *rest != '\t'; rest++)
        ;
    if (*rest)
        *rest++ = 0;
    if (strcmp(w, ".byte") == 0 || strcmp(w, ".short") == 0 || strcmp(w, ".2byte") == 0 || strcmp(w, ".hword") == 0
        || strcmp(w, ".int") == 0 || strcmp(w, ".4byte") == 0 || strcmp(w, ".word") == 0 || strcmp(w, ".long") == 0)
    {
        int size = w[1] == 'b' ? 1 : (w[1] == 's' || w[1] == '2' || w[1] == 'h') ? 2 : 4;
        n = SplitArgs(rest, args, MAX_ARGS);
        for (i = 0; i < n; i++)
        {
            if (!Eval(s, args[i], &v))
                return FALSE;
            if (size == 1)
                s->byte(s->ctx, (u8)v);
            else if (size == 2)
                s->halfword(s->ctx, (u16)v);
            else
                s->word(s->ctx, (u32)v);
        }
        return TRUE;
    }
    if (strcmp(w, "ptrvalue") == 0 || strcmp(w, ".quad") == 0)
    {
        while (*rest == ' ')
            rest++;
        return s->pointer(s->ctx, rest);
    }
    if (strcmp(w, ".set") == 0)
    {
        n = SplitArgs(rest, args, 2);
        if (n != 2 || !Eval(s, args[1], &v))
            return FALSE;
        for (i = 0; i < sSetCount && strcmp(sSets[i].name, args[0]); i++)
            ;
        if (i == sSetCount && sSetCount < (int)ARRAY_COUNT(sSets))
            snprintf(sSets[sSetCount++].name, sizeof(sSets[0].name), "%s", args[0]);
        if (i < (int)ARRAY_COUNT(sSets))
            sSets[i].value = v;
        return TRUE;
    }
    if (strcmp(w, ".warning") == 0 || strcmp(w, ".align") == 0)
        return TRUE;
    if (w[0] == '.')
    {
        char msg[96];
        snprintf(msg, sizeof(msg), "the assembler line %s isn't known", w);
        s->error(s->ctx, msg);
        return FALSE;
    }
    {
        const struct MakerMacro *m = bsearch(w, sMakerMacros, ARRAY_COUNT(sMakerMacros), sizeof(sMakerMacros[0]), CmpMacro);
        if (!m)
        {
            char msg[96];
            snprintf(msg, sizeof(msg), "no script command named %s", w);
            s->error(s->ctx, msg);
            return FALSE;
        }
        n = SplitArgs(rest, args, MAX_ARGS);
        return RunMacro(s, m, args, n, depth + 1);
    }
}

// \name -> the argument's text (gas: the longest parameter name that matches)
static void Substitute(const char *line, char *out, size_t cap, char pnames[][32], char **values, int np)
{
    size_t o = 0;
    while (*line && o + 1 < cap)
    {
        if (*line == '\\')
        {
            int best = -1;
            size_t bestLen = 0;
            int k;
            for (k = 0; k < np; k++)
            {
                size_t l = strlen(pnames[k]);
                if (l > bestLen && strncmp(line + 1, pnames[k], l) == 0 && !(isalnum((unsigned char)line[1 + l]) || line[1 + l] == '_'))
                    best = k, bestLen = l;
            }
            if (best >= 0)
            {
                const char *v = values[best] ? values[best] : "";
                while (*v && o + 1 < cap)
                    out[o++] = *v++;
                line += 1 + bestLen;
                if (line[0] == '\\' && line[1] == '(' && line[2] == ')')
                    line += 3;
                continue;
            }
        }
        out[o++] = *line++;
    }
    out[o] = 0;
}

static bool8 RunMacro(struct MakerRawSink *s, const struct MakerMacro *m, char **args, int nargs, int depth)
{
    char pnames[MAX_ARGS][32], pdefault[MAX_ARGS][48], *values[MAX_ARGS], params[256], *pp[MAX_ARGS], body[6144];
    char line[2048], *bl, *next, kw[MAX_ARGS][128];
    int np, i, top = 0;
    // .if nesting: taking (this branch runs), taken (a branch already ran), outer (the enclosing one runs)
    bool8 taking[16], taken[16], outer[16];
    if (depth > MAX_DEPTH)
    {
        s->error(s->ctx, "macros nest too deep");
        return FALSE;
    }
    snprintf(params, sizeof(params), "%s", m->params);
    for (i = 0; params[i]; i++)   // ("a:req b c=1": spaces between parameters work too)
        if (params[i] == ' ' && i > 0 && params[i - 1] != ',')
        {
            int j = i;
            while (params[j] == ' ')
                j++;
            if (params[j] && params[j] != ',' && params[j] != '=')
                params[i] = ',';
        }
    np = SplitArgs(params, pp, MAX_ARGS);
    {   // keyword arguments ("waitstate implicit=1"): put in their parameter's place
        char *pos[MAX_ARGS];
        int k, npos = 0;
        for (i = 0; i < MAX_ARGS; i++)
            kw[i][0] = 0;
        for (i = 0; i < nargs; i++)
        {
            char *eq = strchr(args[i], '=');
            int matched = -1;
            if (eq && eq > args[i] && eq[1] != '=' && eq[-1] != '!' && eq[-1] != '<' && eq[-1] != '>')
                for (k = 0; k < np; k++)
                {
                    size_t l = strcspn(pp[k], ":=");
                    size_t al = (size_t)(eq - args[i]);
                    while (al && args[i][al - 1] == ' ')
                        al--;
                    if (l == al && strncmp(pp[k], args[i], l) == 0)
                        matched = k;
                }
            if (matched >= 0)
            {
                char *v = eq + 1;
                while (*v == ' ')
                    v++;
                snprintf(kw[matched], sizeof(kw[matched]), "%s", v);
            }
            else if (npos < MAX_ARGS)
                pos[npos++] = args[i];
        }
        for (k = 0, i = 0; k < np; k++)
            if (kw[k][0])
                args[k] = kw[k];
            else
                args[k] = i < npos ? pos[i++] : (char *)"";
        nargs = np;
    }
    for (i = 0; i < np; i++)
    {
        char *colon = strchr(pp[i], ':'), *eq = strchr(pp[i], '=');
        pdefault[i][0] = 0;
        if (eq)
        {
            *eq = 0;
            snprintf(pdefault[i], sizeof(pdefault[i]), "%s", eq + 1);
        }
        if (colon)
            *colon = 0;
        snprintf(pnames[i], sizeof(pnames[i]), "%s", pp[i]);
        values[i] = i < nargs && args[i][0] ? args[i] : (pdefault[i][0] ? pdefault[i] : NULL);
        if (colon && strncmp(colon + 1, "req", 3) == 0 && !values[i])
        {
            char msg[96];
            snprintf(msg, sizeof(msg), "%s needs its %s", m->name, pnames[i]);
            s->error(s->ctx, msg);
            return FALSE;
        }
    }
    snprintf(body, sizeof(body), "%s", m->body);
    taking[0] = taken[0] = outer[0] = TRUE;
    for (bl = body; bl && *bl; bl = next)
    {
        char *t;
        next = strchr(bl, '\n');
        if (next)
            *next++ = 0;
        Substitute(bl, line, sizeof(line), pnames, values, np);
        for (t = line; *t == ' ' || *t == '\t'; t++)
            ;
        if (strncmp(t, ".if", 3) == 0 && (t[3] == ' ' || t[3] == '\t' || t[3] == 'b' || t[3] == 'n' || t[3] == 'd'))
        {
            bool8 run = taking[top];
            s32 v = 0;
            if (top >= 15)
                return FALSE;
            if (strncmp(t, ".ifdef", 6) == 0 || strncmp(t, ".ifndef", 7) == 0)   // is it a constant (or a label)
            {
                char *a = t + (t[3] == 'd' ? 6 : 7), name[64];
                int k = 0;
                s32 dummy;
                while (*a == ' ' || *a == '\t')
                    a++;
                while (a[k] && a[k] != ' ' && k < 63)
                    name[k] = a[k], k++;
                name[k] = 0;
                v = Maker_RawSymbol(name, &dummy) || Maker_RawLabel(name) != NULL;
                if (t[3] == 'n')
                    v = !v;
            }
            else if (t[3] == 'b' || (t[3] == 'n' && t[4] == 'b'))   // .ifb / .ifnb: is the argument blank
            {
                char *a = t + (t[3] == 'b' ? 4 : 5);
                while (*a == ' ' || *a == '\t')
                    a++;
                v = (*a == 0) ^ (t[3] == 'n');
            }
            else if (run && !Eval(s, t + 3, &v))
                return FALSE;
            top++;
            outer[top] = run;
            taking[top] = run && v;
            taken[top] = taking[top];
            continue;
        }
        if (strncmp(t, ".elseif", 7) == 0)
        {
            s32 v = 0;
            if (!top)
                return FALSE;
            if (outer[top] && !taken[top] && !Eval(s, t + 7, &v))
                return FALSE;
            taking[top] = outer[top] && !taken[top] && v;
            taken[top] |= taking[top];
            continue;
        }
        if (strncmp(t, ".else", 5) == 0)
        {
            if (!top)
                return FALSE;
            taking[top] = outer[top] && !taken[top];
            taken[top] = TRUE;
            continue;
        }
        if (strncmp(t, ".endif", 6) == 0)
        {
            if (top)
                top--;
            continue;
        }
        if (!taking[top])
            continue;
        if (!Line(s, t, depth))
            return FALSE;
    }
    return TRUE;
}

// One `raw` line of an event: a macro of the game's scripts with its arguments, or an assembler line (.byte ...)
bool8 Maker_RawLine(struct MakerRawSink *s, const char *text)
{
    char buf[2048];   // (a decompiled text is inlined as a quoted string: room for the longest)
    snprintf(buf, sizeof(buf), "%s", text);
    return Line(s, buf, 0);
}

void Maker_RawReset(void)   // a new assembly: the symbols .set by the last one are gone
{
    sSetCount = 0;
    {
        s32 v;
        if (!Maker_RawSymbol("_last_implicit_waitstate", &v))
        {
            snprintf(sSets[0].name, sizeof(sSets[0].name), "_last_implicit_waitstate");
            sSets[0].value = -1;
            sSetCount = 1;
        }
    }
}
