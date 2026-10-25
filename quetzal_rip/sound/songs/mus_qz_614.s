	.include "asm/macros/bit_width.inc"

	.include "MPlayDef.s"

/* Moemon Quetzal song 614, disassembled from the ROM by tools/quetzal_rip/songs.py.
   "org" comments give the ROM address of the bytes that follow. */

	.equ	mus_qz_614_grp, voicegroup_qz_songs
	.equ	mus_qz_614_pri, 0
	.equ	mus_qz_614_rev, reverb_set+0
	.equ	mus_qz_614_mvl, 127
	.equ	mus_qz_614_key, 0
	.equ	mus_qz_614_tbs, 1
	.equ	mus_qz_614_exg, 1
	.equ	mus_qz_614_cmp, 1

	.section .rodata
	.global	mus_qz_614
	.p2align	2
/* org 0x0966FD44 */

/********************** Track  1 **********************/

mus_qz_614_1:
	.byte		KEYSH , mus_qz_614_key+0
	.byte		TEMPO , 120*mus_qz_614_tbs/2
	.byte		VOICE , 10
	.byte		PAN   , c_v-36
	.byte		VOL   , 48*mus_qz_614_mvl/mxv
	.byte		PRIO  , 64
	.byte	W12
	.byte	W96
	.byte	W96
	.byte		N32   , Cs4 , v100 , gtp3
	.byte	W12
	.byte		N04   , Gs3 , v084
	.byte	W12
	.byte		        Cn4 , v100
	.byte	W12
	.byte		N23   , Cs4 , v108
	.byte	W24
	.byte		        Ds4 , v100
	.byte	W24
	.byte		N05   , Fn4 , v108
	.byte	W11
	.byte		N01   , Gn4 , v080
	.byte	W01
mus_qz_614_1_000:
	.byte		N32   , Gs4 , v112 , gtp3
	.byte	W36
	.byte		N03   , Fn4 , v104
	.byte	W24
	.byte		N23   , Ds4 , v108
	.byte	W24
	.byte		N03   , Fn4 , v100
	.byte	W12
	.byte	PEND
	.byte		N80   , Cs4 , v100 , gtp3
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
mus_qz_614_1_B1:
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W92
	.byte	W03
	.byte		N01   , Dn4 , v088
	.byte	W01
	.byte		N10   , Ds4 , v108
	.byte	W12
	.byte		N11   , Cs4 , v100
	.byte	W12
	.byte		N05   , Cn4 , v108
	.byte	W12
	.byte		N11   , Gs4 , v116
	.byte	W24
	.byte		N23   , Fs4 , v104
	.byte	W24
	.byte		N11   , Fn4
	.byte	W12
	.byte		        Ds4 , v112
	.byte	W12
	.byte		N05   , Fn4 , v104
	.byte	W12
	.byte		        Ds4 , v116
	.byte	W12
	.byte		N23   , Cs4 , v104
	.byte	W60
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		N11   , Cs4 , v100
	.byte	W12
	.byte		N04   , Gs3 , v084
	.byte	W12
	.byte		        Cn4 , v100
	.byte	W12
	.byte		N23   , Cs4 , v108
	.byte	W24
	.byte		        Ds4 , v100
	.byte	W24
	.byte		N05   , Fn4 , v108
	.byte	W11
	.byte		N01   , Gn4 , v080
	.byte	W01
	.byte	PATT
	 ptrvalue	mus_qz_614_1_000
	.byte		N80   , Cs4 , v100 , gtp3
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_614_1_B1
	.byte	FINE

/********************** Track  2 **********************/

mus_qz_614_2:
	.byte		KEYSH , mus_qz_614_key+0
	.byte		VOICE , 23
	.byte		PAN   , c_v-49
	.byte		VOL   , 44*mus_qz_614_mvl/mxv
	.byte		PRIO  , 62
	.byte		N02   , Cn3 , v036
	.byte		N02   , Gs3 , v044
	.byte	W03
	.byte		        Cs3 , v032
	.byte		N02   , As3 , v036
	.byte	W03
	.byte		        Ds3 , v048
	.byte		N02   , Cn4 , v052
	.byte	W03
	.byte		        Fn3 , v064
	.byte		N02   , Cs4 , v068
	.byte	W03
	.byte		N05   , Fn3 , v072
	.byte		N05   , Ds4 , v100
	.byte	W12
	.byte		        Fn3 , v036
	.byte		N05   , Ds4 , v060
	.byte	W12
	.byte		        Fn3 , v020
	.byte		N05   , Ds4 , v040
	.byte	W12
	.byte		        Fn3 , v072
	.byte		N05   , Cs4 , v100
	.byte	W12
	.byte		        Fn3 , v032
	.byte		N05   , Cs4 , v060
	.byte	W12
	.byte		        Fn3 , v020
	.byte		N05   , Cs4 , v036
	.byte	W12
	.byte		        Fn3 , v012
	.byte		N05   , Cs4 , v020
	.byte	W12
	.byte		        Fn3 , v012
	.byte		N05   , Cs4
	.byte	W12
	.byte		        Fn3 , v072
	.byte		N05   , Cn4 , v100
	.byte	W12
	.byte		        Fn3 , v032
	.byte		N05   , Cn4 , v056
	.byte	W12
	.byte		        Fn3 , v020
	.byte		N05   , Cn4 , v040
	.byte	W12
	.byte		        Fn3 , v072
	.byte		N05   , Cs4 , v104
	.byte	W12
	.byte		        Fn3 , v032
	.byte		N05   , Cs4 , v056
	.byte	W12
	.byte		        Fn3 , v020
	.byte		N05   , Cs4 , v036
	.byte	W12
	.byte		N23   , Gs3 , v072
	.byte		N23   , Fn4 , v088
	.byte	W24
mus_qz_614_2_000:
	.byte		N05   , Gs3 , v088
	.byte		N05   , Ds4
	.byte	W12
	.byte		        Gs3 , v044
	.byte		N05   , Ds4
	.byte	W12
	.byte		        Gs3 , v032
	.byte		N05   , Ds4
	.byte	W12
	.byte		        Gs3 , v088
	.byte		N05   , Cs4
	.byte	W12
	.byte		        Gs3 , v040
	.byte		N05   , Cs4 , v044
	.byte	W12
	.byte		        Gs3 , v032
	.byte		N05   , Cs4
	.byte	W12
	.byte		        Gs3 , v080
	.byte		N05   , Fn4
	.byte	W12
	.byte		        Gs3 , v048
	.byte		N05   , Fn4
	.byte	W12
	.byte	PEND
mus_qz_614_2_001:
	.byte		N05   , Gs3 , v088
	.byte		N05   , Ds4
	.byte	W12
	.byte		        Gs3 , v044
	.byte		N05   , Ds4 , v040
	.byte	W12
	.byte		        Gs3 , v032
	.byte		N05   , Ds4
	.byte	W12
	.byte		        An3 , v088
	.byte		N05   , Fn4
	.byte	W12
	.byte		        An3 , v040
	.byte		N05   , Fn4 , v044
	.byte	W12
	.byte		        An3 , v032
	.byte		N05   , Fn4
	.byte	W12
	.byte		        Cn4 , v080
	.byte		N05   , An4 , v084
	.byte	W12
	.byte		        Cn4 , v044
	.byte		N05   , An4
	.byte	W12
	.byte	PEND
mus_qz_614_2_002:
	.byte		N05   , Fn3 , v088
	.byte		N05   , Cn4
	.byte	W12
	.byte		        Fn3 , v036
	.byte		N05   , Cn4 , v044
	.byte	W12
	.byte		        Fn3 , v032
	.byte		N05   , Cn4
	.byte	W12
	.byte		        Fn3 , v088
	.byte		N05   , Cs4
	.byte	W12
	.byte		        Fn3 , v044
	.byte		N05   , Cs4
	.byte	W12
	.byte		        Fn3 , v032
	.byte		N05   , Cs4
	.byte	W12
	.byte		        Fn3 , v028
	.byte		N05   , Cs4
	.byte	W12
	.byte		        Fn3 , v020
	.byte		N05   , Cs4
	.byte	W12
	.byte	PEND
mus_qz_614_2_003:
	.byte		N05   , Fn3 , v088
	.byte		N05   , Ds4
	.byte	W12
	.byte		        Fn3 , v036
	.byte		N05   , Ds4 , v044
	.byte	W12
	.byte		        Fn3 , v032
	.byte		N05   , Ds4
	.byte	W12
	.byte		        Fn3 , v088
	.byte		N05   , Cs4
	.byte	W12
	.byte		        Fn3 , v036
	.byte		N05   , Cs4 , v044
	.byte	W12
	.byte		        Fn3 , v032
	.byte		N05   , Cs4
	.byte	W12
	.byte		        Fn3 , v084
	.byte		N05   , Cs4
	.byte	W12
	.byte		        Fn3 , v048
	.byte		N05   , Cs4
	.byte	W12
	.byte	PEND
mus_qz_614_2_004:
	.byte		N05   , As3 , v088
	.byte		N05   , Fs4 , v092
	.byte	W12
	.byte		        As3 , v040
	.byte		N05   , Fs4 , v048
	.byte	W12
	.byte		        As3 , v032
	.byte		N05   , Fs4
	.byte	W12
	.byte		        As3 , v088
	.byte		N05   , Fs4 , v092
	.byte	W12
	.byte		        As3 , v036
	.byte		N05   , Fs4 , v048
	.byte	W12
	.byte		        As3 , v032
	.byte		N05   , Fs4
	.byte	W12
	.byte		        As3 , v020
	.byte		N05   , Fs4
	.byte	W12
	.byte		        As3
	.byte		N05   , Fs4
	.byte	W12
	.byte	PEND
mus_qz_614_2_005:
	.byte		N05   , Gn3 , v088
	.byte		N05   , En4 , v092
	.byte	W12
	.byte		        Gn3 , v040
	.byte		N05   , En4 , v048
	.byte	W12
	.byte		        Gn3 , v032
	.byte		N05   , En4
	.byte	W12
	.byte		        Gn3 , v088
	.byte		N05   , Ds4 , v092
	.byte	W12
	.byte		        Gn3 , v044
	.byte		N05   , Ds4 , v048
	.byte	W12
	.byte		        Gn3 , v032
	.byte		N05   , Ds4
	.byte	W12
	.byte		        Gn3 , v088
	.byte		N05   , Cs4 , v092
	.byte	W12
	.byte		        Gn3 , v028
	.byte		N05   , Cs4 , v048
	.byte	W12
	.byte	PEND
mus_qz_614_2_006:
	.byte		N05   , Ds3 , v088
	.byte		N05   , Cn4
	.byte	W12
	.byte		        Ds3 , v044
	.byte		N05   , Cn4 , v048
	.byte	W12
	.byte		        Ds3 , v032
	.byte		N05   , Cn4
	.byte	W12
	.byte		        Cs3 , v088
	.byte		N05   , As3 , v092
	.byte	W12
	.byte		        Cs3 , v040
	.byte		N05   , As3 , v048
	.byte	W12
	.byte		        Cs3 , v032
	.byte		N05   , As3
	.byte	W12
	.byte		        Gs3 , v088
	.byte		N05   , Cs4 , v092
	.byte	W12
	.byte		        Gs3 , v036
	.byte		N05   , Cs4 , v044
	.byte	W12
	.byte	PEND
mus_qz_614_2_007:
	.byte		N11   , Cn3 , v072
	.byte		N11   , Gs3 , v088
	.byte	W12
	.byte		N04   , Cs3 , v060
	.byte		N04   , As3 , v076
	.byte	W24
	.byte		N23   , Ds3 , v072
	.byte		N23   , Cn4 , v088
	.byte	W24
	.byte		N02   , Dn4 , v084
	.byte		N02   , Gn4 , v088
	.byte	W03
	.byte		N08   , Ds4 , v092
	.byte		N08   , Gs4 , v096
	.byte	W09
	.byte		N11   , Cn4 , v088
	.byte		N11   , Fs4 , v092
	.byte	W12
	.byte		N04   , Gs3 , v080
	.byte		N04   , Ds4 , v084
	.byte	W12
	.byte	PEND
