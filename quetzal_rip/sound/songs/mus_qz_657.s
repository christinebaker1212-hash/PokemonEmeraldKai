	.include "asm/macros/bit_width.inc"

	.include "MPlayDef.s"

/* Moemon Quetzal song 657, disassembled from the ROM by tools/quetzal_rip/songs.py.
   "org" comments give the ROM address of the bytes that follow. */

	.equ	mus_qz_657_grp, voicegroup_qz_songs
	.equ	mus_qz_657_pri, 0
	.equ	mus_qz_657_rev, reverb_set+0
	.equ	mus_qz_657_mvl, 127
	.equ	mus_qz_657_key, 0
	.equ	mus_qz_657_tbs, 1
	.equ	mus_qz_657_exg, 1
	.equ	mus_qz_657_cmp, 1

	.section .rodata
	.global	mus_qz_657
	.p2align	2
/* org 0x0969F110 */

/********************** Track  1 **********************/

mus_qz_657_1:
	.byte		KEYSH , mus_qz_657_key+0
	.byte		TEMPO , 148*mus_qz_657_tbs/2
	.byte		VOICE , 35
	.byte		VOL   , 71*mus_qz_657_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		BENDR , 6
	.byte		PRIO  , 64
	.byte		VOL   , 55*mus_qz_657_mvl/mxv
	.byte		N05   , Dn4 , v116
	.byte	W11
	.byte		BEND  , c_v-16
	.byte	W01
	.byte		N32   , Dn5 , v116 , gtp3
	.byte	W01
	.byte		BEND  , c_v-11
	.byte	W02
	.byte		        c_v-5
	.byte	W01
	.byte		        c_v+0
	.byte	W32
	.byte		N04   , Cn5
	.byte	W12
	.byte		        Bn4
	.byte	W12
	.byte		        As4
	.byte	W12
	.byte		        An4
	.byte	W12
	.byte		        En4
	.byte		N04   , Gs4
	.byte	W12
	.byte		        Ds4
	.byte		N04   , Gn4
	.byte	W12
	.byte		        Dn4
	.byte		N04   , Fs4
	.byte	W12
	.byte		        Cs4
	.byte		N04   , Fn4
	.byte	W12
	.byte		N11   , Cn4
	.byte		N11   , En4
	.byte	W12
	.byte		N04   , Dn4
	.byte	W12
	.byte		        En4
	.byte	W12
	.byte		        Fs4
	.byte	W11
	.byte		BEND  , c_v-16
	.byte	W01
