	.include "asm/macros/bit_width.inc"

	.include "MPlayDef.s"

/* Moemon Quetzal song 627, disassembled from the ROM by tools/quetzal_rip/songs.py.
   "org" comments give the ROM address of the bytes that follow. */

	.equ	mus_qz_627_grp, voicegroup_qz_songs
	.equ	mus_qz_627_pri, 0
	.equ	mus_qz_627_rev, reverb_set+0
	.equ	mus_qz_627_mvl, 127
	.equ	mus_qz_627_key, 0
	.equ	mus_qz_627_tbs, 1
	.equ	mus_qz_627_exg, 1
	.equ	mus_qz_627_cmp, 1

	.section .rodata
	.global	mus_qz_627
	.p2align	2
/* org 0x0967F1E0 */

/********************** Track  1 **********************/

mus_qz_627_1:
	.byte		KEYSH , mus_qz_627_key+0
	.byte		TEMPO , 138*mus_qz_627_tbs/2
	.byte		VOICE , 13
	.byte		VOL   , 53*mus_qz_627_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte	W54
	.byte	W96
	.byte	W96
	.byte	W96
mus_qz_627_1_B1:
	.byte		VOL   , 20*mus_qz_627_mvl/mxv
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		        22*mus_qz_627_mvl/mxv
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
	.byte		N05   , An3 , v088
	.byte	W06
	.byte		        As3
	.byte	W06
	.byte		N44   , Cn4 , v088 , gtp3
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_627_1_B1
	.byte	FINE

/********************** Track  2 **********************/

mus_qz_627_2:
	.byte		KEYSH , mus_qz_627_key+0
	.byte		VOICE , 6
	.byte		VOL   , 50*mus_qz_627_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte	W54
	.byte	W96
	.byte	W96
	.byte		VOL   , 63*mus_qz_627_mvl/mxv
	.byte	W72
	.byte		VOICE , 6
	.byte		PAN   , c_v+52
	.byte	W18
	.byte		N02   , Dn4 , v088
	.byte	W03
	.byte		        En4
	.byte	W03
mus_qz_627_2_B1:
	.byte		N02   , Fn4 , v016
	.byte	W03
	.byte		        Fn4 , v020
	.byte	W03
	.byte		        Fn4 , v028
	.byte	W03
	.byte		        Fn4 , v036
	.byte	W03
	.byte		        Fn4 , v048
	.byte	W03
	.byte		        Fn4 , v084
	.byte	W03
	.byte		        Fn4 , v120
	.byte	W03
	.byte		        Fn4 , v088
	.byte	W03
	.byte		        Fn4 , v120
	.byte	W03
	.byte		        Fn4 , v088
	.byte	W03
	.byte		        Fn4 , v120
	.byte	W03
	.byte		        Fn4 , v088
	.byte	W03
	.byte		N05   , En4
	.byte	W12
	.byte		        Fn4
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        En4
	.byte	W12
	.byte		N11   , Fn4
	.byte	W12
	.byte		N05   , Gn4
	.byte	W12
	.byte		N02   , An4
	.byte	W03
	.byte		        An4 , v028
	.byte	W03
	.byte		        An4 , v056
	.byte	W03
	.byte		        An4 , v024
	.byte	W03
	.byte		        An4 , v088
	.byte	W03
	.byte		        An4 , v032
	.byte	W03
	.byte		        An4 , v056
	.byte	W03
	.byte		        An4 , v032
	.byte	W03
	.byte		N05   , An4 , v092
	.byte	W12
	.byte		N11   , Fn4 , v088
	.byte	W12
	.byte		N02   , Cn4
	.byte	W03
	.byte		        Cn4 , v028
	.byte	W03
	.byte		        Cn4 , v056
	.byte	W03
	.byte		        Cn4 , v024
	.byte	W03
	.byte		        Cn4 , v088
	.byte	W03
	.byte		        Cn4 , v032
	.byte	W03
	.byte		        Cn4 , v056
	.byte	W03
	.byte		        Cn4 , v032
	.byte	W03
	.byte		N05   , Cn4 , v092
	.byte	W12
	.byte		N11   , Cs4 , v088
	.byte	W12
	.byte		N02   , Dn4 , v016
	.byte	W03
	.byte		        Dn4 , v020
	.byte	W03
	.byte		        Dn4 , v024
	.byte	W03
	.byte		        Dn4 , v032
	.byte	W03
	.byte		        Dn4 , v044
	.byte	W03
	.byte		        Dn4 , v056
	.byte	W03
	.byte		        Dn4 , v072
	.byte	W03
	.byte		        Dn4 , v080
	.byte	W03
	.byte		        Dn4 , v100
	.byte	W03
	.byte		        Dn4 , v112
	.byte	W03
	.byte	N02
	.byte	W03
	.byte	N02
	.byte	W03
	.byte		N05   , Cs4 , v088
	.byte	W12
	.byte		        Dn4
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Cs4
	.byte	W12
	.byte		N11   , Dn4
	.byte	W12
	.byte		N05   , En4
	.byte	W12
	.byte		N02   , Fn4 , v104
	.byte	W03
	.byte		        Fn4 , v028
	.byte	W03
	.byte		        Fn4 , v056
	.byte	W03
	.byte		        Fn4 , v024
	.byte	W03
	.byte		        Fn4 , v100
	.byte	W03
	.byte		        Fn4 , v032
	.byte	W03
	.byte		        Fn4 , v056
	.byte	W03
	.byte		        Fn4 , v032
	.byte	W03
	.byte		N05   , Fn4 , v104
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Dn4 , v088
	.byte	W12
	.byte		N02   , An3 , v104
	.byte	W03
	.byte		        An3 , v028
	.byte	W03
	.byte		        An3 , v056
	.byte	W03
	.byte		        An3 , v024
	.byte	W03
	.byte		        An3 , v100
	.byte	W03
	.byte		        An3 , v060
	.byte	W03
	.byte		        An3 , v064
	.byte	W03
	.byte	N02
	.byte	W03
	.byte		N05   , An3 , v092
	.byte	W06
	.byte		        An3 , v080
	.byte	W06
	.byte		N11   , Gn3 , v088
	.byte	W12
	.byte	W12
	.byte		N02   , An3
	.byte	W03
	.byte	N05
	.byte	W09
	.byte		N11   , As3
	.byte	W12
	.byte		N05   , Dn4
	.byte	W12
	.byte		        Bn3 , v100
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        Cn4 , v100
	.byte	W06
	.byte		        Cn4 , v056
	.byte	W06
	.byte		N02   , As3 , v088
	.byte	W03
	.byte	N02
	.byte	W03
	.byte		N05   , As3 , v076
	.byte	W06
	.byte		        As3 , v104
	.byte	W06
	.byte		        As3 , v080
	.byte	W06
	.byte	W12
	.byte		N02   , Bn3 , v088
	.byte	W03
	.byte	N05
	.byte	W09
	.byte		N11   , Cn4
	.byte	W12
	.byte		N05   , En4
	.byte	W12
	.byte		        Dn4 , v100
	.byte	W06
	.byte		        Dn4 , v068
	.byte	W06
	.byte		        Dn4 , v104
	.byte	W06
	.byte		        Dn4 , v076
	.byte	W06
	.byte		        Cn4 , v112
	.byte	W06
	.byte		        Cn4 , v076
	.byte	W06
	.byte		        Cn4 , v112
	.byte	W06
	.byte		N02   , Cn4 , v080
	.byte	W03
	.byte	N02
	.byte	W03
	.byte	W15
	.byte		        Cn4 , v088
	.byte	W03
	.byte		N05   , Cs4
	.byte	W06
	.byte		N11   , Dn4
	.byte	W12
	.byte		N05   , Fn4
	.byte	W12
	.byte		        Gn4 , v092
	.byte	W06
	.byte		        Gn4 , v088
	.byte	W06
	.byte		        Fn4
	.byte	W12
	.byte		N11   , En4
	.byte	W12
	.byte		N05   , Fn4
	.byte	W12
	.byte		PAN   , c_v+48
	.byte		N05   , Gn4
	.byte	W12
	.byte		        Cn5 , v080
	.byte	W06
	.byte		N02   , Dn5
	.byte	W03
	.byte		        Dn5 , v088
	.byte	W03
	.byte		PAN   , c_v+38
	.byte		N05   , Cn5 , v080
	.byte	W06
	.byte		        As4
	.byte	W06
	.byte		        An4
	.byte	W06
	.byte		        Gn4
	.byte	W06
	.byte		PAN   , c_v+23
	.byte		N02   , As4
	.byte	W03
	.byte		        As4 , v088
	.byte	W03
	.byte		N05   , An4 , v080
	.byte	W06
	.byte		PAN   , c_v+9
	.byte		N05   , Gn4
	.byte	W06
	.byte		N02   , Fn4
	.byte	W03
	.byte		        Fn4 , v088
	.byte	W03
	.byte		PAN   , c_v-25
	.byte		N05   , Gn4 , v092
	.byte	W06
	.byte		        Fn4 , v072
	.byte	W06
	.byte		        En4 , v084
	.byte	W06
	.byte		        Cn4 , v064
	.byte	W06
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		PAN   , c_v-25
	.byte		N05   , An3 , v088
	.byte	W12
	.byte		        Fn3
	.byte	W12
	.byte		        Cn4
	.byte	W12
	.byte		        Fn3
	.byte	W12
	.byte		        An3
	.byte	W06
	.byte		        An3 , v052
	.byte	W06
	.byte		        Fn3 , v088
	.byte	W12
	.byte		        Cn4
	.byte	W12
	.byte		        Fn3
	.byte	W12
	.byte		        An3
	.byte	W12
	.byte		        Fn3
	.byte	W12
	.byte		        Cn4
	.byte	W12
	.byte		        Fn3
	.byte	W24
	.byte		        Cn3
	.byte	W12
	.byte		VOICE , 6
	.byte		PAN   , c_v+36
	.byte		        c_v+52
	.byte		N05   , Fn3
	.byte	W18
	.byte		N02   , Dn4
	.byte	W03
	.byte		        En4
	.byte	W03
	.byte	GOTO
	 ptrvalue	mus_qz_627_2_B1
	.byte	FINE

/********************** Track  3 **********************/

