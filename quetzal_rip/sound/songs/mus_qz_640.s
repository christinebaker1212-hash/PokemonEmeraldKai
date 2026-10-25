	.include "asm/macros/bit_width.inc"

	.include "MPlayDef.s"

/* Moemon Quetzal song 640, disassembled from the ROM by tools/quetzal_rip/songs.py.
   "org" comments give the ROM address of the bytes that follow. */

	.equ	mus_qz_640_grp, voicegroup_qz_songs
	.equ	mus_qz_640_pri, 0
	.equ	mus_qz_640_rev, reverb_set+0
	.equ	mus_qz_640_mvl, 127
	.equ	mus_qz_640_key, 0
	.equ	mus_qz_640_tbs, 1
	.equ	mus_qz_640_exg, 1
	.equ	mus_qz_640_cmp, 1

	.section .rodata
	.global	mus_qz_640
	.p2align	2
/* org 0x0968D998 */

/********************** Track  1 **********************/

mus_qz_640_1:
	.byte		KEYSH , mus_qz_640_key+0
mus_qz_640_1_B1:
	.byte		TEMPO , 130*mus_qz_640_tbs/2
	.byte		VOICE , 23
	.byte		VOL   , 68*mus_qz_640_mvl/mxv
	.byte		PAN   , c_v-42
	.byte		VOL   , 53*mus_qz_640_mvl/mxv
	.byte		PRIO  , 60
	.byte		N05   , Cn3 , v100
	.byte		N05   , En3 , v108
	.byte	W12
	.byte		        Cn3 , v096
	.byte		N05   , En3 , v100
	.byte	W18
	.byte		        Bn2 , v088
	.byte		N05   , Fs3 , v092
	.byte	W18
	.byte		N23   , Cn3 , v096
	.byte		N23   , En3 , v100
	.byte	W30
	.byte		N17   , Cn3 , v096
	.byte		N17   , Fs3 , v100
	.byte	W18
mus_qz_640_1_000:
	.byte		N05   , Cn3 , v100
	.byte		N05   , En3 , v108
	.byte	W12
	.byte		        Cn3 , v096
	.byte		N05   , En3 , v100
	.byte	W18
	.byte		        Bn2 , v088
	.byte		N05   , Fs3 , v092
	.byte	W18
	.byte		N23   , Cn3 , v096
	.byte		N23   , En3 , v100
	.byte	W24
	.byte		        Cn3 , v096
	.byte		N23   , Fs3 , v100
	.byte	W24
	.byte	PEND
	.byte		N05   , Cn3
	.byte		N05   , En3 , v108
	.byte	W12
	.byte		        Cn3 , v096
	.byte		N05   , En3 , v100
	.byte	W18
	.byte		        Bn2 , v088
	.byte		N05   , Fs3 , v092
	.byte	W18
	.byte		N23   , Cn3 , v096
	.byte		N23   , En3 , v100
	.byte	W30
	.byte		N17   , Cn3 , v096
	.byte		N17   , Fs3 , v100
	.byte	W18
	.byte	PATT
	 ptrvalue	mus_qz_640_1_000
	.byte		N05   , Cn3 , v100
	.byte		N05   , An3 , v108
	.byte	W12
	.byte		        Cn3 , v096
	.byte		N05   , An3 , v100
	.byte	W18
	.byte		        Bn2 , v088
	.byte		N05   , Fs3 , v092
	.byte	W18
	.byte		N23   , Cn3 , v096
	.byte		N23   , En3 , v100
	.byte	W24
	.byte		        Cn3 , v096
	.byte		N23   , Fs3 , v100
	.byte	W24
	.byte		N05   , Cn3 , v096
	.byte		N05   , En3 , v100
	.byte	W12
	.byte		        Cn3 , v096
	.byte		N05   , En3 , v100
	.byte	W18
	.byte		        Bn2 , v088
	.byte		N05   , Fs3 , v092
	.byte	W18
	.byte		N23   , Cn3 , v096
	.byte		N23   , En3 , v100
	.byte	W30
	.byte		N17   , Cn3 , v092
	.byte		N17   , Fs3 , v100
	.byte	W18
	.byte		N05   , Dn3 , v104
	.byte		N05   , An3 , v108
	.byte	W12
	.byte		        Dn3 , v096
	.byte		N05   , An3 , v100
	.byte	W18
	.byte		        Cs3 , v088
	.byte		N05   , Gs3 , v092
	.byte	W18
	.byte		N23   , Dn3 , v096
	.byte		N23   , Fn3 , v100
	.byte	W24
	.byte		        Dn3 , v096
	.byte		N23   , Gs3 , v100
	.byte	W24
	.byte		N05   , Dn3 , v104
	.byte		N05   , Fn3 , v108
	.byte	W12
	.byte		        Dn3 , v096
	.byte		N05   , Fn3 , v100
	.byte	W18
	.byte		        Cs3 , v088
	.byte		N05   , Gs3 , v092
	.byte	W18
	.byte		N23   , Dn3 , v096
	.byte		N23   , Fn3 , v100
	.byte	W30
	.byte		N17   , Dn3 , v096
	.byte		N17   , Gs3 , v100
	.byte	W18
mus_qz_640_1_001:
	.byte		N05   , Cn3 , v104
	.byte		N05   , En3 , v108
	.byte	W12
	.byte		        Cn3 , v096
	.byte		N05   , En3 , v100
	.byte	W18
	.byte		        Bn2 , v088
	.byte		N05   , Fs3 , v092
	.byte	W18
	.byte		N23   , Cn3 , v096
	.byte		N23   , En3 , v100
	.byte	W24
	.byte		        Cn3 , v096
	.byte		N23   , Fs3 , v100
	.byte	W24
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_640_1_001
mus_qz_640_1_002:
	.byte		N05   , Cn3 , v116
	.byte		N05   , En3 , v108
	.byte	W12
	.byte		        Cn3 , v112
	.byte		N05   , En3 , v100
	.byte	W18
	.byte		        Bn2 , v104
	.byte		N05   , Fs3 , v092
	.byte	W18
	.byte		N23   , Cn3 , v112
	.byte		N23   , En3 , v100
	.byte	W30
	.byte		N17   , Cn3 , v112
	.byte		N17   , Fs3 , v100
	.byte	W18
	.byte	PEND