mus_qz_657_1_B1:
	.byte		N80   , Gn4 , v124 , gtp3
	.byte	W01
	.byte		BEND  , c_v-8
	.byte	W02
	.byte		        c_v-4
	.byte	W01
	.byte		        c_v+0
	.byte	W56
	.byte	W03
	.byte		VOL   , 51*mus_qz_657_mvl/mxv
	.byte	W03
	.byte		        47*mus_qz_657_mvl/mxv
	.byte	W03
	.byte		        43*mus_qz_657_mvl/mxv
	.byte	W03
	.byte		        37*mus_qz_657_mvl/mxv
	.byte	W03
	.byte		        31*mus_qz_657_mvl/mxv
	.byte	W03
	.byte		        29*mus_qz_657_mvl/mxv
	.byte	W03
	.byte		        25*mus_qz_657_mvl/mxv
	.byte	W03
	.byte		        22*mus_qz_657_mvl/mxv
	.byte	W03
	.byte		        21*mus_qz_657_mvl/mxv
	.byte	W03
	.byte		        19*mus_qz_657_mvl/mxv
	.byte	W03
	.byte		        17*mus_qz_657_mvl/mxv
	.byte	W02
	.byte		BEND  , c_v-16
	.byte	W01
	.byte		VOL   , 55*mus_qz_657_mvl/mxv
	.byte		N42   , An4
	.byte	W01
	.byte		BEND  , c_v-13
	.byte	W01
	.byte		        c_v-4
	.byte	W03
	.byte		        c_v+0
	.byte	W28
	.byte		VOL   , 48*mus_qz_657_mvl/mxv
	.byte	W03
	.byte		        45*mus_qz_657_mvl/mxv
	.byte	W01
	.byte		        43*mus_qz_657_mvl/mxv
	.byte	W02
	.byte		        36*mus_qz_657_mvl/mxv
	.byte	W03
	.byte		        34*mus_qz_657_mvl/mxv
	.byte	W02
	.byte		        32*mus_qz_657_mvl/mxv
	.byte	W01
	.byte		        31*mus_qz_657_mvl/mxv
	.byte	W03
	.byte		        55*mus_qz_657_mvl/mxv
	.byte		N42   , Bn4
	.byte	W32
	.byte	W01
	.byte		VOL   , 48*mus_qz_657_mvl/mxv
	.byte	W03
	.byte		        46*mus_qz_657_mvl/mxv
	.byte	W01
	.byte		        42*mus_qz_657_mvl/mxv
	.byte	W02
	.byte		        35*mus_qz_657_mvl/mxv
	.byte	W03
	.byte		        34*mus_qz_657_mvl/mxv
	.byte	W01
	.byte		        33*mus_qz_657_mvl/mxv
	.byte	W02
	.byte		        30*mus_qz_657_mvl/mxv
	.byte	W02
	.byte		BEND  , c_v-26
	.byte	W01
	.byte		        c_v-18
	.byte		N48   , An4
	.byte	W01
	.byte		VOL   , 55*mus_qz_657_mvl/mxv
	.byte		BEND  , c_v-13
	.byte	W01
	.byte		        c_v-6
	.byte	W01
	.byte		        c_v+0
	.byte	W32
	.byte	W01
	.byte		VOL   , 50*mus_qz_657_mvl/mxv
	.byte	W03
	.byte		        43*mus_qz_657_mvl/mxv
	.byte	W03
	.byte		        38*mus_qz_657_mvl/mxv
	.byte	W03
	.byte		        34*mus_qz_657_mvl/mxv
	.byte	W15
	.byte		        55*mus_qz_657_mvl/mxv
	.byte		N05   , Gs4
	.byte	W12
	.byte		        An4
	.byte	W12
	.byte		N52   , Gs4 , v124 , gtp1
	.byte	W12
	.byte	W24
	.byte	W03
	.byte		VOL   , 51*mus_qz_657_mvl/mxv
	.byte	W01
	.byte		        47*mus_qz_657_mvl/mxv
	.byte	W02
	.byte		        44*mus_qz_657_mvl/mxv
	.byte	W01
	.byte		        42*mus_qz_657_mvl/mxv
	.byte	W02
	.byte		        39*mus_qz_657_mvl/mxv
	.byte	W01
	.byte		        35*mus_qz_657_mvl/mxv
	.byte	W02
	.byte		        31*mus_qz_657_mvl/mxv
	.byte	W03
	.byte		        29*mus_qz_657_mvl/mxv
	.byte	W03
	.byte		        27*mus_qz_657_mvl/mxv
	.byte	W03
	.byte		        22*mus_qz_657_mvl/mxv
	.byte	W04
	.byte		        55*mus_qz_657_mvl/mxv
	.byte	W11
	.byte		N07   , Fn4
	.byte	W24
	.byte		N05   , En4
	.byte	W11
	.byte		BEND  , c_v-16
	.byte	W01
	.byte		N88   , Cn5 , v124 , gtp1
	.byte	W01
	.byte		BEND  , c_v-8
	.byte	W01
	.byte		        c_v-5
	.byte		        c_v+0
	.byte	W52
	.byte		VOL   , 50*mus_qz_657_mvl/mxv
	.byte	W03
	.byte		        45*mus_qz_657_mvl/mxv
	.byte		        48*mus_qz_657_mvl/mxv
	.byte	W03
	.byte		        42*mus_qz_657_mvl/mxv
	.byte	W03
	.byte		        39*mus_qz_657_mvl/mxv
	.byte	W03
	.byte		        35*mus_qz_657_mvl/mxv
	.byte	W03
	.byte		        33*mus_qz_657_mvl/mxv
	.byte	W03
	.byte		        30*mus_qz_657_mvl/mxv
	.byte	W03
	.byte		        27*mus_qz_657_mvl/mxv
	.byte	W03
	.byte		        25*mus_qz_657_mvl/mxv
	.byte	W03
	.byte		        23*mus_qz_657_mvl/mxv
	.byte	W03
	.byte		        22*mus_qz_657_mvl/mxv
	.byte	W11
	.byte		BEND  , c_v-16
	.byte	W01
	.byte		VOL   , 55*mus_qz_657_mvl/mxv
	.byte		BEND  , c_v-13
	.byte		N44   , Dn5
	.byte	W01
	.byte		BEND  , c_v-7
	.byte	W01
	.byte		        c_v+0
	.byte	W32
	.byte	W02
	.byte		VOL   , 46*mus_qz_657_mvl/mxv
	.byte	W03
	.byte		        38*mus_qz_657_mvl/mxv
	.byte	W03
	.byte		        29*mus_qz_657_mvl/mxv
	.byte	W03
	.byte		        24*mus_qz_657_mvl/mxv
	.byte	W03
	.byte		        55*mus_qz_657_mvl/mxv
	.byte		N44   , Cn5
	.byte	W32
	.byte	W01
	.byte		VOL   , 51*mus_qz_657_mvl/mxv
	.byte	W03
	.byte		        41*mus_qz_657_mvl/mxv
	.byte	W03
	.byte		        35*mus_qz_657_mvl/mxv
	.byte	W03
	.byte		        33*mus_qz_657_mvl/mxv
	.byte	W03
	.byte		        27*mus_qz_657_mvl/mxv
	.byte	W03
	.byte		        55*mus_qz_657_mvl/mxv
	.byte		N56   , Bn4
	.byte	W48
	.byte	W03
	.byte		VOL   , 48*mus_qz_657_mvl/mxv
	.byte	W01
	.byte		        44*mus_qz_657_mvl/mxv
	.byte	W02
	.byte		        38*mus_qz_657_mvl/mxv
	.byte	W01
	.byte		        35*mus_qz_657_mvl/mxv
	.byte	W02
	.byte		        31*mus_qz_657_mvl/mxv
	.byte	W01
	.byte		        25*mus_qz_657_mvl/mxv
	.byte	W02
	.byte		        55*mus_qz_657_mvl/mxv
	.byte		N05   , Dn5
	.byte	W12
	.byte		        Bn4
	.byte	W12
	.byte		        An4 , v112
	.byte	W06
	.byte		        An4 , v096
	.byte	W06
	.byte		        An4 , v124
	.byte	W48
	.byte		        An3 , v088
	.byte	W06
	.byte		        Cn4 , v068
	.byte	W06
	.byte		        Ds4 , v088
	.byte	W06
	.byte		        Fs4 , v068
	.byte	W06
	.byte		        Ds4 , v088
	.byte	W06
	.byte		        Fs4 , v068
	.byte	W06
	.byte		        An4 , v088
	.byte	W06
	.byte		        Cn5 , v068
	.byte	W06
	.byte	GOTO
	 ptrvalue	mus_qz_657_1_B1
	.byte	FINE

