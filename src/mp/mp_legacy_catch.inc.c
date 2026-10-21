// Moemon Co-op: the catch routines of the GENESIS BALL (Pokémon Red's POKé BALL)
// and the EPOCH BALL (Pokémon Crystal's POKé BALL), as plain integer code.
//
// Included by mp_legacy_balls.c and by tools/test/legacy_catch_test.c (which
// checks them against a byte-level model of the original assembly), so it only
// uses standard C types. The Makefile skips *.inc.c files.
//
// Inputs are the Kai battle's values: the encounter species' catch rate, the
// target's current / max HP and its Emerald status (mapped below). Every random
// byte comes from `rand8` (0-255), drawn in the same order the originals draw.
//
// References (pret, pinned): pokered af51989 engine/items/item_effects.asm
// ItemUseBall; pokecrystal 97187cd engine/items/item_effects.asm PokeBallEffect,
// engine/battle_anims/pokeball_wobble.asm, data/battle/wobble_probabilities.asm,
// docs/bugs_and_glitches.md.

#include <stdint.h>

// Emerald status1 bits (include/constants/battle.h)
#define LEGACY_STATUS1_SLEEP          0x07
#define LEGACY_STATUS1_POISON         (1 << 3)
#define LEGACY_STATUS1_BURN           (1 << 4)
#define LEGACY_STATUS1_FREEZE         (1 << 5)
#define LEGACY_STATUS1_PARALYSIS      (1 << 6)
#define LEGACY_STATUS1_TOXIC_POISON   (1 << 7)

// Status classes both games use: sleep / freeze, then burn / poison / paralysis.
// Toxic poison counts as poison (in Red it's poison plus a battle flag; Crystal too).
enum { LEGACY_STATUS_NONE, LEGACY_STATUS_MINOR, LEGACY_STATUS_SLEEP_FREEZE };

static int LegacyStatusClass(uint32_t status1)
{
    if (status1 & (LEGACY_STATUS1_SLEEP | LEGACY_STATUS1_FREEZE))
        return LEGACY_STATUS_SLEEP_FREEZE;
    if (status1 & (LEGACY_STATUS1_POISON | LEGACY_STATUS1_BURN | LEGACY_STATUS1_PARALYSIS | LEGACY_STATUS1_TOXIC_POISON))
        return LEGACY_STATUS_MINOR;
    return LEGACY_STATUS_NONE;
}

// ---------------------------------------------------------------------------
// GENESIS BALL: Red's POKé BALL
//
// Result: caught, or how the failure shows: a missed ball (0) or 1-3 shakes.
// The miss is only how an already failed catch looks: it never adds a check.

struct LegacyRedResult
{
    uint8_t caught;
    uint8_t shakes;   // 0 = the ball missed (failures only); a catch shows 3
    // for tests: the values the routine worked with
    uint8_t rand1, rand2, usedRand2;
    uint16_t w;       // ((MaxHP * 255) / 12) / max(HP / 4, 1), before the cap
    uint8_t x, z;
};

static struct LegacyRedResult LegacyRed_Catch(uint8_t catchRate, uint16_t hp, uint16_t maxHp, uint32_t status1,
                                              uint8_t (*rand8)(void *), void *randCtx)
{
    struct LegacyRedResult r = {0};
    int status = LegacyStatusClass(status1);
    uint32_t b, w, hpDiv4;
    uint8_t ailment = status == LEGACY_STATUS_SLEEP_FREEZE ? 25 : status == LEGACY_STATUS_MINOR ? 12 : 0;

    // Rand1: any byte will do for the basic POKé BALL
    r.rand1 = rand8(randCtx);
    b = r.rand1;
    // Status is subtracted from Rand1; going below zero catches it outright
    if (ailment != 0)
    {
        if (b < ailment)
        {
            r.caught = 1;
            r.shakes = 3;
            return r;
        }
        b -= ailment;
    }

    // W = ((MaxHP * 255) / BallFactor) / max(HP / 4, 1); BallFactor 12. Both
    // divisions floor. (Red keeps HP / 4 in one byte: HP never passes 1023 here.)
    hpDiv4 = hp >> 2;
    if (hpDiv4 == 0)
        hpDiv4 = 1;
    w = ((uint32_t)maxHp * 255 / 12) / hpDiv4;
    r.w = w > 0xFFFF ? 0xFFFF : (uint16_t)w;
    r.x = w > 255 ? 255 : (uint8_t)w;

    // Rand1 - Status > CatchRate: the ball fails (so <= passes)
    if (b > catchRate)
        goto failed;
    // W > 255: caught (the low-HP plateau: about a third of the HP or less)
    if (w > 255)
    {
        r.caught = 1;
        r.shakes = 3;
        return r;
    }
    // Rand2 > X: the ball fails (so <= catches)
    r.rand2 = rand8(randCtx);
    r.usedRand2 = 1;
    if (r.rand2 <= r.x)
    {
        r.caught = 1;
        r.shakes = 3;
        return r;
    }

failed:
    {
        // Y = (CatchRate * 100) / 255 (BallFactor2 of the POKé BALL); Z = (X * Y) / 255 + Status2
        uint32_t y = (uint32_t)catchRate * 100 / 255;
        uint32_t z = (uint32_t)r.x * y / 255;
        z += status == LEGACY_STATUS_SLEEP_FREEZE ? 10 : status == LEGACY_STATUS_MINOR ? 5 : 0;
        r.z = (uint8_t)z;   // at most 100 + 10
        r.shakes = z < 10 ? 0 : z < 30 ? 1 : z < 70 ? 2 : 3;
    }
    return r;
}

