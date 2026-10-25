	.include "asm/macros/bit_width.inc"

	.include "MPlayDef.s"

/* Moemon Quetzal song 645, disassembled from the ROM by tools/quetzal_rip/songs.py.
   "org" comments give the ROM address of the bytes that follow. */

	.equ	mus_qz_645_grp, voicegroup_qz_songs
	.equ	mus_qz_645_pri, 0
	.equ	mus_qz_645_rev, reverb_set+0
	.equ	mus_qz_645_mvl, 127
	.equ	mus_qz_645_key, 0
	.equ	mus_qz_645_tbs, 1
	.equ	mus_qz_645_exg, 1
	.equ	mus_qz_645_cmp, 1

	.section .rodata
	.global	mus_qz_645
	.p2align	2
/* org 0x09692E90 */

/********************** Track  1 **********************/

mus_qz_645_1:
	.byte		KEYSH , mus_qz_645_key+0
	.byte		TEMPO , 120*mus_qz_645_tbs/2
	.byte		VOICE , 29
	.byte		VOL   , 68*mus_qz_645_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte		VOL   , 68*mus_qz_645_mvl/mxv
	.byte		PAN   , c_v+38
	.byte	W96
	.byte		VOL   , 68*mus_qz_645_mvl/mxv
	.byte	W96
mus_qz_645_1_B1:
	.byte		PAN   , c_v+16
	.byte		N15   , Ds3 , v100
	.byte	W16
	.byte		N03   , Dn3
	.byte	W08
	.byte		        Ds3
	.byte	W24
	.byte		N15   , Gs3
	.byte	W16
	.byte		N03   , Gn3
	.byte	W08
	.byte		        Gs3
	.byte	W24
	.byte		N15   , Fn3
	.byte	W16
	.byte		N07   , En3
	.byte	W08
	.byte		N15   , Fn3
	.byte	W16
	.byte		N07   , Gn3
	.byte	W08
	.byte		N23   , Gs3
	.byte	W48
	.byte		N15   , As3
	.byte	W16
	.byte		N03   , Gs3
	.byte	W08
	.byte		        Gn3
	.byte	W24
	.byte	N15
	.byte	W16
	.byte		N03   , Fn3
	.byte	W08
	.byte		        Ds3
	.byte	W24
	.byte		        Gs3
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		        Gn3
	.byte	W08
	.byte		N15   , Gs3
	.byte	W16
	.byte		N07   , Fn3
	.byte	W08
	.byte		N23   , Ds3
	.byte	W48
mus_qz_645_1_000:
	.byte		N15   , Gs3 , v100
	.byte	W16
	.byte		N07   , Gn3
	.byte	W08
	.byte		N15   , Gs3
	.byte	W16
	.byte		N07   , As3
	.byte	W08
	.byte		N23   , Gs3
	.byte	W48
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_645_1_000
	.byte		N03   , Gn3 , v100
	.byte	W16
	.byte		        Gs3
	.byte	W08
	.byte		        Gn3
	.byte	W16
	.byte		        Gs3
	.byte	W08
	.byte		        As3
	.byte	W16
	.byte		        Fn3
	.byte	W08
	.byte		        Gn3
	.byte	W16
	.byte		        Gs3
	.byte	W08
	.byte		VOL   , 68*mus_qz_645_mvl/mxv
	.byte		N66   , As3 , v100 , gtp1
	.byte	W06
	.byte		VOL   , 64*mus_qz_645_mvl/mxv
	.byte	W02
	.byte		        62*mus_qz_645_mvl/mxv
	.byte	W04
	.byte		        61*mus_qz_645_mvl/mxv
	.byte	W03
	.byte		        61*mus_qz_645_mvl/mxv
	.byte	W03
	.byte		        56*mus_qz_645_mvl/mxv
	.byte	W02
	.byte		        55*mus_qz_645_mvl/mxv
	.byte	W18
	.byte		        58*mus_qz_645_mvl/mxv
	.byte	W04
	.byte		        61*mus_qz_645_mvl/mxv
	.byte	W02
	.byte		        64*mus_qz_645_mvl/mxv
	.byte	W04
	.byte		        68*mus_qz_645_mvl/mxv
	.byte	W44
	.byte		        61*mus_qz_645_mvl/mxv
	.byte	W04
	.byte		        55*mus_qz_645_mvl/mxv
	.byte		N36   , Gs3 , v100 , gtp3
	.byte		N36   , Cn4 , v100 , gtp3
	.byte	W40
	.byte		N07   , As3
	.byte		N07   , Cs4
	.byte	W08
	.byte		N23   , Cn4
	.byte		N23   , Ds4
	.byte	W24
	.byte		        As3
	.byte		N23   , Cs4
	.byte	W24
