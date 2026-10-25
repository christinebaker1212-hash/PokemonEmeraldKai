	.include "asm/macros/bit_width.inc"

	.include "MPlayDef.s"

/* Moemon Quetzal song 656, disassembled from the ROM by tools/quetzal_rip/songs.py.
   "org" comments give the ROM address of the bytes that follow. */

	.equ	mus_qz_656_grp, voicegroup_qz_songs
	.equ	mus_qz_656_pri, 5
	.equ	mus_qz_656_rev, reverb_set+0
	.equ	mus_qz_656_mvl, 127
	.equ	mus_qz_656_key, 0
	.equ	mus_qz_656_tbs, 1
	.equ	mus_qz_656_exg, 1
	.equ	mus_qz_656_cmp, 1

	.section .rodata
	.global	mus_qz_656
	.p2align	2
/* org 0x0969EEE0 */

/********************** Track  1 **********************/

mus_qz_656_1:
	.byte		KEYSH , mus_qz_656_key+0
	.byte		TEMPO , 152*mus_qz_656_tbs/2
	.byte		VOICE , 0
	.byte		VOL   , 76*mus_qz_656_mvl/mxv
	.byte		PAN   , c_v+6
	.byte		VOL   , 68*mus_qz_656_mvl/mxv
	.byte		PRIO  , 64
	.byte		N04   , Cn4 , v104
	.byte		N04   , Ds4 , v116
	.byte	W06
	.byte		        Cn4 , v056
	.byte		N04   , Ds4 , v064
	.byte	W06
	.byte		        As3 , v092
	.byte		N04   , Cs4 , v104
	.byte	W06
	.byte		        As3 , v048
	.byte		N04   , Cs4 , v056
	.byte	W06
	.byte		        Gs3 , v100
	.byte		N04   , Cn4 , v112
	.byte	W06
	.byte		        Gs3 , v052
	.byte		N04   , Cn4 , v060
	.byte	W06
	.byte		        Cn4 , v100
	.byte		N04   , Ds4 , v108
	.byte	W06
	.byte		        Cn4 , v048
	.byte		N04   , Ds4 , v056
	.byte	W06
	.byte		        Gs3 , v104
	.byte		N04   , Fn4 , v116
	.byte	W06
	.byte		        Gs3 , v056
	.byte		N04   , Fn4 , v064
	.byte	W12
	.byte		        Cn4 , v092
	.byte		N04   , Gs4 , v104
	.byte	W06
	.byte		        Cn4 , v048
	.byte		N04   , Gs4 , v056
	.byte	W12
	.byte		        Ds4 , v108
	.byte		N04   , Cn5 , v120
	.byte	W06
	.byte		        Ds4 , v056
	.byte		N04   , Cn5 , v064
	.byte	W06
	.byte		N05   , Gn4 , v112
	.byte		N05   , Ds5 , v120
	.byte	W06
	.byte		        Gn4 , v060
	.byte		N05   , Ds5 , v072
	.byte	W12
	.byte		        As3 , v104
	.byte		N05   , Gn4 , v116
	.byte	W06
	.byte		        As3 , v056
	.byte		N05   , Gn4 , v064
	.byte	W12
	.byte		        Cn4 , v112
	.byte		N05   , Gs4 , v120
	.byte	W06
	.byte		        Cn4 , v032
	.byte		N05   , Gs4 , v040
	.byte	W06
	.byte		        Cn4 , v008
	.byte		N05   , Gs4
	.byte	W05
	.byte	FINE

/********************** Track  2 **********************/

mus_qz_656_2:
	.byte		KEYSH , mus_qz_656_key+0
	.byte		VOICE , 22
	.byte		VOL   , 76*mus_qz_656_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		VOL   , 59*mus_qz_656_mvl/mxv
	.byte		PRIO  , 62
	.byte		N05   , Gs1 , v112
	.byte	W12
	.byte		N02   , Cn2 , v096
	.byte	W06
	.byte		        Ds2 , v108
	.byte	W06
	.byte		N05   , Gn1
	.byte	W12
	.byte		N02   , Cn2 , v096
	.byte	W06
	.byte		        Ds2 , v108
	.byte	W06
	.byte		N05   , Fn1 , v120
	.byte	W06
	.byte		N01   , Fn1 , v108
	.byte	W12
	.byte		        Fn1 , v100
	.byte	W06
	.byte		N05   , Ds1 , v108
	.byte	W12
	.byte		        Cn1 , v100
	.byte	W12
	.byte		        Ds1 , v116
	.byte	W18
	.byte		        Ds1 , v108
	.byte	W18
	.byte		        Gs1 , v120
	.byte	W05
	.byte	FINE

/********************** Track  3 **********************/

mus_qz_656_3:
	.byte		KEYSH , mus_qz_656_key+0
	.byte		VOICE , 3
	.byte		VOL   , 82*mus_qz_656_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		VOL   , 64*mus_qz_656_mvl/mxv
	.byte		PRIO  , 60
	.byte		N05   , Ds5 , v116
	.byte	W06
	.byte		        Ds5 , v072
	.byte	W06
	.byte		        Cs5 , v108
	.byte	W06
	.byte		        Cs5 , v068
	.byte	W06
	.byte		        Cn5 , v116
	.byte	W06
	.byte		        Cn5 , v076
	.byte	W06
	.byte		        Ds5 , v112
	.byte	W06
	.byte		        Ds5 , v068
	.byte	W06
	.byte		        Fn5 , v116
	.byte	W06
	.byte		        Fn5 , v072
	.byte	W12
	.byte		        Gs5 , v116
	.byte	W06
	.byte		        Gs5 , v072
	.byte	W12
	.byte		        Cn6 , v120
	.byte	W06
	.byte		        Cn6 , v076
	.byte	W06
	.byte		        Ds6 , v124
	.byte	W06
	.byte		        Ds6 , v068
	.byte	W12
	.byte		        Gn5 , v108
	.byte	W06
	.byte		        Gn5 , v072
	.byte	W11
	.byte		        Gs5 , v120
	.byte	W07
	.byte		N04   , Gs5 , v052
	.byte	W06
	.byte		        Gs5 , v028
	.byte	W06
	.byte		        Gs5 , v008
	.byte	W04
	.byte	FINE

