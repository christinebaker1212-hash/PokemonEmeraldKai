# Moemon VMU pet

The Dreamcast VMU virtual pet for Moemon Emerald Kai: one MOéMON raised from an Egg, sent to Kai through a VM2.
The user's decisions: `docs/VMU_PET_DECISIONS.md`.

## Layout

- `linktest/`: LINK TEST, the first program: two VMU programs talking over the top connector, saving what they get.
- `inc/`: shared assembly: `sfr.inc` (registers), `header.inc` (Sega's GHEAD: vectors and BIOS calls), `lcd.inc`
  (the screen, a 3x5 font), `sio.inc` (VMU-to-VMU serial, every byte echoed).
- `tools/`: `vmufs.py` (VMU images, the VM2's format: make / ls / get / vmi), `gen_font.py`, `find_linktest.py`
  (finds LINK TEST's saves on an SD card), `pmd_read.py` + `onebit.py` (PMD Collab sprites to 1 bit),
  `get_tools.sh` (the assembler and emulator at pinned commits into `.tools/`, never committed).
- `test/`: `vmutest` (two VMUs in libevmu joined by a cable, run by a script), `scripts/*.txt`, `run.sh`.
- `prebuilt/`: the programs as built, for putting on a VM2 without building.

## Building and testing

```
vmu_pet/tools/get_tools.sh      # once: waterbear (Rust) and libevmu (C)
make -C vmu_pet                 # build/LINKTEST.VMS + .VMI, build/linktest.bin (a whole VMU image)
make -C vmu_pet test            # the emulator tests
```

Sources: Sega's VMU hardware manual and tutorial (dreamcast.wiki File:VMU.pdf: serial VMD-100..113 and 229,
samples VMT-72..88, flash VMT "Reading and Writing Flash Memory"), libevmu's headers for the file system.

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

What to report: whether step 2 worked, which speeds said OK both ways, any error words (NO ANSWER, ECHO WRONG,
NO ACK, LOST IT, BAD DATA), and what step 4 printed.
