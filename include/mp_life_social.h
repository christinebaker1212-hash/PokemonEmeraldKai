#ifndef GUARD_MP_LIFE_SOCIAL_H
#define GUARD_MP_LIFE_SOCIAL_H

// Moemon Co-op: MOéMON GARDEN among friends (src/mp/mp_life_social.c, docs/POKEMON_GARDEN.md 6.4, 7, 10)
struct MpState;
struct MpLifeRec;

void MP_LifeSocial_CollectState(struct MpState *s);   // mp_main.c: the follower's own life in our wire state
void MP_LifeSocial_OnRelay(u8 from, const u8 *data, u16 len);
void MP_LifeSocial_Update(void);                       // every frame (mp_main.c)
bool8 MP_LifeSocial_TryFriendFollower(void);           // A in front of a friend's follower (mp_overworld.c)
void MP_LifeSocial_AskRecord(u32 key);                 // a Pokémon from elsewhere: does a friend's game know it?
void MP_LifeSocial_SendRecord(u8 target, u32 key, bool8 traded);
// mp_life_field.c: meeting a friend's follower (or a friend) while the Trainers pause together
bool8 MP_LifeSocial_MingleTarget(struct MpLifeRec *rec, u8 *friendId, s16 *x, s16 *y, s16 *fx, s16 *fy, u8 *emote, u8 *pose, bool8 *greetTrainer);
void MP_LifeSocial_Met(u8 friendId, bool8 trainer);
void MP_LifeSocial_Debug(const char *args);

#endif // GUARD_MP_LIFE_SOCIAL_H
