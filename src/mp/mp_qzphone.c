// Moemon Kai: JOHTO's phone, Crystal's own (pokecrystal engine/phone, data/phone; converted by
// tools/quetzal_port/gen_phone.py into src/data/qz_phone.h): 28 trainers who offer their number once beaten, call the
// player (their news, their POKéMON, a rematch, a gift waiting) and can be called from the POKéGEAR, with Crystal's
// words and Crystal's rematch teams.
//
// One interpreter runs every converted script with Crystal's semantics: `end` returns from a call, the conditions test
// the last value (wScriptVar). It runs three ways:
// - in the field (QzPhone_EventScript_Run in data/scripts/qz_phone.inc): a beaten contact talked to (the trainer's
//   own Crystal `.Script`: rematch, gift, asking for the number) and incoming calls; each step returns what the field
//   script must do (a text box, YES / NO, an item, a battle) and the field script calls back for the next one;
// - for the POKéGEAR (MP_QzPhone_Call): the words of a call, as lines for its screen;
// - silent, to see how a call would go before it rings (OPTIONS > CO-OP > MOENAV CALLS: BATTLES keeps only the calls
//   that ask for a battle, as the MOENAV in HOENN): the same seed plays the same call again for real.
// State (region vars 0x5260-0x5278, saved with the region; 0x5240-0x5258 until save format 3, where SINNOH's
// Platinum vars are: mp_savefix.c moved them): numbers registered, Crystal's flags and events for the
// phone, each contact's fight count, the step count to the next call.
#include "global.h"
#include "battle_setup.h"
#include "data.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "fieldmap.h"
#include "item.h"
#include "overworld.h"
#include "pokedex.h"
#include "pokemon.h"
#include "random.h"
#include "region_map.h"
#include "rtc.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "text.h"
#include "wild_encounter.h"
#include "constants/battle_setup.h"
#include "constants/items.h"
#include "constants/map_types.h"
#include "constants/wild_encounter.h"
#include "constants/songs.h"
#include "constants/species.h"
#include "constants/trainers.h"
#include "constants/qz_port.h"
#include "mp.h"
#include "mp_region.h"
#include "../data/qz_phone.h"
#include <stdio.h>
extern char *getenv(const char *);
#include <string.h>

// ---- state (region vars)
#define VAR_QZPHONE_NUMBERS 0x5260   // 3 words: the contacts registered (by Crystal's PHONE_* number)
#define VAR_QZPHONE_FLAGS   0x5263   // 10 words: Crystal's phone flags and events (QZPHONE_FLAGS of them)
#define VAR_QZPHONE_FIGHTS  0x526D   // 8 words: each contact's fight count, 3 bits each
#define VAR_QZPHONE_STEPS   0x5276   // steps to the next call
#define VAR_QZPHONE_KENJI   0x5277   // KENJI's break countdown
#define VAR_QZPHONE_SWARM   0x5278   // 1: QWILFISH on ROUTE 32, 2: REMORAID on ROUTE 44 (RALPH's / WILTON's news)
#define QZPHONE_STATE_WORDS (VAR_QZPHONE_SWARM + 1 - VAR_QZPHONE_NUMBERS)
STATIC_ASSERT(QZPHONE_FLAGS <= 10 * 16, qzPhoneFlagsFit);
STATIC_ASSERT(QZPHONE_CONTACT_SLOTS <= 3 * 16, qzPhoneContactsFit);
STATIC_ASSERT(QZPHONE_CONTACT_SLOTS * 3 + 16 <= 8 * 16, qzPhoneFightsFit);

static bool8 Bit(u16 var, u32 n)
{
    return (VarGet(var + n / 16) >> (n % 16)) & 1;
}

static void SetBit(u16 var, u32 n, bool8 on)
{
    u16 v = VarGet(var + n / 16);
    VarSet(var + n / 16, on ? (v | (1 << (n % 16))) : (v & ~(1 << (n % 16))));
}

static u8 FightCount(u8 c)
{
    u32 bit = c * 3;
    u32 v = VarGet(VAR_QZPHONE_FIGHTS + bit / 16) | ((u32)VarGet(VAR_QZPHONE_FIGHTS + bit / 16 + 1) << 16);
    return (v >> (bit % 16)) & 7;
}

