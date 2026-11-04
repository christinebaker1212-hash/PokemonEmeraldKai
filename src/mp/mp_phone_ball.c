// Moemon Co-op: a Moemon's POKé BALL on the phone (the app's POKé BALL page, docs/PHONE_LINK.md "The POKé BALL
// page"): its ball, color and decoration as the summary's POKé BALL page changes them (pokemon_summary_screen.c
// MpBall_*: another kind of ball costs one from the BAG, a color only when the save has earned it), and its
// entrance / recall effect as the ball effect editor sets it (mp_ballfx_editor.c: only pieces the save has, MY PRESET
// 1-6, APPLY TO PARTY). The snapshot's "fxu" (what the save has earned) and "fxp" (MY PRESETs) are written here;
// each Moemon's "bst" / "fx" / "fxs" by mp_phone.c BufBoxMon.
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "item.h"
#include "pokemon.h"
#include "constants/items.h"
#include "mp_phone_int.h"

u8 ItemIdToBallId(u16 ballItem);

// The snapshot's "fxu": per kind (COLOR, SEND-OUT, PARTICLES, RETURN) the pieces this save may pick, as hex (a bit
// each, the first piece first); "fxp": MY PRESET 1-6 as [pack, sound] or 0 when empty
void PhoneBall_Snapshot(struct Buf *b)
{
    u8 kind, i;
    BufAdd(b, ",\"fxu\":[");
    for (kind = 0; kind < MPFX_PIECE_KINDS; kind++)
    {
        u8 n = MP_BallUnlock_Count(kind);
        BufAdd(b, "%s\"", kind ? "," : "");
        for (i = 0; i < n; i += 4)
        {
            u8 nib = 0, k;
            for (k = 0; k < 4 && i + k < n; k++)
                nib |= MP_BallUnlock_Has(kind, i + k) << k;
            BufAdd(b, "%X", nib);
        }
        BufAdd(b, "\"");
    }
    BufAdd(b, "],\"fxp\":[");
    for (i = 0; i < MPFX_USER_PRESETS; i++)
    {
        struct MpBallFx fx;
        if (MP_BallFx_GetUserPreset(i, &fx))
            BufAdd(b, "%s[%u,%u]", i ? "," : "", (unsigned)MP_BallFx_Pack(&fx), fx.sound);
        else
            BufAdd(b, "%s0", i ? "," : "");
    }
    BufAdd(b, "]");
}

// (the snapshot's rev: new pieces earned, MY PRESETs changed)
u32 PhoneBall_Hash(void)
{
    u32 h = 2166136261u, kind, i;
    for (kind = 0; kind < MPFX_PIECE_KINDS; kind++)
        for (i = 0; i < MP_BallUnlock_Count(kind); i++)
            h = (h ^ MP_BallUnlock_Has(kind, i)) * 16777619u;
    for (i = 0; i < MPFX_USER_PRESETS; i++)
    {
        struct MpBallFx fx;
        u32 v = MP_BallFx_GetUserPreset(i, &fx) ? (MP_BallFx_Pack(&fx) | ((u32)fx.sound << 24) | 0x80000000u) : 0;
        h = (h ^ v) * 16777619u;
    }
    return h;
}

// Every piece of fx is one this save may pick, or the one `had` already carries
static bool8 FxAllowed(const struct MpBallFx *fx, const struct MpBallFx *had)
{
    if (fx->form >= MPFX_FORM_COUNT || fx->part >= MPFX_PART_COUNT || fx->recall >= MPFX_RECALL_COUNT
     || fx->color >= MP_BALL_COLORS || fx->sound >= MPFX_SOUND_COUNT)
        return FALSE;
    if (fx->form != had->form && !MP_BallUnlock_Has(MPFX_PIECE_FORM, fx->form))
        return FALSE;
    if (fx->part != had->part && !MP_BallUnlock_Has(MPFX_PIECE_PART, fx->part))
        return FALSE;
    if (fx->recall != had->recall && !MP_BallUnlock_Has(MPFX_PIECE_RECALL, fx->recall))
        return FALSE;
    if (fx->color != had->color && !MP_BallUnlock_Has(MPFX_PIECE_COLOR, fx->color))
        return FALSE;
    return TRUE;
}

static bool8 ParseFx(const char *json, struct MpBallFx *out)
{
    long long pack, snd = 0;
    if (!JsonNum(json, "fx", &pack) || pack < 0 || pack > 0xFFFFFF)
        return FALSE;
    JsonNum(json, "fxs", &snd);
    if (snd < 0 || snd >= MPFX_SOUND_COUNT)
        return FALSE;
    MP_BallFx_Unpack((u32)pack, (u8)snd, out);
    return TRUE;
}

