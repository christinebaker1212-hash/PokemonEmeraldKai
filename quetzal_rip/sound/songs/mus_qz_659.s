	.include "asm/macros/bit_width.inc"

	.include "MPlayDef.s"

/* Moemon Quetzal song 659, disassembled from the ROM by tools/quetzal_rip/songs.py.
   "org" comments give the ROM address of the bytes that follow. */

	.equ	mus_qz_659_grp, voicegroup_qz_songs
	.equ	mus_qz_659_pri, 0
	.equ	mus_qz_659_rev, reverb_set+0
	.equ	mus_qz_659_mvl, 127
	.equ	mus_qz_659_key, 0
	.equ	mus_qz_659_tbs, 1
	.equ	mus_qz_659_exg, 1
	.equ	mus_qz_659_cmp, 1

	.section .rodata
	.global	mus_qz_659
	.p2align	2
/* org 0x096A02F8 */

/********************** Track  1 **********************/

mus_qz_659_1:
	.byte		KEYSH , mus_qz_659_key+0
	.byte		TEMPO , 132*mus_qz_659_tbs/2
	.byte		VOICE , 0
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte		VOL   , 55*mus_qz_659_mvl/mxv
	.byte		PAN   , c_v-9
	.byte	W48
mus_qz_659_1_000:
	.byte	W24
	.byte		N05   , As3 , v100
	.byte	W24
	.byte	N05
	.byte	W24
	.byte	PEND
mus_qz_659_1_B1:
	.byte	W24
	.byte		N05   , As3 , v100
	.byte	W24
	.byte	N05
	.byte	W24
	.byte	PATT
	 ptrvalue	mus_qz_659_1_000
	.byte	PATT
	 ptrvalue	mus_qz_659_1_000
mus_qz_659_1_001:
	.byte	W24
	.byte		N05   , Gn3 , v100
	.byte	W24
	.byte	N05
	.byte	W24
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_659_1_001
	.byte	PATT
	 ptrvalue	mus_qz_659_1_001
	.byte	PATT
	 ptrvalue	mus_qz_659_1_001
	.byte		VOICE , 26
	.byte	W24
	.byte		N05   , As3 , v100
	.byte	W24
	.byte	N05
	.byte	W24
	.byte	PATT
	 ptrvalue	mus_qz_659_1_000
	.byte	PATT
	 ptrvalue	mus_qz_659_1_000
	.byte	PATT
	 ptrvalue	mus_qz_659_1_000
	.byte	PATT
	 ptrvalue	mus_qz_659_1_001
	.byte	PATT
	 ptrvalue	mus_qz_659_1_001
	.byte	PATT
	 ptrvalue	mus_qz_659_1_001
	.byte	PATT
	 ptrvalue	mus_qz_659_1_001
	.byte		VOICE , 0
	.byte	W24
	.byte		N05   , As3 , v100
	.byte	W24
	.byte	N05
	.byte	W24
	.byte	GOTO
	 ptrvalue	mus_qz_659_1_B1
	.byte	FINE

/********************** Track  2 **********************/

mus_qz_659_2:
	.byte		KEYSH , mus_qz_659_key+0
	.byte		VOICE , 0
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte		VOL   , 55*mus_qz_659_mvl/mxv
	.byte		PAN   , c_v-9
	.byte		N05   , An1 , v100
	.byte	W06
	.byte		        Fs1
	.byte	W06
	.byte		        Ds1
	.byte	W06
	.byte		        Cn1
	.byte	W06
	.byte		N23   , An0
	.byte	W24
mus_qz_659_2_000:
	.byte		N11   , Cn2 , v100
	.byte	W24
	.byte		N05   , Fs3
	.byte	W24
	.byte		        An3
	.byte	W24
	.byte	PEND
mus_qz_659_2_B1:
	.byte		N11   , Gn1 , v100
	.byte	W24
	.byte		N05   , Cn3
	.byte	W24
	.byte		        Ds3
	.byte	W24
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_659_2_000
	.byte	PATT
	 ptrvalue	mus_qz_659_2_B1
