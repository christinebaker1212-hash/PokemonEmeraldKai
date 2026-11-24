// Moemon Kai: Yu-Gi-Oh! duelists on the map. Who plays Yu-Gi-Oh! rather than the Pokemon cards (src/tcg/tcg_field.c
// asks here first), the deck they bring (a theme by who they are, built from the cards that work, stronger with
// badges, with a nod to the show's duelists of that type), the special duelists and their signature cards, and the
// CARD CLUB's Yu-Gi-Oh! duelist in the POKeMON MARTS.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "item.h"
#include "mp.h"
#include "mp_region.h"
#include "script.h"
#include "constants/event_objects.h"
#include "constants/flags.h"
#include "constants/items.h"
#include "constants/opponents.h"
#include "constants/songs.h"
#include "constants/trainers.h"
#include "constants/trainer_types.h"
#include "data.h"
#include "tcg/integration.h"
#include "tcg/quotes.h"
#include "ygo/integration.h"
#include "ygo/npc.h"
#include "ygo/duel.h"
#include "ygo_internal.h"

// ---------------------------------------------------------------------------
// Themes

enum
{
    TH_SPELLCASTER,
    TH_FIEND,
    TH_ZOMBIE,
    TH_INSECT,
    TH_WARRIOR,
    TH_HARPIE,
    TH_DRAGON,
    TH_ROCK,
    TH_PYRO,
    TH_MACHINE,
    TH_SEA,
    TH_DINO,
    TH_TOON,
    TH_BEAST,
    TH_PLANT,
    TH_AQUA,
    TH_FAIRY,
    TH_REPTILE,
    TH_BEAST_WARRIOR,
    TH_COUNT
};
#define RACE(r) (1u << (r))
struct Theme
{
    u32 races;
    u8 attribute;                 // 0 any
    const char *field, *equip[2];
    const char *nods[6];          // the show's duelists of this type: their cards, when they work
};
static const struct Theme sThemes[TH_COUNT] = {
    [TH_SPELLCASTER] = {RACE(YGO_SPELLCASTER), 0, "Yami", {"Book of Secret Arts", "Black Pendant"},
                        {"Dark Magician", "Magician of Faith", "Mystical Elf", "Dark Magician Girl"}},
    [TH_FIEND] = {RACE(YGO_FIEND), 0, "Yami", {"Dark Energy", "Axe of Despair"},
                  {"Summoned Skull", "Archfiend Soldier"}},
    [TH_ZOMBIE] = {RACE(YGO_ZOMBIE), 0, "Wasteland", {"Violet Crystal", "Axe of Despair"},
                   {"Clown Zombie", "Armored Zombie", "Dragon Zombie", "Pyramid Turtle"}},
    [TH_INSECT] = {RACE(YGO_INSECT), 0, "Forest", {"Laser Cannon Armor", "Malevolent Nuzzler"},
                   {"Basic Insect", "Insect Queen", "Killer Needle", "Hercules Beetle", "Man-Eater Bug"}},
    [TH_WARRIOR] = {RACE(YGO_WARRIOR), 0, "Sogen", {"Legendary Sword", "Black Pendant"},
                    {"Time Wizard", "Baby Dragon", "Flame Swordsman", "Panther Warrior", "Gearfried the Iron Knight"}},
    [TH_HARPIE] = {RACE(YGO_WINGED_BEAST), YGO_WIND, "Mountain", {"Follow Wind", "Malevolent Nuzzler"},
                   {"Harpie Lady", "Harpie Lady Sisters", "Harpie's Pet Dragon", "Harpie's Feather Duster"}},
    [TH_DRAGON] = {RACE(YGO_DRAGON), 0, "Mountain", {"Dragon Treasure", "Axe of Despair"},
                   {"Lord of D.", "Spear Dragon", "Blue-Eyes White Dragon"}},
    [TH_ROCK] = {RACE(YGO_ROCK), 0, "Wasteland", {"Axe of Despair", NULL},
                 {"Giant Soldier of Stone", "Guardian Sphinx"}},
    [TH_PYRO] = {RACE(YGO_PYRO), 0, NULL, {"Raise Body Heat", "Axe of Despair"},
                 {"Hinotama", "Tremendous Fire", "Lava Golem"}},
    [TH_MACHINE] = {RACE(YGO_MACHINE), 0, NULL, {"Machine Conversion Factory", "Axe of Despair"},
                    {"Barrel Dragon", "Slot Machine", "Launcher Spider", "Cannon Soldier", "Mechanicalchaser"}},
    [TH_SEA] = {RACE(YGO_FISH) | RACE(YGO_SEA_SERPENT), 0, "Umi", {"Power of Kaishin", NULL},
                {"The Legendary Fisherman", "Fortress Whale", "Kairyu-Shin", "Great White", "Flying Fish"}},
    [TH_DINO] = {RACE(YGO_DINOSAUR), 0, "Wasteland", {"Raise Body Heat", "Axe of Despair"},
                 {"Two-Headed King Rex", "Sword Arm of Dragon", "Uraby", "Megazowler"}},
    [TH_TOON] = {RACE(YGO_SPELLCASTER) | RACE(YGO_FIEND), 0, "Yami", {"Axe of Despair", NULL},
                 {"Toon World", "Toon Mermaid", "Toon Summoned Skull", "Relinquished"}},
    [TH_BEAST] = {RACE(YGO_BEAST), 0, "Forest", {"Beast Fangs", "Axe of Despair"},
                  {"Mother Grizzly", "Nimble Momonga", "Giant Rat"}},
    [TH_PLANT] = {RACE(YGO_PLANT), 0, "Forest", {"Vile Germs", NULL}, {"Mystic Tomato"}},
    [TH_AQUA] = {RACE(YGO_AQUA), YGO_WATER, "Umi", {"Power of Kaishin", NULL}, {"Mother Grizzly"}},
    [TH_FAIRY] = {RACE(YGO_FAIRY), 0, NULL, {"Silver Bow and Arrow", "Horn of the Unicorn"},
                  {"Shining Angel", "Mystical Elf", "Gemini Elf"}},
    [TH_REPTILE] = {RACE(YGO_REPTILE), 0, "Wasteland", {"Axe of Despair", NULL}, {"Ancient Lizard Warrior"}},
    [TH_BEAST_WARRIOR] = {RACE(YGO_BEAST_WARRIOR), 0, "Sogen", {"Axe of Despair", "Black Pendant"},
                          {"Vorse Raider"}},
};
// Spells and Traps anyone might play, weakest tier first
static const char *const sSupport[4][10] = {
    {"Dian Keto the Cure Master", "Trap Hole", "Sparks", "Hinotama", "Castle Walls", "Reinforcements", "Fissure",
     "Waboku", "Red Medicine", "Dragon Capture Jar"},
    {"Monster Reborn", "Swords of Revealing Light", "Just Desserts", "Ookazi", "Rush Recklessly", "Stop Defense",
     "Goblin's Secret Remedy"},
    {"Mystical Space Typhoon", "Dark Hole", "Change of Heart", "Mirror Force", "Magic Cylinder", "Negate Attack",
     "Premature Burial", "Call of the Haunted", "Megamorph", "Mystic Plasma Zone"},
    {"Raigeki", "Heavy Storm", "Harpie's Feather Duster", "Pot of Greed", "Graceful Charity", "Solemn Judgment",
     "Torrential Tribute", "Snatch Steal", "Ring of Magnetism", "Upstart Goblin"},
};

