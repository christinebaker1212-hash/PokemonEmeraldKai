	.include "asm/macros/bit_width.inc"

	.include "MPlayDef.s"

/* Moemon Quetzal song 619, disassembled from the ROM by tools/quetzal_rip/songs.py.
   "org" comments give the ROM address of the bytes that follow. */

	.equ	mus_qz_619_grp, voicegroup_qz_songs
	.equ	mus_qz_619_pri, 0
	.equ	mus_qz_619_rev, reverb_set+0
	.equ	mus_qz_619_mvl, 127
	.equ	mus_qz_619_key, 0
	.equ	mus_qz_619_tbs, 1
	.equ	mus_qz_619_exg, 1
	.equ	mus_qz_619_cmp, 1

	.section .rodata
	.global	mus_qz_619
	.p2align	2
/* org 0x096764BC */

/********************** Track  1 **********************/

mus_qz_619_1:
	.byte		KEYSH , mus_qz_619_key+0
	.byte		TEMPO , 128*mus_qz_619_tbs/2
	.byte		VOICE , 23
	.byte		VOL   , 62*mus_qz_619_mvl/mxv
	.byte		PAN   , c_v-46
	.byte		VOL   , 62*mus_qz_619_mvl/mxv
	.byte		BENDR , 6
	.byte		PRIO  , 62
	.byte	W12
	.byte		N05   , Bn4 , v112
	.byte	W06
	.byte		        Cn5 , v096
	.byte	W06
	.byte		        Bn4 , v104
	.byte	W06
	.byte		        An4 , v092
	.byte	W06
	.byte		        Gn4 , v104
	.byte	W06
	.byte		        Fs4 , v092
	.byte	W06
	.byte		        An4 , v100
	.byte	W06
	.byte		        Gn4 , v092
	.byte	W06
	.byte		        En4 , v104
	.byte	W06
	.byte		        Fs4 , v092
	.byte	W06
	.byte		        Dn4 , v096
	.byte	W06
	.byte		        En4 , v088
	.byte	W06
	.byte		BEND  , c_v+1
	.byte		N23   , Cn4 , v096
	.byte	W24
	.byte		BEND  , c_v+0
	.byte		N02   , Cn4 , v108
	.byte	W06
	.byte		N05   , Cn4 , v096
	.byte	W06
	.byte		        Bn3 , v088
	.byte	W06
	.byte		N02   , Cn4 , v096
	.byte	W06
	.byte		        Cn4 , v108
	.byte	W06
	.byte		N05   , Cn4 , v096
	.byte	W06
	.byte		        Bn3 , v088
	.byte	W06
	.byte		N02   , Cn4 , v092
	.byte	W06
	.byte		N05   , Gn4 , v112
	.byte	W06
	.byte		N02   , Cn4 , v096
	.byte	W06
	.byte		N05   , Bn3 , v088
	.byte	W06
	.byte		N02   , Cn4 , v092
	.byte	W06
	.byte		        Cn4 , v108
	.byte	W06
	.byte		N05   , Cn4 , v096
	.byte	W06
	.byte		        Bn3 , v088
	.byte	W06
	.byte		N02   , Cn4 , v096
	.byte	W06
	.byte		N05   , Bn3 , v104
	.byte	W06
	.byte		        Cn4 , v096
	.byte	W06
	.byte		N68   , Dn4 , v112 , gtp3
	.byte	W12
	.byte		VOL   , 57*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        51*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        46*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        40*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        43*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        47*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        50*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        54*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        62*mus_qz_619_mvl/mxv
	.byte	W12
	.byte		N05   , An3 , v096
	.byte	W06
	.byte		        Bn3 , v088
	.byte	W06
	.byte		N02   , Cn4 , v112
	.byte	W06
	.byte		N05   , Cn4 , v096
	.byte	W06
	.byte		        Bn3 , v088
	.byte	W06
	.byte		N02   , Cn4 , v092
	.byte	W06
	.byte		N05   , An3 , v112
	.byte	W06
	.byte		        Bn3 , v096
	.byte	W06
	.byte		N02   , Cn4 , v108
	.byte	W06
	.byte		        Cn4 , v096
	.byte	W06
	.byte		        An3 , v112
	.byte	W48
mus_qz_619_1_B1:
	.byte		N05   , Bn3 , v108
	.byte	W36
	.byte		N02   , Bn3 , v088
	.byte	W06
	.byte		        Bn3 , v076
	.byte	W06
	.byte		        Bn3 , v096
	.byte	W12
	.byte		        Bn3 , v088
	.byte	W36
	.byte		N05   , An3 , v108
	.byte	W36
	.byte		N02   , An3 , v100
	.byte	W06
	.byte		        An3 , v088
	.byte	W06
	.byte		N23   , An3 , v108
	.byte	W24
	.byte		N11   , Dn4
	.byte	W12
	.byte		        Cn4 , v096
	.byte	W12
	.byte		N05   , Bn3
	.byte	W06
	.byte		        Cn4 , v088
	.byte	W06
	.byte		        Bn3 , v104
	.byte	W06
	.byte		        Cn4 , v096
	.byte	W06
	.byte		N44   , Bn3 , v104
	.byte	W30
	.byte		VOL   , 58*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        54*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        49*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        46*mus_qz_619_mvl/mxv
	.byte	W12
	.byte		        62*mus_qz_619_mvl/mxv
	.byte	W12
	.byte		N03   , Gn3 , v108
	.byte	W36
	.byte		N02   , Gn3 , v100
	.byte	W06
	.byte		        Gn3 , v092
	.byte	W06
	.byte		        Gn3 , v108
	.byte	W12
	.byte		N08   , Gn3 , v096
	.byte	W12
	.byte		N03   , Gn3 , v108
	.byte	W08
	.byte		        Gn3 , v092
	.byte	W08
	.byte		N07   , Gs3 , v100
	.byte	W08
	.byte		N05   , An3 , v104
	.byte	W36
	.byte		N02   , An3 , v092
	.byte	W06
	.byte		        An3 , v080
	.byte	W06
	.byte		        An3 , v096
	.byte	W12
	.byte		        An3 , v088
	.byte	W36
	.byte		N05   , An3 , v104
	.byte	W36
	.byte		N02   , An3 , v092
	.byte	W06
	.byte		        An3 , v080
	.byte	W06
	.byte		        An3 , v096
	.byte	W06
	.byte		        An3 , v088
	.byte	W06
	.byte		N03   , An3 , v096
	.byte	W24
	.byte		N11   , As3 , v092
	.byte	W12
	.byte		N05   , Bn3 , v104
	.byte	W36
	.byte		N02   , Bn3 , v092
	.byte	W06
	.byte		        Bn3 , v084
	.byte	W06
	.byte		        Bn3 , v104
	.byte	W12
	.byte		        Bn3 , v092
	.byte	W36
	.byte		N05   , Bn3 , v108
	.byte	W36
	.byte		N02   , Bn3 , v092
	.byte	W06
	.byte		        Bn3 , v084
	.byte	W06
	.byte		        Bn3 , v100
	.byte	W12
	.byte		        Bn3 , v092
	.byte	W12
	.byte		N03   , Bn3 , v104
	.byte	W08
	.byte		        Bn3 , v092
	.byte	W08
	.byte		N06   , Bn3 , v100
	.byte	W08
	.byte		N11   , Gs3 , v112
	.byte	W12
	.byte		N05   , An3 , v100
	.byte	W24
	.byte		N11   , Bn3 , v112
	.byte	W24
	.byte		        Cn4 , v120
	.byte	W24
	.byte		        En4 , v104
	.byte	W12
	.byte		N44   , Fn4 , v120 , gtp3
	.byte	W48
	.byte		        An4 , v108 , gtp3
	.byte	W36
	.byte		VOL   , 58*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        50*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        62*mus_qz_619_mvl/mxv
	.byte		N03   , Bn3 , v096
	.byte		N03   , Gn4 , v120
	.byte	W12
	.byte		TIE   , Gn3 , v084
	.byte		TIE   , Dn4 , v108
	.byte	W12
	.byte		VOL   , 58*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        52*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        47*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        43*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        38*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        32*mus_qz_619_mvl/mxv
	.byte	W18
	.byte		        38*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        41*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        44*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        47*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        49*mus_qz_619_mvl/mxv
	.byte	W12
	.byte		        54*mus_qz_619_mvl/mxv
	.byte	W12
	.byte		        57*mus_qz_619_mvl/mxv
	.byte	W12
	.byte		        60*mus_qz_619_mvl/mxv
	.byte	W12
	.byte		        62*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        58*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        52*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        47*mus_qz_619_mvl/mxv
	.byte	W05
	.byte		EOT   , Gn3
	.byte		        Dn4
	.byte	W01
	.byte		VOL   , 40*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        34*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        27*mus_qz_619_mvl/mxv
	.byte	W12
	.byte		        61*mus_qz_619_mvl/mxv
	.byte		N23   , Cn3 , v092
	.byte	W24
	.byte		        An2 , v080
	.byte	W24
	.byte		        Fn3 , v096
	.byte	W24
	.byte		        Cn3 , v084
	.byte	W24
	.byte		N02   , An2 , v108
	.byte	W06
	.byte		N05   , An2 , v096
	.byte	W06
	.byte		        Gs2 , v108
	.byte	W06
	.byte		N02   , An2 , v092
	.byte	W06
	.byte		        Cn3 , v108
	.byte	W06
	.byte		N05   , Cn3 , v096
	.byte	W06
	.byte		        Bn2 , v108
	.byte	W06
	.byte		N02   , Cn3 , v100
	.byte	W06
	.byte		        Fn3 , v108
	.byte	W06
	.byte		N05   , Fn3 , v100
	.byte	W06
	.byte		        En3 , v108
	.byte	W06
	.byte		N02   , Fn3 , v100
	.byte	W06
	.byte		        An3 , v108
	.byte	W06
	.byte		N05   , An3 , v100
	.byte	W06
	.byte		N02   , Gs3 , v108
	.byte	W06
	.byte		N05   , An3 , v100
	.byte	W06
	.byte		N02   , Dn4 , v108
	.byte	W06
	.byte		        Dn4 , v104
	.byte	W06
	.byte		N05   , Cn4 , v112
	.byte	W06
	.byte		        Dn4 , v104
	.byte	W06
	.byte		        Cn4 , v108
	.byte	W06
	.byte		N02   , Dn4 , v100
	.byte	W06
	.byte		N05   , Cn4 , v112
	.byte	W06
	.byte		N02   , Dn4 , v104
	.byte	W06
	.byte		        Cn4 , v108
	.byte	W06
	.byte		        Cn4 , v096
	.byte	W06
	.byte		N05   , Bn3 , v108
	.byte	W06
	.byte		N02   , Cn4 , v096
	.byte	W06
	.byte		N05   , Dn4 , v108
	.byte	W06
	.byte		        En4 , v096
	.byte	W06
	.byte		        An3 , v104
	.byte	W06
	.byte		        Gs3 , v092
	.byte	W06
	.byte		N44   , Gn3 , v108 , gtp3
	.byte	W03
	.byte		VOL   , 25*mus_qz_619_mvl/mxv
	.byte	W03
	.byte		        28*mus_qz_619_mvl/mxv
	.byte	W03
	.byte		        32*mus_qz_619_mvl/mxv
	.byte	W03
	.byte		        34*mus_qz_619_mvl/mxv
	.byte	W03
	.byte		        38*mus_qz_619_mvl/mxv
	.byte	W03
	.byte		        40*mus_qz_619_mvl/mxv
	.byte	W03
	.byte		        43*mus_qz_619_mvl/mxv
	.byte	W03
	.byte		        47*mus_qz_619_mvl/mxv
	.byte	W03
	.byte		        50*mus_qz_619_mvl/mxv
	.byte	W03
	.byte		        54*mus_qz_619_mvl/mxv
	.byte	W03
	.byte		        58*mus_qz_619_mvl/mxv
	.byte	W03
	.byte		        62*mus_qz_619_mvl/mxv
	.byte	W12
	.byte		N02   , Dn3
	.byte	W12
	.byte		N08   , Dn3 , v096
	.byte	W12
	.byte		N03   , Fn3 , v108
	.byte	W08
	.byte		        Fn3 , v092
	.byte	W08
	.byte		        Fs3 , v108
	.byte	W08
	.byte	GOTO
	 ptrvalue	mus_qz_619_1_B1
	.byte	FINE