mus_qz_645_1_001:
	.byte		N23   , Gs3 , v100
	.byte		N23   , Cn4
	.byte	W24
	.byte		        Gn3
	.byte		N23   , As3
	.byte	W24
	.byte		        Fn3
	.byte		N23   , Gs3
	.byte	W24
	.byte		        Ds3
	.byte		N23   , Gn3
	.byte	W24
	.byte	PEND
	.byte		        Cs3
	.byte		N23   , Fn3
	.byte	W48
	.byte		        Ds3
	.byte		N23   , Gn3
	.byte	W48
	.byte		N72   , Fn3 , v100 , gtp3
	.byte		N72   , Gs3 , v100 , gtp3
	.byte	W36
	.byte		VOL   , 55*mus_qz_645_mvl/mxv
	.byte	W06
	.byte		        52*mus_qz_645_mvl/mxv
	.byte	W02
	.byte		        55*mus_qz_645_mvl/mxv
	.byte	W04
	.byte		        56*mus_qz_645_mvl/mxv
	.byte	W02
	.byte		        57*mus_qz_645_mvl/mxv
	.byte	W04
	.byte		        61*mus_qz_645_mvl/mxv
	.byte	W24
	.byte	W03
	.byte		        55*mus_qz_645_mvl/mxv
	.byte	W15
	.byte		N36   , Gn3 , v100 , gtp3
	.byte		N36   , As3 , v100 , gtp3
	.byte	W40
	.byte		N07   , Gs3
	.byte		N07   , Cn4
	.byte	W08
	.byte		N23   , Gn3
	.byte		N23   , As3
	.byte	W24
	.byte		        Fn3
	.byte		N23   , Gs3
	.byte	W24
	.byte		        Ds3
	.byte		N23   , Gn3
	.byte	W24
	.byte		        Cs3
	.byte		N23   , Fn3
	.byte	W24
	.byte		        Cn3
	.byte		N23   , Ds3
	.byte	W24
	.byte		        As2
	.byte		N23   , Cs3
	.byte	W24
	.byte		N19   , Gs2
	.byte		N19   , Cn3
	.byte	W24
	.byte		N15   , Gs2
	.byte		N15   , Cn3
	.byte	W16
	.byte		N03   , Gn2
	.byte		N03   , Bn2
	.byte	W08
	.byte		N11   , Gs2
	.byte		N11   , Cn3
	.byte	W24
	.byte		        As2
	.byte		N11   , Cs3
	.byte	W24
	.byte		N72   , Gn2 , v100 , gtp3
	.byte		N72   , Ds3 , v100 , gtp3
	.byte	W24
	.byte		VOL   , 55*mus_qz_645_mvl/mxv
	.byte	W02
	.byte		        52*mus_qz_645_mvl/mxv
	.byte	W06
	.byte		        50*mus_qz_645_mvl/mxv
	.byte	W04
	.byte		        52*mus_qz_645_mvl/mxv
	.byte	W08
	.byte		        55*mus_qz_645_mvl/mxv
	.byte	W04
	.byte		        56*mus_qz_645_mvl/mxv
	.byte	W02
	.byte		        59*mus_qz_645_mvl/mxv
	.byte	W04
	.byte		        61*mus_qz_645_mvl/mxv
	.byte	W36
	.byte	W03
	.byte		        55*mus_qz_645_mvl/mxv
	.byte	W03
	.byte		N36   , Gs3 , v100 , gtp3
	.byte		N36   , Cn4 , v100 , gtp3
	.byte	W40
	.byte		N07   , As3
	.byte		N07   , Cs4
	.byte	W08
	.byte		N23   , Cn4
	.byte		N23   , Ds4
	.byte	W24
	.byte		        As3
	.byte		N23   , Cs4
	.byte	W24
	.byte	PATT
	 ptrvalue	mus_qz_645_1_001
	.byte		N23   , Fn3 , v100
	.byte		N23   , Gs3
	.byte	W48
	.byte	N23
	.byte		N23   , Cs4
	.byte	W48
	.byte		N07   , Dn4
	.byte		N07   , Fn4
	.byte	W24
	.byte		N11   , Fn3
	.byte		N11   , Gs3
	.byte	W24
	.byte		        Ds3
	.byte		N11   , Gn3
	.byte	W24
	.byte		        Dn3
	.byte		N11   , Fn3
	.byte	W24
	.byte		N15   , Cn3
	.byte		N15   , Ds3
	.byte	W48
	.byte		        Dn3
	.byte		N15   , Fn3
	.byte	W48
	.byte		        Cs3
	.byte		N15   , Gn3
	.byte	W48
	.byte		        Cn3
	.byte		N15   , Gs3
	.byte	W48
	.byte		N11   , Fn3
	.byte		N11   , As3
	.byte	W24
	.byte		        Gs3
	.byte		N11   , Cs4
	.byte	W24
	.byte	N11
	.byte		N11   , Fn4
	.byte	W24
	.byte		        Cs3
	.byte		N11   , Fn3
	.byte	W24
	.byte		        Dn3
	.byte		N11   , Gn3
	.byte	W24
	.byte		        Dn3
	.byte		N11   , Gs3
	.byte	W24
	.byte		        Dn3
	.byte		N11   , Gn3
	.byte	W24
	.byte		        Bn2
	.byte		N11   , Fn3
	.byte	W24
	.byte		N03   , Cn3
	.byte		N03   , Ds3
	.byte	W24
	.byte		        Bn2
	.byte		N03   , Dn3
	.byte	W24
	.byte		        Cn3
	.byte		N03   , Ds3
	.byte	W24
	.byte		        Cs3
	.byte		N03   , Fn3
	.byte	W24
	.byte		        As2
	.byte		N03   , Ds3
	.byte	W24
	.byte		        As2
	.byte		N03   , Ds3
	.byte	W16
	.byte		        Gs3
	.byte		N03   , Cs4
	.byte	W08
	.byte		N23   , Gn3
	.byte		N23   , Cn4
	.byte	W24
	.byte		        Cs3
	.byte		N23   , Gn3
	.byte	W24
	.byte		VOL   , 47*mus_qz_645_mvl/mxv
	.byte		N68   , Cn3 , v100 , gtp3
	.byte		N68   , Gs3 , v100 , gtp3
	.byte	W36
	.byte		VOL   , 44*mus_qz_645_mvl/mxv
	.byte	W02
	.byte		        43*mus_qz_645_mvl/mxv
	.byte	W04
	.byte		        41*mus_qz_645_mvl/mxv
	.byte	W02
	.byte		        36*mus_qz_645_mvl/mxv
	.byte	W04
	.byte		        34*mus_qz_645_mvl/mxv
	.byte	W02
	.byte		        32*mus_qz_645_mvl/mxv
	.byte	W04
	.byte		        30*mus_qz_645_mvl/mxv
	.byte	W02
	.byte		        23*mus_qz_645_mvl/mxv
	.byte	W04
	.byte		        21*mus_qz_645_mvl/mxv
	.byte	W02
	.byte		        15*mus_qz_645_mvl/mxv
	.byte	W04
	.byte		        9*mus_qz_645_mvl/mxv
	.byte	W02
	.byte		        3*mus_qz_645_mvl/mxv
	.byte	W04
	.byte		        2*mus_qz_645_mvl/mxv
	.byte	W02
	.byte		        0*mus_qz_645_mvl/mxv
	.byte	W04
	.byte		        0*mus_qz_645_mvl/mxv
	.byte	W18
	.byte	W96
	.byte		        61*mus_qz_645_mvl/mxv
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_645_1_B1
	.byte	FINE

/********************** Track  2 **********************/

mus_qz_645_2:
	.byte		KEYSH , mus_qz_645_key+0
	.byte		VOICE , 13
	.byte		VOL   , 68*mus_qz_645_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte		PRIO  , 53
	.byte		VOL   , 68*mus_qz_645_mvl/mxv
	.byte		PAN   , c_v-32
	.byte		        c_v+19
	.byte		VOL   , 13*mus_qz_645_mvl/mxv
	.byte		        30*mus_qz_645_mvl/mxv
	.byte	W02
	.byte		N03   , Cn4 , v100
	.byte	W16
	.byte		        Bn3
	.byte	W08
	.byte		        Cn4
	.byte	W08
	.byte		        Cs4
	.byte	W08
	.byte		        Ds4
	.byte	W08
	.byte		N15   , Fs4
	.byte	W16
	.byte		N03   , Fn4
	.byte	W04
	.byte		        En4
	.byte	W04
	.byte		N15   , Ds4
	.byte	W16
	.byte		N03   , Fn4
	.byte	W04
	.byte		        Gn4
	.byte	W02
	.byte	W02
	.byte		        Gs4
	.byte	W92
	.byte	W02
mus_qz_645_2_B1:
	.byte		VOICE , 29
	.byte		PAN   , c_v-32
	.byte	W04
	.byte		N15   , Ds3 , v100
	.byte	W16
	.byte		N03   , Dn3
	.byte	W08
	.byte		        Ds3
	.byte	W24
	.byte		N15   , Gs3
	.byte	W16
	.byte		N03   , Gn3
	.byte	W08
	.byte		        Gs3
	.byte	W20
	.byte	W04
	.byte		N15   , Fn3
	.byte	W16
	.byte		N07   , En3
	.byte	W08
	.byte		N15   , Fn3
	.byte	W16
	.byte		N07   , Gn3
	.byte	W08
	.byte		N23   , Gs3
	.byte	W44
	.byte	W04
	.byte		N15   , As3
	.byte	W16
	.byte		N03   , Gs3
	.byte	W08
	.byte		        Gn3
	.byte	W24
	.byte	N15
	.byte	W16
	.byte		N03   , Fn3
	.byte	W08
	.byte		        Ds3
	.byte	W20
	.byte	W04
	.byte		        Gs3
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		        Gn3
	.byte	W08
	.byte		N15   , Gs3
	.byte	W16
	.byte		N07   , Fn3
	.byte	W08
	.byte		N23   , Ds3
	.byte	W44
mus_qz_645_2_000:
	.byte	W04
	.byte		N15   , Gs3 , v100
	.byte	W16
	.byte		N07   , Gn3
	.byte	W08
	.byte		N15   , Gs3
	.byte	W16
	.byte		N07   , As3
	.byte	W08
	.byte		N23   , Gs3
	.byte	W44
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_645_2_000
	.byte	W04
	.byte		N03   , Gn3 , v100
	.byte	W16
	.byte		        Gs3
	.byte	W08
	.byte		        Gn3
	.byte	W16
	.byte		        Gs3
	.byte	W08
	.byte		        As3
	.byte	W16
	.byte		        Fn3
	.byte	W08
	.byte		        Gn3
	.byte	W16
	.byte		        Gs3
	.byte	W04
	.byte	W04
	.byte		N66   , As3 , v100 , gtp1
	.byte	W92
	.byte		VOL   , 33*mus_qz_645_mvl/mxv
	.byte	W04
	.byte		N36   , Cn4 , v100 , gtp3
	.byte	W40
	.byte		N07   , Cs4
	.byte	W08
	.byte		N23   , Ds4
	.byte	W24
	.byte		        Cs4
	.byte	W20