/********************** Track  4 **********************/

mus_qz_656_4:
	.byte		KEYSH , mus_qz_656_key+0
	.byte		VOICE , 39
	.byte		VOL   , 64*mus_qz_656_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		VOL   , 50*mus_qz_656_mvl/mxv
	.byte		PRIO  , 58
	.byte		N05   , Cn1 , v116
	.byte		N04   , Fs2 , v084
	.byte	W12
	.byte		        Fs2 , v076
	.byte	W06
	.byte		        Fs2 , v056
	.byte	W06
	.byte		N10   , Ds1 , v116
	.byte		N01   , Fs2 , v064
	.byte	W03
	.byte		        Fs2 , v048
	.byte	W03
	.byte		        Fs2 , v056
	.byte	W03
	.byte		        Fs2 , v036
	.byte	W03
	.byte		N05   , Cn1 , v088
	.byte		N01   , Fs2 , v044
	.byte	W03
	.byte		        Fs2 , v028
	.byte	W03
	.byte		        Fs2 , v036
	.byte	W03
	.byte		        Fs2 , v020
	.byte	W03
	.byte		N05   , Cn1 , v116
	.byte	W06
	.byte		N10   , Ds1 , v100
	.byte		N04   , Fs2 , v064
	.byte	W12
	.byte		        Fs2 , v076
	.byte	W05
	.byte		N10   , Ds1 , v120
	.byte	W13
	.byte		        Ds1 , v108
	.byte		N04   , Fs2 , v056
	.byte	W12
	.byte		N05   , Cn1 , v116
	.byte		N10   , As1 , v092
	.byte	W12
	.byte		N01   , Fs1 , v036
	.byte	W06
	.byte		N05   , Cn1 , v100
	.byte		N10   , As1 , v088
	.byte	W12
	.byte		N01   , Fs1 , v036
	.byte	W06
	.byte		N05   , Cn1 , v116
	.byte		N11   , Dn2 , v104
	.byte	W11
	.byte	FINE

/********************** Track  5 **********************/

mus_qz_656_5:
	.byte		KEYSH , mus_qz_656_key+0
	.byte		VOICE , 4
	.byte		VOL   , 20*mus_qz_656_mvl/mxv
	.byte		PAN   , c_v-54
	.byte		VOL   , 16*mus_qz_656_mvl/mxv
	.byte		PRIO  , 40
	.byte	W06
	.byte		N11   , Ds5 , v116
	.byte	W12
	.byte		        Cs5 , v108
	.byte	W12
	.byte		        Cn5 , v112
	.byte	W12
	.byte		        Ds5 , v108
	.byte	W12
	.byte		N10   , Fn5 , v116
	.byte	W18
	.byte		        Gs5 , v112
	.byte	W18
	.byte		        Cn6 , v120
	.byte	W06
	.byte	W06
	.byte		N11   , Ds6
	.byte	W18
	.byte		        Gn5 , v112
	.byte	W17
	.byte		N05   , Gs5 , v124
	.byte	W07
	.byte		N04   , Gs5 , v056
	.byte	W06
	.byte		        Gs5 , v008
	.byte	W04
	.byte	FINE

/********************** Track  6 **********************/

mus_qz_656_6:
	.byte		KEYSH , mus_qz_656_key+0
	.byte		VOICE , 23
	.byte		VOL   , 59*mus_qz_656_mvl/mxv
	.byte		PAN   , c_v-44
	.byte		VOL   , 46*mus_qz_656_mvl/mxv
	.byte		PRIO  , 54
	.byte		N02   , Gs3 , v100
	.byte		N02   , Ds4 , v112
	.byte	W24
	.byte		        Cn4 , v092
	.byte		N02   , Gs4 , v108
	.byte	W24
	.byte		        Ds4 , v100
	.byte		N02   , Cn5 , v112
	.byte	W12
	.byte		N01   , Gs3 , v072
	.byte		N01   , Fn4 , v084
	.byte	W06
	.byte		N02   , Cn4 , v100
	.byte		N02   , Gs4 , v112
	.byte	W12
	.byte		N01   , Ds3 , v072
	.byte		N01   , Cn4 , v084
	.byte	W06
	.byte		N02   , Cn4 , v100
	.byte		N02   , Gs4 , v112
	.byte	W12
	.byte		        Ds4 , v100
	.byte		N02   , As4 , v112
	.byte	W18
	.byte		        Ds4 , v088
	.byte		N02   , Cs5 , v100
	.byte	W18
	.byte		        Ds4 , v096
	.byte		N02   , Cn5 , v108
	.byte	W02
	.byte	FINE

/******************************************************/
	.p2align	2

mus_qz_656:
	.byte	6	/* NumTrks */
	.byte	0	/* NumBlks */
	.byte	mus_qz_656_pri	/* Priority */
	.byte	mus_qz_656_rev	/* Reverb. */
space64 4
	ptrvalue	mus_qz_656_grp

	ptrvalue	mus_qz_656_1
	ptrvalue	mus_qz_656_2
	ptrvalue	mus_qz_656_3
	ptrvalue	mus_qz_656_4
	ptrvalue	mus_qz_656_5
	ptrvalue	mus_qz_656_6

	.end