mus_qz_614_2_B1:
	.byte		N05   , Gs3 , v088
	.byte		N05   , Fn4
	.byte	W12
	.byte		        Gs3 , v040
	.byte		N05   , Fn4
	.byte	W12
	.byte		        Gs3 , v024
	.byte		N05   , Fn4
	.byte	W12
	.byte		        Fn3 , v084
	.byte		N05   , Ds4
	.byte	W12
	.byte		        Fn3 , v048
	.byte		N05   , Ds4
	.byte	W12
	.byte		        Fn3 , v028
	.byte		N05   , Ds4
	.byte	W12
	.byte		        Gs3 , v080
	.byte		N05   , Fn4
	.byte	W12
	.byte		        Gs3 , v044
	.byte		N05   , Fn4 , v048
	.byte	W12
	.byte		        Cn4 , v076
	.byte		N05   , As4
	.byte	W12
	.byte		        Cn4 , v020
	.byte		N05   , As4
	.byte	W12
	.byte		        Cn4 , v024
	.byte		N05   , As4
	.byte	W12
	.byte		        Cn4 , v076
	.byte		N05   , An4
	.byte	W12
	.byte		        Cn4 , v036
	.byte		N05   , An4 , v032
	.byte	W12
	.byte		        Cn4 , v024
	.byte		N05   , An4
	.byte	W12
	.byte		        An3 , v076
	.byte		N05   , Fn4
	.byte	W12
	.byte		        An3 , v032
	.byte		N05   , Fn4 , v012
	.byte	W12
	.byte		        Fn3 , v076
	.byte		N05   , Cn4
	.byte	W12
	.byte		        Fn3 , v020
	.byte		N05   , Cn4 , v036
	.byte	W12
	.byte		        Fn3 , v024
	.byte		N05   , Cn4
	.byte	W12
	.byte		        Fn3 , v076
	.byte		N05   , Cs4
	.byte	W12
	.byte		        Fn3 , v020
	.byte		N05   , Cs4 , v032
	.byte	W12
	.byte		        Fn3 , v024
	.byte		N05   , Cs4
	.byte	W12
	.byte		        Fn3 , v020
	.byte		N05   , Cs4 , v012
	.byte	W12
	.byte		        Fn3
	.byte		N05   , Cs4
	.byte	W12
	.byte		        Fn3 , v076
	.byte		N05   , Ds4
	.byte	W12
	.byte		        Fn3 , v036
	.byte		N05   , Ds4
	.byte	W12
	.byte		        Fn3 , v024
	.byte		N05   , Ds4
	.byte	W12
	.byte		        Gn3 , v076
	.byte		N05   , Cs4
	.byte	W12
	.byte		        Gn3 , v032
	.byte		N05   , Cs4
	.byte	W12
	.byte		        Gn3 , v024
	.byte		N05   , Cs4
	.byte	W12
	.byte		        Gn3 , v012
	.byte		N05   , Cs4
	.byte	W24
	.byte		        As3 , v076
	.byte		N05   , Fn4
	.byte	W12
	.byte		        As3 , v036
	.byte		N05   , Fn4 , v032
	.byte	W12
	.byte		        As3 , v024
	.byte		N05   , Fn4
	.byte	W12
	.byte		        As3 , v076
	.byte		N05   , Fs4
	.byte	W12
	.byte		        As3 , v036
	.byte		N05   , Fs4
	.byte	W12
	.byte		        As3 , v024
	.byte		N05   , Fs4
	.byte	W12
	.byte		        As3 , v012
	.byte		N05   , Fs4
	.byte	W12
	.byte		        As3
	.byte		N05   , Fs4
	.byte	W12
	.byte		        As3 , v076
	.byte		N05   , Fn4
	.byte	W12
	.byte		        As3 , v020
	.byte		N05   , Fn4 , v032
	.byte	W12
	.byte		        As3 , v024
	.byte		N05   , Fn4
	.byte	W12
	.byte		        Gn3 , v076
	.byte		N05   , Ds4
	.byte	W12
	.byte		        Gn3 , v036
	.byte		N05   , Ds4 , v032
	.byte	W12
	.byte		        Gn3 , v024
	.byte		N05   , Ds4
	.byte	W12
	.byte		        Gn3 , v064
	.byte		N05   , Cs4 , v056
	.byte	W12
	.byte		        Gn3 , v020
	.byte		N05   , Cs4
	.byte	W12
	.byte		        Ds3 , v076
	.byte		N05   , Cn4
	.byte	W12
	.byte		        Ds3 , v032
	.byte		N05   , Cn4
	.byte	W12
	.byte		        Ds3 , v024
	.byte		N05   , Cn4
	.byte	W12
	.byte		        Ds3 , v076
	.byte		N05   , As3
	.byte	W12
	.byte		        Ds3 , v036
	.byte		N05   , As3 , v032
	.byte	W12
	.byte		        Ds3 , v024
	.byte		N05   , As3
	.byte	W12
	.byte		        Fs3 , v076
	.byte		N05   , Cs4
	.byte	W12
	.byte		        Fs3 , v032
	.byte		N05   , Cs4
	.byte	W12
	.byte		N11   , Ds3 , v088
	.byte	W12
	.byte		        Cn3 , v076
	.byte	W21
	.byte		N02   , Bn3 , v084
	.byte	W03
	.byte		N32   , Cn4 , v092 , gtp3
	.byte	W36
	.byte		N11   , As3 , v076
	.byte	W12
	.byte		        Fs3 , v072
	.byte	W12
	.byte		        Ds4 , v104
	.byte	W12
	.byte		        Cs4 , v096
	.byte	W12
	.byte		N05   , Cn4 , v088
	.byte	W12
	.byte		N11   , Gs4 , v108
	.byte	W24
	.byte		N23   , Fs4
	.byte	W24
	.byte		N11   , Fn4 , v092
	.byte	W12
	.byte		        Ds4 , v104
	.byte	W12
	.byte		N05   , Fn4 , v096
	.byte	W12
	.byte		        Ds4 , v108
	.byte	W12
	.byte		N23   , Cs4 , v092
	.byte	W36
	.byte		N22   , Fn3 , v104
	.byte	W24
	.byte	W96
	.byte	W96
	.byte		N11   , Cs4 , v088
	.byte	W12
	.byte		        As3 , v076
	.byte	W12
	.byte		        Fs3 , v084
	.byte	W12
	.byte		N23   , Cs3 , v076
	.byte	W24
	.byte		N11   , Fs3 , v084
	.byte	W24
	.byte		        As3 , v076
	.byte	W12
	.byte		        Cs4 , v092
	.byte	W12
	.byte		        As3 , v076
	.byte	W12
	.byte		        Gn3 , v084
	.byte	W12
	.byte		N23   , Cs3 , v088
	.byte	W24
	.byte		N11   , Gn3 , v084
	.byte	W24
	.byte		        Cs4 , v080
	.byte	W12
	.byte		        Ds4 , v088
	.byte	W12
	.byte		        Cs4 , v084
	.byte	W12
	.byte		        Gs3 , v088
	.byte	W12
	.byte		        Ds3 , v080
	.byte	W12
	.byte		        Cs3 , v092
	.byte	W12
	.byte		        Gs2 , v084
	.byte	W12
	.byte		        Gs3 , v092
	.byte	W12
	.byte		        Cs3 , v084
	.byte	W12
	.byte		VOL   , 6*mus_qz_614_mvl/mxv
	.byte		N80   , Gs2 , v100 , gtp3
	.byte	W06
	.byte		VOL   , 9*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        11*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        14*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        17*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        21*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        25*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        29*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        33*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        39*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        43*mus_qz_614_mvl/mxv
	.byte	W36
	.byte	PATT
	 ptrvalue	mus_qz_614_2_000
	.byte	PATT
	 ptrvalue	mus_qz_614_2_001
	.byte	PATT
	 ptrvalue	mus_qz_614_2_002
	.byte	PATT
	 ptrvalue	mus_qz_614_2_003
	.byte	PATT
	 ptrvalue	mus_qz_614_2_004
	.byte	PATT
	 ptrvalue	mus_qz_614_2_005
	.byte	PATT
	 ptrvalue	mus_qz_614_2_006
	.byte	PATT
	 ptrvalue	mus_qz_614_2_007
	.byte	GOTO
	 ptrvalue	mus_qz_614_2_B1
	.byte	FINE

/********************** Track  3 **********************/

mus_qz_614_3:
	.byte		KEYSH , mus_qz_614_key+0
	.byte		VOICE , 35
	.byte		PAN   , c_v+6
	.byte		VOL   , 51*mus_qz_614_mvl/mxv
	.byte		BENDR , 6
	.byte		PRIO  , 64
	.byte		MOD   , 2
	.byte		LFOS  , 28
	.byte		LFODL , 10
	.byte		BEND  , c_v+2
	.byte	W12
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
mus_qz_614_3_000:
	.byte	W72
	.byte		N11   , As3 , v120
	.byte	W12
	.byte		N05   , Cs4 , v116
	.byte	W10
	.byte		N01   , Fn4 , v064
	.byte	W02
	.byte	PEND
mus_qz_614_3_001:
	.byte		N44   , Fs4 , v127 , gtp3
	.byte	W36
	.byte		VOL   , 47*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        41*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        52*mus_qz_614_mvl/mxv
	.byte	W12
	.byte		N11   , Fs4 , v120
	.byte	W12
	.byte		        Fn4 , v112
	.byte	W12
	.byte		N05   , Ds4 , v100
	.byte	W09
	.byte		N02   , Cn4 , v064
	.byte	W03
	.byte	PEND
mus_qz_614_3_002:
	.byte		N32   , Cs4 , v120 , gtp3
	.byte	W24
	.byte		VOL   , 47*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        41*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        52*mus_qz_614_mvl/mxv
	.byte		N05   , Cn4 , v116
	.byte	W24
	.byte		N23   , Cn4 , v120
	.byte	W24
	.byte		N05   , Cs4 , v112
	.byte	W09
	.byte		N02   , Dn4 , v060
	.byte	W03
	.byte	PEND
mus_qz_614_3_003:
	.byte		N68   , Ds4 , v116 , gtp3
	.byte	W60
	.byte		VOL   , 47*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        41*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        52*mus_qz_614_mvl/mxv
	.byte		N23   , Cs4 , v120
	.byte	W24
	.byte	PEND
mus_qz_614_3_004:
	.byte		N11   , Cn4 , v120
	.byte	W12
	.byte		N05   , Cs4
	.byte	W24
	.byte		N32   , Ds4 , v120 , gtp3
	.byte	W24
	.byte		VOL   , 47*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        41*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        52*mus_qz_614_mvl/mxv
	.byte	W12
	.byte		N02   , Gs4 , v088
	.byte	W06
	.byte		N05   , Gs4 , v080
	.byte	W06
	.byte	PEND
mus_qz_614_3_B1:
	.byte		N11   , Cs5 , v100
	.byte	W12
	.byte		N05   , Gs4 , v088
	.byte	W12
	.byte		        Cn5 , v096
	.byte	W12
	.byte		N23   , Cs5 , v100
	.byte	W24
	.byte		        Ds5 , v092
	.byte	W24
	.byte		N05   , Fn5 , v088
	.byte	W09
	.byte		N02   , Gn5 , v080
	.byte	W03
	.byte		N23   , Gs5 , v100
	.byte	W24
	.byte		N05   , Fn5 , v088
	.byte	W12
	.byte		N23   , An5 , v104
	.byte	W24
	.byte		N05   , Fn5 , v096
	.byte	W12
	.byte		N11   , Ds5 , v100
	.byte	W12
	.byte		N05   , Fn5 , v092
	.byte	W09
	.byte		N02   , Cn5 , v072
	.byte	W03
	.byte		N80   , Cs5 , v092 , gtp3
	.byte	W54
	.byte		VOL   , 50*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        48*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        44*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        41*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        37*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        35*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        32*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        29*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        26*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        24*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        21*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        18*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        17*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        14*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        13*mus_qz_614_mvl/mxv
	.byte	W24
	.byte		        51*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		N11   , As3
	.byte	W12
	.byte		N05   , Cs4 , v088
	.byte	W12
	.byte		N52   , Fs4 , v104 , gtp1
	.byte	W30
	.byte		VOL   , 48*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        44*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        40*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        36*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        32*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        51*mus_qz_614_mvl/mxv
	.byte		N11   , Fs4 , v096
	.byte	W12
	.byte		        Fn4 , v088
	.byte	W12
	.byte		N05   , Ds4 , v096
	.byte	W12
	.byte		N32   , Cs4 , v096 , gtp3
	.byte	W24
	.byte		VOL   , 47*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        41*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        52*mus_qz_614_mvl/mxv
	.byte		N11   , Cn4 , v092
	.byte	W24
	.byte	N23
	.byte	W24
	.byte		N05   , Cs4
	.byte	W09
	.byte		N02   , Dn4
	.byte	W03
	.byte		N56   , Ds4 , v092 , gtp3
	.byte	W36
	.byte		VOL   , 48*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        44*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        40*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        36*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        32*mus_qz_614_mvl/mxv
	.byte	W24
	.byte		        51*mus_qz_614_mvl/mxv
	.byte		N11   , Cs4 , v100
	.byte	W12
	.byte		        Cn4 , v108
	.byte	W12
	.byte		        Gs3
	.byte	W21
	.byte		N02   , Gn4
	.byte	W03
	.byte		N32   , Gs4 , v108 , gtp3
	.byte	W12
	.byte		VOL   , 48*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        44*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        40*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        36*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        51*mus_qz_614_mvl/mxv
	.byte		N11   , Fs4
	.byte	W12
	.byte		        Fn4
	.byte	W12
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		        Fn4 , v100
	.byte	W12
	.byte		        Ds4
	.byte	W12
	.byte		        Cs4
	.byte	W12
	.byte		N23   , Fn4
	.byte	W24
	.byte		N11   , Ds4
	.byte	W24
	.byte		        Cs4
	.byte	W12
	.byte		        Fn4
	.byte	W12
	.byte		        Ds4
	.byte	W12
	.byte		        Cs4
	.byte	W12
	.byte		N23   , Fn4
	.byte	W24
	.byte		N11   , As4
	.byte	W24
	.byte		        Cn5
	.byte	W12
	.byte		N64   , Gs4 , v100 , gtp1
	.byte	W42
	.byte		VOL   , 48*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        44*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        40*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        36*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        32*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        51*mus_qz_614_mvl/mxv
	.byte		N23   , Ds4
	.byte	W24
	.byte		VOL   , 18*mus_qz_614_mvl/mxv
	.byte		        19*mus_qz_614_mvl/mxv
	.byte		N80   , Cn4 , v124 , gtp3
	.byte	W06
	.byte		VOL   , 23*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        25*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        29*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        33*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        36*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        39*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        42*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        46*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        50*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        54*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        48*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        44*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        40*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        36*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        31*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        50*mus_qz_614_mvl/mxv
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_614_3_000
	.byte	PATT
	 ptrvalue	mus_qz_614_3_001
	.byte	PATT
	 ptrvalue	mus_qz_614_3_002
	.byte	PATT
	 ptrvalue	mus_qz_614_3_003
	.byte	PATT
	 ptrvalue	mus_qz_614_3_004
	.byte	GOTO
	 ptrvalue	mus_qz_614_3_B1
	.byte	FINE

/********************** Track  4 **********************/

mus_qz_614_4:
	.byte		KEYSH , mus_qz_614_key+0
	.byte		VOICE , 34
	.byte		PAN   , c_v+0
	.byte		VOL   , 55*mus_qz_614_mvl/mxv
	.byte		BENDR , 6
	.byte		PRIO  , 64
	.byte		BEND  , c_v+1
	.byte	W12
	.byte	W96
	.byte	W72
	.byte		N19   , Gs3 , v104
	.byte	W21
	.byte		N02   , Cn4 , v084
	.byte	W03
mus_qz_614_4_000:
	.byte		N11   , Cs4 , v104
	.byte	W12
	.byte		N05   , Gs3 , v088
	.byte	W12
	.byte		        Cn4 , v104
	.byte	W12
	.byte		N20   , Cs4 , v112
	.byte	W24
	.byte		N23   , Ds4
	.byte	W24
	.byte		N05   , Fn4 , v100
	.byte	W12
	.byte	PEND
mus_qz_614_4_001:
	.byte		N32   , Gs4 , v112 , gtp3
	.byte	W18
	.byte		VOL   , 50*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        43*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        36*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        51*mus_qz_614_mvl/mxv
	.byte		N05   , Fn4 , v100
	.byte	W24
	.byte		N23   , Ds4 , v112
	.byte	W24
	.byte		N05   , Fn4 , v104
	.byte	W12
	.byte	PEND
