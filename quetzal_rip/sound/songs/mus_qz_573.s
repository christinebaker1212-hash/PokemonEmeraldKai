	.include "asm/macros/bit_width.inc"

	.include "MPlayDef.s"

/* Moemon Quetzal song 573, disassembled from the ROM by tools/quetzal_rip/songs.py.
   "org" comments give the ROM address of the bytes that follow. */

	.equ	mus_qz_573_grp, voicegroup_bard
	.equ	mus_qz_573_pri, 4
	.equ	mus_qz_573_rev, 0
	.equ	mus_qz_573_mvl, 127
	.equ	mus_qz_573_key, 0
	.equ	mus_qz_573_tbs, 1
	.equ	mus_qz_573_exg, 1
	.equ	mus_qz_573_cmp, 1

	.section .rodata
	.global	mus_qz_573
	.p2align	2
/* org 0x096695A8 */

/********************** Track  1 **********************/

mus_qz_573_1:
	.byte		KEYSH , mus_qz_573_key+0
	.byte		TEMPO , 120*mus_qz_573_tbs/2
	.byte		VOICE , 14
	.byte		VOL   , 127*mus_qz_573_mvl/mxv
	.byte		N54   , Cn3 , v127
	.byte	W54
	.byte	FINE

/******************************************************/
	.p2align	2

mus_qz_573:
	.byte	1	/* NumTrks */
	.byte	0	/* NumBlks */
	.byte	mus_qz_573_pri	/* Priority */
	.byte	mus_qz_573_rev	/* Reverb. */
space64 4
	ptrvalue	mus_qz_573_grp

	ptrvalue	mus_qz_573_1

	.end
