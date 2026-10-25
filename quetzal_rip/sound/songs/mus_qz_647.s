	.include "asm/macros/bit_width.inc"

	.include "MPlayDef.s"

/* Moemon Quetzal song 647, disassembled from the ROM by tools/quetzal_rip/songs.py.
   "org" comments give the ROM address of the bytes that follow. */

	.equ	mus_qz_647_grp, voicegroup_qz_songs
	.equ	mus_qz_647_pri, 0
	.equ	mus_qz_647_rev, reverb_set+0
	.equ	mus_qz_647_mvl, 127
	.equ	mus_qz_647_key, 0
	.equ	mus_qz_647_tbs, 1
	.equ	mus_qz_647_exg, 1
	.equ	mus_qz_647_cmp, 1

	.section .rodata
	.global	mus_qz_647
	.p2align	2
/* org 0x09695198 */

/********************** Track  1 **********************/

mus_qz_647_1:
	.byte		KEYSH , mus_qz_647_key+0
	.byte		TEMPO , 128*mus_qz_647_tbs/2
	.byte		VOICE , 19
	.byte		VOL   , 18*mus_qz_647_mvl/mxv
	.byte		        79*mus_qz_647_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte	W36
	.byte		VOL   , 37*mus_qz_647_mvl/mxv
	.byte		        60*mus_qz_647_mvl/mxv
	.byte	W24
	.byte		N06   , Cs3 , v100
	.byte	W36
	.byte		N24   , Dn3
	.byte	W24
	.byte		N06   , Cn3
	.byte	W12
mus_qz_647_1_000:
	.byte	W12
	.byte		N06   , Cs3 , v100
	.byte	W36
	.byte		        Bn2
	.byte	W24
	.byte		N12   , As2
	.byte	W12
	.byte		N06   , Bn2
	.byte	W12
	.byte	PEND
mus_qz_647_1_001:
	.byte	W24
	.byte		N06   , Cs3 , v100
	.byte	W36
	.byte		N24   , Dn3
	.byte	W24
	.byte		N06   , Cn3
	.byte	W12
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_647_1_000
mus_qz_647_1_B1:
	.byte	PATT
	 ptrvalue	mus_qz_647_1_001
	.byte	PATT
	 ptrvalue	mus_qz_647_1_000
	.byte	PATT
	 ptrvalue	mus_qz_647_1_001
	.byte	PATT
	 ptrvalue	mus_qz_647_1_000
	.byte	GOTO
	 ptrvalue	mus_qz_647_1_B1
	.byte	FINE

/********************** Track  2 **********************/

mus_qz_647_2:
	.byte		KEYSH , mus_qz_647_key+0
	.byte		VOICE , 3
	.byte		VOL   , 27*mus_qz_647_mvl/mxv
	.byte		        79*mus_qz_647_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte		N02   , En4 , v100
	.byte	W03
	.byte		        Fs4
	.byte	W03
	.byte		        Gs4
	.byte	W03
	.byte		        An4
	.byte	W03
	.byte		N23   , Bn4
	.byte	W24
	.byte		VOICE , 19
	.byte		VOL   , 60*mus_qz_647_mvl/mxv
	.byte	W24
	.byte		N06   , En3
	.byte	W36
	.byte		N24   , Fs3
	.byte	W24
	.byte		N06   , Ds3
	.byte	W12
mus_qz_647_2_000:
	.byte	W12
	.byte		N06   , En3 , v100
	.byte	W36
	.byte		        Dn3
	.byte	W24
	.byte		N12   , Cs3
	.byte	W12
	.byte		N06   , Dn3
	.byte	W12
	.byte	PEND
mus_qz_647_2_001:
	.byte	W24
	.byte		N06   , En3 , v100
	.byte	W36
	.byte		N24   , Fs3
	.byte	W24
	.byte		N06   , Ds3
	.byte	W12
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_647_2_000
mus_qz_647_2_B1:
	.byte	PATT
	 ptrvalue	mus_qz_647_2_001
	.byte	PATT
	 ptrvalue	mus_qz_647_2_000
	.byte	PATT
	 ptrvalue	mus_qz_647_2_001
	.byte	PATT
	 ptrvalue	mus_qz_647_2_000
	.byte	GOTO
	 ptrvalue	mus_qz_647_2_B1
	.byte	FINE

/********************** Track  3 **********************/

mus_qz_647_3:
	.byte		KEYSH , mus_qz_647_key+0
	.byte		VOICE , 3
	.byte		VOL   , 46*mus_qz_647_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte	W36
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		        c_v+15
	.byte	W84
	.byte		N05   , Gs4 , v100
	.byte	W12
mus_qz_647_3_B1:
	.byte		N05   , An4 , v100
	.byte	W36
	.byte	N05
	.byte	W60
	.byte	W96
	.byte	W96
	.byte	W84
	.byte		        En4
	.byte	W12
	.byte	GOTO
	 ptrvalue	mus_qz_647_3_B1
	.byte	FINE

/********************** Track  4 **********************/

mus_qz_647_4:
	.byte		KEYSH , mus_qz_647_key+0
	.byte		VOICE , 3
	.byte		VOL   , 46*mus_qz_647_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte	W36
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		        c_v+15
	.byte	W84
	.byte		N05   , Bn3 , v100
	.byte	W12
mus_qz_647_4_B1:
	.byte		N05   , Cs4 , v100
	.byte	W36
	.byte	N05
	.byte	W60
	.byte	W96
	.byte	W96
	.byte	W84
	.byte		        Dn4
	.byte	W12
	.byte	GOTO
	 ptrvalue	mus_qz_647_4_B1
	.byte	FINE

