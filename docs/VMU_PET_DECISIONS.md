# The VMU pet: the user's decisions

A virtual pet for the Dreamcast VMU that raises one MOéMON from an Egg and sends it to Moemon Emerald Kai.
Started 2026-11-20. Nothing is built yet; graphics come from the user.

## Decided (the user, 2026-11-20 / 21)

- **The target is the VMU** (a standalone VMU minigame: a `.vms` GAME file, LC86K87 assembly).
- **One MOéMON, raised from an Egg.** The Egg comes from the VMU program itself, never from Kai.
- **Full maturity unlocks the transfer.** Before it, the transfer option isn't offered.
- **Transfers go through a VM2** (DreamMods' SD-card VMU). An original VMU can't reach the PC.
- **On the VMU:** with the VM2 plugged into the VMU's top while the program runs on both, the VMU asks
  "OK TO TRANSFER TO VM2?". YES copies the MOéMON to the VM2, then resets the VMU, **which starts a new Egg**.
- **On the PC: the SD card** (not the VM2's USB link, whose serial protocol is unpublished and needs DreamMods'
  Windows GUI). The player puts the VM2's microSD in the PC, runs Kai, and on the title menu of the save they want picks
  "TRANSFER MOEMON FROM VMU".
- The user's VM2 runs the latest firmware (the newest found online: 1.5.5, August 2025; GUI 1.9.9.1).
- **Any MOéMON that can hatch from an Egg can hatch** (2026-11-21). In Kai's tables 874 of the 1025 species are in a
  breedable egg group, plus the babies: about 900 species whose art the VMU must hold, every stage of each line.
- **Full maturity is level 50.**
- **Care is Digimon-style** (V-Pet: hunger and strength, training, battles, care mistakes, sleep, sickness...).
- **No Dreamcast** (the user has one but would rather not use it): the program reaches the VM2 from the SD card, and an
  original VMU from the VM2 by the VMU's own file transfer (top to top).

- **Sprites from the PMD Collab** (https://sprites.pmdcollab.org/), as few as possible, converted to 1 bit
  (2026-11-21). **The Normal portrait is the VMU icon.**
- **What comes into Kai:** the species it reached, level 50, nature / Ability / gender / IVs rolled at hatching, a
  1/4096 shiny chance, its four newest level-up moves, the save's trainer as OT; party, or a PC box when the party is
  full. **Good care benefits it.**
- **Care decides** the evolutions that need more than a level (stones, trades, friendship, branches like EEVEE's).
- **It's never lost**: no death, no running away.
- **Battles** both on the VMU and VMU to VMU, **only when the player starts one**.
- **The main goal (the user's own words, 2026-11-21):** the person it's for works where electronics aren't allowed and
  can't check the VMU regularly or answer care in time. **A day's care (by the VMU's clock) must fit in a 5-minute
  window**, a restroom break.

- **Region editions** (2026-11-21): one program per region, each hatching only its own Pokémon, like versions:
  **every region, KANTO through PALDEA**, each with its game's own regional POKéDEX (KANTO = FireRed's 151, JOHTO =
  Crystal's, HOENN = Emerald's, SINNOH = Platinum's, UNOVA = Black's...), evolutions only within the edition.
  This replaces the one 900-species program: an edition is about 130-200 species, so it takes far fewer blocks and the
  VMU icon can be the pet's own Normal portrait (proposed: the program rewrites its own file icon).
- **KANTO is the first edition.**
- **One week of daily care reaches level 50**, faster with the currency below.
- **No steps** (2026-11-21: the VMU has no step sensor; a pedometer was ruled out), and not time the program runs
  either: **the currency comes from a Cookie Clicker-like game inside** (the user's idea; the rest are Claude's
  recommendations, which the user took whole, 2026-11-21):
  - **The pet is the cookie**: A taps it (it hops, reacts), each tap earns a little; upgrades raise a tap's worth.
  - **Helpers** (BERRY trees, a DAY-CARE helper...) earn over time **with the program closed too**, worked out from the
    clock on opening, **up to 12 hours' worth** since the last visit. The program never needs to run all day (batteries).
  - Good care makes the helpers earn more. Mashing never beats upgrades (the VMU's buttons are small).
  - The currency is **SPARKS**.
  - **Prices**: a workday's earnings buy a level or an IV raise, a week's the shiny, two weeks' a MASTER BALL, a month's
    an ASTRAL BALL; items come into Kai's BAG with the MOéMON.
  - **Per pet**: helpers and upgrades reset with each new Egg; each MOéMON sent to the VM2 leaves a small permanent
    bonus (a clicker's prestige).
- **Every Egg is equally likely** among the edition's hatchable species.
- **A day starts at 4 AM** (care late at night counts for that day).
- **The link test comes first** (VMU-to-VM2, minigame to minigame), then the screen mock-ups.

## Sprites (determined 2026-11-21)

- **One frame a species**: PMD's Idle, facing down, frame 1 (vmu_pet/tools/pmd_read.py reads Kai's own
  graphics/moemon/pmd/pmd.bin). Movement comes from shared moves (bob, hop, turn by mirroring); eating, sleeping, poop,
  ZZZ, food are drawn once for all.
- **24x24, 1 bit** (vmu_pet/tools/onebit.py: the outline where it meets transparency, dark colours filled; bigger ones
  scaled down to fit). The median Idle frame is 25 px; 16x16 lost too much (docs/vmu_pet/sprites_16.png against
  sprites_24.png; on the screen: docs/vmu_pet/screen_mock.png).
- All 974 species Kai has PMD sprites for take about 53 KB at 24x24, so the program spills past 64 KB into the second
  flash bank: **about 160-170 of the VMU's 200 blocks; the VMU is the pet's alone.**
- 51 species have no PMD sprite in Kai's import (newer collab sprites may exist for some): list them and fill them
  before building.

## Care: built for 5-minute windows (proposed)

- **Care is judged by the day, never the minute.** Nothing calls, nothing beeps, nothing goes wrong because the VMU
  wasn't checked in time. A day (by the VMU's clock) asks for: feeding until full, a few training rounds, cleaning up,
  medicine if sick. Doing them any time that day counts.
- **The whole day's care in about 2 minutes**, leaving room in 5: the screen opens on what today still needs; each
  task is one or two presses, a training round about 10 seconds.
- **A missed day is a care mistake**, never a loss: it weighs on the bonuses in Kai and on evolutions.
- **Silent by default**; the VMU sleeps when not in use (batteries); a reset or backwards clock counts as no time
  passing, never as missed days.

## Space (from the hardware)

- Code runs from flash bank 0 (64 KB); LDF reads all 128 KB (the bank bit at SFR 0x154). A minigame starts at block 0
  and is contiguous, so data past 64 KB can live in blocks 128-199 of the same GAME file.
- About 900 species: a 16x16 1-bit frame is 32 bytes (about 29 KB a frame for all of them), 24x24 is 72 bytes (about
  64 KB). Two 16x16 frames each, or one 24x24, fits only if the program takes nearly the whole VMU (about 190 blocks,
  no other saves beside it). Stats, types and evolutions for 900 species take about 10 KB more; names about 7 KB.

- **The PC side** (2026-11-21): ideally the VM2 plugged into the PC by USB, every valid MOéMON VMU file listed, the
  player picks which to bring in, and that file / save is deleted in the process. The VM2's USB protocol is
  unpublished (DreamMods' Windows GUI only), so for now **our own program reads the VM2's SD card directly and handles
  the transfer**. USB later if DreamMods shares the protocol, or by recording the GUI's traffic.
- **A VMU manager of our own** (the user, 2026-11-21): every VMU image on an SD card listed, browsed and edited; icons
  uploaded and edited (both the colour icon and the VMU's 1-bit one); files extracted and loaded back; and a
  connection to Moemon Kai while it runs (the transfer). After the link test.
- **Environments from Chao Adventure 2's sprite sheet** may be used (the user, 2026-11-21:
  https://chao-island.com/guides/sonic-adventure-2/chao-adventure-2/sprite-sheet.html), backgrounds only.

- **The Eggs** (the user, 2026-11-21): docs/user_images/96.png is the base and style guide: about 18x20, three or four
  colours, outlined in a dark shade of the Egg's own colour (no black), a pattern on each (flames, bands, roses,
  stars). **An Egg looks like the MOéMON inside**: its colours from the species, its pattern from a small library chosen
  by the species (type and look). Mock-ups before building. (95.png, a yellow spotted Egg, came first and was replaced.)

- **The game talks to the VM2 itself** (the user, 2026-11-21, once its USB protocol was known): no SD card swapping
  for Moemon Kai; the manager is for managing VMUs in general (any game's).
- **Kai gives Eggs too** (2026-11-21): both ways. Kai drops an Egg off on the VM2 (a real one: its species, moves,
  parents, from Kai's boxes) to raise on the VMU, and takes grown MOéMON back. The VMU still makes its own Egg when it
  has none. (This widens "the Egg comes from the VMU program".)
- **Where** (the user's words, 2026-11-21): "A PC option that is enabled ONLY if the VMU is connected. It then opens a PC
  box to grab and drop the Egg from, and place a new egg into if you have one available. After completion, it will
  disconnect the VM2 via software." So: a VM2 option on the PC's menu, there only while a VM2 is plugged in (and in
  GUI-mode); it opens a box-like screen with the VM2 as a box: take what's on it (a grown MOéMON, or an Egg) into
  Kai's boxes, put an Egg from Kai's boxes onto it; leaving ends the connection by software (proposed: @REST00, the
  VM2 restarts out of GUI-mode, ready to use). This replaces the title menu's TRANSFER MOEMON FROM VMU. Mock-ups first.

## The pet's first draft (2026-11-21)

The user: "fix it on your own, and give me a first draft of the actual program", then "make certain there are basic
animations for the Pokemon too, as well as at least one positive and negative reaction". `vmu_pet/pet/` (README
"The pet"): KANTO edition, Egg to level 50, Digimon-style daily care, SPARKS clicker + helpers + shop, evolutions,
LINK, animations (hop / step / turn, eating, happy with a heart, unhappy with a sweat drop, the Egg's wobble).
Fixed on the way, without the hardware: the emulator harness now enforces what the user's VMU showed (XRAM's missing
bytes, VCCR's rules, drawing only on RC, a VM2's receiver garbling slow bytes); LINK TEST 4 and the pet default to the
fast RC link. Not in this draft: battles, Kai's side (reading the save page, giving Eggs); the Eggs and the colour icon came
next (below). Screens:
docs/vmu_pet/pet_screens.png.

## Eggs for each MOéMON and the colour file icon (2026-11-21)

The user: "let's continue battles, color file icon, species specific eggs, and Kai's side".
- **Eggs** (`vmu_pet/tools/eggs.py`): the guide's own 19x20 silhouette (docs/user_images/96.png, leaning as drawn),
  one of 8 patterns by the first type (FLAMES fire / dragon, WAVES water / ice, ROSES grass / bug, STARS psychic /
  fairy, BANDS ground / rock / fighting / steel, SPOTS normal, ZIGZAG electric / flying, BLOTCHES poison / ghost /
  dark), in one of 2 phases and maybe mirrored by the second type and number (32 looks on the LCD), and in colour the
  MOéMON's own: the shell and pattern from its PMD sprite's two main colours, the outline a dark shade of the
  pattern's. The LCD draws the outline and the pattern; the Egg turns as it rocks before hatching.
- **The colour file icon** (`vmu_pet/pet/icon.inc`): the pet redraws its own header's icon when its stage or
  MOéMON changes: an Egg in its design and colours; a hatched MOéMON as its PMD Normal portrait. The 151 portraits
  are 40 KB even at 4 colours, and a VMU program can be 64 KB at most (Sega's manual), so they're a data file of
  their own, PICTURES (`tools/gen_icons.py`, 81 blocks; the VMU's file menu copies data files, VM2 to VMU too):
  the pet finds it by its mark wherever its blocks are. Without it, a hatched MOéMON's icon is its LCD sprite in its
  Egg's outline colour. Checked pixel for pixel in the emulator (`tools/icon_check.py`, test pet_icon).

## Battles (built 2026-11-21)

`vmu_pet/pet/battle.inc` (screens: docs/vmu_pet/battle_screens.png). Only when the player starts one; nothing is ever
lost (a loss only means no prize). The two face each other (ours on the left), 3 HP each, taking turns. A hit comes
from power against power (level + today's STRENGTH x 2 + IVs / 16, +3 with no care mistakes, so the day's training
helps) and type against type (the full type chart): super effective hits for 2, not very effective less often.
- **WILD** (MENU > BATTLE): a MOéMON of the edition within 2 levels; on our turns a marker runs along
  [....##....] and A on ## is a sure hit. Prizes: SPARKS (the foe's level x 4) and 2 EXP for each of the first 3 wins
  of a day (so battles help without breaking the week to level 50).
- **LINK** (LINK > BATTLE, two units top to top): both wait, A on either calls; they swap a card (edition, MOéMON,
  level, power, a seed) and both play out the same battle from the shared seed, each with its own MOéMON on the left.
  The caller attacks first. Counted in the MOéMON's record (battles, wins: bytes $62-$64, which go to Kai with it).
- Tests: pet_battle (WILD), pet_battle_link (two units: each counts one battle, one wins).

## Kai's side: mock-ups for review (2026-11-21)

`docs/vmu_pet/kai_vm2_mockups.png` (drawn over the game's own PC menu and box screen with its own font and pictures:
`vmu_pet/tools/kai_mockups.py`, `gbafont.py`; the base shots in docs/vmu_pet/kai_src/). Proposed, waiting on the user:
1. The PC menu gets **VM2** above LOG OFF; with no VM2 answering (plugged in, GUI-mode) it's greyed and does nothing.
2. Choosing it reads the VM2 ("Connected to the VM2. It holds a MOéMON EGG.").
3. The box screen opens at a **VM2 box** (a VMU drawn as its wallpaper, one place: its screen); L / R go on to the
   player's own boxes, so the EGG or the grown MOéMON can be dropped anywhere, and an EGG of the edition put on it.
4. CLOSE BOX writes what changed back to the VM2 and restarts it out of GUI-mode ("The VM2 was disconnected.
   It's safe to unplug it.").
Built already, not yet used: the link itself in the game (src/platform/vm2.c, include/vm2.h; test
tools/test/vm2/vm2_test.sh against a pretend VM2).
Open: an EGG from the VMU comes in as Kai's EGG of its MOéMON (its IVs, personality kept); a grown one as Lv50 with
its IVs, nature, shiny, POKé BALL, the MASTER / ASTRAL BALLS bought into the BAG; "good care benefits it in Kai":
proposed friendship 255 less 10 a care mistake (70 at least) and EVs from its trainings. An EGG given from Kai must be
of the edition's hatchable MOéMON (KANTO: 73).

## Sending the pet to a VMU without a Dreamcast (the user's idea, 2026-11-21)

The VMU's own file menu won't copy a GAME (only saves), so the first copy needs a Dreamcast once. After that, the
pet on the VMU can take **updates** from the VM2 (the user: "the VMU a slave program to the VM2, the main transfer
program"): LINK > UPDATE on both, the VM2's newer pet sends its own program over the top connector, the VMU writes it
into its own file page by page (fm_wrt_ex) and checks it. To be safe: the receiving code sits at a fixed place at
the end of the file that an update never rewrites (a cut cable leaves a VMU that can try again), and the file is
made bigger than the pet needs at the first install (a GAME's size can't change later). To decide: how many blocks
to keep (the pet is 56 now; PICTURES 81 beside it).

## The VMU manager (built 2026-11-21)

`vmu_pet/manager/` (README.txt; Dear ImGui on SDL2, Windows and Linux): VMU images on the VM2's SD card over USB
(the VM2's protocol: docs/VM2_USB_PROTOCOL.md, learned from DreamMods' GUI at the user's request), in a card reader
or folder, single .bin / .vmu files, and the VM2's internal memory. Files as cards with their animated icons, the 200
blocks as a map, an inspector (names, descriptions, icons: each save's colour icon, ICONDATA_VMS's 1-bit and colour
ones, PNG in and out), drag a file onto another VMU to copy it, drop .VMS / .VMI / .DCI / images on the window,
.VMS + .VMI and .DCI out. Changes stay in the window until Save; whatever is overwritten is copied into backups/
first. The user found the first version "INCREDIBLY ugly": redone the same day in a light, warm theme, larger
type, cards, plain words. `vm2cli` does the VM2's commands from a prompt. Tests: `make -C vmu_pet/manager test`
(the VMU code's self test; every VM2 command against test/fake_vm2.py on a pseudo-terminal). Next: Moemon Kai's
side (bringing a pet in), once the pet exists.

## Built so far (2026-11-21)

- `vmu_pet/` (README): LINK TEST, shared assembly (Sega's GHEAD, the screen and a 3x5 font, VMU-to-VMU serial with
  every byte echoed, flash saving through the BIOS), vmufs.py (VMU images and .VMI), the two-VMU emulator harness
  (libevmu, its serial link done by the harness), `make test`. LINK TEST passes in the emulator both ways at speeds 1
  and 4; the hardware test is in the README.
- Learned: the 32 kHz crystal runs about 2,700 instructions a second (a line of text took a second to draw), so the
  programs draw on the RC clock and use the crystal for HALT and the serial link (Sega's advice).

## How it works (proposed, to confirm as it's built)

- **The VMU-to-VM2 link:** the same program runs on both units. The receiving side (the VM2) waits for the sender;
  the sender wipes its MOéMON only after the receiver confirms that it wrote the pet and the checksum matched,
  so a cable pulled midway loses nothing.
- **No duplicates:** each pet has a random transfer ID; a Kai save keeps the IDs it took in and refuses a repeat.
  Kai also clears the pet from the VM2's image on the SD card after taking it.
- **Finding the pet on the SD card:** the VM2 keeps full 128 KB VMU images (`.bin` / `.vmu`, per game in
  `GIDxxx/GAME.VMU`). Kai looks through every image on the card for the pet's save file in the VMU directory, so it
  doesn't depend on which image the VM2 had active.

## Hardware checks only the user's VM2 can answer

1. The VM2's product page lists "VM-to-VM connection for game sharing"; whether two minigames can talk over it (as
   Chao Adventure did) is unconfirmed. A small link-test program comes before the pet.
2. Where a minigame running on the VM2 saves: its internal 128 KB or the active SD image, and when that reaches the
   card.
3. That the VM2 can send the program to an original VMU by the VMU's file transfer (no Dreamcast), at its full size.

## Tools

- waterbear (assembler, Rust) and libevmu (the ElysianVMU core, C, MIT) for headless tests. libevmu defines the serial
  registers but doesn't emulate the link, so a two-unit link test needs a bridge of our own.

## Open questions

See the conversation of 2026-11-21: species, what "full maturity" means, what the MOéMON brings into Kai, where it
lands, how the pet's care works.
