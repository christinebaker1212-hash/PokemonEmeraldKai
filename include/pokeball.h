#ifndef GUARD_POKEBALL_H
#define GUARD_POKEBALL_H

enum
{
    BALL_POKE,
    BALL_GREAT,
    BALL_SAFARI,
    BALL_ULTRA,
    BALL_MASTER,
    BALL_NET,
    BALL_DIVE,
    BALL_NEST,
    BALL_REPEAT,
    BALL_TIMER,
    BALL_LUXURY,
    BALL_PREMIER,
    POKEBALL_COUNT
};

// Moemon Co-op: the HEARTGOLD / SOULSILVER balls (tools/moemon_balls/hgss_balls.py): eleven 16x32 frames per ball
#define BALL_FRAME_TILES   8                          // a 16x32 frame
#define BALL_FRAME_COUNT   11
#define BALL_SHEET_SIZE    (BALL_FRAME_COUNT * BALL_FRAME_TILES * TILE_SIZE_4BPP)
enum {
    BALL_FRAME_REST,       // closed
    BALL_FRAME_OPEN,
    BALL_FRAME_SQUISH,     // squashed, just before it opens
    BALL_FRAME_SPIN,       // 3-10: spinning in the air
};
#define BALL_ANIM_SPIN 7   // sBallAnimSequences: the spin frames over and over (a ball in the air)

enum {
    BALL_AFFINE_ANIM_0,
    BALL_ROTATE_RIGHT,
    BALL_ROTATE_LEFT,
    BALL_AFFINE_ANIM_3,
    BALL_AFFINE_ANIM_4
};

extern const struct CompressedSpriteSheet gBallSpriteSheets[];
extern const struct CompressedSpritePalette gBallSpritePalettes[];
extern const struct SpriteTemplate gBallSpriteTemplates[];

#define POKEBALL_PLAYER_SENDOUT     0xFF
#define POKEBALL_OPPONENT_SENDOUT   0xFE

u8 DoPokeballSendOutAnimation(s16 pan, u8 kindOfThrow);
void CreatePokeballSpriteToReleaseMon(u8 monSpriteId, u8 monPalNum, u8 x, u8 y, u8 oamPriority, u8 subpriority, u8 delay, u32 fadePalettes, u16 species);
u8 CreateTradePokeballSprite(u8 monSpriteId, u8 monPalNum, u8 x, u8 y, u8 oamPriority, u8 subPriority, u8 delay, u32 fadePalettes);
void StartHealthboxSlideIn(u8 battler);
void DoHitAnimHealthboxEffect(u8 battler);
void LoadBallGfx(u8 ballId);
void FreeBallGfx(u8 ballId);
bool8 StartBallSpin(struct Sprite *sprite);   // Moemon Co-op: FALSE = no spin frames (turn it instead)

#endif // GUARD_POKEBALL_H
