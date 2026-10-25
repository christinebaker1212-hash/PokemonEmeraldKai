	.include "asm/macros/bit_width.inc"

	.include "MPlayDef.s"

/* Moemon Quetzal song 632, disassembled from the ROM by tools/quetzal_rip/songs.py.
   "org" comments give the ROM address of the bytes that follow. */

	.equ	mus_qz_632_grp, voicegroup_qz_songs
	.equ	mus_qz_632_pri, 0
	.equ	mus_qz_632_rev, reverb_set+0
	.equ	mus_qz_632_mvl, 127
	.equ	mus_qz_632_key, 0
	.equ	mus_qz_632_tbs, 1
	.equ	mus_qz_632_exg, 1
	.equ	mus_qz_632_cmp, 1

	.section .rodata
	.global	mus_qz_632
	.p2align	2
/* org 0x09685138 */

/********************** Track  1 **********************/

mus_qz_632_1:
	.byte		KEYSH , mus_qz_632_key+0
	.byte		TEMPO , 84*mus_qz_632_tbs/2
	.byte		VOICE , 5
	.byte		VOL   , 73*mus_qz_632_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte		N23   , Cn5 , v092
	.byte	W24
	.byte		        As4
	.byte	W24
	.byte		N11   , Cs5
	.byte	W12
	.byte		        Ds5
	.byte	W12
	.byte		        Cn5
	.byte	W12
	.byte		        As4
	.byte	W12
mus_qz_632_1_000:
	.byte		N23   , Cn5 , v092
	.byte	W24
	.byte		        As4
	.byte	W24
	.byte		        Cn5
	.byte	W24
	.byte		        As4
	.byte	W24
	.byte	PEND
mus_qz_632_1_B1:
	.byte		N11   , Cs5 , v092
	.byte	W12
	.byte		        Ds5
	.byte	W12
	.byte		        Cn5
	.byte	W12
	.byte		        As4
	.byte	W12
	.byte		N23   , Cn5
	.byte	W24
	.byte		        As4
	.byte	W24
	.byte	PEND
mus_qz_632_1_001:
	.byte		N23   , Cn5 , v092
	.byte	W24
	.byte		        As4
	.byte	W24
	.byte		N11   , Cs5
	.byte	W12
	.byte		        Ds5
	.byte	W12
	.byte		        Cn5
	.byte	W12
	.byte		        As4
	.byte	W12
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_632_1_000
	.byte	PATT
	 ptrvalue	mus_qz_632_1_B1
	.byte	PATT
	 ptrvalue	mus_qz_632_1_001
	.byte	PATT
	 ptrvalue	mus_qz_632_1_000
	.byte	PATT
	 ptrvalue	mus_qz_632_1_B1
	.byte	PATT
	 ptrvalue	mus_qz_632_1_001
	.byte	PATT
	 ptrvalue	mus_qz_632_1_000
	.byte	PATT
	 ptrvalue	mus_qz_632_1_B1
	.byte	PATT
	 ptrvalue	mus_qz_632_1_001
	.byte	PATT
	 ptrvalue	mus_qz_632_1_000
	.byte	GOTO
	 ptrvalue	mus_qz_632_1_B1
	.byte	FINE

/********************** Track  2 **********************/

mus_qz_632_2:
	.byte		KEYSH , mus_qz_632_key+0
	.byte		VOICE , 5
	.byte		VOL   , 26*mus_qz_632_mvl/mxv
	.byte		PAN   , c_v+56
	.byte		PRIO  , 62
	.byte	W12
	.byte		N23   , Cn5 , v100
	.byte	W24
	.byte		        As4
	.byte	W24
	.byte		N11   , Cs5
	.byte	W12
	.byte		        Ds5
	.byte	W12
	.byte		        Cn5
	.byte	W12
mus_qz_632_2_000:
	.byte		N11   , As4 , v100
	.byte	W12
	.byte		N23   , Cn5
	.byte	W24
	.byte		        As4
	.byte	W24
	.byte		        Cn5
	.byte	W24
	.byte		        As4
	.byte	W12
	.byte	PEND
mus_qz_632_2_B1:
	.byte	W12
	.byte		N11   , Cs5 , v100
	.byte	W12
	.byte		        Ds5
	.byte	W12
	.byte		        Cn5
	.byte	W12
	.byte		        As4
	.byte	W12
	.byte		N23   , Cn5
	.byte	W24
	.byte		        As4
	.byte	W12
	.byte	PEND
mus_qz_632_2_001:
	.byte	W12
	.byte		N23   , Cn5 , v100
	.byte	W24
	.byte		        As4
	.byte	W24
	.byte		N11   , Cs5
	.byte	W12
	.byte		        Ds5
	.byte	W12
	.byte		        Cn5
	.byte	W12
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_632_2_000
	.byte	PATT
	 ptrvalue	mus_qz_632_2_B1
	.byte	PATT
	 ptrvalue	mus_qz_632_2_001
	.byte	PATT
	 ptrvalue	mus_qz_632_2_000
	.byte	PATT
	 ptrvalue	mus_qz_632_2_B1
	.byte	PATT
	 ptrvalue	mus_qz_632_2_001
	.byte	PATT
	 ptrvalue	mus_qz_632_2_000
	.byte	PATT
	 ptrvalue	mus_qz_632_2_B1
	.byte	PATT
	 ptrvalue	mus_qz_632_2_001
	.byte	PATT
	 ptrvalue	mus_qz_632_2_000
	.byte	GOTO
	 ptrvalue	mus_qz_632_2_B1
	.byte	W11
	.byte	FINE

/********************** Track  3 **********************/

mus_qz_632_3:
	.byte		KEYSH , mus_qz_632_key+0
	.byte		VOICE , 3
	.byte		VOL   , 57*mus_qz_632_mvl/mxv
	.byte		PAN   , c_v-54
	.byte		PRIO  , 60
	.byte	W96
	.byte	W96