mus_qz_645_2_001:
	.byte	W04
	.byte		N23   , Cn4 , v100
	.byte	W24
	.byte		        As3
	.byte	W24
	.byte		        Gs3
	.byte	W24
	.byte		        Gn3
	.byte	W20
	.byte	PEND
	.byte	W04
	.byte		        Fn3
	.byte	W48
	.byte		        Gn3
	.byte	W44
	.byte	W04
	.byte		N72   , Gs3 , v100 , gtp3
	.byte	W92
	.byte	W04
	.byte		N36   , As3 , v100 , gtp3
	.byte	W40
	.byte		N07   , Cn4
	.byte	W08
	.byte		N23   , As3
	.byte	W24
	.byte		        Gs3
	.byte	W20
	.byte	W04
	.byte		        Gn3
	.byte	W24
	.byte		        Fn3
	.byte	W24
	.byte		        Ds3
	.byte	W24
	.byte		        Cs3
	.byte	W20
	.byte	W04
	.byte		N19   , Cn3
	.byte	W24
	.byte	N15
	.byte	W16
	.byte		N03   , Bn2
	.byte	W08
	.byte		N11   , Cn3
	.byte	W24
	.byte		        Cs3
	.byte	W20
	.byte	W04
	.byte		N72   , Ds3 , v100 , gtp3
	.byte	W92
	.byte	W04
	.byte		N36   , Cn4 , v100 , gtp3
	.byte	W40
	.byte		N07   , Cs4
	.byte	W08
	.byte		N23   , Ds4
	.byte	W24
	.byte		        Cs4
	.byte	W20
	.byte	PATT
	 ptrvalue	mus_qz_645_2_001
	.byte	W04
	.byte		N23   , Gs3 , v100
	.byte	W48
	.byte		        Cs4
	.byte	W44
	.byte	W04
	.byte		N07   , Fn4
	.byte	W24
	.byte		N11   , Gs3
	.byte	W24
	.byte		        Gn3
	.byte	W24
	.byte		        Fn3
	.byte	W20
	.byte	W04
	.byte		N15   , Ds3
	.byte	W48
	.byte		        Fn3
	.byte	W44
	.byte	W04
	.byte		        Gn3
	.byte	W48
	.byte		        Gs3
	.byte	W44
	.byte	W04
	.byte		N11   , As3
	.byte	W24
	.byte		        Cs4
	.byte	W24
	.byte		        Fn4
	.byte	W24
	.byte		        Fn3
	.byte	W20
	.byte	W04
	.byte		        Gn3
	.byte	W24
	.byte		        Gs3
	.byte	W24
	.byte		        Gn3
	.byte	W24
	.byte		        Fn3
	.byte	W20
	.byte	W04
	.byte		N03   , Ds3
	.byte	W24
	.byte		        Dn3
	.byte	W24
	.byte		        Ds3
	.byte	W24
	.byte		        Fn3
	.byte	W20
	.byte	W04
	.byte		        Ds3
	.byte	W24
	.byte	N03
	.byte	W16
	.byte		        Cs4
	.byte	W08
	.byte		N23   , Cn4
	.byte	W24
	.byte		        Gn3
	.byte	W20
	.byte	W04
	.byte		N68   , Gs3 , v100 , gtp3
	.byte	W28
	.byte		VOL   , 30*mus_qz_645_mvl/mxv
	.byte	W08
	.byte		        26*mus_qz_645_mvl/mxv
	.byte	W04
	.byte		        21*mus_qz_645_mvl/mxv
	.byte	W04
	.byte		        15*mus_qz_645_mvl/mxv
	.byte	W04
	.byte		        14*mus_qz_645_mvl/mxv
	.byte	W04
	.byte		        10*mus_qz_645_mvl/mxv
	.byte	W04
	.byte		        8*mus_qz_645_mvl/mxv
	.byte	W04
	.byte		        6*mus_qz_645_mvl/mxv
	.byte	W04
	.byte		        4*mus_qz_645_mvl/mxv
	.byte	W04
	.byte		        2*mus_qz_645_mvl/mxv
	.byte	W04
	.byte		        0*mus_qz_645_mvl/mxv
	.byte	W04
	.byte		        0*mus_qz_645_mvl/mxv
	.byte	W16
	.byte	W96
	.byte		        30*mus_qz_645_mvl/mxv
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_645_2_B1
	.byte	FINE

/********************** Track  3 **********************/

mus_qz_645_3:
	.byte		KEYSH , mus_qz_645_key+0
	.byte		VOICE , 5
	.byte		VOL   , 68*mus_qz_645_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte		PRIO  , 60
	.byte		VOL   , 68*mus_qz_645_mvl/mxv
	.byte		        30*mus_qz_645_mvl/mxv
	.byte		PAN   , c_v-20
	.byte		        c_v-25
	.byte	W96
	.byte	W96
mus_qz_645_3_B1:
	.byte	W24
	.byte		N15   , Ds4 , v100
	.byte	W16
	.byte		N07   , Gs4
	.byte	W08
	.byte		N03   , Cn5
	.byte	W24
	.byte		N15   , Cn4
	.byte	W16
	.byte		N07   , Ds4
	.byte	W08
	.byte		N03   , Fn4
	.byte	W16
	.byte		        En4
	.byte	W08
	.byte		        Fn4
	.byte	W16
	.byte		        Gs4
	.byte	W08
	.byte		N23   , Cs5
	.byte	W24
	.byte		        Fn4
	.byte	W24
	.byte		        Gn4
	.byte	W24
	.byte		N15   , Ds4
	.byte	W16
	.byte		N07   , Gn4
	.byte	W08
	.byte		N03   , As4
	.byte	W24
	.byte		N15   , Ds4
	.byte	W16
	.byte		N07   , Cs5
	.byte	W08
	.byte		N03   , Cn5
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		        Bn4
	.byte	W08
	.byte		N15   , Cn5
	.byte	W16
	.byte		N07   , Gs4
	.byte	W08
	.byte		N15   , Ds4
	.byte	W16
	.byte		N07   , Gs4
	.byte	W08
	.byte		N03   , Cn4
	.byte	W08
	.byte		        Cs4
	.byte	W08
	.byte		        Ds4
	.byte	W08
	.byte		N07   , Fn4
	.byte	W16
	.byte	N07
	.byte	W08
	.byte		N03   , Cs4
	.byte	W08
	.byte		        Fn4
	.byte	W08
	.byte		        Gs4
	.byte	W08
	.byte		N23   , Cs5
	.byte	W24
	.byte		        Fn4
	.byte	W24
	.byte		N07   , Ds4
	.byte	W16
	.byte	N07
	.byte	W08
	.byte		N03   , Cn4
	.byte	W08
	.byte		        Ds4
	.byte	W08
	.byte		        As4
	.byte	W08
	.byte		N23   , Cn5
	.byte	W24
	.byte		        Ds4
	.byte	W24
	.byte		        Dn4
	.byte	W24
	.byte	N23
	.byte	W24
	.byte		        As3
	.byte	W24
	.byte		        Fn4
	.byte	W24
	.byte		N03   , Ds4
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		        Fn4
	.byte	W08
	.byte		        Ds4
	.byte	W08
	.byte		        Gn4
	.byte	W08
	.byte		        As4
	.byte	W08
	.byte		        Ds5
	.byte	W16
	.byte	N03
	.byte	W08
	.byte		N23   , Ds4
	.byte	W24
	.byte	W24
	.byte		        Gs4
	.byte	W24
	.byte		        Cn5
	.byte	W24
	.byte		        Cs5
	.byte	W24
	.byte		        Ds5
	.byte	W24
	.byte		        Ds4
	.byte	W24
	.byte		        Cn5
	.byte	W24
	.byte		        Ds5
	.byte	W24
	.byte		N44   , Cs5 , v100 , gtp3
	.byte	W48
	.byte		        Fn4 , v100 , gtp3
	.byte	W48
	.byte		N23   , Gs4
	.byte	W24
	.byte		        Cs5
	.byte	W24
	.byte		N15   , Fn4
	.byte	W16
	.byte	N07
	.byte	W08
	.byte		N23   , Gs4
	.byte	W24
	.byte		        Gn4
	.byte	W24
	.byte		N44   , Ds4 , v100 , gtp3
	.byte	W48
	.byte		N23   , Gn4
	.byte	W24
	.byte		N44   , Cs5 , v100 , gtp3
	.byte	W48
	.byte		        Ds4 , v100 , gtp3
	.byte	W48
	.byte		N23   , Gs4
	.byte	W24
	.byte		        Ds4
	.byte	W24
	.byte		N44   , Cn5 , v100 , gtp3
	.byte	W48
	.byte		N23   , Gn4
	.byte	W24
	.byte		        Ds4
	.byte	W24
	.byte		        Cs5
	.byte	W24
	.byte		        Ds4
	.byte	W24
	.byte		N44   , Gs4 , v100 , gtp3
	.byte	W48
	.byte	N23
	.byte	W24
	.byte		        As4
	.byte	W24
	.byte		N44   , Cn5 , v100 , gtp3
	.byte	W48
	.byte		        Ds5 , v100 , gtp3
	.byte	W48
	.byte		N23   , Cs5
	.byte	W24
	.byte		        Cs4
	.byte	W24
	.byte		        Cs5
	.byte	W24
	.byte		        Cs4
	.byte	W24
	.byte		        Dn5
	.byte	W24
	.byte		        Dn4
	.byte	W24
	.byte		        Ds4
	.byte	W24
	.byte		        Fn4
	.byte	W24
