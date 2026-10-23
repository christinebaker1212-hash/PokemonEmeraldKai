// Moemon Co-op: the shared PC's Pokémon, shown in the PC box screen.
//
// The server keeps 14 boxes of 30 (mp_store.c). Here we keep a mirror of them shaped like the game's own
// PokemonStorage, so the stock box screen can show and change them: while the box screen shows the shared boxes,
// gPokemonStoragePtr points at the mirror (MP_SharedBox_ShowShared), and the player's own boxes are reached through
// MP_PersonalStorage() (the save code always uses that).
//
// The box screen just changes the mirror. Every frame (when the screen is in a steady state) the mirror is compared
// with the server's last word ("base") and the differences become one transaction (MP_ST_TXN): a Pokémon that left
// its slot is picked up into our hand on the server (HOLD, only if it's still there: whoever's first gets it), then
// put down (PLACE), sent to our own game (OUT: the party or our own boxes) or released (RELEASE); one that comes from
// our own game goes in (IN); one with new data (markings, a new name) is SET. One transaction at a time: what changes
// meanwhile goes out with the next one. The server answers with the boxes that changed (to everyone) and what didn't
// work: a Pokémon somebody else got first is taken back from wherever we put it ("lost"), one that didn't fit is
// given back to our own game ("refused").
//
// Friends' cursors: each player sends where their cursor is (MP_ST_CURSOR); the box screen draws the others'.
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "mp_net.h"
#include "load_save.h"
#include "main.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "pokedex.h"
#include "sound.h"
#include "string_util.h"
#include "constants/songs.h"
#include "constants/characters.h"

STATIC_ASSERT(sizeof(struct Pokemon) + PLAYER_NAME_LENGTH + 1 <= MP_STORE_MON_BYTES, StoredMonFits2);
STATIC_ASSERT(MP_SHARED_BOXES == TOTAL_BOXES_COUNT && MP_SHARED_BOX_SLOTS == IN_BOX_COUNT, SharedBoxesLikeOurs);
STATIC_ASSERT(MP_STORE_BOXNAME == BOX_NAME_LENGTH + 1, SharedBoxNameLikeOurs);

#define SLOTS     MP_STORE_MONS
#define BLOB      MP_STORE_MON_BYTES
#define BY_OFFSET sizeof(struct Pokemon)
#define MAX_HELD  (IN_BOX_COUNT + 2)

void MP_AsciiToGame(u8 *dst, const char *src, int maxLen);
void MP_GameToAscii(char *dst, const u8 *src, int maxLen);

struct Ident
{
    u32 pers, otid;
};

static EWRAM_DATA struct PokemonStorage sMirror = {0};            // the shared boxes as the box screen sees them
static EWRAM_DATA u8 sBaseUsed[SLOTS] = {0};                      // the server's last word (+ what we sent since)
static EWRAM_DATA u8 sBase[SLOTS][BLOB] = {0};
static EWRAM_DATA u8 sBaseNames[TOTAL_BOXES_COUNT][MP_STORE_BOXNAME] = {0};
static EWRAM_DATA u8 sBaseWallpapers[TOTAL_BOXES_COUNT] = {0};
static EWRAM_DATA u8 sHand[MP_STORE_HAND][BLOB] = {0};            // what the server has in our hand
static EWRAM_DATA u8 sHandCount = 0;
// boxes that arrived while a transaction was out (applied with its answer)
static EWRAM_DATA u8 sStash[TOTAL_BOXES_COUNT][1 + MP_STORE_BOXNAME + 1 + SLOTS / TOTAL_BOXES_COUNT * (1 + BLOB) + 1] = {0};
static EWRAM_DATA u16 sStashLen[TOTAL_BOXES_COUNT] = {0};
static EWRAM_DATA u8 sResult[MP_MAX_MSG] = {0};                     // a transaction's answer, applied when steady
static EWRAM_DATA u16 sResultLen = 0;
// Pokémon somebody else got first: never ours, wherever they turn up
static EWRAM_DATA struct Ident sLost[64] = {0};
static EWRAM_DATA u8 sLostCount = 0;
// Pokémon the player released in the box screen (only these may be RELEASEd: a shared Pokémon gone missing on our
// side for any other reason goes back into a free slot)
static EWRAM_DATA struct Ident sReleased[64] = {0};
static EWRAM_DATA u8 sReleasedCount = 0;

static struct PokemonStorage *sPersonal;   // our own boxes while the mirror is shown
static bool8 sOpen;                        // the shared PC's Pokémon are in use (box screen, or closing)
static u16 sLoadedMask;                    // boxes heard from since opening
static bool8 sPending;                     // a transaction is out
static u16 sSeq;
static u16 sPendingTimer;
static bool8 sClosing;                     // the box screen is gone: flush what's left, then done
static bool8 sChangedForUi;                // boxes changed under the box screen (it redraws what it shows)
static u32 sChangedCells[TOTAL_BOXES_COUNT];
static u32 sChangedInfo;                   // boxes whose name (bits 0-15) or wallpaper (16-31) changed under the box screen
static char sNote[64];                     // a message for the box screen ("Someone else took ...")

// ---------------------------------------------------------------------------
// Small helpers

static bool8 Online(void)
{
    return MP_IsConnected() && gMpLocalId >= 0;
}

static void StoreSend(const u8 *data, u16 len)
{
    if (Online())
        MPNet_ClientSend(MPMSG_C_STORE, data, len);
}

static void Put32(u8 *p, u32 v)
{
    p[0] = v; p[1] = v >> 8; p[2] = v >> 16; p[3] = v >> 24;
}

static u32 Get32(const u8 *p)
{
    return p[0] | (p[1] << 8) | (p[2] << 16) | ((u32)p[3] << 24);
}

