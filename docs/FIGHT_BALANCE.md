# Fight balance audit

Made by `tools/fight/balance_audit.py` (6.5) from the files in `mugen/`; run it again after any change to the characters (tools/fight/ingest.py does). Numbers are MUGEN's units: life points, frames at 60 a second. Timing is estimated from each move's animation and hit boxes; advantage = hit (or guard) stun minus the frames left after the first active one.

Baseline: the roster's median. Life 1000.0, attack 100.0, defence 100.0, power 3000.0; damage by class: light 28, medium 132, heavy 180, special 144.25, super 225.

| | Likely mistakes | Character traits | Needs a runtime check |
|---|---|---|---|
| ryu | 0 | 0 | 13 |
| ken | 0 | 0 | 9 |
| Chun-Li | 0 | 1 | 0 |
| Dhalsim | 0 | 1 | 2 |
| cammy | 0 | 0 | 8 |
| Fei Long | 0 | 0 | 8 |
| SSFDeejay | 0 | 1 | 19 |
| T.HAWK | 0 | 0 | 20 |
| Vega | 0 | 1 | 12 |
| gouki | 0 | 0 | 5 |

## Likely mistakes

None.

## Character traits (intentional differences)

- **Chun-Li** power: power 1000 against the baseline's 3000
- **Dhalsim** power: power 1000 against the baseline's 3000
- **SSFDeejay** damage: its hits are 48% of the roster's on the whole
- **Vega** power: power 1000 against the baseline's 3000

## Needs a runtime check

