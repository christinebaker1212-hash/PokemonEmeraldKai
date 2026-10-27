#ifndef MUGEN_CNSPARSE_H
#define MUGEN_CNSPARSE_H

/*
 * CNS parse helpers (cnsparse.c), shared by the state controller and trigger
 * parsers: parameter values in a fixed format, postype, AssertSpecial flags,
 * F/S anim and sound prefixes, hitdefattr, hitflag, gethitvar names, command
 * labels and the formatted text of DisplayToClipboard/AppendToClipboard.
 */

/* Cns_ParseValues / Cns_ParseArgs format: 'n' any value, 'i' int, 'f' float, then ':' and the accepted terminators. */

/* GetHitVar_Lookup indices (Trigger.value of gethitvar). */
#define GETHITVAR_XVELADD               0
#define GETHITVAR_YVELADD               1
#define GETHITVAR_TYPE                  2
#define GETHITVAR_ZOFF                  16  /* only for characters older than GameVars.compat_version[1] */
#define GETHITVAR_FALL_ENVSHAKE_PHASE   35
#define GETHITVAR_COUNT                 36

/* HitDefAttr_Parse result: state bits, then attack class bits for N/S/H x A/T/P. */
#define HITDEFATTR_S        0x001
#define HITDEFATTR_C        0x002
#define HITDEFATTR_A        0x004

/* AssertSpecial_ParseFlag kinds. */
#define ASSERT_GLOBAL       0   /* intro, roundnotover, noko... (index 0..10) */
#define ASSERT_PLAYER       1   /* nostandguard, nocrouchguard... (index 0..8) */

/* Parameters of a formatted text (CnsText_Parse). */
#define CNSTEXT_MAX_PARAMS  6

/* A parsed "text" parameter: the text with its format specifiers, allocated as one block. */
typedef struct CnsText {
    int pos[CNSTEXT_MAX_PARAMS];    /* 0x00 offset in text of each parameter's specifier */
    int len[CNSTEXT_MAX_PARAMS];    /* 0x18 length of each specifier */
    short count;                    /* 0x30 parameters */
    char types[CNSTEXT_MAX_PARAMS]; /* 0x32 EXPR_INT + 1 (%d, %i) or EXPR_FLOAT + 1 (%e, %f, %g) */
    char text[1];                   /* 0x38 unquoted, escapes resolved, "%%" made "%" */
} CnsText;
MUGEN_ASSERT_OFFSET(CnsText, len, 0x18);
MUGEN_ASSERT_OFFSET(CnsText, count, 0x30);
MUGEN_ASSERT_OFFSET(CnsText, types, 0x32);
MUGEN_ASSERT_OFFSET(CnsText, text, 0x38);

#endif /* MUGEN_CNSPARSE_H */
