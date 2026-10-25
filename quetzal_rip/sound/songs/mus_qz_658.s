	.include "asm/macros/bit_width.inc"

	.include "MPlayDef.s"

/* Moemon Quetzal song 658, disassembled from the ROM by tools/quetzal_rip/songs.py.
   "org" comments give the ROM address of the bytes that follow. */

	.equ	mus_qz_658_grp, voicegroup_qz_songs
	.equ	mus_qz_658_pri, 0
	.equ	mus_qz_658_rev, reverb_set+0
	.equ	mus_qz_658_mvl, 127
	.equ	mus_qz_658_key, 0
	.equ	mus_qz_658_tbs, 1
	.equ	mus_qz_658_exg, 1
	.equ	mus_qz_658_cmp, 1

	.section .rodata
	.global	mus_qz_658
	.p2align	2
/* org 0x0969F970 */

/********************** Track  1 **********************/

mus_qz_658_1:
	.byte		KEYSH , mus_qz_658_key+0
	.byte		TEMPO , 160*mus_qz_658_tbs/2
	.byte		VOICE , 13
	.byte		VOL   , 80*mus_qz_658_mvl/mxv
	.byte		PAN   , c_v+36
	.byte		VOL   , 80*mus_qz_658_mvl/mxv
	.byte		PRIO  , 60
	.byte		N05   , Gn2 , v072
	.byte		N05   , Bn2 , v092
	.byte	W06
	.byte		        Gs2 , v064
	.byte		N05   , Cn3 , v084
	.byte	W06
	.byte		        An2 , v072
	.byte		N05   , Cs3 , v092
	.byte	W06
	.byte		        As2 , v064
	.byte		N05   , Dn3 , v084
	.byte	W06
	.byte		N92   , Bn2 , v072 , gtp3
	.byte		N92   , Gn3 , v092 , gtp3
	.byte	W09
	.byte		VOL   , 72*mus_qz_658_mvl/mxv
	.byte	W03
	.byte		        65*mus_qz_658_mvl/mxv
	.byte	W02
	.byte		        58*mus_qz_658_mvl/mxv
	.byte	W03
	.byte		        50*mus_qz_658_mvl/mxv
	.byte	W03
	.byte		        44*mus_qz_658_mvl/mxv
	.byte	W04
	.byte		        36*mus_qz_658_mvl/mxv
	.byte	W02
	.byte		        27*mus_qz_658_mvl/mxv
	.byte	W03
	.byte		        18*mus_qz_658_mvl/mxv
	.byte	W03
	.byte		        13*mus_qz_658_mvl/mxv
	.byte	W06
	.byte		        20*mus_qz_658_mvl/mxv
	.byte	W03
	.byte		        32*mus_qz_658_mvl/mxv
	.byte	W03
	.byte		        44*mus_qz_658_mvl/mxv
	.byte	W04
	.byte		        53*mus_qz_658_mvl/mxv
	.byte	W02
	.byte		        62*mus_qz_658_mvl/mxv
	.byte	W03
	.byte		        80*mus_qz_658_mvl/mxv
	.byte	W42
	.byte	W01
	.byte		        67*mus_qz_658_mvl/mxv
	.byte		N03   , Cn3 , v072
	.byte		N03   , Fn3 , v092
	.byte	W24
	.byte		N23   , Cn3 , v068
	.byte		N23   , Fn3 , v088
	.byte	W36
	.byte		N32   , Gs2 , v072 , gtp3
	.byte		N32   , Fs3 , v092 , gtp3
	.byte	W36
mus_qz_658_1_B1:
	.byte		N11   , Dn2 , v076
	.byte	W12
	.byte		N05   , Gn3
	.byte	W12
	.byte		        Bn3 , v068
	.byte	W12
	.byte		        Gn3 , v076
	.byte	W12
	.byte		        Dn4 , v084
	.byte	W12
	.byte		        Cn4 , v072
	.byte	W24
	.byte		        Cn4 , v068
	.byte	W12
	.byte	W96
	.byte		N11   , Dn2 , v076
	.byte	W12
	.byte		N05   , Gn3
	.byte	W12
	.byte		        Bn3 , v068
	.byte	W12
	.byte		        Gn3 , v076
	.byte	W12
	.byte		        Dn4 , v080
	.byte	W12
	.byte		        Cn4 , v076
	.byte	W24
	.byte		        Cn4 , v068
	.byte	W12
	.byte	W96
	.byte		N11   , Dn2 , v076
	.byte	W12
	.byte		N05   , Gn3 , v072
	.byte	W12
	.byte		        Bn3 , v068
	.byte	W12
	.byte		        Gn3 , v076
	.byte	W12
	.byte		        Dn4 , v084
	.byte	W12
	.byte		        Cn4 , v076
	.byte	W24
	.byte	N05
	.byte	W12
	.byte	W96
	.byte		N11   , Dn2
	.byte	W12
	.byte		N05   , Gn3 , v068
	.byte	W12
	.byte		        Bn3 , v064
	.byte	W12
	.byte		        Gn3 , v072
	.byte	W12
	.byte		        Dn4 , v076
	.byte	W12
	.byte		        Cn4 , v072
	.byte	W24
	.byte		        Cn4 , v068
	.byte	W12
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_658_1_B1
	.byte	FINE

/********************** Track  2 **********************/

mus_qz_658_2:
	.byte		KEYSH , mus_qz_658_key+0
	.byte		VOICE , 5
	.byte		VOL   , 73*mus_qz_658_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte	W24
	.byte	W96
	.byte	W96