static bool8 HasMon(const struct BoxPokemon *b)
{
    return GetBoxMonData((struct BoxPokemon *)b, MON_DATA_SANITY_HAS_SPECIES);
}

static struct Ident BoxIdent(const struct BoxPokemon *b)
{
    struct Ident id;
    id.pers = Get32((const u8 *)b);
    id.otid = Get32((const u8 *)b + 4);
    return id;
}

static struct Ident BlobIdent(const u8 *blob)
{
    struct Ident id;
    id.pers = Get32(blob);
    id.otid = Get32(blob + 4);
    return id;
}

static bool8 SameIdent(struct Ident a, struct Ident b)
{
    return a.pers == b.pers && a.otid == b.otid;
}

static bool8 IsLost(struct Ident id)
{
    u8 i;
    for (i = 0; i < sLostCount; i++)
        if (SameIdent(sLost[i], id))
            return TRUE;
    return FALSE;
}

static void AddLost(struct Ident id)
{
    if (!IsLost(id) && sLostCount < ARRAY_COUNT(sLost))
        sLost[sLostCount++] = id;
}

// The 112 bytes for a box Pokémon: the full Pokemon (stats worked out) and who put it in
static void MakeBlob(const struct BoxPokemon *b, const u8 *by, u8 *blob)
{
    struct Pokemon mon;
    memset(blob, 0, BLOB);
    BoxMonToMon(b, &mon);
    memcpy(blob, &mon, sizeof(mon));
    memcpy(blob + BY_OFFSET, by, PLAYER_NAME_LENGTH);
    blob[BY_OFFSET + PLAYER_NAME_LENGTH] = EOS;
}

static bool8 MirrorMatchesBase(u16 slot)
{
    const struct BoxPokemon *m = &sMirror.boxes[slot / IN_BOX_COUNT][slot % IN_BOX_COUNT];
    if (!sBaseUsed[slot])
        return !HasMon(m);
    return HasMon(m) && memcmp(m, sBase[slot], sizeof(struct BoxPokemon)) == 0;
}

static void MarkChanged(u16 slot)
{
    sChangedCells[slot / IN_BOX_COUNT] |= 1u << (slot % IN_BOX_COUNT);
    sChangedForUi = TRUE;
}

struct PokemonStorage *MP_PersonalStorage(void)
{
    return sPersonal ? sPersonal : gPokemonStoragePtr;
}

bool8 MP_SharedBox_IsMirrorShown(void)
{
    return sPersonal != NULL;
}

// ---------------------------------------------------------------------------
// Our own Pokémon (the party and our own boxes)

static bool8 FindPersonal(struct Ident id, u8 *where, u8 *box, u8 *pos)
{
    struct PokemonStorage *own = MP_PersonalStorage();
    u8 b, p;
    for (p = 0; p < PARTY_SIZE; p++)
        if (GetMonData(&gPlayerParty[p], MON_DATA_SANITY_HAS_SPECIES) && SameIdent(BoxIdent(&gPlayerParty[p].box), id))
        {
            *where = 0;
            *pos = p;
            return TRUE;
        }
    for (b = 0; b < TOTAL_BOXES_COUNT; b++)
        for (p = 0; p < IN_BOX_COUNT; p++)
            if (HasMon(&own->boxes[b][p]) && SameIdent(BoxIdent(&own->boxes[b][p]), id))
            {
                *where = 1;
                *box = b;
                *pos = p;
                return TRUE;
            }
    return FALSE;
}

static void RemovePersonal(struct Ident id)
{
    u8 where, box, pos;
    if (!FindPersonal(id, &where, &box, &pos))
        return;
    if (where == 0)
    {
        ZeroMonData(&gPlayerParty[pos]);
        CompactPartySlots();
        CalculatePlayerPartyCount();
    }
    else
        ZeroBoxMonData(&MP_PersonalStorage()->boxes[box][pos]);
    MP_PcStorage_RequestRebuild();
    MPNet_Log("shared PC: took back a Pokemon somebody else got first");
}

// Back into our own game: the party if there's room, else the first free slot of our own boxes
static bool8 GiveBackPersonal(const struct BoxPokemon *b)
{
    struct PokemonStorage *own = MP_PersonalStorage();
    u8 i, box, pos;
    CalculatePlayerPartyCount();
    if (gPlayerPartyCount < PARTY_SIZE)
    {
        BoxMonToMon(b, &gPlayerParty[gPlayerPartyCount]);
        CalculatePlayerPartyCount();
        MP_PcStorage_RequestRebuild();
        return TRUE;
    }
    for (i = 0; i < TOTAL_BOXES_COUNT; i++)
    {
        box = (own->currentBox + i) % TOTAL_BOXES_COUNT;
        for (pos = 0; pos < IN_BOX_COUNT; pos++)
            if (!HasMon(&own->boxes[box][pos]))
            {
                own->boxes[box][pos] = *b;
                MP_PcStorage_RequestRebuild();
                return TRUE;
            }
    }
    return FALSE;
}

// ---------------------------------------------------------------------------
// Showing the mirror or our own boxes in the box screen

void MP_SharedBox_ShowShared(bool8 shared)
{
    if (shared && sPersonal == NULL)
    {
        sPersonal = gPokemonStoragePtr;
        gPokemonStoragePtr = &sMirror;
    }
    else if (!shared && sPersonal != NULL)
    {
        gPokemonStoragePtr = sPersonal;
        sPersonal = NULL;
    }
}

// load_save.c: the save blocks moved; our own boxes are there now
void MP_SharedBox_SaveBlocksMoved(struct PokemonStorage *own)
{
    if (sPersonal != NULL)
    {
        sPersonal = own;
        gPokemonStoragePtr = &sMirror;
    }
}