mus_qz_645_3_000:
	.byte		N15   , Ds4 , v100
	.byte	W16
	.byte	N07
	.byte	W08
	.byte		N23   , Ds5
	.byte	W24
	.byte		N15   , Ds4
	.byte	W16
	.byte	N07
	.byte	W08
	.byte		N23   , Ds5
	.byte	W24
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_645_3_000
	.byte		N23   , Fn4 , v100
	.byte	W24
	.byte	N23
	.byte	W24
	.byte		N44   , Cs5 , v100 , gtp3
	.byte	W48
	.byte		N23   , Fn4
	.byte	W24
	.byte	N23
	.byte	W24
	.byte		N44   , Dn5 , v100 , gtp3
	.byte	W48
	.byte		N03   , Ds5
	.byte	W24
	.byte		        Dn5
	.byte	W24
	.byte		        Ds5
	.byte	W24
	.byte		        Fn5
	.byte	W24
	.byte		N23   , Ds5
	.byte	W24
	.byte		        Ds4
	.byte	W48
	.byte	N23
	.byte	W24
	.byte		N03   , Gs4
	.byte	W16
	.byte		        Ds5
	.byte	W08
	.byte		        Cn5
	.byte	W08
	.byte		        Cs5
	.byte	W08
	.byte		        Ds5
	.byte	W08
	.byte		N15   , Fs5
	.byte	W16
	.byte		N03   , Fn5
	.byte	W04
	.byte		        En5
	.byte	W04
	.byte		N15   , Ds5
	.byte	W16
	.byte		N03   , Fn5
	.byte	W04
	.byte		        Gn5
	.byte	W04
	.byte		        Gs5
	.byte	W16
	.byte		        Ds5
	.byte	W08
	.byte		        Cn5
	.byte	W08
	.byte		        Cs5
	.byte	W08
	.byte		        Ds5
	.byte	W08
	.byte		N15   , Fs5
	.byte	W16
	.byte		N03   , Fn5
	.byte	W04
	.byte		        En5
	.byte	W04
	.byte		N15   , Ds5
	.byte	W16
	.byte		N03   , Fn5
	.byte	W04
	.byte		        Gn5
	.byte	W04
	.byte		        Gs5
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_645_3_B1
	.byte	FINE

/********************** Track  4 **********************/

mus_qz_645_4:
	.byte		KEYSH , mus_qz_645_key+0
	.byte		VOICE , 13
	.byte		VOL   , 68*mus_qz_645_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte		PRIO  , 60
	.byte		VOL   , 68*mus_qz_645_mvl/mxv
	.byte		PAN   , c_v-22
	.byte		VOL   , 47*mus_qz_645_mvl/mxv
	.byte		N03   , Cn4 , v100
	.byte	W16
	.byte		        Bn3
	.byte	W08
	.byte		        Cn4
	.byte	W08
	.byte		        Cs4
	.byte	W08
	.byte		        Ds4
	.byte	W08
	.byte		N15   , Fs4
	.byte	W16
	.byte		N03   , Fn4
	.byte	W04
	.byte		        En4
	.byte	W04
	.byte		N15   , Ds4
	.byte	W16
	.byte		N03   , Fn4
	.byte	W04
	.byte		        Gn4
	.byte	W04
mus_qz_645_4_000:
	.byte		N03   , Gs4 , v100
	.byte	W12
	.byte		VOL   , 40*mus_qz_645_mvl/mxv
	.byte	W04
	.byte		N03   , Cn4
	.byte		N03   , Ds4
	.byte	W08
	.byte		        Ds3
	.byte	W16
	.byte		        Cn4
	.byte		N03   , Ds4
	.byte	W08
	.byte		        Gs3
	.byte	W16
	.byte		        Cn4
	.byte		N03   , Ds4
	.byte	W08
	.byte		        Ds3
	.byte	W16
	.byte		        Cn4
	.byte		N03   , Ds4
	.byte	W08
	.byte	PEND
mus_qz_645_4_B1:
	.byte		N03   , Gs3 , v100
	.byte	W16
	.byte		        Cn4
	.byte		N03   , Ds4
	.byte	W08
	.byte		        Ds3
	.byte	W16
	.byte		        Cn4
	.byte		N03   , Ds4
	.byte	W08
	.byte		        Gs3
	.byte	W16
	.byte		        Cn4
	.byte		N03   , Ds4
	.byte	W08
	.byte		        Ds3
	.byte	W16
	.byte		        Cn4
	.byte		N03   , Ds4
	.byte	W08
	.byte	PEND
	.byte		        Fn3
	.byte	W16
	.byte		        Gs3
	.byte		N03   , Cs4
	.byte	W08
	.byte		        Cs3
	.byte	W16
	.byte		        Gs3
	.byte		N03   , Cs4
	.byte	W08
	.byte		        Fn3
	.byte	W16
	.byte		        Gs3
	.byte		N03   , Cs4
	.byte	W08
	.byte		        Cs3
	.byte	W16
	.byte		        Gs3
	.byte		N03   , Cs4
	.byte	W08
	.byte		        Gn3
	.byte	W16
	.byte		        As3
	.byte		N03   , Cs4
	.byte	W08
	.byte		        Ds3
	.byte	W16
	.byte		        As3
	.byte		N03   , Cs4
	.byte	W08
	.byte		        Gn3
	.byte	W16
	.byte		        As3
	.byte		N03   , Cs4
	.byte	W08
	.byte		        Ds3
	.byte	W16
	.byte		        As3
	.byte		N03   , Cs4
	.byte	W08
	.byte	PATT
	 ptrvalue	mus_qz_645_4_B1
	.byte		N03   , Fn3 , v100
	.byte	W16
	.byte		        Gs3
	.byte		N03   , Cs4
	.byte	W08
	.byte		        Fn3
	.byte	W16
	.byte		        Gs3
	.byte		N03   , Cs4
	.byte	W08
	.byte		        Cs3
	.byte	W08
	.byte		        Fn3
	.byte	W08
	.byte		        Gs3
	.byte	W08
	.byte	N03
	.byte		N03   , Cs4
	.byte	W24
	.byte		        Ds3
	.byte	W16
	.byte		        Gs3
	.byte		N03   , Cn4
	.byte	W08
	.byte		        Ds3
	.byte	W16
	.byte		        Gs3
	.byte		N03   , Cn4
	.byte	W08
	.byte		        Cn3
	.byte	W08
	.byte		        Ds3
	.byte	W08
	.byte		        Gs3
	.byte	W08
	.byte	N03
	.byte		N03   , Cn4
	.byte	W24
	.byte		        Fn3
	.byte	W16
	.byte		        Gs3
	.byte		N03   , Dn4
	.byte	W08
	.byte		        Fn3
	.byte	W16
	.byte		        Gs3
	.byte		N03   , Dn4
	.byte	W08
	.byte		        As2
	.byte	W16
	.byte		        Gs3
	.byte		N03   , Dn4
	.byte	W08
	.byte		        As2
	.byte	W16
	.byte		        Gs3
	.byte		N03   , Dn4
	.byte	W08
	.byte		        Gn3
	.byte	W16
	.byte		        As3
	.byte		N03   , Ds4
	.byte	W08
	.byte		        Ds3
	.byte	W08
	.byte		        Gn3
	.byte	W08
	.byte		        As3
	.byte	W08
	.byte		        Ds3
	.byte	W16
	.byte		        As3
	.byte		N03   , Ds4
	.byte	W08
	.byte		        Ds3
	.byte	W16
	.byte		        Bn3
	.byte		N03   , Ds4
	.byte	W08
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		VOL   , 47*mus_qz_645_mvl/mxv
	.byte		N03   , Cn4
	.byte	W16
	.byte		        Bn3
	.byte	W08
	.byte		        Cn4
	.byte	W08
	.byte		        Cs4
	.byte	W08
	.byte		        Ds4
	.byte	W08
	.byte		N15   , Fs4
	.byte	W16
	.byte		N03   , Fn4
	.byte	W04
	.byte		        En4
	.byte	W04
	.byte		N15   , Ds4
	.byte	W16
	.byte		N03   , Fn4
	.byte	W04
	.byte		        Gn4
	.byte	W04
	.byte		        Gs4
	.byte	W16
	.byte		        Ds4
	.byte	W08
	.byte		        Cn4
	.byte	W08
	.byte		        Cs4
	.byte	W08
	.byte		        Ds4
	.byte	W08
	.byte		N15   , Fs4
	.byte	W16
	.byte		N03   , Fn4
	.byte	W04
	.byte		        En4
	.byte	W04
	.byte		N15   , Ds4
	.byte	W16
	.byte		N03   , Fn4
	.byte	W04
	.byte		        Gn4
	.byte	W04
	.byte	PATT
	 ptrvalue	mus_qz_645_4_000
	.byte	PATT
	 ptrvalue	mus_qz_645_4_B1
	.byte	GOTO
	 ptrvalue	mus_qz_645_4_B1
	.byte	FINE

