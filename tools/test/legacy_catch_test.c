// GENESIS / EPOCH BALL catch checks (host program; tools/test/legacy_catch_test.sh builds and runs it).
//
// Compares src/mp/mp_legacy_catch.inc.c with register-level transliterations of
// the original routines (pret pokered af51989 ItemUseBall, pokecrystal 97187cd
// PokeBallEffect + GetPokeBallWobble), written instruction by instruction with
// 8-bit registers and the carry flag, then checks the plan's properties:
//   - Red: every catch rate, Rand1, Rand2 at and around X, all statuses, many HPs
//     (full, half, a third, low, 1; max HP 1-714): same result and shakes
//   - Crystal: the final value for every catch rate, HP and max HP up to 714, all
//     statuses: equal to the original with the two documented fixes wherever the
//     fixed original is defined (max HP <= 680), equal to the unfixed original
//     (status fix aside) up to 341; never 0, saturates at 255, success = rand <= A
//   - monotonic: less HP or a status never lowers the catch probability
//   - wobbles: the table lookup of the original
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../src/mp/mp_legacy_catch.inc.c"

static int sFails;
#define CHECK(cond, ...) do { if (!(cond)) { if (sFails++ < 20) { printf("FAIL: " __VA_ARGS__); printf("\n"); } } } while (0)

// A fixed list of random bytes
struct Rands { const uint8_t *v; int n, at; };
static uint8_t NextRand(void *ctx)
{
    struct Rands *r = ctx;
    return r->at < r->n ? r->v[r->at++] : 0;
}

// Gen 1 / 2 status byte from an Emerald one (the same mapping the game makes):
// bits 0-2 sleep, 3 poison, 4 burn, 5 freeze, 6 paralysis; toxic = poison
static uint8_t GbStatus(uint32_t st)
{
    uint8_t s = st & 0x7F;
    if (st & LEGACY_STATUS1_TOXIC_POISON)
        s |= LEGACY_STATUS1_POISON;
    return s;
}
#define SLP_MASK 0x07
#define FRZ 5

// ---------------------------------------------------------------------------
// pokered ItemUseBall, POKé BALL path (wCurItem == POKE_BALL), transliterated

struct RedAsm { int caught; int animData; };

static struct RedAsm RedAsm(uint8_t catchRate, uint16_t hp, uint16_t maxHp, uint8_t status, const uint8_t *rands)
{
    struct RedAsm out = {0, 0};
    uint8_t a, b, c, carry, q2, q3, X;
    uint32_t product, quotient;
    int ri = 0;

    // .loop: call Random / ld b, a; POKE_BALL -> .checkForAilments
    a = rands[ri++];
    b = a;
    // .checkForAilments
    a = status;
    if (a != 0)                                   // and a / jr z
    {
        a &= (1 << FRZ) | SLP_MASK;
        c = 12;
        if (a != 0)
            c = 25;
        // ld a, b / sub c / jp c, .captured / ld b, a
        carry = b < c;
        a = (uint8_t)(b - c);
        if (carry)
            goto captured;
        b = a;
    }
    // push bc; MaxHP * 255
    product = (uint32_t)maxHp * 255;              // hMultiplicand (3 bytes) * hMultiplier
    // BallFactor 12 (not a GREAT_BALL); Divide (4 bytes) by 12
    quotient = product / 12;
    // HP / 4 in a: ld b, [hp hi]; ld a, [hp lo]; srl b; rr a; srl b; rr a
    {
        uint8_t hi = hp >> 8, lo = hp & 0xFF;
        uint8_t cy;
        cy = hi & 1; hi >>= 1; lo = (lo >> 1) | (cy << 7);
        cy = hi & 1; hi >>= 1; lo = (lo >> 1) | (cy << 7);
        a = lo;
    }
    if (a == 0)                                   // and a / jr nz / inc a
        a = 1;
    quotient = quotient / a;                      // Divide by HP/4
    q2 = (quotient >> 8) & 0xFF;                  // hQuotient + 2
    q3 = quotient & 0xFF;                         // hQuotient + 3
    if (q2 != 0)
        q3 = 255;
    // pop bc; ld a, [catch rate]; cp b; jr c, .failedToCapture
    if (catchRate < b)
        goto failed;
    if (q2 != 0)
        goto captured;
    // call Random; ld b, a; ldh a, [q3]; cp b; jr c, .failedToCapture
    b = rands[ri++];
    if (q3 < b)
        goto failed;
captured:
    out.caught = 1;
    out.animData = 0x43;
    return out;
failed:
    X = q3;
    {
        // Y = CatchRate * 100 / 255 (POKE_BALL: BallFactor2 255)
        uint32_t y = (uint32_t)catchRate * 100 / 255;
        uint32_t z;
        if (((y >> 8) & 0xFF) != 0)
        {
            out.animData = 0x63;
            return out;
        }
        z = (y * X) / 255;                        // Multiply by X, Divide by 255
        a = z & 0xFF;                             // hQuotient + 3
        if (status != 0)
        {
            uint8_t s2 = (status & ((1 << FRZ) | SLP_MASK)) ? 10 : 5;
            a = (uint8_t)(a + s2);                // add b (8 bits)
        }
        if (a < 10) out.animData = 0x20;
        else if (a < 30) out.animData = 0x61;
        else if (a < 70) out.animData = 0x62;
        else out.animData = 0x63;
    }
    return out;
}

