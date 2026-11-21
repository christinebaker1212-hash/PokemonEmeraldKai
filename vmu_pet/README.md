# Moemon VMU pet

The Dreamcast VMU virtual pet for Moemon Emerald Kai: one MOéMON raised from an Egg, sent to Kai through a VM2.
The user's decisions: `docs/VMU_PET_DECISIONS.md`.

## Layout

- `pet/`: the pet, KANTO edition (first draft, 2026-11-21): `pet.s`, `icon.inc` (its colour file icon); its data
  from Kai's tables and PMD sprites (`tools/gen_edition.py kanto`: 151 MOeMON, 73 hatchable, 1-bit 24x24 sprites, an
  Egg for each: `tools/eggs.py`). Screens: `docs/vmu_pet/pet_screens.png`.
- PICTURES (`make pictures`: `tools/fetch_portraits.sh`, `tools/gen_icons.py`): the 151 PMD Normal portraits at 32x32
  in 4 colours, a data file of 81 blocks beside the pet; with it a hatched MOeMON's file icon is its portrait.
  `build/pet_pictures.bin` is a VMU image with both.
- `linktest/`: LINK TEST, the first program: two VMU programs talking over the top connector, saving what they get.
- `inc/`: shared assembly: `sfr.inc` (registers), `header.inc` (Sega's GHEAD: vectors and BIOS calls), `lcd.inc`
  (the screen, a 3x5 font), `sio.inc` (VMU-to-VMU serial, every byte echoed).
- `tools/`: `vmufs.py` (VMU images, the VM2's format: make / ls / get / vmi), `gen_font.py`, `find_linktest.py`
  (finds LINK TEST's saves on an SD card), `pmd_read.py` + `onebit.py` (PMD Collab sprites to 1 bit),
  `get_tools.sh` (the assembler and emulator at pinned commits into `.tools/`, never committed).
- `test/`: `vmutest` (two VMUs in libevmu joined by a cable, run by a script), `scripts/*.txt`, `run.sh`.
- `manager/`: the VMU manager (README.txt there): VMU images on the VM2 (USB: docs/VM2_USB_PROTOCOL.md), SD cards,
  folders and files; `make -C vmu_pet/manager` / `windows` / `test`.
- `prebuilt/`: the programs as built, for putting on a VM2 without building.

## Building and testing

```
vmu_pet/tools/get_tools.sh      # once: waterbear (Rust) and libevmu (C)
make -C vmu_pet                 # build/LINKTEST.VMS + .VMI, build/linktest.bin (a whole VMU image)
make -C vmu_pet test            # the emulator tests
```

Sources: Sega's VMU hardware manual and tutorial (dreamcast.wiki File:VMU.pdf: serial VMD-100..113 and 229,
samples VMT-72..88, flash VMT "Reading and Writing Flash Memory"), libevmu's headers for the file system.

## The pet (first draft)

An Egg (its pattern by the MOeMON inside) hatches 30 minutes after it appears. A taps the pet for SPARKS; B is the
menu: FEED (MEAL / VITAMIN), TRAIN (three rounds: A while the cursor is over ##), CLEAN, MEDICINE, STATUS (four
pages), SHOP (TAP UP, HELPER, RARE CANDY, IV UP, SHINY, MASTER, ASTRAL), LINK (SEND a level-50 MOeMON to a VM2 or
another VMU, RECEIVE one in place of an Egg), ABOUT. UP / DOWN on the pet opens STATUS. MODE leaves (it saves).

- Care by the day (from 4 AM): 4 meals, 3 trainings, no poop left; a missed one is a care mistake; poop left makes it
  sick. All done: 10 EXP. About a week of care to level 50 (8 EXP a level).
- Animations: it hops, steps and turns to face the way it went; eats a meal in three bites; jumps with a heart when
  happy (a meal, a vitamin, a clean-up, medicine, good training, hatching); shakes its head with a sweat drop when not
  (full already, a bad training); the Egg wobbles before hatching.
- Evolutions at their levels; stones and friendship at 30, trades at 36; EEVEE at 25 by care (no mistakes VAPOREON,
  trained more than fed FLAREON, else JOLTEON).
- Helpers earn SPARKS each ten minutes, also while it's closed (12 hours at most). Sending one on resets the helpers
  and TAP UP and adds a SPARK a tap for good.
- The save page (in the program's own file): "MOEMONVMUPET", format 1, edition 1, then RAM $40-$7b (the MOeMON:
  $40-$67, what LINK sends; the owner: $68-$7b). Kai reads it there.
- Drawn only on RC, VCCR written whole, the button interrupt only around HALT (it's level-triggered), buttons sampled
  while drawing (`lcd_hook`), the BIOS's idle count reset on a press (it sleeps after ~2 minutes without).
- The file icon (`icon.inc`): rewritten when the stage or MOeMON changes (the header's $25e / $25f say what it
  shows): an Egg's design in its colours, the portrait from PICTURES, or the LCD sprite. `tools/icon_check.py IMAGE`
  checks a saved unit's icon pixel for pixel (run.sh does it for every `OUT/icon_*.bin`).
- Tests: `test/scripts/pet_icon.txt` (the Egg's icon, then the portrait's; skipped without `make pictures`),
  `test/scripts/pet_life.txt` (hatch, meals, full, clean, training, status, a day judged, shop, helpers),
  `pet_link.txt` (to a VM2), `pet_reload.txt` (a saved unit started again). `tools/screen_text.py` reads a shot's text;
  `tools/shots_sheet.py` makes a sheet; `tools/fix_branches.py` lengthens branches that can't reach.

## Testing LINK TEST on hardware (a VMU and a VM2)

1. **Put it on the VM2**: import `prebuilt/LINKTEST.VMS` + `.VMI` with the VM2 GUI (or VMU Explorer), or put
   `prebuilt/linktest.bin` on the SD card as a VMU image.
2. **Copy it to the VMU without a Dreamcast**: join the VM2 and the VMU top to top and copy LINKTEST from the VM2
   in the VMU's file menu. (Does this work? That's check 3.)
3. **Run LINK TEST on both**, joined. Each should say CABLE OK. Press A on the VMU: it says SENDING..., then
   SENT OK! and the VM2 says GOT IT OK! (speed 1 takes about 15 seconds). Then A on the VM2 the other way.
   Try SPEED 4 (UP three times, on both): about 2 seconds a send.
4. **Find the save**: put the VM2's SD card in the PC and run `python3 vmu_pet/tools/find_linktest.py <SD card>`.
   The image whose GOT went up is where a program on the VM2 saves.

5. **The raw test** (LINK TEST 2): B sends one byte and nothing else. The other side shows `RAW A0 001` (the byte
   in hex, how many have come); the sender shows `B>A0 BACK xx` (what went, and what its port held afterwards).

What to report: which speeds said OK both ways; the exact words on both screens when it fails (`NO ANSWER`,
`ECHO 4D>-- ` with `AT BYTE n` under it, `NO ACK`, `LOST IT`, `BAD DATA`); whether B's raw byte shows up on the other
side (and the `BACK` value); and what step 4 printed.

LINK TEST 4 (2026-11-21) starts at speed 8 and writes VCCR whole (`mov`, never `set1` / `clr1`: it's write-only, and
bit instructions on it set its other bits; LINK TEST 2 / 3's screen stayed broken by that). The emulator harness now
enforces what the user's hardware showed: XRAM's missing bytes, VCCR's rules, drawing only on RC, and `vm2 UNIT`
(a VM2's receiver garbles bytes clocked slower than 1 ms a bit). `tools/where.py` finds an address's source line.

Speeds (LINK TEST 3): 1-4 run the link on the 32 kHz crystal (Sega's advice; XT on the screen), 5-8 on the RC clock
(about 25 times faster; RC). Both units at the same speed.

Hardware notes (learned on the user's VMU, 2026-11-21): LINK TEST 2 on a VMU and a VM2: bytes from the VM2 reached
the VMU whole (the VM2's BACK showed its previous byte: the VMU's port handing it back), bytes from the VMU reached the
VM2 garbled and with extra bytes (crystal speed 1); the RC speeds are for that. The screen still broke up: the clear
wrote XRAM's unimplemented bytes (low digit C-F of every 16; Sega's simulator manual), fixed in LINK TEST 3. the screen broke up at the bottom on a real VMU (not on the
VM2 or the emulator) because LINK TEST 1 switched clocks without Sega's VCCR bit 6 rule (clk_fast / clk_slow).