static unsigned CardNamed(const char *name)
{
    unsigned id;
    if (!name)
        return 0;
    for (id = 1; id <= YGO_CARD_COUNT; id++)
        if (!strcmp(gYgoCards[id].name, name))
            return id;
    return 0;
}
static u32 Next(u32 *s)
{
    *s ^= *s << 13;
    *s ^= *s >> 17;
    *s ^= *s << 5;
    return *s;
}
static int InTheme(const struct YgoCard *c, unsigned theme)
{
    const struct Theme *t = &sThemes[theme];
    return (t->races & RACE(c->race)) && (!t->attribute || c->attribute == t->attribute);
}
static int Playable(const struct YgoCard *c)
{
    return c && c->frame != YGO_TOKEN && Ygo_CardReady(c->id) && Ygo_GameLimit(c->id);
}
// What a tier allows: Level 4 or lower up to this ATK, this many tribute monsters, this many of Level 7 or higher
static const u16 sLowAtk[4] = {1400, 1600, 1800, 2100};
static const u8 sTributes[4] = {1, 3, 4, 6}, sBig[4] = {0, 1, 2, 3};

struct Build
{
    u16 *main, *extra;
    unsigned count, extras, tributes, big, tier;
};
static unsigned Copies(const struct Build *b, unsigned id)
{
    unsigned i, n = 0;
    for (i = 0; i < b->count; i++)
        n += b->main[i] == id;
    return n;
}
// Adds up to `copies` of a card the tier allows; returns how many went in
static unsigned Add(struct Build *b, unsigned id, unsigned copies)
{
    const struct YgoCard *c = Ygo_Card(id);
    unsigned n = 0, limit;
    if (!Playable(c) || Ygo_IsExtra(c))
        return 0;
    limit = Ygo_GameLimit(id);
    if (Ygo_IsMonster(c))
    {
        if (c->frame == YGO_RITUAL)
            return 0;
        if (c->level >= 7 && b->big >= sBig[b->tier])
            return 0;
        if (c->level >= 5 && b->tributes >= sTributes[b->tier])
            return 0;
        if (c->level <= 4 && c->atk != YGO_STAT_UNKNOWN && c->atk > sLowAtk[b->tier])
            return 0;
        if (c->level >= 5)
            copies = 1;
    }
    while (n < copies && Copies(b, id) < limit && b->count < 40)
    {
        b->main[b->count++] = (u16)id;
        n++;
        if (Ygo_IsMonster(c) && c->level >= 5)
            b->tributes++;
        if (Ygo_IsMonster(c) && c->level >= 7)
            b->big++;
    }
    return n;
}
static int Score(const struct YgoCard *c)
{
    int atk = c->atk == YGO_STAT_UNKNOWN ? 1000 : c->atk, def = c->def == YGO_STAT_UNKNOWN ? 1000 : c->def;
    int s = atk > def ? atk : def * 4 / 5;
    if (c->frame == YGO_EFFECT)
        s += 250;
    return s;
}
// The monsters: the theme's first (the strongest the tier allows, with a little chance), others to fill
static void AddMonsters(struct Build *b, unsigned theme, unsigned want, int onTheme, u32 *rng)
{
    static u16 list[YGO_CARD_COUNT];
    static int score[YGO_CARD_COUNT];
    unsigned n = 0, id, i, j;
    for (id = 1; id <= YGO_CARD_COUNT; id++)
    {
        const struct YgoCard *c = Ygo_Card(id);
        if (!Playable(c) || !Ygo_IsMonster(c) || Ygo_IsExtra(c) || c->frame == YGO_RITUAL)
            continue;
        if (onTheme != InTheme(c, theme))
            continue;
        list[n] = (u16)id;
        score[n] = Score(c) + (int)(Next(rng) % (unsigned)(500 - 100 * (int)b->tier));
        n++;
    }
    for (i = 1; i < n; i++)
        for (j = i; j > 0 && score[j] > score[j - 1]; j--)
        {
            u16 t = list[j];
            int s = score[j];
            list[j] = list[j - 1], score[j] = score[j - 1];
            list[j - 1] = t, score[j - 1] = s;
        }
    for (i = 0; i < n && want; i++)
    {
        unsigned got = Add(b, list[i], b->tier >= 2 ? 3 : 2);
        want = got >= want ? 0 : want - got;
    }
}
static unsigned Monsters(const struct Build *b)
{
    unsigned i, n = 0;
    for (i = 0; i < b->count; i++)
        n += Ygo_IsMonster(Ygo_Card(b->main[i]));
    return n;
}
// A deck for a theme at a tier (0-3): 40 cards and, when its monsters make one, a Fusion Deck
static void BuildDeck(struct YgoOpponent *o, unsigned theme, unsigned tier, u32 seed)
{
    struct Build b;
    const struct Theme *t = &sThemes[theme];
    u32 rng = seed | 1;
    unsigned i, k, id;
    memset(o->main, 0, sizeof(o->main));
    memset(o->extra, 0, sizeof(o->extra));
    memset(&b, 0, sizeof(b));
    b.main = o->main;
    b.extra = o->extra;
    b.tier = tier > 3 ? 3 : tier;
    // a nod to the show's duelists of the type
    for (i = 0; i < ARRAY_COUNT(t->nods); i++)
        Add(&b, CardNamed(t->nods[i]), b.tier >= 2 ? 2 : 1);
    AddMonsters(&b, theme, 22 > Monsters(&b) ? 22 - Monsters(&b) : 0, 1, &rng);
    if (Monsters(&b) < 20)
        AddMonsters(&b, theme, 20 - Monsters(&b), 0, &rng);
    // the theme's field and equips
    Add(&b, CardNamed(t->field), b.tier >= 1 ? 2 : 1);
    for (i = 0; i < 2; i++)
        Add(&b, CardNamed(t->equip[i]), b.tier >= 2 ? 2 : 1);
    // support: the best the tier allows first
    for (k = b.tier + 1; k-- > 0 && b.count < 40;)
    {
        unsigned start = Next(&rng) % 10;
        for (i = 0; i < 10 && b.count < 40; i++)
            Add(&b, CardNamed(sSupport[k][(start + i) % 10]), k == 0 ? 2 : 1);
    }
    // the Fusions its monsters make
    for (id = 1; id <= YGO_CARD_COUNT && b.extras < YGO_FUSION_MAX; id++)
    {
        const struct YgoCard *c = Ygo_Card(id);
        const struct YgoFusion *f;
        int ok = 1;
        if (c->frame != YGO_FUSION || !Ygo_CardReady(id) || !Ygo_GameLimit(id) || !(f = Ygo_FusionOf(id)))
            continue;
        for (i = 0; i < 5 && f->materials[i]; i++)
            ok &= !(f->materials[i] & (YGO_MAT_ANY | YGO_MAT_ATTR)) && Copies(&b, f->materials[i]) > 0;
        if (ok && i)
            b.extra[b.extras++] = (u16)id;
    }
    if (b.extras)
        Add(&b, CardNamed("Polymerization"), 2);
    // anything short of 40: more monsters
    if (b.count < 40)
        AddMonsters(&b, theme, 40 - b.count, 0, &rng);
    for (i = 0; b.count < 40 && i < 400; i++)
    {
        const struct YgoCard *c = Ygo_Card(1 + Next(&rng) % YGO_CARD_COUNT);
        if (c->frame == YGO_NORMAL && c->level <= 4)
            Add(&b, c->id, 1);
    }
}
// Their signature card: the strongest working monster of the type outside the Starter and Structure Decks
static unsigned Signature(unsigned theme)
{
    unsigned id, best = 0, k;
    int bestScore = -1;
    for (id = 1; id <= YGO_CARD_COUNT; id++)
    {
        const struct YgoCard *c = Ygo_Card(id);
        int s;
        if (!Playable(c) || !Ygo_IsMonster(c) || !InTheme(c, theme) || c->frame == YGO_RITUAL || Ygo_IsExtra(c))
            continue;
        for (k = 0; k < c->setCount && !Ygo_SetIsDeck(gYgoCardSets[c->setFirst + k]); k++)
            ;
        if (k < c->setCount)
            continue;
        s = Score(c) + 120 * c->rarity;
        if (s > bestScore)
            bestScore = s, best = id;
    }
    return best;
}

