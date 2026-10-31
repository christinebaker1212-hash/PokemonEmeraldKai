// Moemon Co-op: battle intro diagnostics (AllowSmallTweaks = 1 in moemon_coop.ini, or MOEMON_BATTLEDIAG=1 in tests).
//
// As a battle's intro begins, the log gets what was queued for every battler: species, level, HP, the cry
// (its number in the cry tables and whether that entry holds a sound), the POKeMON picture's sprite, and then
// each intro step as it's reached, each cry and front / back animation as it starts. The intro ends at the first
// action menu ("intro done"). If a step doesn't move on for 10 seconds (20 in a link battle), what the intro is
// waiting on is written in the battle's text box (and the log): the step, the foe's species and level, whether its
// sprite is shown and whether its cry played. Nothing is shown when all goes as it should.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "battle.h"
#include "battle_message.h"
#include "battle_anim.h"
#include "data.h"
#include "sprite.h"
#include "text.h"
#include "string_util.h"
#include "constants/battle.h"
#include "mp.h"

bool8 MP_ConfigAllowsTweaks(void);
void MP_GameToAscii(char *dst, const u8 *src, int maxLen);
void MP_AsciiToGame(u8 *dst, const char *src, int maxLen);
u16 SpeciesToCryId(u16 species);
bool8 MP_CryValid(u32 cry);

#define STUCK_FRAMES 600

static struct
{
    bool8 on, checked, active, reported, dumped;
    const char *stage;
    u16 stageFrames, frames;
    u8 stagesSeen;
    u16 cries[8], cryCount;
    bool8 cryOk[8];
    u16 anims[8], animCount, animSpecies[8];
} sDiag;

static bool8 On(void)
{
    if (!sDiag.checked)
    {
        sDiag.checked = TRUE;
        sDiag.on = MP_ConfigAllowsTweaks() || getenv("MOEMON_BATTLEDIAG") != NULL;
    }
    return sDiag.on;
}

static const char *Name(u16 species, char *buf)
{
    if (species == SPECIES_NONE || species >= NUM_SPECIES)
    {
        sprintf(buf, "#%u", species);
        return buf;
    }
    MP_GameToAscii(buf, gSpeciesNames[species], POKEMON_NAME_LENGTH);
    return buf;
}

static bool8 SpriteShown(u8 battler)
{
    u8 id = gBattlerSpriteIds[battler];
    return id < MAX_SPRITES && gSprites[id].inUse && !gSprites[id].invisible;
}

// Everything queued for the battlers, once their data is in
static void Dump(void)
{
    u8 b;
    char name[16];
    printf("[diag] battle: type 0x%08x, %u battlers\n", (unsigned)gBattleTypeFlags, gBattlersCount);
    for (b = 0; b < gBattlersCount && b < MAX_BATTLERS_COUNT; b++)
    {
        u16 species = gBattleMons[b].species;
        u32 cry = species ? SpeciesToCryId(species - 1) : 0;
        u8 id = gBattlerSpriteIds[b];
        printf("[diag]   battler %u (%s): %s Lv%u HP %u/%u, cry %u (%s), sprite %u%s\n", b,
               GetBattlerSide(b) == B_SIDE_PLAYER ? "player" : "foe", Name(species, name), gBattleMons[b].level,
               gBattleMons[b].hp, gBattleMons[b].maxHP, (unsigned)cry, MP_CryValid(cry) ? "has a sound" : "NO SOUND",
               id, id < MAX_SPRITES && gSprites[id].inUse ? "" : " (not made yet)");
    }
}

static void Report(const char *why)
{
    u8 b, foe = B_POSITION_OPPONENT_LEFT;
    char line[160], name[16];
    static u8 text[200];   // (the text printer reads it after this returns)
    u16 species = gBattleMons[foe].species;
    bool8 cried = FALSE;
    u32 i;
    for (i = 0; i < sDiag.cryCount && i < ARRAY_COUNT(sDiag.cries); i++)
        cried |= sDiag.cries[i] == species;
    printf("[diag] intro STUCK at %s for %u frames: %s\n", sDiag.stage, sDiag.stageFrames, why);
    Dump();
    for (b = 0; b < gBattlersCount && b < MAX_BATTLERS_COUNT; b++)
    {
        u8 id = gBattlerSpriteIds[b];
        if (id < MAX_SPRITES && gSprites[id].inUse)
            printf("[diag]   battler %u sprite: (%d,%d) %s, callback %p\n", b, gSprites[id].x + gSprites[id].x2,
                   gSprites[id].y + gSprites[id].y2, gSprites[id].invisible ? "hidden" : "shown",
                   (void *)gSprites[id].callback);
    }
    // (the text box: two lines)
    snprintf(line, sizeof(line), "INTRO STUCK: %s\n%s Lv%u %s, %s", sDiag.stage, Name(species, name),
             gBattleMons[foe].level, SpriteShown(foe) ? "shown" : "NOT SHOWN", cried ? "cried" : "NO CRY");
    MP_AsciiToGame(text, line, sizeof(text) - 1);
    BattlePutTextOnWindow(text, B_WIN_MSG);
    fflush(stdout);
}

