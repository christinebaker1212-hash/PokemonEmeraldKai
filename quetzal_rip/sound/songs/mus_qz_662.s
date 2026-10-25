	.include "asm/macros/bit_width.inc"

	.include "MPlayDef.s"

/* Moemon Quetzal song 662, disassembled from the ROM by tools/quetzal_rip/songs.py.
   "org" comments give the ROM address of the bytes that follow. */

	.equ	mus_qz_662_grp, voicegroup_qz_songs
	.equ	mus_qz_662_pri, 0
	.equ	mus_qz_662_rev, reverb_set+0
	.equ	mus_qz_662_mvl, 127
	.equ	mus_qz_662_key, 0
	.equ	mus_qz_662_tbs, 1
	.equ	mus_qz_662_exg, 1
	.equ	mus_qz_662_cmp, 1

	.section .rodata
	.global	mus_qz_662
	.p2align	2
/* org 0x096A19E8 */

/********************** Track  1 **********************/

mus_qz_662_1:
	.byte		KEYSH , mus_qz_662_key+0
	.byte		TEMPO , 144*mus_qz_662_tbs/2
	.byte		VOICE , 0
	.byte		VOL   , 74*mus_qz_662_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		VOL   , 58*mus_qz_662_mvl/mxv
	.byte		PRIO  , 62
	.byte		N04   , Bn3 , v100
	.byte	W02
	.byte		        Bn4 , v116
	.byte	W10
	.byte		N32   , Bn4 , v108 , gtp3
	.byte	W02
	.byte		        Bn5 , v120 , gtp3
	.byte	W32
	.byte	W02
	.byte		N10   , En5 , v108
	.byte	W12
	.byte		        Ds5 , v092
	.byte	W12
	.byte		        Cs5 , v104
	.byte	W12
	.byte		        Cn5 , v088
	.byte	W12
mus_qz_662_1_B1:
	.byte		N10   , Bn4 , v112
	.byte	W24
	.byte		        Ds5 , v100
	.byte	W24
	.byte		        En5 , v112
	.byte	W24
	.byte		        Ds5 , v100
	.byte	W24
mus_qz_662_1_000:
	.byte		N10   , Cs5 , v112
	.byte	W12
	.byte		        Cn5 , v100
	.byte	W12
	.byte		        Bn4 , v108
	.byte	W12
	.byte		        An4 , v096
	.byte	W12
	.byte		        Gs4 , v108
	.byte	W12
	.byte		        An4 , v096
	.byte	W12
	.byte		        As4 , v108
	.byte	W12
	.byte		        Cs5 , v096
	.byte	W12
	.byte	PEND
mus_qz_662_1_001:
	.byte		N10   , Bn4 , v112
	.byte	W24
	.byte		        Cs5 , v096
	.byte	W24
	.byte		        Bn4 , v108
	.byte	W24
	.byte		        An4 , v096
	.byte	W24
	.byte	PEND
mus_qz_662_1_002:
	.byte		N10   , Gs4 , v108
	.byte	W12
	.byte		        Fs4 , v096
	.byte	W12
	.byte		        En4 , v100
	.byte	W12
	.byte		        Ds4 , v092
	.byte	W12
	.byte		        En4 , v108
	.byte	W12
	.byte		        Fs4 , v092
	.byte	W12
	.byte		        Gs4 , v104
	.byte	W12
	.byte		        An4 , v092
	.byte	W12
	.byte	PEND
	.byte		        Bn4 , v112
	.byte	W24
	.byte		        Ds5 , v100
	.byte	W24
	.byte		        En5 , v112
	.byte	W24
	.byte		        Ds5 , v100
	.byte	W24
	.byte	PATT
	 ptrvalue	mus_qz_662_1_000
	.byte	PATT
	 ptrvalue	mus_qz_662_1_001
	.byte	PATT
	 ptrvalue	mus_qz_662_1_002
	.byte	GOTO
	 ptrvalue	mus_qz_662_1_B1
	.byte	FINE

/********************** Track  2 **********************/

mus_qz_662_2:
	.byte		KEYSH , mus_qz_662_key+0
	.byte		VOICE , 36
	.byte		VOL   , 76*mus_qz_662_mvl/mxv
	.byte		PAN   , c_v+36
	.byte		VOL   , 59*mus_qz_662_mvl/mxv
	.byte		PRIO  , 50
	.byte	W96
