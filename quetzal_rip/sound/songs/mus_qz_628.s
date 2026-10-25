	.include "asm/macros/bit_width.inc"

	.include "MPlayDef.s"

/* Moemon Quetzal song 628, disassembled from the ROM by tools/quetzal_rip/songs.py.
   "org" comments give the ROM address of the bytes that follow. */

	.equ	mus_qz_628_grp, voicegroup_qz_songs
	.equ	mus_qz_628_pri, 0
	.equ	mus_qz_628_rev, reverb_set+0
	.equ	mus_qz_628_mvl, 127
	.equ	mus_qz_628_key, 0
	.equ	mus_qz_628_tbs, 1
	.equ	mus_qz_628_exg, 1
	.equ	mus_qz_628_cmp, 1

	.section .rodata
	.global	mus_qz_628
	.p2align	2
/* org 0x09680330 */

/********************** Track  1 **********************/

mus_qz_628_1:
	.byte		KEYSH , mus_qz_628_key+0
	.byte		TEMPO , 92*mus_qz_628_tbs/2
	.byte		VOICE , 42
	.byte		VOL   , 68*mus_qz_628_mvl/mxv
	.byte		PAN   , c_v-32
	.byte		BENDR , 6
	.byte		PRIO  , 62
	.byte		MOD   , 3
	.byte		LFOS  , 36
	.byte		LFODL , 10
	.byte	W09
	.byte		N05   , Bn3 , v096
	.byte		N05   , En4 , v112
	.byte	W12
	.byte		        En2 , v084
	.byte		N05   , An3 , v100
	.byte	W12
	.byte		        Bn3 , v112
	.byte		N05   , En4
	.byte	W12
	.byte		        En2 , v084
	.byte		N05   , An3 , v100
	.byte	W12
	.byte		        En4 , v096
	.byte		N05   , Bn4 , v112
	.byte	W12
	.byte		        Dn4 , v084
	.byte		N05   , An4 , v100
	.byte	W12
	.byte		        Bn3 , v096
	.byte		N05   , Fn4 , v112
	.byte	W12
	.byte		        Dn4 , v084
	.byte		N05   , An4 , v100
	.byte	W12
	.byte		N02   , Dn4 , v048
	.byte	W03
	.byte		        Fn4 , v044
	.byte	W03
	.byte		        Dn4 , v056
	.byte	W03
	.byte		        Fn4 , v052
	.byte	W03
	.byte		        Dn4 , v060
	.byte	W03
	.byte		        Fn4 , v056
	.byte	W03
	.byte		        Dn4 , v068
	.byte	W03
	.byte		        Fn4 , v064
	.byte	W03
	.byte		        Dn4 , v076
	.byte	W03
	.byte		        Fn4 , v072
	.byte	W03
	.byte		        Dn4 , v084
	.byte	W03
	.byte		        Fn4 , v080
	.byte	W03
	.byte		        Dn4 , v088
	.byte	W03
	.byte		        Fn4 , v084
	.byte	W03
	.byte		        Dn4 , v100
	.byte	W03
	.byte		        Fn4 , v092
	.byte	W03
	.byte		        Bn3 , v112
	.byte	W03
	.byte		        En4 , v096
	.byte	W03
	.byte		        Bn3 , v112
	.byte	W03
	.byte		        En4 , v096
	.byte	W03
	.byte		        An3 , v100
	.byte	W03
	.byte		        Dn4 , v084
	.byte	W03
	.byte		        An3 , v100
	.byte	W03
	.byte		        Dn4 , v084
	.byte	W03
	.byte		        En3
	.byte	W03
	.byte		        Bn3 , v072
	.byte	W03
	.byte		        En3 , v084
	.byte	W03
	.byte		        Bn3 , v072
	.byte	W03
	.byte		        An3 , v080
	.byte	W03
	.byte		        Dn4 , v068
	.byte	W03
	.byte		        An3 , v080
	.byte	W03
	.byte		        Dn4 , v068
	.byte	W03