static void SetFightCount(u8 c, u8 n)
{
    u32 bit = c * 3, k;
    for (k = 0; k < 3; k++)
        SetBit(VAR_QZPHONE_FIGHTS, bit + k, (n >> k) & 1);
}

static bool8 IsContact(u8 c)
{
    return c < QZPHONE_CONTACT_SLOTS && sQzPhoneContacts[c].name[0] != 0;
}

bool8 MP_QzPhone_Registered(u8 c)
{
    return IsContact(c) && Bit(VAR_QZPHONE_NUMBERS, c);
}

// ---- the interpreter
enum { MODE_FIELD, MODE_LINES, MODE_SILENT };
enum { STEP_END, STEP_TEXT, STEP_YESNO, STEP_ITEM, STEP_BATTLE, STEP_FANFARE };

static struct
{
    u16 pc, stack[8];
    u8 sp, mode, caller, battleContact, battleFight;
    s16 var;
    u32 seed;
    bool8 running, wantsBattle;
    u16 pendingText;   // (a text waiting to be shown with what comes next: YES / NO)
    u16 winLossText;
    u8 *lines;         // MODE_LINES: where the words go (paragraphs separated by \p)
    u16 linesSize, linesLen;
} sVm;

static u16 Rand(void)
{
    sVm.seed = sVm.seed * 1103515245 + 24691;
    return sVm.seed >> 16;
}

static bool8 Story(u16 s)
{
    static const u16 sFly[] = {0x11BE, 0x11BF, 0x11C0, 0x11C1, 0x11C3, 0x11C4, 0x11C5, 0x2262};   // (Quetzal's fly flags)
    u16 radio = VarGet(VAR_QZ_0x500A);
    if (s <= QZSTORY_ENGINE_FLYPOINT_VERMILION)
        return FlagGet(sFly[s - QZSTORY_ENGINE_FLYPOINT_GOLDENROD]);
    switch (s)
    {
    case QZSTORY_ENGINE_PLAINBADGE:            return FlagGet(MP_Region_BadgeFlag(REGION_JOHTO, 2));
    case QZSTORY_ENGINE_FOGBADGE:              return FlagGet(MP_Region_BadgeFlag(REGION_JOHTO, 3));
    case QZSTORY_ENGINE_RISINGBADGE:           return FlagGet(MP_Region_BadgeFlag(REGION_JOHTO, 7));
    case QZSTORY_ENGINE_MARSHBADGE:            return FlagGet(MP_Region_BadgeFlag(REGION_KANTO, 5));
    case QZSTORY_ENGINE_ROCKETS_IN_RADIO_TOWER: return radio >= 6 && radio < 10;
    case QZSTORY_EVENT_BEAT_ELITE_FOUR:        return MP_Region_Cleared(REGION_JOHTO);
    case QZSTORY_EVENT_CLEARED_RADIO_TOWER:    return radio >= 10;
    case QZSTORY_EVENT_CLEARED_ROCKET_HIDEOUT: return radio >= 6;   // (TEAM ROCKET takes the TOWER after it)
    case QZSTORY_EVENT_RESTORED_POWER_TO_KANTO: return MP_Region_Cleared(REGION_JOHTO) && (gRegionState.started & (1 << REGION_KANTO));
    case QZSTORY_EVENT_OPENED_MT_SILVER:       return MP_Region_Cleared(REGION_KANTO);
    case QZSTORY_EVENT_JASMINE_RETURNED_TO_GYM: return MP_Region_BadgeCount(REGION_JOHTO) >= 5;
    }
    return FALSE;   // (the BUG-CATCHING CONTEST of the day, the DEPT. STORE's sale, KANTO's SNORLAX)
}

static u8 *Buffer(u8 n)
{
    return n == 3 ? gStringVar1 : n == 4 ? gStringVar2 : gStringVar3;
}

