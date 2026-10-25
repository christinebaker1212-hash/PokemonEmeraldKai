	.include "asm/macros/bit_width.inc"

	.include "MPlayDef.s"

/* Moemon Quetzal song 594, disassembled from the ROM by tools/quetzal_rip/songs.py.
   "org" comments give the ROM address of the bytes that follow. */

	.equ	mus_qz_594_grp, voicegroup_bard
	.equ	mus_qz_594_pri, 4
	.equ	mus_qz_594_rev, 0
	.equ	mus_qz_594_mvl, 127
	.equ	mus_qz_594_key, 0
	.equ	mus_qz_594_tbs, 1
	.equ	mus_qz_594_exg, 1
	.equ	mus_qz_594_cmp, 1

	.section .rodata
	.global	mus_qz_594
	.p2align	2
/* org 0x0966982C */

/********************** Track  1 **********************/

mus_qz_594_1:
	.byte		KEYSH , mus_qz_594_key+0
	.byte		TEMPO , 120*mus_qz_594_tbs/2
	.byte		VOICE , 35
	.byte		VOL   , 127*mus_qz_594_mvl/mxv
	.byte		N54   , Cn3 , v127
	.byte	W54
	.byte	FINE

/******************************************************/
	.p2align	2

mus_qz_594:
	.byte	1	/* NumTrks */
	.byte	0	/* NumBlks */
	.byte	mus_qz_594_pri	/* Priority */
	.byte	mus_qz_594_rev	/* Reverb. */
space64 4
	ptrvalue	mus_qz_594_grp

	ptrvalue	mus_qz_594_1

	.end
