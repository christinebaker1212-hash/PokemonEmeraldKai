	.include "asm/macros/bit_width.inc"

	.include "MPlayDef.s"

/* Moemon Quetzal song 615, disassembled from the ROM by tools/quetzal_rip/songs.py.
   "org" comments give the ROM address of the bytes that follow. */

	.equ	mus_qz_615_grp, voicegroup_qz_songs
	.equ	mus_qz_615_pri, 0
	.equ	mus_qz_615_rev, reverb_set+0
	.equ	mus_qz_615_mvl, 127
	.equ	mus_qz_615_key, 0
	.equ	mus_qz_615_tbs, 1
	.equ	mus_qz_615_exg, 1
	.equ	mus_qz_615_cmp, 1

	.section .rodata
	.global	mus_qz_615
	.p2align	2
/* org 0x09671848 */

/********************** Track  1 **********************/

mus_qz_615_1:
	.byte		KEYSH , mus_qz_615_key+0
	.byte		TEMPO , 136*mus_qz_615_tbs/2
	.byte		VOICE , 29
	.byte		VOL   , 61*mus_qz_615_mvl/mxv
	.byte		PAN   , c_v+6
	.byte		PRIO  , 64
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
mus_qz_615_1_B1:
	.byte	W16
	.byte		N01   , Cs4 , v084
	.byte		N01   , Fn4
	.byte	W24
	.byte		N07   , Gs3 , v076
	.byte		N07   , Cs4 , v084
	.byte	W08
	.byte		N01   , Gs3 , v076
	.byte		N01   , Ds4 , v084
	.byte	W24
	.byte		N19   , Gs3 , v076
	.byte		N19   , Cn4 , v084
	.byte	W24
	.byte		N15   , Fn3 , v080
	.byte		N15   , Cs4 , v088
	.byte	W16
	.byte		N02   , Fn3 , v048
	.byte		N02   , Cn4 , v056
	.byte	W08
	.byte		N15   , Fn3 , v064
	.byte		N15   , As3 , v072
	.byte	W16
	.byte		N02   , Fn3 , v048
	.byte		N02   , Cn4 , v056
	.byte	W08
	.byte		N36   , Fn3 , v076 , gtp3
	.byte		N36   , Gs3 , v084 , gtp3
	.byte	W48
	.byte	W16
	.byte		N02   , Fs3 , v076
	.byte		N02   , As3 , v084
	.byte	W24
	.byte		N07   , Gs3 , v076
	.byte		N07   , Cs4 , v084
	.byte	W08
	.byte		        Fs3 , v076
	.byte		N07   , As3 , v084
	.byte	W16
	.byte		N02   , Cs4
	.byte		N02   , As4
	.byte	W08
	.byte		N15   , Cn4 , v076
	.byte		N15   , Gs4 , v084
	.byte	W16
	.byte		N03   , As3 , v076
	.byte		N03   , Fs4 , v084
	.byte	W08
	.byte		N15   , As3 , v064
	.byte		N15   , Fn4 , v072
	.byte	W16
	.byte		N02   , Cn4 , v048
	.byte		N02   , Fs4 , v056
	.byte	W08
	.byte		N15   , Cn4 , v064
	.byte		N15   , Gs4 , v072
	.byte	W16
	.byte		N02   , Cn4 , v048
	.byte		N02   , Fn4 , v056
	.byte	W08
	.byte		N36   , Gs3 , v076 , gtp3
	.byte		N36   , Ds4 , v084 , gtp3
	.byte	W48
	.byte	W16
	.byte		N30   , Fn3 , v084 , gtp1
	.byte		N30   , Cs4 , v096 , gtp1
	.byte	W32
	.byte		N03   , Ds3 , v076
	.byte		N03   , Cn4 , v092
	.byte	W24
	.byte		        Gs3 , v076
	.byte		N03   , Ds4 , v092
	.byte	W24
	.byte	W16
	.byte		        Fn3 , v076
	.byte		N03   , Cs4 , v092
	.byte	W32
	.byte		N07   , Ds3 , v080
	.byte		N07   , Cn4 , v088
	.byte	W08
	.byte		        Dn3 , v064
	.byte		N07   , Bn3 , v072
	.byte	W08
	.byte		        Cs3
	.byte		N07   , As3 , v080
	.byte	W08
	.byte		N23   , Cn3 , v068
	.byte		N23   , Gs3 , v076
	.byte	W24
	.byte	W16
	.byte		N03   , As2 , v080
	.byte		N03   , Fs3 , v084
	.byte	W32
	.byte		N23   , As2 , v076
	.byte		N23   , Fs3 , v084
	.byte	W24
	.byte		        Cs3 , v076
	.byte		N23   , As3 , v084
	.byte	W24
	.byte		N03   , Cn3
	.byte		N03   , Gs3
	.byte	W24
	.byte		N19   , Cn3
	.byte		N19   , Gs3
	.byte	W24
	.byte		N23   , Cs3 , v076
	.byte		N23   , As3 , v084
	.byte	W24
	.byte		N19   , Ds3 , v076
	.byte		N19   , Cn4 , v092
	.byte	W24
	.byte		N36   , Cs4 , v116 , gtp3
	.byte	W40
	.byte		N03   , Fn4 , v120
	.byte	W24
	.byte		N30   , Cs4 , v112 , gtp1
	.byte	W32
	.byte		N36   , Cn4 , v116 , gtp3
	.byte	W40
	.byte		N03   , Ds4
	.byte	W24
	.byte		N30   , Cn4 , v112 , gtp1
	.byte	W32
	.byte		N36   , As3 , v108 , gtp3
	.byte	W40
	.byte		N03   , Cs4 , v112
	.byte	W24
	.byte		N30   , As3 , v104 , gtp1
	.byte	W32
	.byte		N07   , Cn4 , v112
	.byte	W24
	.byte		N23   , Gs3 , v104
	.byte	W24
	.byte		        As3 , v092
	.byte	W24
	.byte		N07   , Gs3 , v100
	.byte	W08
	.byte		        As3
	.byte	W08
	.byte		        Cn4 , v112
	.byte	W08
	.byte		N36   , Cs4 , v112 , gtp3
	.byte	W40
	.byte		N03   , Fn4 , v116
	.byte	W24
	.byte		N30   , Cs4 , v112 , gtp1
	.byte	W32
	.byte		N36   , Cn4 , v112 , gtp3
	.byte	W40
	.byte		N03   , Ds4 , v120
	.byte	W24
	.byte		N07   , Cn4 , v112
	.byte	W08
	.byte		        Ds4 , v116
	.byte	W08
	.byte		        Cs4 , v112
	.byte	W08
	.byte		        Cn4
	.byte	W08
	.byte		N36   , As3 , v108 , gtp3
	.byte	W40
	.byte		N03   , Cs4
	.byte	W24
	.byte		N30   , As3 , v084 , gtp1
	.byte	W32
	.byte		N03   , Cn4 , v112
	.byte	W24
	.byte		N22   , Gs3
	.byte	W24
	.byte		        As3 , v100
	.byte	W24
	.byte		N19   , Cn4 , v116
	.byte	W24
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_615_1_B1
	.byte	FINE

/********************** Track  2 **********************/

mus_qz_615_2:
	.byte		KEYSH , mus_qz_615_key+0
	.byte		VOICE , 20
	.byte		VOL   , 65*mus_qz_615_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 54
	.byte		N03   , Cs3 , v088
	.byte	W48
	.byte		        Gs2
	.byte	W48
mus_qz_615_2_000:
	.byte		N03   , Ds3 , v088
	.byte	W48
	.byte		        Gs2
	.byte	W48
	.byte	PEND
mus_qz_615_2_001:
	.byte		N03   , Cs3 , v088
	.byte	W48
	.byte		        Gs2
	.byte	W48
	.byte	PEND
mus_qz_615_2_002:
	.byte		N03   , Ds3 , v088
	.byte	W48
	.byte		N23   , Gs2
	.byte	W24
	.byte		N07   , Gs1
	.byte	W08
	.byte		        As1 , v068
	.byte	W08
	.byte		        Cn2 , v080
	.byte	W08
	.byte	PEND