// A contact's team now (their fight count picks Crystal's rematch team; the first is Quetzal's trainer)
static u16 TeamSpecies(u8 c, u8 k, u8 *count)
{
    const struct QzPhoneContact *q = &sQzPhoneContacts[c];
    u8 f = FightCount(c);
    if (f > 0 && f < q->fightCount && q->fights[f].party != NULL)
    {
        *count = q->fights[f].count;
        return q->fights[f].party[k % q->fights[f].count].species;
    }
    if (q->trainer == TRAINER_NONE)
    {
        *count = 1;
        return SPECIES_RATTATA;
    }
    {
        const struct Trainer *t = MP_Region_TrainerData(q->trainer);   // (Quetzal's trainers aren't in gTrainers)
        *count = t->partySize ? t->partySize : 1;
        k %= *count;
        switch (t->partyFlags & (F_TRAINER_PARTY_CUSTOM_MOVESET | F_TRAINER_PARTY_HELD_ITEM))
        {
        case 0:                              return t->party.NoItemDefaultMoves[k].species;
        case F_TRAINER_PARTY_CUSTOM_MOVESET: return t->party.NoItemCustomMoves[k].species;
        case F_TRAINER_PARTY_HELD_ITEM:      return t->party.ItemDefaultMoves[k].species;
        default:                             return t->party.ItemCustomMoves[k].species;
        }
    }
}

static u16 WildOn(u16 map, bool8 unseen)
{
    u32 i, k;
    for (i = 0; gWildMonHeaders[i].mapGroup != MAP_GROUP(MAP_UNDEFINED); i++)
    {
        const struct WildPokemonInfo *land;
        if (gWildMonHeaders[i].mapGroup != MAP_GROUP(map) || gWildMonHeaders[i].mapNum != MAP_NUM(map))
            continue;
        land = gWildMonHeaders[i].landMonsInfo ? MP_Region_WildInfo(gWildMonHeaders[i].landMonsInfo) : NULL;
        if (land == NULL)
            land = gWildMonHeaders[i].waterMonsInfo ? MP_Region_WildInfo(gWildMonHeaders[i].waterMonsInfo) : NULL;
        if (land == NULL)
            return SPECIES_NONE;
        if (!unseen)
            return land->wildPokemon[Rand() % 12].species;
        for (k = 0; k < 12; k++)
        {
            u16 sp = land->wildPokemon[(k + Rand()) % 12].species;
            if (!GetSetPokedexFlag(SpeciesToNationalPokedexNum(sp), FLAG_GET_SEEN))
                return sp;
        }
        return SPECIES_NONE;
    }
    return SPECIES_NONE;
}

static void Special(u16 id)
{
    u16 sp;
    u8 n;
    switch (id)
    {
    case 0:   // RandomPhoneMon: one of the caller's team
        sp = TeamSpecies(sVm.caller, Rand(), &n);
        StringCopy(gStringVar2, gSpeciesNames[sp]);
        break;
    case 1:   // RandomPhoneWildMon: one met where the caller is
        sp = WildOn(sQzPhoneContacts[sVm.caller].map, FALSE);
        StringCopy(gStringVar2, gSpeciesNames[sp ? sp : SPECIES_RATTATA]);
        break;
    case 2:   // RandomUnseenWildMon: one not yet seen there (the value: none found)
        sp = WildOn(sQzPhoneContacts[sVm.caller].map, TRUE);
        sVm.var = sp == SPECIES_NONE;
        if (sp)
            StringCopy(gStringVar2, gSpeciesNames[sp]);
        break;
    case 3:   // ActivateFishingSwarm (the value set before: QWILFISH / REMORAID)
        VarSet(VAR_QZPHONE_SWARM, sVm.var);
        break;
    case 4:   // SampleKenjiBreakCountdown
        VarSet(VAR_QZPHONE_KENJI, 1 + Rand() % 4);
        break;
    }
}

static void AddLines(const u8 *text)
{
    u8 buf[0x200];
    u16 len;
    if (sVm.lines == NULL)
        return;
    StringExpandPlaceholders(buf, text);
    len = StringLength(buf);
    if (sVm.linesLen + len + 2 >= sVm.linesSize)
        return;
    if (sVm.linesLen)
        sVm.lines[sVm.linesLen++] = CHAR_PROMPT_CLEAR;   // (\p: a new page)
    memcpy(sVm.lines + sVm.linesLen, buf, len);
    sVm.linesLen += len;
    sVm.lines[sVm.linesLen] = EOS;
}

