	.include "asm/macros/bit_width.inc"

	.include "MPlayDef.s"

/* Moemon Quetzal song 625, disassembled from the ROM by tools/quetzal_rip/songs.py.
   "org" comments give the ROM address of the bytes that follow. */

	.equ	mus_qz_625_grp, voicegroup_qz_songs
	.equ	mus_qz_625_pri, 0
	.equ	mus_qz_625_rev, reverb_set+0
	.equ	mus_qz_625_mvl, 127
	.equ	mus_qz_625_key, 0
	.equ	mus_qz_625_tbs, 1
	.equ	mus_qz_625_exg, 1
	.equ	mus_qz_625_cmp, 1

	.section .rodata
	.global	mus_qz_625
	.p2align	2
/* org 0x0967CCC8 */

/********************** Track  1 **********************/

mus_qz_625_1:
	.byte		KEYSH , mus_qz_625_key+0
	.byte		TEMPO , 136*mus_qz_625_tbs/2
	.byte		VOICE , 35
	.byte		VOL   , 66*mus_qz_625_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		BENDR , 6
	.byte		PRIO  , 64
	.byte		BEND  , c_v+1
	.byte		N05   , Cs4 , v072
	.byte		N05   , Fs4 , v092
	.byte	W24
	.byte		        Cs4 , v064
	.byte		N05   , Fs4 , v084
	.byte	W24
	.byte		        Cn4 , v072
	.byte		N05   , Fs4 , v092
	.byte	W12
	.byte		        Cn4 , v064
	.byte		N05   , Fs4 , v084
	.byte	W24
	.byte		        Bn3 , v064
	.byte		N05   , Fs4 , v092
	.byte	W12
	.byte	W24
	.byte		TEMPO , 114*mus_qz_625_tbs/2
	.byte		N30   , As3 , v056
	.byte		N30   , Fs4 , v084
	.byte	W36
	.byte		TEMPO , 136*mus_qz_625_tbs/2
	.byte		N11   , Fs4 , v092
	.byte	W12
	.byte		N05   , Gs4 , v080
	.byte	W12
	.byte		        As4 , v088
	.byte	W12
mus_qz_625_1_B1:
	.byte		TEMPO , 138*mus_qz_625_tbs/2
	.byte		N17   , Cs5 , v092
	.byte	W18
	.byte		N03   , Fs4 , v072
	.byte	W06
	.byte		N05   , Ds5 , v064
	.byte	W12
	.byte		N28   , Bn4 , v088 , gtp1
	.byte	W48
	.byte		N05   , Bn4 , v092
	.byte	W06
	.byte		        Cs5
	.byte	W06
	.byte		N11   , Ds5
	.byte	W12
	.byte		N05   , Dn5
	.byte	W12
	.byte		        Ds5
	.byte	W12
	.byte		N23   , En5
	.byte	W36
	.byte		N05   , Ds5 , v072
	.byte		N05   , Ds5 , v088
	.byte	W06
	.byte		        En5 , v092
	.byte	W06
	.byte		        Fs5
	.byte	W12
	.byte		N17   , Cs5 , v076
	.byte	W18
	.byte		N03   , Fs4 , v080
	.byte	W06
	.byte		N05   , Ds5 , v088
	.byte	W12
	.byte		N28   , Bn4 , v072 , gtp1
	.byte	W48
	.byte		N05   , Bn4 , v084
	.byte	W06
	.byte		        Cs5 , v072
	.byte	W06
	.byte		N11   , Ds5 , v088
	.byte	W12
	.byte		N05   , Dn5
	.byte	W12
	.byte		        Ds5 , v092
	.byte	W12
	.byte		N23   , Gs5
	.byte	W36
	.byte		N05   , En5 , v072
	.byte	W06
	.byte		        Ds5 , v092
	.byte	W06
	.byte		        Cs5
	.byte	W12
	.byte		N17   , Ds5
	.byte	W18
	.byte		N05   , Gs4 , v088
	.byte	W06
	.byte		        En5
	.byte	W12
	.byte		N32   , Cs5 , v088 , gtp3
	.byte	W60
	.byte	W12
	.byte		N28   , Gs4 , v092 , gtp1
	.byte	W36
	.byte		N05   , As4 , v100
	.byte	W24
	.byte		        Bn4
	.byte	W24
	.byte		N32   , Ds5 , v096 , gtp3
	.byte	W36
	.byte		N05   , En5 , v092
	.byte	W06
	.byte		        Ds5 , v080
	.byte	W06
	.byte		        Cs5 , v092
	.byte	W12
	.byte		N11   , Bn4
	.byte	W12
	.byte		        As4
	.byte	W12
	.byte		        Bn4
	.byte	W12
	.byte		        Cs5
	.byte	W44
	.byte	W01
	.byte		N02   , As5 , v060
	.byte	W03
	.byte		N11   , Bn5 , v112
	.byte	W12
	.byte		        Fs5
	.byte	W12
	.byte		        En5
	.byte	W12
	.byte		        Ds5
	.byte	W12
	.byte		N17   , Cs5
	.byte	W18
	.byte		N03   , Fs4 , v088
	.byte	W06
	.byte		N05   , Ds5
	.byte	W12
	.byte		N17   , Bn4 , v108
	.byte	W24
	.byte		N05   , Bn4 , v020
	.byte	W24
	.byte		        Bn4 , v112
	.byte	W06
	.byte		        Cs5
	.byte	W06
	.byte		N17   , Ds5
	.byte	W36
	.byte		N23   , En5
	.byte	W24
	.byte		N11   , Ds5 , v108
	.byte	W12
	.byte		N05   , En5 , v100
	.byte	W12
	.byte		        Fs5 , v108
	.byte	W12
	.byte		N17   , Cs5 , v084
	.byte	W18
	.byte		N03   , Fs4 , v088
	.byte	W06
	.byte		N05   , Ds5 , v108
	.byte	W12
	.byte		N17   , Bn4 , v088
	.byte	W24
	.byte		N05   , Bn4 , v020
	.byte	W24
	.byte		        Bn4 , v112
	.byte	W06
	.byte		        Cs5
	.byte	W06
	.byte		N17   , Ds5 , v108
	.byte	W36
	.byte		N23   , Gs5 , v112
	.byte	W24
	.byte		N17   , Fs5 , v092
	.byte	W24
	.byte		N05   , Bn4 , v100
	.byte	W06
	.byte		        Cs5 , v088
	.byte	W06
	.byte		N17   , Ds5 , v112
	.byte	W18
	.byte		N05   , Gs4 , v104
	.byte	W06
	.byte		        En5
	.byte	W12
	.byte		N28   , Cs5 , v104 , gtp1
	.byte	W60
	.byte	W12
	.byte		N11   , Gs4 , v112
	.byte	W12
	.byte		        Gn4 , v100
	.byte	W12
	.byte		        Gs4 , v112
	.byte	W12
	.byte		        As4
	.byte	W12
	.byte		        Gs4
	.byte	W12
	.byte		        As4
	.byte	W12
	.byte		N05   , Bn4 , v120
	.byte	W12
	.byte		N32   , Ds5 , v112 , gtp3
	.byte	W36
	.byte		N05   , En5 , v120
	.byte	W06
	.byte		        Ds5 , v104
	.byte	W06
	.byte		        Cs5 , v092
	.byte	W12
	.byte		        Bn4 , v120
	.byte	W12
	.byte		N11   , As4
	.byte	W12
	.byte		N05   , Bn4
	.byte	W12
	.byte		N08   , Cs5
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
	 ptrvalue	mus_qz_625_1_B1
	.byte	FINE

/********************** Track  2 **********************/

mus_qz_625_2:
	.byte		KEYSH , mus_qz_625_key+0
	.byte		VOICE , 3
	.byte		VOL   , 47*mus_qz_625_mvl/mxv
	.byte		PAN   , c_v-42
	.byte		PRIO  , 62
	.byte		N05   , As3 , v080
	.byte		N05   , Ds4
	.byte	W24
	.byte		        As3 , v072
	.byte		N05   , Ds4
	.byte	W24
	.byte		        An3 , v080
	.byte		N05   , Dn4
	.byte	W12
	.byte		        An3 , v072
	.byte		N05   , Dn4
	.byte	W24
	.byte		        Gs3 , v068
	.byte		N05   , Ds4
	.byte	W12
	.byte	W24
	.byte		N28   , En3 , v080 , gtp1
	.byte		N28   , Cs4 , v080 , gtp1
	.byte	W36
	.byte		N11   , As2 , v072
	.byte		N11   , Fs3 , v084
	.byte	W12
	.byte		N05   , Bn2 , v068
	.byte		N05   , Gs3 , v080
	.byte	W12
	.byte		        Cs3 , v072
	.byte		N05   , As3 , v084
	.byte	W12