mus_qz_658_2_B1:
	.byte		N11   , Gn2 , v084
	.byte	W12
	.byte		N05   , Gn3 , v096
	.byte		N05   , Bn3 , v108
	.byte	W12
	.byte		        Bn3 , v088
	.byte		N05   , Dn4 , v100
	.byte	W12
	.byte		        Gn3 , v096
	.byte		N05   , Bn3 , v108
	.byte	W12
	.byte		        Dn4 , v096
	.byte		N05   , Gn4 , v108
	.byte	W12
	.byte		        Cn4 , v088
	.byte		N05   , Fn4 , v100
	.byte	W24
	.byte		        Cn4 , v096
	.byte		N05   , Fn4 , v108
	.byte	W12
	.byte	PEND
	.byte	W96
	.byte		N11   , Gn2 , v084
	.byte	W12
	.byte		N05   , Gn3 , v096
	.byte		N05   , Bn3 , v108
	.byte	W12
	.byte		        Bn3 , v088
	.byte		N05   , Dn4 , v100
	.byte	W12
	.byte		        Gn3 , v096
	.byte		N05   , Bn3 , v108
	.byte	W12
	.byte		        Dn4 , v096
	.byte		N05   , Gn4 , v108
	.byte	W12
	.byte		        Cn4 , v088
	.byte		N05   , Fn4 , v100
	.byte	W24
	.byte		        Fn4 , v096
	.byte		N05   , Gs4 , v108
	.byte	W12
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_658_2_B1
	.byte	W96
	.byte		N11   , Gn2 , v084
	.byte	W12
	.byte		N05   , Gn3 , v096
	.byte		N05   , Bn3 , v108
	.byte	W12
	.byte		        Bn3 , v088
	.byte		N05   , Dn4 , v100
	.byte	W12
	.byte		        Gn3 , v096
	.byte		N05   , Bn3 , v108
	.byte	W12
	.byte		        Dn4 , v096
	.byte		N05   , Gn4 , v108
	.byte	W12
	.byte		        Cn4 , v088
	.byte		N05   , Fn4 , v100
	.byte	W24
	.byte		        Fn4 , v088
	.byte		N05   , Gs4 , v108
	.byte	W12
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_658_2_B1
	.byte	FINE

/********************** Track  3 **********************/

mus_qz_658_3:
	.byte		KEYSH , mus_qz_658_key+0
	.byte		VOICE , 22
	.byte		VOL   , 80*mus_qz_658_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 58
	.byte	W24
	.byte		N11   , Gn0 , v108
	.byte	W12
	.byte		N05   , Gn1 , v092
	.byte	W12
	.byte		        Bn0 , v104
	.byte	W12
	.byte		        Dn1 , v092
	.byte	W12
	.byte		N11   , Gn0 , v104
	.byte	W12
	.byte		N05   , Dn1 , v100
	.byte	W12
	.byte		        Bn1 , v108
	.byte	W12
	.byte		        An1 , v100
	.byte	W12
	.byte		        Fn1 , v108
	.byte	W24
	.byte		N23   , Dn1 , v096
	.byte	W36
	.byte		        Gs0 , v108
	.byte	W24
	.byte		N05   , Gs1 , v096
	.byte	W06
	.byte		        Fn1 , v084
	.byte	W06
mus_qz_658_3_B1:
	.byte		N05   , Gn1 , v104
	.byte	W12
	.byte		N11   , Gn0 , v092
	.byte	W12
	.byte		N05   , Dn2 , v100
	.byte	W12
	.byte		N11   , Gn1 , v092
	.byte	W12
	.byte		        Gn1 , v108
	.byte	W12
	.byte		N05   , Gn2 , v096
	.byte	W12
	.byte		        Dn2 , v108
	.byte	W12
	.byte		        Bn2 , v100
	.byte	W12
	.byte		N11   , Fn1 , v108
	.byte	W12
	.byte		N05   , Ds2 , v092
	.byte	W12
	.byte		        Fn1 , v100
	.byte	W12
	.byte		        Ds2 , v092
	.byte	W12
	.byte		        Fn1 , v104
	.byte	W12
	.byte		        Ds2 , v092
	.byte	W12
	.byte		        Fn1 , v100
	.byte	W12
	.byte		        Ds2 , v092
	.byte	W12
	.byte		        Gn1 , v100
	.byte	W12
	.byte		N11   , Gn0
	.byte	W12
	.byte		N05   , Dn2
	.byte	W12
	.byte		N11   , Dn1
	.byte	W12
	.byte		        Gn1
	.byte	W12
	.byte		N05   , Gn2
	.byte	W12
	.byte		        Gn1
	.byte	W12
	.byte		        Gn2
	.byte	W12
	.byte		        Gs1
	.byte	W12
	.byte		        Ds2 , v092
	.byte	W12
	.byte		        Gs1 , v100
	.byte	W12
	.byte		        Ds2 , v092
	.byte	W12
	.byte		        Gs1 , v100
	.byte	W12
	.byte		        Ds2 , v092
	.byte	W12
	.byte		        Gs1 , v100
	.byte	W12
	.byte		        Ds2 , v092
	.byte	W12
mus_qz_658_3_000:
	.byte		N11   , Gn1 , v108
	.byte	W12
	.byte		N05   , Gn2 , v100
	.byte	W12
	.byte		        Gn1 , v108
	.byte	W12
	.byte		        Gn2 , v100
	.byte	W12
	.byte		        Gn1 , v108
	.byte	W12
	.byte		        Gn2 , v100
	.byte	W12
	.byte		        Gn1 , v108
	.byte	W12
	.byte		        Gn2 , v100
	.byte	W12
	.byte	PEND
	.byte		        Fn1 , v108
	.byte	W12
	.byte		        Ds2 , v100
	.byte	W12
	.byte		        Fn1 , v108
	.byte	W12
	.byte		        Ds2 , v100
	.byte	W12
	.byte		        Fn1 , v108
	.byte	W12
	.byte		        Ds2 , v100
	.byte	W12
	.byte		        Fn1 , v108
	.byte	W12
	.byte		        Ds2 , v100
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_658_3_000
	.byte		N05   , Gs1 , v108
	.byte	W12
	.byte		        Ds2 , v100
	.byte	W12
	.byte		        Gs1 , v108
	.byte	W12
	.byte		        Ds2 , v100
	.byte	W12
	.byte		        Gs1 , v108
	.byte	W12
	.byte		        Ds2 , v100
	.byte	W12
	.byte		        Gs1 , v108
	.byte	W12
	.byte		        Ds2 , v100
	.byte	W12
	.byte	GOTO
	 ptrvalue	mus_qz_658_3_B1
	.byte	FINE

