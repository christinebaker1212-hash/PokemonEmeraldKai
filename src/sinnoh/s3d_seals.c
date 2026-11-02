// Moemon Kai: Platinum's Seals as this game's POKé BALL effects (the user, 2026-11-02: "Poke Ball effects take the place
// of Seals wherever they would appear in the story, or in menus or shops"). Each of Platinum's Seals stands for one of
// the earned ball effect pieces (mp_ballunlock.c: the MOEGATCHA prizes, in order, round again past the last), so:
// - a Seal given (GiveOrTakeSeal: the SOLACEON house) unlocks its piece;
// - a Seal counted (CountSealOccurence) is 99 (Platinum's full case) once its piece is unlocked, else 0; the unique
//   count is the pieces unlocked;
// - a Seal's name is its piece's ("SEND-OUT: SAKURA STORM");
// - the SUNYSHORE MARKET's Seal shop sells the day's Seals' pieces at Platinum's prices (gSinnohSealPrices /
//   gSinnohSealShops from tools/sinnoh/tv_data.py);
// - the PC's BALL CAPSULES are the summary's POKé BALL page (its editor: mp_ballfx_editor.c), as for every region.
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "event_data.h"
#include "menu.h"
#include "money.h"
#include "script.h"
#include "string_util.h"
#include "mp.h"
#include "s3d.h"

extern const u16 gSinnohSealPrices[];
extern const u8 gSinnohSealShops[];

#define SEAL_COUNT 82

static bool8 SealPiece(u16 seal, u8 *kind, u8 *index)
{
    u8 n = MP_BallUnlock_PrizeCount();
    if (seal == 0 || seal >= SEAL_COUNT || n == 0)
        return FALSE;
    return MP_BallUnlock_Prize((seal - 1) % n, kind, index);
}

static bool8 SealHas(u16 seal)
{
    u8 kind, index;
    return SealPiece(seal, &kind, &index) && MP_BallUnlock_Has(kind, index);
}

static void SealName(u16 seal, u8 *dest)
{
    u8 kind, index;
    char name[40];
    if (!SealPiece(seal, &kind, &index))
    {
        *dest = EOS;
        return;
    }
    MP_BallUnlock_PieceName(kind, index, name, sizeof(name));
    MP_AsciiToGame(dest, name, 39);
}

static u16 UniqueCount(void)
{
    u8 kind, i;
    u16 n = 0;
    for (kind = 0; kind < MPFX_PIECE_KINDS; kind++)
        for (i = MP_BallUnlock_First(kind); i < MP_BallUnlock_Count(kind); i++)
            n += MP_BallUnlock_Has(kind, i);
    return n;
}

// The SUNYSHORE MARKET's shop: its lines, then a purchase (SEALCALL_SHOP_MENU builds them into the SINNOH menu)
static u8 sShop;

static void Sinnoh_MenuAddText(const u8 *text, u16 value);   // (below: s3d_scrcmd.c's menu)

enum {
    SEALCALL_GIVE,         // A: the Seal (a value or variable), B: how many (a value or variable): its piece unlocked
    SEALCALL_COUNT,        // A: the Seal, B: variable <- 99 when its piece is unlocked, else 0
    SEALCALL_UNIQUE,       // A: variable <- the pieces unlocked
    SEALCALL_NAME,         // A: the string slot, B: the Seal: its piece's name
    SEALCALL_SHOP_MENU,    // A: Platinum's MART_SEAL_ID (0-6, Monday first): the SINNOH menu's lines, each its Seal
    SEALCALL_BUY,          // A: the Seal's variable, B: variable <- 0 not enough money, 1 bought, 2 had it already
};

u8 *StringVar(u16 slot);   // (s3d_scrcmd.c)

void Sinnoh_SealCall(u16 call, u16 a, u16 b)
{
    u8 kind, index;
    switch (call)
    {
    case SEALCALL_GIVE:
        if ((s16)VarGet(b) > 0 && SealPiece(VarGet(a), &kind, &index))
            MP_BallUnlock_Give(kind, index);
        break;
    case SEALCALL_COUNT:
        VarSet(b, SealHas(VarGet(a)) ? 99 : 0);
        break;
    case SEALCALL_UNIQUE:
        VarSet(a, UniqueCount());
        break;
    case SEALCALL_NAME:
        SealName(VarGet(b), StringVar(a));
        break;
    case SEALCALL_SHOP_MENU:
    {
        u32 i;
        sShop = VarGet(a) < 7 ? VarGet(a) : 0;
        for (i = 0; i < 8; i++)
        {
            u16 seal = gSinnohSealShops[sShop * 8 + i];
            u8 line[64], *p;
            if (seal == 0)
                break;
            SealName(seal, line);
            p = line + StringLength(line);
            *p++ = CHAR_SPACE;
            if (SealHas(seal))
            {
                static const u8 sHad[] = _("(OWNED)");
                StringCopy(p, sHad);
            }
            else
            {
                *p++ = CHAR_CURRENCY;
                ConvertIntToDecimalStringN(p, gSinnohSealPrices[seal], STR_CONV_MODE_LEFT_ALIGN, 4);
            }
            Sinnoh_MenuAddText(line, seal);
        }
        break;
    }
    case SEALCALL_BUY:
    {
        u16 seal = VarGet(a);
        if (SealHas(seal))
            VarSet(b, 2);
        else if (seal >= SEAL_COUNT || !IsEnoughMoney(&gSaveBlock1Ptr->money, gSinnohSealPrices[seal]))
            VarSet(b, 0);
        else
        {
            RemoveMoney(&gSaveBlock1Ptr->money, gSinnohSealPrices[seal]);
            if (SealPiece(seal, &kind, &index))
                MP_BallUnlock_Give(kind, index);
            SealName(seal, gStringVar1);
            VarSet(b, 1);
        }
        break;
    }
    }
}

static void Sinnoh_MenuAddText(const u8 *text, u16 value)
{
    void Sinnoh_MenuAddLine(const u8 *text, u16 value);   // (s3d_scrcmd.c)
    Sinnoh_MenuAddLine(text, value);
}