mus_qz_628_1_B1:
	.byte		N05   , Bn3 , v096
	.byte		N05   , En4 , v112
	.byte	W12
	.byte		        En2 , v084
	.byte		N05   , An3 , v100
	.byte	W12
	.byte		        Bn3 , v112
	.byte		N05   , En4
	.byte	W12
	.byte		        En2 , v084
	.byte		N05   , An3 , v100
	.byte	W12
	.byte		        En4 , v096
	.byte		N05   , Bn4 , v112
	.byte	W12
	.byte		        Dn4 , v084
	.byte		N05   , An4 , v100
	.byte	W12
	.byte		        Bn3 , v096
	.byte		N05   , Fn4 , v112
	.byte	W12
	.byte		        Dn4 , v084
	.byte		N05   , An4 , v100
	.byte	W12
	.byte		N02   , Dn4 , v044
	.byte	W03
	.byte		        Fn4 , v052
	.byte	W03
	.byte		        Dn4 , v048
	.byte	W03
	.byte		        Fn4 , v056
	.byte	W03
	.byte		        Dn4 , v052
	.byte	W03
	.byte		        Fn4 , v064
	.byte	W03
	.byte		        Dn4 , v060
	.byte	W03
	.byte		        Fn4 , v072
	.byte	W03
	.byte		        Dn4 , v068
	.byte	W03
	.byte		        Fn4 , v080
	.byte	W03
	.byte		        Dn4 , v076
	.byte	W03
	.byte		        Fn4 , v088
	.byte	W03
	.byte		        Dn4 , v080
	.byte	W03
	.byte		        Fn4 , v092
	.byte	W03
	.byte		        Dn4 , v088
	.byte	W03
	.byte		        Fn4 , v104
	.byte	W03
	.byte		        Bn3 , v100
	.byte	W03
	.byte		        En4 , v096
	.byte	W03
	.byte		        Bn3 , v100
	.byte	W03
	.byte		        En4 , v096
	.byte	W03
	.byte		        An3 , v100
	.byte	W03
	.byte		        Dn4 , v092
	.byte	W03
	.byte		        An3 , v100
	.byte	W03
	.byte		        Dn4 , v092
	.byte	W03
	.byte		        En3 , v084
	.byte	W03
	.byte		        Bn3 , v080
	.byte	W03
	.byte		        En3 , v084
	.byte	W03
	.byte		        Bn3 , v080
	.byte	W03
	.byte		        An3
	.byte	W03
	.byte		        Dn4 , v072
	.byte	W03
	.byte		        An3 , v080
	.byte	W03
	.byte		        Dn4 , v072
	.byte	W03
	.byte		N32   , Dn2 , v120 , gtp3
	.byte	W36
	.byte		N04   , En2 , v116
	.byte	W06
	.byte		        Fn2 , v104
	.byte	W05
	.byte		BEND  , c_v-11
	.byte	W01
	.byte		N44   , An2 , v124 , gtp3
	.byte	W02
	.byte		BEND  , c_v+0
	.byte	W44
	.byte	W02
	.byte		N32   , Bn2 , v124 , gtp3
	.byte	W36
	.byte		N04   , An2 , v116
	.byte	W06
	.byte		        Bn2 , v100
	.byte	W05
	.byte		BEND  , c_v-11
	.byte	W01
	.byte		N32   , En3 , v124 , gtp3
	.byte	W02
	.byte		BEND  , c_v+0
	.byte	W32
	.byte	W02
	.byte		N04   , Dn3 , v116
	.byte	W06
	.byte		        En3 , v104
	.byte	W06
	.byte		N05   , Bn2 , v100
	.byte		N05   , Fn3
	.byte	W06
	.byte		        Dn3 , v096
	.byte		N05   , An3
	.byte	W06
	.byte		        Fn3 , v104
	.byte		N05   , Bn3
	.byte	W06
	.byte		        An3 , v116
	.byte		N05   , Dn4
	.byte	W06
	.byte		        Bn3
	.byte		N05   , En4
	.byte	W06
	.byte		        An3 , v104
	.byte		N05   , Dn4
	.byte	W06
	.byte		        Bn3 , v116
	.byte		N05   , En4
	.byte	W06
	.byte		        Dn4 , v104
	.byte		N05   , An4
	.byte	W06
	.byte		N11   , Fn4 , v116
	.byte		N11   , Bn4
	.byte	W12
	.byte		N03   , Dn4
	.byte		N03   , An4
	.byte	W04
	.byte		        En4 , v104
	.byte		N03   , Bn4
	.byte	W04
	.byte		        Dn4 , v116
	.byte		N03   , An4
	.byte	W04
	.byte		N07   , Bn3
	.byte		N07   , Fn4
	.byte	W12
	.byte		        An3 , v104
	.byte		N07   , Dn4
	.byte	W12
	.byte		N05   , Bn3 , v116
	.byte		N05   , En4
	.byte	W06
	.byte		        An3 , v104
	.byte		N05   , Dn4
	.byte	W06
	.byte		        Dn3 , v116
	.byte		N05   , An3
	.byte	W06
	.byte		        En3 , v104
	.byte		N05   , Bn3
	.byte	W06
	.byte		N11   , Fn4 , v116
	.byte		N11   , An4
	.byte	W12
	.byte		N03   , Bn3
	.byte		N03   , Fn4
	.byte	W04
	.byte		        Dn4 , v104
	.byte		N03   , An4
	.byte	W04
	.byte		        Bn3 , v116
	.byte		N03   , Fn4
	.byte	W04
	.byte		N05   , An3
	.byte		N05   , En4
	.byte	W12
	.byte		        Bn3
	.byte		N05   , Fn4
	.byte	W06
	.byte		        Bn3 , v104
	.byte		N05   , Fn4
	.byte	W06
	.byte		        An3 , v116
	.byte		N05   , En4
	.byte	W12
	.byte		N11   , Fn3 , v104
	.byte		N11   , Dn4
	.byte	W12
	.byte		N05   , Bn3 , v100
	.byte		N05   , En4 , v116
	.byte	W12
	.byte		        En2 , v088
	.byte		N05   , An3 , v104
	.byte	W12
	.byte		        Bn3 , v116
	.byte		N05   , En4
	.byte	W12
	.byte		        En2 , v088
	.byte		N05   , An3 , v104
	.byte	W12
	.byte		        En4 , v100
	.byte		N05   , Bn4 , v116
	.byte	W12
	.byte		        Dn4 , v088
	.byte		N05   , An4 , v104
	.byte	W12
	.byte		        Bn3 , v100
	.byte		N05   , Fn4 , v116
	.byte	W12
	.byte		        Dn4 , v088
	.byte		N05   , An4 , v104
	.byte	W12
	.byte		N02   , Dn4 , v052
	.byte	W03
	.byte		        Fn4 , v048
	.byte	W03
	.byte		        Dn4 , v060
	.byte	W03
	.byte		        Fn4 , v056
	.byte	W03
	.byte		        Dn4 , v064
	.byte	W03
	.byte		        Fn4 , v060
	.byte	W03
	.byte		        Dn4 , v072
	.byte	W03
	.byte		        Fn4 , v068
	.byte	W03
	.byte		        Dn4 , v080
	.byte	W03
	.byte		        Fn4 , v076
	.byte	W03
	.byte		        Dn4 , v088
	.byte	W03
	.byte		        Fn4 , v084
	.byte	W03
	.byte		        Dn4 , v096
	.byte	W03
	.byte		        Fn4 , v092
	.byte	W03
	.byte		        Dn4 , v104
	.byte	W03
	.byte		        Fn4 , v100
	.byte	W03
	.byte		        Bn3 , v116
	.byte	W03
	.byte		        En4 , v108
	.byte	W03
	.byte		        Bn3 , v116
	.byte	W03
	.byte		        En4 , v108
	.byte	W03
	.byte		        An3 , v104
	.byte	W03
	.byte		        Dn4 , v096
	.byte	W03
	.byte		        An3 , v104
	.byte	W03
	.byte		        Dn4 , v096
	.byte	W03
	.byte		        En3 , v088
	.byte	W03
	.byte		        Bn3 , v080
	.byte	W03
	.byte		        En3 , v088
	.byte	W03
	.byte		        Bn3 , v080
	.byte	W03
	.byte		        An3 , v084
	.byte	W03
	.byte		        Dn4 , v076
	.byte	W03
	.byte		        An3 , v084
	.byte	W03
	.byte		        Dn4 , v076
	.byte	W03
	.byte	GOTO
	 ptrvalue	mus_qz_628_1_B1
	.byte	FINE

