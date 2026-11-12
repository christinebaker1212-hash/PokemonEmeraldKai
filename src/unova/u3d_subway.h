// Moemon Kai: UNOVA's BATTLE SUBWAY (src/unova/u3d_subway.c): what the save keeps of it, inside UNOVA's VM state
// (u3d_vm.c sWork). Black's BsubwayPlayData and BsubwayScoreData (arm9 gds_profile.c), the fields its scripts use
#ifndef GUARD_U3D_SUBWAY_H
#define GUARD_U3D_SUBWAY_H

#define U3D_SUBWAY_MODES 9   // (single, double, multi, wireless multi, Wi-Fi; super single, double, multi, wireless)

struct U3DSubwayDraw { u32 otId, pids[2]; u16 ids[2]; u8 natures[2], pad[2]; };   // (a partner's two drawn sets)

struct U3DSubwaySave
{
    struct
    {
        u8 mode, round, flag01, count03, partnerNo, partnerResult, byte10, byte11;
        u16 count04, count06;
        u8 members[4];
        u16 trainerNo[14];
        struct U3DSubwayDraw draw;
    } play;
    struct
    {
        u16 winCount[U3D_SUBWAY_MODES], maxWin[U3D_SUBWAY_MODES], clearCount[U3D_SUBWAY_MODES];
        u32 flags;
        u8 level, streak, pad[2];
    } score;
};

// u3d_vm.c's BSUBWAY_SCR_CmdTool: what the VM does after the command (U3D_Subway_Tool's answer)
enum { U3D_SUBWAY_NONE, U3D_SUBWAY_PARTY, U3D_SUBWAY_BATTLE, U3D_SUBWAY_MESSAGE, U3D_SUBWAY_YESNO, U3D_SUBWAY_RESET,
       U3D_SUBWAY_VM };   // (VM: a command about the field's people or places, the VM's own)

void U3D_Subway_Bind(struct U3DSubwaySave *save);
void U3D_Subway_CreateWork(u16 resume, u16 playMode);
void U3D_Subway_DeleteWork(void);
u8 U3D_Subway_Tool(u16 cmd, u16 op1, u16 op2, u16 *result, u16 *msgBank, u16 *msg);
void U3D_Subway_PartyChosen(void);
void U3D_Subway_StartBattle(void);
void U3D_Subway_BattleEnded(bool8 won);
bool8 U3D_Subway_Return(u8 index, u16 *zone, s16 *x, s16 *z);
bool8 U3D_Subway_Passenger(u32 i, u16 *code, s16 *x, s16 *z, u16 *param);
u8 U3D_Subway_PlayMode(void);
u8 U3D_Subway_MemberNum(void);
bool8 U3D_Subway_Choosing(void);
bool8 U3D_Subway_Banned(u16 species);
bool8 U3D_Subway_PartyUsable(u16 regulation);

#endif