// ---------------------------------------------------------------------------
// Who plays

static unsigned TierOfBadges(void)
{
    u8 b = MP_Badges();   // (the region the player is in: HOENN is never the default)
    return b >= 7 ? 3 : b >= 4 ? 2 : b >= 2 ? 1 : 0;
}

enum { REQ_NONE, REQ_TRAINER };
enum { PLAY_YGO = YGO_PLAYS_YGO, PLAY_BOTH = YGO_PLAYS_BOTH };
struct Duelist
{
    u8 gfx, gfx2, req, theme, play;
    u16 reqArg;
    const char *name, *title;
    u16 music;
    const u8 *accept, *refuse;
};
static const u8 sText_SidneyYes[] = _("Yu-Gi-Oh!, huh? My FIENDS play dirty,\nkid. Hope you can take it!");
static const u8 sText_PhoebeYes[] = _("Ooh, a duel! My ZOMBIES never stay\nin the GRAVEYARD for long...");
static const u8 sText_GlaciaYes[] = _("Yu-Gi-Oh!... Let us see if your deck\ncan endure the cold depths of mine.");
static const u8 sText_DrakeYes[] = _("A duel! My DRAGONS will test whether\nyou have the will to win!");
static const u8 sText_EliteNo[] = _("Defeat me in battle first.\nThen we may talk about cards.");
static const u8 sText_MaxieYes[] = _("Yu-Gi-Oh!... A game of land and fire.\nVery well. My PYROS will show you.");
static const u8 sText_ArchieYes[] = _("Bwahahaha! A duel? My deck rules the\nsea! Let's see you swim!");
static const u8 sText_AnabelYes[] = _("A duel... I'd like to see how you\nthink. My SPELLCASTERS are ready.");
static const u8 sText_TuckerYes[] = _("A duel! The crowd will go wild!\nMy WARRIORS put on a great show!");
static const u8 sText_SpenserYes[] = _("Heh, Yu-Gi-Oh!? My PLANTS have deep\nroots, kid. Come on!");
static const u8 sText_GretaYes[] = _("All right! My BEAST-WARRIORS are\nfired up! Let's duel!");
static const u8 sText_NolandYes[] = _("A duel? Sure! My MACHINES are built\nfor exactly this!");
static const u8 sText_LucyYes[] = _("...A duel. My REPTILES. Fine.");
static const u8 sText_StevenYes[] = _("You play cards? So do I: rare stones\nand rare cards. Which game shall we play?");
static const u8 sText_RivalYes[] = _("You've got a CARD CASE too?\nWhich game? I'm up for either!");
static const u8 sText_WallyYes[] = _("You play cards! I've been practicing\nboth games. Which one do you want?");

enum
{
    YGO_SPECIAL_SIDNEY,
    YGO_SPECIAL_PHOEBE,
    YGO_SPECIAL_GLACIA,
    YGO_SPECIAL_DRAKE,
    YGO_SPECIAL_MAXIE,
    YGO_SPECIAL_ARCHIE,
    YGO_SPECIAL_ANABEL,
    YGO_SPECIAL_TUCKER,
    YGO_SPECIAL_SPENSER,
    YGO_SPECIAL_GRETA,
    YGO_SPECIAL_NOLAND,
    YGO_SPECIAL_LUCY,
    YGO_SPECIAL_STEVEN,
    YGO_SPECIAL_RIVAL,
    YGO_SPECIAL_WALLY,
    YGO_SPECIAL_COUNT
};
static const struct Duelist sDuelists[YGO_SPECIAL_COUNT] = {
    [YGO_SPECIAL_SIDNEY] = {OBJ_EVENT_GFX_SIDNEY, 0, REQ_TRAINER, TH_FIEND, PLAY_YGO, TRAINER_SIDNEY, "SIDNEY", "ELITE FOUR", MUS_VS_ELITE_FOUR, sText_SidneyYes, sText_EliteNo},
    [YGO_SPECIAL_PHOEBE] = {OBJ_EVENT_GFX_PHOEBE, 0, REQ_TRAINER, TH_ZOMBIE, PLAY_YGO, TRAINER_PHOEBE, "PHOEBE", "ELITE FOUR", MUS_VS_ELITE_FOUR, sText_PhoebeYes, sText_EliteNo},
    [YGO_SPECIAL_GLACIA] = {OBJ_EVENT_GFX_GLACIA, 0, REQ_TRAINER, TH_AQUA, PLAY_YGO, TRAINER_GLACIA, "GLACIA", "ELITE FOUR", MUS_VS_ELITE_FOUR, sText_GlaciaYes, sText_EliteNo},
    [YGO_SPECIAL_DRAKE] = {OBJ_EVENT_GFX_DRAKE, 0, REQ_TRAINER, TH_DRAGON, PLAY_YGO, TRAINER_DRAKE, "DRAKE", "ELITE FOUR", MUS_VS_ELITE_FOUR, sText_DrakeYes, sText_EliteNo},
    [YGO_SPECIAL_MAXIE] = {OBJ_EVENT_GFX_MAXIE, 0, REQ_NONE, TH_PYRO, PLAY_YGO, 0, "MAXIE", "MAGMA LEADER", MUS_VS_AQUA_MAGMA_LEADER, sText_MaxieYes, NULL},
    [YGO_SPECIAL_ARCHIE] = {OBJ_EVENT_GFX_ARCHIE, 0, REQ_NONE, TH_SEA, PLAY_YGO, 0, "ARCHIE", "AQUA LEADER", MUS_VS_AQUA_MAGMA_LEADER, sText_ArchieYes, NULL},
    [YGO_SPECIAL_ANABEL] = {OBJ_EVENT_GFX_ANABEL, 0, REQ_NONE, TH_SPELLCASTER, PLAY_YGO, 0, "ANABEL", "SALON MAIDEN", MUS_VS_FRONTIER_BRAIN, sText_AnabelYes, NULL},
    [YGO_SPECIAL_TUCKER] = {OBJ_EVENT_GFX_TUCKER, 0, REQ_NONE, TH_WARRIOR, PLAY_YGO, 0, "TUCKER", "DOME ACE", MUS_VS_FRONTIER_BRAIN, sText_TuckerYes, NULL},
    [YGO_SPECIAL_SPENSER] = {OBJ_EVENT_GFX_SPENSER, 0, REQ_NONE, TH_PLANT, PLAY_YGO, 0, "SPENSER", "PALACE MAVEN", MUS_VS_FRONTIER_BRAIN, sText_SpenserYes, NULL},
    [YGO_SPECIAL_GRETA] = {OBJ_EVENT_GFX_GRETA, 0, REQ_NONE, TH_BEAST_WARRIOR, PLAY_YGO, 0, "GRETA", "ARENA TYCOON", MUS_VS_FRONTIER_BRAIN, sText_GretaYes, NULL},
    [YGO_SPECIAL_NOLAND] = {OBJ_EVENT_GFX_NOLAND, 0, REQ_NONE, TH_MACHINE, PLAY_YGO, 0, "NOLAND", "FACTORY HEAD", MUS_VS_FRONTIER_BRAIN, sText_NolandYes, NULL},
    [YGO_SPECIAL_LUCY] = {OBJ_EVENT_GFX_LUCY, 0, REQ_NONE, TH_REPTILE, PLAY_YGO, 0, "LUCY", "PIKE QUEEN", MUS_VS_FRONTIER_BRAIN, sText_LucyYes, NULL},
    [YGO_SPECIAL_STEVEN] = {OBJ_EVENT_GFX_STEVEN, 0, REQ_NONE, TH_ROCK, PLAY_BOTH, 0, "STEVEN", "", MUS_VS_CHAMPION, sText_StevenYes, NULL},
    [YGO_SPECIAL_RIVAL] = {OBJ_EVENT_GFX_RIVAL_MAY_NORMAL, OBJ_EVENT_GFX_RIVAL_BRENDAN_NORMAL, REQ_NONE, TH_BEAST, PLAY_BOTH, 0, NULL, "", MUS_VS_RIVAL, sText_RivalYes, NULL},
    [YGO_SPECIAL_WALLY] = {OBJ_EVENT_GFX_WALLY, 0, REQ_NONE, TH_FAIRY, PLAY_BOTH, 0, "WALLY", "", MUS_VS_RIVAL, sText_WallyYes, NULL},
};
#define YGO_SPECIAL_CLUB 31   /* (specialBeaten bit) the CARD CLUB's duelist */