// ---------------------------------------------------------------------------
// The server's word on a box

// One box from a BOX_DATA message into the mirror. Cells the box screen changed since our last transaction are
// rebased: if somebody else changed the same slot, theirs wins there (a Pokémon we put in that slot moves to a free
// one, one we picked up from it is taken back).
static void ApplyBox(const u8 *data, u16 len)
{
    u8 box = data[1], count, i, fresh[IN_BOX_COUNT] = {0};
    static u8 sTruth[IN_BOX_COUNT][BLOB];
    u8 oldName[MP_STORE_BOXNAME], oldWallpaper;
    u16 pos;
    if (box >= TOTAL_BOXES_COUNT || len < 2 + MP_STORE_BOXNAME + 2)
        return;
    count = data[2 + MP_STORE_BOXNAME + 1];
    pos = 2 + MP_STORE_BOXNAME + 2;
    for (i = 0; i < count && pos + 1 + BLOB <= len; i++)
    {
        u8 at = data[pos];
        if (at < IN_BOX_COUNT)
        {
            memcpy(sTruth[at], data + pos + 1, BLOB);
            fresh[at] = 1;
        }
        pos += 1 + BLOB;
    }
    // name and wallpaper: theirs, unless we changed them since (what's shown changed: the box screen redraws it)
    memcpy(oldName, sMirror.boxNames[box], MP_STORE_BOXNAME);
    oldWallpaper = sMirror.boxWallpapers[box];
    if (memcmp(sMirror.boxNames[box], sBaseNames[box], MP_STORE_BOXNAME) == 0)
        memcpy(sMirror.boxNames[box], data + 2, MP_STORE_BOXNAME);
    memcpy(sBaseNames[box], data + 2, MP_STORE_BOXNAME);
    if (sMirror.boxWallpapers[box] == sBaseWallpapers[box])
        sMirror.boxWallpapers[box] = data[2 + MP_STORE_BOXNAME];
    sBaseWallpapers[box] = data[2 + MP_STORE_BOXNAME];
    if (!(sLoadedMask & (1 << box)))
    {
        // first time: just take it
        memcpy(sMirror.boxNames[box], data + 2, MP_STORE_BOXNAME);
        sMirror.boxWallpapers[box] = data[2 + MP_STORE_BOXNAME];
    }
    else
    {
        if (memcmp(oldName, sMirror.boxNames[box], MP_STORE_BOXNAME) != 0)
            sChangedInfo |= 1u << box;
        if (oldWallpaper != sMirror.boxWallpapers[box])
            sChangedInfo |= 1u << (16 + box);
    }

    for (i = 0; i < IN_BOX_COUNT; i++)
    {
        u16 slot = box * IN_BOX_COUNT + i;
        struct BoxPokemon *m = &sMirror.boxes[box][i];
        bool8 baseUsed = sBaseUsed[slot], truthUsed = fresh[i];
        bool8 remoteChange = (baseUsed != truthUsed) || (baseUsed && memcmp(sBase[slot], sTruth[i], BLOB) != 0);
        bool8 localChange = (sLoadedMask & (1 << box)) && !MirrorMatchesBase(slot);

        if (!localChange || !(sLoadedMask & (1 << box)))
        {
            if (truthUsed)
                *m = *(struct BoxPokemon *)sTruth[i];
            else
                ZeroBoxMonData(m);
            if (remoteChange)
                MarkChanged(slot);
        }
        else if (remoteChange)
        {
            // both of us changed this slot
            struct Ident baseId = BlobIdent(sBase[slot]), truthId = BlobIdent(sTruth[i]);
            if (baseUsed && truthUsed && SameIdent(baseId, truthId))
            {
                // the same Pokémon, its data changed over there (markings, a name): theirs, if it's still here for us;
                // if we picked it up meanwhile, that stands (the server checks it's still there)
                if (HasMon(m) && SameIdent(BoxIdent(m), truthId))
                {
                    *m = *(struct BoxPokemon *)sTruth[i];
                    MarkChanged(slot);
                }
            }
            else
            {
                // the Pokémon that was here is gone or somebody else's: any copy we made of it isn't ours
                if (baseUsed)
                    AddLost(baseId);
                if (truthUsed)
                {
                    // one of theirs is here now; one we put here moves to a free slot (sent with the next transaction)
                    if (HasMon(m) && !SameIdent(BoxIdent(m), truthId) && !(baseUsed && SameIdent(BoxIdent(m), baseId)))
                    {
                        struct BoxPokemon ours = *m;
                        u8 b, p, found = FALSE;
                        for (b = 0; b < TOTAL_BOXES_COUNT && !found; b++)
                            for (p = 0; p < IN_BOX_COUNT && !found; p++)
                            {
                                u16 s = ((box + b) % TOTAL_BOXES_COUNT) * IN_BOX_COUNT + p;
                                struct BoxPokemon *dst = &sMirror.boxes[s / IN_BOX_COUNT][s % IN_BOX_COUNT];
                                if (s != slot && !HasMon(dst) && !sBaseUsed[s])
                                {
                                    *dst = ours;
                                    MarkChanged(s);
                                    found = TRUE;
                                }
                            }
                        if (!found && !MP_SharedBox_IsShared(&ours))
                            GiveBackPersonal(&ours);   // can't be (the boxes would be full), but never lose ours
                    }
                    *m = *(struct BoxPokemon *)sTruth[i];
                }
                // (empty over there: whatever we put here stays, the slot is free for it)
                MarkChanged(slot);
            }
        }
        sBaseUsed[slot] = truthUsed;
        if (truthUsed)
            memcpy(sBase[slot], sTruth[i], BLOB);
        else
            memset(sBase[slot], 0, BLOB);
    }
    sLoadedMask |= 1 << box;
}