mus_qz_614_4_002:
	.byte		N92   , Cs4 , v112 , gtp3
	.byte	W42
	.byte		VOL   , 48*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        41*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        35*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        28*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        24*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        18*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        13*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        9*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        6*mus_qz_614_mvl/mxv
	.byte	W06
	.byte	PEND
	.byte	W48
	.byte		        51*mus_qz_614_mvl/mxv
	.byte	W48
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W84
	.byte		N02   , Gs3 , v100
	.byte	W06
	.byte		N04   , Gs3 , v092
	.byte	W06
mus_qz_614_4_B1:
	.byte		N11   , Cs4 , v104
	.byte	W12
	.byte		N05   , Gs3 , v092
	.byte	W12
	.byte		        Cn4 , v100
	.byte	W12
	.byte		N23   , Cs4 , v108
	.byte	W24
	.byte		        Ds4 , v100
	.byte	W24
	.byte		N05   , Fn4 , v092
	.byte	W09
	.byte		N02   , Gn4
	.byte	W03
	.byte		N23   , Gs4 , v112
	.byte	W24
	.byte		N05   , Fn4 , v100
	.byte	W12
	.byte		N22   , An4 , v112
	.byte	W24
	.byte		N04   , Fn4 , v104
	.byte	W12
	.byte		N11   , Ds4 , v112
	.byte	W12
	.byte		N05   , Fn4 , v100
	.byte	W09
	.byte		N02   , Cn4 , v088
	.byte	W03
	.byte		N84   , Cs4 , v108 , gtp1
	.byte	W48
	.byte		VOL   , 48*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        41*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        35*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        28*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        24*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        18*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        13*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        9*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        6*mus_qz_614_mvl/mxv
	.byte	W72
	.byte		PAN   , c_v+19
	.byte		VOL   , 51*mus_qz_614_mvl/mxv
	.byte		N11   , As3 , v100
	.byte	W12
	.byte		N05   , Cs4 , v092
	.byte	W12
	.byte		N52   , Fs4 , v112 , gtp1
	.byte	W36
	.byte		VOL   , 50*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        47*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        43*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        50*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		N11   , Fs4 , v108
	.byte	W12
	.byte		        Fn4 , v096
	.byte	W12
	.byte		N05   , Ds4
	.byte	W12
	.byte		N32   , Cs4 , v112 , gtp3
	.byte	W36
	.byte		N05   , Cn4 , v100
	.byte	W24
	.byte		N23   , Cn4 , v112
	.byte	W24
	.byte		N05   , Cs4 , v100
	.byte	W09
	.byte		N02   , Dn4 , v080
	.byte	W03
	.byte		N68   , Ds4 , v108 , gtp3
	.byte	W36
	.byte		VOL   , 48*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        44*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        40*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        36*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        32*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        28*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        25*mus_qz_614_mvl/mxv
	.byte	W12
	.byte		        51*mus_qz_614_mvl/mxv
	.byte		N11   , As3 , v088
	.byte	W12
	.byte		        Gs3 , v084
	.byte	W12
	.byte		        Ds3 , v072
	.byte	W21
	.byte		N02   , As3 , v060
	.byte	W03
	.byte		N32   , Cn4 , v084 , gtp3
	.byte	W36
	.byte		N11   , As3 , v088
	.byte	W12
	.byte		        Fs3 , v072
	.byte	W12
	.byte	W96
	.byte	W96
	.byte		        Cn3 , v104
	.byte		N11   , Ds3 , v116
	.byte	W12
	.byte		        As2 , v092
	.byte		N11   , Cs3 , v104
	.byte	W12
	.byte		N03   , Gs2
	.byte		N03   , Cn3 , v116
	.byte	W12
	.byte		N11   , Cn3 , v108
	.byte		N11   , Gs3 , v120
	.byte	W24
	.byte		N23   , Fn3 , v092
	.byte		N23   , An3 , v108
	.byte	W24
	.byte		N11   , Fn3
	.byte		N11   , Cn4 , v120
	.byte	W12
	.byte		        Fn3 , v100
	.byte		N11   , Cn4 , v112
	.byte	W12
	.byte		N04   , Fn3 , v100
	.byte		N04   , Cs4 , v112
	.byte	W12
	.byte		        Fn3 , v100
	.byte		N04   , Cn4 , v112
	.byte	W12
	.byte		N23   , Cs3 , v100
	.byte		N23   , Fn3 , v112
	.byte	W24
	.byte		        As2 , v092
	.byte		N23   , Ds3 , v104
	.byte	W24
	.byte		N10   , As2 , v100
	.byte		N10   , Cs3 , v112
	.byte	W12
	.byte		N11   , Fn4 , v108
	.byte	W12
	.byte		        Ds4 , v096
	.byte	W12
	.byte		        Cs4 , v100
	.byte	W12
	.byte		N23   , Fn4 , v108
	.byte	W24
	.byte		N11   , Ds4 , v100
	.byte	W24
	.byte		        Cs4 , v092
	.byte	W12
	.byte		        Fn4 , v104
	.byte	W12
	.byte		        Ds4 , v096
	.byte	W12
	.byte		        Cs4 , v104
	.byte	W12
	.byte		N23   , Fn4
	.byte	W24
	.byte		N11   , As4 , v096
	.byte	W24
	.byte		        Cn5 , v092
	.byte	W12
	.byte		N68   , Gs4 , v104 , gtp3
	.byte	W72
	.byte		N23   , Ds4 , v092
	.byte	W24
	.byte		VOL   , 7*mus_qz_614_mvl/mxv
	.byte		N76   , Cn4 , v104 , gtp1
	.byte	W06
	.byte		VOL   , 10*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        13*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        17*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        20*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        25*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        29*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        34*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        39*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        44*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        50*mus_qz_614_mvl/mxv
	.byte	W24
	.byte		N05   , Gs3 , v100
	.byte	W06
	.byte		        Cn4
	.byte	W06
	.byte	PATT
	 ptrvalue	mus_qz_614_4_000
	.byte	PATT
	 ptrvalue	mus_qz_614_4_001
	.byte	PATT
	 ptrvalue	mus_qz_614_4_002
	.byte	W48
	.byte		VOL   , 51*mus_qz_614_mvl/mxv
	.byte	W48
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W84
	.byte		N02   , Gs3 , v092
	.byte	W06
	.byte		N04   , Gs3 , v084
	.byte	W06
	.byte	GOTO
	 ptrvalue	mus_qz_614_4_B1
	.byte	FINE

/********************** Track  5 **********************/

mus_qz_614_5:
	.byte		KEYSH , mus_qz_614_key+0
	.byte		VOICE , 14
	.byte		PAN   , c_v+41
	.byte		VOL   , 51*mus_qz_614_mvl/mxv
	.byte		BENDR , 6
	.byte		PRIO  , 56
	.byte	W12
	.byte		N40   , Cs2 , v104 , gtp1
	.byte	W12
	.byte		N32   , Gs2 , v084 , gtp2
	.byte	W12
	.byte		N28   , Cs3 , v096 , gtp1
	.byte	W12
	.byte		        Fn3 , v088 , gtp1
	.byte	W24
	.byte		N32   , Gs2 , v092 , gtp2
	.byte	W12
	.byte		N22   , Cs3 , v088
	.byte	W12
	.byte		N10   , Fn3 , v096
	.byte	W12
	.byte		N32   , Cs2 , v088 , gtp3
	.byte		N16   , Ds3 , v092
	.byte	W12
	.byte		N23   , Fn2 , v084
	.byte	W12
	.byte		N11   , Ds3 , v096
	.byte	W12
	.byte		N23   , Cn3 , v104
	.byte	W03
	.byte		BEND  , c_v+2
	.byte	W02
	.byte		        c_v+6
	.byte	W01
	.byte		        c_v+10
	.byte	W18
	.byte		        c_v+0
	.byte		N28   , Fn2 , v100 , gtp1
	.byte	W12
	.byte		N22   , Gs2 , v084
	.byte	W12
	.byte		N10   , Cs3 , v100
	.byte	W12
mus_qz_614_5_000:
	.byte		N32   , Cs2 , v104 , gtp3
	.byte	W12
	.byte		        Fn2 , v092 , gtp2
	.byte	W12
	.byte		N28   , Gs2 , v096
	.byte	W12
	.byte		N22   , Cs3 , v092
	.byte	W24
	.byte		N28   , Fn2 , v096 , gtp1
	.byte	W12
	.byte		N17   , Gs2 , v088
	.byte	W12
	.byte		N10   , Cs3 , v100
	.byte	W12
	.byte	PEND
mus_qz_614_5_001:
	.byte		N92   , Fn1 , v104 , gtp2
	.byte	W12
	.byte		N32   , Fn2 , v088 , gtp2
	.byte	W12
	.byte		N28   , Cn3 , v092 , gtp1
	.byte	W12
	.byte		N22   , Ds3 , v088
	.byte	W24
	.byte		        An2 , v100
	.byte	W12
	.byte		N17   , Cn3 , v092
	.byte	W12
	.byte		N10   , Ds3 , v100
	.byte	W12
	.byte	PEND
mus_qz_614_5_002:
	.byte		N28   , As1 , v100
	.byte		N10   , Fn2 , v092
	.byte	W01
	.byte		        Cs3
	.byte	W11
	.byte		N16   , Fn2 , v088
	.byte	W12
	.byte		N04   , Cs3 , v096
	.byte	W12
	.byte		N44   , An1 , v100 , gtp2
	.byte	W01
	.byte		N22   , Fn2 , v092
	.byte		N22   , An2
	.byte	W01
	.byte		        Cs3 , v100
	.byte	W22
	.byte		        Fn2 , v088
	.byte	W12
	.byte		N10   , An2 , v096
	.byte	W12
	.byte		N44   , Gs1 , v100 , gtp2
	.byte	W01
	.byte		N17   , Fn2 , v092
	.byte		N22   , Gs2
	.byte	W01
	.byte		        Cs3 , v100
	.byte	W10
	.byte	PEND
mus_qz_614_5_003:
	.byte	W12
	.byte		N18   , Fn2 , v088
	.byte	W12
	.byte		N07   , Gs2 , v092
	.byte	W12
	.byte		N22   , Gn1 , v088
	.byte	W01
	.byte		        Fn2 , v092
	.byte		N22   , As2
	.byte	W01
	.byte		        Cs3 , v100
	.byte	W22
	.byte		N32   , Fs1 , v100 , gtp1
	.byte		N17   , Fs2 , v092
	.byte	W12
	.byte		        As2 , v088
	.byte	W12
	.byte		N10   , Cs3 , v096
	.byte	W12
	.byte	PEND
mus_qz_614_5_004:
	.byte		N64   , Fs1 , v100
	.byte	W12
	.byte		N23   , Fs2 , v088
	.byte	W12
	.byte		        As2 , v100
	.byte	W12
	.byte		N28   , Cs3 , v096
	.byte	W24
	.byte		N23   , Fs2
	.byte	W12
	.byte		N22   , Fs1 , v088
	.byte		N22   , As2
	.byte	W12
	.byte		N10   , Cs3 , v096
	.byte	W12
	.byte	PEND
mus_qz_614_5_005:
	.byte		N80   , Gn1 , v100 , gtp3
	.byte	W12
	.byte		N23   , Gn2 , v092
	.byte	W12
	.byte		N19   , As2 , v100
	.byte	W12
	.byte		N16   , Cs3 , v092
	.byte	W24
	.byte		N07   , Ds3 , v108
	.byte	W04
	.byte		BEND  , c_v+21
	.byte	W04
	.byte		        c_v+0
	.byte		N03   , Ds3 , v096
	.byte	W04
	.byte		N10   , As2 , v092
	.byte	W12
	.byte		        Gn2
	.byte	W12
	.byte	PEND
mus_qz_614_5_006:
	.byte		N80   , Gs1 , v100 , gtp2
	.byte	W12
	.byte		N23   , Gs2 , v088
	.byte	W12
	.byte		        Cn3 , v092
	.byte	W12
	.byte		N22   , Ds3
	.byte	W24
	.byte		N23   , Ds2
	.byte	W12
	.byte		N17   , Gs2
	.byte	W12
	.byte		N10   , Cs3
	.byte	W12
	.byte	PEND
	.byte		N05   , Gs1 , v100
	.byte	W01
	.byte		N04   , Ds2 , v076
	.byte		N04   , Gs2
	.byte	W01
	.byte		        Cn3 , v092
	.byte	W10
	.byte		        Gs1
	.byte	W01
	.byte		        Ds2 , v076
	.byte		N04   , As2 , v068
	.byte	W01
	.byte		N03   , Cs3 , v088
	.byte	W22
	.byte		N22   , Gs1 , v092
	.byte	W01
	.byte		N21   , Gs2 , v088
	.byte	W01
	.byte		        Cn3 , v092
	.byte	W01
	.byte		        Ds3 , v100
	.byte	W21
	.byte		BEND  , c_v-22
	.byte	W01
	.byte		N10   , Gs2 , v092
	.byte		N10   , Cn3
	.byte	W05
	.byte		BEND  , c_v+0
	.byte	W07
	.byte		N05   , As2 , v080
	.byte		N05   , Cs3
	.byte	W11
	.byte		N02   , Cn3 , v092
	.byte	W01
	.byte		        Ds3
	.byte	W11