mus_qz_662_2_B1:
	.byte		VOL   , 76*mus_qz_662_mvl/mxv
	.byte		N08   , En1 , v112
	.byte	W24
	.byte		N05   , En1 , v100
	.byte	W24
	.byte		        En1 , v108
	.byte	W24
	.byte		        En1 , v096
	.byte	W12
	.byte		N11   , Bn1 , v120
	.byte	W12
	.byte		N08   , En1 , v108
	.byte	W24
	.byte		N05   , En1 , v096
	.byte	W24
	.byte		        En1 , v108
	.byte	W24
	.byte		        En1 , v096
	.byte	W12
	.byte		N11   , Bn0 , v120
	.byte	W12
	.byte		N08   , En1 , v112
	.byte	W24
	.byte		N05   , En1 , v096
	.byte	W24
	.byte		        En1 , v108
	.byte	W24
	.byte		        En1 , v096
	.byte	W12
	.byte		N11   , Bn1 , v100
	.byte	W12
	.byte		N08   , En1 , v112
	.byte	W24
	.byte		N05   , En1 , v096
	.byte	W24
	.byte		        En1 , v108
	.byte	W24
	.byte		        Bn0 , v092
	.byte	W12
	.byte		N11   , Bn1 , v116
	.byte	W12
	.byte		VOL   , 76*mus_qz_662_mvl/mxv
	.byte		N08   , En1 , v112
	.byte	W24
	.byte		N05   , En1 , v100
	.byte	W24
	.byte		        En1 , v108
	.byte	W24
	.byte		        Bn0 , v096
	.byte	W12
	.byte		N11   , Bn1 , v120
	.byte	W12
	.byte		N08   , Fs1 , v108
	.byte	W24
	.byte		N05   , Cs2 , v096
	.byte	W24
	.byte		        Bn0 , v108
	.byte	W24
	.byte		        Fs1 , v096
	.byte	W12
	.byte		N11   , Bn0 , v120
	.byte	W12
	.byte		N08   , En1 , v112
	.byte	W24
	.byte		N05   , En1 , v096
	.byte	W24
	.byte		        En1 , v108
	.byte	W24
	.byte		        Bn0 , v096
	.byte	W12
	.byte		N11   , Bn1 , v100
	.byte	W12
	.byte		N08   , En1 , v112
	.byte	W24
	.byte		N05   , En1 , v096
	.byte	W24
	.byte		        En1 , v108
	.byte	W24
	.byte		        En1 , v092
	.byte	W12
	.byte		N11   , Bn1 , v116
	.byte	W12
	.byte	GOTO
	 ptrvalue	mus_qz_662_2_B1
	.byte	FINE

/********************** Track  3 **********************/

mus_qz_662_3:
	.byte		KEYSH , mus_qz_662_key+0
	.byte		VOICE , 39
	.byte		VOL   , 76*mus_qz_662_mvl/mxv
	.byte		PAN   , c_v-19
	.byte		VOL   , 59*mus_qz_662_mvl/mxv
	.byte		PRIO  , 40
	.byte	W84
	.byte		N02   , As0 , v068
	.byte	W03
	.byte		        As0 , v056
	.byte	W03
	.byte		        As0 , v084
	.byte	W03
	.byte		        As0 , v072
	.byte	W03
mus_qz_662_3_B1:
	.byte		N05   , As0 , v096
	.byte	W24
	.byte		        As0 , v072
	.byte		N11   , Fs2 , v056
	.byte	W24
	.byte		N05   , As0 , v088
	.byte	W24
	.byte		        As0 , v072
	.byte		N11   , Fs2 , v056
	.byte	W12
	.byte		N02   , As0 , v068
	.byte	W03
	.byte		        As0 , v056
	.byte	W03
	.byte		        As0 , v084
	.byte	W03
	.byte		        As0 , v072
	.byte	W03
	.byte		N05   , As0 , v100
	.byte	W24
	.byte		        As0 , v068
	.byte		N11   , Fs2 , v056
	.byte	W24
	.byte		N05   , As0 , v088
	.byte	W24
	.byte		        As0 , v060
	.byte		N11   , Fs2 , v056
	.byte	W12
	.byte		N02   , As0 , v068
	.byte		N11   , Fs2 , v048
	.byte	W03
	.byte		N02   , As0 , v052
	.byte	W03
	.byte		        As0 , v084
	.byte	W03
	.byte		        As0 , v072
	.byte	W03
	.byte		N05   , As0 , v096
	.byte	W24
	.byte		        As0 , v068
	.byte		N11   , Fs2 , v056
	.byte	W24
	.byte		N05   , As0 , v084
	.byte	W24
	.byte		        As0 , v056
	.byte		N11   , Fs2
	.byte	W12
	.byte		N02   , As0 , v064
	.byte	W03
	.byte		        As0 , v056
	.byte	W03
	.byte		        As0 , v084
	.byte	W03
	.byte		        As0 , v072
	.byte	W03
	.byte		N05   , As0 , v096
	.byte	W24
	.byte		        As0 , v068
	.byte		N11   , Fs2 , v056
	.byte	W24
	.byte		N05   , As0 , v084
	.byte	W24
	.byte		        As0 , v068
	.byte		N11   , Fs2 , v056
	.byte	W12
	.byte		N02   , As0 , v060
	.byte		N11   , Fs2 , v048
	.byte	W03
	.byte		N02   , As0 , v052
	.byte	W03
	.byte		        As0 , v080
	.byte	W03
	.byte		        As0 , v072
	.byte	W03
