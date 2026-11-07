# POKéMON STADIUM on BATTLE ISLAND

The user's request (2026-11-05, verbatim; the work list is T15a-T15d). Decisions and status below it.

```
POKEMON STADIUM
Using Pokémon Stadium's decomp, we will port the Stadium modes into Moemon Emerald.
A special ticket to BATTLE ISLAND will be given after defeating the Elite 4 in any region. BATTLE ISLAND can be traveled to from any region as well.
Upon travel to BATTLE ISLAND, the player will happen across a giant building, where entering takes them into a copy of MOEMON EMERALD's Elite Four Pokémon Center.
The NURSE NPC will give information about prior results of Cups, and current champions, if any.
The MART NPC will explain the STADIUM, its rules, and the basics.
Upon entering, the player will be given the selection of different Stadiums to enter and participate in.
If a player doesn't already have a ticket to BATTLE ISLAND upon loading a save, they will be given one by an NPC that fits the region they are currently in.
https://github.com/pret/pokestadium
CORE FEATURES:

* Battle system is unchanged from Kai - it will use Kai's battle system.
* Every cup from Pokémon Stadium.
* Register teams for use in the Stadium, exactly how Pokémon Stadium does it. Stadium's menu is referenced, but the style will be the style of Moemon Emerald.
* View and select Pokémon from all available boxes, including the PERSONAL, SHARED, and GLOBAL PC to create a team of up to 6, or use Rental Pokémon, defined by the original game, or a mixture of both. The manner of selection is Pokémon Stadium's.
* Every rental Pokémon available from said cups.
* Special Stadium and Cup rules apply. Level restrictions on Pokémon apply.
* The rules for special cups like the Petit Cup will still be followed exactly, referencing Pokedex height and weight.
* Rules such as special clauses will be explained on the screen prior to battle.
* Graphics for the STADIUM SELECTION will be taken from Stadium directly, and upon entering POKEMON STADIUM, will be the only prompt available, other than to exit.
* Trainer graphics, pre-battle, and Pokémon selection will work identically to Pokémon Stadium, with Kai's graphic style and Moemon graphics. No Stadium 3D models will be used.
* All music will be Pokémon Stadium's music, converted to Kaisong format.
* Teams saved to the Stadium will remain saved, and be usable by other players as well who have played on the server.

The following Cups will be available, with their Poke Ball, Great Ball, Ultra Ball, and Master Ball variations.
Petit Cup
Pika Cup
Poké Cup
Prime Cup
Challenge Cup
Challenge cup has extremely special rules. The Challenge Cup will ONLY use the Pokémon provided in Stadium 2, as a special case. Please refer to https://bulbapedia.bulbagarden.net/wiki/Challenge_Cup for specific implementation.
Gym Leader Tower will be available as an additional mode, with R1 and R2 available.
All teams, movesets, TRAINER names, TRAINER IDs, and Pokémon nicknames will be identical to their Pokémon Stadium counterparts. Natures given to the Pokémon will specifically compliment their primary attack type, and an additional stat that benefits them, such as speed for Persian, or HP for Snorlax. IVs and DVs will be converted to their closest equivalent to Gen 3. This counts for Rentals as well as TRAINER Pokémon. No moves outside of Gen 1 will be given or can be used.
Achievements will be given for every step of Pokémon Stadium cups that are completed, with great rewards upon full completion. The rewards for beating the Stadiums must be several orders of magnitude better than the original rewards, such as perfect IV Pokémon. A special Stadium ribbon will be given to Pokémon who complete different tiers of the Stadiums. Exceptional rewards will be given for completions using only Rental Pokémon.
```

The user's cup options (2026-11-06, verbatim):

```
I believe that for each cup, there should be the ability to turn on or off certain restrictions in order to gain more rewards.

Here are some of the restrictions available.

LEVEL CAP - Any entry Pokemon will be adjusted to the level cap of the Stadium Cup in question, still keeping the same exclusions as uncapped. If turned off, greater rewards are given.
SPECIES - Only species from GEN 1 are allowed. If this setting is turned off, then any Pokemon that is NOT a restricted & Legendary Pokémon, such as Major box-art legendaries and mythicals (e.g., Mewtwo, Groudon, Kyogre, Dialga, Palkia, Zacian, Koraidon, Miraidon) are permitted to enter, with the exception of Cups that would allow all Pokemon in Gen 1.  If turned on, greater rewards are given.
HELD ITEMS - If this setting is on, allows you to have Held Items on entry Pokemon.  If turned off, greater rewards are given.
RENTALS ONLY - if a trainer registers a team of Pokemon consisting only of Rentals and completes a Cup, they will greater rewards.

If every restriction is in place, the player receives great rewards.
```

