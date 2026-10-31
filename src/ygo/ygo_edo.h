/* Yu-Gi-Oh! duels on EDOPro's engine (edopro/ocgcore): the data it reads (ygo_edo_data.c) and the bridge between it
 * and the game's duel API in include/ygo/duel.h (ygo_edo.c). Internal to src/ygo. */
#ifndef MOEMON_YGO_EDO_H
#define MOEMON_YGO_EDO_H
#include <stddef.h>
#include <stdint.h>
#include "ygo/ygo.h"

/* One card in EDOPro's database (graphics/ygo/edo.pak cards.bin) */
struct YgoEdoRec
{
    uint32_t code, alias;
    uint64_t setcode;
    uint32_t type, level, attribute;
    uint64_t race;
    int32_t atk, def;
    uint32_t ot;
    const char *name, *desc;
    const char *str[16];   /* the effect strings (aux.Stringid(code, n)) */
};

/* EDOPro's card types, attributes, races, locations, positions (CardScripts constant.lua) */
enum
{
    EDO_TYPE_MONSTER = 0x1, EDO_TYPE_SPELL = 0x2, EDO_TYPE_TRAP = 0x4, EDO_TYPE_NORMAL = 0x10, EDO_TYPE_EFFECT = 0x20,
    EDO_TYPE_FUSION = 0x40, EDO_TYPE_RITUAL = 0x80, EDO_TYPE_TRAPMONSTER = 0x100, EDO_TYPE_SPIRIT = 0x200,
    EDO_TYPE_UNION = 0x400, EDO_TYPE_GEMINI = 0x800, EDO_TYPE_TUNER = 0x1000, EDO_TYPE_SYNCHRO = 0x2000,
    EDO_TYPE_TOKEN = 0x4000, EDO_TYPE_QUICKPLAY = 0x10000, EDO_TYPE_CONTINUOUS = 0x20000, EDO_TYPE_EQUIP = 0x40000,
    EDO_TYPE_FIELD = 0x80000, EDO_TYPE_COUNTER = 0x100000, EDO_TYPE_FLIP = 0x200000, EDO_TYPE_TOON = 0x400000,
    EDO_TYPE_XYZ = 0x800000, EDO_TYPE_PENDULUM = 0x1000000, EDO_TYPE_LINK = 0x4000000
};
enum
{
    EDO_LOC_DECK = 0x1, EDO_LOC_HAND = 0x2, EDO_LOC_MZONE = 0x4, EDO_LOC_SZONE = 0x8, EDO_LOC_GRAVE = 0x10,
    EDO_LOC_REMOVED = 0x20, EDO_LOC_EXTRA = 0x40, EDO_LOC_OVERLAY = 0x80
};
enum { EDO_POS_FACEUP_ATTACK = 1, EDO_POS_FACEDOWN_ATTACK = 2, EDO_POS_FACEUP_DEFENSE = 4, EDO_POS_FACEDOWN_DEFENSE = 8 };

/* The pack: 1 when it's there and whole. Everything below needs it. */
int YgoEdo_DataInit(void);
/* A file of the pack, inflated (malloc'd: the caller frees it), NULL when it isn't there */
uint8_t *YgoEdo_File(const char *name, uint32_t *size);
int YgoEdo_HasFile(const char *name);
const struct YgoEdoRec *YgoEdo_Rec(uint32_t code);
unsigned YgoEdo_RecCount(void);
const struct YgoEdoRec *YgoEdo_RecAt(unsigned index);
/* The game's card id for an EDOPro code: the catalog's card, else one made from EDOPro's database (ids past
 * YGO_CARD_COUNT: tokens and every other card an effect can bring into a duel); 0 unknown */
unsigned YgoEdo_IdOfCode(uint32_t code);
uint32_t YgoEdo_CodeOfId(unsigned id);
/* The catalog card a code is (another printing of it too), 0 when the catalog hasn't got it */
unsigned YgoEdo_CatalogOf(uint32_t code);
/* A string by EDOPro's description number: a card's effect string (code << 20 | n) or a system string */
const char *YgoEdo_Desc(uint64_t desc);
const char *YgoEdo_System(unsigned id);
const char *YgoEdo_Counter(unsigned type);
/* Does EDOPro play the card fully (a script for it, or a Normal Monster that needs none)? */
int YgoEdo_Playable(uint32_t code);

/* Forbidden & Limited lists: the game's own (the catalog's) and every list EDOPro ships */
#define YGO_EDO_LISTS_MAX 16
unsigned YgoEdo_ListCount(void);
const char *YgoEdo_ListName(unsigned list);
/* copies allowed of code in list (3 when the list doesn't name it) */
unsigned YgoEdo_ListLimit(unsigned list, uint32_t code);
#endif