// ---------------------------------------------------------------------------
// Transactions

struct Held
{
    struct Ident id;
    const struct BoxPokemon *mon;
};

static u8 sTxn[MP_MAX_MSG - 64];

static bool8 InList(const struct Held *list, u8 n, struct Ident id)
{
    u8 i;
    for (i = 0; i < n; i++)
        if (SameIdent(list[i].id, id))
            return TRUE;
    return FALSE;
}

// Where a Pokémon is in the mirror now (SLOTS: not there)
static u16 MirrorFind(struct Ident id)
{
    u16 s;
    for (s = 0; s < SLOTS; s++)
    {
        const struct BoxPokemon *m = &sMirror.boxes[s / IN_BOX_COUNT][s % IN_BOX_COUNT];
        if (HasMon(m) && SameIdent(BoxIdent(m), id))
            return s;
    }
    return SLOTS;
}

static bool8 HandHas(struct Ident id)
{
    u8 i;
    for (i = 0; i < sHandCount; i++)
        if (SameIdent(BlobIdent(sHand[i]), id))
            return TRUE;
    return FALSE;
}

static const u8 *KnownBy(struct Ident id)
{
    u16 s;
    u8 i;
    for (s = 0; s < SLOTS; s++)
        if (sBaseUsed[s] && SameIdent(BlobIdent(sBase[s]), id))
            return sBase[s] + BY_OFFSET;
    for (i = 0; i < sHandCount; i++)
        if (SameIdent(BlobIdent(sHand[i]), id))
            return sHand[i] + BY_OFFSET;
    return gSaveBlock2Ptr->playerName;   // from our own game: we put it in
}

// Takes back Pokémon somebody else got first, wherever we put them: held, in our own game, or in a shared slot
// other than the one the server says it's in. Then they're forgotten (they're only where the server says now).
static void TakeBackLost(void)
{
    u8 i;
    u16 s;
    for (i = 0; i < sLostCount; i++)
    {
        u8 where, box, pos;
        for (s = 0; s < SLOTS; s++)
        {
            struct BoxPokemon *m = &sMirror.boxes[s / IN_BOX_COUNT][s % IN_BOX_COUNT];
            if (HasMon(m) && SameIdent(BoxIdent(m), sLost[i]) && !(sBaseUsed[s] && SameIdent(BlobIdent(sBase[s]), sLost[i])))
            {
                ZeroBoxMonData(m);
                MarkChanged(s);
            }
        }
        while (FindPersonal(sLost[i], &where, &box, &pos))
            RemovePersonal(sLost[i]);
        MP_PcStorage_DropHeld(sLost[i].pers, sLost[i].otid);
    }
    sLostCount = 0;
}

