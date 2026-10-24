// Moemon Co-op: MOéMON GARDEN, the journal (docs/POKEMON_GARDEN.md 2.2).
//
// Plain sentences, written as the Trainer's observations, never numbers. A line shows once the Trainer
// knows it (MpLifeRec.known: they were there, they watched, or they simply know the Pokémon well enough);
// everything not known yet is one "??? (not known yet)" line at the end.
#include <string.h>
#include "global.h"
#include "decoration.h"
#include "constants/decorations.h"
#include "mp.h"
#include "mp_life.h"
#include "item.h"
#include "pokemon.h"
#include "random.h"
#include "region_map.h"
#include "string_util.h"
#include "data.h"
#include "constants/berry.h"
#include "constants/characters.h"
#include "constants/region_map_sections.h"
#include "constants/species.h"
#include "constants/weather.h"

// ---------------------------------------------------------------------------
// Words

static const u8 sTemperHigh[LIFE_AXES][40] = {
    [LIFE_BOLD]    = _("Bold. Steps up to anything."),
    [LIFE_WARM]    = _("Warm and affectionate."),
    [LIFE_LIVELY]  = _("Lively. Hardly ever sits still."),
    [LIFE_CURIOUS] = _("Curious about everything new."),
    [LIFE_SOCIAL]  = _("Loves company."),
    [LIFE_PROUD]   = _("Proud. Hates to look silly."),
};
static const u8 sTemperLow[LIFE_AXES][40] = {
    [LIFE_BOLD]    = _("Timid. Startles easily."),
    [LIFE_WARM]    = _("Aloof. Keeps to itself."),
    [LIFE_LIVELY]  = _("Calm and unhurried."),
    [LIFE_CURIOUS] = _("Likes the familiar best."),
    [LIFE_SOCIAL]  = _("Enjoys time alone."),
    [LIFE_PROUD]   = _("Modest and easygoing."),
};

static const u8 *const sFlavors[FLAVOR_COUNT] = {
    [FLAVOR_SPICY] = (const u8[])_("spicy"), [FLAVOR_DRY] = (const u8[])_("dry"), [FLAVOR_SWEET] = (const u8[])_("sweet"),
    [FLAVOR_BITTER] = (const u8[])_("bitter"), [FLAVOR_SOUR] = (const u8[])_("sour"),
};

static const u8 sTypeNeedText[TNEED_COUNT][40] = {
    [TNEED_COZY]    = _("Loves a soft, cozy spot."),
    [TNEED_SPAR]    = _("Practices its moves when it can."),
    [TNEED_SKY]     = _("Happiest under an open sky."),
    [TNEED_DAMP]    = _("Likes damp, murky places."),
    [TNEED_DIG]     = _("Loves to dig."),
    [TNEED_STONE]   = _("Likes sitting on warm rocks."),
    [TNEED_FOLIAGE] = _("Loves rustling through leaves."),
    [TNEED_SHADE]   = _("Seeks out shade and dusk."),
    [TNEED_SHINE]   = _("Loves shiny things."),
    [TNEED_BASK]    = _("Loves to bask in warmth."),
    [TNEED_SOAK]    = _("Loves soaking in water."),
    [TNEED_SUN]     = _("Turns its face to the sun."),
    [TNEED_STORM]   = _("Perks up in a storm."),
    [TNEED_STILL]   = _("Likes a still, quiet moment."),
    [TNEED_COOL]    = _("Likes cool places."),
    [TNEED_HEIGHT]  = _("Loves high places."),
    [TNEED_NIGHT]   = _("Comes alive at night."),
};

static const u8 sVerbIng[LIFE_VERBS][16] = {
    [LIFE_VERB_TALK] = _("talked to"), [LIFE_VERB_PET] = _("petted"), [LIFE_VERB_FEED] = _("fed"),
    [LIFE_VERB_PLAY] = _("played with"), [LIFE_VERB_PRAISE] = _("praised"), [LIFE_VERB_COMFORT] = _("fussed over"),
    [LIFE_VERB_WATCH] = _("watched"), [LIFE_VERB_GROOM] = _("groomed"), [LIFE_VERB_KEEPSAKE] = _("given things"),
    [LIFE_VERB_PICKUP] = _("picked up"),
};