// {"op":"ballstyle","at":..,"p":..,"o":..,"item":BALL,"color":C,"deco":D}: as the summary's POKé BALL page confirms
static const char *DoStyle(const char *json)
{
    struct Loc at;
    long long pid, ot, item, color = 0, deco = 0;
    struct BoxPokemon *box;
    struct Pokemon tmp;
    u16 cur, style, oldStyle;
    u32 it;
    if (!ParseLoc(json, "at", &at) || !JsonNum(json, "p", &pid) || !JsonNum(json, "o", &ot) || Empty(&at)
     || !JsonNum(json, "item", &item))
        return "bad";
    JsonNum(json, "color", &color);
    JsonNum(json, "deco", &deco);
    if (!Is(&at, (u32)pid, (u32)ot))
        return "gone";
    box = BoxAt(&at);
    if (GetBoxMonData(box, MON_DATA_IS_EGG))
        return "egg";
    if (item <= 0 || item >= ITEMS_COUNT || !MP_IsBallItem((u16)item))
        return "bad";
    if (color < 0 || color >= MP_BALL_COLORS || deco < 0 || deco >= MP_BallStyle_DecorationCount())
        return "bad";
    BoxMonToMon(box, &tmp);
    cur = MP_BallStyle_DisplayItem(&tmp);
    oldStyle = MP_BallStyle_OfBoxMon(box);
    if (MP_BallStyle_ItemCanColor((u16)item) && color != MP_BallStyle_Color(oldStyle)
     && !MP_BallUnlock_Has(MPFX_PIECE_COLOR, (u8)color))
        return "locked";
    if (item != cur)
    {
        if (!MP_BallStyle_CanRehouse((u16)item))
            return "bad";
        if (!CheckBagHasItem((u16)item, 1))
            return "noitem";
        RemoveBagItem((u16)item, 1);   // (another kind of ball: it costs one of them)
        sBagChanged = TRUE;
    }
    style = MP_BallStyle_Make(MP_BallStyle_ItemCanColor((u16)item) ? (u8)color : 0,
                              MP_BallStyle_ItemCanDecorate((u16)item) ? (u8)deco : 0) | MP_BallStyle_ForItem((u16)item);
    it = MP_SpecialBall_FromItem((u16)item) ? ITEM_POKE_BALL : (u32)item;   // (special balls: a POKé BALL underneath)
    SetBoxMonData(box, MON_DATA_POKEBALL, &it);
    SetBoxMonData(box, MON_DATA_ENCRYPT_SEPARATOR, &style);
    if (at.kind == LOC_PARTY)
    {
        struct Pokemon *mon = &OwnParty()[at.slot];
        SetMonData(mon, MON_DATA_POKEBALL, &it);
        SetMonData(mon, MON_DATA_ENCRYPT_SEPARATOR, &style);
    }
    if (style != 0)
        MP_Achieve_Unlock("ballstyle");
    Changed(&at);
    return NULL;
}

// {"op":"ballfx","at":..,"p":..,"o":..,"fx":PACK,"fxs":SOUND}: one Moemon's effect, as the editor saves it
static const char *DoFx(const char *json)
{
    struct Loc at;
    long long pid, ot;
    struct BoxPokemon *box;
    struct MpBallFx fx, had;
    if (!ParseLoc(json, "at", &at) || !JsonNum(json, "p", &pid) || !JsonNum(json, "o", &ot) || Empty(&at)
     || !ParseFx(json, &fx))
        return "bad";
    if (!Is(&at, (u32)pid, (u32)ot))
        return "gone";
    box = BoxAt(&at);
    if (GetBoxMonData(box, MON_DATA_IS_EGG))
        return "egg";
    MP_BallFx_OfBoxMon(box, &had);
    if (!FxAllowed(&fx, &had))
        return "locked";
    MP_BallFx_SetBoxMon(box, &fx);
    Changed(&at);
    return NULL;
}

// {"op":"fxparty","fx":PACK,"fxs":SOUND}: every Moemon in the party that isn't an EGG (the editor's APPLY TO PARTY)
static const char *DoParty(const char *json)
{
    struct MpBallFx fx, had;
    u32 i, n = 0;
    if (!ParseFx(json, &fx))
        return "bad";
    memset(&had, 0xFF, sizeof(had));   // (nothing carried counts: every piece must be the save's)
    if (!FxAllowed(&fx, &had))
        return "locked";
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &OwnParty()[i];
        if (GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE || GetMonData(mon, MON_DATA_IS_EGG))
            continue;
        MP_BallFx_SetBoxMon(&mon->box, &fx);
        n++;
    }
    if (n == 0)
        return "nothing";
    PartyChanged();
    return NULL;
}

// {"op":"fxpreset","slot":0-5,"fx":PACK,"fxs":SOUND}: SAVE AS MY PRESET N
static const char *DoPreset(const char *json)
{
    struct MpBallFx fx, had;
    long long slot;
    if (!JsonNum(json, "slot", &slot) || slot < 0 || slot >= MPFX_USER_PRESETS || !ParseFx(json, &fx))
        return "bad";
    memset(&had, 0xFF, sizeof(had));
    if (!FxAllowed(&fx, &had))
        return "locked";
    MP_BallFx_SaveUserPreset((u8)slot, &fx);
    return NULL;
}

const char *PhoneBall_Apply(const char *op, const char *json)
{
    if (strcmp(op, "ballstyle") == 0)
        return DoStyle(json);
    if (strcmp(op, "ballfx") == 0)
        return DoFx(json);
    if (strcmp(op, "fxparty") == 0)
        return DoParty(json);
    if (strcmp(op, "fxpreset") == 0)
        return DoPreset(json);
    return (const char *)-1;   // (not one of these)
}