// One transaction from the differences between the mirror and the base (FALSE: nothing to send)
static bool8 Flush(void)
{
    struct Held held[MAX_HELD];
    const struct BoxPokemon *heldMons[MAX_HELD];
    u8 heldCount, i, b;
    u16 n = 0, s;
    u8 newHand[MP_STORE_HAND][BLOB];
    u8 newHandCount = 0;
    bool8 any = FALSE, truncated = FALSE;

    TakeBackLost();
    heldCount = sClosing ? 0 : MP_PcStorage_Held(heldMons, MAX_HELD);
    for (i = 0; i < heldCount; i++)
    {
        held[i].mon = heldMons[i];
        held[i].id = BoxIdent(heldMons[i]);
    }

    sTxn[n++] = MP_ST_TXN;
    sTxn[n++] = (u8)(sSeq + 1);
    sTxn[n++] = (u8)((sSeq + 1) >> 8);

    // 1. Pokémon that left their slot (or changed): into our hand, or new data
    for (s = 0; s < SLOTS; s++)
    {
        struct BoxPokemon *m = &sMirror.boxes[s / IN_BOX_COUNT][s % IN_BOX_COUNT];
        struct Ident id;
        if (!sBaseUsed[s] || MirrorMatchesBase(s))
            continue;
        id = BlobIdent(sBase[s]);
        if (HasMon(m) && SameIdent(BoxIdent(m), id))
        {
            // the same Pokémon with new data (markings, a new name)
            u8 blob[BLOB];
            if (n + 11 + BLOB > sizeof(sTxn))
            {
                truncated = TRUE;
                break;
            }
            MakeBlob(m, sBase[s] + BY_OFFSET, blob);
            sTxn[n++] = MP_TXN_SET;
            sTxn[n++] = s / IN_BOX_COUNT;
            sTxn[n++] = s % IN_BOX_COUNT;
            Put32(sTxn + n, id.pers);
            Put32(sTxn + n + 4, id.otid);
            n += 8;
            memcpy(sTxn + n, blob, BLOB);
            n += BLOB;
            memcpy(sBase[s], blob, BLOB);
            any = TRUE;
            continue;
        }
        if (n + 11 > sizeof(sTxn) || newHandCount >= MP_STORE_HAND)
        {
            truncated = TRUE;   // the rest next time (nothing below may guess about Pokémon not held yet)
            break;
        }
        sTxn[n++] = MP_TXN_HOLD;
        sTxn[n++] = s / IN_BOX_COUNT;
        sTxn[n++] = s % IN_BOX_COUNT;
        MPNet_Log("shared PC: hold box %d slot %d", s / IN_BOX_COUNT + 1, s % IN_BOX_COUNT);
        Put32(sTxn + n, id.pers);
        Put32(sTxn + n + 4, id.otid);
        n += 8;
        memcpy(newHand[newHandCount++], sBase[s], BLOB);
        sBaseUsed[s] = 0;
        memset(sBase[s], 0, BLOB);
        any = TRUE;
    }
    // what was already in our hand counts too
    for (i = 0; i < sHandCount && newHandCount < MP_STORE_HAND; i++)
        memcpy(newHand[newHandCount++], sHand[i], BLOB);

    // 2. Slots with a Pokémon the server doesn't have there: from our hand (PLACE), or from our own game (IN)
    for (s = 0; s < SLOTS && !truncated; s++)
    {
        struct BoxPokemon *m = &sMirror.boxes[s / IN_BOX_COUNT][s % IN_BOX_COUNT];
        struct Ident id;
        u8 k, blob[BLOB];
        if (!HasMon(m) || MirrorMatchesBase(s))
            continue;
        id = BoxIdent(m);
        for (k = 0; k < newHandCount; k++)
            if (SameIdent(BlobIdent(newHand[k]), id))
                break;
        if (n + 3 + BLOB > sizeof(sTxn))
        {
            truncated = TRUE;
            break;
        }
        MPNet_Log("shared PC: %s box %d slot %d", k < newHandCount ? "place" : "put in", s / IN_BOX_COUNT + 1, s % IN_BOX_COUNT);
        if (k < newHandCount)
        {
            MakeBlob(m, newHand[k] + BY_OFFSET, blob);
            sTxn[n++] = MP_TXN_PLACE;
            memcpy(newHand[k], newHand[newHandCount - 1], BLOB);
            newHandCount--;
        }
        else
        {
            MakeBlob(m, gSaveBlock2Ptr->playerName, blob);
            sTxn[n++] = MP_TXN_IN;
        }
        sTxn[n++] = s / IN_BOX_COUNT;
        sTxn[n++] = s % IN_BOX_COUNT;
        memcpy(sTxn + n, blob, BLOB);
        n += BLOB;
        sBaseUsed[s] = 1;
        memcpy(sBase[s], blob, BLOB);
        any = TRUE;
    }

    // 3. What's left in our hand: still held, or gone to our own game (OUT), or released
    sHandCount = 0;
    for (i = 0; i < newHandCount; i++)
    {
        struct Ident id = BlobIdent(newHand[i]);
        u8 where, box, pos, r;
        if (InList(held, heldCount, id) || MirrorFind(id) < SLOTS || truncated)
        {
            // still held, or waiting to be put down (a cut-short transaction): still in our hand
            memcpy(sHand[sHandCount++], newHand[i], BLOB);
            continue;
        }
        for (r = 0; r < sReleasedCount && !SameIdent(sReleased[r], id); r++)
            ;
        if (!FindPersonal(id, &where, &box, &pos) && r == sReleasedCount)
        {
            // not held, not in a box, not ours, not released: it can't just go. Back into a free shared slot (put
            // down with the next transaction)
            u16 s2;
            for (s2 = 0; s2 < SLOTS; s2++)
            {
                struct BoxPokemon *dst = &sMirror.boxes[s2 / IN_BOX_COUNT][s2 % IN_BOX_COUNT];
                if (!HasMon(dst) && !sBaseUsed[s2])
                {
                    *dst = *(struct BoxPokemon *)newHand[i];
                    MarkChanged(s2);
                    break;
                }
            }
            memcpy(sHand[sHandCount++], newHand[i], BLOB);
            MPNet_Log("shared PC: a shared Moemon went missing here; back in box %d slot %d", s2 / IN_BOX_COUNT + 1,
                      s2 % IN_BOX_COUNT);
            continue;
        }
        if (r < sReleasedCount)
            sReleased[r] = sReleased[--sReleasedCount];
        if (n + 9 > sizeof(sTxn))
        {
            memcpy(sHand[sHandCount++], newHand[i], BLOB);
            continue;
        }
        if (FindPersonal(id, &where, &box, &pos))
        {
            // ours now: into the POKeDEX like a catch
            struct Pokemon mon;
            memcpy(&mon, newHand[i], sizeof(mon));
            if (!GetMonData(&mon, MON_DATA_IS_EGG))
            {
                u16 dex = SpeciesToNationalPokedexNum(GetMonData(&mon, MON_DATA_SPECIES));
                GetSetPokedexFlag(dex, FLAG_SET_SEEN);
                GetSetPokedexFlag(dex, FLAG_SET_CAUGHT);
            }
            sTxn[n++] = MP_TXN_OUT;
        }
        else
            sTxn[n++] = MP_TXN_RELEASE;
        Put32(sTxn + n, id.pers);
        Put32(sTxn + n + 4, id.otid);
        n += 8;
        any = TRUE;
    }

    // 4. Box names and wallpapers
    for (b = 0; b < TOTAL_BOXES_COUNT; b++)
    {
        if (memcmp(sMirror.boxNames[b], sBaseNames[b], MP_STORE_BOXNAME) == 0 && sMirror.boxWallpapers[b] == sBaseWallpapers[b])
            continue;
        if (n + 3 + MP_STORE_BOXNAME > sizeof(sTxn))
            break;
        sTxn[n++] = MP_TXN_BOXINFO;
        sTxn[n++] = b;
        memcpy(sTxn + n, sMirror.boxNames[b], MP_STORE_BOXNAME);
        sTxn[n + MP_STORE_BOXNAME - 1] = EOS;
        n += MP_STORE_BOXNAME;
        sTxn[n++] = sMirror.boxWallpapers[b];
        memcpy(sBaseNames[b], sMirror.boxNames[b], MP_STORE_BOXNAME);
        sBaseWallpapers[b] = sMirror.boxWallpapers[b];
        any = TRUE;
    }

    if (!any)
        return FALSE;
    sSeq++;
    sPending = TRUE;
    sPendingTimer = 0;
    StoreSend(sTxn, n);
    MPNet_Log("shared PC: transaction %d (%d bytes, %d in hand)", sSeq, n, sHandCount);
    return TRUE;
}

