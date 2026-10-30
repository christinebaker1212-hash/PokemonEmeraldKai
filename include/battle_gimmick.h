#ifndef GUARD_BATTLE_GIMMICK_H
#define GUARD_BATTLE_GIMMICK_H

// Moemon Kai: the battle GIMMICKS (MEGA EVOLUTION, Z-MOVES, DYNAMAX, TERASTALLIZATION). Each trainer may use one
// of them once a battle. The battle side (what each one does) is in src/battle_gen59.c, the rest (settings, the
// move menu's button, the pictures and tints every screen shows, the MEGA forms' data) in src/battle_gimmick.c.

#define GIMMICK_NONE    0
#define GIMMICK_MEGA    1
#define GIMMICK_Z       2
#define GIMMICK_DYNAMAX 3
#define GIMMICK_TERA    4
#define GIMMICK_COUNT   5

// OPTIONS > GAME / HOST A GAME > GAME RULES: each gimmick AUTO (from its own generation: MEGA 6, Z 7, DYNAMAX 8,
// TERA 9), ON or OFF; 2 bits each in gMpConfig.gimmicks (gimmick - 1)
#define GIMMICK_SET_AUTO 0
#define GIMMICK_SET_ON   1
#define GIMMICK_SET_OFF  2

// what the move menu may offer (the engine sends it with the move choice, BtlController_EmitChooseMove byte 3):
// bit 0 MEGA, bit 1 DYNAMAX, bit 2 TERA, bits 3-7 the Z-CRYSTAL's type + 1
#define GIMMICK_AVAIL_MEGA    (1 << 0)
#define GIMMICK_AVAIL_DYNAMAX (1 << 1)
#define GIMMICK_AVAIL_TERA    (1 << 2)
#define GIMMICK_AVAIL_Z_TYPE(avail) (((avail) >> 3) - 1)   // (only when (avail) >> 3 != 0)

// the move choice's byte (gBattleBufferB[battler][2]): the move's slot | gimmick << 4
#define GIMMICK_MOVE_SHIFT 4

// the gimmick animation's argument (B_ANIM_MP_GIMMICK): kind << 8 | value
#define GIMMICK_ANIM_MEGA        1   // value: the MEGA form
#define GIMMICK_ANIM_TERA        2   // value: the TERA type
#define GIMMICK_ANIM_DYNAMAX     3
#define GIMMICK_ANIM_DYNAMAX_END 4

struct MegaForm
{
    u16 species;
    u16 stone;           // ITEM_NONE: RAYQUAZA, which needs DRAGON ASCENT
    u8 types[2];
    u16 ability;
    u8 stats[5];         // ATK DEF SPEED SPATK SPDEF (HP stays)
    u8 frontY, backY;    // the pictures' y offsets (as gMonFrontPicCoords)
    const u32 *front, *back, *palette, *shinyPalette;
};

u8 MP_Gimmick_Setting(u8 gimmick);             // GIMMICK_SET_* (online, the host's)
bool8 MP_Gimmick_On(u8 gimmick);               // the setting and the GENERATION
bool8 MP_Gimmick_ItemAllowed(u16 item);        // a MEGA STONE / Z-CRYSTAL while its gimmick is on (marts)
u8 MP_Mega_Count(void);
const struct MegaForm *MP_Mega_Get(u8 form);

// the battle (src/battle_gen59.c)
u8 MP_Gimmick_Available(u8 battler);           // GIMMICK_AVAIL_*
void MP_Gimmick_Choose(u8 battler, u8 gimmick);
bool8 MP_Gimmick_TurnStart(void);              // TRUE: a script was started (call again until FALSE)
void MP_Gimmick_TurnStartReset(void);
s32 MP_Gimmick_Stab(s32 damage, u8 attacker, u8 moveType);
u16 MP_Gimmick_MovePower(u16 move, u8 attacker, u16 power);
u16 MP_Gimmick_MoveUsed(u8 battler, u16 move); // DYNAMAX: a status move is MAX GUARD (PROTECT)
bool8 MP_Gimmick_Dynamaxed(u8 battler);
u8 MP_Gimmick_TeraType(u8 battler);            // TYPE_MYSTERY when not terastallized
void MP_Gimmick_BattleStart(void);
void MP_Gimmick_SwitchIn(u8 battler);
void MP_Gimmick_TurnEnd(void);

// the screens (src/battle_gimmick.c): every machine in a link battle runs these through the animation
void MP_Gimmick_ResetPictures(void);
void MP_Gimmick_AfterMonGfx(struct Pokemon *mon, u8 battler);   // BattleLoad*MonSpriteGfx: a MEGA's picture, tints
u8 MP_Gimmick_YDelta(u8 battler, u8 yDelta);
void AnimTask_MpGimmick(u8 taskId);
u8 MP_Gimmick_AiPick(u8 battler, u16 move);   // the trainer AI's choice (opponent controller)
const u8 *MP_Gimmick_MenuLabel(u8 gimmick, u8 type);   // the move menu's line for a chosen gimmick

#endif // GUARD_BATTLE_GIMMICK_H