/********************** Track  2 **********************/

mus_qz_619_2:
	.byte		KEYSH , mus_qz_619_key+0
	.byte		VOICE , 29
	.byte		VOL   , 73*mus_qz_619_mvl/mxv
	.byte		PAN   , c_v-9
	.byte		VOL   , 73*mus_qz_619_mvl/mxv
	.byte		PRIO  , 64
	.byte	W12
	.byte		N05   , Gn3 , v088
	.byte		N05   , Bn3 , v100
	.byte	W36
	.byte		N02   , Gn3 , v076
	.byte		N02   , Bn3 , v088
	.byte	W06
	.byte		        Gn3 , v064
	.byte		N02   , Bn3 , v072
	.byte	W06
	.byte		        Gn3 , v084
	.byte		N02   , Bn3 , v096
	.byte	W12
	.byte		        Gn3 , v080
	.byte		N02   , Bn3 , v088
	.byte	W36
	.byte		N05   , Gn3
	.byte		N05   , Cn4 , v100
	.byte	W36
	.byte		N02   , Gn3 , v080
	.byte		N02   , Cn4 , v092
	.byte	W06
	.byte		        Gn3 , v068
	.byte		N02   , Cn4 , v080
	.byte	W06
	.byte		        Gn3 , v084
	.byte		N02   , Cn4 , v092
	.byte	W06
	.byte		        Gn3 , v072
	.byte		N02   , Cn4 , v080
	.byte	W06
	.byte		        Gn3
	.byte		N02   , Cn4 , v088
	.byte	W36
	.byte		N05   , Gn3
	.byte		N05   , Bn3 , v100
	.byte	W36
	.byte		N02   , Gn3 , v072
	.byte		N02   , Bn3 , v080
	.byte	W06
	.byte		        Gn3 , v064
	.byte		N02   , Bn3 , v076
	.byte	W06
	.byte		        Gn3 , v080
	.byte		N02   , Bn3 , v092
	.byte	W12
	.byte		        Gn3 , v072
	.byte		N02   , Bn3 , v084
	.byte	W36
	.byte		N05   , Dn3
	.byte		N05   , An3 , v092
	.byte	W36
	.byte		N02   , Dn3 , v072
	.byte		N02   , An3 , v084
	.byte	W06
	.byte		        Dn3 , v064
	.byte		N02   , An3 , v076
	.byte	W06
	.byte		        Dn3 , v084
	.byte		N02   , An3 , v092
	.byte	W06
	.byte		        Dn3 , v072
	.byte		N02   , An3 , v080
	.byte	W06
	.byte		N11   , Dn3 , v064
	.byte		N32   , An3 , v072 , gtp3
	.byte	W12
	.byte		N05   , Dn3 , v116
	.byte	W06
	.byte		        En3 , v108
	.byte	W06
	.byte		        Fn3 , v116
	.byte	W06
	.byte		        Fs3 , v112
	.byte	W06
mus_qz_619_2_B1:
	.byte		N52   , Gn3 , v124 , gtp1
	.byte	W36
	.byte		VOL   , 67*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        63*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        58*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        51*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        73*mus_qz_619_mvl/mxv
	.byte		N03   , Dn4 , v120
	.byte	W12
	.byte		        Bn3 , v108
	.byte	W12
	.byte		        Gn3 , v112
	.byte	W12
	.byte		N56   , Fn4 , v120 , gtp2
	.byte	W36
	.byte		VOL   , 69*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        64*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        59*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        53*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        73*mus_qz_619_mvl/mxv
	.byte		N11   , En4 , v112
	.byte	W12
	.byte		        Dn4 , v120
	.byte	W12
	.byte		        Cn4 , v112
	.byte	W12
	.byte		N05   , Bn3 , v120
	.byte	W06
	.byte		        Cn4 , v108
	.byte	W06
	.byte		        Bn3 , v116
	.byte	W06
	.byte		        Cn4 , v108
	.byte	W06
	.byte		N52   , Bn3 , v124 , gtp1
	.byte	W36
	.byte		VOL   , 66*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        61*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        55*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        51*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        47*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        40*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        73*mus_qz_619_mvl/mxv
	.byte		N03   , Dn3 , v104
	.byte		N03   , Gn3 , v116
	.byte	W36
	.byte		N02   , Dn3 , v096
	.byte		N02   , Gn3 , v108
	.byte	W06
	.byte		        Dn3 , v084
	.byte		N02   , Gn3 , v096
	.byte	W06
	.byte		        Dn3 , v092
	.byte		N02   , Gn3 , v104
	.byte	W12
	.byte		N08   , Dn3
	.byte		N08   , Gn3 , v116
	.byte	W12
	.byte		N03   , Dn3 , v092
	.byte		N03   , Gn3 , v116
	.byte	W08
	.byte		        Dn3 , v084
	.byte		N03   , Gn3 , v108
	.byte	W08
	.byte		        Dn3 , v092
	.byte		N03   , Gs3 , v116
	.byte	W08
	.byte		N05   , Cn3 , v080
	.byte		N05   , An3 , v096
	.byte	W36
	.byte		N02   , Cn3 , v080
	.byte		N02   , An3 , v096
	.byte	W06
	.byte		        Cn3 , v076
	.byte		N02   , An3 , v088
	.byte	W06
	.byte		        Cn3 , v080
	.byte		N02   , An3 , v096
	.byte	W12
	.byte		        Cn3 , v080
	.byte		N02   , An3 , v096
	.byte	W36
	.byte		N05   , Cn3 , v080
	.byte		N05   , An3 , v096
	.byte	W36
	.byte		N02   , Cn3 , v080
	.byte		N02   , An3 , v096
	.byte	W06
	.byte		        Cn3 , v076
	.byte		N02   , An3 , v088
	.byte	W06
	.byte		        Cn3 , v080
	.byte		N02   , An3 , v096
	.byte	W06
	.byte		        Cn3 , v080
	.byte		N02   , An3 , v096
	.byte	W06
	.byte		        Cn3 , v088
	.byte		N02   , An3 , v100
	.byte	W36
	.byte		N05   , Dn3 , v080
	.byte		N05   , Bn3 , v096
	.byte	W36
	.byte		N02   , Dn3 , v080
	.byte		N02   , Bn3 , v096
	.byte	W06
	.byte		        Dn3 , v076
	.byte		N02   , Bn3 , v088
	.byte	W06
	.byte		        Dn3 , v080
	.byte		N02   , Bn3 , v096
	.byte	W12
	.byte		        Dn3 , v080
	.byte		N02   , Bn3 , v096
	.byte	W36
	.byte		N05   , Dn3 , v080
	.byte		N05   , Bn3 , v096
	.byte	W36
	.byte		N02   , Dn3 , v080
	.byte		N02   , Bn3 , v096
	.byte	W06
	.byte		        Dn3 , v076
	.byte		N02   , Bn3 , v088
	.byte	W06
	.byte		        Dn3 , v080
	.byte		N02   , Bn3 , v096
	.byte	W12
	.byte		        Dn3 , v080
	.byte		N02   , Bn3 , v096
	.byte	W12
	.byte		N03   , Dn3 , v080
	.byte		N03   , Bn3 , v096
	.byte	W08
	.byte		        Dn3 , v072
	.byte		N03   , Bn3 , v088
	.byte	W08
	.byte		N06   , Dn3 , v080
	.byte		N06   , Bn3 , v096
	.byte	W08
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		N23   , Fn3
	.byte		N23   , An3 , v104
	.byte	W24
	.byte		        Gn3 , v092
	.byte		N23   , Bn3 , v100
	.byte	W24
	.byte		        An3
	.byte		N23   , Cn4 , v108
	.byte	W24
	.byte		        Cn4 , v092
	.byte		N23   , En4 , v100
	.byte	W24
	.byte		N44   , Dn4 , v108 , gtp2
	.byte		N44   , Fn4 , v112 , gtp2
	.byte	W48
	.byte		        Fn4 , v100 , gtp2
	.byte		N44   , Cn5 , v108 , gtp2
	.byte	W48
	.byte		N03   , Dn4 , v100
	.byte		N03   , Bn4 , v108
	.byte	W12
	.byte		N80   , Dn4 , v108 , gtp3
	.byte		N80   , Gn4 , v112 , gtp3
	.byte	W66
	.byte		VOL   , 67*mus_qz_619_mvl/mxv
	.byte	W03
	.byte		        64*mus_qz_619_mvl/mxv
	.byte	W03
	.byte		        60*mus_qz_619_mvl/mxv
	.byte	W03
	.byte		        57*mus_qz_619_mvl/mxv
	.byte	W03
	.byte		        52*mus_qz_619_mvl/mxv
	.byte	W03
	.byte		        46*mus_qz_619_mvl/mxv
	.byte	W03
	.byte		        73*mus_qz_619_mvl/mxv
	.byte		N03   , Dn4 , v100
	.byte		N03   , Bn4 , v108
	.byte	W12
	.byte		N32   , Dn4 , v108 , gtp3
	.byte		N32   , Gn4 , v116 , gtp3
	.byte	W36
	.byte		N03   , Dn4 , v100
	.byte		N03   , Gn4 , v108
	.byte	W12
	.byte		        Dn4 , v096
	.byte		N03   , Gn4 , v104
	.byte	W12
	.byte		        Bn3 , v100
	.byte		N03   , Fn4 , v108
	.byte	W08
	.byte		        Bn3 , v092
	.byte		N03   , Fn4 , v100
	.byte	W08
	.byte		        Bn3 , v112
	.byte		N03   , Fs4 , v127
	.byte	W08
	.byte	GOTO
	 ptrvalue	mus_qz_619_2_B1
	.byte	FINE

