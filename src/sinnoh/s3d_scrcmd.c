// Moemon Kai: this game's SINNOH script command (data/script_cmd_table.inc 0xF6, `sinnoh OP, ARG`): what Platinum's
// scripts do that this game's commands don't, for the scripts tools/sinnoh/gen_scripts.py converts from Platinum's own
// (docs/SINNOH_HANDOFF.md step 4).
#include "global.h"
#include "event_data.h"
#include "script.h"
#include "string_util.h"
#include "constants/vars.h"

enum {
    SINNOH_BUFFER_PLAYER = 1,       // ARG: Platinum's string buffer 0-2 (STR_VAR_1-3) <- the player's name
    SINNOH_BUFFER_RIVAL,            // ... <- the rival's (BARRY)
    SINNOH_BUFFER_COUNTERPART,      // ... <- the counterpart's (DAWN for a boy, LUCAS for a girl)
    SINNOH_YESNO,                   // ARG: a variable <- the YES / NO just asked, as Platinum's menus give it (YES 0)
};

static u8 *StringVar(u16 n)
{
    return n == 0 ? gStringVar1 : n == 1 ? gStringVar2 : gStringVar3;
}

bool8 ScrCmd_sinnoh(struct ScriptContext *ctx)
{
    static const u8 sBarry[] = _("BARRY"), sDawn[] = _("DAWN"), sLucas[] = _("LUCAS");
    u8 op = ScriptReadByte(ctx);
    u16 arg = ScriptReadHalfword(ctx);
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
    }
    return FALSE;
}