/********************** Track  4 **********************/

mus_qz_658_4:
	.byte		KEYSH , mus_qz_658_key+0
	.byte		VOICE , 6
	.byte		VOL   , 64*mus_qz_658_mvl/mxv
	.byte		PAN   , c_v+46
	.byte		PRIO  , 56
	.byte	W24
	.byte	W96
	.byte	W96
mus_qz_658_4_B1:
	.byte	W96
	.byte	W06
	.byte		N05   , Fn4 , v100
	.byte	W06
	.byte		        Ds4 , v084
	.byte	W06
	.byte		        Dn4 , v092
	.byte	W06
	.byte		PAN   , c_v+29
	.byte		N05   , Cn4 , v080
	.byte	W06
	.byte		        Ds3 , v088
	.byte	W06
	.byte		        Gs3 , v076
	.byte	W06
	.byte		        Cn3 , v080
	.byte	W06
	.byte		PAN   , c_v-24
	.byte		N05   , Ds3 , v072
	.byte	W06
	.byte		        Gs2 , v076
	.byte	W06
	.byte		        Fn3 , v072
	.byte	W06
	.byte		        Cn3 , v084
	.byte	W06
	.byte		PAN   , c_v-50
	.byte		N05   , Gn3 , v076
	.byte	W06
	.byte		        Dn3 , v088
	.byte	W06
	.byte		        Gs3 , v080
	.byte	W06
	.byte		        Fn3 , v092
	.byte	W06
	.byte	W96
	.byte		VOL   , 62*mus_qz_658_mvl/mxv
	.byte		N05   , Gs4 , v100
	.byte	W06
	.byte		        As4 , v088
	.byte	W06
	.byte		        Gs4 , v096
	.byte	W06
	.byte		        Gn4 , v084
	.byte	W06
	.byte		PAN   , c_v-21
	.byte		N05   , Fn4 , v092
	.byte	W06
	.byte		        Gs3 , v080
	.byte	W06
	.byte		        Cn4 , v088
	.byte	W06
	.byte		        Fn3 , v080
	.byte	W06
	.byte		PAN   , c_v+30
	.byte		N05   , Gs3 , v088
	.byte	W06
	.byte		        Cn3 , v080
	.byte	W06
	.byte		        Gs3 , v092
	.byte	W06
	.byte		        Ds3 , v080
	.byte	W06
	.byte		PAN   , c_v+50
	.byte		N05   , As3 , v092
	.byte	W06
	.byte		        Fn3 , v080
	.byte	W06
	.byte		        Cn4 , v092
	.byte	W06
	.byte		        Gs3 , v080
	.byte	W06
	.byte	W96
	.byte		        Ds4 , v100
	.byte	W06
	.byte		        Fn4 , v088
	.byte	W06
	.byte		        Ds4 , v096
	.byte	W06
	.byte		        Dn4 , v088
	.byte	W06
	.byte		PAN   , c_v+29
	.byte		N05   , Cn4 , v092
	.byte	W06
	.byte		        Ds3 , v080
	.byte	W06
	.byte		        Gs3 , v088
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W06
	.byte		PAN   , c_v-24
	.byte		N05   , Ds3 , v084
	.byte	W06
	.byte		        Gs2 , v072
	.byte	W06
	.byte		        Fn3 , v084
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W06
	.byte		PAN   , c_v-50
	.byte		N05   , Gn3 , v088
	.byte	W06
	.byte		        Dn3 , v076
	.byte	W06
	.byte		        Gs3 , v084
	.byte	W06
	.byte		        Fn3 , v072
	.byte	W06
	.byte	W96
	.byte		        Gs4 , v100
	.byte	W06
	.byte		        As4 , v088
	.byte	W06
	.byte		        Gs4 , v092
	.byte	W06
	.byte		        Gn4 , v080
	.byte	W06
	.byte		PAN   , c_v-21
	.byte		N05   , Fn4 , v088
	.byte	W06
	.byte		        Gs3 , v080
	.byte	W06
	.byte		        Cn4 , v088
	.byte	W06
	.byte		        Fn3 , v080
	.byte	W06
	.byte		PAN   , c_v+30
	.byte		N05   , Gs3 , v084
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W06
	.byte		        Gs3 , v084
	.byte	W06
	.byte		        Ds3 , v072
	.byte	W06
	.byte		PAN   , c_v+50
	.byte		N05   , As3 , v084
	.byte	W06
	.byte		        Fn3 , v076
	.byte	W06
	.byte		        Cn4 , v084
	.byte	W06
	.byte		        Gs3 , v076
	.byte	W06
	.byte	GOTO
	 ptrvalue	mus_qz_658_4_B1
	.byte	FINE

/********************** Track  5 **********************/

mus_qz_658_5:
	.byte		KEYSH , mus_qz_658_key+0
	.byte		VOICE , 29
	.byte		VOL   , 86*mus_qz_658_mvl/mxv
	.byte		PAN   , c_v-19
	.byte		PRIO  , 54
	.byte	W24
	.byte	W12
	.byte		N05   , Dn4 , v100
	.byte	W24
	.byte		        Gn3 , v092
	.byte	W24
	.byte		        Cs4 , v100
	.byte	W06
	.byte		        Dn4 , v092
	.byte	W18
	.byte		        Fs4 , v100
	.byte	W06
	.byte		        Gn4 , v092
	.byte	W06
	.byte		        Gs3 , v088
	.byte	W24
	.byte		N23   , Gs3 , v092
	.byte	W36
	.byte		N11   , Dn3 , v084
	.byte	W12
	.byte		N23   , Dn3 , v092
	.byte	W24