/********************** Track  2 **********************/

mus_qz_628_2:
	.byte		KEYSH , mus_qz_628_key+0
	.byte		VOICE , 35
	.byte		VOL   , 74*mus_qz_628_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte		MOD   , 3
	.byte		LFOS  , 36
	.byte		LFODL , 10
	.byte		VOL   , 57*mus_qz_628_mvl/mxv
	.byte		N02   , An4 , v108
	.byte	W03
	.byte		        Bn4 , v100
	.byte	W03
	.byte		        Dn5 , v108
	.byte	W03
	.byte		N36   , En5 , v100 , gtp2
	.byte	W30
	.byte	W01
	.byte		VOL   , 56*mus_qz_628_mvl/mxv
	.byte	W02
	.byte		        51*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        46*mus_qz_628_mvl/mxv
	.byte	W02
	.byte		        40*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        34*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        57*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		N02   , Dn5 , v108
	.byte	W03
	.byte		        En5 , v100
	.byte	W03
	.byte		        An5 , v108
	.byte	W03
	.byte		N44   , Bn5 , v100 , gtp3
	.byte	W36
	.byte	W03
	.byte		VOL   , 51*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        46*mus_qz_628_mvl/mxv
	.byte	W02
	.byte		        40*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        33*mus_qz_628_mvl/mxv
	.byte	W02
	.byte		        29*mus_qz_628_mvl/mxv
	.byte	W02
	.byte		        59*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		N44   , Fn2 , v088 , gtp3
	.byte	W42
	.byte		VOL   , 53*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        47*mus_qz_628_mvl/mxv
	.byte	W02
	.byte		        40*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        30*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        59*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		N40   , En2 , v076 , gtp1
	.byte	W36
	.byte		VOL   , 54*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        48*mus_qz_628_mvl/mxv
	.byte	W02
	.byte		        40*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        32*mus_qz_628_mvl/mxv
	.byte	W02
	.byte		        59*mus_qz_628_mvl/mxv
	.byte		N02   , En5 , v108
	.byte	W03
	.byte		        Fn5 , v104
	.byte	W03