mus_qz_640_1_003:
	.byte		N05   , Cn3 , v116
	.byte		N05   , En3 , v108
	.byte	W12
	.byte		        Cn3 , v112
	.byte		N05   , En3 , v100
	.byte	W18
	.byte		        Bn2 , v104
	.byte		N05   , Fs3 , v092
	.byte	W18
	.byte		N23   , Cn3 , v112
	.byte		N23   , En3 , v100
	.byte	W24
	.byte		        Cn3 , v112
	.byte		N23   , Fs3 , v100
	.byte	W24
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_640_1_002
	.byte	PATT
	 ptrvalue	mus_qz_640_1_003
	.byte		N05   , Cn3 , v116
	.byte		N05   , An3 , v108
	.byte	W12
	.byte		        Cn3 , v112
	.byte		N05   , An3 , v100
	.byte	W18
	.byte		        Bn2 , v104
	.byte		N05   , Fs3 , v092
	.byte	W18
	.byte		N23   , Cn3 , v112
	.byte		N23   , En3 , v100
	.byte	W24
	.byte		        Cn3 , v112
	.byte		N23   , Fs3 , v100
	.byte	W24
	.byte		N05   , Cn3 , v112
	.byte		N05   , En3 , v100
	.byte	W12
	.byte		        Cn3 , v112
	.byte		N05   , En3 , v100
	.byte	W18
	.byte		        Bn2 , v104
	.byte		N05   , Fs3 , v092
	.byte	W18
	.byte		N23   , Cn3 , v112
	.byte		N23   , En3 , v100
	.byte	W30
	.byte		N17   , Cn3 , v112
	.byte		N17   , Fs3 , v100
	.byte	W18
	.byte		N05   , Dn3 , v120
	.byte		N05   , An3 , v108
	.byte	W12
	.byte		        Dn3 , v112
	.byte		N05   , An3 , v100
	.byte	W18
	.byte		        Cs3 , v104
	.byte		N05   , Gs3 , v092
	.byte	W18
	.byte		N23   , Dn3 , v112
	.byte		N23   , Fn3 , v100
	.byte	W24
	.byte		        Dn3 , v112
	.byte		N23   , Gs3 , v100
	.byte	W24
	.byte		N05   , Dn3 , v120
	.byte		N05   , Fn3 , v108
	.byte	W12
	.byte		        Dn3 , v112
	.byte		N05   , Fn3 , v100
	.byte	W18
	.byte		        Cs3 , v104
	.byte		N05   , Gs3 , v092
	.byte	W18
	.byte		N23   , Dn3 , v112
	.byte		N23   , Fn3 , v100
	.byte	W30
	.byte		N17   , Dn3 , v112
	.byte		N17   , Gs3 , v100
	.byte	W18
mus_qz_640_1_004:
	.byte		N05   , Cn3 , v120
	.byte		N05   , En3 , v108
	.byte	W12
	.byte		        Cn3 , v112
	.byte		N05   , En3 , v100
	.byte	W18
	.byte		        Bn2 , v104
	.byte		N05   , Fs3 , v092
	.byte	W18
	.byte		N23   , Cn3 , v112
	.byte		N23   , En3 , v100
	.byte	W24
	.byte		        Cn3 , v112
	.byte		N23   , Fs3 , v100
	.byte	W24
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_640_1_004
	.byte		N32   , Cs3 , v120
	.byte		N32   , Cs4 , v116
	.byte	W36
	.byte		N05   , Cs3 , v104
	.byte		N05   , Cs4 , v100
	.byte	W06
	.byte		N02   , Ds3 , v092
	.byte		N02   , Ds4
	.byte	W06
	.byte		N04   , Fn3 , v124
	.byte		N04   , Fn4 , v120
	.byte	W12
	.byte		        Ds3 , v112
	.byte		N04   , Ds4 , v108
	.byte	W12
	.byte		        Cs3 , v124
	.byte		N04   , Cs4 , v120
	.byte	W12
	.byte		        Fn3 , v112
	.byte		N04   , Fn4
	.byte	W12
	.byte		N32   , Ds3 , v124
	.byte		N32   , Ds4 , v120
	.byte	W36
	.byte		N05   , Ds3 , v108
	.byte		N05   , Ds4
	.byte	W06
	.byte		N02   , Fn3 , v100
	.byte		N02   , Fn4
	.byte	W06
	.byte		N04   , Gn3 , v120
	.byte		N04   , Gn4 , v116
	.byte	W12
	.byte		        Fn3 , v108
	.byte		N04   , Fn4
	.byte	W12
	.byte		        Ds3 , v120
	.byte		N04   , Ds4 , v116
	.byte	W12
	.byte		        Gn3 , v108
	.byte		N04   , Gn4 , v104
	.byte	W12
	.byte		N32   , En3 , v120
	.byte		N32   , En4 , v116
	.byte	W36
	.byte		N04   , En3 , v112
	.byte		N04   , En4 , v108
	.byte	W06
	.byte		        Fs3 , v100
	.byte		N04   , Fs4
	.byte	W06
	.byte		        Gs3 , v120
	.byte		N04   , Gs4 , v116
	.byte	W12
	.byte		        Fs3 , v108
	.byte		N04   , Fs4
	.byte	W12
	.byte		        En3 , v120
	.byte		N04   , En4 , v116
	.byte	W12
	.byte		        Gs3 , v108
	.byte		N04   , Gs4 , v104
	.byte	W12
	.byte		N32   , Fs3 , v120
	.byte		N32   , Fs4 , v116
	.byte	W36
	.byte		N04   , Fs3 , v108
	.byte		N04   , Fs4
	.byte	W06
	.byte		        Gs3 , v096
	.byte		N04   , Gs4 , v092
	.byte	W06
	.byte		        Fs3 , v116
	.byte		N04   , Fs4 , v112
	.byte	W12
	.byte		        Fn3 , v104
	.byte		N04   , Fn4 , v100
	.byte	W12
	.byte		        En3 , v112
	.byte		N04   , En4
	.byte	W12
	.byte		        Ds3 , v108
	.byte		N04   , Ds4 , v104
	.byte	W12
	.byte	GOTO
	 ptrvalue	mus_qz_640_1_B1
	.byte	FINE

/********************** Track  2 **********************/

mus_qz_640_2:
	.byte		KEYSH , mus_qz_640_key+0
mus_qz_640_2_B1:
	.byte		VOICE , 25
	.byte		VOL   , 84*mus_qz_640_mvl/mxv
	.byte		PAN   , c_v+51
	.byte		VOL   , 76*mus_qz_640_mvl/mxv
	.byte		PRIO  , 62
	.byte		N05   , Cn1 , v108
	.byte	W06
	.byte		        Bn0 , v092
	.byte	W06
	.byte		        Cn1 , v112
	.byte	W12
	.byte		N06   , Ds1 , v100
	.byte	W24
	.byte		        Cn1 , v112
	.byte	W24
	.byte		        Cn1 , v104
	.byte	W24
mus_qz_640_2_000:
	.byte		N05   , Cn1 , v108
	.byte	W24
	.byte		N06   , Ds1 , v100
	.byte	W24
	.byte		        Cn1 , v108
	.byte	W24
	.byte		        Cn1 , v116
	.byte	W24
	.byte	PEND
mus_qz_640_2_001:
	.byte		N05   , Cn1 , v116
	.byte	W06
	.byte		        Bn0 , v092
	.byte	W06
	.byte		        Cn1 , v112
	.byte	W12
	.byte		N06   , Ds1 , v100
	.byte	W24
	.byte		        Cn1 , v112
	.byte	W24
	.byte		        Cn1 , v104
	.byte	W24
	.byte	PEND
mus_qz_640_2_002:
	.byte		N05   , Cn1 , v112
	.byte	W24
	.byte		N06   , Ds1 , v100
	.byte	W24
	.byte		        Cn1 , v116
	.byte	W24
	.byte		        Cn1 , v108
	.byte	W24
	.byte	PEND
mus_qz_640_2_003:
	.byte		N05   , Cn1 , v108
	.byte	W06
	.byte		        Bn0 , v092
	.byte	W06
	.byte		        Cn1 , v112
	.byte	W12
	.byte		N06   , Ds1 , v100
	.byte	W24
	.byte		        Cn1 , v116
	.byte	W24
	.byte		        Cn1 , v104
	.byte	W24
	.byte	PEND
