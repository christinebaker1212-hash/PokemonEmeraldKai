// Moemon Co-op: trades by mail, the game's side (docs/COMPANION_APP.md "Pillar 4: friends", docs/PHONE_LINK.md
// "Trades by mail").
//
// A friend's phone offers one of its POKéMON (it stays where it is, ON OFFER: no release, walk or training); this
// phone accepts with one of ours. The swap goes through the co-op server's escrow (mp_server.c ESC_*): each game
// deposits its POKéMON there, and neither can be taken until both are in, so nothing is ever lost or copied:
//   - depositing: "<save>.trades" is written first (the POKéMON as it left), then it leaves the save and the game saves
//     at once, then the server hears of it (a save that still holds it on loading: it never left);
//   - receiving: the friend's POKéMON is written down, then into the party (else the PC) and saved at once (a save
//     that already holds it on loading: it came);
//   - until both are in, either side may call it off (the phone's DECLINE / CANCEL, or 7 days without an answer):
//     a deposit comes back home the same careful way.
// Every step can be repeated (the server's answers say where the trade stands), so a game closed at any moment picks
// up where it was. Once traded, the game's own trade scene plays (and any trade evolution) the next time the player
// is free on the field. Deposits are sealed with the key only the two friends share (X25519, mp_phone_friends.c).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "global.h"
#include "orre_shadow.h"   // Moemon Kai: Shadow Pokémon
#include "mp.h"
#include "mp_life.h"
#include "main.h"
#include "event_data.h"
#include "item.h"
#include "mail.h"
#include "pokedex.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "save.h"
#include "overworld.h"
#include "sound.h"
#include "string_util.h"
#include "palette.h"
#include "tv.h"
#include "new_game.h"
#include "fieldmap.h"
#include "constants/characters.h"
#include "constants/items.h"
#include "constants/songs.h"
#include "phone_proto.h"
#include "mp_net.h"
#include "mp_phone_int.h"
#include "aw_file.h"   // (sidecars written in the background: src/platform/aw_file.c)

extern char gPlatformSavePath[512];
u8 MP_CopyMonToPC(struct Pokemon *mon);                                         // pokemon.c
bool8 MP_Lobby_ServerAddress(char *out, int len);                               // mp_main.c
bool8 MP_Phone_InTraining(u32 pid, u32 ot);                                     // mp_phone_walk.c
void MP_MailTrade_Scene(u8 partySlot, const u8 *friendName, void (*onEnd)(void));   // trade.c

#define TRADES_MAGIC    0x4452544D   // "MTRD"
#define TRADES_VERSION  1
#define TRADES_MAX      24
#define TRADE_DAYS      7
#define REC_MAX         2000         // (a Garden record that travels with a POKéMON; bigger ones stay behind)
#define DEPOSIT_VERSION 1

enum
{
    TS_NONE,
    TS_OFFERED,      // ours ON OFFER (we offered): waiting for their deposit
    TS_ACCEPTING,    // ours chosen (we accepted): to deposit
    TS_DEPOSITING,   // ours left the save: the server hasn't said it has it
    TS_DEPOSITED,    // ours in the escrow: waiting for theirs
    TS_RECEIVING,    // theirs taken: into the save
    TS_DONE,         // traded
    TS_RETURNING,    // called off after ours left: back into the save
    TS_OFF,          // called off (ours never left, or came back)
};
static const char *const sStateNames[] = {"none", "offered", "accepting", "depositing", "deposited", "receiving",
                                          "done", "returning", "off"};

struct TradeRec
{
    u8 t[16];                     // the trade's ID: its escrow key (only the two friends know it)
    u8 peer[PHONE_SAVEID_SIZE];
    u8 peerName[PLAYER_NAME_LENGTH + 1];
    u8 role;                      // 0: we offered (side 0), 1: we accepted their offer (side 1)
    u8 state;
    u8 shown;                     // (the trade scene played)
    u8 cancel;                    // (the phone asked to call it off)
    u8 why;                       // (TS_OFF: 0 called off, 1 turned down / they called it off, 2 ran out, 3 ours gone)
    u8 theirsIn;                  // (we offered: their deposit is in, ours goes as soon as the game is free)
    u8 pad[2];
    u32 created, expires, changed;
    u32 myPid, myOt;
    u16 want;                     // (role 0: the species we asked for, 0 any)
    u16 recLen;
    char srv[64];
    struct BoxPokemon mine;       // ours as it left (from TS_DEPOSITING on)
    struct BoxPokemon theirs;     // theirs as it came (from TS_RECEIVING on)
    u32 theirTrainer;             // (their Trainer ID and name: whose Garden record comes with it)
    u8 theirTrainerName[PLAYER_NAME_LENGTH + 1];
    u8 pad2[3];
    u8 rec[REC_MAX];              // theirs: its Garden record
};