mus_qz_662_3_000:
	.byte		N05   , As0 , v080
	.byte		N05   , Gs4 , v088
	.byte	W12
	.byte		        Gs4 , v060
	.byte	W12
	.byte		        As0 , v056
	.byte		N11   , An4 , v100
	.byte	W12
	.byte		N05   , Gs4 , v048
	.byte	W12
	.byte		        As0 , v056
	.byte		N05   , Gs4 , v088
	.byte	W12
	.byte		        Gs4 , v060
	.byte	W12
	.byte		        As0 , v052
	.byte		N11   , An4 , v100
	.byte	W12
	.byte		N02   , As0 , v080
	.byte		N05   , Gs4 , v048
	.byte	W03
	.byte		N02   , As0 , v080
	.byte	W03
	.byte	N02
	.byte	W03
	.byte	N02
	.byte	W03
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_662_3_000
	.byte	PATT
	 ptrvalue	mus_qz_662_3_000
	.byte	PATT
	 ptrvalue	mus_qz_662_3_000
	.byte	GOTO
	 ptrvalue	mus_qz_662_3_B1
	.byte	FINE

/********************** Track  4 **********************/

mus_qz_662_4:
	.byte		KEYSH , mus_qz_662_key+0
	.byte		VOICE , 33
	.byte		VOL   , 64*mus_qz_662_mvl/mxv
	.byte		PAN   , c_v+4
	.byte		VOL   , 50*mus_qz_662_mvl/mxv
	.byte		PRIO  , 64
	.byte	W96
mus_qz_662_4_B1:
	.byte		N06   , Gs3 , v088
	.byte	W24
	.byte		        Bn3 , v076
	.byte	W24
	.byte		        Cs4 , v088
	.byte	W24
	.byte		        Bn3 , v076
	.byte	W24
	.byte	PEND
	.byte		N10   , Cs4 , v088
	.byte	W12
	.byte		        Cn4 , v076
	.byte	W12
	.byte		        Bn3 , v080
	.byte	W12
	.byte		        An3 , v072
	.byte	W12
	.byte		        Gs3 , v084
	.byte	W12
	.byte		        An3 , v072
	.byte	W12
	.byte		        As3 , v084
	.byte	W12
	.byte		        Cs4 , v072
	.byte	W12
	.byte		N04   , Gs3 , v088
	.byte	W24
	.byte		        An3 , v072
	.byte	W24
	.byte		        Bn3 , v084
	.byte	W24
	.byte		        An3 , v072
	.byte	W24
	.byte		N10   , Gs3 , v084
	.byte	W12
	.byte		        Fs3 , v072
	.byte	W12
	.byte		        En3 , v076
	.byte	W12
	.byte		        Ds3 , v068
	.byte	W12
	.byte		        En3 , v080
	.byte	W12
	.byte		        Fs3 , v072
	.byte	W12
	.byte		        Gs3 , v080
	.byte	W12
	.byte		        An3 , v068
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_662_4_B1
	.byte		N10   , En4 , v076
	.byte	W12
	.byte		        Ds4 , v064
	.byte	W12
	.byte		        Dn4 , v072
	.byte	W12
	.byte		        Cs4 , v064
	.byte	W12
	.byte		        Cn4 , v072
	.byte	W12
	.byte		        Cs4 , v064
	.byte	W12
	.byte		        Ds4 , v072
	.byte	W12
	.byte		        Fs4 , v060
	.byte	W12
	.byte		N04   , En4 , v088
	.byte	W24
	.byte		        Fs4 , v072
	.byte	W24
	.byte		        En4 , v084
	.byte	W24
	.byte		        Ds4 , v072
	.byte	W24
	.byte		N10   , Bn3 , v080
	.byte	W12
	.byte		        An3 , v068
	.byte	W12
	.byte		        Gs3 , v076
	.byte	W12
	.byte		        Fs3 , v068
	.byte	W12
	.byte		        Gs3 , v080
	.byte	W12
	.byte		        An3 , v068
	.byte	W12
	.byte		        Bn3 , v076
	.byte	W12
	.byte		        Cs4 , v068
	.byte	W12
	.byte	GOTO
	 ptrvalue	mus_qz_662_4_B1
	.byte	FINE

/********************** Track  5 **********************/