mus_qz_615_2_B1:
	.byte		N06   , Cs2 , v096
	.byte	W18
	.byte		N01   , Cs2 , v028
	.byte	W06
	.byte		N06   , Fn2 , v076
	.byte	W16
	.byte		N03   , Cs2 , v088
	.byte	W08
	.byte		N06   , Cn2 , v096
	.byte	W24
	.byte		        Gs2 , v088
	.byte	W16
	.byte		N03   , Cn2
	.byte	W08
	.byte	PEND
mus_qz_615_2_003:
	.byte		N06   , As1 , v100
	.byte	W24
	.byte		        As2 , v088
	.byte	W16
	.byte		N03   , As1
	.byte	W08
	.byte		N06   , Gs1 , v100
	.byte	W24
	.byte		        Gs2 , v088
	.byte	W16
	.byte		N03   , Gs1
	.byte	W08
	.byte	PEND
mus_qz_615_2_004:
	.byte		N06   , Fs1 , v100
	.byte	W24
	.byte		        Fs2 , v088
	.byte	W16
	.byte		N03   , Cs2
	.byte	W08
	.byte		N06   , Ds2 , v096
	.byte	W24
	.byte		        As2 , v088
	.byte	W16
	.byte		N03   , Ds2
	.byte	W08
	.byte	PEND
mus_qz_615_2_005:
	.byte		N06   , Gs1 , v088
	.byte	W24
	.byte	N06
	.byte	W24
	.byte		        As1
	.byte	W24
	.byte		N07   , Gs1
	.byte	W08
	.byte		        As1
	.byte	W08
	.byte		N03   , Cn2
	.byte	W08
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_615_2_B1
	.byte	PATT
	 ptrvalue	mus_qz_615_2_003
	.byte	PATT
	 ptrvalue	mus_qz_615_2_004
	.byte	PATT
	 ptrvalue	mus_qz_615_2_005
	.byte		N06   , Cs2 , v096
	.byte	W18
	.byte		N01   , Cs2 , v028
	.byte	W06
	.byte		N04   , Fn2 , v076
	.byte	W16
	.byte		N03   , Cs2 , v088
	.byte	W08
	.byte		N06   , Gs1 , v096
	.byte	W18
	.byte		N01   , Cs2 , v028
	.byte	W06
	.byte		N04   , Fn2 , v076
	.byte	W16
	.byte		N03   , Cs2 , v088
	.byte	W08
	.byte		N06   , Cn2 , v100
	.byte	W24
	.byte		N04   , Gs2 , v088
	.byte	W16
	.byte		N03   , Cn2
	.byte	W08
	.byte		N06   , Gs1 , v100
	.byte	W24
	.byte		N04   , Gs2 , v088
	.byte	W16
	.byte		N03   , Gs1
	.byte	W08
	.byte		N06   , As1 , v100
	.byte	W24
	.byte		N04   , Fs2 , v088
	.byte	W16
	.byte		N03   , As1
	.byte	W08
	.byte		N06   , Fs1 , v096
	.byte	W24
	.byte		N04   , Fs2 , v088
	.byte	W16
	.byte		N03   , Ds2
	.byte	W08
	.byte	PATT
	 ptrvalue	mus_qz_615_2_005
	.byte		N06   , Cs2 , v096
	.byte	W18
	.byte		N01   , Cs2 , v028
	.byte	W06
	.byte		N06   , Fn2 , v076
	.byte	W16
	.byte		N03   , Cs2 , v088
	.byte	W08
	.byte		N06   , Gs1 , v096
	.byte	W18
	.byte		N01   , Cs2 , v028
	.byte	W06
	.byte		N06   , Fn2 , v076
	.byte	W16
	.byte		N03   , Cs2 , v088
	.byte	W08
	.byte		N06   , Cn2 , v100
	.byte	W24
	.byte		        Gs2 , v088
	.byte	W16
	.byte		N03   , Cn2
	.byte	W08
	.byte		N06   , Gs1 , v100
	.byte	W24
	.byte		        Ds2 , v088
	.byte	W16
	.byte		N03   , Gs1
	.byte	W08
	.byte		N06   , As1 , v100
	.byte	W24
	.byte		        Fs2 , v088
	.byte	W16
	.byte		N03   , Cs2
	.byte	W08
	.byte		N06   , Fs1 , v096
	.byte	W24
	.byte		        Fs2 , v088
	.byte	W16
	.byte		N03   , Cs2
	.byte	W08
	.byte	PATT
	 ptrvalue	mus_qz_615_2_005
	.byte	PATT
	 ptrvalue	mus_qz_615_2_001
	.byte	PATT
	 ptrvalue	mus_qz_615_2_000
	.byte	PATT
	 ptrvalue	mus_qz_615_2_001
	.byte	PATT
	 ptrvalue	mus_qz_615_2_002
	.byte	GOTO
	 ptrvalue	mus_qz_615_2_B1
	.byte	FINE

/********************** Track  3 **********************/

mus_qz_615_3:
	.byte		KEYSH , mus_qz_615_key+0
	.byte		VOICE , 26
	.byte		VOL   , 44*mus_qz_615_mvl/mxv
	.byte		PAN   , c_v+56
	.byte		PRIO  , 56
	.byte	W24
	.byte		N03   , Cs4 , v080
	.byte		N03   , Gs4 , v088
	.byte	W48
	.byte		        Cs4 , v084
	.byte		N03   , Gs4 , v088
	.byte	W24
mus_qz_615_3_000:
	.byte	W24
	.byte		N03   , Ds4 , v084
	.byte		N03   , Gs4
	.byte	W48
	.byte		        Ds4
	.byte		N03   , Gs4
	.byte	W24
	.byte	PEND
mus_qz_615_3_001:
	.byte	W24
	.byte		N03   , Cs4 , v076
	.byte		N03   , Gs4 , v084
	.byte	W48
	.byte		        Cs4 , v060
	.byte		N03   , Gs4 , v084
	.byte	W24
	.byte	PEND
mus_qz_615_3_002:
	.byte		N03   , Ds4 , v088
	.byte		N03   , Gs4 , v084
	.byte	W96
	.byte	PEND
mus_qz_615_3_B1:
	.byte		N05   , Cs3 , v088
	.byte	W16
	.byte		        Gs3
	.byte	W08
	.byte		        Cs4
	.byte		N05   , Fn4
	.byte	W24
	.byte		        Cn3
	.byte	W16
	.byte		        Gs3
	.byte	W08
	.byte		        Cn4
	.byte		N05   , Ds4
	.byte	W24
	.byte		        As2
	.byte	W16
	.byte		        Fn3
	.byte	W08
	.byte		        As3
	.byte		N05   , Cs4
	.byte	W24
	.byte		        Gs2
	.byte	W16
	.byte		        Ds3
	.byte	W08
	.byte		        Gs3
	.byte		N05   , Cn4
	.byte	W24
	.byte		        Fs2
	.byte	W16
	.byte		        Fn3
	.byte	W08
	.byte		        As3
	.byte		N05   , Cs4
	.byte	W24
	.byte		        Ds2
	.byte	W16
	.byte		        Fs3
	.byte	W08
	.byte		        As3
	.byte		N05   , Cs4
	.byte	W24
	.byte		        Gs2
	.byte		N05   , Gs3
	.byte		N05   , Cn4
	.byte	W24
	.byte		        Gs2
	.byte		N05   , Gs3
	.byte		N05   , Cn4
	.byte	W24
	.byte		        As2
	.byte		N05   , As3
	.byte		N05   , Cs4
	.byte	W24
	.byte		        Cn3
	.byte		N05   , Cn4
	.byte		N05   , Ds4
	.byte	W24
	.byte		N03   , Cs3
	.byte	W08
	.byte		        Gs3
	.byte	W08
	.byte		        Cs4
	.byte	W08
	.byte		        Fn4
	.byte	W24
	.byte		        Cn3
	.byte	W08
	.byte		        Gs3
	.byte	W08
	.byte		        Cn4
	.byte	W08
	.byte		        Ds4
	.byte	W24
	.byte		        As2
	.byte	W08
	.byte		        Fn3
	.byte	W08
	.byte		        Gs3
	.byte	W08
	.byte		        Cs4
	.byte	W24
	.byte		        Gs2
	.byte	W08
	.byte		        Ds3
	.byte	W08
	.byte		        Gs3
	.byte	W08
	.byte		        Cn4
	.byte	W24
	.byte		        Fs2
	.byte	W08
	.byte		        Cs3
	.byte	W08
	.byte		        Fs3
	.byte	W08
	.byte		        As3
	.byte	W24
	.byte		        Ds2
	.byte	W08
	.byte		        Cs3
	.byte	W08
	.byte		        Fs3
	.byte	W08
	.byte		        As3
	.byte	W24
	.byte		        Gs2
	.byte		N03   , Cn4
	.byte	W24
	.byte		        Gs2
	.byte		N03   , Cn4
	.byte	W08
	.byte		        Ds3
	.byte	W08
	.byte		        Cn4
	.byte	W08
	.byte		        As2
	.byte		N03   , Cs4
	.byte	W08
	.byte		        Fn3
	.byte	W08
	.byte		        Cs4
	.byte	W08
	.byte		        Cn3
	.byte		N07   , Ds4
	.byte	W08
	.byte		N03   , Gs3
	.byte	W08
	.byte		        Ds4
	.byte	W08
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W24
	.byte		        Cs4 , v080
	.byte		N03   , Gs4 , v088
	.byte	W48
	.byte		        Cs4 , v084
	.byte		N03   , Gs4 , v088
	.byte	W24
	.byte	PATT
	 ptrvalue	mus_qz_615_3_000
	.byte	PATT
	 ptrvalue	mus_qz_615_3_001
	.byte	PATT
	 ptrvalue	mus_qz_615_3_002
	.byte	GOTO
	 ptrvalue	mus_qz_615_3_B1
	.byte	FINE

