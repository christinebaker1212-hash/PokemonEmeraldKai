	.include "asm/macros/bit_width.inc"

	.include "MPlayDef.s"

/* Moemon Quetzal song 649, disassembled from the ROM by tools/quetzal_rip/songs.py.
   "org" comments give the ROM address of the bytes that follow. */

	.equ	mus_qz_649_grp, voicegroup_qz_songs
	.equ	mus_qz_649_pri, 0
	.equ	mus_qz_649_rev, reverb_set+0
	.equ	mus_qz_649_mvl, 127
	.equ	mus_qz_649_key, 0
	.equ	mus_qz_649_tbs, 1
	.equ	mus_qz_649_exg, 1
	.equ	mus_qz_649_cmp, 1

	.section .rodata
	.global	mus_qz_649
	.p2align	2
/* org 0x096965F0 */

/********************** Track  1 **********************/

mus_qz_649_1:
	.byte		KEYSH , mus_qz_649_key+0
	.byte		TEMPO , 182*mus_qz_649_tbs/2
	.byte		VOICE , 11
	.byte		PAN   , c_v+0
	.byte		VOL   , 70*mus_qz_649_mvl/mxv
	.byte		MOD   , 3
	.byte		LFOS  , 20
	.byte		LFODL , 19
	.byte		PRIO  , 64
	.byte	W72
	.byte		N02   , Cn3 , v008
	.byte	W03
	.byte		PAN   , c_v-41
	.byte		N02   , Dn3 , v012
	.byte	W03
	.byte		PAN   , c_v-34
	.byte		N02   , Ds3 , v020
	.byte	W03
	.byte		PAN   , c_v-27
	.byte		N02   , En3 , v032
	.byte	W03
	.byte		PAN   , c_v-20
	.byte		N02   , Fn3 , v040
	.byte	W03
	.byte		PAN   , c_v-13
	.byte		N02   , Gn3 , v048
	.byte	W03
	.byte		PAN   , c_v-6
	.byte		N02   , An3 , v068
	.byte	W03
	.byte		PAN   , c_v-3
	.byte		N02   , Bn3 , v080
	.byte	W03
mus_qz_649_1_B1:
	.byte		PAN   , c_v+0
	.byte		N68   , Cn4 , v092
	.byte	W54
	.byte		VOL   , 63*mus_qz_649_mvl/mxv
	.byte	W06
	.byte		        56*mus_qz_649_mvl/mxv
	.byte	W06
	.byte		        45*mus_qz_649_mvl/mxv
	.byte	W06
	.byte		        70*mus_qz_649_mvl/mxv
	.byte	N11
	.byte	W12
	.byte		N02   , Dn4 , v084
	.byte	W12
	.byte		        Dn4 , v060
	.byte	W03
	.byte		N05   , Ds4 , v088
	.byte	W21
	.byte		N23   , Fn4 , v100
	.byte	W24
	.byte		N02   , Ds4 , v068
	.byte	W12
	.byte		N20   , Dn4 , v088
	.byte	W24
	.byte		N76   , Cn4 , v088 , gtp1
	.byte	W12
	.byte	W48
	.byte		VOL   , 63*mus_qz_649_mvl/mxv
	.byte	W06
	.byte		        56*mus_qz_649_mvl/mxv
	.byte	W06
	.byte		        45*mus_qz_649_mvl/mxv
	.byte	W06
	.byte		        70*mus_qz_649_mvl/mxv
	.byte	W06
	.byte		N02   , Gs3 , v056
	.byte	W03
	.byte		N08   , An3 , v088
	.byte	W09
	.byte		N11   , Gn3 , v068
	.byte	W12
	.byte		        An3 , v088
	.byte	W12
	.byte		N56   , Fn3 , v076 , gtp3
	.byte	W42
	.byte		VOL   , 63*mus_qz_649_mvl/mxv
	.byte	W06
	.byte		        56*mus_qz_649_mvl/mxv
	.byte	W06
	.byte		        45*mus_qz_649_mvl/mxv
	.byte	W06
	.byte		        70*mus_qz_649_mvl/mxv
	.byte		N02   , Fn3 , v024
	.byte	W03
	.byte		        Gn3 , v044
	.byte	W03
	.byte		        Gs3 , v052
	.byte	W03
	.byte		        As3 , v064
	.byte	W03
	.byte		        Bn3 , v080
	.byte	W03
	.byte		        Cn4 , v092
	.byte	W03
	.byte		        Dn4 , v104
	.byte	W03
	.byte		        En4 , v120
	.byte	W03
	.byte		N68   , Fn4 , v076
	.byte	W54
	.byte		VOL   , 63*mus_qz_649_mvl/mxv
	.byte	W06
	.byte		        56*mus_qz_649_mvl/mxv
	.byte	W06
	.byte		        45*mus_qz_649_mvl/mxv
	.byte	W06
	.byte		        70*mus_qz_649_mvl/mxv
	.byte		N05   , Fn4 , v092
	.byte	W12
	.byte		N02   , Gn4 , v072
	.byte	W12
	.byte		        Gn4 , v064
	.byte	W03
	.byte		N05   , Gs4 , v088
	.byte	W21
	.byte		N23   , As4 , v096
	.byte	W24
	.byte		N02   , Gs4 , v084
	.byte	W12
	.byte		N20   , Gn4 , v072
	.byte	W24
	.byte		N56   , Fn4 , v084 , gtp3
	.byte	W12
	.byte	W30
	.byte		VOL   , 63*mus_qz_649_mvl/mxv
	.byte	W06
	.byte		        56*mus_qz_649_mvl/mxv
	.byte	W06
	.byte		        45*mus_qz_649_mvl/mxv
	.byte	W06
	.byte		        70*mus_qz_649_mvl/mxv
	.byte	W24
	.byte		N02   , Cs4 , v052
	.byte	W03
	.byte		N08   , Dn4 , v084
	.byte	W09
	.byte		N11   , Cn4 , v072
	.byte	W12
	.byte		        Dn4 , v084
	.byte	W12
	.byte		N56   , Fn4 , v092 , gtp3
	.byte	W42
	.byte		VOL   , 63*mus_qz_649_mvl/mxv
	.byte	W06
	.byte		        56*mus_qz_649_mvl/mxv
	.byte	W06
	.byte		        45*mus_qz_649_mvl/mxv
	.byte	W06
	.byte		        70*mus_qz_649_mvl/mxv
	.byte	W24
	.byte		PAN   , c_v-45
	.byte		N05   , Gn2 , v072
	.byte	W06
	.byte		        Fs2 , v060
	.byte	W06
	.byte		N23   , Cn3 , v076
	.byte	W24
	.byte		N11   , Bn2 , v064
	.byte	W12
	.byte		N02   , Ds3 , v060
	.byte	W03
	.byte		N20   , En3 , v084
	.byte	W21
	.byte		N11   , Ds3 , v076
	.byte	W12
	.byte		        En3 , v064
	.byte	W12
	.byte		        Cn4 , v080
	.byte	W12
	.byte		N02   , Cn4 , v060
	.byte	W12
	.byte		PAN   , c_v+46
	.byte		N11   , Cn4 , v032
	.byte	W12
	.byte		PAN   , c_v-34
	.byte		N02   , Cn4 , v016
	.byte	W12
	.byte		N11   , Cn4 , v012
	.byte	W12
	.byte		N02   , Cn4 , v004
	.byte	W12
	.byte		PAN   , c_v+32
	.byte	N11
	.byte	W12
	.byte	W06
	.byte		PAN   , c_v-45
	.byte		N02   , Gs4 , v048
	.byte	W03
	.byte		        An4 , v040
	.byte	W03
	.byte		N20   , As4 , v068
	.byte	W24
	.byte		N05   , Gs4 , v060
	.byte	W12
	.byte		        Gn4 , v052
	.byte	W06
	.byte		        Fn4 , v044
	.byte	W06
	.byte		        Dn4 , v060
	.byte	W12
	.byte		        Cn4 , v048
	.byte	W06
	.byte		        As3 , v040
	.byte	W06
	.byte		        Dn4 , v064
	.byte	W06
	.byte		N02   , En4 , v036
	.byte	W03
	.byte		        Fn4 , v028
	.byte	W03
	.byte		        Fs4 , v048
	.byte	W03
	.byte		        Gn4 , v036
	.byte	W03
	.byte		        Gs4 , v056
	.byte	W03
	.byte		        An4 , v044
	.byte	W03
	.byte		N08   , As4 , v060
	.byte	W12
	.byte		N02   , As4 , v048
	.byte	W12
	.byte		N08   , As4 , v020
	.byte	W12
	.byte		N02   , As4 , v004
	.byte	W60
	.byte		PAN   , c_v+0
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_649_1_B1
	.byte	FINE

/********************** Track  2 **********************/

mus_qz_649_2:
	.byte		KEYSH , mus_qz_649_key+0
	.byte		VOICE , 17
	.byte		PAN   , c_v+46
	.byte		VOL   , 59*mus_qz_649_mvl/mxv
	.byte		MOD   , 3
	.byte		BENDR , 6
	.byte		LFOS  , 20
	.byte		LFODL , 17
	.byte		PRIO  , 62
	.byte		BEND  , c_v-22
	.byte		N14   , Gs4 , v088
	.byte	W03
	.byte		BEND  , c_v-11
	.byte	W01
	.byte		        c_v+0
	.byte	W20
	.byte		N14   , Gs4 , v080
	.byte	W24
	.byte		BEND  , c_v-22
	.byte		N10   , Gs4 , v088
	.byte	W03
	.byte		BEND  , c_v-11
	.byte	W01
	.byte		        c_v+0
	.byte	W08
	.byte		N05   , Gs4 , v068
	.byte	W12
	.byte		BEND  , c_v-22
	.byte		N10   , Gs4 , v088
	.byte	W03
	.byte		BEND  , c_v-11
	.byte	W01
	.byte		        c_v+0
	.byte	W08
	.byte		N05   , Gs4 , v068
	.byte	W12