static struct
{
    u32 magic, version, count;
    struct TradeRec r[TRADES_MAX];
} sTr;
static bool8 sLoaded;
static s8 sAsking = -1;           // the record whose question the server hasn't answered (one at a time)
static u32 sAskedAt, sTick;
static s8 sScene = -1;            // the record whose trade scene is playing
static u8 sSceneSlot;
static struct Pokemon sSceneBackup;
static bool8 sSceneInParty;

// --------------------------------------------------------------------------------------------- the sidecar

static void TradesPath(char *out, size_t size)
{
    snprintf(out, size, "%s.trades", gPlatformSavePath);
}

static bool8 TradesWrite(void)
{
    char path[600];
    AwFile *f;
    bool8 ok;
    TradesPath(path, sizeof(path));
    f = AwFile_Open(path);   // (written in the background, swapped in whole: aw_file.c)
    ok = AwFile_Write(&sTr, sizeof(sTr), 1, f) == 1;
    ok = AwFile_Close(f) == 0 && ok;
    return ok;
}

static struct TradeRec *Find(const u8 t[16])
{
    u32 i;
    for (i = 0; i < sTr.count; i++)
        if (memcmp(sTr.r[i].t, t, 16) == 0)
            return &sTr.r[i];
    return NULL;
}

// ------------------------------------------------------------------------------------------- the save

static bool8 SaveNow(void)
{
    if (gDifferentSaveFile && gSaveFileStatus != SAVE_STATUS_EMPTY && gSaveFileStatus != SAVE_STATUS_CORRUPT)
        return FALSE;
    SaveMapView();
    return TrySavingData(SAVE_NORMAL) == SAVE_STATUS_OK;
}

// Where this POKéMON (personality, Trainer ID) is: the party (box 0xFF) or the player's own boxes
static bool8 Where(u32 pid, u32 ot, struct Loc *loc)
{
    struct PokemonStorage *st = Storage();
    u32 b, i;
    for (i = 0; i < PARTY_SIZE; i++)
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) != SPECIES_NONE
         && GetMonData(&gPlayerParty[i], MON_DATA_PERSONALITY) == pid && GetMonData(&gPlayerParty[i], MON_DATA_OT_ID) == ot)
        {
            loc->kind = LOC_PARTY, loc->box = 0, loc->slot = i;
            return TRUE;
        }
    for (b = 0; b < TOTAL_BOXES_COUNT; b++)
        for (i = 0; i < IN_BOX_COUNT; i++)
            if (GetBoxMonData(&st->boxes[b][i], MON_DATA_SPECIES) != SPECIES_NONE
             && GetBoxMonData(&st->boxes[b][i], MON_DATA_PERSONALITY) == pid
             && GetBoxMonData(&st->boxes[b][i], MON_DATA_OT_ID) == ot)
            {
                loc->kind = LOC_BOX, loc->box = b, loc->slot = i;
                return TRUE;
            }
    return FALSE;
}

static bool8 InSave(const struct BoxPokemon *box)
{
    struct Loc loc;
    return Where(GetBoxMonData((struct BoxPokemon *)box, MON_DATA_PERSONALITY),
                 GetBoxMonData((struct BoxPokemon *)box, MON_DATA_OT_ID), &loc);
}

bool8 MP_Phone_OnOffer(u32 pid, u32 ot)
{
    u32 i;
    for (i = 0; sLoaded && i < sTr.count; i++)
        if ((sTr.r[i].state == TS_OFFERED || sTr.r[i].state == TS_ACCEPTING) && sTr.r[i].myPid == pid && sTr.r[i].myOt == ot)
            return TRUE;
    return FALSE;
}

// Into the save: the party when there's room, else the PC
static void PutIn(struct Pokemon *mon)
{
    if (gPlayerPartyCount < PARTY_SIZE)
    {
        gPlayerParty[gPlayerPartyCount] = *mon;
        CalculatePlayerPartyCount();
        PartyChanged();
    }
    else
        MP_CopyMonToPC(mon);
}

static void Toast(const char *fmt, const struct TradeRec *r, const struct BoxPokemon *box)
{
    char peer[16], mon[16], msg[120];
    u8 nick[POKEMON_NAME_LENGTH + 1];
    MP_GameToAscii(peer, r->peerName, PLAYER_NAME_LENGTH);
    mon[0] = '\0';
    if (box != NULL)
    {
        GetBoxMonData((struct BoxPokemon *)box, MON_DATA_NICKNAME, nick);
        nick[POKEMON_NAME_LENGTH] = EOS;
        MP_GameToAscii(mon, nick, POKEMON_NAME_LENGTH);
    }
    snprintf(msg, sizeof(msg), fmt, peer, mon);
    MP_Events_QueueToast(msg);
}

