# SUPER CONTESTS: the build (SINNOH, 1:1 with Platinum)

The user's call (2026-11-02): "Super Contests should be added, as well as their unique functions. Poke Ball effects take
the place of Seals wherever they would appear in the story, or in menus or shops." Progress is tracked in
docs/SINNOH_CHECKLIST.md section 5; this file holds the rules as read from pret/pokeplatinum (6eeb1b0) and how they
map onto this game.

## Done
- The FASHION CASE (src/sinnoh/s3d_fashion.c): 100 accessories (the first 61 up to 9 each, the rest one), 18 backdrops,
  "<save>.region" chunk FASH; the scripts' commands convert. POFFINS are this game's POKéBLOCKS (the notes' rule).
- Seals are POKé BALL effect pieces (src/sinnoh/s3d_seals.c, COOP_NOTES "Seals as POKé BALL effects").
- The art: sinnoh/platinum_gfx.pak "CNTS" (tools/sinnoh/build_contest.py): every accessory and backdrop as RGB555
  (S3D_Gfx_ContestPicture).

## Platinum's flow (src/contest.c FieldTask_RunContest)
The hall's receptionist (scripts_contests.s) picks rank, type, the POKéMON, then the contest runs as scenes in order:
VISUAL (dress-up, then its judging), DANCE (official / link: always; practice: when chosen), ACTING (the same), FINAL
SCORING, back to the hall. Four entrants: the player (entry 0) and three opponents from contest_data.narc member 0
(by rank, type, game progress: unk_02094EDC.c sub_02094F04).

## VISUAL round rules (ov17_0223F118.c, unk_02094EDC.c)
- Condition score: primary condition + (the two neighbours + sheen) / 2, times the scarf: 110% the type's own scarf,
  105% the neighbours' (COOL: RED; TOUGH / BEAUTY 105. BEAUTY: BLUE; RED / PINK. CUTE: PINK; BLUE / GREEN. SMART: GREEN;
  PINK / YELLOW. TOUGH: YELLOW; GREEN / RED). Neighbours: COOL (TOUGH, BEAUTY), BEAUTY (COOL, CUTE), CUTE (BEAUTY,
  SMART), SMART (CUTE, TOUGH), TOUGH (SMART, COOL). In this game: the save's own five conditions and sheen.
- Condition hearts 0-8 (sub_02095928): count of thresholds passed, per rank:
  NORMAL 10 20 30 40 50 60 70 80; GREAT 90 110 ... 230 (+20); ULTRA 170 200 ... 380 (+30); MASTER 320 360 ... 600
  (+40); LINK 100 200 300 400 450 500 550 600.
- Dress-up score: the sum, over the (up to 20) accessories worn, of the theme's table (contest_data.narc members 3-14:
  12 themes x 100 accessories, one byte each). Dress-up hearts (sub_0209598C): 0 when nothing fits; else 1 + the
  thresholds passed: NORMAL 3 5 8, GREAT 5 10 15, ULTRA 7 15 23, MASTER / LINK 10 20 30.

## DANCE round (ov17_0223DAD0.c, ov17_0224CFB8.c, ov17_0224E930.c)
- The song by rank and type (Unk_ov17_0225312C / Unk_ov17_02253084): NORMAL one song, GREAT another, ULTRA / MASTER one
  per type; each with its tempo (120, 120, 100, 60, 150, 132, 140), the lead's steps (3 on the first, else 4) and the
  judging distances in 30ths of a second ({2,2,3,3} or {1,1,2,2}).
- Four rounds, each entrant leads one: the lead's half (8 beats) then the backups' half copying it.
- A step's grade (ov17_0224DE54 / ov17_0224DD90): its distance from the nearest half-beat; 0-1 EXCELLENT (+2 points),
  2-3 GOOD (+1), past that MISS; a backup's step needs the lead's step at the same half-beat, in the same direction.
- Computer leads (ov17_0224E990): NORMAL / GREAT on whole beats 1-7, ULTRA / MASTER on half beats 1-15 never next to
  another (odd ones half as often); each nudged by a wobble from its skill (1-4: x2 NORMAL, /2 ULTRA, /3 MASTER); the
  first direction random, then the same as the last 90% (NORMAL) / 40% (GREAT) / never, the last one random.
- Computer backups (ov17_0224EE90): a wobble (skill x2 at NORMAL / GREAT, x1.5 ULTRA, x1 MASTER; Platinum's own code
  throws its other terms away) and a missed direction one time in (wobble + 8 when the lead's direction changed, by
  rank) per hundred.

## ACTING round (ov17_0223B140.c, ov17_02245F14.c, ov17_02243AC4.c, ov17_02246ECC.c)
- Four turns; each entrant picks a move (not the last one, unless an effect allowed it) and a judge (the middle one is
  the special judge). The first turn's order is the entry order (practice: reversed); then the lowest first, effects
  moving some first or last or shuffling all.
- A performance: the effect's appeal (10 = a heart); the performer's own effect (23 kinds: first / last next turn, 2
  hearts when the voltage rises, the judge's voltage as points, lowering every judge's voltage, doubling next turn,
  stealing the voltage bonus of the one before, suppressing voltage for the rest, shuffling, doubling the final act,
  low voltage bonus, first / last performance bonus, no voltage down, two voltage rises in a row, later turns score
  more, after a full-voltage performance); the doubled effect from the turn before; the judge's voltage: +1 for the
  contest's type, -1 for the two far types (a table per type), full at 5: +5 hearts (+8 from the special judge).
- After everyone: each judge's impression: +3 alone with a judge, +2 two, +1 three, 0 all four; the judge-count effects
  (+2 per other on the same judge, +3 alone, +15 when all four share one); pity points (+3 for the lowest).
- The computer's choice: Platinum's table of 165 rows (place in the order, a condition, a target: a move type, an effect
  or one with 2+ hearts, a judge rule, weights by AI level); the highest-scored move, its best judge; sometimes (by rank)
  away from the player's judge when nothing pulls it anywhere.

## FINAL scoring (ov17_02250744.c)
- Each round in shares of 100 (official: a third each; VISUAL+DANCE 70/30; VISUAL+ACTING 60/40; a single round 100):
  each entrant's round points times the share over the round's best; the total; ties broken by chance.
- VISUAL's points are its hearts (condition 0-8 + dress-up 0-4); DANCE's its step points; ACTING's its points.

## What happens after (src/contest.c Contest_EndContest)
- Official contests only: a win sets the ribbon (here the game's contest ribbon of the type at rank + 1), the first
  win's accessory (Contest_CalcFirstTimeVictoryAccessoryReward), the winner's photo per type, friendship, the TV's
  CONTEST HALL segment.

## Built (src/sinnoh/s3d_contest.c, s3d_contest_ui.c)
- Every rule above; the hall's commands; the screens (dress-up: the FASHION CASE's accessories on the POKeMON over a
  backdrop for a minute, L / R backdrops; the judging; the dance with the D-pad as UP / FRONT / LEFT / RIGHT; acting
  menus; results bars).

## Still to build
- Contests with friends (co-op link contests: StartBattleClient / Server answer "cancelled" now), JUBILIFE TV's
  dress-up photos (ScrCmd_0A6, ShowDressUpPhoto), the link records (ShowLinkContestRecords), the POFFIN HOUSE's
  cooking (the game's BERRY BLENDER), Platinum's stage art (contest_bg.narc / contest_obj.narc).