mus_qz_649_2_B1:
	.byte		VOICE , 17
	.byte	W96
	.byte	W96
	.byte		N08   , Cn5 , v080
	.byte	W12
	.byte		N11   , An4 , v072
	.byte	W12
	.byte		        As4 , v076
	.byte	W12
	.byte		N23   , Cn5
	.byte	W18
	.byte		BEND  , c_v-11
	.byte	W01
	.byte		        c_v-22
	.byte	W02
	.byte		        c_v-32
	.byte	W01
	.byte		        c_v-43
	.byte	W02
	.byte		        c_v+0
	.byte		N01   , Fn4 , v060
	.byte	W03
	.byte	N01
	.byte	W03
	.byte	N01
	.byte	W03
	.byte	N01
	.byte	W03
	.byte		N11   , An4 , v080
	.byte	W12
	.byte		        As4 , v076
	.byte	W12
	.byte		N02   , Cn5 , v084
	.byte	W12
	.byte		N05   , Cn5 , v076
	.byte	W12
	.byte		N11   , Bn4 , v084
	.byte	W12
	.byte		N23   , Cn5 , v080
	.byte	W18
	.byte		BEND  , c_v-11
	.byte	W01
	.byte		        c_v-22
	.byte	W02
	.byte		        c_v-32
	.byte	W01
	.byte		        c_v-43
	.byte	W02
	.byte		        c_v+0
	.byte		N02   , Gn4 , v060
	.byte	W03
	.byte		        Dn4
	.byte	W09
	.byte		        Dn3
	.byte	W06
	.byte	N02
	.byte	W06
	.byte	N02
	.byte	W06
	.byte	N03
	.byte	W06
	.byte		N11   , Dn3 , v088
	.byte	W12
	.byte		N01   , En2 , v064
	.byte	W12
	.byte	N01
	.byte	W12
	.byte		N11   , Dn3 , v080
	.byte	W12
	.byte		N01   , En2 , v064
	.byte	W12
	.byte	N01
	.byte	W12
	.byte		N11   , Cs3 , v088
	.byte	W12
	.byte		        Ds3
	.byte	W12
	.byte		N02   , Gn3 , v060
	.byte	W03
	.byte		N05   , Gs3 , v080
	.byte	W21
	.byte		N23   , As3 , v084
	.byte	W24
	.byte		N02   , Gs3 , v060
	.byte	W12
	.byte		N20   , Gn3 , v076
	.byte	W21
	.byte		N01   , Gs4 , v060
	.byte	W06
	.byte	N01
	.byte	W09
	.byte		N11   , Dn5 , v080
	.byte	W12
	.byte		N01   , Fn4 , v060
	.byte	W06
	.byte	N01
	.byte	W06
	.byte		N08   , Fn4 , v076
	.byte	W12
	.byte		N11   , Dn5 , v080
	.byte	W12
	.byte		N01   , Fn4 , v060
	.byte	W06
	.byte	N01
	.byte	W06
	.byte		N11   , Fn4 , v080
	.byte	W12
	.byte		        Cs5 , v084
	.byte	W12
	.byte		        Dn5 , v080
	.byte	W12
	.byte		N05   , Fn5
	.byte	W12
	.byte		N01   , As4 , v060
	.byte	W06
	.byte	N01
	.byte	W06
	.byte		N11   , En5 , v072
	.byte	W12
	.byte		        Fn5 , v068
	.byte	W12
	.byte		N01   , Fn5 , v060
	.byte	W12
	.byte	N01
	.byte	W06
	.byte	N01
	.byte	W06
	.byte		VOICE , 17
	.byte		N05   , Ds4 , v076
	.byte	W06
	.byte		        En4 , v068
	.byte	W06
	.byte		        Fn4 , v072
	.byte	W06
	.byte		        Fs4 , v068
	.byte	W06
	.byte		BEND  , c_v-22
	.byte		N68   , Gn4 , v080 , gtp3
	.byte	W03
	.byte		BEND  , c_v-11
	.byte	W01
	.byte		        c_v+0
	.byte	W68
	.byte		N17   , Fn4 , v072
	.byte	W18
	.byte		N02   , Cn4 , v060
	.byte	W03
	.byte		        Dn4
	.byte	W03
	.byte		PAN   , c_v+34
	.byte		N11   , En4 , v076
	.byte	W12
	.byte		N02   , En4 , v060
	.byte	W12
	.byte		PAN   , c_v-41
	.byte		N11   , En4 , v024
	.byte	W12
	.byte	N02
	.byte	W12
	.byte		PAN   , c_v+34
	.byte		N11   , En4 , v008
	.byte	W12
	.byte	N02
	.byte	W12
	.byte		PAN   , c_v-41
	.byte		N11   , En4 , v004
	.byte	W12
	.byte	N02
	.byte	W06
	.byte	N02
	.byte	W06
	.byte		PAN   , c_v+46
	.byte		BEND  , c_v-22
	.byte		N68   , Fn4 , v076 , gtp3
	.byte	W03
	.byte		BEND  , c_v-11
	.byte	W01
	.byte		        c_v+0
	.byte	W68
	.byte		N17   , Ds4
	.byte	W18
	.byte		N02   , As3 , v060
	.byte	W03
	.byte		        Cn4
	.byte	W03
	.byte		N11   , Dn4 , v076
	.byte	W12
	.byte	N01
	.byte	W12
	.byte		N11   , Dn4 , v024
	.byte	W12
	.byte	N01
	.byte	W12
	.byte		N11   , Fs2 , v100
	.byte	W12
	.byte		        Fn2 , v084
	.byte	W12
	.byte		        Ds2 , v092
	.byte	W12
	.byte		        Cs2 , v084
	.byte	W12
	.byte		BEND  , c_v-22
	.byte		N32   , Cn2 , v096
	.byte	W03
	.byte		BEND  , c_v-11
	.byte	W01
	.byte		        c_v+0
	.byte	W28
	.byte	W01
	.byte		N01   , Ds2 , v064
	.byte	W03
	.byte		BEND  , c_v-22
	.byte		N32   , Ds2 , v088
	.byte	W03
	.byte		BEND  , c_v-11
	.byte	W01
	.byte		        c_v+0
	.byte	W32
	.byte		N14   , Fn2 , v096
	.byte	W18
	.byte		N01   , Cn2 , v064
	.byte	W03
	.byte	N01
	.byte	W03
	.byte		BEND  , c_v-32
	.byte		N23   , Cn2 , v100
	.byte	W01
	.byte		BEND  , c_v-22
	.byte	W02
	.byte		        c_v-11
	.byte	W01
	.byte		        c_v+0
	.byte	W24
	.byte	W02
	.byte		N01   , Ds2 , v064
	.byte	W03
	.byte	N01
	.byte	W03
	.byte		BEND  , c_v-22
	.byte		N32   , Ds2 , v088
	.byte	W03
	.byte		BEND  , c_v-11
	.byte	W01
	.byte		        c_v+0
	.byte	W32
	.byte		N14   , Fs2 , v100
	.byte	W18
	.byte		N01   , Cn2 , v036
	.byte	N01
	.byte	W03
	.byte	N01
	.byte	N01
	.byte	W03
	.byte		BEND  , c_v-32
	.byte		N23   , Cn2 , v084
	.byte	W01
	.byte		BEND  , c_v-22
	.byte	W02
	.byte		        c_v-11
	.byte	W01
	.byte		        c_v+0
	.byte	W24
	.byte	W02
	.byte		N01   , Ds2 , v036
	.byte	W03
	.byte	N01
	.byte	W03
	.byte		BEND  , c_v-22
	.byte		N32   , Ds2 , v076
	.byte	W03
	.byte		BEND  , c_v-11
	.byte	W01
	.byte		        c_v+0
	.byte	W32
	.byte		N23   , Cs2 , v084
	.byte	W24
	.byte		N08   , Cn2
	.byte	W12
	.byte		        Ds2 , v072
	.byte	W12
	.byte		N01   , Ds2 , v056
	.byte	W12
	.byte		BEND  , c_v-22
	.byte		N32   , Ds2 , v084
	.byte	W03
	.byte		BEND  , c_v-11
	.byte	W01
	.byte		        c_v+0
	.byte	W32
	.byte		N01   , Ds3 , v060
	.byte	W06
	.byte	N01
	.byte	W06
	.byte	N01
	.byte	W06
	.byte	N01
	.byte	W06
	.byte	GOTO
	 ptrvalue	mus_qz_649_2_B1
	.byte	FINE

/********************** Track  3 **********************/