mus_qz_659_2_001:
	.byte		N11   , An1 , v100
	.byte	W24
	.byte		N05   , Ds3
	.byte	W24
	.byte		        Fs3
	.byte	W24
	.byte	PEND
mus_qz_659_2_002:
	.byte		N11   , En1 , v100
	.byte	W24
	.byte		N05   , An2
	.byte	W24
	.byte		        Cn3
	.byte	W24
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_659_2_001
	.byte	PATT
	 ptrvalue	mus_qz_659_2_002
	.byte		VOICE , 26
	.byte		N11   , Cn2 , v100
	.byte	W12
	.byte		N05   , Ds3
	.byte	W12
	.byte		        Fs3
	.byte	W12
	.byte		        Ds3
	.byte	W12
	.byte		        An3
	.byte	W12
	.byte		        Ds3
	.byte	W12
mus_qz_659_2_003:
	.byte		N11   , Gn1 , v100
	.byte	W12
	.byte		N05   , Cn3
	.byte	W12
	.byte		        Ds3
	.byte	W12
	.byte		        Cn3
	.byte	W12
	.byte		        An3
	.byte	W12
	.byte		        Cn3
	.byte	W12
	.byte	PEND
	.byte		N11   , Cn2
	.byte	W12
	.byte		N05   , Ds3
	.byte	W12
	.byte		        Fs3
	.byte	W12
	.byte		        Ds3
	.byte	W12
	.byte		        An3
	.byte	W12
	.byte		        Ds3
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_659_2_003
mus_qz_659_2_004:
	.byte		N11   , An1 , v100
	.byte	W12
	.byte		N05   , Cn3
	.byte	W12
	.byte		        Ds3
	.byte	W12
	.byte		        Cn3
	.byte	W12
	.byte		        Fs3
	.byte	W12
	.byte		        Cn3
	.byte	W12
	.byte	PEND
mus_qz_659_2_005:
	.byte		N11   , En1 , v100
	.byte	W12
	.byte		N05   , An2
	.byte	W12
	.byte		        Cn3
	.byte	W12
	.byte		        An2
	.byte	W12
	.byte		        Fs3
	.byte	W12
	.byte		        An2
	.byte	W12
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_659_2_004
	.byte	PATT
	 ptrvalue	mus_qz_659_2_005
	.byte		VOICE , 0
	.byte		N11   , Cn2 , v100
	.byte	W24
	.byte		N05   , Fs3
	.byte	W24
	.byte		        An3
	.byte	W24
	.byte	GOTO
	 ptrvalue	mus_qz_659_2_B1
	.byte	FINE

/********************** Track  3 **********************/

mus_qz_659_3:
	.byte		KEYSH , mus_qz_659_key+0
	.byte		VOICE , 32
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte		LFOS  , 25
	.byte		LFODL , 1
	.byte		MOD   , 6
	.byte		VOL   , 42*mus_qz_659_mvl/mxv
	.byte		PAN   , c_v+23
	.byte	W48
mus_qz_659_3_000:
	.byte		N32   , As4 , v100 , gtp3
	.byte	W36
	.byte		N11   , An4
	.byte	W12
	.byte		        Gs4
	.byte	W12
	.byte		        Gn4
	.byte	W12
	.byte	PEND
mus_qz_659_3_B1:
	.byte		N32   , Fs4 , v100 , gtp3
	.byte	W36
	.byte		N11   , Fn4
	.byte	W12
	.byte		        Fs4
	.byte	W12
	.byte		        An4
	.byte	W12
	.byte	PEND
mus_qz_659_3_001:
	.byte		N22   , An4 , v024
	.byte	W24
	.byte		N17   , An4 , v008
	.byte	W48
	.byte	PEND
	.byte	W72
mus_qz_659_3_002:
	.byte		N32   , Gn4 , v100 , gtp3
	.byte	W36
	.byte		N11   , Fs4
	.byte	W12
	.byte		        Fn4
	.byte	W12
	.byte		        En4
	.byte	W12
	.byte	PEND
