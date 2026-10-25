// Moemon Co-op: MOéMON GARDEN in battle (docs/POKEMON_GARDEN.md 9).
//
// Presentation only: nothing here changes a battle. The battle is watched through the party data every
// client keeps up to date (the HP the controllers write, gBattlerPartyIndexes, the attacker sent with each
// command), so it works the same for the host of a link battle and for everyone else.
//   - Standout moments become memories: hanging on at low HP, fainting, the knockout that won it, a
//     comeback. Routine wins only add a little confidence (TRAIT_CONFIDENT), so grinding floods nothing.
//   - Partners (Double / Triple Battles, co-op) grow trust and respect; a save in the nick of time is a
//     memory for both.
//   - Notable foes (rivals, bosses, legends, a friend's Pokémon in PvP) earn respect, and recurring ones
//     become rivals. Friction only turns into dislike for competitive temperaments.
//   - Pokémon battle emotes: the round pink bubbles of the field (mp_life_emote.c) on the Pokémon's own
//     sprite, with a quiet bloop (mp_life_sound.c). The Trainers' battle emotes are white boxes, silent.
//   - The exceptionally rare battle lines: only for the strongest memories (a best friend saved at low HP,
//     a long rivalry, the first Gym, the CHAMPION, a boss comeback) and at most one per 5 hours of play
//     for the whole save. Pink, in the message box after the moment's own message, with a chime; they
//     move on by themselves at the battle's text speed.
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "mp_life.h"
#include "mp_life_battle.h"
#include "mp_life_emote.h"
#include "mp_life_sound.h"
#include "battle.h"
#include "battle_anim.h"
#include "battle_main.h"
#include "util.h"
#include "battle_controllers.h"
#include "battle_message.h"
#include "battle_setup.h"
#include "data.h"
#include "event_data.h"
#include "main.h"
#include "palette.h"
#include "pokemon.h"
#include "string_util.h"
#include "text.h"
#include "constants/battle_string_ids.h"
#include "constants/flags.h"
#include "constants/rgb.h"
#include "constants/trainers.h"

extern const struct Evolution gEvolutionTable[][EVOS_PER_MON];

#define LINE_GAP_MINUTES   300     // one rare line per 5 hours of play, at most (docs 9)
#define LINE_TIMEOUT       900     // a line its moment's message never came for is dropped (frames)
#define EMOTE_GAP          150     // frames between two emotes of one Pokémon
#define EMOTES_PER_BATTLE  6

bool8 gMpBattleTextPink;

enum { LB_SKIP, LB_WILD, LB_TRAINER, LB_BOSS, LB_LEGEND, LB_PVP };

struct LbSlot
{
    u32 key;            // bond key (0: none)
    u32 personality;
    u16 species;
    u16 lastHp, maxHp;
    bool8 seen, mine, fainted, hungOn, onePoint, emotedLow, behindPart, notable, facedOnce;
    u8 lowestPct;
    u8 kos;
    u8 emotes;
    u16 lastEmote;      // frame of its last emote
    u8 faced;           // (ours) foe slots it stood against
    u8 together;        // (ours) party slots on the field with it
    u8 kodBy;           // (foe) our party slot + 1 that knocked it out
    u8 kodMine;         // (foe) our party slots it knocked out
    u8 savedBy;         // (ours) slot + 1 of the partner that saved it
    u8 saved;           // (ours) slot + 1 of the partner it saved
    u8 name[POKEMON_NAME_LENGTH + 1];   // short name for lines
};

static struct
{
    bool8 active;
    u8 kind;
    u8 imp;             // how much this battle means (added to memory strength)
    u8 badgesAtStart;
    u16 trainerA, trainerB;
    u16 frames;
    bool8 behind, won, lost;
    u8 finalKoBy;       // our slot + 1
    u8 emotes;
    struct LbSlot ally[PARTY_SIZE];   // the player side's party slots (ours and partners')
    struct LbSlot foe[PARTY_SIZE];
} sB;

enum { LINE_IDLE, LINE_QUEUED, LINE_READ_FIRST, LINE_PRINTING };
enum { AFTER_FAINT, AFTER_WIN };

static struct
{
    u8 state;
    u8 after;
    u8 battler;
    u16 timer;
    u32 key;            // whose line (its record notes the minute)
    u8 text[160];
} sLine;

static bool8 sLineForce;   // tests: no 5-hour gap

// ---------------------------------------------------------------------------
// Helpers

static bool8 IsBossClass(u8 cls)
{
    switch (cls)
    {
    case TRAINER_CLASS_LEADER: case TRAINER_CLASS_ELITE_FOUR: case TRAINER_CLASS_CHAMPION: case TRAINER_CLASS_RIVAL:
    case TRAINER_CLASS_MAGMA_LEADER: case TRAINER_CLASS_AQUA_LEADER: case TRAINER_CLASS_MAGMA_ADMIN:
    case TRAINER_CLASS_AQUA_ADMIN:
        return TRUE;
    }
    return FALSE;
}

static u8 TrainerClassOf(u16 trainer)
{
    return trainer != 0 && trainer < TRAINERS_COUNT ? gTrainers[trainer].trainerClass : 0;
}

// The first species of its evolution family (May's TORCHIC and later COMBUSKEN are one rival)
static u16 FamilyBase(u16 species)
{
    u8 step;
    for (step = 0; step < 3; step++)
    {
        u16 j;
        u8 k;
        bool8 found = FALSE;
        for (j = 1; j < NUM_SPECIES && !found; j++)
            for (k = 0; k < EVOS_PER_MON; k++)
                if (gEvolutionTable[j][k].targetSpecies == species)
                {
                    species = j;
                    found = TRUE;
                    break;
                }
        if (!found)
            break;
    }
    return species;
}

// (at most max letters: a name that fills its array has no end marker)
static u32 HashText(u32 h, const u8 *text, u8 max)
{
    while (max-- && *text != EOS)
        h = (h ^ *text++) * 16777619u;
    return h;
}