mus_qz_649_3:
	.byte		KEYSH , mus_qz_649_key+0
	.byte		VOICE , 17
	.byte		PAN   , c_v+26
	.byte		VOL   , 50*mus_qz_649_mvl/mxv
	.byte		MOD   , 10
	.byte		BENDR , 6
	.byte		LFOS  , 20
	.byte		LFODL , 19
	.byte		PRIO  , 50
	.byte		BEND  , c_v-22
	.byte		N14   , Ds4 , v080
	.byte	W03
	.byte		BEND  , c_v-11
	.byte	W01
	.byte		        c_v+0
	.byte	W20
	.byte		N14   , Ds4 , v068
	.byte	W24
	.byte		BEND  , c_v-22
	.byte		N09   , Ds4 , v076
	.byte	W03
	.byte		BEND  , c_v-11
	.byte	W01
	.byte		        c_v+0
	.byte	W08
	.byte		N05   , Ds4 , v072
	.byte	W12
	.byte		BEND  , c_v-22
	.byte		N09   , Ds4 , v076
	.byte	W03
	.byte		BEND  , c_v-11
	.byte	W01
	.byte		        c_v+0
	.byte	W08
	.byte	N05
	.byte	W12
mus_qz_649_3_B1:
	.byte	W96
	.byte	W96
	.byte		N08   , An4 , v084
	.byte	W12
	.byte		N11   , Fn4
	.byte	W12
	.byte		        Gn4
	.byte	W12
	.byte		N23   , An4
	.byte	W12
	.byte		BEND  , c_v-11
	.byte	W01
	.byte		        c_v-22
	.byte	W02
	.byte		        c_v-32
	.byte	W01
	.byte		        c_v-43
	.byte	W02
	.byte		        c_v+0
	.byte	W06
	.byte		N02   , Dn4
	.byte	W03
	.byte	N02
	.byte	W03
	.byte	N02
	.byte	W03
	.byte	N02
	.byte	W03
	.byte		N11   , Cn5
	.byte	W12
	.byte		        Dn5
	.byte	W12
	.byte		N02   , Ds5 , v108
	.byte	W12
	.byte		N05   , Ds5 , v084
	.byte	W12
	.byte		N11   , Dn5
	.byte	W12
	.byte		N23   , Ds5
	.byte	W12
	.byte		BEND  , c_v-11
	.byte	W01
	.byte		        c_v-22
	.byte	W02
	.byte		        c_v-32
	.byte	W01
	.byte		        c_v-43
	.byte	W02
	.byte		        c_v+0
	.byte	W06
	.byte		N02   , As4 , v060
	.byte	W03
	.byte		        Fn4
	.byte	W09
	.byte		        Fn3 , v100
	.byte	W06
	.byte	N02
	.byte	W06
	.byte	N02
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		VOICE , 17
	.byte		N11   , As2 , v088
	.byte	W12
	.byte		N02   , Cn2 , v060
	.byte	W12
	.byte	N02
	.byte	W12
	.byte		N11   , As2 , v088
	.byte	W12
	.byte		N02   , Cn2 , v060
	.byte	W12
	.byte	N02
	.byte	W12
	.byte		N11   , An2 , v088
	.byte	W12
	.byte		        As2
	.byte	W12
	.byte		N02   , En3 , v076
	.byte	W03
	.byte		N05   , Fn3
	.byte	W21
	.byte		N23   , Gn3
	.byte	W24
	.byte		N02   , Fn3
	.byte	W12
	.byte		N20   , Ds3
	.byte	W21
	.byte		N02   , Fn4 , v060
	.byte	W06
	.byte	N02
	.byte	W09
	.byte		N11   , As4 , v084
	.byte	W12
	.byte		N02   , Dn4 , v060
	.byte	W06
	.byte	N02
	.byte	W06
	.byte		N08   , Dn4 , v084
	.byte	W12
	.byte		N11   , As4
	.byte	W12
	.byte		N02   , Dn4 , v040
	.byte	W06
	.byte	N02
	.byte	W06
	.byte		N11   , Dn4 , v084
	.byte	W12
	.byte		        An4
	.byte	W12
	.byte		        As4
	.byte	W12
	.byte	N05
	.byte	W12
	.byte		N02   , Fn4 , v060
	.byte	W06
	.byte	N02
	.byte	W06
	.byte		N11   , An4 , v084
	.byte	W12
	.byte		        As4
	.byte	W12
	.byte	N02
	.byte	W12
	.byte		        As4 , v108
	.byte	W06
	.byte	N02
	.byte	W06
	.byte	N02
	.byte	W06
	.byte	N02
	.byte	W06
	.byte		N11   , As4 , v084
	.byte	W12
	.byte		VOICE , 17
	.byte		N05   , Cn4 , v076
	.byte	W06
	.byte		        Bn3
	.byte	W06
	.byte		N56   , Cn4 , v076 , gtp3
	.byte	W60
	.byte		N17   , As3
	.byte	W18
	.byte		N02   , En3 , v056
	.byte	W03
	.byte		        Fs3
	.byte	W03
	.byte		PAN   , c_v-22
	.byte		N11   , Gn3 , v076
	.byte	W12
	.byte	N05
	.byte	W12
	.byte		PAN   , c_v+41
	.byte		N11   , Gn3 , v024
	.byte	W12
	.byte	N05
	.byte	W12
	.byte		PAN   , c_v-22
	.byte		N11   , Gn3 , v008
	.byte	W12
	.byte	N05
	.byte	W12
	.byte		PAN   , c_v+25
	.byte	W12
	.byte	W12
	.byte		N05   , As3 , v076
	.byte	W06
	.byte		        An3
	.byte	W06
	.byte		N56   , As3 , v076 , gtp3
	.byte	W60
	.byte		N17   , Gs3
	.byte	W18
	.byte		N02   , Dn3 , v056
	.byte	W03
	.byte		        En3
	.byte	W03
	.byte		N11   , Fn3 , v076
	.byte	W12
	.byte	N05
	.byte	W12
	.byte		N11   , Fn3 , v024
	.byte	W12
	.byte	N05
	.byte	W60
	.byte		PAN   , c_v+54
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W72
	.byte		N01   , As2 , v060
	.byte	W06
	.byte	N01
	.byte	W06
	.byte	N01
	.byte	W06
	.byte	N01
	.byte	W06
	.byte	GOTO
	 ptrvalue	mus_qz_649_3_B1
	.byte	FINE

/********************** Track  4 **********************/

mus_qz_649_4:
	.byte		KEYSH , mus_qz_649_key+0
	.byte		VOICE , 17
	.byte		PAN   , c_v-40
	.byte		VOL   , 45*mus_qz_649_mvl/mxv
	.byte		MOD   , 2
	.byte		BENDR , 6
	.byte		LFOS  , 20
	.byte		LFODL , 19
	.byte		PRIO  , 58
	.byte		BEND  , c_v-22
	.byte		N14   , Gs1 , v080
	.byte		N14   , Gs2
	.byte	W03
	.byte		BEND  , c_v-11
	.byte	W01
	.byte		        c_v+0
	.byte	W20
	.byte		N14   , Gs1 , v068
	.byte		N14   , Gs2
	.byte	W24
	.byte		BEND  , c_v-22
	.byte		N09   , Gs1 , v076
	.byte		N09   , Gs2
	.byte	W03
	.byte		BEND  , c_v-11
	.byte	W01
	.byte		        c_v+0
	.byte	W08
	.byte		N05   , Gs1 , v068
	.byte		N05   , Gs2
	.byte	W12
	.byte		BEND  , c_v-22
	.byte		N09   , Gs1 , v076
	.byte		N09   , Gs2
	.byte	W03
	.byte		BEND  , c_v-11
	.byte	W01
	.byte		        c_v+0
	.byte	W08
	.byte		N05   , Gs1 , v068
	.byte		N05   , Gs2
	.byte	W12
