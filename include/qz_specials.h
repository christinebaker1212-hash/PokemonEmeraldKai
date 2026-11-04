#ifndef GUARD_QZ_SPECIALS_H
#define GUARD_QZ_SPECIALS_H
// Moemon Kai: Moemon Quetzal's specials for the Johto / Kanto scripts (data/qz_specials.inc). Ported ones live in
// src/qz_specials.c; the rest are stubs (src/qz_specials_stub.c) that log their name and give VAR_RESULT 0.
u16 QzSpecialStub(const char *name);
bool8 QzBugContest_Active(void);       // src/qz_bugcontest.c: the Bug-Catching Contest runs (no SAVE)
bool8 QzBugContest_FieldCheck(void);   // its end by time / catches / balls, or leaving the park
bool8 MP_LegendForeign(u16 species, u8 region);   // a legendary of another region (each region's only there)
void MP_HideForeignLegends(void);                 // after a map's transition scripts (overworld.c)
void MP_JohtoWeekdayPeople(void);                // Crystal's one-day people (the WEEKDAY siblings) out on their day (mp_region.c)
void MP_JohtoBerries_Init(void);                  // the same: JOHTO's BERRY patches planted once (mp_apricorn.c)
#define QZ_STUB(name) u16 name(void) { return QzSpecialStub(#name); }
#endif