mus_qz_640_2_004:
	.byte		N05   , Cn1 , v108
	.byte	W06
	.byte		        Bn0 , v092
	.byte	W06
	.byte		        Cn1 , v112
	.byte	W12
	.byte		N06   , Ds1 , v100
	.byte	W24
	.byte		        Cn1 , v112
	.byte	W24
	.byte		        Cn1 , v108
	.byte	W24
	.byte	PEND
mus_qz_640_2_005:
	.byte		N05   , Dn1 , v116
	.byte	W06
	.byte		        Cs1 , v092
	.byte	W06
	.byte		        Dn1 , v112
	.byte	W12
	.byte		N06   , Fn1 , v100
	.byte	W24
	.byte		        Dn1 , v108
	.byte	W24
	.byte		        Dn1 , v100
	.byte	W24
	.byte	PEND
mus_qz_640_2_006:
	.byte		N05   , Dn1 , v100
	.byte	W24
	.byte		N06   , Fn1
	.byte	W24
	.byte		        Dn1 , v108
	.byte	W24
	.byte		        Dn1 , v100
	.byte	W24
	.byte	PEND
mus_qz_640_2_007:
	.byte		N05   , Cn1 , v108
	.byte	W06
	.byte		        Bn0 , v092
	.byte	W06
	.byte		        Cn1 , v112
	.byte	W12
	.byte		N06   , Ds1 , v100
	.byte	W24
	.byte		        Cn1 , v116
	.byte	W24
	.byte		        Cn1 , v108
	.byte	W24
	.byte	PEND
mus_qz_640_2_008:
	.byte		N05   , Cn1 , v108
	.byte	W06
	.byte		        Bn0 , v092
	.byte	W06
	.byte		        Cn1 , v112
	.byte	W12
	.byte		N06   , Ds1 , v100
	.byte	W24
	.byte		        Cn1 , v112
	.byte	W24
	.byte		        Cn1 , v104
	.byte	W24
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_640_2_008
	.byte	PATT
	 ptrvalue	mus_qz_640_2_000
	.byte	PATT
	 ptrvalue	mus_qz_640_2_001
	.byte	PATT
	 ptrvalue	mus_qz_640_2_002
	.byte	PATT
	 ptrvalue	mus_qz_640_2_003
	.byte	PATT
	 ptrvalue	mus_qz_640_2_004
	.byte	PATT
	 ptrvalue	mus_qz_640_2_005
	.byte	PATT
	 ptrvalue	mus_qz_640_2_006
	.byte	PATT
	 ptrvalue	mus_qz_640_2_007
	.byte	PATT
	 ptrvalue	mus_qz_640_2_008
	.byte		N05   , As0 , v108
	.byte	W06
	.byte		        An0 , v092
	.byte	W06
	.byte		        As0 , v112
	.byte	W12
	.byte		N06   , Cs1 , v100
	.byte	W24
	.byte		        As0 , v116
	.byte	W24
	.byte		        As0 , v108
	.byte	W24
	.byte		N05   , Cn1
	.byte	W06
	.byte		        Bn0 , v092
	.byte	W06
	.byte		        Cn1 , v112
	.byte	W12
	.byte		N06   , Ds1 , v100
	.byte	W24
	.byte		        Cn1 , v112
	.byte	W24
	.byte		        Ds1 , v104
	.byte	W24
	.byte		N05   , Cs1 , v108
	.byte	W06
	.byte		        Cn1 , v092
	.byte	W06
	.byte		        Cs1 , v112
	.byte	W12
	.byte		N06   , En1 , v100
	.byte	W24
	.byte		        Cs1 , v116
	.byte	W24
	.byte		        Cs1 , v108
	.byte	W24
	.byte		N05   , Ds1
	.byte	W06
	.byte		        Dn1 , v092
	.byte	W06
	.byte		        Ds1 , v112
	.byte	W12
	.byte		N06   , Fs1 , v100
	.byte	W24
	.byte		        Ds1 , v112
	.byte	W12
	.byte		        Dn1 , v104
	.byte	W12
	.byte		        Cs1 , v116
	.byte	W12
	.byte		        Cn1 , v104
	.byte	W12
	.byte	GOTO
	 ptrvalue	mus_qz_640_2_B1
	.byte	FINE

/********************** Track  3 **********************/

mus_qz_640_3:
	.byte		KEYSH , mus_qz_640_key+0