/********************** Track  2 **********************/

mus_qz_657_2:
	.byte		KEYSH , mus_qz_657_key+0
	.byte		VOICE , 20
	.byte		VOL   , 80*mus_qz_657_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 62
	.byte	W96
	.byte	W48
	.byte		N32   , Dn2 , v116 , gtp3
	.byte	W36
	.byte		N05   , En2 , v096
	.byte	W06
	.byte		        Fs2 , v108
	.byte	W06
mus_qz_657_2_B1:
	.byte		N23   , Gn2 , v116
	.byte	W36
	.byte		N02   , Gn2 , v088
	.byte	W12
	.byte		N28   , Dn2 , v116 , gtp1
	.byte	W36
	.byte		N02   , Dn2 , v088
	.byte	W12
	.byte		N28   , Gn2 , v116 , gtp1
	.byte	W36
	.byte		N02   , Gn2 , v088
	.byte	W12
	.byte		N11   , Dn2 , v116
	.byte	W12
	.byte		N02   , Bn1 , v072
	.byte	W12
	.byte		N11   , Dn2 , v116
	.byte	W12
	.byte		N02   , Gn2 , v072
	.byte	W12
	.byte		N28   , En2 , v116 , gtp1
	.byte	W36
	.byte		N02   , En2 , v100
	.byte	W12
	.byte		N23   , Bn2 , v116
	.byte	W24
	.byte		N02   , Bn2 , v100
	.byte	W12
	.byte		N28   , En3 , v116 , gtp1
	.byte	W12
	.byte	W24
	.byte		N05   , Dn3 , v108
	.byte	W06
	.byte		        En3 , v100
	.byte	W06
	.byte		        Dn3 , v108
	.byte	W06
	.byte		        An2 , v100
	.byte	W06
	.byte		N11   , Gs2 , v108
	.byte	W12
	.byte		N03   , Bn2 , v100
	.byte	W12
	.byte		N11   , Gs2 , v116
	.byte	W12
	.byte		N03   , En2 , v100
	.byte	W12
	.byte		N30   , An2 , v116
	.byte	W36
	.byte		N02   , An2 , v072
	.byte	W12
	.byte		N30   , En2 , v116
	.byte	W36
	.byte		N02   , En2 , v072
	.byte	W12
	.byte		N28   , An2 , v116 , gtp1
	.byte	W36
	.byte		N02   , An2 , v072
	.byte	W12
	.byte		N20   , Cn3 , v116
	.byte	W24
	.byte		        En3
	.byte	W24
	.byte		N28   , Dn2 , v116 , gtp1
	.byte	W36
	.byte		N02   , Dn2 , v072
	.byte	W12
	.byte		N28   , An2 , v116 , gtp1
	.byte	W36
	.byte	N11
	.byte	W12
	.byte		N09   , Dn3
	.byte	W48
	.byte		N44   , Dn2
	.byte	W48
	.byte	GOTO
	 ptrvalue	mus_qz_657_2_B1
	.byte	FINE

/********************** Track  3 **********************/

mus_qz_657_3:
	.byte		KEYSH , mus_qz_657_key+0
	.byte		VOICE , 27
	.byte		VOL   , 51*mus_qz_657_mvl/mxv
	.byte		PAN   , c_v+48
	.byte		PRIO  , 56
	.byte	W96
	.byte		N04   , En4 , v108
	.byte	W12
	.byte		        Ds4 , v096
	.byte	W12
	.byte		        Dn4 , v108
	.byte	W12
	.byte		        Cs4 , v096
	.byte	W12
	.byte		N11   , Cn4 , v108
	.byte	W12
	.byte		N04   , Dn4 , v096
	.byte	W12
	.byte		        En4 , v108
	.byte	W12
	.byte		        Fs4 , v096
	.byte	W12
