// Moemon Rhythm: the song list (rh_songs.c)
#ifndef GUARD_RH_SONGS_H
#define GUARD_RH_SONGS_H

#include "rhythm/chart.h"

#define RH_GROUPS 64

struct RhEntry
{
    struct RhSong song;
    char music[600], banner[600], background[600], card[600];
    int group;
};

const char *RhSongs_Root(void);
int RhSongs_Scan(void);
void RhSongs_Free(void);
int RhSongs_Count(void);
struct RhEntry *RhSongs_Get(int i);
uint32_t RhSongs_ChartKey(const struct RhEntry *e, int diff);

#endif
