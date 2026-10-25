	.include "asm/macros/bit_width.inc"

	.include "MPlayDef.s"

/* Moemon Quetzal song 654, disassembled from the ROM by tools/quetzal_rip/songs.py.
   "org" comments give the ROM address of the bytes that follow. */

	.equ	mus_qz_654_grp, voicegroup_qz_songs
	.equ	mus_qz_654_pri, 0
	.equ	mus_qz_654_rev, reverb_set+0
	.equ	mus_qz_654_mvl, 127
	.equ	mus_qz_654_key, 0
	.equ	mus_qz_654_tbs, 1
	.equ	mus_qz_654_exg, 1
	.equ	mus_qz_654_cmp, 1

	.section .rodata
	.global	mus_qz_654
	.p2align	2
/* org 0x0969DEE8 */

/********************** Track  1 **********************/

mus_qz_654_1:
	.byte		KEYSH , mus_qz_654_key+0
	.byte		TEMPO , 82*mus_qz_654_tbs/2
	.byte		VOICE , 42
	.byte		VOL   , 84*mus_qz_654_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte		MOD   , 3
	.byte		LFOS  , 41
	.byte		LFODL , 9
	.byte	W36
mus_qz_654_1_B1:
	.byte	W24
	.byte		N11   , Bn3 , v104
	.byte	W12
	.byte		        Dn4 , v096
	.byte	W12
	.byte		N23   , En4 , v108
	.byte	W24
	.byte		        Bn4 , v104
	.byte	W24
	.byte		N11   , An4 , v100
	.byte	W12
	.byte		        Gn4 , v088
	.byte	W12
	.byte		        En4 , v096
	.byte	W12
	.byte		        Dn4 , v088
	.byte	W12
	.byte		N44   , En4 , v100 , gtp3
	.byte	W48
	.byte		N32   , As3 , v096 , gtp3
	.byte	W36
	.byte		N11   , Dn4 , v088
	.byte	W12
	.byte		        En4 , v096
	.byte	W12
	.byte		        Dn4 , v088
	.byte	W12
	.byte		        As3 , v092
	.byte	W12
	.byte		        Gn3 , v084
	.byte	W12
	.byte		N23   , An3 , v100
	.byte	W24
	.byte		N11   , Gn3 , v092
	.byte	W12
	.byte		N03   , As3 , v088
	.byte	W04
	.byte		        Dn4 , v084
	.byte	W04
	.byte		        As3 , v088
	.byte	W04
	.byte		TEMPO , 80*mus_qz_654_tbs/2
	.byte		LFOS  , 42
	.byte		LFODL , 8
	.byte		N11   , An3 , v096
	.byte	W12
	.byte		N03   , Gn4 , v072
	.byte	W04
	.byte		        An4 , v068
	.byte	W04
	.byte		        Gn4 , v076
	.byte	W04
	.byte		TEMPO , 76*mus_qz_654_tbs/2
	.byte		LFOS  , 44
	.byte		N11   , En4 , v068
	.byte	W12
	.byte		TEMPO , 74*mus_qz_654_tbs/2
	.byte		LFOS  , 45
	.byte		N11   , Cn4 , v064
	.byte	W12
	.byte		TEMPO , 82*mus_qz_654_tbs/2
	.byte		LFOS  , 41
	.byte		LFODL , 9
	.byte	W12
	.byte		N11   , Dn4 , v080
	.byte	W12
	.byte		        En4 , v092
	.byte	W12
	.byte		        Fn4 , v084
	.byte	W12
	.byte		        Dn4 , v092
	.byte		N11   , An4
	.byte	W12
	.byte		        Fn4 , v088
	.byte	W12
	.byte		        En4 , v100
	.byte	W12
	.byte		        Dn4 , v088
	.byte	W12
	.byte		        As3 , v084
	.byte	W12
	.byte		        Dn4
	.byte	W12
	.byte		        Fn4 , v096
	.byte	W12
	.byte		        Dn4 , v092
	.byte	W12
	.byte		N44   , En4 , v100 , gtp3
	.byte	W48
	.byte		N11   , Fn4 , v096
	.byte	W12
	.byte		        En4 , v088
	.byte	W12
	.byte		        Dn4 , v092
	.byte	W18
	.byte		N05   , Dn4 , v084
	.byte	W06
	.byte		N44   , En4 , v092 , gtp3
	.byte	W48
	.byte		N11   , Fn4 , v096
	.byte	W12
	.byte		        En4 , v084
	.byte	W12
	.byte		        Fn4 , v092
	.byte	W12
	.byte		N03   , Fn4 , v088
	.byte	W04
	.byte		        An4 , v076
	.byte	W04
	.byte		        Fn4 , v084
	.byte	W04
	.byte		TEMPO , 72*mus_qz_654_tbs/2
	.byte		LFOS  , 46
	.byte		LFODL , 7
	.byte		N44   , En4 , v088 , gtp3
	.byte	W44
	.byte	W03
	.byte		TEMPO , 82*mus_qz_654_tbs/2
	.byte		LFOS  , 41
	.byte		LFODL , 9
	.byte	W01
	.byte	GOTO
	 ptrvalue	mus_qz_654_1_B1
	.byte	FINE

