/* Helpers most Yu-Gi-Oh! card effects share: filters, collecting and targeting, searching, discarding, generic Equip
 * and Field Spells, Fusion and Ritual Summons. */
#include <string.h>
#include "ygo_fx.h"

/* ---- filters */
int F_Any(const struct YgoState *s, unsigned o) { (void)s; (void)o; return 1; }
int F_Monster(const struct YgoState *s, unsigned o) { return s->objs[o].location == YGO_LOC_MZONE; }
int F_FaceUp(const struct YgoState *s, unsigned o) { return Ygo_OnField(s, o) && s->objs[o].faceUp; }
int F_FaceUpMonster(const struct YgoState *s, unsigned o) { return Ygo_FaceUpMonster(s, o); }
int F_FaceDownMonster(const struct YgoState *s, unsigned o)
{
    return s->objs[o].location == YGO_LOC_MZONE && !s->objs[o].faceUp;
}
int F_AttackPos(const struct YgoState *s, unsigned o) { return Ygo_FaceUpMonster(s, o) && s->objs[o].attackPos; }
int F_DefensePos(const struct YgoState *s, unsigned o)
{
    return s->objs[o].location == YGO_LOC_MZONE && !s->objs[o].attackPos;
}
int F_SpellTrap(const struct YgoState *s, unsigned o) { return s->objs[o].location == YGO_LOC_SZONE; }
int F_Spell(const struct YgoState *s, unsigned o)
{
    const struct YgoCard *c = Ygo_ObjCard(s, o);
    return c && c->frame == YGO_SPELL;
}
int F_Trap(const struct YgoState *s, unsigned o)
{
    const struct YgoCard *c = Ygo_ObjCard(s, o);
    return c && c->frame == YGO_TRAP;
}
int F_MonsterCard(const struct YgoState *s, unsigned o)
{
    const struct YgoCard *c = Ygo_ObjCard(s, o);
    return c && c->frame <= YGO_RITUAL;
}
int F_FaceUpTrap(const struct YgoState *s, unsigned o) { return F_SpellTrap(s, o) && s->objs[o].faceUp && F_Trap(s, o); }
int F_FaceUpSpell(const struct YgoState *s, unsigned o)
{
    return F_SpellTrap(s, o) && s->objs[o].faceUp && F_Spell(s, o);
}
int Fx_Name(const struct YgoState *s, unsigned o, unsigned card) { return s->objs[o].card == card; }

/* ---- collecting */
unsigned Fx_Collect(const struct YgoState *s, unsigned player, unsigned locations,
                    int (*filter)(const struct YgoState *, unsigned), uint8_t *out)
{
    static const uint8_t locs[7] = {YGO_LOC_HAND, YGO_LOC_MZONE, YGO_LOC_SZONE, YGO_LOC_GRAVE, YGO_LOC_DECK,
                                    YGO_LOC_BANISHED, YGO_LOC_EXTRA};
    unsigned p, l, i, n = 0, k;
    uint8_t list[YGO_OBJS];
    for (p = 0; p < 2; p++)
    {
        if (player != 2 && p != player)
            continue;
        for (l = 0; l < 7; l++)
        {
            if (!(locations & (1u << l)))
                continue;
            k = Ygo_List(s, p, locs[l], list);
            for (i = 0; i < k; i++)
                if (!filter || filter(s, list[i]))
                    out[n++] = list[i];
        }
    }
    return n;
}

unsigned Fx_Targets(const struct YgoState *s, unsigned asker, unsigned player, unsigned locations,
                    int (*filter)(const struct YgoState *, unsigned), uint8_t *out)
{
    uint8_t list[YGO_OBJS];
    unsigned n = Fx_Collect(s, player, locations, filter, list), i, k = 0;
    for (i = 0; i < n; i++)
        if (!Ygo_OnField(s, list[i]) || Ygo_CanTarget(s, list[i], asker))
            out[k++] = list[i];
    return k;
}

int Fx_HasTarget(const struct YgoState *s, unsigned asker, unsigned player, unsigned locations,
                 int (*filter)(const struct YgoState *, unsigned))
{
    uint8_t list[YGO_OBJS];
    return Fx_Targets(s, asker, player, locations, filter, list) > 0;
}