mus_qz_657_3_B1:
	.byte	W24
	.byte		N04   , Fs3 , v080
	.byte		N04   , As3
	.byte	W06
	.byte		        Gn3 , v068
	.byte		N04   , Bn3
	.byte	W06
	.byte		N05   , An3 , v092
	.byte		N05   , Cn4
	.byte	W24
	.byte		        Bn3 , v100
	.byte		N05   , Dn4
	.byte	W24
	.byte		        Bn3 , v092
	.byte		N05   , Dn4
	.byte	W12
	.byte		N23   , Bn3 , v100
	.byte		N23   , Dn4
	.byte	W24
	.byte		N05   , An3 , v092
	.byte		N05   , Cn4
	.byte	W12
	.byte		        Gn3
	.byte		N05   , Bn3
	.byte	W24
	.byte		N10   , Fs3
	.byte		N10   , An3
	.byte	W24
	.byte		        Gn3
	.byte		N10   , Bn3
	.byte	W12
	.byte	W24
	.byte		N05   , En3
	.byte		N05   , An3
	.byte	W24
	.byte		        En3
	.byte		N05   , An3
	.byte	W06
	.byte		        En3 , v080
	.byte		N05   , As3
	.byte	W06
	.byte		        Fs3 , v092
	.byte		N05   , Bn3
	.byte	W12
	.byte		        En3
	.byte		N05   , An3
	.byte	W12
	.byte		N32   , En3 , v100 , gtp3
	.byte		N32   , Gs3 , v100 , gtp3
	.byte	W12
	.byte	W24
	.byte		N05   , Bn2
	.byte		N05   , En3
	.byte	W06
	.byte		        Dn3 , v092
	.byte		N05   , Fn3
	.byte	W06
	.byte		        En3
	.byte		N05   , Gs3
	.byte	W24
	.byte		N11   , En3
	.byte		N11   , Gs3
	.byte	W12
	.byte		N04   , Dn3 , v088
	.byte		N04   , Fn3
	.byte	W12
	.byte		        Bn2 , v096
	.byte		N04   , En3
	.byte	W12
	.byte		N21   , An3 , v100
	.byte		N21   , Cn4
	.byte	W24
	.byte		N05   , Gn3 , v084
	.byte		N05   , Bn3
	.byte	W06
	.byte		        An3
	.byte		N05   , Cn4
	.byte	W06
	.byte		        Bn3 , v092
	.byte		N05   , Dn4
	.byte	W24
	.byte		N04   , An3 , v084
	.byte		N04   , Cn4
	.byte	W06
	.byte		        Gs3 , v080
	.byte		N04   , Bn3
	.byte	W06
	.byte		N05   , An3 , v092
	.byte		N05   , Cn4
	.byte	W12
	.byte		        Bn3
	.byte		N05   , Dn4
	.byte	W12
	.byte		N17   , Cn4 , v100
	.byte		N17   , En4
	.byte	W24
	.byte		N11   , Cn4 , v092
	.byte		N11   , En4
	.byte	W12
	.byte		N08   , Bn3
	.byte		N08   , Dn4
	.byte	W24
	.byte		N28   , An3 , v100 , gtp1
	.byte		N28   , Cn4 , v100 , gtp1
	.byte	W36
	.byte		N23   , Fs3 , v108
	.byte		N23   , Bn3
	.byte	W24
	.byte		N05   , En3 , v092
	.byte		N05   , An3
	.byte	W06
	.byte		        En3
	.byte		N05   , As3
	.byte	W06
	.byte		        Fs3
	.byte		N05   , Bn3
	.byte	W24
	.byte	N05
	.byte		N05   , Dn4
	.byte	W12
	.byte		        En3
	.byte		N05   , Bn3
	.byte	W12
	.byte		        Fs3
	.byte		N05   , An3
	.byte	W06
	.byte		        Fs3 , v080
	.byte		N05   , An3
	.byte	W06
	.byte		N11   , Fs3 , v092
	.byte		N11   , An3
	.byte	W48
	.byte		N44   , Ds3 , v100 , gtp3
	.byte		N23   , Fs3
	.byte	W24
	.byte		        An3
	.byte	W24
	.byte	GOTO
	 ptrvalue	mus_qz_657_3_B1
	.byte	FINE

/********************** Track  4 **********************/

mus_qz_657_4:
	.byte		KEYSH , mus_qz_657_key+0
	.byte		VOICE , 5
	.byte		VOL   , 42*mus_qz_657_mvl/mxv
	.byte		PAN   , c_v-34
	.byte		PRIO  , 52
	.byte		N05   , Dn4 , v116
	.byte	W12
	.byte		N32   , Dn5 , v120 , gtp3
	.byte	W36
	.byte		N04   , Cn5
	.byte	W12
	.byte		        Bn4 , v108
	.byte	W12
	.byte		        As4 , v120
	.byte	W12
	.byte		        An4 , v108
	.byte	W12
	.byte		        Gs4 , v120
	.byte	W12
	.byte		        Gn4 , v108
	.byte	W12
	.byte		        Fs4 , v120
	.byte	W12
	.byte		        Fn4 , v108
	.byte	W12
	.byte		N11   , En4 , v120
	.byte	W12
	.byte		N04   , Dn4 , v108
	.byte	W12
	.byte		        En4 , v120
	.byte	W24