static void OnTxnResult(const u8 *data, u16 len)
{
    u16 pos = 3;
    u8 nRefused, nLost, i, b;
    if (len < 4)
        return;
    nRefused = data[pos++];
    for (i = 0; i < nRefused && pos + 8 <= len; i++, pos += 8)
    {
        struct Ident id = {Get32(data + pos), Get32(data + pos + 4)};
        u16 s = MirrorFind(id);
        // the shared PC is full: back into our own game
        if (s < SLOTS)
        {
            struct BoxPokemon mon = sMirror.boxes[s / IN_BOX_COUNT][s % IN_BOX_COUNT];
            ZeroBoxMonData(&sMirror.boxes[s / IN_BOX_COUNT][s % IN_BOX_COUNT]);
            sBaseUsed[s] = 0;
            memset(sBase[s], 0, BLOB);
            MarkChanged(s);
            GiveBackPersonal(&mon);
        }
        snprintf(sNote, sizeof(sNote), "The shared PC is full.");
        MPNet_Log("shared PC: a Pokemon didn't fit and came back");
    }
    if (pos >= len)
        return;
    nLost = data[pos++];
    for (i = 0; i < nLost && pos + 8 <= len; i++, pos += 8)
    {
        struct Ident id = {Get32(data + pos), Get32(data + pos + 4)};
        AddLost(id);
        snprintf(sNote, sizeof(sNote), "Someone else got that one first!");
        MPNet_Log("shared PC: lost a Pokemon to somebody quicker");
    }
    // our hand, as the server sees it
    if (pos < len)
    {
        u8 count = data[pos++];
        sHandCount = 0;
        for (i = 0; i < count && pos + BLOB <= len && sHandCount < MP_STORE_HAND; i++, pos += BLOB)
            memcpy(sHand[sHandCount++], data + pos, BLOB);
    }
    sPending = FALSE;
    // boxes that came while we waited
    for (b = 0; b < TOTAL_BOXES_COUNT; b++)
        if (sStashLen[b])
        {
            ApplyBox(sStash[b], sStashLen[b]);
            sStashLen[b] = 0;
        }
    if (sLostCount)
        TakeBackLost();
}

void MP_SharedBox_OnMessage(const u8 *data, u16 len)
{
    if (!sOpen || len < 1)
        return;
    switch (data[0])
    {
    case MP_ST_BOX_DATA:
        // kept (the latest of each box) until the box screen is steady and no transaction is out
        if (len >= 2 && data[1] < TOTAL_BOXES_COUNT && len <= sizeof(sStash[0]))
        {
            memcpy(sStash[data[1]], data, len);
            sStashLen[data[1]] = len;
        }
        break;
    case MP_ST_TXN_RESULT:
        if (len >= 3 && len <= sizeof(sResult) && sPending && (data[1] | (data[2] << 8)) == sSeq)
        {
            memcpy(sResult, data, len);
            sResultLen = len;
        }
        break;
    case MP_ST_CURSOR:
        MP_SharedBox_OnCursor(data, len);
        break;
    }
}

// ---------------------------------------------------------------------------
// Opening, every frame, closing

void MP_SharedBox_Begin(void)
{
    u8 op[2] = {MP_ST_BOX_GET, 0xFF};
    memset(&sMirror, 0, sizeof(sMirror));
    memset(sBaseUsed, 0, sizeof(sBaseUsed));
    memset(sBaseNames, 0, sizeof(sBaseNames));
    memset(sBaseWallpapers, 0, sizeof(sBaseWallpapers));
    memset(sStashLen, 0, sizeof(sStashLen));
    sResultLen = 0;
    memset(sChangedCells, 0, sizeof(sChangedCells));
    sChangedInfo = 0;
    sHandCount = 0;
    sLostCount = 0;
    sReleasedCount = 0;
    sLoadedMask = 0;
    sPending = FALSE;
    sClosing = FALSE;
    sChangedForUi = FALSE;
    sNote[0] = 0;
    sOpen = TRUE;
    sMirror.currentBox = 0;
    StoreSend(op, 2);
    MP_SharedBox_ResetCursors();
}

bool8 MP_SharedBox_Loaded(void)
{
    return sOpen && sLoadedMask == (1 << TOTAL_BOXES_COUNT) - 1;
}

bool8 MP_SharedBox_IsOpen(void)
{
    return sOpen;
}

// Every frame (MP_FrameUpdate): sends what changed, when the box screen is steady
void MP_SharedBox_Update(void)
{
    if (!sOpen)
        return;
    if (!Online())
    {
        MP_SharedBox_ConnectionLost();
        return;
    }
    if (sPending && !sResultLen)
    {
        if (++sPendingTimer > 60 * 30)
        {
            // no answer in half a minute: the link is as good as gone
            MPNet_Log("shared PC: no answer from the server");
            MP_SharedBox_ConnectionLost();
        }
        return;
    }
    // everything that touches the boxes waits for the box screen to be steady (between moves)
    if (!sClosing && !MP_PcStorage_Steady() && MP_SharedBox_Loaded())
        return;
    if (sResultLen)
    {
        OnTxnResult(sResult, sResultLen);   // also applies the boxes that came meanwhile
        sResultLen = 0;
    }
    else
    {
        u8 b;
        for (b = 0; b < TOTAL_BOXES_COUNT; b++)
            if (sStashLen[b])
            {
                ApplyBox(sStash[b], sStashLen[b]);
                sStashLen[b] = 0;
            }
    }
    if (!MP_SharedBox_Loaded())
        return;
    Flush();
    if (sClosing && !sPending)
    {
        sOpen = FALSE;
        sClosing = FALSE;
        MPNet_Log("shared PC: closed");
    }
}

