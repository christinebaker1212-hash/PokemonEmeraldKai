# Moemon Quetzal songs (ids 559-665)

Ripped from Moemon Quetzal (English Alpha 9) by `tools/quetzal_rip/songs.py`, which rebuilds everything here from the ROM. Song `mus_qz_<id>` is Quetzal song id `<id>` (the rip's `MUS_QZ_<id>`).

## Files

- `songs/mus_qz_<id>.s`: the song as it is in the ROM, in mid2agb's assembly format (Kai flavour: `ptrvalue` pointers, `space64` in the header). Assembled for 32 bits and linked at the ROM address it gives the ROM's bytes back exactly. Kai's Makefile only builds `sound/songs/midi/*.mid`, so a `.s` needs its own rule or a copy into a data `.s` file.
- `midi/mus_qz_<id>.mid` + `midi/midi.cfg`: the same songs for Kai's normal build path (drop the .mid files into `sound/songs/midi/` and append the cfg lines). 24 ticks per quarter note (mid2agb's clock), one MIDI track per song track, loop as `[` / `]` markers, VOL / PAN / BEND / BENDR / LFOS / MODT / TUNE / LFODL / MOD / PRIO / xIECV / xIECL as the controllers mid2agb reads. `-V127` keeps the ROM's VOL values.
- `voicegroups/`, `voice_groups.inc`: voicegroups Kai does not have (only the instruments these songs play are filled in; the other slots are the usual `voice_square_1` filler). Sub-groups (drumsets, keysplit groups) come first in `voice_groups.inc`.
- `direct_sound_samples/*.wav` + `direct_sound_data.inc`: new DirectSound samples. They are 8-bit wavs with `smpl` (loop), `agbp` (exact pitch) and `agbl` (exact length) chunks, so Kai's `wav2agb -b` turns them back into the ROM's bytes. Samples that are byte-identical to one of Kai's are referenced by Kai's name instead.
- `programmable_wave_samples/`, `programmable_wave_data.inc`, `keysplit_tables.inc`: new wave tables / key splits (if any).
- `song_table.inc`: `song` rows with the ROM's music player and priority.

## Voicegroups