mus_qz_659_3_003:
	.byte		N32   , Ds4 , v100 , gtp3
	.byte	W36
	.byte		N11   , Dn4
	.byte	W12
	.byte		        Ds4
	.byte	W12
	.byte		        Fs4
	.byte	W12
	.byte	PEND
mus_qz_659_3_004:
	.byte		N22   , Fs4 , v024
	.byte	W24
	.byte		N17   , Fs4 , v008
	.byte	W48
	.byte	PEND
	.byte	W48
	.byte		VOICE , 23
	.byte	W18
	.byte		VOL   , 45*mus_qz_659_mvl/mxv
	.byte		PAN   , c_v-22
	.byte	W06
	.byte	PATT
	 ptrvalue	mus_qz_659_3_000
	.byte	PATT
	 ptrvalue	mus_qz_659_3_B1
	.byte	PATT
	 ptrvalue	mus_qz_659_3_001
	.byte	W72
	.byte	PATT
	 ptrvalue	mus_qz_659_3_002
	.byte	PATT
	 ptrvalue	mus_qz_659_3_003
	.byte	PATT
	 ptrvalue	mus_qz_659_3_004
	.byte		VOICE , 32
	.byte		VOL   , 35*mus_qz_659_mvl/mxv
	.byte	W42
	.byte		PAN   , c_v+18
	.byte	W30
	.byte	PATT
	 ptrvalue	mus_qz_659_3_000
	.byte	GOTO
	 ptrvalue	mus_qz_659_3_B1
	.byte	FINE

/********************** Track  4 **********************/

mus_qz_659_4:
	.byte		KEYSH , mus_qz_659_key+0
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte		VOL   , 43*mus_qz_659_mvl/mxv
	.byte	W48
	.byte	W24
	.byte		VOICE , 12
	.byte	W48
mus_qz_659_4_B1:
	.byte	W68
	.byte		PAN   , c_v+14
	.byte	W04
	.byte		N23   , Ds3 , v100
	.byte	W24
	.byte		N05   , Cn3
	.byte	W06
	.byte		        Cn3 , v020
	.byte	W06
	.byte		        Ds3 , v100
	.byte	W06
	.byte		        Ds3 , v020
	.byte	W06
	.byte		        Cn3 , v100
	.byte	W06
	.byte		        Cn3 , v020
	.byte	W06
	.byte		        Ds3 , v100
	.byte	W06
	.byte		        Ds3 , v020
	.byte	W06
	.byte		PAN   , c_v-14
	.byte		N23   , Fs4 , v100
	.byte	W24
	.byte		N05   , Cn4
	.byte	W06
	.byte		        Cn4 , v020
	.byte	W06
	.byte		        Fs4 , v100
	.byte	W06
	.byte		        Fs4 , v020
	.byte	W06
	.byte		        Cn4 , v100
	.byte	W06
	.byte		        Cn4 , v020
	.byte	W06
	.byte		        Fs4 , v100
	.byte	W06
	.byte		        Fs4 , v020
	.byte	W06
	.byte	W72
	.byte	W72
	.byte		PAN   , c_v+14
	.byte		N23   , Cn3 , v100
	.byte	W24
	.byte		N05   , An2
	.byte	W06
	.byte		        An2 , v020
	.byte	W06
	.byte		        Cn3 , v100
	.byte	W06
	.byte		        Cn3 , v020
	.byte	W06
	.byte		        Ds3 , v100
	.byte	W06
	.byte		        Ds3 , v020
	.byte	W06
	.byte		        Cn3 , v100
	.byte	W06
	.byte		        Cn3 , v020
	.byte	W06
	.byte		PAN   , c_v-14
	.byte		N23   , Ds4 , v100
	.byte	W24
	.byte		N05   , An3
	.byte	W06
	.byte		        An3 , v020
	.byte	W06
	.byte		        Ds4 , v100
	.byte	W06
	.byte		        Ds4 , v020
	.byte	W06
	.byte		        Fs4 , v100
	.byte	W06
	.byte		        Fs4 , v020
	.byte	W06
	.byte		        Cn4 , v100
	.byte	W06
	.byte		        Cn4 , v020
	.byte	W06
	.byte	W72
	.byte	W72
	.byte		PAN   , c_v-13
	.byte		N23   , Ds3 , v100
	.byte	W24
	.byte		N05   , Cn3
	.byte	W06
	.byte		        Cn3 , v020
	.byte	W06
	.byte		        Ds3 , v100
	.byte	W06
	.byte		        Ds3 , v020
	.byte	W06
	.byte		        Cn3 , v100
	.byte	W06
	.byte		        Cn3 , v020
	.byte	W06
	.byte		        Ds3 , v100
	.byte	W06
	.byte		        Ds3 , v020
	.byte	W06
	.byte		PAN   , c_v+15
	.byte		N23   , Fs4 , v100
	.byte	W24
	.byte		N05   , Cn4
	.byte	W06
	.byte		        Cn4 , v020
	.byte	W06
	.byte		        Fs4 , v100
	.byte	W06
	.byte		        Fs4 , v020
	.byte	W06
	.byte		        Cn4 , v100
	.byte	W06
	.byte		        Cn4 , v020
	.byte	W06
	.byte		        Fs4 , v100
	.byte	W06
	.byte		        Fs4 , v020
	.byte	W06
	.byte	W72
	.byte	W72
	.byte		PAN   , c_v-16
	.byte		N23   , Cn4 , v100
	.byte	W24
	.byte		N05   , An3
	.byte	W06
	.byte		        An3 , v020
	.byte	W06
	.byte		        Cn4 , v100
	.byte	W06
	.byte		        Cn4 , v020
	.byte	W06
	.byte		        Ds4 , v100
	.byte	W06
	.byte		        Ds4 , v020
	.byte	W06
	.byte		        Cn4 , v100
	.byte	W06
	.byte		        Cn4 , v020
	.byte	W06
	.byte		PAN   , c_v+15
	.byte		N23   , Ds4 , v100
	.byte	W24
	.byte		N05   , An3
	.byte	W06
	.byte		        An3 , v020
	.byte	W06
	.byte		        Ds4 , v100
	.byte	W06
	.byte		        Ds4 , v020
	.byte	W06
	.byte		        Fs4 , v100
	.byte	W06
	.byte		        Fs4 , v020
	.byte	W06
	.byte		        Cn4 , v100
	.byte	W06
	.byte		        Cn4 , v020
	.byte	W06
	.byte	W72
	.byte	GOTO
	 ptrvalue	mus_qz_659_4_B1
	.byte	FINE