/********************** Track  4 **********************/

mus_qz_615_4:
	.byte		KEYSH , mus_qz_615_key+0
	.byte		VOICE , 3
	.byte		VOL   , 56*mus_qz_615_mvl/mxv
	.byte		PAN   , c_v-32
	.byte		PRIO  , 60
	.byte		N03   , Cs3 , v108
	.byte	W16
	.byte		        Gs3 , v076
	.byte	W08
	.byte		        Cs4 , v080
	.byte		N03   , Fn4 , v108
	.byte	W16
	.byte		        Gs3 , v084
	.byte	W08
	.byte		        Gs2 , v112
	.byte	W16
	.byte		        Gs3 , v072
	.byte	W08
	.byte		        Cs4 , v080
	.byte		N03   , Fn4 , v104
	.byte	W16
	.byte		        Gs3 , v088
	.byte	W08
mus_qz_615_4_000:
	.byte		N03   , Ds3 , v100
	.byte	W16
	.byte		        As3 , v092
	.byte	W08
	.byte		        Cs4 , v100
	.byte		N03   , Fs4 , v108
	.byte	W08
	.byte		        As3 , v100
	.byte	W08
	.byte		        Cs4
	.byte		N03   , Fs4 , v108
	.byte	W08
	.byte		        Gs2 , v076
	.byte	W16
	.byte		        As3
	.byte	W08
	.byte		        Cs4 , v080
	.byte		N03   , Fs4 , v112
	.byte	W16
	.byte		        As3 , v084
	.byte	W08
	.byte	PEND
mus_qz_615_4_001:
	.byte		N03   , Cs3 , v104
	.byte	W16
	.byte		        Gs3 , v084
	.byte	W08
	.byte		        Cs4 , v100
	.byte		N03   , Fn4 , v108
	.byte	W16
	.byte		        Gs3 , v080
	.byte	W08
	.byte		        Gs2 , v108
	.byte	W16
	.byte		        Gs3 , v088
	.byte	W08
	.byte		        Cs4 , v068
	.byte		N03   , Fn4 , v108
	.byte	W16
	.byte		        Gs3 , v048
	.byte	W08
	.byte	PEND
mus_qz_615_4_002:
	.byte		N03   , Ds3 , v100
	.byte		N03   , As3 , v076
	.byte		N03   , Cs4 , v084
	.byte		N03   , Fs4
	.byte	W48
	.byte		N44   , Gs2 , v092 , gtp3
	.byte		N44   , An3 , v104 , gtp3
	.byte		N44   , Cn4 , v108 , gtp3
	.byte		N23   , Fn4 , v112
	.byte	W24
	.byte		        Ds4 , v092
	.byte	W24
	.byte	PEND