/********************** Track  3 **********************/

mus_qz_619_3:
	.byte		KEYSH , mus_qz_619_key+0
	.byte		VOICE , 23
	.byte		VOL   , 64*mus_qz_619_mvl/mxv
	.byte		PAN   , c_v-36
	.byte		PRIO  , 60
	.byte	W12
	.byte		N05   , Dn4 , v088
	.byte	W06
	.byte		        En4 , v072
	.byte	W06
	.byte		        Dn4 , v080
	.byte	W06
	.byte		        Cn4 , v076
	.byte	W06
	.byte		        Bn3 , v084
	.byte	W06
	.byte		        An3 , v072
	.byte	W06
	.byte		        Cn4 , v080
	.byte	W06
	.byte		        Bn3 , v072
	.byte	W06
	.byte		        Gn3 , v088
	.byte	W06
	.byte		        An3 , v076
	.byte	W06
	.byte		        Fs3 , v080
	.byte	W06
	.byte		        Gn3 , v076
	.byte	W06
	.byte		N23   , Gn3 , v088
	.byte	W24
	.byte		N05   , Fn3
	.byte	W06
	.byte		        Fs3 , v076
	.byte	W06
	.byte		        Gn3 , v080
	.byte	W12
	.byte		        Gn3 , v088
	.byte	W12
	.byte		        Fn3 , v072
	.byte	W06
	.byte		        Fs3 , v080
	.byte	W06
	.byte		        Gn3 , v088
	.byte	W12
	.byte		        Gn3 , v076
	.byte	W12
	.byte		        Gn3 , v080
	.byte	W12
	.byte		        Gn3 , v072
	.byte	W12
	.byte		        Gn3 , v080
	.byte	W06
	.byte		        An3 , v072
	.byte	W06
	.byte		        Bn3 , v088
	.byte	W12
	.byte		        Bn3 , v080
	.byte	W12
	.byte	N05
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        Dn4 , v084
	.byte	W12
	.byte		        Dn4 , v076
	.byte	W12
	.byte		        Gn4 , v084
	.byte	W12
	.byte		        En4 , v076
	.byte	W06
	.byte		        Fs4 , v084
	.byte	W06
	.byte		        Gn4 , v072
	.byte	W12
	.byte		        Gn4 , v060
	.byte	W12
	.byte		        En4 , v072
	.byte	W06
	.byte		        Fs4 , v060
	.byte	W06
	.byte		        Gn4 , v068
	.byte	W12
	.byte		N03   , Gn4 , v080
	.byte	W12
	.byte		N32   , Cn4 , v096 , gtp3
	.byte	W36
mus_qz_619_3_B1:
	.byte		N05   , Dn3 , v096
	.byte	W36
	.byte		N02   , Dn3 , v076
	.byte	W06
	.byte		        Dn3 , v068
	.byte	W06
	.byte		        Dn3 , v084
	.byte	W12
	.byte		        Dn3 , v076
	.byte	W36
	.byte		N05   , Cn3 , v092
	.byte	W36
	.byte		N02   , Cn3 , v088
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W06
	.byte		N23   , Cn3 , v100
	.byte	W24
	.byte		        Dn3 , v088
	.byte	W24
	.byte		N05   , Gn3 , v096
	.byte	W06
	.byte		        An3 , v088
	.byte	W06
	.byte		        Gn3 , v104
	.byte	W06
	.byte		        An3 , v096
	.byte	W06
	.byte		N44   , Gn3 , v104
	.byte	W72
	.byte		N03   , Dn3 , v096
	.byte	W36
	.byte		N02   , Dn3 , v092
	.byte	W06
	.byte		        Dn3 , v084
	.byte	W06
	.byte		        Dn3 , v096
	.byte	W12
	.byte		N08   , Dn3 , v088
	.byte	W12
	.byte		N03   , Dn3 , v108
	.byte	W08
	.byte		        Dn3 , v092
	.byte	W08
	.byte		N07   , Ds3 , v100
	.byte	W08
	.byte		N05   , En3 , v104
	.byte	W36
	.byte		N02   , En3 , v092
	.byte	W06
	.byte		        En3 , v080
	.byte	W06
	.byte		        En3 , v096
	.byte	W12
	.byte		        En3 , v088
	.byte	W36
	.byte		N05   , En3 , v104
	.byte	W36
	.byte		N02   , En3 , v092
	.byte	W06
	.byte		        En3 , v080
	.byte	W06
	.byte		        En3 , v096
	.byte	W06
	.byte		        En3 , v088
	.byte	W06
	.byte		N03   , En3 , v096
	.byte	W24
	.byte		N11   , Fn3 , v092
	.byte	W12
	.byte		N05   , Gn3 , v104
	.byte	W36
	.byte		N02   , Gn3 , v092
	.byte	W06
	.byte		        Gn3 , v084
	.byte	W06
	.byte		        Gn3 , v104
	.byte	W12
	.byte		        Gn3 , v092
	.byte	W36
	.byte		N05   , Dn3 , v108
	.byte	W36
	.byte		N02   , Dn3 , v092
	.byte	W06
	.byte		        Dn3 , v084
	.byte	W06
	.byte		        Dn3 , v100
	.byte	W12
	.byte		        Dn3 , v092
	.byte	W12
	.byte		N03   , Dn3 , v104
	.byte	W08
	.byte		        Dn3 , v092
	.byte	W16
	.byte		N11   , Cn3 , v088
	.byte	W12
	.byte		N05   , Cn3 , v076
	.byte	W24
	.byte		N11   , Fn3 , v084
	.byte	W24
	.byte		        Fn3 , v092
	.byte	W24
	.byte		        An3 , v084
	.byte	W12
	.byte		N44   , An3 , v092 , gtp3
	.byte	W48
	.byte		        Cn4 , v080 , gtp3
	.byte	W48
	.byte		N04   , Gn4 , v068
	.byte		N04   , Bn4 , v076
	.byte	W06
	.byte		        An4 , v056
	.byte		N04   , Cn5 , v064
	.byte	W06
	.byte		        Gn4 , v060
	.byte		N04   , Bn4 , v072
	.byte	W06
	.byte		N02   , Dn4 , v060
	.byte		N02   , Gn4 , v064
	.byte	W06
	.byte		N04   , Gn4 , v068
	.byte		N04   , Bn4 , v076
	.byte	W06
	.byte		        An4 , v052
	.byte		N04   , Cn5 , v060
	.byte	W06
	.byte		        Gn4
	.byte		N04   , Bn4 , v068
	.byte	W06
	.byte		N02   , Dn4 , v060
	.byte		N02   , Gn4 , v064
	.byte	W06
	.byte		N04   , Gn4 , v072
	.byte		N04   , Bn4 , v080
	.byte	W06
	.byte		        An4 , v064
	.byte		N04   , Cn5 , v072
	.byte	W06
	.byte		        Gn4 , v068
	.byte		N04   , Bn4 , v076
	.byte	W06
	.byte		N02   , Dn4 , v064
	.byte		N02   , Gn4
	.byte	W06
	.byte		N04   , Gn4 , v076
	.byte		N04   , Bn4 , v084
	.byte	W06
	.byte		        An4 , v064
	.byte		N04   , Cn5 , v072
	.byte	W06
	.byte		        Gn4 , v068
	.byte		N04   , Bn4 , v076
	.byte	W06
	.byte		N02   , Dn4 , v072
	.byte		N02   , Gn4 , v064
	.byte	W06
	.byte		N05   , Cn5 , v080
	.byte		N05   , En5 , v092
	.byte	W06
	.byte		        Bn4 , v068
	.byte		N05   , Dn5 , v076
	.byte	W06
	.byte		        An4 , v072
	.byte		N05   , Cn5 , v080
	.byte	W06
	.byte		        Gn4 , v068
	.byte		N05   , Bn4 , v076
	.byte	W06
	.byte		        Bn3 , v080
	.byte		N05   , Gn4 , v088
	.byte	W06
	.byte		        Cn4 , v072
	.byte		N05   , An4 , v080
	.byte	W06
	.byte		        Gn3 , v076
	.byte		N05   , En4 , v084
	.byte	W06
	.byte		        Fs3 , v068
	.byte		N05   , Dn4 , v076
	.byte	W06
	.byte		        En3 , v084
	.byte		N05   , Cn4 , v092
	.byte	W06
	.byte		        Dn3 , v068
	.byte		N05   , Bn3 , v080
	.byte	W06
	.byte		        Cn3 , v076
	.byte		N05   , Gn3 , v084
	.byte	W06
	.byte		        Dn3 , v068
	.byte		N05   , An3 , v076
	.byte	W06
	.byte		        En3 , v080
	.byte		N05   , Bn3 , v088
	.byte	W06
	.byte		        Fs3 , v068
	.byte		N05   , Cn4 , v076
	.byte	W06
	.byte		        Bn2
	.byte		N05   , Dn3 , v084
	.byte	W06
	.byte		        Dn3 , v068
	.byte		N05   , Gn3 , v076
	.byte	W06
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_619_3_B1
	.byte	FINE

/********************** Track  4 **********************/

mus_qz_619_4:
	.byte		KEYSH , mus_qz_619_key+0
	.byte		VOICE , 25
	.byte		VOL   , 84*mus_qz_619_mvl/mxv
	.byte		PAN   , c_v+56
	.byte		VOL   , 84*mus_qz_619_mvl/mxv
	.byte		PRIO  , 54
	.byte	W12
	.byte		N05   , Gn0 , v124
	.byte	W96
	.byte	N05
	.byte	W84
	.byte		N04   , Gn0 , v084
	.byte	W06
	.byte		        Gn0 , v076
	.byte	W06
	.byte		N05   , Gn0 , v124
	.byte	W96
	.byte	N05
	.byte	W36
	.byte		        Gn0 , v100
	.byte	W24
	.byte		N32   , Dn1 , v088 , gtp3
	.byte	W36
