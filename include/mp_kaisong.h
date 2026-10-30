#ifndef GUARD_MP_KAISONG_H
#define GUARD_MP_KAISONG_H

// Moemon Kai: custom songs (.kaisong files from Moemon Music Studio, docs/KAISONG_FORMAT.md) in the game's music folder.
// music/<song>.kaisong replaces a song everywhere, music/<song>@<place>.kaisong only on that map (place = the map's
// MAP_ id without MAP_, lowercase: mus_route101@route103.kaisong). docs/SONG_TABLE.md lists every song and place.

struct SongHeader;
struct WarpData;

// The header to play for song n: a custom one for the current map (or everywhere) when there is one, else builtin.
// Always the same pointer for the same file (m4aSongNumStartOrChange compares headers).
struct SongHeader *MP_Kaisong_Header(u16 song, struct SongHeader *builtin);
// The song number a playing header belongs to (custom or built in); 0 = none.
u16 MP_Kaisong_SongOf(const struct SongHeader *header);
// Spectating: the place variant of the playing header (0 = the song's own, 1-63 a place) and playing song n in it.
u8 MP_Kaisong_VariantOf(const struct SongHeader *header);
void MP_Kaisong_StartVariant(u16 song, u8 variant);
// Song n is playing, but the map (warp NULL = the current one) would play another version of it.
bool8 MP_Kaisong_Differs(u16 song, const struct WarpData *warp);
// MPlayStart on the BGM player: the song's own voice count (INFO voices=), 5 for the game's songs.
void MP_Kaisong_Started(const struct SongHeader *header);
// Debug / tests: list what music/ has ("mp kaisong").
void MP_Kaisong_Log(void);
bool8 MP_Kaisong_SameMusic(u16 song, const struct WarpData *warp);   // the music playing already is that song's here
u16 MP_Kaisong_Gain(const struct SongHeader *header);   // 256 = as made (INFO gain=)
bool8 MP_Kaisong_PlayFile(const char *file);   // tests: one file's song (mp playkai)

#endif