mus_qz_632_3_B1:
	.byte		N02   , Cs2 , v076
	.byte		N02   , Gs2 , v096
	.byte	W06
	.byte		        Cs2 , v016
	.byte		N02   , Gs2 , v024
	.byte	W06
	.byte		        Gs2 , v068
	.byte		N02   , Cs3 , v088
	.byte	W06
	.byte		        Gs2 , v012
	.byte		N02   , Cs3 , v020
	.byte	W06
	.byte		        Cs3 , v080
	.byte		N02   , En3 , v104
	.byte	W06
	.byte		        Cs3 , v016
	.byte		N02   , En3 , v028
	.byte	W66
	.byte		        Cs2 , v076
	.byte		N02   , Gs2 , v096
	.byte	W06
	.byte		        Cs2 , v016
	.byte		N02   , Gs2 , v024
	.byte	W06
	.byte		        Gs2 , v064
	.byte		N02   , Cs3 , v084
	.byte	W06
	.byte		        Gs2 , v008
	.byte		N02   , Cs3 , v016
	.byte	W06
	.byte		        Gs2 , v084
	.byte		N02   , En3 , v108
	.byte	W06
	.byte		        Gs2 , v020
	.byte		N02   , En3 , v032
	.byte	W66
mus_qz_632_3_000:
	.byte		N02   , Cn2 , v100
	.byte	W12
	.byte		        Gs2 , v084
	.byte	W12
	.byte		        En3 , v108
	.byte	W12
	.byte		        Gs3 , v100
	.byte	W12
	.byte		        Cn4
	.byte	W12
	.byte		        Cn3 , v108
	.byte	W12
	.byte		        En3 , v100
	.byte	W12
	.byte		        Gs3
	.byte	W12
	.byte	PEND
mus_qz_632_3_001:
	.byte		N02   , Cs2 , v100
	.byte	W12
	.byte		        Gn2 , v084
	.byte	W12
	.byte		        En3 , v108
	.byte	W12
	.byte		        Bn3 , v100
	.byte	W12
	.byte		        En4
	.byte	W12
	.byte		        En3 , v108
	.byte	W12
	.byte		        Bn3 , v100
	.byte	W12
	.byte		        En4
	.byte	W12
	.byte	PEND
	.byte		        Dn2
	.byte	W12
	.byte		        Gs2 , v084
	.byte	W12
	.byte		        Dn3 , v108
	.byte	W12
	.byte		        Gs3 , v100
	.byte	W12
	.byte		        Cn4
	.byte	W12
	.byte		        Dn3 , v108
	.byte	W12
	.byte		        Gs3 , v100
	.byte	W12
	.byte		        Fn4
	.byte	W12
	.byte		        Cs2
	.byte	W12
	.byte		        Fn2 , v084
	.byte	W12
	.byte		        Bn2 , v108
	.byte	W12
	.byte		        Fn3 , v100
	.byte	W12
	.byte		        Bn3
	.byte	W12
	.byte		        Bn2 , v108
	.byte	W12
	.byte		        Fn3 , v100
	.byte	W12
	.byte		        Bn3
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_632_3_000
	.byte	PATT
	 ptrvalue	mus_qz_632_3_001
	.byte		N02   , Dn2 , v100
	.byte	W12
	.byte		        Gs2 , v084
	.byte	W12
	.byte		        Dn3 , v108
	.byte	W12
	.byte		        Gs3 , v100
	.byte	W12
	.byte		        Cn4 , v108
	.byte	W12
	.byte		        Dn3
	.byte	W12
	.byte		        Gs3 , v100
	.byte	W12
	.byte		        Fn4
	.byte	W12
	.byte		        Bn1
	.byte	W12
	.byte		        Fs2 , v084
	.byte	W12
	.byte		        Bn2 , v108
	.byte	W12
	.byte		        Ds3 , v100
	.byte	W12
	.byte		        Gn3 , v108
	.byte	W12
	.byte		        Ds3
	.byte	W12
	.byte		        Gn3 , v100
	.byte	W12
	.byte		        Ds4
	.byte	W12
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_632_3_B1
	.byte	FINE

/********************** Track  4 **********************/

mus_qz_632_4:
	.byte		KEYSH , mus_qz_632_key+0
	.byte		VOICE , 23
	.byte		VOL   , 65*mus_qz_632_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 58
	.byte	W96
	.byte	W96
mus_qz_632_4_B1:
	.byte	W96
	.byte	W96
	.byte		N92   , Cn1 , v100 , gtp3
	.byte	W24
	.byte		VOL   , 67*mus_qz_632_mvl/mxv
	.byte	W30
	.byte		        71*mus_qz_632_mvl/mxv
	.byte	W24
	.byte		        73*mus_qz_632_mvl/mxv
	.byte	W18
	.byte		N92   , Cs1 , v108 , gtp3
	.byte	W06
	.byte		VOL   , 75*mus_qz_632_mvl/mxv
	.byte	W30
	.byte		        79*mus_qz_632_mvl/mxv
	.byte	W30
	.byte		        84*mus_qz_632_mvl/mxv
	.byte	W30
	.byte		N92   , Dn1 , v112 , gtp3
	.byte	W18
	.byte		VOL   , 76*mus_qz_632_mvl/mxv
	.byte	W48
	.byte		        71*mus_qz_632_mvl/mxv
	.byte	W30
	.byte		N92   , Cs1 , v108 , gtp3
	.byte	W12
	.byte		VOL   , 68*mus_qz_632_mvl/mxv
	.byte	W36
	.byte		        64*mus_qz_632_mvl/mxv
	.byte	W36
	.byte		        67*mus_qz_632_mvl/mxv
	.byte	W12
	.byte		N92   , Cn1 , v108 , gtp3
	.byte	W30
	.byte		VOL   , 71*mus_qz_632_mvl/mxv
	.byte	W36
	.byte		        76*mus_qz_632_mvl/mxv
	.byte	W30
	.byte		N92   , Cs1 , v112 , gtp3
	.byte	W18
	.byte		VOL   , 79*mus_qz_632_mvl/mxv
	.byte	W42
	.byte		        85*mus_qz_632_mvl/mxv
	.byte	W36
	.byte		N92   , Dn1 , v116 , gtp3
	.byte	W06
	.byte		VOL   , 82*mus_qz_632_mvl/mxv
	.byte	W42
	.byte		        79*mus_qz_632_mvl/mxv
	.byte	W42
	.byte		        76*mus_qz_632_mvl/mxv
	.byte	W06
	.byte		N92   , Bn0 , v108 , gtp3
	.byte	W36
	.byte		VOL   , 73*mus_qz_632_mvl/mxv
	.byte	W48
	.byte		        69*mus_qz_632_mvl/mxv
	.byte	W12
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_632_4_B1
	.byte	FINE

/********************** Track  5 **********************/

