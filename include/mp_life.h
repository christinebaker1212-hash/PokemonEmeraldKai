#ifndef GUARD_MP_LIFE_H
#define GUARD_MP_LIFE_H

struct BoxPokemon;

// Moemon Co-op: MOéMON GARDEN, the A-Life (docs/POKEMON_GARDEN.md, src/mp/mp_life*.c).
//
// Every Pokémon the player owns gets a record: its temperament, needs, feelings, memories, learned
// traits, tastes and the places it knows. Bonds (how one Pokémon feels about a Trainer or another
// Pokémon) are kept in a list of their own. All of it is written next to the save ("<save>.life")
// whenever the game saves, so it always matches the save it belongs to.

// Temperament axes (0..255, 128 = in the middle)
enum
{
    LIFE_BOLD,      // timid .. bold
    LIFE_WARM,      // aloof .. warm
    LIFE_LIVELY,    // calm .. lively
    LIFE_CURIOUS,   // settled .. curious
    LIFE_SOCIAL,    // solitary .. social
    LIFE_PROUD,     // humble .. proud
    LIFE_AXES
};

// Needs (0..255: how much it would like to do this). Background needs only build preferences.
enum
{
    NEED_HUNGER,
    NEED_REST,
    NEED_PLAY,
    NEED_EXPLORE,
    NEED_COMPANY,   // social ones: wants company
    NEED_QUIET,     // solitary ones: wants a quiet moment alone
    NEED_TYPE_A,    // its first type's need
    NEED_TYPE_B,    // its second type's need (same as A for one type)
    LIFE_NEEDS
};

// Type needs (one per type; dual types have two, the temperament decides which leads)
enum
{
    TNEED_NONE,
    TNEED_COZY,     // NORMAL: a soft, cozy spot
    TNEED_SPAR,     // FIGHTING: practice moves
    TNEED_SKY,      // FLYING: open sky
    TNEED_DAMP,     // POISON: damp, murky places
    TNEED_DIG,      // GROUND: dig
    TNEED_STONE,    // ROCK: rocks and hard ground
    TNEED_FOLIAGE,  // BUG: leaves and tall grass
    TNEED_SHADE,    // GHOST: shade, dusk
    TNEED_SHINE,    // STEEL: shiny things, polishing
    TNEED_BASK,     // FIRE: bask in warmth
    TNEED_SOAK,     // WATER: soak in water
    TNEED_SUN,      // GRASS: sunlight on its leaves
    TNEED_STORM,    // ELECTRIC: perks up in storms
    TNEED_STILL,    // PSYCHIC: a still, quiet moment
    TNEED_COOL,     // ICE: cold, cool places
    TNEED_HEIGHT,   // DRAGON: high places
    TNEED_NIGHT,    // DARK: night, darkness
    TNEED_COUNT
};

// The eight short-lived emotions (0..100)
enum
{
    EMO_JOY,
    EMO_FEAR,
    EMO_FRUSTRATION,
    EMO_SADNESS,
    EMO_SURPRISE,
    EMO_AFFECTION,
    EMO_PRIDE,
    EMO_CALM,
    LIFE_EMOTIONS
};

// How it was involved in what happened (docs: took part / witnessed / heard later)
enum
{
    TIER_PART,
    TIER_WITNESS,
    TIER_HEARD,
};

// Kinds of memories
enum
{
    MEM_NONE,
    MEM_MET,            // came to its Trainer (caught, param: how: 0 caught, 1 hatched, 2 received, 3 traded)
    MEM_EVOLVED,
    MEM_BADGE,          // param: badge number
    MEM_STORY,          // param: story beat (LIFE_STORY_*)
    MEM_LEGEND,         // param: species seen
    MEM_NEW_PLACE,      // first time in a town or city (place)
    MEM_WEATHER,        // first time in a weather (param: weather)
    MEM_FRIGHT,         // something scared it (param: LIFE_TRIG_*)
    MEM_OVERCAME,       // a fear softened (param: LIFE_TRIG_*)
    MEM_NEAR_FAINT,     // hung on at low HP
    MEM_FAINTED,
    MEM_CLUTCH,         // the knockout that won it
    MEM_COMEBACK,       // won from behind
    MEM_PARTNER,        // a partner saved it / it saved a partner (who: the partner)
    MEM_RIVAL,          // a battle with a rival (who: the rival)
    MEM_COMFORTED,      // comforted when it was upset (who: the comforter)
    MEM_FED,            // fed when it was actually looking for food
    MEM_PLAYED,         // played a game it wanted to play (who)
    MEM_PRAISED,        // praised after doing well
    MEM_REUNION,        // met its Trainer again after the adventure moved on
    MEM_KEEPSAKE,       // was given a keepsake (param: item)
    MEM_HEARD,          // heard news from a teammate (who: the teller, param: the memory kind heard)
    MEM_RARE,           // something rare happened (param: LIFE_RARE_*)
    MEM_PURSUIT,        // a pursuit milestone (param: pursuit << 8 | level)
    MEM_FRIEND,         // a friend was made (who)
    MEM_FAMILY,         // met its family (who: parent / hatchling)
    MEM_GARDEN,         // first time in the Garden
    MEM_KINDS
};