// ------------------------------------------------------------------------------------------- the server

static bool8 ServerIs(const char *srv)
{
    char cur[96], h1[128], h2[128];
    int p1 = MP_DEFAULT_PORT, p2 = MP_DEFAULT_PORT;
    if (MPNet_ClientStatus() != MPNET_CONNECTED)
        return FALSE;
    if (!MP_Lobby_ServerAddress(cur, sizeof(cur)))
        snprintf(cur, sizeof(cur), "%s", MP_OnlineLobbyAddress());
    MPNet_ParseAddress(cur, h1, sizeof(h1), &p1);
    MPNet_ParseAddress(srv[0] ? srv : MP_OnlineLobbyAddress(), h2, sizeof(h2), &p2);
    return p1 == p2 && strcmp(h1, h2) == 0;
}

static void SideSecret(const struct TradeRec *r, u8 out[16])
{
    u8 tk[PHONE_KEY_SIZE];
    char hex[33];
    PhoneFriends_TradeKey(tk);
    PhoneProto_ToHex(r->t, 16, hex);
    PhoneProto_Derive(tk, hex, out);   // (the phone works out the same: its DECLINE / CANCEL go straight to the server)
}

static void Ask(struct TradeRec *r, u8 op, const u8 *data, u16 n)
{
    static u8 msg[40 + ESC_DATA_MAX];
    u32 at = 0;
    u8 secret[16];
    msg[at++] = op;
    memcpy(msg + at, r->t, 16);
    at += 16;
    if (op != ESC_LOOK)
    {
        SideSecret(r, secret);
        msg[at++] = r->role;
        memcpy(msg + at, secret, 16);
        at += 16;
    }
    if (op == ESC_PUT)
    {
        msg[at++] = n & 0xFF, msg[at++] = n >> 8;
        memcpy(msg + at, data, n);
        at += n;
    }
    MPNet_ClientSend(MPMSG_C_MAIL, msg, (u16)at);
    sAsking = r - sTr.r;
    sAskedAt = sTick;
}

// Our deposit, sealed with the pair key: [version][our Trainer ID][our name][BoxPokemon][Garden record]
static void SendDeposit(struct TradeRec *r)
{
    static u8 plain[1 + 4 + PLAYER_NAME_LENGTH + 1 + sizeof(struct BoxPokemon) + 2 + REC_MAX];
    static u8 sealed[sizeof(plain) + PHONE_SEAL_EXTRA];
    u8 pk[PHONE_KEY_SIZE];
    u32 at = 0, id = GetPlayerIDAsU32();
    u16 rec = 0;
    if (!PhoneFriends_PairKey(r->peer, pk))
        return;
    plain[at++] = DEPOSIT_VERSION;
    plain[at++] = id, plain[at++] = id >> 8, plain[at++] = id >> 16, plain[at++] = id >> 24;
    memcpy(plain + at, gSaveBlock2Ptr->playerName, PLAYER_NAME_LENGTH);
    plain[at + PLAYER_NAME_LENGTH] = EOS;
    at += PLAYER_NAME_LENGTH + 1;
    memcpy(plain + at, &r->mine, sizeof(struct BoxPokemon));
    at += sizeof(struct BoxPokemon);
    if (MP_Life_FindKey(MP_Life_BoxKey(&r->mine)))
        rec = MP_Life_PackRecord(MP_Life_BoxKey(&r->mine), plain + at + 2, REC_MAX);
    if (at + 2 + rec + PHONE_SEAL_EXTRA > ESC_DATA_MAX)
        rec = 0;   // (too big to travel: its Garden story stays here)
    plain[at++] = rec & 0xFF, plain[at++] = rec >> 8;
    at += rec;
    PhoneProto_Seal(pk, 0, plain, at, sealed);
    Ask(r, ESC_PUT, sealed, (u16)(at + PHONE_SEAL_EXTRA));
}