void Fx_Target(struct YgoCtx *c, unsigned player, unsigned locations, int (*filter)(const struct YgoState *, unsigned),
               unsigned count, unsigned prompt)
{
    uint8_t list[YGO_OBJS], picks[8];
    unsigned n = Fx_Targets(S, ME, player, locations, filter, list), k, i;
    if (count > 8)
        count = 8;
    /* never the card itself */
    for (i = k = 0; i < n; i++)
        if (list[i] != c->obj)
            list[k++] = list[i];
    k = Ygo_AskCards(S, ME, prompt, CARD, list, k, count, count, picks);
    for (i = 0; i < k; i++)
    {
        c->link->targetMoves[c->link->targetCount] = S->objs[picks[i]].moves;
        c->link->targets[c->link->targetCount++] = picks[i];
    }
}

int Fx_TargetOk(struct YgoCtx *c, unsigned i, int (*filter)(const struct YgoState *, unsigned))
{
    unsigned o;
    if (!c->link || i >= c->link->targetCount)
        return 0;
    o = c->link->targets[i];
    if (o == NO_OBJ)
        return 0;
    /* it must still be where it was targeted (a card that moves is a new card) */
    if (S->objs[o].moves != c->link->targetMoves[i])
        return 0;
    return !filter || filter(S, o);
}

unsigned Fx_DestroyAll(struct YgoState *s, unsigned player, unsigned locations,
                       int (*filter)(const struct YgoState *, unsigned), unsigned reason)
{
    uint8_t list[YGO_OBJS];
    unsigned n = Fx_Collect(s, player, locations, filter, list), i, k = 0;
    for (i = 0; i < n; i++)
        k += Ygo_Destroy(s, list[i], reason);
    return k;
}

unsigned Fx_Search(struct YgoState *s, unsigned player, int (*filter)(const struct YgoState *, unsigned), uint16_t card)
{
    uint8_t list[YGO_OBJS], pick;
    unsigned n = Fx_Collect(s, player, YL_DECK, filter, list);
    if (!n)
        return NO_OBJ;
    Ygo_AskCards(s, player, YGO_P_ADD_TO_HAND, card, list, n, 1, 1, &pick);
    Ygo_ToHand(s, pick, YGO_REASON_EFFECT);
    Ygo_Reveal(s, pick);
    Ygo_ShuffleDeck(s, player);
    return pick;
}

unsigned Fx_Discard(struct YgoState *s, unsigned player, unsigned count, int random, unsigned reason)
{
    uint8_t hand[YGO_HAND_MAX], picks[YGO_HAND_MAX];
    unsigned n = Ygo_List(s, player, YGO_LOC_HAND, hand), i, k;
    if (count > n)
        count = n;
    if (!count)
        return 0;
    if (random)
    {
        for (i = 0; i < count; i++)
        {
            unsigned j = i + Ygo_Rand(s, n - i);
            uint8_t t = hand[i];
            hand[i] = hand[j];
            hand[j] = t;
            picks[i] = hand[i];
        }
        k = count;
    }
    else
        k = Ygo_AskCards(s, player, YGO_P_DISCARD, 0, hand, n, count, count, picks);
    for (i = 0; i < k; i++)
        Ygo_Discard(s, picks[i], reason);
    return k;
}

int Fx_OnFieldFaceUp(const struct YgoState *s, unsigned player, unsigned card)
{
    unsigned o;
    for (o = 0; o < s->objCount; o++)
        if (s->objs[o].card == card && Ygo_OnField(s, o) && s->objs[o].faceUp &&
            (player == 2 || s->objs[o].controller == player))
            return 1;
    return 0;
}

unsigned Fx_EquippedTo(const struct YgoState *s, unsigned equip)
{
    unsigned m = s->objs[equip].equipTo;
    if (!m || !Ygo_OnField(s, equip) || !Ygo_FaceUpMonster(s, m - 1))
        return NO_OBJ;
    return m - 1;
}

unsigned Fx_EquipCount(const struct YgoState *s, unsigned monster)
{
    unsigned o, n = 0;
    for (o = 0; o < s->objCount; o++)
        if (s->objs[o].equipTo == monster + 1 && Ygo_OnField(s, o))
            n++;
    return n;
}

int Fx_CanPay(const struct YgoState *s, unsigned player, int lp) { return s->players[player].lp >= lp; }
void Fx_Pay(struct YgoState *s, unsigned player, int lp)
{
    s->players[player].lp -= lp;
    Ygo_Event(s, YGO_EV_DAMAGE, player, NO_OBJ, NO_OBJ, lp, 2);
}

/* A monster card (from the hand, or a monster on the field) placed in player's Spell & Trap Zone as an Equip Card
 * on monster (Cocoon of Evolution, Relinquished) */