static u8 Pct(u16 hp, u16 maxHp)
{
    return maxHp ? (u8)((u32)hp * 100 / maxHp) : 0;
}

static bool8 IsLocalBattler(u8 b)
{
    if (GetBattlerSide(b) != B_SIDE_PLAYER)
        return FALSE;
    if (gMpCoopBattle)
        return MP_Coop_BattlerPlayer(b) == gMpLocalId;
    if ((gBattleTypeFlags & BATTLE_TYPE_INGAME_PARTNER) && GetBattlerPosition(b) == B_POSITION_PLAYER_RIGHT)
        return FALSE;   // STEVEN's
    return TRUE;
}

static u8 AbleCount(struct Pokemon *party)
{
    u8 i, n = 0;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (GetMonData(&party[i], MON_DATA_SPECIES) != SPECIES_NONE && !GetMonData(&party[i], MON_DATA_IS_EGG)
         && GetMonData(&party[i], MON_DATA_HP) > 0)
            n++;
    }
    return n;
}

static struct MpLifeRec *RecOf(struct LbSlot *s)
{
    return s->mine && s->key ? MP_Life_FindKey(s->key) : NULL;
}

static u8 BattlerOfSlot(u8 side, u8 idx)
{
    u8 b;
    for (b = 0; b < gBattlersCount; b++)
        if (GetBattlerSide(b) == side && gBattlerPartyIndexes[b] == idx && !(gAbsentBattlerFlags & gBitTable[b]))
            return b;
    return MAX_BATTLERS_COUNT;
}

// ---------------------------------------------------------------------------
// Pokémon battle emotes

static void Emote(struct LbSlot *s, u8 icon, bool8 happy)
{
    u8 idx = s - sB.ally, b;
    if (!s->mine || s->fainted || sB.emotes >= EMOTES_PER_BATTLE)
        return;
    if (s->emotes && sB.frames - s->lastEmote < EMOTE_GAP)
        return;
    b = BattlerOfSlot(B_SIDE_PLAYER, idx);
    if (b >= MAX_BATTLERS_COUNT)
        return;
    s->emotes++;
    s->lastEmote = sB.frames;
    sB.emotes++;
    MP_LifeEmote_Show(LIFE_EMOTE_SLOT_BATTLER(b), icon);
    MP_LifeSfx_Play(happy ? LIFE_SFX_BLOOP_UP : LIFE_SFX_BLOOP_DOWN);
    MPNet_Log("life battle: emote %u on battler %u", icon, b);
    if (gBattleTypeFlags & BATTLE_TYPE_LINK)
    {
        // the others in the battle see it over this Pokémon too (battler ids are the same on every game)
        u8 buf[4] = {MPR_BATTLE_MONEMOTE, b, icon, happy};
        MP_Battle_SendToPeers(buf, sizeof(buf));
    }
}

// mp_battle.c: a friend's (or the PvP opponent's) Pokémon emoted on their game
void MP_LifeBattle_OnPeerEmote(u8 battler, u8 icon, bool8 happy)
{
    if (battler >= gBattlersCount || battler >= MAX_BATTLERS_COUNT || icon >= LIFE_EMOTE_COUNT || icon == LIFE_EMOTE_NONE
     || (gAbsentBattlerFlags & gBitTable[battler]))
        return;
    MP_LifeEmote_Show(LIFE_EMOTE_SLOT_BATTLER(battler), icon);
    MP_LifeSfx_Play(happy ? LIFE_SFX_BLOOP_UP : LIFE_SFX_BLOOP_DOWN);
    MPNet_Log("life battle: a friend's emote %u on battler %u", icon, battler);
}

static void PlaceEmotes(void)
{
    u8 b;
    for (b = 0; b < MAX_BATTLERS_COUNT; b++)
    {
        u8 id = b < gBattlersCount ? gBattlerSpriteIds[b] : MAX_SPRITES;
        struct Sprite *sp;
        bool8 visible = id < MAX_SPRITES && !(gAbsentBattlerFlags & gBitTable[b]);
        if (visible)
        {
            sp = &gSprites[id];
            visible = sp->inUse && !sp->invisible;
        }
        if (!visible)
        {
            MP_LifeEmote_Place(LIFE_EMOTE_SLOT_BATTLER(b), FALSE, 0, 0, 0);
            continue;
        }
        // at the top corner of the sprite away from the screen's edge
        MP_LifeEmote_Place(LIFE_EMOTE_SLOT_BATTLER(b), TRUE, sp->x + sp->x2 + 18, sp->y + sp->y2 - 22, 1);
    }
}

// ---------------------------------------------------------------------------
// Rare lines

static bool8 LineAllowed(void)
{
    u32 last = MP_Life_LastLineMin(), now = MP_Life_PlayMinutes();
    if (sLine.state != LINE_IDLE)
        return FALSE;
    if (sLineForce)
        return TRUE;
    return last == 0 || now >= last + LINE_GAP_MINUTES;
}

// Breaks a line at a space so it fits the message box (two lines)
static void FitLine(u8 *text)
{
    int len = StringLength(text), i, lastSpace = -1;
    for (i = 0; i < len; i++)
    {
        u8 c = text[i];
        if (c == CHAR_SPACE)
        {
            u8 save = text[i];
            text[i] = EOS;
            if (GetStringWidth(FONT_NORMAL, text, 0) > 204)
            {
                text[i] = save;
                if (lastSpace >= 0)
                    text[lastSpace] = CHAR_NEWLINE;
                return;
            }
            text[i] = save;
            lastSpace = i;
        }
    }
    if (GetStringWidth(FONT_NORMAL, text, 0) > 204 && lastSpace >= 0)
        text[lastSpace] = CHAR_NEWLINE;
}