mus_qz_619_4_B1:
	.byte		N05   , Gn0 , v127
	.byte	W96
	.byte		        Gn0 , v124
	.byte	W96
	.byte	N05
	.byte	W96
	.byte	N05
	.byte	W68
	.byte	W03
	.byte		N03   , Gn0 , v092
	.byte	W08
	.byte		        Gn0 , v076
	.byte	W08
	.byte		        Gs0 , v084
	.byte	W09
	.byte		N05   , Fn0 , v124
	.byte	W96
	.byte	N05
	.byte	W96
	.byte		        Gn0
	.byte	W96
	.byte	N05
	.byte	W72
	.byte		N03   , Gn0 , v092
	.byte	W08
	.byte		        Gn0 , v076
	.byte	W08
	.byte		        Gn0 , v084
	.byte	W08
	.byte		N44   , Fn1 , v080 , gtp3
	.byte	W48
	.byte		        En1 , v068 , gtp3
	.byte	W48
	.byte		N23   , Dn1 , v076
	.byte	W24
	.byte		        Cn1 , v064
	.byte	W24
	.byte		        Fn1 , v080
	.byte	W24
	.byte		        Dn1 , v072
	.byte	W24
	.byte		N68   , Gn0 , v088 , gtp3
	.byte	W48
	.byte		VOL   , 77*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        73*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        68*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        62*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        84*mus_qz_619_mvl/mxv
	.byte		N23   , Dn1 , v080
	.byte	W24
	.byte		        Fn1 , v084
	.byte	W24
	.byte		        En1 , v072
	.byte	W24
	.byte		        Dn1 , v064
	.byte	W24
	.byte		        Gn1 , v068
	.byte	W24
	.byte		        Fn1 , v080
	.byte	W24
	.byte		        En1 , v068
	.byte	W24
	.byte		        Dn1 , v076
	.byte	W24
	.byte		        Cn1 , v068
	.byte	W24
	.byte		        Dn1 , v080
	.byte	W24
	.byte		        Cn1 , v072
	.byte	W24
	.byte		        An1 , v080
	.byte	W24
	.byte		        Dn1 , v072
	.byte	W24
	.byte		N76   , Gn0 , v096 , gtp1
	.byte	W54
	.byte		VOL   , 77*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        73*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        68*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        62*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        55*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        84*mus_qz_619_mvl/mxv
	.byte		N04   , Gn0 , v088
	.byte	W06
	.byte		        Gn0 , v076
	.byte	W06
	.byte		N44   , Gn0 , v096 , gtp3
	.byte	W24
	.byte		VOL   , 79*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        73*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        67*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        63*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        84*mus_qz_619_mvl/mxv
	.byte		N05   , Gn0 , v084
	.byte	W12
	.byte		        Gn0 , v072
	.byte	W12
	.byte		N03   , Fn0 , v088
	.byte	W08
	.byte		        Fn0 , v072
	.byte	W08
	.byte		        Fs0 , v096
	.byte	W08
	.byte	GOTO
	 ptrvalue	mus_qz_619_4_B1
	.byte	FINE

/********************** Track  5 **********************/

mus_qz_619_5:
	.byte		KEYSH , mus_qz_619_key+0
	.byte		VOICE , 30
	.byte		VOL   , 84*mus_qz_619_mvl/mxv
	.byte		PAN   , c_v+16
	.byte		PRIO  , 52
	.byte		N05   , Dn1 , v104
	.byte	W12
mus_qz_619_5_000:
	.byte		N09   , Gn1 , v104
	.byte	W60
	.byte		        Dn1 , v092
	.byte	W24
	.byte		N02   , Dn1 , v076
	.byte	W06
	.byte		        Dn1 , v068
	.byte	W06
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_619_5_000
	.byte	PATT
	 ptrvalue	mus_qz_619_5_000
	.byte		N09   , Gn1 , v104
	.byte	W60
	.byte		N23   , Dn2
	.byte	W24
	.byte		N02   , Dn1 , v076
	.byte	W06
	.byte		        Dn1 , v068
	.byte	W06
mus_qz_619_5_B1:
	.byte		N09   , Gn1 , v104
	.byte	W60
	.byte		N11   , Dn1
	.byte	W24
	.byte		N02   , Dn1 , v076
	.byte	W06
	.byte		        Dn1 , v068
	.byte	W06
	.byte		N09   , Gn1 , v104
	.byte	W60
	.byte		N11   , Dn1 , v088
	.byte	W12
	.byte		        Dn2 , v104
	.byte	W12
	.byte		        An1 , v096
	.byte	W12
	.byte		N09   , Gn1 , v108
	.byte	W60
	.byte		        Dn1 , v096
	.byte	W24
	.byte	N02
	.byte	W06
	.byte		        Dn1 , v084
	.byte	W06
	.byte		N09   , Gn1 , v108
	.byte	W48
	.byte		N05   , Gn1 , v100
	.byte	W12
	.byte		N11   , Dn2 , v092
	.byte	W12
	.byte		N03   , Gn1 , v108
	.byte	W08
	.byte		        Gn1 , v096
	.byte	W08
	.byte		        Fs1 , v108
	.byte	W08
	.byte		N09   , Fn1 , v104
	.byte	W60
	.byte		        Fn1 , v096
	.byte	W24
	.byte		N02   , Fn1 , v100
	.byte	W06
	.byte		        Fn1 , v092
	.byte	W06
	.byte		N09   , Fn1 , v108
	.byte	W60
	.byte		N11   , Fn1 , v100
	.byte	W24
	.byte		N02   , Fs1
	.byte	W06
	.byte		        Fs1 , v092
	.byte	W06
	.byte		N09   , Gn1 , v104
	.byte	W60
	.byte		N11   , Dn1
	.byte	W24
	.byte		N02   , Dn1 , v096
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte		N09   , Gn1 , v108
	.byte	W60
	.byte	N11
	.byte	W12
	.byte		N03   , Gn1 , v100
	.byte	W08
	.byte		        Gn1 , v088
	.byte	W08
	.byte		        Fn1 , v092
	.byte	W08
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		N21   , Fn2 , v112
	.byte	W24
	.byte		        En2 , v096
	.byte	W24
	.byte		        Dn2 , v108
	.byte	W24
	.byte		        Cn2 , v100
	.byte	W24
	.byte		N44   , Dn2 , v112 , gtp3
	.byte	W48
	.byte		N21   , An2 , v116
	.byte	W24
	.byte		        Dn2 , v108
	.byte	W24
	.byte		N76   , Gn1 , v112 , gtp1
	.byte	W84
	.byte		N02   , Dn2 , v096
	.byte	W06
	.byte		        Dn2 , v088
	.byte	W06
	.byte		N44   , Gn1 , v112 , gtp3
	.byte	W48
	.byte		N11   , Gn1 , v100
	.byte	W12
	.byte		        Dn2 , v088
	.byte	W12
	.byte		N03   , Fn2 , v108
	.byte	W08
	.byte		        Fn2 , v096
	.byte	W08
	.byte		        Fs2 , v124
	.byte	W08
	.byte	GOTO
	 ptrvalue	mus_qz_619_5_B1
	.byte	FINE

/********************** Track  6 **********************/

mus_qz_619_6:
	.byte		KEYSH , mus_qz_619_key+0
	.byte		VOICE , 23
	.byte		VOL   , 64*mus_qz_619_mvl/mxv
	.byte		PAN   , c_v+36
	.byte		VOL   , 64*mus_qz_619_mvl/mxv
	.byte		PRIO  , 50
	.byte	W12
	.byte		N05   , Gn2 , v092
	.byte	W36
	.byte		N02   , Gn2 , v084
	.byte	W06
	.byte		        Gn2 , v068
	.byte	W06
	.byte		        Gn2 , v092
	.byte	W12
	.byte		        Gn2 , v084
	.byte	W36
	.byte		N05   , Gn2 , v092
	.byte	W36
	.byte		N02   , Gn2 , v088
	.byte	W06
	.byte		        Gn2 , v076
	.byte	W06
	.byte		        Gn2 , v088
	.byte	W06
	.byte		        Gn2 , v076
	.byte	W06
	.byte		        Gn2 , v084
	.byte	W36
	.byte		N05   , Gn2 , v092
	.byte	W36
	.byte		N02   , Gn2 , v076
	.byte	W06
	.byte		        Gn2 , v072
	.byte	W06
	.byte		        Gn2 , v088
	.byte	W12
	.byte		        Gn2 , v080
	.byte	W36
	.byte		N05   , Gn2 , v088
	.byte	W36
	.byte		N02   , Gn2 , v080
	.byte	W06
	.byte		        Gn2 , v072
	.byte	W06
	.byte		        Gn2 , v088
	.byte	W06
	.byte		        Gn2 , v076
	.byte	W06
	.byte		N32   , Dn3 , v084 , gtp3
	.byte	W36