mus_qz_628_2_B1:
	.byte		N36   , En5 , v100 , gtp2
	.byte	W32
	.byte	W01
	.byte		VOL   , 50*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        42*mus_qz_628_mvl/mxv
	.byte	W02
	.byte		        34*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        28*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        57*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		N02   , Dn5 , v108
	.byte	W03
	.byte		        En5 , v100
	.byte	W03
	.byte		        An5 , v108
	.byte	W03
	.byte		N44   , Bn5 , v100 , gtp3
	.byte	W42
	.byte		VOL   , 48*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        42*mus_qz_628_mvl/mxv
	.byte	W02
	.byte		        37*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        29*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        57*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		N44   , Fn2 , v088 , gtp3
	.byte	W42
	.byte		VOL   , 47*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        41*mus_qz_628_mvl/mxv
	.byte	W02
	.byte		        35*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        29*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        57*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		N44   , En2 , v076 , gtp3
	.byte	W42
	.byte	W01
	.byte		VOL   , 51*mus_qz_628_mvl/mxv
	.byte	W02
	.byte		        45*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        39*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        57*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		N05   , Fn3 , v108
	.byte	W06
	.byte		        An3 , v096
	.byte	W06
	.byte		        Bn3 , v108
	.byte	W06
	.byte		        Dn4 , v096
	.byte	W06
	.byte		        En4 , v108
	.byte	W06
	.byte		        Dn4 , v096
	.byte	W06
	.byte		        En4 , v108
	.byte	W06
	.byte		        An4 , v096
	.byte	W06
	.byte		N02   , As4 , v108
	.byte	W03
	.byte		N08   , Bn4
	.byte	W09
	.byte		N03   , An4
	.byte	W04
	.byte		        Bn4
	.byte	W04
	.byte		        An4
	.byte	W04
	.byte		N11   , Fn4
	.byte	W12
	.byte		        Dn4
	.byte	W12
	.byte		N05   , En4
	.byte	W06
	.byte		        Dn4 , v096
	.byte	W06
	.byte		        An3 , v108
	.byte	W06
	.byte		        Bn3 , v096
	.byte	W06
	.byte		N11   , An4 , v108
	.byte	W12
	.byte		N03   , Fn4
	.byte	W04
	.byte		        An4
	.byte	W04
	.byte		        Fn4
	.byte	W04
	.byte		N02   , Ds4
	.byte	W03
	.byte		N08   , En4
	.byte	W09
	.byte		N11   , Dn4
	.byte	W12
	.byte		        Bn3
	.byte	W12
	.byte		        Dn4
	.byte	W12
	.byte		N44   , Dn2 , v088 , gtp3
	.byte	W40
	.byte		VOL   , 52*mus_qz_628_mvl/mxv
	.byte	W02
	.byte		        48*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        43*mus_qz_628_mvl/mxv
	.byte	W02
	.byte		        34*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        29*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        57*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		N44   , Fn2 , v076 , gtp3
	.byte	W42
	.byte		VOL   , 51*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        46*mus_qz_628_mvl/mxv
	.byte	W02
	.byte		        42*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        37*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        57*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		N80   , En2 , v080 , gtp1
	.byte	W72
	.byte		VOL   , 52*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        49*mus_qz_628_mvl/mxv
	.byte	W02
	.byte		        44*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        40*mus_qz_628_mvl/mxv
	.byte	W02
	.byte		        36*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        33*mus_qz_628_mvl/mxv
	.byte	W02
	.byte		        30*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        28*mus_qz_628_mvl/mxv
	.byte	W02
	.byte		        23*mus_qz_628_mvl/mxv
	.byte	W02
	.byte		        59*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		N02   , An4 , v108
	.byte	W03
	.byte		        Bn4
	.byte	W03
	.byte		        Dn5
	.byte	W03
	.byte		N36   , En5 , v100 , gtp2
	.byte	W30
	.byte	W01
	.byte		VOL   , 53*mus_qz_628_mvl/mxv
	.byte	W02
	.byte		        48*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        44*mus_qz_628_mvl/mxv
	.byte	W02
	.byte		        37*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        32*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        57*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		N02   , Dn5 , v108
	.byte	W03
	.byte		        En5 , v100
	.byte	W03
	.byte		        An5 , v108
	.byte	W03
	.byte		N44   , Bn5 , v100 , gtp3
	.byte	W36
	.byte	W03
	.byte		VOL   , 51*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        46*mus_qz_628_mvl/mxv
	.byte	W02
	.byte		        40*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        33*mus_qz_628_mvl/mxv
	.byte	W02
	.byte		        29*mus_qz_628_mvl/mxv
	.byte	W02
	.byte		        59*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		N44   , Fn2 , v088 , gtp3
	.byte	W42
	.byte		VOL   , 53*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        47*mus_qz_628_mvl/mxv
	.byte	W02
	.byte		        40*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        30*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        59*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		N40   , En2 , v076 , gtp1
	.byte	W36
	.byte		VOL   , 54*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        48*mus_qz_628_mvl/mxv
	.byte	W02
	.byte		        40*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        32*mus_qz_628_mvl/mxv
	.byte	W02
	.byte		        59*mus_qz_628_mvl/mxv
	.byte		N02   , En5 , v108
	.byte	W03
	.byte		        Fn5 , v100
	.byte	W03
	.byte	GOTO
	 ptrvalue	mus_qz_628_2_B1
	.byte	FINE

/********************** Track  3 **********************/

mus_qz_628_3:
	.byte		KEYSH , mus_qz_628_key+0
	.byte		VOICE , 20
	.byte		VOL   , 73*mus_qz_628_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 60
	.byte	W09
mus_qz_628_3_000:
	.byte		N02   , En2 , v120
	.byte	W12
	.byte		        Bn1 , v112
	.byte	W12
	.byte		        En2 , v120
	.byte	W12
	.byte		        Bn1 , v112
	.byte	W12
	.byte		        En2 , v120
	.byte	W12
	.byte		        Bn1 , v112
	.byte	W12
	.byte		        En2 , v120
	.byte	W12
	.byte		        Bn1 , v112
	.byte	W12
	.byte	PEND
mus_qz_628_3_001:
	.byte		N44   , Dn2 , v088 , gtp3
	.byte	W48
	.byte		        En2 , v096 , gtp3
	.byte	W48
	.byte	PEND
mus_qz_628_3_B1:
	.byte	PATT
	 ptrvalue	mus_qz_628_3_000
	.byte		N44   , Dn2 , v100 , gtp3
	.byte	W48
	.byte		        En2 , v096 , gtp3
	.byte	W48