mus_qz_614_5_B1:
	.byte		N44   , Cs2 , v092 , gtp3
	.byte	W12
	.byte		N32   , Gs2 , v080 , gtp3
	.byte	W12
	.byte		N28   , Cs3 , v088 , gtp1
	.byte	W12
	.byte		N22   , Fn3 , v092
	.byte	W24
	.byte		N23   , Gs2 , v080
	.byte	W12
	.byte		N17   , Cs3 , v088
	.byte	W12
	.byte		N10   , Fn3 , v092
	.byte	W12
	.byte		N32   , Fn1 , v104 , gtp3
	.byte	W12
	.byte		N16   , Gs2 , v092
	.byte	W12
	.byte		N05   , Cn3
	.byte	W12
	.byte		N22   , Cn2 , v096
	.byte	W01
	.byte		        An2 , v080
	.byte		N22   , Cn3
	.byte	W01
	.byte		        Ds3 , v092
	.byte	W22
	.byte		N17   , Fn2
	.byte	W12
	.byte		N23   , An1 , v096
	.byte		N17   , Cn3 , v088
	.byte	W12
	.byte		N10   , Ds3 , v092
	.byte	W12
	.byte		N28   , As1 , v096
	.byte		N10   , Cs3 , v092
	.byte		N10   , Fn3
	.byte	W12
	.byte		N16   , Gs2
	.byte	W12
	.byte		N04   , Fn3
	.byte	W12
	.byte		N32   , An1 , v096 , gtp3
	.byte	W01
	.byte		N22   , An2 , v092
	.byte		N22   , Cs3 , v088
	.byte	W01
	.byte		        Fn3 , v092
	.byte	W22
	.byte		N10   , An2
	.byte	W12
	.byte		        Cs3 , v088
	.byte	W12
	.byte		N40   , Gs1 , v096
	.byte	W01
	.byte		N22   , Gs2 , v092
	.byte		N28   , Cs3 , v092 , gtp1
	.byte	W01
	.byte		        Fn3 , v092 , gtp1
	.byte	W10
	.byte	W12
	.byte		N17   , Gs2
	.byte	W12
	.byte		N05   , Cs3 , v088
	.byte	W01
	.byte		        Fn3 , v092
	.byte	W11
	.byte		N23   , Gn1 , v096
	.byte	W01
	.byte		N21   , Gn2 , v084
	.byte		N21   , As2
	.byte	W01
	.byte		        Ds3 , v092
	.byte	W22
	.byte		N17   , Fs1 , v088
	.byte	W01
	.byte		        Fs2 , v076
	.byte		N17   , As2
	.byte		N17   , Cs3 , v084
	.byte	W23
	.byte		N01   , Fs1 , v036
	.byte		N01   , Cs2
	.byte		N01   , As2
	.byte	W06
	.byte		        Fs1 , v020
	.byte		N01   , Cs2
	.byte		N01   , As2
	.byte	W06
	.byte		N21   , Fs1 , v092
	.byte	W01
	.byte		        Cs2 , v076
	.byte	W01
	.byte		        As2 , v096
	.byte	W22
	.byte		N01   , Fs1 , v028
	.byte	W01
	.byte		        Cs2 , v056
	.byte	W01
	.byte		        As2 , v076
	.byte	W10
	.byte		N40   , Fs1 , v092 , gtp1
	.byte	W01
	.byte		        Cs2 , v076 , gtp1
	.byte	W01
	.byte		        As2 , v096 , gtp1
	.byte	W44
	.byte	W02
	.byte		N01   , Fs1 , v036
	.byte		N01   , Cs2 , v024
	.byte		N01   , As2 , v040
	.byte	W06
	.byte		        Fs1 , v020
	.byte		N01   , Cs2 , v012
	.byte		N01   , As2 , v020
	.byte	W06
	.byte		N22   , Gn1 , v092
	.byte	W01
	.byte		        Cs2 , v072
	.byte	W01
	.byte		        As2 , v096
	.byte	W22
	.byte		N01   , Gn1 , v028
	.byte	W01
	.byte		        Cs2 , v052
	.byte	W01
	.byte		        As2 , v076
	.byte	W10
	.byte		N40   , Gn1 , v092 , gtp1
	.byte	W01
	.byte		        Cs2 , v072 , gtp1
	.byte	W01
	.byte		        As2 , v096 , gtp1
	.byte	W44
	.byte	W02
	.byte		N01   , Gn1 , v060
	.byte		N01   , Cs2 , v044
	.byte		N01   , As2 , v064
	.byte	W06
	.byte		        Gn1 , v020
	.byte		N01   , Cs2 , v008
	.byte		N01   , As2 , v020
	.byte	W06
	.byte		N22   , Gs1 , v092
	.byte	W01
	.byte		        Ds2 , v076
	.byte	W01
	.byte		        Cn3 , v096
	.byte	W22
	.byte		N01   , Gs1 , v028
	.byte	W01
	.byte		        Ds2 , v056
	.byte	W01
	.byte		        Cn3 , v076
	.byte	W10
	.byte		N40   , Gs1 , v092 , gtp1
	.byte	W01
	.byte		        Ds2 , v076 , gtp1
	.byte	W01
	.byte		        Cn3 , v096 , gtp1
	.byte	W44
	.byte	W02
	.byte		N01   , Gs1 , v036
	.byte		N01   , Ds2 , v024
	.byte		N01   , Cn3 , v040
	.byte	W06
	.byte		        Gs1 , v020
	.byte		N01   , Ds2 , v012
	.byte		N01   , Cn3 , v020
	.byte	W06
	.byte		N10   , Gs1 , v092
	.byte	W01
	.byte		        Ds2 , v076
	.byte	W01
	.byte		        Cn3 , v096
	.byte	W10
	.byte		N02   , Gs1 , v080
	.byte	W01
	.byte		        Ds2 , v064
	.byte	W01
	.byte		        Cn3 , v084
	.byte	W22
	.byte		N30   , Gs1 , v092 , gtp1
	.byte	W01
	.byte		        Ds2 , v076 , gtp1
	.byte	W01
	.byte		        Cn3 , v096 , gtp1
	.byte	W32
	.byte	W02
	.byte		N01   , Gs1 , v036
	.byte		N01   , Ds2 , v024
	.byte		N01   , Cn3 , v040
	.byte	W06
	.byte		        Gs1 , v020
	.byte		N01   , Ds2 , v012
	.byte		N01   , Cn3 , v020
	.byte	W06
	.byte		N10   , Gs1 , v092
	.byte	W01
	.byte		        Ds2 , v076
	.byte	W01
	.byte		        Cn3 , v096
	.byte	W10
	.byte		N05   , Fn1 , v044
	.byte	W01
	.byte		        Gs2
	.byte		N05   , Cn3
	.byte	W01
	.byte		        Ds3 , v048
	.byte	W10
	.byte		        Fn1
	.byte	W01
	.byte		        Gs2
	.byte		N05   , Cn3
	.byte	W01
	.byte		        Ds3 , v056
	.byte	W10
	.byte		        Fn1 , v052
	.byte	W01
	.byte		        Gs2
	.byte		N05   , Cn3
	.byte	W01
	.byte		        Ds3 , v060
	.byte	W10
	.byte		        Fn1 , v056
	.byte	W01
	.byte		        Gs2
	.byte		N05   , Cn3
	.byte	W01
	.byte		        Ds3 , v064
	.byte	W10
	.byte		        Fn1 , v060
	.byte	W01
	.byte		        An2
	.byte		N05   , Cn3
	.byte	W01
	.byte		        Ds3 , v068
	.byte	W10
	.byte		        Fn1 , v064
	.byte	W01
	.byte		        An2
	.byte		N05   , Cn3
	.byte	W01
	.byte		        Ds3 , v072
	.byte	W10
	.byte		        Fn1 , v080
	.byte	W01
	.byte		        An2
	.byte	W01
	.byte		        Cn3
	.byte		N05   , Ds3 , v088
	.byte	W10
	.byte		        Fn1
	.byte	W01
	.byte		        An2
	.byte	W01
	.byte		        Cn3
	.byte		N05   , Ds3 , v096
	.byte	W10
	.byte		N28   , As1 , v104 , gtp1
	.byte	W01
	.byte		        Gs2 , v092 , gtp1
	.byte	W01
	.byte		        Cs3 , v092 , gtp1
	.byte		N28   , Fn3 , v104 , gtp1
	.byte	W32
	.byte	W02
	.byte		N21   , An1
	.byte	W01
	.byte		        An2 , v092
	.byte	W01
	.byte		        Cs3
	.byte		N21   , Fn3 , v104
	.byte	W22
	.byte		N17   , An2 , v096
	.byte	W12
	.byte		        Cs3 , v088
	.byte	W12
	.byte		N11   , Fn3 , v108
	.byte	W12
	.byte		N05   , Fn1 , v056
	.byte	W01
	.byte		        Gs2
	.byte		N05   , Cn3
	.byte	W01
	.byte		        Ds3 , v060
	.byte	W10
	.byte		        Fn1
	.byte	W01
	.byte		        Gs2
	.byte		N05   , Cn3
	.byte	W01
	.byte		        Ds3 , v068
	.byte	W10
	.byte		        Fn1 , v064
	.byte	W01
	.byte		        Gs2
	.byte		N05   , Cn3
	.byte	W01
	.byte		        Ds3 , v072
	.byte	W10
	.byte		        Fn1
	.byte	W01
	.byte		        Gs2
	.byte		N05   , Cn3
	.byte	W01
	.byte		        Ds3 , v076
	.byte	W10
	.byte		        Fn1
	.byte	W01
	.byte		        An2
	.byte		N05   , Cn3
	.byte	W01
	.byte		        Ds3 , v080
	.byte	W10
	.byte		        Fn1
	.byte	W01
	.byte		        An2
	.byte		N05   , Cn3
	.byte	W01
	.byte		        Ds3 , v084
	.byte	W10
	.byte		        Fn1 , v080
	.byte	W01
	.byte		        An2
	.byte	W01
	.byte		        Cn3
	.byte		N05   , Ds3 , v088
	.byte	W10
	.byte		        Fn1 , v100
	.byte	W01
	.byte		        An2 , v088
	.byte	W01
	.byte		        Cn3 , v096
	.byte		N05   , Cn3 , v072
	.byte		N05   , Ds3 , v100
	.byte	W10
	.byte		N28   , As1 , v112 , gtp1
	.byte	W01
	.byte		        Gs2 , v100 , gtp1
	.byte		N28   , Cs3 , v104 , gtp1
	.byte	W01
	.byte		        Fn3 , v112 , gtp1
	.byte	W32
	.byte	W02
	.byte		N44   , Ds2 , v112 , gtp3
	.byte	W01
	.byte		N22   , As2 , v100
	.byte		N22   , Cs3
	.byte	W01
	.byte		N21   , Fn3
	.byte	W22
	.byte		N23   , As2
	.byte	W12
	.byte		N11   , Cs3 , v084
	.byte	W12
	.byte		        As2 , v088
	.byte	W12
	.byte		N92   , Fs1 , v104 , gtp3
	.byte	W12
	.byte		N32   , As2 , v092 , gtp3
	.byte	W12
	.byte		N28   , Cs3 , v096 , gtp1
	.byte	W12
	.byte		        Fs3 , v100 , gtp1
	.byte	W24
	.byte		N32   , As2 , v088 , gtp3
	.byte	W12
	.byte		N23   , Cs3 , v096
	.byte	W12
	.byte		N11   , Fs3 , v092
	.byte	W12
	.byte		N92   , Gn1 , v100 , gtp3
	.byte	W12
	.byte		N32   , As2 , v092 , gtp3
	.byte	W12
	.byte		N28   , Cs3 , v096 , gtp1
	.byte	W12
	.byte		        Gn3 , v100 , gtp1
	.byte	W24
	.byte		N32   , As2 , v088 , gtp3
	.byte	W12
	.byte		N23   , Cs3 , v096
	.byte	W12
	.byte		N11   , Gn3 , v092
	.byte	W12
	.byte		N68   , Gs1 , v100 , gtp3
	.byte	W12
	.byte		N23   , Gs2
	.byte	W12
	.byte		        Cs3 , v088
	.byte	W12
	.byte		        Ds3 , v100
	.byte	W24
	.byte		N07   , Ds3 , v104
	.byte	W04
	.byte		BEND  , c_v+21
	.byte	W04
	.byte		        c_v+0
	.byte		N03   , Ds3 , v092
	.byte	W04
	.byte		N11   , Cn3 , v100
	.byte	W12
	.byte		        Gs2 , v088
	.byte	W12
	.byte		N92   , Gs1 , v108 , gtp3
	.byte	W12
	.byte		N23   , Gs2 , v096
	.byte	W12
	.byte		        Cn3 , v104
	.byte	W12
	.byte		        Ds3 , v096
	.byte	W24
	.byte		N09   , Gs2 , v092
	.byte	W01
	.byte		        Cn3 , v104
	.byte	W11
	.byte		        As2 , v088
	.byte	W01
	.byte		        Cs3 , v100
	.byte	W11
	.byte		        Cn3 , v088
	.byte	W01
	.byte		        Ds3 , v100
	.byte	W11
	.byte	PATT
	 ptrvalue	mus_qz_614_5_000
	.byte	PATT
	 ptrvalue	mus_qz_614_5_001
	.byte	PATT
	 ptrvalue	mus_qz_614_5_002
	.byte	PATT
	 ptrvalue	mus_qz_614_5_003
	.byte	PATT
	 ptrvalue	mus_qz_614_5_004
	.byte	PATT
	 ptrvalue	mus_qz_614_5_005
	.byte	PATT
	 ptrvalue	mus_qz_614_5_006
	.byte		N05   , Gs1 , v100
	.byte	W01
	.byte		N04   , Ds2 , v076
	.byte		N04   , Gs2
	.byte	W01
	.byte		        Cn3 , v092
	.byte	W10
	.byte		        Gs1
	.byte	W01
	.byte		        Ds2 , v076
	.byte		N04   , As2 , v068
	.byte	W01
	.byte		N03   , Cs3 , v088
	.byte	W22
	.byte		N22   , Gs1 , v092
	.byte	W01
	.byte		N21   , Gs2 , v088
	.byte	W01
	.byte		        Cn3 , v092
	.byte	W01
	.byte		        Ds3 , v100
	.byte	W21
	.byte		BEND  , c_v-22
	.byte	W01
	.byte		N10   , Gs2 , v092
	.byte		N10   , Cn3
	.byte	W05
	.byte		BEND  , c_v+0
	.byte	W06
	.byte		N05   , As2 , v080
	.byte		N05   , Cs3
	.byte	W12
	.byte		N02   , Cn3 , v092
	.byte	W01
	.byte		        Ds3
	.byte	W11
	.byte	GOTO
	 ptrvalue	mus_qz_614_5_B1
	.byte	FINE

/********************** Track  6 **********************/

mus_qz_614_6:
	.byte		KEYSH , mus_qz_614_key+0
	.byte		VOICE , 11
	.byte		PAN   , c_v-36
	.byte		VOL   , 35*mus_qz_614_mvl/mxv
	.byte		PRIO  , 54
	.byte		MOD   , 40
	.byte		LFOS  , 28
	.byte		MODT  , mod_tre
	.byte		LFODL , 7
	.byte	W12
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
mus_qz_614_6_000:
	.byte	W78
	.byte		N01   , Cn2 , v004
	.byte	W02
	.byte		        Dn2
	.byte	W01
	.byte		        En2 , v008
	.byte	W02
	.byte		        Fn2
	.byte	W01
	.byte		        Gn2 , v012
	.byte	W02
	.byte		        An2 , v020
	.byte	W01
	.byte		        Bn2 , v024
	.byte	W02
	.byte		        Cn3 , v032
	.byte	W01
	.byte		        Dn3 , v044
	.byte	W02
	.byte		        En3 , v048
	.byte	W01
	.byte		        Fn3 , v060
	.byte	W02
	.byte		        Gn3 , v068
	.byte	W01
	.byte	PEND