/********************** Track  5 **********************/

mus_qz_645_5:
	.byte		KEYSH , mus_qz_645_key+0
	.byte		VOL   , 68*mus_qz_645_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte		PRIO  , 60
	.byte		VOL   , 68*mus_qz_645_mvl/mxv
	.byte	W96
	.byte	W96
mus_qz_645_5_B1:
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W88
	.byte		VOICE , 30
	.byte	W04
	.byte		PAN   , c_v-25
	.byte		VOL   , 47*mus_qz_645_mvl/mxv
	.byte	W04
	.byte		N15   , Cn3 , v100
	.byte		N15   , Ds3
	.byte	W16
	.byte		N07   , Cn3
	.byte		N07   , Ds3
	.byte	W32
	.byte	N15
	.byte		N15   , Gs3
	.byte	W16
	.byte		N07   , Ds3
	.byte		N07   , Gs3
	.byte	W32
	.byte	W24
	.byte		N15   , Ds3
	.byte		N15   , As3
	.byte	W16
	.byte		N07   , Ds3
	.byte		N07   , As3
	.byte	W08
	.byte		N44   , Gs3 , v100 , gtp3
	.byte		N44   , Cn4 , v100 , gtp3
	.byte	W48
	.byte		N23   , Fn3
	.byte		N23   , Cs4
	.byte	W24
	.byte		N07   , Fn3
	.byte	W08
	.byte		        Gs3
	.byte	W08
	.byte		        Cs4
	.byte	W08
	.byte		N23   , Gn3
	.byte		N23   , Ds4
	.byte	W24
	.byte		N07   , Gs3
	.byte	W08
	.byte		        Cs4
	.byte	W08
	.byte		        Ds4
	.byte	W08
	.byte		N23   , Cs4
	.byte		N23   , Fn4
	.byte	W24
	.byte		N07   , Cs4
	.byte	W08
	.byte		        Gs3
	.byte	W08
	.byte		        Cs4
	.byte	W08
	.byte	N23
	.byte		N23   , Fn4
	.byte	W24
	.byte		        Cs3
	.byte		N23   , Gs3
	.byte	W24
	.byte		N15   , Ds3
	.byte		N15   , Gn3
	.byte	W16
	.byte		N07   , Ds3
	.byte		N07   , Gn3
	.byte	W32
	.byte		N15   , Ds3
	.byte		N15   , Gn3
	.byte	W16
	.byte		N07   , Ds3
	.byte		N07   , Gn3
	.byte	W32
	.byte	W24
	.byte	N15
	.byte		N15   , As3
	.byte	W16
	.byte		N07   , Gn3
	.byte		N07   , As3
	.byte	W08
	.byte		N23   , Gn3
	.byte		N23   , Cn4
	.byte	W24
	.byte		        Ds3
	.byte		N23   , Gn3
	.byte	W24
	.byte		        Ds3
	.byte		N23   , Gs3
	.byte	W24
	.byte		N07   , Cn3
	.byte	W08
	.byte		        Ds3
	.byte	W08
	.byte		        Gs3
	.byte	W08
	.byte		N23   , Ds3
	.byte		N23   , Cn4
	.byte	W24
	.byte		N07   , Gs3
	.byte	W08
	.byte		        Cn4
	.byte	W08
	.byte		        Cs4
	.byte	W08
	.byte		N23   , Gn3
	.byte		N23   , Ds4
	.byte	W24
	.byte		N15   , As3
	.byte		N15   , Ds4
	.byte	W16
	.byte		N07   , Cs4
	.byte		N07   , Fn4
	.byte	W08
	.byte		N23   , As3
	.byte		N23   , Ds4
	.byte	W24
	.byte		        As3
	.byte		N23   , Cs4
	.byte	W24
	.byte		N44   , Gs3 , v100 , gtp3
	.byte		N44   , Ds4 , v100 , gtp3
	.byte	W48
	.byte		        Cn3 , v100 , gtp3
	.byte		N44   , Gs3 , v100 , gtp3
	.byte	W48
	.byte		N23   , Cn3
	.byte		N23   , Ds3
	.byte	W24
	.byte		N07   , Cn3
	.byte	W08
	.byte		        Ds3
	.byte	W08
	.byte		        Gn3
	.byte	W08
	.byte		N23   , Gs3
	.byte		N23   , Cn4
	.byte	W24
	.byte	N23
	.byte		N23   , Ds4
	.byte	W24
	.byte		        Cs4
	.byte		N23   , Fn4
	.byte	W24
	.byte		N07   , Gs3
	.byte	W08
	.byte		        Cs4
	.byte	W08
	.byte		        Fn4
	.byte	W08
	.byte		N23   , Ds4
	.byte		N23   , Gn4
	.byte	W24
	.byte		N07   , Fn3
	.byte	W08
	.byte		        Gs3
	.byte	W08
	.byte		        Cs4
	.byte	W08
	.byte		        Fn4
	.byte		N07   , Gs4
	.byte	W24
	.byte		        Fn3
	.byte	W24
	.byte		        Gn3
	.byte	W24
	.byte		        Gs3
	.byte	W24
	.byte		        Ds3
	.byte	W24
	.byte		N19   , Ds2
	.byte	W24
	.byte		N07   , Fn3
	.byte	W24
	.byte		N19   , Fn2
	.byte	W24
	.byte		N07   , Gn3
	.byte	W24
	.byte		N19   , Gn2
	.byte	W24
	.byte		N07   , Gs3
	.byte	W24
	.byte		N19   , Gs2
	.byte	W24
	.byte		VOL   , 47*mus_qz_645_mvl/mxv
	.byte	N11
	.byte		N11   , Cs3
	.byte	W24
	.byte		N07   , Gs2
	.byte		N07   , Cs3
	.byte	W08
	.byte	N07
	.byte		N07   , Fn3
	.byte	W08
	.byte	N07
	.byte		N07   , Gs3
	.byte	W08
	.byte	N07
	.byte		N07   , Cs4
	.byte	W24
	.byte		VOL   , 41*mus_qz_645_mvl/mxv
	.byte	N23
	.byte		N23   , Fn4
	.byte	W24
	.byte		VOL   , 47*mus_qz_645_mvl/mxv
	.byte		N11   , Gn2
	.byte		N11   , Dn3
	.byte	W24
	.byte		N07   , Gn2
	.byte		N07   , Dn3
	.byte	W08
	.byte	N07
	.byte		N07   , Fn3
	.byte	W08
	.byte	N07
	.byte		N07   , Gs3
	.byte	W08
	.byte		        Gn3
	.byte		N07   , Dn4
	.byte	W24
	.byte		VOL   , 41*mus_qz_645_mvl/mxv
	.byte	N23
	.byte		N23   , Fn4
	.byte	W24
	.byte		N07   , Ds4
	.byte		N07   , Gs4
	.byte	W24
	.byte		        Fn4
	.byte		N07   , Bn4
	.byte	W24
	.byte		        Gs4
	.byte		N07   , Cn5
	.byte	W24
	.byte		        Gs4
	.byte		N07   , Cs5
	.byte	W24
	.byte		N03   , Gn4
	.byte		N03   , Ds5
	.byte	W24
	.byte		VOL   , 47*mus_qz_645_mvl/mxv
	.byte		N03   , As3
	.byte		N03   , Ds4
	.byte	W24
	.byte		N07   , Ds3
	.byte	W08
	.byte		        Gn3
	.byte	W08
	.byte		        As3
	.byte	W08
	.byte		N23   , Gn3
	.byte		N23   , Cs4
	.byte	W24
	.byte		VOL   , 31*mus_qz_645_mvl/mxv
	.byte		N68   , Ds3 , v100 , gtp3
	.byte		N68   , Cn4 , v100 , gtp3
	.byte	W28
	.byte		VOL   , 26*mus_qz_645_mvl/mxv
	.byte	W08
	.byte		        20*mus_qz_645_mvl/mxv
	.byte	W08
	.byte		        16*mus_qz_645_mvl/mxv
	.byte	W04
	.byte		        11*mus_qz_645_mvl/mxv
	.byte	W04
	.byte		        8*mus_qz_645_mvl/mxv
	.byte	W04
	.byte		        5*mus_qz_645_mvl/mxv
	.byte	W04
	.byte		        1*mus_qz_645_mvl/mxv
	.byte	W04
	.byte		        0*mus_qz_645_mvl/mxv
	.byte	W32
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_645_5_B1
	.byte	FINE