How it's read (noted for the user): each switch's harder side earns more: LEVEL CAP OFF (the POKéMON must fit the
cup's levels as they are; ON scales them to the cap), SPECIES ON (the first 151 and Generation I moves; OFF opens every
POKéMON but the restricted legendaries and mythicals, and Generation I moves aren't required), HELD ITEMS OFF, RENTALS
ONLY. "Every restriction in place" = all four on their hard side: the greatest rewards. In a cup that allows all of
Generation I (the PRIME CUP, MEWTWO and MEW included) SPECIES OFF also allows the restricted legendaries. The CHALLENGE
CUP's POKéMON are given, so its switches don't apply (its own rewards).

## Status

- T15b (data) done: every STADIUM trainer, team and rental list (src/data/mp_stadium_data.h, tools/stadium/gen_data.py)
  and the CHALLENGE CUP's pools, templates and trainers (src/data/mp_stadium_cc.h, tools/stadium/gen_challenge.py from
  tools/stadium/challenge_cup.json); src/mp/mp_stadium.c builds them (Stadium_BuildMon, StadiumCc_MakeTeam); test
  stadium_data (mp stadium / mp ccteam). The cups' rules: docs/stadium/RULES.md (from the six pages the user sent).
- BATTLE ISLAND's boat (1.163.0): HOENN's ferry clerks list BATTLE ISLAND among their destinations (LILYCOVE's
  SSTIDAL_SELECTION_BATTLE_ISLAND in script_menu.c, SLATEPORT's MULTI_SSTIDAL_SLATEPORT_*_BI; JOHTO's, KANTO's and
  SINNOH's ports still ask first, EventScript_BattleIslandOffer). The boat lands on the tile in front of the island's
  sailor (object 0) as the map has it, Moemon Maker's edits included (mp_battleisland.c LandingSpot): move or turn
  the sailor in the editor and the landing follows. Tests battle_island_voyage, battle_island_lilycove.
- T15a (BATTLE ISLAND, BATTLE TICKET, the building): built (mp_battleisland.c); T15c (screens, src/mp/mp_stadium_ui.c),
  saved teams (mp_stadium_teams.c) and T15d (music, rewards, ribbons, achievements) done: the sections below.
- Still missing: the user's GYM LEADER CASTLE picture (graphics/stadium/gym_leader_castle.png, 160x120; the link the
  user sent couldn't be fetched from here: tools/stadium/extract_gfx.py picks it up when the file is there).

## Music (T15d)

STADIUM's own songs from the user's USF rips (34 of them), converted with Moemon Music Studio into
music/stadium_*.kaisong. A shipped music file named `stadium_*` is a new track of its own (mp_kaisong.c ScanDir, like
the Maker's MAKER_SONG_FIRST tracks), found by name: mp_stadium_cup.c MP_Stadium_Track (log "stadium: song NAME").
The screens: stadium_menu (STADIUM SELECT, the classes and options), stadium_select (registering, VS, saved teams),
the castle's / MEWTWO's / CHALLENGE CUP's own menus; battles by cup and stage (`stadium_<cup>_1` battles 1-3, `_2`
4-6, `_semi`, `_final`), the castle's trainers, leaders, ELITE FOUR and RIVAL, MEWTWO, the CHALLENGE CUP's FREE BATTLE;
results stadium_won / stadium_cup_won / castle / MEWTWO won / stadium_lost. Playback quality: the user, 2026-11-06,
"The current playback quality is fine." (an earlier ask for 98-99% fidelity was dropped).

## Rewards (T15d)

src/mp/mp_stadium_rewards.c. A class won for the first time at a hardness (16 of them: each switch's hard side, so a
class can be won again harder for more) gives: a prize POKéMON with perfect IVs (3, one more for each restriction in
place; the others no lower than 15-31), shiny only in a MASTER BALL class (POKé / PRIME / CHALLENGE CUP) won with rentals
only or with every restriction (the user, 2026-11-07); from the cup's rental list (STADIUM's own POKéMON, moves and
nature), the castle's and CHALLENGE CUP's from STADIUM's gift POKéMON (BULBASAUR, CHARMANDER, SQUIRTLE, HITMONLEE,
HITMONCHAN, EEVEE, OMANYTE, KABUTO), MEWTWO (Round 1) / MEW (Round 2) for beating MEWTWO; RARE CANDIES and PP MAXes,
money (the class, the round, each restriction); a MASTER BALL for rentals only, an ASTRAL BALL only in a MASTER BALL
class with every restriction. The CHALLENGE CUP counts as every restriction (its six are given). A win already won at
that hardness: money and a RARE CANDY. The result screen lists them (StadiumRewards_Last).

