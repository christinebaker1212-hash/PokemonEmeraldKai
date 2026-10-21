// Moemon Co-op: the GENESIS BALL (Pokémon Red's POKé BALL) and the EPOCH BALL
// (Pokémon Crystal's POKé BALL).
//
// Each runs its own game's catch routine (mp_legacy_catch.inc.c, checked against
// the originals by tools/test/legacy_catch_test.sh) in place of Emerald's, from
// Cmd_handleballthrow after its target and eligibility checks. Only the game
// running the battle resolves a throw; the result reaches every game in the
// usual BALLTHROWANIM command (caseId: BALL_NO_SHAKES..BALL_3_SHAKES_SUCCESS, or
// BALL_MP_MISS for a GENESIS BALL that missed), so all of them show the same.
// The ball's messages use its own game's wording (battle_message.c asks
// MP_LegacyBall_String), and its throw, wobbles and sounds are its own game's
// (mp_legacy_anim.c).
#include "global.h"
#include "mp.h"
#include "battle.h"
#include "battle_controllers.h"
#include "pokemon.h"
#include "random.h"
#include "constants/battle_string_ids.h"
#include "constants/items.h"

#include "mp_legacy_catch.inc.c"

// Tests: random bytes to use for the next throws (`mp legacyrand 12,200,...`), else Random()
static u8 sRigged[16], sRiggedCount, sRiggedAt;

void MP_LegacyBall_RigRandom(const u8 *bytes, u8 count)
{
    if (count > ARRAY_COUNT(sRigged))
        count = ARRAY_COUNT(sRigged);
    memcpy(sRigged, bytes, count);
    sRiggedCount = count;
    sRiggedAt = 0;
}

static uint8_t GameRand8(void *ctx)
{
    (void)ctx;
    if (sRiggedAt < sRiggedCount)
        return sRigged[sRiggedAt++];
    return Random() >> 8;
}

u8 MP_LegacyBall_Kind(u16 item)
{
    u8 special = MP_SpecialBall_FromItem(item);
    return (special == MP_SPECIAL_GENESIS || special == MP_SPECIAL_EPOCH) ? special : MP_SPECIAL_NONE;
}

// The encounter's own species: a Transformed target keeps its species' catch rate
static u8 TargetCatchRate(u8 target)
{
    u16 species = GetMonData(&gEnemyParty[gBattlerPartyIndexes[target]], MON_DATA_SPECIES);
    if (species == SPECIES_NONE || species >= NUM_SPECIES)
        species = gBattleMons[target].species;
    return gSpeciesInfo[species].catchRate;
}

// One throw at `target`: TRUE when caught. *caseId is what the animation shows.
bool8 MP_LegacyBall_ResolveThrow(u16 item, u8 target, u8 *caseId)
{
    struct BattlePokemon *mon = &gBattleMons[target];
    u8 kind = MP_LegacyBall_Kind(item), catchRate = TargetCatchRate(target);
    u16 hp = mon->hp, maxHp = mon->maxHP;

    if (kind == MP_SPECIAL_GENESIS)
    {
        struct LegacyRedResult r = LegacyRed_Catch(catchRate, hp, maxHp, mon->status1, GameRand8, NULL);
        if (r.caught)
            *caseId = BALL_3_SHAKES_SUCCESS;
        else
            *caseId = r.shakes == 0 ? BALL_MP_MISS : r.shakes;   // BALL_1_SHAKE .. BALL_3_SHAKES_FAIL
        MPNet_Log("legacy ball: GENESIS at b%d (rate %d, HP %d/%d, status %x): rand %d/%d, W %d -> %s %d",
                  target, catchRate, hp, maxHp, (unsigned)mon->status1, r.rand1, r.usedRand2 ? r.rand2 : -1, r.w,
                  r.caught ? "caught" : r.shakes == 0 ? "missed" : "shakes", r.shakes);
        return r.caught;
    }
    else
    {
        struct LegacyCrystalResult r = LegacyCrystal_Catch(catchRate, hp, maxHp, mon->status1, GameRand8, NULL);
        *caseId = r.caught ? BALL_3_SHAKES_SUCCESS : r.wobbles;   // BALL_NO_SHAKES .. BALL_3_SHAKES_FAIL
        MPNet_Log("legacy ball: EPOCH at b%d (rate %d, HP %d/%d, status %x): A %d, rand %d -> %s, %d wobbles",
                  target, catchRate, hp, maxHp, (unsigned)mon->status1, r.finalRate, r.rand,
                  r.caught ? "caught" : "broke out", r.wobbles);
        return r.caught;
    }
}

