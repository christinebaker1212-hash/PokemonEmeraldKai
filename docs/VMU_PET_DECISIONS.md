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
- **One week of daily care reaches level 50**, faster with steps.
- **Steps are a currency**: spent on faster levels, better IVs, turning it shiny, rare items (MASTER BALL, ASTRAL BALL)
  that come into Kai with it. How steps reach the VMU is open: the VMU has no step sensor.

## Sprites (determined 2026-11-21)

- **One frame a species**: PMD's Idle, facing down, frame 1 (tools/vmu_pet/pmd_read.py reads Kai's own
  graphics/moemon/pmd/pmd.bin). Movement comes from shared moves (bob, hop, turn by mirroring); eating, sleeping, poop,
  ZZZ, food are drawn once for all.
- **24x24, 1 bit** (tools/vmu_pet/onebit.py: the outline where it meets transparency, dark colours filled; bigger ones
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
