#ifndef MUGEN_TRIGGER_H
#define MUGEN_TRIGGER_H

/*
 * CNS triggers (trigger.c): the Trigger struct is in expr.h. Trigger_Parse
 * turns "name" / "name(args)" / "redirect(id), name" into a Trigger token
 * (with the argument expressions before it in the postfix code);
 * Trigger_Eval pops the arguments from the evaluation stack and pushes the
 * value.
 */

/* Trigger.id (TRIGGER_VAR and TRIGGER_FVAR are in expr.h). */
#define TRIGGER_TIME                0x000   /* time, statetime */
#define TRIGGER_ANIMTIME            0x001
#define TRIGGER_ANIMELEM            0x002   /* animelem = n[, op value] */
#define TRIGGER_ANIM                0x003
#define TRIGGER_POS                 0x004   /* pos x / y / z: component in value */
#define TRIGGER_SCREENPOS           0x005
#define TRIGGER_VEL                 0x006
#define TRIGGER_TIMEMOD             0x007   /* timemod = n, op value */
#define TRIGGER_P2DIST              0x008
#define TRIGGER_P2BODYDIST          0x009
#define TRIGGER_P2LIFE              0x00A
#define TRIGGER_FRONTEDGEDIST       0x00B   /* also frontedgebodydist (arg 1) */
#define TRIGGER_BACKEDGEDIST        0x00C   /* also backedgebodydist (arg 1) */
#define TRIGGER_LIFE                0x00F
#define TRIGGER_LIFEMAX             0x010
#define TRIGGER_POWER               0x011
#define TRIGGER_POWERMAX            0x012
#define TRIGGER_ALIVE               0x013
#define TRIGGER_COMMAND             0x014   /* command = "name" */
#define TRIGGER_CAMERAPOS           0x015
#define TRIGGER_CAMERAZOOM          0x016
#define TRIGGER_AILEVEL             0x019
#define TRIGGER_HITCOUNT            0x01E
#define TRIGGER_UNIQHITCOUNT        0x01F
#define TRIGGER_MOVEHIT             0x020
#define TRIGGER_MOVEGUARDED         0x021
#define TRIGGER_MOVECONTACT         0x022
#define TRIGGER_MOVEREVERSED        0x023
#define TRIGGER_INGUARDDIST         0x024
#define TRIGGER_STATENO             0x028
#define TRIGGER_PREVSTATENO         0x029
#define TRIGGER_P2STATENO           0x02A
#define TRIGGER_STATETYPE           0x02B   /* statetype = S/C/A/L */
#define TRIGGER_P2STATETYPE         0x02C
#define TRIGGER_MOVETYPE            0x02D   /* movetype = I/A/H */
#define TRIGGER_P2MOVETYPE          0x02E
#define TRIGGER_CTRL                0x02F
#define TRIGGER_HITDEFATTR          0x030   /* hitdefattr = SCA, NA... */
#define TRIGGER_RANDOM              0x033
#define TRIGGER_NUMPROJ             0x03C
#define TRIGGER_PROJHIT             0x03E   /* projhit<ID> = 0/1[, op time] */
#define TRIGGER_PROJGUARDED         0x03F
#define TRIGGER_PROJCONTACT         0x040
#define TRIGGER_P1NAME              0x05A   /* also name */
#define TRIGGER_P2NAME              0x05B
#define TRIGGER_P3NAME              0x05C
#define TRIGGER_P4NAME              0x05D
#define TRIGGER_AUTHORNAME          0x05E
#define TRIGGER_NUMPARTNER          0x05F
#define TRIGGER_NUMENEMY            0x060
#define TRIGGER_ID                  0x061
#define TRIGGER_FACING              0x063
#define TRIGGER_HITFALL             0x064
#define TRIGGER_HITSHAKEOVER        0x065
#define TRIGGER_HITOVER             0x066
#define TRIGGER_HITVEL              0x067
#define TRIGGER_HITPAUSETIME        0x068
#define TRIGGER_CANRECOVER          0x069
#define TRIGGER_PALNO               0x06B
#define TRIGGER_WINPERFECT          0x06C
#define TRIGGER_WIN                 0x06D   /* win, winko, wintime: type in arg */
#define TRIGGER_LOSE                0x06E
#define TRIGGER_DRAWGAME            0x06F
#define TRIGGER_MATCHOVER           0x070
#define TRIGGER_GAMETIME            0x071
#define TRIGGER_ROUNDNO             0x072
#define TRIGGER_ROUNDSTATE          0x073
#define TRIGGER_MATCHNO             0x074
#define TRIGGER_ROUNDSEXISTED       0x075
#define TRIGGER_TEAMSIDE            0x078
#define TRIGGER_TEAMMODE            0x079   /* teammode = single/simul/turns */
#define TRIGGER_ISHOMETEAM          0x07A
#define TRIGGER_NUMEXPLOD           0x096
#define TRIGGER_NUMTARGET           0x0A0
#define TRIGGER_GETHITVAR           0x0B4   /* gethitvar(name): index in value */
#define TRIGGER_CONST               0x0B5   /* const(name): index into g_triggerConsts in value */
#define TRIGGER_STAGEVAR            0x0B6   /* stagevar(name): TRIGGER_STAGEVAR_* in arg */
#define TRIGGER_TICKSPERSECOND      0x0BE
#define TRIGGER_ISHELPER            0x0C9
#define TRIGGER_NUMHELPER           0x0CA
#define TRIGGER_PARENTDIST          0x0CB
#define TRIGGER_ROOTDIST            0x0CC
#define TRIGGER_STANDBY             0x0CD
#define TRIGGER_GAMEWIDTH           0x0D2
#define TRIGGER_GAMEHEIGHT          0x0D3
#define TRIGGER_SCREENWIDTH         0x0D4
#define TRIGGER_SCREENHEIGHT        0x0D5
#define TRIGGER_BACKEDGE            0x0D6
#define TRIGGER_FRONTEDGE           0x0D7
#define TRIGGER_LEFTEDGE            0x0D8
#define TRIGGER_RIGHTEDGE           0x0D9
#define TRIGGER_TOPEDGE             0x0DA
#define TRIGGER_BOTTOMEDGE          0x0DB
#define TRIGGER_PI                  0x12C
#define TRIGGER_E                   0x12D
#define TRIGGER_IFELSE              0x131   /* 3 arguments */
#define TRIGGER_LOG                 0x136   /* 2 arguments */
#define TRIGGER_SIN                 0x140   /* 0x140-0x15F: 1 argument */
#define TRIGGER_COS                 0x141
#define TRIGGER_TAN                 0x142
#define TRIGGER_ASIN                0x143
#define TRIGGER_ACOS                0x144
#define TRIGGER_ATAN                0x145
#define TRIGGER_EXP                 0x146
#define TRIGGER_LN                  0x147
#define TRIGGER_ABS                 0x148
#define TRIGGER_FLOOR               0x149
#define TRIGGER_CEIL                0x14A
#define TRIGGER_ANIMELEMTIME        0x14B
#define TRIGGER_ANIMEXIST           0x14C
#define TRIGGER_SELFANIMEXIST       0x14D
#define TRIGGER_PLAYERIDEXIST       0x14E
#define TRIGGER_ISHELPER_ID         0x14F   /* ishelper(ID) */
#define TRIGGER_NUMHELPER_ID        0x150   /* numhelper(ID) */
#define TRIGGER_PROJHITTIME         0x151
#define TRIGGER_PROJGUARDEDTIME     0x152
#define TRIGGER_PROJCONTACTTIME     0x153
#define TRIGGER_NUMPROJID           0x154
#define TRIGGER_NUMEXPLOD_ID        0x155   /* numexplod(ID) */
#define TRIGGER_NUMTARGET_ID        0x156   /* numtarget(ID) */
#define TRIGGER_SYSVAR              0x159
#define TRIGGER_SYSFVAR             0x15A
#define TRIGGER_PROJCANCELTIME      0x15B
#define TRIGGER_ANIMELEMNO          0x15C
#define TRIGGER_CONST240P           0x15D
#define TRIGGER_CONST480P           0x15E
#define TRIGGER_CONST720P           0x15F
#define TRIGGER_COND                0x190   /* 3 Exprs in data */