mus_qz_628_3_002:
	.byte		N44   , Dn2 , v100 , gtp3
	.byte	W48
	.byte		        Fn2 , v096 , gtp3
	.byte	W48
	.byte	PEND
	.byte		N92   , En2 , v088 , gtp3
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_628_3_002
	.byte		N92   , En2 , v088 , gtp3
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_628_3_000
	.byte	PATT
	 ptrvalue	mus_qz_628_3_001
	.byte	GOTO
	 ptrvalue	mus_qz_628_3_B1
	.byte	FINE

/********************** Track  4 **********************/

mus_qz_628_4:
	.byte		KEYSH , mus_qz_628_key+0
	.byte		VOICE , 67
	.byte		VOL   , 56*mus_qz_628_mvl/mxv
	.byte		PAN   , c_v+26
	.byte		PRIO  , 58
	.byte	W09
mus_qz_628_4_000:
	.byte		N03   , Bn2 , v092
	.byte		N03   , En3 , v100
	.byte	W12
	.byte		        Cn3 , v088
	.byte		N03   , Fn3 , v096
	.byte	W06
	.byte		        Cn3 , v076
	.byte		N03   , Fn3 , v084
	.byte	W06
	.byte		        Bn2 , v092
	.byte		N03   , En3 , v100
	.byte	W12
	.byte		        Cn3 , v088
	.byte		N03   , Fn3 , v096
	.byte	W06
	.byte		        Cn3 , v076
	.byte		N03   , Fn3 , v084
	.byte	W06
	.byte		        Bn2 , v092
	.byte		N03   , En3 , v100
	.byte	W12
	.byte		        Cn3 , v088
	.byte		N03   , Fn3 , v096
	.byte	W06
	.byte		        Cn3 , v076
	.byte		N03   , Fn3 , v084
	.byte	W06
	.byte		        Bn2 , v092
	.byte		N03   , En3 , v100
	.byte	W12
	.byte		        Cn3 , v088
	.byte		N03   , Fn3 , v096
	.byte	W06
	.byte		        Cn3 , v076
	.byte		N03   , Fn3 , v084
	.byte	W06
	.byte	PEND
mus_qz_628_4_001:
	.byte		N02   , An2 , v088
	.byte	W03
	.byte		        An2 , v072
	.byte	W03
	.byte		        An2 , v088
	.byte	W03
	.byte		        An2 , v072
	.byte	W03
	.byte		        An2 , v088
	.byte	W03
	.byte		        An2 , v072
	.byte	W03
	.byte		        An2 , v088
	.byte	W03
	.byte		        An2 , v068
	.byte	W03
	.byte		N03   , An2 , v088
	.byte	W04
	.byte		        An2 , v080
	.byte	W04
	.byte		        An2 , v072
	.byte	W04
	.byte		        An2 , v068
	.byte	W04
	.byte		        An2 , v060
	.byte	W04
	.byte		        An2 , v056
	.byte	W04
	.byte		N02   , Bn2 , v088
	.byte	W03
	.byte		        Bn2 , v072
	.byte	W03
	.byte		        Bn2 , v088
	.byte	W03
	.byte		        Bn2 , v072
	.byte	W03
	.byte		        Bn2 , v088
	.byte	W03
	.byte		        Bn2 , v072
	.byte	W03
	.byte		        Bn2 , v088
	.byte	W03
	.byte		        Bn2 , v068
	.byte	W03
	.byte		N03   , Bn2 , v088
	.byte	W04
	.byte		        Bn2 , v080
	.byte	W04
	.byte		        Bn2 , v072
	.byte	W04
	.byte		        Bn2 , v068
	.byte	W04
	.byte		        Bn2 , v060
	.byte	W04
	.byte		        Bn2 , v056
	.byte	W04
	.byte	PEND
mus_qz_628_4_B1:
	.byte	PATT
	 ptrvalue	mus_qz_628_4_000
	.byte	PATT
	 ptrvalue	mus_qz_628_4_001
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_628_4_000
	.byte	PATT
	 ptrvalue	mus_qz_628_4_001
	.byte	GOTO
	 ptrvalue	mus_qz_628_4_B1
	.byte	FINE

/********************** Track  5 **********************/

mus_qz_628_5:
	.byte		KEYSH , mus_qz_628_key+0
	.byte		VOICE , 23
	.byte		VOL   , 70*mus_qz_628_mvl/mxv
	.byte		PAN   , c_v-24
	.byte		PRIO  , 56
	.byte		VOL   , 20*mus_qz_628_mvl/mxv
	.byte	W09
	.byte	W96
	.byte		        23*mus_qz_628_mvl/mxv
	.byte		N44   , An2 , v084 , gtp3
	.byte		N44   , Dn3 , v100 , gtp3
	.byte	W06
	.byte		VOL   , 27*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        32*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        37*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        43*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        48*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        52*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        56*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        59*mus_qz_628_mvl/mxv
	.byte		N44   , Bn2 , v076 , gtp3
	.byte		N44   , En3 , v092 , gtp3
	.byte	W06
	.byte		VOL   , 57*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        53*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        48*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        45*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        39*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        28*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        19*mus_qz_628_mvl/mxv
	.byte	W06