mus_qz_625_2_B1:
	.byte		N04   , Ds3 , v084
	.byte		N04   , As3 , v092
	.byte	W24
	.byte		N23   , Ds3 , v076
	.byte		N23   , As3 , v088
	.byte	W36
	.byte		N04   , Ds3 , v072
	.byte		N04   , As3 , v084
	.byte	W36
	.byte		N23   , Ds3 , v088
	.byte		N23   , Gs3
	.byte	W36
	.byte		N04   , Ds3 , v084
	.byte		N04   , As3 , v092
	.byte	W24
	.byte		N28   , Ds3 , v076 , gtp1
	.byte		N28   , Gs3 , v088 , gtp1
	.byte	W36
	.byte		N04   , Ds3 , v084
	.byte		N04   , An3 , v092
	.byte	W24
	.byte		N23   , Ds3 , v084
	.byte		N23   , An3 , v092
	.byte	W36
	.byte		N04   , Ds3 , v072
	.byte		N04   , An3 , v084
	.byte	W36
	.byte		N23   , Ds3 , v088
	.byte		N23   , Gs3
	.byte	W36
	.byte		N04   , Ds3 , v076
	.byte		N04   , An3 , v088
	.byte	W24
	.byte		N28   , Ds3 , v084 , gtp1
	.byte		N28   , Gs3 , v092 , gtp1
	.byte	W36
	.byte		N04   , Ds3 , v084
	.byte		N04   , Gs3 , v092
	.byte	W24
	.byte		N23   , Ds3 , v084
	.byte		N23   , Gs3 , v092
	.byte	W36
	.byte		N04   , Ds3 , v068
	.byte		N04   , Gs3 , v076
	.byte	W36
	.byte	W12
	.byte		N28   , Dn3 , v088 , gtp1
	.byte		N28   , Gs3 , v088 , gtp1
	.byte	W36
	.byte		N05   , Dn3 , v084
	.byte		N05   , As3 , v092
	.byte	W24
	.byte		        Dn3 , v084
	.byte		N05   , Bn3 , v092
	.byte	W24
	.byte		        En3 , v084
	.byte		N05   , As3 , v092
	.byte	W24
	.byte		N23   , En3 , v084
	.byte		N23   , As3 , v092
	.byte	W36
	.byte		N11   , En3 , v084
	.byte		N11   , Gs3 , v092
	.byte	W12
	.byte		N05   , As3
	.byte	W12
	.byte		        En3 , v084
	.byte		N05   , Bn3 , v092
	.byte	W12
	.byte		        En3 , v084
	.byte		N05   , As3 , v092
	.byte	W96
	.byte		N04   , Ds3 , v080
	.byte		N04   , As3 , v092
	.byte	W24
	.byte		N23   , Ds3 , v080
	.byte		N23   , As3 , v092
	.byte	W36
	.byte		N04   , Ds3 , v076
	.byte		N04   , As3 , v088
	.byte	W36
	.byte		N23   , Ds3 , v084
	.byte		N23   , Gs3 , v088
	.byte	W36
	.byte		N04   , Ds3 , v080
	.byte		N04   , As3 , v092
	.byte	W24
	.byte		N28   , Ds3 , v080 , gtp1
	.byte		N28   , Gs3 , v092 , gtp1
	.byte	W36
	.byte		N04   , Ds3 , v080
	.byte		N04   , An3 , v092
	.byte	W24
	.byte		N23   , Ds3 , v080
	.byte		N23   , An3 , v092
	.byte	W36
	.byte		N04   , Ds3 , v076
	.byte		N04   , An3 , v088
	.byte	W36
	.byte		N23   , Ds3 , v084
	.byte		N23   , Gs3 , v088
	.byte	W36
	.byte		N04   , Ds3 , v076
	.byte		N04   , An3 , v088
	.byte	W24
	.byte		N28   , Ds3 , v080 , gtp1
	.byte		N28   , Bn3 , v092 , gtp1
	.byte	W36
	.byte		N04   , Ds3 , v080
	.byte		N04   , Gs3 , v092
	.byte	W24
	.byte		N23   , Ds3 , v080
	.byte		N23   , Gs3 , v092
	.byte	W36
	.byte		N04   , Ds3 , v076
	.byte		N04   , Gs3 , v088
	.byte	W36
	.byte	W12
	.byte		N28   , Bn2 , v084 , gtp1
	.byte		N28   , Gs3 , v088 , gtp1
	.byte	W36
	.byte		N05   , Cs3 , v080
	.byte		N05   , As3 , v092
	.byte	W24
	.byte		        Cs3 , v080
	.byte		N05   , Bn3 , v092
	.byte	W24
	.byte		        En3 , v080
	.byte		N05   , As3 , v092
	.byte	W24
	.byte		N23   , En3 , v080
	.byte		N23   , As3 , v092
	.byte	W36
	.byte		N11   , En3 , v080
	.byte		N11   , Gs3 , v092
	.byte	W12
	.byte		N05   , As3
	.byte	W12
	.byte		        En3 , v080
	.byte		N05   , Bn3 , v092
	.byte	W12
	.byte		N08   , En3 , v080
	.byte		N08   , As3 , v092
	.byte	W54
	.byte		N04   , En3 , v060
	.byte		N04   , Gs3 , v072
	.byte	W18
	.byte		N23   , En3 , v080
	.byte		N23   , Cs4 , v092
	.byte	W24
mus_qz_625_2_000:
	.byte		N04   , Cs3 , v080
	.byte		N04   , As3 , v092
	.byte	W24
	.byte		N23   , Cs3 , v080
	.byte		N23   , As3 , v092
	.byte	W36
	.byte		N04   , Cs3 , v072
	.byte		N04   , As3 , v084
	.byte	W36
	.byte	PEND
mus_qz_625_2_001:
	.byte		N23   , Cs3 , v072
	.byte		N23   , Gn3 , v100
	.byte	W36
	.byte		N04   , Cs3 , v072
	.byte		N04   , Gs3 , v084
	.byte	W24
	.byte		N28   , Cs3 , v080 , gtp1
	.byte		N28   , As3 , v092 , gtp1
	.byte	W36
	.byte	PEND
	.byte		N04   , Ds3 , v080
	.byte		N04   , Bn3 , v092
	.byte	W24
	.byte		N23   , Ds3 , v080
	.byte		N23   , Bn3 , v092
	.byte	W36
	.byte		N04   , Ds3 , v072
	.byte		N05   , As3 , v084
	.byte	W36
	.byte		N23   , Ds3
	.byte		N23   , Gs3 , v088
	.byte	W36
	.byte		N04   , Ds3 , v072
	.byte		N04   , As3 , v084
	.byte	W24
	.byte		N28   , Ds3 , v080 , gtp1
	.byte		N28   , Bn3 , v092 , gtp1
	.byte	W36
	.byte	PATT
	 ptrvalue	mus_qz_625_2_000
	.byte	PATT
	 ptrvalue	mus_qz_625_2_001
mus_qz_625_2_002:
	.byte		N04   , Ds3 , v080
	.byte		N04   , Bn3 , v092
	.byte	W24
	.byte		N23   , Ds3 , v080
	.byte		N23   , Bn3 , v092
	.byte	W36
	.byte		N04   , Ds3 , v072
	.byte		N04   , Bn3 , v084
	.byte	W36
	.byte	PEND
	.byte		N23   , Bn2
	.byte		N23   , Gs3 , v088
	.byte	W36
	.byte		N04   , Cs3 , v072
	.byte		N04   , As3 , v084
	.byte	W24
	.byte		N28   , Ds3 , v080 , gtp1
	.byte		N28   , Bn3 , v092 , gtp1
	.byte	W36
	.byte	PATT
	 ptrvalue	mus_qz_625_2_002
	.byte		N23   , Dn3 , v072
	.byte		N23   , Gs3 , v100
	.byte	W36
	.byte		N04   , Dn3 , v072
	.byte		N04   , As3 , v084
	.byte	W24
	.byte		N28   , Cs3 , v080 , gtp1
	.byte		N28   , Bn3 , v092 , gtp1
	.byte	W36
	.byte		N04   , En3 , v080
	.byte		N04   , As3 , v092
	.byte	W24
	.byte		N23   , En3 , v080
	.byte		N23   , As3 , v092
	.byte	W36
	.byte		N04   , En3 , v072
	.byte		N05   , Gs3 , v084
	.byte	W36
	.byte		N11   , En3
	.byte		N11   , As3 , v088
	.byte	W12
	.byte		N04   , En3 , v080
	.byte		N04   , Gs3 , v092
	.byte	W24
	.byte		N32   , En3 , v080 , gtp3
	.byte		N32   , Fs3 , v092 , gtp3
	.byte	W60
	.byte	GOTO
	 ptrvalue	mus_qz_625_2_B1
	.byte	FINE

/********************** Track  3 **********************/

mus_qz_625_3:
	.byte		KEYSH , mus_qz_625_key+0
	.byte		VOICE , 20
	.byte		VOL   , 76*mus_qz_625_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 58
	.byte		N05   , Ds3 , v116
	.byte	W24
	.byte	N05
	.byte	W24
	.byte		        Dn3
	.byte	W12
	.byte	N05
	.byte	W24
	.byte		        Cs3
	.byte	W12
	.byte	W24
	.byte		N30   , Fs2 , v108 , gtp1
	.byte	W36
	.byte	N11
	.byte	W12
	.byte		N05   , Cs3
	.byte	W12
	.byte		        Fs2
	.byte	W12