mus_qz_662_5:
	.byte		KEYSH , mus_qz_662_key+0
	.byte		VOICE , 35
	.byte		VOL   , 70*mus_qz_662_mvl/mxv
	.byte		PAN   , c_v+6
	.byte		VOL   , 54*mus_qz_662_mvl/mxv
	.byte		PRIO  , 64
	.byte		N04   , Bn4 , v108
	.byte	W12
	.byte		N80   , Bn5
	.byte	W36
	.byte		VOL   , 70*mus_qz_662_mvl/mxv
	.byte	W01
	.byte		        66*mus_qz_662_mvl/mxv
	.byte	W02
	.byte		        62*mus_qz_662_mvl/mxv
	.byte	W01
	.byte		        59*mus_qz_662_mvl/mxv
	.byte	W01
	.byte		        55*mus_qz_662_mvl/mxv
	.byte	W02
	.byte		        52*mus_qz_662_mvl/mxv
	.byte	W01
	.byte		        49*mus_qz_662_mvl/mxv
	.byte	W02
	.byte		        45*mus_qz_662_mvl/mxv
	.byte	W01
	.byte		        42*mus_qz_662_mvl/mxv
	.byte	W02
	.byte		        40*mus_qz_662_mvl/mxv
	.byte	W01
	.byte		        36*mus_qz_662_mvl/mxv
	.byte	W02
	.byte		        33*mus_qz_662_mvl/mxv
	.byte	W02
	.byte		        29*mus_qz_662_mvl/mxv
	.byte	W01
	.byte		        27*mus_qz_662_mvl/mxv
	.byte	W02
	.byte		        25*mus_qz_662_mvl/mxv
	.byte	W01
	.byte		        22*mus_qz_662_mvl/mxv
	.byte	W02
	.byte		        20*mus_qz_662_mvl/mxv
	.byte	W01
	.byte		        18*mus_qz_662_mvl/mxv
	.byte	W02
	.byte		        16*mus_qz_662_mvl/mxv
	.byte	W01
	.byte		        14*mus_qz_662_mvl/mxv
	.byte	W02
	.byte		        12*mus_qz_662_mvl/mxv
	.byte	W01
	.byte		        9*mus_qz_662_mvl/mxv
	.byte	W02
	.byte		        8*mus_qz_662_mvl/mxv
	.byte	W01
	.byte		        7*mus_qz_662_mvl/mxv
	.byte	W02
	.byte		        5*mus_qz_662_mvl/mxv
	.byte	W02
	.byte		        3*mus_qz_662_mvl/mxv
	.byte	W01
	.byte		        3*mus_qz_662_mvl/mxv
	.byte	W01
	.byte		        2*mus_qz_662_mvl/mxv
	.byte	W02
	.byte		        1*mus_qz_662_mvl/mxv
	.byte	W01
	.byte		        0*mus_qz_662_mvl/mxv
	.byte	W02
	.byte		        0*mus_qz_662_mvl/mxv
	.byte	W03
mus_qz_662_5_B1:
	.byte		VOL   , 61*mus_qz_662_mvl/mxv
	.byte		N06   , Bn5 , v088
	.byte	W24
	.byte		        Ds6 , v076
	.byte	W24
	.byte		        En6 , v088
	.byte	W24
	.byte		        Ds6 , v076
	.byte	W21
	.byte		N01   , Bn5 , v060
	.byte	W01
	.byte		        Cn6 , v048
	.byte	W02
mus_qz_662_5_000:
	.byte		N10   , Cs6 , v088
	.byte	W12
	.byte		        Cn6 , v076
	.byte	W12
	.byte		        Bn5 , v080
	.byte	W12
	.byte		        An5 , v072
	.byte	W12
	.byte		        Gs5 , v084
	.byte	W12
	.byte		        An5 , v072
	.byte	W12
	.byte		        As5 , v084
	.byte	W12
	.byte		        Cs6 , v072
	.byte	W12
	.byte	PEND
	.byte		N04   , Bn5 , v088
	.byte	W24
	.byte		        Cs6 , v072
	.byte	W24
	.byte		        Bn5 , v084
	.byte	W24
	.byte		        An5 , v072
	.byte	W21
	.byte		N01   , En5 , v060
	.byte	W01
	.byte		        Fs5 , v048
	.byte	W02