mus_qz_632_5:
	.byte		KEYSH , mus_qz_632_key+0
	.byte		VOICE , 26
	.byte		VOL   , 62*mus_qz_632_mvl/mxv
	.byte		PAN   , c_v-32
	.byte		PRIO  , 54
	.byte	W96
	.byte	W96
mus_qz_632_5_B1:
	.byte	W96
	.byte	W96
	.byte		N05   , Cn2 , v088
	.byte	W06
	.byte		        Cn2 , v032
	.byte	W06
	.byte		        En3 , v076
	.byte		N05   , Gs3 , v088
	.byte	W06
	.byte		        En3 , v024
	.byte		N05   , Gs3 , v032
	.byte	W06
	.byte		        Gs3 , v076
	.byte		N05   , Cn4 , v088
	.byte	W06
	.byte		        Gs3 , v024
	.byte		N05   , Cn4 , v032
	.byte	W06
	.byte		        En4 , v076
	.byte		N05   , Gs4 , v084
	.byte	W06
	.byte		        En4 , v024
	.byte		N05   , Gs4 , v032
	.byte	W54
mus_qz_632_5_000:
	.byte		N05   , Cs2 , v092
	.byte	W06
	.byte		        Cs2 , v036
	.byte	W06
	.byte		        Gn3 , v076
	.byte		N05   , Bn3 , v088
	.byte	W06
	.byte		        Gn3 , v024
	.byte		N05   , Bn3 , v032
	.byte	W06
	.byte		        Bn3 , v076
	.byte		N05   , En4 , v088
	.byte	W06
	.byte		        Bn3 , v024
	.byte		N05   , En4 , v032
	.byte	W06
	.byte		        En4 , v076
	.byte		N05   , Gs4 , v084
	.byte	W06
	.byte		        En4 , v024
	.byte		N05   , Gs4 , v032
	.byte	W54
	.byte	PEND
	.byte		        Dn2 , v088
	.byte	W06
	.byte		        Dn2 , v036
	.byte	W06
	.byte		        Gs3 , v076
	.byte		N05   , Dn4 , v088
	.byte	W06
	.byte		        Gs3 , v024
	.byte		N05   , Dn4 , v032
	.byte	W06
	.byte		        Dn4 , v076
	.byte		N05   , Gs4 , v088
	.byte	W06
	.byte		        Dn4 , v024
	.byte		N05   , Gs4 , v032
	.byte	W06
	.byte		        Gs4 , v076
	.byte		N05   , Cn5 , v084
	.byte	W06
	.byte		        Gs4 , v024
	.byte		N05   , Cn5 , v032
	.byte	W54
	.byte		        Cs2 , v096
	.byte	W06
	.byte		        Cs2 , v040
	.byte	W06
	.byte		        Fn3 , v076
	.byte		N05   , Cs4 , v088
	.byte	W06
	.byte		        Fn3 , v024
	.byte		N05   , Cs4 , v032
	.byte	W06
	.byte		        Cs4 , v076
	.byte		N05   , Fn4 , v088
	.byte	W06
	.byte		        Cs4 , v024
	.byte		N05   , Fn4 , v032
	.byte	W06
	.byte		        Fn4 , v076
	.byte		N05   , Bn4 , v084
	.byte	W06
	.byte		        Fn4 , v024
	.byte		N05   , Bn4 , v032
	.byte	W54
	.byte		        Cn2 , v092
	.byte	W06
	.byte		        Cn2 , v036
	.byte	W06
	.byte		        En3 , v076
	.byte		N05   , Gs3 , v088
	.byte	W06
	.byte		        En3 , v024
	.byte		N05   , Gs3 , v032
	.byte	W06
	.byte		        Gs3 , v076
	.byte		N05   , Cn4 , v088
	.byte	W06
	.byte		        Gs3 , v024
	.byte		N05   , Cn4 , v032
	.byte	W06
	.byte		        En4 , v076
	.byte		N05   , Gs4 , v084
	.byte	W06
	.byte		        En4 , v024
	.byte		N05   , Gs4 , v032
	.byte	W54
	.byte	PATT
	 ptrvalue	mus_qz_632_5_000
	.byte		N05   , Dn2 , v096
	.byte	W06
	.byte		        Dn2 , v040
	.byte	W06
	.byte		        Gs3 , v076
	.byte		N05   , Dn4 , v088
	.byte	W06
	.byte		        Gs3 , v024
	.byte		N05   , Dn4 , v032
	.byte	W06
	.byte		        Dn4 , v076
	.byte		N05   , Gs4 , v088
	.byte	W06
	.byte		        Dn4 , v024
	.byte		N05   , Gs4 , v032
	.byte	W06
	.byte		        Gs4 , v076
	.byte		N05   , Cn5 , v084
	.byte	W06
	.byte		        Gs4 , v024
	.byte		N05   , Cn5 , v032
	.byte	W54
	.byte		        Bn1 , v088
	.byte	W06
	.byte		        Bn1 , v036
	.byte	W06
	.byte		        Fs3 , v076
	.byte		N05   , Bn3 , v088
	.byte	W06
	.byte		        Fs3 , v024
	.byte		N05   , Bn3 , v032
	.byte	W06
	.byte		        Bn3 , v076
	.byte		N05   , Ds4 , v088
	.byte	W06
	.byte		        Bn3 , v024
	.byte		N05   , Ds4 , v032
	.byte	W06
	.byte		        Ds4 , v076
	.byte		N05   , Gn4 , v084
	.byte	W06
	.byte		        Ds4 , v024
	.byte		N05   , Gn4 , v032
	.byte	W54
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_632_5_B1
	.byte	FINE

/********************** Track  6 **********************/

mus_qz_632_6:
	.byte		KEYSH , mus_qz_632_key+0
	.byte		VOICE , 3
	.byte		VOL   , 14*mus_qz_632_mvl/mxv
	.byte		PAN   , c_v+56
	.byte		PRIO  , 40
	.byte	W96
	.byte	W96
mus_qz_632_6_B1:
	.byte	W12
	.byte		N02   , Gs2 , v100
	.byte	W06
	.byte		        Gs2 , v028
	.byte	W06
	.byte		        Cs3 , v092
	.byte	W06
	.byte		        Cs3 , v024
	.byte	W06
	.byte		        En3 , v108
	.byte	W06
	.byte		        En3 , v032
	.byte	W54
	.byte	W12
	.byte		        Gs2 , v100
	.byte	W06
	.byte		        Gs2 , v028
	.byte	W06
	.byte		        Cs3 , v088
	.byte	W06
	.byte		        Cs3 , v020
	.byte	W06
	.byte		        En3 , v112
	.byte	W06
	.byte		        En3 , v036
	.byte	W54