mus_qz_625_3_B1:
	.byte		N28   , Bn1 , v108 , gtp1
	.byte	W36
	.byte		N04   , Bn1 , v060
	.byte	W12
	.byte		N28   , Fs1 , v116 , gtp1
	.byte	W36
	.byte		N04   , Fs1 , v060
	.byte	W12
	.byte	PEND
mus_qz_625_3_000:
	.byte		N28   , Bn1 , v100 , gtp1
	.byte	W36
	.byte		N04   , Bn1 , v088
	.byte	W12
	.byte		N23   , Fs1 , v116
	.byte	W48
	.byte	PEND
mus_qz_625_3_001:
	.byte		N23   , Bn1 , v096
	.byte	W36
	.byte	N04
	.byte	W12
	.byte		N23   , Fs1 , v108
	.byte	W36
	.byte		N04   , Fs1 , v060
	.byte	W12
	.byte	PEND
mus_qz_625_3_002:
	.byte		N23   , Bn1 , v116
	.byte	W36
	.byte		N04   , Bn1 , v076
	.byte	W12
	.byte		N28   , Fs1 , v092 , gtp1
	.byte	W48
	.byte	PEND
	.byte		N23   , En1 , v116
	.byte	W36
	.byte		N04   , Gs1 , v076
	.byte	W12
	.byte		N17   , En2 , v092
	.byte	W24
	.byte		N11   , Bn1
	.byte	W12
	.byte		N04   , Ds2 , v076
	.byte	W12
	.byte		N23   , Fn2 , v116
	.byte	W36
	.byte		N04   , Cs2 , v076
	.byte	W12
	.byte		N14   , Fn1 , v092
	.byte	W24
	.byte		N23   , Gs2
	.byte	W24
	.byte		        Fs1 , v116
	.byte	W36
	.byte		N04   , Fs1 , v076
	.byte	W12
	.byte		N23   , Cs2 , v116
	.byte	W36
	.byte		N04   , Cs2 , v076
	.byte	W12
	.byte		N08   , Fs1 , v116
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_625_3_B1
	.byte	PATT
	 ptrvalue	mus_qz_625_3_000
	.byte	PATT
	 ptrvalue	mus_qz_625_3_001
	.byte	PATT
	 ptrvalue	mus_qz_625_3_002
	.byte		N23   , En1 , v116
	.byte	W36
	.byte		N04   , Gs1 , v076
	.byte	W12
	.byte		N28   , En2 , v092 , gtp1
	.byte	W36
	.byte		N04   , Gs1 , v076
	.byte	W12
	.byte		N23   , Cs2 , v116
	.byte	W36
	.byte		N04   , Cs2 , v076
	.byte	W12
	.byte		N23   , Fn2 , v088
	.byte	W24
	.byte		        Gs2 , v112
	.byte	W24
	.byte		        Fs2 , v116
	.byte	W36
	.byte		N04   , Fs1 , v076
	.byte	W12
	.byte		N23   , Cs2 , v116
	.byte	W36
	.byte		N04   , As1 , v076
	.byte	W12
	.byte		N11   , Fs1 , v116
	.byte	W48
	.byte		N23   , Cs2 , v092
	.byte	W24
	.byte		        Fs1
	.byte	W24
	.byte		N28   , Ds2 , v112 , gtp1
	.byte	W36
	.byte		N04   , Ds2 , v060
	.byte	W12
	.byte		N28   , Gn2 , v116 , gtp1
	.byte	W36
	.byte		N04   , Gn2 , v060
	.byte	W12
	.byte		N28   , As2 , v100 , gtp1
	.byte	W36
	.byte		N04   , Gn2 , v088
	.byte	W12
	.byte		N17   , Ds2 , v116
	.byte	W24
	.byte		        Gn2
	.byte	W24
mus_qz_625_3_003:
	.byte		N28   , Gs2 , v096 , gtp1
	.byte	W36
	.byte	N04
	.byte	W12
	.byte		N23   , Gn2 , v108
	.byte	W36
	.byte		N04   , Gn2 , v060
	.byte	W12
	.byte	PEND
	.byte		N28   , Fs2 , v116 , gtp1
	.byte	W36
	.byte		N04   , Fs2 , v076
	.byte	W12
	.byte		N28   , Fn2 , v108 , gtp1
	.byte	W36
	.byte		N04   , Fs2 , v076
	.byte	W12
	.byte		N23   , Gn2 , v116
	.byte	W36
	.byte		N04   , Gn2 , v076
	.byte	W12
	.byte		N22   , Ds2 , v092
	.byte	W24
	.byte		        As1
	.byte	W24
	.byte		N23   , Ds2 , v116
	.byte	W36
	.byte		N04   , Ds2 , v076
	.byte	W12
	.byte		N23   , Gn2 , v108
	.byte	W24
	.byte		        As2
	.byte	W24
	.byte	PATT
	 ptrvalue	mus_qz_625_3_003
	.byte		N28   , Fs2 , v116 , gtp1
	.byte	W36
	.byte		N04   , Fs2 , v068
	.byte	W12
	.byte		N28   , Fn2 , v092 , gtp1
	.byte	W36
	.byte		N04   , Fn2 , v076
	.byte	W12
	.byte		N28   , En2 , v096 , gtp1
	.byte	W36
	.byte	N04
	.byte	W12
	.byte		N23   , Bn1 , v108
	.byte	W36
	.byte		N04   , Bn1 , v060
	.byte	W12
	.byte		N28   , Fn2 , v116 , gtp1
	.byte	W36
	.byte		N04   , Fn2 , v076
	.byte	W12
	.byte		N28   , Cs2 , v092 , gtp1
	.byte	W36
	.byte		N04   , Cs2 , v076
	.byte	W12
	.byte		N28   , Fs2 , v116 , gtp1
	.byte	W36
	.byte		N04   , Fs2 , v076
	.byte	W12
	.byte		N28   , Cs2 , v092 , gtp1
	.byte	W36
	.byte		N04   , As1 , v076
	.byte	W12
	.byte		N05   , Fs1 , v116
	.byte	W12
	.byte	N05
	.byte	W24
	.byte		N40   , Fs1 , v116 , gtp1
	.byte	W48
	.byte		N04   , As1 , v076
	.byte	W12
	.byte	GOTO
	 ptrvalue	mus_qz_625_3_B1
	.byte	FINE

/********************** Track  4 **********************/

mus_qz_625_4:
	.byte		KEYSH , mus_qz_625_key+0
	.byte		VOICE , 23
	.byte		VOL   , 39*mus_qz_625_mvl/mxv
	.byte		PAN   , c_v-54
	.byte		PRIO  , 56
	.byte		VOL   , 30*mus_qz_625_mvl/mxv
	.byte	W96
	.byte	W96
