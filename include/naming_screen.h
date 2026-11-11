#ifndef GUARD_NAMING_SCREEN_H
#define GUARD_NAMING_SCREEN_H

#include "main.h"

enum {
    NAMING_SCREEN_PLAYER,
    NAMING_SCREEN_BOX,
    NAMING_SCREEN_CAUGHT_MON,
    NAMING_SCREEN_NICKNAME,
    NAMING_SCREEN_WALDA,
    NAMING_SCREEN_COOP_ROOM,      // Moemon Co-op: server name
    NAMING_SCREEN_COOP_PASSWORD,  // Moemon Co-op: server password (may be left empty)
    NAMING_SCREEN_COOP_CODE,      // Moemon Co-op: invite code from a friend
    NAMING_SCREEN_COOP_NOTE1,     // Moemon Co-op: the host's welcome note, line 1 (may be left empty)
    NAMING_SCREEN_COOP_NOTE2,     // Moemon Co-op: the host's welcome note, line 2
    NAMING_SCREEN_RIVAL,          // Moemon Kai: JOHTO's rival, as Crystal names him (qz_specials.c MP_JohtoRivalName)
    NAMING_SCREEN_ORRE_PARTNER,   // Moemon Kai: ORRE's partner, as Colosseum names her (src/orre/orre_vm.c Orre_VmName)
};

void NamingScreen_SetOrreTitle(const u8 *title);   // Moemon Kai: NAMING_SCREEN_ORRE_PARTNER's title (Colosseum's own line)
void DoNamingScreen(u8 templateNum, u8 *destBuffer, u16 monSpecies, u16 monGender, u32 monPersonality, MainCallback returnCallback);

#endif // GUARD_NAMING_SCREEN_H