enum { LN_SAVED, LN_SAVED_BY, LN_FIRST_GYM, LN_CHAMPION, LN_RIVAL, LN_COMEBACK, LN_COUNT };

// {STR_VAR_1} = the Pokémon, {STR_VAR_2} = the other one. Three temperaments each: bold / warm / proud and
// a default (index 3).
static const u8 sTxt_1[] = _("{EMOJI_NOTE} {STR_VAR_1} stood firm in front of {STR_VAR_2}!");
static const u8 sTxt_2[] = _("{EMOJI_NOTE} {STR_VAR_1} rushed to protect {STR_VAR_2}!");
static const u8 sTxt_3[] = _("{EMOJI_NOTE} {STR_VAR_1} puffs up. Nobody hurts {STR_VAR_2}!");
static const u8 sTxt_4[] = _("{EMOJI_NOTE} {STR_VAR_1} stood between {STR_VAR_2} and the foe!");
static const u8 sTxt_5[] = _("{EMOJI_NOTE} {STR_VAR_1} and {STR_VAR_2} trade a fierce grin!");
static const u8 sTxt_6[] = _("{EMOJI_NOTE} {STR_VAR_1} looks at {STR_VAR_2} with shining eyes!");
static const u8 sTxt_7[] = _("{EMOJI_NOTE} {STR_VAR_1} pretends it had it handled... but thanks {STR_VAR_2}.");
static const u8 sTxt_8[] = _("{EMOJI_NOTE} {STR_VAR_1} won't forget how {STR_VAR_2} stepped in!");
static const u8 sTxt_9[] = _("{EMOJI_NOTE} {STR_VAR_1} stands tall beside you!");
static const u8 sTxt_10[] = _("{EMOJI_NOTE} {STR_VAR_1} looks back at you, beaming!");
static const u8 sTxt_11[] = _("{EMOJI_NOTE} {STR_VAR_1} strikes a pose. Your first GYM, won together!");
static const u8 sTxt_12[] = _("{EMOJI_NOTE} {STR_VAR_1} can't stop grinning at you!");
static const u8 sTxt_13[] = _("{EMOJI_NOTE} {STR_VAR_1} lets out a cry that fills the room!");
static const u8 sTxt_14[] = _("{EMOJI_NOTE} {STR_VAR_1} runs to you. You did it, together!");
static const u8 sTxt_15[] = _("{EMOJI_NOTE} {STR_VAR_1} knew you two could do it!");
static const u8 sTxt_16[] = _("{EMOJI_NOTE} {STR_VAR_1} can hardly believe it... CHAMPION!");
static const u8 sTxt_17[] = _("{EMOJI_NOTE} {STR_VAR_1} and {STR_VAR_2} nod. Same time next time.");
static const u8 sTxt_18[] = _("{EMOJI_NOTE} {STR_VAR_1} and {STR_VAR_2} share a look. What a rivalry!");
static const u8 sTxt_19[] = _("{EMOJI_NOTE} {STR_VAR_1} grins at {STR_VAR_2}. It's still not over!");
static const u8 sTxt_20[] = _("{EMOJI_NOTE} {STR_VAR_1} and {STR_VAR_2} have come a long way!");
static const u8 sTxt_21[] = _("{EMOJI_NOTE} {STR_VAR_1} never doubted it for a second!");
static const u8 sTxt_22[] = _("{EMOJI_NOTE} {STR_VAR_1} pulled through for everyone!");
static const u8 sTxt_23[] = _("{EMOJI_NOTE} {STR_VAR_1} turned it all around. And it knows it!");
static const u8 sTxt_24[] = _("{EMOJI_NOTE} {STR_VAR_1} is shaking... but it did it!");
static const u8 sTxt_25[] = _("'s ");

static const u8 *const sLines[LN_COUNT][4] = {
    [LN_SAVED] = {
        sTxt_1,
        sTxt_2,
        sTxt_3,
        sTxt_4,
    },
    [LN_SAVED_BY] = {
        sTxt_5,
        sTxt_6,
        sTxt_7,
        sTxt_8,
    },
    [LN_FIRST_GYM] = {
        sTxt_9,
        sTxt_10,
        sTxt_11,
        sTxt_12,
    },
    [LN_CHAMPION] = {
        sTxt_13,
        sTxt_14,
        sTxt_15,
        sTxt_16,
    },
    [LN_RIVAL] = {
        sTxt_17,
        sTxt_18,
        sTxt_19,
        sTxt_20,
    },
    [LN_COMEBACK] = {
        sTxt_21,
        sTxt_22,
        sTxt_23,
        sTxt_24,
    },
};

static u8 Flavor(struct MpLifeRec *rec)
{
    if (rec == NULL)
        return 3;
    if (rec->cur[LIFE_PROUD] >= 170 && rec->cur[LIFE_PROUD] >= rec->cur[LIFE_WARM])
        return 2;
    if (rec->cur[LIFE_WARM] >= 160)
        return 1;
    if (rec->cur[LIFE_BOLD] >= 170)
        return 0;
    return 3;
}

static void QueueLine(u8 kind, struct LbSlot *who, const u8 *other, u8 after)
{
    struct MpLifeRec *rec = RecOf(who);
    if (rec == NULL || !LineAllowed())
        return;
    StringCopy(gStringVar1, who->name);
    StringCopy(gStringVar2, other ? other : who->name);
    StringExpandPlaceholders(sLine.text, sLines[kind][Flavor(rec)]);
    FitLine(sLine.text);
    sLine.state = LINE_QUEUED;
    sLine.after = after;
    sLine.timer = 0;
    sLine.key = who->key;
    MP_Life_SetLastLineMin(MP_Life_PlayMinutes() ? MP_Life_PlayMinutes() : 1);
    rec->lastLineMin = MP_Life_PlayMinutes();
    MPNet_Log("life battle: line %u queued", kind);
}