The STADIUM RIBBONS go on the registered team's own POKéMON (party and PC; rentals are STADIUM's, the shared / global
PCs' copies stay as their owners keep them) on every win, a repeat one too, so a POKéMON new to the team gets them
(the result screen counts only the ones that didn't have them yet), in the save's spare gift ribbon slots, so the
summary and the POKéNAV show them with their own words (gift_ribbon_descriptions.h 65-67, MAX_GIFT_RIBBON 67): STADIUM
RIBBON (MARINE's slot, any cup but the CHALLENGE CUP), STADIUM MASTER RIBBON (LAND's: a MASTER BALL class, the castle or
MEWTWO), STADIUM LEGEND RIBBON (SKY's: LEVEL CAP off, SPECIES on and HELD ITEMS off; RENTALS ONLY isn't asked, as
rentals can't carry ribbons: before 1.164.0 it was, so the ribbon reached no one).

The opponents pick their three by a weighted draw (mp_stadium_cup.c StadiumCup_FoePicks): each of their six weighs by
how its types do against the player's six, squared (cubed in the ULTRA / MASTER BALL classes, the castle and MEWTWO),
so good matchups come most often but any can; the draw mixes the PC's clock into the game's random numbers (before
1.164.0 the matchups decided and the same three came every time: the user, "too predictable"). Log "stadium: foe picks".

The screens fade (1.164.0, the user: "so that it doesn't seem too janky"): the field fades to black, then the screens
fade in; one screen crossfades into the next (the last picture kept, mp_stadium_ui.c U.snap); leaving (B, a battle,
the SUMMARY) fades them to black first; a battle starts from black (no field transition) and hands back a black field
(MP_Stadium_FieldStaysBlack) the screens fade in over. Test stadium_fades.

## The announcer (1.165.0)

The user (2026-11-07): "add support for Pokemon Stadium's announcer to fire off voice lines based on the situation ...
an additional option in the options menu, on by default. Keep in mind the length of the voice lines, as well as the
animations being different from the original game ... intelligently determine which line could or could not make it in
time, or may get cut off by another line if the player is mashing the A button to speed up text, and if the text speed
is on the highest setting."

- The voices: Stadium's speech archive (an S1 container at ROM 0x197C1E0: 165, 1 and 657 MORT streams = clips 0-822,
  16000 Hz) copied out of Pokemon Stadium (USA) 1.0 into stadium/announcer.s1 (tools/stadium/mort/make_voice_pack.py,
  6.5 MB; make_pack.sh downloads the ROM from the STADIUM_ROM_URL secret in the release; never committed: stadium/ is
  ignored). Without the pack the game slices the archive out of the player's own ROM dropped in stadium/
  ("Pokemon Stadium (USA).z64", .v64 / .n64 too). MOEMON_STADIUM_VOICE=file for tests.
- Decoding: SubDrag's MORT decoder (mort/, public domain, mort/README.txt), on a worker thread: the clips a battle may
  want (both teams' send-outs and moves, the generic lines) are asked for when it starts; all 823 decode in about a
  second. A clip not decoded yet when wanted is skipped. MOEMON_ANNOUNCER_DUMP=folder writes each decoded clip as a WAV
  (+ MOEMON_ANNOUNCER_DUMPALL=1: every clip).
- Mixing: src/mp/mp_stadium_announcer.c MP_StadiumAnnouncer_Mix after the game's own sound (music_player.c
  m4aSoundVSync), resampled to 42048 Hz, each clip's peak brought to the same level, the game's sound lowered to 72%
  under the voice.
- The clips (the map and many words from StadiumBattleFX's research, MIT: research/ANNOUNCER.md and
  announcer_catalog.json; every line used here checked by transcribing the decoded clip): 0-150 short names,
  151-164 PIKACHU's voice, 166-199 battle state (166 / 167 last one, 181 / 182 first move, 183-188 turn prompts),
  200-221 tournament introductions, 222 the castle, 223-230 BROCK..GIOVANNI, 231-234 LORELEI..LANCE, 235 the rival,
  239-244 a gym's first / second / third trainer, 245-255 switches, 261-347 damage, effectiveness, misses, failures
  and statuses, 348-368 faints and results (365 the win, 366 champion, 367 stage clear, 368 the castle cleared),
  369-519 "Oh! It's ...!" (368 + dex), 584-748 the moves (583 + move id, Gen I's 165), 749-822 battle flow.
- Cues: every battle message in a STADIUM battle (battle_message.c BufferStringBattle -> MP_Announcer_OnBattleString):
  send-outs (dex 1-151), moves (1-165), super / not very effective, critical hits, misses, failures, the six statuses,
  faints (both down within 150 frames, the last one, both down to the last, the win when the foe's last falls),
  switches (the player's / the foe's). The player choosing for 150+ frames: the first move / a turn prompt / a
  heated battle (both under half HP after turn 3); 900+: "What's the matter, trainer?" (battle_controller_player.c
  MP_BattlePlayer_Choosing). The screens (mp_stadium_ui.c): the castle's introductions on the VS screen, stage clear /
  the castle / champion on the result screen.
- Timing (the user's point): each clip's length is in its MORT header (frames x 160 / rate), so nothing is decoded to
  know it. Each kind of cue has a window: the frames that went by from it to the next battle message, learned as a
  running average through the battle, kept apart for when A is being pressed (2+ presses in a second: mashing); until
  learned, a guess from the text speed (SLOW 1.6x, MID 1.25x), the host's animation speed and mashing (0.6x). A line
  plays only if it fits its window (15% over allowed; none when the hit already took the target's HP, so a faint
  is next); none fits: a faint or the win may run 40% over, anything else is skipped. A playing line is never cut
  off: a new cue is dropped, or (a faint, the win) held and said when the line ends if it still fits the time left.
  Lines said lately aren't repeated while another fits. The log says each decision ("announcer: clip N (why, F
  frames)", "skipped (nothing fits W frames)", "dropped (a line is playing)", "held"); `mp announcer` prints the
  windows learned. Test stadium_announcer (needs stadium/announcer.s1).
- OPTIONS > BATTLE > ANNOUNCER (gMpConfig.noAnnouncer, ini stadium_announcer = 1 / 0), on by default.

Achievements: the STADIUM tab (mp_achieve.c AK_STADIUM, read from the save's wins, so old wins count): a first win,
each cup, the castle, MEWTWO, the CHALLENGE CUP, ROUND 2, rentals only (1, 10), every restriction (1, all of ROUND 1),
100 battles. Test stadium_rewards (`mp stadiumwin ROUND CUP CLASS HARDNESS [1 = show the result screen]`).

## The data (T15b)

- The user's ROM and soundtrack (2026-11-06): https://drive.google.com/file/d/1BGhq3Tyo2oeNhAfaM4nZQl-FR6dzlbc7
  ("ROM and Music.zip": Pokemon Stadium (USA) 1.0 / Rev 1 / Rev 2, the soundtrack as USF rips of NUS-CP2J = the same
  game). Kept in ~/.cache/moemon/stadium_dl, never committed or shipped. The 1.0 ROM is pret/pokestadium's
  (md5 ed1378bc12115f71209a77844965ba50).
- tools/stadium/extract.py ROM OUT.json reads every trainer and rental: the table of tables at 0x898000 (Data Crystal's
  ROM map), 62 blocks: Round 1 / Round 2 of the Petit, Pika, Prime (POKé / GREAT / ULTRA / MASTER BALL) and Poké
  (the same four) Cups, the Gym Leader Castle (8 gyms of 3 trainers + the leader, the Elite Four, the rival), Mewtwo,
  Battle Now, and the rental lists (Petit 45, Pika 91, Poké / Castle 149, Prime 149 / 150). Record formats in the
  script's header. Trainer names are the game's (JR.♂ / COOL♀ are stored with ¾ / © for the symbols).
- The Challenge Cup is Stadium 2's (the user's rule): pret/pokestadium oldnotes/stadiumgs/rosters/challengecup.
- Conversions (the user's rules, src/mp/mp_stadium.c Stadium_Convert): Gen 1 moves only (move ids 1-165 are the same numbers); DVs to IVs as 2 x DV + 1
  (15 -> 31); stat EXP to EVs; natures that raise the attacking stat the moveset uses (and the stat the user named:
  speed for the fast ones, HP-like bulk for the slow ones go to the EVs, since no nature raises HP).
- Trainer IDs: STADIUM stores 0 for every trainer's and rental's POKéMON; they keep it (00000), with STADIUM's OT names.
- The rival's seven teams (VENUSAUR, BLASTOISE, CHARIZARD, VAPOREON, JOLTEON, FLAREON leads, and a seventh): the one
  that counters the player's six best is chosen (StadiumRival_Pick, with the screens).
- Pictures: STADIUM's classes as KANTO's FireRed pictures (gen_data.py PICS); MEWTWO shows MEWTWO.

## Saved teams (the user, 2026-11-06)

"players must be able to register up to 99 teams per Server, and erase teams as well without prior authorization, copy
teams as well, and be able to use each others."

A cup's options screen has SAVED TEAMS: 99 slots. Online they are the server's (mp_scores.c, store ops MP_ST_TEAM_PUT /
ERASE / GET / LIST, "stadium_teams.dat" where the server runs): everyone on the server sees the same 99, anyone can USE
(the six become the cup's registered team), COPY (to the first free slot), SAVE OVER or ERASE any of them, and the
room hears of each change at once. Offline the save keeps its own 99 ("<save>.stadiumteams"). ERASE asks once on the
player's own screen; nobody else's permission is needed. src/mp/mp_stadium_teams.c; test stadium_teams_online.

## RENTALS ONLY and LEVEL CAP (the user, 2026-11-06)

"you cannot have Level Cap as well as Rentals Only on, because they logically cannot coexist together. Rentals already
are the perfect level. If you turn on Rentals Only, Level Cap turns off automatically." Then: "Not the reverse. Just the
way I mentioned." and "You can still choose rentals without Rentals Only on, for the record."

RENTALS ONLY is a switch (STADIUM_OPT_RENTALS): on, it turns LEVEL CAP off and keeps it off (LEVEL CAP greyed, A on it
explains why); turning LEVEL CAP on never turns RENTALS ONLY off. With it on only rentals can be registered; with it off
rentals and the player's own may both enter.

## The picks, SET style and QUIT (the user, 2026-11-06)

"Just let the player scroll through all 6 left and right. The Summary screen should be viewable from this screen."
"all battles in the STADIUM must be set to Set, not Shift." "Add a confirmation so that players don't get too far and
accidentally quit."

The VS (picks) screen's cursor goes on from the third POKéMON to the fourth with RIGHT (and back with LEFT); UP / DOWN
still move by rows. R opens the summary screen on the six (Task_OpenVsSummary), coming back on the one last looked at.
B / QUIT asks first (U.confirm: B again keeps playing, A quits). Each STADIUM battle sets OPTIONS_BATTLE_STYLE_SET and
CB2_EndStadiumBattle puts the player's own style back (mp_stadium_cup.c sSavedStyle). Test stadium_picks.

## The lobby's fans (1.168.0)

The user (2026-11-07): "add more NPCs to Pokemon Stadium, who emphatically talk about the Stadiums themselves, similar
to trainers in the BATTLE FRONTIER." Seven people in the lobby (data/maps/BattleIsland_Stadium), each crazy about one
part: the PETIT, PIKA, POKé and PRIME CUPS, the CHALLENGE CUP, the GYM LEADER CASTLE, and a reporter. Their words
follow the cups' own rules in mp_stadium_cup.c (levels, sums, the PETIT CUP's height and weight); change them there
first if a cup's rules change.

## The trainers' switching (1.173.0)

src/mp/mp_stadium_ai.c (the user, 2026-11-07: "allow STADIUM trainers to switch out Pokemon intelligently"): in a STADIUM
battle battle_ai_switch_items.c's ShouldSwitch asks MP_StadiumAi_ShouldSwitch after its trap / PERISH SONG / WONDER
GUARD checks, and GetMostSuitableMonToSwitchInto asks MP_StadiumAi_Replacement after a faint. A matchup is the best
move's damage each way as a share of the target's HP (power, STAB, the type chart, the stats, LEVITATE / absorbing
abilities / WONDER GUARD), plus or minus a knock-out first by Speed. A trainer switches when a bench POKéMON's matchup
(less half the hit it takes coming in) beats the one out by 35 points of HP and the one out is in danger (60%+ a hit)
or can't hurt the foe (under 15%); never twice in two turns, never out of boosts, never when it finishes the foe
first, never when nearly gone anyway. `mp stadiumai` runs fixed matchups (test stadium_ai).
