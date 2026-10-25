	.include "asm/macros/bit_width.inc"

	.include "MPlayDef.s"

/* Moemon Quetzal song 603, disassembled from the ROM by tools/quetzal_rip/songs.py.
   "org" comments give the ROM address of the bytes that follow. */

	.equ	mus_qz_603_grp, voicegroup_bard
	.equ	mus_qz_603_pri, 4
	.equ	mus_qz_603_rev, 0
	.equ	mus_qz_603_mvl, 127
	.equ	mus_qz_603_key, 0
	.equ	mus_qz_603_tbs, 1
	.equ	mus_qz_603_exg, 1
	.equ	mus_qz_603_cmp, 1

	.section .rodata
	.global	mus_qz_603
	.p2align	2
/* org 0x09669940 */

/********************** Track  1 **********************/

mus_qz_603_1:
	.byte		KEYSH , mus_qz_603_key+0
	.byte		TEMPO , 120*mus_qz_603_tbs/2
	.byte		VOICE , 44
	.byte		VOL   , 127*mus_qz_603_mvl/mxv
	.byte		N54   , Cn3 , v127
	.byte	W54
	.byte	FINE

/******************************************************/
	.p2align	2

mus_qz_603:
	.byte	1	/* NumTrks */
	.byte	0	/* NumBlks */
	.byte	mus_qz_603_pri	/* Priority */
	.byte	mus_qz_603_rev	/* Reverb. */
space64 4
	ptrvalue	mus_qz_603_grp

	ptrvalue	mus_qz_603_1

	.end