mus_qz_614_6_B1:
	.byte		N80   , Gs3 , v068 , gtp3
	.byte	W84
	.byte		N05   , Fs3
	.byte	W06
	.byte		        Fn3 , v060
	.byte	W06
	.byte		N32   , Ds3 , v072
	.byte	W36
	.byte		        Fn3 , v064 , gtp3
	.byte	W36
	.byte		N11   , Ds3 , v072
	.byte	W12
	.byte		N03   , Fn3 , v060
	.byte	W12
	.byte		N64   , Cs3 , v072 , gtp1
	.byte	W68
	.byte	W03
	.byte		N01   , Ds3 , v056
	.byte	W01
	.byte		N22   , Fn3 , v076
	.byte	W24
	.byte		N23   , Ds3 , v072
	.byte	W24
	.byte		N02   , Fn3 , v076
	.byte	W12
	.byte		N32   , Cs3 , v064 , gtp3
	.byte	W42
	.byte		N01   , Bn1 , v004
	.byte	W02
	.byte		        Cn2
	.byte	W01
	.byte		        Dn2 , v008
	.byte	W02
	.byte		        En2 , v012
	.byte	W01
	.byte		        Fn2 , v016
	.byte	W02
	.byte		        Gn2 , v020
	.byte	W01
	.byte		        An2 , v028
	.byte	W02
	.byte		        Bn2 , v036
	.byte	W01
	.byte		        Cn3 , v048
	.byte	W02
	.byte		        Dn3 , v052
	.byte	W01
	.byte		        En3 , v064
	.byte	W02
	.byte		        Fn3 , v072
	.byte	W01
	.byte		N92   , Cs3 , v068 , gtp2
	.byte	W01
	.byte		        Fs3 , v072 , gtp2
	.byte	W92
	.byte	W03
	.byte		        As2 , v068 , gtp2
	.byte	W01
	.byte		N32   , En3 , v076 , gtp2
	.byte	W36
	.byte		        Ds3 , v072 , gtp2
	.byte	W36
	.byte		N22   , Cs3 , v068
	.byte	W23
	.byte		N32   , Gs2 , v068 , gtp2
	.byte		N32   , Cn3 , v076 , gtp2
	.byte	W36
	.byte		        Ds2 , v072 , gtp2
	.byte		N32   , As2 , v080 , gtp2
	.byte	W36
	.byte		N17   , Gs2 , v060
	.byte		N17   , Cs3 , v068
	.byte	W20
	.byte		N01   , Gs2 , v072
	.byte	W01
	.byte		        As2 , v068
	.byte	W02
	.byte		        Bn2 , v076
	.byte	W01
	.byte	W01
	.byte		N10   , Cn3 , v080
	.byte	W11
	.byte		N04   , Gs2 , v076
	.byte	W24
	.byte	W01
	.byte		N32   , Ds3 , v076 , gtp2
	.byte	W36
	.byte		N10   , Cs3 , v072
	.byte	W12
	.byte		        Cn3 , v080
	.byte	W11
	.byte	W96
	.byte	W78
	.byte		N01   , Gn1 , v004
	.byte	W02
	.byte		        An1
	.byte	W01
	.byte		        Bn1 , v008
	.byte	W02
	.byte		        Cn2
	.byte	W01
	.byte		        Dn2 , v012
	.byte	W02
	.byte		        En2 , v020
	.byte	W01
	.byte		        Fs2 , v024
	.byte	W02
	.byte		        Gn2 , v032
	.byte	W01
	.byte		        An2 , v044
	.byte	W02
	.byte		        Bn2 , v048
	.byte	W01
	.byte		        Cn3 , v060
	.byte	W02
	.byte		        Dn3 , v068
	.byte	W01
	.byte		N44   , Gs2 , v072 , gtp3
	.byte	W01
	.byte		        Ds3 , v068 , gtp2
	.byte	W44
	.byte	W03
	.byte		N40   , An2
	.byte	W01
	.byte		        Fn3 , v060
	.byte	W40
	.byte	W01
	.byte		N01   , Bn2 , v052
	.byte	W02
	.byte		        Cn3 , v056
	.byte	W01
	.byte		        Dn3 , v068
	.byte	W02
	.byte		        En3 , v076
	.byte	W01
	.byte		N32   , As2 , v072 , gtp2
	.byte	W01
	.byte		        Fn3 , v068 , gtp1
	.byte	W32
	.byte	W03
	.byte		        As2 , v068 , gtp3
	.byte	W01
	.byte		        Gn3 , v060 , gtp3
	.byte	W32
	.byte	W03
	.byte		N01   , Fn3 , v064
	.byte	W02
	.byte		        En3 , v056
	.byte	W01
	.byte		        Dn3 , v048
	.byte	W02
	.byte		        Cn3 , v044
	.byte	W01
	.byte		        Bn2 , v032
	.byte	W02
	.byte		        An2 , v024
	.byte	W01
	.byte		        Gn2 , v016
	.byte	W02
	.byte		        Fn2 , v012
	.byte	W01
	.byte		        En2 , v008
	.byte	W02
	.byte		        Dn2
	.byte	W01
	.byte		        Cn2 , v004
	.byte	W02
	.byte		        Bn1
	.byte	W07
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
	.byte	PATT
	 ptrvalue	mus_qz_614_6_000
	.byte	GOTO
	 ptrvalue	mus_qz_614_6_B1
	.byte	FINE

/********************** Track  7 **********************/

mus_qz_614_7:
	.byte		KEYSH , mus_qz_614_key+0
	.byte		VOICE , 32
	.byte		PAN   , c_v-24
	.byte		VOL   , 48*mus_qz_614_mvl/mxv
	.byte		PRIO  , 52
	.byte	W12
	.byte		N68   , Gs2 , v052 , gtp1
	.byte		N68   , Fn3 , v056 , gtp1
	.byte	W72
	.byte		N22   , Gs2 , v032
	.byte		N17   , Cs3 , v048
	.byte	W18
	.byte		N02   , Cn3 , v056
	.byte	W03
	.byte		        Cs3
	.byte	W03
	.byte		N32   , Gs2 , v040 , gtp2
	.byte		N32   , Ds3 , v056 , gtp2
	.byte	W36
	.byte		        Gs2 , v040 , gtp2
	.byte		N32   , Cs3 , v048 , gtp2
	.byte	W36
	.byte		N23   , Cs3 , v052
	.byte		N23   , Fn3 , v064
	.byte	W24
	.byte	W96
	.byte	W96
mus_qz_614_7_000:
	.byte		N05   , As2 , v120
	.byte	W12
	.byte		        Cs3 , v112
	.byte	W12
	.byte		        Fn3 , v124
	.byte	W12
	.byte		N23   , An2 , v120
	.byte	W24
	.byte		N05   , Cs3 , v116
	.byte	W12
	.byte		        Fn3 , v108
	.byte	W12
	.byte		N23   , Gs2 , v124
	.byte	W12
	.byte	PEND
mus_qz_614_7_001:
	.byte	W12
	.byte		N05   , As2 , v112
	.byte	W12
	.byte		        Gs2
	.byte	W12
	.byte		N23   , Gn2 , v120
	.byte	W24
	.byte		N11   , Fs2 , v112
	.byte	W36
	.byte	PEND
mus_qz_614_7_002:
	.byte		N68   , Fs2 , v052 , gtp2
	.byte		N68   , As2 , v064 , gtp2
	.byte	W72
	.byte		N22   , As2 , v044
	.byte		N22   , Cs3 , v060
	.byte	W24
	.byte	PEND
mus_qz_614_7_003:
	.byte		N32   , Gn2 , v052 , gtp2
	.byte		N32   , Cs3 , v072 , gtp2
	.byte	W36
	.byte		N56   , As2 , v052 , gtp3
	.byte		N32   , En3 , v064 , gtp3
	.byte	W36
	.byte		N23   , Cs3 , v076
	.byte	W24
	.byte	PEND
mus_qz_614_7_004:
	.byte		N66   , Gs2 , v052 , gtp1
	.byte		N66   , Ds3 , v072 , gtp1
	.byte	W72
	.byte		N17   , Gs2 , v064
	.byte		N17   , Cs3 , v072
	.byte	W24
	.byte	PEND
mus_qz_614_7_005:
	.byte		N11   , Gs2 , v052
	.byte		N11   , Cn3 , v064
	.byte	W12
	.byte		        As2 , v052
	.byte		N11   , Cs3 , v064
	.byte	W24
	.byte		N44   , Cn3 , v052 , gtp3
	.byte		N44   , Ds3 , v064 , gtp3
	.byte	W60
	.byte	PEND
mus_qz_614_7_B1:
	.byte	W96
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_614_7_000
	.byte	PATT
	 ptrvalue	mus_qz_614_7_001
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		N44   , Gs2 , v080 , gtp1
	.byte		N44   , Ds3 , v068 , gtp1
	.byte	W48
	.byte		        An2 , v072 , gtp1
	.byte		N44   , Fn3 , v060 , gtp1
	.byte	W48
	.byte		N28   , As2 , v080 , gtp1
	.byte		N28   , Fn3 , v068 , gtp1
	.byte	W36
	.byte		        An2 , v072 , gtp1
	.byte		N28   , Fn3 , v060 , gtp1
	.byte	W60
	.byte	W96
	.byte	W96
	.byte		N92   , As2 , v080 , gtp2
	.byte		N92   , Fs3 , v068 , gtp2
	.byte	W96
	.byte		N44   , As2 , v080 , gtp3
	.byte		N44   , Gn3 , v064 , gtp3
	.byte	W48
	.byte		N22   , As2 , v072
	.byte		N22   , Ds3 , v060
	.byte	W24
	.byte		        Ds3 , v072
	.byte		N22   , Gn3 , v060
	.byte	W24
	.byte		N92   , Cs3 , v080 , gtp2
	.byte		N92   , Gs3 , v064 , gtp2
	.byte	W96
	.byte		VOL   , 6*mus_qz_614_mvl/mxv
	.byte		N88   , Cn3 , v072 , gtp1
	.byte		N88   , Gs3 , v060 , gtp1
	.byte	W06
	.byte		VOL   , 9*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        12*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        16*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        18*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        23*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        27*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        31*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        36*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        41*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        46*mus_qz_614_mvl/mxv
	.byte	W36
	.byte	W96
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_614_7_000
	.byte	PATT
	 ptrvalue	mus_qz_614_7_001
	.byte	PATT
	 ptrvalue	mus_qz_614_7_002
	.byte	PATT
	 ptrvalue	mus_qz_614_7_003
	.byte	PATT
	 ptrvalue	mus_qz_614_7_004
	.byte	PATT
	 ptrvalue	mus_qz_614_7_005
	.byte	GOTO
	 ptrvalue	mus_qz_614_7_B1
	.byte	FINE

/********************** Track  8 **********************/

mus_qz_614_8:
	.byte		KEYSH , mus_qz_614_key+0
	.byte		VOICE , 39
	.byte		PAN   , c_v+0
	.byte		VOL   , 44*mus_qz_614_mvl/mxv
	.byte		PRIO  , 40
	.byte	W12
	.byte		N02   , Fs1 , v092
	.byte	W12
	.byte		N03   , Fs1 , v056
	.byte	W12
	.byte		        Fs1 , v076
	.byte	W12
	.byte		        Fs1 , v092
	.byte	W12
	.byte		        Fs1 , v060
	.byte	W12
	.byte		        Fs1 , v076
	.byte	W12
	.byte		N11   , Fs0 , v092
	.byte		N03   , Fs1 , v088
	.byte	W06
	.byte		        Fs1 , v036
	.byte	W06
	.byte		N11   , As1 , v088
	.byte	W12
	.byte		N02   , Fs1 , v064
	.byte	W12
	.byte		N03   , Fs1 , v052
	.byte	W12
	.byte		N02   , Fs1 , v064
	.byte	W12
	.byte		N03   , Fs1 , v052
	.byte	W12
	.byte		N05   , Fs0 , v096
	.byte		N03   , Fs1 , v064
	.byte	W06
	.byte		N05   , Fs0 , v052
	.byte		N03   , Fs1
	.byte	W06
	.byte		N11   , Gn1 , v100
	.byte	W12
	.byte		        Fs0 , v092
	.byte		N02   , Fs1 , v064
	.byte	W06
	.byte		        Fs1 , v052
	.byte	W06
	.byte		N11   , Fn1 , v108
	.byte		N11   , As1 , v088
	.byte	W12
	.byte		N23   , En2 , v100
	.byte	W24
	.byte		N03   , Fs1 , v076
	.byte	W12
	.byte		        Fs1 , v092
	.byte	W12
	.byte		        Fs1 , v064
	.byte	W12
	.byte		        Fs1 , v076
	.byte	W12
	.byte		N11   , As0 , v092
	.byte		N03   , Fs1 , v088
	.byte	W06
	.byte		        Fs1 , v056
	.byte	W06
	.byte		N11   , As1 , v088
	.byte	W12
	.byte		N02   , Fs1 , v092
	.byte	W12
	.byte		N03   , Fs1 , v064
	.byte	W12
	.byte		        Fs1 , v076
	.byte	W12
	.byte		        Fs1 , v092
	.byte	W12
	.byte		        Fs1 , v064
	.byte	W12
	.byte		        Fs1 , v076
	.byte	W12
	.byte		N11   , As0 , v092
	.byte		N03   , Fs1 , v088
	.byte	W06
	.byte		        Fs1 , v056
	.byte	W06
	.byte		N11   , As1 , v088
	.byte	W12
	.byte		N02   , Fs1 , v064
	.byte	W12
	.byte	N03
	.byte	W06
	.byte		        Fs1 , v048
	.byte	W06
	.byte		N11   , As1 , v088
	.byte	W12
	.byte		N23   , En2 , v096
	.byte	W24
	.byte		N03   , Fs1 , v064
	.byte	W06
	.byte		        Fs1 , v052
	.byte	W06
	.byte		N11   , As0 , v092
	.byte		N03   , Fs1 , v064
	.byte	W06
	.byte		        Fs1 , v052
	.byte	W06
	.byte		N11   , As1 , v088
	.byte	W12