// ---------------------------------------------------------------------------
// pokecrystal PokeBallEffect, POKé BALL path, up to wFinalCatchRate, transliterated.
// fixStatus / fixHp: the two fixes from docs/bugs_and_glitches.md. Returns -1 for
// a divide by zero (the broken original can get there with max HP > 341).

static int CrystalAsm(uint8_t catchRate, uint16_t hp, uint16_t maxHp, uint8_t status, int fixStatus, int fixHp)
{
    uint8_t a, b, c, d, e, carry;
    uint16_t hl, de, bc;
    uint32_t prod, q;

    // ld b, [hp hi]; ld c, [hp lo]; ld d, [max hi]; ld e, [max lo]
    b = hp >> 8; c = hp & 0xFF; d = maxHp >> 8; e = maxHp & 0xFF;
    // sla c; rl b
    carry = c >> 7; c <<= 1; b = (b << 1) | carry;
    // ld h, d; ld l, e; add hl, de; add hl, de; ld d, h; ld e, l
    de = (d << 8) | e;
    hl = de; hl += de; hl += de;
    d = hl >> 8; e = hl & 0xFF;
    if (d != 0)                                   // ld a, d; and a; jr z, .okay_1
    {
        // srl d; rr e (x2); srl b; rr c (x2)
        de = (d << 8) | e; bc = (b << 8) | c;
        de >>= 2; bc >>= 2;
        if (fixHp && (de >> 8) != 0)
        {
            de >>= 1;
            bc >>= 1;
        }
        d = de >> 8; e = de & 0xFF; b = bc >> 8; c = bc & 0xFF;
        if (c == 0)                               // ld a, c; and a; jr nz; ld c, 1
            c = 1;
    }
    // .okay_1: ld b, e
    b = e;
    // push bc; ld a, b; sub c; ldh [hMultiplier], a; Multiply (catch rate * (b - c))
    a = (uint8_t)(b - c);
    prod = (uint32_t)catchRate * a;
    // pop bc; ld a, b; ldh [hDivisor], a; Divide
    if (b == 0)
        return -1;
    q = prod / b;
    a = q & 0xFF;                                 // ldh a, [hQuotient + 3]
    if (a == 0)
        a = 1;
    // .statuscheck
    b = a;
    a = status;
    a &= (1 << FRZ) | SLP_MASK;
    c = 10;
    if (a == 0)
    {
        if (fixStatus)
            a = status;                           // the fix: ld a, [wEnemyMonStatus]
        c = 5;
        if (a == 0)                               // and a; jr nz, .addstatus
            c = 0;
    }
    // .addstatus: ld a, b; add c; jr nc; ld a, $ff
    {
        unsigned s = (unsigned)b + c;
        a = s > 255 ? 255 : (uint8_t)s;
    }
    return a;
}

