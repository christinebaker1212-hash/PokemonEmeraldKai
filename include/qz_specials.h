#ifndef GUARD_QZ_SPECIALS_H
#define GUARD_QZ_SPECIALS_H
// Moemon Kai: Moemon Quetzal's specials for the Johto / Kanto scripts (data/qz_specials.inc). Ported ones live in
// src/qz_specials.c; the rest are stubs (src/qz_specials_stub.c) that log their name and give VAR_RESULT 0.
u16 QzSpecialStub(const char *name);
#define QZ_STUB(name) u16 name(void) { return QzSpecialStub(#name); }
#endif
