	.include "asm/macros/bit_width.inc"

	.include "MPlayDef.s"

/* Moemon Quetzal song 590, disassembled from the ROM by tools/quetzal_rip/songs.py.
   "org" comments give the ROM address of the bytes that follow. */

	.equ	mus_qz_590_grp, voicegroup_bard
	.equ	mus_qz_590_pri, 4
	.equ	mus_qz_590_rev, 0
	.equ	mus_qz_590_mvl, 127
	.equ	mus_qz_590_key, 0
	.equ	mus_qz_590_tbs, 1
	.equ	mus_qz_590_exg, 1
	.equ	mus_qz_590_cmp, 1

	.section .rodata
	.global	mus_qz_590
	.p2align	2
/* org 0x096697AC */

/********************** Track  1 **********************/

mus_qz_590_1:
	.byte		KEYSH , mus_qz_590_key+0
	.byte		TEMPO , 120*mus_qz_590_tbs/2
	.byte		VOICE , 31
	.byte		VOL   , 127*mus_qz_590_mvl/mxv
	.byte		TIE   , Cn3 , v127
	.byte	W96
mus_qz_590_1_B1:
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_590_1_B1
	.byte	W48
	.byte		EOT   , Cn3
	.byte	FINE

/******************************************************/
	.p2align	2

mus_qz_590:
	.byte	1	/* NumTrks */
	.byte	0	/* NumBlks */
	.byte	mus_qz_590_pri	/* Priority */
	.byte	mus_qz_590_rev	/* Reverb. */
space64 4
	ptrvalue	mus_qz_590_grp

	ptrvalue	mus_qz_590_1

	.end