mus_qz_658_5_B1:
	.byte		N05   , Gn3 , v108
	.byte	W12
	.byte		        Bn3 , v092
	.byte	W24
	.byte		        Gn4 , v112
	.byte	W24
	.byte		        Dn4 , v100
	.byte	W24
	.byte		N11   , Fn4 , v108
	.byte	W12
	.byte		N05   , Ds4 , v112
	.byte	W06
	.byte		        Fn4 , v096
	.byte	W06
	.byte		        Ds4 , v108
	.byte	W06
	.byte		        Dn4 , v092
	.byte	W06
	.byte		        Cn4 , v112
	.byte	W12
	.byte		        Gs3 , v100
	.byte	W24
	.byte	N05
	.byte	W12
	.byte		        As3 , v088
	.byte	W12
	.byte		        Cn4 , v100
	.byte	W12
	.byte		        Bn3
	.byte	W12
	.byte		        Gn3 , v088
	.byte	W24
	.byte		        Bn3 , v092
	.byte	W24
	.byte		        Fn4 , v096
	.byte	W24
	.byte		N11   , Gs4 , v100
	.byte	W12
	.byte		N05   , Gs4 , v112
	.byte	W06
	.byte		        As4 , v096
	.byte	W06
	.byte		        Gs4 , v100
	.byte	W06
	.byte		        Gn4 , v092
	.byte	W06
	.byte		        Fn4 , v112
	.byte	W12
	.byte		        Cn4 , v100
	.byte	W24
	.byte		        Gs3
	.byte	W12
	.byte		        As3 , v088
	.byte	W12
	.byte		        Cn4 , v100
	.byte	W12
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_658_5_B1
	.byte	FINE

/********************** Track  6 **********************/

mus_qz_658_6:
	.byte		KEYSH , mus_qz_658_key+0
	.byte		VOICE , 29
	.byte		VOL   , 80*mus_qz_658_mvl/mxv
	.byte		PAN   , c_v+6
	.byte		PRIO  , 52
	.byte	W24
	.byte	W12
	.byte		N04   , Gn4 , v072
	.byte	W24
	.byte		        Dn4 , v060
	.byte	W24
	.byte		N05   , As4 , v068
	.byte	W06
	.byte		        Bn4 , v060
	.byte	W18
	.byte		        Cs5 , v072
	.byte	W06
	.byte		        Dn5 , v056
	.byte	W06
	.byte		        Dn4 , v072
	.byte	W24
	.byte		N23   , Dn4 , v064
	.byte	W36
	.byte		N11   , Gs3 , v068
	.byte	W12
	.byte	N23
	.byte	W24
mus_qz_658_6_B1:
	.byte		N04   , Bn2 , v084
	.byte	W12
	.byte		        Dn3 , v072
	.byte	W24
	.byte		        Dn4 , v084
	.byte	W24
	.byte		        Fn3 , v072
	.byte	W24
	.byte		N11   , Dn4 , v080
	.byte	W12
	.byte		N05   , Cn4 , v092
	.byte	W06
	.byte		        Ds4 , v080
	.byte	W06
	.byte		        Cn4 , v088
	.byte	W06
	.byte		        Gs3 , v076
	.byte	W06
	.byte		        Fn3 , v088
	.byte	W12
	.byte		        Ds3 , v080
	.byte	W24
	.byte		N04   , Fn3 , v076
	.byte	W12
	.byte		        Gn3 , v088
	.byte	W12
	.byte		        Gs3 , v080
	.byte	W12
	.byte		        Dn3 , v084
	.byte	W12
	.byte		        Bn2 , v072
	.byte	W24
	.byte		        Dn3 , v084
	.byte	W24
	.byte		N05   , Cn4 , v076
	.byte	W24
	.byte		N11   , Ds4 , v088
	.byte	W12
	.byte		N05   , Dn4
	.byte	W06
	.byte		        Ds4 , v076
	.byte	W06
	.byte		        Dn4 , v084
	.byte	W06
	.byte		        Bn3 , v076
	.byte	W06
	.byte		        Gs3 , v088
	.byte	W12
	.byte		        Ds3 , v076
	.byte	W23
	.byte		N04   , Fn3
	.byte	W12
	.byte		        Gn3 , v088
	.byte	W12
	.byte		        Gs3 , v080
	.byte	W13
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_658_6_B1
	.byte	FINE

/********************** Track  7 **********************/