mus_qz_627_3:
	.byte		KEYSH , mus_qz_627_key+0
	.byte		VOICE , 13
	.byte		VOL   , 53*mus_qz_627_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		BENDR , 6
	.byte		PRIO  , 64
	.byte		N02   , Dn4 , v088
	.byte	W03
	.byte		        En4
	.byte	W03
	.byte		VOL   , 50*mus_qz_627_mvl/mxv
	.byte		PAN   , c_v+37
	.byte		BEND  , c_v+0
	.byte		N11   , Fn4
	.byte	W12
	.byte		N05   , En4
	.byte	W12
	.byte		N11   , Dn4
	.byte	W12
	.byte		N05   , As3
	.byte	W12
	.byte		N40   , Cn4 , v088 , gtp1
	.byte	W42
	.byte		N02   , Gn3
	.byte	W03
	.byte		        An3
	.byte	W03
	.byte		N11   , As3
	.byte	W12
	.byte		N05   , An3
	.byte	W12
	.byte		N11   , Gn3
	.byte	W12
	.byte		        En3
	.byte	W12
	.byte		N92   , Fn3 , v088 , gtp3
	.byte	W96
	.byte	N05
	.byte	W12
	.byte		        An3
	.byte	W09
	.byte		N11   , Cn2
	.byte	W15
	.byte		N05   , Cn3
	.byte	W24
	.byte		        Cs3
	.byte	W24
	.byte		        Cn2 , v072
	.byte	W06
	.byte		PAN   , c_v+31
	.byte		N02   , Dn3
	.byte	W03
	.byte		VOL   , 35*mus_qz_627_mvl/mxv
	.byte		N02   , En3
	.byte	W03
mus_qz_627_3_B1:
	.byte		VOL   , 27*mus_qz_627_mvl/mxv
	.byte		N32   , Fn3 , v104 , gtp3
	.byte	W12
	.byte		VOL   , 51*mus_qz_627_mvl/mxv
	.byte	W12
	.byte		        80*mus_qz_627_mvl/mxv
	.byte	W12
	.byte		        50*mus_qz_627_mvl/mxv
	.byte		N05   , En3
	.byte	W12
	.byte		N11   , Fn3
	.byte	W12
	.byte		N05   , En3
	.byte	W12
	.byte		N11   , Fn3
	.byte	W12
	.byte		N05   , Gn3
	.byte	W12
	.byte		VOL   , 23*mus_qz_627_mvl/mxv
	.byte		N02   , Gs3
	.byte	W03
	.byte		N32   , An3
	.byte	W09
	.byte		VOL   , 51*mus_qz_627_mvl/mxv
	.byte	W12
	.byte		        64*mus_qz_627_mvl/mxv
	.byte	W12
	.byte		        50*mus_qz_627_mvl/mxv
	.byte		N11   , Fn3
	.byte	W12
	.byte		N23   , Cn3
	.byte	W36
	.byte		N11   , Cs3
	.byte	W12
	.byte		VOL   , 23*mus_qz_627_mvl/mxv
	.byte		N32   , Dn3 , v104 , gtp3
	.byte	W12
	.byte		VOL   , 57*mus_qz_627_mvl/mxv
	.byte	W06
	.byte		        69*mus_qz_627_mvl/mxv
	.byte	W06
	.byte		        80*mus_qz_627_mvl/mxv
	.byte	W12
	.byte		        50*mus_qz_627_mvl/mxv
	.byte		N05   , Cs3
	.byte	W12
	.byte		N11   , Dn3
	.byte	W12
	.byte		N05   , Cs3
	.byte	W12
	.byte		N11   , Dn3
	.byte	W12
	.byte		N05   , En3
	.byte	W12
	.byte		VOL   , 23*mus_qz_627_mvl/mxv
	.byte		N32   , Fn3 , v104 , gtp3
	.byte	W12
	.byte		VOL   , 57*mus_qz_627_mvl/mxv
	.byte	W06
	.byte		        69*mus_qz_627_mvl/mxv
	.byte	W06
	.byte		        80*mus_qz_627_mvl/mxv
	.byte	W12
	.byte		        50*mus_qz_627_mvl/mxv
	.byte		N05   , Dn3
	.byte	W12
	.byte		N32   , An2 , v104 , gtp3
	.byte	W36
	.byte		N11   , Gn2
	.byte	W12
	.byte	W12
	.byte		N02   , An2
	.byte	W03
	.byte	N05
	.byte	W09
	.byte		N11   , As2
	.byte	W12
	.byte		N05   , Dn3
	.byte	W12
	.byte		N23   , Cn3
	.byte	W24
	.byte		        As2
	.byte	W24
	.byte	W12
	.byte		N02   , Bn2
	.byte	W03
	.byte	N05
	.byte	W09
	.byte		N11   , Cn3
	.byte	W12
	.byte		N05   , En3
	.byte	W12
	.byte		N23   , Dn3
	.byte	W24
	.byte		        Cn3
	.byte	W24
	.byte	W15
	.byte	N02
	.byte	W03
	.byte		N05   , Cs3
	.byte	W06
	.byte		N11   , Dn3
	.byte	W12
	.byte		N05   , Fn3
	.byte	W12
	.byte		N11   , Gn3
	.byte	W12
	.byte		N05   , Fn3
	.byte	W12
	.byte		N11   , En3
	.byte	W12
	.byte		N05   , Fn3
	.byte	W12
	.byte		        Gn3
	.byte	W24
	.byte		N23   , Cn3
	.byte	W24
	.byte		        Dn3
	.byte	W24
	.byte		        En3
	.byte	W24
	.byte		VOL   , 30*mus_qz_627_mvl/mxv
	.byte		PAN   , c_v+56
	.byte	N11
	.byte		N11   , Gs3
	.byte		N11   , Bn3
	.byte	W12
	.byte		N05   , Fn3 , v088
	.byte		N05   , An3
	.byte		N05   , Cn4
	.byte	W24
	.byte		        Fn3
	.byte		N05   , An3
	.byte		N05   , Cn4
	.byte	W24
	.byte		        An3
	.byte		N05   , Cs4
	.byte		N05   , Fn4
	.byte		N05   , Gn4
	.byte	W24
	.byte		        An3
	.byte		N05   , Cs4
	.byte		N05   , Fn4
	.byte		N05   , Gn4
	.byte	W12
	.byte		N11   , Gn3
	.byte		N11   , As3
	.byte		N11   , Cs4
	.byte		N11   , En4
	.byte	W12
	.byte		N05   , Gn3
	.byte		N05   , As3
	.byte		N05   , Dn4
	.byte		N05   , Fn4
	.byte	W12
	.byte		        Dn3
	.byte		N05   , Gn3
	.byte		N05   , As3
	.byte		N05   , Dn4
	.byte	W12
	.byte		        Fs2
	.byte		N05   , An2
	.byte		N05   , Dn3
	.byte	W06
	.byte		N28   , Fs3 , v088 , gtp1
	.byte		N28   , An3 , v088 , gtp1
	.byte		N28   , Cn4 , v088 , gtp1
	.byte		N28   , Dn4 , v088 , gtp1
	.byte	W42
	.byte		N11   , Gn3
	.byte		N11   , As3
	.byte		N11   , Dn4
	.byte		N11   , Fn4
	.byte	W12
	.byte		        Ds3
	.byte		N11   , Fs3
	.byte		N11   , An3
	.byte		N11   , Bn3
	.byte	W12
	.byte		N05   , En3
	.byte		N05   , Gn3
	.byte		N05   , As3
	.byte		N05   , Cn4
	.byte	W24
	.byte		        En3
	.byte		N05   , Gn3
	.byte		N05   , As3
	.byte		N05   , Cn4
	.byte	W24
	.byte		        Gn3 , v100
	.byte		N05   , As3
	.byte		N05   , En4
	.byte		N05   , Gn4
	.byte	W24
	.byte		        En3 , v088
	.byte		N05   , Gn3
	.byte		N05   , As3
	.byte		N05   , En4
	.byte	W12
	.byte		N11   , Fn3
	.byte		N11   , Gs3
	.byte		N11   , Bn3
	.byte	W12
	.byte		N05   , Fn3
	.byte		N05   , An3
	.byte		N05   , Cn4
	.byte	W24
	.byte		        Fn3
	.byte		N05   , Gs3
	.byte		N05   , Bn3
	.byte	W24
	.byte		        Fn3
	.byte		N05   , Gn3
	.byte		N05   , As3
	.byte	W24
	.byte		        Fn3
	.byte		N05   , Gn3
	.byte		N05   , An3
	.byte	W12
	.byte		N11   , En3
	.byte		N11   , Gs3
	.byte		N11   , Bn3
	.byte	W12
	.byte		N05   , Fn3
	.byte		N05   , An3
	.byte		N05   , Cn4
	.byte	W24
	.byte		        Fn3
	.byte		N05   , An3
	.byte		N05   , Cn4
	.byte	W24
	.byte		        Fs3
	.byte		N05   , An3
	.byte		N05   , Cn4
	.byte		N05   , Ds4
	.byte	W24
	.byte		        Fn3
	.byte		N05   , An3
	.byte		N05   , Bn3
	.byte		N05   , Dn4
	.byte	W12
	.byte		N11   , Gn3
	.byte		N11   , As3
	.byte		N11   , Cs4
	.byte		N11   , En4
	.byte	W12
	.byte		N05   , Gn3
	.byte		N05   , As3
	.byte		N05   , Dn4
	.byte		N05   , Fn4
	.byte	W12
	.byte		N02   , Gn2
	.byte		N02   , Dn3
	.byte	W06
	.byte		        As2
	.byte		N02   , Gn3
	.byte	W06
	.byte		        Dn3
	.byte		N02   , An3
	.byte	W06
	.byte		N32   , Fn3 , v088 , gtp3
	.byte		N32   , As3 , v088 , gtp3
	.byte		N32   , Dn4 , v088 , gtp3
	.byte		N32   , Gn4 , v088 , gtp3
	.byte	W42
	.byte		VOL   , 63*mus_qz_627_mvl/mxv
	.byte		N05   , En4
	.byte	W06
	.byte		        Dn4
	.byte	W06
	.byte		PAN   , c_v+41
	.byte		N23   , Cn4
	.byte	W24
	.byte		        As3
	.byte	W24
	.byte		        An3
	.byte	W24
	.byte		        Gn3 , v056
	.byte	W24
	.byte		        Cn4 , v088
	.byte	W24
	.byte		        As3
	.byte	W24
	.byte		        An3
	.byte	W24
	.byte		        Dn4 , v064
	.byte	W24
	.byte		        Cn4 , v088
	.byte	W18
	.byte		BEND  , c_v+12
	.byte	W03
	.byte		        c_v+0
	.byte	W03
	.byte		N23   , As3
	.byte	W24
	.byte		N11   , An3
	.byte	W12
	.byte		N05   , Cn3 , v076
	.byte	W12
	.byte		N11   , Gn3 , v088
	.byte	W12
	.byte		N05   , Cn3 , v032
	.byte	W12
	.byte		N80   , Fn3 , v104 , gtp3
	.byte	W24
	.byte		VOL   , 47*mus_qz_627_mvl/mxv
	.byte	W02
	.byte		        44*mus_qz_627_mvl/mxv
	.byte	W03
	.byte		        43*mus_qz_627_mvl/mxv
	.byte	W03
	.byte		        41*mus_qz_627_mvl/mxv
	.byte	W06
	.byte		        36*mus_qz_627_mvl/mxv
	.byte	W06
	.byte		        34*mus_qz_627_mvl/mxv
	.byte	W06
	.byte		        30*mus_qz_627_mvl/mxv
	.byte	W03
	.byte		        28*mus_qz_627_mvl/mxv
	.byte	W09
	.byte		        28*mus_qz_627_mvl/mxv
	.byte	W06
	.byte		        27*mus_qz_627_mvl/mxv
	.byte	W03
	.byte		        25*mus_qz_627_mvl/mxv
	.byte	W03
	.byte		        24*mus_qz_627_mvl/mxv
	.byte	W06
	.byte		        23*mus_qz_627_mvl/mxv
	.byte	W03
	.byte		        22*mus_qz_627_mvl/mxv
	.byte	W01
	.byte		N05   , Fn3 , v127
	.byte	W02
	.byte		VOL   , 22*mus_qz_627_mvl/mxv
	.byte	W04
	.byte		N05   , Gn3
	.byte	W06
	.byte		N44   , An3 , v088 , gtp3
	.byte	W02
	.byte		VOL   , 19*mus_qz_627_mvl/mxv
	.byte	W03
	.byte		        19*mus_qz_627_mvl/mxv
	.byte	W03
	.byte		        18*mus_qz_627_mvl/mxv
	.byte	W06
	.byte		        17*mus_qz_627_mvl/mxv
	.byte	W01
	.byte		        14*mus_qz_627_mvl/mxv
	.byte	W02
	.byte		        15*mus_qz_627_mvl/mxv
	.byte	W03
	.byte		        20*mus_qz_627_mvl/mxv
	.byte	W04
	.byte		        25*mus_qz_627_mvl/mxv
	.byte	W02
	.byte		        35*mus_qz_627_mvl/mxv
	.byte	W03
	.byte		        46*mus_qz_627_mvl/mxv
	.byte	W03
	.byte		        59*mus_qz_627_mvl/mxv
	.byte	W28
	.byte		N05   , Cn3 , v076
	.byte	W12
	.byte		        Fn3 , v080
	.byte	W12
	.byte		        En3 , v088
	.byte	W06
	.byte		PAN   , c_v+42
	.byte		N02   , Dn3
	.byte	W03
	.byte		VOL   , 35*mus_qz_627_mvl/mxv
	.byte		N02   , En3
	.byte	W03
	.byte	GOTO
	 ptrvalue	mus_qz_627_3_B1
	.byte	FINE