static const u8 sTrigText[LIFE_TRIG_COUNT][20] = {
    [LIFE_TRIG_THUNDER] = _("thunderstorms"), [LIFE_TRIG_DARK] = _("the dark"), [LIFE_TRIG_WATER] = _("deep water"),
    [LIFE_TRIG_HEIGHTS] = _("high places"), [LIFE_TRIG_GHOSTS] = _("ghosts"), [LIFE_TRIG_CROWDS] = _("crowds"),
    [LIFE_TRIG_LEGEND] = _("huge MOéMON"), [LIFE_TRIG_SAND] = _("sandstorms"),
};

static const u8 sBadgeNames[8][16] = {
    _("STONE BADGE"), _("KNUCKLE BADGE"), _("DYNAMO BADGE"), _("HEAT BADGE"),
    _("BALANCE BADGE"), _("FEATHER BADGE"), _("MIND BADGE"), _("RAIN BADGE"),
};

static const u8 sStoryText[LIFE_STORY_COUNT][44] = {
    [LIFE_STORY_RIVAL]        = _("Battled {STR_VAR_1}'s rival together."),
    [LIFE_STORY_WALLY]        = _("Saw how strong WALLY has become."),
    [LIFE_STORY_MAGMA]        = _("Stood up to TEAM MAGMA."),
    [LIFE_STORY_AQUA]         = _("Stood up to TEAM AQUA."),
    [LIFE_STORY_MT_CHIMNEY]   = _("Was there at MT. CHIMNEY."),
    [LIFE_STORY_WEATHER_INST] = _("Helped at the WEATHER INSTITUTE."),
    [LIFE_STORY_SPACE_CENTER] = _("Defended the SPACE CENTER."),
    [LIFE_STORY_SEAFLOOR]     = _("Went down to the SEAFLOOR CAVERN."),
    [LIFE_STORY_GROUDON]      = _("Saw GROUDON awaken."),
    [LIFE_STORY_KYOGRE]       = _("Saw KYOGRE awaken."),
    [LIFE_STORY_RAYQUAZA]     = _("Saw RAYQUAZA descend."),
    [LIFE_STORY_SOOTOPOLIS]   = _("Saw the skies over SOOTOPOLIS clear."),
    [LIFE_STORY_ELITE_FOUR]   = _("Faced the ELITE FOUR."),
    [LIFE_STORY_CHAMPION]     = _("Became CHAMPION with {STR_VAR_1}!"),
};

static const u8 sTxt_MetCaught[] = _("Met {STR_VAR_1} in {STR_VAR_2}.");
static const u8 sTxt_MetHatched[] = _("Hatched in {STR_VAR_2}.");
static const u8 sTxt_MetReceived[] = _("Came to {STR_VAR_1} in {STR_VAR_2}.");
static const u8 sTxt_MetTraded[] = _("Came to {STR_VAR_1} in a trade.");
static const u8 sTxt_Evolved[] = _("Evolved in {STR_VAR_2}.");
static const u8 sTxt_Badge[] = _("Won the {STR_VAR_3} together.");
static const u8 sTxt_Legend[] = _("Saw {STR_VAR_3} with its own eyes.");
static const u8 sTxt_NewPlace[] = _("First saw {STR_VAR_2} with {STR_VAR_1}.");
static const u8 sTxt_WeatherSnow[] = _("Saw snow fall for the first time.");
static const u8 sTxt_WeatherAsh[] = _("Saw ash fall like snow in {STR_VAR_2}.");
static const u8 sTxt_WeatherSand[] = _("Braved a sandstorm in {STR_VAR_2}.");
static const u8 sTxt_WeatherStorm[] = _("Saw a fierce storm in {STR_VAR_2}.");
static const u8 sTxt_WeatherOther[] = _("Saw strange weather in {STR_VAR_2}.");
static const u8 sTxt_Fright[] = _("Got scared by {STR_VAR_3} in {STR_VAR_2}.");
static const u8 sTxt_Overcame[] = _("Isn't afraid of {STR_VAR_3} anymore.");
static const u8 sTxt_NearFaint[] = _("Hung on at the last moment in {STR_VAR_2}.");
static const u8 sTxt_Fainted[] = _("Fainted in a hard battle in {STR_VAR_2}.");
static const u8 sTxt_Clutch[] = _("Won a battle with the final blow.");
static const u8 sTxt_Comeback[] = _("Turned a battle around in {STR_VAR_2}.");
static const u8 sTxt_Partner[] = _("Fought side by side with {STR_VAR_3}.");
static const u8 sTxt_Rival[] = _("Has a rival: {STR_VAR_3}.");
static const u8 sTxt_Comforted[] = _("{STR_VAR_3} comforted it when it was upset.");
static const u8 sTxt_Fed[] = _("{STR_VAR_3} fed it when it was hungry.");
static const u8 sTxt_Played[] = _("Played a game with {STR_VAR_3}.");
static const u8 sTxt_Praised[] = _("{STR_VAR_3} praised it. It glowed.");
static const u8 sTxt_Reunion[] = _("Was so happy to see {STR_VAR_1} again.");
static const u8 sTxt_KeepsakeMem[] = _("Got a {STR_VAR_3} from {STR_VAR_1}.");
static const u8 sTxt_Heard[] = _("Heard all about it from {STR_VAR_3}.");
static const u8 sTxt_Rare[] = _("Saw something rare in {STR_VAR_2}.");
static const u8 sTxt_Pursuit[] = _("Got better at what it loves.");
static const u8 sTxt_FriendMem[] = _("Made friends with {STR_VAR_3}.");
static const u8 sTxt_Family[] = _("Met its family.");
static const u8 sTxt_FamilyChild[] = _("Met its little one, {STR_VAR_3}.");
static const u8 sTxt_FamilyParent[] = _("Met its parent, {STR_VAR_3}.");
static const u8 sTxt_FamilySibling[] = _("Met its sibling, {STR_VAR_3}.");