mus_qz_625_4_B1:
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W72
	.byte		N02   , Ds2 , v052
	.byte		N02   , Bn2 , v056
	.byte	W03
	.byte		        En2
	.byte		N02   , Cs3 , v064
	.byte	W03
	.byte		        Fs2 , v072
	.byte		N02   , Ds3 , v076
	.byte	W04
	.byte		        Gs2 , v088
	.byte		N02   , En3 , v092
	.byte	W03
	.byte		        Bn2 , v096
	.byte		N02   , Fs3 , v100
	.byte	W04
	.byte		        Cn3 , v108
	.byte		N02   , Gs3 , v112
	.byte	W03
	.byte		        Dn3 , v116
	.byte		N02   , As3 , v120
	.byte	W04
	.byte		N92   , Ds3 , v080 , gtp2
	.byte		N92   , Bn3 , v092 , gtp2
	.byte	W72
	.byte		VOL   , 29*mus_qz_625_mvl/mxv
	.byte	W06
	.byte		        27*mus_qz_625_mvl/mxv
	.byte	W06
	.byte		        26*mus_qz_625_mvl/mxv
	.byte	W06
	.byte		        23*mus_qz_625_mvl/mxv
	.byte	W06
	.byte		        30*mus_qz_625_mvl/mxv
	.byte		N32   , Ds3 , v072 , gtp2
	.byte		N32   , As3 , v084 , gtp2
	.byte	W36
	.byte		N22   , Ds3 , v064
	.byte		N22   , Bn3 , v076
	.byte	W24
	.byte		N32   , Bn2 , v068 , gtp2
	.byte		N32   , Fs3 , v080 , gtp2
	.byte	W24
	.byte		VOL   , 28*mus_qz_625_mvl/mxv
	.byte	W06
	.byte		        27*mus_qz_625_mvl/mxv
	.byte	W06
	.byte		        30*mus_qz_625_mvl/mxv
	.byte		N92   , Ds3 , v072 , gtp2
	.byte		N92   , An3 , v080 , gtp2
	.byte	W72
	.byte		VOL   , 29*mus_qz_625_mvl/mxv
	.byte	W06
	.byte		        27*mus_qz_625_mvl/mxv
	.byte	W06
	.byte		        26*mus_qz_625_mvl/mxv
	.byte	W06
	.byte		        23*mus_qz_625_mvl/mxv
	.byte	W06
	.byte		        30*mus_qz_625_mvl/mxv
	.byte		N32   , Bn2 , v080 , gtp2
	.byte		N32   , Gs3 , v092 , gtp2
	.byte	W36
	.byte		N22   , Ds3 , v072
	.byte		N22   , An3 , v084
	.byte	W24
	.byte		N32   , Ds3 , v080 , gtp2
	.byte		N32   , Bn3 , v096 , gtp2
	.byte	W24
	.byte		VOL   , 28*mus_qz_625_mvl/mxv
	.byte	W06
	.byte		        27*mus_qz_625_mvl/mxv
	.byte	W06
	.byte		        30*mus_qz_625_mvl/mxv
	.byte		N32   , Bn2 , v076 , gtp2
	.byte		N32   , Gs3 , v092 , gtp2
	.byte	W36
	.byte		        Cs3 , v072 , gtp2
	.byte		N32   , As3 , v088 , gtp2
	.byte	W36
	.byte		N22   , Ds3 , v080
	.byte		N22   , Bn3 , v096
	.byte	W24
	.byte	W12
	.byte		N11   , Cs3 , v064
	.byte		N11   , Fn3
	.byte	W12
	.byte		        Bn2 , v056
	.byte		N11   , Ds3
	.byte	W12
	.byte		        Cs3 , v064
	.byte		N11   , Fn3
	.byte	W12
	.byte		        Cs3 , v080
	.byte		N11   , Fn3
	.byte	W12
	.byte		        Gs2 , v068
	.byte		N11   , Cs3
	.byte	W12
	.byte		        Cs3 , v080
	.byte		N11   , Fn3
	.byte	W12
	.byte		        Cs3 , v076
	.byte		N11   , Gs3
	.byte	W12
	.byte		N32   , Cs3 , v080 , gtp3
	.byte		N32   , As3 , v084 , gtp3
	.byte	W36
	.byte		N05   , En3 , v072
	.byte		N05   , Cs4 , v080
	.byte	W06
	.byte		        Ds3 , v072
	.byte		N05   , Bn3 , v080
	.byte	W06
	.byte		        Cs3 , v072
	.byte		N05   , As3 , v080
	.byte	W12
	.byte		N10   , Bn2
	.byte		N10   , Gs3 , v084
	.byte	W12
	.byte		        Cs3 , v080
	.byte		N10   , As3 , v084
	.byte	W12
	.byte		        Cs3 , v080
	.byte		N10   , Bn3 , v084
	.byte	W12
	.byte		N09   , Cs3 , v076
	.byte		N09   , As3 , v084
	.byte	W24
	.byte		N05   , As3 , v104
	.byte	W06
	.byte		        Cs3 , v088
	.byte	W06
	.byte		        Fs3 , v112
	.byte	W06
	.byte		        As3 , v100
	.byte	W06
	.byte		        Bn3 , v116
	.byte	W06
	.byte		        Cs3 , v104
	.byte	W06
	.byte		        Fs3 , v120
	.byte	W06
	.byte		        Bn3 , v108
	.byte	W06
	.byte		        Cs4 , v124
	.byte	W06
	.byte		        Fs3 , v112
	.byte	W06
	.byte		        Bn3 , v124
	.byte	W06
	.byte		        Cs4 , v112
	.byte	W06
	.byte		N68   , Gn3 , v092 , gtp3
	.byte		N68   , Ds4 , v092 , gtp3
	.byte	W72
	.byte		N23   , Ds3 , v084
	.byte		N23   , As3
	.byte	W24
	.byte		N32   , Gn3 , v088 , gtp3
	.byte		N32   , Ds4 , v088 , gtp3
	.byte	W36
	.byte		        Gs3 , v080 , gtp3
	.byte		N32   , Fn4 , v080 , gtp3
	.byte	W36
	.byte		N23   , As3 , v084
	.byte		N23   , Gn4
	.byte	W24
	.byte		N44   , Bn3 , v088 , gtp3
	.byte		N44   , Gs4 , v088 , gtp3
	.byte	W36
	.byte		VOL   , 28*mus_qz_625_mvl/mxv
	.byte	W06
	.byte		        27*mus_qz_625_mvl/mxv
	.byte	W06
	.byte		        30*mus_qz_625_mvl/mxv
	.byte		N44   , Bn3 , v080 , gtp3
	.byte		N44   , Gn4 , v080 , gtp3
	.byte	W36
	.byte		VOL   , 28*mus_qz_625_mvl/mxv
	.byte	W06
	.byte		        27*mus_qz_625_mvl/mxv
	.byte	W06
	.byte		        30*mus_qz_625_mvl/mxv
	.byte		N32   , Bn3 , v084 , gtp3
	.byte		N32   , Fs4 , v084 , gtp3
	.byte	W24
	.byte		VOL   , 28*mus_qz_625_mvl/mxv
	.byte	W06
	.byte		        27*mus_qz_625_mvl/mxv
	.byte	W06
	.byte		        30*mus_qz_625_mvl/mxv
	.byte		N56   , Bn3 , v080 , gtp3
	.byte		N56   , Fn4 , v080 , gtp3
	.byte	W36
	.byte		VOL   , 29*mus_qz_625_mvl/mxv
	.byte	W06
	.byte		        27*mus_qz_625_mvl/mxv
	.byte	W06
	.byte		        26*mus_qz_625_mvl/mxv
	.byte	W06
	.byte		        23*mus_qz_625_mvl/mxv
	.byte	W06
	.byte		        30*mus_qz_625_mvl/mxv
	.byte		N56   , Gn3 , v088 , gtp3
	.byte		N56   , Ds4 , v088 , gtp3
	.byte	W48
	.byte		VOL   , 28*mus_qz_625_mvl/mxv
	.byte	W06
	.byte		        27*mus_qz_625_mvl/mxv
	.byte	W06
	.byte		        30*mus_qz_625_mvl/mxv
	.byte		N11   , Ds3 , v080
	.byte		N11   , Gn3
	.byte	W12
	.byte		        Gn3 , v072
	.byte		N11   , As3
	.byte	W12
	.byte		        As3 , v076
	.byte		N11   , Ds4
	.byte	W12
	.byte		        Ds4 , v084
	.byte		N11   , Gn4
	.byte	W12
	.byte		        As3 , v072
	.byte		N11   , Ds4
	.byte	W12
	.byte		        Gn3 , v080
	.byte		N11   , As3
	.byte	W12
	.byte		        Ds4 , v084
	.byte		N11   , Gn4
	.byte	W12
	.byte		        Ds4 , v076
	.byte		N11   , Gs4
	.byte	W12
	.byte		        As3 , v080
	.byte		N11   , Ds4
	.byte	W12
	.byte		        Gn4 , v076
	.byte		N11   , As4
	.byte	W12
	.byte		        As3 , v092
	.byte		N11   , Ds4
	.byte	W12
	.byte		N32   , Ds4 , v088 , gtp3
	.byte		N32   , As4 , v092 , gtp3
	.byte	W36
	.byte		        Ds4 , v088 , gtp3
	.byte		N32   , Bn4 , v092 , gtp3
	.byte	W36
	.byte		N23   , Ds4 , v088
	.byte		N23   , Gs4 , v092
	.byte	W24
	.byte		N32   , Ds4 , v076 , gtp3
	.byte		N32   , Fs4 , v080 , gtp3
	.byte	W36
	.byte		N22   , Gs3 , v076
	.byte		N22   , Fn4 , v092
	.byte	W24
	.byte		N32   , Cs4 , v076 , gtp3
	.byte		N32   , Gs4 , v080 , gtp3
	.byte	W36
	.byte		N11   , Gs3 , v100
	.byte		N11   , En4 , v108
	.byte	W12
	.byte		N05   , Bn2 , v076
	.byte		N05   , Gs3 , v084
	.byte	W12
	.byte		N11   , En3 , v080
	.byte		N11   , Bn3 , v088
	.byte	W12
	.byte		N08   , Gs3 , v092
	.byte		N08   , En4 , v100
	.byte	W12
	.byte		N11   , En3 , v092
	.byte		N11   , Bn3 , v100
	.byte	W12
	.byte		        Gs3 , v080
	.byte		N11   , En4 , v088
	.byte	W12
	.byte		N08   , Bn3 , v096
	.byte		N08   , Gs4 , v108
	.byte	W12
	.byte		        Ds4 , v084
	.byte		N08   , Bn4 , v092
	.byte	W12
	.byte		N11   , Gs3
	.byte		N11   , Fn4 , v100
	.byte	W12
	.byte		N05   , Cs3 , v076
	.byte		N05   , Gs3 , v084
	.byte	W12
	.byte		N11   , Dn3 , v080
	.byte		N11   , Bn3 , v088
	.byte	W12
	.byte		N09   , Fn3 , v092
	.byte		N09   , Dn4 , v100
	.byte	W12
	.byte		N11   , Fn3 , v084
	.byte		N11   , Cs4 , v092
	.byte	W12
	.byte		        Gs3
	.byte		N11   , Fn4 , v100
	.byte	W12
	.byte		        Cs4 , v092
	.byte		N11   , Gs4 , v104
	.byte	W12
	.byte		        Cs4 , v100
	.byte		N11   , Bn4 , v112
	.byte	W12
	.byte		        Cs4 , v100
	.byte		N11   , As4 , v112
	.byte	W12
	.byte		N05   , Fs3 , v080
	.byte		N05   , Cs4
	.byte	W12
	.byte		N11   , As3 , v088
	.byte		N11   , Fs4 , v096
	.byte	W12
	.byte		        Cs4 , v092
	.byte		N11   , As4 , v100
	.byte	W12
	.byte		        Fs3 , v092
	.byte		N11   , Cs4 , v100
	.byte	W12
	.byte		        As2 , v068
	.byte		N11   , Fs3 , v076
	.byte	W12
	.byte		        Fs3 , v080
	.byte		N11   , Cs4 , v088
	.byte	W12
	.byte		        As3 , v100
	.byte		N11   , Fs4 , v112
	.byte	W12
	.byte		        Cs4 , v092
	.byte		N11   , As4 , v100
	.byte	W12
	.byte		N05   , Cs4 , v084
	.byte		N05   , Gs4 , v092
	.byte	W24
	.byte		N40   , As3 , v092 , gtp1
	.byte		N40   , Fs4 , v108 , gtp1
	.byte	W30
	.byte		VOL   , 28*mus_qz_625_mvl/mxv
	.byte	W06
	.byte		        27*mus_qz_625_mvl/mxv
	.byte	W06
	.byte		        25*mus_qz_625_mvl/mxv
	.byte	W12
	.byte		        30*mus_qz_625_mvl/mxv
	.byte	W06
	.byte	GOTO
	 ptrvalue	mus_qz_625_4_B1
	.byte	FINE