mus_qz_628_5_B1:
	.byte	W96
	.byte		VOL   , 23*mus_qz_628_mvl/mxv
	.byte		N44   , An2 , v072 , gtp3
	.byte		N44   , Dn3 , v084 , gtp3
	.byte	W06
	.byte		VOL   , 27*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        32*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        37*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        43*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        48*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        52*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        56*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        59*mus_qz_628_mvl/mxv
	.byte		N44   , Bn2 , v064 , gtp3
	.byte		N44   , En3 , v076 , gtp3
	.byte	W06
	.byte		VOL   , 62*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        57*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        55*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        50*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        45*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        38*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        31*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        26*mus_qz_628_mvl/mxv
	.byte		N44   , Dn2 , v072 , gtp3
	.byte		N44   , Cn3 , v064 , gtp3
	.byte		N44   , Fn3 , v076 , gtp3
	.byte	W06
	.byte		VOL   , 30*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        34*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        37*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        40*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        44*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        47*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        51*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		N44   , Fn2 , v068 , gtp3
	.byte		N44   , Dn3 , v060 , gtp3
	.byte		N44   , An3 , v072 , gtp3
	.byte	W24
	.byte		VOL   , 45*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        39*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        34*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        31*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        27*mus_qz_628_mvl/mxv
	.byte		N92   , En2 , v072 , gtp3
	.byte		N92   , Bn2 , v064 , gtp3
	.byte		N92   , En3 , v076 , gtp3
	.byte	W06
	.byte		VOL   , 29*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        32*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        35*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        39*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        43*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        46*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        51*mus_qz_628_mvl/mxv
	.byte	W24
	.byte		        47*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        41*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        37*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        33*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        25*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        28*mus_qz_628_mvl/mxv
	.byte		N44   , Dn2 , v076 , gtp3
	.byte		N44   , Cn3 , v072 , gtp3
	.byte		N44   , Fn3 , v084 , gtp3
	.byte	W06
	.byte		VOL   , 30*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        34*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        36*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        39*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        42*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        45*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        48*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		N44   , Fn2 , v084 , gtp3
	.byte		N44   , An2 , v072 , gtp3
	.byte		N44   , Dn3 , v088 , gtp3
	.byte	W24
	.byte		VOL   , 44*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        39*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        29*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        24*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        28*mus_qz_628_mvl/mxv
	.byte		N92   , En2 , v076 , gtp3
	.byte		N92   , An2 , v072 , gtp3
	.byte		N92   , En3 , v084 , gtp3
	.byte	W06
	.byte		VOL   , 30*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        34*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        37*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        40*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        44*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        46*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        50*mus_qz_628_mvl/mxv
	.byte	W36
	.byte		        42*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        35*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        27*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        18*mus_qz_628_mvl/mxv
	.byte	W96
	.byte		        23*mus_qz_628_mvl/mxv
	.byte		N44   , An2 , v072 , gtp3
	.byte		N44   , Dn3 , v084 , gtp3
	.byte	W06
	.byte		VOL   , 27*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        32*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        37*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        43*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        48*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        52*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        56*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        59*mus_qz_628_mvl/mxv
	.byte		N44   , Bn2 , v064 , gtp3
	.byte		N44   , En3 , v076 , gtp3
	.byte	W06
	.byte		VOL   , 61*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        57*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        53*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        50*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        45*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        37*mus_qz_628_mvl/mxv
	.byte	W06
	.byte		        20*mus_qz_628_mvl/mxv
	.byte		        26*mus_qz_628_mvl/mxv
	.byte	W06
	.byte	GOTO
	 ptrvalue	mus_qz_628_5_B1
	.byte	FINE

/********************** Track  6 **********************/

mus_qz_628_6:
	.byte		KEYSH , mus_qz_628_key+0
	.byte		VOICE , 39
	.byte		VOL   , 37*mus_qz_628_mvl/mxv
	.byte		PAN   , c_v+56
	.byte		PRIO  , 44
	.byte	W09
mus_qz_628_6_000:
	.byte		N05   , Fn4 , v088
	.byte	W12
	.byte		        Fn4 , v052
	.byte	W06
	.byte		        Fn4 , v072
	.byte	W06
	.byte		        Fn4 , v088
	.byte	W12
	.byte		        Fn4 , v052
	.byte	W06
	.byte		        Fn4 , v072
	.byte	W06
	.byte		        Fn4 , v088
	.byte	W12
	.byte		        Fn4 , v052
	.byte	W06
	.byte		        Fn4 , v072
	.byte	W06
	.byte		        Fn4 , v088
	.byte	W12
	.byte		        Fn4 , v052
	.byte	W06
	.byte		        Fn4 , v072
	.byte	W06
	.byte	PEND
mus_qz_628_6_001:
	.byte		N05   , Fn4 , v088
	.byte	W06
	.byte		        Fn4 , v052
	.byte	W06
	.byte		        Fn4 , v072
	.byte	W06
	.byte		        Fn4 , v088
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Fn4 , v052
	.byte	W06
	.byte		        Fn4 , v072
	.byte	W06
	.byte		        Fn4 , v088
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Fn4 , v052
	.byte	W06
	.byte		        Fn4 , v072
	.byte	W06
	.byte		        Fn4 , v088
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Fn4 , v052
	.byte	W06
	.byte		        Fn4 , v072
	.byte	W06
	.byte		        Fn4 , v088
	.byte	W06
	.byte	PEND