mus_qz_614_8_000:
	.byte		N02   , Fs1 , v076
	.byte	W12
	.byte		        Fs1 , v064
	.byte	W12
	.byte		N11   , As0 , v092
	.byte		N02   , Fs1 , v064
	.byte	W12
	.byte		N28   , Dn2 , v088 , gtp1
	.byte	W36
	.byte		N05   , As0 , v092
	.byte		N03   , Fs1 , v064
	.byte	W06
	.byte		N05   , As0 , v076
	.byte		N03   , Fs1 , v036
	.byte	W06
	.byte		N11   , As1 , v088
	.byte	W12
	.byte	PEND
	.byte		N02   , Fs1 , v064
	.byte		N23   , En2 , v100
	.byte	W12
	.byte		N02   , Fs1 , v052
	.byte	W12
	.byte		        Fs1 , v064
	.byte	W12
	.byte		N11   , As1 , v088
	.byte	W12
	.byte		N03   , Fs1 , v064
	.byte	W06
	.byte		        Fs1 , v048
	.byte	W18
	.byte		N11   , As0 , v092
	.byte	W12
	.byte		N03   , Fs1 , v064
	.byte	W06
	.byte		        Fs1 , v052
	.byte	W06
	.byte		N02   , Fs1 , v064
	.byte	W12
	.byte		N11   , As1 , v088
	.byte	W30
	.byte		N03   , Fs1 , v048
	.byte	W06
	.byte		        Fs1 , v064
	.byte	W06
	.byte		        Fs1 , v048
	.byte	W18
	.byte		N11   , As0 , v092
	.byte		N03   , Fs1 , v064
	.byte	W06
	.byte		        Fs1 , v048
	.byte	W06
	.byte		N11   , As1 , v088
	.byte	W12
	.byte		N02   , Fs1 , v048
	.byte	W12
	.byte		        Fs1 , v032
	.byte	W12
	.byte		N11   , As1 , v088
	.byte	W18
	.byte		N03   , Fs1 , v048
	.byte	W06
	.byte		        Fs1 , v064
	.byte	W06
	.byte		        Fs1 , v048
	.byte	W18
	.byte		N11   , As0 , v092
	.byte	W12
	.byte		N03   , Fs1 , v064
	.byte	W06
	.byte		        Fs1 , v044
	.byte	W06
	.byte		N02   , Fs1 , v064
	.byte	W12
	.byte		N05   , As1 , v088
	.byte	W06
	.byte		N02   , Fs1 , v036
	.byte	W06
	.byte		N08   , Fn1 , v088
	.byte	W12
	.byte		N23   , Dn2 , v100
	.byte	W24
	.byte		N02   , Fs0 , v036
	.byte	W03
	.byte		        Fs0 , v052
	.byte	W03
	.byte		        Fs0 , v060
	.byte	W03
	.byte		        Fs0 , v072
	.byte	W03
	.byte		N11   , Fs0 , v100
	.byte	W12
	.byte		        Fn1
	.byte	W12
mus_qz_614_8_B1:
	.byte		N23   , En2 , v100
	.byte	W24
	.byte		N11   , Fs0
	.byte		N02   , Fs1 , v036
	.byte	W18
	.byte		N03   , Fs1 , v044
	.byte	W06
	.byte		        Fs1 , v064
	.byte	W06
	.byte		        Fs1 , v044
	.byte	W18
	.byte		N11   , Fs0 , v100
	.byte	W12
	.byte		N03   , Fs1 , v064
	.byte	W06
	.byte		        Fs1 , v048
	.byte	W06
	.byte		N02   , Fs1 , v064
	.byte	W12
	.byte		N11   , As1 , v088
	.byte	W12
	.byte		        Fs0 , v100
	.byte		N02   , Fs1 , v064
	.byte	W12
	.byte		N11   , As1 , v088
	.byte	W12
	.byte		N03   , Fs1 , v064
	.byte	W06
	.byte	N03
	.byte	W12
	.byte	N03
	.byte	W06
	.byte		N11   , Fs0 , v100
	.byte		N03   , Fs1 , v064
	.byte	W12
	.byte		N11   , An1 , v072
	.byte		N11   , As1 , v088
	.byte	W12
	.byte		N23   , Dn2 , v100
	.byte	W24
	.byte		N11   , Fs0
	.byte	W12
	.byte		N23   , En2
	.byte	W24
	.byte		N11   , Bn1
	.byte	W12
	.byte		        Fs0
	.byte		N11   , Fn1 , v088
	.byte	W12
	.byte		N23   , Dn2 , v100
	.byte	W12
	.byte	W24
	.byte		N05   , Fs0
	.byte	W12
	.byte		N28   , En2 , v100 , gtp1
	.byte	W12
	.byte		N02   , Fs0 , v016
	.byte	W03
	.byte		        Fs0 , v032
	.byte	W03
	.byte		        Fs0 , v052
	.byte	W03
	.byte		        Fs0 , v088
	.byte	W03
	.byte		N05   , Fs0 , v104
	.byte	W06
	.byte		        Fs0 , v064
	.byte	W06
	.byte		        Bn1 , v100
	.byte	W06
	.byte		        An1 , v080
	.byte	W06
	.byte		N08   , Fn1 , v100
	.byte	W12
	.byte		N23   , En2
	.byte	W24
	.byte		N05   , Fs0
	.byte		N02   , Fs1 , v064
	.byte	W18
	.byte	N03
	.byte	W06
	.byte		        Fs1 , v044
	.byte	W06
	.byte		        Fs1 , v064
	.byte	W18
	.byte		N05   , Fs0 , v100
	.byte	W12
	.byte		N03   , Fs1 , v064
	.byte	W06
	.byte		        Fs1 , v048
	.byte	W06
	.byte		N02   , Fs1 , v064
	.byte	W12
	.byte		N11   , As1 , v088
	.byte	W12
	.byte		N05   , Fs0 , v100
	.byte		N02   , Fs1 , v064
	.byte	W18
	.byte	N03
	.byte	W06
	.byte		        Fs1 , v048
	.byte	W06
	.byte		        Fs1 , v064
	.byte	W18
	.byte		N05   , Fs0 , v100
	.byte	W12
	.byte		N11   , An1 , v088
	.byte		N11   , As1
	.byte	W12
	.byte		N02   , Fs1 , v064
	.byte	W12
	.byte		        Fs1 , v048
	.byte	W12
	.byte		N05   , Fs0 , v100
	.byte		N11   , As1 , v088
	.byte	W18
	.byte		N03   , Fs1 , v064
	.byte	W06
	.byte		        Fs1 , v044
	.byte	W06
	.byte		        Fs1 , v064
	.byte	W18
	.byte		N05   , Fs0 , v100
	.byte	W12
	.byte		N03   , Fs1 , v064
	.byte	W06
	.byte		        Fs1 , v048
	.byte	W06
	.byte		N02   , Fs1 , v064
	.byte	W12
	.byte		N11   , As1 , v088
	.byte	W12
	.byte		N05   , Fs0 , v100
	.byte		N02   , Fs1 , v064
	.byte	W18
	.byte	N03
	.byte	W06
	.byte		        Fs1 , v044
	.byte	W06
	.byte		        Fs1 , v064
	.byte	W18
	.byte		N11   , Bn1 , v088
	.byte	W12
	.byte		        An1 , v076
	.byte	W12
	.byte		N23   , En2 , v088
	.byte	W24
	.byte		N05   , Fs0 , v100
	.byte		N02   , Fs1 , v064
	.byte	W18
	.byte	N03
	.byte	W06
	.byte		        Fs1 , v040
	.byte	W06
	.byte		        Fs1 , v064
	.byte	W06
	.byte		N11   , As1 , v088
	.byte	W06
	.byte		N02   , Fs0 , v032
	.byte	W03
	.byte		        Fs0 , v028
	.byte	W03
	.byte		N05   , Fs0 , v100
	.byte	W12
	.byte	N05
	.byte		N03   , Fs1 , v064
	.byte	W06
	.byte		        Fs1 , v044
	.byte	W06
	.byte		N02   , Fs1 , v064
	.byte	W12
	.byte		N11   , As1 , v088
	.byte	W12
	.byte		N05   , Fs0 , v100
	.byte		N03   , Fs1 , v064
	.byte	W06
	.byte		        Fs1 , v044
	.byte	W06
	.byte		N17   , Cs2 , v088
	.byte	W18
	.byte		N03   , Fs1 , v044
	.byte	W18
	.byte		N05   , Fs0 , v080
	.byte	W06
	.byte		        Fs0 , v096
	.byte	W06
	.byte		N11   , An1 , v068
	.byte		N11   , As1 , v088
	.byte	W12
	.byte		N02   , Fs1 , v064
	.byte	W12
	.byte		N11   , As1 , v088
	.byte	W12
	.byte		N05   , Fs0 , v100
	.byte		N02   , Fs1 , v064
	.byte	W18
	.byte	N03
	.byte	W06
	.byte		        Fs1 , v040
	.byte	W06
	.byte		        Fs1 , v064
	.byte	W06
	.byte		N11   , As1 , v088
	.byte	W12
	.byte		N05   , Fs0 , v100
	.byte	W12
	.byte		N11   , Fn1 , v080
	.byte		N03   , Fs1 , v064
	.byte	W06
	.byte		        Fs1 , v040
	.byte	W06
	.byte		N02   , Fs1 , v064
	.byte	W12
	.byte		N11   , As1 , v088
	.byte	W12
	.byte		N05   , Fs0 , v100
	.byte		N02   , Fs1 , v064
	.byte	W18
	.byte	N03
	.byte	W06
	.byte		        Fs1 , v044
	.byte	W06
	.byte		        Fs1 , v064
	.byte	W18
	.byte		N05   , Fs0 , v100
	.byte		N02   , Fs1 , v064
	.byte	W12
	.byte		N11   , An1 , v088
	.byte		N11   , As1
	.byte	W12
	.byte		N02   , Fs1 , v064
	.byte	W12
	.byte		N11   , As1 , v088
	.byte	W12
	.byte		N05   , Fs0 , v100
	.byte		N02   , Fs1 , v064
	.byte	W18
	.byte	N03
	.byte	W06
	.byte		        Fs1 , v044
	.byte	W06
	.byte		        Fs1 , v064
	.byte	W18
	.byte		N05   , Fs0 , v100
	.byte		N02   , Fs1 , v064
	.byte	W12
	.byte	N03
	.byte	W06
	.byte		        Fs1 , v048
	.byte	W06
	.byte	W12
	.byte		N11   , As1 , v088
	.byte	W12
	.byte		N05   , Fs0 , v100
	.byte	W18
	.byte		N03   , Fs1 , v064
	.byte	W06
	.byte		        Fs1 , v044
	.byte	W06
	.byte		        Fs1 , v064
	.byte	W18
	.byte		N05   , Fs0 , v100
	.byte		N02   , Fs1 , v064
	.byte	W06
	.byte		        Fs1 , v048
	.byte	W06
	.byte		N11   , Gn1 , v072
	.byte		N11   , As1 , v088
	.byte	W12
	.byte		N02   , Fs1 , v064
	.byte	W12
	.byte		N11   , As1 , v088
	.byte	W12
	.byte		N05   , Fs0 , v100
	.byte		N02   , Fs1 , v064
	.byte	W18
	.byte	N03
	.byte	W06
	.byte		        Fs1 , v048
	.byte	W06
	.byte		        Fs1 , v064
	.byte	W18
	.byte		N05   , Fs0 , v100
	.byte	W12
	.byte		N03   , Fs1 , v064
	.byte	W06
	.byte		        Fs1 , v052
	.byte	W06
	.byte		N02   , Fs1 , v064
	.byte	W12
	.byte		N11   , As1 , v088
	.byte	W06
	.byte		N02   , Fs0 , v040
	.byte	W03
	.byte		        Fs0 , v032
	.byte	W03
	.byte		N05   , Fs0 , v100
	.byte		N02   , Fs1 , v064
	.byte	W12
	.byte		N11   , Fs0 , v100
	.byte	W06
	.byte		N03   , Fs1 , v064
	.byte	W06
	.byte		        Fs1 , v052
	.byte	W06
	.byte		        Fs1 , v064
	.byte	W18
	.byte		N05   , Fs0 , v100
	.byte		N02   , Fs1 , v064
	.byte	W06
	.byte		N05   , Fs0 , v060
	.byte		N02   , Fs1 , v048
	.byte	W06
	.byte		N11   , Fn1 , v100
	.byte		N11   , As1 , v080
	.byte	W12
	.byte		N23   , En2 , v100
	.byte	W24
	.byte		N11   , As0 , v092
	.byte		N03   , Fs1 , v076
	.byte	W12
	.byte		        Fs1 , v092
	.byte	W12
	.byte		        Fs1 , v064
	.byte	W12
	.byte		        Fs1 , v076
	.byte	W12
	.byte		N11   , As0 , v092
	.byte		N03   , Fs1 , v088
	.byte	W06
	.byte		        Fs1 , v056
	.byte	W06
	.byte		N11   , As1 , v088
	.byte	W12
	.byte		N02   , Fs1 , v092
	.byte	W12
	.byte		N03   , Fs1 , v064
	.byte	W12
	.byte		N11   , As0 , v092
	.byte		N03   , Fs1 , v076
	.byte	W12
	.byte		        Fs1 , v092
	.byte	W12
	.byte		        Fs1 , v064
	.byte	W12
	.byte		        Fs1 , v076
	.byte	W12
	.byte		N11   , As0 , v092
	.byte		N03   , Fs1 , v088
	.byte	W06
	.byte		        Fs1 , v056
	.byte	W06
	.byte		N11   , As1 , v088
	.byte	W12
	.byte		N02   , Fs1 , v064
	.byte	W12
	.byte	N03
	.byte	W06
	.byte		        Fs1 , v048
	.byte	W06
	.byte		N11   , As0 , v092
	.byte		N11   , As1 , v088
	.byte	W12
	.byte		N23   , En2 , v096
	.byte	W24
	.byte		N03   , Fs1 , v064
	.byte	W03
	.byte		        Fs1 , v044
	.byte	W03
	.byte		        Fs1 , v056
	.byte	W06
	.byte		N11   , As0 , v092
	.byte		N03   , Fs1 , v064
	.byte	W06
	.byte		        Fs1 , v044
	.byte	W06
	.byte		N11   , As1 , v088
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_614_8_000
	.byte		N02   , Fs1 , v064
	.byte		N23   , En2 , v100
	.byte	W12
	.byte		N02   , Fs1 , v052
	.byte	W12
	.byte		N11   , As0 , v092
	.byte		N02   , Fs1 , v064
	.byte	W12
	.byte		N11   , As1 , v088
	.byte	W12
	.byte		N03   , Fs1 , v064
	.byte	W06
	.byte		        Fs1 , v048
	.byte	W18
	.byte		N11   , As0 , v092
	.byte	W12
	.byte		N03   , Fs1 , v064
	.byte	W06
	.byte		        Fs1 , v048
	.byte	W06
	.byte		N02   , Fs1 , v064
	.byte	W12
	.byte		N11   , As1 , v088
	.byte	W12
	.byte		        As0 , v092
	.byte	W18
	.byte		N03   , Fs1 , v048
	.byte	W06
	.byte		        Fs1 , v064
	.byte	W06
	.byte		        Fs1 , v048
	.byte	W18
	.byte		N11   , As0 , v092
	.byte		N03   , Fs1 , v064
	.byte	W06
	.byte		        Fs1 , v048
	.byte	W06
	.byte		N11   , As1 , v088
	.byte	W12
	.byte		N02   , Fs1 , v048
	.byte	W12
	.byte		        Fs1 , v032
	.byte	W12
	.byte		N11   , As0 , v092
	.byte		N11   , As1 , v088
	.byte	W18
	.byte		N03   , Fs1 , v048
	.byte	W06
	.byte		        Fs1 , v064
	.byte	W06
	.byte		        Fs1 , v048
	.byte	W06
	.byte		N02   , Fs0 , v016
	.byte	W03
	.byte		        Fs0 , v028
	.byte	W03
	.byte		        Fs0 , v044
	.byte	W03
	.byte		        Fs0 , v080
	.byte	W03
	.byte		N05   , Fs0 , v100
	.byte	W06
	.byte		        Fs0 , v096
	.byte	W06
	.byte		N11   , Gn1 , v104
	.byte	W12
	.byte		N02   , Fs1 , v064
	.byte	W12
	.byte		N05   , As1 , v088
	.byte	W06
	.byte		N02   , Fs1 , v036
	.byte	W06
	.byte		N08   , Fn1 , v088
	.byte	W12
	.byte		N23   , Dn2 , v100
	.byte	W24
	.byte		N02   , Fs0 , v036
	.byte	W03
	.byte		        Fs0 , v052
	.byte	W03
	.byte		        Fs0 , v060
	.byte	W03
	.byte		        Fs0 , v072
	.byte	W03
	.byte		N11   , Fs0 , v100
	.byte	W12
	.byte		        Fn1 , v108
	.byte	W12
	.byte	GOTO
	 ptrvalue	mus_qz_614_8_B1
	.byte	FINE