mus_qz_662_5_001:
	.byte		N10   , Gs5 , v084
	.byte	W12
	.byte		        Fs5 , v072
	.byte	W12
	.byte		        En5 , v076
	.byte	W12
	.byte		        Ds5 , v068
	.byte	W12
	.byte		        En5 , v080
	.byte	W12
	.byte		        Fs5 , v072
	.byte	W12
	.byte		        Gs5 , v080
	.byte	W12
	.byte		        An5 , v068
	.byte	W12
	.byte	PEND
	.byte		N07   , Bn5 , v088
	.byte	W24
	.byte		        Ds6 , v076
	.byte	W24
	.byte		        En6 , v088
	.byte	W24
	.byte		        Ds6 , v076
	.byte	W21
	.byte		N01   , Bn5 , v044
	.byte	W01
	.byte		        Cn6 , v032
	.byte	W02
	.byte	PATT
	 ptrvalue	mus_qz_662_5_000
	.byte		N06   , Bn5 , v088
	.byte	W24
	.byte		        Cs6 , v072
	.byte	W24
	.byte		        Bn5 , v084
	.byte	W24
	.byte		        An5 , v072
	.byte	W21
	.byte		N01   , En5 , v044
	.byte	W01
	.byte		        Fs5 , v032
	.byte	W02
	.byte	PATT
	 ptrvalue	mus_qz_662_5_001
	.byte	GOTO
	 ptrvalue	mus_qz_662_5_B1
	.byte	FINE

/********************** Track  6 **********************/

mus_qz_662_6:
	.byte		KEYSH , mus_qz_662_key+0
	.byte		VOICE , 33
	.byte		VOL   , 20*mus_qz_662_mvl/mxv
	.byte		PAN   , c_v-54
	.byte		VOL   , 16*mus_qz_662_mvl/mxv
	.byte		PRIO  , 40
	.byte	W96
mus_qz_662_6_B1:
	.byte	W12
	.byte		N06   , Gs3 , v088
	.byte	W24
	.byte		        Bn3 , v076
	.byte	W24
	.byte		        Cs4 , v088
	.byte	W24
	.byte		        Bn3 , v076
	.byte	W12
	.byte	W12
	.byte		N10   , Cs4 , v088
	.byte	W12
	.byte		        Cn4 , v076
	.byte	W12
	.byte		        Bn3 , v080
	.byte	W12
	.byte		        An3 , v072
	.byte	W12
	.byte		        Gs3 , v084
	.byte	W12
	.byte		        An3 , v072
	.byte	W12
	.byte		        As3 , v084
	.byte	W12
	.byte		        Cs4 , v072
	.byte	W12
	.byte		N04   , Gs3 , v088
	.byte	W24
	.byte		        An3 , v072
	.byte	W24
	.byte		        Bn3 , v084
	.byte	W24
	.byte		        An3 , v072
	.byte	W12
	.byte	W12
	.byte		N10   , Gs3 , v084
	.byte	W12
	.byte		        Fs3 , v072
	.byte	W12
	.byte		        En3 , v076
	.byte	W12
	.byte		        Ds3 , v068
	.byte	W12
	.byte		        En3 , v080
	.byte	W12
	.byte		        Fs3 , v072
	.byte	W12
	.byte		        Gs3 , v080
	.byte	W12
	.byte		        An3 , v068
	.byte	W12
	.byte		N06   , Gs3 , v088
	.byte	W24
	.byte		        Bn3 , v076
	.byte	W24
	.byte		        Cs4 , v088
	.byte	W24
	.byte		        Bn3 , v076
	.byte	W12
	.byte	W12
	.byte		N10   , En4
	.byte	W12
	.byte		        Ds4 , v064
	.byte	W12
	.byte		        Dn4 , v072
	.byte	W12
	.byte		        Cs4 , v064
	.byte	W12
	.byte		        Cn4 , v072
	.byte	W12
	.byte		        Cs4 , v064
	.byte	W12
	.byte		        Ds4 , v072
	.byte	W12
	.byte		        Fs4 , v060
	.byte	W12
	.byte		N04   , En4 , v088
	.byte	W24
	.byte		        Fs4 , v072
	.byte	W24
	.byte		        En4 , v084
	.byte	W24
	.byte		        Ds4 , v072
	.byte	W12
	.byte	W12
	.byte		N10   , Bn3 , v080
	.byte	W12
	.byte		        An3 , v068
	.byte	W12
	.byte		        Gs3 , v076
	.byte	W12
	.byte		        Fs3 , v068
	.byte	W12
	.byte		        Gs3 , v080
	.byte	W12
	.byte		        An3 , v068
	.byte	W12
	.byte		        Bn3 , v076
	.byte	W12
	.byte	GOTO
	 ptrvalue	mus_qz_662_6_B1
	.byte	FINE

/********************** Track  7 **********************/

mus_qz_662_7:
	.byte		KEYSH , mus_qz_662_key+0
	.byte		VOICE , 0
	.byte		VOL   , 20*mus_qz_662_mvl/mxv
	.byte		PAN   , c_v+56
	.byte		VOL   , 16*mus_qz_662_mvl/mxv
	.byte		PRIO  , 38
	.byte	W12
	.byte		N04   , Bn4 , v116
	.byte	W12
	.byte		N32   , Bn5 , v120 , gtp3
	.byte	W36
	.byte		N10   , En5 , v108
	.byte	W12
	.byte		        Ds5 , v092
	.byte	W12
	.byte		        Cs5 , v104
	.byte	W12