mus_qz_632_6_000:
	.byte	W06
	.byte		N02   , Cn2 , v100
	.byte	W12
	.byte		        Gs2 , v088
	.byte	W12
	.byte		        En3 , v112
	.byte	W12
	.byte		        Gs3
	.byte	W12
	.byte		        Cn4
	.byte	W12
	.byte		        Cn3
	.byte	W12
	.byte		        En3
	.byte	W12
	.byte		        Gs3
	.byte	W06
	.byte	PEND
mus_qz_632_6_001:
	.byte	W06
	.byte		N02   , Cs2 , v100
	.byte	W12
	.byte		        Gn2 , v088
	.byte	W12
	.byte		        En3 , v112
	.byte	W12
	.byte		        Bn3
	.byte	W12
	.byte		        En4
	.byte	W12
	.byte		        En3
	.byte	W12
	.byte		        Bn3
	.byte	W12
	.byte		        En4
	.byte	W06
	.byte	PEND
mus_qz_632_6_002:
	.byte	W06
	.byte		N02   , Dn2 , v100
	.byte	W12
	.byte		        Gs2 , v088
	.byte	W12
	.byte		        Dn3 , v112
	.byte	W12
	.byte		        Gs3
	.byte	W12
	.byte		        Cn4
	.byte	W12
	.byte		        Dn3
	.byte	W12
	.byte		        Gs3
	.byte	W12
	.byte		        Fn4
	.byte	W06
	.byte	PEND
	.byte	W06
	.byte		        Cs2 , v100
	.byte	W12
	.byte		        Fn2 , v088
	.byte	W12
	.byte		        Bn2 , v112
	.byte	W12
	.byte		        Fn3
	.byte	W12
	.byte		        Bn3
	.byte	W12
	.byte		        Bn2
	.byte	W12
	.byte		        Fn3
	.byte	W12
	.byte		        Bn3
	.byte	W06
	.byte	PATT
	 ptrvalue	mus_qz_632_6_000
	.byte	PATT
	 ptrvalue	mus_qz_632_6_001
	.byte	PATT
	 ptrvalue	mus_qz_632_6_002
	.byte	W06
	.byte		N02   , Bn1 , v100
	.byte	W12
	.byte		        Fs2 , v088
	.byte	W12
	.byte		        Bn2 , v112
	.byte	W12
	.byte		        Ds3
	.byte	W12
	.byte		        Gn3
	.byte	W12
	.byte		        Bn2
	.byte	W12
	.byte		        Ds3
	.byte	W12
	.byte		        Gn3
	.byte	W06
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_632_6_B1
	.byte	FINE

/********************** Track  7 **********************/

mus_qz_632_7:
	.byte		KEYSH , mus_qz_632_key+0
	.byte		VOICE , 8
	.byte		VOL   , 47*mus_qz_632_mvl/mxv
	.byte		PAN   , c_v+26
	.byte		PRIO  , 50
	.byte	W96
	.byte	W96
mus_qz_632_7_B1:
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		N32   , Gn3 , v088 , gtp3
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_632_7_B1
	.byte	FINE

/********************** Track  8 **********************/

mus_qz_632_8:
	.byte		KEYSH , mus_qz_632_key+0
	.byte		VOICE , 28
	.byte		VOL   , 90*mus_qz_632_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 42
	.byte	W96
	.byte	W96
mus_qz_632_8_B1:
	.byte	W96
	.byte	W96
	.byte		N23   , Bn1 , v108
	.byte	W96
	.byte	W96
	.byte	W96
mus_qz_632_8_000:
	.byte	W84
	.byte		N05   , Bn1 , v088
	.byte	W06
	.byte	N05
	.byte	W06
	.byte	PEND
	.byte		N23   , Bn1 , v108
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_632_8_000
	.byte		N23   , Bn1 , v108
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_632_8_B1
	.byte	FINE

/********************** Track  9 **********************/

mus_qz_632_9:
	.byte		KEYSH , mus_qz_632_key+0
	.byte		VOICE , 39
	.byte		VOL   , 26*mus_qz_632_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 45
	.byte	W96
	.byte	W96
mus_qz_632_9_B1:
	.byte		VOL   , 19*mus_qz_632_mvl/mxv
	.byte	W96
mus_qz_632_9_000:
	.byte	W48
	.byte		N44   , Gn0 , v072 , gtp3
	.byte	W12
	.byte		VOL   , 26*mus_qz_632_mvl/mxv
	.byte	W06
	.byte		        32*mus_qz_632_mvl/mxv
	.byte	W06
	.byte		        38*mus_qz_632_mvl/mxv
	.byte	W06
	.byte		        41*mus_qz_632_mvl/mxv
	.byte	W06
	.byte		        44*mus_qz_632_mvl/mxv
	.byte	W06
	.byte		        48*mus_qz_632_mvl/mxv
	.byte	W06
	.byte	PEND
	.byte		N40   , En2 , v068 , gtp1
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_632_9_000
	.byte		N40   , En2 , v068 , gtp1
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_632_9_B1
	.byte	FINE

/********************** Track 10 **********************/

mus_qz_632_10:
	.byte		KEYSH , mus_qz_632_key+0
	.byte		VOICE , 23
	.byte		VOL   , 34*mus_qz_632_mvl/mxv
	.byte		PAN   , c_v+46
	.byte		PRIO  , 56
	.byte	W96
	.byte	W96