/********************** Track  6 **********************/

mus_qz_645_6:
	.byte		KEYSH , mus_qz_645_key+0
	.byte		VOICE , 8
	.byte		VOL   , 68*mus_qz_645_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte		PRIO  , 60
	.byte		VOL   , 68*mus_qz_645_mvl/mxv
	.byte	W96
	.byte	W96
mus_qz_645_6_B1:
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		VOL   , 32*mus_qz_645_mvl/mxv
	.byte		PAN   , c_v+28
	.byte	W24
	.byte		N23   , Ds3 , v100
	.byte	W48
	.byte		        Fn3
	.byte	W24
	.byte	W24
	.byte		        Gn3
	.byte	W48
	.byte		        Gs3
	.byte	W24
	.byte	W48
	.byte		        Cs3
	.byte	W24
	.byte		        Cs4
	.byte	W24
	.byte	W48
	.byte		        Dn3
	.byte	W24
	.byte		        Dn4
	.byte	W24
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_645_6_B1
	.byte	FINE

/********************** Track  7 **********************/

mus_qz_645_7:
	.byte		KEYSH , mus_qz_645_key+0
	.byte		VOICE , 35
	.byte		VOL   , 68*mus_qz_645_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		BENDR , 6
	.byte		PRIO  , 64
	.byte		PRIO  , 60
	.byte		VOL   , 68*mus_qz_645_mvl/mxv
	.byte		        43*mus_qz_645_mvl/mxv
	.byte		PAN   , c_v+10
	.byte	W96
	.byte	W96
mus_qz_645_7_B1:
	.byte	W06
	.byte		BEND  , c_v+1
	.byte	W18
	.byte		N03   , Cn4 , v100
	.byte	W08
	.byte		        Cs4
	.byte	W08
	.byte		        Ds4
	.byte	W08
	.byte		N07   , Gs4
	.byte	W08
	.byte		N01   , As4
	.byte	W02
	.byte		        Gs4 , v056
	.byte	W02
	.byte		        As4
	.byte	W02
	.byte		        Gs4
	.byte	W02
	.byte		        As4
	.byte	W02
	.byte		N05   , Gs4 , v052
	.byte	W06
	.byte		N03   , Cn4 , v100
	.byte	W08
	.byte		        Ds4
	.byte	W08
	.byte		        Gs4
	.byte	W08
	.byte		N07   , Cs5
	.byte	W08
	.byte		N01   , Ds5
	.byte	W02
	.byte		        Cs5 , v056
	.byte	W02
	.byte		        Ds5
	.byte	W02
	.byte		        Cs5
	.byte	W02
	.byte		        Ds5
	.byte	W02
	.byte		N05   , Cs5 , v052
	.byte	W30
	.byte		N03   , Cs4 , v100
	.byte	W08
	.byte		        Fn4
	.byte	W08
	.byte		        Gs4
	.byte	W08
	.byte		N07   , Fn5
	.byte	W08
	.byte		N01   , Gn5
	.byte	W02
	.byte		        Fn5 , v056
	.byte	W02
	.byte		        Gn5
	.byte	W02
	.byte		        Fn5
	.byte	W02
	.byte		        Gn5
	.byte	W02
	.byte		N05   , Fn5 , v052
	.byte	W06
	.byte	W24
	.byte		N03   , Cs4 , v100
	.byte	W08
	.byte		        Ds4
	.byte	W08
	.byte		        Gn4
	.byte	W08
	.byte		N07   , As4
	.byte	W08
	.byte		N01   , Cn5
	.byte	W02
	.byte		        As4 , v056
	.byte	W02
	.byte		        Cn5
	.byte	W02
	.byte		        As4
	.byte	W02
	.byte		        Cn5
	.byte	W02
	.byte		N05   , As4 , v052
	.byte	W06
	.byte		N03   , Cs4 , v100
	.byte	W08
	.byte		        Ds4
	.byte	W08
	.byte		        As4
	.byte	W08
	.byte		N07   , Ds5
	.byte	W08
	.byte		N01   , Fn5
	.byte	W02
	.byte		        Ds5 , v056
	.byte	W02
	.byte		        Fn5
	.byte	W02
	.byte		        Ds5
	.byte	W02
	.byte		        Fn5
	.byte	W02
	.byte		N05   , Ds5 , v052
	.byte	W30
	.byte		N03   , Ds4 , v100
	.byte	W08
	.byte		        Gs4
	.byte	W08
	.byte		        Cn5
	.byte	W08
	.byte		N07   , Ds5
	.byte	W08
	.byte		N01   , Fn5
	.byte	W02
	.byte		        Ds5 , v056
	.byte	W02
	.byte		        Fn5
	.byte	W02
	.byte		        Ds5
	.byte	W02
	.byte		        Fn5
	.byte	W02
	.byte		N05   , Ds5 , v052
	.byte	W06
	.byte		N03   , Gs4 , v100
	.byte	W16
	.byte		        Cs5
	.byte	W08
	.byte		        Gs4
	.byte	W24
	.byte		        Fn4
	.byte	W08
	.byte		        Gs4
	.byte	W08
	.byte		        Cs5
	.byte	W08
	.byte		N07   , Fn5
	.byte	W08
	.byte		N01   , Gn5
	.byte	W02
	.byte		        Fn5 , v056
	.byte	W02
	.byte		        Gn5
	.byte	W02
	.byte		        Fn5
	.byte	W02
	.byte		        Gn5
	.byte	W02
	.byte		N05   , Fn5 , v052
	.byte	W06
	.byte		N03   , Gs4 , v100
	.byte	W16
	.byte		        Cn5
	.byte	W08
	.byte		        Gs4
	.byte	W24
	.byte		        Ds4
	.byte	W08
	.byte		        Gs4
	.byte	W08
	.byte		        Cn5
	.byte	W08
	.byte		N07   , Ds5
	.byte	W08
	.byte		N01   , Fn5
	.byte	W02
	.byte		        Ds5 , v056
	.byte	W02
	.byte		        Fn5
	.byte	W02
	.byte		        Ds5
	.byte	W02
	.byte		        Fn5
	.byte	W02
	.byte		N05   , Ds5 , v052
	.byte	W06
	.byte		N03   , Dn5 , v100
	.byte	W16
	.byte		        Cn5
	.byte	W08
	.byte		        Dn5
	.byte	W16
	.byte		        Cn5
	.byte	W08
	.byte		        As4
	.byte	W16
	.byte		        Cn5
	.byte	W08
	.byte		        Dn5
	.byte	W08
	.byte		        Ds5
	.byte	W08
	.byte		        Fn5
	.byte	W08
	.byte		N07   , Ds5
	.byte	W08
	.byte		N01   , Fn5
	.byte	W02
	.byte		        Ds5 , v056
	.byte	W02
	.byte		        Fn5
	.byte	W02
	.byte		        Ds5
	.byte	W02
	.byte		        Fn5
	.byte	W02
	.byte		        Ds5
	.byte	W02
	.byte		        Fn5
	.byte	W02
	.byte		        Ds5
	.byte	W02
	.byte		        Fn5
	.byte	W02
	.byte		        Ds5
	.byte	W02
	.byte		        Fn5
	.byte	W02
	.byte		        Ds5
	.byte	W02
	.byte		        Fn5
	.byte	W02
	.byte		        Ds5
	.byte	W02
	.byte		        Fn5
	.byte	W02
	.byte		        Ds5
	.byte	W02
	.byte		        Fn5
	.byte	W02
	.byte		        Ds5
	.byte	W02
	.byte		        Fn5
	.byte	W02
	.byte		        Ds5
	.byte	W02
	.byte		        Fn5
	.byte	W02
	.byte		N17   , Ds5
	.byte	W44
	.byte	W02
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_645_7_B1
	.byte	FINE