// GetPokeBallWobble's table search
static uint8_t CrystalWobbleAsm(uint8_t finalRate)
{
    static const uint8_t table[] = {1, 63, 2, 75, 3, 84, 4, 90, 5, 95, 7, 103, 10, 113, 15, 126, 20, 134, 30, 149,
        40, 160, 50, 169, 60, 177, 80, 191, 100, 201, 120, 211, 140, 220, 160, 227, 180, 234, 200, 240, 220, 246,
        240, 251, 254, 253, 255, 255};
    const uint8_t *hl = table;
    for (;;)
    {
        uint8_t a = *hl++;                        // ld a, [hli]; cp b; jr nc, .checkwobble
        if (a >= finalRate)
            return *hl;
        hl++;
    }
}

// ---------------------------------------------------------------------------

static const uint32_t sStatuses[] = {
    0, LEGACY_STATUS1_SLEEP & 1, LEGACY_STATUS1_SLEEP, LEGACY_STATUS1_POISON, LEGACY_STATUS1_BURN,
    LEGACY_STATUS1_FREEZE, LEGACY_STATUS1_PARALYSIS, LEGACY_STATUS1_TOXIC_POISON | (3 << 8),
};
#define NSTAT (int)(sizeof(sStatuses) / sizeof(sStatuses[0]))

// Red's catch probability (out of 65536) for one input, by enumerating both random bytes
static uint32_t RedOdds(uint8_t cr, uint16_t hp, uint16_t maxHp, uint32_t st)
{
    uint32_t n = 0;
    int r1, r2;
    for (r1 = 0; r1 < 256; r1++)
        for (r2 = 0; r2 < 256; r2++)
        {
            uint8_t v[2] = {(uint8_t)r1, (uint8_t)r2};
            struct Rands rs = {v, 2, 0};
            n += LegacyRed_Catch(cr, hp, maxHp, st, NextRand, &rs).caught;
        }
    return n;
}