static bool8 LineMatches(u16 stringId)
{
    if (sLine.after == AFTER_FAINT)
        return stringId == STRINGID_TARGETFAINTED || stringId == STRINGID_ATTACKERFAINTED;
    return stringId == STRINGID_PLAYERDEFEATEDTRAINER1 || stringId == STRINGID_TWOENEMIESDEFEATED
        || stringId == STRINGID_BATTLEEND;
}

// Called by the battle controllers when a message finished printing. A queued line waits for its moment's
// message (the foe fainting, "... defeated ...!"), lets it be read a moment like any message, then prints in
// pink with a chime. TRUE while it's showing (the controller completes after).
bool8 MP_LifeBattle_TextHold(void)
{
    static const u16 sPink = RGB(31, 22, 26);
    if (sLine.state == LINE_IDLE)
        return FALSE;
    if (sLine.state == LINE_QUEUED)
    {
        u16 stringId;
        if (gBattleBufferA[gActiveBattler][0] != CONTROLLER_PRINTSTRING)
            return FALSE;
        stringId = gBattleBufferA[gActiveBattler][2] | (gBattleBufferA[gActiveBattler][3] << 8);
        if (!LineMatches(stringId))
            return FALSE;
        sLine.battler = gActiveBattler;
        sLine.state = LINE_READ_FIRST;
        sLine.timer = 0;
    }
    if (gActiveBattler != sLine.battler)
        return FALSE;
    switch (sLine.state)
    {
    case LINE_READ_FIRST:
        sLine.timer++;
        if (sLine.timer < 24 && !(sLine.timer > 8 && !(gBattleTypeFlags & BATTLE_TYPE_LINK) && JOY_NEW(A_BUTTON | B_BUTTON)))
            return TRUE;
        LoadPalette(&sPink, BG_PLTT_ID(0) + 8, sizeof(sPink));
        gMpBattleTextPink = TRUE;
        BattlePutTextOnWindow(sLine.text, B_WIN_MSG);
        gMpBattleTextPink = FALSE;
        MP_LifeSfx_Play(LIFE_SFX_CHIME);
        sLine.state = LINE_PRINTING;
        {
            char ascii[160];
            MP_GameToAscii(ascii, sLine.text, sizeof(ascii) - 1);
            MPNet_Log("life battle: line shown: %s", ascii);
        }
        return TRUE;
    case LINE_PRINTING:
        if (IsTextPrinterActive(B_WIN_MSG))
            return TRUE;
        sLine.state = LINE_IDLE;
        return FALSE;
    }
    return FALSE;
}

// ---------------------------------------------------------------------------
// Following the battle

static void Begin(void)
{
    u8 i;
    memset(&sB, 0, sizeof(sB));
    memset(&sLine, 0, sizeof(sLine));
    sB.active = TRUE;
    for (i = 0; i < 8; i++)
        if (FlagGet(FLAG_BADGE01_GET + i))
            sB.badgesAtStart++;
    if (gBattleTypeFlags & (BATTLE_TYPE_RECORDED | BATTLE_TYPE_RECORDED_LINK | BATTLE_TYPE_WALLY_TUTORIAL | BATTLE_TYPE_SAFARI))
        sB.kind = LB_SKIP;
    else if ((gBattleTypeFlags & BATTLE_TYPE_LINK) && !gMpCoopBattle)
        sB.kind = LB_PVP;
    else if (gMpCoopWild || !(gBattleTypeFlags & BATTLE_TYPE_TRAINER))
        sB.kind = (gBattleTypeFlags & (BATTLE_TYPE_LEGENDARY | BATTLE_TYPE_KYOGRE_GROUDON | BATTLE_TYPE_REGI | BATTLE_TYPE_GROUDON
                                       | BATTLE_TYPE_KYOGRE | BATTLE_TYPE_RAYQUAZA | BATTLE_TYPE_ROAMER)) ? LB_LEGEND : LB_WILD;
    else
    {
        sB.trainerA = gTrainerBattleOpponent_A;
        sB.trainerB = (gBattleTypeFlags & BATTLE_TYPE_TWO_OPPONENTS) ? gTrainerBattleOpponent_B : 0;
        if (gBattleTypeFlags & (BATTLE_TYPE_FRONTIER | BATTLE_TYPE_SECRET_BASE | BATTLE_TYPE_TRAINER_HILL | BATTLE_TYPE_EREADER_TRAINER))
            sB.trainerA = sB.trainerB = 0;   // no lasting identity
        sB.kind = IsBossClass(TrainerClassOf(sB.trainerA)) || IsBossClass(TrainerClassOf(sB.trainerB)) ? LB_BOSS : LB_TRAINER;
    }
    switch (sB.kind)
    {
    case LB_TRAINER: sB.imp = 25; break;
    case LB_BOSS: case LB_LEGEND: sB.imp = 70; break;
    case LB_PVP: sB.imp = 50; break;
    default: sB.imp = 0; break;
    }
    for (i = 0; i < MAX_BATTLERS_COUNT; i++)
        MP_LifeEmote_Place(LIFE_EMOTE_SLOT_BATTLER(i), FALSE, 0, 0, 0);
    MPNet_Log("life battle: begin kind %u", sB.kind);
}

// At most max letters of src, then an end marker; returns where the marker is. Names are copied this way
// only: a name that fills its array has no end marker, and an unbounded copy ran past this buffer and
// over the stack (the 1.15.0 crash on opening the party menu in a co-op battle, to switch or use REVIVE).
static u8 *CopyName(u8 *dst, const u8 *src, u8 max)
{
    u8 i;
    for (i = 0; i < max && src[i] != EOS; i++)
        *dst++ = src[i];
    *dst = EOS;
    return dst;
}

#define OWNER_MAX 12   // >= MP_NAME_LEN, PLAYER_NAME_LENGTH, TRAINER_NAME_LENGTH