mus_qz_640_3_B1:
	.byte		VOICE , 32
	.byte		VOL   , 84*mus_qz_640_mvl/mxv
	.byte		PAN   , c_v-19
	.byte		VOL   , 82*mus_qz_640_mvl/mxv
	.byte		PRIO  , 64
	.byte	W96
	.byte	W90
	.byte		N04   , Cn3 , v104
	.byte	W06
	.byte		N44   , En3 , v112 , gtp2
	.byte	W48
	.byte		N22   , Cn4 , v100
	.byte	W24
	.byte		        Bn3 , v096
	.byte	W24
	.byte		N84   , En3 , v104 , gtp2
	.byte	W56
	.byte	W01
	.byte		VOL   , 81*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        78*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        74*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        70*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        62*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        57*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        52*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        47*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        32*mus_qz_640_mvl/mxv
	.byte	W09
	.byte		        82*mus_qz_640_mvl/mxv
	.byte		N05   , Ds3
	.byte	W06
	.byte		N44   , En3 , v112 , gtp2
	.byte	W48
	.byte		N22   , Cn4 , v096
	.byte	W24
	.byte		        Bn3 , v088
	.byte	W24
	.byte		N88   , En4 , v108 , gtp1
	.byte	W72
	.byte		VOL   , 80*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        76*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        71*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        67*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        64*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        61*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        52*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        45*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        82*mus_qz_640_mvl/mxv
	.byte		N44   , Dn4 , v108 , gtp2
	.byte	W48
	.byte		N22   , Cs4 , v092
	.byte	W24
	.byte		        An3 , v084
	.byte	W24
	.byte		N44   , Cn4 , v108 , gtp1
	.byte	W48
	.byte		N22   , As3 , v096
	.byte	W24
	.byte		        Fn3 , v088
	.byte	W24
	.byte		N64   , An3 , v108 , gtp1
	.byte	W44
	.byte	W01
	.byte		VOL   , 78*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        73*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        68*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        64*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        59*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        55*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        50*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        47*mus_qz_640_mvl/mxv
	.byte	W06
	.byte		        82*mus_qz_640_mvl/mxv
	.byte		N05   , Dn3 , v092
	.byte	W06
	.byte		        Gs3 , v080
	.byte	W06
	.byte		        An3 , v084
	.byte	W06
	.byte		        Cs4 , v092
	.byte	W06
	.byte		N64   , Dn4 , v108
	.byte	W66
	.byte		N02   , Cs4 , v100
	.byte	W03
	.byte		        Bn3 , v088
	.byte	W03
	.byte		VOL   , 78*mus_qz_640_mvl/mxv
	.byte		N02   , An3 , v080
	.byte	W03
	.byte		VOL   , 74*mus_qz_640_mvl/mxv
	.byte		N02   , Gn3 , v068
	.byte	W03
	.byte		VOL   , 68*mus_qz_640_mvl/mxv
	.byte		N02   , Fs3 , v056
	.byte	W03
	.byte		VOL   , 61*mus_qz_640_mvl/mxv
	.byte		N02   , En3 , v044
	.byte	W03
	.byte		VOL   , 55*mus_qz_640_mvl/mxv
	.byte		N02   , Dn3 , v032
	.byte	W03
	.byte		VOL   , 50*mus_qz_640_mvl/mxv
	.byte		N02   , Cs3 , v024
	.byte	W03
	.byte		VOL   , 43*mus_qz_640_mvl/mxv
	.byte		N02   , Bn2 , v016
	.byte	W03
	.byte		VOL   , 37*mus_qz_640_mvl/mxv
	.byte		N02   , An2 , v012
	.byte	W03
	.byte		VOL   , 82*mus_qz_640_mvl/mxv
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
	.byte		N05   , Fn2 , v108
	.byte		N05   , Cs3 , v092
	.byte	W12
	.byte		        Fn2 , v100
	.byte		N05   , Cs3 , v088
	.byte	W18
	.byte		N17   , Gs2 , v100
	.byte		N17   , Fn3 , v088
	.byte	W18
	.byte		        Fn2 , v100
	.byte		N17   , Cs3 , v088
	.byte	W18
	.byte		N02   , Fn2
	.byte		N02   , Cs3
	.byte	W03
	.byte		        Fs2
	.byte		N02   , Ds3
	.byte	W03
	.byte		N20   , Gs2 , v100
	.byte		N20   , Fn3 , v088
	.byte	W24
	.byte		N05   , Gn2 , v108
	.byte		N05   , Ds3 , v092
	.byte	W12
	.byte		        Gn2 , v100
	.byte		N05   , Ds3 , v088
	.byte	W18
	.byte		N17   , Ds3 , v100
	.byte		N17   , Gn3 , v088
	.byte	W18
	.byte		        Gn2 , v100
	.byte		N17   , Ds3 , v088
	.byte	W18
	.byte		N02   , Gn2
	.byte		N02   , Ds3
	.byte	W03
	.byte		        Gs2
	.byte		N02   , Fn3
	.byte	W03
	.byte		N20   , As2 , v100
	.byte		N20   , Gn3 , v088
	.byte	W24
	.byte		N05   , Gs2 , v116
	.byte		N05   , En3 , v100
	.byte	W12
	.byte		        Gs2 , v108
	.byte		N05   , En3 , v092
	.byte	W18
	.byte		N17   , Bn2 , v108
	.byte		N17   , Gs3 , v092
	.byte	W18
	.byte		        Gs2 , v108
	.byte		N17   , En3 , v092
	.byte	W18
	.byte		N02   , Gs2
	.byte		N02   , En3
	.byte	W03
	.byte		        An2
	.byte		N02   , Fs3
	.byte	W03
	.byte		N20   , Bn2 , v108
	.byte		N20   , Gs3 , v092
	.byte	W24
	.byte		N05   , As2 , v108
	.byte		N05   , Fs3 , v092
	.byte	W12
	.byte		        As2 , v100
	.byte		N05   , Fs3 , v088
	.byte	W18
	.byte		N17   , Cs3 , v100
	.byte		N17   , As3 , v088
	.byte	W18
	.byte		N05   , As2 , v100
	.byte		N05   , Fs3 , v088
	.byte	W12
	.byte		        An2 , v096
	.byte		N05   , Fn3 , v080
	.byte	W12
	.byte		        Gs2 , v100
	.byte		N05   , En3 , v088
	.byte	W12
	.byte		        Gn2 , v096
	.byte		N05   , Ds3 , v080
	.byte	W12
	.byte	GOTO
	 ptrvalue	mus_qz_640_3_B1
	.byte	FINE

/********************** Track  4 **********************/

mus_qz_640_4:
	.byte		KEYSH , mus_qz_640_key+0
mus_qz_640_4_B1:
	.byte		VOICE , 30
	.byte		VOL   , 79*mus_qz_640_mvl/mxv
	.byte		PAN   , c_v+16
	.byte		VOL   , 66*mus_qz_640_mvl/mxv
	.byte		PRIO  , 58
	.byte		N11   , Cn2 , v088
	.byte	W30
	.byte		N17   , Bn1 , v084
	.byte	W24
	.byte		        Cn2 , v092
	.byte	W24
	.byte		        Fs2 , v096
	.byte	W18
mus_qz_640_4_000:
	.byte		N11   , Cn2 , v088
	.byte	W30
	.byte		N17   , Bn1 , v092
	.byte	W24
	.byte		        Cn2 , v084
	.byte	W24
	.byte		        Fs2 , v100
	.byte	W18
	.byte	PEND
mus_qz_640_4_001:
	.byte		N11   , Cn2 , v088
	.byte	W30
	.byte		N17   , Bn1 , v084
	.byte	W24
	.byte		        Cn2 , v092
	.byte	W24
	.byte		        Fs2 , v096
	.byte	W18
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_640_4_000
	.byte	PATT
	 ptrvalue	mus_qz_640_4_001
	.byte	PATT
	 ptrvalue	mus_qz_640_4_000
mus_qz_640_4_002:
	.byte		N11   , Dn2 , v088
	.byte	W30
	.byte		N17   , Cs2 , v084
	.byte	W24
	.byte		        Dn2 , v092
	.byte	W24
	.byte		        Gs2 , v096
	.byte	W18
	.byte	PEND
mus_qz_640_4_003:
	.byte		N11   , Dn2 , v088
	.byte	W30
	.byte		N17   , Cs2 , v092
	.byte	W24
	.byte		        Dn2 , v084
	.byte	W24
	.byte		        Gs2 , v100
	.byte	W18
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_640_4_001
	.byte	PATT
	 ptrvalue	mus_qz_640_4_000
	.byte	PATT
	 ptrvalue	mus_qz_640_4_001
	.byte	PATT
	 ptrvalue	mus_qz_640_4_000
	.byte	PATT
	 ptrvalue	mus_qz_640_4_001
	.byte	PATT
	 ptrvalue	mus_qz_640_4_000
	.byte	PATT
	 ptrvalue	mus_qz_640_4_001
	.byte	PATT
	 ptrvalue	mus_qz_640_4_000
	.byte	PATT
	 ptrvalue	mus_qz_640_4_002
	.byte	PATT
	 ptrvalue	mus_qz_640_4_003
	.byte	PATT
	 ptrvalue	mus_qz_640_4_001
	.byte	PATT
	 ptrvalue	mus_qz_640_4_000
	.byte		N11   , As1 , v088
	.byte	W48
	.byte		        Cs2 , v084
	.byte	W24
	.byte		        As1 , v100
	.byte	W24
	.byte		        Cn2 , v088
	.byte	W48
	.byte		        Ds2 , v084
	.byte	W24
	.byte		        Cn2 , v100
	.byte	W24
	.byte		        Cs2 , v088
	.byte	W48
	.byte		        En2 , v084
	.byte	W24
	.byte		        Cs2 , v100
	.byte	W24
	.byte		        Ds2 , v088
	.byte	W48
	.byte		N06   , Ds2 , v096
	.byte	W12
	.byte		        Dn2 , v084
	.byte	W12
	.byte		        Cs2 , v092
	.byte	W12
	.byte		        Cn2 , v084
	.byte	W12
	.byte	GOTO
	 ptrvalue	mus_qz_640_4_B1
	.byte	FINE

/********************** Track  5 **********************/

mus_qz_640_5:
	.byte		KEYSH , mus_qz_640_key+0