/********************** Track  5 **********************/

mus_qz_625_5:
	.byte		KEYSH , mus_qz_625_key+0
	.byte		VOICE , 14
	.byte		VOL   , 52*mus_qz_625_mvl/mxv
	.byte		PAN   , c_v+46
	.byte		BENDR , 6
	.byte		PRIO  , 54
	.byte		N08   , Fs3 , v072
	.byte	W24
	.byte	N08
	.byte	W24
	.byte	N08
	.byte	W12
	.byte	N08
	.byte	W24
	.byte	N08
	.byte	W12
	.byte	W24
	.byte		N28   , Fs3 , v072 , gtp1
	.byte	W72
mus_qz_625_5_B1:
	.byte		N11   , Bn1 , v092
	.byte	W12
	.byte		        Fs2 , v080
	.byte	W12
	.byte		        As2 , v084
	.byte	W12
	.byte		        Cs3 , v088
	.byte	W12
	.byte		        Fs1 , v092
	.byte	W12
	.byte		N23   , Bn2 , v088
	.byte	W24
	.byte		N11   , As2
	.byte	W06
	.byte		BEND  , c_v+10
	.byte	W05
	.byte		        c_v-1
	.byte	W01
	.byte		N11   , Bn1 , v092
	.byte	W12
	.byte		        Fs2 , v080
	.byte	W12
	.byte		        Bn2 , v084
	.byte	W12
	.byte		N23   , Ds3 , v088
	.byte	W24
	.byte		N11   , Fs2 , v080
	.byte	W12
	.byte		        Bn2 , v084
	.byte	W12
	.byte		        Ds3 , v088
	.byte	W12
	.byte		        Bn1 , v092
	.byte	W12
	.byte		        An2 , v080
	.byte	W12
	.byte		        Bn2 , v084
	.byte	W12
	.byte		        Ds3 , v088
	.byte	W12
	.byte		        Fs1 , v092
	.byte	W12
	.byte		N23   , Bn2 , v088
	.byte	W24
	.byte		N11   , As2
	.byte	W06
	.byte		BEND  , c_v+10
	.byte	W05
	.byte		        c_v+0
	.byte	W01
	.byte		N11   , Bn1 , v092
	.byte	W12
	.byte		        Bn2 , v080
	.byte	W12
	.byte		        Ds3 , v084
	.byte	W12
	.byte		        Fs3 , v088
	.byte	W12
	.byte		        Fs1 , v092
	.byte	W12
	.byte		        En3
	.byte	W04
	.byte		BEND  , c_v+21
	.byte	W04
	.byte		        c_v+0
	.byte	W04
	.byte		N11   , Ds3 , v088
	.byte	W12
	.byte		        Bn2
	.byte	W12
	.byte		        En2
	.byte	W12
	.byte		        Bn2
	.byte	W12
	.byte		        Ds3 , v084
	.byte	W12
	.byte		        Gs3 , v088
	.byte	W12
	.byte		        Bn1 , v092
	.byte	W12
	.byte		        Gs2 , v088
	.byte	W12
	.byte		        Cs3
	.byte	W12
	.byte		        En3
	.byte	W12
	.byte		        Fn2 , v080
	.byte	W12
	.byte		        Gs2 , v088
	.byte	W12
	.byte		        Bn2 , v084
	.byte	W12
	.byte		        Dn3 , v088
	.byte	W12
	.byte		        Fn1 , v092
	.byte	W12
	.byte		        Gs2 , v088
	.byte	W12
	.byte		        Bn2 , v084
	.byte	W12
	.byte		        Fn3 , v088
	.byte	W12
	.byte		        Fs1 , v092
	.byte	W12
	.byte		        Fs2 , v080
	.byte	W12
	.byte		        As2 , v084
	.byte	W12
	.byte		N23   , Ds3 , v088
	.byte	W24
	.byte		N11   , Fs2 , v080
	.byte	W12
	.byte		        As2 , v088
	.byte	W12
	.byte		        Bn2 , v084
	.byte	W12
mus_qz_625_5_000:
	.byte		N08   , Cs3 , v088
	.byte		N08   , Gn3 , v068
	.byte	W96
	.byte	PEND
	.byte		N11   , Bn1 , v092
	.byte	W12
	.byte		        Fs2 , v088
	.byte	W12
	.byte		        As2 , v080
	.byte	W12
	.byte		        Cs3 , v088
	.byte	W12
	.byte		        Fs1 , v092
	.byte	W12
	.byte		N23   , Bn2 , v088
	.byte	W24
	.byte		N11   , As2
	.byte	W06
	.byte		BEND  , c_v+10
	.byte	W05
	.byte		        c_v-1
	.byte	W01
	.byte		N11   , Bn1 , v092
	.byte	W12
	.byte		        Fs2 , v088
	.byte	W12
	.byte		        Bn2 , v080
	.byte	W12
	.byte		N23   , Ds3 , v088
	.byte	W24
	.byte		N11   , Fs2
	.byte	W12
	.byte		        Bn2 , v080
	.byte	W12
	.byte		        Ds3 , v088
	.byte	W12
	.byte		        Bn1 , v092
	.byte	W12
	.byte		        An2 , v088
	.byte	W12
	.byte		        Bn2
	.byte	W12
	.byte		        Ds3
	.byte	W12
	.byte		        Fs1 , v092
	.byte	W12
	.byte		N23   , Bn2 , v088
	.byte	W24
	.byte		N11   , As2
	.byte	W06
	.byte		BEND  , c_v+10
	.byte	W05
	.byte		        c_v+0
	.byte	W01
	.byte		N11   , Bn1 , v092
	.byte	W12
	.byte		        Bn2 , v088
	.byte	W12
	.byte		        Ds3
	.byte	W12
	.byte		        Fs3
	.byte	W12
	.byte		        Fs1 , v092
	.byte	W12
	.byte		        En3 , v088
	.byte	W04
	.byte		BEND  , c_v+21
	.byte	W04
	.byte		        c_v+0
	.byte	W04
	.byte		N11   , Ds3
	.byte	W12
	.byte		        Bn2
	.byte	W12
	.byte		        En2 , v092
	.byte	W12
	.byte		        Bn2 , v088
	.byte	W12
	.byte		        Ds3 , v080
	.byte	W12
	.byte		        Gs3 , v088
	.byte	W12
	.byte		        Bn1 , v092
	.byte	W12
	.byte		        Bn2 , v088
	.byte	W12
	.byte		        Ds3 , v080
	.byte	W12
	.byte		        Gs3 , v088
	.byte	W12
	.byte		        Cs2 , v092
	.byte	W12
	.byte		        Fn2 , v088
	.byte	W12
	.byte		        Bn2
	.byte	W12
	.byte		        Fn3
	.byte	W12
	.byte		        Fn2
	.byte	W12
	.byte		        Bn2
	.byte	W12
	.byte		        Gs2
	.byte	W12
	.byte		        Cs3
	.byte	W12
	.byte		        Fs1 , v092
	.byte	W12
	.byte		        Fs2 , v088
	.byte	W12
	.byte		        As2 , v080
	.byte	W12
	.byte		N23   , Ds3 , v088
	.byte	W24
	.byte		N11   , Fs2
	.byte	W12
	.byte		        As2 , v080
	.byte	W12
	.byte		        Cs3 , v088
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_625_5_000
mus_qz_625_5_001:
	.byte		N11   , Ds2 , v088
	.byte	W12
	.byte		        Gn2
	.byte	W12
	.byte		        As2
	.byte	W12
	.byte		N23   , Ds3
	.byte	W24
	.byte		N11   , Gn2
	.byte	W12
	.byte		        As2
	.byte	W12
	.byte		        Ds3
	.byte	W12
	.byte	PEND