// Story beats (MEM_STORY's param; mp_life_world.c tags them)
enum
{
    LIFE_STORY_NONE,
    LIFE_STORY_RIVAL,           // a battle with May / Brendan
    LIFE_STORY_WALLY,           // a battle with Wally
    LIFE_STORY_MAGMA,           // stood up to Team Magma
    LIFE_STORY_AQUA,            // stood up to Team Aqua
    LIFE_STORY_MT_CHIMNEY,
    LIFE_STORY_WEATHER_INST,
    LIFE_STORY_SPACE_CENTER,
    LIFE_STORY_SEAFLOOR,
    LIFE_STORY_GROUDON,         // saw Groudon awaken
    LIFE_STORY_KYOGRE,
    LIFE_STORY_RAYQUAZA,
    LIFE_STORY_SOOTOPOLIS,      // the crisis was calmed
    LIFE_STORY_ELITE_FOUR,
    LIFE_STORY_CHAMPION,
    LIFE_STORY_COUNT
};

// What a fear or a learned trait is about
enum
{
    LIFE_TRIG_NONE,
    LIFE_TRIG_THUNDER,
    LIFE_TRIG_DARK,
    LIFE_TRIG_WATER,
    LIFE_TRIG_HEIGHTS,
    LIFE_TRIG_GHOSTS,
    LIFE_TRIG_CROWDS,
    LIFE_TRIG_LEGEND,
    LIFE_TRIG_SAND,
    LIFE_TRIG_COUNT
};

// Rare moments (MEM_RARE's param; docs 12: only when temperament, bonds, place, weather and time line up)
enum
{
    LIFE_RARE_NONE,
    LIFE_RARE_STARGAZE,     // two best friends stargazing on a clear night
    LIFE_RARE_PADDLE,       // a Water type teaching a nervous Fire type to paddle (param on the learner: who taught)
    LIFE_RARE_RIVAL_NAP,    // old rivals finally sharing a nap
    LIFE_RARE_FAMILY,       // grooming its family
    LIFE_RARE_COUNT
};

// Own pursuits (MpLifeRec.pursuit; docs 12: no stats, no prizes)
enum
{
    LIFE_PURSUIT_NONE,
    LIFE_PURSUIT_CLIMB,     // the grove's trees
    LIFE_PURSUIT_SWIM,      // splashing at the pond's edge
    LIFE_PURSUIT_SING,
    LIFE_PURSUIT_COLLECT,   // pretty stones
    LIFE_PURSUIT_DIG,       // tunnels in the sand
    LIFE_PURSUIT_TEACH,     // showing the younger ones
    LIFE_PURSUIT_COUNT
};
#define LIFE_PURSUIT_MILESTONES 3   // at pursuitLevel 60, 150, 250 (MEM_PURSUIT param: pursuit << 8 | milestone)

// Learned traits
enum
{
    TRAIT_NONE,
    TRAIT_AFRAID,       // param: LIFE_TRIG_*
    TRAIT_BRAVE,        // param: LIFE_TRIG_* (got over it, or never minded)
    TRAIT_LOVES_PLACE,  // param: map section
    TRAIT_PROTECTIVE,   // param: low 16 bits of the bond key it protects
    TRAIT_CONFIDENT,    // battles went well
    TRAIT_COMFORT_FOOD, // param: flavor
    TRAIT_DISLIKES,     // param: verb it came to dislike (LIFE_VERB_*)
    TRAIT_FAMILY_SONG,  // a song its family sings (eggs, docs 12; param: low 16 bits of the parent's key)
    TRAIT_FAMILY_GREETING, // greets the way its family does (param: as above)
    TRAIT_COUNT
};