mus_qz_657_4_B1:
	.byte	W24
	.byte		N05   , Bn3 , v100
	.byte		N05   , Gn4 , v108
	.byte	W12
	.byte		        Bn3 , v044
	.byte		N05   , Gn4 , v048
	.byte	W24
	.byte		        Bn3 , v100
	.byte		N05   , Gn4 , v108
	.byte	W12
	.byte		        Bn3 , v044
	.byte		N05   , Gn4 , v048
	.byte	W24
	.byte	W24
	.byte		        Cn4 , v100
	.byte		N05   , An4 , v108
	.byte	W12
	.byte		        Cn4 , v044
	.byte		N05   , An4 , v048
	.byte	W24
	.byte		        Dn4 , v100
	.byte		N05   , Bn4 , v108
	.byte	W12
	.byte		        Dn4 , v044
	.byte		N05   , Bn4 , v048
	.byte	W24
	.byte	W24
	.byte		        Dn4 , v092
	.byte		N05   , An4 , v108
	.byte	W12
	.byte		        Dn4 , v036
	.byte		N05   , An4 , v048
	.byte	W24
	.byte		        En4 , v100
	.byte		N05   , Gs4 , v108
	.byte	W12
	.byte		        En4 , v044
	.byte		N05   , Gs4 , v048
	.byte	W24
	.byte	W24
	.byte		        En4 , v100
	.byte		N05   , Gs4 , v108
	.byte	W12
	.byte		        En4 , v044
	.byte		N05   , Gs4 , v048
	.byte	W24
	.byte		        Dn4 , v100
	.byte		N05   , Fn4 , v108
	.byte	W24
	.byte		        Bn3 , v088
	.byte		N05   , En4 , v096
	.byte	W12
	.byte	W24
	.byte		        Cn4 , v100
	.byte		N05   , An4 , v108
	.byte	W12
	.byte		        Cn4 , v044
	.byte		N05   , An4 , v048
	.byte	W24
	.byte		        Cn4 , v100
	.byte		N05   , An4 , v108
	.byte	W12
	.byte		        Cn4 , v044
	.byte		N05   , An4 , v048
	.byte	W24
	.byte	W24
	.byte		        Dn4 , v100
	.byte		N05   , Bn4 , v108
	.byte	W12
	.byte		        Dn4 , v044
	.byte		N05   , Bn4 , v048
	.byte	W24
	.byte		        Cn4 , v100
	.byte		N05   , An4 , v108
	.byte	W12
	.byte		        Cn4 , v044
	.byte		N05   , An4 , v048
	.byte	W24
	.byte	W24
	.byte		        Dn4 , v100
	.byte		N05   , Bn4 , v108
	.byte	W12
	.byte		        Dn4 , v044
	.byte		N05   , Bn4 , v048
	.byte	W24
	.byte		        Dn4 , v100
	.byte		N05   , Bn4 , v108
	.byte	W12
	.byte		        Dn4 , v044
	.byte		N05   , Bn4 , v048
	.byte	W24
	.byte		        Dn4 , v100
	.byte		N05   , An4 , v108
	.byte	W48
	.byte		N44   , Dn4 , v096 , gtp3
	.byte		N23   , Fs4 , v108
	.byte	W24
	.byte		        An4 , v112
	.byte	W24
	.byte	GOTO
	 ptrvalue	mus_qz_657_4_B1
	.byte	FINE

/********************** Track  5 **********************/

mus_qz_657_5:
	.byte		KEYSH , mus_qz_657_key+0
	.byte		VOICE , 39
	.byte		VOL   , 66*mus_qz_657_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 42
	.byte	W96
	.byte	W48
	.byte		N05   , Bn0 , v092
	.byte		N23   , En2 , v088
	.byte	W24
	.byte		N05   , Bn0 , v092
	.byte		N05   , Bn1 , v088
	.byte	W06
	.byte		        An1 , v068
	.byte	W06
	.byte		N11   , Fn1 , v080
	.byte	W12