// Crystal's words (paragraphs run together) into gStringVar4 for the field's message box: two lines a page, each as
// wide as the box holds
static void FieldText(const u8 *text)
{
    u8 buf[0x200], line[0x80];
    u16 i = 0, o = 0, l = 0, lines = 0, start;
    StringExpandPlaceholders(buf, text);
    while (buf[i] != EOS)
    {
        start = i;
        while (buf[i] != EOS && buf[i] != CHAR_SPACE)
            i++;
        // try the word on this line
        memcpy(line + l, buf + start, i - start);
        line[l + i - start] = EOS;
        if (l && GetStringWidth(FONT_NORMAL, line, 0) > 208)
        {
            gStringVar4[o++] = (++lines % 2) ? CHAR_NEWLINE : CHAR_PROMPT_SCROLL;
            l = 0;
            memcpy(line, buf + start, i - start);
            line[i - start] = EOS;
        }
        else if (l)
        {
            gStringVar4[o++] = CHAR_SPACE;
        }
        memcpy(gStringVar4 + o, buf + start, i - start);
        o += i - start;
        l += i - start;
        line[l] = CHAR_SPACE;
        l++;
        if (buf[i] == CHAR_SPACE)
            i++;
        if (o > 0x3F0)
            break;
    }
    gStringVar4[o] = EOS;
}

static void SetupBattleTrainer(void);

