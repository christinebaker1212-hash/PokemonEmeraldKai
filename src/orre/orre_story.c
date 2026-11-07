// Moemon Kai: Colosseum's story flags in ORRE (the numbers its scripts and tables test: std081_flagCheckTrue). Until
// ORRE's scripts are converted nothing sets them but `mp orreflag`; they last while the game runs (a sidecar later:
// saves never change layout, docs/ORRE_DECISIONS.md)
#include "global.h"
#include "orre.h"

#define ORRE_FLAGS 0x1000

static u8 sFlags[ORRE_FLAGS / 8];

bool8 Orre_Flag(u16 flag)
{
    return flag < ORRE_FLAGS && (sFlags[flag >> 3] >> (flag & 7) & 1);
}

void Orre_SetFlag(u16 flag, bool8 on)
{
    if (flag >= ORRE_FLAGS)
        return;
    if (on)
        sFlags[flag >> 3] |= 1 << (flag & 7);
    else
        sFlags[flag >> 3] &= ~(1 << (flag & 7));
}
