	.include "asm/macros/bit_width.inc"

	.include "MPlayDef.s"

/* Moemon Quetzal song 560, disassembled from the ROM by tools/quetzal_rip/songs.py.
   "org" comments give the ROM address of the bytes that follow. */

	.equ	mus_qz_560_grp, voicegroup_bard
	.equ	mus_qz_560_pri, 4
	.equ	mus_qz_560_rev, 0
	.equ	mus_qz_560_mvl, 127
	.equ	mus_qz_560_key, 0
	.equ	mus_qz_560_tbs, 1
	.equ	mus_qz_560_exg, 1
	.equ	mus_qz_560_cmp, 1

	.section .rodata
	.global	mus_qz_560
	.p2align	2
/* org 0x09669414 */

/********************** Track  1 **********************/

mus_qz_560_1:
	.byte		KEYSH , mus_qz_560_key+0
	.byte		TEMPO , 120*mus_qz_560_tbs/2
	.byte		VOICE , 1
	.byte		VOL   , 117*mus_qz_560_mvl/mxv
	.byte		TIE   , Cn3 , v127
	.byte	W96
mus_qz_560_1_B1:
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_560_1_B1
	.byte	W48
	.byte		EOT   , Cn3
	.byte	FINE

/******************************************************/
	.p2align	2

mus_qz_560:
	.byte	1	/* NumTrks */
	.byte	0	/* NumBlks */
	.byte	mus_qz_560_pri	/* Priority */
	.byte	mus_qz_560_rev	/* Reverb. */
space64 4
	ptrvalue	mus_qz_560_grp

	ptrvalue	mus_qz_560_1

	.end
