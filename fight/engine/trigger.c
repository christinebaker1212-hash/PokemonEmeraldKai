/*
 * CNS triggers: parser, evaluator and the expression stack helpers
 * (original module 0x453810-0x458C2F).
 *
 * Trigger_Parse reads one trigger call for Expr_Parse: an optional
 * redirection ("parent, ", "helper(ID), "...), the name, the arguments of
 * function triggers (compiled as postfix tokens placed before the trigger
 * token) and, for the few triggers that compare by themselves (command,
 * animelem, statetype, string triggers...), their "= value" part.
 * Trigger_Eval resolves the redirection, pops the arguments from the
 * evaluation stack and pushes the value; when something is missing it pushes
 * "bottom" (EXPR_BOTTOM).
 *
 * The module ends with the expression stack underflow helper: the code from
 * 0x458C30 on (compile-a-parameter helpers, Cns_ParseArgs, the AssertSpecial
 * flag names...) belongs to the next unit, the CNS parse helpers shared with
 * the state controller parser (its string literals follow ours in .rdata).
 *
 * Quirks kept from the original (see the functions for details):
 * - Trigger_Copy clones the redirection argument over a bitwise copy of it, so
 *   Expr_Clone first frees the token arrays of the source (use after free for
 *   computed redirection arguments). Copying an authorname or stagevar trigger
 *   is a fatal "Unrecognized trigger data block".
 * - ifelse tests its operands' types against 3 and 2 instead of EXPR_BOTTOM and
 *   EXPR_FLOAT: a float condition is tested by its bits, bottom is not propagated.
 * - Redirected life/power read the team leader of the caller when GameType is
 *   not "VS"; pos z of 1.0 characters is 0; Trigger_Parse accepts "z" only for
 *   pre-1.0 characters.
 * - const() and stagevar() accept anything but the end of the string where the
 *   closing parenthesis should be.
 * - Names and gethitvar/const/stagevar words are copied without a length check
 *   (stack buffers of 0x100 bytes, as in the original; the layout differs).
 */
#include "mugen.h"

/* Triggers that compare by themselves (Trigger.op/value), see Trigger_Eval. */
static int Trigger_SelfCompares(const Trigger *t)
{
    int id = t->id;

    return id == TRIGGER_COMMAND || id == TRIGGER_ANIMELEM || (id >= TRIGGER_P1NAME && id <= TRIGGER_P4NAME) ||
           id == TRIGGER_STATETYPE || id == TRIGGER_P2STATETYPE || id == TRIGGER_MOVETYPE ||
           id == TRIGGER_P2MOVETYPE || id == TRIGGER_TIMEMOD || id == TRIGGER_PROJHIT || id == TRIGGER_PROJCONTACT ||
           id == TRIGGER_PROJGUARDED || id == TRIGGER_AUTHORNAME || id == TRIGGER_TEAMMODE ||
           Trigger_IsStageVarString(t);
}

/* isspace() as the original inlines it: only for 1..127. */
static int is_blank(int c)
{
    return (unsigned int)(c - 1) <= 0x7E && isspace(c);
}

/* ------------------------------------------------------------ x87 math */

enum { CI_SIN, CI_COS, CI_TAN, CI_ASIN, CI_ACOS, CI_ATAN, CI_LOG, CI_EXP };

static float Trigger_Math(unsigned int fn, float x)
{
    double v = x;

    switch (fn) {
    case CI_SIN:
        return (float)sin(v);
    case CI_COS:
        return (float)cos(v);
    case CI_TAN:
        return (float)tan(v);
    case CI_ASIN:
        return (float)asin(v);
    case CI_ACOS:
        return (float)acos(v);
    case CI_ATAN:
        return (float)atan(v);
    case CI_LOG:
        return (float)log(v);
    default:
        return (float)exp(v);
    }
}

static float Trigger_LogBase(float x, float base)
{
    double lx = log((double)x);

    return (float)(lx / log((double)base));
}

/* ------------------------------------------------------------ evaluation stack */

/* 0x00453810 */
void ExprStack_Push(int value, int type)
{
    int top = g_exprStackTop;

    if (top >= EXPR_STACK_SIZE - 1) {
        sprintf(g_libraryErrorText, "Expression stack overflow.");
        Error_Exit();
    }
    top++;
    g_exprStackTop = top;
    g_exprStackValue[top].i = value;
    g_exprStackType[top] = type;
}

/* 0x00453860: 0 when the stack is empty. */
int ExprStack_Pop(int *value, int *type)
{
    int top = g_exprStackTop;

    if (top <= -1) {
        return 0;
    }
    *value = g_exprStackValue[top].i;
    g_exprStackTop = top - 1;
    *type = g_exprStackType[top];
    return 1;
}

/* 0x00453890: stagevar(info.name / info.displayname / info.author) compares a string. */
int Trigger_IsStageVarString(const Trigger *t)
{
    return t->id == TRIGGER_STAGEVAR && (t->arg == TRIGGER_STAGEVAR_NAME || t->arg == TRIGGER_STAGEVAR_DISPLAYNAME ||
                                         t->arg == TRIGGER_STAGEVAR_AUTHOR);
}

/* 0x00458C10 */
void ExprStack_Underflow(void)
{
    sprintf(g_libraryErrorText, "Fatal: Expression stack underflow.");
    Error_Exit();
}

/* 0x00458B00: pops an int argument. Returns 1, 2 for a float (truncated, warning when p), 3 for bottom. */
int Trigger_PopInt(Player *p, int *out)
{
    int top = g_exprStackTop;
    ExprValue v;
    int type;

    if (top <= -1) {
        sprintf(g_libraryErrorText, "Fatal: Expression stack underflow.");
        Error_Exit();
    }
    v = g_exprStackValue[top];
    type = g_exprStackType[top];
    g_exprStackTop = top - 1;
    if (type == EXPR_FLOAT) {
        *out = (int)v.f;
        if (p != NULL) {
            Player_Warn(p, "trigger arg truncated to int");
        }
        return 2;
    }
    *out = v.i;
    return type == EXPR_BOTTOM ? 3 : 1;
}

/* 0x00458BB0: pops a float argument (ints converted). Returns 2, or 3 for bottom. */
int Trigger_PopFloat(float *out)
{
    int top = g_exprStackTop;
    ExprValue v;
    int type;

    if (top <= -1) {
        sprintf(g_libraryErrorText, "Fatal: Expression stack underflow.");
        Error_Exit();
    }
    v = g_exprStackValue[top];
    type = g_exprStackType[top];
    g_exprStackTop = top - 1;
    if (type == EXPR_FLOAT) {
        *out = v.f;
        return 2;
    }
    *out = (float)v.i;
    return type == EXPR_BOTTOM ? 3 : 2;
}

/* ------------------------------------------------------------ parsing */

/* 0x004561B0: arguments a trigger pops from the evaluation stack. */
int Trigger_ArgCount(int id)
{
    if ((unsigned int)(id - TRIGGER_SIN) <= 0x1F) {
        return 1;
    }
    if (id == TRIGGER_LOG) {
        return 2;
    }
    return id == TRIGGER_IFELSE ? 3 : 0;
}

/*
 * 0x00455E60: the comparison after a trigger that compares by itself:
 * "= v", "!= v", "> v", ">= v", "< v", "<= v", "v" (as "= v"), and the ranges
 * "= [a, b]", "= (a, b)" (mixed brackets too). Values are ints or floats as
 * t->is_float says. Returns 0 if nothing usable was found.
 */
int Trigger_ParseCompare(const char *s, Trigger *t, const char **end)
{
    int kind = s[0];
    int next = s[1];
    int i = 0;
    int c;
    const char *p;

    if (s[0] == '\0') {
        return 0;
    }
    if (((unsigned int)(kind - 1) <= 0x7E && isdigit(kind)) || kind == '-') {
        kind = '=';
    }
    /* Up to the first digit or '-'; "= (" and "= [" make it a range. */
    while (s[i] != '\0') {
        c = s[i];
        if ((unsigned int)(c - 1) <= 0x7E && isdigit(c)) {
            break;
        }
        if (s[i] == '-') {
            break;
        }
        i++;
        if (kind == '=') {
            if (s[i] == '(') {
                kind = '(';
            } else if (s[i] == '[') {
                kind = '[';
            }
        }
    }
    p = s + i;
    if (*p == '\0') {
        return 0;
    }
    switch (kind) {
    case '=':
        t->op = TRIGGER_OP_EQ;
        break;
    case '!':
        if (next != '=') {
            return 0;
        }
        t->op = TRIGGER_OP_NE;
        break;
    case '>':
        t->op = TRIGGER_OP_GT;
        if (next == '=') {
            t->op = TRIGGER_OP_GE;
        }
        break;
    case '<':
        t->op = TRIGGER_OP_LT;
        if (next == '=') {
            t->op = TRIGGER_OP_LE;
        }
        break;
    case '(':
    case '[':
        if (t->is_float == 0) {
            t->value = atoi(p);
        } else {
            t->fvalue = (float)atof(p);
        }
        while (s[i] != '\0' && s[i] != ',') {
            i++;
        }
        while (s[i] != '\0' && (s[i] < '0' || s[i] > '9') && s[i] != '-') {
            i++;
        }
        p = s + i;
        if (t->is_float == 0) {
            t->value2 = atoi(p);
        } else {
            t->fvalue2 = (float)atof(p);
        }
        while (s[i] != '\0' && s[i] != ')' && s[i] != ']') {
            i++;
        }
        *end = s + i + 1;
        c = s[i];
        if (kind == '[') {
            if (c == ']') {
                t->op = TRIGGER_OP_IN_CC;
                return 1;
            }
            if (c == ')') {
                t->op = TRIGGER_OP_IN_CO;
                return 1;
            }
        } else if (kind == '(') {
            if (c == ']') {
                t->op = TRIGGER_OP_IN_OC;
                return 1;
            }
            if (c == ')') {
                t->op = TRIGGER_OP_IN_OO;
                return 1;
            }
        }
        return 0;
    default:
        return 0;
    }
    if (t->is_float == 0) {
        t->value = strtol(p, (char **)end, 10);
    } else {
        t->fvalue = (float)strtod(p, (char **)end);
    }
    return 1;
}

/* Trigger names, in the order the original tests them (first match wins). */
enum {
    NAME_EXACT = 0,
    NAME_PREFIX = 1,            /* Str_PrefixMatchI on len characters */
    NAME_BODY = 2               /* exact; frontedgebodydist/backedgebodydist (arg = 1) */
};

typedef struct TriggerName {
    const char *name;
    int kind;
    int len;
    int id;
} TriggerName;