/********************** Track  9 **********************/

mus_qz_614_9:
	.byte		KEYSH , mus_qz_614_key+0
	.byte		VOICE , 39
	.byte		PAN   , c_v+0
	.byte		VOL   , 55*mus_qz_614_mvl/mxv
	.byte		PRIO  , 38
	.byte		N05   , Cn1 , v068
	.byte	W06
	.byte		N06   , Cn1 , v100
	.byte	W06
	.byte		        Cn1 , v112
	.byte	W36
	.byte		        Cn1 , v096
	.byte	W24
	.byte		        Cn1 , v104
	.byte	W36
	.byte		        Cn1 , v116
	.byte	W36
	.byte		        Cn1 , v100
	.byte	W60
mus_qz_614_9_000:
	.byte		N06   , Cn1 , v116
	.byte	W36
	.byte		        Cn1 , v096
	.byte	W24
	.byte		        Cn1 , v104
	.byte	W36
	.byte	PEND
mus_qz_614_9_001:
	.byte		N06   , Cn1 , v116
	.byte	W36
	.byte		        Cn1 , v096
	.byte	W48
	.byte		        Cn1 , v108
	.byte	W12
	.byte	PEND
mus_qz_614_9_002:
	.byte		N06   , Cn1 , v116
	.byte	W24
	.byte		        Cn1 , v088
	.byte	W12
	.byte		        Cn1 , v116
	.byte	W24
	.byte		        Cn1 , v096
	.byte	W24
	.byte		        Cn1 , v116
	.byte	W12
	.byte	PEND
mus_qz_614_9_003:
	.byte	W12
	.byte		N05   , Cn1 , v100
	.byte	W24
	.byte		N06   , Cn1 , v116
	.byte	W24
	.byte		        Cn1 , v104
	.byte	W24
	.byte		N05   , Cn1 , v080
	.byte	W12
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_614_9_000
mus_qz_614_9_004:
	.byte		N06   , Cn1 , v116
	.byte	W36
	.byte		        Cn1 , v096
	.byte	W24
	.byte		        Cn1 , v108
	.byte	W36
	.byte	PEND
mus_qz_614_9_005:
	.byte		N06   , Cn1 , v116
	.byte	W36
	.byte		        Cn1 , v096
	.byte	W12
	.byte		N05   , Cn1 , v112
	.byte	W36
	.byte		        Cn1 , v100
	.byte	W12
	.byte	PEND
mus_qz_614_9_006:
	.byte		N06   , Cn1 , v116
	.byte	W12
	.byte		        Cn1 , v096
	.byte	W24
	.byte		N05   , Cn1 , v112
	.byte	W24
	.byte		N06   , Cn1 , v096
	.byte	W24
	.byte	N06
	.byte	W12
	.byte	PEND
mus_qz_614_9_B1:
	.byte	PATT
	 ptrvalue	mus_qz_614_9_000
	.byte	PATT
	 ptrvalue	mus_qz_614_9_000
	.byte		N06   , Cn1 , v116
	.byte	W36
	.byte		        Cn1 , v096
	.byte	W24
	.byte		        Cn1 , v108
	.byte	W24
	.byte		        Cn1 , v116
	.byte	W12
	.byte	W12
	.byte		N05   , Cn1 , v112
	.byte	W24
	.byte		N06   , Cn1 , v096
	.byte	W24
	.byte		        Cn1 , v104
	.byte	W24
	.byte		        Cn1 , v088
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_614_9_000
	.byte		N06   , Cn1 , v116
	.byte	W36
	.byte		        Cn1 , v096
	.byte	W12
	.byte		N05   , Cn1 , v104
	.byte	W12
	.byte		N06   , Cn1 , v096
	.byte	W36
	.byte	PATT
	 ptrvalue	mus_qz_614_9_000
	.byte		N06   , Cn1 , v116
	.byte	W36
	.byte		        Cn1 , v096
	.byte	W12
	.byte		N05   , Cn1 , v112
	.byte	W12
	.byte		N06   , Cn1 , v096
	.byte	W24
	.byte	N06
	.byte	W12
mus_qz_614_9_007:
	.byte		N06   , Cn1 , v116
	.byte	W36
	.byte		        Cn1 , v096
	.byte	W12
	.byte		N05   , Cn1 , v112
	.byte	W12
	.byte		N06   , Cn1 , v096
	.byte	W36
	.byte	PEND
	.byte		        Cn1 , v116
	.byte	W12
	.byte		N05   , Cn1 , v112
	.byte	W24
	.byte		N06   , Cn1 , v096
	.byte	W24
	.byte		        Cn1 , v104
	.byte	W24
	.byte		        Cn1 , v096
	.byte	W12
	.byte		        Cn1 , v116
	.byte	W36
	.byte		        Cn1 , v104
	.byte	W24
	.byte		        Cn1 , v108
	.byte	W36
	.byte		        Cn1 , v116
	.byte	W12
	.byte		N05   , Cn1 , v112
	.byte	W24
	.byte		N06   , Cn1 , v104
	.byte	W12
	.byte		N05   , Cn1 , v112
	.byte	W12
	.byte		N06   , Cn1 , v100
	.byte	W24
	.byte		        Cn1 , v096
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_614_9_000
	.byte	PATT
	 ptrvalue	mus_qz_614_9_007
	.byte	PATT
	 ptrvalue	mus_qz_614_9_004
	.byte		N06   , Cn1 , v116
	.byte	W12
	.byte		N05   , Cn1 , v112
	.byte	W36
	.byte		N06   , Cn1 , v100
	.byte	W18
	.byte		        Cn1 , v112
	.byte	W18
	.byte		        Cn1 , v084
	.byte	W12
	.byte	W36
	.byte		        Cn1 , v096
	.byte	W24
	.byte		        Cn1 , v104
	.byte	W36
	.byte	PATT
	 ptrvalue	mus_qz_614_9_001
	.byte	PATT
	 ptrvalue	mus_qz_614_9_002
	.byte	PATT
	 ptrvalue	mus_qz_614_9_003
	.byte	PATT
	 ptrvalue	mus_qz_614_9_000
	.byte	PATT
	 ptrvalue	mus_qz_614_9_004
	.byte	PATT
	 ptrvalue	mus_qz_614_9_005
	.byte	PATT
	 ptrvalue	mus_qz_614_9_006
	.byte	GOTO
	 ptrvalue	mus_qz_614_9_B1
	.byte		N06   , Cn1 , v116
	.byte	W06
	.byte	FINE

/********************** Track 10 **********************/

mus_qz_614_10:
	.byte		KEYSH , mus_qz_614_key+0
	.byte		VOICE , 20
	.byte		PAN   , c_v+0
	.byte		VOL   , 68*mus_qz_614_mvl/mxv
	.byte		BENDR , 6
	.byte		PRIO  , 60
	.byte		N11   , Gs1 , v100
	.byte	W05
	.byte		BEND  , c_v+10
	.byte	W01
	.byte		        c_v+21
	.byte	W02
	.byte		        c_v+32
	.byte	W04
	.byte		        c_v+0
	.byte		N06   , Cs2 , v120
	.byte	W36
	.byte		        Cs2 , v112
	.byte	W24
	.byte	N06
	.byte	W12
	.byte		N23   , Gs1 , v120
	.byte	W24
	.byte		N06   , Cs2
	.byte	W36
	.byte		        Cs2 , v108
	.byte	W24
	.byte		        Cs2 , v116
	.byte	W12
	.byte		N23   , Gs2 , v120
	.byte	W24
mus_qz_614_10_000:
	.byte		N05   , Cs2 , v120
	.byte	W36
	.byte		        Cs2 , v112
	.byte	W24
	.byte	N05
	.byte	W12
	.byte		N11   , Gs1 , v120
	.byte	W12
	.byte		N05   , Cs2 , v108
	.byte	W12
	.byte	PEND
mus_qz_614_10_001:
	.byte		N05   , Fn2 , v124
	.byte	W24
	.byte		        Cn2 , v112
	.byte	W12
	.byte		N23   , Fn1 , v120
	.byte	W24
	.byte		N11   , Cn2 , v108
	.byte	W12
	.byte		        Fn2 , v120
	.byte	W12
	.byte		        An2 , v108
	.byte	W12
	.byte	PEND
mus_qz_614_10_002:
	.byte		N11   , As2 , v120
	.byte	W12
	.byte		N05   , Cs3 , v108
	.byte	W12
	.byte		        Fn3 , v112
	.byte	W12
	.byte		N23   , An2 , v120
	.byte	W24
	.byte		N05   , Cs3 , v108
	.byte	W12
	.byte		        Fn3 , v112
	.byte	W12
	.byte		N23   , Gs2 , v120
	.byte	W12
	.byte	PEND
mus_qz_614_10_003:
	.byte	W12
	.byte		N05   , As2 , v120
	.byte	W12
	.byte		        Gs2 , v104
	.byte	W12
	.byte		N23   , Gn2 , v112
	.byte	W24
	.byte		N11   , Fs2
	.byte	W12
	.byte		        Cs3 , v108
	.byte	W12
	.byte		N05   , As2 , v096
	.byte	W12
	.byte	PEND
mus_qz_614_10_004:
	.byte		N05   , Fs2 , v120
	.byte	W36
	.byte		        Fs2 , v112
	.byte	W24
	.byte	N05
	.byte	W12
	.byte		N23   , Cs2 , v124
	.byte	W24
	.byte	PEND
mus_qz_614_10_005:
	.byte		N05   , Gn2 , v120
	.byte	W36
	.byte		        Gn2 , v112
	.byte	W24
	.byte		N23   , Cs2 , v120
	.byte	W24
	.byte		N05   , Gn2 , v112
	.byte	W12
	.byte	PEND
mus_qz_614_10_006:
	.byte		N05   , Gs2 , v124
	.byte	W36
	.byte		        Gs2 , v112
	.byte	W12
	.byte		        Ds2 , v120
	.byte	W12
	.byte		N23   , Cs2 , v116
	.byte	W24
	.byte		N05   , Gs2 , v112
	.byte	W12
	.byte	PEND
mus_qz_614_10_007:
	.byte		N05   , Gs1 , v120
	.byte	W12
	.byte		        Gs1 , v112
	.byte	W24
	.byte		N17   , Gs1 , v120
	.byte	W24
	.byte	N11
	.byte	W12
	.byte		        As1 , v112
	.byte	W12
	.byte		        Ds2
	.byte	W12
	.byte	PEND