mus_qz_619_6_B1:
	.byte	W24
	.byte		N05   , Gn2 , v088
	.byte	W06
	.byte		        An2 , v080
	.byte	W06
	.byte		        Bn2 , v092
	.byte	W06
	.byte		        Cn3 , v084
	.byte	W06
	.byte		        Bn2 , v092
	.byte	W06
	.byte		        An2 , v080
	.byte	W06
	.byte		        Gn2 , v092
	.byte	W06
	.byte		        Fs2 , v084
	.byte	W06
	.byte		N23   , Gn2 , v096
	.byte	W24
	.byte		N05   , An2 , v088
	.byte	W06
	.byte		        As2 , v080
	.byte	W06
	.byte		N03   , Cn3 , v096
	.byte	W12
	.byte		        Cn3 , v084
	.byte	W12
	.byte		N05   , An2 , v092
	.byte	W06
	.byte		        As2 , v084
	.byte	W06
	.byte		N03   , Cn3 , v096
	.byte	W12
	.byte		        Cn3 , v088
	.byte	W12
	.byte		N23   , Fs2 , v092
	.byte	W24
	.byte		N05   , Dn3 , v088
	.byte	W06
	.byte		        En3 , v080
	.byte	W06
	.byte		        Dn3 , v092
	.byte	W06
	.byte		        En3 , v088
	.byte	W06
	.byte		N44   , Dn3 , v092 , gtp3
	.byte	W30
	.byte		VOL   , 60*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        56*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        52*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        47*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        43*mus_qz_619_mvl/mxv
	.byte	W18
	.byte		        64*mus_qz_619_mvl/mxv
	.byte		N05   , Gn2
	.byte	W36
	.byte		N02   , Gn2 , v080
	.byte	W06
	.byte		        Gn2 , v068
	.byte	W06
	.byte		        Gn2 , v092
	.byte	W12
	.byte		        Gn2 , v084
	.byte	W12
	.byte		N03   , Gn2 , v092
	.byte	W08
	.byte		        Gn2 , v076
	.byte	W08
	.byte		        Gs2 , v088
	.byte	W08
	.byte		N05   , Fn2 , v092
	.byte	W36
	.byte		N02   , Fn2 , v084
	.byte	W06
	.byte		        Fn2 , v076
	.byte	W06
	.byte		        Fn2 , v088
	.byte	W12
	.byte		        Fn2 , v084
	.byte	W36
	.byte		N05   , Fn2 , v092
	.byte	W36
	.byte		N02   , Fn2 , v076
	.byte	W06
	.byte		        Fn2 , v068
	.byte	W06
	.byte		        Fn2 , v084
	.byte	W06
	.byte		        Fn2 , v080
	.byte	W42
	.byte		N05   , Gn2 , v088
	.byte	W36
	.byte		N02   , Gn2 , v080
	.byte	W06
	.byte		        Gn2 , v068
	.byte	W06
	.byte		        Gn2 , v088
	.byte	W12
	.byte		        Gn2 , v076
	.byte	W36
	.byte		N05   , Gn2 , v092
	.byte	W36
	.byte		N02   , Gn2 , v080
	.byte	W06
	.byte		        Gn2 , v068
	.byte	W06
	.byte		        Gn2 , v092
	.byte	W12
	.byte		        Gn2 , v084
	.byte	W11
	.byte		N03   , Gn2 , v092
	.byte	W08
	.byte		        Gn2 , v076
	.byte	W08
	.byte		        Gn2 , v088
	.byte	W09
	.byte		N44   , Fn2 , v100 , gtp3
	.byte	W30
	.byte		VOL   , 58*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        52*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        47*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        64*mus_qz_619_mvl/mxv
	.byte		N23   , Gn2 , v092
	.byte	W24
	.byte		        Cn2 , v096
	.byte	W24
	.byte		N44   , An2 , v104 , gtp3
	.byte	W30
	.byte		VOL   , 58*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        52*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        47*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        64*mus_qz_619_mvl/mxv
	.byte		N23   , Cn3 , v096
	.byte	W24
	.byte		        Fn3 , v088
	.byte	W24
	.byte		N32   , Bn2 , v104 , gtp3
	.byte	W17
	.byte		VOL   , 58*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        52*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        47*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        64*mus_qz_619_mvl/mxv
	.byte	W01
	.byte		N05   , As2 , v080
	.byte	W06
	.byte		        An2 , v068
	.byte	W06
	.byte		N44   , Gn2 , v092 , gtp3
	.byte	W30
	.byte		VOL   , 58*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        52*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        47*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        63*mus_qz_619_mvl/mxv
	.byte	W96
	.byte		N05   , Fn2 , v108
	.byte	W12
	.byte		        Cn2 , v096
	.byte	W06
	.byte		        Fn2 , v088
	.byte	W06
	.byte		N23   , Cn3 , v112
	.byte	W24
	.byte		N05   , Cn3 , v100
	.byte	W12
	.byte		        An2 , v092
	.byte	W06
	.byte		        Cn3 , v080
	.byte	W06
	.byte		N23   , En3 , v104
	.byte	W24
	.byte		N02   , Fn2 , v100
	.byte	W06
	.byte		N05   , Fn2 , v088
	.byte	W06
	.byte		        En2 , v080
	.byte	W06
	.byte		N02   , Fn2 , v092
	.byte	W06
	.byte		        An2 , v104
	.byte	W06
	.byte		N05   , An2 , v092
	.byte	W06
	.byte		        Gs2 , v084
	.byte	W06
	.byte		N02   , An2 , v092
	.byte	W06
	.byte		        Cn3 , v112
	.byte	W06
	.byte		N05   , Cn3 , v104
	.byte	W06
	.byte		        Bn2 , v096
	.byte	W06
	.byte		N02   , Cn3 , v100
	.byte	W06
	.byte		        Fn3 , v120
	.byte	W06
	.byte		N05   , Fn3 , v108
	.byte	W06
	.byte		        En3 , v100
	.byte	W06
	.byte		N02   , Fn3 , v104
	.byte	W06
	.byte		        Gn3 , v112
	.byte	W06
	.byte		N05   , Gn3 , v100
	.byte	W06
	.byte		        Fs3 , v092
	.byte	W06
	.byte		N02   , Gn3 , v100
	.byte	W06
	.byte		N05   , Fs3 , v112
	.byte	W06
	.byte		N02   , Gn3 , v100
	.byte	W06
	.byte		N05   , Fs3 , v112
	.byte	W06
	.byte		N02   , Gn3 , v104
	.byte	W06
	.byte		        Gn3 , v112
	.byte	W06
	.byte		        Gn3 , v100
	.byte	W06
	.byte		N05   , Fs3 , v104
	.byte	W06
	.byte		N02   , Gn3 , v100
	.byte	W06
	.byte		N05   , Gn3 , v112
	.byte	W06
	.byte		        An3 , v104
	.byte	W06
	.byte		        Dn3 , v116
	.byte	W06
	.byte		        Cn3 , v104
	.byte	W06
	.byte		VOL   , 13*mus_qz_619_mvl/mxv
	.byte		N44   , Bn2 , v104 , gtp3
	.byte	W03
	.byte		VOL   , 16*mus_qz_619_mvl/mxv
	.byte	W03
	.byte		        19*mus_qz_619_mvl/mxv
	.byte	W03
	.byte		        22*mus_qz_619_mvl/mxv
	.byte	W03
	.byte		        24*mus_qz_619_mvl/mxv
	.byte	W03
	.byte		        28*mus_qz_619_mvl/mxv
	.byte	W03
	.byte		        32*mus_qz_619_mvl/mxv
	.byte	W03
	.byte		        36*mus_qz_619_mvl/mxv
	.byte	W03
	.byte		        39*mus_qz_619_mvl/mxv
	.byte	W03
	.byte		        46*mus_qz_619_mvl/mxv
	.byte	W03
	.byte		        51*mus_qz_619_mvl/mxv
	.byte	W03
	.byte		        60*mus_qz_619_mvl/mxv
	.byte	W03
	.byte		        64*mus_qz_619_mvl/mxv
	.byte	W12
	.byte		N02   , Gn2
	.byte	W12
	.byte		N08   , Gn2 , v092
	.byte	W12
	.byte		N03   , Bn2 , v108
	.byte	W08
	.byte		        Bn2 , v096
	.byte	W08
	.byte		        Cn3 , v120
	.byte	W08
	.byte	GOTO
	 ptrvalue	mus_qz_619_6_B1
	.byte	FINE

/********************** Track  7 **********************/

mus_qz_619_7:
	.byte		KEYSH , mus_qz_619_key+0
	.byte		VOICE , 32
	.byte		VOL   , 73*mus_qz_619_mvl/mxv
	.byte		PAN   , c_v-19
	.byte		VOL   , 73*mus_qz_619_mvl/mxv
	.byte		PRIO  , 56
	.byte	W12
	.byte		N06   , Gn2 , v100
	.byte		N06   , Dn3 , v108
	.byte	W36
	.byte		N02   , Gn2 , v100
	.byte		N02   , Dn3 , v108
	.byte	W06
	.byte		        Gn2 , v092
	.byte		N02   , Dn3 , v100
	.byte	W06
	.byte		        Gn2
	.byte		N02   , Dn3 , v108
	.byte	W12
	.byte		        Gn2 , v092
	.byte		N02   , Dn3 , v100
	.byte	W36
	.byte		N06   , Gn2
	.byte		N06   , Cn3 , v116
	.byte	W36
	.byte		N02   , Gn2 , v100
	.byte		N02   , Cn3 , v116
	.byte	W06
	.byte		        Gn2 , v092
	.byte		N02   , Cn3 , v108
	.byte	W06
	.byte		        Gn2 , v100
	.byte		N02   , Cn3 , v116
	.byte	W06
	.byte		        Gn2 , v092
	.byte		N02   , Cn3 , v108
	.byte	W06
	.byte		        Gn2 , v104
	.byte		N02   , Cn3 , v120
	.byte	W36
	.byte		N06   , Gn2 , v100
	.byte		N06   , Dn3 , v108
	.byte	W36
	.byte		N02   , Gn2 , v100
	.byte		N02   , Dn3 , v108
	.byte	W06
	.byte		        Gn2 , v092
	.byte		N02   , Dn3 , v100
	.byte	W06
	.byte		        Gn2
	.byte		N02   , Dn3 , v108
	.byte	W12
	.byte		        Gn2 , v092
	.byte		N02   , Dn3 , v100
	.byte	W12
	.byte		        Dn3 , v116
	.byte		N02   , Gn3 , v108
	.byte	W12
	.byte		        Gn2 , v116
	.byte		N02   , Dn3 , v108
	.byte	W12
	.byte		N06   , Gn2 , v100
	.byte		N06   , Bn2 , v116
	.byte	W36
	.byte		N02   , Gn2 , v100
	.byte		N02   , Bn2 , v116
	.byte	W06
	.byte		        Gn2 , v092
	.byte		N02   , Bn2 , v108
	.byte	W06
	.byte		        Gn2 , v100
	.byte		N02   , Bn2 , v116
	.byte	W06
	.byte		        Gn2 , v092
	.byte		N02   , Bn2 , v108
	.byte	W06
	.byte		N11   , Fs2 , v088
	.byte		N11   , Cn3 , v104
	.byte	W12
	.byte		N22   , An2 , v100
	.byte		N22   , Dn3 , v116
	.byte	W24