void Fx_PlaceAsEquip(struct YgoState *s, unsigned o, unsigned player, unsigned monster)
{
    unsigned i;
    int zone = -1;
    for (i = 0; i < YGO_ZONES; i++)
        if (!s->players[player].spells[i])
        {
            zone = (int)i;
            break;
        }
    if (zone < 0 || !Ygo_FaceUpMonster(s, monster))
        return;
    /* out of where it is, into the zone, face up */
    if (s->objs[o].location == YGO_LOC_MZONE)
        s->players[s->objs[o].controller].monsters[s->objs[o].seq] = 0;
    else if (s->objs[o].location == YGO_LOC_HAND)
    {
        struct YgoPlayer *p = &s->players[s->objs[o].owner];
        for (i = 0; i < p->handCount; i++)
            if (p->hand[i] == o)
            {
                for (; i + 1 < p->handCount; i++)
                    p->hand[i] = p->hand[i + 1];
                p->handCount--;
                break;
            }
    }
    else
        return;
    s->objs[o].moves++;
    s->objs[o].location = YGO_LOC_SZONE;
    s->objs[o].seq = zone;
    s->objs[o].controller = player;
    s->objs[o].faceUp = 1;
    s->objs[o].attackPos = 0;
    s->objs[o].atkMod = s->objs[o].defMod = s->objs[o].atkTurn = s->objs[o].defTurn = 0;
    s->objs[o].enteredTurn = s->turn;
    s->players[player].spells[zone] = o + 1;
    Ygo_Equip(s, o, monster);
}

/* ---- Equip Spells that only raise (or lower) stats, for one race or attribute */
static const struct
{
    uint16_t card;
    uint8_t race, attribute;   /* 0: any */
    int16_t atk, def;
} sEquips[] = {
    {YGO_CARD_LEGENDARY_SWORD, YGO_WARRIOR, 0, 300, 300},
    {YGO_CARD_BEAST_FANGS, YGO_BEAST, 0, 300, 300},
    {YGO_CARD_VIOLET_CRYSTAL, YGO_ZOMBIE, 0, 300, 300},
    {YGO_CARD_BOOK_OF_SECRET_ARTS, YGO_SPELLCASTER, 0, 300, 300},
    {YGO_CARD_POWER_OF_KAISHIN, YGO_AQUA, 0, 300, 300},
    {YGO_CARD_DARK_ENERGY, YGO_FIEND, 0, 300, 300},
    {YGO_CARD_LASER_CANNON_ARMOR, YGO_INSECT, 0, 300, 300},
    {YGO_CARD_VILE_GERMS, YGO_PLANT, 0, 300, 300},
    {YGO_CARD_SILVER_BOW_AND_ARROW, YGO_FAIRY, 0, 300, 300},
    {YGO_CARD_DRAGON_TREASURE, YGO_DRAGON, 0, 300, 300},
    {YGO_CARD_ELECTRO_WHIP, YGO_THUNDER, 0, 300, 300},
    {YGO_CARD_MYSTICAL_MOON, YGO_BEAST_WARRIOR, 0, 300, 300},
    {YGO_CARD_MACHINE_CONVERSION_FACTORY, YGO_MACHINE, 0, 300, 300},
    {YGO_CARD_RAISE_BODY_HEAT, YGO_DINOSAUR, 0, 300, 300},
    {YGO_CARD_FOLLOW_WIND, YGO_WINGED_BEAST, 0, 300, 300},
    {YGO_CARD_SWORD_OF_DARK_DESTRUCTION, 0, YGO_DARK, 400, -200},
    {YGO_CARD_INVIGORATION, 0, YGO_EARTH, 400, -200},
    {YGO_CARD_SWORD_OF_DEEP_SEATED, 0, 0, 500, 500},
    {YGO_CARD_MALEVOLENT_NUZZLER, 0, 0, 700, 0},
    {YGO_CARD_BLACK_PENDANT, 0, 0, 500, 0},
    {YGO_CARD_STIM_PACK, 0, 0, 700, 0},
    {YGO_CARD_RING_OF_MAGNETISM, 0, 0, -500, -500},
    {YGO_CARD_GERM_INFECTION, 0, 0, 0, 0},
    {YGO_CARD_PARALYZING_POTION, 0, 0, 0, 0},
};