/********************** Track  5 **********************/

mus_qz_647_5:
	.byte		KEYSH , mus_qz_647_key+0
	.byte		VOICE , 5
	.byte		VOL   , 39*mus_qz_647_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte	W36
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
mus_qz_647_5_B1:
	.byte		PAN   , c_v-38
	.byte		VOL   , 20*mus_qz_647_mvl/mxv
	.byte	W24
	.byte		N11   , An4 , v100
	.byte	W12
	.byte		        En5
	.byte	W12
	.byte		        En5 , v048
	.byte	W12
	.byte		        An4 , v100
	.byte	W12
	.byte		        En5
	.byte	W12
	.byte		N23   , En5 , v048
	.byte	W12
	.byte	W24
	.byte		N11   , An4 , v100
	.byte	W12
	.byte		        En5
	.byte	W12
	.byte		        En5 , v048
	.byte	W12
	.byte		        An5 , v100
	.byte	W12
	.byte		        En5
	.byte	W12
	.byte		N23   , En5 , v048
	.byte	W12
	.byte	W24
	.byte		PAN   , c_v+40
	.byte		N11   , An4 , v100
	.byte	W12
	.byte		        En5
	.byte	W12
	.byte		        En5 , v048
	.byte	W12
	.byte		        An4 , v100
	.byte	W12
	.byte		        En5
	.byte	W12
	.byte		N23   , En5 , v048
	.byte	W12
	.byte	W24
	.byte		N11   , An4 , v100
	.byte	W12
	.byte		        En5
	.byte	W12
	.byte		        En5 , v048
	.byte	W12
	.byte		        En4 , v100
	.byte	W12
	.byte		        En5
	.byte	W12
	.byte		        En5 , v048
	.byte	W12
	.byte	GOTO
	 ptrvalue	mus_qz_647_5_B1
	.byte	FINE

/********************** Track  6 **********************/

mus_qz_647_6:
	.byte		KEYSH , mus_qz_647_key+0
	.byte		VOICE , 19
	.byte		VOL   , 79*mus_qz_647_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte		N02   , Bn1 , v100
	.byte	W03
	.byte		        An1
	.byte	W03
	.byte		        Gs1
	.byte	W03
	.byte		        Fs1
	.byte	W03
	.byte		N23   , En1
	.byte	W24
mus_qz_647_6_000:
	.byte		N06   , An1 , v127
	.byte	W36
	.byte	N06
	.byte	W12
	.byte		        Dn1
	.byte	W48
	.byte	PEND
mus_qz_647_6_001:
	.byte		N06   , An0 , v127
	.byte	W36
	.byte	N06
	.byte	W12
	.byte		N05   , En1
	.byte	W48
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_647_6_000
	.byte	PATT
	 ptrvalue	mus_qz_647_6_001
mus_qz_647_6_B1:
	.byte		N06   , An0 , v127
	.byte	W36
	.byte	N06
	.byte	W12
	.byte		        Dn1
	.byte	W48
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_647_6_001
	.byte	PATT
	 ptrvalue	mus_qz_647_6_B1
	.byte	PATT
	 ptrvalue	mus_qz_647_6_001
	.byte	GOTO
	 ptrvalue	mus_qz_647_6_B1
	.byte	FINE

/********************** Track  7 **********************/

mus_qz_647_7:
	.byte		KEYSH , mus_qz_647_key+0
	.byte		VOICE , 39
	.byte		VOL   , 56*mus_qz_647_mvl/mxv
	.byte	W36
mus_qz_647_7_000:
	.byte		N06   , Cn1 , v100
	.byte	W12
	.byte		N12   , As1 , v064
	.byte	W12
	.byte		N06   , En1 , v088
	.byte	W12
	.byte		        Fs1 , v064
	.byte	W12
	.byte	N06
	.byte	W12
	.byte		N24   , As1
	.byte	W24
	.byte		N06   , En1 , v088
	.byte	W12
	.byte	PEND
mus_qz_647_7_001:
	.byte		N06   , Cn1 , v100
	.byte	W12
	.byte		N24   , As1 , v064
	.byte	W24
	.byte		N06   , En1 , v088
	.byte	W12
	.byte		        Fs1 , v064
	.byte	W12
	.byte	N06
	.byte	W12
	.byte		N12   , En1 , v088
	.byte	W12
	.byte		N06   , Cn1 , v100
	.byte	W12
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_647_7_000
	.byte	PATT
	 ptrvalue	mus_qz_647_7_001
mus_qz_647_7_B1:
	.byte	PATT
	 ptrvalue	mus_qz_647_7_000
	.byte	PATT
	 ptrvalue	mus_qz_647_7_001
	.byte	PATT
	 ptrvalue	mus_qz_647_7_000
	.byte	PATT
	 ptrvalue	mus_qz_647_7_001
	.byte	GOTO
	 ptrvalue	mus_qz_647_7_B1
	.byte	FINE

/******************************************************/
	.p2align	2

mus_qz_647:
	.byte	7	/* NumTrks */
	.byte	0	/* NumBlks */
	.byte	mus_qz_647_pri	/* Priority */
	.byte	mus_qz_647_rev	/* Reverb. */
space64 4
	ptrvalue	mus_qz_647_grp

	ptrvalue	mus_qz_647_1
	ptrvalue	mus_qz_647_2
	ptrvalue	mus_qz_647_3
	ptrvalue	mus_qz_647_4
	ptrvalue	mus_qz_647_5
	ptrvalue	mus_qz_647_6
	ptrvalue	mus_qz_647_7

	.end