/********************** Track  2 **********************/

mus_qz_654_2:
	.byte		KEYSH , mus_qz_654_key+0
	.byte		VOICE , 42
	.byte		VOL   , 69*mus_qz_654_mvl/mxv
	.byte		PAN   , c_v-44
	.byte		PRIO  , 62
	.byte		MOD   , 1
	.byte		LFOS  , 41
	.byte		LFODL , 8
	.byte	W36
mus_qz_654_2_B1:
	.byte	W24
	.byte		N11   , En3 , v084
	.byte	W12
	.byte		        An3 , v080
	.byte	W12
	.byte		N23   , Bn3 , v100
	.byte	W24
	.byte		        Cn4 , v092
	.byte	W24
	.byte		        Dn4 , v100
	.byte	W24
	.byte		N11   , Bn3 , v088
	.byte	W12
	.byte		        Bn3 , v084
	.byte	W12
	.byte		N23   , Gn3 , v092
	.byte	W24
	.byte		N11   , Gn3 , v080
	.byte	W12
	.byte		        An3 , v072
	.byte	W12
	.byte		N32   , Fn3 , v100 , gtp3
	.byte	W36
	.byte		N11   , An3 , v084
	.byte	W12
	.byte		        As3 , v096
	.byte	W12
	.byte		        An3 , v088
	.byte	W12
	.byte		N23   , En3 , v096
	.byte	W24
	.byte		        Dn3 , v100
	.byte	W36
	.byte		N11   , Dn3 , v088
	.byte	W12
	.byte		LFOS  , 42
	.byte		LFODL , 7
	.byte		N23   , En3 , v096
	.byte	W24
	.byte		LFOS  , 44
	.byte		N11   , En3 , v084
	.byte	W12
	.byte		LFOS  , 45
	.byte		N11   , Gn3 , v076
	.byte	W12
	.byte		LFOS  , 41
	.byte		LFODL , 8
	.byte	W12
	.byte		N32   , An3 , v084 , gtp3
	.byte	W48
	.byte	N11
	.byte	W12
	.byte		        As3 , v092
	.byte	W12
	.byte		        Gn3 , v084
	.byte	W12
	.byte		N23   , En3 , v088
	.byte	W24
	.byte		        As3 , v080
	.byte	W24
	.byte		N44   , An3 , v088 , gtp3
	.byte	W48
	.byte		N23   , As3 , v096
	.byte	W24
	.byte		N11   , Gn3 , v092
	.byte	W12
	.byte		        As3 , v084
	.byte	W12
	.byte		N44   , An3 , v096 , gtp3
	.byte	W48
	.byte		N23   , As3
	.byte	W24
	.byte		N11   , Gn3 , v088
	.byte	W12
	.byte		        Gn3 , v080
	.byte	W12
	.byte		LFOS  , 46
	.byte		LFODL , 6
	.byte		N44   , An3 , v088 , gtp3
	.byte	W44
	.byte	W03
	.byte		LFOS  , 41
	.byte		LFODL , 8
	.byte	W01
	.byte	GOTO
	 ptrvalue	mus_qz_654_2_B1
	.byte	FINE