static int EquipIndex(unsigned card)
{
    unsigned i;
    for (i = 0; i < sizeof(sEquips) / sizeof(sEquips[0]); i++)
        if (sEquips[i].card == card)
            return (int)i;
    return -1;
}

/* Can this Equip Spell go on that monster? (nonmachine: Germ Infection, Paralyzing Potion; own: Ring of Magnetism) */
static int EquipFits(const struct YgoState *s, unsigned card, unsigned player, unsigned o)
{
    int i = EquipIndex(card);
    const struct YgoCard *m = Ygo_ObjCard(s, o);
    if (i < 0 || !Ygo_FaceUpMonster(s, o) || !m)
        return 0;
    if (sEquips[i].race && m->race != sEquips[i].race)
        return 0;
    if (sEquips[i].attribute && m->attribute != sEquips[i].attribute)
        return 0;
    if ((card == YGO_CARD_GERM_INFECTION || card == YGO_CARD_PARALYZING_POTION) && m->race == YGO_MACHINE)
        return 0;
    if (card == YGO_CARD_RING_OF_MAGNETISM && s->objs[o].controller != player)
        return 0;
    return Ygo_CanTarget(s, o, player);
}

int Equip_Can(struct YgoCtx *c)
{
    unsigned o;
    for (o = 0; o < S->objCount; o++)
        if (EquipFits(S, CARD, ME, o))
            return 1;
    return 0;
}

void Equip_Target(struct YgoCtx *c)
{
    uint8_t list[YGO_OBJS], pick;
    unsigned o, n = 0;
    for (o = 0; o < S->objCount; o++)
        if (EquipFits(S, CARD, ME, o))
            list[n++] = o;
    if (Ygo_AskCards(S, ME, YGO_P_EQUIP, CARD, list, n, 1, 1, &pick))
    {
        c->link->targetMoves[c->link->targetCount] = S->objs[pick].moves;
        c->link->targets[c->link->targetCount++] = pick;
    }
}

void Equip_Op(struct YgoCtx *c)
{
    unsigned t = TARGET(0);
    if (t == NO_OBJ || !Ygo_OnField(S, c->obj) || !Fx_TargetOk(c, 0, NULL) || !EquipFits(S, CARD, ME, t))
        return;
    Ygo_Equip(S, c->obj, t);
}

int Equip_Stat(const struct YgoState *s, unsigned src, unsigned target, int which, int value)
{
    int i = EquipIndex(s->objs[src].card);
    if (i < 0 || s->objs[src].equipTo != target + 1)
        return value;
    return value + (which == YS_ATK ? sEquips[i].atk : which == YS_DEF ? sEquips[i].def : 0);
}

/* ---- Field Spells: +200 / -200 by race (today's numbers) */
static const struct
{
    uint16_t card;
    uint8_t up[4], down[2];
} sFields[] = {
    {YGO_CARD_FOREST, {YGO_INSECT, YGO_BEAST, YGO_PLANT, YGO_BEAST_WARRIOR}, {0, 0}},
    {YGO_CARD_WASTELAND, {YGO_DINOSAUR, YGO_ZOMBIE, YGO_ROCK, 0}, {0, 0}},
    {YGO_CARD_MOUNTAIN, {YGO_DRAGON, YGO_WINGED_BEAST, YGO_THUNDER, 0}, {0, 0}},
    {YGO_CARD_SOGEN, {YGO_WARRIOR, YGO_BEAST_WARRIOR, 0, 0}, {0, 0}},
    {YGO_CARD_UMI, {YGO_FISH, YGO_SEA_SERPENT, YGO_THUNDER, YGO_AQUA}, {YGO_MACHINE, YGO_PYRO}},
    {YGO_CARD_YAMI, {YGO_FIEND, YGO_SPELLCASTER, 0, 0}, {YGO_FAIRY, 0}},
};

int Field_Stat(const struct YgoState *s, unsigned src, unsigned target, int which, int value)
{
    unsigned i, k;
    const struct YgoCard *m = Ygo_ObjCard(s, target);
    if (which == YS_LEVEL || !m || !s->objs[target].faceUp)
        return value;
    for (i = 0; i < sizeof(sFields) / sizeof(sFields[0]); i++)
        if (sFields[i].card == s->objs[src].card)
        {
            for (k = 0; k < 4; k++)
                if (sFields[i].up[k] && sFields[i].up[k] == m->race)
                    return value + 200;
            for (k = 0; k < 2; k++)
                if (sFields[i].down[k] && sFields[i].down[k] == m->race)
                    return value - 200;
        }
    return value;
}