mus_qz_649_4_B1:
	.byte		VOL   , 45*mus_qz_649_mvl/mxv
	.byte		N08   , Cn2 , v092
	.byte	W01
	.byte		        Fn2 , v084
	.byte	W11
	.byte		N02   , Cn2 , v076
	.byte	W01
	.byte		        Fn2 , v068
	.byte	W11
	.byte		        As1 , v076
	.byte	W01
	.byte		        Ds2 , v084
	.byte	W11
	.byte		N32   , Cn2 , v092
	.byte	W01
	.byte		        Fn2
	.byte	W32
	.byte	W03
	.byte		N11   , Cn2 , v084
	.byte	W01
	.byte		        Fn2
	.byte	W11
	.byte		N05   , Dn2
	.byte	W01
	.byte		        Gn2
	.byte	W11
	.byte		N08   , Ds2 , v092
	.byte	W01
	.byte		        Gs2
	.byte	W11
	.byte		N02   , Ds2 , v076
	.byte	W01
	.byte		        Gs2 , v068
	.byte	W05
	.byte		        Cn2 , v076
	.byte	W01
	.byte		        Fn2 , v068
	.byte	W05
	.byte		N23   , Dn2 , v084
	.byte	W01
	.byte		        As2 , v068
	.byte	W23
	.byte		N05   , Cn2 , v084
	.byte	W01
	.byte		        Gs2
	.byte	W11
	.byte		N23   , Dn2 , v092
	.byte	W01
	.byte		        Gn2
	.byte	W23
	.byte		N11   , En2 , v084
	.byte	W01
	.byte		        Bn2
	.byte	W11
	.byte		N08   , Fn2 , v092
	.byte	W01
	.byte		        Cn3
	.byte	W11
	.byte		N02   , Fn2 , v076
	.byte	W01
	.byte		        Cn3 , v068
	.byte	W11
	.byte		        Fn2 , v076
	.byte	W01
	.byte		        Cn3 , v068
	.byte	W11
	.byte		N11   , Fn2 , v092
	.byte	W01
	.byte		        Cn3
	.byte	W11
	.byte		N02   , Fn2 , v076
	.byte	W01
	.byte		        Cn3 , v084
	.byte	W11
	.byte		        Fn2 , v056
	.byte	W01
	.byte		        Cn3 , v060
	.byte	W11
	.byte		N11   , Cn2 , v084
	.byte	W01
	.byte		        Fn2
	.byte	W11
	.byte		        As1
	.byte	W01
	.byte		        Ds2
	.byte	W11
	.byte	W12
	.byte		N10   , Cn2 , v092
	.byte	W01
	.byte		        Fn2
	.byte	W11
	.byte		N14   , Fn2 , v084
	.byte	W01
	.byte		        An2
	.byte	W23
	.byte		N02   , Fn2 , v076
	.byte	W01
	.byte		        An2 , v068
	.byte	W05
	.byte		        Fn2 , v076
	.byte	W01
	.byte		        An2 , v068
	.byte	W05
	.byte		N23   , Cn2 , v084
	.byte	W01
	.byte		        Gs2
	.byte	W23
	.byte		N11   , Ds2
	.byte	W01
	.byte		        As2
	.byte	W11
	.byte		N08   , Fn2 , v100
	.byte	W01
	.byte		        As2 , v092
	.byte	W11
	.byte		N02   , Fn2 , v076
	.byte	W01
	.byte		        As2 , v068
	.byte	W11
	.byte		        Ds2 , v084
	.byte	W01
	.byte		        Gs2
	.byte	W11
	.byte		N20   , Fn2
	.byte	W01
	.byte		        As2
	.byte	W23
	.byte		N02   , Fn2 , v072
	.byte	W01
	.byte		        As2 , v060
	.byte	W05
	.byte		        Fn2 , v072
	.byte	W01
	.byte		        As2 , v060
	.byte	W05
	.byte		N11   , Fn2 , v084
	.byte	W01
	.byte		        As2
	.byte	W11
	.byte		        Gn2
	.byte	W01
	.byte		        Cn3
	.byte	W11
	.byte		N23   , Fn2 , v092
	.byte	W01
	.byte		        Cs3
	.byte	W23
	.byte		        Gn2 , v084
	.byte	W01
	.byte		        Ds3
	.byte	W23
	.byte		N11   , Fn2
	.byte	W01
	.byte		        Cs3
	.byte	W11
	.byte		N17   , Gn2
	.byte	W01
	.byte		        Cn3
	.byte	W23
	.byte		N02   , Gn2 , v076
	.byte	W01
	.byte		        Cn3 , v068
	.byte	W05
	.byte		        Gn2 , v076
	.byte	W01
	.byte		        Cn3 , v068
	.byte	W05
	.byte		N08   , Fn2 , v100
	.byte	W01
	.byte		        As2 , v092
	.byte	W11
	.byte		N02   , Fn2 , v076
	.byte	W01
	.byte		        As2 , v068
	.byte	W11
	.byte		N05   , Ds2 , v076
	.byte	W01
	.byte		        Gs2 , v084
	.byte	W11
	.byte		N20   , Fn2
	.byte	W01
	.byte		        As2
	.byte	W23
	.byte		N02   , Fn2 , v060
	.byte	W01
	.byte		        As2 , v048
	.byte	W11
	.byte		N11   , Fn2 , v084
	.byte	W01
	.byte		        As2 , v080
	.byte	W11
	.byte		        Gs2 , v084
	.byte	W01
	.byte		        Cn3 , v072
	.byte	W11
	.byte		N08   , Fn2 , v100
	.byte	W01
	.byte		        As2 , v092
	.byte	W11
	.byte		N02   , Fn2 , v076
	.byte	W01
	.byte		        As2 , v068
	.byte	W11
	.byte		        Ds2 , v076
	.byte		N05   , Gs2 , v084
	.byte	W12
	.byte		N11   , Fn2
	.byte	W01
	.byte		        As2
	.byte	W23
	.byte		        Fn2 , v064
	.byte	W01
	.byte		        As2
	.byte	W11
	.byte		        En2 , v084
	.byte	W01
	.byte		        An2
	.byte	W11
	.byte		        Fn2 , v080
	.byte	W01
	.byte		        As2
	.byte	W11
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W48
	.byte		N05   , Cn3 , v092
	.byte	W01
	.byte		N12   , Fs3 , v088
	.byte	W05
	.byte		N05   , As2 , v092
	.byte	W06
	.byte		N11   , Gs2
	.byte	W01
	.byte		N12   , Fn3 , v088
	.byte	W11
	.byte		N05   , As2 , v092
	.byte	W01
	.byte		N12   , Ds3 , v088
	.byte	W05
	.byte		N05   , Gs2 , v092
	.byte	W06
	.byte		        Fs2
	.byte	W01
	.byte		        Cs3 , v088
	.byte	W05
	.byte		N01   , Fn2 , v052
	.byte	N01
	.byte	W01
	.byte		        Cn3 , v060
	.byte	W02
	.byte		        Fn2 , v052
	.byte	N01
	.byte	W01
	.byte		        Cn3 , v060
	.byte	W02
	.byte		N32   , Fn2 , v092
	.byte	W01
	.byte		        Cn3 , v088 , gtp1
	.byte	W32
	.byte		N01   , Gs2 , v052
	.byte	W01
	.byte		        Ds3 , v060
	.byte	W02
	.byte		N32   , Gs2 , v092
	.byte	W01
	.byte		        Ds3 , v088 , gtp1
	.byte	W32
	.byte	W03
	.byte		N11   , As2 , v092
	.byte	W01
	.byte		        Fn3 , v088
	.byte	W17
	.byte		N01   , Fn2 , v052
	.byte	W01
	.byte		        Cn3
	.byte	W02
	.byte		        Fn2
	.byte	W01
	.byte		        Cn3
	.byte	W02
	.byte		N23   , Fn2 , v092
	.byte	W01
	.byte		N24   , Cn3 , v088
	.byte	W28
	.byte	W01
	.byte		N01   , Gs2 , v052
	.byte	W01
	.byte		        Ds3 , v060
	.byte	W02
	.byte		        Gs2 , v052
	.byte	W01
	.byte		        Ds3 , v060
	.byte	W02
	.byte		N32   , Gs2 , v092
	.byte	W01
	.byte		        Ds3 , v088 , gtp1
	.byte	W32
	.byte	W03
	.byte		N11   , Bn2 , v092
	.byte	W01
	.byte		N12   , Fs3 , v088
	.byte	W17
	.byte		N01   , Fn2 , v052
	.byte	N01
	.byte	W01
	.byte		        Cn3 , v060
	.byte	W02
	.byte		        Fn2 , v052
	.byte	N01
	.byte	W01
	.byte		        Cn3 , v060
	.byte	W02
	.byte		N23   , Fn2 , v092
	.byte	W01
	.byte		N24   , Cn3 , v088
	.byte	W28
	.byte	W01
	.byte		N01   , Gs2 , v052
	.byte	W01
	.byte		        Ds3 , v060
	.byte	W02
	.byte		        Gs2 , v052
	.byte	W01
	.byte		        Ds3 , v060
	.byte	W02
	.byte		N32   , Gs2 , v092
	.byte	W01
	.byte		        Ds3 , v088 , gtp1
	.byte	W32
	.byte	W03
	.byte		N20   , Fs2 , v092
	.byte	W01
	.byte		N21   , Cs3 , v088
	.byte	W23
	.byte		N11   , Fn2 , v092
	.byte	W01
	.byte		        Cn3 , v088
	.byte	W11
	.byte		N08   , Gs2 , v092
	.byte	W01
	.byte		N09   , Ds3 , v088
	.byte	W11
	.byte		N01   , Gs2 , v052
	.byte	W01
	.byte		        Ds3 , v088
	.byte	W11
	.byte		N32   , Gs2 , v092 , gtp2
	.byte	W01
	.byte		        Ds3 , v088 , gtp2
	.byte	W20
	.byte		VOL   , 41*mus_qz_649_mvl/mxv
	.byte	W03
	.byte		        36*mus_qz_649_mvl/mxv
	.byte	W03
	.byte		        29*mus_qz_649_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_649_mvl/mxv
	.byte	W03
	.byte		        23*mus_qz_649_mvl/mxv
	.byte	W03
	.byte		        45*mus_qz_649_mvl/mxv
	.byte		N01   , Gs2 , v032
	.byte	W01
	.byte		        Ds3 , v060
	.byte	W05
	.byte		        Gs2 , v032
	.byte	W01
	.byte		        Ds3 , v060
	.byte	W05
	.byte		        Gs2 , v032
	.byte	W01
	.byte		        Ds3 , v060
	.byte	W05
	.byte		        Gs2 , v032
	.byte	W01
	.byte		        Ds3 , v060
	.byte	W05
	.byte	GOTO
	 ptrvalue	mus_qz_649_4_B1
	.byte	FINE

/********************** Track  5 **********************/

mus_qz_649_5:
	.byte		KEYSH , mus_qz_649_key+0
	.byte		VOICE , 20
	.byte		PAN   , c_v-10
	.byte		VOL   , 70*mus_qz_649_mvl/mxv
	.byte		BENDR , 6
	.byte		PRIO  , 54
	.byte		BEND  , c_v-22
	.byte		N14   , Gs2 , v088
	.byte	W02
	.byte		BEND  , c_v-11
	.byte	W02
	.byte		        c_v+0
	.byte	W20
	.byte		N14   , Gs2 , v080
	.byte	W23
	.byte		BEND  , c_v-22
	.byte	W01
	.byte		N09   , Gs2 , v088
	.byte	W02
	.byte		BEND  , c_v-11
	.byte	W02
	.byte		        c_v+0
	.byte	W08
	.byte		N04   , Gs2 , v068
	.byte	W11
	.byte		BEND  , c_v-22
	.byte	W01
	.byte		N09   , Gs2 , v088
	.byte	W02
	.byte		BEND  , c_v-11
	.byte	W02
	.byte		        c_v+0
	.byte	W08
	.byte		N04   , Gs2 , v068
	.byte	W12