// Interactions (the verb wheel)
enum
{
    LIFE_VERB_TALK,
    LIFE_VERB_PET,
    LIFE_VERB_FEED,
    LIFE_VERB_PLAY,
    LIFE_VERB_PRAISE,
    LIFE_VERB_COMFORT,
    LIFE_VERB_WATCH,
    LIFE_VERB_GROOM,
    LIFE_VERB_KEEPSAKE,
    LIFE_VERB_PICKUP,   // being picked up (a boundary only)
    LIFE_VERBS
};

#define LIFE_TRAITS    12
#define LIFE_MEMORIES  32
#define LIFE_PLACES    24

struct MpLifeTrait
{
    u8 id;          // TRAIT_*
    u8 strength;    // 0..255; gone at 0
    u16 param;
};

struct MpLifeMemory
{
    u8 kind;        // MEM_*
    u8 feeling;     // EMO_* it felt most
    u8 strength;    // 0..255
    u8 tier;        // TIER_*
    u16 place;      // where: its map section (MAPSEC_*), MAPSEC_NONE = nowhere in particular
    u16 param;
    u32 who;        // bond key of who was part of it (0 = nobody in particular)
    u32 when;       // play time in minutes
};

struct MpLifePlace
{
    u16 map;        // map section (MAPSEC_*): a town, a route, a cave
    u8 level;       // familiarity 0..255
    u8 flags;       // LIFE_PLACE_*
};

#define LIFE_PLACE_MET      (1 << 0)
#define LIFE_PLACE_EVOLVED  (1 << 1)
#define LIFE_PLACE_FAVORITE (1 << 2)
#define LIFE_PLACE_HOME     (1 << 3)

struct MpLifeRec
{
    u32 personality;
    u32 otId;
    u16 species;            // last seen species (evolution is noticed by a change)
    u8 version;
    u8 flags;               // LIFE_REC_*
    u8 base[LIFE_AXES];     // the temperament it was born with
    u8 cur[LIFE_AXES];      // today's (drifts slowly within a range around base)
    u8 need[LIFE_NEEDS];
    u8 emo[LIFE_EMOTIONS];
    s8 mood;                // the slow mood: -100 unsettled .. 100 content
    u8 typeLean;            // 0..255: how much NEED_TYPE_A leads over NEED_TYPE_B
    u16 boundaries;         // LIFE_VERB_* bits it doesn't like
    s8 taste[5];            // FLAVOR_*: -100 dislikes .. 100 loves
    u8 pursuit;             // LIFE_PURSUIT_* it took up (0 = none)
    u8 pursuitLevel;
    u8 watchMark;           // when watching last taught something (MP_Life_Observe; 0 = never)
    u16 keepsake;           // item it was given and keeps
    u16 favDecor;           // its favorite Garden decoration (0 = none)
    u16 lastPartyMark;      // the adventure's progress when it was last with its Trainer
    u32 lastLineMin;        // play minute of its last battle line (rare lines, docs 9)
    u32 known[2];           // journal facts the Trainer has learned (LIFE_FACT_* bits)
    u32 parents[2];         // its parents' bond keys (eggs), 0 = unknown
    u32 rareSeen;           // LIFE_RARE_* bits the Trainer witnessed
    u32 createdMin;
    u32 lastTickMin;
    struct MpLifeTrait trait[LIFE_TRAITS];
    struct MpLifeMemory mem[LIFE_MEMORIES];
    struct MpLifePlace place[LIFE_PLACES];
};

#define LIFE_REC_RAISED_BEFORE (1 << 0)   // owned before the Garden existed (Friendship gave its bond a head start)
#define LIFE_REC_IN_PARTY      (1 << 1)   // was in the party at the last check (reunions)
#define LIFE_REC_FAMILY_KNOWN  (1 << 2)
#define LIFE_REC_EGG           (1 << 3)   // made for an egg (its parents known): hatching finishes it

// How one Pokémon feels about a Trainer or another Pokémon (one-sided: each direction is its own bond)
enum
{
    BOND_FAMILIAR,
    BOND_TRUST,
    BOND_AFFECTION,
    BOND_RESPECT,
    BOND_FRICTION,
    BOND_PARTS
};

struct MpLifeBond
{
    u32 from;               // the Pokémon's key
    u32 to;                 // the Trainer's or Pokémon's key
    u8 v[BOND_PARTS];
    u8 flags;               // LIFE_BOND_*
    u16 shared;             // shared experiences
    u32 lastMin;            // last shared moment (play minutes)
};