static bool8 OpenDeposit(struct TradeRec *r, const u8 *data, u16 len)
{
    static u8 plain[ESC_DATA_MAX];
    u8 pk[PHONE_KEY_SIZE];
    u32 seq, at = 0;
    int n;
    u16 rec;
    if (!PhoneFriends_PairKey(r->peer, pk) || len > sizeof(plain) + PHONE_SEAL_EXTRA)
        return FALSE;
    n = PhoneProto_Open(pk, data, len, &seq, plain);
    if (n < (int)(1 + 4 + PLAYER_NAME_LENGTH + 1 + sizeof(struct BoxPokemon) + 2) || plain[0] != DEPOSIT_VERSION)
        return FALSE;
    at = 1;
    r->theirTrainer = plain[at] | (plain[at + 1] << 8) | (plain[at + 2] << 16) | ((u32)plain[at + 3] << 24);
    at += 4;
    memcpy(r->theirTrainerName, plain + at, PLAYER_NAME_LENGTH);
    r->theirTrainerName[PLAYER_NAME_LENGTH] = EOS;
    at += PLAYER_NAME_LENGTH + 1;
    memcpy(&r->theirs, plain + at, sizeof(struct BoxPokemon));
    at += sizeof(struct BoxPokemon);
    rec = plain[at] | (plain[at + 1] << 8);
    at += 2;
    r->recLen = rec <= REC_MAX && at + rec <= (u32)n ? rec : 0;
    memcpy(r->rec, plain + at, r->recLen);
    {
        u16 species = GetBoxMonData(&r->theirs, MON_DATA_SPECIES);
        if (species == SPECIES_NONE || species >= NUM_SPECIES || GetBoxMonData(&r->theirs, MON_DATA_SANITY_IS_BAD_EGG))
            return FALSE;   // (not a POKéMON this game can take: the trade stays as it is)
    }
    return TRUE;
}

// ------------------------------------------------------------------------------------------- custody

static void SetState(struct TradeRec *r, u8 state)
{
    r->state = state;
    r->changed = (u32)time(NULL);
}

// Ours leaves the save (it was ON OFFER): written down first, then gone and saved at once, then to the server
static void Deposit(struct TradeRec *r)
{
    struct Loc loc;
    struct BoxPokemon *box;
    u8 state = r->state;
    if (!Where(r->myPid, r->myOt, &loc))
    {
        r->why = 3;
        r->cancel = TRUE;   // (it's gone from the save: the trade is called off)
        return;
    }
    box = BoxAt(&loc);
    if (loc.kind == LOC_PARTY && !GetBoxMonData(box, MON_DATA_IS_EGG) && NonEggsInParty() <= 1)
        return;   // (the party's last: it waits until there's another)
    if (ItemIsMail(GetBoxMonData(box, MON_DATA_HELD_ITEM)))
    {
        r->why = 3;
        r->cancel = TRUE;
        return;
    }
    {
        static struct Pokemon sTmp;
        BoxMonToMon(box, &sTmp);
        if (MP_ReturnHeldHm(&sTmp))   // (a held HM stays with the player)
        {
            u16 none = ITEM_NONE;
            SetBoxMonData(box, MON_DATA_HELD_ITEM, &none);
            sBagChanged = TRUE;
        }
    }
    r->mine = *box;
    SetState(r, TS_DEPOSITING);
    if (!TradesWrite())
    {
        SetState(r, state);
        return;
    }
    if (loc.kind == LOC_PARTY)
    {
        ZeroMonData(&gPlayerParty[loc.slot]);
        CompactPartySlots();
        CalculatePlayerPartyCount();
        PartyChanged();
    }
    else
    {
        ZeroBoxMonData(box);
        Changed(&loc);
    }
    if (!SaveNow())
    {
        // (couldn't save: it stays, and the trade waits)
        struct Pokemon mon;
        BoxMonToMon(&r->mine, &mon);
        if (loc.kind == LOC_PARTY)
            PutIn(&mon);
        else
            *box = r->mine;
        SetState(r, state);
        TradesWrite();
        return;
    }
    MPNet_Log("phone trade: deposited %u", (unsigned)r->myPid);
    SendDeposit(r);
}

// Theirs into the save: written down (it is already: TS_RECEIVING), in, saved at once
static void Receive(struct TradeRec *r)
{
    struct Pokemon mon;
    if (!InSave(&r->theirs))
    {
        u8 friendship = 70;
        BoxMonToMon(&r->theirs, &mon);
        if (!GetMonData(&mon, MON_DATA_IS_EGG))
            SetMonData(&mon, MON_DATA_FRIENDSHIP, &friendship);   // (as the game's trades do)
        MP_StripHeldHm(&mon);
        PutIn(&mon);
        if (!GetMonData(&mon, MON_DATA_IS_EGG))
        {
            u16 nat = SpeciesToNationalPokedexNum(GetMonData(&mon, MON_DATA_SPECIES));
            GetSetPokedexFlag(nat, FLAG_SET_SEEN);
            GetSetPokedexFlag(nat, FLAG_SET_CAUGHT);
        }
        if (r->recLen)
            MP_Life_UnpackRecord(r->rec, r->recLen, TRUE, r->theirTrainerName,
                                 MP_Life_TrainerKey(r->theirTrainer, r->theirTrainerName));
        if (!SaveNow())
            return;   // (tried again on the next tick; on loading, a save without it gets it again)
        MP_Events_Did(MP_DID_TRADE);
        Toast("Traded with %s by mail! %s arrived.", r, &r->theirs);
        PlaySE(SE_SUCCESS);
    }
    r->shown = FALSE;
    SetState(r, TS_DONE);
    TradesWrite();
    MPNet_Log("phone trade: received %u", (unsigned)GetBoxMonData(&r->theirs, MON_DATA_PERSONALITY));
}