// owner: an end-marked name of at most OWNER_MAX letters (or NULL)
static void NoteOwnedName(u32 key, const u8 *owner, const u8 *name)
{
    u8 buf[OWNER_MAX + 3 + POKEMON_NAME_LENGTH + 1];
    u8 *end = buf;
    if (owner && owner[0] != EOS)
    {
        end = CopyName(buf, owner, OWNER_MAX);
        end = CopyName(end, sTxt_25, 3);   // "'s "
    }
    CopyName(end, name, POKEMON_NAME_LENGTH);
    MP_Life_NoteName(key, buf);
}

// A player's name into buf (OWNER_MAX + 1), or NULL. Our own entry in gMpPlayers is never filled in, and a
// slot nobody is in is cleared: ours comes from the save.
static const u8 *PlayerName(s8 p, u8 *buf)
{
    if (p < 0 || p >= MP_MAX_PLAYERS)
        return NULL;
    if (p == gMpLocalId)
    {
        CopyName(buf, gSaveBlock2Ptr->playerName, PLAYER_NAME_LENGTH);
        return buf;
    }
    if (!gMpPlayers[p].present)
        return NULL;
    MP_CopyPlayerName(buf, p);
    return buf;
}

static void InitSlot(struct LbSlot *s, u8 b, struct Pokemon *mon, u8 side)
{
    u32 personality = GetMonData(mon, MON_DATA_PERSONALITY), otId = GetMonData(mon, MON_DATA_OT_ID);
    memset(s, 0, sizeof(*s));
    s->seen = TRUE;
    s->personality = personality;
    s->species = GetMonData(mon, MON_DATA_SPECIES);
    s->lastHp = GetMonData(mon, MON_DATA_HP);
    s->maxHp = GetMonData(mon, MON_DATA_MAX_HP);
    s->lowestPct = Pct(s->lastHp, s->maxHp);
    GetMonData(mon, MON_DATA_NICKNAME, s->name);
    s->name[POKEMON_NAME_LENGTH] = EOS;
    StringGet_Nickname(s->name);
    if (side == B_SIDE_PLAYER)
    {
        s->key = MP_Life_MonKey(personality, otId);
        s->mine = IsLocalBattler(b) && MP_Life_FindKey(s->key) != NULL;
        if (!s->mine)
        {
            // a friend's (co-op) or STEVEN's Pokémon: remembered by its Trainer's name
            u8 owner[OWNER_MAX + 1];
            NoteOwnedName(s->key, PlayerName(MP_Coop_BattlerPlayer(b), owner), s->name);
        }
        return;
    }
    switch (sB.kind)
    {
    case LB_PVP:
    {
        u8 owner[OWNER_MAX + 1];
        s->key = MP_Life_MonKey(personality, otId);
        s->notable = TRUE;
        NoteOwnedName(s->key, PlayerName(MP_Battle_PeerId(), owner), s->name);
        break;
    }
    case LB_LEGEND:
        s->key = MP_Life_MonKey(personality, otId);
        s->notable = TRUE;
        NoteOwnedName(s->key, NULL, gSpeciesNames[s->species]);
        break;
    case LB_TRAINER:
    case LB_BOSS:
    {
        // a trainer's Pokémon: the trainer and the evolution family (the same rival across the story)
        u16 trainer = (sB.trainerB && GetBattlerPosition(b) == B_POSITION_OPPONENT_RIGHT) ? sB.trainerB : sB.trainerA;
        u8 owner[OWNER_MAX + 1];
        if (trainer == 0)
            break;
        s->key = HashText(2166136261u ^ FamilyBase(s->species), gTrainers[trainer].trainerName, TRAINER_NAME_LENGTH + 1) & 0x7FFFFFFF;
        if (s->key == 0)
            s->key = 1;
        s->notable = IsBossClass(gTrainers[trainer].trainerClass);
        CopyName(s->name, gSpeciesNames[s->species], POKEMON_NAME_LENGTH);
        CopyName(owner, gTrainers[trainer].trainerName, TRAINER_NAME_LENGTH);
        NoteOwnedName(s->key, owner, gSpeciesNames[s->species]);
        break;
    }
    }
}

// Its partner was knocked out in front of it
static void AllyFainted(u8 idx)
{
    u8 i;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct LbSlot *o = &sB.ally[i];
        struct MpLifeBond *bond;
        if (i == idx || !o->mine || o->fainted || BattlerOfSlot(B_SIDE_PLAYER, i) >= MAX_BATTLERS_COUNT)
            continue;
        bond = MP_Life_Bond(o->key, sB.ally[idx].key, FALSE);
        if (bond && bond->v[BOND_AFFECTION] >= 90)
        {
            struct MpLifeRec *rec = RecOf(o);
            if (rec)
                MP_Life_Feel(rec, EMO_SADNESS, 25);
            Emote(o, LIFE_EMOTE_TEAR, FALSE);
        }
    }
}

static bool8 IsBestFriend(u32 from, u32 to)
{
    u32 fr, rv, fa, fv;
    MP_Life_Closest(from, &fr, &rv, &fa, &fv);
    return fr == to;
}

// A knocked out the foe while its partner Y was in trouble
static void PartnerSave(u8 aIdx, u8 yIdx)
{
    struct LbSlot *a = &sB.ally[aIdx], *y = &sB.ally[yIdx];
    bool8 close = (u32)y->lastHp * 8 <= y->maxHp;
    if (a->key == 0 || y->key == 0)
        return;
    if (a->mine)
    {
        a->saved = yIdx + 1;
        Emote(a, LIFE_EMOTE_SPARKLE, TRUE);
    }
    if (y->mine)
    {
        y->savedBy = aIdx + 1;
        Emote(y, LIFE_EMOTE_HEART, TRUE);
    }
    MPNet_Log("life battle: slot %u saved slot %u%s", aIdx, yIdx, close ? " (close)" : "");
    if (!close)
        return;
    if (y->mine && IsBestFriend(y->key, a->key))
        QueueLine(LN_SAVED_BY, y, a->name, AFTER_FAINT);
    else if (a->mine && IsBestFriend(a->key, y->key))
        QueueLine(LN_SAVED, a, y->name, AFTER_FAINT);
}