/********************** Track  5 **********************/

mus_qz_659_5:
	.byte		KEYSH , mus_qz_659_key+0
	.byte		VOICE , 32
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte		VOL   , 54*mus_qz_659_mvl/mxv
	.byte		N05   , Cn3 , v100
	.byte	W06
	.byte		        Ds3
	.byte	W06
	.byte		        Fs3
	.byte	W06
	.byte		        An3
	.byte	W06
	.byte		N23   , Cn4
	.byte	W03
	.byte		VOL   , 58*mus_qz_659_mvl/mxv
	.byte	W04
	.byte		        59*mus_qz_659_mvl/mxv
	.byte	W02
	.byte		        60*mus_qz_659_mvl/mxv
	.byte	W03
	.byte		        63*mus_qz_659_mvl/mxv
	.byte	W01
	.byte		        65*mus_qz_659_mvl/mxv
	.byte	W02
	.byte		        66*mus_qz_659_mvl/mxv
	.byte	W01
	.byte		        68*mus_qz_659_mvl/mxv
	.byte	W02
	.byte		        70*mus_qz_659_mvl/mxv
	.byte	W02
	.byte		        71*mus_qz_659_mvl/mxv
	.byte	W01
	.byte		        73*mus_qz_659_mvl/mxv
	.byte	W03
	.byte		N11   , Cn4 , v024
	.byte	W72