static const TriggerName trigger_names[] = {
    { "time", NAME_EXACT, 0, TRIGGER_TIME },
    { "statetime", NAME_EXACT, 0, TRIGGER_TIME },
    { "animtime", NAME_EXACT, 0, TRIGGER_ANIMTIME },
    { "animelem", NAME_EXACT, 0, TRIGGER_ANIMELEM },
    { "anim", NAME_EXACT, 0, TRIGGER_ANIM },
    { "vel", NAME_EXACT, 0, TRIGGER_VEL },
    { "pos", NAME_EXACT, 0, TRIGGER_POS },
    { "screenpos", NAME_EXACT, 0, TRIGGER_SCREENPOS },
    { "camerapos", NAME_EXACT, 0, TRIGGER_CAMERAPOS },
    { "camerazoom", NAME_EXACT, 0, TRIGGER_CAMERAZOOM },
    { "timemod", NAME_EXACT, 0, TRIGGER_TIMEMOD },
    { "p2dist", NAME_EXACT, 0, TRIGGER_P2DIST },
    { "p2bodydist", NAME_EXACT, 0, TRIGGER_P2BODYDIST },
    { "p2life", NAME_EXACT, 0, TRIGGER_P2LIFE },
    { "winperfect", NAME_EXACT, 0, TRIGGER_WINPERFECT },
    { "win", NAME_PREFIX, 3, TRIGGER_WIN },
    { "lose", NAME_PREFIX, 4, TRIGGER_LOSE },
    { "drawgame", NAME_EXACT, 0, TRIGGER_DRAWGAME },
    { "matchover", NAME_EXACT, 0, TRIGGER_MATCHOVER },
    { "gametime", NAME_EXACT, 0, TRIGGER_GAMETIME },
    { "roundno", NAME_EXACT, 0, TRIGGER_ROUNDNO },
    { "roundstate", NAME_EXACT, 0, TRIGGER_ROUNDSTATE },
    { "matchno", NAME_EXACT, 0, TRIGGER_MATCHNO },
    { "roundsexisted", NAME_EXACT, 0, TRIGGER_ROUNDSEXISTED },
    { "teamside", NAME_EXACT, 0, TRIGGER_TEAMSIDE },
    { "teammode", NAME_EXACT, 0, TRIGGER_TEAMMODE },
    { "ishometeam", NAME_EXACT, 0, TRIGGER_ISHOMETEAM },
    { "frontedgedist", NAME_EXACT, 0, TRIGGER_FRONTEDGEDIST },
    { "backedgedist", NAME_EXACT, 0, TRIGGER_BACKEDGEDIST },
    { "frontedgebodydist", NAME_BODY, 0, TRIGGER_FRONTEDGEDIST },
    { "backedgebodydist", NAME_BODY, 0, TRIGGER_BACKEDGEDIST },
    { "life", NAME_EXACT, 0, TRIGGER_LIFE },
    { "lifemax", NAME_EXACT, 0, TRIGGER_LIFEMAX },
    { "power", NAME_EXACT, 0, TRIGGER_POWER },
    { "powermax", NAME_EXACT, 0, TRIGGER_POWERMAX },
    { "alive", NAME_EXACT, 0, TRIGGER_ALIVE },
    { "command", NAME_EXACT, 0, TRIGGER_COMMAND },
    { "ailevel", NAME_EXACT, 0, TRIGGER_AILEVEL },
    { "hitcount", NAME_EXACT, 0, TRIGGER_HITCOUNT },
    { "uniqhitcount", NAME_EXACT, 0, TRIGGER_UNIQHITCOUNT },
    { "movehit", NAME_EXACT, 0, TRIGGER_MOVEHIT },
    { "moveguarded", NAME_EXACT, 0, TRIGGER_MOVEGUARDED },
    { "movecontact", NAME_EXACT, 0, TRIGGER_MOVECONTACT },
    { "movereversed", NAME_EXACT, 0, TRIGGER_MOVEREVERSED },
    { "inguarddist", NAME_EXACT, 0, TRIGGER_INGUARDDIST },
    { "stateno", NAME_EXACT, 0, TRIGGER_STATENO },
    { "prevstateno", NAME_EXACT, 0, TRIGGER_PREVSTATENO },
    { "p2stateno", NAME_EXACT, 0, TRIGGER_P2STATENO },
    { "statetype", NAME_EXACT, 0, TRIGGER_STATETYPE },
    { "p2statetype", NAME_EXACT, 0, TRIGGER_P2STATETYPE },
    { "movetype", NAME_EXACT, 0, TRIGGER_MOVETYPE },
    { "p2movetype", NAME_EXACT, 0, TRIGGER_P2MOVETYPE },
    { "ctrl", NAME_EXACT, 0, TRIGGER_CTRL },
    { "hitdefattr", NAME_EXACT, 0, TRIGGER_HITDEFATTR },
    { "var", NAME_EXACT, 0, TRIGGER_VAR },
    { "fvar", NAME_EXACT, 0, TRIGGER_FVAR },
    { "sysvar", NAME_EXACT, 0, TRIGGER_SYSVAR },
    { "sysfvar", NAME_EXACT, 0, TRIGGER_SYSFVAR },
    { "random", NAME_EXACT, 0, TRIGGER_RANDOM },
    { "numproj", NAME_EXACT, 0, TRIGGER_NUMPROJ },
    { "numprojid", NAME_EXACT, 0, TRIGGER_NUMPROJID },
    { "projcanceltime", NAME_EXACT, 0, TRIGGER_PROJCANCELTIME },
    { "projhittime", NAME_EXACT, 0, TRIGGER_PROJHITTIME },
    { "projhit", NAME_PREFIX, 7, TRIGGER_PROJHIT },
    { "projguardedtime", NAME_EXACT, 0, TRIGGER_PROJGUARDEDTIME },
    { "projguarded", NAME_PREFIX, 11, TRIGGER_PROJGUARDED },
    { "projcontacttime", NAME_EXACT, 0, TRIGGER_PROJCONTACTTIME },
    { "projcontact", NAME_PREFIX, 11, TRIGGER_PROJCONTACT },
    { "p1name", NAME_EXACT, 0, TRIGGER_P1NAME },
    { "name", NAME_EXACT, 0, TRIGGER_P1NAME },
    { "p2name", NAME_EXACT, 0, TRIGGER_P2NAME },
    { "p3name", NAME_EXACT, 0, TRIGGER_P3NAME },
    { "p4name", NAME_EXACT, 0, TRIGGER_P4NAME },
    { "authorname", NAME_EXACT, 0, TRIGGER_AUTHORNAME },
    { "numpartner", NAME_EXACT, 0, TRIGGER_NUMPARTNER },
    { "numenemy", NAME_EXACT, 0, TRIGGER_NUMENEMY },
    { "id", NAME_EXACT, 0, TRIGGER_ID },
    { "facing", NAME_EXACT, 0, TRIGGER_FACING },
    { "hitfall", NAME_EXACT, 0, TRIGGER_HITFALL },
    { "hitshakeover", NAME_EXACT, 0, TRIGGER_HITSHAKEOVER },
    { "hitover", NAME_EXACT, 0, TRIGGER_HITOVER },
    { "hitvel", NAME_EXACT, 0, TRIGGER_HITVEL },
    { "hitpausetime", NAME_EXACT, 0, TRIGGER_HITPAUSETIME },
    { "canrecover", NAME_EXACT, 0, TRIGGER_CANRECOVER },
    { "animelemno", NAME_EXACT, 0, TRIGGER_ANIMELEMNO },
    { "animexist", NAME_EXACT, 0, TRIGGER_ANIMEXIST },
    { "selfanimexist", NAME_EXACT, 0, TRIGGER_SELFANIMEXIST },
    { "playeridexist", NAME_EXACT, 0, TRIGGER_PLAYERIDEXIST },
    { "palno", NAME_EXACT, 0, TRIGGER_PALNO },
    { "numexplod", NAME_EXACT, 0, TRIGGER_NUMEXPLOD },
    { "numtarget", NAME_EXACT, 0, TRIGGER_NUMTARGET },
    { "gethitvar", NAME_EXACT, 0, TRIGGER_GETHITVAR },
    { "const", NAME_EXACT, 0, TRIGGER_CONST },
    { "const240p", NAME_EXACT, 0, TRIGGER_CONST240P },
    { "const480p", NAME_EXACT, 0, TRIGGER_CONST480P },
    { "const720p", NAME_EXACT, 0, TRIGGER_CONST720P },
    { "stagevar", NAME_EXACT, 0, TRIGGER_STAGEVAR },
    { "tickspersecond", NAME_EXACT, 0, TRIGGER_TICKSPERSECOND },
    { "ishelper", NAME_EXACT, 0, TRIGGER_ISHELPER },
    { "numhelper", NAME_EXACT, 0, TRIGGER_NUMHELPER },
    { "parentdist", NAME_EXACT, 0, TRIGGER_PARENTDIST },
    { "rootdist", NAME_EXACT, 0, TRIGGER_ROOTDIST },
    { "standby", NAME_EXACT, 0, TRIGGER_STANDBY },
    { "gamewidth", NAME_EXACT, 0, TRIGGER_GAMEWIDTH },
    { "gameheight", NAME_EXACT, 0, TRIGGER_GAMEHEIGHT },
    { "screenwidth", NAME_EXACT, 0, TRIGGER_SCREENWIDTH },
    { "screenheight", NAME_EXACT, 0, TRIGGER_SCREENHEIGHT },
    { "backedge", NAME_EXACT, 0, TRIGGER_BACKEDGE },
    { "frontedge", NAME_EXACT, 0, TRIGGER_FRONTEDGE },
    { "leftedge", NAME_EXACT, 0, TRIGGER_LEFTEDGE },
    { "rightedge", NAME_EXACT, 0, TRIGGER_RIGHTEDGE },
    { "topedge", NAME_EXACT, 0, TRIGGER_TOPEDGE },
    { "bottomedge", NAME_EXACT, 0, TRIGGER_BOTTOMEDGE },
    { "pi", NAME_EXACT, 0, TRIGGER_PI },
    { "e", NAME_EXACT, 0, TRIGGER_E },
    { "ifelse", NAME_EXACT, 0, TRIGGER_IFELSE },
    { "log", NAME_EXACT, 0, TRIGGER_LOG },
    { "sin", NAME_EXACT, 0, TRIGGER_SIN },
    { "cos", NAME_EXACT, 0, TRIGGER_COS },
    { "tan", NAME_EXACT, 0, TRIGGER_TAN },
    { "asin", NAME_EXACT, 0, TRIGGER_ASIN },
    { "acos", NAME_EXACT, 0, TRIGGER_ACOS },
    { "atan", NAME_EXACT, 0, TRIGGER_ATAN },
    { "exp", NAME_EXACT, 0, TRIGGER_EXP },
    { "ln", NAME_EXACT, 0, TRIGGER_LN },
    { "abs", NAME_EXACT, 0, TRIGGER_ABS },
    { "floor", NAME_EXACT, 0, TRIGGER_FLOOR },
    { "ceil", NAME_EXACT, 0, TRIGGER_CEIL },
    { "animelemtime", NAME_EXACT, 0, TRIGGER_ANIMELEMTIME },
    { "cond", NAME_EXACT, 0, TRIGGER_COND },
};
#define TRIGGER_NAME_COUNT (int)(sizeof(trigger_names) / sizeof(trigger_names[0]))