// Ours back into the save (the trade was called off after it left)
static void Return(struct TradeRec *r)
{
    if (!InSave(&r->mine))
    {
        struct Pokemon mon;
        BoxMonToMon(&r->mine, &mon);
        PutIn(&mon);
        if (!SaveNow())
            return;
        Toast("The trade with %s is off. %s came back.", r, &r->mine);
    }
    SetState(r, TS_OFF);
    TradesWrite();
    MPNet_Log("phone trade: %u came back", (unsigned)r->myPid);
}

static void Off(struct TradeRec *r, u8 why)
{
    static const char *const sLines[] = {"The trade with %s is off.", "%s turned down the trade.",
                                         "The trade with %s ran out of time.", "The trade with %s is off: %s is gone."};
    struct Loc loc;
    struct BoxPokemon *box = NULL;
    if (Where(r->myPid, r->myOt, &loc))
        box = BoxAt(&loc);
    r->why = why;
    SetState(r, TS_OFF);
    TradesWrite();
    Toast(sLines[why < 4 ? why : 0], r, box);
}

// --------------------------------------------------------------------------------------------- answers

// MPMSG_S_MAIL: op, key, (ESC_*) state, u16 len, data
void PhoneTrade_OnServer(const u8 *data, u16 len)
{
    struct TradeRec *r;
    u8 op, state;
    u16 n = 0;
    const u8 *payload = NULL;
    if (!sLoaded || len < 18)
        return;
    op = data[0];
    if (op < ESC_PUT && op != MAIL_DENIED)
        return;   // (the mailbox's own answers: nothing to do)
    r = Find(data + 1);
    if (r == NULL)
        return;
    if (sAsking == r - sTr.r)
        sAsking = -1;
    state = data[17];
    if (len >= 20)
    {
        n = data[18] | (data[19] << 8);
        payload = data + 20;
        if (20 + n > len)
            n = 0;
    }
    if (op == MAIL_DENIED)
    {
        MPNet_Log("phone trade: the server refused our side's secret");
        return;
    }
    switch (r->state)
    {
    case TS_OFFERED:
    case TS_ACCEPTING:
        if (state & ESC_CLOSED)
            Off(r, r->cancel ? r->why : r->role == 0 ? 1 : 0);
        else if (state & (ESC_HAVE0 << (r->role ^ 1)))
        {
            r->theirsIn = TRUE;   // (theirs is in: ours goes as soon as the game is free; too late to call it off)
            r->cancel = FALSE;
        }
        break;
    case TS_DEPOSITING:
        if (state & (ESC_HAVE0 << r->role))
        {
            SetState(r, TS_DEPOSITED);
            TradesWrite();
            if (state & (ESC_HAVE0 << (r->role ^ 1)))
                Ask(r, ESC_TAKE, NULL, 0);
        }
        else if (state & ESC_CLOSED)
        {
            SetState(r, TS_RETURNING);   // (called off before ours got there: back home once the game is free)
            TradesWrite();
        }
        break;
    case TS_DEPOSITED:
        if (op == ESC_TAKE && n > 0 && !(state & ESC_CLOSED))
        {
            if (!OpenDeposit(r, payload, n))
            {
                MPNet_Log("phone trade: their deposit couldn't be read");
                break;
            }
            SetState(r, TS_RECEIVING);
            if (!TradesWrite())
                break;
            if (Safe())
                Receive(r);
        }
        else if (state & ESC_CLOSED)
        {
            if (op != ESC_CANCEL)
                Ask(r, ESC_CANCEL, NULL, 0);   // (on the server too: ours stays there no longer than needed)
            else
            {
                SetState(r, TS_RETURNING);
                TradesWrite();
            }
        }
        break;
    }
}

// --------------------------------------------------------------------------------------------- every frame

