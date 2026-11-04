# KANTO / JOHTO audit: handoff (paused 2026-11-04)

The work: every case in `docs/REGIONS_AUDIT.md` (the user's 157 decisions on the KANTO vs FireRed / JOHTO vs Crystal
script audit, plus case 158). The user's instruction: "Please continue until all audit cases are complete." The user
paused the work on 2026-11-04 to finish later. Trigger: when the user says to carry on with the audit, read this file,
then `docs/REGIONS_AUDIT.md` (the decisions and the rules at its top), then the case's finding in
`docs/regions_audit/<K1..K4|J1..J3>_findings.md`.

## Where it stands

- **110 of 158 cases are ticked** (`- [x]` in REGIONS_AUDIT.md; its status line says which release did which).
- Released: 1.88.0 (batch A), 1.89.0 (B), 1.90.0 (C, KANTO), 1.91.0 (D, JOHTO), 1.92.0 (E part 1: hidden items,
  weekday siblings, the Sunday lady...), **1.93.0** (E part 2: GOLDENROD's tutor, GAME CORNER TM prizes, NAME RATER,
  UNDERGROUND, CIANWOOD's SEER, the tutor / VAR_RESULT fix): pushed when the session paused; check its release run
  (Actions, "Release") went green, and re-queue it (workflow_dispatch) if not.
- **Prepared, not run:** `tools/quetzal_port/fix_floria.py` (case 100: ROUTE 36's FLORIA by the odd tree, the FLOWER
  SHOP's teacher and FLORIA on Crystal's conditions, the lass, new flags 0x1542-0x1545 and their MP_JohtoWeekdayPeople
  lines). Read it, run it once, then `python3 tools/quetzal_port/restore_words.py`, build, and write a test. Its last
  line in restore_words (the RUINS gate's "gramps") points Crystal's gramps words at obj 1 (3,5) of
  Johto_Route36RuinsOfAlphGate, which may be the wrong person: check the gate's three people against Crystal's
  (officer + wandering gramps) first.

## The 48 cases left (`grep "^- \[ \]" docs/REGIONS_AUDIT.md`)

Small (a person, a sign, some lines): 46, 49 (JOHTO's HEADBUTT man on the tutor flow), 50, 66, 89, 100 (prepared),
101, 105, 115, 117, 122, 124, 126, 138 (rest: VIOLET's signs, ELM's window and house PC, the RUINS chamber scientists,
the RUINS gate officer / POKéFAN, ROUTE 33's sign).

Medium: 25 + 71 (CELADON DEPT. STORE), 45 (leaders' rematches / DOJO), 47 (DAY-CARE through the PC), 74 (CYCLING
ROAD slope), 88 (FAN CLUB), 96 (GOLDENROD DEPT. STORE; 5F still has Quetzal's MEGA STONE / Z-CRYSTAL clerks: keep
every item obtainable, `python3 tools/moemon_gen59/item_audit.py`), 106 (MT. SILVER / RED), 110 (BLACKTHORN / DRAGON'S
DEN gating), 116 (remove HGSS events), 120 (VIRIDIAN's catching demo: reuse 133's MP_JohtoCatchingTutorial pattern).

Large: 10 (TRAINER TOWER port), 37 (Crystal's BATTLE TOWER), 57 (FireRed / Crystal trainer teams; 17, 24, 112, 130
depend on it), 58 (trades), 59 (marts by town), 60 (legendaries; the user's rule at the top of REGIONS_AUDIT), 61
(fossils), 62 (both SAFARI ZONEs; 146's PHOTO STUDIO door is Quetzal's CLIFF EDGE CAVE to the JOHTO SAFARI ZONE, and
156's SAFARI part go with it), 64 (POKéDEX rating / NATIONAL DEX), 65 (legendary story; with 139 WISE TRIO / SUICUNE
and 113 LANCE / EUSINE scenes), 15 + 104 (Crystal / KANTO TMs, WHIRLPOOL), 97 (Bug-Catching Contest), 125 (KANTO
GAME CORNER games), 141 (BUENA's PASSWORD).

## How each case is done (the loop)

1. Read the finding; read the original (pokecrystal in `/tmp/claude-0/frlg/pokecrystal`, a sparse clone: add paths
   with `git sparse-checkout add DIR`; FireRed via tools/quetzal_port/fix_move_tutors.py's fetch(), cached in
   /tmp/claude-0/frlg). A new session's /tmp is empty: re-clone pret/pokecrystal (sparse) the same way.
2. Words are never written by hand: add `OUR_MAP OUR_LABEL gsc|fr THEIR_MAP|path THEIR_LABEL` lines to
   `tools/quetzal_port/restore_words.txt` and run `restore_words.py` (crystal_words.py / firered_words.py convert;
   placeholders `.string "x$"` get filled). Only a Crystal text that starts with a text_ram buffer is typed by hand
   (keep Crystal's words, the buffer as STR_VAR_n).
3. Objects: map.json edits; `kai_gone.py MAP id:N` removes Quetzal's people (FLAG_KAI_GONE keeps local ids);
   `map_ascii.py MAP [X0 Y0 X1 Y1]` shows collision and objects to pick spots. People who come on certain days / times
   wear a FLAG_JOHTO_HIDE_* set or cleared in `src/mp/mp_region.c` MP_JohtoWeekdayPeople (runs on every JOHTO map
   load).
4. New flags: JOHTO's in `include/constants/kai_regions.h` (free run used up to 0x1545 by fix_floria.py; hidden items
   0x1600-0x1639); daily flags 0x923-0x929, 0x935-0x939 used (0x93A-0x942 still free). Menus: KAI_MENUS in
   `tools/quetzal_port/gen_multichoice.py` (run it, then `touch src/script_menu.c`).
5. A test per case in `tools/test/scenarios/johto_*` / `kanto_*` (+ a row in tools/test/README.md), notes in
   docs/COOP_NOTES.md (the "1.9x.0 (the audit's batch ...)" sections near the end), tick the case and the status line
   in REGIONS_AUDIT.md, a player-facing line in the next `tools/release/notes/v<ver>.md`.
6. Release (CLAUDE.md): bump MOEMON_VERSION, gen_patchnotes.py, `make linux`, the region tests
   `NOBUILD=1 OUT=dir tools/test/regress.sh $(ls tools/test/scenarios | grep -E "^(region_|johto_|kanto_)") hm_rescue tm_reuse`,
   `tools/release/build_windows.sh /tmp/claude-0/sdl/SDL2-2.30.9` (re-fetch SDL2 mingw if missing), commit, push to
   the default branch (`git push origin next:claude/clever-clarke-hqati2`; fetch and rebase first: other sessions push
   there), watch the run.

## Things learned this time (don't trip on them again)

- `mp getflag N` already exists (prints `flag <decimal> = <0|1>`): don't add another debug command with a used name.
- JOHTO / KANTO maps run `QzJohtoTravelingEventCheck` on ON_RESUME; it no longer writes VAR_RESULT (1.93.0). Any other
  special run from an ON_RESUME / ON_LOAD map script must not either, or a script back from a menu reads the wrong
  answer.
- `mp hour H` (new) and `mp weekday D` set the game clock (hour first); `mp weekday` fails silently if FLAG 0x51 is set.
- Talking across a counter needs the counter metatile's behavior: Quetzal's maps have plain counter tiles beside the
  clerks' (GOLDENROD DEPT. STORE 5F needed one tile swapped to the clerks' 726).
- The harness's `typetext` doesn't reach the naming screen; test the NO branch instead.
- Hidden items: elevation 0 (matches any height); spots chosen by eye against Crystal's maps (gen_hidden_items.py
  SPOTS); its preview script lived in the scratchpad (render Crystal's .blk with its tileset png beside ours).