mus_qz_615_4_B1:
	.byte		N03   , Cs3 , v088
	.byte	W08
	.byte		        Gs3
	.byte	W08
	.byte		        Cs4
	.byte	W08
	.byte		        Fn4
	.byte	W24
	.byte		        Cn3
	.byte	W08
	.byte		        Gs3
	.byte	W08
	.byte		        Cn4
	.byte	W08
	.byte		        Ds4
	.byte	W24
	.byte		        As2
	.byte	W08
	.byte		        Fn3
	.byte	W08
	.byte		        Gs3
	.byte	W08
	.byte		        Cs4
	.byte	W24
	.byte		        Gs2
	.byte	W08
	.byte		        Ds3
	.byte	W08
	.byte		        Gs3
	.byte	W08
	.byte		        Cn4
	.byte	W24
	.byte		        Fs2
	.byte	W08
	.byte		        Cs3
	.byte	W08
	.byte		        Fs3
	.byte	W08
	.byte		        As3
	.byte	W24
	.byte		        Ds2
	.byte	W08
	.byte		        Cs3
	.byte	W08
	.byte		        Fs3
	.byte	W08
	.byte		        As3
	.byte	W24
	.byte		        Gs2
	.byte		N03   , Cn4
	.byte	W24
	.byte		        Gs2
	.byte		N03   , Cn4
	.byte	W08
	.byte		        Ds3
	.byte	W08
	.byte		        Cn4
	.byte	W08
	.byte		        As2
	.byte		N03   , Cs4
	.byte	W08
	.byte		        Fn3
	.byte	W08
	.byte		        Cs4
	.byte	W08
	.byte		        Cn3
	.byte		N07   , Ds4
	.byte	W08
	.byte		N03   , Gs3
	.byte	W08
	.byte		        Ds4
	.byte	W08
	.byte		N05   , Cs3
	.byte	W16
	.byte		        Gs3
	.byte	W08
	.byte		        Cs4
	.byte		N05   , Fn4
	.byte	W24
	.byte		        Cn3
	.byte	W16
	.byte		        Gs3
	.byte	W08
	.byte		        Cn4
	.byte		N05   , Ds4
	.byte	W24
	.byte		        As2
	.byte	W16
	.byte		        Fn3
	.byte	W08
	.byte		        As3
	.byte		N05   , Cs4
	.byte	W24
	.byte		        Gs2
	.byte	W16
	.byte		        Ds3
	.byte	W08
	.byte		        Gs3
	.byte		N05   , Cn4
	.byte	W24
	.byte		        Fs2
	.byte	W16
	.byte		        Fn3
	.byte	W08
	.byte		        As3
	.byte		N05   , Cs4
	.byte	W24
	.byte		        Ds2
	.byte	W16
	.byte		        Fs3
	.byte	W08
	.byte		        As3
	.byte		N05   , Cs4
	.byte	W24
	.byte		        Gs2
	.byte		N05   , Gs3
	.byte		N05   , Cn4
	.byte	W24
	.byte		        Gs2
	.byte		N05   , Gs3
	.byte		N05   , Cn4
	.byte	W24
	.byte		        As2
	.byte		N05   , As3
	.byte		N05   , Cs4
	.byte	W24
	.byte		        Cn3
	.byte		N05   , Cn4
	.byte		N05   , Ds4
	.byte	W24
	.byte		N36   , Fn3 , v116 , gtp3
	.byte		N36   , Cs4 , v112 , gtp3
	.byte	W40
	.byte		N03   , Gs3
	.byte		N03   , Fn4
	.byte	W24
	.byte		N24   , Fn3 , v120 , gtp3
	.byte		N24   , Cs4 , v108 , gtp3
	.byte	W32
	.byte		N32   , Ds3 , v092 , gtp3
	.byte		N32   , Cn4 , v116 , gtp3
	.byte	W40
	.byte		N03   , Gs3 , v108
	.byte		N03   , Ds4 , v116
	.byte	W24
	.byte		N24   , Ds3 , v108 , gtp3
	.byte		N24   , Cn4 , v112 , gtp3
	.byte	W32
	.byte		N32   , Cs3 , v104 , gtp3
	.byte		N32   , As3 , v104 , gtp3
	.byte	W40
	.byte		N03   , Fs3
	.byte		N03   , Cs4
	.byte	W24
	.byte		N24   , Cs3 , v108 , gtp3
	.byte		N24   , As3 , v092 , gtp3
	.byte	W32
	.byte		N07   , Gs3 , v104
	.byte		N07   , Cn4 , v112
	.byte	W24
	.byte		N19   , Ds3
	.byte		N19   , Gs3
	.byte	W24
	.byte		        Ds3 , v064
	.byte		N19   , As3 , v104
	.byte	W24
	.byte		        Ds3 , v112
	.byte		N19   , Cn4 , v116
	.byte	W24
	.byte	W16
	.byte		N07   , Fn3 , v108
	.byte	W08
	.byte		        Cs4 , v084
	.byte		N07   , Fn4 , v112
	.byte	W08
	.byte		        Fn3 , v080
	.byte	W08
	.byte		        Cs4 , v052
	.byte		N07   , Fn4 , v112
	.byte	W24
	.byte		        Fn3
	.byte	W08
	.byte		        Cs4 , v100
	.byte		N07   , Fn4 , v116
	.byte	W08
	.byte		        Fn3 , v092
	.byte	W08
	.byte		        Cs4 , v056
	.byte		N07   , Fn4 , v092
	.byte	W08
	.byte	W16
	.byte		        Ds3
	.byte	W08
	.byte		        Cn4 , v112
	.byte		N07   , Ds4
	.byte	W08
	.byte		        Ds3 , v072
	.byte	W08
	.byte		        Cn4 , v080
	.byte		N07   , Ds4 , v104
	.byte	W24
	.byte		        Ds3 , v112
	.byte	W08
	.byte		        Gs3 , v104
	.byte		N07   , Cn4 , v112
	.byte	W08
	.byte		        Ds3 , v108
	.byte	W08
	.byte		        Gs3 , v048
	.byte		N07   , Cn4 , v080
	.byte	W08
	.byte	W16
	.byte		        Cs3 , v100
	.byte	W08
	.byte		        Fs3 , v092
	.byte		N07   , As3 , v104
	.byte	W08
	.byte		        Cs3 , v092
	.byte	W08
	.byte		        Fs3 , v068
	.byte		N07   , As3 , v056
	.byte	W24
	.byte		        Fn3 , v112
	.byte	W08
	.byte		        As3 , v072
	.byte		N07   , Cs4 , v076
	.byte	W08
	.byte		        Fn3 , v088
	.byte	W08
	.byte		        As3 , v044
	.byte		N07   , Cs4 , v068
	.byte	W08
	.byte		N11   , Gs3 , v112
	.byte		N11   , Cn4 , v116
	.byte	W24
	.byte		N21   , Ds3 , v104
	.byte		N21   , Gs3
	.byte	W24
	.byte		        Fn3 , v064
	.byte		N21   , As3 , v100
	.byte	W24
	.byte		        Ds3 , v092
	.byte		N21   , Cn4 , v116
	.byte	W24
	.byte		N03   , Cs3 , v108
	.byte	W16
	.byte		        Gs3 , v076
	.byte	W08
	.byte		        Cs4 , v080
	.byte		N03   , Fn4 , v108
	.byte	W16
	.byte		        Gs3 , v084
	.byte	W08
	.byte		        Gs2 , v112
	.byte	W16
	.byte		        Gs3 , v072
	.byte	W08
	.byte		        Cs4 , v080
	.byte		N03   , Fn4 , v104
	.byte	W16
	.byte		        Gs3 , v088
	.byte	W08
	.byte	PATT
	 ptrvalue	mus_qz_615_4_000
	.byte	PATT
	 ptrvalue	mus_qz_615_4_001
	.byte	PATT
	 ptrvalue	mus_qz_615_4_002
	.byte	GOTO
	 ptrvalue	mus_qz_615_4_B1
	.byte	FINE

/********************** Track  5 **********************/

mus_qz_615_5:
	.byte		KEYSH , mus_qz_615_key+0
	.byte		VOICE , 39
	.byte		VOL   , 36*mus_qz_615_mvl/mxv
	.byte		PAN   , c_v-44
	.byte		PRIO  , 44
	.byte		N03   , Gn3 , v028
	.byte	W16
	.byte		        Gn3 , v008
	.byte	W08
	.byte		        Gn3 , v028
	.byte	W16
	.byte		        Gn3 , v008
	.byte	W08
	.byte		        Gn3 , v028
	.byte	W16
	.byte		        Gn3 , v008
	.byte	W08
	.byte		        Gn3 , v028
	.byte	W16
	.byte		        Gn3 , v008
	.byte	W08
mus_qz_615_5_000:
	.byte		N03   , Gn3 , v028
	.byte	W16
	.byte		        Gn3 , v008
	.byte	W08
	.byte		        Gn3 , v028
	.byte	W16
	.byte		        Gn3 , v008
	.byte	W08
	.byte		        Gn3 , v028
	.byte	W16
	.byte		        Gn3 , v008
	.byte	W08
	.byte		        Gn3 , v028
	.byte	W16
	.byte		        Gn3 , v008
	.byte	W08
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_615_5_000
mus_qz_615_5_001:
	.byte		N03   , Gn3 , v028
	.byte	W48
	.byte	N03
	.byte	W16
	.byte		        Gn3 , v008
	.byte	W08
	.byte		        Gn3 , v028
	.byte	W16
	.byte		        Gn3 , v008
	.byte	W08
	.byte	PEND
mus_qz_615_5_B1:
	.byte		N15   , An2 , v088
	.byte	W16
	.byte		N03   , Gn3 , v040
	.byte	W08
	.byte		N11   , Cs1 , v088
	.byte	W16
	.byte		N03   , Gn3 , v040
	.byte	W08
	.byte		        Gn3 , v076
	.byte	W16
	.byte		        Gn3 , v040
	.byte	W08
	.byte		N11   , Cs1 , v088
	.byte	W16
	.byte		N03   , Gn3 , v040
	.byte	W08
	.byte	PEND
mus_qz_615_5_002:
	.byte		N03   , Gn3 , v076
	.byte	W16
	.byte		        Gn3 , v040
	.byte	W08
	.byte		N11   , Cs1 , v088
	.byte	W16
	.byte		N03   , Gn3 , v040
	.byte	W08
	.byte		        Gn3 , v076
	.byte	W16
	.byte		        Gn3 , v040
	.byte	W08
	.byte		N11   , Cs1 , v088
	.byte	W16
	.byte		N03   , Gn3 , v040
	.byte	W08
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_615_5_002
	.byte	PATT
	 ptrvalue	mus_qz_615_5_002
	.byte	PATT
	 ptrvalue	mus_qz_615_5_002
	.byte	PATT
	 ptrvalue	mus_qz_615_5_002
	.byte	PATT
	 ptrvalue	mus_qz_615_5_002
	.byte	PATT
	 ptrvalue	mus_qz_615_5_002
	.byte	PATT
	 ptrvalue	mus_qz_615_5_B1
	.byte	PATT
	 ptrvalue	mus_qz_615_5_002
	.byte	PATT
	 ptrvalue	mus_qz_615_5_002
	.byte	PATT
	 ptrvalue	mus_qz_615_5_002
	.byte	PATT
	 ptrvalue	mus_qz_615_5_002
	.byte	PATT
	 ptrvalue	mus_qz_615_5_002
	.byte	PATT
	 ptrvalue	mus_qz_615_5_002
	.byte	PATT
	 ptrvalue	mus_qz_615_5_002
	.byte	PATT
	 ptrvalue	mus_qz_615_5_000
	.byte	PATT
	 ptrvalue	mus_qz_615_5_000
	.byte	PATT
	 ptrvalue	mus_qz_615_5_000
	.byte	PATT
	 ptrvalue	mus_qz_615_5_001
	.byte	GOTO
	 ptrvalue	mus_qz_615_5_B1
	.byte	FINE