// Where a duelist stands in the world (the user, 2026-11-22: "Tearlaments and meta decks shouldn't be played by
// random people. Assign NPCs decks based on your ELO, their ELO, and their position in the Pokemon world"): how
// strong a YGOPRODeck deck they may bring (shares of the way up the decks the GEN allows, of 1024) and how
// competitive (ygo_npc_pool.c: 0 casual, 1 strong home-made, 2 meta, 3 tournament)
enum { STAND_TOWN, STAND_TRAINER, STAND_SKILLED, STAND_ACE, STAND_COUNT };
static const struct
{
    u16 lo, hi;
    u8 comp;
} sStandings[STAND_COUNT] = {
    [STAND_TOWN] = {51, 461, 0},      // townsfolk: home-made casual decks, never past the middle
    [STAND_TRAINER] = {154, 666, 1},  // trainers and the TEAMS' grunts: home-made, some strong
    [STAND_SKILLED] = {358, 870, 2},  // COOLTRAINERS, EXPERTS, the TEAMS' ADMINS: meta decks
    [STAND_ACE] = {563, 1024, 3},     // ... once the region's LEAGUE is beaten: tournament decks too
};
static unsigned StandingNow(unsigned standing)
{
    return standing == STAND_SKILLED && MP_Region_ClearedHere() ? STAND_ACE : standing;
}

// Trainer classes that duel with Yu-Gi-Oh! cards, their themes and where they stand
static const struct
{
    u8 trainerClass, theme, alt, standing;
} sClasses[] = {
    {TRAINER_CLASS_PSYCHIC, TH_SPELLCASTER, TH_FAIRY, STAND_TRAINER},
    {TRAINER_CLASS_HEX_MANIAC, TH_ZOMBIE, TH_FIEND, STAND_TRAINER},
    {TRAINER_CLASS_NINJA_BOY, TH_INSECT, TH_WARRIOR, STAND_TRAINER},
    {TRAINER_CLASS_BLACK_BELT, TH_WARRIOR, TH_BEAST_WARRIOR, STAND_TRAINER},
    {TRAINER_CLASS_BATTLE_GIRL, TH_HARPIE, TH_WARRIOR, STAND_TRAINER},
    {TRAINER_CLASS_COOLTRAINER, TH_DRAGON, TH_SPELLCASTER, STAND_SKILLED},
    {TRAINER_CLASS_COOLTRAINER_2, TH_DRAGON, TH_HARPIE, STAND_SKILLED},
    {TRAINER_CLASS_RUIN_MANIAC, TH_ROCK, TH_ZOMBIE, STAND_TRAINER},
    {TRAINER_CLASS_KINDLER, TH_PYRO, TH_DINO, STAND_TRAINER},
    {TRAINER_CLASS_COLLECTOR, TH_TOON, TH_DRAGON, STAND_TRAINER},
    {TRAINER_CLASS_EXPERT, TH_MACHINE, TH_BEAST_WARRIOR, STAND_SKILLED},
    {TRAINER_CLASS_GUITARIST, TH_FIEND, TH_WARRIOR, STAND_TRAINER},
    {TRAINER_CLASS_TEAM_AQUA, TH_SEA, TH_AQUA, STAND_TRAINER},
    {TRAINER_CLASS_AQUA_ADMIN, TH_SEA, TH_AQUA, STAND_SKILLED},
    {TRAINER_CLASS_TEAM_MAGMA, TH_DINO, TH_PYRO, STAND_TRAINER},
    {TRAINER_CLASS_MAGMA_ADMIN, TH_DINO, TH_PYRO, STAND_SKILLED},
};
// Townsfolk who duel with Yu-Gi-Oh! cards: the teens and young adults (and the ones a class above dresses as)
static const struct
{
    u16 gfx;
    u8 theme, alt, standing;
    const char *label;
} sTownsfolk[] = {
    {OBJ_EVENT_GFX_BOY_3, TH_WARRIOR, TH_DRAGON, STAND_TOWN, "TEEN"},
    {OBJ_EVENT_GFX_GIRL_2, TH_FAIRY, TH_HARPIE, STAND_TOWN, "TEEN"},
    {OBJ_EVENT_GFX_MAN_2, TH_BEAST, TH_WARRIOR, STAND_TOWN, "YOUNG MAN"},
    {OBJ_EVENT_GFX_MAN_3, TH_WARRIOR, TH_DRAGON, STAND_TOWN, "YOUNG MAN"},
    {OBJ_EVENT_GFX_MAN_4, TH_MACHINE, TH_FIEND, STAND_TOWN, "YOUNG MAN"},
    {OBJ_EVENT_GFX_MAN_5, TH_SPELLCASTER, TH_WARRIOR, STAND_TOWN, "YOUNG MAN"},
    {OBJ_EVENT_GFX_WOMAN_2, TH_HARPIE, TH_FAIRY, STAND_TOWN, "WOMAN"},
    {OBJ_EVENT_GFX_WOMAN_3, TH_SPELLCASTER, TH_PLANT, STAND_TOWN, "WOMAN"},
    {OBJ_EVENT_GFX_WOMAN_5, TH_FAIRY, TH_AQUA, STAND_TOWN, "WOMAN"},
    {OBJ_EVENT_GFX_PSYCHIC_M, TH_SPELLCASTER, TH_FIEND, STAND_TRAINER, "PSYCHIC"},
    {OBJ_EVENT_GFX_HEX_MANIAC, TH_ZOMBIE, TH_FIEND, STAND_TRAINER, "HEX MANIAC"},
    {OBJ_EVENT_GFX_NINJA_BOY, TH_INSECT, TH_WARRIOR, STAND_TRAINER, "NINJA BOY"},
    {OBJ_EVENT_GFX_BLACK_BELT, TH_WARRIOR, TH_BEAST_WARRIOR, STAND_TRAINER, "BLACK BELT"},
    {OBJ_EVENT_GFX_EXPERT_M, TH_MACHINE, TH_BEAST_WARRIOR, STAND_SKILLED, "EXPERT"},
    {OBJ_EVENT_GFX_EXPERT_F, TH_SPELLCASTER, TH_FAIRY, STAND_SKILLED, "EXPERT"},
    {OBJ_EVENT_GFX_AQUA_MEMBER_M, TH_SEA, TH_AQUA, STAND_TRAINER, "TEAM AQUA GRUNT"},
    {OBJ_EVENT_GFX_AQUA_MEMBER_F, TH_SEA, TH_AQUA, STAND_TRAINER, "TEAM AQUA GRUNT"},
    {OBJ_EVENT_GFX_MAGMA_MEMBER_M, TH_DINO, TH_PYRO, STAND_TRAINER, "TEAM MAGMA GRUNT"},
    {OBJ_EVENT_GFX_MAGMA_MEMBER_F, TH_DINO, TH_PYRO, STAND_TRAINER, "TEAM MAGMA GRUNT"},
};
static const u8 sText_Duel1[] = _("A CARD CASE! You play Yu-Gi-Oh!?\nAll right... It's time to duel!");
static const u8 sText_Duel2[] = _("I've been waiting for a real duelist.\nGet your game on!");
static const u8 sText_Duel3[] = _("My deck has never let me down.\nLet's duel!");
static const u8 sText_DuelTeam[] = _("Hah! Our TEAM plays Yu-Gi-Oh!, too.\nI'll crush your deck!");
static const u8 sText_DuelNo1[] = _("Yu-Gi-Oh!? Sorry, my deck's at home.\nNext time!");
static const u8 sText_DuelNo2[] = _("I'm still building my deck...\nCome back later!");
static const u8 sText_YgoBattleFirst[] = _("A duel? First, a MOéMON battle!\nBeat me, and then we'll talk!");
static const u8 *const sDuelLines[] = {sText_Duel1, sText_Duel2, sText_Duel3};

