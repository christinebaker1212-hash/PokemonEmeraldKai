// Moemon Kai: this game's SINNOH script command (data/script_cmd_table.inc 0xF6, `sinnoh OP, ARG`): what Platinum's
// scripts do that this game's commands don't, for the scripts tools/sinnoh/gen_scripts.py converts from Platinum's own
// (docs/SINNOH_HANDOFF.md step 4).
#include "global.h"
#include "event_data.h"
#include "script.h"
#include "string_util.h"
#include "constants/vars.h"
#include "constants/item.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "item.h"
#include "fieldmap.h"
#include "party_menu.h"
#include "mp_region.h"
#include "s3d.h"

#include "data.h"
#include "pokedex.h"

enum {
    SINNOH_BUFFER_PLAYER = 1,       // ARG: Platinum's string buffer 0-2 (STR_VAR_1-3) <- the player's name
    SINNOH_BUFFER_RIVAL,            // ... <- the rival's (BARRY)
    SINNOH_BUFFER_COUNTERPART,      // ... <- the counterpart's (DAWN for a boy, LUCAS for a girl)
    SINNOH_YESNO,                   // ARG: a variable <- the YES / NO just asked, as Platinum's menus give it (YES 0)
    SINNOH_GET_DIR,                 // ARG: a variable <- the way the player faces, Platinum's numbers (N 0 S 1 W 2 E 3)
    SINNOH_GET_POS,                 // ARG, ARG2: variables <- the player's tile in Platinum's map coordinates
    SINNOH_GET_POCKET,              // ARG: an item (or a variable holding it), ARG2: a variable <- its Platinum pocket
    SINNOH_IS_TMHM,                 // ARG: an item, ARG2: a variable <- TRUE for a TM / HM
    SINNOH_BUFFER_TMHM_MOVE,        // ARG: a string buffer, ARG2: a TM / HM <- its move's name
    SINNOH_BUFFER_POCKET,           // ARG: a string buffer, ARG2: a Platinum pocket <- its name
    SINNOH_BADGE_CHECK,             // ARG: Platinum's badge (BADGE_ID_*): VAR_RESULT and the comparison <- held or not
    SINNOH_NATIONAL_DEX,            // ARG: a variable <- the NATIONAL DEX is on
};

// Platinum's pockets (include/constants/items.h there): ITEMS 0, MEDICINE 1, BALLS 2, TMHMS 3, BERRIES 4, MAIL 5,
// BATTLE ITEMS 6, KEY ITEMS 7
static u16 PlatinumPocket(u16 item)
{
    switch (GetItemPocket(item))
    {
    case POCKET_POKE_BALLS: return 2;
    case POCKET_TM_HM: return 3;
    case POCKET_BERRIES: return 4;
    case POCKET_KEY_ITEMS: return 7;
    default: return 0;
    }
}

static u8 *StringVar(u16 n)
{
    n = VarGet(n);   // (a buffer number, or a variable holding one)
    return n == 0 ? gStringVar1 : n == 1 ? gStringVar2 : gStringVar3;
}

static const u8 sPocket0[] = _("ITEMS"), sPocket1[] = _("MEDICINE"), sPocket2[] = _("POKé BALLS"),
                sPocket3[] = _("TMs & HMs"), sPocket4[] = _("BERRIES"), sPocket5[] = _("MAIL"),
                sPocket6[] = _("BATTLE ITEMS"), sPocket7[] = _("KEY ITEMS");
static const u8 *const sPockets[] = {sPocket0, sPocket1, sPocket2, sPocket3, sPocket4, sPocket5, sPocket6, sPocket7};

bool8 ScrCmd_sinnoh(struct ScriptContext *ctx)
{
    static const u8 sBarry[] = _("BARRY"), sDawn[] = _("DAWN"), sLucas[] = _("LUCAS");
    u8 op = ScriptReadByte(ctx);
    u16 arg = ScriptReadHalfword(ctx);
    u16 arg2 = ScriptReadHalfword(ctx);
    switch (op)
    {
    case SINNOH_BUFFER_PLAYER:
        StringCopy(StringVar(arg), gSaveBlock2Ptr->playerName);
        break;
    case SINNOH_BUFFER_RIVAL:
        StringCopy(StringVar(arg), sBarry);
        break;
    case SINNOH_BUFFER_COUNTERPART:
        StringCopy(StringVar(arg), gSaveBlock2Ptr->playerGender == MALE ? sDawn : sLucas);
        break;
    case SINNOH_YESNO:
        VarSet(arg, gSpecialVar_Result == 1 ? 0 : 1);   // (yesnobox: 1 YES)
        break;
    case SINNOH_GET_DIR:
        switch (GetPlayerFacingDirection())
        {
        case DIR_NORTH: VarSet(arg, 0); break;
        case DIR_WEST: VarSet(arg, 2); break;
        case DIR_EAST: VarSet(arg, 3); break;
        default: VarSet(arg, 1); break;
        }
        break;
    case SINNOH_GET_POS:
    {
        s32 cx = 0, cz = 0;
        struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
        S3D_MapCells(&cx, &cz);
        VarSet(arg, player->currentCoords.x - MAP_OFFSET + cx * 32);
        VarSet(arg2, player->currentCoords.y - MAP_OFFSET + cz * 32);
        break;
    }
    case SINNOH_GET_POCKET:
        VarSet(arg2, PlatinumPocket(VarGet(arg)));
        break;
    case SINNOH_IS_TMHM:
        VarSet(arg2, GetItemPocket(VarGet(arg)) == POCKET_TM_HM);
        break;
    case SINNOH_BUFFER_TMHM_MOVE:
    {
        u16 item = VarGet(arg2), move = GetItemPocket(item) == POCKET_TM_HM ? ItemIdToBattleMoveId(item) : MOVE_NONE;
        StringCopy(StringVar(arg), gMoveNames[move < MOVES_COUNT ? move : MOVE_NONE]);
        break;
    }
    case SINNOH_BUFFER_POCKET:
        StringCopy(StringVar(arg), sPockets[VarGet(arg2) < 8 ? VarGet(arg2) : 0]);
        break;
    case SINNOH_NATIONAL_DEX:
        VarSet(arg, IsNationalPokedexEnabled());
        break;
    case SINNOH_BADGE_CHECK:
        gSpecialVar_Result = arg < 8 && FlagGet(FLAG_SINNOH_BADGE01 + arg);
        ctx->comparisonResult = gSpecialVar_Result;   // (as checkflag: goto_if TRUE / FALSE)
        break;
    }
    return FALSE;
}