/********************** Track  6 **********************/

mus_qz_615_6:
	.byte		KEYSH , mus_qz_615_key+0
	.byte		VOICE , 23
	.byte		VOL   , 40*mus_qz_615_mvl/mxv
	.byte		PAN   , c_v-34
	.byte		PRIO  , 60
	.byte		VOL   , 40*mus_qz_615_mvl/mxv
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
mus_qz_615_6_B1:
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W72
	.byte	W02
	.byte		N01   , As1 , v064
	.byte		N01   , Fs2
	.byte	W02
	.byte		        Cn2 , v072
	.byte		N01   , Gs2
	.byte	W02
	.byte		        Cs2 , v076
	.byte		N01   , As2
	.byte	W02
	.byte		        Ds2 , v080
	.byte		N01   , Cn3
	.byte	W02
	.byte		        Fn2 , v084
	.byte		N01   , Cs3
	.byte	W02
	.byte		        Gn2 , v088
	.byte		N01   , Ds3
	.byte	W02
	.byte		        Gs2 , v092
	.byte		N01   , Fn3
	.byte	W02
	.byte		        As2 , v096
	.byte		N01   , Fs3
	.byte	W02
	.byte		        Cn3 , v104
	.byte		N01   , Gs3
	.byte	W02
	.byte		        Cs3
	.byte		N01   , As3
	.byte	W02
	.byte		        Ds3 , v108
	.byte		N01   , Cn4
	.byte	W02
	.byte		N54   , Fn3 , v104 , gtp1
	.byte		N54   , Cs4 , v112 , gtp1
	.byte	W40
	.byte		VOL   , 37*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        34*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        31*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        29*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        26*mus_qz_615_mvl/mxv
	.byte	W08
	.byte		        40*mus_qz_615_mvl/mxv
	.byte		N30   , Gs3 , v084 , gtp1
	.byte		N30   , Fn4 , v100 , gtp1
	.byte	W24
	.byte		VOL   , 35*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        31*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        40*mus_qz_615_mvl/mxv
	.byte		N54   , Gs3 , v068 , gtp1
	.byte		N54   , Ds4 , v112 , gtp1
	.byte	W36
	.byte	W03
	.byte		VOL   , 37*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        34*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        31*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        29*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        26*mus_qz_615_mvl/mxv
	.byte	W08
	.byte		        40*mus_qz_615_mvl/mxv
	.byte	W01
	.byte		N30   , Ds3 , v104 , gtp1
	.byte		N30   , Cn4 , v112 , gtp1
	.byte	W23
	.byte		VOL   , 35*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        31*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        40*mus_qz_615_mvl/mxv
	.byte	W01
	.byte		N54   , Fs3 , v108 , gtp1
	.byte		N54   , Cs4 , v100 , gtp1
	.byte	W40
	.byte		VOL   , 37*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        34*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        31*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        29*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        26*mus_qz_615_mvl/mxv
	.byte	W08
	.byte		        40*mus_qz_615_mvl/mxv
	.byte		N30   , Cs3 , v104 , gtp1
	.byte		N30   , As3 , v104 , gtp1
	.byte	W24
	.byte		VOL   , 35*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        31*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        40*mus_qz_615_mvl/mxv
	.byte		N07   , Ds3 , v076
	.byte		N07   , Cn4 , v112
	.byte	W24
	.byte		N23   , Ds3 , v092
	.byte		N23   , Cn4 , v112
	.byte	W24
	.byte		        Fn3 , v108
	.byte		N23   , Cs4
	.byte	W24
	.byte		N07   , Ds3 , v080
	.byte		N07   , Cn4
	.byte	W08
	.byte		        Fn3 , v092
	.byte		N07   , Cs4
	.byte	W08
	.byte		        Fs3 , v104
	.byte		N07   , Ds4 , v112
	.byte	W08
	.byte		N32   , Gs3 , v080 , gtp3
	.byte		N32   , Fn4 , v112 , gtp3
	.byte	W20
	.byte		VOL   , 36*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        34*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        32*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        29*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        26*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        40*mus_qz_615_mvl/mxv
	.byte		N07   , Gs3
	.byte		N07   , Cs4
	.byte	W24
	.byte		N30   , Gs3 , v100 , gtp1
	.byte		N30   , Fn4 , v112 , gtp1
	.byte	W20
	.byte		VOL   , 37*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        34*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        31*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        40*mus_qz_615_mvl/mxv
	.byte		N36   , Gs3 , v104 , gtp3
	.byte		N36   , Ds4 , v112 , gtp3
	.byte	W20
	.byte		VOL   , 36*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        34*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        32*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        29*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        26*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        40*mus_qz_615_mvl/mxv
	.byte		N07   , Gs3 , v104
	.byte		N07   , Cn4 , v112
	.byte	W24
	.byte		N30   , Gs3 , v112 , gtp1
	.byte		N30   , Ds4 , v112 , gtp1
	.byte	W20
	.byte		VOL   , 37*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        34*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        31*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        40*mus_qz_615_mvl/mxv
	.byte		N36   , Fs3 , v076 , gtp3
	.byte		N36   , Cs4 , v108 , gtp3
	.byte	W20
	.byte		VOL   , 36*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        34*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        32*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        29*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        26*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        40*mus_qz_615_mvl/mxv
	.byte		N07   , Fs3 , v104
	.byte		N07   , As3
	.byte	W24
	.byte		N30   , Fs3 , v092 , gtp1
	.byte		N30   , Cs4 , v108 , gtp1
	.byte	W20
	.byte		VOL   , 37*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        34*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        31*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        40*mus_qz_615_mvl/mxv
	.byte		N07   , Ds3 , v112
	.byte		N07   , Cn4
	.byte	W24
	.byte		N23   , Ds3 , v092
	.byte		N23   , Cn4 , v112
	.byte	W24
	.byte		        Fn3
	.byte		N23   , Cs4 , v108
	.byte	W24
	.byte		        Fs3 , v104
	.byte		N23   , Ds4 , v112
	.byte	W24
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_615_6_B1
	.byte	FINE

/********************** Track  7 **********************/

mus_qz_615_7:
	.byte		KEYSH , mus_qz_615_key+0
	.byte		VOICE , 35
	.byte		VOL   , 49*mus_qz_615_mvl/mxv
	.byte		PAN   , c_v+6
	.byte		PRIO  , 64
	.byte	W96
mus_qz_615_7_000:
	.byte		N03   , Fn4 , v088
	.byte		N03   , Gs4
	.byte	W16
	.byte		N07   , Fn4 , v032
	.byte		N07   , Gs4
	.byte	W08
	.byte		        Fs4 , v036
	.byte		N07   , As4
	.byte	W08
	.byte		        Fn4 , v040
	.byte		N07   , Gs4
	.byte	W08
	.byte		        Fs4 , v044
	.byte		N07   , As4
	.byte	W08
	.byte		        Fn4 , v048
	.byte		N07   , Gs4
	.byte	W08
	.byte		        Fs4 , v064
	.byte		N07   , As4
	.byte	W08
	.byte		        Fn4 , v072
	.byte		N07   , Gs4
	.byte	W08
	.byte		        Fs4 , v084
	.byte		N07   , As4
	.byte	W08
	.byte		        Fn4 , v092
	.byte		N07   , Gs4
	.byte	W08
	.byte		        Fs4
	.byte		N07   , As4
	.byte	N07
	.byte	W08
	.byte	PEND
	.byte		N03   , Fn4 , v068
	.byte		N03   , Gs4
	.byte	W96
	.byte	W96