/********************** Track  8 **********************/

mus_qz_645_8:
	.byte		KEYSH , mus_qz_645_key+0
	.byte		VOICE , 19
	.byte		VOL   , 68*mus_qz_645_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte		PRIO  , 60
	.byte		VOL   , 68*mus_qz_645_mvl/mxv
	.byte		        87*mus_qz_645_mvl/mxv
	.byte		N03   , Gs1 , v127
	.byte	W16
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		        Gn1
	.byte	W08
	.byte		        Gs1
	.byte	W08
	.byte		N11   , Ds2
	.byte	W24
	.byte		N15   , Ds1
	.byte	W16
	.byte		N03   , Fn1
	.byte	W04
	.byte		        Gn1
	.byte	W04
mus_qz_645_8_000:
	.byte		N11   , Gs1 , v127
	.byte	W24
	.byte		        Ds1
	.byte	W24
	.byte		        Gs1
	.byte	W24
	.byte		        Ds1
	.byte	W24
	.byte	PEND
mus_qz_645_8_B1:
	.byte	PATT
	 ptrvalue	mus_qz_645_8_000
mus_qz_645_8_001:
	.byte		N11   , Cs1 , v127
	.byte	W24
	.byte		        Gs1
	.byte	W24
	.byte		        Cs1
	.byte	W24
	.byte		        Gs1
	.byte	W24
	.byte	PEND
mus_qz_645_8_002:
	.byte		N11   , Ds1 , v127
	.byte	W24
	.byte		        As1
	.byte	W24
	.byte		        Ds1
	.byte	W24
	.byte		        As1
	.byte	W24
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_645_8_000
	.byte		N11   , Cs1 , v127
	.byte	W24
	.byte	N11
	.byte	W72
	.byte		        Gs1
	.byte	W24
	.byte	N11
	.byte	W72
	.byte		        Dn1
	.byte	W24
	.byte		        As0
	.byte	W24
	.byte		        Cn1
	.byte	W24
	.byte		        Dn1
	.byte	W24
	.byte		        Ds1
	.byte	W24
	.byte		        As1
	.byte	W24
	.byte		        Ds2
	.byte	W24
	.byte		        Ds1
	.byte	W24
	.byte	PATT
	 ptrvalue	mus_qz_645_8_000
	.byte		N11   , Gs1 , v127
	.byte	W24
	.byte		        Ds1
	.byte	W24
	.byte		        Cn2
	.byte	W24
	.byte		        Ds1
	.byte	W24
	.byte	PATT
	 ptrvalue	mus_qz_645_8_001
	.byte		N11   , Cs2 , v127
	.byte	W24
	.byte		        Gs1
	.byte	W24
	.byte		        Fn1
	.byte	W24
	.byte		        Gs1
	.byte	W24
	.byte	PATT
	 ptrvalue	mus_qz_645_8_002
	.byte		N11   , Ds1 , v127
	.byte	W24
	.byte	N11
	.byte	W24
	.byte		        Fn1
	.byte	W24
	.byte		        Gn1
	.byte	W24
	.byte		        Gs1
	.byte	W24
	.byte		        Ds2
	.byte	W24
	.byte		        Gs1
	.byte	W24
	.byte		        Ds2
	.byte	W24
	.byte		        As1
	.byte	W24
	.byte		        Ds1
	.byte	W24
	.byte		        Fn1
	.byte	W24
	.byte		        Gn1
	.byte	W24
	.byte	PATT
	 ptrvalue	mus_qz_645_8_000
	.byte		N11   , Gs1 , v127
	.byte	W24
	.byte		        Ds1
	.byte	W24
	.byte		        Gs1
	.byte	W24
	.byte		        Cn2
	.byte	W24
	.byte		        Cs2
	.byte	W24
	.byte		        Gs1
	.byte	W24
	.byte		        Cs2
	.byte	W24
	.byte		        Gs1
	.byte	W24
	.byte		        Dn2
	.byte	W24
	.byte		        Dn1
	.byte	W24
	.byte		        Ds1
	.byte	W24
	.byte		        Fn1
	.byte	W24
mus_qz_645_8_003:
	.byte	W24
	.byte		N11   , Ds1 , v127
	.byte	W48
	.byte	N11
	.byte	W24
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_645_8_003
	.byte		N11   , Cs1 , v127
	.byte	W96
	.byte		        Dn1
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_645_8_003
	.byte		N11   , Ds1 , v127
	.byte	W24
	.byte	N11
	.byte	W48
	.byte	N11
	.byte	W16
	.byte	N03
	.byte	W08
mus_qz_645_8_004:
	.byte		N03   , Gs1 , v127
	.byte	W16
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		        Gn1
	.byte	W08
	.byte		        Gs1
	.byte	W08
	.byte		N15   , Ds2
	.byte	W24
	.byte		        Ds1
	.byte	W16
	.byte		N03   , Fn1
	.byte	W04
	.byte		        Gn1
	.byte	W04
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_645_8_004
	.byte	PATT
	 ptrvalue	mus_qz_645_8_000
	.byte	PATT
	 ptrvalue	mus_qz_645_8_000
	.byte	GOTO
	 ptrvalue	mus_qz_645_8_B1
	.byte	FINE

/********************** Track  9 **********************/