static void OnWin(void);

static void OnHp(struct LbSlot *s, u8 side, u8 idx, u16 hp)
{
    u16 old = s->lastHp;
    s->lastHp = hp;
    if (Pct(hp, s->maxHp) < s->lowestPct)
        s->lowestPct = Pct(hp, s->maxHp);
    if (side == B_SIDE_PLAYER)
    {
        if (hp == 0 && old > 0)
        {
            u8 a = gBattlerAttacker;
            s->fainted = TRUE;
            if (a < gBattlersCount && GetBattlerSide(a) == B_SIDE_OPPONENT && gBattlerPartyIndexes[a] < PARTY_SIZE)
                sB.foe[gBattlerPartyIndexes[a]].kodMine |= 1 << idx;
            AllyFainted(idx);
        }
        else if (hp > 0 && hp < old && (u32)hp * 8 <= s->maxHp)
        {
            s->hungOn = TRUE;
            if (hp == 1)
                s->onePoint = TRUE;
            if (s->mine && !s->emotedLow)
            {
                struct MpLifeRec *rec = RecOf(s);
                s->emotedLow = TRUE;
                if (rec && rec->cur[LIFE_PROUD] >= 170)
                    Emote(s, LIFE_EMOTE_SPARKLE, TRUE);      // won't show it
                else if (rec && rec->cur[LIFE_BOLD] >= 180 && rec->cur[LIFE_WARM] < 110)
                    Emote(s, LIFE_EMOTE_ANGRY, TRUE);        // fired up
                else
                    Emote(s, LIFE_EMOTE_SWEAT, FALSE);
            }
        }
        if (!sB.behind && AbleCount(gPlayerParty) == 1 && AbleCount(gEnemyParty) >= 2 && sB.kind != LB_WILD)
            sB.behind = TRUE;
        if (!sB.lost && AbleCount(gPlayerParty) == 0)
            sB.lost = TRUE;
        return;
    }
    // a foe went down
    if (hp == 0 && old > 0)
    {
        u8 a = gBattlerAttacker, b;
        if (a < gBattlersCount && GetBattlerSide(a) == B_SIDE_PLAYER && gBattlerPartyIndexes[a] < PARTY_SIZE)
        {
            u8 aIdx = gBattlerPartyIndexes[a];
            struct LbSlot *as = &sB.ally[aIdx];
            as->kos++;
            s->kodBy = aIdx + 1;
            for (b = 0; b < gBattlersCount; b++)
            {
                u8 yIdx;
                if (b == a || GetBattlerSide(b) != B_SIDE_PLAYER || (gAbsentBattlerFlags & gBitTable[b]))
                    continue;
                yIdx = gBattlerPartyIndexes[b];
                if (yIdx < PARTY_SIZE && yIdx != aIdx && sB.ally[yIdx].seen && sB.ally[yIdx].lastHp > 0
                 && (u32)sB.ally[yIdx].lastHp * 5 <= sB.ally[yIdx].maxHp)
                    PartnerSave(aIdx, yIdx);
            }
            if (AbleCount(gEnemyParty) == 0)
                sB.finalKoBy = aIdx + 1;
            else if (as->mine && (u32)as->lastHp * 4 <= as->maxHp && sB.kind != LB_WILD)
                Emote(as, LIFE_EMOTE_SPARKLE, TRUE);
        }
        if (!sB.won && AbleCount(gEnemyParty) == 0)
        {
            sB.won = TRUE;
            OnWin();
        }
    }
}

static void Poll(void)
{
    u8 b, onField[2] = {0, 0}, i;
    for (b = 0; b < gBattlersCount; b++)
    {
        u8 side, idx;
        struct Pokemon *mon;
        struct LbSlot *s;
        u16 species, hp;
        if (gAbsentBattlerFlags & gBitTable[b])
            continue;
        idx = gBattlerPartyIndexes[b];
        if (idx >= PARTY_SIZE)
            continue;
        side = GetBattlerSide(b);
        mon = side == B_SIDE_PLAYER ? &gPlayerParty[idx] : &gEnemyParty[idx];
        species = GetMonData(mon, MON_DATA_SPECIES);
        if (species == SPECIES_NONE || GetMonData(mon, MON_DATA_IS_EGG))
            continue;
        s = side == B_SIDE_PLAYER ? &sB.ally[idx] : &sB.foe[idx];
        if (!s->seen || s->personality != GetMonData(mon, MON_DATA_PERSONALITY))
            InitSlot(s, b, mon, side);   // (a friend joining in progress moves Pokémon between slots)
        onField[side] |= 1 << idx;
        hp = GetMonData(mon, MON_DATA_HP);
        s->maxHp = GetMonData(mon, MON_DATA_MAX_HP);
        if (hp != s->lastHp)
            OnHp(s, side, idx, hp);
    }
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct LbSlot *s = &sB.ally[i];
        u8 f;
        if (!(onField[B_SIDE_PLAYER] & (1 << i)) || !s->mine || s->lastHp == 0)
            continue;
        s->together |= onField[B_SIDE_PLAYER] & ~(1 << i);
        if (sB.behind)
            s->behindPart = TRUE;
        for (f = 0; f < PARTY_SIZE; f++)
        {
            struct LbSlot *fs = &sB.foe[f];
            if (!(onField[B_SIDE_OPPONENT] & (1 << f)) || fs->lastHp == 0)
                continue;
            if (!(s->faced & (1 << f)) && fs->key)
            {
                // meeting a rival again, or a legend it's afraid of
                struct MpLifeBond *bond = MP_Life_Bond(s->key, fs->key, FALSE);
                struct MpLifeRec *rec = RecOf(s);
                if (bond && (bond->flags & LIFE_BOND_RIVAL))
                    Emote(s, bond->v[BOND_FRICTION] >= 80 ? LIFE_EMOTE_ANGRY : LIFE_EMOTE_EXCLAIM, TRUE);
                else if (sB.kind == LB_LEGEND && rec && MP_Life_FindTrait(rec, TRAIT_AFRAID, LIFE_TRIG_LEGEND))
                    Emote(s, LIFE_EMOTE_SWEAT, FALSE);
            }
            s->faced |= 1 << f;
        }
    }
}