// Runs until the field must show something; returns what (STEP_*). In MODE_LINES / MODE_SILENT it runs to the end.
static u8 Run(void)
{
    const u16 *code = sQzPhoneCode;
    while (sVm.running)
    {
        u16 op = code[sVm.pc++];
        u16 a, b;
        if (getenv("QZPHONE_TRACE"))
            printf("[qzphone] pc %u op %u args %u %u var %d\n", sVm.pc - 1, op, code[sVm.pc], code[sVm.pc + 1], sVm.var);
        switch (op)
        {
        case QZOP_END:
            if (sVm.sp)
            {
                sVm.pc = sVm.stack[--sVm.sp];
                break;
            }
            sVm.running = FALSE;
            if (sVm.pendingText != 0xFFFF && sVm.mode == MODE_FIELD)
            {
                FieldText(sQzPhoneTexts[sVm.pendingText]);
                sVm.pendingText = 0xFFFF;
                return STEP_TEXT;
            }
            return STEP_END;
        case QZOP_JUMP:
            sVm.pc = code[sVm.pc];
            break;
        case QZOP_CALL:
            a = code[sVm.pc++];
            if (sVm.sp < ARRAY_COUNT(sVm.stack))
                sVm.stack[sVm.sp++] = sVm.pc;
            sVm.pc = a;
            break;
        case QZOP_IFTRUE:
        case QZOP_IFFALSE:
            a = code[sVm.pc++];
            if ((sVm.var != 0) == (op == QZOP_IFTRUE))
                sVm.pc = a;
            break;
        case QZOP_IFEQ:
        case QZOP_IFNE:
        case QZOP_IFGT:
        case QZOP_IFLT:
            a = code[sVm.pc++];
            b = code[sVm.pc++];
            if ((op == QZOP_IFEQ && sVm.var == a) || (op == QZOP_IFNE && sVm.var != a)
             || (op == QZOP_IFGT && sVm.var > a) || (op == QZOP_IFLT && sVm.var < a))
                sVm.pc = b;
            break;
        case QZOP_CHECKFLAG:
            sVm.var = Bit(VAR_QZPHONE_FLAGS, code[sVm.pc++]);
            break;
        case QZOP_SETFLAG:
        case QZOP_CLEARFLAG:
            a = code[sVm.pc++];
            SetBit(VAR_QZPHONE_FLAGS, a, op == QZOP_SETFLAG);
            if (op == QZOP_SETFLAG && a == sQzPhoneContacts[sVm.caller].rematchFlag)
                sVm.wantsBattle = TRUE;
            break;
        case QZOP_CHECKSTORY:
            sVm.var = Story(code[sVm.pc++]);
            break;
        case QZOP_TEXT:
            a = code[sVm.pc++];
            if (a == 0xFFFF)
                break;
            if (sVm.mode == MODE_LINES)
                AddLines(sQzPhoneTexts[a]);
            else if (sVm.mode == MODE_FIELD)
            {
                if (sVm.pendingText != 0xFFFF)
                {
                    // (two texts in a row: the first shown now, this one waits)
                    FieldText(sQzPhoneTexts[sVm.pendingText]);
                    sVm.pendingText = a;
                    return STEP_TEXT;
                }
                sVm.pendingText = a;
            }
            break;
        case QZOP_PROMPT:
            if (sVm.mode == MODE_FIELD && sVm.pendingText != 0xFFFF)
            {
                FieldText(sQzPhoneTexts[sVm.pendingText]);
                sVm.pendingText = 0xFFFF;
                return STEP_TEXT;
            }
            break;
        case QZOP_YESNO:
            if (sVm.mode != MODE_FIELD)
            {
                sVm.var = FALSE;
                break;
            }
            FieldText(sVm.pendingText != 0xFFFF ? sQzPhoneTexts[sVm.pendingText] : (const u8 *)"\xFF");
            sVm.pendingText = 0xFFFF;
            return STEP_YESNO;
        case QZOP_RANDOM:
            a = code[sVm.pc++];
            sVm.var = a ? Rand() % a : 0;
            break;
        case QZOP_READVAR:
            a = code[sVm.pc++];
            RtcCalcLocalTime();
            switch (a)
            {
            case 0: sVm.var = gLocalTime.days % 7; break;   // (Crystal's weekday: SUNDAY 0)
            case 1: sVm.var = gLocalTime.hours; break;
            case 2: sVm.var = sVm.caller; break;
            case 3: sVm.var = gSaveBlock1Ptr->location.mapGroup; break;
            case 4: sVm.var = VarGet(VAR_QZPHONE_KENJI); break;
            case 5: sVm.var = gMapHeader.mapType == MAP_TYPE_UNDERGROUND; break;
            case 6: sVm.var = 1; break;
            default: sVm.var = 0; break;
            }
            break;
        case QZOP_CHECKTIME:
            a = code[sVm.pc++];
            RtcCalcLocalTime();
            sVm.var = (a == 1 && gLocalTime.hours >= 4 && gLocalTime.hours < 10)
                   || (a == 2 && gLocalTime.hours >= 10 && gLocalTime.hours < 18)
                   || (a == 4 && (gLocalTime.hours >= 18 || gLocalTime.hours < 4));
            break;
        case QZOP_TRAINERNAME:
        case QZOP_CLASSNAME:
            a = code[sVm.pc++];
            b = code[sVm.pc++];
            if (IsContact(b))
                StringCopy(Buffer(a), op == QZOP_TRAINERNAME ? sQzPhoneContacts[b].name : sQzPhoneContacts[b].className);
            break;
        case QZOP_LANDMARK:
        case QZOP_STRING:
            a = code[sVm.pc++];
            b = code[sVm.pc++];
            if (b != 0xFFFF)
                StringCopy(Buffer(a), sQzPhoneTexts[b]);
            break;
        case QZOP_CURLANDMARK:
            a = code[sVm.pc++];
            GetMapName(Buffer(a), gMapHeader.regionMapSectionId, 0);
            break;
        case QZOP_MONNAME:
            a = code[sVm.pc++];
            b = code[sVm.pc++];
            StringCopy(Buffer(a), gSpeciesNames[sQzPhoneMonNames[b]]);
            break;
        case QZOP_SPECIAL:
            Special(code[sVm.pc++]);
            break;
        case QZOP_SETCALLER:
            sVm.caller = code[sVm.pc++];
            break;
        case QZOP_CHECKCELL:
            sVm.var = MP_QzPhone_Registered(code[sVm.pc++]);
            break;
        case QZOP_ASKNUMBER:
            // Crystal's askforphonenumber: the question already shown waits for YES / NO; the value is 0 (registered),
            // 1 (the phone full: never, the phone has room for them all) or 2 (refused)
            a = code[sVm.pc++];
            sVm.caller = a;
            if (sVm.mode != MODE_FIELD)
            {
                sVm.var = 2;
                break;
            }
            sVm.battleContact = a;   // (who the answer is for)
            FieldText(sVm.pendingText != 0xFFFF ? sQzPhoneTexts[sVm.pendingText] : (const u8 *)"\xFF");
            sVm.pendingText = 0xFFFF;
            sVm.var = 0x100;   // (QzPhone_Answer: an ASKNUMBER's YES registers)
            return STEP_YESNO;
        case QZOP_READMEM:
            sVm.var = FightCount(code[sVm.pc++]);
            break;
        case QZOP_LOADMEM:
            a = code[sVm.pc++];
            b = code[sVm.pc++];
            SetFightCount(a, b);
            break;
        case QZOP_LOADTRAINER:
            sVm.battleContact = code[sVm.pc++];
            sVm.battleFight = code[sVm.pc++];
            break;
        case QZOP_STARTBATTLE:
            if (sVm.mode != MODE_FIELD)
                break;
            if (sVm.pendingText != 0xFFFF)
            {
                FieldText(sQzPhoneTexts[sVm.pendingText]);
                sVm.pendingText = 0xFFFF;
                sVm.pc--;   // (the battle after the words)
                return STEP_TEXT;
            }
            return STEP_BATTLE;
        case QZOP_WINLOSS:
            sVm.winLossText = code[sVm.pc++];
            break;
        case QZOP_GIVEITEM:
            a = code[sVm.pc++];
            if (sVm.mode != MODE_FIELD)
            {
                sVm.var = TRUE;
                break;
            }
            if (sVm.pendingText != 0xFFFF)
            {
                FieldText(sQzPhoneTexts[sVm.pendingText]);
                sVm.pendingText = 0xFFFF;
                sVm.pc -= 2;
                return STEP_TEXT;
            }
            VarSet(VAR_0x8004, sQzPhoneItems[a]);
            VarSet(VAR_0x8005, 1);
            return STEP_ITEM;
        case QZOP_SETVAL:
            sVm.var = code[sVm.pc++];
            break;
        case QZOP_SWARM:
            sVm.pc++;
            break;
        case QZOP_REGSOUND:
            if (sVm.mode != MODE_FIELD)
                break;
            if (sVm.pendingText != 0xFFFF)
            {
                FieldText(sQzPhoneTexts[sVm.pendingText]);
                sVm.pendingText = 0xFFFF;
            }
            return STEP_FANFARE;
        case QZOP_CHECKMONEY:
            sVm.var = 0;
            break;
        default:
            sVm.running = FALSE;
            return STEP_END;
        }
    }
    return STEP_END;
}