- `voicegroup_bard` (ROM 0x0937F09C): Kai already has it (same instruments); songs 559-609
- `voicegroup_qz_610` (ROM 0x09393004): new, dumped here; songs 610
- `voicegroup_qz_songs` (ROM 0x09394018): new, dumped here; songs 611-665
- `voicegroup_qz_610_drumset_1` (ROM 0x093933AC): new drumset
- `voicegroup_qz_610_tubular_bells` (ROM 0x09393C4C): new keysplit group
- `voicegroup_qz_610_percussive_organ` (ROM 0x09393CAC): new keysplit group
- `voicegroup_qz_610_clean_guitar` (ROM 0x09393D54): new keysplit group
- `voicegroup_qz_610_acoustic_bass` (ROM 0x09393DD8): new keysplit group
- `voicegroup_qz_610_string_ensemble_1` (ROM 0x09393E8C): new keysplit group
- `voicegroup_qz_610_trumpet` (ROM 0x09393ED4): new keysplit group
- `voicegroup_qz_610_trombone` (ROM 0x09393EF8): new keysplit group
- `voicegroup_qz_610_french_horn` (ROM 0x09393F58): new keysplit group
- `voicegroup_qz_songs_acoustic_grand_piano` (ROM 0x09394C18): new keysplit group
- `voicegroup_qz_songs_bright_piano` (ROM 0x09394C54): new keysplit group
- `voicegroup_qz_songs_electric_grand_piano` (ROM 0x09394C6C): new keysplit group
- `voicegroup_qz_songs_honky_tonk_piano` (ROM 0x09394C9C): new keysplit group
- `voicegroup_qz_songs_electric_piano_1` (ROM 0x09394CB4): new keysplit group
- `voicegroup_qz_songs_electric_piano_2` (ROM 0x09394CCC): new keysplit group
- `voicegroup_qz_songs_harpsichord` (ROM 0x09394CE4): new keysplit group
- `voicegroup_qz_songs_clavinet` (ROM 0x09394D08): new keysplit group
- `voicegroup_qz_songs_celesta` (ROM 0x09394D2C): new keysplit group
- `voicegroup_qz_songs_glockenspiel` (ROM 0x09394D44): new keysplit group
- `voicegroup_qz_songs_music_box` (ROM 0x09394D5C): new keysplit group
- `voicegroup_qz_songs_vibraphone` (ROM 0x09394D80): new keysplit group
- `voicegroup_qz_songs_marimba` (ROM 0x09394DA4): new keysplit group
- `voicegroup_qz_songs_xylophone` (ROM 0x09394DC8): new keysplit group
- `voicegroup_qz_songs_tubular_bells` (ROM 0x09394DEC): new keysplit group
- `voicegroup_qz_songs_dulcimer` (ROM 0x09394E1C): new keysplit group
- `voicegroup_qz_songs_drawbar_organ` (ROM 0x09394E4C): new keysplit group
- `voicegroup_qz_songs_percussive_organ` (ROM 0x09394E7C): new keysplit group
- `voicegroup_qz_songs_rock_organ` (ROM 0x09394EAC): new keysplit group
- `voicegroup_qz_songs_church_organ` (ROM 0x09394EDC): new keysplit group
- `voicegroup_qz_songs_reed_organ` (ROM 0x09394EF4): new keysplit group
- `voicegroup_qz_songs_harmonica` (ROM 0x09394F0C): new keysplit group
- `voicegroup_qz_songs_tango_accordion` (ROM 0x09394F24): new keysplit group
- `voicegroup_qz_songs_nylon_guitar` (ROM 0x09394F54): new keysplit group
- `voicegroup_qz_songs_steel_guitar` (ROM 0x09394F78): new keysplit group
- `voicegroup_qz_songs_jazz_guitar` (ROM 0x09394F9C): new keysplit group
- `voicegroup_qz_songs_clean_guitar` (ROM 0x09394FD8): new keysplit group
- `voicegroup_qz_songs_muted_guitar` (ROM 0x09394FFC): new keysplit group
- `voicegroup_qz_songs_overdriven_guitar` (ROM 0x09395020): new keysplit group
- `voicegroup_qz_songs_distortion_guitar` (ROM 0x09395044): new keysplit group
- `voicegroup_qz_songs_guitar_harmonics` (ROM 0x09395068): new keysplit group
- `voicegroup_qz_songs_acoustic_bass` (ROM 0x0939508C): new keysplit group
- `voicegroup_qz_songs_fingered_bass` (ROM 0x093950B0): new keysplit group
- `voicegroup_qz_songs_picked_bass` (ROM 0x093950D4): new keysplit group
- `voicegroup_qz_songs_fretless_bass` (ROM 0x093950F8): new keysplit group
- `voicegroup_qz_songs_slap_bass_1` (ROM 0x09395128): new keysplit group
- `voicegroup_qz_songs_slap_bass_2` (ROM 0x0939514C): new keysplit group
- `voicegroup_qz_songs_drumset_39` (ROM 0x093942AC): new drumset
- `voicegroup_qz_songs_drumset_40` (ROM 0x093945F4): new drumset
- `voicegroup_qz_songs_cello` (ROM 0x093951A0): new keysplit group
- `voicegroup_qz_songs_synth_brass_2` (ROM 0x09393E74): new keysplit group
- `voicegroup_qz_songs_baritone_sax` (ROM 0x093951C4): new keysplit group
- `voicegroup_qz_songs_square_lead` (ROM 0x093951E8): new keysplit group

192 new DirectSound samples, 0 of Kai's samples reused.

## Songs

Length = to the loop end (or the end) at the song's tempo; "loop" = where it loops back to. Names are guesses from the maps / scripts of this rip that play the song.