mus_qz_640_5_B1:
	.byte		VOICE , 39
	.byte		VOL   , 79*mus_qz_640_mvl/mxv
	.byte		PAN   , c_v-36
	.byte		VOL   , 61*mus_qz_640_mvl/mxv
	.byte		PRIO  , 40
	.byte		N07   , Dn1 , v100
	.byte		N28   , Cn2 , v116 , gtp1
	.byte	W12
	.byte		N05   , Dn1 , v072
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v092
	.byte	W06
	.byte		        Dn1 , v080
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v072
	.byte	W06
	.byte		        Dn1 , v108
	.byte	W06
	.byte		        Dn1 , v072
	.byte	W12
	.byte		        Dn1 , v084
	.byte	W06
	.byte		N02   , Dn1 , v100
	.byte	W03
	.byte		        Dn1 , v076
	.byte	W03
	.byte		        Dn1 , v084
	.byte	W03
	.byte		        Dn1 , v060
	.byte	W03
	.byte		        Dn1 , v080
	.byte	W03
	.byte		        Dn1 , v068
	.byte	W03
	.byte		        Dn1 , v088
	.byte	W03
	.byte		        Dn1 , v064
	.byte	W03
	.byte		N07   , Dn1 , v100
	.byte	W12
	.byte		N05   , Dn1 , v072
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v092
	.byte	W06
	.byte		        Dn1 , v080
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v072
	.byte	W06
	.byte		        Dn1 , v108
	.byte	W06
	.byte		        Dn1 , v072
	.byte	W12
	.byte		        Dn1 , v084
	.byte	W06
	.byte		        Dn1 , v100
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v092
	.byte	W06
	.byte		        Dn1 , v080
	.byte	W06
	.byte		N07   , Dn1 , v100
	.byte		N28   , Cn2 , v116 , gtp1
	.byte	W12
	.byte		N05   , Dn1 , v072
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v092
	.byte	W06
	.byte		        Dn1 , v080
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v072
	.byte	W06
	.byte		        Dn1 , v108
	.byte	W06
	.byte		        Dn1 , v072
	.byte	W12
	.byte		        Dn1 , v084
	.byte	W06
	.byte		        Dn1 , v092
	.byte	W06
	.byte		        Dn1 , v080
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v072
	.byte	W06
	.byte		N07   , Dn1 , v100
	.byte	W12
	.byte		N05   , Dn1 , v072
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v092
	.byte	W06
	.byte		        Dn1 , v080
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v072
	.byte	W06
	.byte		        Dn1 , v108
	.byte	W06
	.byte		        Dn1 , v072
	.byte	W12
	.byte		        Dn1 , v084
	.byte	W06
	.byte		N02   , Dn1 , v100
	.byte	W03
	.byte		        Dn1 , v076
	.byte	W03
	.byte		        Dn1 , v084
	.byte	W03
	.byte		        Dn1 , v060
	.byte	W03
	.byte		        Dn1 , v080
	.byte	W03
	.byte		        Dn1 , v068
	.byte	W03
	.byte		        Dn1 , v088
	.byte	W03
	.byte		        Dn1 , v064
	.byte	W03
	.byte		N07   , Dn1 , v100
	.byte	W12
	.byte		N05   , Dn1 , v072
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v092
	.byte	W06
	.byte		        Dn1 , v080
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v072
	.byte	W06
	.byte		        Dn1 , v108
	.byte	W06
	.byte		        Dn1 , v072
	.byte	W12
	.byte		        Dn1 , v084
	.byte	W06
	.byte		        Dn1 , v092
	.byte	W06
	.byte		        Dn1 , v080
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v072
	.byte	W06
	.byte		N07   , Dn1 , v100
	.byte	W12
	.byte		N05   , Dn1 , v072
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v092
	.byte	W06
	.byte		        Dn1 , v080
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v072
	.byte	W06
	.byte		        Dn1 , v108
	.byte	W06
	.byte		        Dn1 , v072
	.byte	W12
	.byte		N03   , Dn1 , v084
	.byte	W06
	.byte		N02   , Dn1 , v096
	.byte	W03
	.byte		        Dn1 , v076
	.byte	W03
	.byte		N05   , Dn1 , v084
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v076
	.byte	W06
	.byte		        Dn1 , v100
	.byte		N28   , Cn2 , v116 , gtp1
	.byte	W12
	.byte		N05   , Dn1 , v072
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v092
	.byte	W06
	.byte		        Dn1 , v080
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v072
	.byte	W06
	.byte		        Dn1 , v108
	.byte	W06
	.byte		        Dn1 , v072
	.byte	W12
	.byte		        Dn1 , v084
	.byte	W06
	.byte		        Dn1 , v092
	.byte	W06
	.byte		        Dn1 , v080
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v072
	.byte	W06
mus_qz_640_5_000:
	.byte		N05   , Dn1 , v100
	.byte	W12
	.byte		        Dn1 , v072
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v092
	.byte	W06
	.byte		        Dn1 , v080
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v072
	.byte	W06
	.byte		        Dn1 , v108
	.byte	W06
	.byte		        Dn1 , v072
	.byte	W12
	.byte		        Dn1 , v084
	.byte	W06
	.byte		        Dn1 , v092
	.byte	W06
	.byte		        Dn1 , v080
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v072
	.byte	W06
	.byte	PEND
	.byte		        Dn1 , v100
	.byte	W12
	.byte		        Dn1 , v072
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v092
	.byte	W06
	.byte		        Dn1 , v080
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v072
	.byte	W06
	.byte		        Dn1 , v100
	.byte	W06
	.byte		        Dn1 , v072
	.byte	W12
	.byte		        Dn1 , v084
	.byte	W06
	.byte		N02   , Dn1 , v100
	.byte	W03
	.byte		        Dn1 , v076
	.byte	W03
	.byte		N05   , Dn1 , v092
	.byte	W06
	.byte		        Dn1 , v084
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v100
	.byte	W12
	.byte		        Dn1 , v072
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v092
	.byte	W06
	.byte		        Dn1 , v080
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v072
	.byte	W06
	.byte		        Dn1 , v108
	.byte	W06
	.byte		        Dn1 , v072
	.byte	W12
	.byte		        Dn1 , v084
	.byte	W06
	.byte		        Dn1 , v092
	.byte		N22   , Fn1 , v124
	.byte	W06
	.byte		N05   , Dn1 , v080
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v072
	.byte	W06
	.byte		        Dn1 , v100
	.byte		N28   , Cn2 , v124 , gtp1
	.byte	W12
	.byte		N05   , Dn1 , v072
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v092
	.byte	W06
	.byte		        Dn1 , v080
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v072
	.byte	W06
	.byte		        Dn1 , v100
	.byte	W06
	.byte		        Dn1 , v072
	.byte	W12
	.byte		        Dn1 , v084
	.byte	W06
	.byte		        Dn1 , v092
	.byte	W06
	.byte		        Dn1 , v080
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v072
	.byte	W06
mus_qz_640_5_001:
	.byte		N05   , Dn1 , v100
	.byte	W12
	.byte		        Dn1 , v072
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v092
	.byte	W06
	.byte		        Dn1 , v080
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v072
	.byte	W06
	.byte		        Dn1 , v108
	.byte	W06
	.byte		        Dn1 , v072
	.byte	W12
	.byte		        Dn1 , v084
	.byte	W06
	.byte		N02   , Dn1 , v100
	.byte	W03
	.byte		        Dn1 , v076
	.byte	W03
	.byte		        Dn1 , v084
	.byte	W03
	.byte		        Dn1 , v060
	.byte	W03
	.byte		        Dn1 , v080
	.byte	W03
	.byte		        Dn1 , v068
	.byte	W03
	.byte		        Dn1 , v088
	.byte	W03
	.byte		        Dn1 , v064
	.byte	W03
	.byte	PEND