static void Start(u16 pc, u8 caller, u8 mode, u32 seed)
{
    memset(&sVm, 0, sizeof(sVm));
    sVm.pc = pc;
    sVm.caller = caller;
    sVm.mode = mode;
    sVm.seed = seed;
    sVm.running = TRUE;
    sVm.pendingText = 0xFFFF;
    sVm.winLossText = 0xFFFF;
}

// ---- the field script (data/scripts/qz_phone.inc)
// EventScript_NoNormalTrainerBattle: a beaten JOHTO contact talked to runs their Crystal script (TRUE)
bool8 MP_QzPhone_TrainerTalk(void)
{
    u32 c;
    if (MP_Region_OfGroup(gSaveBlock1Ptr->location.mapGroup) != REGION_JOHTO)
        return FALSE;
    for (c = 0; c < QZPHONE_CONTACT_SLOTS; c++)
        if (IsContact(c) && sQzPhoneContacts[c].trainer == gTrainerBattleOpponent_A && gTrainerBattleOpponent_A != TRAINER_NONE)
        {
            Start(sQzPhoneContacts[c].script, c, MODE_FIELD, Random32());
            printf("[mp] qzphone: contact %u talked to\n", (unsigned)c);
            return TRUE;
        }
    return FALSE;
}

void QzPhone_TrainerTalk(void)
{
    gSpecialVar_Result = MP_QzPhone_TrainerTalk();
}

void QzPhone_Step(void)
{
    gSpecialVar_Result = sVm.running ? Run() : STEP_END;
}

// After YES / NO (VAR_RESULT: 1 YES)
void QzPhone_Answer(void)
{
    bool8 yes = gSpecialVar_Result == 1;
    if (sVm.var == 0x100)   // (askforphonenumber)
    {
        if (yes)
        {
            SetBit(VAR_QZPHONE_NUMBERS, sVm.battleContact, TRUE);
            printf("[mp] qzphone: registered %u\n", sVm.battleContact);
        }
        sVm.var = yes ? 0 : 2;
    }
    else
    {
        sVm.var = yes;
    }
}

// After giveitem (VAR_RESULT: TRUE given)
void QzPhone_ItemGiven(void)
{
    sVm.var = gSpecialVar_Result;
    printf("[mp] qzphone: item %u x%u given %d\n", gSpecialVar_0x8004, gSpecialVar_0x8005, gSpecialVar_Result);
}