mus_qz_625_5_002:
	.byte		N11   , Cs3 , v088
	.byte	W12
	.byte		        Gn2
	.byte	W12
	.byte		        As2
	.byte	W12
	.byte		N23   , Ds3
	.byte	W24
	.byte		N11   , Gn2
	.byte	W12
	.byte		        Gs2
	.byte	W12
	.byte		        As2
	.byte	W12
	.byte	PEND
mus_qz_625_5_003:
	.byte		N11   , Gs1 , v088
	.byte	W12
	.byte		        Gs2
	.byte	W12
	.byte		        Bn2
	.byte	W12
	.byte		N23   , Ds3
	.byte	W24
	.byte		N11   , Gn2
	.byte	W12
	.byte		        Bn2
	.byte	W12
	.byte		        Ds3
	.byte	W12
	.byte	PEND
	.byte		        Fs1
	.byte	W12
	.byte		        Fs2
	.byte	W12
	.byte		        As2
	.byte	W12
	.byte		        Ds3
	.byte	W12
	.byte		        Fn1
	.byte	W12
	.byte		        En3
	.byte	W04
	.byte		BEND  , c_v+21
	.byte	W04
	.byte		        c_v+0
	.byte	W04
	.byte		N11   , Ds3
	.byte	W12
	.byte		        Bn2
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_625_5_001
	.byte	PATT
	 ptrvalue	mus_qz_625_5_002
	.byte	PATT
	 ptrvalue	mus_qz_625_5_003
	.byte		N11   , Fs1 , v088
	.byte	W12
	.byte		        Fs2
	.byte	W12
	.byte		        As2
	.byte	W12
	.byte		        Cs3
	.byte	W12
	.byte		        Fn1
	.byte	W12
	.byte		        En3
	.byte	W04
	.byte		BEND  , c_v+21
	.byte	W04
	.byte		        c_v+0
	.byte	W04
	.byte		N11   , Bn2
	.byte	W12
	.byte		        Ds3
	.byte	W12
	.byte		        En1
	.byte	W12
	.byte		        Gs2
	.byte	W12
	.byte		        Bn2
	.byte	W12
	.byte		N23   , En3
	.byte	W24
	.byte		N11   , En2
	.byte	W12
	.byte		        Gs2
	.byte	W12
	.byte		        Ds3 , v084
	.byte	W12
	.byte		        Dn3 , v088
	.byte	W12
	.byte		        Gs2
	.byte	W12
	.byte		        Bn2
	.byte	W12
	.byte		N23   , Dn3
	.byte	W24
	.byte		N11   , Fn2
	.byte	W12
	.byte		        Gs2
	.byte	W12
	.byte		        Bn2 , v084
	.byte	W12
	.byte		        As2 , v088
	.byte	W12
	.byte		        Cs2
	.byte	W12
	.byte		        Fs2 , v080
	.byte	W12
	.byte		N23   , As2 , v088
	.byte	W24
	.byte		N11   , Fs2
	.byte	W12
	.byte		        Gs2
	.byte	W12
	.byte		        As2 , v080
	.byte	W12
	.byte		N10   , Fs2 , v100
	.byte		N10   , En3 , v088
	.byte	W12
	.byte		N04   , Fs2 , v092
	.byte		N04   , En3
	.byte	W24
	.byte		N32   , Fs2 , v092 , gtp3
	.byte		N32   , Cs3 , v092 , gtp3
	.byte	W60
	.byte	GOTO
	 ptrvalue	mus_qz_625_5_B1
	.byte	FINE

/********************** Track  6 **********************/

mus_qz_625_6:
	.byte		KEYSH , mus_qz_625_key+0
	.byte		VOICE , 5
	.byte		VOL   , 66*mus_qz_625_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte	W96
	.byte	W96
mus_qz_625_6_B1:
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
	.byte	W72
	.byte		N11   , Bn3 , v108
	.byte	W12
	.byte		N05   , Cs4 , v100
	.byte	W12
	.byte		N11   , Ds4 , v112
	.byte	W24
	.byte		        Ds4 , v108
	.byte	W24
	.byte		        Ds4 , v112
	.byte	W12
	.byte		        Dn4 , v104
	.byte	W12
	.byte		N05   , Ds4 , v112
	.byte	W09
	.byte		N02   , Gs4 , v064
	.byte	W03
	.byte		N32   , As4 , v112 , gtp3
	.byte	W12
	.byte	W24
	.byte		N05   , Gn4 , v108
	.byte	W24
	.byte		        Ds4 , v100
	.byte	W24
	.byte		        Cs4 , v112
	.byte	W24
	.byte		N17   , Cs4 , v108
	.byte	W18
	.byte		N02   , Cs4 , v080
	.byte	W03
	.byte		        Dn4 , v092
	.byte	W03
	.byte		N05   , Ds4 , v108
	.byte	W12
	.byte		N23   , Bn3
	.byte	W24
	.byte		N11   , As3 , v096
	.byte	W12
	.byte		        Bn3 , v100
	.byte	W12
	.byte		N28   , Cs4 , v112 , gtp1
	.byte	W12
	.byte	W24
	.byte		N11   , Ds4 , v100
	.byte	W12
	.byte		N23   , Gs4 , v112
	.byte	W36
	.byte		N11   , Bn3 , v108
	.byte	W12
	.byte		N05   , Cs4 , v100
	.byte	W12
	.byte		N11   , Ds4 , v112
	.byte	W24
	.byte		        Ds4 , v104
	.byte	W24
	.byte		        Ds4 , v112
	.byte	W12
	.byte		        Dn4 , v100
	.byte	W12
	.byte		N05   , Ds4 , v108
	.byte	W09
	.byte		N02   , Gs4 , v092
	.byte	W03
	.byte		N32   , As4 , v108 , gtp3
	.byte	W12
	.byte	W24
	.byte		N05   , Gn4
	.byte	W06
	.byte		        Fs4 , v092
	.byte	W06
	.byte		        Gn4 , v108
	.byte	W12
	.byte		        Gs4
	.byte	W06
	.byte		        Gn4 , v092
	.byte	W06
	.byte		        Gs4 , v108
	.byte	W12
	.byte		        As4
	.byte	W06
	.byte		        An4 , v092
	.byte	W06
	.byte		        As4 , v108
	.byte	W12
	.byte	N17
	.byte	W24
	.byte		N05   , Bn4 , v100
	.byte	W12
	.byte		N23   , Gs4 , v112
	.byte	W24
	.byte		N11   , Gn4 , v108
	.byte	W12
	.byte		        Gs4 , v100
	.byte	W12
	.byte		N28   , As4 , v112 , gtp1
	.byte	W12
	.byte	W24
	.byte		N05   , Bn4 , v100
	.byte	W12
	.byte		N17   , Gs4 , v108
	.byte	W21
	.byte		N02   , Cn5 , v092
	.byte	W03
	.byte		N11   , Cs5 , v108
	.byte	W12
	.byte		N05   , Bn4 , v100
	.byte	W12
	.byte		        As4 , v112
	.byte	W12
	.byte		N76   , Bn4 , v112 , gtp1
	.byte	W90
	.byte		N02   , Gs4 , v092
	.byte	W03
	.byte		        As4 , v108
	.byte	W03
	.byte		N56   , Bn4 , v100 , gtp3
	.byte	W60
	.byte		N11   , Gs4 , v108
	.byte	W12
	.byte		N03   , As4 , v100
	.byte	W12
	.byte		        Bn4 , v092
	.byte	W12
	.byte		N56   , As4 , v108 , gtp3
	.byte	W60
	.byte		N11   , Gs4
	.byte	W12
	.byte		        As4 , v100
	.byte	W12
	.byte		N02   , Gs4 , v092
	.byte	W12
	.byte		N05   , As4 , v108
	.byte	W12
	.byte		        Gs4 , v100
	.byte	W24
	.byte		N56   , Fs4 , v108 , gtp3
	.byte	W60
	.byte	GOTO
	 ptrvalue	mus_qz_625_6_B1
	.byte	FINE

/********************** Track  7 **********************/

mus_qz_625_7:
	.byte		KEYSH , mus_qz_625_key+0
	.byte		VOICE , 26
	.byte		VOL   , 44*mus_qz_625_mvl/mxv
	.byte		PAN   , c_v-24
	.byte		PRIO  , 54
	.byte		N08   , Cs4 , v088
	.byte	W24
	.byte		        Cs4 , v080
	.byte	W24
	.byte		        Cn4 , v088
	.byte	W12
	.byte		        Cn4 , v080
	.byte	W24
	.byte		        Bn3 , v088
	.byte	W12
	.byte	W24
	.byte		        Fs3
	.byte	W72
