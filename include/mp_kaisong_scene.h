#ifndef GUARD_MP_KAISONG_SCENE_H
#define GUARD_MP_KAISONG_SCENE_H

// Moemon Kai: which scene asks for a song (music/<song>@<scene>.kaisong, src/mp/mp_kaisong.c).
// Every call that starts a song (sound.h, m4a.h, pokemon.h) is wrapped so the song remembers the source file that asked
// for it: a file of a scene (tools/kaisong/gen.py lists them; src/tcg, src/ygo, src/tt and src/rhythm are a scene each,
// other files one each) or of the world (field, scripts, battles: the map's places). Calls inside a wrapped call keep
// the outer caller. Files that define the wrapped functions say MP_KAISONG_NO_SCENE before their includes.
#ifdef PORTABLE
void MP_Kaisong_Request(const char *file);
void MP_Kaisong_RequestEnd(void);
void MP_Kaisong_Hold(const char *file);
void MP_Kaisong_Release(void);
#define MP_KAISONG_WRAP(call) (MP_Kaisong_Request(__FILE__), (call), MP_Kaisong_RequestEnd())
// A scene whose songs come from the game's shared code (the BATTLE SIMULATOR's battles): every song asked for until
// MP_MUSIC_SCENE_RELEASE() is this file's scene. Name the songs it inherits: tools/kaisong/gen.py lists them for it.
#define MP_MUSIC_SCENE_HOLD(...) MP_Kaisong_Hold(__FILE__)
#define MP_MUSIC_SCENE_RELEASE() MP_Kaisong_Release()
// A scene handing the music back to the map (the song it saved when it began): the map's places apply again
#define MP_MUSIC_TO_MAP(call) (MP_Kaisong_Request(NULL), (call), MP_Kaisong_RequestEnd())
#else
#define MP_MUSIC_TO_MAP(call) (call)
#define MP_KAISONG_WRAP(call) (call)
#define MP_MUSIC_SCENE_HOLD(...)
#define MP_MUSIC_SCENE_RELEASE()
#endif

#endif