mus_qz_615_7_B1:
	.byte	W10
	.byte		N01   , Cs5 , v028
	.byte	W01
	.byte		        Ds5 , v060
	.byte		N07   , Fn5 , v120
	.byte	W28
	.byte	W01
	.byte		        Cs5 , v112
	.byte	W08
	.byte		N03   , Ds5 , v104
	.byte	W24
	.byte		N23   , Cn5 , v116
	.byte	W24
mus_qz_615_7_001:
	.byte		N15   , Cs5 , v100
	.byte	W16
	.byte		N07   , Cn5 , v104
	.byte	W08
	.byte		N15   , As4 , v112
	.byte	W16
	.byte		N07   , Cn5 , v108
	.byte	W08
	.byte		N36   , Gs4 , v112 , gtp3
	.byte	W48
	.byte	PEND
	.byte	W12
	.byte	N03
	.byte	W04
	.byte		        As4 , v120
	.byte	W24
	.byte		N07   , Cs5 , v112
	.byte	W08
	.byte		N11   , As4
	.byte	W16
	.byte		N07   , As5
	.byte	W08
	.byte		N15   , Gs5
	.byte	W16
	.byte		N07   , Fs5 , v104
	.byte	W08
	.byte		N15   , Fn5 , v116
	.byte	W16
	.byte		N07   , Fs5 , v100
	.byte	W08
	.byte		N15   , Gs5 , v112
	.byte	W16
	.byte		N07   , Fn5
	.byte	W08
	.byte		N32   , Ds5 , v104 , gtp3
	.byte	W48
	.byte	W10
	.byte		N01   , Cs5 , v068
	.byte	W02
	.byte		N03   , Ds5 , v112
	.byte	W04
	.byte		N07   , Fn5 , v120
	.byte	W24
	.byte		        Cs5 , v112
	.byte	W08
	.byte		N03   , Ds5 , v104
	.byte	W24
	.byte		N23   , Cn5 , v116
	.byte	W24
	.byte	PATT
	 ptrvalue	mus_qz_615_7_001
	.byte	W12
	.byte		N03   , Gs4 , v084
	.byte	W04
	.byte		        As4 , v120
	.byte	W24
	.byte		N07   , Cs5 , v112
	.byte	W08
	.byte		N11   , As4
	.byte	W16
	.byte		N07   , As5
	.byte	W08
	.byte		N15   , Gs5
	.byte	W16
	.byte		N07   , Fs5 , v104
	.byte	W08
	.byte		N15   , Gs5 , v116
	.byte	W16
	.byte		N07   , As5 , v100
	.byte	W08
	.byte		N15   , Cn6 , v112
	.byte	W16
	.byte		N07   , Cs6
	.byte	W08
	.byte		N36   , Ds6 , v104 , gtp3
	.byte	W48
	.byte		N32   , Cs5 , v116 , gtp3
	.byte	W40
	.byte		N03   , Fn5 , v120
	.byte	W24
	.byte		N28   , Cs5 , v112 , gtp1
	.byte	W32
	.byte		N36   , Cn5 , v116 , gtp1
	.byte	W36
	.byte	W01
	.byte		N04   , Cs5 , v104
	.byte	W03
	.byte		N05   , Ds5 , v112
	.byte	W22
	.byte		N30   , Cn5 , v120 , gtp1
	.byte	W32
	.byte	W02
	.byte		N36   , As4 , v116 , gtp1
	.byte	W36
	.byte	W02
	.byte		N04   , Cn5 , v112
	.byte	W03
	.byte		N05   , Cs5
	.byte	W22
	.byte		N30   , As4 , v116 , gtp1
	.byte	W32
	.byte	W01
	.byte		N07   , Cn5 , v120
	.byte	W24
	.byte		N21   , Gs4 , v116
	.byte	W24
	.byte		        As4 , v112
	.byte	W24
	.byte		        Cn5 , v116
	.byte	W24
	.byte		N32   , Cs5 , v108 , gtp3
	.byte	W36
	.byte		N06   , Ds5
	.byte	W04
	.byte		N05   , Fn5 , v120
	.byte	W22
	.byte		N28   , Cs5 , v112 , gtp1
	.byte	W32
	.byte	W02
	.byte		N36   , Cn5 , v116
	.byte	W36
	.byte		N06   , Cs5 , v100
	.byte	W03
	.byte		N05   , Ds5 , v112
	.byte	W24
	.byte	W01
	.byte		N07   , Cn5
	.byte	W08
	.byte		N09   , Ds5 , v104
	.byte	W07
	.byte		N07   , Cs5
	.byte	W08
	.byte		        Cn5 , v112
	.byte	W08
	.byte		N30   , As4 , v112 , gtp1
	.byte	W01
	.byte	W36
	.byte		N06   , As4 , v076
	.byte	W03
	.byte		N04   , Cn5 , v104
	.byte	W01
	.byte		N08   , Cs5 , v112
	.byte	W24
	.byte		N30   , As4 , v116 , gtp1
	.byte	W30
	.byte	W01
	.byte		N05   , Cn5 , v120
	.byte	W01
	.byte	W24
	.byte		N21   , Gs4 , v112
	.byte	W24
	.byte		N23   , As4 , v116
	.byte	W24
	.byte		N21   , Cn5 , v120
	.byte	W24
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_615_7_000
	.byte		N03   , Fn4 , v068
	.byte		N03   , Gs4
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_615_7_B1
	.byte	FINE

/********************** Track  8 **********************/

mus_qz_615_8:
	.byte		KEYSH , mus_qz_615_key+0
	.byte		VOICE , 4
	.byte		VOL   , 13*mus_qz_615_mvl/mxv
	.byte		PAN   , c_v+56
	.byte		PRIO  , 38
	.byte	W06
	.byte		N03   , Cs3 , v108
	.byte	W16
	.byte		        Gs3 , v076
	.byte	W08
	.byte		        Cs4 , v080
	.byte		N03   , Fn4 , v108
	.byte	W16
	.byte		        Gs3 , v084
	.byte	W08
	.byte		        Gs2 , v112
	.byte	W16
	.byte		        Gs3 , v072
	.byte	W08
	.byte		        Cs4 , v080
	.byte		N03   , Fn4 , v104
	.byte	W16
	.byte		        Gs3 , v088
	.byte	W02
mus_qz_615_8_000:
	.byte	W06
	.byte		N03   , Ds3 , v100
	.byte	W16
	.byte		        As3 , v092
	.byte	W08
	.byte		        Cs4 , v100
	.byte		N03   , Fs4 , v108
	.byte	W08
	.byte		        As3 , v100
	.byte	W08
	.byte		        Cs4
	.byte		N03   , Fs4 , v108
	.byte	W08
	.byte		        Gs2 , v076
	.byte	W16
	.byte		        As3
	.byte	W08
	.byte		        Cs4 , v080
	.byte		N03   , Fs4 , v112
	.byte	W16
	.byte		        As3 , v084
	.byte	W02
	.byte	PEND
mus_qz_615_8_001:
	.byte	W06
	.byte		N03   , Cs3 , v104
	.byte	W16
	.byte		        Gs3 , v084
	.byte	W08
	.byte		        Cs4 , v100
	.byte		N03   , Fn4 , v108
	.byte	W16
	.byte		        Gs3 , v080
	.byte	W08
	.byte		        Gs2 , v108
	.byte	W16
	.byte		        Gs3 , v088
	.byte	W08
	.byte		        Cs4 , v068
	.byte		N03   , Fn4 , v108
	.byte	W16
	.byte		        Gs3 , v048
	.byte	W02
	.byte	PEND
mus_qz_615_8_002:
	.byte	W06
	.byte		N03   , Ds3 , v100
	.byte		N03   , As3 , v076
	.byte		N03   , Cs4 , v084
	.byte		N03   , Fs4
	.byte	W48
	.byte		N44   , Gs2 , v092 , gtp3
	.byte		N44   , An3 , v104 , gtp3
	.byte		N44   , Cn4 , v108 , gtp3
	.byte		N23   , Fn4 , v112
	.byte	W24
	.byte		        Ds4 , v092
	.byte	W18
	.byte	PEND