static void Tick(struct TradeRec *r)
{
    u32 now = (u32)time(NULL);
    switch (r->state)
    {
    case TS_OFFERED:
    case TS_ACCEPTING:
        if (r->theirsIn)
            break;   // (PhoneTrade_Update deposits ours)
        if (r->cancel || now > r->expires)
        {
            if (!r->cancel)
                r->why = 2;
            r->cancel = TRUE;
            Ask(r, ESC_CANCEL, NULL, 0);
        }
        else if (r->state == TS_ACCEPTING && Safe())
            Deposit(r);
        else if (r->state == TS_OFFERED)
            Ask(r, ESC_LOOK, NULL, 0);
        break;
    case TS_DEPOSITING:
        SendDeposit(r);
        break;
    case TS_DEPOSITED:
        if (r->role == 1 && now > r->expires + 86400)
            Ask(r, ESC_CANCEL, NULL, 0);   // (they never came: ours comes home)
        else
            Ask(r, ESC_TAKE, NULL, 0);
        break;
    }
}

static void SceneEnd(void);

void PhoneTrade_Update(void)
{
    u32 i, now = (u32)time(NULL);
    sTick++;
    if (!sLoaded)
        return;
    // custody steps that wait for the game to be free (never in a battle, a menu or a script)
    for (i = 0; i < sTr.count; i++)
    {
        struct TradeRec *r = &sTr.r[i];
        if (r->state == TS_RECEIVING && Safe())
            Receive(r);
        else if (r->state == TS_RETURNING && Safe())
            Return(r);
        else if (r->state == TS_OFFERED && r->theirsIn && sAsking < 0 && Safe() && ServerIs(r->srv))
            Deposit(r);
    }
    // the trade scene, once free on the field
    if (sScene < 0 && MP_IsInOverworld() && MP_IsLocalFree() && !gPaletteFade.active && !gMain.inBattle)
        for (i = 0; i < sTr.count; i++)
            if (sTr.r[i].state == TS_DONE && !sTr.r[i].shown)
            {
                struct TradeRec *r = &sTr.r[i];
                struct Loc loc;
                if (!Where(GetBoxMonData(&r->theirs, MON_DATA_PERSONALITY), GetBoxMonData(&r->theirs, MON_DATA_OT_ID), &loc))
                {
                    r->shown = TRUE;   // (traded on already: no scene)
                    TradesWrite();
                    continue;
                }
                sScene = i;
                sSceneInParty = loc.kind == LOC_PARTY;
                sSceneSlot = sSceneInParty ? loc.slot : gPlayerPartyCount < PARTY_SIZE ? gPlayerPartyCount : PARTY_SIZE - 1;
                sSceneBackup = gPlayerParty[sSceneSlot];
                BoxMonToMon(BoxAt(&loc), &gEnemyParty[0]);
                BoxMonToMon(&r->mine, &gPlayerParty[sSceneSlot]);
                MPNet_Log("phone trade: the trade scene");
                MP_MailTrade_Scene(sSceneSlot, r->peerName, SceneEnd);
                break;
            }
    if (sAsking >= 0 && sTick - sAskedAt < 60 * 10)
        return;   // (an answer due: one question at a time, ten seconds each)
    sAsking = -1;
    if (sTick % 60 != 0)
        return;
    for (i = 0; i < sTr.count; i++)
    {
        // each trade asked about every 15 seconds, in turn
        struct TradeRec *r = &sTr.r[(i + sTick / 60) % sTr.count];
        if (r->state == TS_NONE || r->state >= TS_RECEIVING || !ServerIs(r->srv) || now - r->changed < 2)
            continue;
        if ((sTick / 60) % 15 != (u32)(r - sTr.r) % 15 && r->state != TS_DEPOSITING && !r->cancel)
            continue;
        Tick(r);
        if (sAsking >= 0)
            break;
    }
}

// After the scene: theirs (maybe evolved) where it belongs, the party slot as it was, saved
static void SceneEnd(void)
{
    struct TradeRec *r = &sTr.r[sScene];
    struct Pokemon got = gPlayerParty[sSceneSlot];
    if (!sSceneInParty)
    {
        struct Loc loc;
        if (Where(GetMonData(&got, MON_DATA_PERSONALITY), GetMonData(&got, MON_DATA_OT_ID), &loc) && loc.kind == LOC_BOX)
            *BoxAt(&loc) = got.box;
        gPlayerParty[sSceneSlot] = sSceneBackup;
        CalculatePlayerPartyCount();
    }
    r->theirs = got.box;
    r->shown = TRUE;
    TradesWrite();
    sScene = -1;
    SaveNow();   // (a trade evolution kept)
    PartyChanged();
}

// --------------------------------------------------------------------------------------------- loading