// ---- rematches: Crystal's team for the fight loaded (MP_Region_TrainerData)
static bool8 sRematch;
static u8 sRematchContact, sRematchFight;
static u8 sBeatenText[0x200];

static void LevelMoves(u16 species, u8 level, u16 *moves)
{
    static struct Pokemon sTmp;
    u32 k;
    CreateMon(&sTmp, species, level, 0, TRUE, 0, OT_ID_RANDOM_NO_SHINY, 0);
    for (k = 0; k < MAX_MON_MOVES; k++)
        moves[k] = GetMonData(&sTmp, MON_DATA_MOVE1 + k);
}

const struct Trainer *MP_QzPhone_RematchTrainer(u16 trainerNum, const struct Trainer *base)
{
    static struct Trainer sTrainer;
    static struct TrainerMonItemCustomMoves sParty[PARTY_SIZE];
    const struct QzPhoneFight *f;
    u32 k;
    if (!sRematch || sQzPhoneContacts[sRematchContact].trainer != trainerNum)
        return base;
    f = &sQzPhoneContacts[sRematchContact].fights[sRematchFight];
    if (sRematchFight == 0 || f->party == NULL)
        return base;
    sTrainer = *base;
    sTrainer.partySize = f->count < PARTY_SIZE ? f->count : PARTY_SIZE;
    for (k = 0; k < sTrainer.partySize; k++)
    {
        sParty[k] = f->party[k];
        if (!f->customMoves)
            LevelMoves(sParty[k].species, sParty[k].lvl, sParty[k].moves);
    }
    sTrainer.partyFlags = F_TRAINER_PARTY_CUSTOM_MOVESET | F_TRAINER_PARTY_HELD_ITEM;
    sTrainer.party.ItemCustomMoves = sParty;
    return &sTrainer;
}

// Before `dotrainerbattle` (qz_phone.inc): the contact's trainer, no intro, Crystal's words when beaten
void QzPhone_SetupBattle(void)
{
    static u8 sData[16];
    u16 trainer = sQzPhoneContacts[sVm.battleContact].trainer;
    const u8 *beaten = sBeatenText;
    if (sVm.winLossText != 0xFFFF)
    {
        FieldText(sQzPhoneTexts[sVm.winLossText]);
        StringCopy(sBeatenText, gStringVar4);
    }
    else
    {
        sBeatenText[0] = EOS;
    }
    sRematch = TRUE;
    sRematchContact = sVm.battleContact;
    sRematchFight = sVm.battleFight;
    // (BattleSetup_ConfigureTrainerBattle's TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT: mode, trainer, local id, defeat text)
    sData[0] = TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT;
    sData[1] = trainer & 0xFF;
    sData[2] = trainer >> 8;
    sData[3] = gSpecialVar_LastTalked & 0xFF;
    sData[4] = gSpecialVar_LastTalked >> 8;
    memcpy(&sData[5], &beaten, sizeof(beaten));
    BattleSetup_ConfigureTrainerBattle(sData);
    printf("[mp] qzphone: rematch %u fight %u (trainer %u)\n", sRematchContact, sRematchFight, trainer);
}

void MP_QzPhone_BattleOver(void)
{
    sRematch = FALSE;
}

// ---- the POKéGEAR: the numbers and a call's words
u8 MP_QzPhone_Numbers(u8 *out, u8 max)
{
    u8 c, n = 0;
    for (c = 0; c < QZPHONE_CONTACT_SLOTS && n < max; c++)
        if (MP_QzPhone_Registered(c))
            out[n++] = c;
    return n;
}

static const u8 sEmpty[] = _("");

const u8 *MP_QzPhone_Name(u8 c)
{
    return IsContact(c) ? sQzPhoneContacts[c].name : sEmpty;
}

const u8 *MP_QzPhone_ClassName(u8 c)
{
    return IsContact(c) ? sQzPhoneContacts[c].className : sEmpty;
}

// The words of calling contact C (Crystal's callee script), pages separated by \p
void MP_QzPhone_Call(u8 c, u8 *out, u16 size)
{
    out[0] = EOS;
    if (!IsContact(c))
        return;
    Start(sQzPhoneContacts[c].callee, c, MODE_LINES, Random32());
    sVm.lines = out;
    sVm.linesSize = size;
    Run();
    sVm.lines = NULL;
}

// ---- incoming calls: a registered contact rings now and then while walking in JOHTO
static u8 sCaller;
static u32 sCallSeed;