mus_qz_625_7_B1:
	.byte	W12
	.byte		N08   , Fs3 , v088
	.byte	W12
	.byte		        Cs4 , v076
	.byte	W12
	.byte		        Fs4 , v088
	.byte	W60
	.byte	PEND
	.byte	W12
	.byte		        As3
	.byte	W12
	.byte		        Fs4 , v076
	.byte	W12
	.byte		        As4 , v088
	.byte	W24
	.byte		        Fs4 , v084
	.byte	W36
	.byte	PATT
	 ptrvalue	mus_qz_625_7_B1
	.byte	W12
	.byte		N08   , An3 , v088
	.byte	W12
	.byte		        Ds4 , v076
	.byte	W12
	.byte		        An4 , v088
	.byte	W24
	.byte		        Ds4 , v080
	.byte	W36
	.byte	W12
	.byte		        Gs3 , v088
	.byte	W12
	.byte		        Ds4 , v076
	.byte	W12
	.byte		        Gs4 , v088
	.byte	W24
	.byte		        Ds3
	.byte	W12
	.byte		        Gs3 , v076
	.byte	W12
	.byte		        En4 , v088
	.byte	W12
	.byte	W12
	.byte		        Gs3
	.byte	W12
	.byte		        Cs4 , v076
	.byte	W12
	.byte		        En4 , v088
	.byte	W24
	.byte		        Cs4
	.byte	W12
	.byte		        En4 , v076
	.byte	W12
	.byte		        Gs4 , v088
	.byte	W12
	.byte	W12
	.byte		        Fs3
	.byte	W12
	.byte		        Cs4 , v076
	.byte	W12
	.byte		        Fs4 , v088
	.byte	W12
	.byte		        Cs4 , v076
	.byte	W12
	.byte		        As3 , v088
	.byte	W12
	.byte		        Cs4 , v076
	.byte	W12
	.byte		        Fs4 , v088
	.byte	W12
	.byte		        As4 , v084
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
	 ptrvalue	mus_qz_625_7_B1
	.byte	FINE

/********************** Track  8 **********************/

mus_qz_625_8:
	.byte		KEYSH , mus_qz_625_key+0
	.byte		VOICE , 39
	.byte		VOL   , 52*mus_qz_625_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 40
	.byte		N02   , Gs4 , v080
	.byte	W12
	.byte		        Gs4 , v056
	.byte	W12
	.byte		N11   , An4 , v108
	.byte	W12
	.byte		N02   , Gs4 , v068
	.byte	W12
	.byte		        Gs4 , v096
	.byte	W12
	.byte		        Gs4 , v068
	.byte	W24
	.byte	N02
	.byte	W12
	.byte	W12
	.byte		        Gs4 , v048
	.byte	W06
	.byte		        Gs4 , v028
	.byte	W06
	.byte		N40   , Dn2 , v068 , gtp1
	.byte	W48
	.byte		N02   , Gs4 , v048
	.byte	W06
	.byte		        Gs4 , v028
	.byte	W06
	.byte		N11   , An4 , v100
	.byte	W12
mus_qz_625_8_B1:
	.byte		N40   , An2 , v072 , gtp1
	.byte	W48
	.byte		N03   , Gs4 , v080
	.byte	W12
	.byte		        Gs4 , v056
	.byte	W12
	.byte		N11   , An4 , v108
	.byte	W12
	.byte		N03   , Gs4 , v056
	.byte	W12
mus_qz_625_8_000:
	.byte		N03   , Gs4 , v080
	.byte	W12
	.byte		        Gs4 , v056
	.byte	W12
	.byte		N11   , An4 , v108
	.byte	W12
	.byte		N03   , Gs4 , v068
	.byte	W12
	.byte		        Gs4 , v080
	.byte	W12
	.byte		        Gs4 , v056
	.byte	W12
	.byte		N11   , An4 , v108
	.byte	W12
	.byte		N03   , Gs4 , v056
	.byte	W12
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_625_8_000
	.byte	PATT
	 ptrvalue	mus_qz_625_8_000
	.byte		N40   , Bn2 , v072 , gtp1
	.byte	W48
	.byte		N03   , Gs4 , v080
	.byte	W12
	.byte		        Gs4 , v056
	.byte	W12
	.byte		N11   , An4 , v108
	.byte	W12
	.byte		N03   , Gs4 , v056
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_625_8_000
	.byte	PATT
	 ptrvalue	mus_qz_625_8_000
	.byte		N03   , Gs4 , v080
	.byte	W18
	.byte		        Gs4 , v040
	.byte	W06
	.byte		N11   , An4 , v108
	.byte	W12
	.byte		N03   , Gs4 , v068
	.byte	W06
	.byte	N03
	.byte	W06
	.byte		N11   , An4 , v108
	.byte	W12
	.byte		N03   , Gs4 , v056
	.byte	W12
	.byte		N11   , An1 , v092
	.byte		N03   , Gs4 , v068
	.byte	W12
	.byte		N11   , Fn1 , v076
	.byte		N03   , Gs4 , v056
	.byte	W12
	.byte		N40   , An2 , v072 , gtp1
	.byte	W24
	.byte		N05   , Cs1 , v088
	.byte	W24
	.byte		N03   , Gs4 , v076
	.byte	W12
	.byte		        Gs4 , v052
	.byte	W12
	.byte		N05   , Cs1 , v088
	.byte		N11   , An4 , v100
	.byte	W12
	.byte		N03   , Gs4 , v052
	.byte	W12
mus_qz_625_8_001:
	.byte		N03   , Gs4 , v076
	.byte	W12
	.byte		        Gs4 , v052
	.byte	W12
	.byte		N05   , Cs1 , v088
	.byte		N11   , An4 , v100
	.byte	W12
	.byte		N03   , Gs4 , v064
	.byte	W12
	.byte		        Gs4 , v076
	.byte	W12
	.byte		        Gs4 , v052
	.byte	W12
	.byte		N05   , Cs1 , v088
	.byte		N11   , An4 , v100
	.byte	W12
	.byte		N03   , Gs4 , v052
	.byte	W12
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_625_8_001
	.byte	PATT
	 ptrvalue	mus_qz_625_8_001
	.byte		N40   , Bn2 , v072 , gtp1
	.byte	W24
	.byte		N05   , Cs1 , v088
	.byte	W24
	.byte		N03   , Gs4 , v076
	.byte	W12
	.byte		        Gs4 , v052
	.byte	W12
	.byte		N05   , Cs1 , v088
	.byte		N11   , An4 , v100
	.byte	W12
	.byte		N03   , Gs4 , v052
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_625_8_001
	.byte	PATT
	 ptrvalue	mus_qz_625_8_001
	.byte		N23   , Ds2 , v072
	.byte		N03   , Gs4 , v084
	.byte	W18
	.byte		        Gs4 , v040
	.byte	W06
	.byte		N11   , An4 , v108
	.byte	W12
	.byte		N03   , Gs4 , v072
	.byte	W06
	.byte	N03
	.byte	W06
	.byte		N11   , An4 , v108
	.byte	W04
	.byte		N03   , An1 , v076
	.byte	W02
	.byte		N15   , Bn1 , v100
	.byte	W06
	.byte		N03   , Gs4 , v072
	.byte	W06
	.byte		        Gs4 , v084
	.byte	W06
	.byte		N11   , An1 , v100
	.byte		N03   , Gs4 , v084
	.byte	W12
	.byte		N11   , Fn1 , v100
	.byte		N03   , Gs4 , v072
	.byte	W12
mus_qz_625_8_002:
	.byte		N23   , Dn2 , v072
	.byte	W24
	.byte		N11   , Fs0 , v088
	.byte		N11   , Bn2 , v080
	.byte	W48
	.byte		        Fs0 , v092
	.byte		N11   , Bn2 , v080
	.byte	W24
	.byte	PEND
mus_qz_625_8_003:
	.byte	W24
	.byte		N11   , Fs0 , v088
	.byte		N11   , Bn2 , v080
	.byte	W48
	.byte		        Fs0 , v092
	.byte		N11   , Bn2 , v080
	.byte	W24
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_625_8_003
mus_qz_625_8_004:
	.byte	W24
	.byte		N11   , Fs0 , v088
	.byte		N11   , Bn2 , v080
	.byte	W30
	.byte		N10   , Fs0 , v048
	.byte		N10   , Bn2 , v044
	.byte	W18
	.byte		N11   , Fs0 , v092
	.byte		N11   , Bn2 , v080
	.byte	W24
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_625_8_002
	.byte	PATT
	 ptrvalue	mus_qz_625_8_003
	.byte	PATT
	 ptrvalue	mus_qz_625_8_003
	.byte	PATT
	 ptrvalue	mus_qz_625_8_004
	.byte	PATT
	 ptrvalue	mus_qz_625_8_003
	.byte	PATT
	 ptrvalue	mus_qz_625_8_003
	.byte	W24
	.byte		N11   , Fs0 , v088
	.byte		N11   , Bn2 , v080
	.byte	W48
	.byte		N05   , Bn1 , v100
	.byte	W06
	.byte		        An1 , v080
	.byte	W06
	.byte		N11   , Fn1 , v100
	.byte	W12
	.byte	W12
	.byte		        Bn1 , v072
	.byte	W24
	.byte		N28   , Ds2 , v072 , gtp1
	.byte	W24
	.byte		N03   , Fs0 , v016
	.byte	W04
	.byte		        Fs0 , v048
	.byte	W04
	.byte		        Fs0 , v076
	.byte	W04
	.byte		N07   , Fs0 , v092
	.byte	W12
	.byte		N11   , Fn1 , v100
	.byte	W12
	.byte	GOTO
	 ptrvalue	mus_qz_625_8_B1
	.byte	FINE