// ---------------------------------------------------------------------------
// EPOCH BALL: Crystal's POKé BALL, with two documented fixes: burn / poison /
// paralysis add their +5 (the original tested the wrong value), and a max HP
// above 341 no longer overflows its byte (the original truncated it).

struct LegacyCrystalResult
{
    uint8_t caught;
    uint8_t wobbles;   // a failed ball: 0-3 wobbles before it breaks out; a catch shows 3
    uint8_t finalRate; // the final catch value A: caught when a random byte <= A
    uint8_t rand;      // for tests
};

// The final catch value (wFinalCatchRate)
static uint8_t LegacyCrystal_FinalRate(uint8_t catchRate, uint16_t hp, uint16_t maxHp, uint32_t status1)
{
    uint32_t m = (uint32_t)maxHp * 3, h = (uint32_t)hp * 2, a;
    int status = LegacyStatusClass(status1);

    if (maxHp == 0)
        m = 3;   // can't happen in battle; keeps the divisor safe
    if (h > m)
        h = m;   // can't happen either (HP <= max HP)
    // 3 * MaxHP past one byte: both values quartered, then halved until it fits
    // (Crystal quartered once and kept the low byte; the documented fix halves
    // once more; any max HP the game can have needs at most one more halving)
    if (m > 255)
    {
        m >>= 2;
        h >>= 2;
        while (m > 255)
        {
            m >>= 1;
            h >>= 1;
        }
        if (h == 0)
            h = 1;
    }
    if (m == 0)
        m = 1;
    // A = CatchRate * (3 * MaxHP - 2 * HP) / (3 * MaxHP), at least 1
    a = (uint32_t)catchRate * (m - h) / m;
    if (a == 0)
        a = 1;
    // + 10 for sleep / freeze, + 5 for burn / poison / paralysis (the fix), up to 255
    a += status == LEGACY_STATUS_SLEEP_FREEZE ? 10 : status == LEGACY_STATUS_MINOR ? 5 : 0;
    if (a > 255)
        a = 255;
    return (uint8_t)a;
}

// Crystal's wobble table: catch value, chance of wobbling / 255 ((x/255)^4 ~ value/255)
static const uint8_t sLegacyWobbleTable[][2] = {
    {1, 63}, {2, 75}, {3, 84}, {4, 90}, {5, 95}, {7, 103}, {10, 113}, {15, 126},
    {20, 134}, {30, 149}, {40, 160}, {50, 169}, {60, 177}, {80, 191}, {100, 201}, {120, 211},
    {140, 220}, {160, 227}, {180, 234}, {200, 240}, {220, 246}, {240, 251}, {254, 253}, {255, 255},
};

static uint8_t LegacyCrystal_WobbleChance(uint8_t finalRate)
{
    unsigned i;
    for (i = 0; i < sizeof(sLegacyWobbleTable) / sizeof(sLegacyWobbleTable[0]); i++)
        if (sLegacyWobbleTable[i][0] >= finalRate)
            return sLegacyWobbleTable[i][1];
    return 255;
}

// Success is decided first, with one random byte (caught when it's <= A). Only a
// failed ball then rolls its wobbles (GetPokeBallWobble: each one happens while a
// random byte is below the table's chance; after three it breaks out anyway).
// The wobbles never decide the catch.
static struct LegacyCrystalResult LegacyCrystal_Catch(uint8_t catchRate, uint16_t hp, uint16_t maxHp, uint32_t status1,
                                                      uint8_t (*rand8)(void *), void *randCtx)
{
    struct LegacyCrystalResult r = {0};
    r.finalRate = LegacyCrystal_FinalRate(catchRate, hp, maxHp, status1);
    r.rand = rand8(randCtx);
    if (r.rand <= r.finalRate)
    {
        r.caught = 1;
        r.wobbles = 3;
        return r;
    }
    {
        uint8_t chance = LegacyCrystal_WobbleChance(r.finalRate);
        while (r.wobbles < 3 && rand8(randCtx) < chance)
            r.wobbles++;
    }
    return r;
}