#define LIFE_BOND_TRAINER   (1 << 0)
#define LIFE_BOND_RIVAL     (1 << 1)
#define LIFE_BOND_FAMILY    (1 << 2)
#define LIFE_BOND_MET       (1 << 3)

// Things that happened that Pokémon tell each other about (word spreads, docs 8.1)
#define LIFE_NEWS 16
struct MpLifeNews
{
    u8 kind;        // MEM_BADGE, MEM_STORY, MEM_LEGEND, MEM_CLUTCH...
    u8 pad;
    u16 param;
    u32 when;       // play minute
    u32 teller;     // who it happened to (0 = everyone who was there)
};
const struct MpLifeNews *MP_Life_News(u8 *count);
void MP_Life_AddNews(u8 kind, u16 param, u32 teller);
// The MOéMON GARDEN's own save data (mp_garden.c), kept in the .life sidecar
#define GARDEN_DECOR_MAX 16
#define MP_GARDEN_VISITED  (1 << 0)
#define MP_GARDEN_SHORTCUT (1 << 1)   // (1.15: came by the shortcut; no longer used)
#define MP_GARDEN_LASTPC   (1 << 2)   // lastPc* hold the last POKéMON CENTER visited
struct MpGardenDecor
{
    u8 decor;       // DECOR_* (0 = none)
    u8 x, y;        // map tile (no MAP_OFFSET)
    u8 pad;
};
struct MpGardenSave
{
    u32 flags;
    u8 retGroup, retNum, retX, retY;   // (1.15: where the shortcut came from; no longer used)
    struct MpGardenDecor decor[GARDEN_DECOR_MAX];
    u8 lastPcGroup, lastPcNum, lastPcWarp, lastPcPad;   // the last POKéMON CENTER visited: its 1F, its stairs
    u32 reserved[3];
};
struct MpGardenSave *MP_Life_GardenSave(void);
u32 MP_Life_LastLineMin(void);           // the last rare battle line (play minutes, 0 = never)
void MP_Life_SetLastLineMin(u32 minute);

// Keys: a Pokémon is its personality and OT ID; a Trainer is their ID and name
u32 MP_Life_MonKey(u32 personality, u32 otId);
u32 MP_Life_BoxKey(struct BoxPokemon *box);
u32 MP_Life_TrainerKey(u32 otId, const u8 *name);
u32 MP_Life_LocalTrainerKey(void);
bool8 MP_Life_IsTrainerKey(u32 key);

struct MpLifeRec *MP_Life_Get(struct BoxPokemon *box, bool8 create);
struct MpLifeRec *MP_Life_FindKey(u32 key);
struct BoxPokemon *MP_Life_PartyBox(u32 key);   // NULL when that Pokémon isn't in the party

// Experiences
struct MpLifeMemory *MP_Life_Remember(struct MpLifeRec *rec, u8 kind, u8 strength, u8 tier, u32 who, u16 param);
void MP_Life_Feel(struct MpLifeRec *rec, u8 emotion, u8 amount);
u8 MP_Life_TopEmotion(struct MpLifeRec *rec, u8 *amount);
void MP_Life_AddTrait(struct MpLifeRec *rec, u8 id, u16 param, u8 strength);
struct MpLifeTrait *MP_Life_FindTrait(struct MpLifeRec *rec, u8 id, u16 param);
void MP_Life_SoftenTrait(struct MpLifeRec *rec, u8 id, u16 param, u8 amount);
u8 MP_Life_TypeNeed(struct MpLifeRec *rec, bool8 second);   // TNEED_* of NEED_TYPE_A / B
void MP_Life_Satisfy(struct MpLifeRec *rec, u8 need, u8 amount);
u8 MP_Life_PressingNeed(struct MpLifeRec *rec, u8 *amount);  // its strongest need right now

// Bonds
struct MpLifeBond *MP_Life_Bond(u32 from, u32 to, bool8 create);
void MP_Life_BondShare(u32 from, u32 to, s8 familiar, s8 trust, s8 affection, s8 respect, s8 friction);
int MP_Life_BondWarmth(const struct MpLifeBond *b);  // a rough "how much it likes them" for choices
void MP_Life_BondNudge(u32 from, u32 to, u8 part, u8 amount, u8 cap);
u16 MP_Life_PackRecord(u32 key, u8 *out, u16 max);   // a record and its bonds for a friend's game (0 = none)
bool8 MP_Life_UnpackRecord(const u8 *in, u16 len, bool8 traded, const u8 *fromTrainerName, u32 fromTrainerKey);
void MP_Life_Closest(u32 from, u32 *friendKey, u32 *rivalKey, u32 *familyKey, u32 *favoriteKey);   // 0 = none