mus_qz_649_5_B1:
	.byte		N08   , Fn2 , v088
	.byte	W12
	.byte		N05   , Cn2 , v072
	.byte	W12
	.byte		N11   , Dn2 , v080
	.byte	W12
	.byte		N23   , Ds2 , v084
	.byte	W24
	.byte		N11   , Cn2 , v076
	.byte	W12
	.byte		        Dn2 , v064
	.byte	W12
	.byte		        Ds2 , v068
	.byte	W12
	.byte		N08   , Fn2 , v088
	.byte	W12
	.byte		N05   , Cn2 , v072
	.byte	W12
	.byte		N11   , Dn2 , v080
	.byte	W12
	.byte		N23   , Ds2 , v084
	.byte	W24
	.byte		N11   , Cn2
	.byte	W12
	.byte		        Dn2 , v072
	.byte	W12
	.byte		        Ds2 , v076
	.byte	W12
	.byte		N08   , Fn2 , v088
	.byte	W12
	.byte		N05   , Cn2 , v076
	.byte	W12
	.byte		N11   , Dn2 , v084
	.byte	W12
	.byte		N23   , Ds2 , v080
	.byte	W24
	.byte		N11   , Cn2 , v076
	.byte	W12
	.byte		        Dn2 , v088
	.byte	W12
	.byte		        Ds2 , v080
	.byte	W12
	.byte		N08   , Fn2 , v092
	.byte	W12
	.byte		N05   , Cn2 , v080
	.byte	W12
	.byte		N11   , Dn2 , v084
	.byte	W12
	.byte		N23   , Ds2 , v092
	.byte	W24
	.byte		N11   , Cn2 , v076
	.byte	W12
	.byte		        Dn2 , v096
	.byte	W12
	.byte		        Ds2 , v088
	.byte	W12
	.byte		N08   , As2 , v092
	.byte	W12
	.byte		N05   , Fn2 , v076
	.byte	W12
	.byte		N11   , Gn2 , v080
	.byte	W12
	.byte		N23   , Gs2 , v076
	.byte	W24
	.byte		N11   , Fn2 , v072
	.byte	W12
	.byte		        Gn2 , v080
	.byte	W12
	.byte		        Gs2 , v072
	.byte	W12
	.byte		N08   , As2 , v092
	.byte	W12
	.byte		N05   , Fn2 , v072
	.byte	W12
	.byte		N11   , Gn2 , v084
	.byte	W12
	.byte		N23   , Gs2 , v076
	.byte	W24
	.byte		N11   , Fn2 , v072
	.byte	W12
	.byte		        Gn2 , v080
	.byte	W12
	.byte		        Gs2 , v076
	.byte	W12
	.byte		N08   , As2 , v092
	.byte	W12
	.byte		N05   , Fn2 , v076
	.byte	W12
	.byte		N11   , Gn2 , v084
	.byte	W12
	.byte		N23   , Gs2 , v080
	.byte	W24
	.byte		N11   , Fn2 , v072
	.byte	W12
	.byte		        Gn2 , v084
	.byte	W12
	.byte		        Gs2 , v076
	.byte	W12
	.byte		N08   , As2 , v092
	.byte	W12
	.byte		N05   , Fn2 , v072
	.byte	W12
	.byte		N11   , Gn2 , v084
	.byte	W12
	.byte		N23   , Gs2 , v076
	.byte	W24
	.byte		N11   , Fn2 , v072
	.byte	W12
	.byte		        Gn2 , v088
	.byte	W12
	.byte		        Gs2 , v084
	.byte	W12
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W60
	.byte		        Cn3 , v088
	.byte	W12
	.byte		N08   , Gs2 , v076
	.byte	W12
	.byte		        Fs2 , v072
	.byte	W12
	.byte		N23   , Fn2 , v088
	.byte	W24
	.byte		N11   , Gn2 , v092
	.byte	W12
	.byte		N23   , Ds2 , v080
	.byte	W24
	.byte		N11   , Cn2
	.byte	W12
	.byte		        Dn2 , v068
	.byte	W12
	.byte		        Ds2 , v076
	.byte	W12
	.byte		N23   , Fn2 , v092
	.byte	W24
	.byte		N11   , Gn2 , v088
	.byte	W12
	.byte		N23   , Ds2 , v076
	.byte	W24
	.byte		N11   , Cn2
	.byte	W12
	.byte		        Dn2 , v068
	.byte	W12
	.byte		        Ds2 , v072
	.byte	W12
	.byte		N23   , Fn2 , v088
	.byte	W24
	.byte		N11   , Gn2 , v084
	.byte	W12
	.byte		N23   , Ds2 , v068
	.byte	W24
	.byte		N11   , Cn2 , v080
	.byte	W12
	.byte		        Dn2 , v068
	.byte	W12
	.byte		        Ds2 , v076
	.byte	W12
	.byte		        Gs2 , v088
	.byte	W12
	.byte		        Fs2 , v076
	.byte	W12
	.byte		        Fn2 , v080
	.byte	W12
	.byte		N23   , Ds2 , v076
	.byte	W15
	.byte		VOL   , 64*mus_qz_649_mvl/mxv
	.byte	W03
	.byte		        61*mus_qz_649_mvl/mxv
	.byte	W03
	.byte		        56*mus_qz_649_mvl/mxv
	.byte	W03
	.byte		        70*mus_qz_649_mvl/mxv
	.byte		N11   , As1 , v080
	.byte	W12
	.byte		        Cn2 , v088
	.byte	W12
	.byte		        Ds2 , v076
	.byte	W12
	.byte	GOTO
	 ptrvalue	mus_qz_649_5_B1
	.byte	FINE

/********************** Track  6 **********************/

mus_qz_649_6:
	.byte		KEYSH , mus_qz_649_key+0
	.byte		VOICE , 21
	.byte		PAN   , c_v+0
	.byte		VOL   , 61*mus_qz_649_mvl/mxv
	.byte		BENDR , 6
	.byte		PRIO  , 57
	.byte		BEND  , c_v-22
	.byte		N14   , Gs0 , v088
	.byte	W03
	.byte		BEND  , c_v-11
	.byte	W01
	.byte		        c_v+0
	.byte	W20
	.byte		N14   , Gs0 , v080
	.byte	W24
	.byte		BEND  , c_v-22
	.byte		N09   , Gs0 , v088
	.byte	W03
	.byte		BEND  , c_v-11
	.byte	W01
	.byte		        c_v+0
	.byte	W08
	.byte		N05   , Gs0 , v068
	.byte	W12
	.byte		BEND  , c_v-22
	.byte		N09   , Gs0 , v088
	.byte	W03
	.byte		BEND  , c_v-11
	.byte	W01
	.byte		        c_v+0
	.byte	W08
	.byte		N05   , Gs0 , v068
	.byte	W12
mus_qz_649_6_B1:
	.byte		N05   , Fn1 , v092
	.byte	W12
	.byte		N02   , Cn1 , v076
	.byte	W12
	.byte		N11   , Dn1 , v088
	.byte	W12
	.byte		N23   , Ds1 , v084
	.byte	W24
	.byte		N11   , Cn1 , v080
	.byte	W12
	.byte		        Dn1 , v092
	.byte	W12
	.byte		        Ds1 , v084
	.byte	W12
	.byte		N05   , Fn1 , v092
	.byte	W12
	.byte		N02   , Cn1 , v076
	.byte	W12
	.byte		N11   , Dn1 , v084
	.byte	W12
	.byte		N23   , Ds1 , v088
	.byte	W24
	.byte		N11   , Cn1
	.byte	W12
	.byte		        Dn1 , v080
	.byte	W12
	.byte		        Ds1 , v084
	.byte	W12
	.byte		N05   , Fn1 , v092
	.byte	W12
	.byte		N02   , Cn1 , v080
	.byte	W12
	.byte		N11   , Dn1 , v088
	.byte	W12
	.byte		N23   , Ds1 , v084
	.byte	W24
	.byte		N11   , Cn1 , v080
	.byte	W12
	.byte		        Dn1 , v092
	.byte	W12
	.byte		        Ds1 , v084
	.byte	W12
	.byte		N05   , Fn1 , v092
	.byte	W12
	.byte		N02   , Cn1 , v080
	.byte	W12
	.byte		N11   , Dn1 , v092
	.byte	W12
	.byte		N23   , Ds1 , v088
	.byte	W24
	.byte		N11   , Cn1 , v080
	.byte	W12
	.byte		        Dn1 , v092
	.byte	W12
	.byte		        Ds1 , v080
	.byte	W12
	.byte		        As0 , v076
	.byte	W12
	.byte		N02   , Fn0
	.byte	W12
	.byte		N11   , Gn0
	.byte	W12
	.byte		N23   , Gs0
	.byte	W24
	.byte		N11   , Fn0
	.byte	W12
	.byte		        Gn0
	.byte	W12
	.byte		        Gs0
	.byte	W12
	.byte		N05   , As0
	.byte	W12
	.byte		        Fn0
	.byte	W12
	.byte		N11   , Gn0
	.byte	W12
	.byte		N23   , Gs0
	.byte	W24
	.byte		N11   , Fn0
	.byte	W12
	.byte		        Gn0
	.byte	W12
	.byte		        Gs0
	.byte	W12
	.byte		        As0
	.byte	W12
	.byte		N05   , Fn0
	.byte	W12
	.byte		N11   , Gn0
	.byte	W12
	.byte		N23   , Gs0
	.byte	W24
	.byte		N11   , Fn0
	.byte	W12
	.byte		        Gn0
	.byte	W12
	.byte		        Gs0
	.byte	W12
	.byte		N05   , As0 , v100
	.byte	W12
	.byte		        Fn0 , v076
	.byte	W12
	.byte		N11   , Gn0
	.byte	W12
	.byte		N23   , Gs0
	.byte	W24
	.byte		N11   , Fn0
	.byte	W12
	.byte		        Gn0
	.byte	W12
	.byte		        Gs0
	.byte	W12
	.byte		N23   , Cn1
	.byte	W24
	.byte		N11   , Gn0
	.byte	W12
	.byte		N23   , As0
	.byte	W24
	.byte		N11   , Gn0
	.byte	W12
	.byte		        As0
	.byte	W12
	.byte		        Bn0
	.byte	W12
	.byte	N02
	.byte	W03
	.byte		N08   , Cn1
	.byte	W09
	.byte	N05
	.byte	W12
	.byte		N11   , Cn1 , v016
	.byte	W12
	.byte	N05
	.byte	W12
	.byte		N11   , Cn1 , v008
	.byte	W12
	.byte	N05
	.byte	W12
	.byte		N11   , Cn1 , v004
	.byte	W12
	.byte	N05
	.byte	W09
	.byte		N02   , An0 , v076
	.byte	W03
	.byte		N11   , As0
	.byte	W12
	.byte	N05
	.byte	W12
	.byte		N11   , Fn0
	.byte	W12
	.byte		N23   , Gs0
	.byte	W24
	.byte		N11   , Fn0
	.byte	W12
	.byte		        Gs0
	.byte	W12
	.byte		        An0
	.byte	W12
	.byte		        As0
	.byte	W12
	.byte	N05
	.byte	W36
	.byte		N11   , Fn1
	.byte	W12
	.byte		        Cn1 , v080
	.byte		N05   , Fn1 , v076
	.byte	W12
	.byte		N08   , Gs0 , v080
	.byte	W12
	.byte		        Fs0
	.byte	W12
