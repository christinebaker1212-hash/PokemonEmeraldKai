#ifndef GUARD_MP_QZPHONE_H
#define GUARD_MP_QZPHONE_H
// JOHTO's phone, Crystal's (src/mp/mp_qzphone.c)
bool8 MP_QzPhone_Registered(u8 contact);
u8 MP_QzPhone_Numbers(u8 *out, u8 max);          // the registered contacts, in Crystal's order
const u8 *MP_QzPhone_Name(u8 contact);
const u8 *MP_QzPhone_ClassName(u8 contact);
void MP_QzPhone_Call(u8 contact, u8 *out, u16 size);   // calling them: their words, pages split by \p
bool8 MP_QzPhone_TryCall(void);                  // a step in the field: a contact calls (TRUE: its script is set up)
#endif