// The places it knows (by map section: towns, routes, caves; region-independent)
struct MpLifePlace *MP_Life_Place(struct MpLifeRec *rec, u16 mapsec, bool8 create);
u16 MP_Life_CurrentPlace(void);

// Names of the Pokémon and Trainers it knows (for journal lines about friends in other games)
void MP_Life_NoteName(u32 key, const u8 *name);
bool8 MP_Life_NameOf(u32 key, u8 *out);   // FALSE when unknown

// Journal facts (MpLifeRec.known bits): what the Trainer has come to know
#define LIFE_FACT_TEMPER(axis)  (axis)          // 0..5
#define LIFE_FACT_LIKES         6
#define LIFE_FACT_DISLIKES      7
#define LIFE_FACT_TYPE_NEED     8
#define LIFE_FACT_BOUNDARY      9
#define LIFE_FACT_PURSUIT       10
#define LIFE_FACT_KEEPSAKE      11
#define LIFE_FACT_TRAIT(slot)   (12 + (slot))   // 12..23
#define LIFE_FACT_FRIEND        24
#define LIFE_FACT_RIVAL         25
#define LIFE_FACT_FAMILY        26
#define LIFE_FACT_FAVORITE      27              // a favorite Trainer other than its own
#define LIFE_FACT_MEMORY(slot)  (28 + (slot))   // 28..59
#define LIFE_FACT_PLACE         60
#define LIFE_FACT_DECOR         61

// The journal (summary screen): lines of game text, revealed facts only (others as "???")
#define LIFE_JOURNAL_LINES 16
#define LIFE_JOURNAL_TEXT  96
u8 MP_Life_Journal(struct BoxPokemon *box, u8 lines[][LIFE_JOURNAL_TEXT], u8 max);
// Watching it (WATCH QUIETLY, the GARDEN's WATCH): something about it the Trainer can see for themselves (its
// temperament, what it loves doing, its pursuit; never its past), at most once per half hour of play for each
// Pokémon. TRUE: a new journal line.
bool8 MP_Life_Observe(struct BoxPokemon *box);
void MP_Life_Reveal(struct MpLifeRec *rec, u8 fact);
u8 MP_Life_JournalKnownCount(void);   // known lines in the last journal built (no "???" line)

bool8 MP_Life_WantsOut(struct BoxPokemon *box);
void MP_Life_NoteEgg(struct BoxPokemon *egg, struct BoxPokemon *parentA, struct BoxPokemon *parentB);   // daycare.c
u8 MP_Life_PursuitFor(struct MpLifeRec *rec);   // the pursuit that fits its instincts and temperament (LIFE_PURSUIT_*)
bool8 MP_Life_NewBody(struct MpLifeRec *rec);   // evolved within the last quarter hour: still getting used to it
// A teammate's evolution (docs 12: friends react): how this one takes it, once per evolution (the first
// time it sees the new body: the follower, or in the Garden). Applies the feelings and bond changes.
enum { LIFE_EVO_REACT_NONE, LIFE_EVO_REACT_PROUD, LIFE_EVO_REACT_AWED, LIFE_EVO_REACT_JEALOUS };
u8 MP_Life_EvolutionReaction(struct MpLifeRec *watcher, struct MpLifeRec *evolved);
struct MpLifeTrait *MP_Life_FamilyTrait(struct MpLifeRec *rec, u8 id);   // TRAIT_FAMILY_*, whatever the param
bool8 MP_Life_IsFamily(u32 a, u32 b);   // asking to come out (party screen, follower picker)

// Every frame on the map (mp_main.c MP_FrameUpdate) and the minute tick
void MP_Life_FrameUpdate(void);
u32 MP_Life_PlayMinutes(void);
u16 MP_Life_ProgressMark(void);

// The save sidecar ("<save>.life")
void MP_Life_WriteSidecar(void);
void MP_Life_LoadFromSave(void);
void MP_Life_Reset(void);

// Debug (mp_debug.c: mp life ...)
void MP_Life_Debug(const char *args);

#endif // GUARD_MP_LIFE_H