mus_qz_649_6_000:
	.byte		N23   , Fn0 , v080
	.byte	W24
	.byte		N11   , Gn0
	.byte	W12
	.byte		N20   , Ds0
	.byte	W24
	.byte		N11   , Cn1
	.byte	W12
	.byte		        Dn1
	.byte	W12
	.byte		        Ds1
	.byte	W12
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_649_6_000
	.byte	PATT
	 ptrvalue	mus_qz_649_6_000
	.byte		N11   , Gs1 , v080
	.byte	W12
	.byte		        Fs1
	.byte	W12
	.byte		        Fn1
	.byte	W12
	.byte		N23   , Ds1
	.byte	W15
	.byte		VOL   , 56*mus_qz_649_mvl/mxv
	.byte	W03
	.byte		        53*mus_qz_649_mvl/mxv
	.byte	W03
	.byte		        49*mus_qz_649_mvl/mxv
	.byte	W03
	.byte		        60*mus_qz_649_mvl/mxv
	.byte		N11   , As0
	.byte	W12
	.byte		        Cn1 , v088
	.byte	W12
	.byte		        Ds1 , v076
	.byte	W12
	.byte	GOTO
	 ptrvalue	mus_qz_649_6_B1
	.byte	FINE

/********************** Track  7 **********************/

mus_qz_649_7:
	.byte		KEYSH , mus_qz_649_key+0
	.byte		VOICE , 11
	.byte		PAN   , c_v-41
	.byte		VOL   , 59*mus_qz_649_mvl/mxv
	.byte		MOD   , 3
	.byte		LFOS  , 20
	.byte		LFODL , 19
	.byte		PRIO  , 52
	.byte	W96
mus_qz_649_7_B1:
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		N05   , En2 , v072
	.byte	W06
	.byte		        Ds2 , v060
	.byte	W06
	.byte		N23   , Gn2 , v076
	.byte	W24
	.byte		N11   , Fs2 , v064
	.byte	W12
	.byte		N02   , Bn2 , v056
	.byte	W03
	.byte		N20   , Cn3 , v076
	.byte	W21
	.byte		N11   , Bn2 , v080
	.byte	W12
	.byte		        Cn3 , v072
	.byte	W12
	.byte		        Gn3 , v080
	.byte	W12
	.byte		N02   , Gn3 , v060
	.byte	W12
	.byte		N11   , Gn3 , v024
	.byte	W12
	.byte		N02   , Gn3 , v016
	.byte	W12
	.byte		N11   , Gn3 , v008
	.byte	W12
	.byte		N02   , Gn3 , v004
	.byte	W12
	.byte	N11
	.byte	W12
	.byte	W06
	.byte		N02   , Ds4 , v064
	.byte	W03
	.byte		        En4 , v052
	.byte	W03
	.byte		N20   , Fn4 , v076
	.byte	W24
	.byte		N05   , Dn4 , v072
	.byte	W12
	.byte		        Cn4 , v060
	.byte	W06
	.byte		        As3 , v052
	.byte	W06
	.byte		        Gs3 , v064
	.byte	W12
	.byte		        Gn3 , v056
	.byte	W06
	.byte		        Fn3 , v048
	.byte	W06
	.byte		        As3 , v068
	.byte	W06
	.byte		N02   , Bn3 , v060
	.byte	W03
	.byte		        Cn4 , v048
	.byte	W03
	.byte		        Cs4 , v052
	.byte	W03
	.byte		        Dn4 , v044
	.byte	W03
	.byte		        Ds4 , v048
	.byte	W03
	.byte		        En4 , v044
	.byte	W03
	.byte		N08   , Fn4 , v072
	.byte	W12
	.byte		N02   , Fn4 , v052
	.byte	W12
	.byte		N08   , Fn4 , v008
	.byte	W72
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_649_7_B1
	.byte	FINE

/********************** Track  8 **********************/

mus_qz_649_8:
	.byte		KEYSH , mus_qz_649_key+0
	.byte		VOICE , 39
	.byte		VOL   , 81*mus_qz_649_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 42
	.byte		N17   , En1 , v096
	.byte		N23   , Dn2 , v092
	.byte	W24
	.byte		N17   , En1 , v080
	.byte		N23   , En2 , v092
	.byte	W24
	.byte		N05   , En1 , v088
	.byte	W06
	.byte		        En1 , v060
	.byte	W06
	.byte		        En1 , v072
	.byte	W06
	.byte		        En1 , v064
	.byte	W06
	.byte		        Bn1 , v104
	.byte	W06
	.byte		        An1 , v092
	.byte	W06
	.byte		        Gn1 , v104
	.byte	W06
	.byte		        Fn1 , v092
	.byte	W06
mus_qz_649_8_B1:
	.byte		N28   , Cs2 , v108 , gtp1
	.byte	W24
	.byte		N11   , En1 , v084
	.byte	W48
	.byte	N11
	.byte	W12
	.byte		        As1 , v076
	.byte	W12
mus_qz_649_8_000:
	.byte	W24
	.byte		N11   , En1 , v084
	.byte	W48
	.byte	N11
	.byte	W24
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_649_8_000
	.byte	W24
	.byte		N11   , En1 , v084
	.byte	W48
	.byte	N11
	.byte	W12
	.byte		        As1 , v076
	.byte	W12
	.byte		N28   , En2 , v100 , gtp1
	.byte	W24
	.byte		N11   , En1 , v084
	.byte	W60
	.byte	N11
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_649_8_000
	.byte	PATT
	 ptrvalue	mus_qz_649_8_000
	.byte	W24
	.byte		N11   , En1 , v084
	.byte	W48
	.byte	N11
	.byte	W12
	.byte		        As1 , v088
	.byte	W10
	.byte		N01   , En1 , v056
	.byte	W02
	.byte		N20   , En1 , v076
	.byte		N23   , Cs2
	.byte	W36
	.byte		N11   , As1 , v048
	.byte	W24
	.byte		        As1 , v088
	.byte	W24
	.byte		        As1 , v056
	.byte	W10
	.byte		N01   , En1
	.byte	W02
	.byte		N11   , En1 , v076
	.byte		N24   , As2 , v076 , gtp2
	.byte	W12
	.byte		N11   , En1 , v100
	.byte		N11   , As1 , v088
	.byte	W12
	.byte		        En1 , v040
	.byte	W12
	.byte		        En1 , v004
	.byte	W36
	.byte		N23   , En2 , v076
	.byte	W24
	.byte		N20   , En1
	.byte		N23   , Cs2
	.byte	W36
	.byte		N11   , As1 , v060
	.byte	W24
	.byte		        As1 , v104
	.byte	W24
	.byte		        As1 , v068
	.byte	W10
	.byte		N01   , En1 , v056
	.byte	W02
	.byte		N11   , En1 , v076
	.byte		N11   , As2
	.byte	W12
	.byte		N23   , En1 , v088
	.byte		N01   , As1 , v060
	.byte	W24
	.byte		N05   , En1 , v068
	.byte	W06
	.byte		        En1 , v052
	.byte	W06
	.byte		N11   , En1 , v068
	.byte	W12
	.byte		        En1 , v088
	.byte	W12
	.byte		N05   , Bn1 , v108
	.byte	W06
	.byte		        An1 , v100
	.byte	W06
	.byte		        Gn1 , v104
	.byte	W06
	.byte		        Fn1 , v092
	.byte	W06
	.byte		N32   , Cs2 , v100 , gtp3
	.byte	W24
	.byte		N17   , En1 , v076
	.byte	W48
	.byte		N11   , En1 , v080
	.byte	W24
	.byte		N32   , En2 , v096 , gtp3
	.byte	W24
	.byte		N17   , En1 , v076
	.byte	W48
	.byte	N11
	.byte	W12
	.byte		N05   , En1 , v068
	.byte	W06
	.byte		        En1 , v056
	.byte	W06
	.byte		N32   , Dn2 , v100 , gtp3
	.byte	W24
	.byte		N17   , En1 , v076
	.byte	W48
	.byte		N11   , En1 , v080
	.byte	W24
	.byte		N23   , Ds2 , v096
	.byte	W24
	.byte		N17   , En1 , v076
	.byte	W12
	.byte		N23   , En2 , v100
	.byte	W36
	.byte		N05   , Bn1
	.byte	W06
	.byte		        An1 , v084
	.byte	W06
	.byte		        Gs1 , v092
	.byte	W06
	.byte		        Fn1 , v084
	.byte	W06
	.byte	GOTO
	 ptrvalue	mus_qz_649_8_B1
	.byte	FINE