mus_qz_657_5_B1:
	.byte		N05   , Bn0 , v092
	.byte		N23   , An2
	.byte	W24
	.byte		N11   , Fs0 , v100
	.byte		N11   , As1 , v072
	.byte	W12
	.byte		N05   , Bn0 , v068
	.byte		N02   , Fs1 , v048
	.byte	W12
	.byte		N05   , Bn0 , v092
	.byte		N02   , Fs1 , v036
	.byte	W06
	.byte		        Fs1 , v048
	.byte	W06
	.byte		N11   , As1 , v072
	.byte	W12
	.byte		        Fs0 , v092
	.byte	W12
	.byte		N05   , Bn0 , v068
	.byte		N02   , Fs1 , v036
	.byte	W12
	.byte		N05   , Bn0 , v092
	.byte		N02   , Fs1 , v048
	.byte	W12
	.byte		        Fs1 , v036
	.byte	W12
	.byte		N11   , Fs0 , v100
	.byte		N02   , Fs1 , v048
	.byte	W12
	.byte		N05   , Bn0 , v068
	.byte		N02   , Fs1 , v036
	.byte	W12
	.byte		N05   , Bn0 , v092
	.byte		N02   , Fs1 , v048
	.byte	W06
	.byte		        Fs1 , v036
	.byte	W06
	.byte		N11   , As1 , v072
	.byte	W12
	.byte		        Fs0 , v092
	.byte		N02   , Fs1 , v048
	.byte	W12
	.byte		N05   , Bn0 , v068
	.byte		N02   , Fs1 , v036
	.byte	W12
	.byte		N05   , Bn0 , v092
	.byte		N02   , Fs1 , v048
	.byte	W12
	.byte		        Fs1 , v036
	.byte	W12
	.byte		N11   , Fs0 , v100
	.byte		N02   , Fs1 , v048
	.byte	W12
	.byte		N05   , Bn0 , v068
	.byte		N02   , Fs1 , v036
	.byte	W06
	.byte	N02
	.byte	W06
	.byte		N05   , Bn0 , v092
	.byte		N11   , As1 , v072
	.byte	W12
	.byte		        Fs0 , v080
	.byte		N05   , Bn1
	.byte	W06
	.byte		        An1 , v072
	.byte	W06
	.byte		N11   , Gn1 , v088
	.byte	W12
	.byte		        Bn0 , v092
	.byte		N23   , Ds2
	.byte	W12
	.byte	W12
	.byte		N02   , Fs1 , v036
	.byte	W06
	.byte	N02
	.byte	W06
	.byte		N11   , Fs0 , v100
	.byte		N11   , As1 , v072
	.byte	W12
	.byte		N05   , Bn0 , v068
	.byte		N02   , Fs1 , v036
	.byte	W12
	.byte		N05   , Bn0 , v092
	.byte		N02   , Fs1 , v036
	.byte	W06
	.byte		        Fs1 , v028
	.byte	W06
	.byte		N11   , As1 , v072
	.byte	W12
	.byte		N05   , Bn0 , v044
	.byte		N11   , Bn1 , v088
	.byte	W12
	.byte		N05   , Bn0 , v068
	.byte		N11   , Fn1
	.byte	W12
	.byte		N05   , Bn0 , v092
	.byte		N23   , Cs2
	.byte	W24
	.byte		N11   , Fs0 , v100
	.byte		N02   , Fs1 , v048
	.byte	W12
	.byte		N05   , Bn0 , v068
	.byte		N02   , Fs1 , v036
	.byte	W12
	.byte		N05   , Bn0 , v092
	.byte		N02   , Fs1 , v048
	.byte	W06
	.byte		        Fs1 , v032
	.byte	W06
	.byte		N11   , As1 , v072
	.byte	W12
	.byte		        Fs0 , v092
	.byte		N05   , Bn0 , v044
	.byte		N02   , Fs1 , v048
	.byte	W12
	.byte		N05   , Bn0 , v068
	.byte		N02   , Fs1 , v036
	.byte	W12
	.byte		N05   , Bn0 , v092
	.byte		N02   , Fs1 , v048
	.byte	W12
	.byte		        Fs1 , v036
	.byte	W12
	.byte		N11   , Fs0 , v100
	.byte		N02   , Fs1 , v048
	.byte	W12
	.byte		N05   , Bn0 , v068
	.byte		N02   , Fs1 , v036
	.byte	W12
	.byte		N05   , Bn0 , v092
	.byte		N11   , As1 , v072
	.byte	W12
	.byte		        Fs0 , v092
	.byte		N02   , Fs1 , v036
	.byte	W12
	.byte		N05   , Bn0 , v044
	.byte		N02   , Fs1 , v048
	.byte	W06
	.byte		        Fs1 , v036
	.byte	W06
	.byte		N05   , Bn0 , v068
	.byte		N11   , As1 , v072
	.byte	W12
	.byte		N05   , Bn0 , v092
	.byte		N02   , Fs1 , v048
	.byte	W12
	.byte		        Fs1 , v036
	.byte	W12
	.byte		N11   , Fs0 , v100
	.byte		N02   , Fs1 , v048
	.byte	W12
	.byte		N05   , Bn0 , v068
	.byte		N02   , Fs1 , v036
	.byte	W12
	.byte		N05   , Bn0 , v092
	.byte		N11   , As1 , v072
	.byte	W12
	.byte		        Fs0 , v092
	.byte		N02   , Fs1 , v036
	.byte	W12
	.byte		N11   , Bn1 , v068
	.byte	W12
	.byte		N05   , An1 , v080
	.byte	W06
	.byte		        Fn1
	.byte	W06
	.byte		N11   , Fs0 , v100
	.byte		N05   , Bn0 , v092
	.byte		N08   , As1 , v072
	.byte	W12
	.byte		N02   , Fs1 , v012
	.byte	W12
	.byte		        Fs1 , v048
	.byte	W06
	.byte		        Fs1 , v020
	.byte	W06
	.byte		N11   , As1 , v068
	.byte	W12
	.byte		N02   , Fs1 , v036
	.byte	W12
	.byte		        Fs1 , v020
	.byte	W12
	.byte		N05   , Bn0 , v092
	.byte		N05   , Bn1 , v088
	.byte	W06
	.byte		        An1 , v068
	.byte	W06
	.byte		        Bn0
	.byte		N11   , Fn1 , v080
	.byte	W12
	.byte	GOTO
	 ptrvalue	mus_qz_657_5_B1
	.byte	FINE

/********************** Track  6 **********************/

