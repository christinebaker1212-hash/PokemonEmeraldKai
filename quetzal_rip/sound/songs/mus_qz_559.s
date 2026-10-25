	.include "asm/macros/bit_width.inc"

	.include "MPlayDef.s"

/* Moemon Quetzal song 559, disassembled from the ROM by tools/quetzal_rip/songs.py.
   "org" comments give the ROM address of the bytes that follow. */

	.equ	mus_qz_559_grp, voicegroup_bard
	.equ	mus_qz_559_pri, 4
	.equ	mus_qz_559_rev, 0
	.equ	mus_qz_559_mvl, 127
	.equ	mus_qz_559_key, 0
	.equ	mus_qz_559_tbs, 1
	.equ	mus_qz_559_exg, 1
	.equ	mus_qz_559_cmp, 1

	.section .rodata
	.global	mus_qz_559
	.p2align	2
/* org 0x096693F8 */

/********************** Track  1 **********************/

mus_qz_559_1:
	.byte		KEYSH , mus_qz_559_key+0
	.byte		TEMPO , 120*mus_qz_559_tbs/2
	.byte		VOICE , 0
	.byte		VOL   , 127*mus_qz_559_mvl/mxv
	.byte		N44   , Cn3 , v127
	.byte	W44
	.byte	W03
	.byte	FINE

/******************************************************/
	.p2align	2

mus_qz_559:
	.byte	1	/* NumTrks */
	.byte	0	/* NumBlks */
	.byte	mus_qz_559_pri	/* Priority */
	.byte	mus_qz_559_rev	/* Reverb. */
space64 4
	ptrvalue	mus_qz_559_grp

	ptrvalue	mus_qz_559_1

	.end