/********************** Track  9 **********************/

mus_qz_649_9:
	.byte		KEYSH , mus_qz_649_key+0
	.byte		VOICE , 39
	.byte		VOL   , 81*mus_qz_649_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 40
	.byte		N11   , Bn0 , v092
	.byte	W24
	.byte		        Bn0 , v084
	.byte	W24
	.byte		        Bn0 , v092
	.byte	W24
	.byte		        Bn0 , v084
	.byte	W12
	.byte		        Bn0 , v076
	.byte	W12
mus_qz_649_9_B1:
	.byte		N11   , Bn0 , v092
	.byte	W12
	.byte		        Bn0 , v084
	.byte	W24
	.byte	N11
	.byte	W24
	.byte		        Bn0 , v092
	.byte	W36
	.byte		        Bn0 , v096
	.byte	W12
	.byte		        Bn0 , v084
	.byte	W24
	.byte		        Bn0 , v088
	.byte	W24
	.byte		        Bn0 , v080
	.byte	W24
	.byte	N11
	.byte	W12
	.byte		        Bn0 , v096
	.byte	W12
	.byte		        Bn0 , v084
	.byte	W24
	.byte		        Bn0 , v080
	.byte	W24
	.byte		        Bn0 , v084
	.byte	W36
	.byte		        Bn0 , v092
	.byte	W12
	.byte		        Bn0 , v084
	.byte	W24
	.byte	N11
	.byte	W24
	.byte	N11
	.byte	W36
	.byte		        Bn0 , v092
	.byte	W12
	.byte		        Bn0 , v080
	.byte	W24
	.byte		        Bn0 , v084
	.byte	W36
	.byte		        Bn0 , v076
	.byte	W24
	.byte		        Bn0 , v088
	.byte	W12
	.byte		        Bn0 , v080
	.byte	W24
	.byte		        Bn0 , v076
	.byte	W24
	.byte	N11
	.byte	W24
	.byte	N11
	.byte	W12
	.byte		        Bn0 , v092
	.byte	W12
	.byte		        Bn0 , v084
	.byte	W24
	.byte	N11
	.byte	W24
	.byte		        Bn0 , v080
	.byte	W24
	.byte		        Bn0 , v084
	.byte	W12
	.byte		        Bn0 , v092
	.byte	W12
	.byte		        Bn0 , v084
	.byte	W24
	.byte	N11
	.byte	W24
	.byte		        Bn0 , v088
	.byte	W36
	.byte	W12
	.byte		        Bn0 , v080
	.byte	W24
	.byte		        Bn0 , v076
	.byte	W24
	.byte		        Bn0 , v084
	.byte	W24
	.byte		        Bn0 , v080
	.byte	W12
	.byte		        Bn0 , v084
	.byte	W12
	.byte		        Bn0 , v092
	.byte	W48
	.byte		        Bn0 , v084
	.byte	W12
	.byte		        Bn0 , v096
	.byte	W24
	.byte	W12
	.byte		        Bn0 , v088
	.byte	W24
	.byte	N11
	.byte	W24
	.byte	N11
	.byte	W24
	.byte	N11
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		        Bn0 , v100
	.byte	W72
	.byte		        Bn0 , v088
	.byte	W12
	.byte		        Bn0 , v100
	.byte	W36
	.byte		        Bn0 , v092
	.byte	W24
	.byte		        Bn0 , v100
	.byte	W24
	.byte		N05   , Bn0 , v088
	.byte	W06
	.byte		        Bn0 , v072
	.byte	W06
	.byte		N11   , Bn0 , v088
	.byte	W36
	.byte		        Bn0 , v084
	.byte	W24
	.byte		        Bn0 , v092
	.byte	W24
	.byte		N05   , Bn0 , v088
	.byte	W06
	.byte		        Bn0 , v072
	.byte	W06
	.byte		N11   , Bn0 , v092
	.byte	W36
	.byte		        Bn0 , v084
	.byte	W24
	.byte		        Bn0 , v092
	.byte	W24
	.byte		N05   , Bn0 , v088
	.byte	W06
	.byte		        Bn0 , v072
	.byte	W06
	.byte		N11   , Bn0 , v092
	.byte	W36
	.byte		        Bn0 , v084
	.byte	W24
	.byte		        Bn0 , v092
	.byte	W23
	.byte		N05   , Bn0 , v088
	.byte	W06
	.byte		        Bn0 , v072
	.byte	W07
	.byte	GOTO
	 ptrvalue	mus_qz_649_9_B1
	.byte	FINE

/********************** Track 10 **********************/

mus_qz_649_10:
	.byte		KEYSH , mus_qz_649_key+0
	.byte		VOICE , 11
	.byte		PAN   , c_v+56
	.byte		VOL   , 16*mus_qz_649_mvl/mxv
	.byte		MOD   , 3
	.byte		LFOS  , 20
	.byte		LFODL , 19
	.byte		PRIO  , 34
	.byte	W84
	.byte		N02   , Cn3 , v024
	.byte	W03
	.byte		        Dn3 , v028
	.byte	W03
	.byte		        En3 , v036
	.byte	W03
	.byte		        Fn3 , v048
	.byte	W03