void PhoneTrade_Load(void)
{
    char path[600];
    FILE *f;
    u32 i, now = (u32)time(NULL);
    memset(&sTr, 0, sizeof(sTr));
    sAsking = sScene = -1;
    TradesPath(path, sizeof(path));
    f = fopen(path, "rb");
    if (f != NULL)
    {
        if (fread(&sTr, sizeof(sTr), 1, f) != 1 || sTr.magic != TRADES_MAGIC || sTr.count > TRADES_MAX)
            memset(&sTr, 0, sizeof(sTr));
        fclose(f);
    }
    sTr.magic = TRADES_MAGIC;
    sTr.version = TRADES_VERSION;
    // a game closed between steps: where each trade really stands
    for (i = 0; i < sTr.count; i++)
    {
        struct TradeRec *r = &sTr.r[i];
        if (r->state == TS_DEPOSITING && InSave(&r->mine))
            SetState(r, r->role == 0 ? TS_OFFERED : TS_ACCEPTING);   // (it never left: the save wasn't written)
        else if (r->state == TS_RECEIVING && InSave(&r->theirs))
            r->state = TS_DONE;
        else if (r->state == TS_RETURNING && InSave(&r->mine))
            r->state = TS_OFF;
    }
    // finished ones kept a week for the phone's list
    for (i = 0; i < sTr.count;)
        if ((sTr.r[i].state == TS_OFF || (sTr.r[i].state == TS_DONE && sTr.r[i].shown)) && now - sTr.r[i].changed > 7 * 86400)
            sTr.r[i] = sTr.r[--sTr.count];
        else
            i++;
    sLoaded = TRUE;
}

// --------------------------------------------------------------------------------------------- the snapshot

u32 PhoneTrade_Hash(void)
{
    u32 h = 2166136261u, i, k;
    for (i = 0; sLoaded && i < sTr.count; i++)
    {
        const u8 *bits[] = {sTr.r[i].t, &sTr.r[i].state, &sTr.r[i].shown, &sTr.r[i].why, &sTr.r[i].cancel};
        const u32 sizes[] = {16, 1, 1, 1, 1};
        for (k = 0; k < 5; k++)
        {
            u32 n;
            for (n = 0; n < sizes[k]; n++)
                h = (h ^ bits[k][n]) * 16777619u;
        }
    }
    return h ^ (sLoaded ? sTr.count : 0);
}

void PhoneTrade_Snapshot(struct Buf *b)
{
    char hex[2 * PHONE_KEY_SIZE + 1];
    u32 i;
    if (!sLoaded)
        return;
    BufAdd(b, ",\"trades\":[");
    for (i = 0; i < sTr.count; i++)
    {
        struct TradeRec *r = &sTr.r[i];
        struct Loc loc;
        PhoneProto_ToHex(r->t, 16, hex);
        BufAdd(b, "%s{\"t\":\"%s\",\"role\":%u,\"st\":\"%s\",\"why\":%u,\"want\":%u,\"at\":%u,\"exp\":%u,\"changed\":%u,\"shown\":%u",
               i ? "," : "", hex, r->role, sStateNames[r->state < ARRAY_COUNT(sStateNames) ? r->state : 0], r->why,
               r->want, (unsigned)r->created, (unsigned)r->expires, (unsigned)r->changed, r->shown);
        PhoneProto_ToHex(r->peer, PHONE_SAVEID_SIZE, hex);
        BufAdd(b, ",\"peer\":\"%s\",\"pn\":", hex);
        BufText(b, r->peerName, PLAYER_NAME_LENGTH);
        BufAdd(b, ",\"mine\":");
        if (r->state >= TS_DEPOSITING && r->state != TS_OFF)
            BufBoxMon(b, &r->mine);
        else if (Where(r->myPid, r->myOt, &loc))
            BufBoxMon(b, BoxAt(&loc));
        else
            BufAdd(b, "0");
        if (r->state == TS_RECEIVING || r->state == TS_DONE)
        {
            BufAdd(b, ",\"theirs\":");
            BufBoxMon(b, &r->theirs);
        }
        BufAdd(b, "}");
    }
    BufAdd(b, "]");
}

// ------------------------------------------------------------------------------------------------- orders