mus_qz_657_6:
	.byte		KEYSH , mus_qz_657_key+0
	.byte		VOICE , 35
	.byte		VOL   , 35*mus_qz_657_mvl/mxv
	.byte		PAN   , c_v+56
	.byte		BENDR , 1
	.byte		PRIO  , 38
	.byte		VOL   , 27*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		N04   , Dn4 , v116
	.byte	W11
	.byte		BEND  , c_v-16
	.byte	W01
	.byte		N32   , Dn5 , v116 , gtp2
	.byte	W01
	.byte		BEND  , c_v-7
	.byte	W03
	.byte		        c_v+0
	.byte	W32
	.byte		N04   , Cn5
	.byte	W12
	.byte		        Bn4
	.byte	W12
	.byte		        As4
	.byte	W12
	.byte		        An4
	.byte	W06
	.byte	W06
	.byte		        Gs4
	.byte	W12
	.byte		        Gn4
	.byte	W12
	.byte		        Fs4
	.byte	W12
	.byte		        Fn4
	.byte	W12
	.byte		N10   , En4
	.byte	W12
	.byte		N04   , Dn4
	.byte	W12
	.byte		        En4
	.byte	W12
	.byte		        Fs4
	.byte	W06
mus_qz_657_6_B1:
	.byte	W05
	.byte		BEND  , c_v-16
	.byte	W01
	.byte		N88   , Gn4 , v116
	.byte	W01
	.byte		BEND  , c_v-8
	.byte	W02
	.byte		        c_v-4
	.byte	W01
	.byte		        c_v+0
	.byte	W56
	.byte		VOL   , 23*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        17*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        12*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        10*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        7*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        6*mus_qz_657_mvl/mxv
	.byte	W05
	.byte		BEND  , c_v-16
	.byte	W01
	.byte		VOL   , 27*mus_qz_657_mvl/mxv
	.byte		N40   , An4
	.byte	W01
	.byte		BEND  , c_v-8
	.byte	W01
	.byte		        c_v-4
	.byte	W03
	.byte		        c_v+0
	.byte	W24
	.byte	W01
	.byte		VOL   , 23*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        19*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        15*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        27*mus_qz_657_mvl/mxv
	.byte		N40   , Bn4
	.byte	W30
	.byte		VOL   , 23*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        19*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        15*mus_qz_657_mvl/mxv
	.byte	W05
	.byte		BEND  , c_v-20
	.byte	W01
	.byte		VOL   , 27*mus_qz_657_mvl/mxv
	.byte		N44   , An4 , v116 , gtp2
	.byte	W01
	.byte		BEND  , c_v-13
	.byte	W01
	.byte		        c_v-2
	.byte	W32
	.byte	W02
	.byte		VOL   , 20*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        15*mus_qz_657_mvl/mxv
	.byte	W18
	.byte		        27*mus_qz_657_mvl/mxv
	.byte		N04   , Gs4
	.byte	W12
	.byte		        An4
	.byte	W12
	.byte		N56   , Gs4 , v116 , gtp2
	.byte	W06
	.byte	W30
	.byte		VOL   , 25*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        20*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        15*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        10*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        27*mus_qz_657_mvl/mxv
	.byte	W12
	.byte		N04   , Fn4
	.byte	W24
	.byte		        En4
	.byte	W06
	.byte	W05
	.byte		BEND  , c_v-16
	.byte	W01
	.byte		N88   , Cn5
	.byte	W01
	.byte		BEND  , c_v-7
	.byte	W01
	.byte		        c_v+0
	.byte	W52
	.byte		VOL   , 23*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        20*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        15*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        12*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        11*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        11*mus_qz_657_mvl/mxv
	.byte	W06
	.byte	W05
	.byte		BEND  , c_v-16
	.byte	W01
	.byte		VOL   , 25*mus_qz_657_mvl/mxv
	.byte		BEND  , c_v-9
	.byte		N40   , Dn5
	.byte	W02
	.byte		BEND  , c_v+0
	.byte	W32
	.byte	W02
	.byte		VOL   , 18*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        11*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        27*mus_qz_657_mvl/mxv
	.byte		N40   , Cn5
	.byte	W30
	.byte		VOL   , 25*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        17*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        12*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        27*mus_qz_657_mvl/mxv
	.byte		N52   , Bn4
	.byte	W48
	.byte		VOL   , 23*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        17*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        12*mus_qz_657_mvl/mxv
	.byte		N04   , Dn5
	.byte	W12
	.byte		        Bn4
	.byte	W12
	.byte		        An4 , v108
	.byte	W06
	.byte		        An4 , v088
	.byte	W06
	.byte		        An4 , v116
	.byte	W48
	.byte		        An3 , v080
	.byte	W06
	.byte		        Cn4 , v060
	.byte	W06
	.byte		        Ds4 , v080
	.byte	W06
	.byte		        Fs4 , v060
	.byte	W06
	.byte		        Ds4 , v080
	.byte	W06
	.byte		        Fs4 , v060
	.byte	W06
	.byte		        An4 , v080
	.byte	W06
	.byte	GOTO
	 ptrvalue	mus_qz_657_6_B1
	.byte	FINE

/********************** Track  7 **********************/

mus_qz_657_7:
	.byte		KEYSH , mus_qz_657_key+0
	.byte		VOICE , 0
	.byte		VOL   , 47*mus_qz_657_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte		N05   , Dn5 , v100
	.byte	W12
	.byte		N32   , Dn6 , v100 , gtp3
	.byte	W84
	.byte	W96