/* Trigger.redirect */
#define TRIGGER_REDIRECT_NONE       0
#define TRIGGER_REDIRECT_PARENT     1
#define TRIGGER_REDIRECT_ROOT       2
#define TRIGGER_REDIRECT_HELPER     3   /* helper(ID), helper = helper(0) */
#define TRIGGER_REDIRECT_TARGET     4   /* target(ID), target = target(-1) */
#define TRIGGER_REDIRECT_PARTNER    5
#define TRIGGER_REDIRECT_ENEMY      6
#define TRIGGER_REDIRECT_ENEMYNEAR  7
#define TRIGGER_REDIRECT_PLAYERID   8   /* the ID is required */

/* Trigger.op for the triggers that parse their own comparison (Trigger_ParseCompare). */
#define TRIGGER_OP_EQ               0
#define TRIGGER_OP_NE               1
#define TRIGGER_OP_GT               2
#define TRIGGER_OP_GE               3
#define TRIGGER_OP_LT               4
#define TRIGGER_OP_LE               5
#define TRIGGER_OP_IN_CC            10  /* [value, value2] */
#define TRIGGER_OP_IN_CO            11  /* [value, value2) */
#define TRIGGER_OP_IN_OC            12  /* (value, value2] */
#define TRIGGER_OP_IN_OO            13  /* (value, value2) */

/* Trigger.arg of stagevar(name). */
#define TRIGGER_STAGEVAR_NAME           0   /* info.name */
#define TRIGGER_STAGEVAR_DISPLAYNAME    1   /* info.displayname */
#define TRIGGER_STAGEVAR_AUTHOR         4   /* info.author */

/* Trigger.arg of win/lose: kind of win (GameVars.win_type). */
#define TRIGGER_WIN_ANY             (-1)
#define TRIGGER_WIN_KO              1
#define TRIGGER_WIN_TIME            3

/* const(name): the CharConsts field with the same index (88 of them). */
#define TRIGGER_CONST_COUNT         88

typedef struct TriggerConstInfo {
    const char *name;           /* 0x00 e.g. "velocity.walk.fwd.x" */
    int is_float;               /* 0x04 the CharConsts field is a float */
    int scaled;                 /* 0x08 in character units: scaled to the game resolution */
} TriggerConstInfo;
MUGEN_ASSERT_SIZE(TriggerConstInfo, 0x0C);

#endif /* MUGEN_TRIGGER_H */