mus_qz_645_9:
	.byte		KEYSH , mus_qz_645_key+0
	.byte		VOICE , 39
	.byte		VOL   , 68*mus_qz_645_mvl/mxv
	.byte		PRIO  , 60
	.byte		        68*mus_qz_645_mvl/mxv
	.byte		        54*mus_qz_645_mvl/mxv
	.byte		N03   , Cn1 , v100
	.byte	W24
	.byte	N03
	.byte	W24
	.byte	N03
	.byte	W24
	.byte	N03
	.byte	W24
mus_qz_645_9_000:
	.byte		N03   , Fs1 , v080
	.byte	W16
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W04
	.byte		        Fs1 , v044
	.byte	W04
	.byte		        Fs1 , v028
	.byte	W04
	.byte		        Fs1 , v020
	.byte	W04
	.byte		        Fs1 , v016
	.byte	W04
	.byte		        Fs1 , v008
	.byte	W04
	.byte		        Fs1 , v080
	.byte	W16
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W04
	.byte		        Fs1 , v044
	.byte	W04
	.byte		        Fs1 , v028
	.byte	W04
	.byte		        Fs1 , v020
	.byte	W04
	.byte		        Fs1 , v016
	.byte	W04
	.byte		        Fs1 , v008
	.byte	W04
	.byte	PEND
mus_qz_645_9_B1:
	.byte	PATT
	 ptrvalue	mus_qz_645_9_000
mus_qz_645_9_001:
	.byte		N03   , Fs1 , v080
	.byte	W16
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W16
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W16
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W04
	.byte		        Fs1 , v044
	.byte	W04
	.byte		        Fs1 , v028
	.byte	W04
	.byte		        Fs1 , v020
	.byte	W04
	.byte		        Fs1 , v016
	.byte	W04
	.byte		        Fs1 , v008
	.byte	W04
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_645_9_000
	.byte	PATT
	 ptrvalue	mus_qz_645_9_001
mus_qz_645_9_002:
	.byte		N03   , Fs1 , v080
	.byte	W16
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		        Fs1 , v044
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		        Fs1 , v080
	.byte	W16
	.byte	N03
	.byte	W08
	.byte		N23   , Dn2
	.byte	W24
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_645_9_002
	.byte	PATT
	 ptrvalue	mus_qz_645_9_000
	.byte		N03   , Fs1 , v080
	.byte	W16
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		        Fs1 , v044
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		        Fs1 , v080
	.byte	W24
	.byte		N23   , Dn2
	.byte	W24
mus_qz_645_9_003:
	.byte		N03   , Dn1 , v080
	.byte		N44   , En2 , v080 , gtp3
	.byte	W16
	.byte		N03   , Dn1
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		        Dn1 , v044
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		        Dn1 , v080
	.byte	W16
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W04
	.byte		        Dn1 , v044
	.byte	W04
	.byte		        Dn1 , v028
	.byte	W04
	.byte		        Dn1 , v020
	.byte	W04
	.byte		        Dn1 , v016
	.byte	W04
	.byte		        Dn1 , v008
	.byte	W04
	.byte	PEND
mus_qz_645_9_004:
	.byte		N03   , Dn1 , v080
	.byte	W16
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W04
	.byte		        Dn1 , v044
	.byte	W04
	.byte		        Dn1 , v028
	.byte	W04
	.byte		        Dn1 , v020
	.byte	W04
	.byte		        Dn1 , v016
	.byte	W04
	.byte		        Dn1 , v008
	.byte	W04
	.byte		        Dn1 , v080
	.byte	W16
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W04
	.byte		        Dn1 , v044
	.byte	W04
	.byte		        Dn1 , v028
	.byte	W04
	.byte		        Dn1 , v020
	.byte	W04
	.byte		        Dn1 , v016
	.byte	W04
	.byte		        Dn1 , v008
	.byte	W04
	.byte	PEND
mus_qz_645_9_005:
	.byte		N03   , Dn1 , v080
	.byte	W16
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		        Dn1 , v044
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		        Dn1 , v080
	.byte	W16
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		        Dn1 , v044
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	PEND
mus_qz_645_9_006:
	.byte		N03   , Dn1 , v080
	.byte	W16
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W04
	.byte		        Dn1 , v044
	.byte	W04
	.byte		        Dn1 , v028
	.byte	W04
	.byte		        Dn1 , v020
	.byte	W04
	.byte		        Dn1 , v016
	.byte	W04
	.byte		        Dn1 , v008
	.byte	W04
	.byte		        Dn1 , v080
	.byte	W16
	.byte	N03
	.byte	W08
	.byte	N03
	.byte		N23   , Dn2
	.byte	W04
	.byte		N03   , Dn1 , v044
	.byte	W04
	.byte		        Dn1 , v028
	.byte	W04
	.byte		        Dn1 , v020
	.byte	W04
	.byte		        Dn1 , v016
	.byte	W04
	.byte		        Dn1 , v008
	.byte	W04
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_645_9_003
	.byte	PATT
	 ptrvalue	mus_qz_645_9_004
	.byte	PATT
	 ptrvalue	mus_qz_645_9_005
	.byte	PATT
	 ptrvalue	mus_qz_645_9_006
	.byte	PATT
	 ptrvalue	mus_qz_645_9_003
mus_qz_645_9_007:
	.byte		N03   , Dn1 , v080
	.byte	W24
	.byte	N03
	.byte	W08
	.byte		        Dn1 , v044
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		        Dn1 , v080
	.byte	W16
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		        Dn1 , v044
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_645_9_004
	.byte	PATT
	 ptrvalue	mus_qz_645_9_007
mus_qz_645_9_008:
	.byte		N23   , Dn2 , v080
	.byte	W24
	.byte		N11   , Cn1 , v100
	.byte	W24
	.byte		N23   , Dn2 , v080
	.byte	W24
	.byte		N11   , Cn1 , v100
	.byte	W24
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_645_9_008
mus_qz_645_9_009:
	.byte		N03   , Dn1 , v080
	.byte	W16
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		        Dn1 , v044
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		        Dn1 , v080
	.byte	W24
	.byte	N03
	.byte		N23   , Dn2
	.byte	W04
	.byte		N03   , Dn1 , v044
	.byte	W04
	.byte		        Dn1 , v028
	.byte	W04
	.byte		        Dn1 , v020
	.byte	W04
	.byte		        Dn1 , v016
	.byte	W04
	.byte		        Dn1 , v008
	.byte	W04
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_645_9_009
	.byte		N03   , Dn1 , v080
	.byte	W16
	.byte	N03
	.byte	W08
	.byte		N23   , Dn2
	.byte	W24
	.byte		N03   , Dn1
	.byte	W16
	.byte	N03
	.byte	W08
	.byte		N23   , Dn2
	.byte	W24
	.byte		N03   , Dn1
	.byte	W16
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		        Dn1 , v044
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		        Dn1 , v080
	.byte	W24
	.byte		N23   , Dn2
	.byte	W24
mus_qz_645_9_010:
	.byte		N03   , Cn1 , v100
	.byte	W24
	.byte	N03
	.byte	W24
	.byte	N03
	.byte	W24
	.byte	N03
	.byte	W24
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_645_9_010
	.byte	PATT
	 ptrvalue	mus_qz_645_9_000
	.byte	PATT
	 ptrvalue	mus_qz_645_9_000
	.byte	GOTO
	 ptrvalue	mus_qz_645_9_B1
	.byte	FINE

/******************************************************/
	.p2align	2

mus_qz_645:
	.byte	9	/* NumTrks */
	.byte	0	/* NumBlks */
	.byte	mus_qz_645_pri	/* Priority */
	.byte	mus_qz_645_rev	/* Reverb. */
space64 4
	ptrvalue	mus_qz_645_grp

	ptrvalue	mus_qz_645_1
	ptrvalue	mus_qz_645_2
	ptrvalue	mus_qz_645_3
	ptrvalue	mus_qz_645_4
	ptrvalue	mus_qz_645_5
	ptrvalue	mus_qz_645_6
	ptrvalue	mus_qz_645_7
	ptrvalue	mus_qz_645_8
	ptrvalue	mus_qz_645_9

	.end