/* Copies an identifier ([A-Za-z0-9_], or with '.' for gethitvar/const/stagevar words) without a length check. */
static int read_word(const char *s, int pos, char *out, int with_dot, int *len)
{
    int n = 0;
    char c;

    for (;;) {
        c = s[pos];
        if (!isalnum(c) && c != '_' && (!with_dot || s[pos] != '.')) {
            break;
        }
        out[n++] = s[pos];
        pos++;
    }
    out[n] = '\0';
    *len = n;
    return pos;
}

/*
 * 0x004538C0: parses one trigger call at s (for Expr_Parse): writes the
 * postfix tokens of its arguments, then the trigger token, to tokens/types
 * (at most max) and sets *end after the call. Returns the token count, 0 on
 * errors (some reported with Error_Printf).
 */
int Trigger_Parse(const char *s, CharData *owner, ExprValue *tokens, char *types, int max, const char **end)
{
    char name[0x100];
    char word[0x100];
    char arg_types[EXPR_MAX_TOKENS];
    ExprValue arg_tokens[EXPR_MAX_TOKENS];
    Trigger t;
    Trigger *trig;
    const char *e = NULL;
    Expr *exprs = NULL;
    unsigned int need;
    int ntok = 0;               /* argument tokens */
    int count = 0;
    int pos, len, id, n, i, c;

    memset(&t, 0, sizeof(t));
    pos = read_word(s, 0, name, 0, &len);
    while (is_blank(s[pos])) {
        pos++;
    }

    /* Redirection: "parent, ", "root, ", "helper(ID), "... */
    t.redirect = TRIGGER_REDIRECT_NONE;
    if (_stricmp(name, "parent") == 0) {
        t.redirect = TRIGGER_REDIRECT_PARENT;
    } else if (_stricmp(name, "root") == 0) {
        t.redirect = TRIGGER_REDIRECT_ROOT;
    } else {
        if (_stricmp(name, "helper") == 0) {
            t.redirect = TRIGGER_REDIRECT_HELPER;
        } else if (_stricmp(name, "target") == 0) {
            t.redirect = TRIGGER_REDIRECT_TARGET;
        } else if (_stricmp(name, "partner") == 0) {
            t.redirect = TRIGGER_REDIRECT_PARTNER;
        }
        if (t.redirect != TRIGGER_REDIRECT_NONE) {
            /* default ID 0 (target: -1); Expr_SetInt inlined, only the type byte is written */
            if (t.redirect_arg.count != EXPR_CONST && t.redirect_arg.count != 0) {
                Expr_Free(&t.redirect_arg);
            }
            t.redirect_arg.count = EXPR_CONST;
            t.redirect_arg.u.value.i = t.redirect == TRIGGER_REDIRECT_TARGET ? -1 : 0;
            t.redirect_arg.t.type = EXPR_INT;
        } else if (_stricmp(name, "enemy") == 0) {
            t.redirect = TRIGGER_REDIRECT_ENEMY;
            Expr_SetInt(&t.redirect_arg, 0);
        } else if (_stricmp(name, "enemynear") == 0) {
            t.redirect = TRIGGER_REDIRECT_ENEMYNEAR;
            Expr_SetInt(&t.redirect_arg, 0);
        } else if (_stricmp(name, "playerid") == 0) {
            t.redirect = TRIGGER_REDIRECT_PLAYERID;
            Expr_SetInt(&t.redirect_arg, -1);
        }
        if (t.redirect != TRIGGER_REDIRECT_NONE) {
            if (s[pos] == '(') {
                if (!Expr_Compile(s + pos + 1, &t.redirect_arg, owner, &e, EXPR_WANT_INT)) {
                    return 0;
                }
                if (*e != ')') {
                    return 0;
                }
                pos = (int)(e - s) + 1;
            } else if (t.redirect == TRIGGER_REDIRECT_PLAYERID) {
                return 0;
            }
        }
    }
    if (t.redirect != TRIGGER_REDIRECT_NONE) {
        /* ", name": s now starts at the trigger name */
        s += pos;
        while (is_blank(*s)) {
            s++;
        }
        if (*s != ',') {
            return 0;
        }
        do {
            s++;
        } while (is_blank(*s));
        pos = read_word(s, 0, name, 0, &len);
    }

    /* The trigger name. */
    id = -1;
    for (i = 0; i < TRIGGER_NAME_COUNT; i++) {
        const TriggerName *tn = &trigger_names[i];
        if (tn->kind == NAME_PREFIX ? Str_PrefixMatchI(name, tn->name, tn->len) : _stricmp(name, tn->name) == 0) {
            id = tn->id;
            if (tn->kind == NAME_BODY) {
                t.arg = 1;
            }
            break;
        }
    }
    if (id == -1) {
        Error_Printf("Invalid trigger: %s\n", name);
        return 0;
    }
    t.id = id;
    need = 0;
    for (i = 0; g_triggersV10[i] != -1; i++) {
        if (id == g_triggersV10[i]) {
            need = 0x01000000;
            break;
        }
    }
    for (i = 0; g_triggersV11[i] != -1; i++) {
        if (id == g_triggersV11[i]) {
            need = 0x01010000;
            break;
        }
    }
    if (owner->mugenversion < need) {
        Error_Printf("Trigger not valid for mugenversion of character: %s\n", name);
        return 0;
    }
    while (is_blank(s[pos])) {
        pos++;
    }

    /* Function forms: ishelper(ID), numhelper(ID), numexplod(ID), numtarget(ID). */
    if (t.id == TRIGGER_ISHELPER || t.id == TRIGGER_NUMHELPER || t.id == TRIGGER_NUMEXPLOD ||
        t.id == TRIGGER_NUMTARGET) {
        if (s[pos] != '(') {
            goto no_args;
        }
        t.id = t.id == TRIGGER_ISHELPER ? TRIGGER_ISHELPER_ID
               : t.id == TRIGGER_NUMHELPER ? TRIGGER_NUMHELPER_ID
               : t.id == TRIGGER_NUMEXPLOD ? TRIGGER_NUMEXPLOD_ID
                                           : TRIGGER_NUMTARGET_ID;
        goto one_arg;
    }
    if (t.id >= TRIGGER_SIN && t.id <= TRIGGER_CONST720P) {
    one_arg:
        if (s[pos] != '(') {
            return 0;
        }
        if (Cns_ParseArgs(s + pos + 1, "n:)", owner, &e, arg_tokens, arg_types, &count, max) != 1) {
            return 0;
        }
        if (*e != ')') {
            return 0;
        }
        ntok = count;
        pos = (int)(e - s) + 1;
    }
    if (t.id == TRIGGER_LOG) {
        while (is_blank(s[pos])) {
            pos++;
        }
        if (s[pos] != '(') {
            return 0;
        }
        if (Cns_ParseArgs(s + pos + 1, "nn:)", owner, &e, arg_tokens + ntok, arg_types + ntok, &count, max - ntok) !=
            2) {
            return 0;
        }
        if (*e != ')') {
            return 0;
        }
        ntok += count;
        pos = (int)(e - s) + 1;
    }
    if (t.id == TRIGGER_IFELSE) {
        while (is_blank(s[pos])) {
            pos++;
        }
        if (s[pos] != '(') {
            return 0;
        }
        if (Cns_ParseArgs(s + pos + 1, "nnn:)", owner, &e, arg_tokens + ntok, arg_types + ntok, &count, max - ntok) !=
            3) {
            return 0;
        }
        if (*e != ')') {
            return 0;
        }
        ntok += count;
        pos = (int)(e - s) + 1;
    }
    if (t.id == TRIGGER_COND) {
        while (is_blank(s[pos])) {
            pos++;
        }
        if (s[pos] != '(') {
            return 0;
        }
        n = Cns_ParseExprList(s + pos + 1, owner, &e, &exprs);
        if (n != 3 || *e != ')') {
            if (n <= 0) {
                return 0;
            }
            for (i = 0; i < n; i++) {
                Expr_Free(&exprs[i]);
            }
            free(exprs);
            return 0;
        }
        t.data_count = n;
        t.data = exprs;
        ntok = 0;
        pos = (int)(e - s) + 1;
    }
    if (t.id == TRIGGER_PROJHIT) {
        t.arg = atoi(name + 7);
    }
    if (t.id == TRIGGER_PROJGUARDED || t.id == TRIGGER_PROJCONTACT) {
        t.arg = atoi(name + 11);
    }
    if (t.id == TRIGGER_WIN || t.id == TRIGGER_LOSE) {
        const char *kind = name + (t.id == TRIGGER_WIN ? 3 : 4);
        if (Str_PrefixMatchI(kind, "ko", 2)) {
            t.arg = TRIGGER_WIN_KO;
        } else if (Str_PrefixMatchI(kind, "time", 2)) {
            t.arg = TRIGGER_WIN_TIME;
        } else if (kind[0] == '\0') {
            t.arg = TRIGGER_WIN_ANY;
        } else {
            Error_Printf(t.id == TRIGGER_WIN ? "Invalid win trigger: %s\n" : "Invalid lose trigger: %s\n", name);
            return 0;
        }
    }
    if (t.id == TRIGGER_GETHITVAR) {
        count = 0;
        if (s[pos] != '(') {
            Error_Printf("Need to specify gethitvar(varname): %s\n", name);
            return 0;
        }
        do {
            pos++;
        } while (is_blank(s[pos]));
        pos = read_word(s, pos, word, 1, &len);
        while (is_blank(s[pos])) {
            pos++;
        }
        if (s[pos] != ')') {
            Error_Printf("Missing closing parenthesis: %s\n", name);
            return 0;
        }
        _strlwr(word);
        if (!GetHitVar_Lookup(word, owner->mugenversion < g_game->compat_version[1], &t.value, &count)) {
            Error_Printf("Invalid varname %s in: %s\n", word, name);
            return 0;
        }
        if (count != 0) {
            t.is_float = 1;
        }
        pos++;
    }
    if (t.id == TRIGGER_CONST) {
        if (s[pos] != '(') {
            Error_Printf("Need to specify const(varname): %s\n", name);
            return 0;
        }
        do {
            pos++;
        } while (is_blank(s[pos]));
        pos = read_word(s, pos, word, 1, &len);
        while (is_blank(s[pos])) {
            pos++;
        }
        /* Any character is taken for the closing parenthesis (original bug). */
        if (s[pos] == '\0') {
            Error_Printf("Missing closing parenthesis: %s\n", name);
            return 0;
        }
        _strlwr(word);
        for (i = 0; i < TRIGGER_CONST_COUNT; i++) {
            if (strcmp(word, g_triggerConsts[i].name) == 0) {
                t.value = i;
                t.is_float = g_triggerConsts[i].is_float == 1;
                break;
            }
        }
        if (i == TRIGGER_CONST_COUNT) {
            Error_Printf("Invalid cnstname %s in: %s\n", word, name);
            return 0;
        }
        pos++;
    }
    if (t.id == TRIGGER_STAGEVAR) {
        if (s[pos] != '(') {
            Error_Printf("Need to specify stagevar(varname): %s\n", name);
            return 0;
        }
        do {
            pos++;
        } while (is_blank(s[pos]));
        pos = read_word(s, pos, word, 1, &len);
        while (is_blank(s[pos])) {
            pos++;
        }
        if (s[pos] == '\0') {
            Error_Printf("Missing closing parenthesis: %s\n", name);
            return 0;
        }
        _strlwr(word);
        if (strcmp(word, "info.name") == 0) {
            t.arg = TRIGGER_STAGEVAR_NAME;
        } else if (strcmp(word, "info.displayname") == 0) {
            t.arg = TRIGGER_STAGEVAR_DISPLAYNAME;
        } else if (strcmp(word, "info.author") == 0) {
            t.arg = TRIGGER_STAGEVAR_AUTHOR;
        } else {
            Error_Printf("Invalid varname %s in: %s\n", word, name);
            return 0;
        }
        pos++;
    }
    if (t.id != TRIGGER_GETHITVAR && t.id != TRIGGER_CONST && t.id != TRIGGER_STAGEVAR) {
    no_args:
        t.is_float = 0;
    }

    /* Result type and the parameters some triggers read before the end of the call. */
    if (t.id > TRIGGER_E) {
        switch (t.id) {
        case TRIGGER_LOG:
        case TRIGGER_SIN:
        case TRIGGER_COS:
        case TRIGGER_TAN:
        case TRIGGER_ASIN:
        case TRIGGER_ACOS:
        case TRIGGER_ATAN:
        case TRIGGER_EXP:
        case TRIGGER_LN:
        case TRIGGER_FVAR:
        case TRIGGER_SYSFVAR:
        case TRIGGER_CONST240P:
        case TRIGGER_CONST480P:
        case TRIGGER_CONST720P:
            t.is_float = 1;
            break;
        }
    } else if (t.id >= TRIGGER_PI) {
        t.is_float = 1;
    } else {
        switch (t.id) {
        case TRIGGER_POS:
        case TRIGGER_SCREENPOS:
        case TRIGGER_VEL:
        case TRIGGER_P2DIST:
        case TRIGGER_P2BODYDIST:
        case TRIGGER_CAMERAPOS:
        case TRIGGER_HITVEL:
        case TRIGGER_PARENTDIST:
        case TRIGGER_ROOTDIST:
            /* component: x, y, or z for pre-1.0 characters */
            t.is_float = 1;
            while (is_blank(s[pos])) {
                pos++;
            }
            c = Char_ToLower(s[pos]);
            if (c == 'x') {
                t.value = 0;
            } else if (c == 'y') {
                t.value = 1;
            } else if (c == 'z') {
                if (owner->mugenversion >= g_game->compat_version[1]) {
                    return 0;
                }
                t.value = 2;
            } else {
                return 0;
            }
            pos++;
            break;
        case TRIGGER_ANIMELEM:
        case TRIGGER_TIMEMOD:
            /* "= n" then an optional ", comparison" (parsed below) */
            while (s[pos] != '\0' && s[pos] != '=') {
                pos++;
            }
            if (s[pos] != '\0') {
                pos++;
            }
            while (is_blank(s[pos])) {
                pos++;
            }
            if (s[pos] < '0' || s[pos] > '9') {
                return 0;
            }
            t.arg = atoi(s + pos);
            if (t.id == TRIGGER_ANIMELEM) {
                t.arg--;
            }
            if (t.arg < 0) {
                if (t.id != TRIGGER_ANIMELEM) {
                    return 0;
                }
                Error_Printf("Anim must be > 0\n");
                return 0;
            }
            if (t.id == TRIGGER_TIMEMOD && t.arg <= 0) {
                return 0;
            }
            while (Char_IsDigit(s[pos]) || Char_IsSpace(s[pos])) {
                pos++;
            }
            if (s[pos] == ',') {
                pos++;
            }
            break;
        case TRIGGER_PROJHIT:
        case TRIGGER_PROJGUARDED:
        case TRIGGER_PROJCONTACT:
            /* "= 0/1" then an optional ", comparison" of the time */
            while (s[pos] != '\0' && s[pos] != '=') {
                pos++;
            }
            if (s[pos] != '\0') {
                pos++;
            }
            while (Char_IsSpace(s[pos])) {
                pos++;
            }
            if (!Char_IsDigit(s[pos])) {
                return 0;
            }
            t.arg2 = atoi(s + pos);
            while (Char_IsDigit(s[pos]) || Char_IsSpace(s[pos])) {
                pos++;
            }
            if (s[pos] == ',') {
                pos++;
            }
            break;
        case TRIGGER_CAMERAZOOM:
        case TRIGGER_GAMEWIDTH:
        case TRIGGER_GAMEHEIGHT:
        case TRIGGER_SCREENWIDTH:
        case TRIGGER_SCREENHEIGHT:
        case TRIGGER_BACKEDGE:
        case TRIGGER_FRONTEDGE:
        case TRIGGER_LEFTEDGE:
        case TRIGGER_RIGHTEDGE:
        case TRIGGER_TOPEDGE:
        case TRIGGER_BOTTOMEDGE:
            t.is_float = 1;
            break;
        }
    }
    while (is_blank(s[pos])) {
        pos++;
    }
    *end = s + pos;

    if (t.id == TRIGGER_COMMAND) {
        /* command = "name" (or !=) */
        t.op = 0;
        while (s[pos] != '\0' && s[pos] != '=') {
            pos++;
        }
        if (s[pos - 1] == '!') {
            t.op = 1;
        }
        if (s[pos] != '\0') {
            pos++;
        }
        while (is_blank(s[pos])) {
            pos++;
        }
        if (s[pos] != '"') {
            return 0;
        }
        if (!Cmd_FindLabel(s + pos + 1, &t.value, owner, end)) {
            Error_Printf("Undefined command label: \"%s\".\nIf not misspelling in CNS, check CMD\n", name);
            return 0;
        }
        if (**end != '"') {
            return 0;
        }
        *end = *end + 1;
        goto finish;
    }
    if (t.id == TRIGGER_STAGEVAR ? Trigger_IsStageVarString(&t)
                                 : t.id >= TRIGGER_P1NAME && t.id <= TRIGGER_AUTHORNAME) {
        /* name = "string" (or !=), compared without case */
        int start, stop;

        t.is_float = 0;
        t.op = 0;
        while (s[pos] != '\0' && s[pos] != '=') {
            pos++;
        }
        if (pos > 0 && s[pos - 1] == '!') {
            t.op = 1;
        }
        if (s[pos] != '\0') {
            pos++;
        }
        while (is_blank(s[pos])) {
            pos++;
        }
        if (s[pos] != '"') {
            _snprintf(g_libraryErrorText, 0x100, "String trigger needs to be in the form trig = \"string value\".");
            return 0;
        }
        start = pos + 1;
        for (stop = start; s[stop] != '\0' && s[stop] != '"'; stop++) {
        }
        if (s[stop] != '"') {
            return 0;
        }
        n = stop - start;
        t.data = Mem_Calloc(1, n + 1);
        if (t.data == NULL) {
            return 0;
        }
        strncpy((char *)t.data, s + start, n);
        ((char *)t.data)[n] = '\0';
        t.value = 1;
        *end = s + stop + 1;
        goto finish;
    }
    switch (t.id) {
    case TRIGGER_STATETYPE:
    case TRIGGER_P2STATETYPE:
        t.op = 0;
        while (s[pos] != '\0' && s[pos] != '=') {
            pos++;
        }
        if (s[pos - 1] == '!') {
            t.op = 1;
        }
        if (s[pos] != '\0') {
            pos++;
        }
        while (is_blank(s[pos])) {
            pos++;
        }
        c = s[pos];
        if ((unsigned int)(c - 1) <= 0x7E) {
            c = tolower(c);
        }
        switch (c) {
        case 'a':
            t.value = STATETYPE_AIR;
            break;
        case 'c':
            t.value = STATETYPE_CROUCH;
            break;
        case 'l':
            t.value = STATETYPE_LIEDOWN;
            break;
        case 's':
            t.value = STATETYPE_STAND;
            break;
        default:
            return 0;
        }
        *end = s + pos + 1;
        goto finish;
    case TRIGGER_MOVETYPE:
    case TRIGGER_P2MOVETYPE:
        t.op = 0;
        while (s[pos] != '\0' && s[pos] != '=') {
            pos++;
        }
        if (s[pos - 1] == '!') {
            t.op = 1;
        }
        if (s[pos] != '\0') {
            pos++;
        }
        while (is_blank(s[pos])) {
            pos++;
        }
        c = Char_ToLower(s[pos]);
        if (c == 'a') {
            t.value = MOVETYPE_ATTACK;
        } else if (c == 'h') {
            t.value = MOVETYPE_HIT;
        } else if (c == 'i') {
            t.value = MOVETYPE_IDLE;
        } else {
            return 0;
        }
        *end = s + pos + 1;
        goto finish;
    case TRIGGER_TEAMMODE: {
        /* teammode = single / simul / turns (or !=) */
        int stop;

        t.op = 0;
        while (s[pos] != '\0' && s[pos] != '=') {
            pos++;
        }
        if (s[pos - 1] == '!') {
            t.op = 1;
        }
        if (s[pos] != '\0') {
            pos++;
        }
        while (Char_IsSpace(s[pos])) {
            pos++;
        }
        stop = pos;
        if (s[pos] != '\0') {
            while (!Char_IsSpace(s[stop]) && s[stop] != ',' && s[stop] != ')') {
                stop++;
                if (s[stop] == '\0') {
                    break;
                }
            }
        }
        strncpy(word, s + pos, stop - pos);
        word[stop - pos] = '\0';
        _strlwr(word);
        if (strcmp(word, "single") == 0) {
            t.value = 0;
        } else if (strcmp(word, "simul") == 0) {
            t.value = 1;
        } else if (strcmp(word, "turns") == 0) {
            t.value = 3;
        } else {
            Error_Printf("Invalid TeamMode %s in %s\n", s + pos, name);
            return 0;
        }
        *end = s + stop;
        goto finish;
    }
    case TRIGGER_HITDEFATTR:
        t.op = 0;
        while (s[pos] != '\0' && s[pos] != '=') {
            pos++;
        }
        if (s[pos - 1] == '!') {
            t.op = 1;
        }
        if (s[pos] != '\0') {
            pos++;
        }
        while (Char_IsSpace(s[pos])) {
            pos++;
        }
        t.value = HitDefAttr_Parse(s + pos, end);
        if (t.value == -1) {
            return 0;
        }
        goto finish;
    case TRIGGER_ANIMELEM:
    case TRIGGER_PROJHIT:
    case TRIGGER_PROJGUARDED:
    case TRIGGER_PROJCONTACT:
        /* without a comparison: "= [0, 0]", i.e. the element starts / the contact happens this tick */
        if (!Trigger_ParseCompare(s + pos, &t, end)) {
            t.op = TRIGGER_OP_IN_CC;
            t.value2 = 0;
            t.value = 0;
        }
        break;
    case TRIGGER_TIMEMOD:
        if (!Trigger_ParseCompare(s + pos, &t, end)) {
            return 0;
        }
        break;
    }

finish:
    n = ntok + 1;
    if (n > max) {
        return 0;
    }
    memcpy(tokens, arg_tokens, ntok * sizeof(ExprValue));
    memcpy(types, arg_types, ntok);
    trig = calloc(1, sizeof(Trigger));
    if (trig == NULL) {
        g_outOfMemory = 1;
    }
    *trig = t;
    tokens[ntok].trig = trig;
    types[ntok] = EXPR_TRIGGER;
    return n;
}