/********************** Track  3 **********************/

mus_qz_654_3:
	.byte		KEYSH , mus_qz_654_key+0
	.byte		VOICE , 42
	.byte		VOL   , 80*mus_qz_654_mvl/mxv
	.byte		PAN   , c_v+51
	.byte		PRIO  , 60
	.byte		MOD   , 1
	.byte		LFOS  , 41
	.byte		LFODL , 8
	.byte	W36
mus_qz_654_3_B1:
	.byte		N23   , En2 , v100
	.byte	W24
	.byte		N11   , Gn2 , v092
	.byte	W12
	.byte		        Bn2 , v088
	.byte	W12
	.byte		N23   , Cn3 , v100
	.byte	W24
	.byte		        An2 , v088
	.byte	W24
	.byte		        Bn2 , v100
	.byte	W24
	.byte		N11   , Dn3 , v088
	.byte	W12
	.byte		        Fs3 , v084
	.byte	W12
	.byte		N44   , En2 , v096 , gtp3
	.byte	W48
	.byte		N23   , Gn2
	.byte	W24
	.byte		        Fn2 , v088
	.byte	W24
	.byte		        En2 , v096
	.byte	W24
	.byte		N11   , Gn2
	.byte	W12
	.byte		        En2 , v088
	.byte	W12
	.byte		N23   , An2 , v100
	.byte	W24
	.byte		        As2 , v088
	.byte	W24
	.byte		LFOS  , 42
	.byte		LFODL , 7
	.byte		N44   , An2 , v096 , gtp3
	.byte	W24
	.byte		LFOS  , 44
	.byte	W12
	.byte		        45
	.byte	W12
	.byte		        41
	.byte		LFODL , 8
	.byte		N23   , Dn3 , v100
	.byte	W24
	.byte		        Cn3 , v088
	.byte	W24
	.byte		        Bn2 , v092
	.byte	W24
	.byte		N11   , Gn2 , v084
	.byte	W12
	.byte		        As2 , v072
	.byte	W12
	.byte		N23   , En2 , v092
	.byte	W24
	.byte		        Gn2 , v084
	.byte	W24
	.byte		        An2 , v092
	.byte	W24
	.byte		N11   , Dn3 , v084
	.byte	W12
	.byte		        En3 , v080
	.byte	W12
	.byte		N23   , Fn3 , v092
	.byte	W24
	.byte		        En2 , v084
	.byte	W24
	.byte		        An2 , v092
	.byte	W24
	.byte		N11   , Dn3 , v100
	.byte	W12
	.byte		        En3 , v092
	.byte	W12
	.byte		N23   , Fn3
	.byte	W24
	.byte		        Gn2 , v084
	.byte	W24
	.byte		LFOS  , 46
	.byte		LFODL , 6
	.byte		N44   , An2 , v088 , gtp3
	.byte	W44
	.byte	W03
	.byte		LFOS  , 41
	.byte		LFODL , 8
	.byte	W01
	.byte	GOTO
	 ptrvalue	mus_qz_654_3_B1
	.byte	FINE

/********************** Track  4 **********************/

mus_qz_654_4:
	.byte		KEYSH , mus_qz_654_key+0
	.byte		VOICE , 20
	.byte		VOL   , 80*mus_qz_654_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		BENDR , 6
	.byte		PRIO  , 58
	.byte		VOL   , 80*mus_qz_654_mvl/mxv
	.byte	W36