mus_qz_659_5_B1:
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W24
	.byte		VOICE , 19
	.byte	W36
	.byte		N11   , Gn1 , v127
	.byte	W12
	.byte		N68   , Cn1 , v127 , gtp3
	.byte	W72
	.byte		        Gn1 , v127 , gtp3
	.byte	W72
	.byte		        Cn2 , v127 , gtp3
	.byte	W72
	.byte		        Gn1 , v127 , gtp3
	.byte	W72
	.byte		        An0 , v127 , gtp3
	.byte	W72
	.byte		        En1 , v127 , gtp3
	.byte	W72
	.byte		        An1 , v127 , gtp3
	.byte	W72
	.byte		        En1 , v127 , gtp3
	.byte	W72
	.byte	W72
	.byte	GOTO
	 ptrvalue	mus_qz_659_5_B1
	.byte	FINE

/********************** Track  6 **********************/

mus_qz_659_6:
	.byte		KEYSH , mus_qz_659_key+0
	.byte		VOICE , 5
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte		        c_v+13
	.byte		VOL   , 25*mus_qz_659_mvl/mxv
	.byte		N05   , Cn4 , v100
	.byte	W06
	.byte		        Ds4
	.byte	W06
	.byte		        Fs4
	.byte	W06
	.byte		        An4
	.byte	W06
	.byte		N23   , Cn5
	.byte	W24
	.byte		VOL   , 45*mus_qz_659_mvl/mxv
	.byte		PAN   , c_v-22
	.byte		N32   , Cn5 , v100 , gtp3
	.byte	W36
	.byte		N11   , Bn4
	.byte	W12
	.byte		        Cn5
	.byte	W12
	.byte		        Dn5
	.byte	W12
mus_qz_659_6_B1:
	.byte		N32   , Ds5 , v100 , gtp3
	.byte	W36
	.byte		N11   , Fn5
	.byte	W12
	.byte		        Fs5
	.byte	W12
	.byte		N52   , Fn5 , v100 , gtp1
	.byte	W12
	.byte	W72
	.byte	W72
	.byte		N32   , An4 , v100 , gtp3
	.byte	W36
	.byte		N11   , Gs4
	.byte	W12
	.byte		        An4
	.byte	W12
	.byte		        Bn4
	.byte	W12
	.byte		N32   , Cn5 , v100 , gtp3
	.byte	W36
	.byte		N11   , Dn5
	.byte	W12
	.byte		        Ds5
	.byte	W12
	.byte		N52   , Dn5 , v100 , gtp1
	.byte	W12
	.byte	W72
	.byte	W72
	.byte		VOICE , 27
	.byte		PAN   , c_v+15
	.byte		VOL   , 63*mus_qz_659_mvl/mxv
	.byte		N11   , Cn3
	.byte	W12
	.byte		        Gn2
	.byte	W12
	.byte		        Cn3
	.byte	W12
	.byte		        Bn2
	.byte	W12
	.byte		        Cn3
	.byte	W12
	.byte		        Dn3
	.byte	W12
	.byte		        Ds3
	.byte	W12
	.byte		        Gn2
	.byte	W12
	.byte		        Ds3
	.byte	W12
	.byte		        Fn3
	.byte	W12
	.byte		        Fs3
	.byte	W12
	.byte		        Fn3
	.byte	W12
	.byte		        Gn3
	.byte	W12
	.byte		        Fn3
	.byte	W12
	.byte		N32   , Fs3 , v100 , gtp3
	.byte	W36
	.byte		N11   , Fn3
	.byte	W12
	.byte		        Gn3
	.byte	W12
	.byte		        An3
	.byte	W12
	.byte		N32   , As3 , v100 , gtp3
	.byte	W36
	.byte		N11   , An3
	.byte	W12
	.byte		        An2
	.byte	W12
	.byte		        En2
	.byte	W12
	.byte		        An2
	.byte	W12
	.byte		        Gs2
	.byte	W12
	.byte		        An2
	.byte	W12
	.byte		        Bn2
	.byte	W12
	.byte		        Cn3
	.byte	W12
	.byte		        En2
	.byte	W12
	.byte		        Cn3
	.byte	W12
	.byte		        Dn3
	.byte	W12
	.byte		        Ds3
	.byte	W12
	.byte		        Dn3
	.byte	W12
	.byte		        En3
	.byte	W12
	.byte		        Dn3
	.byte	W12
	.byte		N32   , Ds3 , v100 , gtp3
	.byte	W36
	.byte		N11   , Dn3
	.byte	W12
	.byte		        En3
	.byte	W12
	.byte		        Fs3
	.byte	W12
	.byte		N32   , Gn3 , v100 , gtp3
	.byte	W36
	.byte		N11   , Fs3
	.byte	W12
	.byte		VOICE , 5
	.byte		PAN   , c_v-19
	.byte		VOL   , 42*mus_qz_659_mvl/mxv
	.byte		N32   , Cn5 , v100 , gtp3
	.byte	W36
	.byte		N11   , Bn4
	.byte	W12
	.byte		        Cn5
	.byte	W12
	.byte		        Dn5
	.byte	W12
	.byte	GOTO
	 ptrvalue	mus_qz_659_6_B1
	.byte	FINE

