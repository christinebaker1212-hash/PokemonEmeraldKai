#ifndef GUARD_MP_LIFE_FIELD_H
#define GUARD_MP_LIFE_FIELD_H

// Moemon Co-op: MOéMON GARDEN, the follower's own life on the map (src/mp/mp_life_field.c) and the
// follower's autonomous movement (src/mp/mp_follower.c).

// Poses while it's on its own
enum
{
    LIFE_POSE_NONE,     // standing (the idle bob)
    LIFE_POSE_SIT,      // still, facing one way
    LIFE_POSE_NAP,      // napping (facing down, settled)
    LIFE_POSE_HOP,      // little hops (playing, excited)
    LIFE_POSE_SHAKE,    // a little shake (rummaging, digging, splashing, practicing)
    LIFE_POSE_SPIN,     // turning around (looking about)
    LIFE_POSE_LOOKUP,   // looking up at the sky
};

// What it's doing (also sent to friends, G4)
enum
{
    LIFE_ACT_NONE,
    LIFE_ACT_LOOK,
    LIFE_ACT_WANDER,
    LIFE_ACT_INVESTIGATE,
    LIFE_ACT_FORAGE,
    LIFE_ACT_NAP,
    LIFE_ACT_PLAY,      // playing on its own, inviting its Trainer
    LIFE_ACT_COMPANY,
    LIFE_ACT_QUIET,
    LIFE_ACT_TYPE,      // its type need (soak, bask, dig...)
    LIFE_ACT_HINT,      // sniffing at something hidden
    LIFE_ACT_RETURN,
    LIFE_ACT_COUNT
};

// mp_follower.c: the local follower on its own
bool8 MP_Follower_AutoStart(void);
bool8 MP_Follower_AutoActive(void);
void MP_Follower_AutoTarget(s16 x, s16 y, u8 speed);
bool8 MP_Follower_AutoArrived(void);
bool8 MP_Follower_AutoStuck(void);
void MP_Follower_AutoPose(u8 pose);
void MP_Follower_AutoFaceTile(s16 x, s16 y);
void MP_Follower_AutoFace(u8 dir);      // 0 down, 1 up, 2 left, 3 right
void MP_Follower_AutoReturn(void);
bool8 MP_Follower_AutoFinishReturn(void);
bool8 MP_Follower_LocalPos(s16 *x, s16 *y, u8 *dir);
bool8 MP_Follower_SlotScreenTop(u8 slot, s16 *x, s16 *top, u8 *subpriority);
bool8 MP_Follower_CanStandAt(s16 x, s16 y);
u16 MP_Follower_SlotDex(u8 slot);

// mp_life_field.c
void MP_LifeField_Update(void);          // every overworld frame, after the followers moved
void MP_LifeField_OnHurryBack(s16 distance);
u8 MP_LifeField_Activity(void);          // LIFE_ACT_*
bool8 MP_LifeField_Inviting(void);       // it's asking its Trainer to play
struct MpLifeRec *MP_LifeField_FollowerRec(void);
void MP_LifeField_Reaction(u8 emote, u8 pose, u16 frames);   // a short reaction (an interaction's answer)

// mp_life_interact.c: talking to and doing things with Pokémon
bool8 MP_LifeInteract_TryFollower(void);   // A in front of the follower (the smart action) or held A (the wheel)
void MP_LifeInteract_Update(void);

#endif // GUARD_MP_LIFE_FIELD_H