/********************** Track  9 **********************/

mus_qz_625_9:
	.byte		KEYSH , mus_qz_625_key+0
	.byte		VOICE , 35
	.byte		VOL   , 14*mus_qz_625_mvl/mxv
	.byte		PAN   , c_v+56
	.byte		PRIO  , 38
	.byte	W96
	.byte	W72
	.byte		N11   , Fs4 , v092
	.byte	W12
	.byte		N05   , Gs4 , v080
	.byte	W12
mus_qz_625_9_B1:
	.byte		N05   , As4 , v088
	.byte	W12
	.byte		N17   , Cs5 , v092
	.byte	W18
	.byte		N03   , Fs4 , v072
	.byte	W06
	.byte		N05   , Ds5 , v064
	.byte	W12
	.byte		N28   , Bn4 , v088 , gtp1
	.byte	W48
	.byte		N05   , Bn4 , v092
	.byte	W06
	.byte		        Cs5
	.byte	W06
	.byte		N11   , Ds5
	.byte	W12
	.byte		N05   , Dn5
	.byte	W12
	.byte		        Ds5
	.byte	W12
	.byte		N23   , En5
	.byte	W36
	.byte		N05   , Ds5 , v072
	.byte		N05   , Ds5 , v088
	.byte	W06
	.byte		        En5 , v092
	.byte	W06
	.byte		        Fs5
	.byte	W12
	.byte		N17   , Cs5 , v076
	.byte	W18
	.byte		N03   , Fs4 , v080
	.byte	W06
	.byte		N05   , Ds5 , v088
	.byte	W12
	.byte		N28   , Bn4 , v072 , gtp1
	.byte	W48
	.byte		N05   , Bn4 , v084
	.byte	W06
	.byte		        Cs5 , v072
	.byte	W06
	.byte		N11   , Ds5 , v088
	.byte	W12
	.byte		N05   , Dn5
	.byte	W12
	.byte		        Ds5 , v092
	.byte	W12
	.byte		N23   , Gs5
	.byte	W36
	.byte		N05   , En5 , v072
	.byte	W06
	.byte		        Ds5 , v092
	.byte	W06
	.byte		        Cs5
	.byte	W12
	.byte		N17   , Ds5
	.byte	W18
	.byte		N05   , Gs4 , v088
	.byte	W06
	.byte		        En5
	.byte	W12
	.byte		N32   , Cs5 , v088 , gtp3
	.byte	W48
	.byte	W24
	.byte		N28   , Gs4 , v092 , gtp1
	.byte	W36
	.byte		N05   , As4 , v100
	.byte	W24
	.byte		        Bn4
	.byte	W12
	.byte	W12
	.byte		N32   , Ds5 , v096 , gtp3
	.byte	W36
	.byte		N05   , En5 , v092
	.byte	W06
	.byte		        Ds5 , v080
	.byte	W06
	.byte		        Cs5 , v092
	.byte	W12
	.byte		N11   , Bn4
	.byte	W12
	.byte		        As4
	.byte	W12
	.byte		        Bn4
	.byte	W12
	.byte		        Cs5
	.byte	W84
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
	 ptrvalue	mus_qz_625_9_B1
	.byte	FINE

/********************** Track 10 **********************/

mus_qz_625_10:
	.byte		KEYSH , mus_qz_625_key+0
	.byte		VOICE , 39
	.byte		VOL   , 44*mus_qz_625_mvl/mxv
	.byte		PAN   , c_v-44
	.byte		PRIO  , 38
	.byte	W96
	.byte	W96
mus_qz_625_10_B1:
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
	.byte	W92
	.byte	W03
	.byte		N03   , Gn4 , v060
	.byte	W01
mus_qz_625_10_000:
	.byte	W11
	.byte		N03   , Gn4 , v036
	.byte	W24
	.byte	N03
	.byte	W12
	.byte		        Gn4 , v060
	.byte	W12
	.byte		        Gn4 , v036
	.byte	W24
	.byte	N03
	.byte	W12
	.byte		        Gn4 , v060
	.byte	W01
	.byte	PEND
mus_qz_625_10_001:
	.byte	W11
	.byte		N03   , Gn4 , v036
	.byte	W24
	.byte		        Gn4 , v048
	.byte	W12
	.byte		        Gn4 , v060
	.byte	W12
	.byte		        Gn4 , v036
	.byte	W24
	.byte	N03
	.byte	W12
	.byte		        Gn4 , v060
	.byte	W01
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_625_10_001
mus_qz_625_10_002:
	.byte	W11
	.byte		N03   , Gn4 , v036
	.byte	W24
	.byte		        Gn4 , v048
	.byte	W12
	.byte		        Gn4 , v060
	.byte	W12
	.byte		        Gn4 , v036
	.byte	W24
	.byte		        Gn4 , v048
	.byte	W12
	.byte		        Gn4 , v060
	.byte	W01
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_625_10_000
	.byte	PATT
	 ptrvalue	mus_qz_625_10_001
	.byte	PATT
	 ptrvalue	mus_qz_625_10_001
	.byte	PATT
	 ptrvalue	mus_qz_625_10_002
	.byte	PATT
	 ptrvalue	mus_qz_625_10_001
	.byte	PATT
	 ptrvalue	mus_qz_625_10_001
	.byte	PATT
	 ptrvalue	mus_qz_625_10_001
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_625_10_B1
	.byte	FINE

/********************** Track 11 **********************/

mus_qz_625_11:
	.byte		KEYSH , mus_qz_625_key+0
	.byte		VOICE , 39
	.byte		VOL   , 61*mus_qz_625_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 34
	.byte	W96
	.byte	W96
mus_qz_625_11_B1:
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
mus_qz_625_11_000:
	.byte		N09   , Cn1 , v100
	.byte	W36
	.byte		        Cn1 , v088
	.byte	W12
	.byte		        Cn1 , v100
	.byte	W48
	.byte	PEND
mus_qz_625_11_001:
	.byte		N09   , Cn1 , v100
	.byte	W36
	.byte		        Cn1 , v088
	.byte	W12
	.byte		        Cn1 , v100
	.byte	W36
	.byte		        Cn1 , v076
	.byte	W12
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_625_11_000
	.byte	PATT
	 ptrvalue	mus_qz_625_11_001
	.byte	PATT
	 ptrvalue	mus_qz_625_11_000
	.byte	PATT
	 ptrvalue	mus_qz_625_11_001
	.byte	PATT
	 ptrvalue	mus_qz_625_11_001
	.byte		N09   , Cn1 , v100
	.byte	W84
	.byte		        Cn1 , v076
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_625_11_000
	.byte	PATT
	 ptrvalue	mus_qz_625_11_001
	.byte	PATT
	 ptrvalue	mus_qz_625_11_000
	.byte	PATT
	 ptrvalue	mus_qz_625_11_001
	.byte	PATT
	 ptrvalue	mus_qz_625_11_000
	.byte	PATT
	 ptrvalue	mus_qz_625_11_001
	.byte	PATT
	 ptrvalue	mus_qz_625_11_000
	.byte	PATT
	 ptrvalue	mus_qz_625_11_001
	.byte	PATT
	 ptrvalue	mus_qz_625_11_001
	.byte	PATT
	 ptrvalue	mus_qz_625_11_000
	.byte	PATT
	 ptrvalue	mus_qz_625_11_001
	.byte		N09   , Cn1 , v088
	.byte	W12
	.byte		        Cn1 , v072
	.byte	W24
	.byte		        Cn1 , v088
	.byte	W60
	.byte	GOTO
	 ptrvalue	mus_qz_625_11_B1
	.byte	FINE

/******************************************************/
	.p2align	2

mus_qz_625:
	.byte	11	/* NumTrks */
	.byte	0	/* NumBlks */
	.byte	mus_qz_625_pri	/* Priority */
	.byte	mus_qz_625_rev	/* Reverb. */
space64 4
	ptrvalue	mus_qz_625_grp

	ptrvalue	mus_qz_625_1
	ptrvalue	mus_qz_625_2
	ptrvalue	mus_qz_625_3
	ptrvalue	mus_qz_625_4
	ptrvalue	mus_qz_625_5
	ptrvalue	mus_qz_625_6
	ptrvalue	mus_qz_625_7
	ptrvalue	mus_qz_625_8
	ptrvalue	mus_qz_625_9
	ptrvalue	mus_qz_625_10
	ptrvalue	mus_qz_625_11

	.end