/********************** Track  7 **********************/

mus_qz_659_7:
	.byte		KEYSH , mus_qz_659_key+0
	.byte		VOICE , 35
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte		LFOS  , 25
	.byte		LFODL , 1
	.byte		VOL   , 35*mus_qz_659_mvl/mxv
	.byte		MOD   , 6
	.byte		PAN   , c_v+18
	.byte	W48
	.byte	W72
mus_qz_659_7_B1:
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W66
	.byte		PAN   , c_v-10
	.byte	W06
	.byte		        c_v+11
	.byte		VOL   , 50*mus_qz_659_mvl/mxv
	.byte		N32   , As5 , v100 , gtp3
	.byte	W24
	.byte		PAN   , c_v+8
	.byte	W12
	.byte		N11   , An5
	.byte	W12
	.byte		PAN   , c_v-10
	.byte		N11   , Gs5
	.byte	W12
	.byte		        Gn5
	.byte	W12
	.byte		PAN   , c_v-18
	.byte		N32   , Fs5 , v100 , gtp3
	.byte	W24
	.byte		PAN   , c_v-29
	.byte	W12
	.byte		N11   , Fn5
	.byte	W12
	.byte		PAN   , c_v-32
	.byte		N11   , Fs5
	.byte	W12
	.byte		        An5
	.byte	W12
	.byte		PAN   , c_v-29
	.byte		N22   , An5 , v024
	.byte	W24
	.byte		PAN   , c_v-24
	.byte		N17   , An5 , v008
	.byte	W24
	.byte		PAN   , c_v-16
	.byte	W24
	.byte		        c_v-8
	.byte	W24
	.byte		        c_v-5
	.byte	W48
	.byte		        c_v-16
	.byte		N32   , Gn5 , v100 , gtp3
	.byte	W24
	.byte		PAN   , c_v-24
	.byte	W12
	.byte		N11   , Fs5
	.byte	W12
	.byte		PAN   , c_v-36
	.byte		N11   , Fn5
	.byte	W12
	.byte		        En5
	.byte	W12
	.byte		PAN   , c_v-39
	.byte		N32   , Ds5 , v100 , gtp3
	.byte	W36
	.byte		N11   , Dn5
	.byte	W12
	.byte		PAN   , c_v-32
	.byte		N11   , Ds5
	.byte	W12
	.byte		        Fs5
	.byte	W12
	.byte		N22   , Fs5 , v024
	.byte	W24
	.byte		PAN   , c_v-24
	.byte		N17   , Fs5 , v008
	.byte	W24
	.byte		PAN   , c_v-5
	.byte	W24
	.byte		        c_v+16
	.byte	W24
	.byte		        c_v+27
	.byte	W24
	.byte		        c_v+22
	.byte	W24
	.byte		        c_v+19
	.byte	W72
	.byte	GOTO
	 ptrvalue	mus_qz_659_7_B1
	.byte	FINE

/********************** Track  8 **********************/

mus_qz_659_8:
	.byte		KEYSH , mus_qz_659_key+0
	.byte		VOICE , 32
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte		LFOS  , 25
	.byte		LFODL , 1
	.byte		PRIO  , 57
	.byte		PAN   , c_v-34
	.byte		MOD   , 6
	.byte		VOL   , 21*mus_qz_659_mvl/mxv
	.byte	W48
