# The VM2's USB protocol

How DreamMods' VM2 talks to a PC over USB-C, learned by reading their VM2 GUI 1.9.9.1 (a .NET program; the user
asked for this, 2026-11-21, for interoperability: nothing of DreamMods' is copied into this project or shipped). The
user's VM2 stays on its current firmware (the newest found online: 1.5.5), so this only has to match it. Our side:
`vmu_pet/manager/vm2link.cpp` (the VMU manager).

## The port

- A **CH340** USB-serial chip (VID 0x1A86): Windows names it "USB-SERIAL CH340 (COMn)" (the GUI's Utilities tab
  installs the CH340 driver); Linux /dev/ttyUSBn.
- 8N1, no flow control, **460800 baud**, or 921600 when 460800 gets no answer.
- The VM2 must be in **GUI-mode** (Settings > System > GUI-mode, or plugging the cable in on its home screen since
  firmware 1.5.4). Otherwise it only streams its screen (below) and answers nothing.

## Commands

A command is 7 ASCII characters, `@` + 6, sometimes followed by bytes or `\r`-terminated paths. Answers are text lines
ending in `\n` (the GUI reads them with ReadLine and checks with Contains, so a `\r` before the `\n` is harmless;
`NACK` contains `ACK`: test for NACK first). The GUI waits 500 ms for an answer, 5 s for SD card commands.

| Command | Then send | The VM2 answers |
|---|---|---|
| `@HAND00` | | `ACK`: GUI-mode is on (the GUI sends it before every operation) |
| `@INFO00` | | one line `<word> <HW revision> <FW version> <device ID>` (space-separated) |
| `@WRTC00` | 10 bytes: second, minute, hour, day of week (0 = Sunday), day, month, year - 1980, 0, 0, 0 | `ACK` |
| `@REST00` | | restarts |
| `@RMEM1` + block (1 byte, 0-255) | | 512 bytes: that block of the **internal** 128 KB memory |
| `@WMEM1` + block (1 byte) | the 512 bytes, then 1 byte: all 512 XORed together | `ACK` (the GUI retries the block until ACK) |

### The SD card (the "uSD Explorer" in the GUI)

Every SD command first answers `ACK`, or `NOSD` when no card is in. Paths use backslashes: a folder is `\NAME`
(listing a folder sends `NAME`, the root `\`), a file `\NAME.BIN` at the root or `\FOLDER\NAME.BIN`; names are
8.3, uppercase, no spaces.

| Command | Then send | The VM2 answers |
|---|---|---|
| `@SDEXPL` | path + `\r` | `>START_DIR`, then a line per entry, then `>END_DIR` |
| `@SDFLRD` | file path + `\r` | 256 blocks of 512 bytes, the PC sending `O` after each block; then `ACK` / `NACK`. Always 128 KB: a smaller file is the first `size` bytes |
| `@SDFLWR` | path + `\r` (in a folder the GUI sends `FOLDER\NAME.BIN`, no leading `\`) | `>START`; then per block the PC sends 512 bytes and the VM2 answers `>NXTBLK`, 256 times; then `ACK` / `NACK`. Only whole 128 KB images |
| `@SDFCPY` | source path `\r`, destination path `\r` | `ACK` / `NACK` |
| `@SDRENM` | old path `\r`, new path `\r` (files and folders) | `ACK` / `NACK` |
| `@SDFDEL` | file path `\r` | `ACK` / `NACK` |
| `@SDDCRT` | folder path `\r` | `ACK` / `NACK` |
| `@SDDDEL` | folder path `\r` | `ACK` / `NACK` |
| `@SDFTTL` | `FOLDER\title.txt` + `\r` | `>START`; the PC sends 128 bytes (the title, ASCII, zero-padded); `ACK` |

A listing line is `name|size|date|isdir|title|dreameye`: `isdir` is `0` for a file, `1` for a folder; a folder's
`title` is its title.txt (the game's name), empty when none; files of 131072 bytes are VMU images, 98370 bytes
DreamEye photos (`dreameye` = `Yes`). `GIDnnn` folders are the per-game VMUs the VM2 makes itself (nnn = the game's
number in its list); the GUI writes a title.txt into those that have none.

## The screen stream

Outside GUI-mode (or with a controller in), the VM2 sends its screen: `>LCD` (normal colours) or `<LCD` (inverted),
then 192 bytes, the 48x32 picture a bit a pixel.