mus_qz_658_7:
	.byte		KEYSH , mus_qz_658_key+0
	.byte		VOICE , 23
	.byte		VOL   , 77*mus_qz_658_mvl/mxv
	.byte		PAN   , c_v-44
	.byte		PRIO  , 50
	.byte		VOL   , 77*mus_qz_658_mvl/mxv
	.byte		N05   , Bn3 , v080
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        Cs4 , v080
	.byte	W06
	.byte		        Dn4 , v072
	.byte	W06
	.byte		N92   , Gn4 , v080 , gtp3
	.byte	W03
	.byte		VOL   , 70*mus_qz_658_mvl/mxv
	.byte	W03
	.byte		        62*mus_qz_658_mvl/mxv
	.byte	W02
	.byte		        57*mus_qz_658_mvl/mxv
	.byte	W03
	.byte		        48*mus_qz_658_mvl/mxv
	.byte	W03
	.byte		        42*mus_qz_658_mvl/mxv
	.byte	W04
	.byte		        36*mus_qz_658_mvl/mxv
	.byte	W02
	.byte		        26*mus_qz_658_mvl/mxv
	.byte	W03
	.byte		        18*mus_qz_658_mvl/mxv
	.byte	W03
	.byte		        12*mus_qz_658_mvl/mxv
	.byte	W06
	.byte		        19*mus_qz_658_mvl/mxv
	.byte	W03
	.byte		        30*mus_qz_658_mvl/mxv
	.byte	W03
	.byte		        44*mus_qz_658_mvl/mxv
	.byte	W04
	.byte		        52*mus_qz_658_mvl/mxv
	.byte	W02
	.byte		        61*mus_qz_658_mvl/mxv
	.byte	W03
	.byte		        77*mus_qz_658_mvl/mxv
	.byte	W48
	.byte	W01
	.byte		N03   , Fn4
	.byte	W24
	.byte		N23   , Fn4 , v076
	.byte	W36
	.byte		N32   , Fs4 , v080 , gtp3
	.byte	W36
mus_qz_658_7_B1:
	.byte	W96
	.byte	W60
	.byte		N05   , Fn4 , v092
	.byte	W12
	.byte		        Gn4 , v084
	.byte	W12
	.byte		        Gs4 , v088
	.byte	W12
	.byte	W96
	.byte	W60
	.byte		        Gs4 , v100
	.byte	W12
	.byte		        As4 , v088
	.byte	W12
	.byte		        Cn5 , v092
	.byte	W12
	.byte		N32   , Bn4 , v096 , gtp3
	.byte	W36
	.byte		N23   , Gn4 , v084
	.byte	W24
	.byte		N11   , Dn4 , v092
	.byte	W12
	.byte		        Bn3 , v080
	.byte	W12
	.byte		        Gn3 , v088
	.byte	W12
	.byte		N32   , Cn4 , v096 , gtp3
	.byte	W36
	.byte		N23   , Ds4 , v084
	.byte	W24
	.byte		N11   , Gs4 , v092
	.byte	W12
	.byte		        Ds5 , v100
	.byte	W12
	.byte		        Cn5 , v088
	.byte	W12
	.byte		N02   , Ds3 , v100
	.byte	W03
	.byte		N08   , Dn3 , v088
	.byte	W09
	.byte		N05   , Cs3 , v104
	.byte	W12
	.byte		        Dn3 , v096
	.byte	W12
	.byte		N11   , Gn3 , v108
	.byte	W12
	.byte		N05   , Fs3 , v104
	.byte	W12
	.byte		        Gn3 , v092
	.byte	W12
	.byte		N11   , Dn4 , v108
	.byte	W12
	.byte		        Bn3 , v100
	.byte	W12
	.byte		N05   , Gs2 , v104
	.byte	W06
	.byte		        Cn3 , v088
	.byte	W06
	.byte		        Bn2 , v104
	.byte	W06
	.byte		        Cn3 , v092
	.byte	W06
	.byte		        Ds3 , v104
	.byte	W06
	.byte		        Gs3 , v096
	.byte	W06
	.byte		        Gn3 , v104
	.byte	W06
	.byte		        Gs3 , v100
	.byte	W06
	.byte		        Cn4 , v112
	.byte	W06
	.byte		        Ds4 , v104
	.byte	W06
	.byte		        Dn4 , v108
	.byte	W06
	.byte		        Ds4 , v104
	.byte	W06
	.byte		        Ds5 , v116
	.byte	W06
	.byte		        Cn5 , v100
	.byte	W06
	.byte		        Bn4 , v108
	.byte	W06
	.byte		        Cn5 , v100
	.byte	W06
	.byte	GOTO
	 ptrvalue	mus_qz_658_7_B1
	.byte	FINE

/********************** Track  8 **********************/

mus_qz_658_8:
	.byte		KEYSH , mus_qz_658_key+0
	.byte		VOICE , 23
	.byte		VOL   , 73*mus_qz_658_mvl/mxv
	.byte		PAN   , c_v-24
	.byte		PRIO  , 48
	.byte		VOL   , 73*mus_qz_658_mvl/mxv
	.byte		N05   , Gn3 , v080
	.byte	W06
	.byte		        An3 , v072
	.byte	W06
	.byte		        As3 , v080
	.byte	W06
	.byte		        Bn3 , v072
	.byte	W06
	.byte		N92   , Bn3 , v080 , gtp3
	.byte	W03
	.byte		VOL   , 67*mus_qz_658_mvl/mxv
	.byte	W03
	.byte		        59*mus_qz_658_mvl/mxv
	.byte	W02
	.byte		        54*mus_qz_658_mvl/mxv
	.byte	W03
	.byte		        46*mus_qz_658_mvl/mxv
	.byte	W03
	.byte		        40*mus_qz_658_mvl/mxv
	.byte	W04
	.byte		        34*mus_qz_658_mvl/mxv
	.byte	W02
	.byte		        25*mus_qz_658_mvl/mxv
	.byte	W03
	.byte		        17*mus_qz_658_mvl/mxv
	.byte	W03
	.byte		        12*mus_qz_658_mvl/mxv
	.byte	W06
	.byte		        18*mus_qz_658_mvl/mxv
	.byte	W03
	.byte		        29*mus_qz_658_mvl/mxv
	.byte	W03
	.byte		        42*mus_qz_658_mvl/mxv
	.byte	W04
	.byte		        48*mus_qz_658_mvl/mxv
	.byte	W02
	.byte		        57*mus_qz_658_mvl/mxv
	.byte	W03
	.byte		        73*mus_qz_658_mvl/mxv
	.byte	W48
	.byte	W01
	.byte		N03   , Gs3
	.byte	W24
	.byte		N23   , Gs3 , v076
	.byte	W36
	.byte		N32   , Cn4 , v080 , gtp3
	.byte	W36