mus_qz_654_4_B1:
	.byte		N44   , En1 , v108 , gtp3
	.byte	W36
	.byte		VOL   , 71*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        63*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        80*mus_qz_654_mvl/mxv
	.byte		BEND  , c_v-22
	.byte		N44   , An1 , v108 , gtp3
	.byte	W03
	.byte		BEND  , c_v-11
	.byte	W03
	.byte		        c_v+0
	.byte	W30
	.byte		VOL   , 71*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        63*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        80*mus_qz_654_mvl/mxv
	.byte		BEND  , c_v-22
	.byte		N44   , Bn1 , v116 , gtp3
	.byte	W03
	.byte		BEND  , c_v-11
	.byte	W03
	.byte		        c_v-5
	.byte	W03
	.byte		        c_v+0
	.byte	W24
	.byte	W03
	.byte		VOL   , 71*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        63*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        80*mus_qz_654_mvl/mxv
	.byte		N44   , Cn2 , v112 , gtp3
	.byte	W36
	.byte		VOL   , 71*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        63*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        80*mus_qz_654_mvl/mxv
	.byte		N23   , Gn1 , v120
	.byte	W24
	.byte		        Fn1 , v108
	.byte	W24
	.byte		N44   , En1 , v116 , gtp3
	.byte	W32
	.byte	W03
	.byte		VOL   , 71*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        63*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        80*mus_qz_654_mvl/mxv
	.byte	W01
	.byte		BEND  , c_v-22
	.byte		N92   , An1 , v112 , gtp3
	.byte	W03
	.byte		BEND  , c_v-11
	.byte	W03
	.byte		        c_v+0
	.byte	W78
	.byte		VOL   , 71*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        63*mus_qz_654_mvl/mxv
	.byte	W05
	.byte		BEND  , c_v-22
	.byte	W01
	.byte		VOL   , 80*mus_qz_654_mvl/mxv
	.byte		N23   , Dn2 , v116
	.byte	W02
	.byte		BEND  , c_v-11
	.byte	W03
	.byte		        c_v+0
	.byte	W19
	.byte		N23   , Cn2 , v104
	.byte	W24
	.byte		        Bn1 , v112
	.byte	W24
	.byte		        Gn1 , v108
	.byte	W24
	.byte		N44   , En1 , v116 , gtp3
	.byte	W36
	.byte		VOL   , 71*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        63*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        80*mus_qz_654_mvl/mxv
	.byte		BEND  , c_v-22
	.byte		N44   , An1 , v108 , gtp3
	.byte	W03
	.byte		BEND  , c_v-11
	.byte	W03
	.byte		        c_v+0
	.byte	W30
	.byte		VOL   , 71*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        63*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        80*mus_qz_654_mvl/mxv
	.byte		N23   , As1 , v116
	.byte	W24
	.byte		        Gn1
	.byte	W12
	.byte		VOL   , 71*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        63*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        80*mus_qz_654_mvl/mxv
	.byte		N44   , An1 , v108 , gtp3
	.byte	W36
	.byte		VOL   , 71*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        63*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        80*mus_qz_654_mvl/mxv
	.byte		N44   , Gn1 , v120 , gtp3
	.byte	W36
	.byte		VOL   , 71*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        63*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        80*mus_qz_654_mvl/mxv
	.byte		N44   , An1 , v112 , gtp3
	.byte	W36
	.byte		VOL   , 71*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        63*mus_qz_654_mvl/mxv
	.byte	W06
	.byte	GOTO
	 ptrvalue	mus_qz_654_4_B1
	.byte		VOL   , 80*mus_qz_654_mvl/mxv
	.byte	FINE

/********************** Track  5 **********************/

mus_qz_654_5:
	.byte		KEYSH , mus_qz_654_key+0
	.byte		VOICE , 23
	.byte		VOL   , 36*mus_qz_654_mvl/mxv
	.byte		PAN   , c_v-34
	.byte		PRIO  , 48
	.byte		MOD   , 1
	.byte		LFOS  , 41
	.byte		LFODL , 7
	.byte		VOL   , 34*mus_qz_654_mvl/mxv
	.byte	W36