// Rare moments (MEM_RARE, docs 12): [kind][0 = it led, 1 = the other one did]
static const u8 sRareText[LIFE_RARE_COUNT][2][44] = {
    [LIFE_RARE_STARGAZE]  = {_("Stargazed with {STR_VAR_3} on a clear night."), _("Stargazed with {STR_VAR_3} on a clear night.")},
    [LIFE_RARE_PADDLE]    = {_("Taught {STR_VAR_3} to paddle in the pond."), _("{STR_VAR_3} taught it to paddle. Brave!")},
    [LIFE_RARE_RIVAL_NAP] = {_("Shared a nap with its old rival {STR_VAR_3}."), _("Shared a nap with its old rival {STR_VAR_3}.")},
    [LIFE_RARE_FAMILY]    = {_("Groomed {STR_VAR_3} one quiet evening."), _("{STR_VAR_3} groomed it one quiet evening.")},
};

// Pursuits (docs 12): what it took up, and its milestones
static const u8 sPursuitText[LIFE_PURSUIT_COUNT][40] = {
    [LIFE_PURSUIT_CLIMB]   = _("Loves climbing the GARDEN's trees."),
    [LIFE_PURSUIT_SWIM]    = _("Loves splashing in the GARDEN pond."),
    [LIFE_PURSUIT_SING]    = _("Loves to sing."),
    [LIFE_PURSUIT_COLLECT] = _("Collects pretty stones."),
    [LIFE_PURSUIT_DIG]     = _("Digs tunnels in the sand."),
    [LIFE_PURSUIT_TEACH]   = _("Likes showing the younger ones."),
};
static const u8 sPursuitMilestone[LIFE_PURSUIT_COUNT][LIFE_PURSUIT_MILESTONES][40] = {
    [LIFE_PURSUIT_CLIMB]   = {_("Climbed its first tree!"), _("Reached the top branch!"), _("Climbs the tallest tree with ease.")},
    [LIFE_PURSUIT_SWIM]    = {_("Got its feet wet in the pond!"), _("Splashes about like a natural!"), _("Its splashes reach across the pond.")},
    [LIFE_PURSUIT_SING]    = {_("Hummed its first real tune."), _("Sang a whole song without stopping!"), _("Its song fills the whole GARDEN.")},
    [LIFE_PURSUIT_COLLECT] = {_("Found its first pretty stone!"), _("Has a little pile of pretty stones."), _("Found the prettiest stone of all!")},
    [LIFE_PURSUIT_DIG]     = {_("Dug its first tunnel in the sand!"), _("Dug a tunnel big enough to hide in!"), _("Its tunnels run all under the sand.")},
    [LIFE_PURSUIT_TEACH]   = {_("Showed a younger one how it's done."), _("The younger ones follow its lead."), _("A patient teacher to everyone.")},
};
static const u8 sTxt_FamilySong[] = _("Hums a song its family sings.");
static const u8 sTxt_FamilyGreeting[] = _("Greets you the way its family does.");
static const u8 sTxt_GardenMem[] = _("Saw the GARDEN for the first time.");
static const u8 sTxt_FavDecor[] = _("Loves the {STR_VAR_3} in the GARDEN.");