mus_qz_615_8_B1:
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
	.byte	W06
	.byte		N03   , Cs3 , v108
	.byte	W16
	.byte		        Gs3 , v076
	.byte	W08
	.byte		        Cs4 , v080
	.byte		N03   , Fn4 , v108
	.byte	W16
	.byte		        Gs3 , v084
	.byte	W08
	.byte		        Gs2 , v112
	.byte	W16
	.byte		        Gs3 , v072
	.byte	W08
	.byte		        Cs4 , v080
	.byte		N03   , Fn4 , v104
	.byte	W16
	.byte		        Gs3 , v088
	.byte	W02
	.byte	PATT
	 ptrvalue	mus_qz_615_8_000
	.byte	PATT
	 ptrvalue	mus_qz_615_8_001
	.byte	PATT
	 ptrvalue	mus_qz_615_8_002
	.byte	GOTO
	 ptrvalue	mus_qz_615_8_B1
	.byte	W05
	.byte	FINE

/********************** Track  9 **********************/

mus_qz_615_9:
	.byte		KEYSH , mus_qz_615_key+0
	.byte		VOICE , 35
	.byte		VOL   , 13*mus_qz_615_mvl/mxv
	.byte		PAN   , c_v+56
	.byte		PRIO  , 36
	.byte	W96
mus_qz_615_9_000:
	.byte	W06
	.byte		N03   , Fn4 , v072
	.byte		N03   , Gs4
	.byte	W16
	.byte		N07   , Fn4 , v032
	.byte		N07   , Gs4
	.byte	W08
	.byte		        Fs4 , v036
	.byte		N07   , As4
	.byte	W08
	.byte		        Fn4 , v040
	.byte		N07   , Gs4
	.byte	W08
	.byte		        Fs4 , v044
	.byte		N07   , As4
	.byte	W08
	.byte		        Fn4 , v048
	.byte		N07   , Gs4
	.byte	W08
	.byte		        Fs4 , v064
	.byte		N07   , As4
	.byte	W08
	.byte		        Fn4 , v072
	.byte		N07   , Gs4
	.byte	W08
	.byte		        Fs4 , v084
	.byte		N07   , As4
	.byte	W08
	.byte		        Fn4 , v092
	.byte		N07   , Gs4
	.byte	W08
	.byte		        Fs4
	.byte		N07   , As4
	.byte	N07
	.byte	W02
	.byte	PEND
mus_qz_615_9_001:
	.byte	W06
	.byte		N03   , Fn4 , v068
	.byte		N03   , Gs4
	.byte	W90
	.byte	PEND
	.byte	W96
mus_qz_615_9_B1:
	.byte	W16
	.byte		N01   , Cs5 , v028
	.byte	W01
	.byte		        Ds5 , v060
	.byte		N07   , Fn5 , v120
	.byte	W28
	.byte	W01
	.byte		        Cs5 , v112
	.byte	W08
	.byte		N03   , Ds5 , v104
	.byte	W24
	.byte		N23   , Cn5 , v116
	.byte	W18
mus_qz_615_9_002:
	.byte	W06
	.byte		N15   , Cs5 , v100
	.byte	W16
	.byte		N07   , Cn5 , v104
	.byte	W08
	.byte		N15   , As4 , v112
	.byte	W16
	.byte		N07   , Cn5 , v108
	.byte	W08
	.byte		N36   , Gs4 , v112 , gtp3
	.byte	W42
	.byte	PEND
	.byte	W18
	.byte	N03
	.byte	W04
	.byte		        As4 , v120
	.byte	W24
	.byte		N07   , Cs5 , v112
	.byte	W08
	.byte		N11   , As4
	.byte	W16
	.byte		N07   , As5
	.byte	W08
	.byte		N15   , Gs5
	.byte	W16
	.byte		N07   , Fs5 , v104
	.byte	W02
	.byte	W06
	.byte		N15   , Fn5 , v116
	.byte	W16
	.byte		N07   , Fs5 , v100
	.byte	W08
	.byte		N15   , Gs5 , v112
	.byte	W16
	.byte		N07   , Fn5
	.byte	W08
	.byte		N32   , Ds5 , v104 , gtp3
	.byte	W42
	.byte	W16
	.byte		N01   , Cs5 , v068
	.byte	W02
	.byte		N03   , Ds5 , v112
	.byte	W04
	.byte		N07   , Fn5 , v120
	.byte	W24
	.byte		        Cs5 , v112
	.byte	W08
	.byte		N03   , Ds5 , v104
	.byte	W24
	.byte		N23   , Cn5 , v116
	.byte	W18
	.byte	PATT
	 ptrvalue	mus_qz_615_9_002
	.byte	W18
	.byte		N03   , Gs4 , v084
	.byte	W04
	.byte		        As4 , v120
	.byte	W24
	.byte		N07   , Cs5 , v112
	.byte	W08
	.byte		N11   , As4
	.byte	W16
	.byte		N07   , As5
	.byte	W08
	.byte		N15   , Gs5
	.byte	W16
	.byte		N07   , Fs5 , v104
	.byte	W02
	.byte	W06
	.byte		N15   , Gs5 , v116
	.byte	W16
	.byte		N07   , As5 , v100
	.byte	W08
	.byte		N15   , Cn6 , v112
	.byte	W16
	.byte		N07   , Cs6
	.byte	W08
	.byte		N36   , Ds6 , v104 , gtp3
	.byte	W42
	.byte	W06
	.byte		N32   , Cs5 , v116 , gtp3
	.byte	W40
	.byte		N03   , Fn5 , v120
	.byte	W24
	.byte		N28   , Cs5 , v112 , gtp1
	.byte	W24
	.byte	W02
	.byte	W06
	.byte		N36   , Cn5 , v116 , gtp1
	.byte	W36
	.byte	W01
	.byte		N04   , Cs5 , v104
	.byte	W03
	.byte		N05   , Ds5 , v112
	.byte	W22
	.byte		N30   , Cn5 , v120 , gtp1
	.byte	W28
	.byte	W06
	.byte		N36   , As4 , v116 , gtp1
	.byte	W36
	.byte	W02
	.byte		N04   , Cn5 , v112
	.byte	W03
	.byte		N05   , Cs5
	.byte	W22
	.byte		N30   , As4 , v116 , gtp1
	.byte	W24
	.byte	W03
	.byte	W06
	.byte		N07   , Cn5 , v120
	.byte	W24
	.byte		N21   , Gs4 , v116
	.byte	W24
	.byte		        As4 , v112
	.byte	W24
	.byte		        Cn5 , v116
	.byte	W18
	.byte	W06
	.byte		N32   , Cs5 , v108 , gtp3
	.byte	W36
	.byte		N06   , Ds5
	.byte	W04
	.byte		N05   , Fn5 , v120
	.byte	W22
	.byte		N28   , Cs5 , v112 , gtp1
	.byte	W28
	.byte	W06
	.byte		N36   , Cn5 , v116
	.byte	W36
	.byte		N06   , Cs5 , v100
	.byte	W03
	.byte		N05   , Ds5 , v112
	.byte	W24
	.byte	W01
	.byte		N07   , Cn5
	.byte	W08
	.byte		N09   , Ds5 , v104
	.byte	W07
	.byte		N07   , Cs5
	.byte	W08
	.byte		        Cn5 , v112
	.byte	W03
	.byte	W05
	.byte		N30   , As4 , v112 , gtp1
	.byte	W36
	.byte	W01
	.byte		N06   , As4 , v076
	.byte	W03
	.byte		N04   , Cn5 , v104
	.byte	W01
	.byte		N08   , Cs5 , v112
	.byte	W24
	.byte		N30   , As4 , v116 , gtp1
	.byte	W24
	.byte	W02
	.byte	W05
	.byte		N05   , Cn5 , v120
	.byte	W24
	.byte	W01
	.byte		N21   , Gs4 , v112
	.byte	W24
	.byte		N23   , As4 , v116
	.byte	W24
	.byte		N21   , Cn5 , v120
	.byte	W18
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_615_9_000
	.byte	PATT
	 ptrvalue	mus_qz_615_9_001
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_615_9_B1
	.byte	FINE

/********************** Track 10 **********************/

