MORT: POKeMON STADIUM's speech codec (the announcer's 823 clips).

MORTDecoder.cpp / .h are SubDrag's decoder, reverse engineered from Pokemon Stadium, from jombo23/N64-Tools
(N64SoundListToolUpdated/N64SoundLibrary), released into the public domain (the Unlicense). Changes here:
- the encoder (ReadWavData, Encode, CalculateBestPredictors) is left out: it needed MFC's CString;
- its `unsigned long` / `signed long` are uint32_t / int32_t (they were 32 bits where it was written; 64 on Linux);
- "StdAfx.h" is stdafx_shim.h (MessageBox to stderr, __int64).

mort_api.cpp is the game's way in (Mort_Decode). The game decodes a clip when the announcer is about to say it
(src/mp/mp_stadium_announcer.c), from the speech archive in stadium/announcer.s1, which comes from the player's own
ROM (tools/stadium/mort/make_voice_pack.py); the clips are never committed.

Where the clips are and what each says: docs/STADIUM.md "The announcer". The clip map and many of the lines' words
come from StadiumBattleFX's research (MIT): research/ANNOUNCER.md and research/announcer_catalog.json.
