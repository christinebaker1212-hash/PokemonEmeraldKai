#ifndef GUARD_MP_CHATTER_H
#define GUARD_MP_CHATTER_H

// Moemon Kai: CHATOT's CHATTER as it was in Platinum (src/mp/mp_chatter.c)
bool8 MP_Chatter_HasSaying(void);                         // the save has a saying (CHATTER in the party menu)
void MP_Chatter_SetSaying(const s8 *samples);             // a new one heard (MIC_RECORD_SAMPLES samples)
void MP_Chatter_SendOwn(void);                            // (online: friends hear it)
void MP_Chatter_OnBlob(u8 id, const u8 *data, u32 len);   // a friend's (MP_BLOB_CHATTER)
void MP_Chatter_OnPlayerLeft(u8 id);
void MP_Chatter_ForBattler(u8 battler);                   // the next cry is this battler's CHATOT
bool8 MP_Chatter_TryCry(u8 mode, s8 pan, s8 volume);      // sound.c: TRUE when the saying plays instead
bool8 MP_Chatter_Playing(void);
void MP_Chatter_Stop(void);
void MP_Chatter_Mix(float *buffer, s32 count);            // music_player.c
bool8 MP_Chatter_Confuses(u8 attacker, u8 target, u16 random);   // CHATTER's chance, by the saying
void MP_Chatter_Debug(void);

#endif // GUARD_MP_CHATTER_H