mus_qz_619_7_B1:
	.byte		N06   , Gn2 , v092
	.byte		N06   , Dn3 , v108
	.byte	W36
	.byte		N02   , Gn2 , v092
	.byte		N02   , Dn3 , v108
	.byte	W06
	.byte		        Gn2 , v084
	.byte		N02   , Dn3 , v100
	.byte	W06
	.byte		        Gn2 , v092
	.byte		N02   , Dn3 , v108
	.byte	W12
	.byte		        Gn2 , v084
	.byte		N02   , Dn3 , v100
	.byte	W36
	.byte		N06   , An2 , v092
	.byte		N06   , Dn3 , v108
	.byte	W36
	.byte		N02   , An2 , v092
	.byte		N02   , Dn3 , v108
	.byte	W06
	.byte		        An2 , v088
	.byte		N02   , Dn3 , v100
	.byte	W06
	.byte		        An2 , v092
	.byte		N02   , Dn3 , v108
	.byte	W06
	.byte		        An2 , v088
	.byte		N02   , Dn3 , v100
	.byte	W06
	.byte		        An2 , v092
	.byte		N02   , Dn3 , v108
	.byte	W12
	.byte		N11   , Cn3 , v088
	.byte		N11   , Fs3 , v100
	.byte	W12
	.byte		        An2 , v088
	.byte		N11   , Dn3 , v100
	.byte	W12
	.byte		N06   , Gn2 , v092
	.byte		N06   , Dn3 , v108
	.byte	W36
	.byte		N02   , Gn2 , v092
	.byte		N02   , Dn3 , v108
	.byte	W06
	.byte		        Gn2 , v088
	.byte		N02   , Dn3 , v104
	.byte	W06
	.byte		        Gn2 , v092
	.byte		N02   , Dn3 , v108
	.byte	W12
	.byte		        Gn2 , v088
	.byte		N02   , Dn3 , v104
	.byte	W36
	.byte		N05   , Gn2 , v108
	.byte		N05   , Bn2 , v092
	.byte	W36
	.byte		N02   , Gn2 , v108
	.byte		N02   , Bn2 , v092
	.byte	W06
	.byte		        Gn2 , v100
	.byte		N02   , Bn2 , v088
	.byte	W06
	.byte		        Gn2 , v108
	.byte		N02   , Bn2 , v092
	.byte	W12
	.byte		N08   , Gn2 , v108
	.byte		N08   , Bn2 , v092
	.byte	W12
	.byte		N03   , Gn2
	.byte		N03   , Dn3 , v108
	.byte	W08
	.byte		        Gn2 , v084
	.byte		N03   , Dn3 , v100
	.byte	W08
	.byte		N07   , Fs2 , v092
	.byte		N07   , Ds3 , v108
	.byte	W08
	.byte		N44   , Cn4 , v100 , gtp3
	.byte	W30
	.byte		VOL   , 67*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        64*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        58*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        51*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        46*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        73*mus_qz_619_mvl/mxv
	.byte		N05   , Cn4 , v096
	.byte	W12
	.byte		        An3 , v080
	.byte	W12
	.byte		        Fn3 , v088
	.byte	W12
	.byte		N56   , Fn4 , v096 , gtp3
	.byte	W36
	.byte		VOL   , 67*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        62*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        59*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        49*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        73*mus_qz_619_mvl/mxv
	.byte		N05   , En4 , v092
	.byte	W12
	.byte		        Dn4 , v084
	.byte	W12
	.byte		        Cn4 , v088
	.byte	W12
	.byte		N32   , Bn3 , v096 , gtp3
	.byte	W36
	.byte		N05   , Gn3 , v084
	.byte	W12
	.byte		N44   , Dn4 , v092 , gtp3
	.byte	W30
	.byte		VOL   , 67*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        64*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        59*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        54*mus_qz_619_mvl/mxv
	.byte	W24
	.byte		        73*mus_qz_619_mvl/mxv
	.byte	W72
	.byte		N92   , Cn3 , v076 , gtp3
	.byte		N44   , Fn3 , v076 , gtp3
	.byte	W48
	.byte		        En3 , v072 , gtp3
	.byte	W48
	.byte		        Fn2 , v076 , gtp3
	.byte		N44   , Dn3 , v076 , gtp3
	.byte	W48
	.byte		N23   , Cn3
	.byte		N23   , Fn3
	.byte	W24
	.byte		        Cn3
	.byte		N23   , An3
	.byte	W24
	.byte		N44   , Gn2 , v088 , gtp3
	.byte		N32   , Bn2 , v076 , gtp3
	.byte	W36
	.byte		N05   , Cn3 , v044
	.byte	W06
	.byte		        Cs3
	.byte	W06
	.byte		N23   , Bn2 , v092
	.byte		N23   , Dn3 , v096
	.byte	W12
	.byte		VOL   , 65*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        59*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        73*mus_qz_619_mvl/mxv
	.byte		N11   , Dn3 , v116
	.byte	W12
	.byte		N05   , Bn3 , v100
	.byte	W12
	.byte		N23   , Cn4 , v124
	.byte	W24
	.byte		N11   , Bn3 , v108
	.byte	W24
	.byte		N23   , An3 , v120
	.byte	W24
	.byte		N11   , Gn3 , v108
	.byte	W24
	.byte		N23   , An2 , v080
	.byte		N23   , Fn3 , v072
	.byte	W24
	.byte		        Cn3 , v084
	.byte		N23   , Gn3 , v076
	.byte	W24
	.byte		        Cn3 , v088
	.byte		N23   , An3 , v084
	.byte	W24
	.byte		        En3
	.byte		N23   , Cn4 , v080
	.byte	W24
	.byte		N44   , Fn3 , v092 , gtp3
	.byte		N44   , Dn4 , v084 , gtp3
	.byte	W30
	.byte		VOL   , 67*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        62*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        57*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        71*mus_qz_619_mvl/mxv
	.byte		N44   , Fn3 , v080 , gtp3
	.byte		N44   , Cn4 , v084 , gtp3
	.byte	W30
	.byte		VOL   , 67*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        62*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        57*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        71*mus_qz_619_mvl/mxv
	.byte		N03   , Dn3 , v072
	.byte		N03   , Bn3 , v080
	.byte	W12
	.byte		N80   , Dn3 , v076 , gtp3
	.byte		N80   , Gn3 , v084 , gtp3
	.byte	W12
	.byte		VOL   , 65*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        59*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        51*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        47*mus_qz_619_mvl/mxv
	.byte	W12
	.byte		        52*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        54*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        59*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        62*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        67*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        73*mus_qz_619_mvl/mxv
	.byte	W12
	.byte		N03   , Dn3 , v072
	.byte		N03   , Bn3 , v080
	.byte	W12
	.byte		N32   , Bn2 , v080 , gtp3
	.byte		N32   , Gn3 , v084 , gtp3
	.byte	W36
	.byte		N03   , Dn3 , v072
	.byte		N03   , Gn3 , v080
	.byte	W12
	.byte		        Dn3 , v068
	.byte		N03   , Gn3 , v076
	.byte	W12
	.byte		        Bn2 , v072
	.byte		N03   , Fn3 , v080
	.byte	W08
	.byte		        Bn2 , v068
	.byte		N03   , Fn3 , v072
	.byte	W08
	.byte		        Bn2 , v080
	.byte		N03   , Fs3 , v096
	.byte	W08
	.byte	GOTO
	 ptrvalue	mus_qz_619_7_B1
	.byte	FINE

/********************** Track  8 **********************/

mus_qz_619_8:
	.byte		KEYSH , mus_qz_619_key+0
	.byte		VOICE , 28
	.byte		VOL   , 67*mus_qz_619_mvl/mxv
	.byte		PAN   , c_v-26
	.byte		PRIO  , 45
	.byte		N05   , Gn1 , v088
	.byte	W06
	.byte		        Gn1 , v076
	.byte	W06
	.byte		N17   , Gn1 , v112
	.byte	W96
	.byte		N23   , Gn1 , v100
	.byte	W96
	.byte		        Gn1 , v108
	.byte	W96
	.byte		        Gn1 , v112
	.byte	W36
	.byte		N08   , Gn1 , v084
	.byte	W12
	.byte		N05   , Fs2 , v100
	.byte	W12
	.byte		N11   , Dn3 , v124
	.byte	W24
	.byte		N04   , Gn1 , v088
	.byte	W06
	.byte	N04
	.byte	W06
mus_qz_619_8_B1:
	.byte		N23   , Gn1 , v108
	.byte	W84
	.byte		N04   , Gn1 , v096
	.byte	W06
	.byte		        Gn1 , v088
	.byte	W06
	.byte		N23   , Gn1 , v108
	.byte	W96
	.byte		        Gn1 , v104
	.byte	W60
	.byte		N11   , Gn1 , v100
	.byte	W24
	.byte		N05   , Gn1 , v092
	.byte	W06
	.byte		        Gn1 , v084
	.byte	W06
	.byte		N23   , Gn1 , v104
	.byte	W48
	.byte		N11   , Gn1 , v096
	.byte	W24
	.byte	N03
	.byte	W08
	.byte		        Gn1 , v084
	.byte	W08
	.byte		N07   , Fs1 , v096
	.byte	W08
	.byte		N23   , Gn1 , v108
	.byte	W84
	.byte		N02   , Gn1 , v100
	.byte	W06
	.byte		        Gn1 , v088
	.byte	W06
	.byte		N17   , Gn1 , v108
	.byte	W60
	.byte		N11   , Gn1 , v100
	.byte	W24
	.byte	N02
	.byte	W06
	.byte		        Gn1 , v092
	.byte	W06
	.byte		N23   , Gn1 , v108
	.byte	W60
	.byte		N11   , Gn1 , v100
	.byte	W24
	.byte	N02
	.byte	W06
	.byte		        Gn1 , v088
	.byte	W06
	.byte		N11   , Gn1 , v108
	.byte	W60
	.byte		        Gn1 , v100
	.byte	W12
	.byte		N03   , Cn2
	.byte	W08
	.byte		        Cn2 , v084
	.byte	W08
	.byte		        Fn1 , v100
	.byte	W08
	.byte		N20   , An1 , v108
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W72
	.byte		N02   , Dn2 , v068
	.byte	W03
	.byte		        Dn2 , v080
	.byte	W03
	.byte		        Dn2 , v088
	.byte	W03
	.byte		        Dn2 , v096
	.byte	W03
	.byte		        Dn2 , v104
	.byte	W03
	.byte		        Dn2 , v112
	.byte	W03
	.byte		        Dn2 , v120
	.byte	W03
	.byte		        Dn2 , v124
	.byte	W03
	.byte		N23   , Gn1
	.byte	W84
	.byte		N04   , Dn1 , v104
	.byte	W06
	.byte		        Dn1 , v092
	.byte	W06
	.byte		N23   , Gn1 , v116
	.byte	W48
	.byte		N11   , Dn1 , v104
	.byte	W12
	.byte		        Gn1 , v100
	.byte	W12
	.byte		N03   , Gn1 , v108
	.byte	W08
	.byte		        Gn1 , v092
	.byte	W08
	.byte		        Fs1 , v120
	.byte	W08
	.byte	GOTO
	 ptrvalue	mus_qz_619_8_B1
	.byte	FINE

/********************** Track  9 **********************/

mus_qz_619_9:
	.byte		KEYSH , mus_qz_619_key+0
	.byte		VOICE , 39
	.byte		VOL   , 71*mus_qz_619_mvl/mxv
	.byte		PAN   , c_v-34
	.byte		PRIO  , 40
	.byte		N05   , Dn1 , v084
	.byte	W06
	.byte		        Dn1 , v072
	.byte	W06
mus_qz_619_9_000:
	.byte		N11   , Dn1 , v092
	.byte		N28   , Cn2 , v100 , gtp1
	.byte	W36
	.byte		N05   , Dn1 , v084
	.byte	W06
	.byte		        Dn1 , v076
	.byte	W06
	.byte		        Dn1 , v096
	.byte	W12
	.byte		        Dn1 , v084
	.byte	W12
	.byte		N02   , Dn1 , v088
	.byte	W03
	.byte		        Dn1 , v064
	.byte	W03
	.byte		N01   , Dn1 , v044
	.byte	W02
	.byte		        Dn1 , v048
	.byte	W02
	.byte		        Dn1 , v024
	.byte	W02
	.byte		N05   , Dn1 , v084
	.byte	W06
	.byte		        Dn1 , v068
	.byte	W06
	.byte	PEND
