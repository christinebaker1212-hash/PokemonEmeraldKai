#ifndef GUARD_DATA_H
#define GUARD_DATA_H

#include "constants/moves.h"

#define SPECIES_SHINY_TAG 500

#define MAX_TRAINER_ITEMS 4

#define TRAINER_PIC_WIDTH 64
#define TRAINER_PIC_HEIGHT 64
#define TRAINER_PIC_SIZE (TRAINER_PIC_WIDTH * TRAINER_PIC_HEIGHT / 2)

// Red and Leaf's back pics have 5 frames, but this is presumably irrelevant in the places this is used.
#define MAX_TRAINER_PIC_FRAMES 4

enum {
    BATTLER_AFFINE_NORMAL,
    BATTLER_AFFINE_EMERGE,
    BATTLER_AFFINE_RETURN,
};

struct MonCoords
{
    // This would use a bitfield, but some function
    // uses it as a u8 and casting won't match.
    u8 size; // u8 width:4, height:4;
    u8 y_offset;
};

#define MON_COORDS_SIZE(width, height) (DIV_ROUND_UP(width, 8) << 4 | DIV_ROUND_UP(height, 8))
#define GET_MON_COORDS_WIDTH(size) ((size >> 4) * 8)
#define GET_MON_COORDS_HEIGHT(size) ((size & 0xF) * 8)

struct TrainerMonNoItemDefaultMoves
{
    u16 iv;
    u8 lvl;
    u16 species;
};

struct TrainerMonItemDefaultMoves
{
    u16 iv;
    u8 lvl;
    u16 species;
    u16 heldItem;
};

struct TrainerMonNoItemCustomMoves
{
    u16 iv;
    u8 lvl;
    u16 species;
    u16 moves[MAX_MON_MOVES];
};

struct TrainerMonItemCustomMoves
{
    u16 iv;
    u8 lvl;
    u16 species;
    u16 heldItem;
    u16 moves[MAX_MON_MOVES];
};

#define NO_ITEM_DEFAULT_MOVES(party) { .NoItemDefaultMoves = party }, .partySize = ARRAY_COUNT(party), .partyFlags = 0
#define NO_ITEM_CUSTOM_MOVES(party) { .NoItemCustomMoves = party }, .partySize = ARRAY_COUNT(party), .partyFlags = F_TRAINER_PARTY_CUSTOM_MOVESET
#define ITEM_DEFAULT_MOVES(party) { .ItemDefaultMoves = party }, .partySize = ARRAY_COUNT(party), .partyFlags = F_TRAINER_PARTY_HELD_ITEM
#define ITEM_CUSTOM_MOVES(party) { .ItemCustomMoves = party }, .partySize = ARRAY_COUNT(party), .partyFlags = F_TRAINER_PARTY_CUSTOM_MOVESET | F_TRAINER_PARTY_HELD_ITEM

union TrainerMonPtr
{
    const struct TrainerMonNoItemDefaultMoves *NoItemDefaultMoves;
    const struct TrainerMonNoItemCustomMoves *NoItemCustomMoves;
    const struct TrainerMonItemDefaultMoves *ItemDefaultMoves;
    const struct TrainerMonItemCustomMoves *ItemCustomMoves;
};

struct Trainer
{
    /*0x00*/ u8 partyFlags;
    /*0x01*/ u8 trainerClass;
    /*0x02*/ u8 encounterMusic_gender; // last bit is gender
    /*0x03*/ u8 trainerPic;
    /*0x04*/ u8 trainerName[TRAINER_NAME_LENGTH + 1];
    /*0x10*/ u16 items[MAX_TRAINER_ITEMS];
    /*0x18*/ bool8 doubleBattle;
    /*0x1C*/ u32 aiFlags;
    /*0x20*/ u8 partySize;
    /*0x24*/ union TrainerMonPtr party;
};

#define TRAINER_ENCOUNTER_MUSIC(trainer) ((gTrainers[trainer].encounterMusic_gender & 0x7F))

extern const u16 gMinigameDigits_Pal[];
extern const u32 gMinigameDigits_Gfx[];

extern const struct SpriteFrameImage gBattlerPicTable_PlayerLeft[];
extern const struct SpriteFrameImage gBattlerPicTable_OpponentLeft[];
extern const struct SpriteFrameImage gBattlerPicTable_PlayerRight[];
extern const struct SpriteFrameImage gBattlerPicTable_OpponentRight[];
extern const struct SpriteFrameImage gTrainerBackPicTable_Brendan[];
extern const struct SpriteFrameImage gTrainerBackPicTable_May[];
extern const struct SpriteFrameImage gTrainerBackPicTable_Red[];
extern const struct SpriteFrameImage gTrainerBackPicTable_Leaf[];
extern const struct SpriteFrameImage gTrainerBackPicTable_RubySapphireBrendan[];
extern const struct SpriteFrameImage gTrainerBackPicTable_RubySapphireMay[];
extern const struct SpriteFrameImage gTrainerBackPicTable_Wally[];
extern const struct SpriteFrameImage gTrainerBackPicTable_Steven[];