static struct TradeRec *NewRec(const char *json, u8 role, const char **why)
{
    char hex[80], srv[64];
    u8 t[16], peer[PHONE_SAVEID_SIZE], pk[PHONE_KEY_SIZE];
    long long p = 0, o = 0, exp = 0, want = 0;
    struct Loc at;
    struct BoxPokemon *box;
    struct TradeRec *r;
    u32 now = (u32)time(NULL);
    *why = "bad";
    if (!JsonStr(json, "trade", hex, sizeof(hex)) || !PhoneProto_FromHex(hex, t, 16)
     || !JsonStr(json, role == 0 ? "to" : "from", hex, sizeof(hex)) || !PhoneProto_FromHex(hex, peer, PHONE_SAVEID_SIZE)
     || !ParseLoc(json, "at", &at) || !JsonNum(json, "p", &p) || !JsonNum(json, "o", &o) || !JsonNum(json, "exp", &exp))
        return NULL;
    JsonNum(json, "want", &want);
    if (!JsonStr(json, "srv", srv, sizeof(srv)))
        srv[0] = '\0';
    if (Find(t) != NULL)
    {
        *why = NULL;   // (the same order again: done already)
        return NULL;
    }
    if (!PhoneFriends_PairKey(peer, pk))
    {
        *why = "nopair";
        return NULL;
    }
    if ((u32)exp < now)
    {
        *why = "expired";
        return NULL;
    }
    if (Empty(&at))
    {
        *why = "gone";
        return NULL;
    }
    if (!Is(&at, (u32)p, (u32)o))
    {
        *why = "changed";
        return NULL;
    }
    box = BoxAt(&at);
    if (ItemIsMail(GetBoxMonData(box, MON_DATA_HELD_ITEM)))
    {
        *why = "mail";
        return NULL;
    }
    if (MP_Phone_OnOffer((u32)p, (u32)o))
    {
        *why = "onoffer";
        return NULL;
    }
    if (Orre_Shadow_IsBox(box))
    {
        *why = "shadow";   // (Moemon Kai: a Shadow Pokémon isn't traded until it's purified: docs/SHADOW_POKEMON.md)
        return NULL;
    }
    if (MP_Phone_InTraining((u32)p, (u32)o))
    {
        *why = "training";
        return NULL;
    }
    if (role == 1 && want != 0 && GetBoxMonData(box, MON_DATA_SPECIES) != (u32)want)
    {
        *why = "notwanted";
        return NULL;
    }
    if (sTr.count >= TRADES_MAX)
    {
        // (the oldest finished one makes room)
        u32 i, oldest = TRADES_MAX;
        for (i = 0; i < sTr.count; i++)
            if ((sTr.r[i].state == TS_OFF || (sTr.r[i].state == TS_DONE && sTr.r[i].shown))
             && (oldest == TRADES_MAX || sTr.r[i].changed < sTr.r[oldest].changed))
                oldest = i;
        if (oldest == TRADES_MAX)
        {
            *why = "full";
            return NULL;
        }
        sTr.r[oldest] = sTr.r[--sTr.count];
    }
    r = &sTr.r[sTr.count++];
    memset(r, 0, sizeof(*r));
    memcpy(r->t, t, 16);
    memcpy(r->peer, peer, PHONE_SAVEID_SIZE);
    memset(r->peerName, EOS, sizeof(r->peerName));
    if (PhoneFriends_Name(peer) != NULL)
        memcpy(r->peerName, PhoneFriends_Name(peer), PLAYER_NAME_LENGTH);
    r->role = role;
    r->created = now;
    r->expires = (u32)exp > now + TRADE_DAYS * 86400 + 3600 ? now + TRADE_DAYS * 86400 : (u32)exp;
    r->myPid = (u32)p;
    r->myOt = (u32)o;
    r->want = (u16)want;
    snprintf(r->srv, sizeof(r->srv), "%s", srv);
    SetState(r, role == 0 ? TS_OFFERED : TS_ACCEPTING);
    *why = NULL;
    return r;
}

// "offer" (ours ON OFFER for a friend: the phone has put the offer in their inbox), "accept" (a friend's offer, with
// one of ours: deposited once the game is on that server), "untrade" (call it off while nothing's deposited)
const char *PhoneTrade_Apply(const char *op, const char *json)
{
    const char *why;
    if (!sLoaded)
        return "busy";
    if (strcmp(op, "offer") == 0 || strcmp(op, "accept") == 0)
    {
        struct TradeRec *r = NewRec(json, op[0] == 'a', &why);
        if (r == NULL)
            return why;
        if (!TradesWrite())
        {
            sTr.count--;
            return "cantsave";
        }
        MPNet_Log("phone trade: %s (%u)", op, (unsigned)r->myPid);
        return NULL;
    }
    if (strcmp(op, "untrade") == 0)
    {
        char hex[40];
        u8 t[16];
        struct TradeRec *r;
        if (!JsonStr(json, "trade", hex, sizeof(hex)) || !PhoneProto_FromHex(hex, t, 16) || (r = Find(t)) == NULL)
            return "bad";
        if (r->state != TS_OFFERED && r->state != TS_ACCEPTING)
            return "deposited";
        r->cancel = TRUE;
        r->why = 0;
        TradesWrite();
        return NULL;
    }
    return "unknown";
}