mus_qz_632_10_B1:
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W90
	.byte		VOL   , 34*mus_qz_632_mvl/mxv
	.byte	W06
	.byte		N04   , Cn4 , v092
	.byte	W06
	.byte		        Gn3 , v068
	.byte	W06
	.byte		        Gs3 , v088
	.byte	W06
	.byte		VOL   , 35*mus_qz_632_mvl/mxv
	.byte		N04   , As3 , v080
	.byte	W06
	.byte		        Cn4 , v092
	.byte	W06
	.byte		        Gn3 , v068
	.byte	W06
	.byte		VOL   , 35*mus_qz_632_mvl/mxv
	.byte		N04   , Gs3 , v088
	.byte	W06
	.byte		        As3 , v080
	.byte	W06
	.byte		        Cn4 , v092
	.byte	W06
	.byte		VOL   , 36*mus_qz_632_mvl/mxv
	.byte		N04   , Gn2 , v088
	.byte	W06
	.byte		VOL   , 38*mus_qz_632_mvl/mxv
	.byte		N04   , Gs3
	.byte	W06
	.byte		        As3 , v080
	.byte	W06
	.byte		        Cn4 , v092
	.byte	W06
	.byte		VOL   , 38*mus_qz_632_mvl/mxv
	.byte		N04   , Cn3 , v088
	.byte	W06
	.byte		        Gs3
	.byte	W06
	.byte		        As3 , v080
	.byte	W06
	.byte		VOL   , 39*mus_qz_632_mvl/mxv
	.byte		N04   , Cs4 , v092
	.byte	W06
	.byte		        Gn3 , v068
	.byte	W06
	.byte		        Gs3 , v088
	.byte	W06
	.byte		        As3 , v080
	.byte	W06
	.byte		VOL   , 40*mus_qz_632_mvl/mxv
	.byte		N04   , Cs4 , v092
	.byte	W06
	.byte		VOL   , 41*mus_qz_632_mvl/mxv
	.byte		N04   , Gn3 , v068
	.byte	W06
	.byte		        Gs3 , v088
	.byte	W06
	.byte		        As3 , v080
	.byte	W06
	.byte		VOL   , 42*mus_qz_632_mvl/mxv
	.byte		N04   , Cs4 , v092
	.byte	W06
	.byte		        Cs3 , v088
	.byte	W06
	.byte		        Gs3
	.byte	W06
	.byte		VOL   , 43*mus_qz_632_mvl/mxv
	.byte		N04   , As3 , v080
	.byte	W06
	.byte		        Cs4 , v092
	.byte	W06
	.byte		VOL   , 43*mus_qz_632_mvl/mxv
	.byte		N04   , Fn3 , v068
	.byte	W06
	.byte		        Gs3 , v088
	.byte	W06
	.byte		        As3 , v080
	.byte	W06
	.byte		VOL   , 44*mus_qz_632_mvl/mxv
	.byte		N04   , Dn4 , v092
	.byte	W06
	.byte		        Fn3 , v068
	.byte	W06
	.byte		        Gs3 , v088
	.byte	W06
	.byte		VOL   , 46*mus_qz_632_mvl/mxv
	.byte		N04   , Cn4 , v080
	.byte	W06
	.byte		        Dn4 , v092
	.byte	W06
	.byte		        Dn3 , v068
	.byte	W06
	.byte		        Gs3 , v088
	.byte	W06
	.byte		        Cn4 , v080
	.byte	W06
	.byte		        Dn4 , v092
	.byte	W06
	.byte		VOL   , 47*mus_qz_632_mvl/mxv
	.byte		N04   , Fn3 , v068
	.byte	W06
	.byte		        Gs3 , v088
	.byte	W06
	.byte		VOL   , 46*mus_qz_632_mvl/mxv
	.byte		N04   , As3 , v080
	.byte	W06
	.byte		VOL   , 46*mus_qz_632_mvl/mxv
	.byte		N04   , Bn3 , v092
	.byte	W06
	.byte		        Dn3 , v068
	.byte	W06
	.byte		VOL   , 44*mus_qz_632_mvl/mxv
	.byte		N04   , Fn3 , v088
	.byte	W06
	.byte		        As3
	.byte	W06
	.byte		VOL   , 43*mus_qz_632_mvl/mxv
	.byte		N04   , Bn3 , v092
	.byte	W06
	.byte		        Cs3 , v068
	.byte	W06
	.byte		VOL   , 43*mus_qz_632_mvl/mxv
	.byte		N04   , Ds3 , v088
	.byte	W06
	.byte		VOL   , 42*mus_qz_632_mvl/mxv
	.byte		N04   , Gn3 , v080
	.byte	W06
	.byte		        Ds4 , v092
	.byte	W06
	.byte		VOL   , 41*mus_qz_632_mvl/mxv
	.byte		N04   , Ds3 , v068
	.byte	W06
	.byte		        Gn3 , v088
	.byte	W06
	.byte		VOL   , 40*mus_qz_632_mvl/mxv
	.byte		N04   , Bn3 , v080
	.byte	W06
	.byte		VOL   , 39*mus_qz_632_mvl/mxv
	.byte		N04   , Gn3 , v092
	.byte	W06
	.byte		        Cs3 , v068
	.byte	W06
	.byte		VOL   , 38*mus_qz_632_mvl/mxv
	.byte		N04   , Ds3 , v088
	.byte	W06
	.byte		        Gn3 , v080
	.byte	W06
	.byte		VOL   , 38*mus_qz_632_mvl/mxv
	.byte		N04   , Ds4 , v092
	.byte	W06
	.byte		        Cs3 , v068
	.byte	W06
	.byte		VOL   , 36*mus_qz_632_mvl/mxv
	.byte		N04   , Ds3 , v088
	.byte	W06
	.byte		VOL   , 35*mus_qz_632_mvl/mxv
	.byte		N04   , Gn3 , v080
	.byte	W06
	.byte	W06
	.byte		VOL   , 35*mus_qz_632_mvl/mxv
	.byte	W90
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_632_10_B1
	.byte	FINE

/********************** Track 11 **********************/

mus_qz_632_11:
	.byte		KEYSH , mus_qz_632_key+0
	.byte		VOICE , 23
	.byte		VOL   , 16*mus_qz_632_mvl/mxv
	.byte		PAN   , c_v-54
	.byte		PRIO  , 38
	.byte	W96
	.byte	W96