static const u8 sTxt_Likes[] = _("Loves {STR_VAR_3} food.");
static const u8 sTxt_Dislikes[] = _("Can't stand {STR_VAR_3} food.");
static const u8 sTxt_Boundary[] = _("Doesn't like being {STR_VAR_3}.");
static const u8 sTxt_Afraid[] = _("Uneasy around {STR_VAR_3}.");
static const u8 sTxt_Brave[] = _("Not afraid of {STR_VAR_3}.");
static const u8 sTxt_LovesPlace[] = _("Loves {STR_VAR_2}.");
static const u8 sTxt_Protective[] = _("Protective of {STR_VAR_3}.");
static const u8 sTxt_Confident[] = _("Confident in battle.");
static const u8 sTxt_ComfortFood[] = _("{STR_VAR_3} food cheers it up.");
static const u8 sTxt_DislikesLearned[] = _("Doesn't like being {STR_VAR_3} anymore.");
static const u8 sTxt_Friend[] = _("Best friends with {STR_VAR_3}.");
static const u8 sTxt_RivalBond[] = _("Always squares up to {STR_VAR_3}.");
static const u8 sTxt_FamilyBond[] = _("Family: {STR_VAR_3}.");
static const u8 sTxt_Favorite[] = _("Runs to greet {STR_VAR_3}.");
static const u8 sTxt_Keepsake[] = _("Treasures its {STR_VAR_3}.");
static const u8 sTxt_FavPlace[] = _("Feels at home in {STR_VAR_2}.");
static const u8 sTxt_Unknown[] = _("??? (not known yet)");

// ---------------------------------------------------------------------------
// Building a line

// {STR_VAR_1} = its Trainer, {STR_VAR_2} = a place, {STR_VAR_3} = anything else
static void Expand(u8 *out, const u8 *tmpl, const u8 *a, const u8 *b, const u8 *c)
{
    u8 *o = out, *end = out + LIFE_JOURNAL_TEXT - 1;
    while (*tmpl != EOS && o < end)
    {
        if (*tmpl == PLACEHOLDER_BEGIN && (tmpl[1] == 2 || tmpl[1] == 3 || tmpl[1] == 4))
        {
            const u8 *src = tmpl[1] == 2 ? a : tmpl[1] == 3 ? b : c;
            while (src && *src != EOS && o < end)
                *o++ = *src++;
            tmpl += 2;
            continue;
        }
        *o++ = *tmpl++;
    }
    *o = EOS;
}

static void PlaceName(u8 *out, u16 mapsec)
{
    if (mapsec >= MAPSEC_NONE)
        StringCopy(out, (const u8[])_("a faraway place"));
    else
        GetMapName(out, mapsec, 0);
}

static void WhoName(u8 *out, u32 key)
{
    if (!MP_Life_NameOf(key, out))
        StringCopy(out, (const u8[])_("a friend"));
}

struct Builder
{
    u8 (*lines)[LIFE_JOURNAL_TEXT];
    u8 max, count, unknown;
    struct MpLifeRec *rec;
    u8 trainer[PLAYER_NAME_LENGTH + 1];
};

static void Add(struct Builder *bd, u8 fact, const u8 *tmpl, u16 mapsec, const u8 *c)
{
    u8 place[32];
    if (!(bd->rec->known[fact / 32] & (1u << (fact % 32))))
    {
        bd->unknown++;
        return;
    }
    if (bd->count >= bd->max - 1)   // keep room for the "???" line
        return;
    PlaceName(place, mapsec);
    Expand(bd->lines[bd->count++], tmpl, bd->trainer, place, c);
}