/* ---- Fusion Summons */

/* The objects that could be material: hand and Monster Zones of player */
static unsigned MaterialPool(const struct YgoState *s, unsigned player, unsigned exclude, uint8_t *out)
{
    uint8_t list[YGO_OBJS];
    unsigned n = Fx_Collect(s, player, YL_HAND | YL_MZONE, F_MonsterCard, list), i, k = 0;
    for (i = 0; i < n; i++)
        if (list[i] != exclude)
            out[k++] = list[i];
    return k;
}

static int MatFits(const struct YgoState *s, unsigned o, unsigned want)
{
    const struct YgoCard *c = Ygo_ObjCard(s, o);
    if (!c)
        return 0;
    if (want & YGO_MAT_ANY)
        return c->race == (want & 0xFF);
    if (want & YGO_MAT_ATTR)
        return c->attribute == (want & 0xFF);
    return c->id == want;
}

/* Can the materials of fusion be found in pool (each object once)? Fills used with a choice. */
static int Assign(const struct YgoState *s, const struct YgoFusion *f, unsigned at, const uint8_t *pool, unsigned n,
                  uint8_t *used)
{
    unsigned i;
    if (at >= 5 || !f->materials[at])
        return 1;
    for (i = 0; i < n; i++)
        if (!used[i] && MatFits(s, pool[i], f->materials[at]))
        {
            used[i] = 1;
            if (Assign(s, f, at + 1, pool, n, used))
                return 1;
            used[i] = 0;
        }
    return 0;
}

static int FusionPossible(const struct YgoState *s, unsigned player, unsigned fusionObj, unsigned exclude)
{
    const struct YgoFusion *f = Ygo_FusionOf(s->objs[fusionObj].card);
    uint8_t pool[YGO_OBJS], used[YGO_OBJS];
    unsigned n;
    if (!f)
        return 0;
    n = MaterialPool(s, player, exclude, pool);
    memset(used, 0, sizeof(used));
    if (!Assign(s, f, 0, pool, n, used))
        return 0;
    /* a zone for it: a free one, or one a material on the field leaves */
    if (Ygo_FreeZones(s, player, 0))
        return 1;
    {
        unsigned i;
        for (i = 0; i < n; i++)
            if (used[i] && s->objs[pool[i]].location == YGO_LOC_MZONE)
                return 1;
    }
    return 0;
}

int Fx_CanFusion(const struct YgoState *s, unsigned player, unsigned exclude)
{
    unsigned i;
    if (Ygo_Rule(s, YR_CANNOT_SPECIAL_SUMMON, player, 0) || Ygo_Rule(s, YR_NO_SUMMON, player, 0))
        return 0;
    for (i = 0; i < s->players[player].extraCount; i++)
        if (FusionPossible(s, player, s->players[player].extra[i], exclude))
            return 1;
    return 0;
}

void Ygo_FusionSummon(struct YgoCtx *c)
{
    uint8_t list[YGO_FUSION_MAX], pick, pool[YGO_OBJS], chosen[5];
    unsigned i, n = 0, k, m;
    const struct YgoFusion *f;
    for (i = 0; i < S->players[ME].extraCount; i++)
        if (FusionPossible(S, ME, S->players[ME].extra[i], c->obj))
            list[n++] = S->players[ME].extra[i];
    if (!n)
        return;
    Ygo_AskCards(S, ME, YGO_P_FUSION, CARD, list, n, 1, 1, &pick);
    f = Ygo_FusionOf(S->objs[pick].card);
    /* each material in turn, from what's left (only choices that still let the rest be found) */
    for (k = 0; k < 5 && f->materials[k]; k++)
    {
        uint8_t options[YGO_OBJS], rest[YGO_OBJS], used[YGO_OBJS];
        unsigned count = 0, p = MaterialPool(S, ME, c->obj, pool), j, r;
        struct YgoFusion tail;
        memset(&tail, 0, sizeof(tail));
        for (j = k + 1; j < 5; j++)
            tail.materials[j - k - 1] = f->materials[j];
        for (j = 0; j < p; j++)
        {
            int taken = 0;
            for (m = 0; m < k; m++)
                taken |= chosen[m] == pool[j];
            if (taken || !MatFits(S, pool[j], f->materials[k]))
                continue;
            for (m = r = 0; m < p; m++)
            {
                unsigned q;
                int t = m == j;
                for (q = 0; q < k; q++)
                    t |= chosen[q] == pool[m];
                if (!t)
                    rest[r++] = pool[m];
            }
            memset(used, 0, sizeof(used));
            if (Assign(S, &tail, 0, rest, r, used))
                options[count++] = pool[j];
        }
        if (!count)
            return;
        Ygo_AskCards(S, ME, YGO_P_MATERIAL, f->materials[k], options, count, 1, 1, &chosen[k]);
    }
    for (m = 0; m < k; m++)
        Ygo_SendToGrave(S, chosen[m], YGO_REASON_MATERIAL | YGO_REASON_EFFECT);
    i = Ygo_AskPosition(S, ME, S->objs[pick].card, 3);
    Ygo_SpecialSummon(S, pick, ME, i, YGO_SUMMON_FUSION);
}