mus_qz_628_6_B1:
	.byte	PATT
	 ptrvalue	mus_qz_628_6_000
	.byte	PATT
	 ptrvalue	mus_qz_628_6_001
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_628_6_000
	.byte	PATT
	 ptrvalue	mus_qz_628_6_001
	.byte	GOTO
	 ptrvalue	mus_qz_628_6_B1
	.byte	FINE

/********************** Track  7 **********************/

mus_qz_628_7:
	.byte		KEYSH , mus_qz_628_key+0
	.byte		VOICE , 39
	.byte		VOL   , 53*mus_qz_628_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 40
	.byte	W09
	.byte		N32   , En2 , v108 , gtp3
	.byte	W96
	.byte	W96
mus_qz_628_7_B1:
	.byte		N32   , En2 , v108 , gtp3
	.byte	W96
	.byte	W96
	.byte		        En2 , v108 , gtp3
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		        En2 , v108 , gtp3
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_628_7_B1
	.byte	FINE

/********************** Track  8 **********************/

mus_qz_628_8:
	.byte		KEYSH , mus_qz_628_key+0
	.byte		VOICE , 35
	.byte		VOL   , 17*mus_qz_628_mvl/mxv
	.byte		PAN   , c_v+56
	.byte		PRIO  , 38
	.byte		MOD   , 3
	.byte		LFOS  , 36
	.byte		LFODL , 10
	.byte	W09
mus_qz_628_8_000:
	.byte		N02   , An4 , v116
	.byte	W03
	.byte		        Bn4 , v108
	.byte	W03
	.byte		        Dn5 , v116
	.byte	W03
	.byte		N36   , En5 , v108 , gtp2
	.byte	W30
	.byte	W01
	.byte		VOL   , 17*mus_qz_628_mvl/mxv
	.byte	W02
	.byte		        13*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        10*mus_qz_628_mvl/mxv
	.byte	W02
	.byte		        6*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        4*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        19*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		N02   , Dn5 , v116
	.byte	W03
	.byte		        En5 , v108
	.byte	W03
	.byte		        An5 , v116
	.byte	W03
	.byte		N44   , Bn5 , v108 , gtp3
	.byte	W36
	.byte	W03
	.byte	PEND
mus_qz_628_8_001:
	.byte		VOL   , 15*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        11*mus_qz_628_mvl/mxv
	.byte	W02
	.byte		        8*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        5*mus_qz_628_mvl/mxv
	.byte	W02
	.byte		        3*mus_qz_628_mvl/mxv
	.byte	W02
	.byte		        20*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		N44   , Fn2 , v088 , gtp3
	.byte	W42
	.byte		VOL   , 17*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        12*mus_qz_628_mvl/mxv
	.byte	W02
	.byte		        8*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        3*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        20*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		N40   , En2 , v076 , gtp1
	.byte	W36
	.byte		VOL   , 17*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        13*mus_qz_628_mvl/mxv
	.byte	W02
	.byte	PEND
mus_qz_628_8_B1:
	.byte		VOL   , 8*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        4*mus_qz_628_mvl/mxv
	.byte	W02
	.byte		        20*mus_qz_628_mvl/mxv
	.byte		N02   , En5 , v116
	.byte	W03
	.byte		        Fn5 , v108
	.byte	W03
	.byte		N36   , En5 , v108 , gtp2
	.byte	W32
	.byte	W01
	.byte		VOL   , 14*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        10*mus_qz_628_mvl/mxv
	.byte	W02
	.byte		        5*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        2*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        19*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		N02   , Dn5 , v116
	.byte	W03
	.byte		        En5 , v108
	.byte	W03
	.byte		        An5 , v116
	.byte	W03
	.byte		N44   , Bn5 , v108 , gtp3
	.byte	W36
	.byte	W03
	.byte	W03
	.byte		VOL   , 13*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        10*mus_qz_628_mvl/mxv
	.byte	W02
	.byte		        6*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        3*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        19*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		N44   , Fn2 , v088 , gtp3
	.byte	W42
	.byte		VOL   , 12*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        9*mus_qz_628_mvl/mxv
	.byte	W02
	.byte		        6*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        3*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        19*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		N44   , En2 , v076 , gtp3
	.byte	W36
	.byte	W03
	.byte	W04
	.byte		VOL   , 14*mus_qz_628_mvl/mxv
	.byte	W02
	.byte		        11*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        8*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        19*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		N05   , Fn3 , v116
	.byte	W06
	.byte		        An3 , v104
	.byte	W06
	.byte		        Bn3 , v116
	.byte	W06
	.byte		        Dn4 , v104
	.byte	W06
	.byte		        En4 , v116
	.byte	W06
	.byte		        Dn4 , v104
	.byte	W06
	.byte		        En4 , v116
	.byte	W06
	.byte		        An4 , v104
	.byte	W06
	.byte		N02   , As4 , v116
	.byte	W03
	.byte		N08   , Bn4
	.byte	W09
	.byte		N03   , An4
	.byte	W04
	.byte		        Bn4
	.byte	W04
	.byte		        An4
	.byte	W04
	.byte		N11   , Fn4
	.byte	W12
	.byte		        Dn4
	.byte	W03
	.byte	W09
	.byte		N05   , En4
	.byte	W06
	.byte		        Dn4 , v104
	.byte	W06
	.byte		        An3 , v116
	.byte	W06
	.byte		        Bn3 , v104
	.byte	W06
	.byte		N11   , An4 , v116
	.byte	W12
	.byte		N03   , Fn4
	.byte	W04
	.byte		        An4
	.byte	W04
	.byte		        Fn4
	.byte	W04
	.byte		N02   , Ds4
	.byte	W03
	.byte		N08   , En4
	.byte	W09
	.byte		N11   , Dn4
	.byte	W12
	.byte		        Bn3
	.byte	W12
	.byte		        Dn4
	.byte	W03
	.byte	W09
	.byte		N44   , Dn2 , v088 , gtp3
	.byte	W40
	.byte		VOL   , 16*mus_qz_628_mvl/mxv
	.byte	W02
	.byte		        13*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        10*mus_qz_628_mvl/mxv
	.byte	W02
	.byte		        5*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        3*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        19*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		N44   , Fn2 , v076 , gtp3
	.byte	W36
	.byte	W03
	.byte	W03
	.byte		VOL   , 15*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        11*mus_qz_628_mvl/mxv
	.byte	W02
	.byte		        10*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        6*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        22*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		N80   , En2 , v080 , gtp1
	.byte	W72
	.byte		VOL   , 16*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        13*mus_qz_628_mvl/mxv
	.byte	W02
	.byte		        10*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        8*mus_qz_628_mvl/mxv
	.byte	W02
	.byte		        6*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        5*mus_qz_628_mvl/mxv
	.byte	W02
	.byte		        3*mus_qz_628_mvl/mxv
	.byte	W01
	.byte		        2*mus_qz_628_mvl/mxv
	.byte	W02
	.byte		        1*mus_qz_628_mvl/mxv
	.byte	W02
	.byte		        20*mus_qz_628_mvl/mxv
	.byte	W01
	.byte	PATT
	 ptrvalue	mus_qz_628_8_000
	.byte	PATT
	 ptrvalue	mus_qz_628_8_001
	.byte	GOTO
	 ptrvalue	mus_qz_628_8_B1
	.byte	W02
	.byte	FINE