mus_qz_632_11_B1:
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		VOL   , 16*mus_qz_632_mvl/mxv
	.byte	W06
	.byte		N04   , Cn4 , v092
	.byte	W06
	.byte		        Gn3 , v068
	.byte	W06
	.byte		        Gs3 , v088
	.byte	W06
	.byte		VOL   , 18*mus_qz_632_mvl/mxv
	.byte		N04   , As3 , v080
	.byte	W06
	.byte		        Cn4 , v092
	.byte	W06
	.byte		        Gn3 , v068
	.byte	W06
	.byte		VOL   , 19*mus_qz_632_mvl/mxv
	.byte		N04   , Gs3 , v088
	.byte	W06
	.byte		        As3 , v080
	.byte	W06
	.byte		        Cn4 , v092
	.byte	W06
	.byte		VOL   , 19*mus_qz_632_mvl/mxv
	.byte		N04   , Gn2 , v088
	.byte	W06
	.byte		VOL   , 20*mus_qz_632_mvl/mxv
	.byte		N04   , Gs3
	.byte	W06
	.byte		        As3 , v080
	.byte	W06
	.byte		        Cn4 , v092
	.byte	W06
	.byte		VOL   , 21*mus_qz_632_mvl/mxv
	.byte		N04   , Cn3 , v088
	.byte	W06
	.byte		        Gs3
	.byte	W06
	.byte		        As3 , v080
	.byte	W06
	.byte		VOL   , 21*mus_qz_632_mvl/mxv
	.byte		N04   , Cs4 , v092
	.byte	W06
	.byte		        Gn3 , v068
	.byte	W06
	.byte		        Gs3 , v088
	.byte	W06
	.byte		        As3 , v080
	.byte	W06
	.byte		VOL   , 22*mus_qz_632_mvl/mxv
	.byte		N04   , Cs4 , v092
	.byte	W06
	.byte		VOL   , 22*mus_qz_632_mvl/mxv
	.byte		N04   , Gn3 , v068
	.byte	W06
	.byte		        Gs3 , v088
	.byte	W06
	.byte		        As3 , v080
	.byte	W06
	.byte		VOL   , 23*mus_qz_632_mvl/mxv
	.byte		N04   , Cs4 , v092
	.byte	W06
	.byte		        Cs3 , v088
	.byte	W06
	.byte		        Gs3
	.byte	W06
	.byte		VOL   , 24*mus_qz_632_mvl/mxv
	.byte		N04   , As3 , v080
	.byte	W06
	.byte		        Cs4 , v092
	.byte	W06
	.byte		VOL   , 24*mus_qz_632_mvl/mxv
	.byte		N04   , Fn3 , v068
	.byte	W06
	.byte		        Gs3 , v088
	.byte	W06
	.byte		        As3 , v080
	.byte	W06
	.byte		VOL   , 25*mus_qz_632_mvl/mxv
	.byte		N04   , Dn4 , v092
	.byte	W06
	.byte		        Fn3 , v068
	.byte	W06
	.byte		        Gs3 , v088
	.byte	W06
	.byte		VOL   , 26*mus_qz_632_mvl/mxv
	.byte		N04   , Cn4 , v080
	.byte	W06
	.byte		        Dn4 , v092
	.byte	W06
	.byte		        Dn3 , v068
	.byte	W06
	.byte		        Gs3 , v088
	.byte	W06
	.byte		        Cn4 , v080
	.byte	W06
	.byte		        Dn4 , v092
	.byte	W06
	.byte		VOL   , 27*mus_qz_632_mvl/mxv
	.byte		N04   , Fn3 , v068
	.byte	W06
	.byte		        Gs3 , v088
	.byte	W06
	.byte		VOL   , 27*mus_qz_632_mvl/mxv
	.byte		N04   , As3 , v080
	.byte	W06
	.byte		VOL   , 26*mus_qz_632_mvl/mxv
	.byte		N04   , Bn3 , v092
	.byte	W06
	.byte		        Dn3 , v068
	.byte	W06
	.byte		VOL   , 25*mus_qz_632_mvl/mxv
	.byte		N04   , Fn3 , v088
	.byte	W06
	.byte		        As3
	.byte	W06
	.byte		VOL   , 24*mus_qz_632_mvl/mxv
	.byte		N04   , Bn3 , v092
	.byte	W06
	.byte		        Cs3 , v068
	.byte	W06
	.byte		VOL   , 24*mus_qz_632_mvl/mxv
	.byte		N04   , Ds3 , v088
	.byte	W06
	.byte		VOL   , 23*mus_qz_632_mvl/mxv
	.byte		N04   , Gn3 , v080
	.byte	W06
	.byte		        Ds4 , v092
	.byte	W06
	.byte		VOL   , 22*mus_qz_632_mvl/mxv
	.byte		N04   , Ds3 , v068
	.byte	W06
	.byte		        Gn3 , v088
	.byte	W06
	.byte		VOL   , 22*mus_qz_632_mvl/mxv
	.byte		N04   , Bn3 , v080
	.byte	W06
	.byte		VOL   , 21*mus_qz_632_mvl/mxv
	.byte		N04   , Gn3 , v092
	.byte	W06
	.byte		        Cs3 , v068
	.byte	W06
	.byte		VOL   , 21*mus_qz_632_mvl/mxv
	.byte		N04   , Ds3 , v088
	.byte	W06
	.byte		        Gn3 , v080
	.byte	W06
	.byte		VOL   , 20*mus_qz_632_mvl/mxv
	.byte		N04   , Ds4 , v092
	.byte	W06
	.byte		        Cs3 , v068
	.byte	W06
	.byte		VOL   , 19*mus_qz_632_mvl/mxv
	.byte		N04   , Ds3 , v088
	.byte	W06
	.byte		VOL   , 19*mus_qz_632_mvl/mxv
	.byte		N04   , Gn3 , v080
	.byte	W12
	.byte		VOL   , 18*mus_qz_632_mvl/mxv
	.byte	W84
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_632_11_B1
	.byte	FINE

/********************** Track 12 **********************/

mus_qz_632_12:
	.byte		KEYSH , mus_qz_632_key+0
	.byte		VOICE , 5
	.byte		VOL   , 16*mus_qz_632_mvl/mxv
	.byte		PAN   , c_v+46
	.byte		PRIO  , 50
	.byte	W96
	.byte	W48
	.byte		N05   , Gn4 , v080
	.byte	W06
	.byte		        Gn4 , v036
	.byte	W06
	.byte		        Gn4 , v012
	.byte	W06
	.byte		        Gn4 , v004
	.byte	W06
	.byte		VOL   , 21*mus_qz_632_mvl/mxv
	.byte		N05   , Fn4 , v080
	.byte	W06
	.byte		        Fn4 , v036
	.byte	W06
	.byte		        Fn4 , v012
	.byte	W06
	.byte		        Fn4 , v004
	.byte	W06
