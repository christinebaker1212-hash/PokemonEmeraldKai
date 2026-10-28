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
    return c && c->frame != YGO_TOKEN && Ygo_CardReady(c->id) && Ygo_CopyLimit(c->id);
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
    if (!Playable(c) || c->frame == YGO_FUSION)
        return 0;
    limit = Ygo_CopyLimit(id);
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
        if (!Playable(c) || !Ygo_IsMonster(c) || c->frame == YGO_FUSION || c->frame == YGO_RITUAL)
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
        if (c->frame != YGO_FUSION || !Ygo_CardReady(id) || !Ygo_CopyLimit(id) || !(f = Ygo_FusionOf(id)))
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
// Their signature card: the strongest working monster of the type outside the Starter Decks
static unsigned Signature(unsigned theme)
{
    unsigned id, best = 0;
    int bestScore = -1;
    for (id = 1; id <= YGO_CARD_COUNT; id++)
    {
        const struct YgoCard *c = Ygo_Card(id);
        int s;
        if (!Playable(c) || !Ygo_IsMonster(c) || !InTheme(c, theme) || c->frame == YGO_RITUAL || c->frame == YGO_FUSION)
            continue;
        if (c->sets & ((1u << YGO_SET_SDY) | (1u << YGO_SET_SDK) | (1u << YGO_SET_SDJ) | (1u << YGO_SET_SDP)))
            continue;
        s = Score(c) + 120 * c->rarity;
        if (s > bestScore)
            bestScore = s, best = id;
    }
    return best;
}

// ---------------------------------------------------------------------------
// Who plays