// At the first action menu: each battler on the field expected shown, with its cry and an animation; what happened
static void Expected(void)
{
    u8 b;
    u32 i;
    char name[16];
    for (b = 0; b < gBattlersCount && b < MAX_BATTLERS_COUNT; b++)
    {
        u16 species = gBattleMons[b].species;
        bool8 cried = FALSE, cryOk = TRUE, animated = FALSE;
        if (species == SPECIES_NONE || gBattleMons[b].hp == 0)
            continue;
        for (i = 0; i < sDiag.cryCount && i < ARRAY_COUNT(sDiag.cries); i++)
            if (sDiag.cries[i] == species)
                cried = TRUE, cryOk &= sDiag.cryOk[i];
        for (i = 0; i < sDiag.animCount && i < ARRAY_COUNT(sDiag.animSpecies); i++)
            animated |= sDiag.animSpecies[i] == species;
        printf("[diag]   battler %u %s: expected shown, cry, animation; got %s, %s, %s%s\n", b, Name(species, name),
               SpriteShown(b) ? "shown" : "NOT SHOWN", cried ? (cryOk ? "cry" : "CRY WITHOUT SOUND") : "NO CRY",
               animated ? "animation" : "no animation",
               SpriteShown(b) && cried && cryOk ? "" : "  <- out of order");
    }
}

// Every battle frame (battle_main.c BattleMainCB1): the intro step now, or NULL once the intro is over
void MP_BattleDiag_Frame(const char *stage, bool8 linked)
{
    if (!On())
        return;
    if (stage != NULL && !sDiag.active)
    {
        memset(&sDiag, 0, sizeof(sDiag));   // (a new battle)
        sDiag.on = sDiag.checked = TRUE;
        sDiag.active = TRUE;
    }
    if (!sDiag.active)
        return;
    sDiag.frames++;
    if (!sDiag.dumped && gBattleMons[B_POSITION_OPPONENT_LEFT].species != SPECIES_NONE)
    {
        sDiag.dumped = TRUE;   // (the battlers' data is in)
        Dump();
    }
    if (stage == NULL)
    {
        Expected();
        printf("[diag] intro done in %u frames (%u steps, %u cries, %u animations)\n", sDiag.frames, sDiag.stagesSeen,
               sDiag.cryCount, sDiag.animCount);
        sDiag.active = FALSE;
        return;
    }
    if (stage != sDiag.stage)
    {
        printf("[diag] step %u: %s at frame %u\n", sDiag.stagesSeen, stage, sDiag.frames);
        sDiag.stage = stage;
        sDiag.stageFrames = 0;
        sDiag.stagesSeen++;
        return;
    }
    // (not while the text box prints or waits for a button: a player reading isn't stuck)
    if (IsTextPrinterActive(B_WIN_MSG))
    {
        sDiag.stageFrames = 0;
        return;
    }
    // (tests: MOEMON_BATTLEDIAG_STUCK=frames, sooner)
    if (++sDiag.stageFrames == (getenv("MOEMON_BATTLEDIAG_STUCK") ? (u16)atoi(getenv("MOEMON_BATTLEDIAG_STUCK"))
                                : linked ? STUCK_FRAMES * 2 : STUCK_FRAMES) && !sDiag.reported)
    {
        u8 foe = B_POSITION_OPPONENT_LEFT;
        sDiag.reported = TRUE;
        Report(!SpriteShown(foe) ? "the foe's sprite isn't shown" : "nothing moved on");
    }
}

// sound.c PlayCryInternal: a cry starting (ok: its table entry holds a sound)
void MP_BattleDiag_Cry(u16 species, u32 cry, bool8 ok)
{
    char name[16];
    if (!sDiag.active || !gMain.inBattle)
        return;
    if (sDiag.cryCount < ARRAY_COUNT(sDiag.cries))
    {
        sDiag.cries[sDiag.cryCount] = species;
        sDiag.cryOk[sDiag.cryCount] = ok;
    }
    sDiag.cryCount++;
    printf("[diag] cry: %s (cry %u) %s\n", Name(species, name), (unsigned)cry, ok ? "playing" : "HAS NO SOUND");
}

// pokemon_animation.c: a front (or back) animation starting on a battler's sprite
void MP_BattleDiag_Anim(u16 species, u8 animId, bool8 back)
{
    char name[16];
    if (!sDiag.active || !gMain.inBattle)
        return;
    if (sDiag.animCount < ARRAY_COUNT(sDiag.anims))
    {
        sDiag.anims[sDiag.animCount] = animId;
        sDiag.animSpecies[sDiag.animCount] = species;
    }
    sDiag.animCount++;
    printf("[diag] %s animation %u: %s\n", back ? "back" : "front", animId, Name(species, name));
}