mus_qz_654_5_B1:
	.byte		VOL   , 34*mus_qz_654_mvl/mxv
	.byte		N44   , Gn1 , v124 , gtp3
	.byte		N44   , En2 , v096 , gtp3
	.byte		N44   , Bn2 , v108 , gtp3
	.byte	W36
	.byte		VOL   , 33*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        31*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        34*mus_qz_654_mvl/mxv
	.byte		N23   , An1 , v120
	.byte		N23   , En2 , v092
	.byte		N23   , Cn3 , v104
	.byte	W24
	.byte		        An1 , v112
	.byte		N23   , An2 , v084
	.byte		N23   , En3 , v096
	.byte	W24
	.byte		N44   , Bn1 , v124 , gtp3
	.byte		N44   , Fs2 , v096 , gtp3
	.byte		N23   , Dn3 , v108
	.byte	W24
	.byte		        Fs3 , v100
	.byte	W12
	.byte		VOL   , 32*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        30*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        36*mus_qz_654_mvl/mxv
	.byte		N44   , Cn2 , v120 , gtp3
	.byte		N44   , Gn2 , v088 , gtp3
	.byte		N44   , Cn3 , v100 , gtp3
	.byte	W24
	.byte		VOL   , 33*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        31*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        29*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        24*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        36*mus_qz_654_mvl/mxv
	.byte		N92   , Gn1 , v124 , gtp3
	.byte		N92   , Dn2 , v096 , gtp3
	.byte		N92   , As2 , v108 , gtp3
	.byte	W36
	.byte		VOL   , 34*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        33*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        31*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        30*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        29*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        27*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        25*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        24*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        22*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        21*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        19*mus_qz_654_mvl/mxv
	.byte		N92   , An1 , v116 , gtp3
	.byte		N92   , En2 , v088 , gtp3
	.byte		N92   , An2 , v100 , gtp3
	.byte	W06
	.byte		VOL   , 21*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        22*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        23*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        24*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        25*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        27*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        28*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        29*mus_qz_654_mvl/mxv
	.byte		LFOS  , 42
	.byte		LFODL , 6
	.byte	W06
	.byte		VOL   , 31*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        32*mus_qz_654_mvl/mxv
	.byte	W12
	.byte		LFOS  , 44
	.byte	W06
	.byte		VOL   , 30*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        28*mus_qz_654_mvl/mxv
	.byte		LFOS  , 45
	.byte	W06
	.byte		VOL   , 26*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        33*mus_qz_654_mvl/mxv
	.byte		LFOS  , 41
	.byte		LFODL , 7
	.byte		N23   , Dn2 , v124
	.byte		N23   , An2 , v096
	.byte		N23   , Dn3 , v108
	.byte	W24
	.byte		        Cn2 , v112
	.byte		N23   , Gn2 , v084
	.byte		N23   , Cn3 , v096
	.byte	W24
	.byte		        Bn1 , v124
	.byte		N23   , Fn2 , v096
	.byte		N23   , Bn2 , v108
	.byte	W24
	.byte		        Gn1 , v112
	.byte		N23   , Dn2 , v084
	.byte		N23   , Gn2 , v096
	.byte	W24
	.byte		N44   , En1 , v124 , gtp3
	.byte		N44   , En2 , v096 , gtp3
	.byte		N44   , As2 , v108 , gtp3
	.byte	W36
	.byte		VOL   , 31*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        30*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        29*mus_qz_654_mvl/mxv
	.byte		N44   , An1 , v116 , gtp3
	.byte		N44   , En2 , v088 , gtp3
	.byte		N44   , An2 , v100 , gtp3
	.byte	W06
	.byte		VOL   , 27*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        25*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        24*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        22*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        21*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        20*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        19*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        19*mus_qz_654_mvl/mxv
	.byte		N44   , Gn1 , v124 , gtp3
	.byte		N44   , En2 , v096 , gtp3
	.byte		N44   , As2 , v108 , gtp3
	.byte	W06
	.byte		VOL   , 20*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        21*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        22*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        22*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        24*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        24*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        26*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        27*mus_qz_654_mvl/mxv
	.byte		N44   , An1 , v116 , gtp3
	.byte		N44   , En2 , v088 , gtp3
	.byte		N44   , An2 , v100 , gtp3
	.byte	W06
	.byte		VOL   , 29*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        30*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        31*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        31*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        29*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        27*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        24*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        22*mus_qz_654_mvl/mxv
	.byte		N44   , As1 , v124 , gtp3
	.byte		N44   , En2 , v096 , gtp3
	.byte		N44   , As2 , v108 , gtp3
	.byte	W06
	.byte		VOL   , 20*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        18*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        20*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        21*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        22*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        24*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        26*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        28*mus_qz_654_mvl/mxv
	.byte		LFOS  , 46
	.byte		LFODL , 6
	.byte		N44   , An1 , v116 , gtp3
	.byte		N44   , En2 , v088 , gtp3
	.byte		N44   , An2 , v100 , gtp3
	.byte	W06
	.byte		VOL   , 31*mus_qz_654_mvl/mxv
	.byte	W06
	.byte		        33*mus_qz_654_mvl/mxv
	.byte	W24
	.byte		        31*mus_qz_654_mvl/mxv
	.byte	W03
	.byte		        28*mus_qz_654_mvl/mxv
	.byte	W03
	.byte		        25*mus_qz_654_mvl/mxv
	.byte	W03
	.byte		        22*mus_qz_654_mvl/mxv
	.byte	W02
	.byte		        33*mus_qz_654_mvl/mxv
	.byte		LFOS  , 41
	.byte		LFODL , 7
	.byte	W01
	.byte	GOTO
	 ptrvalue	mus_qz_654_5_B1
	.byte	FINE