// ---------------------------------------------------------------------------
// Messages: each game's own wording (MOéMON for #MON). After a failed ball the
// battle prints one of STRINGID_PKMNBROKEFREE..SHOOTSOCLOSE by the shakes shown
// (a GENESIS BALL's miss is shake 0).

static const u8 sText_RedMissed[] = _("You missed the\nMOéMON!");
static const u8 sText_RedBrokeFree[] = _("Darn! The MOéMON\nbroke free!");
static const u8 sText_RedAppearedCaught[] = _("Aww! It appeared\nto be caught!");
static const u8 sText_RedSoClose[] = _("Shoot! It was so\nclose too!");
static const u8 sText_RedCaught[] = _("All right!\n{B_OPPONENT_MON1_NAME} was caught!\p");

static const u8 sText_CrystalBrokeFree[] = _("Oh no! The MOéMON\nbroke free!");
static const u8 sText_CrystalAppearedCaught[] = _("Aww! It appeared\nto be caught!");
static const u8 sText_CrystalAlmostHadIt[] = _("Aargh!\nAlmost had it!");
static const u8 sText_CrystalSoClose[] = _("Shoot! It was so\nclose too!");
static const u8 sText_CrystalCaught[] = _("Gotcha! {B_OPPONENT_MON1_NAME}\nwas caught!\p");

// Red's / Crystal's SFX_CAUGHT_MON, played with the caught message (mp_legacy_sound.c;
// the ids are mp_legacy_sfx.h's LEGACY_SFX_RED_CAUGHT_MON / LEGACY_SFX_GSC_CAUGHT_MON)
void MP_LegacyBall_PlayCaughtJingle(void)
{
    u8 kind = MP_LegacyBall_Kind(gLastUsedItem);
    if (kind)
        MP_LegacySfx_Play(kind == MP_SPECIAL_GENESIS ? 3 : 9, 0);
}

// The text for stringId when the last ball was a GENESIS / EPOCH BALL (NULL: the usual one)
const u8 *MP_LegacyBall_String(u16 stringId)
{
    switch (MP_LegacyBall_Kind(gLastUsedItem))
    {
    case MP_SPECIAL_GENESIS:
        switch (stringId)
        {
        case STRINGID_PKMNBROKEFREE:      return sText_RedMissed;
        case STRINGID_ITAPPEAREDCAUGHT:   return sText_RedBrokeFree;
        case STRINGID_AARGHALMOSTHADIT:   return sText_RedAppearedCaught;
        case STRINGID_SHOOTSOCLOSE:       return sText_RedSoClose;
        case STRINGID_GOTCHAPKMNCAUGHTPLAYER: return sText_RedCaught;
        }
        break;
    case MP_SPECIAL_EPOCH:
        switch (stringId)
        {
        case STRINGID_PKMNBROKEFREE:      return sText_CrystalBrokeFree;
        case STRINGID_ITAPPEAREDCAUGHT:   return sText_CrystalAppearedCaught;
        case STRINGID_AARGHALMOSTHADIT:   return sText_CrystalAlmostHadIt;
        case STRINGID_SHOOTSOCLOSE:       return sText_CrystalSoClose;
        case STRINGID_GOTCHAPKMNCAUGHTPLAYER: return sText_CrystalCaught;
        }
        break;
    }
    return NULL;
}
