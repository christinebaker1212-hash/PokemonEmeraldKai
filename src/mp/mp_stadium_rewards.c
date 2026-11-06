// POKéMON STADIUM's rewards (docs/STADIUM.md "Rewards"): given when a cup or class is won, bigger with every one of
// the user's restrictions in place (mp_stadium_cup.c Hardness), the greatest with all four; a STADIUM RIBBON for the
// POKéMON that won it; the achievements.
#include <stdio.h>
#include "global.h"
#include "mp.h"
#include "mp_stadium.h"
#include "mp_stadium_cup.h"

void StadiumRewards_CupWon(u8 round, u8 cup, u8 tier, u8 hardness, bool8 first)
{
    MPNet_Log("stadium: %s class %d round %d won (hardness %d%s)", StadiumCup_Info(cup)->name, tier, round + 1,
              hardness, first ? ", first time" : "");
}