mus_qz_649_10_B1:
	.byte		N02   , Gn3 , v060
	.byte	W03
	.byte		        Gs3 , v072
	.byte	W03
	.byte		        An3 , v084
	.byte	W03
	.byte		        Bn3 , v096
	.byte	W03
	.byte		N68   , Cn4 , v104
	.byte	W48
	.byte		VOL   , 14*mus_qz_649_mvl/mxv
	.byte	W06
	.byte		        12*mus_qz_649_mvl/mxv
	.byte	W06
	.byte		        10*mus_qz_649_mvl/mxv
	.byte	W06
	.byte		        16*mus_qz_649_mvl/mxv
	.byte	W06
	.byte		N11   , Cn4 , v092
	.byte	W12
	.byte		N02   , Dn4 , v084
	.byte	W12
	.byte		        Dn4 , v060
	.byte	W03
	.byte		N05   , Ds4 , v088
	.byte	W21
	.byte		N23   , Fn4 , v100
	.byte	W24
	.byte		N02   , Ds4 , v068
	.byte	W12
	.byte		N20   , Dn4 , v088
	.byte	W24
	.byte		N76   , Cn4 , v088 , gtp1
	.byte	W54
	.byte		VOL   , 14*mus_qz_649_mvl/mxv
	.byte	W06
	.byte		        12*mus_qz_649_mvl/mxv
	.byte	W06
	.byte		        10*mus_qz_649_mvl/mxv
	.byte	W06
	.byte		        16*mus_qz_649_mvl/mxv
	.byte	W12
	.byte		N02   , Gs3 , v056
	.byte	W03
	.byte		N08   , An3 , v088
	.byte	W09
	.byte		N11   , Gn3 , v068
	.byte	W12
	.byte		        An3 , v088
	.byte	W12
	.byte		N56   , Fn3 , v076 , gtp3
	.byte	W36
	.byte		VOL   , 14*mus_qz_649_mvl/mxv
	.byte	W06
	.byte		        12*mus_qz_649_mvl/mxv
	.byte	W06
	.byte		        10*mus_qz_649_mvl/mxv
	.byte	W06
	.byte		        16*mus_qz_649_mvl/mxv
	.byte	W06
	.byte		N02   , Fn3 , v024
	.byte	W03
	.byte		        Gn3 , v028
	.byte	W03
	.byte		        Gs3 , v036
	.byte	W03
	.byte		        As3 , v048
	.byte	W03
	.byte		        Bn3 , v060
	.byte	W03
	.byte		        Cn4 , v072
	.byte	W03
	.byte		        Dn4 , v084
	.byte	W03
	.byte		        En4 , v096
	.byte	W03
	.byte		N68   , Fn4 , v076
	.byte	W48
	.byte		VOL   , 14*mus_qz_649_mvl/mxv
	.byte	W06
	.byte		        12*mus_qz_649_mvl/mxv
	.byte	W06
	.byte		        10*mus_qz_649_mvl/mxv
	.byte	W06
	.byte		        16*mus_qz_649_mvl/mxv
	.byte	W06
	.byte		N05   , Fn4 , v092
	.byte	W12
	.byte		N02   , Gn4 , v072
	.byte	W12
	.byte		        Gn4 , v064
	.byte	W03
	.byte		N05   , Gs4 , v088
	.byte	W21
	.byte		N23   , As4 , v096
	.byte	W24
	.byte		N02   , Gs4 , v084
	.byte	W12
	.byte		N20   , Gn4 , v072
	.byte	W24
	.byte		N56   , Fn4 , v084 , gtp3
	.byte	W36
	.byte		VOL   , 14*mus_qz_649_mvl/mxv
	.byte	W06
	.byte		        12*mus_qz_649_mvl/mxv
	.byte	W06
	.byte		        10*mus_qz_649_mvl/mxv
	.byte	W06
	.byte		        16*mus_qz_649_mvl/mxv
	.byte	W30
	.byte		N02   , Cs4 , v052
	.byte	W03
	.byte		N08   , Dn4 , v084
	.byte	W09
	.byte		N11   , Cn4 , v072
	.byte	W12
	.byte		        Dn4 , v084
	.byte	W12
	.byte		N56   , Fn4 , v092 , gtp3
	.byte	W36
	.byte		VOL   , 14*mus_qz_649_mvl/mxv
	.byte	W06
	.byte		        12*mus_qz_649_mvl/mxv
	.byte	W06
	.byte		        10*mus_qz_649_mvl/mxv
	.byte	W06
	.byte		        16*mus_qz_649_mvl/mxv
	.byte	W18
	.byte	W12
	.byte		N05   , Gn2 , v072
	.byte	W06
	.byte		        Fs2 , v060
	.byte	W06
	.byte		N23   , Cn3 , v076
	.byte	W24
	.byte		N11   , Bn2 , v064
	.byte	W12
	.byte		N02   , Ds3 , v060
	.byte	W03
	.byte		N20   , En3 , v084
	.byte	W21
	.byte		N11   , Ds3 , v076
	.byte	W12
	.byte		        En3 , v064
	.byte	W12
	.byte		        Cn4 , v080
	.byte	W12
	.byte		N02   , Cn4 , v060
	.byte	W12
	.byte		N11   , Cn4 , v032
	.byte	W12
	.byte		N02   , Cn4 , v016
	.byte	W12
	.byte		N11   , Cn4 , v012
	.byte	W12
	.byte		N02   , Cn4 , v004
	.byte	W12
	.byte	N11
	.byte	W12
	.byte	W06
	.byte		N02   , Gs4 , v052
	.byte	W03
	.byte		        An4 , v044
	.byte	W03
	.byte		N20   , As4 , v072
	.byte	W24
	.byte		N05   , Gs4 , v064
	.byte	W12
	.byte		        Gn4 , v056
	.byte	W06
	.byte		        Fn4 , v048
	.byte	W06
	.byte		        Dn4 , v064
	.byte	W12
	.byte		        Cn4 , v052
	.byte	W06
	.byte		        As3 , v044
	.byte	W06
	.byte		        Dn4 , v068
	.byte	W06
	.byte		N02   , En4 , v040
	.byte	W03
	.byte		        Fn4 , v032
	.byte	W03
	.byte		        Fs4 , v052
	.byte	W03
	.byte		        Gn4 , v044
	.byte	W03
	.byte		        Gs4 , v060
	.byte	W03
	.byte		        An4 , v048
	.byte	W03
	.byte		N08   , As4 , v068
	.byte	W12
	.byte		N02   , As4 , v056
	.byte	W12
	.byte		N08   , As4 , v024
	.byte	W12
	.byte		N02   , As4 , v008
	.byte	W48
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_649_10_B1
	.byte	FINE

/********************** Track 11 **********************/

mus_qz_649_11:
	.byte		KEYSH , mus_qz_649_key+0
	.byte		VOICE , 17
	.byte		PAN   , c_v-59
	.byte		VOL   , 18*mus_qz_649_mvl/mxv
	.byte		MOD   , 3
	.byte		BENDR , 6
	.byte		LFOS  , 20
	.byte		LFODL , 17
	.byte		PRIO  , 36
	.byte	W06
	.byte		BEND  , c_v-22
	.byte		N14   , Gs4 , v088
	.byte	W03
	.byte		BEND  , c_v-11
	.byte	W01
	.byte		        c_v+0
	.byte	W20
	.byte		PAN   , c_v+22
	.byte		N14   , Gs4 , v080
	.byte	W24
	.byte		PAN   , c_v-29
	.byte		BEND  , c_v-22
	.byte		N09   , Gs4 , v088
	.byte	W03
	.byte		BEND  , c_v-11
	.byte	W01
	.byte		        c_v+0
	.byte	W08
	.byte		N02   , Gs4 , v060
	.byte	W12
	.byte		PAN   , c_v-52
	.byte		BEND  , c_v-22
	.byte		N09   , Gs4 , v088
	.byte	W03
	.byte		BEND  , c_v-11
	.byte	W01
	.byte		        c_v+0
	.byte	W08
	.byte		N02   , Gs4 , v064
	.byte	W06
mus_qz_649_11_B1:
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
	.byte	W12
	.byte		VOL   , 18*mus_qz_649_mvl/mxv
	.byte	W48
	.byte		N11   , Fs2 , v100
	.byte	W12
	.byte		        Fn2 , v084
	.byte	W12
	.byte		        Ds2 , v092
	.byte	W12
	.byte		BEND  , c_v-22
	.byte		N11   , Cs2 , v084
	.byte	W03
	.byte		BEND  , c_v-11
	.byte	W01
	.byte		        c_v+0
	.byte	W08
	.byte		N32   , Cn2 , v096
	.byte	W24
	.byte		BEND  , c_v-22
	.byte	W03
	.byte		        c_v-11
	.byte	W01
	.byte		        c_v+0
	.byte	W05
	.byte		N01   , Ds2 , v064
	.byte	W03
	.byte		N32   , Ds2 , v088
	.byte	W36
	.byte		N14   , Fn2 , v096
	.byte	W12
	.byte		BEND  , c_v-32
	.byte	W01
	.byte		        c_v-22
	.byte	W02
	.byte		        c_v-11
	.byte	W01
	.byte		        c_v+0
	.byte	W02
	.byte		N01   , Cn2 , v064
	.byte	W03
	.byte	N01
	.byte	W03
	.byte		N23   , Cn2 , v100
	.byte	W24
	.byte		BEND  , c_v-22
	.byte	W03
	.byte		        c_v-11
	.byte	W01
	.byte		        c_v+0
	.byte	W02
	.byte		N01   , Ds2 , v064
	.byte	W03
	.byte	N01
	.byte	W03
	.byte		N32   , Ds2 , v088
	.byte	W36
	.byte		N14   , Fs2 , v100
	.byte	W12
	.byte		BEND  , c_v-32
	.byte	W01
	.byte		        c_v-22
	.byte	W02
	.byte		        c_v-11
	.byte	W01
	.byte		        c_v+0
	.byte	W02
	.byte		N01   , Cn2 , v036
	.byte	N01
	.byte	W03
	.byte	N01
	.byte	N01
	.byte	W03
	.byte		N23   , Cn2 , v084
	.byte	W24
	.byte		BEND  , c_v-22
	.byte	W03
	.byte		        c_v-11
	.byte	W01
	.byte		        c_v+0
	.byte	W02
	.byte		N01   , Ds2 , v036
	.byte	W03
	.byte	N01
	.byte	W03
	.byte		N32   , Ds2 , v076
	.byte	W36
	.byte		N23   , Cs2 , v084
	.byte	W12
	.byte	W12
	.byte		N08   , Cn2
	.byte	W12
	.byte		        Ds2 , v072
	.byte	W12
	.byte		BEND  , c_v-22
	.byte		N01   , Ds2 , v056
	.byte	W03
	.byte		BEND  , c_v-11
	.byte	W01
	.byte		        c_v+0
	.byte	W08
	.byte		N44   , Ds2 , v084 , gtp3
	.byte	W24
	.byte	W03
	.byte		VOL   , 17*mus_qz_649_mvl/mxv
	.byte	W03
	.byte		        16*mus_qz_649_mvl/mxv
	.byte	W03
	.byte		        15*mus_qz_649_mvl/mxv
	.byte	W03
	.byte		        13*mus_qz_649_mvl/mxv
	.byte	W03
	.byte		        11*mus_qz_649_mvl/mxv
	.byte	W03
	.byte		        11*mus_qz_649_mvl/mxv
	.byte	W03
	.byte		        9*mus_qz_649_mvl/mxv
	.byte	W02
	.byte		        18*mus_qz_649_mvl/mxv
	.byte	W01
	.byte	GOTO
	 ptrvalue	mus_qz_649_11_B1
	.byte	FINE

/******************************************************/
	.p2align	2

mus_qz_649:
	.byte	11	/* NumTrks */
	.byte	0	/* NumBlks */
	.byte	mus_qz_649_pri	/* Priority */
	.byte	mus_qz_649_rev	/* Reverb. */
space64 4
	ptrvalue	mus_qz_649_grp

	ptrvalue	mus_qz_649_1
	ptrvalue	mus_qz_649_2
	ptrvalue	mus_qz_649_3
	ptrvalue	mus_qz_649_4
	ptrvalue	mus_qz_649_5
	ptrvalue	mus_qz_649_6
	ptrvalue	mus_qz_649_7
	ptrvalue	mus_qz_649_8
	ptrvalue	mus_qz_649_9
	ptrvalue	mus_qz_649_10
	ptrvalue	mus_qz_649_11

	.end
