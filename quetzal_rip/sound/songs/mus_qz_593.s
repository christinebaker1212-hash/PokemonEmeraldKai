	.include "asm/macros/bit_width.inc"

	.include "MPlayDef.s"

/* Moemon Quetzal song 593, disassembled from the ROM by tools/quetzal_rip/songs.py.
   "org" comments give the ROM address of the bytes that follow. */

	.equ	mus_qz_593_grp, voicegroup_bard
	.equ	mus_qz_593_pri, 4
	.equ	mus_qz_593_rev, 0
	.equ	mus_qz_593_mvl, 127
	.equ	mus_qz_593_key, 0
	.equ	mus_qz_593_tbs, 1
	.equ	mus_qz_593_exg, 1
	.equ	mus_qz_593_cmp, 1

	.section .rodata
	.global	mus_qz_593
	.p2align	2
/* org 0x09669808 */

/********************** Track  1 **********************/

mus_qz_593_1:
	.byte		KEYSH , mus_qz_593_key+0
	.byte		TEMPO , 120*mus_qz_593_tbs/2
	.byte		VOICE , 34
	.byte		VOL   , 127*mus_qz_593_mvl/mxv
	.byte		TIE   , Cn3 , v127
	.byte	W96
mus_qz_593_1_B1:
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_593_1_B1
	.byte	W48
	.byte		EOT   , Cn3
	.byte	FINE

/******************************************************/
	.p2align	2

mus_qz_593:
	.byte	1	/* NumTrks */
	.byte	0	/* NumBlks */
	.byte	mus_qz_593_pri	/* Priority */
	.byte	mus_qz_593_rev	/* Reverb. */
space64 4
	ptrvalue	mus_qz_593_grp

	ptrvalue	mus_qz_593_1

	.end