/********************** Track  4 **********************/

mus_qz_627_4:
	.byte		KEYSH , mus_qz_627_key+0
	.byte		VOICE , 22
	.byte		VOL   , 53*mus_qz_627_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		BENDR , 12
	.byte		PRIO  , 64
	.byte	W06
	.byte		VOL   , 73*mus_qz_627_mvl/mxv
	.byte		PAN   , c_v-9
	.byte		N11   , As1 , v104
	.byte	W12
	.byte		N05   , As2
	.byte	W06
	.byte		        As2 , v044
	.byte	W06
	.byte		N11   , Gn1 , v104
	.byte	W12
	.byte		N05   , Gn2
	.byte	W06
	.byte		        Gn2 , v040
	.byte	W06
	.byte		N11   , Cn2 , v104
	.byte	W12
	.byte		N05   , Cn3 , v127
	.byte	W06
	.byte		        Cn3 , v036
	.byte	W06
	.byte		N11   , Fs2 , v100
	.byte	W12
	.byte		N05   , Gn2 , v127
	.byte	W12
	.byte		N11   , Gn1 , v100
	.byte	W12
	.byte		N05   , Gn2
	.byte	W12
	.byte		N11   , Cn2
	.byte	W12
	.byte		        As2
	.byte	W12
	.byte		N23   , Fn1
	.byte	W24
	.byte		N11   , Cn2
	.byte	W12
	.byte		N05   , Cn3
	.byte	W12
	.byte		N11   , As1
	.byte	W12
	.byte		N05   , As2
	.byte	W12
	.byte		N11   , An1
	.byte	W12
	.byte		N05   , An2
	.byte	W12
	.byte		N11   , Fn1
	.byte	W12
	.byte		N05   , Fn2
	.byte	W12
	.byte		N11   , Gn1
	.byte	W12
	.byte		N05   , Gn2
	.byte	W12
	.byte		N11   , An1
	.byte	W12
	.byte		N05   , An2
	.byte	W12
	.byte		N11   , Gn1
	.byte	W12
	.byte		N05   , Gn2
	.byte	W12
mus_qz_627_4_B1:
	.byte		N11   , Fn1 , v104
	.byte	W42
	.byte		N05   , Cn1
	.byte	W06
	.byte		        Fn1
	.byte	W12
	.byte	N20
	.byte	W24
	.byte		N11   , Cn1
	.byte	W12
	.byte		        Fn1
	.byte	W42
	.byte		N05   , Cn1
	.byte	W06
	.byte		N11   , An1
	.byte	W12
	.byte		        En1
	.byte	W12
	.byte		N05   , Cs1
	.byte	W06
	.byte		N17   , En2
	.byte	W18
	.byte		N11   , Dn2
	.byte	W42
	.byte		N05   , An1
	.byte	W06
	.byte		        Cs2
	.byte	W12
	.byte	N05
	.byte	W24
	.byte		N11   , An1
	.byte	W12
	.byte		BEND  , c_v+0
	.byte		N11   , Cn2
	.byte	W12
	.byte		        An2
	.byte	W06
	.byte		BEND  , c_v+8
	.byte	W03
	.byte		        c_v+0
	.byte	W03
	.byte		N05   , Gn2
	.byte	W06
	.byte		N11   , En2
	.byte	W12
	.byte		N05   , Dn2
	.byte	W06
	.byte		N11   , Bn1
	.byte	W12
	.byte		        Gn1
	.byte	W12
	.byte		N17   , Gn2
	.byte	W18
	.byte		N05   , Dn2
	.byte	W06
	.byte		        As1
	.byte	W36
	.byte		N11   , Fn1
	.byte	W12
	.byte		N05   , As1
	.byte	W12
	.byte		        Dn1
	.byte	W12
	.byte		        Fn1
	.byte	W06
	.byte		N17   , As1
	.byte	W18
	.byte		N11   , Cn2
	.byte	W36
	.byte		        Gn1
	.byte	W12
	.byte		N02   , An2
	.byte	W03
	.byte		        As2
	.byte	W03
	.byte		N05   , An2
	.byte	W06
	.byte		N02   , Fs2
	.byte	W03
	.byte		        En2
	.byte	W03
	.byte		BEND  , c_v+8
	.byte		N17   , Dn2
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W15
	.byte		N11   , Cn2
	.byte	W12
	.byte		        Gn1
	.byte	W12
	.byte		N05   , Dn1
	.byte	W24
	.byte	N05
	.byte	W12
	.byte		N11   , Gn1
	.byte	W12
	.byte		N23   , Dn2
	.byte	W24
	.byte		N11   , Bn1
	.byte	W12
	.byte		N05   , Cn2
	.byte	W24
	.byte	N23
	.byte	W24
	.byte		        Bn1
	.byte	W24
	.byte		        As1
	.byte	W24
	.byte		N11   , Fn1
	.byte	W12
	.byte		N05   , Fn2
	.byte	W24
	.byte	N05
	.byte	W12
	.byte		        En2
	.byte	W12
	.byte		N23   , An1
	.byte	W24
	.byte		N11   , Gn2
	.byte	W12
	.byte		        Cs2
	.byte	W12
	.byte		N05   , Dn2
	.byte	W24
	.byte	N05
	.byte	W06
	.byte		N17   , An1
	.byte	W18
	.byte		N05   , Dn3 , v088
	.byte	W06
	.byte		        Ds3
	.byte	W06
	.byte		        Dn3
	.byte	W06
	.byte		        Cn3
	.byte	W06
	.byte		        As2
	.byte	W06
	.byte		        An2
	.byte	W06
	.byte		N11   , Gn2 , v104
	.byte	W12
	.byte		N05   , Cn2
	.byte	W24
	.byte	N05
	.byte	W12
	.byte		        As1
	.byte	W12
	.byte		N23   , As2
	.byte	W24
	.byte		N05   , Gn2
	.byte	W12
	.byte		N11   , Fn1
	.byte	W12
	.byte		N05   , Cn2
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		N11   , Fn1
	.byte	W12
	.byte		        Dn2
	.byte	W12
	.byte		        Fn1
	.byte	W12
	.byte		N05   , En2
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		N11   , Fn1
	.byte	W12
	.byte		        Fn2
	.byte	W12
	.byte		        Fn1
	.byte	W12
	.byte		N05   , Fn2
	.byte	W24
	.byte	N05
	.byte	W12
	.byte		        Dn2
	.byte	W12
	.byte		N23   , An2
	.byte	W24
	.byte		N11   , Bn1
	.byte	W12
	.byte		        As1
	.byte	W12
	.byte		N05   , Dn2
	.byte	W24
	.byte		        Gn1
	.byte	W06
	.byte		        As1
	.byte	W06
	.byte		BEND  , c_v+63
	.byte		N11   , As0
	.byte	W03
	.byte		BEND  , c_v+57
	.byte	W02
	.byte		        c_v+9
	.byte	W03
	.byte		        c_v+4
	.byte	W04
	.byte		        c_v+0
	.byte	N05
	.byte	W12
	.byte	N08
	.byte	W12
	.byte	N05
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		N05   , As1
	.byte	W12
	.byte		N11   , As0
	.byte	W12
	.byte		N05   , As1
	.byte	W12
	.byte		N11   , En1
	.byte	W12
	.byte		N05   , En2
	.byte	W12
	.byte		N11   , En1
	.byte	W12
	.byte		N05   , En2
	.byte	W12
	.byte		N11   , An0
	.byte	W12
	.byte		N05   , An1
	.byte	W12
	.byte		N11   , An0
	.byte	W12
	.byte		N05   , An1
	.byte	W12
	.byte		N11   , Dn1
	.byte	W12
	.byte		N02   , Dn2
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		N11   , Dn1
	.byte	W12
	.byte		N02   , Dn2
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Gn1
	.byte	W12
	.byte		N02   , Gn2
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Gn1
	.byte	W12
	.byte		N02   , Gn2
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Cn1
	.byte	W12
	.byte		N02   , Cn2
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		N02   , Cn1
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		N11   , En1
	.byte	W12
	.byte		N80   , An1 , v088 , gtp3
	.byte	W84
	.byte		N05   , Gs1
	.byte	W06
	.byte		        Gn1
	.byte	W06
	.byte		N23   , Fn1
	.byte	W24
	.byte		N05   , Fn2
	.byte	W72
	.byte	GOTO
	 ptrvalue	mus_qz_627_4_B1
	.byte	FINE