mus_qz_632_12_B1:
	.byte		VOL   , 22*mus_qz_632_mvl/mxv
	.byte		N05   , Gs4 , v080
	.byte	W06
	.byte		        Gs4 , v028
	.byte	W06
	.byte		        As4 , v080
	.byte	W06
	.byte		        As4 , v028
	.byte	W06
	.byte		        Gn4 , v080
	.byte	W06
	.byte		        Gn4 , v028
	.byte	W06
	.byte		        Fn4 , v080
	.byte	W06
	.byte		        Fn4 , v028
	.byte	W06
	.byte		        Gn4 , v080
	.byte	W06
	.byte		        Gn4 , v036
	.byte	W06
	.byte		        Gn4 , v012
	.byte	W06
	.byte		        Gn4 , v004
	.byte	W06
	.byte		        Fn4 , v080
	.byte	W06
	.byte		        Fn4 , v036
	.byte	W06
	.byte		        Fn4 , v012
	.byte	W06
	.byte		        Fn4 , v004
	.byte	W06
	.byte		        Gs4 , v080
	.byte	W06
	.byte		        Gs4 , v036
	.byte	W06
	.byte		        Gs4 , v012
	.byte	W06
	.byte		        Gs4 , v004
	.byte	W06
	.byte		        Fs4 , v080
	.byte	W06
	.byte		        Fs4 , v036
	.byte	W06
	.byte		        Fs4 , v012
	.byte	W06
	.byte		        Fs4 , v004
	.byte	W06
	.byte		        Gs4 , v080
	.byte	W06
	.byte		        Gs4 , v028
	.byte	W06
	.byte		        As4 , v080
	.byte	W06
	.byte		        As4 , v028
	.byte	W06
	.byte		        Gn4 , v080
	.byte	W06
	.byte		        Gn4 , v028
	.byte	W06
	.byte		        Fn4 , v080
	.byte	W06
	.byte		        Fn4 , v028
	.byte	W06
mus_qz_632_12_000:
	.byte		N05   , Gn4 , v080
	.byte	W06
	.byte		        Gn4 , v036
	.byte	W06
	.byte		        Gn4 , v012
	.byte	W06
	.byte		        Gn4 , v004
	.byte	W06
	.byte		        Fn4 , v080
	.byte	W06
	.byte		        Fn4 , v036
	.byte	W06
	.byte		        Fn4 , v012
	.byte	W06
	.byte		        Fn4 , v004
	.byte	W06
	.byte		        Gn4 , v080
	.byte	W06
	.byte		        Gn4 , v036
	.byte	W06
	.byte		        Gn4 , v012
	.byte	W06
	.byte		        Gn4 , v004
	.byte	W06
	.byte		        Fn4 , v080
	.byte	W06
	.byte		        Fn4 , v036
	.byte	W06
	.byte		        Fn4 , v012
	.byte	W06
	.byte		        Fn4 , v004
	.byte	W06
	.byte	PEND
mus_qz_632_12_001:
	.byte		N05   , Gs4 , v080
	.byte	W06
	.byte		        Gs4 , v028
	.byte	W06
	.byte		        As4 , v080
	.byte	W06
	.byte		        As4 , v028
	.byte	W06
	.byte		        Gn4 , v080
	.byte	W06
	.byte		        Gn4 , v028
	.byte	W06
	.byte		        Fn4 , v080
	.byte	W06
	.byte		        Fn4 , v028
	.byte	W06
	.byte		        Gn4 , v080
	.byte	W06
	.byte		        Gn4 , v036
	.byte	W06
	.byte		        Gn4 , v012
	.byte	W06
	.byte		        Gn4 , v004
	.byte	W06
	.byte		        Fn4 , v080
	.byte	W06
	.byte		        Fn4 , v036
	.byte	W06
	.byte		        Fn4 , v012
	.byte	W06
	.byte		        Fn4 , v004
	.byte	W06
	.byte	PEND
mus_qz_632_12_002:
	.byte		N05   , Gn4 , v080
	.byte	W06
	.byte		        Gn4 , v036
	.byte	W06
	.byte		        Gn4 , v012
	.byte	W06
	.byte		        Gn4 , v004
	.byte	W06
	.byte		        Fn4 , v080
	.byte	W06
	.byte		        Fn4 , v036
	.byte	W06
	.byte		        Fn4 , v012
	.byte	W06
	.byte		        Fn4 , v004
	.byte	W06
	.byte		        Gs4 , v080
	.byte	W06
	.byte		        Gs4 , v028
	.byte	W06
	.byte		        As4 , v080
	.byte	W06
	.byte		        As4 , v028
	.byte	W06
	.byte		        Gn4 , v080
	.byte	W06
	.byte		        Gn4 , v028
	.byte	W06
	.byte		        Fn4 , v080
	.byte	W06
	.byte		        Fn4 , v028
	.byte	W06
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_632_12_000
	.byte	PATT
	 ptrvalue	mus_qz_632_12_001
	.byte	PATT
	 ptrvalue	mus_qz_632_12_002
	.byte	PATT
	 ptrvalue	mus_qz_632_12_000
	.byte	PATT
	 ptrvalue	mus_qz_632_12_001
	.byte	W48
	.byte		VOL   , 19*mus_qz_632_mvl/mxv
	.byte	W48
	.byte	W48
	.byte		N05   , Gn4 , v080
	.byte	W06
	.byte		        Gn4 , v036
	.byte	W06
	.byte		        Gn4 , v012
	.byte	W06
	.byte		        Gn4 , v004
	.byte	W06
	.byte		VOL   , 22*mus_qz_632_mvl/mxv
	.byte		N05   , Fn4 , v080
	.byte	W06
	.byte		        Fn4 , v036
	.byte	W06
	.byte		        Fn4 , v012
	.byte	W06
	.byte		        Fn4 , v004
	.byte	W06
	.byte	GOTO
	 ptrvalue	mus_qz_632_12_B1
	.byte	FINE

/********************** Track 13 **********************/

mus_qz_632_13:
	.byte		KEYSH , mus_qz_632_key+0
	.byte		VOICE , 5
	.byte		VOL   , 8*mus_qz_632_mvl/mxv
	.byte		PAN   , c_v-59
	.byte		PRIO  , 30
	.byte	W96
	.byte	W56
	.byte	W01
	.byte		N05   , Gn4 , v092
	.byte	W06
	.byte		        Gn4 , v048
	.byte	W06
	.byte		        Gn4 , v020
	.byte	W06
	.byte		        Gn4 , v008
	.byte	W06
	.byte		        Fn4 , v092
	.byte	W06
	.byte		        Fn4 , v048
	.byte	W06
	.byte		        Fn4 , v020
	.byte	W03
