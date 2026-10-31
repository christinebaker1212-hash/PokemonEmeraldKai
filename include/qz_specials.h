#ifndef GUARD_QZ_SPECIALS_H
#define GUARD_QZ_SPECIALS_H
// Moemon Kai: Moemon Quetzal's specials for the Johto / Kanto scripts (data/qz_specials.inc). Ported ones live in
// src/qz_specials.c; the rest are stubs (src/qz_specials_stub.c) that log their name and give VAR_RESULT 0.
u16 QzSpecialStub(const char *name);
bool8 QzBugContest_Active(void);       // src/qz_bugcontest.c: the Bug-Catching Contest runs (no SAVE)
bool8 QzBugContest_FieldCheck(void);   // its end by time / catches / balls, or leaving the park
#define QZ_STUB(name) u16 name(void) { return QzSpecialStub(#name); }
#endif