static u8 ThemeAt(u8 theme, u8 alt, u32 h) { return (h >> 9) % 3 == 0 ? alt : theme; }

// The Game Boy Advance decks the player met lately (Ygo_NoteDeckMet, as a duel starts): passed over while others fit
#define RECENT_DECKS 12
static s16 sRecentDecks[RECENT_DECKS] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
static u8 sRecentAt;
static s16 sLastGameDeck = -1;   // the last one GameDeckAt made

static bool8 MetLately(unsigned index)
{
    unsigned i;
    for (i = 0; i < RECENT_DECKS; i++)
        if (sRecentDecks[i] == (s16)index)
            return TRUE;
    return FALSE;
}

static int sLastPoolDeck = -1;   // the last pool deck PoolDeckAt made
void Ygo_NoteDeckMet(const struct YgoOpponent *o)
{
    if (o->gba == 2)
    {
        Ygo_PoolNoteMet(sLastPoolDeck);
        return;
    }
    if (!o->gba || sLastGameDeck < 0 || MetLately((unsigned)sLastGameDeck))
        return;
    sRecentDecks[sRecentAt] = sLastGameDeck;
    sRecentAt = (sRecentAt + 1) % RECENT_DECKS;
}

// How likely a deck is to be picked: World Championship 2006's (the most interesting lists) three times as likely as
// the other Game Boy Advance games'; goatformat.com's only for the skilled and named duelists (sGoat), the stronger
// their guide says they are the likelier
enum { GOAT_NO, GOAT_TOO, GOAT_ONLY };
static u8 sGoat;
static unsigned GameWeight(unsigned index)
{
    if (!strcmp(gYgoNpcDecks[index].game, "GOAT"))
        return sGoat != GOAT_NO ? 1 + gYgoNpcDecks[index].strength : 0;
    if (sGoat == GOAT_ONLY)
        return 0;
    return !strcmp(gYgoNpcDecks[index].game, "WC6") ? 3 : 1;
}

// A deck from lo..hi-1 (weakest first) near the target (target < 0: any of them), by the person's own number: the
// nearer the likelier, weighted by game, the ones met lately only when nothing else fits; -1 when there's none
static int PickGameDeck(unsigned lo, unsigned hi, int target, u32 seed)
{
    unsigned i, best = ~0u, window, fresh = 0;
    u32 total = 0, w;
    for (i = lo; i < hi && target >= 0; i++)
    {
        unsigned off = (unsigned)abs((int)gYgoNpcDecks[i].rating - target);
        if (GameWeight(i))
            best = off < best ? off : best;
    }
    window = target < 0 ? ~0u : best + 60 > 90 ? best + 60 : 90;
    for (i = lo; i < hi; i++)
        fresh += GameWeight(i) && (target < 0 || (unsigned)abs((int)gYgoNpcDecks[i].rating - target) <= window) && !MetLately(i);
    for (i = lo; i < hi; i++)
    {
        unsigned off = target < 0 ? 0 : (unsigned)abs((int)gYgoNpcDecks[i].rating - target);
        if (off > window || (fresh && MetLately(i)))
            continue;
        total += (target < 0 ? 1 : window + 1 - off) * GameWeight(i);
    }
    if (!total)
        return -1;
    seed = (seed * 2654435761u) % total;
    for (i = lo; i < hi; i++)
    {
        unsigned off = target < 0 ? 0 : (unsigned)abs((int)gYgoNpcDecks[i].rating - target);
        if (off > window || (fresh && MetLately(i)))
            continue;
        w = (target < 0 ? 1 : window + 1 - off) * GameWeight(i);
        if (seed < w)
            return (int)i;
        seed -= w;
    }
    return -1;
}