mus_qz_662_7_B1:
	.byte		N10   , Cn5 , v088
	.byte	W12
	.byte		        Bn4 , v112
	.byte	W24
	.byte		        Ds5 , v100
	.byte	W24
	.byte		        En5 , v112
	.byte	W24
	.byte		        Ds5 , v100
	.byte	W12
mus_qz_662_7_000:
	.byte	W12
	.byte		N10   , Cs5 , v112
	.byte	W12
	.byte		        Cn5 , v100
	.byte	W12
	.byte		        Bn4 , v108
	.byte	W12
	.byte		        An4 , v096
	.byte	W12
	.byte		        Gs4 , v108
	.byte	W12
	.byte		        An4 , v096
	.byte	W12
	.byte		        As4 , v108
	.byte	W12
	.byte	PEND
mus_qz_662_7_001:
	.byte		N10   , Cs5 , v096
	.byte	W12
	.byte		        Bn4 , v112
	.byte	W24
	.byte		        Cs5 , v096
	.byte	W24
	.byte		        Bn4 , v108
	.byte	W24
	.byte		        An4 , v096
	.byte	W12
	.byte	PEND
mus_qz_662_7_002:
	.byte	W12
	.byte		N10   , Gs4 , v108
	.byte	W12
	.byte		        Fs4 , v096
	.byte	W12
	.byte		        En4 , v100
	.byte	W12
	.byte		        Ds4 , v092
	.byte	W12
	.byte		        En4 , v108
	.byte	W12
	.byte		        Fs4 , v092
	.byte	W12
	.byte		        Gs4 , v104
	.byte	W12
	.byte	PEND
	.byte		        An4 , v092
	.byte		N10   , Cn5 , v088
	.byte	W12
	.byte		        Bn4 , v112
	.byte	W24
	.byte		        Ds5 , v100
	.byte	W24
	.byte		        En5 , v112
	.byte	W24
	.byte		        Ds5 , v100
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_662_7_000
	.byte	PATT
	 ptrvalue	mus_qz_662_7_001
	.byte	PATT
	 ptrvalue	mus_qz_662_7_002
	.byte	GOTO
	 ptrvalue	mus_qz_662_7_B1
	.byte	FINE

/********************** Track  8 **********************/

mus_qz_662_8:
	.byte		KEYSH , mus_qz_662_key+0
	.byte		VOICE , 6
	.byte		VOL   , 76*mus_qz_662_mvl/mxv
	.byte		PAN   , c_v-46
	.byte		VOL   , 59*mus_qz_662_mvl/mxv
	.byte		PRIO  , 55
	.byte	W96
mus_qz_662_8_B1:
	.byte		N05   , En3 , v080
	.byte		N05   , Gs3 , v092
	.byte	W24
	.byte		        Gs3 , v076
	.byte		N05   , Bn3 , v088
	.byte	W24
	.byte		        Fs3 , v084
	.byte		N05   , An3 , v096
	.byte	W24
	.byte		        Gs3 , v072
	.byte		N05   , Bn3 , v088
	.byte	W12
	.byte		        Fs3 , v068
	.byte		N05   , An3 , v080
	.byte	W12
	.byte	PEND
mus_qz_662_8_000:
	.byte		N05   , En3 , v080
	.byte		N05   , Gs3 , v092
	.byte	W24
	.byte		        Gs3 , v072
	.byte		N05   , Bn3 , v088
	.byte	W24
	.byte		        Fs3 , v080
	.byte		N05   , An3 , v096
	.byte	W24
	.byte		        Fs3 , v072
	.byte		N05   , An3 , v088
	.byte	W12
	.byte		        Gs3 , v068
	.byte		N05   , Bn3 , v080
	.byte	W12
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_662_8_000
mus_qz_662_8_001:
	.byte		N05   , En3 , v080
	.byte		N05   , Gs3 , v092
	.byte	W24
	.byte		        Gs3 , v072
	.byte		N05   , Bn3 , v088
	.byte	W24
	.byte		        Fs3 , v072
	.byte		N05   , An3 , v088
	.byte	W24
	.byte		        Gs3 , v072
	.byte		N05   , Bn3 , v088
	.byte	W12
	.byte		        Fs3 , v068
	.byte		N05   , An3 , v080
	.byte	W12
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_662_8_B1
	.byte		N05   , En3 , v080
	.byte		N05   , An3 , v092
	.byte	W24
	.byte		        An3 , v072
	.byte		N05   , Dn4 , v088
	.byte	W24
	.byte		        Ds3 , v080
	.byte		N05   , An3 , v096
	.byte	W24
	.byte		        Ds3 , v072
	.byte		N05   , Fs3 , v088
	.byte	W12
	.byte		        Gs3 , v068
	.byte		N05   , Cs4 , v080
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_662_8_000
	.byte	PATT
	 ptrvalue	mus_qz_662_8_001
	.byte	GOTO
	 ptrvalue	mus_qz_662_8_B1
	.byte	FINE