/********************** Track  9 **********************/

mus_qz_628_9:
	.byte		KEYSH , mus_qz_628_key+0
	.byte		VOICE , 42
	.byte		VOL   , 14*mus_qz_628_mvl/mxv
	.byte		PAN   , c_v+58
	.byte		BENDR , 6
	.byte		PRIO  , 36
	.byte		MOD   , 3
	.byte		LFOS  , 36
	.byte		LFODL , 10
	.byte	W09
mus_qz_628_9_000:
	.byte	W06
	.byte		N05   , Bn3 , v100
	.byte		N05   , En4 , v116
	.byte	W12
	.byte		        En2 , v088
	.byte		N05   , An3 , v104
	.byte	W12
	.byte		        Bn3 , v116
	.byte		N05   , En4
	.byte	W12
	.byte		        En2 , v088
	.byte		N05   , An3 , v104
	.byte	W12
	.byte		        En4 , v100
	.byte		N05   , Bn4 , v116
	.byte	W12
	.byte		        Dn4 , v088
	.byte		N05   , An4 , v104
	.byte	W12
	.byte		        Bn3 , v100
	.byte		N05   , Fn4 , v116
	.byte	W12
	.byte		        Dn4 , v088
	.byte		N05   , An4 , v104
	.byte	W06
	.byte	PEND
	.byte	W96
mus_qz_628_9_B1:
	.byte	PATT
	 ptrvalue	mus_qz_628_9_000
	.byte	W96
	.byte	W12
	.byte		N32   , Dn2 , v120 , gtp3
	.byte	W36
	.byte		N04   , En2 , v116
	.byte	W06
	.byte		        Fn2 , v104
	.byte	W05
	.byte		BEND  , c_v-11
	.byte	W01
	.byte		N44   , An2 , v124 , gtp3
	.byte	W02
	.byte		BEND  , c_v+0
	.byte	W32
	.byte	W02
	.byte	W12
	.byte		N32   , Bn2 , v124 , gtp3
	.byte	W36
	.byte		N04   , An2 , v116
	.byte	W06
	.byte		        Bn2 , v100
	.byte	W05
	.byte		BEND  , c_v-11
	.byte	W01
	.byte		N32   , En3 , v124 , gtp3
	.byte	W02
	.byte		BEND  , c_v+0
	.byte	W32
	.byte	W02
	.byte		N04   , Dn3 , v116
	.byte	W06
	.byte		        En3 , v104
	.byte	W90
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_628_9_000
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_628_9_B1
	.byte	FINE

/******************************************************/
	.p2align	2

mus_qz_628:
	.byte	9	/* NumTrks */
	.byte	0	/* NumBlks */
	.byte	mus_qz_628_pri	/* Priority */
	.byte	mus_qz_628_rev	/* Reverb. */
space64 4
	ptrvalue	mus_qz_628_grp

	ptrvalue	mus_qz_628_1
	ptrvalue	mus_qz_628_2
	ptrvalue	mus_qz_628_3
	ptrvalue	mus_qz_628_4
	ptrvalue	mus_qz_628_5
	ptrvalue	mus_qz_628_6
	ptrvalue	mus_qz_628_7
	ptrvalue	mus_qz_628_8
	ptrvalue	mus_qz_628_9

	.end