mus_qz_658_8_B1:
	.byte	W96
	.byte	W60
	.byte		N05   , Cn4 , v088
	.byte	W12
	.byte		        Cn4 , v076
	.byte	W12
	.byte		        Cn4 , v088
	.byte	W12
	.byte	W96
	.byte	W60
	.byte		        Fn4
	.byte	W12
	.byte		        Fn4 , v076
	.byte	W12
	.byte		        Gs4 , v084
	.byte	W12
	.byte		N32   , Dn4 , v088 , gtp3
	.byte	W36
	.byte		N20   , Bn3 , v076
	.byte	W24
	.byte		N11   , Gn3 , v072
	.byte	W12
	.byte		        Dn3 , v084
	.byte	W12
	.byte		N05   , Bn2 , v076
	.byte	W06
	.byte		        Dn3 , v068
	.byte	W06
	.byte		        Cn3 , v088
	.byte	W06
	.byte		        Ds3 , v076
	.byte	W06
	.byte		        Dn3 , v084
	.byte	W06
	.byte		        Ds3 , v080
	.byte	W06
	.byte		        Gs3 , v096
	.byte	W06
	.byte		        Ds3 , v080
	.byte	W06
	.byte		        Cn3 , v084
	.byte	W06
	.byte		        Ds3 , v076
	.byte	W06
	.byte		        Cn4 , v092
	.byte	W06
	.byte		        Gs3 , v072
	.byte	W06
	.byte		        Ds3 , v080
	.byte	W06
	.byte		        Gs3 , v076
	.byte	W06
	.byte		        Gn4 , v092
	.byte	W06
	.byte		        Fn4 , v076
	.byte	W06
	.byte		        Dn4 , v084
	.byte	W06
	.byte		        Bn3 , v076
	.byte	W06
	.byte		N02   , Cn3 , v084
	.byte	W03
	.byte		N08   , Bn2 , v096
	.byte	W09
	.byte		N05   , As2 , v092
	.byte	W12
	.byte		        Bn2 , v084
	.byte	W12
	.byte		N11   , Dn3 , v096
	.byte	W12
	.byte		N05   , Cs3 , v092
	.byte	W12
	.byte		        Dn3 , v080
	.byte	W12
	.byte		N11   , Bn3 , v104
	.byte	W12
	.byte		N05   , Gn3 , v092
	.byte	W06
	.byte		        Dn3 , v084
	.byte	W06
	.byte		        Ds2 , v104
	.byte	W06
	.byte		        Gs2 , v084
	.byte	W06
	.byte		        Gn2 , v092
	.byte	W06
	.byte		        Gs2 , v084
	.byte	W06
	.byte		        Cn3 , v100
	.byte	W06
	.byte		        Ds3 , v084
	.byte	W06
	.byte		        Dn3 , v092
	.byte	W06
	.byte		        Ds3 , v084
	.byte	W06
	.byte		        Gs3 , v100
	.byte	W06
	.byte		        Cn4 , v084
	.byte	W06
	.byte		        Bn3 , v088
	.byte	W06
	.byte		        Cn4 , v084
	.byte	W06
	.byte		        Gs4 , v104
	.byte	W06
	.byte		        Ds4 , v088
	.byte	W06
	.byte		        Dn4 , v092
	.byte	W06
	.byte		        Ds4 , v084
	.byte	W06
	.byte	GOTO
	 ptrvalue	mus_qz_658_8_B1
	.byte	FINE

/********************** Track  9 **********************/

mus_qz_658_9:
	.byte		KEYSH , mus_qz_658_key+0
	.byte		VOICE , 7
	.byte		VOL   , 57*mus_qz_658_mvl/mxv
	.byte		PAN   , c_v-29
	.byte		PRIO  , 44
	.byte		N04   , Bn3 , v088
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        Cs4 , v096
	.byte	W06
	.byte		        Dn4 , v080
	.byte	W06
	.byte		N05   , Gn4 , v092
	.byte	W06
	.byte		        Gn4 , v076
	.byte	W06
	.byte		N03   , Gn4 , v088
	.byte	W04
	.byte		        Gn4 , v064
	.byte	W04
	.byte		        Gn4 , v068
	.byte	W04
	.byte		N05   , Dn4 , v092
	.byte	W06
	.byte		        Dn4 , v076
	.byte	W06
	.byte		N03   , Dn4 , v088
	.byte	W04
	.byte		        Dn4 , v064
	.byte	W04
	.byte		        Dn4 , v068
	.byte	W04
	.byte		N05   , Bn4 , v092
	.byte	W06
	.byte		        Bn4 , v076
	.byte	W06
	.byte		N04   , Bn4 , v080
	.byte	W06
	.byte		        Dn4 , v072
	.byte	W06
	.byte		        Fn4 , v092
	.byte	W06
	.byte		        Gn4 , v076
	.byte	W06
	.byte		        Fn4 , v084
	.byte	W06
	.byte		        Bn3 , v076
	.byte	W06
	.byte		        Fn3 , v092
	.byte	W24
	.byte		N05   , Dn3
	.byte	W06
	.byte		        Dn3 , v076
	.byte	W06
	.byte		N03   , Dn3 , v088
	.byte	W04
	.byte		        Dn3 , v064
	.byte	W04
	.byte		        Dn3 , v068
	.byte	W16
	.byte		N05   , Fs3 , v092
	.byte	W06
	.byte		        Fs3 , v076
	.byte	W06
	.byte		N03   , Fs3 , v088
	.byte	W04
	.byte		        Fs3 , v064
	.byte	W04
	.byte		        Fs3 , v068
	.byte	W04
	.byte		N04   , Fs3 , v084
	.byte	W06
	.byte		        Bn3 , v076
	.byte	W06