/********************** Track  9 **********************/

mus_qz_662_9:
	.byte		KEYSH , mus_qz_662_key+0
	.byte		VOICE , 26
	.byte		VOL   , 82*mus_qz_662_mvl/mxv
	.byte		PAN   , c_v-24
	.byte		VOL   , 64*mus_qz_662_mvl/mxv
	.byte		PRIO  , 52
	.byte	W96
mus_qz_662_9_B1:
	.byte	W12
	.byte		N05   , En3 , v076
	.byte	W12
	.byte		        Gs3 , v096
	.byte	W12
	.byte		        Bn3 , v080
	.byte	W24
	.byte		        Gs3 , v092
	.byte	W12
	.byte		        Bn3 , v100
	.byte	W12
	.byte		        En4 , v092
	.byte	W12
	.byte	PEND
mus_qz_662_9_000:
	.byte	W12
	.byte		N05   , Ds4 , v092
	.byte	W12
	.byte		        En4 , v100
	.byte	W12
	.byte		        Fs4 , v092
	.byte	W12
	.byte		        En4 , v100
	.byte	W12
	.byte		        Ds4 , v092
	.byte	W12
	.byte		        Cs4 , v100
	.byte	W12
	.byte		        Cn4 , v084
	.byte	W12
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_662_9_B1
mus_qz_662_9_001:
	.byte	W12
	.byte		N05   , Cs4 , v092
	.byte	W12
	.byte		        Ds4 , v100
	.byte	W12
	.byte		        Cs4 , v092
	.byte	W12
	.byte		        Bn3 , v100
	.byte	W12
	.byte		        An3 , v092
	.byte	W12
	.byte		        Gs3 , v100
	.byte	W12
	.byte		        Fs3 , v084
	.byte	W12
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_662_9_B1
	.byte	PATT
	 ptrvalue	mus_qz_662_9_000
	.byte	PATT
	 ptrvalue	mus_qz_662_9_B1
	.byte	PATT
	 ptrvalue	mus_qz_662_9_001
	.byte	GOTO
	 ptrvalue	mus_qz_662_9_B1
	.byte	FINE

/********************** Track 10 **********************/

mus_qz_662_10:
	.byte		KEYSH , mus_qz_662_key+0
	.byte		VOICE , 35
	.byte		VOL   , 20*mus_qz_662_mvl/mxv
	.byte		PAN   , c_v-54
	.byte		VOL   , 16*mus_qz_662_mvl/mxv
	.byte		PRIO  , 38
	.byte	W12
	.byte		N04   , Bn4 , v100
	.byte	W12
	.byte		N80   , Bn5 , v100 , gtp2
	.byte	W36
	.byte		VOL   , 20*mus_qz_662_mvl/mxv
	.byte	W01
	.byte		        19*mus_qz_662_mvl/mxv
	.byte	W02
	.byte		        17*mus_qz_662_mvl/mxv
	.byte	W01
	.byte		        17*mus_qz_662_mvl/mxv
	.byte	W01
	.byte		        16*mus_qz_662_mvl/mxv
	.byte	W02
	.byte		        15*mus_qz_662_mvl/mxv
	.byte	W01
	.byte		        14*mus_qz_662_mvl/mxv
	.byte	W02
	.byte		        12*mus_qz_662_mvl/mxv
	.byte	W01
	.byte		        12*mus_qz_662_mvl/mxv
	.byte	W02
	.byte		        11*mus_qz_662_mvl/mxv
	.byte	W01
	.byte		        10*mus_qz_662_mvl/mxv
	.byte	W02
	.byte		        9*mus_qz_662_mvl/mxv
	.byte	W02
	.byte		        8*mus_qz_662_mvl/mxv
	.byte	W01
	.byte		        7*mus_qz_662_mvl/mxv
	.byte	W02
	.byte		        7*mus_qz_662_mvl/mxv
	.byte	W01
	.byte		        6*mus_qz_662_mvl/mxv
	.byte	W02
	.byte		        6*mus_qz_662_mvl/mxv
	.byte	W01
	.byte		        5*mus_qz_662_mvl/mxv
	.byte	W02
	.byte		        4*mus_qz_662_mvl/mxv
	.byte	W01
	.byte		        3*mus_qz_662_mvl/mxv
	.byte	W02
	.byte		        3*mus_qz_662_mvl/mxv
	.byte	W01
	.byte		        3*mus_qz_662_mvl/mxv
	.byte	W02
	.byte		        2*mus_qz_662_mvl/mxv
	.byte	W01
	.byte		        1*mus_qz_662_mvl/mxv
	.byte	W02