- **ryu** advantage: normal 200 leaves +5 frames on guard (estimate)
- **ryu** damage: heavy 225 does 334, 176% of what its class does in this character's scale
- **ryu** damage: light 250 does 83, 282% of what its class does in this character's scale
- **ryu** damage: light 255 does 76, 258% of what its class does in this character's scale
- **ryu** damage: heavy 270 does 360, 190% of what its class does in this character's scale
- **ryu** damage: heavy 310 does 334, 176% of what its class does in this character's scale
- **ryu** advantage: normal 460 leaves +5 frames on guard (estimate)
- **ryu** damage: super 3000 does 510, 215% of what its class does in this character's scale
- **ryu** damage: super 3010 does 510, 215% of what its class does in this character's scale
- **ryu** damage: super 3100 does 562, 237% of what its class does in this character's scale
- **ryu** damage: super 3200 does 0, 0% of what its class does in this character's scale
- **ryu** damage: super 3200 does no damage the files show (LifeAdd on itself? a counter?)
- **ryu** timing: 5 moves hit from boxes or states the animation alone doesn't show (900, 1200, 1300, 3000, 3010): their frames need the fights
- **ken** advantage: normal 210 leaves +6 frames on guard (estimate)
- **ken** damage: heavy 225 does 334, 186% of what its class does in this character's scale
- **ken** damage: light 250 does 83, 296% of what its class does in this character's scale
- **ken** damage: heavy 310 does 334, 186% of what its class does in this character's scale
- **ken** damage: special 1500 does 374, 259% of what its class does in this character's scale
- **ken** damage: super 3000 does 562, 250% of what its class does in this character's scale
- **ken** damage: 1 moves give their damage as an expression (1000, e.g. 1000: ceil(ifelse(!var(6),229,ifelse(var(6)=1,): the measured damage settles them
- **ken** timing: 7 moves hit from boxes or states the animation alone doesn't show (355, 360, 900, 1000, 1100, 1200, 1600): their frames need the fights
- **Dhalsim** damage: 1 moves give their damage as an expression (500, e.g. 500: 119 - Var(10)*21): the measured damage settles them
- **Dhalsim** timing: 1 moves hit from boxes or states the animation alone doesn't show (500): their frames need the fights
- **cammy** damage: medium 215 does 236, 179% of what its class does in this character's scale
- **cammy** damage: heavy 220 does 334, 186% of what its class does in this character's scale
- **cammy** advantage: normal 225 leaves +5 frames on guard (estimate)
- **cammy** advantage: normal 265 leaves +5 frames on guard (estimate)
- **cammy** advantage: normal 270 leaves +7 frames on guard (estimate)
- **cammy** damage: super 3000 does 486, 216% of what its class does in this character's scale
- **cammy** damage: 2 moves give their damage as an expression (1000, 1100, e.g. 1000: ceil(ifelse(!var(6),167,ifelse(var(6)=1,): the measured damage settles them
- **cammy** timing: 5 moves hit from boxes or states the animation alone doesn't show (900, 1000, 1100, 1200, 1300): their frames need the fights
- **Fei Long** advantage: normal 210 leaves +5 frames on guard (estimate)
- **Fei Long** advantage: normal 215 leaves +6 frames on guard (estimate)
- **Fei Long** advantage: normal 220 leaves +6 frames on guard (estimate)
- **Fei Long** damage: heavy 225 does 334, 186% of what its class does in this character's scale
- **Fei Long** advantage: normal 270 leaves +7 frames on guard (estimate)
- **Fei Long** advantage: normal 500 leaves +5 frames on guard (estimate)
- **Fei Long** damage: 2 moves give their damage as an expression (1100, 1200, e.g. 1100: ceil(ifelse(!var(6),229,201)*var(52)/3),): the measured damage settles them
- **Fei Long** timing: 6 moves hit from boxes or states the animation alone doesn't show (900, 1000, 1001, 1002, 1100, 1200): their frames need the fights
- **SSFDeejay** damage: light 200 does 23, 171% of what its class does in this character's scale
- **SSFDeejay** advantage: normal 200 leaves +6 frames on guard (estimate)
- **SSFDeejay** advantage: normal 210 leaves +8 frames on guard (estimate)
- **SSFDeejay** advantage: normal 220 leaves +6 frames on guard (estimate)
- **SSFDeejay** advantage: normal 221 leaves +6 frames on guard (estimate)
- **SSFDeejay** damage: light 230 does 26, 193% of what its class does in this character's scale
- **SSFDeejay** advantage: normal 230 leaves +7 frames on guard (estimate)
- **SSFDeejay** damage: light 231 does 39, 290% of what its class does in this character's scale
- **SSFDeejay** advantage: normal 231 leaves +10 frames on guard (estimate)
- **SSFDeejay** advantage: normal 241 leaves +7 frames on guard (estimate)
- **SSFDeejay** advantage: normal 250 leaves +6 frames on guard (estimate)
- **SSFDeejay** advantage: normal 251 leaves +5 frames on guard (estimate)
- **SSFDeejay** damage: light 400 does 23, 171% of what its class does in this character's scale
- **SSFDeejay** advantage: normal 400 leaves +9 frames on guard (estimate)
- **SSFDeejay** advantage: normal 410 leaves +5 frames on guard (estimate)
- **SSFDeejay** damage: light 430 does 28, 208% of what its class does in this character's scale
- **SSFDeejay** damage: special 1600 does 24, 35% of what its class does in this character's scale
- **SSFDeejay** damage: 2 moves give their damage as an expression (1400, 2100, e.g. 1400: ifelse(animelemno(0) = 4, 50, 30)): the measured damage settles them
- **SSFDeejay** timing: 5 moves hit from boxes or states the animation alone doesn't show (1000, 1200, 1400, 1600, 2000): their frames need the fights
- **T.HAWK** advantage: normal 200 leaves +12 frames on guard (estimate)
- **T.HAWK** advantage: normal 201 leaves +12 frames on guard (estimate)
- **T.HAWK** damage: light 230 does 112, 359% of what its class does in this character's scale
- **T.HAWK** advantage: normal 230 leaves +11 frames on guard (estimate)
- **T.HAWK** damage: light 231 does 112, 359% of what its class does in this character's scale
- **T.HAWK** advantage: normal 231 leaves +12 frames on guard (estimate)
- **T.HAWK** advantage: normal 240 leaves +14 frames on guard (estimate)
- **T.HAWK** advantage: normal 241 leaves +10 frames on guard (estimate)
- **T.HAWK** advantage: normal 300 leaves +12 frames on guard (estimate)
- **T.HAWK** advantage: normal 310 leaves +10 frames on guard (estimate)
- **T.HAWK** advantage: normal 320 leaves +6 frames on guard (estimate)
- **T.HAWK** advantage: normal 330 leaves +11 frames on guard (estimate)
- **T.HAWK** advantage: normal 340 leaves +14 frames on guard (estimate)
- **T.HAWK** damage: heavy 350 does 322, 161% of what its class does in this character's scale
- **T.HAWK** damage: super 1230 does 0, 0% of what its class does in this character's scale
- **T.HAWK** damage: super 1230 does no damage the files show (LifeAdd on itself? a counter?)
- **T.HAWK** damage: super 1231 does 0, 0% of what its class does in this character's scale
- **T.HAWK** damage: super 1231 does no damage the files show (LifeAdd on itself? a counter?)
- **T.HAWK** timing: 3 moves hit from boxes or states the animation alone doesn't show (430, 440, 450): their frames need the fights
- **Vega** damage: light 230 does 50, 206% of what its class does in this character's scale
- **Vega** advantage: normal 400 leaves +6 frames on guard (estimate)
- **Vega** damage: light 430 does 50, 206% of what its class does in this character's scale
- **Vega** advantage: normal 430 leaves +5 frames on guard (estimate)
- **Vega** advantage: normal 440 leaves +5 frames on guard (estimate)
- **Vega** advantage: normal 500 leaves +6 frames on guard (estimate)
- **Vega** advantage: normal 510 leaves +5 frames on guard (estimate)
- **Vega** damage: light 530 does 50, 206% of what its class does in this character's scale
- **Vega** advantage: normal 530 leaves +5 frames on guard (estimate)
- **Vega** advantage: normal 540 leaves +5 frames on guard (estimate)
- **Vega** damage: 24 moves give their damage as an expression (200, 210, 220, 300, 310, 320, 400, 410, 420, 450, 500, 510 ..., e.g. 200: IfElse(Var(20) <= 0, 55, 50), 0): the measured damage settles them
- **Vega** timing: 3 moves hit from boxes or states the animation alone doesn't show (800, 810, 3500): their frames need the fights
- **gouki** damage: special 1700 does 315, 200% of what its class does in this character's scale
- **gouki** damage: super 3200 does 0, 0% of what its class does in this character's scale
- **gouki** damage: super 3200 does no damage the files show (LifeAdd on itself? a counter?)
- **gouki** damage: 32 moves give their damage as an expression (200, 205, 210, 215, 220, 225, 250, 255, 260, 265, 270, 275 ..., e.g. 200: ceil(28*var(52)/3)): the measured damage settles them
- **gouki** timing: 27 moves hit from boxes or states the animation alone doesn't show (200, 210, 260, 270, 300, 305, 310, 320, 325, 330, 350, 355 ...): their frames need the fights
- **ken** measured damage: state 3000: hits measured at 41.5 (median of 2) against 562 in the files
- **T.HAWK** measured damage: state 1020: hits measured at 27 (median of 1) against 189 in the files

## Characters

### ryu

Life 1000, attack 100, defence 100, power 3000, air juggle 15; walk 3.43 / -2.27, run 4.6, jump -9.2. Files: `chars/ssf2x_ryu/act/ryu_1.act`, `chars/ssf2x_ryu/act/ryu_10.act`, `chars/ssf2x_ryu/act/ryu_11.act`, `chars/ssf2x_ryu/act/ryu_12.act`, `chars/ssf2x_ryu/act/ryu_2.act`, `chars/ssf2x_ryu/act/ryu_3.act`, `chars/ssf2x_ryu/act/ryu_4.act`, `chars/ssf2x_ryu/act/ryu_5.act`, `chars/ssf2x_ryu/act/ryu_6.act`, `chars/ssf2x_ryu/act/ryu_7.act`, `chars/ssf2x_ryu/act/ryu_9.act`, `chars/ssf2x_ryu/option.txt`, `chars/ssf2x_ryu/ryu.air`, `chars/ssf2x_ryu/ryu.cmd`, `chars/ssf2x_ryu/ryu.cns`, `chars/ssf2x_ryu/ryu.sff`, `chars/ssf2x_ryu/ryu.snd`, `chars/ssf2x_ryu/ssf2x_ryu.def`, `data/common1.cns`.

| state | kind | command | hits | damage (guard) | stun hit / guard | startup / active / recovery | adv. hit / guard | power | cancels | measured |
|---|---|---|---|---|---|---|---|---|---|---|
| 200 | normal | `x` | 1 | 28 | 10/10 | 2 / 3 / 3 | +5 / +5 |  |  | - |
| 205 | normal | `x` | 1 | 28 | 10/10 | 3 / 3 / 4 | +4 / +4 |  |  | 1 hits, median 28 |
| 210 | normal | `y` | 1 | 132 | 13/13 | 3 / 3 / 7 | +4 / +4 |  |  | - |
| 215 | normal | `y` | 1 | 132 | 13/13 | 3 / 2 / 13 | -1 / -1 |  |  | - |
| 220 | normal | `z` | 1 | 167 | 17/17 | 4 / 5 / 17 | -4 / -4 |  |  | - |
| 225 | normal | `z` | 2 | 167 + 167 | 17/17, 17/17 | 3 / 6 / 17 | -5 / -5 |  |  | 2 hits, median 167 |
| 250 | normal | `a` | 1 | 83 | 9/9 | 4 / 6 / 4 | +0 / +0 |  |  | - |
| 255 | normal | `a` | 1 | 76 | 9/9 | 5 / 5 / 3 | +2 / +2 |  |  | - |
| 260 | normal | `b` | 1 | 146 | 13/13 | 6 / 6 / 5 | +3 / +3 |  |  | 1 hits, median 146 |
| 265 | normal | `b` | 1 | 146 | 11/11 | 3 / 4 / 7 | +1 / +1 |  |  | - |
| 270 | normal | `c` | 2 | 180 + 180 | 17/17, 17/17 | 2 / 9 / 13 | -4 / -4 |  |  | - |
| 275 | normal | `c` | 2 | 187 + 28 | 16/16, 16/16 | 6 / 10 / 8 | -1 / -1 |  |  | 1 hits, median 187 |
| 300 | normal | `/$D` | 1 | 28 | 8/8 | 4 / 3 / 3 | +3 / +3 |  |  | - |
| 305 | normal | `/$D` | 1 | 132 | 11/11 | 4 / 3 / 6 | +3 / +3 |  |  | - |
| 310 | normal | `/$D` | 2 | 167 + 167 | 16/16, 16/16 | 3 / 9 / 17 | -9 / -9 |  |  | - |
| 320 | normal | `/$D` | 1 | 28 | 8/8 | 4 / 3 / 4 | +2 / +2 |  |  | - |
| 325 | normal | `/$D` | 1 | 132 | 13/13 | 4 / 5 / 7 | +2 / +2 |  |  | 1 hits, median 132 |
| 330 | normal | `/$D` | 1 | 153 | 19/19 | 5 / 5 / 19 | -4 / -4 |  |  | 1 hits, median 153 |
| 350 | normal | `x` | 1 | 76 | 8/8 | 2 / 1 / -1 | +9 / +9 |  |  | - |
| 355 | normal | `y` | 1 | 132 | 9/9 | 4 / 16 / 8 | -14 / -14 |  |  | - |
| 360 | normal | `z` | 1 | 153 | 8/8 | 4 / 7 / 9 | -7 / -7 |  |  | 1 hits, median 153 |
| 365 | normal | `a` | 1 | 83 | 8/8 | 3 / 1 / -1 | +9 / +9 |  |  | - |
| 370 | normal | `b` | 1 | 132 | 9/9 | 4 / 9 / 9 | -8 / -8 |  |  | - |
| 375 | normal | `c` | 2 | 153 + 153 | 8/8, 8/8 | 2 / 6 / 8 | -5 / -5 |  |  | 1 hits, median 153 |
| 400 | normal | `a` | 1 | 76 | 8/8 | 5 / 1 / -1 | +9 / +9 |  |  | - |
| 405 | normal | `b` | 1 | 132 | 9/9 | 5 / 11 / 7 | -8 / -8 |  |  | - |
| 410 | normal | `c` | 1 | 132 | 8/8 | 5 / 5 / 8 | -4 / -4 |  |  | 1 hits, median 132 |
| 420 | normal | `y` | 2 | 146 + 28 | 8/8, 8/8 | 4 / 9 / 6 | -6 / -6 |  |  | - |
| 450 | normal | `/$F` | 2 | 21 + 180 | 11/11, 12/12 | 14 / 5 / 6 | +1 / +1 |  |  | - |
| 460 | normal | `/$F` | 2 | 28 + 201 | 16/16, 16/16 | 8 / 3 / 9 | +5 / +5 |  |  | - |
| 900 | throw | `/$F` | 13 | 0 + 0 + 132 + 132 + 167 + 167 ... | 0/0, 0/0, 13/13 | - | - |  |  | - |
| 1200 | special | `~D, DF, F, x` | 1 | 125 (21) [p] | 19/19 | None / - / - | - |  +125 |  | 1 hits, median 125 |
| 1300 | special | `~B, DB, D, DF, F, x` | 2 | 118 (12) [p] + 118 (12) [p] | 16/16, 16/16 | None / - / - | - |  +125 |  | 4 hits, median 118 |
| 3000 | super | `~D, DF, F, D, DF, x` | 1 | 102x5 (24) [p] | 18/18 | None / - / - | - | -3000 |  | - |
| 3010 | super | `~D, DF, F, D, DF, x` | 1 | 102x5 (24) [p] | 18/18 | None / - / - | - | -3000 |  | - |
| 3100 | super | `~D, DF, F, D, DF, a` | 5 | 111 (28) + 111 (28) + 118 (28) + 111 (28) + 111 (28) | 28/28, 28/28, 28/28 | 15 / 44 / 5 | -20 / -20 | -3000 |  | - |
| 3200 | super | `~x, x, F, a, z` | 1 | 0 | 0/0 | 21 / 33 / 1 | -33 / -33 | -3000 |  | - |

### ken

Life 1000, attack 100, defence 100, power 3000, air juggle 15; walk 3.43 / -2.27, run 4.6, jump -9.2. Files: `chars/ssf2x_ken/act/ken_1.act`, `chars/ssf2x_ken/act/ken_2.act`, `chars/ssf2x_ken/act/ken_3.act`, `chars/ssf2x_ken/act/ken_4.act`, `chars/ssf2x_ken/act/ken_5.act`, `chars/ssf2x_ken/act/ken_6.act`, `chars/ssf2x_ken/act/ken_7.act`, `chars/ssf2x_ken/act/ken_8.act`, `chars/ssf2x_ken/act/ken_9.act`, `chars/ssf2x_ken/ken.air`, `chars/ssf2x_ken/ken.cmd`, `chars/ssf2x_ken/ken.cns`, `chars/ssf2x_ken/ken.sff`, `chars/ssf2x_ken/ken.snd`, `chars/ssf2x_ken/option.txt`, `chars/ssf2x_ken/ssf2x_ken.def`, `data/common1.cns`.

| state | kind | command | hits | damage (guard) | stun hit / guard | startup / active / recovery | adv. hit / guard | power | cancels | measured |
|---|---|---|---|---|---|---|---|---|---|---|
| 200 | normal | `x` | 1 | 28 | 10/10 | 3 / 3 / 4 | +4 / +4 |  |  | 6 hits, median 28 |
| 205 | normal | `x` | 1 | 28 | 10/10 | 3 / 3 / 5 | +3 / +3 |  |  | 19 hits, median 28 |
| 210 | normal | `y` | 1 | 132 | 13/13 | 5 / 3 / 5 | +6 / +6 |  |  | 1 hits, median 132 |
| 215 | normal | `y` | 1 | 132 | 13/13 | 5 / 1 / 15 | -2 / -2 |  |  | 10 hits, median 132 |
| 220 | normal | `z` | 1 | 167 | 17/17 | 5 / 5 / 16 | -3 / -3 |  |  | 4 hits, median 146.5 |
| 225 | normal | `z` | 2 | 167 + 167 | 17/17, 17/17 | 4 / 6 / 19 | -7 / -7 |  |  | 19 hits, median 167 |
| 250 | normal | `a` | 1 | 83 | 9/9 | 7 / 3 / 5 | +2 / +2 |  |  | 14 hits, median 83 |
| 260 | normal | `b` | 1 | 146 | 13/13 | 8 / 6 / 5 | +3 / +3 |  |  | 8 hits, median 146 |
| 265 | normal | `b` | 1 | 146 | 11/11 | 6 / 6 / 9 | -3 / -3 |  |  | 5 hits, median 146 |
| 270 | normal | `c` | 1 | 160 | 17/17 | 7 / 4 / 16 | -2 / -2 |  |  | 10 hits, median 160 |
| 300 | normal | `/$D` | 1 | 28 | 8/8 | 3 / 3 / 5 | +1 / +1 |  |  | 2 hits, median 28 |
| 305 | normal | `/$D` | 1 | 132 | 11/11 | 4 / 3 / 5 | +4 / +4 |  |  | 1 hits, median 132 |
| 310 | normal | `/$D` | 2 | 167 + 167 | 16/16, 16/16 | 3 / 8 / 18 | -9 / -9 |  |  | 3 hits, median 167 |
| 320 | normal | `/$D` | 1 | 28 | 8/8 | 4 / 3 / 4 | +2 / +2 |  |  | 1 hits, median 28 |
| 325 | normal | `/$D` | 1 | 132 | 13/13 | 4 / 5 / 7 | +2 / +2 |  |  | 5 hits, median 132 |
| 330 | normal | `/$D` | 1 | 153 | 19/19 | 4 / 4 / 20 | -4 / -4 |  |  | 5 hits, median 153 |
| 350 | normal | `x` | 1 | 76 | 8/8 | 2 / 1 / -1 | +9 / +9 |  |  | 7 hits, median 76 |
| 355 | normal | `y` | 1 | 132 | 9/9 | - | - |  |  | 2 hits, median 132 |
| 360 | normal | `z` | 1 | 153 | 8/8 | - | - |  |  | 4 hits, median 153 |
| 365 | normal | `a` | 1 | 83 | 8/8 | 4 / 1 / -1 | +9 / +9 |  |  | - |
| 370 | normal | `b` | 2 | 0 + 132 | 0/0, 9/9 | 1 / 14 / 9 | -22 / -22 |  |  | - |
| 375 | normal | `c` | 3 | 0 + 153 + 153 | 0/0, 8/8, 8/8 | 1 / 8 / 8 | -15 / -15 |  |  | 2 hits, median 125 |
| 400 | normal | `a` | 1 | 76 | 8/8 | 5 / 1 / -1 | +9 / +9 |  |  | 8 hits, median 76 |
| 405 | normal | `b` | 2 | 0 + 146 | 0/0, 9/9 | 1 / 13 / 7 | -19 / -19 |  |  | 5 hits, median 146 |
| 410 | normal | `c` | 2 | 0 + 160 | 0/0, 8/8 | 1 / 9 / 7 | -15 / -15 |  |  | 4 hits, median 160 |
| 900 | throw | `/$F` | 11 | 0 + 0 + 0 + 132 + 132 + 167 ... | 0/0, 0/0, 0/0 | - | - |  |  | - |
| 1000 | special | `~F, D, DF, x` | 5 | 28 (14) + expr (229) + expr (139) + expr (229) + expr (139) | 20/20, 20/20, 20/20 | - | - |  +500 |  | 15 hits, median 187 |
| 1100 | special | `~D, DB, B, a` | 2 | 76 (14) + 76 (14) | 20/20, 20/20 | - | - |  +375 |  | 25 hits, median 76 |
| 1200 | special | `~D, DF, F, x` | 1 | 125 (21) [p] | 19/19 | None / - / - | - |  +125 |  | 10 hits, median 125 |
| 1300 | special | `~D, DF, F, a` | 4 | 28 (7) + 180 (49) + 160 (35) + 160 (35) | 18/18, 18/18, 18/18 | 5 / 7 / 16 | -4 / -4 |  +375 |  | 10 hits, median 28 |
| 1400 | special | `~F, DF, D, a` | 4 | 28 (7) + 180 (49) + 160 (35) + 160 (35) | 17/17, 16/16, 18/18 | 13 / 5 / 10 | +3 / +3 |  +375 |  | 8 hits, median 28 |
| 1500 | special | `~B, DB, D, DF, F, a` | 4 | 187 (42) + 187 (35) + 160 (35) + 160 (35) | 17/17, 16/16, 18/18 | 11 / 6 / 12 | +0 / +0 |  +375 |  | 8 hits, median 187 |
| 1600 | special | `~D, DB, B, a` | 2 | 76 (14) + 76 (14) | 20/20, 20/20 | - | - |  +375 |  | 2 hits, median 76 |
| 3000 | super | `~D, DF, F, D, DF, F, x` | 5 | 111 (28) + 111 (28) + 118 (28) + 111 (28) + 111 (28) | 28/28, 28/28, 28/28 | 15 / 44 / 5 | -20 / -20 | -3000 |  | 2 hits, median 41.5 |

### Chun-Li

Life 1000, attack 100, defence 100, power 1000, air juggle 10; walk 4.0 / -3.0, run 0.0, jump -10.0. Files: `chars/ssf2x_chunli/char/action.st`, `chars/ssf2x_chunli/char/chunli.air`, `chars/ssf2x_chunli/char/chunli.cmd`, `chars/ssf2x_chunli/char/chunli.cns`, `chars/ssf2x_chunli/char/chunli.sff`, `chars/ssf2x_chunli/char/chunli.snd`, `chars/ssf2x_chunli/char/color1.act`, `chars/ssf2x_chunli/char/color10.act`, `chars/ssf2x_chunli/char/color2.act`, `chars/ssf2x_chunli/char/color3.act`, `chars/ssf2x_chunli/char/color4.act`, `chars/ssf2x_chunli/char/color5.act`, `chars/ssf2x_chunli/char/color6.act`, `chars/ssf2x_chunli/char/color7.act`, `chars/ssf2x_chunli/char/color8.act`, `chars/ssf2x_chunli/char/color9.act`, `chars/ssf2x_chunli/char/common.st`, `chars/ssf2x_chunli/char/helper.st`, `chars/ssf2x_chunli/char/state.st`, `chars/ssf2x_chunli/char/tech.st`, `chars/ssf2x_chunli/char/throw.st`, `chars/ssf2x_chunli/ssf2xchunli.def`, `data/common1.cns`.

| state | kind | command | hits | damage (guard) | stun hit / guard | startup / active / recovery | adv. hit / guard | power | cancels | measured |
|---|---|---|---|---|---|---|---|---|---|---|
| 2000 | super | `~60$B, F, B, F, a` | 3 | 98 (21) + 98 (21) + 98 (21) | 22/29, 20/29, 20/29 | 27 / 32 / 14 | -23 / -16 | -1000 |  | - |

### Dhalsim

Life 1000, attack 100, defence 100, power 1000, air juggle 10; walk 1.8 / -1.6, run 0.0, jump -5.6. Files: `chars/ssf2x_dhalsim/char/action.st`, `chars/ssf2x_dhalsim/char/color1.act`, `chars/ssf2x_dhalsim/char/color10.act`, `chars/ssf2x_dhalsim/char/color2.act`, `chars/ssf2x_dhalsim/char/color3.act`, `chars/ssf2x_dhalsim/char/color4.act`, `chars/ssf2x_dhalsim/char/color5.act`, `chars/ssf2x_dhalsim/char/color6.act`, `chars/ssf2x_dhalsim/char/color7.act`, `chars/ssf2x_dhalsim/char/color8.act`, `chars/ssf2x_dhalsim/char/color9.act`, `chars/ssf2x_dhalsim/char/common.st`, `chars/ssf2x_dhalsim/char/dhalsim.air`, `chars/ssf2x_dhalsim/char/dhalsim.cmd`, `chars/ssf2x_dhalsim/char/dhalsim.cns`, `chars/ssf2x_dhalsim/char/dhalsim.sff`, `chars/ssf2x_dhalsim/char/dhalsim.snd`, `chars/ssf2x_dhalsim/char/helper.st`, `chars/ssf2x_dhalsim/char/state.st`, `chars/ssf2x_dhalsim/char/tech.st`, `chars/ssf2x_dhalsim/char/throw.st`, `chars/ssf2x_dhalsim/ssf2xdhalsim.def`, `data/common1.cns`.

| state | kind | command | hits | damage (guard) | stun hit / guard | startup / active / recovery | adv. hit / guard | power | cancels | measured |
|---|---|---|---|---|---|---|---|---|---|---|
| 500 | throw | `` | 3 | 0 + expr + expr | 0/0, 16/16, 16/16 | - | - |  |  | - |

### cammy

Life 1000, attack 100, defence 100, power 3000, air juggle 15; walk 4.57 / None, run 4.6, jump -9.4. Files: `chars/ssf2x_cammy/act/cammy_1.act`, `chars/ssf2x_cammy/act/cammy_2.act`, `chars/ssf2x_cammy/act/cammy_3.act`, `chars/ssf2x_cammy/act/cammy_4.act`, `chars/ssf2x_cammy/act/cammy_5.act`, `chars/ssf2x_cammy/act/cammy_6.act`, `chars/ssf2x_cammy/cammy.air`, `chars/ssf2x_cammy/cammy.cmd`, `chars/ssf2x_cammy/cammy.cns`, `chars/ssf2x_cammy/cammy.sff`, `chars/ssf2x_cammy/cammy.snd`, `chars/ssf2x_cammy/option.txt`, `chars/ssf2x_cammy/ssf2x_cammy.def`, `data/common1.cns`.

| state | kind | command | hits | damage (guard) | stun hit / guard | startup / active / recovery | adv. hit / guard | power | cancels | measured |
|---|---|---|---|---|---|---|---|---|---|---|
| 200 | normal | `x` | 1 | 28 | 9/9 | 3 / 3 / 9 | -2 / -2 |  |  | - |
| 205 | normal | `x` | 1 | 28 | 9/9 | 4 / 3 / 4 | +3 / +3 |  |  | 2 hits, median 28 |
| 210 | normal | `y` | 1 | 118 | 13/13 | 3 / 4 / 6 | +4 / +4 |  |  | - |
| 215 | normal | `y` | 2 | 118 + 118 | 13/13, 13/13 | 3 / 5 / 5 | +4 / +4 |  |  | 3 hits, median 118 |
| 220 | normal | `z` | 2 | 167 + 167 | 17/17, 17/17 | 3 / 7 / 7 | +4 / +4 |  |  | 5 hits, median 166 |
| 225 | normal | `z` | 1 | 167 | 17/17 | 3 / 5 / 8 | +5 / +5 |  |  | 2 hits, median 167 |
| 250 | normal | `a` | 1 | 28 | 9/9 | 3 / 7 / 4 | -1 / -1 |  |  | 4 hits, median 28 |
| 255 | normal | `a` | 1 | 14 | 9/9 | 3 / 5 / 5 | +0 / +0 |  |  | - |
| 260 | normal | `b` | 1 | 118 | 13/13 | 5 / 6 / 8 | +0 / +0 |  |  | 3 hits, median 118 |
| 265 | normal | `b` | 1 | 118 | 13/13 | 4 / 5 / 4 | +5 / +5 |  |  | 4 hits, median 118 |
| 270 | normal | `c` | 1 | 180 | 17/17 | 9 / 4 / 7 | +7 / +7 |  |  | 2 hits, median 180 |
| 275 | normal | `c` | 1 | 180 | 17/17 | 7 / 10 / 9 | -1 / -1 |  |  | 2 hits, median 180 |
| 300 | normal | `/$D` | 1 | 28 | 8/8 | 4 / 3 / 5 | +1 / +1 |  |  | - |
| 305 | normal | `/$D` | 1 | 118 | 11/11 | 4 / 3 / 6 | +3 / +3 |  |  | - |
| 310 | normal | `/$D` | 1 | 180 | 16/16 | 6 / 7 / 9 | +1 / +1 |  |  | - |
| 320 | normal | `/$D` | 1 | 28 | 8/8 | 3 / 3 / 5 | +1 / +1 |  |  | 2 hits, median 28 |
| 325 | normal | `/$D` | 1 | 118 | 13/13 | 4 / 5 / 7 | +2 / +2 |  |  | 1 hits, median 111 |
| 330 | normal | `/$D` | 1 | 180 | 19/19 | 5 / 5 / 19 | -4 / -4 |  |  | 1 hits, median 180 |
| 350 | normal | `x` | 1 | 118 | 8/8 | 2 / 1 / -1 | +9 / +9 |  |  | 3 hits, median 118 |
| 355 | normal | `y` | 2 | 0 + 146 | 0/0, 9/9 | 1 / 13 / 6 | -18 / -18 |  |  | 1 hits, median 146 |
| 360 | normal | `z` | 2 | 0 + 180 | 0/0, 8/8 | 1 / 9 / 6 | -14 / -14 |  |  | 1 hits, median 180 |
| 365 | normal | `a` | 1 | 118 | 8/8 | 3 / 1 / -1 | +9 / +9 |  |  | 2 hits, median 118 |
| 370 | normal | `b` | 2 | 0 + 132 | 0/0, 9/9 | 1 / 13 / 10 | -22 / -22 |  |  | - |
| 375 | normal | `c` | 2 | 0 + 180 | 0/0, 8/8 | 1 / 13 / 10 | -22 / -22 |  |  | 1 hits, median 180 |
| 900 | throw | `/$F` | 11 | 0 + 0 + 118 + 118 + 118 + 167 ... | 0/0, 0/0, 13/13 | - | - |  |  | - |
| 1000 | special | `~D, DF, F, a` | 2 | expr (167) + expr (167) | 25/25, 20/20 | - | - |  +438 |  | 4 hits, median 170.5 |
| 1100 | special | `~F, D, DF, a` | 1 | expr (194) | 21/21 | - | - |  +375 |  | 2 hits, median 194 |
| 1200 | special | `~B, DB, F, x` | 2 | 76 (14) + 167 (42) | 21/21, 21/21 | - | - |  +250 |  | - |
| 1300 | special | `~DB, D, DF, F, UF, x` | 3 | 0 + 0 + 160 (42) | 0/0, 0/0, 21/21 | - | - |  +813 |  | - |
| 3000 | super | `~D, DF, F, D, DF, a` | 5 | 111 (21) + 97 (21) + 97 (21) + 97 (21) + 84 (21) | 22/22, 22/22, 22/22 | 17 / 50 / 10 | -37 / -37 | -3000 | 3001 | 1 hits, median 111 |

### Fei Long

Life 1000, attack 100, defence 100, power 3000, air juggle 15; walk 4.8 / None, run 4.6, jump -9.0. Files: `chars/ssf2x_feilong/act/feilong_1.act`, `chars/ssf2x_feilong/act/feilong_2.act`, `chars/ssf2x_feilong/act/feilong_3.act`, `chars/ssf2x_feilong/act/feilong_4.act`, `chars/ssf2x_feilong/act/feilong_5.act`, `chars/ssf2x_feilong/act/feilong_6.act`, `chars/ssf2x_feilong/feilong.air`, `chars/ssf2x_feilong/feilong.cmd`, `chars/ssf2x_feilong/feilong.cns`, `chars/ssf2x_feilong/feilong.sff`, `chars/ssf2x_feilong/feilong.snd`, `chars/ssf2x_feilong/option.txt`, `chars/ssf2x_feilong/ssf2x_feilong.def`, `data/common1.cns`.

| state | kind | command | hits | damage (guard) | stun hit / guard | startup / active / recovery | adv. hit / guard | power | cancels | measured |
|---|---|---|---|---|---|---|---|---|---|---|
| 200 | normal | `x` | 1 | 28 | 9/9 | 3 / 5 / 3 | +2 / +2 |  |  | 1 hits, median 28 |
| 205 | normal | `x` | 1 | 28 | 9/9 | 4 / 5 / 2 | +3 / +3 |  |  | 4 hits, median 28 |
| 210 | normal | `y` | 1 | 132 | 13/13 | 5 / 4 / 5 | +5 / +5 |  |  | 1 hits, median 132 |
| 215 | normal | `y` | 1 | 132 | 12/12 | 5 / 3 / 4 | +6 / +6 |  |  | - |
| 220 | normal | `z` | 1 | 167 | 17/17 | 5 / 7 / 5 | +6 / +6 |  |  | 2 hits, median 167 |
| 225 | normal | `z` | 2 | 167 + 167 | 17/17, 17/17 | 4 / 11 / 4 | +3 / +3 |  |  | 3 hits, median 167 |
| 250 | normal | `a` | 1 | 28 | 9/9 | 5 / 10 / 3 | -3 / -3 |  |  | 1 hits, median 28 |
| 255 | normal | `a` | 1 | 28 | 9/9 | 5 / 4 / 3 | +3 / +3 |  |  | 3 hits, median 28 |
| 260 | normal | `b` | 1 | 132 | 13/13 | 5 / 7 / 4 | +3 / +3 |  |  | - |
| 265 | normal | `b` | 1 | 132 | 12/12 | 5 / 5 / 4 | +4 / +4 |  |  | 1 hits, median 132 |
| 270 | normal | `c` | 1 | 180 | 17/17 | 7 / 6 / 5 | +7 / +7 |  |  | 1 hits, median 180 |
| 275 | normal | `c` | 2 | 118 + 118 | 17/17, 17/17 | 9 / 8 / 7 | +3 / +3 |  |  | 3 hits, median 118 |
| 300 | normal | `/$D` | 1 | 28 | 7/7 | 4 / 3 / 3 | +2 / +2 |  |  | 1 hits, median 28 |
| 305 | normal | `/$D` | 1 | 132 | 11/11 | 5 / 5 / 6 | +1 / +1 |  |  | - |
| 310 | normal | `/$D` | 1 | 167 | 15/15 | 6 / 6 / 6 | +4 / +4 |  |  | - |
| 320 | normal | `/$D` | 1 | 28 | 8/8 | 5 / 3 / 3 | +3 / +3 |  |  | - |
| 325 | normal | `/$D` | 1 | 139 | 13/13 | 5 / 4 / 6 | +4 / +4 |  |  | - |
| 330 | normal | `/$D` | 1 | 180 | 19/19 | 6 / 3 / 15 | +2 / +2 |  |  | - |
| 350 | normal | `x` | 1 | 83 | 8/8 | 5 / 1 / -1 | +9 / +9 |  |  | - |
| 355 | normal | `y` | 2 | 0 + 132 | 0/0, 9/9 | 1 / 14 / 4 | -17 / -17 |  |  | - |
| 360 | normal | `z` | 2 | 0 + 180 | 0/0, 8/8 | 1 / 9 / 5 | -13 / -13 |  |  | - |
| 365 | normal | `a` | 1 | 83 | 8/8 | 3 / 16 / 3 | -10 / -10 |  |  | - |
| 370 | normal | `b` | 1 | 132 | 9/9 | 4 / 6 / 4 | +0 / +0 |  |  | 1 hits, median 132 |
| 375 | normal | `c` | 1 | 180 | 8/8 | 6 / 6 / 5 | -2 / -2 |  |  | - |
| 380 | normal | `x` | 1 | 83 | 8/8 | 4 / 24 / 0 | -15 / -15 |  |  | - |
| 385 | normal | `y` | 2 | 0 + 132 | 0/0, 9/9 | 1 / 9 / 5 | -13 / -13 |  |  | 1 hits, median 132 |
| 390 | normal | `z` | 2 | 0 + 167 | 0/0, 8/8 | 1 / 9 / 3 | -11 / -11 |  |  | - |
| 400 | normal | `a` | 1 | 83 | 8/8 | 4 / 16 / 3 | -10 / -10 |  |  | 1 hits, median 83 |
| 405 | normal | `b` | 1 | 146 | 9/9 | 4 / 8 / 4 | -2 / -2 |  |  | 2 hits, median 146 |
| 500 | normal | `/$F` | 1 | 146 | 9/9 | 16 / 5 / 0 | +5 / +5 |  |  | - |
| 510 | normal | `/$F` | 2 | 118 + 118 | 15/15, 15/15 | 14 / 8 / 4 | +4 / +4 |  |  | - |
| 900 | throw | `/$F` | 12 | 0 + 0 + 132 + 132 + 167 + 167 ... | 0/0, 0/0, 12/12 | - | - |  |  | - |
| 1000 | special | `~D, DF, F, x` | 2 | 104 (21) + 104 (21) | 9/9, 15/15 | - | - |  +375 |  | - |
| 1001 | special | `~D, DF, F, x` | 2 | 104 (21) + 104 (21) | 9/9, 15/15 | - | - |  +375 |  | - |
| 1002 | special | `~D, DF, F, x` | 2 | 132 (28) + 132 (28) | 9/9, 15/15 | - | - |  +375 |  | - |
| 1100 | special | `~B, D, DB, a` | 2 | expr (229) + expr (132) | 19/19, 19/19 | - | - |  +500 |  | - |
| 1200 | special | `~BD, D, F, UF, a` | 3 | expr (76) + 28 (14) + expr (62) | 15/15, 15/15, 15/15 | - | - |  +438 |  | - |
| 3000 | super | `~D, DF, F, D, DF, F, x` | 5 | 118 (28) + 111 (28) + 111 (28) + 111 (28) + 111 (28) | 15/15, 15/15, 15/15 | 22 / 45 / 21 | -50 / -50 | -3000 |  | - |

### SSFDeejay

Life 1000, attack 94, defence 100, power 3000, air juggle 15; walk 2.9 / -1.9, run 4.6, jump -8.0. Files: `chars/ssf2x_deejay/deejay.ACT`, `chars/ssf2x_deejay/deejay2.ACT`, `chars/ssf2x_deejay/sound.snd`, `chars/ssf2x_deejay/ssfdeejay.air`, `chars/ssf2x_deejay/ssfdeejay.cmd`, `chars/ssf2x_deejay/ssfdeejay.cns`, `chars/ssf2x_deejay/ssfdeejay.def`, `chars/ssf2x_deejay/ssfdeejay.sff`, `data/common1.cns`.

| state | kind | command | hits | damage (guard) | stun hit / guard | startup / active / recovery | adv. hit / guard | power | cancels | measured |
|---|---|---|---|---|---|---|---|---|---|---|
| 200 | normal | `x` | 1 | 23 | 13/13 | 5 / 4 / 4 | +6 / +6 |  +20 |  | 4 hits, median 22 |
| 210 | normal | `y` | 1 | 47 | 17/17 | 5 / 9 / 1 | +8 / +8 |  +45 |  | - |
| 220 | normal | `z` | 1 | 80 | 22/22 | 7 / 9 / 8 | +6 / +6 |  +65 |  | - |
| 221 | normal | `z` | 2 | 45 + 45 | 20/20, 20/20 | 7 / 10 / 5 | +6 / +6 |  +65 |  | 4 hits, median 42 |
| 230 | normal | `a` | 1 | 26 | 13/13 | 5 / 4 / 3 | +7 / +7 |  +20 |  | 2 hits, median 24 |
| 231 | normal | `a` | 1 | 39 | 14/14 | 4 / 4 / 1 | +10 / +10 |  +20 |  | - |
| 240 | normal | `b` | 1 | 35 | 16/16 | 7 / 11 / 7 | -1 / -1 |  +45 |  | 4 hits, median 33 |
| 241 | normal | `b` | 1 | 75 | 12/12 | 8 / 5 / 1 | +7 / +7 |  +45 |  | 1 hits, median 71 |
| 250 | normal | `c` | 1 | 86 | 23/23 | 10 / 9 / 9 | +6 / +6 |  +65 |  | 5 hits, median 81 |
| 251 | normal | `c` | 1 | 96 | 23/23 | 4 / 19 / 0 | +5 / +5 |  +65 |  | 1 hits, median 90 |
| 400 | normal | `x` | 1 | 23 | 12/12 | 3 / 3 / 1 | +9 / +9 |  +20 |  | - |
| 410 | normal | `y` | 1 | 47 | 14/14 | 6 / 6 / 4 | +5 / +5 |  +45 |  | - |
| 420 | normal | `z` | 1 | 87 | 20/20 | 5 / 12 / 6 | +3 / +3 |  +65 |  | - |
| 430 | normal | `a` | 1 | 28 | 12/12 | 5 / 4 / 5 | +4 / +4 |  +20 |  | - |
| 440 | normal | `b` | 1 | 52 | 15/15 | 7 / 6 / 6 | +4 / +4 |  +45 |  | 1 hits, median 49 |
| 450 | normal | `c` | 1 | 75 | 15/15 | 9 / 6 / 15 | -5 / -5 |  +65 |  | - |
| 600 | normal | `x` | 1 | 20 | 15/15 | 3 / 1 / -1 | +16 / +16 |  +22 |  | 1 hits, median 19 |
| 610 | normal | `y` | 1 | 62 | 17/17 | 4 / 18 / 0 | +0 / +0 |  +65 |  | - |
| 611 | normal | `y` | 1 | 52 | 17/17 | 8 / 14 / 0 | +4 / +4 |  +65 |  | - |
| 620 | normal | `z` | 1 | 85 | 23/23 | 12 / 8 / 0 | +16 / +16 |  +65 |  | 1 hits, median 80 |
| 621 | normal | `z` | 1 | 82 | 17/17 | 7 / 13 / 0 | +5 / +5 |  +65 |  | - |
| 630 | normal | `a` | 1 | 26 | 15/15 | 3 / 1 / -1 | +16 / +16 |  +20 |  | - |
| 631 | normal | `a` | 1 | 36 | 6/6 | 3 / 1 / -1 | +7 / +7 |  +20 |  | 1 hits, median 34 |
| 632 | normal | `a` | 1 | 36 | 5/5 | 3 / 1 / -1 | +6 / +6 |  +20 |  | - |
| 640 | normal | `b` | 1 | 60 | 17/17 | 7 / 14 / 0 | +4 / +4 |  +65 |  | - |
| 641 | normal | `b` | 1 | 65 | 17/17 | 7 / 14 / 9 | -5 / -5 |  +65 |  | 1 hits, median 61 |
| 650 | normal | `c` | 1 | 79 | 18/18 | 8 / 10 / 6 | +3 / +3 |  +65 |  | - |
| 651 | normal | `c` | 1 | 80 | 20/20 | 7 / 12 / 4 | +5 / +5 |  +65 |  | - |
| 1000 | special | `~D,DF,F,x` | 1 | 50 [p] | 20/20 | None / - / - | - |  +60 |  | 1 hits, median 47 |
| 1200 | special | `~D,DB,B, a` | 2 | 50 + 50 | 26/26, 20/20 | None / - / - | - |  +65 |  | - |
| 1300 | throw | `c` | 1 | 0 | 0/0 | 1 / 2 / 0 | -1 / -1 |  |  | - |
| 1400 | special | `~F,D,DF,a` | 1 | expr (50) | 20/20 | - | - |  +60 |  | 1 hits, median 28 |
| 1500 | throw | `z` | 1 | 0 | 0/0 | 1 / 2 / 0 | -1 / -1 |  |  | - |
| 1600 | special | `~F,D,DF,x` | 2 | 12 + 12 | 20/20, 20/20 | None / - / - | - |  |  | - |
| 2000 | super | `~D,DB,B,D,DB,B, a` | 1 | 60 | 26/26 | None / - / - | - | -1000 |  | - |
| 2100 | super | `~D,DF,F,D,DF,F, a` | 1 | expr (50) | 20/20 | 6 / 59 / 3 | -41 / -41 | -1000 |  | - |

### T.HAWK

Life 1120, attack 100, defence 100, power 3000, air juggle 15; walk 2.4 / -1.9, run 5.0, jump -8.0. Files: `chars/SSF2X_THAWK/CODE/-2Thawk.cns`, `chars/SSF2X_THAWK/CODE/MOVESThawk.cns`, `chars/SSF2X_THAWK/CODE/THAWK.sff`, `chars/SSF2X_THAWK/CODE/THAWK.snd`, `chars/SSF2X_THAWK/CODE/ThrowThawk.cns`, `chars/SSF2X_THAWK/CODE/animationThawk.air`, `chars/SSF2X_THAWK/CODE/commandThawk.cmd`, `chars/SSF2X_THAWK/CODE/commonThawk.cns`, `chars/SSF2X_THAWK/CODE/constantThawk.cns`, `chars/SSF2X_THAWK/CODE/statesThawk.cns`, `chars/SSF2X_THAWK/SSF2X_THAWK.def`, `chars/SSF2X_THAWK/act/THawk_AMARELLA.act`, `chars/SSF2X_THAWK/act/THawk_BLACK.act`, `chars/SSF2X_THAWK/act/THawk_BROWN.act`, `chars/SSF2X_THAWK/act/THawk_GOLD.act`, `chars/SSF2X_THAWK/act/THawk_GREEN.act`, `chars/SSF2X_THAWK/act/THawk_NewGREEN.act`, `chars/SSF2X_THAWK/act/THawk_ORANGE.act`, `chars/SSF2X_THAWK/act/THawk_ORIGINAL.act`, `chars/SSF2X_THAWK/act/THawk_PINK.act`, `chars/SSF2X_THAWK/act/THawk_PURPLE.act`, `chars/SSF2X_THAWK/act/THawk_TURQUESE.act`, `chars/SSF2X_THAWK/act/THawk_WHITE.act`.

| state | kind | command | hits | damage (guard) | stun hit / guard | startup / active / recovery | adv. hit / guard | power | cancels | measured |
|---|---|---|---|---|---|---|---|---|---|---|
| 200 | normal | `x` | 1 | 28 | 12/19 | 4 / 4 / 4 | +5 / +12 |  |  | - |
| 201 | normal | `x` | 1 | 28 | 12/19 | 4 / 4 / 4 | +5 / +12 |  |  | - |
| 230 | normal | `a` | 1 | 112 | 12/19 | 5 / 5 / 4 | +4 / +11 |  |  | - |
| 231 | normal | `a` | 1 | 112 | 12/19 | 4 / 4 / 4 | +5 / +12 |  |  | 1 hits, median 112 |
| 240 | normal | `b` | 1 | 147 | 16/24 | 9 / 4 / 7 | +6 / +14 |  |  | 1 hits, median 147 |
| 241 | normal | `b` | 1 | 147 | 16/24 | 4 / 8 / 7 | +2 / +10 |  |  | 1 hits, median 147 |
| 300 | normal | `x` | 1 | 28 | 12/19 | 4 / 4 / 4 | +5 / +12 |  |  | - |
| 310 | normal | `y` | 1 | 147 | 16/24 | 7 / 8 / 7 | +2 / +10 |  |  | 1 hits, median 147 |
| 320 | normal | `z` | 2 | 182 + 119 | 20/29, 20/29 | 9 / 14 / 10 | -3 / +6 |  |  | - |
| 330 | normal | `a` | 1 | 28 | 12/19 | 4 / 4 / 5 | +4 / +11 |  |  | - |
| 340 | normal | `b` | 1 | 147 | 16/24 | 7 / 6 / 5 | +6 / +14 |  |  | 1 hits, median 147 |
| 350 | normal | `c` | 2 | 168 + 154 | 20/29, 20/29 | 8 / 22 / 21 | -22 / -13 |  |  | 5 hits, median 168 |
| 400 | normal | `x` | 1 | 119 | 12/19 | 4 / 1 / -1 | +13 / +20 |  |  | 3 hits, median 119 |
| 410 | normal | `y` | 1 | 147 | 12/19 | 6 / 14 / 0 | -1 / +6 |  |  | - |
| 420 | normal | `z` | 1 | 168 | 12/19 | 8 / 12 / 5 | -4 / +3 |  |  | - |
| 421 | normal | `z` | 1 | 168 | 12/19 | 6 / 13 / 0 | +0 / +7 |  |  | - |
| 422 | normal | `/$D` | 1 | 182 | 12/19 | 8 / 1 / -1 | +13 / +20 |  |  | - |
| 430 | normal | `a` | 1 | 119 | 12/19 | - | - |  |  | - |
| 440 | normal | `b` | 1 | 147 | 12/19 | - | - |  |  | - |
| 450 | normal | `c` | 1 | 168 | 12/19 | - | - |  |  | - |
| 1000 | special | `F, ~D, DF, x` | 2 | 189 + 140 | 14/19, 14/19 | 5 / 12 / 0 | +3 / +8 |  |  | 1 hits, median 189 |
| 1010 | special | `F, ~D, DF, y` | 2 | 189 + 77 | 14/19, 14/19 | 5 / 12 / 0 | +3 / +8 |  |  | - |
| 1020 | special | `F, ~D, DF, z` | 3 | 189 + 84 + 84 | 20/29, 14/19, 14/19 | 5 / 12 / 0 | +9 / +18 |  |  | 1 hits, median 27 |
| 1100 | special | `x+y+z` | 1 | 189 | 12/19 | 18 / 4 / 0 | +9 / +16 |  | 1101 | 1 hits, median 189 |
| 1200 | throw | `~F, D, B, U, x` | 1 | 0 | 0/0 | 1 / 1 / 0 | +0 / +0 |  |  | - |
| 1201 | throw | `~F, D, B, U, x` | 1 | 0 | 0/0 | 1 / 1 / 0 | +0 / +0 |  |  | - |
| 1210 | throw | `~F, D, B, U, F, y` | 1 | 0 | 0/0 | 1 / 1 / 0 | +0 / +0 |  |  | - |
| 1211 | throw | `~F, D, B, U, F, y` | 1 | 0 | 0/0 | 1 / 1 / 0 | +0 / +0 |  |  | - |
| 1220 | throw | `~F, D, B, U, z` | 1 | 0 | 0/0 | 1 / 1 / 0 | +0 / +0 |  |  | 1 hits, median 147 |
| 1221 | throw | `~F, D, B, U, z` | 1 | 0 | 0/0 | 1 / 1 / 0 | +0 / +0 |  |  | - |
| 1230 | super | `~F, D, B, U, F, D, B, U,` | 1 | 0 | 0/0 | 1 / 1 / 0 | +0 / +0 | -3000 |  | - |
| 1231 | super | `~F, D, B, U, F, D, B, U,` | 1 | 0 | 0/0 | 1 / 1 / 0 | +0 / +0 | -3000 |  | - |

### Vega

Life 1000, attack 100, defence 100, power 1000, air juggle 20; walk 3.5 / -3.5, run 6.0, jump -10.0. Files: `chars/ssf2x_vega/animation.air`, `chars/ssf2x_vega/command.cmd`, `chars/ssf2x_vega/common.cns`, `chars/ssf2x_vega/override.cns`, `chars/ssf2x_vega/sound.snd`, `chars/ssf2x_vega/sprite2.sff`, `chars/ssf2x_vega/states.cns`, `chars/ssf2x_vega/vega-sf2x.def`, `chars/ssf2x_vega/winquote.cns`, `data/common1.cns`.

| state | kind | command | hits | damage (guard) | stun hit / guard | startup / active / recovery | adv. hit / guard | power | cancels | measured |
|---|---|---|---|---|---|---|---|---|---|---|
| 200 | normal | `x` | 1 | expr (55) | 12/12 | 3 / 4 / 5 | +4 / +4 |  |  | 2 hits, median 55 |
| 210 | normal | `y` | 1 | expr (80) | 14/14 | 10 / 6 / 10 | -1 / -1 |  |  | 7 hits, median 80 |
| 220 | normal | `z` | 1 | expr (105) | 16/16 | 9 / 9 / 13 | -5 / -5 |  |  | 8 hits, median 105 |
| 230 | normal | `a` | 1 | 50 | 12/12 | 5 / 4 / 5 | +4 / +4 |  |  | - |
| 240 | normal | `b` | 1 | 75 | 14/14 | 6 / 6 / 10 | -1 / -1 |  |  | 4 hits, median 75 |
| 250 | normal | `c` | 1 | 100 | 16/16 | 7 / 8 / 12 | -3 / -3 |  |  | 3 hits, median 100 |
| 300 | normal | `x` | 1 | expr (55) | 12/12 | 4 / 4 / 5 | +4 / +4 |  |  | - |
| 310 | normal | `y` | 1 | expr (80) | 14/14 | 5 / 4 / 7 | +4 / +4 |  |  | - |
| 320 | normal | `z` | 1 | expr (105) | 16/16 | 7 / 6 / 19 | -8 / -8 |  |  | - |
| 400 | normal | `x` | 1 | expr (55) | 12/12 | 4 / 4 / 3 | +6 / +6 |  |  | - |
| 410 | normal | `y` | 1 | expr (80) | 14/14 | 4 / 4 / 9 | +2 / +2 |  |  | - |
| 420 | normal | `z` | 1 | expr (105) | 16/16 | 9 / 8 / 12 | -3 / -3 |  |  | - |
| 430 | normal | `a` | 1 | 50 | 12/12 | 4 / 4 / 4 | +5 / +5 |  |  | - |
| 440 | normal | `b` | 1 | 75 | 14/14 | 4 / 4 / 6 | +5 / +5 |  |  | 1 hits, median 75 |
| 450 | normal | `c` | 3 | 100 + 50 (10) + expr (125) | 16/16, 18/18, 18/18 | 5 / 17 / 13 | -13 / -13 |  |  | 6 hits, median 100 |
| 500 | normal | `x` | 1 | expr (55) | 12/12 | 4 / 4 / 3 | +6 / +6 |  |  | - |
| 510 | normal | `y` | 1 | expr (80) | 14/14 | 5 / 4 / 6 | +5 / +5 |  |  | - |
| 520 | normal | `z` | 1 | expr (105) | 16/16 | 7 / 6 / 18 | -7 / -7 |  |  | - |
| 530 | normal | `a` | 1 | 50 | 12/12 | 4 / 4 / 4 | +5 / +5 |  |  | - |
| 540 | normal | `b` | 1 | 75 | 14/14 | 4 / 4 / 6 | +5 / +5 |  |  | - |
| 550 | normal | `c` | 4 | 100 + 50 (10) + expr (125) + 100 | 16/16, 18/18, 18/18 | 5 / 21 / 13 | -17 / -17 |  |  | - |
| 600 | normal | `x` | 1 | expr (55) | 12/12 | 2 / 30 / 7 | -24 / -24 |  |  | - |
| 610 | normal | `y` | 1 | expr (80) | 14/14 | 4 / 14 / 8 | -7 / -7 |  |  | 1 hits, median 80 |
| 620 | normal | `z` | 1 | expr (105) | 16/16 | 6 / 10 / 9 | -2 / -2 |  |  | - |
| 630 | normal | `a` | 1 | 50 | 12/12 | 3 / 20 / 4 | -11 / -11 |  |  | 2 hits, median 50 |
| 640 | normal | `b` | 1 | 75 | 14/14 | 3 / 10 / 4 | +1 / +1 |  |  | 4 hits, median 75 |
| 650 | normal | `c` | 1 | 100 | 16/16 | 6 / 10 / 10 | -3 / -3 |  |  | 1 hits, median 100 |
| 730 | normal | `x` | 1 | 50 | 12/12 | 3 / 20 / 4 | -11 / -11 |  |  | - |
| 740 | normal | `y` | 1 | 75 | 14/14 | 3 / 10 / 4 | +1 / +1 |  |  | - |
| 750 | normal | `z` | 1 | 100 | 16/16 | 5 / 5 / 4 | +8 / +8 |  |  | - |
| 800 | throw | `y` | 4 | 0 + 0 + expr (55) + expr (55) | 0/0, 0/0, 12/12 | - | - |  |  | - |
| 810 | throw | `y` | 2 | 0 + expr (55) | 0/0, 12/12 | - | - |  |  | - |
| 990 | normal | `` | 3 | 50 (10) + expr (125) + 100 | 18/18, 18/18, 16/16 | 1 / 25 / 0 | -6 / -6 |  |  | - |
| 1000 | special | `~50$B, F, x` | 3 | 50 (10) + expr (125) + 100 | 18/18, 18/18, 16/16 | 12 / 10 / 2 | +7 / +7 |  |  | 10 hits, median 87.5 |
| 1130 | special | `` | 5 | expr (130) + 0 + 0 + expr (150) + expr (150) | 18/18, 0/0, 0/0 | 1 / 2 / 0 | +17 / +17 |  | 1250 | - |
| 1210 | special | `~50$D, $U, x` | 5 | expr (130) + 0 + 0 + expr (150) + expr (150) | 18/18, 0/0, 0/0 | 1 / 2 / 0 | +17 / +17 |  | 1250 | - |
| 1500 | special | `~50$DB, F, a` | 1 | 125 (10) | 18/18 | 3 / 14 / 15 | -10 / -10 |  |  | 23 hits, median 125 |
| 3000 | super | `~50$B, $F, $B, $F, x` | 2 | expr (110) + expr (300) | 16/16, 16/16 | 10 / 6 / 10 | +1 / +1 | -1000 |  | - |
| 3500 | super | `~50$B, $F, $B, $F, a` | 6 | 50 (10) + 50 (10) + 50 (10) + 75 (10) + 75 (10) + 75 (10) | 18/18, 18/18, 18/18 | None / - / - | - | -1000 |  | - |

### gouki

Life 926, attack 100, defence 100, power 3000, air juggle 15; walk 3.43 / -2.27, run 4.6, jump -9.37. Files: `chars/ssf2x_gouki/AI.txt`, `chars/ssf2x_gouki/act/gouki_1.act`, `chars/ssf2x_gouki/act/gouki_12.act`, `chars/ssf2x_gouki/act/gouki_2.act`, `chars/ssf2x_gouki/act/gouki_3.act`, `chars/ssf2x_gouki/act/gouki_4.act`, `chars/ssf2x_gouki/act/gouki_5.act`, `chars/ssf2x_gouki/act/gouki_6.act`, `chars/ssf2x_gouki/act/gouki_7.act`, `chars/ssf2x_gouki/gouki.air`, `chars/ssf2x_gouki/gouki.cmd`, `chars/ssf2x_gouki/gouki.cns`, `chars/ssf2x_gouki/gouki.sff`, `chars/ssf2x_gouki/gouki.snd`, `chars/ssf2x_gouki/option.txt`, `chars/ssf2x_gouki/ssf2x_gouki.def`, `data/common1.cns`.

| state | kind | command | hits | damage (guard) | stun hit / guard | startup / active / recovery | adv. hit / guard | power | cancels | measured |
|---|---|---|---|---|---|---|---|---|---|---|
| 200 | normal | `x` | 2 | expr + 35 | 9/9, 10/10 | - | - |  |  | - |
| 205 | normal | `x` | 1 | expr | 9/9 | 3 / 3 / 4 | +3 / +3 |  |  | - |
| 210 | normal | `y` | 3 | expr + 90 + 90 | 13/13, 14/14, 14/14 | - | - |  |  | - |
| 215 | normal | `y` | 1 | expr | 12/12 | 5 / 2 / 10 | +1 / +1 |  |  | - |
| 220 | normal | `z` | 2 | expr + 139 | 17/17, 17/17 | 6 / 4 / 15 | -1 / -1 |  |  | - |
| 225 | normal | `z` | 2 | expr + expr | 15/15, 15/15 | 4 / 6 / 16 | -6 / -6 |  |  | 1 hits, median 167 |
| 250 | normal | `a` | 2 | expr + 35 | 9/9, 9/9 | 7 / 3 / 4 | +3 / +3 |  |  | 1 hits, median 83 |
| 255 | normal | `a` | 1 | expr | 8/8 | 5 / 4 / 3 | +2 / +2 |  |  | 3 hits, median 90 |
| 260 | normal | `b` | 3 | expr + 83 + 62 | 13/13, 14/14, 14/14 | - | - |  |  | - |
| 265 | normal | `b` | 1 | expr | 12/12 | 4 / 5 / 7 | +1 / +1 |  |  | - |
| 270 | normal | `c` | 3 | expr + expr + 111 | 15/15, 15/15, 17/17 | - | - |  |  | - |
| 275 | normal | `c` | 2 | expr + expr | 16/16, 16/16 | 6 / 9 / 7 | +1 / +1 |  |  | - |
| 300 | normal | `/$D` | 2 | expr + 35 | 8/8, 10/10 | - | - |  |  | - |
| 305 | normal | `/$D` | 2 | expr + 90 | 10/10, 13/13 | - | - |  |  | - |
| 310 | normal | `/$D` | 4 | expr + expr + 111 + 111 | 15/15, 15/15, 17/17 | - | - |  |  | - |
| 320 | normal | `/$D` | 2 | expr + 35 | 7/7, 10/10 | - | - |  |  | 14 hits, median 35 |
| 325 | normal | `/$D` | 2 | expr + 83 | 12/12, 13/13 | - | - |  |  | 11 hits, median 118 |
| 330 | normal | `/$D` | 2 | expr + 111 | 19/19, 19/19 | - | - |  |  | 2 hits, median 194 |
| 350 | normal | `x` | 2 | expr + 49 | 8/8, 9/9 | - | - |  |  | - |
| 355 | normal | `y` | 2 | expr + 83 | 9/9, 10/10 | - | - |  |  | - |
| 360 | normal | `z` | 2 | expr + 111 | 8/8, 10/10 | - | - |  |  | - |
| 365 | normal | `a` | 3 | 0 + expr + 62 | 0/0, 8/8, 10/10 | - | - |  |  | - |
| 370 | normal | `b` | 3 | 0 + expr + 83 | 0/0, 9/9, 11/11 | - | - |  |  | - |
| 375 | normal | `c` | 4 | 0 + expr + expr + 111 | 0/0, 8/8, 8/8 | - | - |  |  | - |
| 400 | normal | `a` | 3 | 0 + expr + 56 | 0/0, 8/8, 9/9 | - | - |  |  | - |
| 405 | normal | `b` | 3 | 0 + expr + 69 | 0/0, 9/9, 11/11 | - | - |  |  | - |
| 410 | normal | `c` | 3 | 0 + expr + 90 | 0/0, 8/8, 10/10 | - | - |  |  | - |
| 450 | normal | `b` | 1 | 69 | 14/14 | 5 / 1 / -1 | +15 / +15 |  +17 |  | - |
| 470 | normal | `/$F` | 1 | 40 | 10/10 | 17 / 1 / -1 | +11 / +11 |  +21 |  | - |
| 900 | throw | `/$F` | 2 | 0 + 0 | 0/0, 0/0 | - | - |  |  | - |
| 901 | throw | `/$F` | 19 | 0 + 0 + expr + expr + 90 + 90 ... | 0/0, 0/0, 12/12 | - | - |  |  | - |
| 1200 | special | `~D, DF, F, x` | 4 | expr [p] + expr [p] + 97 (14) [p] + 97 (14) [p] | 19/19, 19/19, 16/16 | - | - |  |  | 27 hits, median 125 |
| 1300 | special | `~F, DF, D, DB, B, x` | 3 | expr [p] + expr (56) [p] + expr (83) [p] | 16/16, 20/20, 16/16 | - | - |  |  | 9 hits, median 56 |
| 1400 | special | `~D, DF, F, x` | 4 | expr [p] + 62 (14) [p] + expr [p] + 62 (14) [p] | 20/20, 13/13, 20/20 | - | - |  |  | - |
| 1700 | special | `~F, D, DF, a` | 5 | 0 + 84 (8) + 84 (8) + 70 + 77 | 0/0, 14/14, 18/18 | None / - / - | - |  +70 |  | - |
| 3200 | super | `~x, x, F, a, z` | 1 | 0 | 0/0 | - | - | -3000 |  | - |
| 3300 | super | `~D, DF, F, D, DF, x` | 1 | expr (65) [p] | 13/13 | - | - | -1000 |  | - |