| id | name guess | player | tracks | voicegroup | length | loop from | played by |
|---|---|---|---|---|---|---|---|
| 559 | stock `PH_TRAP_BLEND` (Kai's `ph_trap_blend`) | SE2 | 1 | bard | 0:01.0 (47 ticks) | - | - |
| 560 | stock `PH_TRAP_HELD` (Kai's `ph_trap_held`) | SE2 | 1 | bard | 0:04.0 (192 ticks) | 96 | - |
| 561 | stock `PH_TRAP_SOLO` (Kai's `ph_trap_solo`) | SE2 | 1 | bard | 0:01.1 (54 ticks) | - | - |
| 562 | stock `PH_FACE_BLEND` (Kai's `ph_face_blend`) | SE2 | 1 | bard | 0:01.1 (54 ticks) | - | - |
| 563 | stock `PH_FACE_HELD` (Kai's `ph_face_held`) | SE2 | 1 | bard | 0:04.0 (192 ticks) | 96 | - |
| 564 | stock `PH_FACE_SOLO` (Kai's `ph_face_solo`) | SE2 | 1 | bard | 0:01.1 (54 ticks) | - | - |
| 565 | stock `PH_CLOTH_BLEND` (Kai's `ph_cloth_blend`) | SE2 | 1 | bard | 0:01.1 (54 ticks) | - | - |
| 566 | stock `PH_CLOTH_HELD` (Kai's `ph_cloth_held`) | SE2 | 1 | bard | 0:04.0 (192 ticks) | 96 | - |
| 567 | stock `PH_CLOTH_SOLO` (Kai's `ph_cloth_solo`) | SE2 | 1 | bard | 0:01.1 (54 ticks) | - | - |
| 568 | stock `PH_DRESS_BLEND` (Kai's `ph_dress_blend`) | SE2 | 1 | bard | 0:01.1 (54 ticks) | - | - |
| 569 | stock `PH_DRESS_HELD` (Kai's `ph_dress_held`) | SE2 | 1 | bard | 0:04.0 (192 ticks) | 96 | - |
| 570 | stock `PH_DRESS_SOLO` (Kai's `ph_dress_solo`) | SE2 | 1 | bard | 0:01.1 (54 ticks) | - | - |
| 571 | stock `PH_FLEECE_BLEND` (Kai's `ph_fleece_blend`) | SE2 | 1 | bard | 0:01.1 (54 ticks) | - | - |
| 572 | stock `PH_FLEECE_HELD` (Kai's `ph_fleece_held`) | SE2 | 1 | bard | 0:04.0 (192 ticks) | 96 | - |
| 573 | stock `PH_FLEECE_SOLO` (Kai's `ph_fleece_solo`) | SE2 | 1 | bard | 0:01.1 (54 ticks) | - | - |
| 574 | stock `PH_KIT_BLEND` (Kai's `ph_kit_blend`) | SE2 | 1 | bard | 0:01.1 (54 ticks) | - | - |
| 575 | stock `PH_KIT_HELD` (Kai's `ph_kit_held`) | SE2 | 1 | bard | 0:04.0 (192 ticks) | 96 | - |
| 576 | stock `PH_KIT_SOLO` (Kai's `ph_kit_solo`) | SE2 | 1 | bard | 0:01.1 (54 ticks) | - | - |
| 577 | stock `PH_PRICE_BLEND` (Kai's `ph_price_blend`) | SE2 | 1 | bard | 0:01.1 (54 ticks) | - | - |
| 578 | stock `PH_PRICE_HELD` (Kai's `ph_price_held`) | SE2 | 1 | bard | 0:04.0 (192 ticks) | 96 | - |
| 579 | stock `PH_PRICE_SOLO` (Kai's `ph_price_solo`) | SE2 | 1 | bard | 0:01.1 (54 ticks) | - | - |
| 580 | stock `PH_LOT_BLEND` (Kai's `ph_lot_blend`) | SE2 | 1 | bard | 0:01.1 (54 ticks) | - | - |
| 581 | stock `PH_LOT_HELD` (Kai's `ph_lot_held`) | SE2 | 1 | bard | 0:04.0 (192 ticks) | 96 | - |
| 582 | stock `PH_LOT_SOLO` (Kai's `ph_lot_solo`) | SE2 | 1 | bard | 0:01.1 (54 ticks) | - | - |
| 583 | stock `PH_GOAT_BLEND` (Kai's `ph_goat_blend`) | SE2 | 1 | bard | 0:01.1 (54 ticks) | - | - |
| 584 | stock `PH_GOAT_HELD` (Kai's `ph_goat_held`) | SE2 | 1 | bard | 0:04.0 (192 ticks) | 96 | - |
| 585 | stock `PH_GOAT_SOLO` (Kai's `ph_goat_solo`) | SE2 | 1 | bard | 0:01.1 (54 ticks) | - | - |
| 586 | stock `PH_THOUGHT_BLEND` (Kai's `ph_thought_blend`) | SE2 | 1 | bard | 0:01.1 (54 ticks) | - | - |
| 587 | stock `PH_THOUGHT_HELD` (Kai's `ph_thought_held`) | SE2 | 1 | bard | 0:04.0 (192 ticks) | 96 | - |
| 588 | stock `PH_THOUGHT_SOLO` (Kai's `ph_thought_solo`) | SE2 | 1 | bard | 0:01.1 (54 ticks) | - | - |
| 589 | stock `PH_CHOICE_BLEND` (Kai's `ph_choice_blend`) | SE2 | 1 | bard | 0:01.1 (54 ticks) | - | - |
| 590 | stock `PH_CHOICE_HELD` (Kai's `ph_choice_held`) | SE2 | 1 | bard | 0:04.0 (192 ticks) | 96 | - |
| 591 | stock `PH_CHOICE_SOLO` (Kai's `ph_choice_solo`) | SE2 | 1 | bard | 0:01.1 (54 ticks) | - | - |
| 592 | stock `PH_MOUTH_BLEND` (Kai's `ph_mouth_blend`) | SE2 | 1 | bard | 0:01.1 (54 ticks) | - | - |
| 593 | stock `PH_MOUTH_HELD` (Kai's `ph_mouth_held`) | SE2 | 1 | bard | 0:04.0 (192 ticks) | 96 | - |
| 594 | stock `PH_MOUTH_SOLO` (Kai's `ph_mouth_solo`) | SE2 | 1 | bard | 0:01.1 (54 ticks) | - | - |
| 595 | stock `PH_FOOT_BLEND` (Kai's `ph_foot_blend`) | SE2 | 1 | bard | 0:01.1 (54 ticks) | - | - |
| 596 | stock `PH_FOOT_HELD` (Kai's `ph_foot_held`) | SE2 | 1 | bard | 0:04.0 (192 ticks) | 96 | - |
| 597 | stock `PH_FOOT_SOLO` (Kai's `ph_foot_solo`) | SE2 | 1 | bard | 0:01.1 (54 ticks) | - | - |
| 598 | stock `PH_GOOSE_BLEND` (Kai's `ph_goose_blend`) | SE2 | 1 | bard | 0:01.1 (54 ticks) | - | - |
| 599 | stock `PH_GOOSE_HELD` (Kai's `ph_goose_held`) | SE2 | 1 | bard | 0:04.0 (192 ticks) | 96 | - |
| 600 | stock `PH_GOOSE_SOLO` (Kai's `ph_goose_solo`) | SE2 | 1 | bard | 0:01.1 (54 ticks) | - | - |
| 601 | stock `PH_STRUT_BLEND` (Kai's `ph_strut_blend`) | SE2 | 1 | bard | 0:01.1 (54 ticks) | - | - |
| 602 | stock `PH_STRUT_HELD` (Kai's `ph_strut_held`) | SE2 | 1 | bard | 0:04.0 (192 ticks) | 96 | - |
| 603 | stock `PH_STRUT_SOLO` (Kai's `ph_strut_solo`) | SE2 | 1 | bard | 0:01.1 (54 ticks) | - | - |
| 604 | stock `PH_CURE_BLEND` (Kai's `ph_cure_blend`) | SE2 | 1 | bard | 0:01.1 (54 ticks) | - | - |
| 605 | stock `PH_CURE_HELD` (Kai's `ph_cure_held`) | SE2 | 1 | bard | 0:04.0 (192 ticks) | 96 | - |
| 606 | stock `PH_CURE_SOLO` (Kai's `ph_cure_solo`) | SE2 | 1 | bard | 0:01.1 (54 ticks) | - | - |
| 607 | stock `PH_NURSE_BLEND` (Kai's `ph_nurse_blend`) | SE2 | 1 | bard | 0:01.1 (54 ticks) | - | - |
| 608 | stock `PH_NURSE_HELD` (Kai's `ph_nurse_held`) | SE2 | 1 | bard | 0:04.0 (192 ticks) | 96 | - |
| 609 | stock `PH_NURSE_SOLO` (Kai's `ph_nurse_solo`) | SE2 | 1 | bard | 0:01.1 (54 ticks) | - | - |
| 610 | Trainer Hill (Johto; own voicegroup) | BGM | 8 | qz_610 | 1:01.6 (3744 ticks) | 192 | Johto_TrainerHillOutside |
| 611 | New Bark Town | BGM | 14 | qz_songs | 1:00.0 (3072 ticks) | 384 | Johto_ElmsHouse, Johto_NewBarkTown, Johto_PlayersHouse1F, Johto_PlayersHouse2F, Johto_PlayersNeighborsHouse, Johto_IndigoPlateauPokecenter1F (playbgm) |
| 612 | Cherrygrove City (also Olivine / Mahogany houses) | BGM | 9 | qz_songs | 0:51.0 (2409 ticks) | 201 | Johto_CherrygroveCity, Johto_CherrygroveEvolutionSpeechHouse, Johto_CherrygroveGymSpeechHouse, Johto_GuideGentsHouse, Johto_MahoganyRedGyaradosSpeechHouse, Johto_MahoganyTown, +6 more maps |
| 613 | Violet City (also Olivine City) | BGM | 14 | qz_songs | 1:04.7 (2784 ticks) | 96 | Johto_EarlsPokemonAcademy, Johto_OlivineCity, Johto_VioletCity, Johto_VioletKylesHouse, Johto_VioletNicknameSpeechHouse |
| 614 | Azalea Town (also Blackthorn City) | BGM | 12 | qz_songs | 1:08.2 (3276 ticks) | 972 | Johto_AzaleaTown, Johto_BlackthornCity, Johto_BlackthornDragonSpeechHouse, Johto_BlackthornEmysHouse, Johto_CharcoalKiln, Johto_IlexForestAzaleaGate, +3 more maps |
| 615 | Goldenrod City | BGM | 11 | qz_songs | 0:42.4 (2304 ticks) | 384 | Johto_BillsFamilysHouse, Johto_GoldenrodCity, Johto_GoldenrodCity_Copy, Johto_GoldenrodDeptStoreElevator, Johto_GoldenrodDeptStoreRoof, Johto_GoldenrodDeptStoreRoofAlt, +12 more maps, Johto_RadioTower5F (playbgm) |
| 616 | Ecruteak City | BGM | 9 | qz_songs | 1:20.0 (2688 ticks) | 0 | Johto_EcruteakCity, Johto_EcruteakItemfinderHouse, Johto_EcruteakLugiaSpeechHouse, Johto_EcruteakTinTowerEntrance, Johto_WiseTriosRoom |
| 617 | Cianwood City | BGM | 7 | qz_songs | 1:13.6 (2592 ticks) | 0 | Johto_CianwoodCity, Johto_CianwoodLugiaSpeechHouse, Johto_CianwoodOldWomansHouse, Johto_CianwoodPharmacy, Johto_ManiasHouse |
| 618 | Route 29 | BGM | 9 | qz_songs | 0:58.6 (2268 ticks) | 300 | Johto_Route29, Johto_Route29Route46Gate |
| 619 | Route 30 (Routes 30-33) | BGM | 14 | qz_songs | 0:37.7 (1932 ticks) | 396 | Johto_MrPokemonsHouse, Johto_Route30, Johto_Route30BerryHouse, Johto_Route31, Johto_Route31VioletGate, Johto_Route32, +2 more maps |
| 620 | Route 34 (Routes 34-37) | BGM | 9 | qz_songs | 0:54.5 (2880 ticks) | 960 | Johto_DayCare, Johto_Route34, Johto_Route34IlexForestGate, Johto_Route34IlexForestGate_Copy, Johto_Route34_Copy, Johto_Route35, +5 more maps |
| 621 | Route 38 (Routes 38-41) | BGM | 9 | qz_songs | 0:53.5 (2784 ticks) | 288 | Johto_Route38, Johto_Route38EcruteakGate, Johto_Route39, Johto_Route39Barn, Johto_Route39Farmhouse, Johto_Route40, +2 more maps |
| 622 | Route 42 / Lake of Rage (Routes 42-46) | BGM | 12 | qz_songs | 0:43.6 (2304 ticks) | 1056 | Johto_LakeOfRage, Johto_LakeOfRageHiddenPowerHouse, Johto_LakeOfRageMagikarpHouse, Johto_LakeOfRage_Alt, Johto_Route42, Johto_Route42EcruteakGate, +6 more maps |
| 623 | Route 26 / 27 | BGM | 12 | qz_songs | 0:34.7 (1776 ticks) | 240 | Johto_DayOfWeekSiblingsHouse, Johto_Route26, Johto_Route26HealHouse, Johto_Route26South, Johto_Route27, Johto_Route27SandstormHouse, +3 more maps |
| 624 | Pokemon Center (Johto) | BGM | 8 | qz_songs | 0:33.7 (1536 ticks) | 0 | Johto_AzaleaPokecenter1F, Johto_BlackthornPokecenter1F, Johto_CherrygrovePokecenter1F, Johto_CianwoodPokecenter1F, Johto_EcruteakPokecenter1F, Johto_GoldenrodPokecenter1F, +7 more maps |
| 625 | Poke Mart (Johto) | BGM | 11 | qz_songs | 0:52.4 (2880 ticks) | 192 | Johto_AzaleaMart, Johto_BlackthornMart, Johto_CherrygroveMart, Johto_EcruteakMart, Johto_GoldenrodBikeShop, Johto_GoldenrodDeptStore1F, +8 more maps |
| 626 | Pokemon Gym (Johto) | BGM | 14 | qz_songs | 0:42.2 (2028 ticks) | 108 | Johto_AzaleaGym, Johto_BlackthornGym1F, Johto_CianwoodGym, Johto_EcruteakGym, Johto_GoldenrodGym, Johto_MahoganyGym, +2 more maps |
| 627 | Prof. Elm's Lab | BGM | 10 | qz_songs | 0:39.2 (2166 ticks) | 342 | Johto_ElmsLab |
| 628 | Dance Theater (Kimono Girls) | BGM | 9 | qz_songs | 0:26.3 (969 ticks) | 201 | Johto_DanceTheater |
| 629 | Goldenrod Game Corner | BGM | 12 | qz_songs | 1:22.2 (4272 ticks) | 1200 | Johto_GoldenrodGameCorner |
| 630 | Sprout Tower | BGM | 10 | qz_songs | 0:55.6 (2400 ticks) | 1536 | Johto_SproutTower1F, Johto_SproutTower2F, Johto_SproutTower3F |
| 631 | Johto caves / forests (Union Cave, Ilex Forest, Dark Cave...) | BGM | 7 | qz_songs | 1:04.1 (2712 ticks) | 0 | Johto_DarkCaveBlackthornEntrance, Johto_DarkCaveVioletEntrance, Johto_IlexForest, Johto_IlexForest_Copy, Johto_MountMortar1FInside, Johto_MountMortar1FOutside, +18 more maps |
| 632 | Ruins of Alph | BGM | 13 | qz_songs | 0:40.0 (1344 ticks) | 192 | Johto_RuinsOfAlphAerodactylWordRoom, Johto_RuinsOfAlphArceusChamber, Johto_RuinsOfAlphChambers, Johto_RuinsOfAlphHoOhWordRoom, Johto_RuinsOfAlphInnerChamber, Johto_RuinsOfAlphKabutoWordRoom, +2 more maps |
| 633 | National Park / Bug-Catching Contest | BGM | 12 | qz_songs | 1:50.9 (4035 ticks) | 771 | Johto_NationalPark, Johto_NationalParkBugContest |
| 634 | Burned Tower | BGM | 11 | qz_songs | 0:52.8 (2016 ticks) | 864 | Johto_BurnedTower1F, Johto_BurnedTowerB1F |
| 635 | Tin (Bell) Tower | BGM | 14 | qz_songs | 0:53.7 (1920 ticks) | 384 | Johto_EcruteakCityNorth, Johto_TinTower1F, Johto_TinTower2F, Johto_TinTower3F, Johto_TinTower4F, Johto_TinTower5F, +6 more maps |
| 636 | Mt. Silver (also Olivine Lighthouse) | BGM | 8 | qz_songs | 0:36.4 (1920 ticks) | 192 | Johto_OlivineLighthouse, Johto_SilverCaveFireRoom, Johto_SilverCaveItemRoom, Johto_SilverCaveMountainside, Johto_SilverCaveOutside, Johto_SilverCaveRoom1, +3 more maps |
| 637 | Team Rocket Hideout | BGM | 11 | qz_songs | 0:48.0 (2496 ticks) | 192 | Johto_TeamRocketBaseB1F, Johto_TeamRocketBaseB2F, Johto_TeamRocketBaseB3F, Johto_MahoganyMart1F (playbgm) |
| 638 | Ice Path | BGM | 9 | qz_songs | 1:21.0 (5184 ticks) | 960 | Johto_IcePath1F, Johto_IcePathB1F, Johto_IcePathB2FBlackthornSide, Johto_IcePathB2FMahoganySide, Johto_IcePathB3F |
| 639 | Dragon's Den / Dragon Shrine | BGM | 14 | qz_songs | 1:12.0 (3744 ticks) | 672 | Johto_DragonShrine, Johto_DragonsDen1F, Johto_DragonsDenB1F |
| 640 | Victory Road (Johto) | BGM | 9 | qz_songs | 0:44.3 (2304 ticks) | 0 | Johto_VictoryRoad1F, Johto_VictoryRoad2F, Johto_VictoryRoad3F |
| 641 | Indigo Plateau / Elite Four rooms | BGM | 8 | qz_songs | 0:46.7 (2688 ticks) | 768 | Johto_BrunosRoom, Johto_IndigoPlateau, Johto_IndigoPlateauPokecenter1F, Johto_KarensRoom, Johto_KogasRoom, Johto_LancesRoom, +2 more maps |
| 642 | Goldenrod Underground (warehouse) | BGM | 9 | qz_songs | 0:37.7 (2412 ticks) | 12 | Johto_GoldenrodUndergroundSwitchRoomEntrances, Johto_GoldenrodUndergroundWarehouse, Johto_GoldenrodCity (playbgm) |
| 643 | Cliff Cave / Routes 47-48 | BGM | 9 | qz_songs | 1:10.8 (2832 ticks) | 144 | Johto_CliffCave, Johto_CliffEdgeCave, Johto_EmbeddedTower, Johto_Route47, Johto_Route48 |
| 644 | Safari Zone Gate | BGM | 7 | qz_songs | 0:56.0 (2688 ticks) | 576 | Johto_SafariZoneEntrance, Johto_SafariZoneGate |
| 645 | Safari Zone (Johto) | BGM | 9 | qz_songs | 1:04.0 (3072 ticks) | 192 | Johto_SafariZone_Beach, Johto_SafariZone_Desert, Johto_SafariZone_EntrancePath, Johto_SafariZone_Forest, Johto_SafariZone_Peak, Johto_SafariZone_Plains, +1 more maps |
| 646 | Prof. Oak (scene at Mr. Pokemon's House) | BGM | 13 | qz_songs | 0:47.8 (3096 ticks) | 600 | Johto_MrPokemonsHouse (playbgm) |
| 647 | Follow me! (Guide Gent in Cherrygrove, Violet scene) | BGM | 7 | qz_songs | 0:15.7 (804 ticks) | 420 | Johto_CherrygroveCity (playbgm), Johto_VioletCity (playbgm) |
| 648 | Rival appears (Johto rival encounter; encounter music 15) | BGM | 10 | qz_songs | 0:27.0 (1968 ticks) | 432 | Johto_AzaleaTown (playbgm), Johto_CherrygroveCity (playbgm), Johto_EcruteakCity (playbgm), +7 more scripts |
| 649 | Rival departs (after rival battles: Cherrygrove, Azalea, Burned Tower) | BGM | 11 | qz_songs | 0:22.4 (1632 ticks) | 96 | Johto_AzaleaTown (playbgm), Johto_BurnedTower1F (playbgm), Johto_CherrygroveCity (playbgm), +2 more scripts |
| 650 | short jingle (? 4 s, no loop; picked by ROM code) | BGM | 3 | qz_songs | 0:04.0 (384 ticks) | 0 | - |
| 651 | event in Cianwood City (Eusine / Suicune scene?) | BGM | 13 | qz_songs | 0:27.5 (1542 ticks) | 390 | Johto_CianwoodCity (playbgm) |
| 652 | Vs. Ho-Oh | BGM | 14 | qz_songs | 1:25.6 (6246 ticks) | 1254 | johto_shared.inc:Quetzal_EventScript_83FB8BE (playbgm) |
| 653 | Vs. Lugia | BGM | 12 | qz_songs | 1:11.2 (5352 ticks) | 1512 | Johto_WhirlIslandLugiaChamber (playbgm) |
| 654 | Lugia / Ho-Oh appears (Whirl Islands chamber, Tin Tower roof) | BGM | 8 | qz_songs | 0:24.8 (804 ticks) | 36 | Johto_WhirlIslandLugiaChamber (playbgm), johto_shared.inc:Quetzal_EventScript_83FB64A (playbgm) |
| 655 | event theme (Goldenrod Underground, Ilex Forest, Ice Path scenes) | BGM | 9 | qz_songs | 0:26.3 (969 ticks) | 201 | Johto_GoldenrodUnderground (playbgm), Johto_IcePath1F (playbgm), Johto_IlexForest (playbgm), +1 more scripts |
| 656 | short sound effect (SE2 player, 2.5 s) | SE2 | 6 | qz_songs | 0:02.5 (154 ticks) | - | - |
| 657 | Trainer eyes meet: Lass / Picnicker / Twins (encounter music 19) | BGM | 8 | qz_songs | 0:16.2 (960 ticks) | 192 | - |
| 658 | Trainer eyes meet: Youngster / Bug Catcher / boys (encounter music 17) | BGM | 11 | qz_songs | 0:15.4 (984 ticks) | 216 | - |
| 659 | Trainer eyes meet: Super Nerd / Juggler (encounter music 21) | BGM | 9 | qz_songs | 0:24.1 (1272 ticks) | 120 | - |
| 660 | Trainer eyes meet: Sage / Hex Maniac (encounter music 23) | BGM | 8 | qz_songs | 0:23.7 (1260 ticks) | 300 | - |
| 661 | Trainer eyes meet: Team Rocket (encounter music 14) | BGM | 9 | qz_songs | 0:13.7 (876 ticks) | 108 | Johto_PlayersHouse1F (fadenewbgm) |
| 662 | Trainer eyes meet: Cooltrainer / Beauty / Swimmer F (encounter music 20) | BGM | 10 | qz_songs | 0:15.0 (864 ticks) | 96 | - |
| 663 | Trainer eyes meet: Swimmer M / Policeman (encounter music 18) | BGM | 11 | qz_songs | 0:14.6 (960 ticks) | 192 | - |
| 664 | Trainer eyes meet: Hiker / Fisherman / Sailor / Black Belt (encounter music 22) | BGM | 6 | qz_songs | 0:22.1 (1272 ticks) | 504 | - |
| 665 | Trainer eyes meet: Champion / PKMN Trainer (encounter music 16) | BGM | 13 | qz_songs | 1:07.8 (4992 ticks) | 1728 | - |