/* ------------------------------------------------------------ copy / free */

/* 0x00458830 */
void Trigger_Free(Trigger *t)
{
    Expr *e;
    int i;

    if (t->id == TRIGGER_COND) {
        e = (Expr *)t->data;
        for (i = 0; i < t->data_count; i++) {
            Expr_Free(e);
            e++;
        }
        free(t->data);
    } else if (t->data != NULL) {
        free(t->data);
    }
    Expr_Free(&t->redirect_arg);
    free(t);
}

/*
 * 0x004588A0: deep copy of src into dst. The redirection argument is cloned
 * over the bitwise copy, so Expr_Clone frees the tokens it shares with src
 * before reading them (original bug, harmless for constant arguments).
 */
int Trigger_Copy(const Trigger *src, Trigger *dst)
{
    int i;

    *dst = *src;
    Expr_Clone(&dst->redirect_arg, &src->redirect_arg);
    if (src->data == NULL) {
        return 1;
    }
    if (src->id < TRIGGER_P1NAME) {
        goto unrecognized;
    }
    if (src->id <= TRIGGER_P4NAME) {
        dst->data = calloc(1, strlen((const char *)src->data) + 1);
        if (dst->data == NULL) {
            g_outOfMemory = 1;
        }
        if (dst->data != NULL) {
            strcpy((char *)dst->data, (const char *)src->data);
            return 1;
        }
    } else {
        if (src->id != TRIGGER_COND) {
            goto unrecognized;
        }
        dst->data = calloc(src->data_count, sizeof(Expr));
        if (dst->data == NULL) {
            g_outOfMemory = 1;
        }
        if (dst->data != NULL) {
            for (i = 0; i < src->data_count; i++) {
                Expr_Clone((Expr *)dst->data + i, (const Expr *)src->data + i);
            }
            return 1;
        }
    }
    sprintf(g_libraryErrorText, "Out of memory while attempting trigcpy");
    Error_Exit();

unrecognized:
    sprintf(g_libraryErrorText, "Unrecognized trigger data block -- engine's fault");
    Error_Exit();
}

