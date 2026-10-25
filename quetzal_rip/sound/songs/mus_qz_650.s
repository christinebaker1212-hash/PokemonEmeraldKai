	.include "asm/macros/bit_width.inc"

	.include "MPlayDef.s"

/* Moemon Quetzal song 650, disassembled from the ROM by tools/quetzal_rip/songs.py.
   "org" comments give the ROM address of the bytes that follow. */

	.equ	mus_qz_650_grp, voicegroup_qz_songs
	.equ	mus_qz_650_pri, 0
	.equ	mus_qz_650_rev, reverb_set+0
	.equ	mus_qz_650_mvl, 127
	.equ	mus_qz_650_key, 0
	.equ	mus_qz_650_tbs, 1
	.equ	mus_qz_650_exg, 1
	.equ	mus_qz_650_cmp, 1

	.section .rodata
	.global	mus_qz_650
	.p2align	2
/* org 0x096975EC */

/********************** Track  1 **********************/

mus_qz_650_1:
	.byte		KEYSH , mus_qz_650_key+0
mus_qz_650_1_B1:
	.byte		TEMPO , 240*mus_qz_650_tbs/2
	.byte		VOICE , 0
	.byte		VOL   , 84*mus_qz_650_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		BENDR , 6
	.byte		PRIO  , 64
	.byte		MOD   , 10
	.byte		LFOS  , 35
	.byte		LFODL , 10
	.byte	W24
	.byte		N23   , Bn6 , v088
	.byte	W20
	.byte		BEND  , c_v-11
	.byte	W02
	.byte		        c_v-22
	.byte	W01
	.byte		        c_v+0
	.byte	W01
	.byte		N23   , Gn6
	.byte	W24
	.byte	N23
	.byte	W04
	.byte		BEND  , c_v+6
	.byte	W01
	.byte		        c_v+13
	.byte	W01
	.byte		        c_v+21
	.byte	W06
	.byte		        c_v-12
	.byte	W03
	.byte		        c_v-23
	.byte	W03
	.byte		        c_v-43
	.byte	W06
	.byte		        c_v+0
	.byte		N23   , Fn6
	.byte	W24
	.byte		        Gn6
	.byte	W06
	.byte		BEND  , c_v-34
	.byte	W02
	.byte		        c_v-54
	.byte	W07
	.byte		        c_v-30
	.byte	W02
	.byte		        c_v-2
	.byte	W03
	.byte		        c_v+0
	.byte	W04
	.byte	N23
	.byte	W24
	.byte	N23
	.byte	W08
	.byte		BEND  , c_v+21
	.byte	W08
	.byte		        c_v-22
	.byte	W08
	.byte		        c_v+0
	.byte		N23   , Fn6
	.byte	W12
	.byte		BEND  , c_v-2
	.byte	W03
	.byte		        c_v-5
	.byte	W03
	.byte		        c_v-7
	.byte	W04
	.byte		        c_v+0
	.byte	W02
	.byte		N28   , Fn6 , v088 , gtp1
	.byte	W06
	.byte		BEND  , c_v-33
	.byte	W04
	.byte		        c_v+0
	.byte	W01
	.byte		        c_v+13
	.byte	W01
	.byte		        c_v+15
	.byte	W01
	.byte		        c_v+21
	.byte	W07
	.byte		        c_v+13
	.byte	W01
	.byte		        c_v+5
	.byte	W02
	.byte		        c_v-10
	.byte	W01
	.byte		        c_v-17
	.byte	W02
	.byte		        c_v-24
	.byte	W01
	.byte		        c_v-31
	.byte	W02
	.byte		        c_v-43
	.byte	W01
	.byte		        c_v+0
	.byte	W96
	.byte	W42
	.byte	GOTO
	 ptrvalue	mus_qz_650_1_B1
	.byte	FINE

/********************** Track  2 **********************/