static struct LbSlot *LeadMine(void)
{
    u8 i;
    if (sB.finalKoBy && sB.ally[sB.finalKoBy - 1].mine)
        return &sB.ally[sB.finalKoBy - 1];
    for (i = 0; i < PARTY_SIZE; i++)
        if (sB.ally[i].mine && !sB.ally[i].fainted && BattlerOfSlot(B_SIDE_PLAYER, i) < MAX_BATTLERS_COUNT)
            return &sB.ally[i];
    return NULL;
}

// The last foe went down: the lines for the strongest wins, and the winners' faces
static void OnWin(void)
{
    struct LbSlot *lead = LeadMine();
    u8 clsA = TrainerClassOf(sB.trainerA), i;
    MPNet_Log("life battle: won (final knockout by slot %d)", (int)sB.finalKoBy - 1);
    if (lead)
    {
        if (clsA == TRAINER_CLASS_CHAMPION)
            QueueLine(LN_CHAMPION, lead, NULL, AFTER_WIN);
        else if (clsA == TRAINER_CLASS_LEADER && sB.badgesAtStart == 0)
            QueueLine(LN_FIRST_GYM, lead, NULL, AFTER_WIN);
        else if (sB.behind && lead->behindPart && (sB.kind == LB_BOSS || sB.kind == LB_PVP) && lead->lowestPct <= 25)
            QueueLine(LN_COMEBACK, lead, NULL, AFTER_WIN);
        else
        {
            // a long rivalry: a foe it has faced many times (this battle included)
            u8 f;
            for (f = 0; f < PARTY_SIZE; f++)
            {
                struct MpLifeBond *bond;
                if (!(lead->faced & (1 << f)) || sB.foe[f].key == 0)
                    continue;
                bond = MP_Life_Bond(lead->key, sB.foe[f].key, FALSE);
                if (bond && (bond->flags & LIFE_BOND_RIVAL) && bond->shared >= 5)
                {
                    QueueLine(LN_RIVAL, lead, sB.foe[f].name, AFTER_WIN);
                    break;
                }
            }
        }
    }
    if (sB.kind == LB_WILD)
        return;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct LbSlot *s = &sB.ally[i];
        struct MpLifeRec *rec = RecOf(s);
        if (rec == NULL || s->fainted || BattlerOfSlot(B_SIDE_PLAYER, i) >= MAX_BATTLERS_COUNT)
            continue;
        // (the one that knocked out the last foe, never just the first of ours still out: a friend's may have)
        if (sB.kind == LB_BOSS || sB.kind == LB_PVP || s->hungOn || (s == lead && (sB.finalKoBy == i + 1 || !gMpCoopBattle)))
            Emote(s, rec->cur[LIFE_PROUD] >= 160 ? LIFE_EMOTE_SPARKLE : LIFE_EMOTE_NOTE, TRUE);
    }
}

static bool8 Competitive(struct MpLifeRec *rec)
{
    return rec->cur[LIFE_PROUD] >= 170 || (rec->cur[LIFE_BOLD] >= 180 && rec->cur[LIFE_WARM] < 110);
}