mus_qz_659_8_000:
	.byte	W06
	.byte		N32   , As4 , v100 , gtp3
	.byte	W36
	.byte		N11   , An4
	.byte	W12
	.byte		        Gs4
	.byte	W12
	.byte		        Gn4
	.byte	W06
	.byte	PEND
mus_qz_659_8_001:
	.byte	W06
	.byte		N32   , Fs4 , v100 , gtp3
	.byte	W36
	.byte		N11   , Fn4
	.byte	W12
	.byte		        Fs4
	.byte	W12
	.byte		        An4
	.byte	W06
	.byte	PEND
mus_qz_659_8_002:
	.byte	W06
	.byte		N22   , An4 , v024
	.byte	W24
	.byte		N17   , An4 , v008
	.byte	W42
	.byte	PEND
	.byte	W72
mus_qz_659_8_003:
	.byte	W06
	.byte		N32   , Gn4 , v100 , gtp3
	.byte	W36
	.byte		N11   , Fs4
	.byte	W12
	.byte		        Fn4
	.byte	W12
	.byte		        En4
	.byte	W06
	.byte	PEND
mus_qz_659_8_004:
	.byte	W06
	.byte		N32   , Ds4 , v100 , gtp3
	.byte	W36
	.byte		N11   , Dn4
	.byte	W12
	.byte		        Ds4
	.byte	W12
	.byte		        Fs4
	.byte	W06
	.byte	PEND
mus_qz_659_8_005:
	.byte	W06
	.byte		N22   , Fs4 , v024
	.byte	W24
	.byte		N17   , Fs4 , v008
	.byte	W42
	.byte	PEND
	.byte	W48
	.byte		VOICE , 23
	.byte	W06
	.byte		PAN   , c_v+23
	.byte	W18
	.byte	PATT
	 ptrvalue	mus_qz_659_8_000
	.byte	PATT
	 ptrvalue	mus_qz_659_8_001
	.byte	PATT
	 ptrvalue	mus_qz_659_8_002
	.byte	W72
	.byte	PATT
	 ptrvalue	mus_qz_659_8_003
	.byte	PATT
	 ptrvalue	mus_qz_659_8_004
	.byte	PATT
	 ptrvalue	mus_qz_659_8_005
	.byte		VOICE , 32
	.byte	W54
	.byte		PAN   , c_v-34
	.byte	W18
	.byte	W06
	.byte		N32   , As4 , v100 , gtp3
	.byte	W36
	.byte		N11   , An4
	.byte	W12
	.byte		        Gs4
	.byte	W12
	.byte		N05   , Gn4
	.byte	W06
	.byte	GOTO
	 ptrvalue	mus_qz_659_8_001
	.byte	FINE

/********************** Track  9 **********************/

mus_qz_659_9:
	.byte		KEYSH , mus_qz_659_key+0
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte		PRIO  , 50
	.byte		VOL   , 13*mus_qz_659_mvl/mxv
	.byte	W48
	.byte	W24
	.byte		VOICE , 12
	.byte	W48
mus_qz_659_9_B1:
	.byte	W72
	.byte		PAN   , c_v-43
	.byte	W06
	.byte		N23   , Ds3 , v100
	.byte	W24
	.byte		N05   , Cn3
	.byte	W06
	.byte		        Cn3 , v020
	.byte	W06
	.byte		        Ds3 , v100
	.byte	W06
	.byte		        Ds3 , v020
	.byte	W06
	.byte		        Cn3 , v100
	.byte	W06
	.byte		        Cn3 , v020
	.byte	W06
	.byte		        Ds3 , v100
	.byte	W06
