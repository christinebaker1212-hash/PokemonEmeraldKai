	.include "asm/macros/bit_width.inc"

	.include "MPlayDef.s"

/* Moemon Quetzal song 564, disassembled from the ROM by tools/quetzal_rip/songs.py.
   "org" comments give the ROM address of the bytes that follow. */

	.equ	mus_qz_564_grp, voicegroup_bard
	.equ	mus_qz_564_pri, 4
	.equ	mus_qz_564_rev, 0
	.equ	mus_qz_564_mvl, 127
	.equ	mus_qz_564_key, 0
	.equ	mus_qz_564_tbs, 1
	.equ	mus_qz_564_exg, 1
	.equ	mus_qz_564_cmp, 1

	.section .rodata
	.global	mus_qz_564
	.p2align	2
/* org 0x09669494 */

/********************** Track  1 **********************/

mus_qz_564_1:
	.byte		KEYSH , mus_qz_564_key+0
	.byte		TEMPO , 120*mus_qz_564_tbs/2
	.byte		VOICE , 5
	.byte		VOL   , 127*mus_qz_564_mvl/mxv
	.byte		N54   , Cn3 , v127
	.byte	W54
	.byte	FINE

/******************************************************/
	.p2align	2

mus_qz_564:
	.byte	1	/* NumTrks */
	.byte	0	/* NumBlks */
	.byte	mus_qz_564_pri	/* Priority */
	.byte	mus_qz_564_rev	/* Reverb. */
space64 4
	ptrvalue	mus_qz_564_grp

	ptrvalue	mus_qz_564_1

	.end