static bool8 CanRing(void)
{
    u8 region = MP_Region_OfGroup(gSaveBlock1Ptr->location.mapGroup);   // (Crystal's ring in KANTO too)
    return (region == REGION_JOHTO || region == REGION_KANTO) && !gMain.inBattle;
}

// Each step in the field (field_control_avatar.c): TRUE when a call starts (its field script set up)
bool8 MP_QzPhone_TryCall(void)
{
    u8 numbers[QZPHONE_CONTACT_SLOTS], n, c, k;
    u16 steps = VarGet(VAR_QZPHONE_STEPS);
    if (!CanRing())
        return FALSE;
    n = MP_QzPhone_Numbers(numbers, ARRAY_COUNT(numbers));
    if (n == 0)
        return FALSE;
    if (steps == 0)
        steps = 200 + Random() % 300;   // (a call every few hundred steps)
    if (--steps)
    {
        VarSet(VAR_QZPHONE_STEPS, steps);
        return FALSE;
    }
    VarSet(VAR_QZPHONE_STEPS, 0);
    for (k = 0; k < 4; k++)
    {
        u16 save[QZPHONE_STATE_WORDS];
        c = numbers[Random() % n];
        if (sQzPhoneContacts[c].map == ((gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum))
            continue;   // (no call from someone on this map, as Crystal)
        sCallSeed = Random32();
        // MOENAV CALLS: BATTLES: only a call that asks for a battle (played silently first; the state put back)
        if (gMpConfig.moenavCalls)
        {
            memcpy(save, MP_Region_VarPointer(VAR_QZPHONE_NUMBERS), sizeof(save));
            Start(sQzPhoneContacts[c].caller, c, MODE_SILENT, sCallSeed);
            Run();
            memcpy(MP_Region_VarPointer(VAR_QZPHONE_NUMBERS), save, sizeof(save));
            if (!sVm.wantsBattle)
                continue;
        }
        sCaller = c;
        Start(sQzPhoneContacts[c].caller, c, MODE_FIELD, sCallSeed);
        printf("[mp] qzphone: %u calls\n", c);
        return TRUE;
    }
    return FALSE;
}

// qz_phone.inc's call: who calls (gStringVar1)
void QzPhone_CallerName(void)
{
    StringCopy(gStringVar1, sQzPhoneContacts[sCaller].name);
}

// ---- debug (mp_debug.c)
void MP_QzPhone_Debug(const char *arg)
{
    unsigned c = 0, v = 0;
    char word[16] = "";
    sscanf(arg, "%15s %u %u", word, &c, &v);
    if (strcmp(word, "register") == 0 && IsContact(c))
        SetBit(VAR_QZPHONE_NUMBERS, c, TRUE);
    else if (strcmp(word, "ready") == 0 && IsContact(c) && sQzPhoneContacts[c].rematchFlag != 0xFF)
        SetBit(VAR_QZPHONE_FLAGS, sQzPhoneContacts[c].rematchFlag, TRUE);
    else if (strcmp(word, "fights") == 0 && IsContact(c))
        SetFightCount(c, v);
    else if (strcmp(word, "flag") == 0 && c < QZPHONE_FLAGS)   // (flag IDX V: one of Crystal's phone flags, QZFLAG_*)
        SetBit(VAR_QZPHONE_FLAGS, c, v);
    else if (strcmp(word, "moenav") == 0)   // (OPTIONS > CO-OP > MOENAV CALLS: 1 BATTLES)
        gMpConfig.moenavCalls = c;
    else if (strcmp(word, "steps") == 0)
        VarSet(VAR_QZPHONE_STEPS, c);
    else if (strcmp(word, "call") == 0 && IsContact(c))
    {
        static u8 sBuf[0x400];
        MP_QzPhone_Call(c, sBuf, sizeof(sBuf));
        printf("[test] qzphone call %u: %u bytes\n", c, (unsigned)StringLength(sBuf));
    }
    printf("[test] qzphone %u: registered %d fights %u ready %d\n", c, MP_QzPhone_Registered(c), FightCount(c),
           IsContact(c) && sQzPhoneContacts[c].rematchFlag != 0xFF ? Bit(VAR_QZPHONE_FLAGS, sQzPhoneContacts[c].rematchFlag) : -1);
}