mus_qz_657_7_B1:
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_657_7_B1
	.byte	FINE

/********************** Track  8 **********************/

mus_qz_657_8:
	.byte		KEYSH , mus_qz_657_key+0
	.byte		VOICE , 23
	.byte		VOL   , 66*mus_qz_657_mvl/mxv
	.byte		PAN   , c_v-39
	.byte		PRIO  , 58
	.byte		VOL   , 43*mus_qz_657_mvl/mxv
	.byte	W96
	.byte	W72
	.byte		N03   , Gn3 , v088
	.byte	W03
	.byte		        An3
	.byte	W03
	.byte		        Bn3
	.byte	W04
	.byte		        Cn4
	.byte	W03
	.byte		        Dn4
	.byte	W04
	.byte		        En4
	.byte	W03
	.byte		        Fs4
	.byte	W04
mus_qz_657_8_B1:
	.byte		N92   , Bn3 , v076 , gtp3
	.byte		N92   , Gn4 , v088 , gtp3
	.byte	W66
	.byte		VOL   , 42*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        40*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        38*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        36*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        35*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		N44   , Gn3 , v076 , gtp3
	.byte		N44   , Dn4 , v088 , gtp3
	.byte	W42
	.byte		VOL   , 35*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        36*mus_qz_657_mvl/mxv
	.byte		N23   , Bn3 , v076
	.byte		N23   , Gn4 , v088
	.byte	W06
	.byte		VOL   , 37*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        38*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        39*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        40*mus_qz_657_mvl/mxv
	.byte		N23   , Dn4 , v072
	.byte		N23   , Bn4 , v084
	.byte	W06
	.byte		VOL   , 41*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        42*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        42*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        43*mus_qz_657_mvl/mxv
	.byte		N52   , An3 , v076 , gtp1
	.byte		N52   , Fs4 , v088 , gtp1
	.byte	W42
	.byte		VOL   , 42*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        39*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        35*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        43*mus_qz_657_mvl/mxv
	.byte		N11   , Bn3 , v076
	.byte		N11   , Gs4 , v088
	.byte	W12
	.byte		N05   , Cn4 , v076
	.byte		N05   , An4 , v088
	.byte	W12
	.byte		N56   , Bn3 , v076 , gtp3
	.byte		N56   , Gs4 , v088 , gtp3
	.byte	W12
	.byte	W12
	.byte		VOL   , 42*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        39*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        38*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        36*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        35*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        33*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        31*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        29*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        42*mus_qz_657_mvl/mxv
	.byte		N11   , Gs3 , v076
	.byte		N11   , Fn4 , v088
	.byte	W24
	.byte		N05   , Gs3 , v076
	.byte		N05   , En4 , v088
	.byte	W12
	.byte		N92   , Cn4 , v076 , gtp3
	.byte		N92   , An4 , v088 , gtp3
	.byte	W72
	.byte		VOL   , 42*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        40*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        39*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        37*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		N44   , Dn4 , v076 , gtp3
	.byte		N44   , Bn4 , v088 , gtp3
	.byte	W30
	.byte		VOL   , 36*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        37*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        38*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        39*mus_qz_657_mvl/mxv
	.byte		N22   , Cn4 , v076
	.byte		N22   , An4 , v088
	.byte	W06
	.byte		VOL   , 39*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        40*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        41*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        42*mus_qz_657_mvl/mxv
	.byte		N22   , En4 , v076
	.byte		N22   , Cn5 , v088
	.byte	W06
	.byte		VOL   , 42*mus_qz_657_mvl/mxv
	.byte	W12
	.byte		        42*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		N56   , Bn3 , v076
	.byte		N56   , Gn4 , v088
	.byte	W42
	.byte		VOL   , 42*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        39*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        38*mus_qz_657_mvl/mxv
	.byte	W06
	.byte		        44*mus_qz_657_mvl/mxv
	.byte		N05   , Cn4 , v076
	.byte		N05   , An4 , v088
	.byte	W12
	.byte		        Bn3 , v072
	.byte		N05   , Gn4 , v080
	.byte	W12
	.byte		N04   , An3 , v084
	.byte		N04   , Fs4 , v096
	.byte	W06
	.byte		        An3 , v076
	.byte		N04   , Fs4 , v088
	.byte	W06
	.byte		N08   , An3
	.byte		N08   , Fs4 , v100
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_657_8_B1
	.byte	FINE

/******************************************************/
	.p2align	2

mus_qz_657:
	.byte	8	/* NumTrks */
	.byte	0	/* NumBlks */
	.byte	mus_qz_657_pri	/* Priority */
	.byte	mus_qz_657_rev	/* Reverb. */
space64 4
	ptrvalue	mus_qz_657_grp

	ptrvalue	mus_qz_657_1
	ptrvalue	mus_qz_657_2
	ptrvalue	mus_qz_657_3
	ptrvalue	mus_qz_657_4
	ptrvalue	mus_qz_657_5
	ptrvalue	mus_qz_657_6
	ptrvalue	mus_qz_657_7
	ptrvalue	mus_qz_657_8

	.end