/********************** Track  6 **********************/

mus_qz_654_6:
	.byte		KEYSH , mus_qz_654_key+0
	.byte		VOICE , 28
	.byte		VOL   , 85*mus_qz_654_mvl/mxv
	.byte		PAN   , c_v-24
	.byte		PRIO  , 64
	.byte	W12
	.byte		N01   , Fn1 , v008
	.byte		N01   , Ds2
	.byte	W01
	.byte		        Fn1 , v004
	.byte		N01   , Ds2
	.byte	W02
	.byte		        Fn1 , v012
	.byte		N01   , Ds2
	.byte	W01
	.byte		        Fn1 , v008
	.byte		N01   , Ds2
	.byte	W02
	.byte		        Fn1 , v020
	.byte		N01   , Ds2
	.byte	W01
	.byte		        Fn1 , v016
	.byte		N01   , Ds2
	.byte	W02
	.byte		        Fn1 , v028
	.byte		N01   , Ds2
	.byte	W01
	.byte		        Fn1 , v024
	.byte		N01   , Ds2
	.byte	W02
	.byte		        Fn1 , v044
	.byte		N01   , Ds2
	.byte	W01
	.byte		        Fn1 , v036
	.byte		N01   , Ds2
	.byte	W02
	.byte		        Fn1 , v056
	.byte		N01   , Ds2
	.byte	W01
	.byte		        Fn1 , v048
	.byte		N01   , Ds2
	.byte	W02
	.byte		        Fn1 , v076
	.byte		N01   , Ds2
	.byte	W01
	.byte		        Fn1 , v068
	.byte		N01   , Ds2
	.byte	W02
	.byte		        Fn1 , v100
	.byte		N01   , Ds2
	.byte	W01
	.byte		        Fn1 , v080
	.byte		N01   , Ds2
	.byte	W02