mus_qz_640_5_002:
	.byte		N05   , Dn1 , v100
	.byte		N28   , Cn2 , v124 , gtp1
	.byte	W12
	.byte		N05   , Dn1 , v072
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v092
	.byte	W06
	.byte		        Dn1 , v080
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v072
	.byte	W06
	.byte		        Dn1 , v108
	.byte	W06
	.byte		        Dn1 , v072
	.byte	W12
	.byte		        Dn1 , v084
	.byte	W06
	.byte		        Dn1 , v092
	.byte	W06
	.byte		        Dn1 , v080
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v072
	.byte	W06
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_640_5_000
	.byte	PATT
	 ptrvalue	mus_qz_640_5_000
	.byte	PATT
	 ptrvalue	mus_qz_640_5_001
	.byte	PATT
	 ptrvalue	mus_qz_640_5_002
	.byte	PATT
	 ptrvalue	mus_qz_640_5_000
	.byte	PATT
	 ptrvalue	mus_qz_640_5_000
	.byte		N05   , Dn1 , v100
	.byte	W12
	.byte		        Dn1 , v072
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v092
	.byte	W06
	.byte		        Dn1 , v080
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v072
	.byte	W06
	.byte		        Dn1 , v108
	.byte	W06
	.byte		        Dn1 , v072
	.byte	W12
	.byte		        Dn1 , v084
	.byte	W06
	.byte		N02   , Dn1 , v100
	.byte		N22   , Fn1 , v124
	.byte	W03
	.byte		N02   , Dn1 , v076
	.byte	W03
	.byte		        Dn1 , v084
	.byte	W03
	.byte		        Dn1 , v060
	.byte	W03
	.byte		        Dn1 , v080
	.byte	W03
	.byte		        Dn1 , v068
	.byte	W03
	.byte		        Dn1 , v088
	.byte	W03
	.byte		        Dn1 , v064
	.byte	W03
	.byte		N10   , Dn1 , v100
	.byte		N28   , Cn2 , v124 , gtp1
	.byte	W12
	.byte		N10   , Dn1 , v072
	.byte	W12
	.byte		N02   , Dn1 , v092
	.byte	W03
	.byte		        Dn1 , v068
	.byte	W03
	.byte		        Dn1 , v080
	.byte	W03
	.byte		        Dn1 , v060
	.byte	W03
	.byte		N05   , Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v072
	.byte	W06
	.byte		        Dn1 , v108
	.byte	W06
	.byte		        Dn1 , v072
	.byte	W12
	.byte		        Dn1 , v084
	.byte	W06
	.byte		        Dn1 , v092
	.byte	W06
	.byte		        Dn1 , v080
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v072
	.byte	W06
	.byte		        Dn1 , v100
	.byte	W12
	.byte		        Dn1 , v072
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v092
	.byte	W06
	.byte		        Dn1 , v080
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v072
	.byte	W06
	.byte		        Dn1 , v108
	.byte	W06
	.byte		        Dn1 , v072
	.byte	W12
	.byte		        Dn1 , v084
	.byte	W06
	.byte		N02   , Dn1 , v100
	.byte		N23   , Cn2 , v124
	.byte	W03
	.byte		N02   , Dn1 , v076
	.byte	W03
	.byte		        Dn1 , v084
	.byte	W03
	.byte		        Dn1 , v060
	.byte	W03
	.byte		        Dn1 , v080
	.byte	W03
	.byte		        Dn1 , v068
	.byte	W03
	.byte		        Dn1 , v088
	.byte	W03
	.byte		        Dn1 , v064
	.byte	W03
	.byte	PATT
	 ptrvalue	mus_qz_640_5_002
	.byte		N05   , Dn1 , v100
	.byte	W12
	.byte		        Dn1 , v072
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v092
	.byte	W06
	.byte		        Dn1 , v080
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v072
	.byte	W06
	.byte		N08   , Dn1 , v108
	.byte	W12
	.byte		        Dn1 , v092
	.byte	W12
	.byte		        Dn1 , v104
	.byte	W12
	.byte		N05   , Dn1 , v092
	.byte	W06
	.byte		        Dn1 , v080
	.byte	W06
	.byte	GOTO
	 ptrvalue	mus_qz_640_5_B1
	.byte	FINE

/********************** Track  6 **********************/

mus_qz_640_6:
	.byte		KEYSH , mus_qz_640_key+0
mus_qz_640_6_B1:
	.byte		VOICE , 28
	.byte		VOL   , 84*mus_qz_640_mvl/mxv
	.byte		PAN   , c_v-34
	.byte		VOL   , 82*mus_qz_640_mvl/mxv
	.byte		PRIO  , 42
	.byte		N20   , As1 , v116
	.byte	W96
	.byte		        As1 , v104
	.byte	W72
	.byte		N02   , Gn2 , v008
	.byte	W03
	.byte		        Gn2 , v012
	.byte	W03
	.byte		        Gn2 , v016
	.byte	W03
	.byte		        Gn2 , v032
	.byte	W03
	.byte		        Gn2 , v048
	.byte	W03
	.byte		        Gn2 , v064
	.byte	W03
	.byte		        Gn2 , v084
	.byte	W03
	.byte		        Gn2 , v096
	.byte	W03
	.byte		N23   , As1 , v116
	.byte	W96
	.byte	W92
	.byte	W03
	.byte	N20
	.byte	W01
	.byte	W96
	.byte	W96
	.byte	N20
	.byte	W96
	.byte	W96
	.byte	N20
	.byte	W96
	.byte	N17
	.byte	W72
	.byte		        As1 , v108
	.byte	W24
	.byte		        As1 , v116
	.byte	W96
	.byte	W72
	.byte		N02   , Gn2 , v008
	.byte	W03
	.byte		        Gn2 , v012
	.byte	W03
	.byte		        Gn2 , v016
	.byte	W03
	.byte		        Gn2 , v032
	.byte	W03
	.byte		        Gn2 , v048
	.byte	W03
	.byte		        Gn2 , v064
	.byte	W03
	.byte		        Gn2 , v084
	.byte	W03
	.byte		        Gn2 , v096
	.byte	W03
	.byte		N17   , As1 , v116
	.byte	W96
	.byte	W96
	.byte	N17
	.byte	W96
	.byte	W96
	.byte	N17
	.byte	W96
	.byte	W96
	.byte	N17
	.byte	W96
	.byte	W96
	.byte	N17
	.byte	W96
	.byte	W72
	.byte	N17
	.byte	W24
	.byte	N17
	.byte	W96
	.byte	W48
	.byte	N17
	.byte	W24
	.byte	N17
	.byte	W24
	.byte	GOTO
	 ptrvalue	mus_qz_640_6_B1
	.byte	FINE

/********************** Track  7 **********************/

mus_qz_640_7:
	.byte		KEYSH , mus_qz_640_key+0
