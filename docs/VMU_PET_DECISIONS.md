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

## Tools

- waterbear (assembler, Rust) and libevmu (the ElysianVMU core, C, MIT) for headless tests. libevmu defines the serial
  registers but doesn't emulate the link, so a two-unit link test needs a bridge of our own.

## Open questions

See the conversation of 2026-11-21: species, what "full maturity" means, what the MOéMON brings into Kai, where it
lands, how the pet's care works.