/********************** Track  5 **********************/

mus_qz_627_5:
	.byte		KEYSH , mus_qz_627_key+0
	.byte		VOICE , 13
	.byte		PAN   , c_v-22
	.byte		VOL   , 43*mus_qz_627_mvl/mxv
	.byte		PRIO  , 64
	.byte		N02   , An3 , v088
	.byte	W03
	.byte		        As3
	.byte	W03
	.byte		N11   , Dn4
	.byte	W12
	.byte		N05   , Cn4
	.byte	W12
	.byte		N11   , As3
	.byte	W12
	.byte		N05   , Gn3
	.byte	W12
	.byte		N40   , An3 , v088 , gtp1
	.byte	W42
	.byte		N02   , En3
	.byte	W03
	.byte		        Fs3
	.byte	W03
	.byte		N11   , Gn3
	.byte	W12
	.byte		N05   , Fn3
	.byte	W12
	.byte		N11   , En3
	.byte	W12
	.byte		N05   , Cs3
	.byte	W12
	.byte		N92   , Cn3 , v088 , gtp3
	.byte	W96
	.byte		PAN   , c_v-41
	.byte		VOL   , 55*mus_qz_627_mvl/mxv
	.byte		N05   , Fn2
	.byte	W12
	.byte		N08   , An2
	.byte	W09
	.byte		N14   , Cn1
	.byte	W15
	.byte		N11   , Cn2
	.byte	W09
	.byte		N14   , Cs1
	.byte	W15
	.byte		N11   , Cs2
	.byte	W09
	.byte		N14   , Cn1
	.byte	W15
	.byte		N11   , Cn2
	.byte	W12
mus_qz_627_5_B1:
	.byte		VOL   , 55*mus_qz_627_mvl/mxv
	.byte		PAN   , c_v-41
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
	.byte		VOL   , 37*mus_qz_627_mvl/mxv
	.byte	W96
	.byte	W96
	.byte		        50*mus_qz_627_mvl/mxv
	.byte		N23   , An2 , v088
	.byte	W24
	.byte		        As2
	.byte	W24
	.byte		        Cn3
	.byte	W24
	.byte		        Fn3
	.byte	W24
	.byte		N17   , An3
	.byte	W18
	.byte		N02   , As3
	.byte	W03
	.byte		        An3
	.byte	W03
	.byte		N23   , Cn3
	.byte	W24
	.byte		        Fn3
	.byte	W24
	.byte		N11   , Gn2
	.byte	W12
	.byte		N05   , En2
	.byte	W12
	.byte		        An3
	.byte	W12
	.byte		        Fn3
	.byte	W12
	.byte		        Cn4
	.byte	W12
	.byte		        Fn3
	.byte	W12
	.byte		        An3
	.byte	W12
	.byte		        Fn3
	.byte	W12
	.byte		        Cn4
	.byte	W12
	.byte		        Fn3
	.byte	W12
	.byte		        An3
	.byte	W12
	.byte		        Fn3
	.byte	W12
	.byte		        Cn4
	.byte	W12
	.byte		        Fn3
	.byte	W60
	.byte	GOTO
	 ptrvalue	mus_qz_627_5_B1
	.byte	FINE

/********************** Track  6 **********************/

mus_qz_627_6:
	.byte		KEYSH , mus_qz_627_key+0
	.byte		VOICE , 8
	.byte		VOL   , 50*mus_qz_627_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte	W54
	.byte	W96
	.byte		VOICE , 8
	.byte		PAN   , c_v+43
	.byte		N23   , Cn4 , v088
	.byte	W24
	.byte		        Cn5
	.byte	W24
	.byte		        As4
	.byte	W24
	.byte		        An4
	.byte	W24
	.byte		        As4
	.byte	W24
	.byte		        An4
	.byte	W24
	.byte		        As4
	.byte	W24
	.byte		        Cn5
	.byte	W24
mus_qz_627_6_B1:
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
	 ptrvalue	mus_qz_627_6_B1
	.byte	FINE

/********************** Track  7 **********************/

mus_qz_627_7:
	.byte		KEYSH , mus_qz_627_key+0
	.byte		VOICE , 13
	.byte		VOL   , 53*mus_qz_627_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte	W06
	.byte		VOICE , 9
	.byte		VOL   , 46*mus_qz_627_mvl/mxv
	.byte		PAN   , c_v-30
	.byte		N11   , Fn4 , v088
	.byte	W12
	.byte		        En4
	.byte	W12
	.byte		        Dn4
	.byte	W12
	.byte		        As3
	.byte	W12
	.byte		N23   , Cn4
	.byte	W24
	.byte		        Gn4
	.byte	W24
	.byte		N11   , As3
	.byte	W12
	.byte		        An3
	.byte	W12
	.byte		        Gn3
	.byte	W12
	.byte		        En3
	.byte	W12
	.byte		N44   , Fn3 , v088 , gtp3
	.byte	W96
	.byte	W96
mus_qz_627_7_B1:
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		VOICE , 9
	.byte		VOL   , 39*mus_qz_627_mvl/mxv
	.byte		PAN   , c_v-22
	.byte		N11   , En4 , v088
	.byte	W12
	.byte		        Fn4
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		        Gn4
	.byte	W12
	.byte		N23   , An4
	.byte	W24
	.byte		N11   , Gn4
	.byte	W12
	.byte		        Fn4
	.byte	W12
	.byte		        Gn4
	.byte	W12
	.byte		        Fn4
	.byte	W12
	.byte		N05   , Gn4
	.byte	W06
	.byte		N11   , Fn4
	.byte	W12
	.byte		N17   , Dn4
	.byte	W18
	.byte		N11   , An3
	.byte	W12
	.byte		        Fn3
	.byte	W12
	.byte		        Fn4
	.byte	W12
	.byte		VOICE , 33
	.byte		PAN   , c_v+16
	.byte		VOL   , 46*mus_qz_627_mvl/mxv
	.byte		N11   , Ds4
	.byte	W12
	.byte		N05   , En4
	.byte	W12
	.byte	N05
	.byte	W12
	.byte		        Fn4
	.byte	W09
	.byte	N02
	.byte	W03
	.byte		        Fs4
	.byte	W03
	.byte		N20   , Gn4
	.byte	W21
	.byte		N05   , Fn4
	.byte	W12
	.byte		        En4
	.byte	W12
	.byte		        Fn4
	.byte	W12
	.byte		        Gn4
	.byte	W12
	.byte		        Fn4
	.byte	W06
	.byte		        Dn4
	.byte	W12
	.byte		N28   , Cn4 , v088 , gtp1
	.byte	W42
	.byte		N11   , Bn3
	.byte	W12
	.byte		VOICE , 36
	.byte		N11   , En4
	.byte	W12
	.byte		N05   , Fn4
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		        Gn4
	.byte	W12
	.byte		N23   , An4
	.byte	W24
	.byte		N11   , Gn4
	.byte	W12
	.byte		        Fn4
	.byte	W12
	.byte		        As4
	.byte	W12
	.byte		        An4
	.byte	W12
	.byte		N05   , As4
	.byte	W06
	.byte		        Bn4
	.byte	W06
	.byte		        Cn5
	.byte	W06
	.byte		N40   , Dn5 , v088 , gtp1
	.byte	W54
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_627_7_B1
	.byte	FINE

/********************** Track  8 **********************/

mus_qz_627_8:
	.byte		KEYSH , mus_qz_627_key+0
	.byte		VOICE , 11
	.byte		VOL   , 53*mus_qz_627_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte	W06
	.byte		        c_v-48
	.byte		VOL   , 36*mus_qz_627_mvl/mxv
	.byte		N23   , Cn3 , v088
	.byte		N23   , Fn3
	.byte		N23   , As3
	.byte	W24
	.byte		        As2
	.byte		N23   , Dn3
	.byte		N23   , Gn3
	.byte	W24
	.byte		        Cn3
	.byte		N23   , Fn3
	.byte		N23   , Gn3
	.byte	W24
	.byte		        Cn3
	.byte		N23   , En3
	.byte		N23   , Gn3
	.byte	W24
	.byte		        Gn2
	.byte		N23   , As2
	.byte		N23   , Dn3
	.byte		N23   , Fn3
	.byte	W24
	.byte		        As2
	.byte		N23   , Cs3
	.byte		N23   , En3
	.byte		N23   , Gn3
	.byte	W24
	.byte		VOL   , 37*mus_qz_627_mvl/mxv
	.byte		N11   , Fn2
	.byte		N92   , An2 , v088 , gtp3
	.byte		N92   , Cn3 , v088 , gtp3
	.byte		N92   , Fn3 , v088 , gtp3
	.byte	W24
	.byte		N11   , Cn2 , v116
	.byte	W24
	.byte		        Dn2 , v120
	.byte	W24
	.byte		        En2
	.byte	W24
	.byte		PAN   , c_v-48
	.byte		N11   , Fn2 , v088
	.byte	W12
	.byte		N02   , An3 , v100
	.byte		N02   , Fn4
	.byte		N02   , Cn5
	.byte	W06
	.byte		        An3 , v084
	.byte		N02   , Fn4
	.byte		N02   , Cn5
	.byte	W06
	.byte		N11   , Cn2 , v088
	.byte	W12
	.byte		N02   , An3 , v100
	.byte		N02   , Fn4
	.byte		N02   , Cn5
	.byte	W06
	.byte		        An3 , v084
	.byte		N02   , Fn4
	.byte		N02   , Cn5
	.byte	W06
	.byte		N11   , Cs2 , v088
	.byte	W12
	.byte		N02   , An3 , v104
	.byte		N02   , Fn4
	.byte		N02   , Cn5
	.byte	W06
	.byte		        An3 , v084
	.byte		N02   , Fn4
	.byte		N02   , Cn5
	.byte	W06
	.byte		N11   , Cn2 , v088
	.byte	W12
	.byte		N02   , An3 , v092
	.byte		N02   , Fn4
	.byte		N02   , Cn5
	.byte	W06
	.byte		        An3 , v088
	.byte		N02   , Fn4
	.byte		N02   , Cn5
	.byte	W06