mus_qz_619_9_001:
	.byte		N11   , Dn1 , v100
	.byte	W36
	.byte		N05   , Dn1 , v084
	.byte	W06
	.byte		        Dn1 , v072
	.byte	W06
	.byte		        Dn1 , v096
	.byte	W06
	.byte		        Dn1 , v084
	.byte	W06
	.byte		N02   , Dn1 , v096
	.byte	W03
	.byte		        Dn1 , v064
	.byte	W03
	.byte		        Dn1 , v036
	.byte	W03
	.byte		        Dn1 , v044
	.byte	W03
	.byte		        Dn1 , v036
	.byte	W03
	.byte		        Dn1 , v040
	.byte	W03
	.byte		        Dn1 , v048
	.byte	W03
	.byte		        Dn1 , v056
	.byte	W03
	.byte		N05   , Dn1 , v072
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte	PEND
	.byte		N11   , Dn1 , v100
	.byte		N28   , Cn2 , v100 , gtp1
	.byte	W36
	.byte		N05   , Dn1 , v092
	.byte	W06
	.byte		        Dn1 , v084
	.byte	W06
	.byte		        Dn1 , v100
	.byte	W12
	.byte		        Dn1 , v092
	.byte	W06
	.byte		        Dn1 , v080
	.byte	W06
	.byte		N02   , Dn1 , v096
	.byte	W03
	.byte		        Dn1 , v064
	.byte	W03
	.byte		        Dn1 , v036
	.byte	W03
	.byte		        Dn1 , v044
	.byte	W03
	.byte		N05   , Dn1 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte	N23
	.byte	W36
	.byte		N05   , Dn1 , v092
	.byte	W06
	.byte		        Dn1 , v080
	.byte	W06
	.byte		        Dn1 , v108
	.byte	W06
	.byte		        Dn1 , v092
	.byte	W05
	.byte		N02   , Dn1 , v088
	.byte	W03
	.byte		        Dn1 , v064
	.byte	W03
	.byte		        Dn1 , v036
	.byte	W03
	.byte		        Dn1 , v044
	.byte	W04
	.byte		N05   , Dn1 , v096
	.byte	W06
	.byte		        Dn1 , v084
	.byte	W06
	.byte		N02   , Dn1 , v096
	.byte	W03
	.byte		        Dn1 , v064
	.byte	W03
	.byte		        Dn1 , v036
	.byte	W03
	.byte		        Dn1 , v044
	.byte	W03
mus_qz_619_9_B1:
	.byte	PATT
	 ptrvalue	mus_qz_619_9_000
	.byte	PATT
	 ptrvalue	mus_qz_619_9_001
	.byte		N11   , Dn1 , v100
	.byte	W36
	.byte		N05   , Dn1 , v092
	.byte	W06
	.byte		        Dn1 , v080
	.byte	W06
	.byte		        Dn1 , v108
	.byte	W12
	.byte		        Dn1 , v096
	.byte	W12
	.byte		N02   , Dn1 , v088
	.byte	W03
	.byte		        Dn1 , v064
	.byte	W03
	.byte		N01   , Dn1 , v044
	.byte	W02
	.byte		        Dn1 , v048
	.byte	W02
	.byte		        Dn1 , v024
	.byte	W02
	.byte		N05   , Dn1 , v084
	.byte	W06
	.byte		        Dn1 , v068
	.byte	W06
	.byte		N11   , Dn1 , v100
	.byte	W36
	.byte		N05   , Dn1 , v092
	.byte	W06
	.byte		        Dn1 , v084
	.byte	W06
	.byte		        Dn1 , v104
	.byte	W12
	.byte		        Dn1 , v092
	.byte	W12
	.byte		N03   , Dn1 , v100
	.byte	W08
	.byte		        Dn1 , v088
	.byte	W08
	.byte		        Dn1 , v092
	.byte	W08
	.byte		N23   , Dn1 , v100
	.byte		N28   , Cn2 , v100 , gtp1
	.byte	W36
	.byte		N05   , Dn1 , v092
	.byte	W06
	.byte		        Dn1 , v080
	.byte	W06
	.byte		        Dn1 , v100
	.byte	W12
	.byte		        Dn1 , v092
	.byte	W12
	.byte		N02   , Dn1 , v088
	.byte	W03
	.byte		        Dn1 , v064
	.byte	W03
	.byte		N01   , Dn1 , v044
	.byte	W02
	.byte		        Dn1 , v048
	.byte	W02
	.byte		        Dn1 , v024
	.byte	W02
	.byte		N05   , Dn1 , v084
	.byte	W06
	.byte		        Dn1 , v068
	.byte	W06
	.byte	PATT
	 ptrvalue	mus_qz_619_9_001
	.byte		N23   , Dn1 , v100
	.byte	W36
	.byte		N05   , Dn1 , v092
	.byte	W06
	.byte		        Dn1 , v080
	.byte	W06
	.byte		        Dn1 , v100
	.byte	W12
	.byte		        Dn1 , v092
	.byte	W12
	.byte		N02   , Dn1 , v088
	.byte	W03
	.byte		        Dn1 , v064
	.byte	W03
	.byte		N01   , Dn1 , v044
	.byte	W02
	.byte		        Dn1 , v048
	.byte	W02
	.byte		        Dn1 , v024
	.byte	W02
	.byte		N05   , Dn1 , v084
	.byte	W06
	.byte		        Dn1 , v068
	.byte	W06
	.byte		N23   , Dn1 , v100
	.byte	W36
	.byte		N05   , Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v080
	.byte	W06
	.byte		        Dn1 , v100
	.byte	W12
	.byte		        Dn1 , v088
	.byte	W12
	.byte		N03   , Dn1 , v100
	.byte	W08
	.byte		        Dn1 , v088
	.byte	W08
	.byte		        Dn1 , v092
	.byte	W08
	.byte		N32   , Cn2 , v100 , gtp3
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W48
	.byte		N44   , Gn0 , v124 , gtp3
	.byte	W48
	.byte		N32   , Cn2 , v112 , gtp3
	.byte	W96
	.byte		        Cn2 , v112 , gtp3
	.byte	W48
	.byte		N05   , Dn1 , v092
	.byte	W12
	.byte		        Dn1 , v084
	.byte	W12
	.byte		N03   , Dn1 , v100
	.byte	W08
	.byte		        Dn1 , v092
	.byte	W08
	.byte		        Dn1 , v112
	.byte	W08
	.byte	GOTO
	 ptrvalue	mus_qz_619_9_B1
	.byte	FINE

/********************** Track 10 **********************/

mus_qz_619_10:
	.byte		KEYSH , mus_qz_619_key+0
	.byte		VOICE , 35
	.byte		VOL   , 52*mus_qz_619_mvl/mxv
	.byte		PAN   , c_v-18
	.byte		VOL   , 52*mus_qz_619_mvl/mxv
	.byte		BENDR , 6
	.byte		PRIO  , 48
	.byte		BEND  , c_v+2
	.byte	W12
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
mus_qz_619_10_B1:
	.byte	W96
	.byte	W96
	.byte	W48
	.byte		N05   , Dn4 , v088
	.byte	W06
	.byte		        Gn4 , v080
	.byte	W06
	.byte		        An4 , v092
	.byte	W06
	.byte		        Gn4 , v084
	.byte	W06
	.byte		        Fn4 , v092
	.byte	W06
	.byte		        En4 , v080
	.byte	W06
	.byte		        Dn4 , v088
	.byte	W06
	.byte		        An3 , v076
	.byte	N05
	.byte	W06
	.byte		        Bn3 , v092
	.byte	W06
	.byte		        Cn4 , v080
	.byte	W06
	.byte		        Bn3 , v088
	.byte	W06
	.byte		        Cn4 , v080
	.byte	W06
	.byte		N32   , Bn3 , v072 , gtp3
	.byte	W24
	.byte		VOL   , 47*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        41*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        36*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        30*mus_qz_619_mvl/mxv
	.byte	W30
	.byte		        52*mus_qz_619_mvl/mxv
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		N04   , Cn5 , v108
	.byte	W06
	.byte		        Bn4 , v088
	.byte	W06
	.byte		        An4 , v096
	.byte	W06
	.byte		        Gn4 , v088
	.byte	W06
	.byte		        Bn3 , v096
	.byte	W06
	.byte		        Gn4 , v084
	.byte	W06
	.byte		        Cn5 , v108
	.byte	W06
	.byte		        Bn4 , v092
	.byte	W06
	.byte		        An4 , v100
	.byte	W06
	.byte		        Gn4 , v092
	.byte	W06
	.byte		        Fn4 , v100
	.byte	W06
	.byte		        En4 , v088
	.byte	W06
	.byte		N02   , Dn4 , v084
	.byte	W03
	.byte		        Cn4 , v072
	.byte	W03
	.byte		        An3 , v080
	.byte	W03
	.byte		        Gn3 , v068
	.byte	W03
	.byte		        Fn3 , v076
	.byte	W03
	.byte		        En3 , v064
	.byte	W03
	.byte		        Dn3 , v072
	.byte	W03
	.byte		        Cn3 , v056
	.byte	W03
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_619_10_B1
	.byte	FINE

/********************** Track 11 **********************/

mus_qz_619_11:
	.byte		KEYSH , mus_qz_619_key+0
	.byte		VOICE , 23
	.byte		VOL   , 13*mus_qz_619_mvl/mxv
	.byte		PAN   , c_v+56
	.byte		VOL   , 13*mus_qz_619_mvl/mxv
	.byte		PRIO  , 36
	.byte	W12
	.byte	W06
	.byte		N05   , Bn4 , v104
	.byte	W06
	.byte		        Cn5 , v088
	.byte	W06
	.byte		        Bn4 , v096
	.byte	W06
	.byte		        An4 , v084
	.byte	W06
	.byte		        Gn4 , v096
	.byte	W06
	.byte		        Fs4 , v084
	.byte	W06
	.byte		        An4 , v092
	.byte	W06
	.byte		        Gn4 , v084
	.byte	W06
	.byte		        En4 , v096
	.byte	W06
	.byte		        Fs4 , v084
	.byte	W06
	.byte		        Dn4 , v092
	.byte	W06
	.byte		        En4 , v084
	.byte	W06
	.byte		N23   , Cn4 , v092
	.byte	W18
	.byte	W06
	.byte		N02   , Cn4 , v104
	.byte	W06
	.byte		N05   , Cn4 , v092
	.byte	W06
	.byte		        Bn3 , v084
	.byte	W06
	.byte		N02   , Cn4 , v092
	.byte	W06
	.byte		        Cn4 , v104
	.byte	W06
	.byte		N05   , Cn4 , v088
	.byte	W06
	.byte		        Bn3 , v080
	.byte	W06
	.byte		N02   , Cn4 , v088
	.byte	W06
	.byte		N05   , Gn4 , v104
	.byte	W06
	.byte		N02   , Cn4 , v088
	.byte	W06
	.byte		N05   , Bn3 , v080
	.byte	W06
	.byte		N02   , Cn4 , v088
	.byte	W06
	.byte		        Cn4 , v100
	.byte	W06
	.byte		N05   , Cn4 , v092
	.byte	W06
	.byte		        Bn3 , v084
	.byte	W06
	.byte		N02   , Cn4 , v088
	.byte	W06
	.byte		N05   , Bn3 , v096
	.byte	W06
	.byte		        Cn4 , v088
	.byte	W06
	.byte		N68   , Dn4 , v104 , gtp3
	.byte	W12
	.byte		VOL   , 12*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        10*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        9*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        8*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        9*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        10*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        10*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        11*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        13*mus_qz_619_mvl/mxv
	.byte	W12
	.byte		N05   , An3 , v092
	.byte	W06
	.byte		        Bn3 , v080
	.byte	W06
	.byte		N02   , Cn4 , v104
	.byte	W06
	.byte		N05   , Cn4 , v092
	.byte	W06
	.byte		        Bn3 , v084
	.byte	W06
	.byte		N02   , Cn4 , v088
	.byte	W06
	.byte		N05   , An3 , v108
	.byte	W06
	.byte		        Bn3 , v092
	.byte	W06
	.byte		N02   , Cn4 , v104
	.byte	W06
	.byte		        An3 , v088
	.byte	W06
	.byte		        An3 , v104
	.byte	W24
	.byte		N05   , Dn3 , v100
	.byte	W06
	.byte		        En3 , v088
	.byte	W06
	.byte		        Fn3 , v108
	.byte	W06