// One of the Game Boy Advance games' opponents' decks (ygo_npc_decks.c, weakest first) into o; 0 when it can't make one
static int GameDeckAt(struct YgoOpponent *o, unsigned index)
{
    if (!Ygo_NpcDeckCards(index, o->main, o->extra))
        return 0;
    sLastGameDeck = (s16)index;
    printf("[ygo] %s plays %s's deck from %s (rating %u)\n", o->name, gYgoNpcDecks[index].name,
           gYgoNpcDecks[index].gameName, gYgoNpcDecks[index].rating);
    return 1;
}
// ... from the quarter of them that matches tier (0-3); 0 when there's none
static int GameDeck(struct YgoOpponent *o, unsigned tier, u32 seed)
{
    unsigned n = gYgoNpcDeckCount, lo, hi;
    if (n < 4 || tier > 3)
        return 0;
    lo = n * tier / 4;
    hi = n * (tier + 1) / 4;
    sGoat = tier >= 2 ? GOAT_TOO : GOAT_NO;   // (goatformat.com's decks: the skilled duelists, from the fourth badge)
    {
        int i = PickGameDeck(lo, hi, -1, seed);
        return GameDeckAt(o, i >= 0 ? (unsigned)i : lo + seed % (hi - lo));
    }
}
// One of the generic duelists' decks from YGOPRODeck (ygo_npc_pool.c) into o; 0 when it can't make one
static int PoolDeckAt(struct YgoOpponent *o, int index)
{
    char name[32];
    unsigned rating = 0, year = 0, placing = 0;
    if (index < 0 || !Ygo_PoolCards((unsigned)index, o->main, o->extra))
        return 0;
    Ygo_PoolInfo((unsigned)index, name, sizeof(name), &rating, &year, &placing);
    sLastPoolDeck = index;
    o->gba = 2;
    o->rating = (s16)rating;
    printf("[ygo] %s (standing %u, rating %d) plays the deck \"%s\" (%u, rating %u, %s, placing %u)\n", o->name,
           o->standing, o->elo, name, year, rating,
           (const char *const[]){"casual", "strong", "meta", "tournament"}[Ygo_PoolComp((unsigned)index) & 3], placing);
    return 1;
}
// What a duelist may bring from YGOPRODeck's decks: the ratings between *lo and *hi (their standing's share of the
// decks the GEN allows; below the LEAGUE the badges keep the top back: 512 of 1024 with none, all with seven) and how
// competitive at most. Their own rating (*elo): a point in that span by the badges, and their own number's lean
// (a person a little better or worse than others like them, the same every day).
static int Band(const struct YgoOpponent *o, int *lo, int *hi, int *elo, unsigned *comp)
{
    unsigned st = o->standing < STAND_COUNT ? o->standing : STAND_TRAINER, tier = o->tier > 3 ? 3 : o->tier;
    unsigned gen = MP_YgoGen(), a = sStandings[st].lo, b = sStandings[st].hi, cap = 512 + tier * 171, span;
    u32 own = o->key * 2654435761u;
    int at;
    if (st < STAND_ACE && b > cap)
        b = cap;
    if (a > b)
        a = b;
    span = b - a;
    at = (int)a + (int)(span * (320 + 110 * tier) / 1024) + (int)span * ((int)((own >> 11) % 241) - 120) / 1024;
    at = at < (int)a ? (int)a : at > (int)b ? (int)b : at;
    *lo = Ygo_PoolQuantile(gen, a);
    *hi = Ygo_PoolQuantile(gen, b);
    *elo = Ygo_PoolQuantile(gen, (unsigned)at);
    *comp = sStandings[st].comp;
    return *lo >= 0 && *hi >= 0 && *elo >= 0;
}
// ... a deck near their own rating, inside it
static int PoolDeck(struct YgoOpponent *o, u32 seed)
{
    int lo, hi, elo;
    unsigned comp;
    if (!Band(o, &lo, &hi, &elo, &comp))
        return 0;
    o->elo = (s16)elo;
    return PoolDeckAt(o, Ygo_PoolPickIn(elo, MP_YgoGen(), comp, lo, hi, seed));
}
static int sThemedOnly;   // (the special duelists: their own type always)
static void Fill(struct YgoOpponent *o, const char *name, const char *title, unsigned theme, unsigned tier,
                 unsigned standing, u32 key, u32 seed)
{
    memset(o, 0, sizeof(*o) - sizeof(o->portrait));
    o->tier = (u8)tier;
    o->key = key;
    o->standing = (u8)StandingNow(standing);
    snprintf(o->name, sizeof(o->name), "%s", name);
    snprintf(o->title, sizeof(o->title), "%s", title ? title : "");
    // most duelists bring a deck from the Game Boy Advance games (by how far the player has come); the rest a deck of
    // their own type; the special duelists (tier past 3) always their own
    // (the user, 2026-11-21: every ordinary duelist brings one of YGOPRODeck's decks: ygo_npc_pool.c)
    o->gba = 0;
    if (!sThemedOnly && PoolDeck(o, seed >> 3))
        ;
    else if (!sThemedOnly && (seed >> 13) % 5 < 3 && GameDeck(o, tier, seed >> 3))
        o->gba = 1;
    else
        BuildDeck(o, theme, tier, seed);
    o->theme = (u8)theme;
    o->tier = (u8)tier;
    o->seed = seed;
    o->fit = 1;
    if (o->gba != 2)
        o->rating = (s16)Ygo_DeckRating(o->main);
    // how well they play: by the badges, some more loosely (the special duelists play their best: set after)
    o->level = (u8)(tier >= 2 ? 2 : 1);
    if ((seed >> 17) % 4 == 0 && o->level)
        o->level--;
    o->key = key;
    o->special = YGO_SPECIAL_NONE;
    o->set = YGO_SET_COUNT;
    o->music = MUS_VS_TRAINER;
    o->tcgSpecial = 0xFF;
}

// Their signature card is in their deck (in place of a small monster)
static void PutSignature(struct YgoOpponent *o)
{
    unsigned n = Ygo_DeckCount(o->main, YGO_DECK_MAX), k, have = 0;
    if (!o->signature)
        return;
    for (k = 0; k < n; k++)
        have |= o->main[k] == o->signature;
    for (k = n; !have && k-- > 0;)
        if (Ygo_IsMonster(Ygo_Card(o->main[k])) && Ygo_Card(o->main[k])->level <= 4)
        {
            o->main[k] = o->signature;
            have = 1;
        }
}

// DECK RECIPES: a special duelist's deck once beaten (include/ygo/npc.h). Their deck changes with the day; the recipe is
// the one they bring at their best, the same every time.
unsigned Ygo_SpecialCount(void)
{
    return YGO_SPECIAL_COUNT;
}
int Ygo_SpecialRecipe(unsigned special, uint16_t *main, uint16_t *extra, char *name, size_t size)
{
    static struct
    {
        u8 built;
        u16 main[YGO_DECK_MAX], extra[YGO_FUSION_MAX];
    } sRecipes[YGO_SPECIAL_COUNT];   /* (built once: the screens ask every frame) */
    const struct Duelist *s;
    if (special >= YGO_SPECIAL_COUNT)
        return 0;
    s = &sDuelists[special];
    if (!sRecipes[special].built)
    {
        struct YgoOpponent *o = malloc(sizeof(*o));
        if (!o)
            return 0;
        sThemedOnly = 1;
        Fill(o, s->name ? s->name : "RIVAL", s->title, s->theme, 3, STAND_ACE, 0xFE000000u | special,
             0x5EC1A1u * (special + 1));
        sThemedOnly = 0;
        o->signature = (u16)Signature(s->theme);
        PutSignature(o);
        memcpy(sRecipes[special].main, o->main, sizeof(o->main));
        memcpy(sRecipes[special].extra, o->extra, sizeof(o->extra));
        sRecipes[special].built = 1;
        free(o);
    }
    memcpy(main, sRecipes[special].main, sizeof(sRecipes[special].main));
    memcpy(extra, sRecipes[special].extra, sizeof(sRecipes[special].extra));
    snprintf(name, size, "%s", s->name ? s->name : "RIVAL");
    return 1;
}