mus_qz_640_7_B1:
	.byte		VOICE , 35
	.byte		VOL   , 63*mus_qz_640_mvl/mxv
	.byte		PAN   , c_v+6
	.byte		VOL   , 49*mus_qz_640_mvl/mxv
	.byte		PRIO  , 54
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
mus_qz_640_7_000:
	.byte	W12
	.byte		N04   , Bn3 , v088
	.byte	W06
	.byte		        Fs4 , v104
	.byte	W06
	.byte		        An4 , v124
	.byte	W06
	.byte		        Bn4 , v104
	.byte	W06
	.byte		        An4 , v112
	.byte	W06
	.byte		        Gn4 , v100
	.byte	W06
	.byte		N02   , Fs4 , v112
	.byte	W03
	.byte		        Gn4 , v080
	.byte	W03
	.byte		        Fs4 , v092
	.byte	W03
	.byte		        Gn4 , v072
	.byte	W03
	.byte		        Fs4 , v084
	.byte	W03
	.byte		        Gn4 , v056
	.byte	W03
	.byte		        Fs4 , v064
	.byte	W03
	.byte		        Gn4 , v048
	.byte	W03
	.byte		        Fs4 , v064
	.byte	W03
	.byte		        Gn4 , v040
	.byte	W03
	.byte		        Fs4 , v048
	.byte	W03
	.byte		        Gn4 , v032
	.byte	W15
	.byte	PEND
	.byte	W96
	.byte	W96
	.byte	W96
mus_qz_640_7_001:
	.byte	W12
	.byte		N04   , Cn4 , v088
	.byte	W05
	.byte		        Fs4 , v104
	.byte	W06
	.byte		        An4 , v124
	.byte	W06
	.byte		        Bn4 , v104
	.byte	W06
	.byte		        An4 , v112
	.byte	W06
	.byte		        Gn4 , v100
	.byte	W06
	.byte		N02   , Fs4 , v112
	.byte	W03
	.byte		        Gn4 , v080
	.byte	W03
	.byte		        Fs4 , v092
	.byte	W03
	.byte		        Gn4 , v072
	.byte	W03
	.byte		        Fs4 , v084
	.byte	W03
	.byte		        Gn4 , v056
	.byte	W03
	.byte		        Fs4 , v064
	.byte	W03
	.byte		        Gn4 , v048
	.byte	W03
	.byte		        Fs4 , v064
	.byte	W03
	.byte		        Gn4 , v040
	.byte	W03
	.byte		        Fs4 , v048
	.byte	W03
	.byte		        Gn4 , v032
	.byte	W16
	.byte	PEND
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_640_7_000
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_640_7_001
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_640_7_B1
	.byte	FINE

/********************** Track  8 **********************/

mus_qz_640_8:
	.byte		KEYSH , mus_qz_640_key+0
mus_qz_640_8_B1:
	.byte		VOICE , 32
	.byte		VOL   , 24*mus_qz_640_mvl/mxv
	.byte		PAN   , c_v+56
	.byte		VOL   , 19*mus_qz_640_mvl/mxv
	.byte		PRIO  , 40
	.byte	W96
	.byte	W96
	.byte		N04   , Cn3 , v080
	.byte	W06
	.byte		N44   , En3 , v092 , gtp2
	.byte	W48
	.byte		N22   , Cn4 , v072
	.byte	W24
	.byte		        Bn3 , v068
	.byte	W18
	.byte	W06
	.byte		N84   , En3 , v076 , gtp2
	.byte	W56
	.byte	W01
	.byte		VOL   , 19*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        18*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        17*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        15*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        13*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        13*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        10*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        10*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        6*mus_qz_640_mvl/mxv
	.byte	W09
	.byte		        19*mus_qz_640_mvl/mxv
	.byte		N05   , Ds3 , v080
	.byte	W06
	.byte		N44   , En3 , v080 , gtp2
	.byte	W48
	.byte		N22   , Cn4 , v068
	.byte	W24
	.byte		        Bn3 , v064
	.byte	W18
	.byte	W06
	.byte		N88   , En4 , v084 , gtp1
	.byte	W72
	.byte		VOL   , 19*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        17*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        16*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        15*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        14*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        13*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        11*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        9*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        19*mus_qz_640_mvl/mxv
	.byte		N44   , Dn4 , v080 , gtp2
	.byte	W48
	.byte		N22   , Cs4 , v068
	.byte	W24
	.byte		        An3 , v060
	.byte	W18
	.byte	W06
	.byte		N44   , Cn4 , v072 , gtp1
	.byte	W48
	.byte		N22   , As3 , v056
	.byte	W24
	.byte		        Fn3 , v060
	.byte	W18
	.byte	W06
	.byte		N64   , An3 , v076 , gtp1
	.byte	W44
	.byte	W01
	.byte		VOL   , 18*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        17*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        15*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        14*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        13*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        12*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        10*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        10*mus_qz_640_mvl/mxv
	.byte	W06
	.byte		        19*mus_qz_640_mvl/mxv
	.byte		N05   , Dn3 , v064
	.byte	W06
	.byte		        Gs3 , v052
	.byte	W06
	.byte		        An3 , v056
	.byte	W06
	.byte		        Cs4 , v064
	.byte	W06
	.byte		N64   , Dn4 , v076 , gtp1
	.byte	W66
	.byte		N02   , Cs4 , v072
	.byte	W03
	.byte		        Bn3 , v060
	.byte	W03
	.byte		VOL   , 18*mus_qz_640_mvl/mxv
	.byte		N02   , An3 , v056
	.byte	W03
	.byte		VOL   , 17*mus_qz_640_mvl/mxv
	.byte		N02   , Gn3 , v044
	.byte	W03
	.byte		VOL   , 15*mus_qz_640_mvl/mxv
	.byte		N02   , Fs3 , v032
	.byte	W03
	.byte		VOL   , 13*mus_qz_640_mvl/mxv
	.byte		N02   , En3 , v024
	.byte	W03
	.byte		VOL   , 12*mus_qz_640_mvl/mxv
	.byte		N02   , Dn3 , v016
	.byte	W03
	.byte		VOL   , 10*mus_qz_640_mvl/mxv
	.byte		N02   , Cs3 , v008
	.byte	W03
	.byte		VOL   , 8*mus_qz_640_mvl/mxv
	.byte		N02   , Bn2 , v004
	.byte	W03
	.byte		VOL   , 7*mus_qz_640_mvl/mxv
	.byte		N02   , An2
	.byte	W03
	.byte		VOL   , 19*mus_qz_640_mvl/mxv
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
	.byte	GOTO
	 ptrvalue	mus_qz_640_8_B1
	.byte	FINE

/********************** Track  9 **********************/

mus_qz_640_9:
	.byte		KEYSH , mus_qz_640_key+0
mus_qz_640_9_B1:
	.byte		VOICE , 29
	.byte		VOL   , 63*mus_qz_640_mvl/mxv
	.byte		PAN   , c_v+6
	.byte		VOL   , 61*mus_qz_640_mvl/mxv
	.byte		PRIO  , 38
	.byte		N03   , En3 , v080
	.byte	W12
	.byte		        En3 , v072
	.byte	W12
	.byte		        Fs3 , v084
	.byte	W06
	.byte		        Fs3 , v076
	.byte	W18
	.byte		        Gs3 , v084
	.byte	W06
	.byte		        Gs3 , v076
	.byte	W18
	.byte		        An3 , v084
	.byte	W06
	.byte		        An3 , v076
	.byte	W18
mus_qz_640_9_000:
	.byte		N03   , En3 , v080
	.byte	W06
	.byte		        En3 , v068
	.byte	W18
	.byte		        Fs3 , v084
	.byte	W06
	.byte		        Fs3 , v072
	.byte	W18
	.byte		        Gs3 , v084
	.byte	W06
	.byte		        Gs3 , v072
	.byte	W18
	.byte		        An3 , v084
	.byte	W06
	.byte		        An3 , v072
	.byte	W18
	.byte	PEND