// The battle is over: memories, bonds, a little confidence
static void Finish(void)
{
    u8 i, j;
    bool8 won = sB.won || gBattleOutcome == B_OUTCOME_WON;
    bool8 lost = !won && (sB.lost || gBattleOutcome == B_OUTCOME_LOST);
    u8 imp = sB.imp;
    sB.active = FALSE;
    if (sLine.state != LINE_IDLE)
    {
        // its moment's message never came: the line isn't spent
        MP_Life_SetLastLineMin(0);
        sLine.state = LINE_IDLE;
    }
    MP_LifeEmote_HideAll();
    if (sB.kind == LB_SKIP)
        return;
    MPNet_Log("life battle: end (won %d lost %d)", won, lost);
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct LbSlot *s = &sB.ally[i];
        struct MpLifeRec *rec = RecOf(s);
        bool8 standout = FALSE;
        if (rec == NULL)
            continue;
        if (s->fainted)
        {
            MP_Life_Remember(rec, MEM_FAINTED, (sB.kind == LB_WILD ? 85 : 85 + imp), TIER_PART, 0, 0);
            standout = TRUE;
        }
        if (s->hungOn && !s->fainted && won)
        {
            MP_Life_Remember(rec, MEM_NEAR_FAINT, 90 + imp + (s->onePoint ? 25 : 0), TIER_PART, 0, 0);
            standout = TRUE;
        }
        if (won && sB.finalKoBy == i + 1 && (sB.kind != LB_WILD ? (s->lowestPct <= 25 || sB.kind != LB_TRAINER) : s->lowestPct <= 12))
        {
            MP_Life_Remember(rec, MEM_CLUTCH, sB.kind == LB_WILD ? 95 : 110 + imp, TIER_PART, 0, 0);
            if (sB.kind == LB_BOSS || sB.kind == LB_PVP)
                MP_Life_AddNews(MEM_CLUTCH, sB.trainerA & 0xFF, s->key);
            standout = TRUE;
        }
        if (won && sB.behind && s->behindPart)
        {
            MP_Life_Remember(rec, MEM_COMEBACK, 150 + imp, TIER_PART, 0, 0);
            MP_Life_AddNews(MEM_COMEBACK, sB.trainerA & 0xFF, s->key);
            standout = TRUE;
        }
        if (s->savedBy)
        {
            MP_Life_Remember(rec, MEM_PARTNER, 130 + imp / 2, TIER_PART, sB.ally[s->savedBy - 1].key, 1);
            MP_Life_BondShare(s->key, sB.ally[s->savedBy - 1].key, 3, 8, 6, 5, 0);
            standout = TRUE;
        }
        if (s->saved)
        {
            u32 other = sB.ally[s->saved - 1].key;
            MP_Life_Remember(rec, MEM_PARTNER, 110 + imp / 2, TIER_PART, other, 0);
            MP_Life_AddTrait(rec, TRAIT_PROTECTIVE, (u16)other, 20);
            MP_Life_BondShare(s->key, other, 3, 4, 3, 3, 0);
            standout = TRUE;
        }
        // side by side: trust and respect grow (docs 9)
        for (j = 0; j < PARTY_SIZE; j++)
            if ((s->together & (1 << j)) && sB.ally[j].key)
                MP_Life_BondShare(s->key, sB.ally[j].key, 2, 1, 0, won ? 2 : 1, 0);
        // foes: respect for the strong, rivals from the recurring
        for (j = 0; j < PARTY_SIZE; j++)
        {
            struct LbSlot *f = &sB.foe[j];
            struct MpLifeBond *bond;
            s8 respect = 2, friction = 0;
            if (!(s->faced & (1 << j)) || f->key == 0)
                continue;
            if (!f->notable && !(f->kodMine & (1 << i)))
                continue;   // an ordinary trainer's Pokémon leaves no bond
            if (f->kodMine)
                respect += 4;
            if (sB.kind == LB_BOSS || sB.kind == LB_LEGEND)
                respect += 2;
            if (f->kodMine & (1 << i))
                friction = Competitive(rec) ? 6 : 1;
            else if (Competitive(rec) && !won)
                friction = 3;
            MP_Life_BondShare(s->key, f->key, 3, 0, 0, respect, friction);
            bond = MP_Life_Bond(s->key, f->key, FALSE);
            if (bond && bond->shared == 1)
                bond->v[BOND_RESPECT] = bond->v[BOND_RESPECT] < 245 ? bond->v[BOND_RESPECT] + 10 : 255;   // first seen across a battle (docs 5.3)
            if (bond && !(bond->flags & LIFE_BOND_RIVAL) && f->notable && bond->shared >= 3 && bond->v[BOND_RESPECT] >= 40)
            {
                bond->flags |= LIFE_BOND_RIVAL;
                MP_Life_Remember(rec, MEM_RIVAL, 150 + imp / 2, TIER_PART, f->key, 0);
                MPNet_Log("life battle: a rival (slot %u)", i);
                standout = TRUE;
            }
            else if (bond && (bond->flags & LIFE_BOND_RIVAL) && won)
            {
                MP_Life_Remember(rec, MEM_RIVAL, 120 + imp / 2, TIER_PART, f->key, 1);
            }
        }
        if (won)
        {
            MP_Life_Feel(rec, EMO_PRIDE, standout ? 30 : 12);
            MP_Life_Feel(rec, EMO_JOY, 10);
            if (!standout)
                MP_Life_AddTrait(rec, TRAIT_CONFIDENT, 0, sB.kind == LB_WILD ? 1 : 2);   // routine wins: a little confidence
        }
        else if (lost)
        {
            MP_Life_Feel(rec, EMO_FRUSTRATION, Competitive(rec) ? 40 : 20);
            MP_Life_Feel(rec, EMO_SADNESS, 15);
        }
    }
}

void MP_LifeBattle_Update(void)
{
    if (!gMain.inBattle || gSaveBlock2Ptr == NULL)
    {
        if (sB.active)
            Finish();
        return;
    }
    if (!sB.active)
        Begin();
    sB.frames++;
    if (sB.kind == LB_SKIP)
        return;
    if (gMain.callback2 == BattleMainCB2)
    {
        if (gBattlersCount > 0 && gBattlersCount <= MAX_BATTLERS_COUNT)
            Poll();
        PlaceEmotes();
    }
    else
    {
        // a menu over the battle (party, bag, summary): the battle's sprites are gone, and the party menu puts
        // the party in battle order, which isn't what gBattlerPartyIndexes point to. Wait for the battle.
        u8 b;
        for (b = 0; b < MAX_BATTLERS_COUNT; b++)
            MP_LifeEmote_Place(LIFE_EMOTE_SLOT_BATTLER(b), FALSE, 0, 0, 0);
    }
    if (sLine.state == LINE_QUEUED && ++sLine.timer > LINE_TIMEOUT)
    {
        sLine.state = LINE_IDLE;
        MP_Life_SetLastLineMin(0);
    }
}

void MP_LifeBattle_Debug(const char *args)
{
    char cmd[32] = "";
    int a = 0;
    sscanf(args, "%31s %d", cmd, &a);
    if (strcmp(cmd, "lineforce") == 0)
    {
        sLineForce = a;
        printf("[test] life battle lineforce %d\n", a);
    }
    else if (strcmp(cmd, "state") == 0)
    {
        u8 i;
        printf("[test] life battle active %d kind %u won %d lost %d behind %d emotes %u line %u\n", sB.active, sB.kind, sB.won,
               sB.lost, sB.behind, sB.emotes, sLine.state);
        for (i = 0; i < PARTY_SIZE; i++)
            if (sB.ally[i].seen)
                printf("[test] life battle ally %u mine %d hp %u/%u low %u%% kos %u fainted %d faced %02x together %02x\n", i,
                       sB.ally[i].mine, sB.ally[i].lastHp, sB.ally[i].maxHp, sB.ally[i].lowestPct, sB.ally[i].kos,
                       sB.ally[i].fainted, sB.ally[i].faced, sB.ally[i].together);
    }
}
