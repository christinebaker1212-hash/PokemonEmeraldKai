#ifndef MUGEN_EXPR_H
#define MUGEN_EXPR_H

/*
 * CNS expressions (trigger and state controller parameter values).
 *
 * Expr_Parse converts the text to postfix (shunting-yard) as an array of
 * tokens plus a parallel array of token types; Expr_Eval runs it on the
 * global evaluation stack (g_exprStack*), which trigger implementations also
 * push their results to.
 */

/* Token types (Expr.t.types), also the types of evaluation stack entries. */
#define EXPR_INT            0
#define EXPR_FLOAT          1
#define EXPR_OP             2   /* token value is an EXPR_OP_* */
#define EXPR_TRIGGER        3   /* token value is a Trigger * */
#define EXPR_BOTTOM         4   /* evaluation stack only: no value (division by zero...) */

/* Expr.count of an expression folded to a single constant. */
#define EXPR_CONST          (-1)

#define EXPR_MAX_TOKENS     128 /* tokens of one expression */
#define EXPR_OP_STACK_SIZE  64  /* parser operator stack */
#define EXPR_STACK_SIZE     128 /* evaluation stack (g_exprStack*) */

/* Expr_Parse/Expr_Compile "want": what a lone constant must be. */
#define EXPR_WANT_ANY       0
#define EXPR_WANT_INT       1   /* a float constant is an error */
#define EXPR_WANT_FLOAT     2   /* an int constant is converted to float */

/* Operators, in the token values of EXPR_OP tokens. */
#define EXPR_OP_EQ          1   /* = */
#define EXPR_OP_NE          2   /* != */
#define EXPR_OP_GT          3   /* > */
#define EXPR_OP_LT          4   /* < */
#define EXPR_OP_GE          5   /* >= */
#define EXPR_OP_LE          6   /* <= */
#define EXPR_OP_IN_OO       7   /* x = (a, b): followed by the = or != token */
#define EXPR_OP_IN_OC       8   /* x = (a, b] */
#define EXPR_OP_IN_CO       9   /* x = [a, b) */
#define EXPR_OP_IN_CC       10  /* x = [a, b] */
#define EXPR_OP_AND         11  /* && */
#define EXPR_OP_OR          12  /* || */
#define EXPR_OP_NOT         13  /* ! */
#define EXPR_OP_XOR         14  /* ^^ */
#define EXPR_OP_POW         15  /* ** */
#define EXPR_OP_BAND        16  /* & */
#define EXPR_OP_BOR         17  /* | */
#define EXPR_OP_BXOR        18  /* ^ */
#define EXPR_OP_NEG         19  /* unary - */
#define EXPR_OP_ASSIGN      20  /* var(n) := x */
#define EXPR_OP_FASSIGN     21  /* fvar(n) := x (the parser turns := into this for fvar) */
#define EXPR_OP_BNOT        22  /* ~ */
#define EXPR_OP_ADD         23  /* + */
#define EXPR_OP_SUB         24  /* binary - */
#define EXPR_OP_MUL         25  /* * */
#define EXPR_OP_DIV         26  /* / */
#define EXPR_OP_MOD         27  /* % */
#define EXPR_OP_LPAREN      28  /* ( (never output) */
#define EXPR_OP_RPAREN      29  /* ) (never output) */

/* Trigger ids the expression code knows about (Trigger.id). */
#define TRIGGER_VAR         0x157
#define TRIGGER_FVAR        0x158

struct Trigger;

/* One token, or one evaluation stack entry. */
typedef union ExprValue {
    int i;
    float f;
    struct Trigger *trig;
} ExprValue;
MUGEN_ASSERT_SIZE(ExprValue, 4);

typedef union ExprCode {
    ExprValue *tokens;          /* count > 0 */
    ExprValue value;            /* count == EXPR_CONST */
} ExprCode;

typedef union ExprTypes {
    char *types;                /* count > 0: EXPR_INT..EXPR_TRIGGER per token */
    char type;                  /* count == EXPR_CONST: EXPR_INT or EXPR_FLOAT */
} ExprTypes;

/* A compiled expression. */
typedef struct Expr {
    ExprCode u;                 /* 0x00 */
    ExprTypes t;                /* 0x04 */
    int count;                  /* 0x08 tokens; 0 = no expression, EXPR_CONST = constant */
} Expr;
MUGEN_ASSERT_OFFSET(Expr, t, 0x04);
MUGEN_ASSERT_OFFSET(Expr, count, 0x08);
MUGEN_ASSERT_SIZE(Expr, 0x0C);

/*
 * A parsed trigger call (token of type EXPR_TRIGGER), see trigger.h. Most
 * triggers only use id and redirect: comparisons are done by the expression.
 * The fields from op on hold what the few triggers that read their own
 * "= value" part (command, animelem, statetype, string triggers...) parsed.
 */
typedef struct Trigger {
    Expr redirect_arg;          /* 0x00 argument of the redirection, e.g. the ID of helper(ID) */
    int id;                     /* 0x0C TRIGGER_* */
    int redirect;               /* 0x10 TRIGGER_REDIRECT_* */
    int op;                     /* 0x14 TRIGGER_OP_*, or 1 for "!=" in command/statetype/string... triggers */
    int is_float;               /* 0x18 the value pushed is a float (fvalue fields used for the comparison) */
    union {
        int value;              /* 0x1C compared value (range start); xyz component, const/gethitvar index, command label */
        int data_count;         /* 0x1C Exprs in data (id 400) */
    };
    int value2;                 /* 0x20 range end */
    int arg;                    /* 0x24 animelem number - 1, timemod divisor, projectile ID, win type, stagevar field, body flag */
    int arg2;                   /* 0x28 projhit/projguarded/projcontact: the "= 0/1" value */
    float fvalue;               /* 0x2C value as a float */
    float fvalue2;              /* 0x30 value2 as a float */
    void *data;                 /* 0x34 string (ids 0x5A-0x5E, stagevar) or Expr array (id 400), owned */
} Trigger;
MUGEN_ASSERT_OFFSET(Trigger, id, 0x0C);
MUGEN_ASSERT_OFFSET(Trigger, redirect, 0x10);
MUGEN_ASSERT_OFFSET(Trigger, op, 0x14);
MUGEN_ASSERT_OFFSET(Trigger, value, 0x1C);
MUGEN_ASSERT_OFFSET(Trigger, data_count, 0x1C);
MUGEN_ASSERT_OFFSET(Trigger, arg, 0x24);
MUGEN_ASSERT_OFFSET(Trigger, fvalue, 0x2C);
MUGEN_ASSERT_OFFSET(Trigger, data, 0x34);
MUGEN_ASSERT_SIZE(Trigger, 0x38);

#endif /* MUGEN_EXPR_H */
