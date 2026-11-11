# UNOVA: the user's decisions

Pokémon Black's UNOVA as a playable region, handled exactly as Platinum's SINNOH was (the user, 2026-11-11: "Every
single decision that stood for Platinum will stand for Unova"). Read with CLAUDE.md's SINNOH and ORRE paragraphs and
docs/SINNOH_HANDOFF.md: whatever was decided or adjusted for SINNOH holds here unless this file says otherwise.

## The user's words (2026-11-11)

- 1:1 with Black and White: every story beat, every POKéMON (already in the game), every line of dialogue.
- The decompilation: https://github.com/Project-Holon/aletheia (a byte-matching decomp of Black / White; code only:
  maps, text, scripts, graphics and sound come from a ROM).
- UNOVA gets its own choice in the new game's intro, and can be sailed to as another region.
- UNOVA's TMs and HMs are appended to the TM list (as TM51-TM139 were: a TM whose number has another move here gets a
  new number).
- Every POKéMON and trainer found applies the GENERATION settings correctly (below).
- Completable from start to finish; optional content later, as with Platinum.
- Anything decided for Platinum and not mentioned here is made here too.

## Answers to the questions (2026-11-11)

- Version: **Black only** (its exclusives, ZEKROM / RESHIRAM as Black has them, BLACK CITY).
- GENERATION: **at least Gen 5 in UNOVA**. Below 5, UNOVA's wild POKéMON and trainers are still Gen 5 and usable
  there and wherever they go; the setting still caps moves and abilities past Gen 5 elsewhere.
- The end of the first playable release: **N's Castle and the credits** (the ELITE FOUR, then N and GHETSIS, the
  HALL OF FAME).
- The Drive file: **a Black / White ROM**. Used as ORRE's disc is: packs built from it at build time, never committed;
  the release fetches it through a new repository secret (UNOVA_ROM_URL) the user adds. Download it outside the repo.
- 3D engine: **its own** (src/unova/, like ORRE's src/orre/: never SINNOH's code; Platinum's renderer may be copied
  as a start, then Black's own camera).
- Black's systems for the first release: **the C-GEAR** as UNOVA's device (its wireless sides done by Kai's co-op),
  **POKéMON MUSICALS** (as SINNOH's SUPER CONTESTS were ported), **seasons** (changing every month by the clock:
  maps, wild POKéMON, DEERLING / SAWSBUCK), **the ENTRALINK** (its forest and missions, co-op friends in place of
  wireless).
- Port: **CASTELIA CITY**.
- Sailing there: **as JOHTO / KANTO / SINNOH** (the same ticket, ferries and voyage scenes).

## The second set of answers (2026-11-11, after UNOVA_ROM_URL was added)

- People: Black's own overworld sprites and trainer battle pictures (as SINNOH's are Platinum's), HILBERT / HILDA as
  LOOKs with Black's action frames; OPTIONS > DISPLAY > SPRITE SCALING gets UNOVA PEOPLE, **MIXED by default** (resized
  to Gen 3's standards, as SINNOH PEOPLE).
- Music: **converted from the ROM** at build time (never committed), through the game's own engine.
- Wild POKéMON: **Black's own tables** (Gen 5 POKéMON before the credits, Black's post-game additions after); they don't
  grow with GENERATION.
- ROTATION trainers: fought as **TRIPLE BATTLES** with the same teams.
- New game: **Black's whole opening** (JUNIPER's speech, waking with CHEREN, BIANCA and the gift box), Kai's own steps
  (name, LOOK, settings) kept.
- The GARDEN: **a door added** to each UNOVA POKéMON CENTER's back wall, in Black's style.
- Tag battles: **Black's partner** (CHEREN / BIANCA / N); friends JOIN as a third, as with other trainers.
- The XTRANSCEIVER: **Black's story calls, and video calls to co-op friends** (their screen and yours, like SPECTATE).
- The CARD CLUB: **CASTELIA CITY** (a building Black leaves empty), sellers in centers as elsewhere.
- The shared GAME CENTER's door: **NIMBASA CITY**.
- The C-GEAR's buttons: **co-op equivalents** (IR: trade / battle the friend in front; WIRELESS: the CO-OP menu;
  ENTRALINK: go there); skins kept.
- The ENTRALINK forest: **Black's own forest list** (Hidden Abilities).
- Releases: **area by area**, each playable; UNOVA offered once the first towns play through.
- Testing: **walk-through story tests run here** (as SINNOH's and KANTO's), plus the user's playthroughs.
- POKéDEX: **JUNIPER's is the NATIONAL DEX** (as KANTO / JOHTO).
- Liberty Garden: **kept**, like FRLG's and Emerald's event islands (the LIBERTY PASS from the same source; VICTINI;
  after the credits).
- Battle backgrounds: **Black's own**, each place's, fitted to Kai's battle screen.
- Seasons in co-op: **the room host's** month (as the time of day); alone, the PC's.
- MUSICALS: **up to four on stage** with co-op friends.
- PASS POWERS: **as Black, shareable** with co-op friends in the room.
- EXP: **Kai's** everywhere.
- Catching: **Black's** in UNOVA (critical captures by the POKéDEX count, the dark grass rate).
- The camera: **Black's own moves while walking** (CASTELIA, the bridges), fitted to 240x160.
- Battle mechanics: **Kai's GENERATION / MOVE KIND settings** (UNOVA's at least Gen 5).
- The ROM: the user's is Pokemon - Black Version (USA, Europe) (NDSi Enhanced), SHA-1
  26ad0b9967aa279c4a266ee69f52b9b2332399a5 (aletheia's `black` target). Kept in ~/.cache/moemon/unova/black.nds.
- Items (the user, 2026-11-11): an item Kai already has is **Kai's own** (its number, picture, words); an item Kai
  doesn't have yet is **added from Black** (its picture, name and description as Black has them).
