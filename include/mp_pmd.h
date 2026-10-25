#ifndef GUARD_MP_PMD_H
#define GUARD_MP_PMD_H

#include "mp_bigsprite.h"

// Moemon Co-op: PMD Collab overworld sprites (mp_pmd.c; data from tools/moemon_pmd) for followers and Garden
// Pokémon. A Pokémon's sheet is one chunk of graphics/moemon/pmd/pmd.bin, inflated when first needed and shared by
// everyone showing that Pokémon; its frames are drawn whole by a big sprite (mp_bigsprite.h).

// The animations (the importer's ANIMS order)
enum
{
    PMD_ANIM_WALK,
    PMD_ANIM_IDLE,
    PMD_ANIM_SLEEP,
    PMD_ANIM_EAT,
    PMD_ANIM_HOP,
    PMD_ANIM_ROTATE,
    PMD_ANIM_ATTACK,
    PMD_ANIM_SHOOT,
    PMD_ANIM_STRIKE,
    PMD_ANIM_LOOKUP,
    PMD_ANIM_CHARGE,
    PMD_ANIM_SHOCK,
    PMD_ANIM_TRIP,
    PMD_ANIM_TUMBLE,
    PMD_ANIM_HURT,
    PMD_ANIM_CRINGE,
    PMD_ANIM_NOD,
    PMD_ANIM_POSE,
    PMD_ANIM_SIT,
    PMD_ANIM_DEEPBREATH,
    PMD_ANIM_LAYING,
    PMD_ANIM_FLOAT,
    PMD_ANIM_LEAPFORTH,
    PMD_ANIM_PULL,
    PMD_ANIM_WAKE,
    PMD_ANIM_EVENTSLEEP,
    PMD_ANIM_SWING,
    PMD_ANIM_DOUBLE,
    PMD_ANIM_QUICKSTRIKE,
    PMD_ANIM_COUNT
};

// The 8 directions (the sheets' rows)
enum
{
    PMD_DIR_DOWN,
    PMD_DIR_DOWN_RIGHT,
    PMD_DIR_RIGHT,
    PMD_DIR_UP_RIGHT,
    PMD_DIR_UP,
    PMD_DIR_UP_LEFT,
    PMD_DIR_LEFT,
    PMD_DIR_DOWN_LEFT,
};

struct MpPmdChunk;

// One Pokémon on screen (a follower, a Garden Pokémon): its sheet, what it's doing, and its big sprite
struct MpPmdView
{
    struct MpPmdChunk *chunk;   // NULL = none
    u8 anim;                    // PMD_ANIM_* asked for
    u8 shown;                   // the animation really playing (the nearest one the sheet has)
    u8 dir;                     // PMD_DIR_*
    u8 frame;
    u16 timer;                  // 60ths into the frame
    bool8 loop;                 // FALSE: holds the last frame (MP_Pmd_Done)
    bool8 done;
    bool8 shiny;
    u16 idle;                   // MP_Pmd_Animate: frames standing with nothing to do
    u8 fastPhase;               // ... running: every other frame
    struct MpBigSprite big;
    u16 extraOwn[240];          // 8 bpp: colors 16+ before fades (big.extraShown = these, faded)
};

struct MpPmdIndex { u16 dex; u8 variant; u32 offset, size, raw; };

bool8 MP_Pmd_Has(u16 dex);   // national dex number
// Shows the Pokémon on `sprite` (its palette slot gets colors16: load them there). FALSE: no sheet for it.
bool8 MP_Pmd_Attach(struct MpPmdView *v, struct Sprite *sprite, u16 dex, bool8 female, bool8 shiny, s16 hue, u16 *colors16);
void MP_Pmd_Detach(struct MpPmdView *v, struct Sprite *sprite);
void MP_Pmd_SetAnim(struct MpPmdView *v, u8 anim, u8 dir, bool8 loop);
void MP_Pmd_Tick(struct MpPmdView *v, struct Sprite *sprite);   // once a frame
bool8 MP_Pmd_Done(const struct MpPmdView *v);                   // a non-looping animation reached its end
u16 MP_Pmd_Height(const struct MpPmdView *v);                   // how far the picture reaches above the ground point
u8 MP_Pmd_DirFrom4(u8 fdir);                                    // the followers' DOWN, UP, LEFT, RIGHT -> PMD_DIR_*
u8 MP_Pmd_DirToward(s16 dx, s16 dy, u8 fallback);               // 8-way, from a vector
void MP_Pmd_FrameUpdate(void);                                  // frees sheets nobody shows any more
bool8 MP_Pmd_HasAnim(const struct MpPmdView *v, u8 anim);
// A follower's / Garden Pokémon's state as an animation: walking, or its pose (LIFE_POSE_*), facing face4 (0 down, 1 up,
// 2 left, 3 right); *pdir turns 45 degrees at a time toward it (*pturn: frames to the next step)
void MP_Pmd_Animate(struct MpPmdView *v, u8 *pdir, u8 *pturn, bool8 walking, u8 face4, u8 pose, bool8 fast);
u16 MP_Pmd_Reach(const struct MpPmdView *v);                    // half the bigger side of the picture now
void MP_Pmd_SetSubpriority(struct Sprite *sprite, u8 elevation, u8 subpriority);   // SetObjectSubpriorityByElevation for it
#define PMD_GROUND_DY 8   // a PMD picture's ground point below the old 32x32 frames' center: the tile's middle

#endif // GUARD_MP_PMD_H