mus_qz_632_13_B1:
	.byte	W03
	.byte		N05   , Fn4 , v008
	.byte	W03
	.byte		        Gs4 , v092
	.byte	W06
	.byte		        Gs4 , v036
	.byte	W06
	.byte		        As4 , v092
	.byte	W06
	.byte		        As4 , v036
	.byte	W06
	.byte		        Gn4 , v092
	.byte	W06
	.byte		        Gn4 , v036
	.byte	W06
	.byte		        Fn4 , v092
	.byte	W06
	.byte		        Fn4 , v036
	.byte	W06
	.byte		        Gn4 , v092
	.byte	W06
	.byte		        Gn4 , v048
	.byte	W06
	.byte		        Gn4 , v020
	.byte	W06
	.byte		        Gn4 , v008
	.byte	W06
	.byte		        Fn4 , v092
	.byte	W06
	.byte		        Fn4 , v048
	.byte	W06
	.byte		        Fn4 , v020
	.byte	W06
	.byte		        Fn4 , v008
	.byte	W06
	.byte		        Gs4 , v092
	.byte	W06
	.byte		        Gs4 , v048
	.byte	W06
	.byte		        Gs4 , v020
	.byte	W06
	.byte		        Gs4 , v008
	.byte	W06
	.byte		        Fs4 , v092
	.byte	W06
	.byte		        Fs4 , v048
	.byte	W06
	.byte		        Fs4 , v020
	.byte	W06
	.byte		        Fs4 , v008
	.byte	W06
	.byte		        Gs4 , v092
	.byte	W06
	.byte		        Gs4 , v036
	.byte	W06
	.byte		        As4 , v092
	.byte	W06
	.byte		        As4 , v036
	.byte	W06
	.byte		        Gn4 , v092
	.byte	W06
	.byte		        Gn4 , v036
	.byte	W06
	.byte		        Fn4 , v092
	.byte	W06
mus_qz_632_13_000:
	.byte		N05   , Fn4 , v036
	.byte	W06
	.byte		        Gn4 , v092
	.byte	W06
	.byte		        Gn4 , v048
	.byte	W06
	.byte		        Gn4 , v020
	.byte	W06
	.byte		        Gn4 , v008
	.byte	W06
	.byte		        Fn4 , v092
	.byte	W06
	.byte		        Fn4 , v048
	.byte	W06
	.byte		        Fn4 , v020
	.byte	W06
	.byte		        Fn4 , v008
	.byte	W06
	.byte		        Gn4 , v092
	.byte	W06
	.byte		        Gn4 , v048
	.byte	W06
	.byte		        Gn4 , v020
	.byte	W06
	.byte		        Gn4 , v008
	.byte	W06
	.byte		        Fn4 , v092
	.byte	W06
	.byte		        Fn4 , v048
	.byte	W06
	.byte		        Fn4 , v020
	.byte	W06
	.byte	PEND
mus_qz_632_13_001:
	.byte		N05   , Fn4 , v008
	.byte	W06
	.byte		        Gs4 , v092
	.byte	W06
	.byte		        Gs4 , v036
	.byte	W06
	.byte		        As4 , v092
	.byte	W06
	.byte		        As4 , v036
	.byte	W06
	.byte		        Gn4 , v092
	.byte	W06
	.byte		        Gn4 , v036
	.byte	W06
	.byte		        Fn4 , v092
	.byte	W06
	.byte		        Fn4 , v036
	.byte	W06
	.byte		        Gn4 , v092
	.byte	W06
	.byte		        Gn4 , v048
	.byte	W06
	.byte		        Gn4 , v020
	.byte	W06
	.byte		        Gn4 , v008
	.byte	W06
	.byte		        Fn4 , v092
	.byte	W06
	.byte		        Fn4 , v048
	.byte	W06
	.byte		        Fn4 , v020
	.byte	W06
	.byte	PEND
mus_qz_632_13_002:
	.byte		N05   , Fn4 , v008
	.byte	W06
	.byte		        Gn4 , v092
	.byte	W06
	.byte		        Gn4 , v048
	.byte	W06
	.byte		        Gn4 , v020
	.byte	W06
	.byte		        Gn4 , v008
	.byte	W06
	.byte		        Fn4 , v092
	.byte	W06
	.byte		        Fn4 , v048
	.byte	W06
	.byte		        Fn4 , v020
	.byte	W06
	.byte		        Fn4 , v008
	.byte	W06
	.byte		        Gs4 , v092
	.byte	W06
	.byte		        Gs4 , v036
	.byte	W06
	.byte		        As4 , v092
	.byte	W06
	.byte		        As4 , v036
	.byte	W06
	.byte		        Gn4 , v092
	.byte	W06
	.byte		        Gn4 , v036
	.byte	W06
	.byte		        Fn4 , v092
	.byte	W06
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_632_13_000
	.byte	PATT
	 ptrvalue	mus_qz_632_13_001
	.byte	PATT
	 ptrvalue	mus_qz_632_13_002
	.byte	PATT
	 ptrvalue	mus_qz_632_13_000
	.byte	PATT
	 ptrvalue	mus_qz_632_13_001
	.byte		N05   , Fn4 , v008
	.byte	W96
	.byte	W54
	.byte		        Gn4 , v092
	.byte	W06
	.byte		        Gn4 , v048
	.byte	W06
	.byte		        Gn4 , v020
	.byte	W06
	.byte		        Gn4 , v008
	.byte	W06
	.byte		        Fn4 , v092
	.byte	W06
	.byte		        Fn4 , v048
	.byte	W06
	.byte		        Fn4 , v020
	.byte	W06
	.byte	GOTO
	 ptrvalue	mus_qz_632_13_B1
	.byte	FINE

/******************************************************/
	.p2align	2

mus_qz_632:
	.byte	13	/* NumTrks */
	.byte	0	/* NumBlks */
	.byte	mus_qz_632_pri	/* Priority */
	.byte	mus_qz_632_rev	/* Reverb. */
space64 4
	ptrvalue	mus_qz_632_grp

	ptrvalue	mus_qz_632_1
	ptrvalue	mus_qz_632_2
	ptrvalue	mus_qz_632_3
	ptrvalue	mus_qz_632_4
	ptrvalue	mus_qz_632_5
	ptrvalue	mus_qz_632_6
	ptrvalue	mus_qz_632_7
	ptrvalue	mus_qz_632_8
	ptrvalue	mus_qz_632_9
	ptrvalue	mus_qz_632_10
	ptrvalue	mus_qz_632_11
	ptrvalue	mus_qz_632_12
	ptrvalue	mus_qz_632_13

	.end