// The person in front: 0 they play Pokemon cards (or nothing), PLAY_YGO they duel with Yu-Gi-Oh! (*line: what
// they say; NULL with *refused set: not now), PLAY_BOTH they ask which (o filled for Yu-Gi-Oh!).
int Ygo_FieldPick(struct ObjectEvent *npc, const uint8_t *script, uint32_t h, uint32_t key, int clubStaff,
                  struct YgoOpponent *o, const uint8_t **line, int *refused)
{
    unsigned i;
    *line = NULL;
    *refused = 0;
    if (clubStaff)
    {
        // the CARD CLUB staff play both: their Starter Deck rotates by the day
        static const u8 starters[4] = {YGO_SET_SDY, YGO_SET_SDK, YGO_SET_SDJ, YGO_SET_SDP};
        unsigned set = starters[Tcg_Today() % 4], n = 0, e = 0, k;
        memset(o, 0, sizeof(*o) - sizeof(o->portrait));
        snprintf(o->name, sizeof(o->name), "CARD CLUB");
        for (i = 0; i < gYgoSets[set].count; i++)
            for (k = 0; k < gYgoSets[set].cards[i].copies; k++)
            {
                const struct YgoCard *c = Ygo_Card(gYgoSets[set].cards[i].card);
                if (!Ygo_CardReady(c->id))
                    continue;
                if (Ygo_IsExtra(c) && e < YGO_FUSION_MAX)
                    o->extra[e++] = c->id;
                else if (n < YGO_DECK_MAX)
                    o->main[n++] = c->id;
            }
        o->key = 0xFE000000u | YGO_SPECIAL_CLUB;
        o->special = YGO_SPECIAL_NONE;
        o->level = 1;   /* (a friendly club duelist with the day's Starter Deck: not fitted) */
        o->voice = QUOTE_VOICE_STAFF;
        o->rating = (s16)Ygo_DeckRating(o->main);
        o->set = YGO_SET_COUNT;
        o->music = MUS_VS_TRAINER;
        o->tcgSpecial = 0xFF;
        return PLAY_BOTH;
    }
    for (i = 0; i < YGO_SPECIAL_COUNT; i++)
    {
        const struct Duelist *s = &sDuelists[i];
        unsigned tier;
        if (OBJ_GFX_ID(npc) != s->gfx && (!s->gfx2 || OBJ_GFX_ID(npc) != s->gfx2))
            continue;
        if (s->req == REQ_TRAINER && !FlagGet(TRAINER_FLAG(s->reqArg)))
        {
            *line = s->refuse;
            *refused = 1;
            return PLAY_YGO;
        }
        tier = s->play == PLAY_BOTH ? TierOfBadges() + 1 : 3;
        sThemedOnly = 1;
        Fill(o, s->name ? s->name : OBJ_GFX_ID(npc) == OBJ_EVENT_GFX_RIVAL_MAY_NORMAL ? "MAY" : "BRENDAN", s->title,
             s->theme, tier, STAND_ACE, 0xFE000000u | i, h ^ (Tcg_Today() * 0x9e3779b9u));
        sThemedOnly = 0;
        o->special = (u8)i;
        o->level = 3;
        o->signature = (u16)Signature(s->theme);
        o->music = s->music;
        // the ELITE FOUR played the Pokemon cards before: their promo cards come with a win (tcg/npc.h order)
        if (i <= YGO_SPECIAL_DRAKE)
            o->tcgSpecial = (u8)(8 + i);
        PutSignature(o);
        o->voice = QUOTE_VOICE_REFINED;
        snprintf(o->quoteWho, sizeof(o->quoteWho), "%s", s->name ? s->name : "RIVAL");
        *line = CardTable_Hello(o->key, QUOTE_GAME_YGO, o->quoteWho, o->voice, s->accept, h);
        return s->play;
    }
    {
        u8 theme = 0xFF, standing = STAND_TRAINER;
        const char *label = NULL;
        int team = 0;
        if (npc->trainerType != TRAINER_TYPE_NONE && script && script[0] == 0x5C)   // trainerbattle: a real trainer
        {
            u16 id = script[2] | script[3] << 8;
            u8 cls = Tcg_IsTrainerId(id) ? gTrainers[id].trainerClass : 0;
            for (i = 0; i < ARRAY_COUNT(sClasses); i++)
                if (sClasses[i].trainerClass == cls)
                {
                    theme = ThemeAt(sClasses[i].theme, sClasses[i].alt, h);
                    standing = sClasses[i].standing;
                }
            if (theme == 0xFF)
                return 0;
            team = cls == TRAINER_CLASS_TEAM_AQUA || cls == TRAINER_CLASS_TEAM_MAGMA || cls == TRAINER_CLASS_AQUA_ADMIN ||
                   cls == TRAINER_CLASS_MAGMA_ADMIN;
            if (!FlagGet(TRAINER_FLAG(id)))
            {
                *line = sText_YgoBattleFirst;
                *refused = 1;
                return PLAY_YGO;
            }
            {
                char clsName[16], who[16], name[28];
                MP_GameToAscii(clsName, gTrainerClassNames[cls], sizeof(clsName));
                MP_GameToAscii(who, gTrainers[id].trainerName, sizeof(who));
                snprintf(name, sizeof(name), "%s %s", clsName, who);
                Fill(o, name, NULL, theme, TierOfBadges(), standing, key, h);
            }
        }
        else
        {
            for (i = 0; i < ARRAY_COUNT(sTownsfolk); i++)
                if (sTownsfolk[i].gfx == Tcg_LookGfx(npc))
                {
                    theme = ThemeAt(sTownsfolk[i].theme, sTownsfolk[i].alt, h);
                    standing = sTownsfolk[i].standing;
                    label = sTownsfolk[i].label;
                    team = Tcg_LookGfx(npc) == OBJ_EVENT_GFX_AQUA_MEMBER_M || Tcg_LookGfx(npc) == OBJ_EVENT_GFX_AQUA_MEMBER_F ||
                           Tcg_LookGfx(npc) == OBJ_EVENT_GFX_MAGMA_MEMBER_M || Tcg_LookGfx(npc) == OBJ_EVENT_GFX_MAGMA_MEMBER_F;
                }
            if (theme == 0xFF)
                return 0;
            if (h % 100 >= 70)   // most of them play
            {
                *line = (h >> 8) % 2 ? sText_DuelNo1 : sText_DuelNo2;
                *refused = 1;
                return PLAY_YGO;
            }
            Fill(o, label, NULL, theme, TierOfBadges(), standing, key, h);
        }
        if (team)
            o->music = MUS_VS_AQUA_MAGMA;
        o->voice = team ? QUOTE_VOICE_TEAM : Tcg_VoiceOf(Tcg_LookGfx(npc));
        // how old they are: a child misplays far more, and comically when far from good (Ygo_Misplays)
        o->age = Tcg_VoiceOf(Tcg_LookGfx(npc)) == QUOTE_VOICE_KID ? YGO_AGE_CHILD
               : label && !strcmp(label, "TEEN") ? YGO_AGE_TEEN : YGO_AGE_ADULT;
        o->quoteWho[0] = 0;
        *line = CardTable_Hello(o->key, QUOTE_GAME_YGO, NULL, o->voice, team ? sText_DuelTeam : sDuelLines[(h >> 20) % 3], h);
        return PLAY_YGO;
    }
}

// ---------------------------------------------------------------------------
// The CARD CLUB's Yu-Gi-Oh! duelist (OLDALE and MAUVILLE POKeMON MARTS, LILYCOVE DEPARTMENT STORE)