mus_qz_619_11_B1:
	.byte		N05   , Fs3 , v096
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W06
	.byte		N11   , Gs3 , v108
	.byte	W12
	.byte		N06   , An3 , v096
	.byte	W24
	.byte		N11   , Bn3 , v108
	.byte	W24
	.byte		        Cn4 , v112
	.byte	W24
	.byte		        En4 , v096
	.byte	W06
	.byte	W06
	.byte		N44   , Fn4 , v112 , gtp3
	.byte	W48
	.byte		        An4 , v100 , gtp3
	.byte	W42
	.byte	W06
	.byte		N03   , Gn4 , v116
	.byte	W12
	.byte		TIE   , Dn4 , v104
	.byte	W12
	.byte		VOL   , 12*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        11*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        10*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        9*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        8*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        7*mus_qz_619_mvl/mxv
	.byte	W18
	.byte		        8*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        8*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        9*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        10*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        10*mus_qz_619_mvl/mxv
	.byte	W12
	.byte		        11*mus_qz_619_mvl/mxv
	.byte	W12
	.byte		        12*mus_qz_619_mvl/mxv
	.byte	W12
	.byte		        13*mus_qz_619_mvl/mxv
	.byte	W12
	.byte		        13*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        12*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        11*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        10*mus_qz_619_mvl/mxv
	.byte	W05
	.byte	EOT
	.byte	W01
	.byte		VOL   , 8*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        7*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        5*mus_qz_619_mvl/mxv
	.byte	W06
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_619_11_B1
	.byte	FINE

/********************** Track 12 **********************/

mus_qz_619_12:
	.byte		KEYSH , mus_qz_619_key+0
	.byte		VOICE , 29
	.byte		VOL   , 18*mus_qz_619_mvl/mxv
	.byte		PAN   , c_v+56
	.byte		VOL   , 18*mus_qz_619_mvl/mxv
	.byte		PRIO  , 36
	.byte	W12
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W78
	.byte		N05   , Dn3 , v116
	.byte	W06
	.byte		        En3 , v108
	.byte	W06
	.byte		        Fn3 , v116
	.byte	W06
mus_qz_619_12_B1:
	.byte		N05   , Fs3 , v112
	.byte	W06
	.byte		N52   , Gn3 , v124 , gtp1
	.byte	W36
	.byte		VOL   , 16*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        16*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        13*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        12*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        18*mus_qz_619_mvl/mxv
	.byte		N03   , Dn4 , v120
	.byte	W12
	.byte		        Bn3 , v108
	.byte	W12
	.byte		        Gn3 , v112
	.byte	W06
	.byte	W06
	.byte		N56   , Fn4 , v120 , gtp1
	.byte	W36
	.byte		VOL   , 16*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        15*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        13*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        12*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        18*mus_qz_619_mvl/mxv
	.byte		N11   , En4 , v112
	.byte	W12
	.byte		        Dn4 , v120
	.byte	W12
	.byte		        Cn4 , v112
	.byte	W06
	.byte	W06
	.byte		N05   , Bn3 , v120
	.byte	W06
	.byte		        Cn4 , v108
	.byte	W06
	.byte		        Bn3 , v116
	.byte	W06
	.byte		        Cn4 , v108
	.byte	W06
	.byte		N56   , Bn3 , v124 , gtp3
	.byte	W30
	.byte		VOL   , 16*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        15*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        13*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        11*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        10*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        8*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        6*mus_qz_619_mvl/mxv
	.byte	W48
	.byte		        18*mus_qz_619_mvl/mxv
	.byte	W48
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W06
	.byte		N22   , An3 , v100
	.byte	W24
	.byte		        Bn3 , v096
	.byte	W24
	.byte		        Cn4 , v104
	.byte	W24
	.byte		        En4 , v100
	.byte	W18
	.byte	W06
	.byte		N44   , Fn4 , v108 , gtp2
	.byte	W48
	.byte		        Cn5 , v100 , gtp2
	.byte	W42
	.byte	W06
	.byte		N03   , Bn4
	.byte	W12
	.byte		N80   , Gn4 , v108 , gtp2
	.byte	W66
	.byte		VOL   , 16*mus_qz_619_mvl/mxv
	.byte	W03
	.byte		        16*mus_qz_619_mvl/mxv
	.byte	W03
	.byte		        15*mus_qz_619_mvl/mxv
	.byte	W03
	.byte		        13*mus_qz_619_mvl/mxv
	.byte	W03
	.byte		        13*mus_qz_619_mvl/mxv
	.byte	W03
	.byte		        11*mus_qz_619_mvl/mxv
	.byte	W03
	.byte		        18*mus_qz_619_mvl/mxv
	.byte		N03   , Bn4 , v100
	.byte	W12
	.byte		N32   , Gn4 , v108 , gtp2
	.byte	W36
	.byte		N03   , Gn4 , v100
	.byte	W12
	.byte		        Gn4 , v096
	.byte	W12
	.byte		N04   , Fn4 , v100
	.byte	W08
	.byte	N04
	.byte	W10
	.byte	GOTO
	 ptrvalue	mus_qz_619_12_B1
	.byte	FINE

/********************** Track 13 **********************/

mus_qz_619_13:
	.byte		KEYSH , mus_qz_619_key+0
	.byte		VOICE , 32
	.byte		VOL   , 16*mus_qz_619_mvl/mxv
	.byte		PAN   , c_v+56
	.byte		VOL   , 16*mus_qz_619_mvl/mxv
	.byte		PRIO  , 34
	.byte	W12
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
mus_qz_619_13_B1:
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W06
	.byte		N44   , Cn4 , v116 , gtp3
	.byte	W30
	.byte		VOL   , 15*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        13*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        13*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        11*mus_qz_619_mvl/mxv
	.byte	W12
	.byte		        16*mus_qz_619_mvl/mxv
	.byte		N05   , Cn4 , v112
	.byte	W12
	.byte		        An3 , v096
	.byte	W12
	.byte		        Fn3 , v104
	.byte	W06
	.byte	W06
	.byte		N56   , Fn4 , v112 , gtp3
	.byte	W42
	.byte		VOL   , 15*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        13*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        13*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        11*mus_qz_619_mvl/mxv
	.byte		N05   , En4
	.byte	W06
	.byte		VOL   , 16*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		N05   , Dn4 , v100
	.byte	W12
	.byte		        Cn4 , v108
	.byte	W06
	.byte	W06
	.byte		N32   , Bn3 , v112 , gtp3
	.byte	W36
	.byte		N05   , Gn3 , v100
	.byte	W12
	.byte		N44   , Dn4 , v112 , gtp3
	.byte	W30
	.byte		VOL   , 15*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        13*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        13*mus_qz_619_mvl/mxv
	.byte	W06
	.byte		        11*mus_qz_619_mvl/mxv
	.byte	W24
	.byte		        16*mus_qz_619_mvl/mxv
	.byte	W66
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_619_13_B1
	.byte	FINE

/********************** Track 14 **********************/

mus_qz_619_14:
	.byte		KEYSH , mus_qz_619_key+0
	.byte		VOICE , 0
	.byte		VOL   , 46*mus_qz_619_mvl/mxv
	.byte		PAN   , c_v-39
	.byte		VOL   , 46*mus_qz_619_mvl/mxv
	.byte		PRIO  , 52
	.byte	W12
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
mus_qz_619_14_B1:
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W48
	.byte		N04   , Gn4 , v088
	.byte	W06
	.byte		        An4 , v072
	.byte	W06
	.byte		        Gn4 , v088
	.byte	W06
	.byte		        An4 , v080
	.byte	W06
	.byte		N10   , Gn4 , v092
	.byte	W12
	.byte		        Dn4 , v080
	.byte	W12
	.byte		N04   , Cn5 , v108
	.byte	W06
	.byte		        Bn4 , v088
	.byte	W06
	.byte		        An4 , v096
	.byte	W06
	.byte		        Gn4 , v088
	.byte	W06
	.byte		        Gn3
	.byte		N04   , Bn3 , v096
	.byte	W06
	.byte		        Dn4 , v076
	.byte		N04   , Gn4 , v084
	.byte	W06
	.byte		        Gn4 , v096
	.byte		N04   , Cn5 , v108
	.byte	W06
	.byte		        Gn4 , v084
	.byte		N04   , Bn4 , v092
	.byte	W06
	.byte		        En4
	.byte		N04   , An4 , v100
	.byte	W06
	.byte		        Dn4 , v084
	.byte		N04   , Gn4 , v092
	.byte	W06
	.byte		        Cn4 , v088
	.byte		N04   , Fn4 , v100
	.byte	W06
	.byte		        Bn3 , v076
	.byte		N04   , En4 , v088
	.byte	W06
	.byte		N02   , Dn4 , v084
	.byte	W03
	.byte		        Cn4 , v072
	.byte	W03
	.byte		        An3 , v080
	.byte	W03
	.byte		        Gn3 , v068
	.byte	W03
	.byte		        Fn3 , v076
	.byte	W03
	.byte		        En3 , v064
	.byte	W03
	.byte		        Dn3 , v072
	.byte	W03
	.byte		        Cn3 , v056
	.byte	W03
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_619_14_B1
	.byte	FINE

/******************************************************/
	.p2align	2

mus_qz_619:
	.byte	14	/* NumTrks */
	.byte	0	/* NumBlks */
	.byte	mus_qz_619_pri	/* Priority */
	.byte	mus_qz_619_rev	/* Reverb. */
space64 4
	ptrvalue	mus_qz_619_grp

	ptrvalue	mus_qz_619_1
	ptrvalue	mus_qz_619_2
	ptrvalue	mus_qz_619_3
	ptrvalue	mus_qz_619_4
	ptrvalue	mus_qz_619_5
	ptrvalue	mus_qz_619_6
	ptrvalue	mus_qz_619_7
	ptrvalue	mus_qz_619_8
	ptrvalue	mus_qz_619_9
	ptrvalue	mus_qz_619_10
	ptrvalue	mus_qz_619_11
	ptrvalue	mus_qz_619_12
	ptrvalue	mus_qz_619_13
	ptrvalue	mus_qz_619_14

	.end
