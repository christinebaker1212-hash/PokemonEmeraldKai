// Moemon Maker: music for the editor (include/maker_api.h; docs/MOEMON_MAKER.md decision 5): the songs a map can play
// (the game's and the user's new tracks), playing one through the game's own player, and the user's files in
// MoemonMaker/music/ (src/mp/mp_kaisong.c reads them: replacements win over music/'s, other names are new tracks).
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "m4a.h"
#include "sound.h"
#include "mp_kaisong.h"
#include "maker.h"
#include "maker_api.h"

int MP_Kaisong_SceneCount(void);
const char *MP_Kaisong_SceneKey(int i);
const char *MP_Kaisong_SongFileName(u16 song);

int MakerApi_TrackCount(void)
{
    return MP_Kaisong_TrackCount();
}

const char *MakerApi_TrackName(int i)
{
    return MP_Kaisong_TrackName(i);
}

// A song by name: the game's (MUS_ROUTE101) or a new track's (mus_my_theme); -1 if none
int MakerApi_SongId(const char *name)
{
    unsigned v;
    if (MakerApi_ValueOf(MAKER_NAMES_SONG, name, &v))
        return v;
    return MP_Kaisong_TrackId(name);
}

// A song's name (the game's MUS_ name, or a new track's own)
const char *MakerApi_SongName(int song)
{
    const char *n;
    if (song >= MAKER_SONG_FIRST)
    {
        n = MP_Kaisong_TrackName(song - MAKER_SONG_FIRST);
        return n ? n : "?";
    }
    n = MakerApi_NameOf(MAKER_NAMES_SONG, (unsigned)song);
    return n && n[0] ? n : "?";
}

const char *MakerApi_SongFileName(int song)
{
    return MP_Kaisong_SongFileName(song);
}

int MakerApi_SceneCount(void)
{
    return MP_Kaisong_SceneCount();
}

const char *MakerApi_SceneKey(int i)
{
    return MP_Kaisong_SceneKey(i);
}

void MakerApi_PlaySong(int song)
{
    m4aMPlayAllStop();
    m4aSongNumStart(song);
}

void MakerApi_StopSong(void)
{
    m4aMPlayAllStop();
}

// The files changed (the editor copied one in, or removed one): read the folders again
void MakerApi_MusicChanged(void)
{
    MP_Kaisong_Rescan();
}