// VAR_RESULT: 0 no Yu-Gi-Oh! deck yet, 1 has one
void Ygo_ClubState(void)
{
    gSpecialVar_Result = Ygo_GetCollection()->optedIn;
}
// VAR_RESULT: 1 the CARD CASE is in the BAG
void Ygo_ClubHasCase(void)
{
    gSpecialVar_Result = CheckBagHasItem(ITEM_CARD_CASE, 1);
}
void Ygo_ClubChooseDeck(void)
{
    YgoUi_OpenStarter();
}
void Ygo_ClubTeachingDuel(void)
{
    YgoUi_OpenLesson();
}
void Ygo_ClubCounter(void)
{
    YgoUi_OpenCounter();
}
// The GAME CENTER CARD SHOP's Yu-Gi-Oh! clerk (RUSTBORO CITY, 2F)
void Ygo_GameCenterShop(void)
{
    YgoUi_OpenGameCenter();
}
// Test harness: build a theme's deck and print it (mp ygo deck THEME TIER)
void Ygo_FieldDebugDeck(unsigned theme, unsigned tier)
{
    struct YgoOpponent *o = malloc(sizeof(*o));
    char reason[160];
    unsigned i, n;
    if (!o || theme >= TH_COUNT)
    {
        free(o);
        return;
    }
    Fill(o, "TEST", NULL, theme, tier, STAND_TRAINER, 1, 12345 + theme * 7919u);
    n = Ygo_DeckCount(o->main, YGO_DECK_MAX);
    printf("[ygo] theme %u tier %u: %u cards + %u, valid %d, signature %s\n", theme, tier, n,
           Ygo_DeckCount(o->extra, YGO_FUSION_MAX), Ygo_DeckValidate(o->main, o->extra, NULL, reason, sizeof(reason)),
           Signature(theme) ? Ygo_Card(Signature(theme))->name : "-");
    for (i = 0; i < n; i++)
        printf("[ygo]   %s\n", Ygo_Card(o->main[i])->name);
    free(o);
}

static void FitOnce(struct YgoOpponent *o, int target);
// Test harness (mp ygo standing STANDING TIER): what a duelist of that standing brings, 24 people, scaling off, and
// with the player at their best and their weakest
void Ygo_FieldDebugStanding(unsigned standing, unsigned tier)
{
    struct YgoOpponent *o = malloc(sizeof(*o));
    unsigned k, worst = 0, past = 0;
    int lo, hi, elo, top = 0;
    unsigned comp;
    if (!o || standing >= STAND_COUNT)
    {
        free(o);
        return;
    }
    for (k = 0; k < 24; k++)
    {
        unsigned c;
        memset(o, 0, sizeof(*o) - sizeof(o->portrait));
        o->standing = (u8)standing, o->tier = (u8)tier, o->key = 0x1234567u * (k + 1), o->seed = 0x9e3779b9u * (k + 7);
        o->special = YGO_SPECIAL_NONE;
        if (!Band(o, &lo, &hi, &elo, &comp) || !PoolDeck(o, o->seed >> 3))
            continue;
        c = Ygo_PoolComp((unsigned)sLastPoolDeck);
        worst = c > worst ? c : worst;
        top = o->rating > top ? o->rating : top;
        past += o->rating < lo || o->rating > hi;
        // against the strongest and the weakest player CARD SCALING could ask for
        o->gba = 2, o->fit = 1;
        FitOnce(o, 4000);
        c = Ygo_PoolComp((unsigned)sLastPoolDeck);
        worst = c > worst ? c : worst;
        top = o->rating > top ? o->rating : top;
        past += o->rating < lo || o->rating > hi;
        FitOnce(o, 0);
        past += o->rating < lo || o->rating > hi;
    }
    printf("[ygo] standing %u tier %u: ratings %d-%d, competitive up to %u; brought up to %d, competitive up to %u, "
           "%u outside\n", standing, tier, lo, hi, comp, top, worst, past);
    free(o);
}

// ---------------------------------------------------------------------------
// CARD SCALING (include/ygo/npc.h)
int Ygo_OpponentFavor(const struct YgoOpponent *o)
{
    // ordinary duelists about 55% for the player, the special ones 45%
    return o->special != YGO_SPECIAL_NONE ? -35 : 35;
}
static unsigned Off(int a, int b) { return (unsigned)(a > b ? a - b : b - a); }
static void FitOnce(struct YgoOpponent *o, int target)
{
    unsigned cap = o->tier > 3 ? 3 : o->tier, i, best = ~0u;
    if (o->gba == 2)
    {
        // a YGOPRODeck deck halfway between their own rating and the player's (CARD SCALING's target), inside what
        // their standing allows: a townsperson stays a townsperson against the best player
        int lo, hi, elo;
        unsigned comp;
        if (Band(o, &lo, &hi, &elo, &comp))
        {
            int want = elo + (target - elo) / 2;
            want = want < lo ? lo : want > hi ? hi : want;
            o->elo = (s16)elo;
            if (PoolDeckAt(o, Ygo_PoolPickIn(want, MP_YgoGen(), comp, lo, hi, o->seed >> 3)))
                return;
        }
    }
    if (o->gba == 1 && gYgoNpcDeckCount >= 4)
    {
        // the Game Boy decks up to the badges' quarter (weakest first), the closest; among those nearly as close,
        // their own pick
        unsigned hi = gYgoNpcDeckCount * (cap + 1) / 4;
        int at;
        sGoat = o->level >= 2 ? GOAT_TOO : GOAT_NO;   // (the skilled ones may bring a goatformat.com deck)
        at = PickGameDeck(0, hi, target, o->seed >> 3);
        i = at >= 0 ? (unsigned)at : hi;
        if (i < hi && GameDeckAt(o, i))
        {
            o->rating = (s16)Ygo_DeckRating(o->main);
            return;
        }
    }
    if (o->special != YGO_SPECIAL_NONE && (o->seed >> 5) % 3 == 0 && gYgoNpcDeckCount >= 4)
    {
        // a third of the named duelists play one of goatformat.com's decks (the one their number picks near the target)
        int at;
        sGoat = GOAT_ONLY;
        at = PickGameDeck(0, gYgoNpcDeckCount, target, o->seed >> 7);
        sGoat = GOAT_NO;
        if (at >= 0 && GameDeckAt(o, (unsigned)at))
        {
            o->gba = 1;
            o->rating = (s16)Ygo_DeckRating(o->main);
            return;
        }
    }
    {
        // their own type's deck, at the tier that comes closest
        unsigned t, bestTier = 0;
        static struct YgoOpponent sTry;
        for (t = 0; t <= cap; t++)
        {
            unsigned off;
            BuildDeck(&sTry, o->theme, t, o->seed);
            off = Off(Ygo_DeckRating(sTry.main), target);
            if (off < best || t == 0)
                best = off, bestTier = t;
        }
        BuildDeck(o, o->theme, bestTier, o->seed);
        o->rating = (s16)Ygo_DeckRating(o->main);
    }
}
void Ygo_FitOpponent(struct YgoOpponent *o, int mine)
{
    if (!gMpConfig.cardScaling || !o->fit)
        return;
    for (;;)
    {
        int target = mine + Ygo_PlayerRating() - Ygo_OpponentFavor(o) - Ygo_LevelRating(o->level);
        FitOnce(o, target);
        // the decks they may bring this early fall well short: they play more carefully instead (SKILLED at most)
        if (o->special != YGO_SPECIAL_NONE || o->level >= 2 || target - o->rating <= 50)
            break;
        o->level++;
    }
}