/* ---- Ritual Summons: Tribute monsters from the hand or field whose Levels add up to the Ritual Monster's or more */
static unsigned RitualTributes(const struct YgoState *s, unsigned player, unsigned exclude, uint8_t *out)
{
    uint8_t list[YGO_OBJS];
    unsigned n = Fx_Collect(s, player, YL_HAND | YL_MZONE, F_MonsterCard, list), i, k = 0;
    for (i = 0; i < n; i++)
        if (list[i] != exclude)
            out[k++] = list[i];
    return k;
}

static int RitualLevels(const struct YgoState *s, const uint8_t *list, unsigned n)
{
    unsigned i;
    int sum = 0;
    for (i = 0; i < n; i++)
        sum += YgoDuel_Level(s, list[i]);
    return sum;
}

int Fx_CanRitual(const struct YgoState *s, unsigned player, unsigned ritualCard, unsigned exclude)
{
    uint8_t hand[YGO_HAND_MAX], tributes[YGO_OBJS];
    unsigned n = Ygo_List(s, player, YGO_LOC_HAND, hand), i;
    if (Ygo_Rule(s, YR_CANNOT_SPECIAL_SUMMON, player, 0) || Ygo_Rule(s, YR_NO_SUMMON, player, 0))
        return 0;
    for (i = 0; i < n; i++)
        if (s->objs[hand[i]].card == ritualCard)
        {
            unsigned t = RitualTributes(s, player, hand[i], tributes), k, j;
            int sum;
            /* the exclude (the ritual spell) isn't a monster anyway */
            (void)exclude;
            for (k = j = 0; k < t; k++)
                if (tributes[k] != exclude)
                    tributes[j++] = tributes[k];
            sum = RitualLevels(s, tributes, j);
            if (sum >= Ygo_Card(ritualCard)->level &&
                (Ygo_FreeZones(s, player, 0) || Ygo_MonsterCount(s, player)))
                return 1;
        }
    return 0;
}

void Ygo_RitualSummon(struct YgoCtx *c, unsigned ritualCard)
{
    uint8_t hand[YGO_HAND_MAX], list[YGO_OBJS], picked[YGO_OBJS], ritual = NO_OBJ, pick;
    unsigned n = Ygo_List(S, ME, YGO_LOC_HAND, hand), i, k = 0;
    int need, sum = 0;
    for (i = 0; i < n; i++)
        if (S->objs[hand[i]].card == ritualCard)
            list[k++] = hand[i];
    if (!k)
        return;
    Ygo_AskCards(S, ME, YGO_P_RITUAL, ritualCard, list, k, 1, 1, &ritual);
    need = Ygo_Card(ritualCard)->level;
    /* tributes one at a time until the Levels are enough */
    n = 0;
    while (sum < need)
    {
        unsigned t = RitualTributes(S, ME, ritual, list), j, m = 0;
        uint8_t options[YGO_OBJS];
        for (j = 0; j < t; j++)
        {
            unsigned q;
            int taken = 0;
            for (q = 0; q < n; q++)
                taken |= picked[q] == list[j];
            if (!taken)
                options[m++] = list[j];
        }
        if (!m)
            return;
        Ygo_AskCards(S, ME, YGO_P_TRIBUTE, ritualCard, options, m, 1, 1, &pick);
        picked[n++] = pick;
        sum += YgoDuel_Level(S, pick);
    }
    for (i = 0; i < n; i++)
        Ygo_SendToGrave(S, picked[i], YGO_REASON_TRIBUTE | YGO_REASON_EFFECT);
    i = Ygo_AskPosition(S, ME, ritualCard, 3);
    Ygo_SpecialSummon(S, ritual, ME, i, YGO_SUMMON_RITUAL);
}
