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
