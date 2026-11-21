MOEMON VMU MANAGER

Your Dreamcast VMUs, all in one place: the VM2's SD card (by USB, or in a card reader), the VM2's own
memory, and VMU files on this PC.

  - Open a VMU: from the VM2 (connect it by USB, left), an SD card or folder, or a .bin / .vmu file.
  - See its files as cards, and its 200 blocks as a map.
  - Drag a file onto another VMU's tab to copy it there.
  - Drop .VMS / .VMI / .DCI files onto the window to put them on the open VMU; .bin / .vmu to open them.
  - Edit names, descriptions, icons (each save's colour icon, and the VMU's own 1-bit and colour icons).
  - Changes stay in the window until you save. Anything overwritten is copied into the "backups" folder
    next to this program first.
  - Light or dark (top right; it starts out as Windows is set). Folders you open are remembered
    (settings.ini).

THE VM2 BY USB
  1. On the VM2: Settings > System > GUI-mode.
  2. Plug it in with its USB-C cable. (The first time, DreamMods' VM2 GUI can install the USB driver:
     Utilities > Install USB Driver.)
  3. Close DreamMods' VM2 GUI if it's open (only one program can talk to the VM2 at a time).
  4. Connect to the VM2.

  Once connected, the manager copies every VMU off the VM2 in the background (about 3 seconds each over
  USB), into the "cache" folder next to it: after that, opening one is instant. An opened VMU is checked
  against the VM2 right away, and shows the VM2's copy if it changed (unless you've changed it yourself).

vm2cli.exe does the same from a command prompt (vm2cli ports / info / ls / get / put ...).

The font is Liberation Sans (SIL Open Font License, FONT_LICENSE.txt). SDL2 (zlib licence).