mus_qz_627_8_B1:
	.byte		VOICE , 12
	.byte		VOL   , 33*mus_qz_627_mvl/mxv
	.byte		PAN   , c_v-51
	.byte		N02   , Fn3 , v088
	.byte		N02   , An3
	.byte		N02   , Fn4
	.byte	W12
	.byte		        Fn3 , v084
	.byte		N02   , An3
	.byte		N02   , En4
	.byte	W12
	.byte		        Fn3 , v088
	.byte		N02   , An3
	.byte		N02   , Dn4
	.byte	W18
	.byte		N05   , An2
	.byte		N05   , Cn3
	.byte		N05   , Fn3
	.byte	W06
	.byte		        En3
	.byte		N05   , An3
	.byte		N05   , Cs4
	.byte	W12
	.byte		N20   , Fn3
	.byte		N20   , As3
	.byte		N20   , Dn4
	.byte	W24
	.byte		N11   , Gn3
	.byte		N11   , As3
	.byte		N11   , Cs4
	.byte	W12
	.byte		VOICE , 12
	.byte		N02   , En3 , v100
	.byte		N02   , Gs3
	.byte		N02   , Bn3
	.byte	W06
	.byte		        Fn3 , v068
	.byte		N02   , An3
	.byte		N02   , Cn4
	.byte	W06
	.byte		        Fn3 , v092
	.byte		N02   , An3
	.byte		N02   , Cn4
	.byte	W06
	.byte		        Fn3 , v060
	.byte		N02   , An3
	.byte		N02   , Cn4
	.byte	W06
	.byte		VOICE , 11
	.byte		N02   , Fn3 , v084
	.byte		N02   , An3
	.byte		N02   , Cn4
	.byte	W06
	.byte		        Fn3 , v060
	.byte		N02   , An3
	.byte		N02   , Cn4
	.byte	W06
	.byte		        Fn3 , v084
	.byte		N02   , An3
	.byte		N02   , Cn4
	.byte	W06
	.byte		N05   , An2 , v088
	.byte		N05   , Cn3
	.byte		N05   , Fn3
	.byte	W06
	.byte		VOICE , 19
	.byte		        12
	.byte		N02   , Cs3
	.byte		N02   , En3
	.byte		N02   , An3
	.byte	W06
	.byte		        Cs3 , v048
	.byte		N02   , En3
	.byte		N02   , An3
	.byte	W06
	.byte		VOICE , 11
	.byte		N02   , Cs3 , v088
	.byte		N02   , En3
	.byte		N02   , An3
	.byte	W06
	.byte		        Cs3 , v056
	.byte		N02   , En3
	.byte		N02   , An3
	.byte	W06
	.byte		N23   , Gn3 , v088
	.byte		N23   , As3
	.byte		N23   , En4
	.byte	W24
	.byte		VOICE , 12
	.byte		N02   , Fn3
	.byte		N02   , An3
	.byte		N02   , Fn4
	.byte	W12
	.byte		        Fn3
	.byte		N02   , An3
	.byte		N02   , En4
	.byte	W12
	.byte		        Fn3
	.byte		N02   , An3
	.byte		N02   , Dn4
	.byte	W18
	.byte		N05   , Fn2
	.byte		N05   , An2
	.byte		N05   , Cs3
	.byte	W06
	.byte		N11   , En3
	.byte		N11   , Gs3
	.byte		N11   , Cn4
	.byte	W12
	.byte		N23   , Fn3
	.byte		N23   , An3
	.byte		N23   , Cs4
	.byte		N23   , Fn4
	.byte	W24
	.byte		N05   , Cs4 , v112
	.byte		N05   , Fn4
	.byte		N05   , An4
	.byte	W12
	.byte		VOICE , 12
	.byte		N02   , Bn2 , v088
	.byte		N02   , En3
	.byte		N02   , Gs3
	.byte	W06
	.byte		        Cn3 , v056
	.byte		N02   , Fn3
	.byte		N02   , An3
	.byte	W06
	.byte		        Cn3 , v088
	.byte		N02   , Fn3
	.byte		N02   , An3
	.byte	W06
	.byte		        Cn3 , v056
	.byte		N02   , Fn3
	.byte		N02   , An3
	.byte	W06
	.byte		VOICE , 11
	.byte		N05   , Bn2 , v092
	.byte		N05   , En3
	.byte		N05   , Gs3
	.byte	W06
	.byte		N02   , Cn3 , v056
	.byte		N02   , Fn3
	.byte		N02   , An3
	.byte	W06
	.byte		N05   , Fn3 , v088
	.byte		N05   , An3
	.byte		N05   , Dn4
	.byte	W12
	.byte		VOICE , 19
	.byte		        12
	.byte		N05   , As2
	.byte		N05   , Cs3
	.byte		N05   , En3
	.byte		N05   , Gs3
	.byte	W06
	.byte		N02   , Bn2 , v056
	.byte		N02   , Dn3
	.byte		N02   , Fn3
	.byte		N02   , An3
	.byte	W06
	.byte		VOICE , 11
	.byte		N02   , Bn2 , v088
	.byte		N02   , Dn3
	.byte		N02   , Fn3
	.byte		N02   , An3
	.byte	W06
	.byte		        Bn2 , v060
	.byte		N02   , Dn3
	.byte		N02   , Fn3
	.byte		N02   , An3
	.byte	W06
	.byte		N11   , En3 , v088
	.byte		N11   , Gn3
	.byte		N11   , Bn3
	.byte		N11   , Dn4
	.byte	W12
	.byte		        Gn3
	.byte		N11   , Bn3
	.byte		N11   , Dn4
	.byte		N11   , Fn4
	.byte	W12
	.byte		VOICE , 12
	.byte		N02   , As2
	.byte		N02   , Dn3
	.byte		N02   , Fn3
	.byte		N02   , An3
	.byte	W12
	.byte		N08   , An2
	.byte		N08   , Cs3
	.byte		N11   , En3
	.byte		N11   , Gs3
	.byte	W12
	.byte		N02   , As2
	.byte		N02   , Dn3
	.byte		N02   , Fn3
	.byte		N02   , An3
	.byte	W12
	.byte		N05   , Fn3
	.byte		N05   , An3
	.byte		N05   , Dn4
	.byte		N05   , Fn4
	.byte	W12
	.byte		VOICE , 19
	.byte		        12
	.byte		N05   , An2 , v092
	.byte		N05   , Cs3
	.byte		N05   , Ds3
	.byte		N05   , Fs3
	.byte	W06
	.byte		N02   , As2 , v064
	.byte		N02   , Dn3
	.byte		N02   , En3
	.byte		N02   , Gn3
	.byte	W06
	.byte		VOICE , 11
	.byte		N02   , As2 , v104
	.byte		N02   , Dn3
	.byte		N02   , En3
	.byte		N02   , Gn3
	.byte	W06
	.byte		        As2 , v060
	.byte		N02   , Dn3
	.byte		N02   , En3
	.byte		N02   , Gn3
	.byte	W12
	.byte		N17   , As2 , v088
	.byte		N17   , Dn3
	.byte		N17   , Fn3
	.byte	W18
	.byte		VOICE , 12
	.byte		N02   , An2
	.byte		N02   , Cn3
	.byte		N02   , En3
	.byte		N02   , Gn3
	.byte	W12
	.byte		N11   , Gs2
	.byte		N11   , Bn2
	.byte		N11   , Ds3
	.byte		N11   , Fs3
	.byte	W12
	.byte		N02   , An2
	.byte		N02   , Cn3
	.byte		N02   , En3
	.byte		N02   , Gn3
	.byte	W12
	.byte		N05   , En3
	.byte		N05   , Gn3
	.byte		N05   , Cn4
	.byte		N05   , En4
	.byte	W12
	.byte		VOICE , 19
	.byte		        12
	.byte		N05   , Fn3
	.byte		N05   , Gs3
	.byte		N05   , Bn3
	.byte	W06
	.byte		N02   , Fs3 , v052
	.byte		N02   , An3
	.byte		N02   , Cn4
	.byte	W06
	.byte		VOICE , 11
	.byte		N02   , Fs3 , v088
	.byte		N02   , An3
	.byte		N02   , Cn4
	.byte	W06
	.byte		        Fs3 , v056
	.byte		N02   , An3
	.byte		N02   , Cn4
	.byte	W06
	.byte		N11   , An3 , v088
	.byte		N11   , Dn4
	.byte		N11   , Fs4
	.byte	W12
	.byte		N02   , An3 , v068
	.byte		N02   , Dn4
	.byte		N02   , An4
	.byte	W12
	.byte		VOICE , 12
	.byte		N11   , Fs3 , v088
	.byte		N11   , An3
	.byte		N11   , Cs4
	.byte		N11   , En4
	.byte	W12
	.byte		N05   , Gn3
	.byte		N05   , As3
	.byte		N05   , Dn4
	.byte		N05   , Fn4
	.byte	W24
	.byte		        Gn3
	.byte		N05   , As3
	.byte		N05   , Dn4
	.byte		N05   , Fn4
	.byte	W12
	.byte		        Fn3
	.byte		N05   , Gn3
	.byte		N05   , Bn3
	.byte		N05   , Dn4
	.byte	W12
	.byte		VOICE , 11
	.byte		N05   , Fn3
	.byte		N05   , Gn3
	.byte		N05   , Bn3
	.byte		N05   , Dn4
	.byte	W12
	.byte		N11   , Fs3
	.byte		N11   , As3
	.byte		N11   , Cs4
	.byte		N11   , En4
	.byte	W12
	.byte		N05   , Fn3
	.byte		N05   , Gn3
	.byte		N05   , Bn3
	.byte		N05   , Dn4
	.byte	W12
	.byte		VOL   , 23*mus_qz_627_mvl/mxv
	.byte		N05   , As3
	.byte		N05   , Dn4
	.byte		N05   , En4
	.byte		N05   , Gn4
	.byte	W24
	.byte		VOL   , 27*mus_qz_627_mvl/mxv
	.byte		N23   , Gn2
	.byte		N23   , En3
	.byte		N23   , Cn4
	.byte	W24
	.byte		VOL   , 30*mus_qz_627_mvl/mxv
	.byte		N23   , Gs2
	.byte		N23   , Bn2
	.byte		N23   , Fn3
	.byte		N23   , Dn4
	.byte	W24
	.byte		VOL   , 38*mus_qz_627_mvl/mxv
	.byte		N23   , As2
	.byte		N23   , Cn3
	.byte		N23   , Gn3
	.byte		N23   , En4
	.byte	W24
	.byte		VOICE , 12
	.byte		PAN   , c_v-32
	.byte		VOL   , 64*mus_qz_627_mvl/mxv
	.byte		N11   , En3
	.byte	W12
	.byte		VOICE , 11
	.byte		N05   , Fn3
	.byte	W12
	.byte	N05
	.byte	W12
	.byte		        Gn3
	.byte	W12
	.byte		N02   , Gs3
	.byte	W03
	.byte		N20   , An3
	.byte	W21
	.byte		VOICE , 12
	.byte		N05   , Gn3
	.byte	W12
	.byte		VOICE , 11
	.byte		N05   , Fn3
	.byte	W12
	.byte		VOICE , 12
	.byte		N05   , Gn3
	.byte	W12
	.byte		VOICE , 11
	.byte		N05   , Fn3
	.byte	W12
	.byte		        Gn3
	.byte	W06
	.byte		N02   , Fn3
	.byte	W12
	.byte		N28   , Dn3 , v088 , gtp1
	.byte	W42
	.byte		N11   , Fn3
	.byte	W12
	.byte		        Ds3
	.byte	W12
	.byte		N05   , En3
	.byte	W12
	.byte	N05
	.byte	W12
	.byte		        Fn3
	.byte	W09
	.byte	N02
	.byte	W03
	.byte		        Fs3
	.byte	W03
	.byte		N20   , Gn3
	.byte	W21
	.byte		N05   , Fn3
	.byte	W12
	.byte		        En3
	.byte	W12
	.byte		        Fn3
	.byte	W12
	.byte		        Gn3
	.byte	W12
	.byte		        Fn3
	.byte	W06
	.byte		        Dn3
	.byte	W12
	.byte		N28   , Cn3 , v088 , gtp1
	.byte	W42
	.byte		N11   , Bn2
	.byte	W12
	.byte		VOL   , 55*mus_qz_627_mvl/mxv
	.byte		PAN   , c_v-24
	.byte		N11   , En3
	.byte	W12
	.byte		N05   , Fn3
	.byte	W12
	.byte	N05
	.byte	W12
	.byte		        Gn3
	.byte	W12
	.byte		N02   , Gs3
	.byte	W03
	.byte		N20   , An3
	.byte	W21
	.byte		N05   , Gn3
	.byte	W12
	.byte		        Fn3
	.byte	W12
	.byte		N11   , As3
	.byte	W12
	.byte		N05   , An3
	.byte	W12
	.byte		        As3
	.byte	W06
	.byte		        Bn3
	.byte	W06
	.byte		        Cn4
	.byte	W06
	.byte		N40   , Dn4 , v088 , gtp1
	.byte	W54
	.byte		VOL   , 36*mus_qz_627_mvl/mxv
	.byte		PAN   , c_v-53
	.byte	W12
	.byte		VOICE , 12
	.byte		N05   , En3
	.byte		N05   , Gs3
	.byte		N05   , Cs4
	.byte	W06
	.byte		VOICE , 11
	.byte		N05   , Fn3
	.byte		N05   , An3
	.byte		N05   , Dn4
	.byte	W18
	.byte		        Fn3
	.byte		N05   , An3
	.byte		N05   , Dn4
	.byte	W24
	.byte		        Gs3
	.byte		N05   , Cn4
	.byte		N05   , Dn4
	.byte		N05   , Gn4
	.byte	W24
	.byte		        Gs3
	.byte		N05   , Cn4
	.byte		N05   , Dn4
	.byte		N05   , Gn4
	.byte	W12
	.byte	W12
	.byte		VOICE , 12
	.byte		N05   , Ds3
	.byte		N05   , Fs3
	.byte		N05   , Bn3
	.byte	W06
	.byte		VOICE , 11
	.byte		N05   , En3
	.byte		N05   , Gn3
	.byte		N05   , Cn4
	.byte	W18
	.byte		        En3
	.byte		N05   , Gn3
	.byte		N05   , Cn4
	.byte	W24
	.byte		VOICE , 12
	.byte		N05   , Fn3
	.byte		N05   , An3
	.byte		N05   , Bn3
	.byte		N05   , En4
	.byte	W06
	.byte		VOICE , 11
	.byte		N05   , Fs3
	.byte		N05   , As3
	.byte		N05   , Cn4
	.byte		N05   , Fn4
	.byte	W06
	.byte		VOICE , 12
	.byte		N05   , An2
	.byte		N05   , Cn3
	.byte		N05   , An3
	.byte	W06
	.byte		        Cn3
	.byte		N05   , Fn3
	.byte		N05   , As3
	.byte	W06
	.byte		VOICE , 11
	.byte		N05   , Fs3
	.byte		N05   , As3
	.byte		N05   , Cn4
	.byte	W06
	.byte		        Fs3
	.byte		N05   , As3
	.byte		N05   , Cn4
	.byte		N05   , Fn4
	.byte	W06
	.byte		VOL   , 36*mus_qz_627_mvl/mxv
	.byte		N11   , Dn3
	.byte		N11   , Fn3
	.byte		N11   , An3
	.byte		N11   , Cn4
	.byte	W12
	.byte		        Fn2
	.byte		N11   , Cn3
	.byte	W12
	.byte		        Dn3
	.byte		N11   , Fn3
	.byte		N11   , Gn3
	.byte		N11   , As3
	.byte	W12
	.byte		        Fn2
	.byte		N11   , As2
	.byte	W12
	.byte	N23
	.byte		N17   , Dn3
	.byte		N23   , Gn3
	.byte		N17   , An3
	.byte	W18
	.byte		N02   , Ds3
	.byte		N02   , As3
	.byte	W03
	.byte		        Dn3
	.byte		N02   , An3
	.byte	W03
	.byte		N23   , As2
	.byte		N23   , Cs3
	.byte		N23   , En3
	.byte		N23   , Gn3
	.byte	W24
	.byte		VOICE , 11
	.byte		VOL   , 37*mus_qz_627_mvl/mxv
	.byte		TIE   , Fn2
	.byte		TIE   , Cn3
	.byte		N80   , An3
	.byte		TIE   , Cn4
	.byte		TIE   , Fn4
	.byte	W96
	.byte	W23
	.byte		EOT   , Fn2
	.byte		        Cn3
	.byte		        Cn4
	.byte		        Fn4
	.byte	W72
	.byte	W01
	.byte	GOTO
	 ptrvalue	mus_qz_627_8_B1
	.byte	FINE