mus_qz_650_2:
	.byte		KEYSH , mus_qz_650_key+0
mus_qz_650_2_B1:
	.byte		VOICE , 22
	.byte		VOL   , 72*mus_qz_650_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte		N44   , Dn1 , v088 , gtp1
	.byte	W48
	.byte		        Dn1 , v088 , gtp1
	.byte	W48
	.byte		        Dn1 , v088 , gtp1
	.byte	W48
	.byte		        Dn1 , v088 , gtp1
	.byte	W96
	.byte	W96
	.byte	W48
	.byte	GOTO
	 ptrvalue	mus_qz_650_2_B1
	.byte	FINE

/********************** Track  3 **********************/

mus_qz_650_3:
	.byte		KEYSH , mus_qz_650_key+0
mus_qz_650_3_B1:
	.byte		VOICE , 0
	.byte		VOL   , 22*mus_qz_650_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		BENDR , 6
	.byte		PRIO  , 40
	.byte		MOD   , 10
	.byte		LFOS  , 35
	.byte		LFODL , 10
	.byte	W36
	.byte		N23   , Bn6 , v088
	.byte	W20
	.byte		BEND  , c_v-11
	.byte	W02
	.byte		        c_v-22
	.byte	W01
	.byte		        c_v+0
	.byte	W01
	.byte		N23   , Gn6
	.byte	W24
	.byte	N23
	.byte	W04
	.byte		BEND  , c_v+6
	.byte	W01
	.byte		        c_v+13
	.byte	W01
	.byte		        c_v+21
	.byte	W06
	.byte		        c_v-12
	.byte	W03
	.byte		        c_v-23
	.byte	W03
	.byte		        c_v-43
	.byte	W06
	.byte		        c_v+0
	.byte		N23   , Fn6
	.byte	W24
	.byte		        Gn6
	.byte	W06
	.byte		BEND  , c_v-34
	.byte	W02
	.byte		        c_v-54
	.byte	W07
	.byte		        c_v-30
	.byte	W02
	.byte		        c_v-2
	.byte	W03
	.byte		        c_v+0
	.byte	W04
	.byte	N23
	.byte	W24
	.byte	N23
	.byte	W08
	.byte		BEND  , c_v+21
	.byte	W08
	.byte		        c_v-22
	.byte	W08
	.byte		        c_v+0
	.byte		N23   , Fn6
	.byte	W12
	.byte		BEND  , c_v-2
	.byte	W03
	.byte		        c_v-5
	.byte	W03
	.byte		        c_v-7
	.byte	W04
	.byte		        c_v+0
	.byte	W02
	.byte		N28   , Fn6 , v088 , gtp1
	.byte	W06
	.byte		BEND  , c_v-33
	.byte	W04
	.byte		        c_v+0
	.byte	W01
	.byte		        c_v+13
	.byte	W01
	.byte		        c_v+15
	.byte	W01
	.byte		        c_v+21
	.byte	W07
	.byte		        c_v+13
	.byte	W01
	.byte		        c_v+5
	.byte	W02
	.byte		        c_v-10
	.byte	W01
	.byte		        c_v-17
	.byte	W02
	.byte		        c_v-24
	.byte	W01
	.byte		        c_v-31
	.byte	W02
	.byte		        c_v-43
	.byte	W01
	.byte		        c_v+0
	.byte	W96
	.byte	W30
	.byte	GOTO
	 ptrvalue	mus_qz_650_3_B1
	.byte	FINE

/******************************************************/
	.p2align	2

mus_qz_650:
	.byte	3	/* NumTrks */
	.byte	0	/* NumBlks */
	.byte	mus_qz_650_pri	/* Priority */
	.byte	mus_qz_650_rev	/* Reverb. */
space64 4
	ptrvalue	mus_qz_650_grp

	ptrvalue	mus_qz_650_1
	ptrvalue	mus_qz_650_2
	ptrvalue	mus_qz_650_3

	.end
