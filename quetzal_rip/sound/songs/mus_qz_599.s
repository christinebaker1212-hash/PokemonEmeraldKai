	.include "asm/macros/bit_width.inc"

	.include "MPlayDef.s"

/* Moemon Quetzal song 599, disassembled from the ROM by tools/quetzal_rip/songs.py.
   "org" comments give the ROM address of the bytes that follow. */

	.equ	mus_qz_599_grp, voicegroup_bard
	.equ	mus_qz_599_pri, 4
	.equ	mus_qz_599_rev, 0
	.equ	mus_qz_599_mvl, 127
	.equ	mus_qz_599_key, 0
	.equ	mus_qz_599_tbs, 1
	.equ	mus_qz_599_exg, 1
	.equ	mus_qz_599_cmp, 1

	.section .rodata
	.global	mus_qz_599
	.p2align	2
/* org 0x096698C0 */

/********************** Track  1 **********************/

mus_qz_599_1:
	.byte		KEYSH , mus_qz_599_key+0
	.byte		TEMPO , 120*mus_qz_599_tbs/2
	.byte		VOICE , 40
	.byte		VOL   , 127*mus_qz_599_mvl/mxv
	.byte		TIE   , Cn3 , v127
	.byte	W96
mus_qz_599_1_B1:
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_599_1_B1
	.byte	W48
	.byte		EOT   , Cn3
	.byte	FINE

/******************************************************/
	.p2align	2

mus_qz_599:
	.byte	1	/* NumTrks */
	.byte	0	/* NumBlks */
	.byte	mus_qz_599_pri	/* Priority */
	.byte	mus_qz_599_rev	/* Reverb. */
space64 4
	ptrvalue	mus_qz_599_grp

	ptrvalue	mus_qz_599_1

	.end
