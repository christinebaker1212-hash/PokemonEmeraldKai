	.include "asm/macros/bit_width.inc"

	.include "MPlayDef.s"

/* Moemon Quetzal song 592, disassembled from the ROM by tools/quetzal_rip/songs.py.
   "org" comments give the ROM address of the bytes that follow. */

	.equ	mus_qz_592_grp, voicegroup_bard
	.equ	mus_qz_592_pri, 4
	.equ	mus_qz_592_rev, 0
	.equ	mus_qz_592_mvl, 127
	.equ	mus_qz_592_key, 0
	.equ	mus_qz_592_tbs, 1
	.equ	mus_qz_592_exg, 1
	.equ	mus_qz_592_cmp, 1

	.section .rodata
	.global	mus_qz_592
	.p2align	2
/* org 0x096697EC */

/********************** Track  1 **********************/

mus_qz_592_1:
	.byte		KEYSH , mus_qz_592_key+0
	.byte		TEMPO , 120*mus_qz_592_tbs/2
	.byte		VOICE , 33
	.byte		VOL   , 127*mus_qz_592_mvl/mxv
	.byte		N54   , Cn3 , v127
	.byte	W54
	.byte	FINE

/******************************************************/
	.p2align	2

mus_qz_592:
	.byte	1	/* NumTrks */
	.byte	0	/* NumBlks */
	.byte	mus_qz_592_pri	/* Priority */
	.byte	mus_qz_592_rev	/* Reverb. */
space64 4
	ptrvalue	mus_qz_592_grp

	ptrvalue	mus_qz_592_1

	.end