mus_qz_640_9_001:
	.byte		N03   , En3 , v080
	.byte	W12
	.byte		        En3 , v072
	.byte	W12
	.byte		        Fs3 , v084
	.byte	W06
	.byte		        Fs3 , v076
	.byte	W18
	.byte		        Gs3 , v084
	.byte	W06
	.byte		        Gs3 , v076
	.byte	W18
	.byte		        An3 , v084
	.byte	W06
	.byte		        An3 , v076
	.byte	W18
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_640_9_000
	.byte	PATT
	 ptrvalue	mus_qz_640_9_001
	.byte	PATT
	 ptrvalue	mus_qz_640_9_000
	.byte		N03   , Fn3 , v080
	.byte	W12
	.byte		        Fn3 , v072
	.byte	W12
	.byte		        Gs3 , v084
	.byte	W06
	.byte		        Gs3 , v076
	.byte	W18
	.byte		        An3 , v084
	.byte	W06
	.byte		        An3 , v076
	.byte	W18
	.byte		        As3 , v084
	.byte	W06
	.byte		        As3 , v076
	.byte	W18
	.byte		        Fn3 , v080
	.byte	W06
	.byte		        Fn3 , v068
	.byte	W18
	.byte		        Gs3 , v084
	.byte	W06
	.byte		        Gs3 , v072
	.byte	W18
	.byte		        An3 , v084
	.byte	W06
	.byte		        An3 , v072
	.byte	W18
	.byte		        Dn4 , v084
	.byte	W06
	.byte		        Dn4 , v072
	.byte	W18
	.byte	PATT
	 ptrvalue	mus_qz_640_9_001
	.byte	PATT
	 ptrvalue	mus_qz_640_9_000
	.byte		N04   , En3 , v080
	.byte	W12
	.byte		        En3 , v072
	.byte	W12
	.byte		        Fs3 , v084
	.byte	W06
	.byte		        Fs3 , v076
	.byte	W18
	.byte		        Gs3 , v084
	.byte	W06
	.byte		        Gs3 , v076
	.byte	W18
	.byte		        An3 , v084
	.byte	W06
	.byte		        An3 , v076
	.byte	W18
	.byte		        En3 , v080
	.byte	W06
	.byte		        En3 , v068
	.byte	W18
	.byte		        Fs3 , v084
	.byte	W06
	.byte		        Fs3 , v072
	.byte	W18
	.byte		        Gs3 , v084
	.byte	W06
	.byte		        Gs3 , v072
	.byte	W18
	.byte		        An3 , v084
	.byte	W06
	.byte		        An3 , v072
	.byte	W15
	.byte		N02   , Bn2 , v076
	.byte		N02   , Ds3
	.byte	W03
	.byte		N44   , Cn3 , v088
	.byte		N44   , En3 , v108
	.byte	W48
	.byte		N22   , En3 , v080
	.byte		N22   , Cn4 , v104
	.byte	W24
	.byte		        Fs3 , v076
	.byte		N22   , Bn3 , v096
	.byte	W24
	.byte		N84   , Cn3 , v092 , gtp2
	.byte		N84   , En3 , v108 , gtp2
	.byte	W66
	.byte		VOL   , 59*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        55*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        52*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        47*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        43*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        40*mus_qz_640_mvl/mxv
	.byte	W09
	.byte		        61*mus_qz_640_mvl/mxv
	.byte		N04   , Bn2 , v080
	.byte		N04   , Ds3 , v096
	.byte	W06
	.byte		N44   , Cn3 , v100 , gtp2
	.byte		N44   , En3 , v116 , gtp2
	.byte	W48
	.byte		N20   , Gn3 , v088
	.byte		N20   , Cn4 , v104
	.byte	W24
	.byte		        Fs3 , v080
	.byte		N20   , Bn3 , v092
	.byte	W24
	.byte		N88   , Gn3 , v092 , gtp1
	.byte		N88   , En4 , v120 , gtp1
	.byte	W48
	.byte		VOL   , 58*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        53*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        52*mus_qz_640_mvl/mxv
	.byte	W06
	.byte		        47*mus_qz_640_mvl/mxv
	.byte	W12
	.byte		        44*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        40*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        35*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        32*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        28*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        24*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        21*mus_qz_640_mvl/mxv
	.byte	W06
	.byte		        61*mus_qz_640_mvl/mxv
	.byte		N44   , An3 , v088 , gtp2
	.byte		N44   , Dn4 , v108 , gtp2
	.byte	W48
	.byte		N21   , Gs3 , v076
	.byte		N21   , Cs4 , v096
	.byte	W24
	.byte		        Fn3 , v068
	.byte		N21   , An3 , v088
	.byte	W24
	.byte		N44   , Gs3 , v080 , gtp1
	.byte		N44   , Cn4 , v104 , gtp1
	.byte	W48
	.byte		N20   , Fn3 , v072
	.byte		N20   , As3 , v092
	.byte	W24
	.byte		        Cn3 , v084
	.byte		N20   , Fn3 , v108
	.byte	W24
	.byte		N64   , En3 , v088 , gtp1
	.byte		N64   , An3 , v112 , gtp1
	.byte	W42
	.byte		VOL   , 60*mus_qz_640_mvl/mxv
	.byte	W06
	.byte		        58*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        57*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        53*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        50*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        47*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        41*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        36*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        31*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        60*mus_qz_640_mvl/mxv
	.byte		N04   , An2 , v084
	.byte		N04   , Dn3 , v100
	.byte	W06
	.byte		        Dn3 , v064
	.byte		N04   , Gs3 , v076
	.byte	W06
	.byte		        En3 , v068
	.byte		N04   , An3 , v080
	.byte	W06
	.byte	N04
	.byte		N04   , Cs4 , v096
	.byte	W06
	.byte		N80   , An3 , v096 , gtp3
	.byte		N80   , Dn4 , v112 , gtp3
	.byte	W48
	.byte		VOL   , 58*mus_qz_640_mvl/mxv
	.byte	W06
	.byte		        53*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        47*mus_qz_640_mvl/mxv
	.byte	W03
	.byte		        43*mus_qz_640_mvl/mxv
	.byte	W06
	.byte		        37*mus_qz_640_mvl/mxv
	.byte	W06
	.byte		        32*mus_qz_640_mvl/mxv
	.byte	W06
	.byte		        29*mus_qz_640_mvl/mxv
	.byte	W15
	.byte		        60*mus_qz_640_mvl/mxv
	.byte	W03
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_640_9_B1
	.byte	FINE

/******************************************************/
	.p2align	2

mus_qz_640:
	.byte	9	/* NumTrks */
	.byte	0	/* NumBlks */
	.byte	mus_qz_640_pri	/* Priority */
	.byte	mus_qz_640_rev	/* Reverb. */
space64 4
	ptrvalue	mus_qz_640_grp

	ptrvalue	mus_qz_640_1
	ptrvalue	mus_qz_640_2
	ptrvalue	mus_qz_640_3
	ptrvalue	mus_qz_640_4
	ptrvalue	mus_qz_640_5
	ptrvalue	mus_qz_640_6
	ptrvalue	mus_qz_640_7
	ptrvalue	mus_qz_640_8
	ptrvalue	mus_qz_640_9

	.end