mus_qz_658_9_B1:
	.byte	W92
	.byte	W03
	.byte		PAN   , c_v-45
	.byte	W01
	.byte	PEND
	.byte		N04   , Gs4 , v084
	.byte	W06
	.byte		        As4 , v072
	.byte	W06
	.byte		PAN   , c_v-27
	.byte		N04   , Gs4 , v076
	.byte	W06
	.byte		        Fn4 , v068
	.byte	W06
	.byte		PAN   , c_v-16
	.byte		N04   , Ds4 , v080
	.byte	W06
	.byte		        Cn4 , v068
	.byte	W06
	.byte		PAN   , c_v+15
	.byte		N04   , Gs3 , v072
	.byte	W06
	.byte		        Ds3 , v064
	.byte	W06
	.byte		PAN   , c_v+27
	.byte		N04   , Gs2 , v084
	.byte	W06
	.byte		        Cn3 , v064
	.byte	W06
	.byte		PAN   , c_v+40
	.byte		N04   , Ds3 , v076
	.byte	W06
	.byte		        Fn3 , v064
	.byte	W06
	.byte		PAN   , c_v+53
	.byte		N04   , Gn3 , v072
	.byte	W06
	.byte		        Gs3 , v056
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        Ds4 , v056
	.byte	W06
	.byte	W96
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_658_9_B1
	.byte		N04   , Gs4 , v076
	.byte	W06
	.byte		        As4 , v064
	.byte	W06
	.byte		PAN   , c_v-27
	.byte		N04   , Gs4 , v068
	.byte	W06
	.byte		        Fn4 , v060
	.byte	W06
	.byte		PAN   , c_v-16
	.byte		N04   , Ds4 , v076
	.byte	W06
	.byte		        Cn4 , v064
	.byte	W06
	.byte		PAN   , c_v+15
	.byte		N04   , Gs3 , v068
	.byte	W06
	.byte		        Ds3 , v060
	.byte	W06
	.byte		PAN   , c_v+27
	.byte		N04   , Gs2 , v076
	.byte	W06
	.byte		        Cn3 , v064
	.byte	W06
	.byte		PAN   , c_v+40
	.byte		N04   , Ds3 , v072
	.byte	W06
	.byte		        Fn3 , v060
	.byte	W06
	.byte		PAN   , c_v+53
	.byte		N04   , Gn3 , v076
	.byte	W06
	.byte		        Gs3 , v060
	.byte	W06
	.byte		        Cn4 , v068
	.byte	W06
	.byte		        Ds4 , v056
	.byte	W06
	.byte	W96
	.byte		PAN   , c_v-45
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_658_9_B1
	.byte	FINE

/********************** Track 10 **********************/

mus_qz_658_10:
	.byte		KEYSH , mus_qz_658_key+0
	.byte		VOICE , 39
	.byte		VOL   , 80*mus_qz_658_mvl/mxv
	.byte		PAN   , c_v-19
	.byte		PRIO  , 40
	.byte		N05   , En1 , v048
	.byte	W06
	.byte		        En1 , v076
	.byte	W06
	.byte		        An1 , v096
	.byte	W06
	.byte		N06   , Fn1
	.byte	W06
	.byte		N11   , Bn0 , v080
	.byte		N24   , An2 , v088
	.byte	W24
	.byte		N12   , En1 , v076
	.byte	W12
	.byte		N11   , Bn0 , v072
	.byte		N12   , As1 , v088
	.byte	W12
	.byte		N11   , Bn0 , v080
	.byte	W12
	.byte		N06   , En1 , v084
	.byte	W06
	.byte		        En1 , v076
	.byte	W06
	.byte		N11   , Bn0 , v072
	.byte	W12
	.byte		N06   , An1 , v084
	.byte	W06
	.byte		        Fn1 , v076
	.byte	W06
	.byte		N11   , Bn0 , v084
	.byte		N24   , An2 , v076
	.byte	W24
	.byte		N11   , Bn0 , v088
	.byte		N24   , Ds2 , v092
	.byte	W24
	.byte		N05   , En1 , v100
	.byte	W06
	.byte		        En1 , v076
	.byte	W06
	.byte		N11   , Bn0 , v084
	.byte		N24   , En2 , v100
	.byte	W12
	.byte		N05   , Bn1 , v104
	.byte	W06
	.byte		        An1 , v084
	.byte	W06
	.byte		        Gn1 , v100
	.byte	W06
	.byte		        Fn1
	.byte	W06
mus_qz_658_10_B1:
	.byte		N11   , Cn1 , v124
	.byte		N32   , Cs2 , v092 , gtp3
	.byte	W24
	.byte		N11   , Fs0 , v116
	.byte	W12
	.byte		N03   , Fs1 , v088
	.byte	W12
	.byte		N11   , Cn1 , v124
	.byte	W12
	.byte		        Cn1 , v108
	.byte		N03   , Fs1 , v088
	.byte	W12
	.byte		N11   , Fs0 , v112
	.byte		N03   , Fs1 , v060
	.byte	W12
	.byte		N11   , As1 , v092
	.byte	W12