/********************** Track  9 **********************/

mus_qz_627_9:
	.byte		KEYSH , mus_qz_627_key+0
	.byte		VOICE , 13
	.byte		VOL   , 53*mus_qz_627_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		BENDR , 6
	.byte		PRIO  , 64
	.byte	W06
	.byte		VOL   , 11*mus_qz_627_mvl/mxv
	.byte		PAN   , c_v-51
	.byte		VOL   , 23*mus_qz_627_mvl/mxv
	.byte		N02   , Dn4 , v088
	.byte	W03
	.byte		        En4
	.byte	W03
	.byte		N11   , Fn4
	.byte	W12
	.byte		N05   , En4
	.byte	W12
	.byte		N11   , Dn4
	.byte	W12
	.byte		N05   , As3
	.byte	W06
	.byte	W06
	.byte		N40   , Cn4 , v088 , gtp1
	.byte	W42
	.byte		N02   , Gn3
	.byte	W03
	.byte		        An3
	.byte	W03
	.byte		N11   , As3
	.byte	W12
	.byte		N05   , An3
	.byte	W12
	.byte		N11   , Gn3
	.byte	W12
	.byte		        En3
	.byte	W06
	.byte	W06
	.byte		N92   , Fn3 , v088 , gtp3
	.byte	W90
	.byte	W96