mus_qz_654_6_B1:
	.byte		N32   , Fn1 , v124 , gtp3
	.byte		N32   , Ds2 , v124 , gtp3
	.byte	W96
	.byte	PEND
	.byte		        Fn1 , v124 , gtp3
	.byte		N32   , Ds2 , v124 , gtp3
	.byte	W48
	.byte		N23   , Fn1
	.byte		N23   , Ds2
	.byte	W32
	.byte	W03
	.byte		N05   , Fn1 , v112
	.byte		N05   , Ds2
	.byte	W06
	.byte		        Fn1 , v100
	.byte		N05   , Ds2
	.byte	W07
	.byte	PATT
	 ptrvalue	mus_qz_654_6_B1
	.byte		N32   , Fn1 , v124 , gtp3
	.byte		N32   , Ds2 , v124 , gtp3
	.byte	W84
	.byte		N01   , Ds2 , v028
	.byte	W01
	.byte		        Ds2 , v016
	.byte	W02
	.byte		        Ds2 , v048
	.byte	W01
	.byte		        Ds2 , v032
	.byte	W02
	.byte		        Ds2 , v072
	.byte	W01
	.byte		        Ds2 , v056
	.byte	W02
	.byte		        Ds2 , v092
	.byte	W01
	.byte		        Ds2 , v112
	.byte	W02
	.byte	PATT
	 ptrvalue	mus_qz_654_6_B1
	.byte		N32   , Fn1 , v124 , gtp3
	.byte		N32   , Ds2 , v124 , gtp3
	.byte	W48
	.byte		N23   , Fn1
	.byte		N23   , Ds2
	.byte	W36
	.byte		N05   , Fn1
	.byte		N05   , Ds2
	.byte	W06
	.byte		        Fn1 , v100
	.byte		N05   , Ds2
	.byte	W06
	.byte	PATT
	 ptrvalue	mus_qz_654_6_B1
	.byte		N32   , Fn1 , v124 , gtp3
	.byte		N32   , Ds2 , v124 , gtp3
	.byte	W72
	.byte		N01   , Fn1 , v012
	.byte		N01   , Ds2
	.byte	W01
	.byte		        Fn1 , v004
	.byte		N01   , Ds2
	.byte	W02
	.byte		        Fn1 , v016
	.byte		N01   , Ds2
	.byte	W01
	.byte		        Fn1 , v012
	.byte		N01   , Ds2
	.byte	W02
	.byte		        Fn1 , v028
	.byte		N01   , Ds2
	.byte	W01
	.byte		        Fn1 , v020
	.byte		N01   , Ds2
	.byte	W02
	.byte		        Fn1 , v036
	.byte		N01   , Ds2
	.byte	W01
	.byte		        Fn1 , v028
	.byte		N01   , Ds2
	.byte	W02
	.byte		        Fn1 , v052
	.byte		N01   , Ds2
	.byte	W01
	.byte		        Fn1 , v044
	.byte		N01   , Ds2
	.byte	W02
	.byte		        Fn1 , v064
	.byte		N01   , Ds2
	.byte	W01
	.byte		        Fn1 , v056
	.byte		N01   , Ds2
	.byte	W02
	.byte		        Fn1 , v088
	.byte		N01   , Ds2
	.byte	W01
	.byte		        Fn1 , v080
	.byte		N01   , Ds2
	.byte	W02
	.byte		        Fn1 , v112
	.byte		N01   , Ds2
	.byte	W01
	.byte		        Fn1 , v088
	.byte		N01   , Ds2
	.byte	W02
	.byte	GOTO
	 ptrvalue	mus_qz_654_6_B1
	.byte	FINE

/********************** Track  7 **********************/

mus_qz_654_7:
	.byte		KEYSH , mus_qz_654_key+0
	.byte		VOICE , 39
	.byte		VOL   , 80*mus_qz_654_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte		VOL   , 80*mus_qz_654_mvl/mxv
	.byte		N32   , Gn0 , v127 , gtp3
	.byte	W32
	.byte	W03
	.byte		VOL   , 62*mus_qz_654_mvl/mxv
	.byte	W01
mus_qz_654_7_B1:
	.byte		N32   , Cn2 , v116 , gtp3
	.byte	W96
mus_qz_654_7_000:
	.byte	W12
	.byte		N05   , Gn3 , v040
	.byte	W12
	.byte		        Gn3 , v024
	.byte	W12
	.byte		        Gn3 , v032
	.byte	W60
	.byte	PEND
	.byte	W96
	.byte	W96
	.byte		N32   , Cn2 , v112 , gtp3
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_654_7_000
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_654_7_B1
	.byte	FINE

/********************** Track  8 **********************/

mus_qz_654_8:
	.byte		KEYSH , mus_qz_654_key+0
	.byte		VOICE , 42
	.byte		VOL   , 18*mus_qz_654_mvl/mxv
	.byte		PAN   , c_v-54
	.byte		PRIO  , 40
	.byte		MOD   , 3
	.byte		LFOS  , 41
	.byte		LFODL , 9
	.byte	W36