// What the Trainer comes to know simply by knowing it well (familiarity with its own Trainer)
static void RevealByFamiliarity(struct MpLifeRec *rec, u8 familiar)
{
    u8 i;
    for (i = 0; i < LIFE_AXES; i++)
    {
        s16 dist = rec->cur[i] > 128 ? rec->cur[i] - 128 : 128 - rec->cur[i];
        if (familiar + dist >= 150)
            MP_Life_Reveal(rec, LIFE_FACT_TEMPER(i));
    }
    if (familiar >= 120)
        MP_Life_Reveal(rec, LIFE_FACT_TYPE_NEED);
    if (familiar >= 150)
    {
        MP_Life_Reveal(rec, LIFE_FACT_LIKES);
        MP_Life_Reveal(rec, LIFE_FACT_DISLIKES);
        MP_Life_Reveal(rec, LIFE_FACT_FRIEND);
        MP_Life_Reveal(rec, LIFE_FACT_RIVAL);
        MP_Life_Reveal(rec, LIFE_FACT_FAMILY);
        MP_Life_Reveal(rec, LIFE_FACT_PLACE);
    }
    if (familiar >= 200)
        for (i = 0; i < LIFE_TRAITS; i++)
            MP_Life_Reveal(rec, LIFE_FACT_TRAIT(i));
}

static void AddMemory(struct Builder *bd, u8 slot)
{
    struct MpLifeMemory *m = &bd->rec->mem[slot];
    u8 who[32], extra[32];
    const u8 *tmpl = NULL;
    u8 fact = LIFE_FACT_MEMORY(slot);

    WhoName(who, m->who);
    extra[0] = EOS;
    switch (m->kind)
    {
    case MEM_MET:
        tmpl = m->param == 1 ? sTxt_MetHatched : m->param == 2 ? sTxt_MetReceived : m->param == 3 ? sTxt_MetTraded : sTxt_MetCaught;
        break;
    case MEM_EVOLVED: tmpl = sTxt_Evolved; break;
    case MEM_BADGE:
        if (m->param < 8)
            StringCopy(extra, sBadgeNames[m->param]);
        tmpl = sTxt_Badge;
        break;
    case MEM_STORY:
        if (m->param < LIFE_STORY_COUNT && sStoryText[m->param][0] != EOS && sStoryText[m->param][0] != 0)
            tmpl = sStoryText[m->param];
        break;
    case MEM_LEGEND:
        if (m->param < NUM_SPECIES)
            StringCopy(extra, gSpeciesNames[m->param]);
        tmpl = sTxt_Legend;
        break;
    case MEM_NEW_PLACE: tmpl = sTxt_NewPlace; break;
    case MEM_WEATHER:
        tmpl = m->param == WEATHER_SNOW ? sTxt_WeatherSnow : m->param == WEATHER_VOLCANIC_ASH ? sTxt_WeatherAsh
             : m->param == WEATHER_SANDSTORM ? sTxt_WeatherSand
             : (m->param == WEATHER_RAIN_THUNDERSTORM || m->param == WEATHER_DOWNPOUR) ? sTxt_WeatherStorm : sTxt_WeatherOther;
        break;
    case MEM_FRIGHT:
    case MEM_OVERCAME:
        StringCopy(extra, m->param < LIFE_TRIG_COUNT && m->param ? sTrigText[m->param] : (const u8[])_("something"));
        tmpl = m->kind == MEM_FRIGHT ? sTxt_Fright : sTxt_Overcame;
        break;
    case MEM_NEAR_FAINT: tmpl = sTxt_NearFaint; break;
    case MEM_FAINTED: tmpl = sTxt_Fainted; break;
    case MEM_CLUTCH: tmpl = sTxt_Clutch; break;
    case MEM_COMEBACK: tmpl = sTxt_Comeback; break;
    case MEM_PARTNER: StringCopy(extra, who); tmpl = sTxt_Partner; break;
    case MEM_RIVAL: StringCopy(extra, who); tmpl = sTxt_Rival; break;
    case MEM_COMFORTED: StringCopy(extra, who); tmpl = sTxt_Comforted; break;
    case MEM_FED: StringCopy(extra, who); tmpl = sTxt_Fed; break;
    case MEM_PLAYED: StringCopy(extra, who); tmpl = sTxt_Played; break;
    case MEM_PRAISED: StringCopy(extra, who); tmpl = sTxt_Praised; break;
    case MEM_REUNION: tmpl = sTxt_Reunion; break;
    case MEM_KEEPSAKE: CopyItemName(m->param, extra); tmpl = sTxt_KeepsakeMem; break;
    case MEM_HEARD: StringCopy(extra, who); tmpl = sTxt_Heard; break;
    case MEM_RARE:
        StringCopy(extra, who);
        tmpl = (m->param & 0xFF) && (m->param & 0xFF) < LIFE_RARE_COUNT ? sRareText[m->param & 0xFF][(m->param >> 8) ? 1 : 0] : sTxt_Rare;
        break;
    case MEM_PURSUIT:
        tmpl = (m->param >> 8) && (m->param >> 8) < LIFE_PURSUIT_COUNT && (m->param & 0xFF) < LIFE_PURSUIT_MILESTONES
             ? sPursuitMilestone[m->param >> 8][m->param & 0xFF] : sTxt_Pursuit;
        break;
    case MEM_FRIEND: StringCopy(extra, who); tmpl = sTxt_FriendMem; break;
    case MEM_FAMILY:
        StringCopy(extra, who);
        tmpl = m->who == 0 ? sTxt_Family : m->param == 0 ? sTxt_FamilyChild : m->param == 1 ? sTxt_FamilyParent : sTxt_FamilySibling;
        break;
    case MEM_GARDEN: tmpl = sTxt_GardenMem; break;
    }
    if (tmpl)
        Add(bd, fact, tmpl, m->place, extra);
}