/* ------------------------------------------------------------ trigger lists */

#define TRIGGER_FATAL_MALFORMED "%4i: Fatal: Player %s (%i) in state %i: malformed expression"

/* 0x004562C0: e evaluated as a condition (bottom is false). */
int Trigger_TestExpr(Player *p, const Expr *e)
{
    float fval;
    int ival;
    int r = Expr_Eval(p, e, &ival, &fval);

    if (r == 0) {
        sprintf(g_libraryErrorText, TRIGGER_FATAL_MALFORMED, g_game->frames, p->name, p->id, p->state_no);
        Error_Exit();
    }
    if (r != 1 && r != 3) {
        return fval != 0.0f;
    }
    return ival != 0;
}

/*
 * 0x004561F0: the triggers of a state controller, Exprs by (group, index):
 * every triggerall (group -1) must hold, then any group 0..count-1 whose
 * triggers all hold makes it true. A missing first trigger of a group is fatal.
 */
int Trigger_TestList(KeyedArray *list, Player *p, int count)
{
    const Expr *e;
    int group, i, slot;

    for (i = 0;; i++) {
        slot = KeyedArray_Find(list, -1, i);
        if (slot == -1) {
            break;
        }
        e = (const Expr *)(list->data + list->elem_size * slot);
        if (e == NULL) {
            break;
        }
        if (!Trigger_TestExpr(p, e)) {
            return 0;
        }
    }
    for (group = 0; group < count; group++) {
        i = 0;
        slot = KeyedArray_Find(list, group, 0);
        if (slot == -1 || (e = (const Expr *)(list->data + list->elem_size * slot)) == NULL) {
            Error_Fatal("TrigArrayTest bug\n");
        }
        while (Trigger_TestExpr(p, e)) {
            i++;
            slot = KeyedArray_Find(list, group, i);
            if (slot == -1) {
                return 1;
            }
            e = (const Expr *)(list->data + list->elem_size * slot);
            if (e == NULL) {
                return 1;
            }
        }
    }
    return 0;
}

/* ------------------------------------------------------------ screen helpers */

/*
 * 0x004589C0: camera position in screen space. The original leaves the pad
 * field of the result uninitialized (copied from its stack); here it is 0.
 */
Coord *Camera_GetPos(Coord *out)
{
    GameVars *g = g_game;
    double k = g_gameCoord[0] / g->stage.unit;
    Coord c;
    float x, y;

    c.type = 2;
    x = (float)(((double)g->camera_x - g->camera_x_base) * k + g->camera_offset_x);
    c.x = x;
    y = (float)(((double)g->camera_y - g->camera_y_base) * k + (double)g->camera_zoom * g->camera_offset_y);
    c.y = y;
    c.z = 0.0;
    c.unit = 1.0;
    c.pad_24 = 0;
    *out = c;
    return out;
}

/* 0x00458A50: y of the top of the screen in stage space. */
double Screen_GetTopEdge(void)
{
    Coord c;

    c.x = 0.0;
    c.y = 0.0;
    c.z = 0.0;
    c.unit = 1.0;
    c.type = 2;
    c.pad_24 = 0;
    return Coord_ScreenToStage(&c, c.x, c.y, c.z, c.unit, c.type, c.pad_24)->y;
}

/* 0x00458AA0: y of the bottom of the screen in stage space. */
double Screen_GetBottomEdge(void)
{
    Coord c;

    c.x = 0.0;
    c.y = (double)(int)g_game->game_coord[1];
    c.z = 0.0;
    c.unit = 1.0;
    c.type = 2;
    c.pad_24 = 0;
    return Coord_ScreenToStage(&c, c.x, c.y, c.z, c.unit, c.type, c.pad_24)->y;
}

/* ------------------------------------------------------------ evaluation */

/* Pushes bottom for a failed redirection, after dropping the trigger's arguments. */
static void Trigger_RedirectFailed(const Trigger *t)
{
    g_exprStackTop -= Trigger_ArgCount(t->id);
    ExprStack_Push(0, EXPR_BOTTOM);
}

/* Warns about p's trigger, naming the redirection target when the trigger was redirected. */
static void Trigger_Warn(Player *p, Player *target, int redirected, const char *msg)
{
    if (redirected) {
        Player_WarnRedirected(p, target, msg);
    } else {
        Player_Warn(target, msg);
    }
}

/* Values pushed from a float: the bits through the FPU, as the original copies them. */
static int float_bits(float f)
{
    ExprValue v;

    v.f = f;
    return v.i;
}

/*
 * 0x00456350: evaluates trigger t for player p (after redirection) and pushes
 * the value on the evaluation stack (its arguments, when it has some, are
 * popped first). Missing players, out of range indices and math errors push
 * bottom, mostly with a warning.
 */