mus_qz_627_9_B1:
	.byte		VOL   , 6*mus_qz_627_mvl/mxv
	.byte		        15*mus_qz_627_mvl/mxv
	.byte		N02   , Dn3 , v072
	.byte	W03
	.byte		        En3
	.byte	W03
	.byte		N32   , Fn3 , v104 , gtp3
	.byte	W36
	.byte		N05   , En3
	.byte	W12
	.byte		N11   , Fn3
	.byte	W12
	.byte		N05   , En3
	.byte	W12
	.byte		N11   , Fn3
	.byte	W12
	.byte		N05   , Gn3
	.byte	W06
	.byte	W06
	.byte		N02   , Gs3
	.byte	W03
	.byte		N32   , An3
	.byte	W32
	.byte	W01
	.byte		N11   , Fn3
	.byte	W12
	.byte		N23   , Cn3
	.byte	W36
	.byte		N11   , Cs3
	.byte	W06
	.byte	W06
	.byte		N32   , Dn3 , v104 , gtp3
	.byte	W36
	.byte		N05   , Cs3
	.byte	W12
	.byte		N11   , Dn3
	.byte	W12
	.byte		N05   , Cs3
	.byte	W12
	.byte		N11   , Dn3
	.byte	W12
	.byte		N05   , En3
	.byte	W06
	.byte	W06
	.byte		N32   , Fn3 , v104 , gtp3
	.byte	W36
	.byte		N05   , Dn3
	.byte	W12
	.byte		N32   , An2 , v104 , gtp3
	.byte	W36
	.byte		N11   , Gn2
	.byte	W06
	.byte	W18
	.byte		N02   , An2
	.byte	W03
	.byte	N05
	.byte	W09
	.byte		N11   , As2
	.byte	W12
	.byte		N05   , Dn3
	.byte	W12
	.byte		N23   , Cn3
	.byte	W24
	.byte		        As2
	.byte	W18
	.byte	W18
	.byte		N02   , Bn2
	.byte	W03
	.byte	N05
	.byte	W09
	.byte		N11   , Cn3
	.byte	W12
	.byte		N05   , En3
	.byte	W12
	.byte		N23   , Dn3
	.byte	W24
	.byte		        Cn3
	.byte	W18
	.byte	W21
	.byte	N02
	.byte	W03
	.byte		N05   , Cs3
	.byte	W06
	.byte		N11   , Dn3
	.byte	W12
	.byte		N05   , Fn3
	.byte	W12
	.byte		N11   , Gn3
	.byte	W12
	.byte		N05   , Fn3
	.byte	W12
	.byte		N11   , En3
	.byte	W12
	.byte		N05   , Fn3
	.byte	W06
	.byte	W06
	.byte		        Gn3
	.byte	W24
	.byte		N23   , Cn3
	.byte	W24
	.byte		        Dn3
	.byte	W24
	.byte		        En3
	.byte	W18
	.byte	W12
	.byte		VOICE , 12
	.byte		VOL   , 18*mus_qz_627_mvl/mxv
	.byte		PAN   , c_v+46
	.byte		VOL   , 19*mus_qz_627_mvl/mxv
	.byte		N11   , En3 , v088
	.byte	W12
	.byte		VOICE , 11
	.byte		N05   , Fn3
	.byte	W12
	.byte	N05
	.byte	W12
	.byte		        Gn3
	.byte	W12
	.byte		N02   , Gs3
	.byte	W03
	.byte		N20   , An3
	.byte	W21
	.byte		VOICE , 12
	.byte		N05   , Gn3
	.byte	W12
	.byte		VOICE , 11
	.byte		N05   , Fn3
	.byte	W12
	.byte		VOICE , 12
	.byte		N05   , Gn3
	.byte	W12
	.byte		VOICE , 11
	.byte		N05   , Fn3
	.byte	W12
	.byte		        Gn3
	.byte	W06
	.byte		N02   , Fn3
	.byte	W12
	.byte		N28   , Dn3 , v088 , gtp1
	.byte	W42
	.byte		N11   , Fn3
	.byte	W12
	.byte		        Ds3
	.byte	W12
	.byte		N05   , En3
	.byte	W12
	.byte	N05
	.byte	W12
	.byte		        Fn3
	.byte	W09
	.byte	N02
	.byte	W03
	.byte		        Fs3
	.byte	W03
	.byte		N20   , Gn3
	.byte	W21
	.byte		N05   , Fn3
	.byte	W12
	.byte		        En3
	.byte	W12
	.byte		        Fn3
	.byte	W12
	.byte		        Gn3
	.byte	W12
	.byte		        Fn3
	.byte	W06
	.byte		        Dn3
	.byte	W12
	.byte		N28   , Cn3 , v088 , gtp1
	.byte	W42
	.byte		N11   , Bn2
	.byte	W12
	.byte		VOL   , 16*mus_qz_627_mvl/mxv
	.byte		PAN   , c_v-24
	.byte		N11   , En3
	.byte	W12
	.byte		N05   , Fn3
	.byte	W12
	.byte	N05
	.byte	W12
	.byte		        Gn3
	.byte	W12
	.byte		N02   , Gs3
	.byte	W03
	.byte		N20   , An3
	.byte	W21
	.byte		N05   , Gn3
	.byte	W12
	.byte		        Fn3
	.byte	W12
	.byte		N11   , As3
	.byte	W12
	.byte		N05   , An3
	.byte	W12
	.byte		        As3
	.byte	W06
	.byte		        Bn3
	.byte	W06
	.byte		        Cn4
	.byte	W06
	.byte		N32   , Dn4 , v088 , gtp3
	.byte	W32
	.byte	W01
	.byte		VOICE , 13
	.byte	W03
	.byte		VOL   , 19*mus_qz_627_mvl/mxv
	.byte		        22*mus_qz_627_mvl/mxv
	.byte		N05   , En4
	.byte	W06
	.byte		        Dn4
	.byte	W06
	.byte		PAN   , c_v-44
	.byte		        c_v+41
	.byte		N23   , Cn4
	.byte	W24
	.byte		        As3
	.byte	W24
	.byte		        An3
	.byte	W24
	.byte		        Gn3 , v056
	.byte	W18
	.byte	W06
	.byte		        Cn4 , v088
	.byte	W24
	.byte		        As3
	.byte	W24
	.byte		        An3
	.byte	W24
	.byte		        Dn4 , v064
	.byte	W18
	.byte	W06
	.byte		        Cn4 , v088
	.byte	W18
	.byte		BEND  , c_v+12
	.byte	W03
	.byte		        c_v+0
	.byte	W03
	.byte		N23   , As3
	.byte	W24
	.byte		N11   , An3
	.byte	W12
	.byte		N05   , Cn3 , v076
	.byte	W12
	.byte		N11   , Gn3 , v088
	.byte	W12
	.byte		N05   , Cn3 , v032
	.byte	W06
	.byte	W06
	.byte		N80   , Fn3 , v104 , gtp3
	.byte	W24
	.byte		VOL   , 16*mus_qz_627_mvl/mxv
	.byte	W02
	.byte		        15*mus_qz_627_mvl/mxv
	.byte	W03
	.byte		        15*mus_qz_627_mvl/mxv
	.byte	W03
	.byte		        14*mus_qz_627_mvl/mxv
	.byte	W06
	.byte		        12*mus_qz_627_mvl/mxv
	.byte	W06
	.byte		        11*mus_qz_627_mvl/mxv
	.byte	W06
	.byte		        10*mus_qz_627_mvl/mxv
	.byte	W03
	.byte		        10*mus_qz_627_mvl/mxv
	.byte	W09
	.byte		        10*mus_qz_627_mvl/mxv
	.byte	W06
	.byte		        9*mus_qz_627_mvl/mxv
	.byte	W03
	.byte		        8*mus_qz_627_mvl/mxv
	.byte	W03
	.byte		        8*mus_qz_627_mvl/mxv
	.byte	W06
	.byte		        8*mus_qz_627_mvl/mxv
	.byte	W03
	.byte		        8*mus_qz_627_mvl/mxv
	.byte	W01
	.byte		N05   , Fn3 , v127
	.byte	W02
	.byte		VOL   , 7*mus_qz_627_mvl/mxv
	.byte	W04
	.byte		N05   , Gn3
	.byte	W06
	.byte		N44   , An3 , v088 , gtp3
	.byte	W02
	.byte		VOL   , 6*mus_qz_627_mvl/mxv
	.byte	W03
	.byte		        6*mus_qz_627_mvl/mxv
	.byte	W03
	.byte		        6*mus_qz_627_mvl/mxv
	.byte	W06
	.byte		        5*mus_qz_627_mvl/mxv
	.byte	W01
	.byte		        5*mus_qz_627_mvl/mxv
	.byte	W02
	.byte		        5*mus_qz_627_mvl/mxv
	.byte	W03
	.byte		        6*mus_qz_627_mvl/mxv
	.byte	W04
	.byte		        8*mus_qz_627_mvl/mxv
	.byte	W02
	.byte		        11*mus_qz_627_mvl/mxv
	.byte	W03
	.byte		        16*mus_qz_627_mvl/mxv
	.byte	W03
	.byte		        20*mus_qz_627_mvl/mxv
	.byte	W28
	.byte		N05   , Cn3 , v076
	.byte	W12
	.byte		        Fn3 , v080
	.byte	W12
	.byte		        En3 , v088
	.byte	W06
	.byte	GOTO
	 ptrvalue	mus_qz_627_9_B1
	.byte	FINE

/********************** Track 10 **********************/

mus_qz_627_10:
	.byte		KEYSH , mus_qz_627_key+0
	.byte		VOICE , 39
	.byte		VOL   , 53*mus_qz_627_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		BENDR , 12
	.byte		PRIO  , 64
	.byte	W06
	.byte		VOL   , 69*mus_qz_627_mvl/mxv
	.byte		BEND  , c_v+16
	.byte		N11   , Bn0 , v088
	.byte	W12
	.byte		N05   , As1 , v104
	.byte	W12
	.byte		N11   , Bn0 , v088
	.byte	W12
	.byte		N05   , As1
	.byte	W12
	.byte		N11   , Bn0
	.byte	W12
	.byte		N05   , As1 , v092
	.byte	W12
	.byte		N11   , Bn0 , v088
	.byte	W12
	.byte		N05   , As1
	.byte	W12
	.byte		N11   , Bn0
	.byte	W12
	.byte		N05   , As1 , v104
	.byte	W12
	.byte		N11   , Bn0 , v088
	.byte	W12
	.byte		N05   , As1
	.byte	W12
	.byte		N11   , Bn0
	.byte	W12
	.byte		        As1
	.byte	W12
	.byte		        As0
	.byte	W12
	.byte		N05   , Cn1
	.byte		N11   , As1
	.byte	W06
	.byte		N05   , As0
	.byte	W06
	.byte		        Bn0
	.byte	W06
	.byte		        As0
	.byte	W06
	.byte		N11   , As1
	.byte	W12
	.byte		        As0
	.byte	W12
	.byte		        As1
	.byte	W12
	.byte		VOL   , 63*mus_qz_627_mvl/mxv
	.byte		N11   , Bn0
	.byte	W12
	.byte		        As0
	.byte		N11   , As1
	.byte	W12
	.byte		N05   , As0 , v100
	.byte	W06
	.byte		        As0 , v088
	.byte	W06
	.byte		N11   , As1
	.byte	W12
	.byte		N05   , As0 , v100
	.byte	W06
	.byte		        As0 , v088
	.byte	W06
	.byte		        As0 , v100
	.byte		N11   , As1
	.byte	W06
	.byte		N05   , As0 , v088
	.byte	W06
	.byte		        As0 , v112
	.byte	W06
	.byte		        As0 , v088
	.byte	W06
	.byte		N11   , Bn0 , v104
	.byte		N11   , As1
	.byte	W12