// The box screen closed (nothing held any more): send what's left, then stop
void MP_SharedBox_End(void)
{
    MP_SharedBox_ShowShared(FALSE);
    if (sOpen)
        sClosing = TRUE;
    MP_SharedBox_SendCursor(0, 0, 0, 0, 0, 0);
}

bool8 MP_SharedBox_Busy(void)
{
    return sOpen && (sPending || sClosing);
}

// The link is gone: whatever we hold that came from the shared PC goes back to it (the server puts it back), our
// own boxes come back to the screen, and the box screen closes
void MP_SharedBox_ConnectionLost(void)
{
    u8 i;
    if (!sOpen)
        return;
    for (i = 0; i < sHandCount; i++)
        AddLost(BlobIdent(sHand[i]));
    sHandCount = 0;
    TakeBackLost();
    sOpen = FALSE;
    sPending = FALSE;
    sClosing = FALSE;
    MP_SharedBox_ShowShared(FALSE);
    MP_PcStorage_ForceClose();
    MPNet_Log("shared PC: the connection was lost");
}

// For the box screen: boxes that changed under it (cells per box; cleared once read)
bool8 MP_SharedBox_TakeChanges(u32 *cells)
{
    bool8 any = sChangedForUi;
    if (any)
        memcpy(cells, sChangedCells, sizeof(sChangedCells));
    memset(sChangedCells, 0, sizeof(sChangedCells));
    sChangedForUi = FALSE;
    return any;
}

// Boxes whose name (bits 0-15) or wallpaper (bits 16-31) changed under the box screen (cleared once read)
u32 MP_SharedBox_TakeInfoChanges(void)
{
    u32 m = sChangedInfo;
    sChangedInfo = 0;
    return m;
}

// A note for the box screen ("" if none; cleared once read)
const char *MP_SharedBox_TakeNote(void)
{
    static char sOut[64];
    snprintf(sOut, sizeof(sOut), "%s", sNote);
    sNote[0] = 0;
    return sOut;
}

// Who put the Pokémon in this shared slot (NULL: unknown)
const u8 *MP_SharedBox_PutInBy(u8 box, u8 pos)
{
    u16 s = box * IN_BOX_COUNT + pos;
    if (box >= TOTAL_BOXES_COUNT || pos >= IN_BOX_COUNT || !sBaseUsed[s])
        return NULL;
    return sBase[s] + BY_OFFSET;
}

// A box Pokémon's "who put it in", wherever it is in the shared PC or our hand
const u8 *MP_SharedBox_PutInByMon(const struct BoxPokemon *b)
{
    return KnownBy(BoxIdent(b));
}

// Is this Pokémon from the shared PC (in a shared slot or our hand)?
bool8 MP_SharedBox_IsShared(const struct BoxPokemon *b)
{
    struct Ident id = BoxIdent(b);
    u16 s;
    if (HandHas(id))
        return TRUE;
    for (s = 0; s < SLOTS; s++)
        if (sBaseUsed[s] && SameIdent(BlobIdent(sBase[s]), id))
            return TRUE;
    return FALSE;
}

// The mirror (for SEND TO SHARED: the first free shared slot)
struct PokemonStorage *MP_SharedBox_Mirror(void)
{
    return &sMirror;
}

// ---------------------------------------------------------------------------
// Friends' cursors. Payload: u8 view (MP_PCV_*), u8 box, s16 x, s16 y, u16 species held, u8 count held, u16 row

static EWRAM_DATA struct MpPcCursor sCursors[MP_MAX_PLAYERS] = {0};
static EWRAM_DATA u8 sLastSent[9] = {0};
static u16 sSendTimer;

void MP_SharedBox_ResetCursors(void)
{
    memset(sCursors, 0, sizeof(sCursors));
    memset(sLastSent, 0xFF, sizeof(sLastSent));
}

// Where our cursor is (sent when it changes, at most every 3 frames, and every 2 seconds anyway)
void MP_SharedBox_SendCursor(u8 view, u8 box, s16 x, s16 y, u16 species, u8 count)
{
    MP_SharedBox_SendCursorRow(view, box, x, y, species, count, 0);
}

void MP_SharedBox_SendCursorRow(u8 view, u8 box, s16 x, s16 y, u16 species, u8 count, u16 row)
{
    u8 msg[1 + 11];
    msg[0] = MP_ST_CURSOR;
    msg[1] = view;
    msg[2] = box;
    msg[3] = x & 0xFF;
    msg[4] = (x >> 8) & 0xFF;
    msg[5] = y & 0xFF;
    msg[6] = (y >> 8) & 0xFF;
    msg[7] = species & 0xFF;
    msg[8] = species >> 8;
    msg[9] = count;
    msg[10] = row & 0xFF;
    msg[11] = row >> 8;
    sSendTimer++;
    if (view == MP_PCV_NONE && sLastSent[0] == MP_PCV_NONE)
        return;
    if (memcmp(sLastSent, msg + 1, sizeof(sLastSent)) == 0 && sSendTimer < 120)
        return;
    if (sSendTimer < 3 && view != MP_PCV_NONE)
        return;
    memcpy(sLastSent, msg + 1, sizeof(sLastSent));
    sSendTimer = 0;
    StoreSend(msg, sizeof(msg));
}

void MP_SharedBox_OnCursor(const u8 *data, u16 len)
{
    struct MpPcCursor *c;
    if (len < 2 + 11 || data[1] >= MP_MAX_PLAYERS || data[1] == gMpLocalId)
        return;
    c = &sCursors[data[1]];
    c->view = data[2];
    c->box = data[3];
    c->x = (s16)(data[4] | (data[5] << 8));
    c->y = (s16)(data[6] | (data[7] << 8));
    c->species = data[8] | (data[9] << 8);
    c->count = data[10];
    c->row = data[11] | (data[12] << 8);
    c->heard = gMain.vblankCounter1;
}