static u8 Badges(void)
{
    u8 i, n = 0;
    for (i = 0; i < NUM_BADGES; i++)
        n += FlagGet(FLAG_BADGE01_GET + i) != 0;
    return n;
}
static unsigned TierOfBadges(void)
{
    u8 b = Badges();
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

// Trainer classes that duel with Yu-Gi-Oh! cards, and their themes
static const struct
{
    u8 trainerClass, theme, alt;
} sClasses[] = {
    {TRAINER_CLASS_PSYCHIC, TH_SPELLCASTER, TH_FAIRY},
    {TRAINER_CLASS_HEX_MANIAC, TH_ZOMBIE, TH_FIEND},
    {TRAINER_CLASS_NINJA_BOY, TH_INSECT, TH_WARRIOR},
    {TRAINER_CLASS_BLACK_BELT, TH_WARRIOR, TH_BEAST_WARRIOR},
    {TRAINER_CLASS_BATTLE_GIRL, TH_HARPIE, TH_WARRIOR},
    {TRAINER_CLASS_COOLTRAINER, TH_DRAGON, TH_SPELLCASTER},
    {TRAINER_CLASS_COOLTRAINER_2, TH_DRAGON, TH_HARPIE},
    {TRAINER_CLASS_RUIN_MANIAC, TH_ROCK, TH_ZOMBIE},
    {TRAINER_CLASS_KINDLER, TH_PYRO, TH_DINO},
    {TRAINER_CLASS_COLLECTOR, TH_TOON, TH_DRAGON},
    {TRAINER_CLASS_EXPERT, TH_MACHINE, TH_BEAST_WARRIOR},
    {TRAINER_CLASS_GUITARIST, TH_FIEND, TH_WARRIOR},
    {TRAINER_CLASS_TEAM_AQUA, TH_SEA, TH_AQUA},
    {TRAINER_CLASS_AQUA_ADMIN, TH_SEA, TH_AQUA},
    {TRAINER_CLASS_TEAM_MAGMA, TH_DINO, TH_PYRO},
    {TRAINER_CLASS_MAGMA_ADMIN, TH_DINO, TH_PYRO},
};
// Townsfolk who duel with Yu-Gi-Oh! cards: the teens and young adults (and the ones a class above dresses as)
static const struct
{
    u16 gfx;
    u8 theme, alt;
    const char *label;
} sTownsfolk[] = {
    {OBJ_EVENT_GFX_BOY_3, TH_WARRIOR, TH_DRAGON, "TEEN"},
    {OBJ_EVENT_GFX_GIRL_2, TH_FAIRY, TH_HARPIE, "TEEN"},
    {OBJ_EVENT_GFX_MAN_2, TH_BEAST, TH_WARRIOR, "YOUNG MAN"},
    {OBJ_EVENT_GFX_MAN_3, TH_WARRIOR, TH_DRAGON, "YOUNG MAN"},
    {OBJ_EVENT_GFX_MAN_4, TH_MACHINE, TH_FIEND, "YOUNG MAN"},
    {OBJ_EVENT_GFX_MAN_5, TH_SPELLCASTER, TH_WARRIOR, "YOUNG MAN"},
    {OBJ_EVENT_GFX_WOMAN_2, TH_HARPIE, TH_FAIRY, "WOMAN"},
    {OBJ_EVENT_GFX_WOMAN_3, TH_SPELLCASTER, TH_PLANT, "WOMAN"},
    {OBJ_EVENT_GFX_WOMAN_5, TH_FAIRY, TH_AQUA, "WOMAN"},
    {OBJ_EVENT_GFX_PSYCHIC_M, TH_SPELLCASTER, TH_FIEND, "PSYCHIC"},
    {OBJ_EVENT_GFX_HEX_MANIAC, TH_ZOMBIE, TH_FIEND, "HEX MANIAC"},
    {OBJ_EVENT_GFX_NINJA_BOY, TH_INSECT, TH_WARRIOR, "NINJA BOY"},
    {OBJ_EVENT_GFX_BLACK_BELT, TH_WARRIOR, TH_BEAST_WARRIOR, "BLACK BELT"},
    {OBJ_EVENT_GFX_EXPERT_M, TH_MACHINE, TH_BEAST_WARRIOR, "EXPERT"},
    {OBJ_EVENT_GFX_EXPERT_F, TH_SPELLCASTER, TH_FAIRY, "EXPERT"},
    {OBJ_EVENT_GFX_AQUA_MEMBER_M, TH_SEA, TH_AQUA, "TEAM AQUA GRUNT"},
    {OBJ_EVENT_GFX_AQUA_MEMBER_F, TH_SEA, TH_AQUA, "TEAM AQUA GRUNT"},
    {OBJ_EVENT_GFX_MAGMA_MEMBER_M, TH_DINO, TH_PYRO, "TEAM MAGMA GRUNT"},
    {OBJ_EVENT_GFX_MAGMA_MEMBER_F, TH_DINO, TH_PYRO, "TEAM MAGMA GRUNT"},
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

static void Fill(struct YgoOpponent *o, const char *name, const char *title, unsigned theme, unsigned tier, u32 key,
                 u32 seed)
{
    memset(o, 0, sizeof(*o) - sizeof(o->portrait));
    snprintf(o->name, sizeof(o->name), "%s", name);
    snprintf(o->title, sizeof(o->title), "%s", title ? title : "");
    BuildDeck(o, theme, tier, seed);
    o->key = key;
    o->special = YGO_SPECIAL_NONE;
    o->set = YGO_SET_COUNT;
    o->music = MUS_VS_TRAINER;
    o->tcgSpecial = 0xFF;
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
                if (c->frame == YGO_FUSION && e < YGO_FUSION_MAX)
                    o->extra[e++] = c->id;
                else if (n < YGO_DECK_MAX)
                    o->main[n++] = c->id;
            }
        o->key = 0xFE000000u | YGO_SPECIAL_CLUB;
        o->special = YGO_SPECIAL_NONE;
        o->set = YGO_SET_COUNT;
        o->music = MUS_VS_TRAINER;
        o->tcgSpecial = 0xFF;
        return PLAY_BOTH;
    }
    for (i = 0; i < YGO_SPECIAL_COUNT; i++)
    {
        const struct Duelist *s = &sDuelists[i];
        unsigned tier;
        if (npc->graphicsId != s->gfx && (!s->gfx2 || npc->graphicsId != s->gfx2))
            continue;
        if (s->req == REQ_TRAINER && !FlagGet(TRAINER_FLAGS_START + s->reqArg))
        {
            *line = s->refuse;
            *refused = 1;
            return PLAY_YGO;
        }
        tier = s->play == PLAY_BOTH ? TierOfBadges() + 1 : 3;
        Fill(o, s->name ? s->name : npc->graphicsId == OBJ_EVENT_GFX_RIVAL_MAY_NORMAL ? "MAY" : "BRENDAN", s->title,
             s->theme, tier, 0xFE000000u | i, h ^ (Tcg_Today() * 0x9e3779b9u));
        o->special = (u8)i;
        o->signature = (u16)Signature(s->theme);
        o->music = s->music;
        // the ELITE FOUR played the Pokemon cards before: their promo cards come with a win (tcg/npc.h order)
        if (i <= YGO_SPECIAL_DRAKE)
            o->tcgSpecial = (u8)(8 + i);
        // their signature card is in their deck (in place of a small monster)
        if (o->signature)
        {
            unsigned n = Ygo_DeckCount(o->main, YGO_DECK_MAX), k, have = 0;
            for (k = 0; k < n; k++)
                have |= o->main[k] == o->signature;
            for (k = n; !have && k-- > 0;)
                if (Ygo_IsMonster(Ygo_Card(o->main[k])) && Ygo_Card(o->main[k])->level <= 4)
                {
                    o->main[k] = o->signature;
                    have = 1;
                }
        }
        *line = s->accept;
        return s->play;
    }
    {
        u8 theme = 0xFF;
        const char *label = NULL;
        int team = 0;
        if (npc->trainerType != TRAINER_TYPE_NONE && script && script[0] == 0x5C)   // trainerbattle: a real trainer
        {
            u16 id = script[2] | script[3] << 8;
            u8 cls = id < TRAINERS_COUNT ? gTrainers[id].trainerClass : 0;
            for (i = 0; i < ARRAY_COUNT(sClasses); i++)
                if (sClasses[i].trainerClass == cls)
                    theme = ThemeAt(sClasses[i].theme, sClasses[i].alt, h);
            if (theme == 0xFF)
                return 0;
            team = cls == TRAINER_CLASS_TEAM_AQUA || cls == TRAINER_CLASS_TEAM_MAGMA || cls == TRAINER_CLASS_AQUA_ADMIN ||
                   cls == TRAINER_CLASS_MAGMA_ADMIN;
            if (!FlagGet(TRAINER_FLAGS_START + id))
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
                Fill(o, name, NULL, theme, TierOfBadges(), key, h);
            }
        }
        else
        {
            for (i = 0; i < ARRAY_COUNT(sTownsfolk); i++)
                if (sTownsfolk[i].gfx == npc->graphicsId)
                {
                    theme = ThemeAt(sTownsfolk[i].theme, sTownsfolk[i].alt, h);
                    label = sTownsfolk[i].label;
                    team = npc->graphicsId == OBJ_EVENT_GFX_AQUA_MEMBER_M || npc->graphicsId == OBJ_EVENT_GFX_AQUA_MEMBER_F ||
                           npc->graphicsId == OBJ_EVENT_GFX_MAGMA_MEMBER_M || npc->graphicsId == OBJ_EVENT_GFX_MAGMA_MEMBER_F;
                }
            if (theme == 0xFF)
                return 0;
            if (h % 100 >= 70)   // most of them play
            {
                *line = (h >> 8) % 2 ? sText_DuelNo1 : sText_DuelNo2;
                *refused = 1;
                return PLAY_YGO;
            }
            Fill(o, label, NULL, theme, TierOfBadges(), key, h);
        }
        if (team)
            o->music = MUS_VS_AQUA_MAGMA;
        *line = team ? sText_DuelTeam : sDuelLines[(h >> 20) % 3];
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
    Fill(o, "TEST", NULL, theme, tier, 1, 12345);
    n = Ygo_DeckCount(o->main, YGO_DECK_MAX);
    printf("[ygo] theme %u tier %u: %u cards + %u, valid %d, signature %s\n", theme, tier, n,
           Ygo_DeckCount(o->extra, YGO_FUSION_MAX), Ygo_DeckValidate(o->main, o->extra, NULL, reason, sizeof(reason)),
           Signature(theme) ? Ygo_Card(Signature(theme))->name : "-");
    for (i = 0; i < n; i++)
        printf("[ygo]   %s\n", Ygo_Card(o->main[i])->name);
    free(o);
}