mus_qz_654_8_B1:
	.byte	W36
	.byte		N11   , Bn3 , v104
	.byte	W12
	.byte		        Dn4 , v096
	.byte	W12
	.byte		N23   , En4 , v108
	.byte	W24
	.byte		        Bn4 , v104
	.byte	W12
	.byte	W12
	.byte		N11   , An4 , v100
	.byte	W12
	.byte		        Gn4 , v088
	.byte	W12
	.byte		        En4 , v096
	.byte	W12
	.byte		        Dn4 , v088
	.byte	W12
	.byte		N44   , En4 , v100 , gtp3
	.byte	W36
	.byte	W12
	.byte		N32   , As3 , v096 , gtp3
	.byte	W36
	.byte		N11   , Dn4 , v088
	.byte	W12
	.byte		        En4 , v096
	.byte	W12
	.byte		        Dn4 , v088
	.byte	W12
	.byte		        As3 , v092
	.byte	W12
	.byte		        Gn3 , v084
	.byte	W12
	.byte		N23   , An3 , v100
	.byte	W24
	.byte		N11   , Gn3 , v092
	.byte	W12
	.byte		LFOS  , 42
	.byte		LFODL , 8
	.byte	W24
	.byte		LFOS  , 44
	.byte	W12
	.byte		        45
	.byte	W12
	.byte		        41
	.byte		LFODL , 9
	.byte	W24
	.byte		N11   , Dn4 , v080
	.byte	W12
	.byte		        En4 , v092
	.byte	W12
	.byte		        Fn4 , v084
	.byte	W12
	.byte		        Dn4 , v092
	.byte		N11   , An4
	.byte	W12
	.byte		        Fn4 , v088
	.byte	W12
	.byte		        En4 , v100
	.byte	W12
	.byte		        Dn4 , v088
	.byte	W12
	.byte		        As3 , v084
	.byte	W12
	.byte		        Dn4
	.byte	W12
	.byte		        Fn4 , v096
	.byte	W12
	.byte		        Dn4 , v092
	.byte	W12
	.byte		N44   , En4 , v100 , gtp3
	.byte	W36
	.byte	W12
	.byte		N11   , Fn4 , v096
	.byte	W12
	.byte		        En4 , v088
	.byte	W12
	.byte		        Dn4 , v092
	.byte	W18
	.byte		N05   , Dn4 , v084
	.byte	W06
	.byte		N44   , En4 , v092 , gtp3
	.byte	W36
	.byte	W12
	.byte		N11   , Fn4 , v096
	.byte	W12
	.byte		        En4 , v084
	.byte	W12
	.byte		        Fn4 , v092
	.byte	W12
	.byte		LFOS  , 46
	.byte		LFODL , 7
	.byte		N03   , Fn4 , v088
	.byte	W04
	.byte		        An4 , v076
	.byte	W04
	.byte		        Fn4 , v084
	.byte	W04
	.byte		N44   , En4 , v088 , gtp3
	.byte	W32
	.byte	W03
	.byte		LFOS  , 41
	.byte		LFODL , 9
	.byte	W01
	.byte	GOTO
	 ptrvalue	mus_qz_654_8_B1
	.byte	W11
	.byte	FINE

/******************************************************/
	.p2align	2

mus_qz_654:
	.byte	8	/* NumTrks */
	.byte	0	/* NumBlks */
	.byte	mus_qz_654_pri	/* Priority */
	.byte	mus_qz_654_rev	/* Reverb. */
space64 4
	ptrvalue	mus_qz_654_grp

	ptrvalue	mus_qz_654_1
	ptrvalue	mus_qz_654_2
	ptrvalue	mus_qz_654_3
	ptrvalue	mus_qz_654_4
	ptrvalue	mus_qz_654_5
	ptrvalue	mus_qz_654_6
	ptrvalue	mus_qz_654_7
	ptrvalue	mus_qz_654_8

	.end