// A friend's cursor (FALSE: not in a shared PC screen, or gone)
bool8 MP_SharedBox_GetCursor(u8 player, struct MpPcCursor *out)
{
    if (player >= MP_MAX_PLAYERS || player == gMpLocalId || !gMpPlayers[player].present)
        return FALSE;
    *out = sCursors[player];
    return out->view != MP_PCV_NONE && gMain.vblankCounter1 - out->heard < 60 * 10;
}

// ---------------------------------------------------------------------------
// Test harness (mp_debug.c)

// Puts a box Pokémon straight into a shared slot (a transaction of its own; its answer is ignored)
void MP_SharedBox_DebugIn(u8 box, u8 pos, const struct BoxPokemon *b)
{
    u8 msg[3 + 3 + BLOB];
    u16 n = 0;
    msg[n++] = MP_ST_TXN;
    msg[n++] = 0xFF;
    msg[n++] = 0xFF;
    msg[n++] = MP_TXN_IN;
    msg[n++] = box;
    msg[n++] = pos;
    MakeBlob(b, gSaveBlock2Ptr->playerName, msg + n);
    n += BLOB;
    StoreSend(msg, n);
}

// The mirror's box: "pos:species(by)" for each Pokémon
void MP_SharedBox_DebugDump(u8 box)
{
    u8 p;
    char line[1024];
    int n = 0;
    if (box >= TOTAL_BOXES_COUNT)
        return;
    n += snprintf(line + n, sizeof(line) - n, "[test] sharedbox %d:", box + 1);
    for (p = 0; p < IN_BOX_COUNT; p++)
    {
        const struct BoxPokemon *m = &sMirror.boxes[box][p];
        if (HasMon(m))
        {
            char by[16] = "?";
            if (sBaseUsed[box * IN_BOX_COUNT + p])
                MP_GameToAscii(by, sBase[box * IN_BOX_COUNT + p] + BY_OFFSET, PLAYER_NAME_LENGTH);
            n += snprintf(line + n, sizeof(line) - n, " %d:%d(%s)", p, (int)GetBoxMonData((struct BoxPokemon *)m, MON_DATA_SPECIES),
                          by);
            if (GetBoxMonData((struct BoxPokemon *)m, MON_DATA_MARKINGS))
                n += snprintf(line + n, sizeof(line) - n, "m%d", (int)GetBoxMonData((struct BoxPokemon *)m, MON_DATA_MARKINGS));
        }
    }
    n += snprintf(line + n, sizeof(line) - n, " | hand %d%s%s", sHandCount, sPending ? " pending" : "", sOpen ? "" : " closed");
    if (sMirror.boxWallpapers[box] != box % 12)
        n += snprintf(line + n, sizeof(line) - n, " wallpaper %d", sMirror.boxWallpapers[box]);
    {
        char name[16], def[16];
        MP_GameToAscii(name, sMirror.boxNames[box], BOX_NAME_LENGTH);
        snprintf(def, sizeof(def), "SHARED%d", box + 1);
        if (strcmp(name, def) != 0)
            n += snprintf(line + n, sizeof(line) - n, " name %s", name);
    }
    printf("%s\n", line);
}

void MP_SharedBox_DebugName(u8 box, const char *name)
{
    u8 buf[64];
    if (box >= TOTAL_BOXES_COUNT)
        return;
    MP_AsciiToGame(buf, name, BOX_NAME_LENGTH);
    memset(sMirror.boxNames[box], EOS, BOX_NAME_LENGTH + 1);
    memcpy(sMirror.boxNames[box], buf, StringLength(buf));
}

// Every Pokémon we know of, by identity: "own" (party and our boxes) and "shared" (the mirror, our hand)
void MP_SharedBox_DebugIds(void)
{
    struct PokemonStorage *own = MP_PersonalStorage();
    u16 s;
    u8 i;
    for (i = 0; i < PARTY_SIZE; i++)
        if (GetMonData(&gPlayerParty[i], MON_DATA_SANITY_HAS_SPECIES))
            printf("[test] monid own %08x:%08x %d party\n", (unsigned)Get32((u8 *)&gPlayerParty[i]),
                   (unsigned)Get32((u8 *)&gPlayerParty[i] + 4), (int)GetMonData(&gPlayerParty[i], MON_DATA_SPECIES));
    for (s = 0; s < SLOTS; s++)
    {
        const struct BoxPokemon *b = &own->boxes[s / IN_BOX_COUNT][s % IN_BOX_COUNT];
        if (HasMon(b))
            printf("[test] monid own %08x:%08x %d box\n", (unsigned)Get32((const u8 *)b), (unsigned)Get32((const u8 *)b + 4),
                   (int)GetBoxMonData((struct BoxPokemon *)b, MON_DATA_SPECIES));
    }
    for (s = 0; s < SLOTS; s++)
        if (sBaseUsed[s])
            printf("[test] monid shared %08x:%08x\n", (unsigned)Get32(sBase[s]), (unsigned)Get32(sBase[s] + 4));
    for (i = 0; i < sHandCount; i++)
        printf("[test] monid hand %08x:%08x\n", (unsigned)Get32(sHand[i]), (unsigned)Get32(sHand[i] + 4));
    printf("[test] monids done\n");
}

// The box screen released this Pokémon (ReleaseMon, a TAG release): if it's a shared one, it may go
void MP_SharedBox_NoteRelease(const struct BoxPokemon *b)
{
    if (!sOpen || sReleasedCount >= ARRAY_COUNT(sReleased))
        return;
    sReleased[sReleasedCount++] = BoxIdent(b);
}