mus_qz_662_10_B1:
	.byte		VOL   , 1*mus_qz_662_mvl/mxv
	.byte	W02
	.byte		        1*mus_qz_662_mvl/mxv
	.byte	W01
	.byte		        0*mus_qz_662_mvl/mxv
	.byte	W01
	.byte		        0*mus_qz_662_mvl/mxv
	.byte	W02
	.byte		        0*mus_qz_662_mvl/mxv
	.byte	W01
	.byte		        0*mus_qz_662_mvl/mxv
	.byte	W02
	.byte		        0*mus_qz_662_mvl/mxv
	.byte	W03
	.byte		        17*mus_qz_662_mvl/mxv
	.byte		N06   , Bn5 , v088
	.byte	W24
	.byte		        Ds6 , v076
	.byte	W24
	.byte		        En6 , v088
	.byte	W24
	.byte		        Ds6 , v076
	.byte	W12
	.byte	W09
	.byte		N01   , Bn5 , v060
	.byte	W01
	.byte		        Cn6 , v048
	.byte	W02
	.byte		N10   , Cs6 , v088
	.byte	W12
	.byte		        Cn6 , v076
	.byte	W12
	.byte		        Bn5 , v080
	.byte	W12
	.byte		        An5 , v072
	.byte	W12
	.byte		        Gs5 , v084
	.byte	W12
	.byte		        An5 , v072
	.byte	W12
	.byte		        As5 , v084
	.byte	W12
	.byte		        Cs6 , v072
	.byte	W12
	.byte		N04   , Bn5 , v088
	.byte	W24
	.byte		        Cs6 , v072
	.byte	W24
	.byte		        Bn5 , v084
	.byte	W24
	.byte		        An5 , v072
	.byte	W12
	.byte	W09
	.byte		N01   , En5 , v060
	.byte	W01
	.byte		        Fs5 , v048
	.byte	W02
	.byte		N10   , Gs5 , v084
	.byte	W12
	.byte		        Fs5 , v072
	.byte	W12
	.byte		        En5 , v076
	.byte	W12
	.byte		        Ds5 , v068
	.byte	W12
	.byte		        En5 , v080
	.byte	W12
	.byte		        Fs5 , v072
	.byte	W12
	.byte		        Gs5 , v080
	.byte	W12
	.byte		        An5 , v068
	.byte	W12
	.byte		N07   , Bn5 , v088
	.byte	W24
	.byte		        Ds6 , v076
	.byte	W24
	.byte		        En6 , v088
	.byte	W24
	.byte		        Ds6 , v076
	.byte	W12
	.byte	W09
	.byte		N01   , Bn5 , v044
	.byte	W01
	.byte		        Cn6 , v032
	.byte	W02
	.byte		N10   , Cs6 , v088
	.byte	W12
	.byte		        Cn6 , v076
	.byte	W12
	.byte		        Bn5 , v080
	.byte	W12
	.byte		        An5 , v072
	.byte	W12
	.byte		        Gs5 , v084
	.byte	W12
	.byte		        An5 , v072
	.byte	W12
	.byte		        As5 , v084
	.byte	W12
	.byte		        Cs6 , v072
	.byte	W12
	.byte		N06   , Bn5 , v088
	.byte	W24
	.byte		        Cs6 , v072
	.byte	W24
	.byte		        Bn5 , v084
	.byte	W24
	.byte		        An5 , v072
	.byte	W12
	.byte	W09
	.byte		N01   , En5 , v044
	.byte	W01
	.byte		        Fs5 , v032
	.byte	W02
	.byte		N10   , Gs5 , v084
	.byte	W12
	.byte		        Fs5 , v072
	.byte	W12
	.byte		        En5 , v076
	.byte	W12
	.byte		        Ds5 , v068
	.byte	W12
	.byte		        En5 , v080
	.byte	W12
	.byte		        Fs5 , v072
	.byte	W12
	.byte		        Gs5 , v080
	.byte	W12
	.byte	GOTO
	 ptrvalue	mus_qz_662_10_B1
	.byte	FINE

/******************************************************/
	.p2align	2

mus_qz_662:
	.byte	10	/* NumTrks */
	.byte	0	/* NumBlks */
	.byte	mus_qz_662_pri	/* Priority */
	.byte	mus_qz_662_rev	/* Reverb. */
space64 4
	ptrvalue	mus_qz_662_grp

	ptrvalue	mus_qz_662_1
	ptrvalue	mus_qz_662_2
	ptrvalue	mus_qz_662_3
	ptrvalue	mus_qz_662_4
	ptrvalue	mus_qz_662_5
	ptrvalue	mus_qz_662_6
	ptrvalue	mus_qz_662_7
	ptrvalue	mus_qz_662_8
	ptrvalue	mus_qz_662_9
	ptrvalue	mus_qz_662_10

	.end