void Trigger_Eval(Player *p, const Trigger *t)
{
    GameVars *g;
    Player *target;
    Player *p2 = NULL;          /* nearest enemy */
    Player *p3 = NULL;          /* partner */
    Player *p4 = NULL;          /* the other enemy */
    Player *leader = p;         /* team leader for life/power */
    Player *parent;             /* parent of a helper target, if still alive */
    Player *root;
    Player *e1, *e2, *other;
    AnimState *st;
    AnimAction *action;
    const Expr *expr;
    ExprValue v1, v2, v3;
    double d;
    float fval = 0.0f, f;
    int ival = 0, arg = 0, negate = 0, op = t->op;
    int redirected, helper, idx, r, type1, type2, type3, scaled, is_float;

    /* Redirection. */
    switch (t->redirect) {
    case TRIGGER_REDIRECT_PARENT:
        if (p->parent == NULL) {
            Player_Warn(p, "has no parent");
            Trigger_RedirectFailed(t);
            return;
        }
        if (p->parent->active == 0 || p->parent->id != p->parent_id) {
            Player_Warn(p, "Parent in retargeted trigger has been destroyed");
        }
        target = p->parent;
        break;
    case TRIGGER_REDIRECT_ROOT:
        target = p->root;
        if (target == NULL) {
            Player_Warn(p, "has no root");
            Trigger_RedirectFailed(t);
            return;
        }
        break;
    case TRIGGER_REDIRECT_HELPER:
        idx = Expr_EvalInt(p, &t->redirect_arg, 1);
        if (idx < 0) {
            idx = 0;
            Player_Warn(p, "Illegal helper ID in retargeted trigger");
        }
        target = Helper_Find(p, idx);
        if (target == NULL) {
            Player_Warnf(p, "has no helper %i", idx);
            Trigger_RedirectFailed(t);
            return;
        }
        break;
    case TRIGGER_REDIRECT_TARGET: {
        Player **entry;

        idx = Expr_EvalInt(p, &t->redirect_arg, 1);
        if (idx < -1) {
            idx = -1;
            Player_Warn(p, "Illegal target ID in retargeted trigger");
        }
        entry = Targets_Find(p->targets, idx);
        if (entry == NULL) {
            Player_Warnf(p, "has no target with hit ID %i", idx);
            Trigger_RedirectFailed(t);
            return;
        }
        target = *entry;
        break;
    }
    case TRIGGER_REDIRECT_PARTNER:
        idx = Expr_EvalInt(p, &t->redirect_arg, 1);
        if (idx < 0) {
            idx = 0;
            Player_Warn(p, "Illegal partner number in retargeted trigger");
        }
        target = Player_GetPartner(p, idx);
        if (target == NULL) {
            Player_Warnf(p, "has no %i-th partner", idx);
            Trigger_RedirectFailed(t);
            return;
        }
        break;
    case TRIGGER_REDIRECT_ENEMY:
        idx = Expr_EvalInt(p, &t->redirect_arg, 1);
        if (idx < 0) {
            idx = 0;
            Player_Warn(p, "Illegal enemy number in retargeted trigger");
        }
        target = Player_GetEnemy(p, idx);
        if (target == NULL) {
            Player_Warnf(p, "has no %i-th enemy", idx);
            Trigger_RedirectFailed(t);
            return;
        }
        break;
    case TRIGGER_REDIRECT_ENEMYNEAR:
        idx = Expr_EvalInt(p, &t->redirect_arg, 1);
        if (idx < 0) {
            idx = 0;
            Player_Warn(p, "Illegal near enemy number in retargeted trigger");
        }
        target = Player_GetNearEnemy(p, idx);
        if (target == NULL) {
            Player_Warnf(p, "has no %i-th nearest enemy", idx);
            Trigger_RedirectFailed(t);
            return;
        }
        break;
    case TRIGGER_REDIRECT_PLAYERID:
        idx = Expr_EvalInt(p, &t->redirect_arg, 1);
        if (idx < 0) {
            idx = 0;
            Player_Warn(p, "Illegal player number in retargeted trigger");
        }
        target = Player_FindById(idx);
        if (target == NULL) {
            Player_Warnf(p, "tried to access non-existent player with ID %i", idx);
            Trigger_RedirectFailed(t);
            return;
        }
        break;
    default:
        target = p;
        break;
    }

    /* The players other triggers refer to, seen from the target. */
    helper = target->helper;
    parent = NULL;
    if (helper == 1 && target->parent != NULL && target->parent->id == target->parent_id &&
        target->parent->active != 0) {
        parent = target->parent;
    }
    root = target->root;
    redirected = target != p;
    g = g_game;
    if (g->game_type_m == 0) {
        e1 = e2 = NULL;
        if (target->team_side == 1) {
            e1 = g->players[2];
            e2 = g->players[4];
        } else if (target->team_side == 2) {
            e1 = g->players[1];
            e2 = g->players[3];
        }
        p2 = target->enemy;
        if (e1 != NULL && e2 != NULL) {
            if (p2 == e1) {
                p4 = e2;
            } else if (p2 == e2) {
                p4 = e1;
            } else {
                d = fabs(Player_DistX(target, e1));
                p4 = fabs(Player_DistX(target, e2)) > d ? e2 : e1;
            }
        } else if (p2 != NULL) {
            p4 = e1;
            if (p2->helper == 1) {
                if (p2->root == e1) {
                    p4 = e2;
                }
            } else if (p2 == e1) {
                p4 = e2;
            }
        }
        if (target->helper == 1) {
            if (root != NULL) {
                idx = root->index + 2;
                if (idx > 4) {
                    idx -= 4;
                }
                p3 = g->players[idx];
            } else if (target->team_side == 1) {
                p3 = g->players[1];
                if (p3 == NULL) {
                    p3 = g->players[3];
                }
            } else if (target->team_side == 2) {
                p3 = g->players[2];
                if (p3 == NULL) {
                    p3 = g->players[4];
                }
            }
        } else {
            idx = target->index + 2;
            if (idx > 4) {
                idx -= 4;
            }
            p3 = g->players[idx];
        }
        if (target->team_side == 1) {
            leader = g->players[1];
        }
        if (target->team_side == 2) {
            leader = g->players[2];
        }
    }

    switch (t->id) {
    case TRIGGER_TIME:
        ival = target->state_time;
        break;
    case TRIGGER_TIMEMOD:
        ival = target->state_time % t->arg;
        break;
    case TRIGGER_P2DIST:
        if (p2 == NULL) {
            goto bottom_zero;
        }
        if (t->value == 0) {
            fval = (float)Player_ToLocalCoord(p, Player_DistX(target, p2));
        } else if (t->value == 1) {
            fval = (float)Player_ToLocalCoord(p, p2->pos.y - target->pos.y);
        }
        break;
    case TRIGGER_P2BODYDIST:
        if (p2 == NULL) {
            goto bottom_zero;
        }
        if (t->value == 0) {
            fval = (float)Player_ToLocalCoord(p, Player_BodyDistX(target, p2));
        } else if (t->value == 1) {
            fval = (float)Player_ToLocalCoord(p, p2->pos.y - target->pos.y);
        }
        break;
    case TRIGGER_P2LIFE:
        if (p2 == NULL) {
            goto bottom_zero;
        }
        ival = p2->life;
        break;
    case TRIGGER_WINPERFECT:
        if (g->win_type == 0 || g->round_winner != target->team_side || !Team_HasFullLife(target->team_side)) {
            ival = 0;
        } else {
            ival = 1;
        }
        break;
    case TRIGGER_WIN:
        if ((g->win_type != t->arg && t->arg != TRIGGER_WIN_ANY) || g->round_winner != target->team_side) {
            ival = 0;
        } else {
            ival = 1;
        }
        break;
    case TRIGGER_LOSE:
        if ((g->win_type != t->arg && t->arg != TRIGGER_WIN_ANY) || g->round_winner == target->team_side ||
            g->round_winner == 0) {
            ival = 0;
        } else {
            ival = 1;
        }
        break;
    case TRIGGER_DRAWGAME:
        ival = g->round_winner == 0 && g->win_type != 0;
        break;
    case TRIGGER_MATCHOVER:
        ival = Match_CheckOver() != 0;
        break;
    case TRIGGER_FRONTEDGEDIST:
    case TRIGGER_BACKEDGEDIST:
        ival = (int)Player_ToLocalCoord(p, Player_EdgeDist(target, t->arg, t->id == TRIGGER_FRONTEDGEDIST));
        break;
    case TRIGGER_LIFE:
        ival = target->life;
        break;
    case TRIGGER_LIFEMAX:
        ival = target->life_max;
        break;
    case TRIGGER_POWER:
        ival = *leader->power_ref;
        break;
    case TRIGGER_POWERMAX:
        ival = *leader->power_max_ref;
        break;
    case TRIGGER_ANIMTIME:
        st = target->anim;
        action = (AnimAction *)(st->anim->actions->data + st->anim->actions->elem_size * st->action);
        ival = st->time - action->total_time;
        break;
    case TRIGGER_ANIMELEM:
        st = target->anim;
        r = Anim_GetElemStart(st, t->arg);
        if (r == -1) {
            goto bottom_zero;
        }
        ival = st->time - r;
        break;
    case TRIGGER_ANIM:
        st = target->anim;
        ival = st->anim->actions->slots[st->action].key2;
        break;
    case TRIGGER_POS:
        if (t->value == 0) {
            fval = (float)Player_ToLocalCoord(p, target->pos.x - g->camera_offset_x);
        } else if (t->value == 1) {
            fval = (float)Player_ToLocalCoord(p, target->pos.y);
        } else if (t->value == 2 && target->data->mugenversion < g->compat_version[1]) {
            goto pos_z;
        }
        break;
    case TRIGGER_SCREENPOS: {
        Coord c;

        if (t->value == 0) {
            fval = (float)Player_ToLocalCoord(p, Player_GetScreenPos(target, &c)->x);
        } else if (t->value == 1) {
            fval = (float)Player_ToLocalCoord(p, Player_GetScreenPos(target, &c)->y);
        } else if (t->value == 2 && target->data->mugenversion < g->compat_version[1]) {
            goto pos_z;
        }
        break;
    }
    pos_z:
        /* pre-1.0 characters: a value by player number from the stage */
        idx = target->index - 1;
        if (root != NULL) {
            idx = root->index - 1;
        }
        if (idx > 4) {
            idx = 4;
        } else if (idx < 0) {
            idx = 0;
        }
        fval = (float)g_game->stage.startz[idx];
        break;
    case TRIGGER_CAMERAPOS:
        if (t->value == 0) {
            fval = (float)Player_ToLocalCoord(p, Camera_GetX());
        } else if (t->value == 1) {
            fval = (float)Player_ToLocalCoord(p, Camera_GetY());
        }
        break;
    case TRIGGER_CAMERAZOOM:
        fval = g->camera_zoom;
        break;
    case TRIGGER_VEL:
        if (t->value == 0) {
            fval = (float)Player_ToLocalCoord(p, (double)target->facing * target->vel.x);
        } else if (t->value == 1) {
            fval = (float)Player_ToLocalCoord(p, target->vel.y);
        }
        break;
    case TRIGGER_ALIVE:
        ival = target->alive;
        break;
    case TRIGGER_COMMAND:
        /* label - 1 when the command is not active, so that only "= label" holds */
        ival = t->value - 1;
        if (target->cmd_active[t->value] != 0) {
            ival = t->value;
            if (op == 0) {
                Ai_AddCommand(target->ai_cmds, ival);
            }
        }
        break;
    case TRIGGER_AILEVEL:
        if (target->ai == 0 || (g->team_variant == 3 && target->dummy_control == 0)) {
            ival = 0;
        } else {
            ival = target->ai_level;
        }
        break;
    case TRIGGER_HITCOUNT:
        ival = target->hitcount;
        break;
    case TRIGGER_UNIQHITCOUNT:
        ival = target->uniqhitcount;
        break;
    case TRIGGER_MOVEHIT:
        ival = target->move_contact == 2 ? target->move_contact_time + 1 : 0;
        break;
    case TRIGGER_MOVEGUARDED:
        ival = target->move_contact == 1 ? target->move_contact_time + 1 : 0;
        break;
    case TRIGGER_MOVECONTACT:
        ival = target->move_contact == 2 || target->move_contact == 1 ? target->move_contact_time + 1 : 0;
        break;
    case TRIGGER_MOVEREVERSED:
        ival = target->move_contact == 4 ? target->move_contact_time + 1 : 0;
        break;
    case TRIGGER_INGUARDDIST:
        ival = Player_InEnemyGuardDist(target);
        break;
    case TRIGGER_STATENO:
        ival = target->state_no;
        break;
    case TRIGGER_PREVSTATENO:
        ival = target->prev_state_no;
        break;
    case TRIGGER_P2STATENO:
        if (p2 == NULL) {
            goto bottom_zero;
        }
        ival = p2->state_no;
        break;
    case TRIGGER_STATETYPE:
        ival = target->statetype;
        break;
    case TRIGGER_P2STATETYPE:
        if (p2 == NULL) {
            goto bottom_zero;
        }
        ival = p2->statetype;
        break;
    case TRIGGER_MOVETYPE:
        ival = target->movetype;
        break;
    case TRIGGER_P2MOVETYPE:
        if (p2 == NULL) {
            goto bottom_zero;
        }
        ival = p2->movetype;
        break;
    case TRIGGER_HITDEFATTR: {
        unsigned int bits;

        ival = 0;
        if (target->movetype == MOVETYPE_ATTACK) {
            bits = target->hitdef.attr & (unsigned int)t->value;
            if ((bits & 7) != 0 && (bits & ~7u) != 0) {
                ival = 1;
            }
        }
        if (t->op == 1) {
            ival = 1 - ival;
        }
        break;
    }
    case TRIGGER_VAR:
        if (Trigger_PopInt(target, &arg) == 3) {
            goto bottom_arg;
        }
        if ((unsigned int)arg > PLAYER_VAR_COUNT - 1) {
            Trigger_Warn(p, target, redirected, "var index out of range");
            goto bottom_zero;
        }
        ival = target->var[arg];
        break;
    case TRIGGER_FVAR:
        if (Trigger_PopInt(target, &arg) == 3) {
            goto bottom_arg;
        }
        if ((unsigned int)arg > PLAYER_FVAR_COUNT - 1) {
            Trigger_Warn(p, target, redirected, "fvar index out of range");
            goto bottom_zero;
        }
        fval = target->fvar[arg];
        break;
    case TRIGGER_SYSVAR:
        if (Trigger_PopInt(target, &arg) == 3) {
            goto bottom_arg;
        }
        if ((unsigned int)arg > 4) {
            Trigger_Warn(p, target, redirected, "sysvar index out of range");
            goto bottom_zero;
        }
        ival = target->sysvar[arg];
        break;
    case TRIGGER_SYSFVAR:
        if (Trigger_PopInt(target, &arg) == 3) {
            goto bottom_arg;
        }
        if ((unsigned int)arg > 4) {
            Trigger_Warn(p, target, redirected, "sysfvar index out of range");
            goto bottom_zero;
        }
        fval = target->sysfvar[arg];
        break;
    case TRIGGER_RANDOM:
        ival = Match_Random() % 1000;
        break;
    case TRIGGER_NUMPROJ:
        ival = Projectiles_Count(target->projectiles);
        break;
    case TRIGGER_NUMPROJID:
        if (Trigger_PopInt(target, &arg) == 3) {
            goto bottom_arg;
        }
        idx = arg;
        if (idx < 0) {
            idx = 0;
            Trigger_Warn(p, target, redirected, "Negative projID specified.");
        }
        ival = Projectiles_CountId(target->projectiles, idx);
        break;
    case TRIGGER_PROJHIT:
    case TRIGGER_PROJGUARDED:
    case TRIGGER_PROJCONTACT:
        /* projhit<ID> = 0/1[, op time]: "= 0" negates the time comparison; no contact gives (value == 0) */
        if (t->id == TRIGGER_PROJCONTACT ? target->proj_contact == 0 || target->proj_contact == 3
                                         : target->proj_contact != (t->id == TRIGGER_PROJHIT ? 2 : 1)) {
            goto no_contact;
        }
        if (target->proj_contact_id != t->arg && t->arg != 0) {
            goto no_contact;
        }
        ival = target->proj_contact_time;
        if (t->arg2 == 0) {
            negate = 1;
        }
        break;
    no_contact:
        ExprStack_Push(t->arg2 == 0, EXPR_INT);
        return;
    case TRIGGER_PROJHITTIME:
    case TRIGGER_PROJCANCELTIME:
    case TRIGGER_PROJGUARDEDTIME:
    case TRIGGER_PROJCONTACTTIME:
        if (Trigger_PopInt(target, &arg) == 3) {
            goto bottom_arg;
        }
        idx = arg;
        if (idx < 0) {
            idx = 0;
            Trigger_Warn(p, target, redirected, "Negative projectile ID specified.");
        }
        if (t->id == TRIGGER_PROJCONTACTTIME ? target->proj_contact == 0 || target->proj_contact == 3
                                             : target->proj_contact != (t->id == TRIGGER_PROJHITTIME     ? 2
                                                                   : t->id == TRIGGER_PROJCANCELTIME ? 3
                                                                                                     : 1)) {
            ival = -1;
        } else if (target->proj_contact_id == idx || idx == 0) {
            ival = target->proj_contact_time;
        } else {
            ival = -1;
        }
        break;
    case TRIGGER_P1NAME:
        ival = _stricmp(target->data->name, (const char *)t->data) == 0;
        break;
    case TRIGGER_P2NAME:
        if (p2 == NULL) {
            ival = 0;
            break;
        }
        ival = _stricmp(p2->data->name, (const char *)t->data) == 0;
        break;
    case TRIGGER_P3NAME:
        if (p3 == NULL) {
            ival = 0;
            break;
        }
        ival = _stricmp(p3->data->name, (const char *)t->data) == 0;
        break;
    case TRIGGER_P4NAME:
        if (p4 == NULL) {
            ival = 0;
            break;
        }
        ival = _stricmp(p4->data->name, (const char *)t->data) == 0;
        break;
    case TRIGGER_AUTHORNAME:
        ival = _stricmp(target->data->author, (const char *)t->data) == 0;
        break;
    case TRIGGER_NUMPARTNER:
        ival = Player_CountPartners(target);
        break;
    case TRIGGER_NUMENEMY:
        ival = Player_CountEnemies(target);
        break;
    case TRIGGER_ID:
        ival = target->id;
        break;
    case TRIGGER_FACING:
        ival = target->facing;
        break;
    case TRIGGER_CTRL:
        ival = target->ctrl;
        break;
    case TRIGGER_HITFALL:
        ival = target->gethit_fall;
        break;
    case TRIGGER_HITSHAKEOVER:
        ival = target->gethit_shaketime <= 0;
        break;
    case TRIGGER_HITOVER:
        ival = target->gethit_hittime < 0;
        break;
    case TRIGGER_HITVEL:
        if (t->value == 0) {
            fval = target->facing * target->gethit_vel_x;
        } else if (t->value == 1) {
            fval = target->gethit_vel_y;
        }
        break;
    case TRIGGER_HITPAUSETIME:
        ival = target->hitpause_time;
        break;
    case TRIGGER_CANRECOVER:
        ival = target->gethit_fall_recover != 0 && target->gethit_fall_time >= target->gethit_fall_recovertime;
        break;
    case TRIGGER_ANIMELEMNO:
        if (Trigger_PopInt(target, &arg) == 3) {
            goto bottom_arg;
        }
        ival = Anim_ElemAtOffset(target->anim, arg);
        if (ival == -1) {
            goto bottom_zero;
        }
        break;
    case TRIGGER_GAMETIME:
        ival = (int)g->frames;
        break;
    case TRIGGER_ROUNDNO:
        ival = g->round_no;
        break;
    case TRIGGER_ROUNDSTATE:
        ival = g->round_state;
        break;
    case TRIGGER_MATCHNO:
        ival = g->arcade_match;
        break;
    case TRIGGER_ROUNDSEXISTED:
        ival = target->rounds_existed;
        break;
    case TRIGGER_TEAMSIDE:
        ival = target->team_side;
        break;
    case TRIGGER_TEAMMODE:
        idx = target->team_index;
        ival = g->match_members[idx].mode;
        if (g->teams[idx].member_count >= 2) {
            ival = 3;
        }
        break;
    case TRIGGER_ISHOMETEAM:
        if (g->arcade_side != 0) {
            ival = target->team_side == 3 - g->arcade_side;
        } else {
            ival = target->team_side == 1;
        }
        break;
    case TRIGGER_PALNO:
        ival = target->pal_no + 1;
        break;
    case TRIGGER_NUMEXPLOD: {
        SysDrawList *list = g->explods;
        SysDrawItem *item;
        int k;

        ival = 0;
        if (list->capacity > 0) {
            item = list->items;
            for (k = list->capacity; k != 0; k--) {
                if (item->used != 0 && item->owner_id == target->id) {
                    ival++;
                }
                item++;
            }
        }
        break;
    }
    case TRIGGER_NUMEXPLOD_ID:
        if (Trigger_PopInt(target, &arg) == 3) {
            goto bottom_arg;
        }
        idx = arg;
        if (idx < -1) {
            idx = -1;
            Trigger_Warn(p, target, redirected, "illegal explod ID for numexplod");
        }
        ival = Explods_Count(g_game->explods, target->id, idx);
        break;
    case TRIGGER_GETHITVAR: {
        float gf = 0.0f;
        int gi = 0;

        is_float = 0;
        scaled = 0;
        if (!GetHitVar_Get(target, t->value, &gi, &gf, &is_float, &scaled)) {
            Trigger_Warn(p, target, redirected, "Illegal gethitvar - programmer's fault");
        }
        if (is_float != 0) {
            if (scaled != 0) {
                gf = (float)Player_ToLocalCoord(p, gf);
            }
            fval = gf;
        } else if (scaled != 0) {
            ival = (int)Player_ToLocalCoord(p, (double)gi);
        } else {
            ival = gi;
        }
        break;
    }
    case TRIGGER_CONST:
        idx = t->value;
        if (g_triggerConsts[idx].is_float == 0) {
            arg = ((const int *)&target->consts)[idx];
            ival = arg;
            if (g_triggerConsts[idx].scaled != 0) {
                f = (float)Player_ToLocalCoord(p, g_gameCoord[0] / target->data->localcoord[0] * (double)arg);
                ival = Math_Round(f);
            }
        } else {
            fval = ((const float *)&target->consts)[idx];
            if (g_triggerConsts[idx].scaled != 0) {
                fval = (float)Player_ToLocalCoord(p, g_gameCoord[0] / target->data->localcoord[0] * (double)fval);
            }
        }
        break;
    case TRIGGER_CONST240P:
        if (Trigger_PopFloat(&fval) == 3) {
            goto bottom_fval;
        }
        fval = (float)Player_ToLocalCoord(p, Units_From320(fval));
        break;
    case TRIGGER_CONST480P:
    case TRIGGER_CONST720P:
        if (Trigger_PopFloat(&fval) == 3) {
            goto bottom_fval;
        }
        f = (float)((double)(int)g_game->game_coord[0] * fval / (t->id == TRIGGER_CONST480P ? 640.0 : 1280.0));
        fval = (float)Player_ToLocalCoord(p, f);
        break;
    case TRIGGER_STAGEVAR:
        if (t->arg == TRIGGER_STAGEVAR_NAME) {
            ival = _stricmp(g_game->stage.name, (const char *)t->data) == 0;
        } else if (t->arg == TRIGGER_STAGEVAR_DISPLAYNAME) {
            ival = _stricmp(g_game->stage.displayname, (const char *)t->data) == 0;
        } else if (t->arg == TRIGGER_STAGEVAR_AUTHOR) {
            ival = _stricmp(g_game->stage.author, (const char *)t->data) == 0;
        } else {
            Player_Warn(target, "invalid StageVar trigger");
            ival = 0;
        }
        break;
    case TRIGGER_NUMTARGET:
        ival = Targets_Count(target->targets, -1);
        break;
    case TRIGGER_NUMTARGET_ID:
        if (Trigger_PopInt(target, &arg) == 3) {
            goto bottom_arg;
        }
        idx = arg;
        if (idx < -1) {
            idx = -1;
            Trigger_Warn(p, target, redirected, "illegal target ID for numtarget");
        }
        ival = Targets_Count(target->targets, idx);
        break;
    case TRIGGER_TICKSPERSECOND:
        ival = g_frameRate;
        break;
    case TRIGGER_ISHELPER:
        ival = helper == 1;
        break;
    case TRIGGER_ISHELPER_ID:
        r = Trigger_PopInt(target, &arg);
        if (target->helper != 1) {
            ival = 0;
            break;
        }
        if (r == 3) {
            goto bottom_arg;
        }
        ival = arg == target->helper_id;
        break;
    case TRIGGER_NUMHELPER:
        ival = Helpers_Count(target, 0);
        break;
    case TRIGGER_NUMHELPER_ID:
        if (Trigger_PopInt(target, &arg) == 3) {
            goto bottom_arg;
        }
        ival = Helpers_Count(target, arg);
        break;
    case TRIGGER_PARENTDIST:
    case TRIGGER_ROOTDIST:
        other = t->id == TRIGGER_PARENTDIST ? parent : root;
        if (other == NULL) {
            goto bottom_zero;
        }
        if (t->value == 0) {
            fval = (float)Player_ToLocalCoord(p, Player_DistX(target, other));
        } else if (t->value == 1) {
            fval = (float)Player_ToLocalCoord(p, other->pos.y - target->pos.y);
        }
        break;
    case TRIGGER_STANDBY:
        ival = target->standby;
        break;
    case TRIGGER_GAMEWIDTH:
        fval = (float)Player_ToLocalCoord(p, (double)(int)g_game->game_coord[0] / (double)g_game->camera_zoom);
        break;
    case TRIGGER_GAMEHEIGHT:
        fval = (float)Player_ToLocalCoord(p, (double)(int)g_game->game_coord[1] / (double)g_game->camera_zoom);
        break;
    case TRIGGER_SCREENWIDTH:
        fval = (float)Player_ToLocalCoord(p, (double)(int)g_game->game_coord[0]);
        break;
    case TRIGGER_SCREENHEIGHT:
        fval = (float)Player_ToLocalCoord(p, (double)(int)g_game->game_coord[1]);
        break;
    case TRIGGER_BACKEDGE:
    case TRIGGER_FRONTEDGE:
    case TRIGGER_LEFTEDGE:
    case TRIGGER_RIGHTEDGE:
        if (t->id == TRIGGER_LEFTEDGE || (t->id == TRIGGER_BACKEDGE && target->facing == 1) ||
            (t->id == TRIGGER_FRONTEDGE && target->facing == -1)) {
            d = Camera_GetX();
            d = d - (double)(int)g_game->game_coord[0] / (double)g_game->camera_zoom * 0.5;
        } else {
            d = Camera_GetX();
            d = (double)(int)g_game->game_coord[0] / (double)g_game->camera_zoom * 0.5 + d;
        }
        fval = (float)Player_ToLocalCoord(p, d);
        break;
    case TRIGGER_TOPEDGE:
        fval = (float)Player_ToLocalCoord(p, Screen_GetTopEdge());
        break;
    case TRIGGER_BOTTOMEDGE:
        fval = (float)Player_ToLocalCoord(p, Screen_GetBottomEdge());
        break;
    case TRIGGER_PI:
        fval = 3.14159274f;
        break;
    case TRIGGER_E:
        fval = 2.71828175f;
        break;
    case TRIGGER_IFELSE:
        /*
         * The types are tested against 3 and 2 instead of EXPR_BOTTOM and
         * EXPR_FLOAT (original bug): bottom is never seen and a float
         * condition is tested by its bits.
         */
        if (!ExprStack_Pop(&v3.i, &type3)) {
            ExprStack_Underflow();
        }
        if (!ExprStack_Pop(&v2.i, &type2)) {
            ExprStack_Underflow();
        }
        if (!ExprStack_Pop(&v1.i, &type1)) {
            ExprStack_Underflow();
        }
        if (type1 == 3) {
            ExprStack_Push(v1.i, EXPR_BOTTOM);
            return;
        }
        if (type2 == 3) {
            ExprStack_Push(v2.i, EXPR_BOTTOM);
            return;
        }
        if (type3 == 3) {
            ExprStack_Push(v3.i, EXPR_BOTTOM);
            return;
        }
        if (type1 == 2) {
            if (0.0f == v1.f) {
                ExprStack_Push(v3.i, type3);
            } else {
                ExprStack_Push(v2.i, type2);
            }
            return;
        }
        if (v1.i != 0) {
            ExprStack_Push(v2.i, type2);
        } else {
            ExprStack_Push(v3.i, type3);
        }
        return;
    case TRIGGER_LOG: {
        float x, base;

        type1 = Trigger_PopFloat(&x);
        if (Trigger_PopFloat(&base) == 3) {
            fval = base;
            goto bottom_fval;
        }
        if (type1 == 3) {
            fval = x;
            goto bottom_fval;
        }
        if (base <= 0.0f || base == 1.0f || x <= 0.0f) {
            Trigger_Warn(p, target, redirected, "took log with non-positive base or argument.");
            goto bottom_zero;
        }
        fval = Trigger_LogBase(x, base);
        break;
    }
    case TRIGGER_SIN:
    case TRIGGER_COS:
    case TRIGGER_TAN:
    case TRIGGER_ATAN:
    case TRIGGER_EXP:
        if (Trigger_PopFloat(&fval) == 3) {
            goto bottom_fval;
        }
        fval = Trigger_Math(t->id == TRIGGER_SIN   ? CI_SIN
                            : t->id == TRIGGER_COS ? CI_COS
                            : t->id == TRIGGER_TAN ? CI_TAN
                            : t->id == TRIGGER_ATAN ? CI_ATAN
                                                    : CI_EXP,
                            fval);
        break;
    case TRIGGER_ASIN:
    case TRIGGER_ACOS:
        if (Trigger_PopFloat(&fval) == 3) {
            goto bottom_fval;
        }
        if (fval < -1.0f || 1.0f < fval) {
            Trigger_Warn(p, target, redirected,
                         t->id == TRIGGER_ASIN ? "asin argument out of range" : "acos argument out of range");
            goto bottom_zero;
        }
        fval = Trigger_Math(t->id == TRIGGER_ASIN ? CI_ASIN : CI_ACOS, fval);
        break;
    case TRIGGER_LN:
        if (Trigger_PopFloat(&fval) == 3) {
            goto bottom_fval;
        }
        if (fval <= 0.0f) {
            Trigger_Warn(p, target, redirected, "took ln of a non-positive number");
            goto bottom_zero;
        }
        fval = Trigger_Math(CI_LOG, fval);
        break;
    case TRIGGER_ABS:
        if (!ExprStack_Pop(&v1.i, &type1)) {
            ExprStack_Underflow();
        }
        if (type1 == EXPR_BOTTOM) {
            ExprStack_Push(v1.i, EXPR_BOTTOM);
            return;
        }
        if (type1 == EXPR_INT) {
            ExprStack_Push(v1.i < 0 ? -v1.i : v1.i, EXPR_INT);
            return;
        }
        if (type1 == EXPR_FLOAT) {
            f = (float)fabs(v1.f);
            ExprStack_Push(float_bits(f), EXPR_FLOAT);
            return;
        }
        sprintf(g_libraryErrorText, "Panic: aborting");
        Error_Exit();
    case TRIGGER_FLOOR:
    case TRIGGER_CEIL:
        if (!ExprStack_Pop(&v1.i, &type1)) {
            ExprStack_Underflow();
        }
        if (type1 == EXPR_BOTTOM) {
            ExprStack_Push(v1.i, EXPR_BOTTOM);
            return;
        }
        if (type1 == EXPR_INT) {
            ival = v1.i;
            break;
        }
        if (type1 != EXPR_FLOAT) {
            sprintf(g_libraryErrorText, "Panic: aborting");
            Error_Exit();
        }
        f = (float)(t->id == TRIGGER_FLOOR ? floor((double)v1.f) : ceil((double)v1.f));
        if (f < -2147483648.0f) {
            ival = (int)0x80000000;
            Player_Warn(target, t->id == TRIGGER_FLOOR ? "Floor() result clamped to INT_MIN"
                                                       : "Ceil() result clamped to INT_MIN");
        } else if (2147483648.0f < f) {
            ival = 0x7FFFFFFF;
            Player_Warn(target, t->id == TRIGGER_FLOOR ? "Floor() result clamped to INT_MAX"
                                                       : "Ceil() result clamped to INT_MAX");
        } else {
            ival = (int)f;
        }
        break;
    case TRIGGER_ANIMELEMTIME:
        if (Trigger_PopInt(target, &arg) == 3) {
            goto bottom_arg;
        }
        st = target->anim;
        r = Anim_GetElemStart(st, arg - 1);
        if (r == -1) {
            goto bottom_zero;
        }
        ival = st->time - r;
        break;
    case TRIGGER_ANIMEXIST:
        if (Trigger_PopInt(target, &arg) == 3) {
            goto bottom_arg;
        }
        ival = Player_AnimExists(target, arg);
        break;
    case TRIGGER_SELFANIMEXIST:
        if (Trigger_PopInt(target, &arg) == 3) {
            goto bottom_arg;
        }
        ival = Player_SelfAnimExists(target, arg);
        break;
    case TRIGGER_PLAYERIDEXIST:
        if (Trigger_PopInt(target, &arg) == 3) {
            goto bottom_arg;
        }
        ival = Player_FindById(arg) != NULL;
        break;
    case TRIGGER_COND: {
        float cf;
        int ci;

        expr = (const Expr *)t->data;
        r = Expr_Eval(target, expr, &ci, &cf);
        if (r == 1) {
            expr += ci != 0 ? 1 : 2;
        } else if (r == 2) {
            expr += cf == 0.0f ? 2 : 1;
        } else if (r == 3) {
            ExprStack_Push(ci, EXPR_BOTTOM);
            return;
        } else {
            Player_Warn(target, "COND condition couldn't be evaluated; assuming False -- engine's fault");
            expr += 2;
        }
        r = Expr_Eval(target, expr, &ci, &cf);
        if (r == 1) {
            ExprStack_Push(ci, EXPR_INT);
        } else if (r == 2) {
            ExprStack_Push(float_bits(cf), EXPR_FLOAT);
        } else if (r == 3) {
            ExprStack_Push(ci, EXPR_BOTTOM);
        } else {
            Player_Warn(target, "COND evaluation caused an error; returning bottom -- engine's fault");
            ExprStack_Push(0, EXPR_BOTTOM);
        }
        return;
    }
    default:
        Error_Printf("Missing trigger code %i\n", t->id);
        Error_Exit();
    }

    /* The triggers that parsed their own "= value" compare now. */
    if (Trigger_SelfCompares(t)) {
        switch (op) {
        case TRIGGER_OP_EQ:
            ival = ival == t->value;
            break;
        case TRIGGER_OP_NE:
            ival = ival != t->value;
            break;
        case TRIGGER_OP_GT:
            ival = ival > t->value;
            break;
        case TRIGGER_OP_GE:
            ival = ival >= t->value;
            break;
        case TRIGGER_OP_LT:
            ival = ival < t->value;
            break;
        case TRIGGER_OP_LE:
            ival = ival <= t->value;
            break;
        case TRIGGER_OP_IN_CC:
            ival = ival >= t->value && ival <= t->value2;
            break;
        case TRIGGER_OP_IN_CO:
            ival = ival >= t->value && ival < t->value2;
            break;
        case TRIGGER_OP_IN_OC:
            ival = ival > t->value && ival <= t->value2;
            break;
        case TRIGGER_OP_IN_OO:
            ival = ival > t->value && ival < t->value2;
            break;
        }
        if (negate) {
            ival = 1 - ival;
        }
    }
    if (t->is_float == 0) {
        ExprStack_Push(ival, EXPR_INT);
        return;
    }
    if (t->is_float == 1) {
        ExprStack_Push(float_bits(fval), EXPR_FLOAT);
        return;
    }
    Error_Exit();

bottom_arg:
    ExprStack_Push(arg, EXPR_BOTTOM);
    return;
bottom_fval:
    /* bottom keeps the value it was popped with (through float) */
    ExprStack_Push((int)fval, EXPR_BOTTOM);
    return;
bottom_zero:
    ExprStack_Push(0, EXPR_BOTTOM);
}