static u8 sKnownCount;

// How many lines the last journal had that the Trainer knows (not counting the "???" line)
u8 MP_Life_JournalKnownCount(void)
{
    return sKnownCount;
}

u8 MP_Life_Journal(struct BoxPokemon *box, u8 lines[][LIFE_JOURNAL_TEXT], u8 max)
{
    struct Builder bd;
    struct MpLifeRec *rec = MP_Life_Get(box, FALSE);
    struct MpLifeBond *owner;
    u32 me;
    u8 i, extra[32];

    sKnownCount = 0;
    if (rec == NULL || max < 2)
        return 0;
    me = MP_Life_MonKey(rec->personality, rec->otId);
    bd.lines = lines;
    bd.max = max;
    bd.count = 0;
    bd.unknown = 0;
    bd.rec = rec;
    StringCopy(bd.trainer, gSaveBlock2Ptr->playerName);
    owner = MP_Life_Bond(me, MP_Life_LocalTrainerKey(), FALSE);
    RevealByFamiliarity(rec, owner ? owner->v[BOND_FAMILIAR] : 0);

    // Who it is: its most distinct sides (up to two)
    {
        u8 first = 0xFF, second = 0xFF;
        s16 d1 = 0, d2 = 0;
        for (i = 0; i < LIFE_AXES; i++)
        {
            s16 d = rec->cur[i] > 128 ? rec->cur[i] - 128 : 128 - rec->cur[i];
            if (d > d1)
                d2 = d1, second = first, d1 = d, first = i;
            else if (d > d2)
                d2 = d, second = i;
        }
        if (first != 0xFF && d1 >= 30)
            Add(&bd, LIFE_FACT_TEMPER(first), rec->cur[first] > 128 ? sTemperHigh[first] : sTemperLow[first], MAPSEC_NONE, NULL);
        if (second != 0xFF && d2 >= 30)
            Add(&bd, LIFE_FACT_TEMPER(second), rec->cur[second] > 128 ? sTemperHigh[second] : sTemperLow[second], MAPSEC_NONE, NULL);
    }
    // What it loves doing (its leading type need)
    {
        u8 a = MP_Life_TypeNeed(rec, FALSE), b = MP_Life_TypeNeed(rec, TRUE);
        u8 lead = (a != b && rec->typeLean < 128) ? b : a;
        if (lead != TNEED_NONE)
            Add(&bd, LIFE_FACT_TYPE_NEED, sTypeNeedText[lead], MAPSEC_NONE, NULL);
    }
    // Tastes
    {
        u8 hi = 0, lo = 0;
        for (i = 1; i < FLAVOR_COUNT; i++)
        {
            if (rec->taste[i] > rec->taste[hi]) hi = i;
            if (rec->taste[i] < rec->taste[lo]) lo = i;
        }
        if (rec->taste[hi] >= 50)
            Add(&bd, LIFE_FACT_LIKES, sTxt_Likes, MAPSEC_NONE, sFlavors[hi]);
        if (rec->taste[lo] <= -50)
            Add(&bd, LIFE_FACT_DISLIKES, sTxt_Dislikes, MAPSEC_NONE, sFlavors[lo]);
    }
    // A pursuit it took up (the GARDEN, docs 12)
    if (rec->pursuit && rec->pursuit < LIFE_PURSUIT_COUNT)
        Add(&bd, LIFE_FACT_PURSUIT, sPursuitText[rec->pursuit], MAPSEC_NONE, NULL);
    // A boundary it showed
    for (i = 0; i < LIFE_VERBS; i++)
        if (rec->boundaries & (1 << i))
        {
            Add(&bd, LIFE_FACT_BOUNDARY, sTxt_Boundary, MAPSEC_NONE, sVerbIng[i]);
            break;
        }
    // Learned traits
    for (i = 0; i < LIFE_TRAITS; i++)
    {
        struct MpLifeTrait *t = &rec->trait[i];
        if (t->strength < 32)
            continue;
        switch (t->id)
        {
        case TRAIT_AFRAID: Add(&bd, LIFE_FACT_TRAIT(i), sTxt_Afraid, MAPSEC_NONE, sTrigText[t->param < LIFE_TRIG_COUNT ? t->param : 0]); break;
        case TRAIT_BRAVE: Add(&bd, LIFE_FACT_TRAIT(i), sTxt_Brave, MAPSEC_NONE, sTrigText[t->param < LIFE_TRIG_COUNT ? t->param : 0]); break;
        case TRAIT_LOVES_PLACE: Add(&bd, LIFE_FACT_TRAIT(i), sTxt_LovesPlace, t->param, NULL); break;
        case TRAIT_PROTECTIVE:
        {
            // param: the low 16 bits of the key it protects; find who that is among its bonds
            u8 name[32];
            StringCopy(name, (const u8[])_("a friend"));
            {
                extern struct MpLifeBond *MP_Life_BondByLow(u32 from, u16 low);
                struct MpLifeBond *b = MP_Life_BondByLow(me, t->param);
                if (b)
                    WhoName(name, b->to);
            }
            Add(&bd, LIFE_FACT_TRAIT(i), sTxt_Protective, MAPSEC_NONE, name);
            break;
        }
        case TRAIT_CONFIDENT: Add(&bd, LIFE_FACT_TRAIT(i), sTxt_Confident, MAPSEC_NONE, NULL); break;
        case TRAIT_COMFORT_FOOD:
            StringCopy(extra, sFlavors[t->param < FLAVOR_COUNT ? t->param : 0]);
            if (extra[0] >= CHAR_a && extra[0] <= CHAR_z)
                extra[0] -= CHAR_a - CHAR_A;   // starts the sentence
            Add(&bd, LIFE_FACT_TRAIT(i), sTxt_ComfortFood, MAPSEC_NONE, extra);
            break;
        case TRAIT_DISLIKES: Add(&bd, LIFE_FACT_TRAIT(i), sTxt_DislikesLearned, MAPSEC_NONE, sVerbIng[t->param < LIFE_VERBS ? t->param : 0]); break;
        case TRAIT_FAMILY_SONG: Add(&bd, LIFE_FACT_TRAIT(i), sTxt_FamilySong, MAPSEC_NONE, NULL); break;
        case TRAIT_FAMILY_GREETING: Add(&bd, LIFE_FACT_TRAIT(i), sTxt_FamilyGreeting, MAPSEC_NONE, NULL); break;
        }
    }
    // Its closest friend, a rival, family, and a favorite Trainer besides its own
    {
        extern void MP_Life_Closest(u32 from, u32 *friendKey, u32 *rivalKey, u32 *familyKey, u32 *favoriteKey);
        u32 fr = 0, rv = 0, fa = 0, fv = 0;
        u8 name[32];
        MP_Life_Closest(me, &fr, &rv, &fa, &fv);
        if (fr) { WhoName(name, fr); Add(&bd, LIFE_FACT_FRIEND, sTxt_Friend, MAPSEC_NONE, name); }
        if (rv) { WhoName(name, rv); Add(&bd, LIFE_FACT_RIVAL, sTxt_RivalBond, MAPSEC_NONE, name); }
        if (fa) { WhoName(name, fa); Add(&bd, LIFE_FACT_FAMILY, sTxt_FamilyBond, MAPSEC_NONE, name); }
        if (fv) { WhoName(name, fv); Add(&bd, LIFE_FACT_FAVORITE, sTxt_Favorite, MAPSEC_NONE, name); }
    }
    // Its keepsake and a favorite place
    if (rec->keepsake)
    {
        CopyItemName(rec->keepsake, extra);
        Add(&bd, LIFE_FACT_KEEPSAKE, sTxt_Keepsake, MAPSEC_NONE, extra);
    }
    // a favorite decoration in the MOéMON GARDEN (mp_garden_decor.c)
    if ((rec->favDecor >> 8) >= 5 && (rec->favDecor & 0xFF) < NUM_DECORATIONS)
    {
        StringCopy(extra, gDecorations[rec->favDecor & 0xFF].name);
        Add(&bd, LIFE_FACT_DECOR, sTxt_FavDecor, MAPSEC_NONE, extra);
    }
    {
        u8 best = 0xFF, lvl = 100;
        for (i = 0; i < LIFE_PLACES; i++)
            if (rec->place[i].level > lvl || (rec->place[i].flags & LIFE_PLACE_FAVORITE))
            {
                lvl = rec->place[i].level;
                best = i;
                if (rec->place[i].flags & LIFE_PLACE_FAVORITE)
                    break;
            }
        if (best != 0xFF)
            Add(&bd, LIFE_FACT_PLACE, sTxt_FavPlace, rec->place[best].map, NULL);
    }
    // Memories: the strongest, newest first
    {
        u8 order[LIFE_MEMORIES], n = 0, j;
        for (i = 0; i < LIFE_MEMORIES; i++)
            if (rec->mem[i].kind != MEM_NONE)
                order[n++] = i;
        for (i = 1; i < n; i++)
            for (j = i; j > 0; j--)
            {
                struct MpLifeMemory *a = &rec->mem[order[j - 1]], *b = &rec->mem[order[j]];
                if (b->strength > a->strength || (b->strength == a->strength && b->when > a->when))
                {
                    u8 t = order[j - 1];
                    order[j - 1] = order[j];
                    order[j] = t;
                }
                else
                {
                    break;
                }
            }
        for (i = 0; i < n; i++)
            AddMemory(&bd, order[i]);
    }
    sKnownCount = bd.count;
    if (bd.unknown && bd.count < max)
        StringCopy(lines[bd.count++], sTxt_Unknown);
    return bd.count;
}