mus_qz_658_10_000:
	.byte		N11   , Cn1 , v124
	.byte	W12
	.byte		N03   , Fs1 , v088
	.byte	W06
	.byte		        Fs1 , v048
	.byte	W06
	.byte		N11   , Fs0 , v108
	.byte	W12
	.byte		        Cn1
	.byte		N03   , Fs1 , v088
	.byte	W12
	.byte		N11   , Cn1 , v124
	.byte	W12
	.byte		N03   , Fs1 , v088
	.byte	W06
	.byte		        Fs1 , v048
	.byte	W06
	.byte		N11   , Fs0 , v112
	.byte		N03   , Fs1 , v088
	.byte	W06
	.byte		        Fs1 , v048
	.byte	W06
	.byte		N11   , As1 , v092
	.byte	W12
	.byte	PEND
	.byte		        Cn1 , v124
	.byte	W12
	.byte		N03   , Fs1 , v088
	.byte	W12
	.byte		N11   , Fs0 , v108
	.byte	W12
	.byte		N03   , Fs1 , v088
	.byte	W12
	.byte		N11   , Cn1 , v124
	.byte	W12
	.byte		        Cn1 , v108
	.byte		N11   , As1 , v092
	.byte	W12
	.byte		        Fs0 , v112
	.byte		N03   , Fs1 , v088
	.byte	W12
	.byte	N03
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_658_10_000
	.byte		N11   , Cn1 , v124
	.byte		N32   , Cs2 , v092 , gtp3
	.byte	W24
	.byte		N11   , Fs0 , v108
	.byte	W12
	.byte		N03   , Fs1 , v092
	.byte	W06
	.byte		        Fs1 , v040
	.byte	W06
	.byte		N11   , Cn1 , v124
	.byte		N03   , Fs1 , v088
	.byte	W06
	.byte		        Fs1 , v036
	.byte	W06
	.byte		N11   , Cn1 , v108
	.byte		N11   , As1 , v092
	.byte	W12
	.byte		        Fs0 , v112
	.byte	W12
	.byte		N03   , Fs1 , v092
	.byte	W06
	.byte		        Fs1 , v040
	.byte	W06
	.byte		N11   , Cn1 , v124
	.byte	W12
	.byte		N03   , Fs1 , v092
	.byte	W06
	.byte		        Fs1 , v056
	.byte	W06
	.byte		N11   , Fs0 , v108
	.byte	W12
	.byte		        Cn1
	.byte		N12   , As1 , v092
	.byte	W12
	.byte		N11   , Cn1 , v124
	.byte	W12
	.byte		N03   , Fs1 , v092
	.byte	W06
	.byte		        Fs1 , v056
	.byte	W06
	.byte		N11   , Fs0 , v112
	.byte		N03   , Fs1 , v092
	.byte	W06
	.byte		        Fs1 , v056
	.byte	W06
	.byte		        Fs1 , v092
	.byte	W06
	.byte		        Fs1 , v056
	.byte	W06
	.byte		N11   , Cn1 , v124
	.byte	W12
	.byte		N03   , Fs1 , v092
	.byte	W06
	.byte		        Fs1 , v040
	.byte	W06
	.byte		N11   , Fs0 , v108
	.byte	W12
	.byte		N03   , Fs1 , v092
	.byte	W06
	.byte		        Fs1 , v040
	.byte	W06
	.byte		N11   , Cn1 , v124
	.byte		N03   , Fs1 , v088
	.byte	W06
	.byte		        Fs1 , v036
	.byte	W06
	.byte		N11   , Cn1 , v108
	.byte		N12   , As1 , v092
	.byte	W12
	.byte		N11   , Fs0 , v112
	.byte	W12
	.byte		N03   , Fs1 , v092
	.byte	W06
	.byte		        Fs1 , v040
	.byte	W06
	.byte		N11   , Cn1 , v124
	.byte	W12
	.byte		N03   , Fs1 , v092
	.byte	W06
	.byte		        Fs1 , v056
	.byte	W06
	.byte		N11   , Fs0 , v108
	.byte	W12
	.byte		        Cn1
	.byte		N12   , As1 , v092
	.byte	W12
	.byte		N11   , Cn1 , v124
	.byte	W12
	.byte		N03   , Fs1 , v092
	.byte	W06
	.byte		        Fs1 , v056
	.byte	W06
	.byte		N11   , Fs0 , v112
	.byte		N03   , Fs1 , v092
	.byte	W06
	.byte		        Fs1 , v056
	.byte	W06
	.byte		N11   , Cn1 , v100
	.byte		N12   , As1 , v092
	.byte	W12
	.byte	GOTO
	 ptrvalue	mus_qz_658_10_B1
	.byte	FINE

/********************** Track 11 **********************/

mus_qz_658_11:
	.byte		KEYSH , mus_qz_658_key+0
	.byte		VOICE , 5
	.byte		VOL   , 15*mus_qz_658_mvl/mxv
	.byte		PAN   , c_v+56
	.byte		PRIO  , 36
	.byte	W24
	.byte	W96
	.byte	W96
mus_qz_658_11_B1:
	.byte	W12
	.byte		N11   , Gn2 , v092
	.byte	W12
	.byte		N05   , Bn3 , v120
	.byte	W12
	.byte		        Dn4
	.byte	W12
	.byte		        Bn3
	.byte	W12
	.byte		        Gn4
	.byte	W12
	.byte		        Fn4
	.byte	W24
	.byte	PEND
	.byte	N05
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_658_11_B1
	.byte		N05   , Gs4 , v120
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_658_11_B1
	.byte		N05   , Fn4 , v120
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_658_11_B1
	.byte		N05   , Fn4 , v120
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_658_11_B1
	.byte	FINE

/******************************************************/
	.p2align	2

mus_qz_658:
	.byte	11	/* NumTrks */
	.byte	0	/* NumBlks */
	.byte	mus_qz_658_pri	/* Priority */
	.byte	mus_qz_658_rev	/* Reverb. */
space64 4
	ptrvalue	mus_qz_658_grp

	ptrvalue	mus_qz_658_1
	ptrvalue	mus_qz_658_2
	ptrvalue	mus_qz_658_3
	ptrvalue	mus_qz_658_4
	ptrvalue	mus_qz_658_5
	ptrvalue	mus_qz_658_6
	ptrvalue	mus_qz_658_7
	ptrvalue	mus_qz_658_8
	ptrvalue	mus_qz_658_9
	ptrvalue	mus_qz_658_10
	ptrvalue	mus_qz_658_11

	.end