mus_qz_614_10_B1:
	.byte		N05   , Cs2 , v124
	.byte	W36
	.byte		        Cs2 , v112
	.byte	W24
	.byte		        Cs2 , v120
	.byte	W12
	.byte		N23   , Gs1
	.byte	W24
	.byte		N05   , Fn2
	.byte	W36
	.byte		N11   , Cn2 , v112
	.byte	W12
	.byte		        Fn1 , v120
	.byte	W12
	.byte		N23   , Fn2 , v116
	.byte	W24
	.byte		N11   , An2 , v120
	.byte	W12
	.byte		N05   , As2
	.byte	W12
	.byte		        Cs3 , v108
	.byte	W12
	.byte		        Fn3 , v112
	.byte	W12
	.byte		N23   , An2 , v120
	.byte	W24
	.byte		N05   , Cs3 , v108
	.byte	W12
	.byte		        Fn3 , v112
	.byte	W12
	.byte		N23   , Gs2 , v124
	.byte	W12
	.byte	W12
	.byte		N05   , As2 , v108
	.byte	W12
	.byte		        Gs2
	.byte	W12
	.byte		N32   , Gn2 , v120 , gtp3
	.byte	W60
	.byte		N05   , Fs2
	.byte	W36
	.byte		        Fs2 , v108
	.byte	W24
	.byte	N05
	.byte	W12
	.byte		N23   , Cs2 , v120
	.byte	W24
	.byte		N05   , Gn2
	.byte	W36
	.byte		        Gn2 , v112
	.byte	W24
	.byte		N23   , Cs2 , v104
	.byte	W24
	.byte		N05   , Gn2 , v112
	.byte	W12
	.byte		        Gs2 , v120
	.byte	W36
	.byte		        Gs2 , v108
	.byte	W12
	.byte		        Ds2 , v112
	.byte	W12
	.byte		N23   , Cs2 , v120
	.byte	W24
	.byte		N05   , Gs2 , v112
	.byte	W12
	.byte		        Gs2 , v124
	.byte	W36
	.byte		        Gs2 , v112
	.byte	W12
	.byte		        Cn2 , v116
	.byte	W12
	.byte		N11   , Gs2 , v124
	.byte	W24
	.byte		        Fs2 , v112
	.byte	W12
	.byte		N05   , Fn2 , v080
	.byte	W12
	.byte		        Fn2 , v084
	.byte	W12
	.byte		        Fn2 , v088
	.byte	W12
	.byte	N05
	.byte	W12
	.byte		        Fn2 , v092
	.byte	W12
	.byte		        Fn2 , v100
	.byte	W12
	.byte		        Fn2 , v104
	.byte	W12
	.byte		        Fn2 , v108
	.byte	W12
	.byte		        As1 , v120
	.byte	W12
	.byte		        Cs2
	.byte	W12
	.byte		N11   , Fn2
	.byte	W12
	.byte		N23   , An1
	.byte	W24
	.byte		N05   , Cs2
	.byte	W24
	.byte		        Fn2
	.byte	W12
	.byte		        Fn2 , v080
	.byte	W12
	.byte		        Fn2 , v084
	.byte	W12
	.byte		        Fn2 , v088
	.byte	W12
	.byte		        Fn2 , v092
	.byte	W12
	.byte		        Fn2 , v100
	.byte	W12
	.byte		        Fn2 , v104
	.byte	W12
	.byte		        Fn2 , v108
	.byte	W12
	.byte		        Fn2 , v112
	.byte	W12
	.byte		N11   , As1 , v120
	.byte	W12
	.byte		N05   , Fn2 , v108
	.byte	W12
	.byte		        As2 , v116
	.byte	W12
	.byte		N23   , Ds2 , v120
	.byte	W24
	.byte		N11   , As1
	.byte	W12
	.byte		N06   , Ds2 , v108
	.byte	W12
	.byte		        Gn2 , v104
	.byte	W12
	.byte		N05   , Fs2 , v120
	.byte	W36
	.byte		        Fs2 , v112
	.byte	W06
	.byte		        Fs2 , v100
	.byte	W06
	.byte		        Fs2 , v116
	.byte	W12
	.byte		N11   , Cs3 , v112
	.byte	W24
	.byte		N05   , Fn2 , v108
	.byte	W06
	.byte		        Fs2 , v100
	.byte	W06
	.byte		        Gn2 , v124
	.byte	W36
	.byte		        Gn2 , v108
	.byte	W06
	.byte		        Gn2 , v096
	.byte	W06
	.byte		        Gn2 , v112
	.byte	W12
	.byte		N11   , Cs3 , v120
	.byte	W24
	.byte		N05   , Fs2 , v108
	.byte	W06
	.byte		        Gn2 , v100
	.byte	W06
	.byte		        Gs2 , v112
	.byte	W36
	.byte		        Gs2 , v108
	.byte	W06
	.byte		        Gs2 , v100
	.byte	W06
	.byte		        Gs2 , v108
	.byte	W12
	.byte		N11   , Ds3 , v116
	.byte	W24
	.byte		N05   , Fs2 , v104
	.byte	W06
	.byte		        Gn2 , v096
	.byte	W06
	.byte		        Gs2 , v112
	.byte	W36
	.byte		        Gs2 , v104
	.byte	W06
	.byte		        Gs2 , v096
	.byte	W06
	.byte		        Gs2 , v120
	.byte	W12
	.byte		N11   , Ds3 , v112
	.byte	W24
	.byte		N05   , Gs1
	.byte	W06
	.byte		        Cn2 , v100
	.byte	W06
	.byte	PATT
	 ptrvalue	mus_qz_614_10_000
	.byte	PATT
	 ptrvalue	mus_qz_614_10_001
	.byte	PATT
	 ptrvalue	mus_qz_614_10_002
	.byte	PATT
	 ptrvalue	mus_qz_614_10_003
	.byte	PATT
	 ptrvalue	mus_qz_614_10_004
	.byte	PATT
	 ptrvalue	mus_qz_614_10_005
	.byte	PATT
	 ptrvalue	mus_qz_614_10_006
	.byte	PATT
	 ptrvalue	mus_qz_614_10_007
	.byte	GOTO
	 ptrvalue	mus_qz_614_10_B1
	.byte	FINE

/********************** Track 11 **********************/

mus_qz_614_11:
	.byte		KEYSH , mus_qz_614_key+0
	.byte		VOICE , 34
	.byte		PAN   , c_v+61
	.byte		VOL   , 13*mus_qz_614_mvl/mxv
	.byte		BENDR , 6
	.byte		PRIO  , 38
	.byte		BEND  , c_v+1
	.byte	W12
	.byte	W96
	.byte	W84
	.byte		N19   , Gs3 , v104
	.byte	W12
mus_qz_614_11_000:
	.byte	W09
	.byte		N02   , Cn4 , v084
	.byte	W03
	.byte		N11   , Cs4 , v104
	.byte	W12
	.byte		N05   , Gs3 , v088
	.byte	W12
	.byte		        Cn4 , v104
	.byte	W12
	.byte		N20   , Cs4 , v112
	.byte	W24
	.byte		N23   , Ds4
	.byte	W24
	.byte	PEND
mus_qz_614_11_001:
	.byte		N05   , Fn4 , v100
	.byte	W12
	.byte		N32   , Gs4 , v112 , gtp3
	.byte	W18
	.byte		VOL   , 13*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        11*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        9*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        13*mus_qz_614_mvl/mxv
	.byte		N05   , Fn4 , v100
	.byte	W24
	.byte		N23   , Ds4 , v112
	.byte	W24
	.byte	PEND
mus_qz_614_11_002:
	.byte		N05   , Fn4 , v104
	.byte	W12
	.byte		N92   , Cs4 , v112 , gtp3
	.byte	W42
	.byte		VOL   , 13*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        11*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        9*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        7*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        6*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        4*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        3*mus_qz_614_mvl/mxv
	.byte	W06
	.byte	PEND
mus_qz_614_11_003:
	.byte		VOL   , 2*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        1*mus_qz_614_mvl/mxv
	.byte	W54
	.byte		        13*mus_qz_614_mvl/mxv
	.byte	W36
	.byte	PEND
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
mus_qz_614_11_B1:
	.byte		N02   , Gs3 , v092
	.byte	W06
	.byte		N04   , Gs3 , v084
	.byte	W06
	.byte		N11   , Cs4 , v096
	.byte	W12
	.byte		N05   , Gs3 , v084
	.byte	W12
	.byte		        Cn4 , v092
	.byte	W12
	.byte		N23   , Cs4 , v100
	.byte	W24
	.byte		        Ds4 , v092
	.byte	W24
	.byte		N05   , Fn4 , v084
	.byte	W09
	.byte		N02   , Gn4
	.byte	W03
	.byte		N23   , Gs4 , v104
	.byte	W24
	.byte		N05   , Fn4 , v092
	.byte	W12
	.byte		N22   , An4 , v104
	.byte	W24
	.byte		N04   , Fn4 , v096
	.byte	W12
	.byte		N11   , Ds4 , v104
	.byte	W12
	.byte		N05   , Fn4 , v092
	.byte	W09
	.byte		N02   , Cn4 , v080
	.byte	W03
	.byte		N84   , Cs4 , v100 , gtp1
	.byte	W48
	.byte		VOL   , 13*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        11*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        9*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        7*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        6*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        4*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        3*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        2*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        1*mus_qz_614_mvl/mxv
	.byte	W72
	.byte		PAN   , c_v+19
	.byte		VOL   , 13*mus_qz_614_mvl/mxv
	.byte		N11   , As3 , v092
	.byte	W12
	.byte		N05   , Cs4 , v084
	.byte	W12
	.byte		N52   , Fs4 , v104 , gtp1
	.byte	W36
	.byte		VOL   , 13*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        13*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        11*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        13*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		N11   , Fs4 , v100
	.byte	W12
	.byte		        Fn4 , v088
	.byte	W12
	.byte		N05   , Ds4
	.byte	W12
	.byte		N32   , Cs4 , v104 , gtp3
	.byte	W36
	.byte		N05   , Cn4 , v092
	.byte	W24
	.byte		N23   , Cn4 , v104
	.byte	W24
	.byte		N05   , Cs4 , v092
	.byte	W09
	.byte		N02   , Dn4 , v076
	.byte	W03
	.byte		N68   , Ds4 , v100 , gtp3
	.byte	W36
	.byte		VOL   , 13*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        12*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        10*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        9*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        8*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        7*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        6*mus_qz_614_mvl/mxv
	.byte	W12
	.byte		        13*mus_qz_614_mvl/mxv
	.byte		N11   , As3 , v088
	.byte	W12
	.byte		        Gs3 , v084
	.byte	W12
	.byte		        Ds3 , v068
	.byte	W21
	.byte		N02   , As3 , v056
	.byte	W03
	.byte		N32   , Cn4 , v080 , gtp3
	.byte	W36
	.byte		N11   , As3 , v084
	.byte	W12
	.byte		        Fs3 , v068
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W12
	.byte		        Fn4 , v108
	.byte	W12
	.byte		        Ds4 , v096
	.byte	W12
	.byte		        Cs4 , v100
	.byte	W12
	.byte		N23   , Fn4 , v108
	.byte	W24
	.byte		N11   , Ds4 , v100
	.byte	W24
	.byte		        Cs4 , v092
	.byte	W12
	.byte		        Fn4 , v104
	.byte	W12
	.byte		        Ds4 , v096
	.byte	W12
	.byte		        Cs4 , v104
	.byte	W12
	.byte		N23   , Fn4
	.byte	W24
	.byte		N11   , As4 , v096
	.byte	W24
	.byte		        Cn5 , v092
	.byte	W12
	.byte		N68   , Gs4 , v104 , gtp3
	.byte	W72
	.byte		N23   , Ds4 , v092
	.byte	W12
	.byte	W12
	.byte		VOL   , 2*mus_qz_614_mvl/mxv
	.byte		N76   , Cn4 , v104 , gtp1
	.byte	W06
	.byte		VOL   , 2*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        3*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        4*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        5*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        6*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        8*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        9*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        10*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        12*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        13*mus_qz_614_mvl/mxv
	.byte	W24
	.byte	PATT
	 ptrvalue	mus_qz_614_11_000
	.byte	PATT
	 ptrvalue	mus_qz_614_11_001
	.byte	PATT
	 ptrvalue	mus_qz_614_11_002
	.byte	PATT
	 ptrvalue	mus_qz_614_11_003
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_614_11_B1
	.byte	FINE

/********************** Track 12 **********************/

mus_qz_614_12:
	.byte		KEYSH , mus_qz_614_key+0
	.byte		VOICE , 35
	.byte		PAN   , c_v+56
	.byte		VOL   , 9*mus_qz_614_mvl/mxv
	.byte		PRIO  , 36
	.byte	W12
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W84
	.byte		N11   , As3 , v120
	.byte	W12
mus_qz_614_12_000:
	.byte		N05   , Cs4 , v116
	.byte	W10
	.byte		N01   , Fn4 , v064
	.byte	W02
	.byte		N44   , Fs4 , v127 , gtp3
	.byte	W36
	.byte		VOL   , 9*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        8*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        10*mus_qz_614_mvl/mxv
	.byte	W12
	.byte		N11   , Fs4 , v120
	.byte	W12
	.byte		        Fn4 , v112
	.byte	W12
	.byte	PEND
mus_qz_614_12_001:
	.byte		N05   , Ds4 , v100
	.byte	W09
	.byte		N02   , Cn4 , v064
	.byte	W03
	.byte		N32   , Cs4 , v120 , gtp3
	.byte	W24
	.byte		VOL   , 9*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        8*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        10*mus_qz_614_mvl/mxv
	.byte		N05   , Cn4 , v116
	.byte	W24
	.byte		N23   , Cn4 , v120
	.byte	W24
	.byte	PEND
mus_qz_614_12_002:
	.byte		N05   , Cs4 , v112
	.byte	W09
	.byte		N02   , Dn4 , v060
	.byte	W03
	.byte		N68   , Ds4 , v116 , gtp3
	.byte	W60
	.byte		VOL   , 9*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        8*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        10*mus_qz_614_mvl/mxv
	.byte		N23   , Cs4 , v120
	.byte	W12
	.byte	PEND
mus_qz_614_12_003:
	.byte	W12
	.byte		N11   , Cn4 , v120
	.byte	W12
	.byte		N05   , Cs4
	.byte	W24
	.byte		N32   , Ds4 , v120 , gtp3
	.byte	W24
	.byte		VOL   , 9*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        8*mus_qz_614_mvl/mxv
	.byte	W06
	.byte		        10*mus_qz_614_mvl/mxv
	.byte	W12
	.byte	PEND
mus_qz_614_12_B1:
	.byte		N02   , Gs4 , v088
	.byte	W06
	.byte		N05   , Gs4 , v080
	.byte	W90
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
	.byte	W84
	.byte		N11   , As3 , v120
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_614_12_000
	.byte	PATT
	 ptrvalue	mus_qz_614_12_001
	.byte	PATT
	 ptrvalue	mus_qz_614_12_002
	.byte	PATT
	 ptrvalue	mus_qz_614_12_003
	.byte	GOTO
	 ptrvalue	mus_qz_614_12_B1
	.byte	FINE

/******************************************************/
	.p2align	2

mus_qz_614:
	.byte	12	/* NumTrks */
	.byte	0	/* NumBlks */
	.byte	mus_qz_614_pri	/* Priority */
	.byte	mus_qz_614_rev	/* Reverb. */
space64 4
	ptrvalue	mus_qz_614_grp

	ptrvalue	mus_qz_614_1
	ptrvalue	mus_qz_614_2
	ptrvalue	mus_qz_614_3
	ptrvalue	mus_qz_614_4
	ptrvalue	mus_qz_614_5
	ptrvalue	mus_qz_614_6
	ptrvalue	mus_qz_614_7
	ptrvalue	mus_qz_614_8
	ptrvalue	mus_qz_614_9
	ptrvalue	mus_qz_614_10
	ptrvalue	mus_qz_614_11
	ptrvalue	mus_qz_614_12

	.end