extern const union AffineAnimCmd *const gAffineAnims_BattleSpritePlayerSide[];
extern const union AffineAnimCmd *const gAffineAnims_BattleSpriteOpponentSide[];
extern const union AffineAnimCmd *const gAffineAnims_BattleSpriteContest[];

extern const union AnimCmd *const gAnims_MonPic[];
extern const struct CompressedSpriteSheet gMonStillFrontPicTable[];
extern const union AnimCmd *const *const gTrainerFrontAnimsPtrTable[];
extern const struct MonCoords gTrainerFrontPicCoords[];
extern const struct CompressedSpriteSheet gTrainerFrontPicTable[];
extern const struct CompressedSpritePalette gTrainerFrontPicPaletteTable[];
extern const union AnimCmd *const *const gTrainerBackAnimsPtrTable[];
extern const struct MonCoords gTrainerBackPicCoords[];
extern const struct CompressedSpriteSheet gTrainerBackPicTable[]; // functionally unused
extern const struct CompressedSpritePalette gTrainerBackPicPaletteTable[];

// Moemon Co-op: runtime data for TRAINER_PIC_MP_* / TRAINER_BACK_PIC_MP_LOCAL
// (LZ77 streams, see MP_Pack_WriteLz77Literal)
#define MP_PIC_LZ_WORDS ((4 + TRAINER_PIC_SIZE + TRAINER_PIC_SIZE / 8 + 8) / 4)
#define MP_PAL_LZ_WORDS 16
extern u32 gMpTrainerFrontPicLz[2][MP_PIC_LZ_WORDS];
extern u32 gMpTrainerFrontPalLz[2][MP_PAL_LZ_WORDS];
extern u32 gMpTrainerBackPalLz[MP_PAL_LZ_WORDS];
extern u8 gMpTrainerBackTiles[4 * TRAINER_PIC_SIZE];
extern const struct SpriteFrameImage gMpTrainerBackFrames[4];
extern u32 gMpPartnerBackPalLz[MP_PAL_LZ_WORDS];
extern u8 gMpPartnerBackTiles[4 * TRAINER_PIC_SIZE];
extern const struct SpriteFrameImage gMpPartnerBackFrames[4];
extern u32 gMpPartner2BackPalLz[MP_PAL_LZ_WORDS];
extern u8 gMpPartner2BackTiles[4 * TRAINER_PIC_SIZE];
extern const struct SpriteFrameImage gMpPartner2BackFrames[4];

// Moemon Co-op: OPTIONS > CREATURES picks Kai's battle pictures (_Moe) or the original
// POKéMON ones (_Pokemon, src/mp/mp_monart_data.c from tools/moemon_monart), with their own
// coordinates, elevations and animations. Code reading the old names gets the chosen set.
extern const struct CompressedSpriteSheet gMonFrontPicTable_Moe[], gMonFrontPicTable_Pokemon[];
extern const struct CompressedSpriteSheet gMonBackPicTable_Moe[], gMonBackPicTable_Pokemon[];
extern const struct CompressedSpritePalette gMonPaletteTable_Moe[], gMonPaletteTable_Pokemon[];
extern const struct CompressedSpritePalette gMonShinyPaletteTable_Moe[], gMonShinyPaletteTable_Pokemon[];
extern const struct MonCoords gMonFrontPicCoords_Moe[], gMonFrontPicCoords_Pokemon[];
extern const struct MonCoords gMonBackPicCoords_Moe[], gMonBackPicCoords_Pokemon[];
extern const u8 gEnemyMonElevation_Moe[NUM_SPECIES], gEnemyMonElevation_Pokemon[NUM_SPECIES];
extern const union AnimCmd *const *const gMonFrontAnimsPtrTable_Moe[], *const *const gMonFrontAnimsPtrTable_Pokemon[];
bool8 MP_MonArtPokemon(void);   // mp_monart.c
#define MP_MONART(table) (MP_MonArtPokemon() ? table##_Pokemon : table##_Moe)
#define gMonFrontPicTable      MP_MONART(gMonFrontPicTable)
#define gMonBackPicTable       MP_MONART(gMonBackPicTable)
#define gMonPaletteTable       MP_MONART(gMonPaletteTable)
#define gMonShinyPaletteTable  MP_MONART(gMonShinyPaletteTable)
#define gMonFrontPicCoords     MP_MONART(gMonFrontPicCoords)
#define gMonBackPicCoords      MP_MONART(gMonBackPicCoords)
#define gEnemyMonElevation     MP_MONART(gEnemyMonElevation)
#define gMonFrontAnimsPtrTable MP_MONART(gMonFrontAnimsPtrTable)

extern const struct Trainer gTrainers[];
extern const u8 gTrainerClassNames[][13];
extern const u8 gSpeciesNames[][POKEMON_NAME_LENGTH + 1];
extern const u8 gMoveNames[MOVES_COUNT][MOVE_NAME_LENGTH + 1];

#endif // GUARD_DATA_H