mus_qz_659_9_000:
	.byte		N05   , Ds3 , v020
	.byte	W06
	.byte		PAN   , c_v+40
	.byte		N23   , Fs4 , v100
	.byte	W24
	.byte		N05   , Cn4
	.byte	W06
	.byte		        Cn4 , v020
	.byte	W06
	.byte		        Fs4 , v100
	.byte	W06
	.byte		        Fs4 , v020
	.byte	W06
	.byte		        Cn4 , v100
	.byte	W06
	.byte		        Cn4 , v020
	.byte	W06
	.byte		        Fs4 , v100
	.byte	W06
	.byte	PEND
	.byte		        Fs4 , v020
	.byte	W72
	.byte	W72
	.byte		PAN   , c_v-36
	.byte	W06
	.byte		N23   , Cn3 , v100
	.byte	W24
	.byte		N05   , An2
	.byte	W06
	.byte		        An2 , v020
	.byte	W06
	.byte		        Cn3 , v100
	.byte	W06
	.byte		        Cn3 , v020
	.byte	W06
	.byte		        Ds3 , v100
	.byte	W06
	.byte		        Ds3 , v020
	.byte	W06
	.byte		        Cn3 , v100
	.byte	W06
	.byte		        Cn3 , v020
	.byte	W06
	.byte		PAN   , c_v+40
	.byte		N23   , Ds4 , v100
	.byte	W24
	.byte		N05   , An3
	.byte	W06
	.byte		        An3 , v020
	.byte	W06
	.byte		        Ds4 , v100
	.byte	W06
	.byte		        Ds4 , v020
	.byte	W06
	.byte		        Fs4 , v100
	.byte	W06
	.byte		        Fs4 , v020
	.byte	W06
	.byte		        Cn4 , v100
	.byte	W06
	.byte		        Cn4 , v020
	.byte	W72
	.byte	W72
	.byte		PAN   , c_v-36
	.byte	W06
	.byte		N23   , Ds3 , v100
	.byte	W24
	.byte		N05   , Cn3
	.byte	W06
	.byte		        Cn3 , v020
	.byte	W06
	.byte		        Ds3 , v100
	.byte	W06
	.byte		        Ds3 , v020
	.byte	W06
	.byte		        Cn3 , v100
	.byte	W06
	.byte		        Cn3 , v020
	.byte	W06
	.byte		        Ds3 , v100
	.byte	W06
	.byte	PATT
	 ptrvalue	mus_qz_659_9_000
	.byte		N05   , Fs4 , v020
	.byte	W72
	.byte	W72
	.byte		PAN   , c_v-36
	.byte	W06
	.byte		N23   , Cn4 , v100
	.byte	W24
	.byte		N05   , An3
	.byte	W06
	.byte		        An3 , v020
	.byte	W06
	.byte		        Cn4 , v100
	.byte	W06
	.byte		        Cn4 , v020
	.byte	W06
	.byte		        Ds4 , v100
	.byte	W06
	.byte		        Ds4 , v020
	.byte	W06
	.byte		        Cn4 , v100
	.byte	W06
	.byte		        Cn4 , v020
	.byte	W06
	.byte		PAN   , c_v+40
	.byte		N23   , Ds4 , v100
	.byte	W24
	.byte		N05   , An3
	.byte	W06
	.byte		        An3 , v020
	.byte	W06
	.byte		        Ds4 , v100
	.byte	W06
	.byte		        Ds4 , v020
	.byte	W06
	.byte		        Fs4 , v100
	.byte	W06
	.byte		        Fs4 , v020
	.byte	W06
	.byte		        Cn4 , v100
	.byte	W06
	.byte		        Cn4 , v020
	.byte	W72
	.byte	GOTO
	 ptrvalue	mus_qz_659_9_B1
	.byte	FINE

/******************************************************/
	.p2align	2

mus_qz_659:
	.byte	9	/* NumTrks */
	.byte	0	/* NumBlks */
	.byte	mus_qz_659_pri	/* Priority */
	.byte	mus_qz_659_rev	/* Reverb. */
space64 4
	ptrvalue	mus_qz_659_grp

	ptrvalue	mus_qz_659_1
	ptrvalue	mus_qz_659_2
	ptrvalue	mus_qz_659_3
	ptrvalue	mus_qz_659_4
	ptrvalue	mus_qz_659_5
	ptrvalue	mus_qz_659_6
	ptrvalue	mus_qz_659_7
	ptrvalue	mus_qz_659_8
	ptrvalue	mus_qz_659_9

	.end