mus_qz_627_10_B1:
	.byte		BEND  , c_v+16
	.byte		N11   , Bn0 , v088
	.byte		N05   , Ds2 , v112
	.byte	W12
	.byte		N11   , As1 , v088
	.byte	W12
	.byte		        As0
	.byte	W12
	.byte		        As1
	.byte	W06
	.byte		N05   , Cn1
	.byte	W06
	.byte		N11   , Bn0
	.byte	W12
	.byte		        As1
	.byte	W12
	.byte		        As0
	.byte	W12
	.byte		        As1
	.byte	W12
	.byte		        Bn0
	.byte	W12
	.byte		        As1
	.byte	W12
	.byte		        As0
	.byte	W12
	.byte		        As1
	.byte	W06
	.byte		N05   , Cn1
	.byte	W06
	.byte		N11   , Bn0
	.byte	W12
	.byte	N11
	.byte		N11   , As1
	.byte	W12
	.byte		N05   , As0
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		N11   , Fn1
	.byte		N11   , As1
	.byte	W12
	.byte		        Bn0
	.byte	W12
	.byte		        As1
	.byte	W12
	.byte		        As0
	.byte	W12
	.byte		        As1
	.byte	W06
	.byte		N05   , Cn1
	.byte	W06
	.byte		N11   , Bn0
	.byte	W12
	.byte		        As1
	.byte	W12
	.byte		        As0
	.byte	W12
	.byte		        As1
	.byte	W12
	.byte		        Bn0
	.byte	W12
	.byte		N05   , As0
	.byte		N11   , As1
	.byte	W06
	.byte		N05   , As0 , v044
	.byte	W06
	.byte		        As0 , v088
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		N08   , As1 , v084
	.byte	W06
	.byte		N05   , As0 , v080
	.byte	W06
	.byte		N11   , Bn0 , v088
	.byte	W12
	.byte		        As1
	.byte	W12
	.byte		        As0
	.byte	W12
	.byte		        As1
	.byte	W06
	.byte		N05   , As0
	.byte	W06
	.byte		N11   , Bn0
	.byte	W12
	.byte		        As1
	.byte	W12
	.byte		        As0
	.byte	W12
	.byte		        As1
	.byte	W12
	.byte		        Bn0
	.byte	W12
	.byte		        As1
	.byte	W12
	.byte		N05   , Bn0
	.byte	W06
	.byte		        As0
	.byte	W06
	.byte		N11   , As1
	.byte	W12
	.byte		        Bn0
	.byte	W12
	.byte		        As1
	.byte	W12
	.byte		        As0
	.byte	W12
	.byte		        As1
	.byte	W12
	.byte		N05   , As0
	.byte	W06
	.byte	N05
	.byte	W06
	.byte	N05
	.byte		N11   , As1
	.byte	W06
	.byte		N05   , As0
	.byte	W06
	.byte		BEND  , c_v-32
	.byte		N05   , As0 , v100
	.byte	W06
	.byte		        As0 , v088
	.byte	W06
	.byte		BEND  , c_v+16
	.byte	N05
	.byte		N11   , As1
	.byte	W06
	.byte		N05   , As0
	.byte	W06
	.byte		N11   , Bn0
	.byte	W12
	.byte		        As1
	.byte	W12
	.byte		        As0
	.byte	W12
	.byte		N05   , Bn0
	.byte		N11   , As1
	.byte	W06
	.byte		N05   , As0
	.byte	W06
	.byte		        Cn1
	.byte	W06
	.byte		        As0
	.byte	W06
	.byte		        Bn0
	.byte		N11   , As1
	.byte	W12
	.byte		N05   , As0
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		N11   , As1 , v116
	.byte	W12
	.byte		        As0 , v104
	.byte		N08   , As2
	.byte	W12
	.byte		N11   , Cn1 , v088
	.byte	W12
	.byte		        Bn0
	.byte	W12
	.byte		        As1
	.byte	W12
	.byte		        Bn0
	.byte	W12
	.byte		        As1
	.byte	W12
	.byte		        As0
	.byte	W12
	.byte		        As1
	.byte	W12
mus_qz_627_10_000:
	.byte		N11   , Bn0 , v088
	.byte	W12
	.byte		        As1
	.byte	W12
	.byte		        As0
	.byte	W12
	.byte		        As1
	.byte	W12
	.byte		        Bn0
	.byte	W12
	.byte		        As1
	.byte	W12
	.byte		        As0
	.byte	W12
	.byte		        As1
	.byte	W12
	.byte	PEND
	.byte		        Bn0
	.byte	W12
	.byte		        As1
	.byte	W12
	.byte		        As0
	.byte	W12
	.byte		        As1
	.byte	W06
	.byte		N05   , Bn0
	.byte	W06
	.byte	N11
	.byte	W12
	.byte	N11
	.byte		N11   , As1
	.byte	W12
	.byte		N05   , Bn0
	.byte	W06
	.byte		        As0
	.byte	W06
	.byte		N11   , As1 , v104
	.byte	W03
	.byte		BEND  , c_v+21
	.byte	W09
mus_qz_627_10_001:
	.byte		BEND  , c_v+16
	.byte		N11   , Bn0 , v088
	.byte	W12
	.byte		        As1
	.byte	W12
	.byte		        As0
	.byte	W12
	.byte		        As1
	.byte	W12
	.byte		        Bn0
	.byte	W12
	.byte		        As1
	.byte	W12
	.byte		        As0
	.byte	W12
	.byte		        As1
	.byte	W12
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_627_10_000
	.byte	PATT
	 ptrvalue	mus_qz_627_10_000
	.byte		N11   , Bn0 , v088
	.byte	W12
	.byte		        As1
	.byte	W12
	.byte		        As0
	.byte	W12
	.byte		        As1
	.byte	W06
	.byte		N05   , Bn0
	.byte	W06
	.byte	N11
	.byte	W12
	.byte	N11
	.byte		N11   , As1
	.byte	W12
	.byte		N05   , Bn0
	.byte	W06
	.byte		        As0
	.byte	W06
	.byte		N11   , Bn0
	.byte		N11   , As1 , v104
	.byte	W03
	.byte		BEND  , c_v+21
	.byte	W09
	.byte	PATT
	 ptrvalue	mus_qz_627_10_001
	.byte		N11   , Bn0 , v088
	.byte	W12
	.byte		        As1
	.byte	W12
	.byte		        Bn0
	.byte	W12
	.byte		        As1
	.byte	W12
	.byte		        Bn0
	.byte	W12
	.byte	N11
	.byte		N11   , As1
	.byte	W12
	.byte		        Bn0
	.byte	W12
	.byte	N11
	.byte		N11   , As1
	.byte	W12
	.byte		        Bn0
	.byte	W12
	.byte		        Cn1
	.byte		N11   , As1
	.byte	W12
	.byte		        Bn0
	.byte	W12
	.byte		        Cn1
	.byte		N11   , As1
	.byte	W12
	.byte		N05   , As0 , v052
	.byte		N05   , Bn0 , v088
	.byte	W06
	.byte		        As0 , v056
	.byte		N05   , Bn0
	.byte	W06
	.byte		        As0 , v060
	.byte		N05   , Bn0 , v088
	.byte	W06
	.byte		        As0 , v068
	.byte		N05   , Bn0 , v056
	.byte	W06
	.byte		N02   , As0 , v088
	.byte		N05   , Bn0
	.byte	W03
	.byte		N02   , As0
	.byte	W03
	.byte	N05
	.byte		N05   , Bn0
	.byte	W06
	.byte		        As0
	.byte		N05   , Bn0
	.byte	W06
	.byte		        As0
	.byte		N05   , Bn0
	.byte	W06
	.byte		BEND  , c_v+0
	.byte	N11
	.byte		N11   , En2
	.byte	W12
	.byte		        Fs2 , v056
	.byte	W12
	.byte		        Bn0 , v088
	.byte	W12
	.byte		        Fs2 , v056
	.byte	W12
	.byte		        Bn0 , v088
	.byte		N05   , Fs2 , v056
	.byte	W06
	.byte		        Fs2 , v024
	.byte	W06
	.byte		N11   , Fs2 , v068
	.byte	W12
	.byte		        Bn0 , v088
	.byte	W12
	.byte		        Fs2 , v056
	.byte	W12
	.byte		        Bn0 , v088
	.byte		N05   , Fs2 , v056
	.byte	W06
	.byte		        Fs2 , v024
	.byte	W06
	.byte		N11   , Fs2 , v068
	.byte	W12
	.byte		        Bn0 , v088
	.byte	W12
	.byte		        Fs2 , v056
	.byte	W12
	.byte		BEND  , c_v+15
	.byte		N11   , Gs2 , v088
	.byte	W12
	.byte		BEND  , c_v+60
	.byte	N11
	.byte	W12
	.byte		BEND  , c_v+40
	.byte		N11   , Gs2 , v056
	.byte		N08   , As2 , v088
	.byte	W12
	.byte		BEND  , c_v+0
	.byte	W12
	.byte	GOTO
	 ptrvalue	mus_qz_627_10_B1
	.byte	FINE

/******************************************************/
	.p2align	2

mus_qz_627:
	.byte	10	/* NumTrks */
	.byte	0	/* NumBlks */
	.byte	mus_qz_627_pri	/* Priority */
	.byte	mus_qz_627_rev	/* Reverb. */
space64 4
	ptrvalue	mus_qz_627_grp

	ptrvalue	mus_qz_627_1
	ptrvalue	mus_qz_627_2
	ptrvalue	mus_qz_627_3
	ptrvalue	mus_qz_627_4
	ptrvalue	mus_qz_627_5
	ptrvalue	mus_qz_627_6
	ptrvalue	mus_qz_627_7
	ptrvalue	mus_qz_627_8
	ptrvalue	mus_qz_627_9
	ptrvalue	mus_qz_627_10

	.end