int main(int argc, char **argv)
{
    static const uint16_t sMaxHps[] = {1, 2, 3, 4, 11, 12, 13, 20, 35, 85, 86, 100, 150, 255, 256, 300,
                                       340, 341, 342, 400, 511, 512, 600, 680, 681, 700, 714};
    long checked = 0;
    unsigned mi;
    int quick = argc > 1 && strcmp(argv[1], "quick") == 0;

    // ---- Red: against the transliteration
    for (mi = 0; mi < sizeof(sMaxHps) / sizeof(sMaxHps[0]); mi++)
    {
        uint16_t maxHp = sMaxHps[mi];
        uint16_t hps[8] = {maxHp, (uint16_t)(maxHp / 2), (uint16_t)(maxHp / 3), (uint16_t)(maxHp / 3 + 1),
                           (uint16_t)(maxHp / 4), (uint16_t)(maxHp / 12), 4, 1};
        int hi;
        for (hi = 0; hi < 8; hi++)
        {
            uint16_t hp = hps[hi];
            int si, cr, r1;
            if (hp == 0 || hp > maxHp)
                continue;
            for (si = 0; si < NSTAT; si++)
                for (cr = 0; cr < 256; cr++)
                    for (r1 = 0; r1 < 256; r1++)
                    {
                        // Rand2 around X (the only values that can change the outcome), and its ends
                        uint8_t probe[2] = {(uint8_t)r1, 0};
                        struct Rands rs0 = {probe, 2, 0};
                        struct LegacyRedResult r0 = LegacyRed_Catch(cr, hp, maxHp, sStatuses[si], NextRand, &rs0);
                        int r2s[6] = {0, r0.x - 1, r0.x, r0.x + 1, 254, 255}, k;
                        for (k = 0; k < 6; k++)
                        {
                            uint8_t v[2];
                            struct Rands rs;
                            struct LegacyRedResult r;
                            struct RedAsm ref;
                            if (r2s[k] < 0 || r2s[k] > 255)
                                continue;
                            v[0] = r1; v[1] = r2s[k];
                            rs.v = v; rs.n = 2; rs.at = 0;
                            r = LegacyRed_Catch(cr, hp, maxHp, sStatuses[si], NextRand, &rs);
                            ref = RedAsm(cr, hp, maxHp, GbStatus(sStatuses[si]), v);
                            CHECK(r.caught == ref.caught, "red caught cr %d hp %d/%d st %x r %d %d: %d vs %d", cr, hp, maxHp, sStatuses[si], v[0], v[1], r.caught, ref.caught);
                            if (ref.caught)
                                CHECK(r.shakes == 3, "red caught shakes %d", r.shakes);
                            else
                                CHECK(r.shakes == (ref.animData == 0x20 ? 0 : ref.animData - 0x60),
                                      "red shakes cr %d hp %d/%d st %x r %d %d: %d vs anim %x", cr, hp, maxHp, sStatuses[si], v[0], v[1], r.shakes, ref.animData);
                            checked++;
                        }
                    }
        }
    }
    printf("red: %ld throws match the original\n", checked);

    // Red's plateau: at a third of the HP or less (W > 255), passing the catch-rate check catches
    {
        uint8_t v[2] = {0, 255};
        struct Rands rs = {v, 2, 0};
        // max HP 300: W = 6375 / (HP / 4) passes 255 at HP / 4 <= 24, so HP 99 and under (just under a third)
        struct LegacyRedResult r = LegacyRed_Catch(255, 99, 300, 0, NextRand, &rs);
        CHECK(r.caught && !r.usedRand2, "red plateau just under 1/3 HP: caught %d usedRand2 %d w %d", r.caught, r.usedRand2, r.w);
        rs.at = 0;
        r = LegacyRed_Catch(255, 100, 300, 0, NextRand, &rs);
        CHECK(r.usedRand2, "red above the plateau needs Rand2 (w %d)", r.w);
        CHECK(RedOdds(45, 100, 300, 0) == RedOdds(45, 1, 300, 0), "red: below the plateau HP doesn't help");
    }

    // ---- Red: monotonic (less HP / a status never lowers the odds)
    {
        static const uint16_t maxes[] = {20, 120, 341, 342, 512, 714};
        static const uint8_t rates[] = {3, 45, 120, 190, 255};
        unsigned i, j;
        long n = 0;
        for (i = 0; i < sizeof(maxes) / sizeof(maxes[0]); i++)
            for (j = 0; j < sizeof(rates) / sizeof(rates[0]); j++)
            {
                uint16_t m = maxes[i], hp, step = quick ? m / 7 + 1 : m / 40 + 1;
                uint32_t prev = 0;
                for (hp = m; hp >= 1; hp = hp > step ? hp - step : (hp == 1 ? 0 : 1))
                {
                    uint32_t o = RedOdds(rates[j], hp, m, 0), os, of;
                    CHECK(o >= prev, "red odds fell as HP fell: cr %d max %d hp %d", rates[j], m, hp);
                    prev = o;
                    os = RedOdds(rates[j], hp, m, LEGACY_STATUS1_POISON);
                    of = RedOdds(rates[j], hp, m, LEGACY_STATUS1_SLEEP);
                    CHECK(os >= o && of >= os, "red status lowered odds: cr %d max %d hp %d", rates[j], m, hp);
                    n++;
                    if (hp == 1)
                        break;
                }
            }
        printf("red: %ld odds checks monotonic\n", n);
    }

    // ---- Crystal: the final value against the transliteration, every HP up to 714
    checked = 0;
    {
        int maxHp, hp, cr, si;
        for (maxHp = 1; maxHp <= 714; maxHp++)
            for (hp = 1; hp <= maxHp; hp += (quick && maxHp > 50) ? 7 : 1)
                for (si = 0; si < NSTAT; si++)
                {
                    uint8_t gb = GbStatus(sStatuses[si]);
                    int prevA = -1;
                    for (cr = 0; cr < 256; cr++)
                    {
                        uint8_t a = LegacyCrystal_FinalRate(cr, hp, maxHp, sStatuses[si]);
                        int fixed = CrystalAsm(cr, hp, maxHp, gb, 1, 1);
                        CHECK(a >= 1, "crystal A = 0");
                        CHECK(a >= prevA, "crystal A fell as the catch rate rose: cr %d hp %d/%d", cr, hp, maxHp);
                        prevA = a;
                        if (maxHp <= 680)
                            CHECK(a == fixed, "crystal fixed cr %d hp %d/%d st %x: %d vs %d", cr, hp, maxHp, sStatuses[si], a, fixed);
                        if (maxHp <= 341)
                        {
                            // the unfixed original only differs by the missing +5
                            int orig = CrystalAsm(cr, hp, maxHp, gb, 0, 0);
                            int minor = LegacyStatusClass(sStatuses[si]) == LEGACY_STATUS_MINOR;
                            int expect = minor ? (orig + 5 > 255 ? 255 : orig + 5) : orig;
                            if (minor && orig == 255)
                                expect = 255;
                            CHECK(a == expect, "crystal vs original cr %d hp %d/%d st %x: %d vs %d", cr, hp, maxHp, sStatuses[si], a, expect);
                        }
                        checked++;
                    }
                }
    }
    printf("crystal: %ld final values match the original (fixed)\n", checked);

    // Crystal: less HP never lowers A; a status never lowers it; 255 always catches
    {
        int maxHp, hp, cr;
        long n = 0;
        for (maxHp = 1; maxHp <= 714; maxHp += quick ? 37 : 1)
            for (cr = 0; cr < 256; cr += 17)
            {
                int prev = -1;
                for (hp = maxHp; hp >= 1; hp--)
                {
                    int a = LegacyCrystal_FinalRate(cr, hp, maxHp, 0);
                    int p = LegacyCrystal_FinalRate(cr, hp, maxHp, LEGACY_STATUS1_PARALYSIS);
                    int s = LegacyCrystal_FinalRate(cr, hp, maxHp, LEGACY_STATUS1_FREEZE);
                    CHECK(a >= prev, "crystal A fell as HP fell: cr %d hp %d/%d", cr, hp, maxHp);
                    CHECK(p >= a && s >= p, "crystal status lowered A: cr %d hp %d/%d", cr, hp, maxHp);
                    prev = a;
                    n++;
                }
            }
        printf("crystal: %ld HP / status checks monotonic\n", n);
    }
    {
        int rb, a;
        // success is exactly rand <= A: count successes over all 256 bytes for some A
        for (a = 0; a < 256; a += 5)
        {
            int succ = 0;
            uint8_t A = LegacyCrystal_FinalRate((uint8_t)a, 1, 1, 0);
            for (rb = 0; rb < 256; rb++)
            {
                uint8_t v[4] = {(uint8_t)rb, 255, 255, 255};
                struct Rands rs = {v, 4, 0};
                succ += LegacyCrystal_Catch((uint8_t)a, 1, 1, 0, NextRand, &rs).caught;
            }
            CHECK(succ == A + 1, "crystal: A %d caught %d of 256", A, succ);
        }
        {
            uint8_t v[4] = {255, 0, 0, 0};
            struct Rands rs = {v, 4, 0};
            CHECK(LegacyCrystal_FinalRate(255, 1, 714, LEGACY_STATUS1_SLEEP) == 255, "crystal saturates");
            CHECK(LegacyCrystal_Catch(255, 1, 714, LEGACY_STATUS1_SLEEP, NextRand, &rs).caught, "crystal 255 always catches");
        }
    }
    // Crystal wobbles: the table matches; a failed ball's wobbles follow the bytes, at most 3
    {
        int a;
        for (a = 1; a <= 255; a++)
            CHECK(LegacyCrystal_WobbleChance(a) == CrystalWobbleAsm(a), "wobble chance %d", a);
        {
            // A = 1 (rate 63): bytes 62 wobble, 63 breaks out
            uint8_t v[5] = {200, 62, 62, 63, 0};
            struct Rands rs = {v, 5, 0};
            struct LegacyCrystalResult r = LegacyCrystal_Catch(1, 1, 1, 0, NextRand, &rs);
            CHECK(!r.caught && r.wobbles == 2, "crystal wobbles: caught %d wobbles %d", r.caught, r.wobbles);
            v[3] = 0; rs.at = 0;
            r = LegacyCrystal_Catch(1, 1, 1, 0, NextRand, &rs);
            CHECK(!r.caught && r.wobbles == 3, "crystal 3 wobbles then breaks out: %d", r.wobbles);
        }
    }

    if (sFails)
    {
        printf("%d failures\n", sFails);
        return 1;
    }
    printf("all legacy catch checks passed\n");
    return 0;
}