mus_qz_615_10:
	.byte		KEYSH , mus_qz_615_key+0
	.byte		VOICE , 8
	.byte		VOL   , 46*mus_qz_615_mvl/mxv
	.byte		PAN   , c_v-44
	.byte		PRIO  , 50
	.byte		VOL   , 41*mus_qz_615_mvl/mxv
	.byte		        41*mus_qz_615_mvl/mxv
	.byte		N56   , Fn3 , v076 , gtp3
	.byte	W48
	.byte		VOL   , 34*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        28*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        21*mus_qz_615_mvl/mxv
	.byte	W40
	.byte		        41*mus_qz_615_mvl/mxv
	.byte	W48
	.byte		        37*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        33*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        26*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        20*mus_qz_615_mvl/mxv
	.byte	W36
	.byte		        41*mus_qz_615_mvl/mxv
	.byte		N56   , Fn3 , v076 , gtp3
	.byte	W48
	.byte		VOL   , 35*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        29*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        22*mus_qz_615_mvl/mxv
	.byte	W40
	.byte		        41*mus_qz_615_mvl/mxv
	.byte	W48
	.byte		        36*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        31*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        26*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        22*mus_qz_615_mvl/mxv
	.byte	W32
	.byte		        39*mus_qz_615_mvl/mxv
	.byte	W04
mus_qz_615_10_B1:
	.byte		N42   , Fn3 , v076 , gtp1
	.byte	W48
	.byte		        Ds3 , v080 , gtp1
	.byte	W48
	.byte	PEND
	.byte		        Cs3 , v076 , gtp1
	.byte	W48
	.byte		        Cn3 , v080 , gtp1
	.byte	W48
	.byte		N60   , As2 , v076 , gtp3
	.byte	W96
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_615_10_B1
	.byte		N42   , Cs3 , v076 , gtp1
	.byte	W48
	.byte		N23   , Cn3
	.byte	W24
	.byte		        Fn3 , v080
	.byte	W24
	.byte		N66   , As2 , v076 , gtp1
	.byte	W96
	.byte	W96
	.byte		N56   , Cs3 , v084 , gtp3
	.byte	W48
	.byte		VOL   , 36*mus_qz_615_mvl/mxv
	.byte	W03
	.byte		        34*mus_qz_615_mvl/mxv
	.byte	W03
	.byte		        30*mus_qz_615_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_615_mvl/mxv
	.byte	W36
	.byte	W03
	.byte		        41*mus_qz_615_mvl/mxv
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		N56   , Cs3 , v088 , gtp3
	.byte	W48
	.byte		VOL   , 36*mus_qz_615_mvl/mxv
	.byte	W03
	.byte		        34*mus_qz_615_mvl/mxv
	.byte	W03
	.byte		        30*mus_qz_615_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_615_mvl/mxv
	.byte	W36
	.byte	W03
	.byte		        41*mus_qz_615_mvl/mxv
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		N44   , Fn3 , v060 , gtp3
	.byte	W96
	.byte		        Fs3 , v060 , gtp3
	.byte	W96
	.byte		N42   , Cs3 , v060 , gtp1
	.byte	W48
	.byte		        Fn3 , v068 , gtp1
	.byte	W48
	.byte		N10   , Ds3 , v060
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_615_10_B1
	.byte	FINE

/********************** Track 11 **********************/

mus_qz_615_11:
	.byte		KEYSH , mus_qz_615_key+0
	.byte		VOICE , 8
	.byte		VOL   , 10*mus_qz_615_mvl/mxv
	.byte		PAN   , c_v+56
	.byte		PRIO  , 36
	.byte		VOL   , 9*mus_qz_615_mvl/mxv
	.byte	W18
	.byte		        9*mus_qz_615_mvl/mxv
	.byte		N56   , Fn3 , v076 , gtp3
	.byte	W48
	.byte		VOL   , 7*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        6*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        4*mus_qz_615_mvl/mxv
	.byte	W22
	.byte	W18
	.byte		        9*mus_qz_615_mvl/mxv
	.byte	W48
	.byte		        8*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        7*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        5*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        4*mus_qz_615_mvl/mxv
	.byte	W18
	.byte	W18
	.byte		        9*mus_qz_615_mvl/mxv
	.byte		N56   , Fn3 , v076 , gtp3
	.byte	W48
	.byte		VOL   , 7*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        6*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        4*mus_qz_615_mvl/mxv
	.byte	W22
	.byte	W18
	.byte		        9*mus_qz_615_mvl/mxv
	.byte	W48
	.byte		        7*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        7*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        5*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		        4*mus_qz_615_mvl/mxv
	.byte	W18
mus_qz_615_11_B1:
	.byte	W14
	.byte		VOL   , 9*mus_qz_615_mvl/mxv
	.byte	W04
	.byte		N42   , Fn3 , v076 , gtp1
	.byte	W48
	.byte		        Ds3 , v080 , gtp1
	.byte	W30
	.byte	W18
	.byte		        Cs3 , v076 , gtp1
	.byte	W48
	.byte		        Cn3 , v080 , gtp1
	.byte	W30
	.byte	W18
	.byte		N60   , As2 , v076 , gtp3
	.byte	W78
	.byte	W96
	.byte	W18
	.byte		N42   , Fn3 , v076 , gtp1
	.byte	W48
	.byte		        Ds3 , v080 , gtp1
	.byte	W30
	.byte	W18
	.byte		        Cs3 , v076 , gtp1
	.byte	W48
	.byte		N23   , Cn3
	.byte	W24
	.byte		        Fn3 , v080
	.byte	W06
	.byte	W18
	.byte		N66   , As2 , v076 , gtp1
	.byte	W78
	.byte	W96
	.byte	W18
	.byte		N56   , Cs3 , v084 , gtp3
	.byte	W48
	.byte		VOL   , 7*mus_qz_615_mvl/mxv
	.byte	W03
	.byte		        7*mus_qz_615_mvl/mxv
	.byte	W03
	.byte		        6*mus_qz_615_mvl/mxv
	.byte	W03
	.byte		        5*mus_qz_615_mvl/mxv
	.byte	W21
	.byte	W18
	.byte		        9*mus_qz_615_mvl/mxv
	.byte	W78
	.byte	W96
	.byte	W96
	.byte	W18
	.byte		N56   , Cs3 , v088 , gtp3
	.byte	W48
	.byte		VOL   , 7*mus_qz_615_mvl/mxv
	.byte	W03
	.byte		        7*mus_qz_615_mvl/mxv
	.byte	W03
	.byte		        6*mus_qz_615_mvl/mxv
	.byte	W03
	.byte		        5*mus_qz_615_mvl/mxv
	.byte	W21
	.byte	W18
	.byte		        9*mus_qz_615_mvl/mxv
	.byte	W78
	.byte	W96
	.byte	W96
	.byte	W18
	.byte		N44   , Fn3 , v060 , gtp3
	.byte	W78
	.byte	W18
	.byte		        Fs3 , v060 , gtp3
	.byte	W78
	.byte	W18
	.byte		N42   , Cs3 , v060 , gtp1
	.byte	W48
	.byte		        Fn3 , v068 , gtp1
	.byte	W30
	.byte	W18
	.byte		N10   , Ds3 , v048
	.byte	W78
	.byte	GOTO
	 ptrvalue	mus_qz_615_11_B1
	.byte	FINE

/******************************************************/
	.p2align	2

mus_qz_615:
	.byte	11	/* NumTrks */
	.byte	0	/* NumBlks */
	.byte	mus_qz_615_pri	/* Priority */
	.byte	mus_qz_615_rev	/* Reverb. */
space64 4
	ptrvalue	mus_qz_615_grp

	ptrvalue	mus_qz_615_1
	ptrvalue	mus_qz_615_2
	ptrvalue	mus_qz_615_3
	ptrvalue	mus_qz_615_4
	ptrvalue	mus_qz_615_5
	ptrvalue	mus_qz_615_6
	ptrvalue	mus_qz_615_7
	ptrvalue	mus_qz_615_8
	ptrvalue	mus_qz_615_9
	ptrvalue	mus_qz_615_10
	ptrvalue	mus_qz_615_11

	.end
