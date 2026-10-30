# Third-party code in Moemon Music Studio

The studio plays music ripped from other consoles in emulators of those consoles. Their code is here,
with small changes marked `Moemon Music Studio` (capture hooks, portability). Because some of it is
GPL, **the studio as a whole is distributed under the GNU GPL version 2 or later**; the studio's own
code (`src/`) may also be used under the same terms as the rest of this repository.

| folder | what | from | license |
|---|---|---|---|
| `he/` | Highly Experimental: PlayStation 1/2 sound emulation (IOP, SPU/SPU2), and mkhebios, which makes its BIOS from the player's own PS2 BIOS at run time | Neill Corlett; github.com/kode54/Highly_Experimental | released as source by its author (no license file) |
| `psflib/` | PSF / PSF2 file loading, `_lib` chains, the PSF2 file system | Christopher Snowhill (kode54), github.com/kode54/psflib | MIT (`psflib/LICENSE`) |
| `lazyusf2/` | Nintendo 64 emulation for USF (Mupen64Plus-based) | Christopher Snowhill; github.com/derselbst/lazyusf2 (mirror) | GPL 2 (Mupen64Plus) |
| `desmume_2sf/` | Nintendo DS emulation for 2SF (DeSmuME 0.9.9 as modified by vio2sf / in_2sf) | DeSmuME team, Naram Qashat (in_xsf, github.com/CyberBotX/in_xsf) | GPL 2+ (DeSmuME); in_xsf: `desmume_2sf/IN_XSF_LICENSE` |
| `vbam_gsf/` | Game Boy Advance emulation for GSF (VBA-M as cut down by in_gsf) | VBA-M team, Naram Qashat | GPL 2+ (`vbam_gsf/gpl.txt`) |
| `gme_gbs/` | Game Boy emulation for GBS (Game_Music_Emu's GBS player) | Shay Green (blargg); github.com/libgme/game-music-emu | LGPL 2.1+ (`gme_gbs/license.txt`) |
| `zlib/` | zlib's inflate (PSF files are compressed) | Jean-loup Gailly, Mark Adler | zlib (`zlib/LICENSE`) |
| `compat/` | a helper the in_xsf cores expect | | |

Nothing here contains a BIOS: PlayStation music needs the player's own PlayStation 2 BIOS
(Display > BIOS folder in the studio).