// ---------------------------------------------------------------------------
// Watching (docs 2.2: the journal fills in as the Trainer notices things)

#define WATCH_MARK_MINUTES 1    // watchMark counts play minutes (1..255, wrapping; 0 = never)
#define WATCH_WAIT_STEPS   30   // half an hour between things learned just by watching one Pokémon

static u8 WatchMarkNow(void)
{
    return (MP_Life_PlayMinutes() / WATCH_MARK_MINUTES) % 255 + 1;
}

bool8 MP_Life_Observe(struct BoxPokemon *box)
{
    static u8 lines[LIFE_JOURNAL_LINES][LIFE_JOURNAL_TEXT];
    static const u8 sSeen[] = {
        LIFE_FACT_TEMPER(0), LIFE_FACT_TEMPER(1), LIFE_FACT_TEMPER(2), LIFE_FACT_TEMPER(3), LIFE_FACT_TEMPER(4),
        LIFE_FACT_TEMPER(5), LIFE_FACT_TYPE_NEED, LIFE_FACT_PURSUIT,
    };
    struct MpLifeRec *rec = MP_Life_Get(box, FALSE);
    u8 base, start, k, now;
    if (rec == NULL)
        return FALSE;
    now = WatchMarkNow();
    if (rec->watchMark != 0 && (now + 255 - rec->watchMark) % 255 < WATCH_WAIT_STEPS)
        return FALSE;   // watched it not long ago: nothing new yet
    MP_Life_Journal(box, lines, LIFE_JOURNAL_LINES);
    base = MP_Life_JournalKnownCount();
    start = Random() % ARRAY_COUNT(sSeen);
    for (k = 0; k < ARRAY_COUNT(sSeen); k++)
    {
        u8 fact = sSeen[(start + k) % ARRAY_COUNT(sSeen)];
        if (rec->known[fact / 32] & (1u << (fact % 32)))
            continue;
        MP_Life_Reveal(rec, fact);
        MP_Life_Journal(box, lines, LIFE_JOURNAL_LINES);
        if (MP_Life_JournalKnownCount() > base)
        {
            rec->watchMark = now;
            return TRUE;
        }
        rec->known[fact / 32] &= ~(1u << (fact % 32));   // (no line for it: nothing to learn there)
    }
    return FALSE;
}
