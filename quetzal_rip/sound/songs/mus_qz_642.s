	.include "asm/macros/bit_width.inc"

	.include "MPlayDef.s"

/* Moemon Quetzal song 642, disassembled from the ROM by tools/quetzal_rip/songs.py.
   "org" comments give the ROM address of the bytes that follow. */

	.equ	mus_qz_642_grp, voicegroup_qz_songs
	.equ	mus_qz_642_pri, 0
	.equ	mus_qz_642_rev, reverb_set+0
	.equ	mus_qz_642_mvl, 127
	.equ	mus_qz_642_key, 0
	.equ	mus_qz_642_tbs, 1
	.equ	mus_qz_642_exg, 1
	.equ	mus_qz_642_cmp, 1

	.section .rodata
	.global	mus_qz_642
	.p2align	2
/* org 0x0968F278 */

/********************** Track  1 **********************/

mus_qz_642_1:
	.byte		KEYSH , mus_qz_642_key+0
	.byte		TEMPO , 160*mus_qz_642_tbs/2
	.byte		VOICE , 29
	.byte		VOL   , 59*mus_qz_642_mvl/mxv
	.byte		PAN   , c_v-4
	.byte		PRIO  , 64
	.byte		VOL   , 59*mus_qz_642_mvl/mxv
	.byte		N05   , Cn4 , v124
	.byte	W06
	.byte		        Dn4 , v120
	.byte	W06
mus_qz_642_1_B1:
	.byte		N04   , Ds4 , v127
	.byte	W12
	.byte		        Dn4 , v120
	.byte	W12
	.byte		        Cs4 , v124
	.byte	W12
	.byte		        Cn4 , v120
	.byte	W24
	.byte		N32   , Bn3 , v120 , gtp3
	.byte	W36
	.byte	W12
	.byte		N04   , Gn3 , v116
	.byte	W36
	.byte		N44   , An3 , v112 , gtp3
	.byte	W48
	.byte		N32   , As3 , v120 , gtp3
	.byte	W36
	.byte		N04   , An3 , v112
	.byte	W24
	.byte		        As3 , v120
	.byte	W12
	.byte		        An3 , v112
	.byte	W12
	.byte		        Cn5 , v124
	.byte	W12
	.byte	W12
	.byte		        Gn3 , v120
	.byte	W36
	.byte		N23   , Fn4 , v112
	.byte	W24
	.byte		N05   , Cn4 , v116
	.byte	W06
	.byte		        Dn4 , v112
	.byte	W06
	.byte		        Ds4 , v116
	.byte	W06
	.byte		        Fn4 , v112
	.byte	W06
	.byte		N32   , Fs4 , v120 , gtp3
	.byte	W36
	.byte		N04   , Ds4 , v112
	.byte	W24
	.byte		        Fs4 , v120
	.byte	W12
	.byte		        Ds4 , v112
	.byte	W12
	.byte		        Ds5 , v120
	.byte	W12
	.byte		VOL   , 46*mus_qz_642_mvl/mxv
	.byte	W12
	.byte		N04   , Fn3 , v116
	.byte	W48
	.byte		        Fs3 , v112
	.byte	W36
	.byte		VOL   , 51*mus_qz_642_mvl/mxv
	.byte	W12
	.byte		N04   , Fn3 , v116
	.byte	W48
	.byte		        Fs3 , v112
	.byte	W36
	.byte		VOL   , 59*mus_qz_642_mvl/mxv
	.byte	W12
	.byte		N04   , Fn3 , v116
	.byte	W48
	.byte		        Fs3 , v112
	.byte	W36
	.byte		N05   , Gs3 , v124
	.byte	W06
	.byte		        Fn3 , v112
	.byte	W06
	.byte		        Dn3 , v120
	.byte	W06
	.byte		        Bn2 , v112
	.byte	W06
	.byte		        An3 , v124
	.byte	W06
	.byte		        Fs3 , v112
	.byte	W06
	.byte		        Ds3 , v120
	.byte	W06
	.byte		        Cn3 , v112
	.byte	W06
	.byte		        As3 , v124
	.byte	W06
	.byte		        Gn3 , v112
	.byte	W06
	.byte		        En3 , v120
	.byte	W06
	.byte		        Cs3 , v112
	.byte	W06
	.byte		        Bn3 , v124
	.byte	W06
	.byte		        Gs3 , v112
	.byte	W06
	.byte		        Fn3 , v120
	.byte	W06
	.byte		        Dn3 , v112
	.byte	W06
	.byte	W21
	.byte		N02   , Dn4
	.byte	W03
	.byte		N05   , Ds4 , v124
	.byte	W24
	.byte	N05
	.byte	W12
	.byte		        Cn4 , v116
	.byte	W24
	.byte		N32   , Fs4 , v124 , gtp3
	.byte	W12
	.byte	W06
	.byte		VOL   , 51*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        47*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        43*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        59*mus_qz_642_mvl/mxv
	.byte		N23   , Fn4 , v116
	.byte	W06
	.byte		VOL   , 54*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        49*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        46*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        59*mus_qz_642_mvl/mxv
	.byte		N05   , Ds4 , v120
	.byte	W24
	.byte		        Cn4
	.byte	W12
	.byte		        Dn4 , v116
	.byte	W12
	.byte		        Ds4 , v120
	.byte	W12
	.byte		        Ds3 , v116
	.byte	W12
	.byte		        Ds3 , v112
	.byte	W12
	.byte		N02   , An4 , v116
	.byte	W03
	.byte		TIE   , As4 , v124
	.byte	W44
	.byte	W01
	.byte		VOL   , 57*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        52*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        48*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        43*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        39*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        34*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        31*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        27*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        23*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        20*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        16*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        14*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        11*mus_qz_642_mvl/mxv
	.byte	W02
	.byte	EOT
	.byte	W04
	.byte		VOL   , 8*mus_qz_642_mvl/mxv
	.byte	W12
	.byte		        59*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		N05   , Cn5 , v120
	.byte	W06
	.byte		        Dn5 , v116
	.byte	W06
	.byte	W21
	.byte		N02   , Dn4
	.byte	W03
	.byte		N05   , Ds4 , v124
	.byte	W24
	.byte	N05
	.byte	W12
	.byte		        Cn4 , v120
	.byte	W24
	.byte		N32   , Fs4 , v120 , gtp3
	.byte	W12
	.byte	W06
	.byte		VOL   , 54*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        49*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        46*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        59*mus_qz_642_mvl/mxv
	.byte		N23   , Fn4 , v116
	.byte	W06
	.byte		VOL   , 54*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        49*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        46*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        59*mus_qz_642_mvl/mxv
	.byte		N05   , Ds4 , v120
	.byte	W24
	.byte		        Cn4 , v124
	.byte	W12
	.byte		        Dn4 , v116
	.byte	W12
	.byte		        Ds4 , v124
	.byte	W12
	.byte		        Ds3 , v120
	.byte	W12
	.byte		        Ds3 , v116
	.byte	W12
	.byte		N02   , Bn3
	.byte	W03
	.byte		TIE   , Cn4 , v124
	.byte	W44
	.byte	W01
	.byte		VOL   , 57*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        52*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        48*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        43*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        39*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        34*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        31*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        27*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        23*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        20*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        16*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        14*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        11*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        8*mus_qz_642_mvl/mxv
	.byte	W02
	.byte	EOT
	.byte	W04
	.byte		VOL   , 59*mus_qz_642_mvl/mxv
	.byte		N05   , As3 , v120
	.byte	W06
	.byte		        An3 , v112
	.byte	W06
	.byte		        Gs3 , v120
	.byte	W06
	.byte		        Gn3 , v112
	.byte	W06
	.byte		N02   , Bn3
	.byte	W03
	.byte		N08   , Cn4 , v116
	.byte	W09
	.byte		N11   , Ds4 , v124
	.byte	W12
	.byte		        Cn4 , v120
	.byte	W12
	.byte		N23   , Fs4 , v124
	.byte	W06
	.byte		VOL   , 53*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        49*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        44*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        59*mus_qz_642_mvl/mxv
	.byte		N23   , Fn4 , v120
	.byte	W06
	.byte		VOL   , 54*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        49*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        46*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        59*mus_qz_642_mvl/mxv
	.byte		N11   , Cn4 , v124
	.byte	W12
	.byte		N02   , Dn4 , v116
	.byte	W03
	.byte		N24   , Ds4 , v124 , gtp2
	.byte	W09
	.byte		VOL   , 54*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        49*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        46*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        59*mus_qz_642_mvl/mxv
	.byte		N02   , Dn4 , v120
	.byte	W03
	.byte		        Cs4 , v112
	.byte	W03
	.byte		N32   , Cn4 , v120 , gtp3
	.byte	W18
	.byte		VOL   , 54*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        49*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        46*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        59*mus_qz_642_mvl/mxv
	.byte		N05   , Fn4 , v124
	.byte	W12
	.byte		        Fs4 , v116
	.byte	W12
	.byte		N11   , Cn4 , v120
	.byte	W12
	.byte		        Ds4 , v116
	.byte	W12
	.byte		        Cn4 , v120
	.byte	W12
	.byte		N23   , Fn4 , v124
	.byte	W06
	.byte		VOL   , 54*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        49*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        46*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        59*mus_qz_642_mvl/mxv
	.byte		N23   , Fs4 , v120
	.byte	W06
	.byte		VOL   , 54*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        49*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        46*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        59*mus_qz_642_mvl/mxv
	.byte		N11   , Cn4 , v116
	.byte	W12
	.byte		N05   , Bn3 , v124
	.byte	W12
	.byte		        Cn4 , v116
	.byte	W12
	.byte		        Dn4 , v120
	.byte	W12
	.byte		N23   , Ds4
	.byte	W06
	.byte		VOL   , 54*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        49*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        46*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        59*mus_qz_642_mvl/mxv
	.byte	W12
	.byte		N05   , Dn4 , v124
	.byte	W12
	.byte		        Cn4 , v120
	.byte	W06
	.byte		        Gn3 , v116
	.byte	W06
	.byte		N02   , Bn3 , v120
	.byte	W03
	.byte		N44   , Cn4 , v124
	.byte	W15
	.byte		VOL   , 55*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        49*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        47*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        42*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        37*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        59*mus_qz_642_mvl/mxv
	.byte		N44   , Cs4 , v120 , gtp3
	.byte	W48
	.byte		        Dn4 , v124 , gtp3
	.byte	W18
	.byte		VOL   , 55*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        49*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        47*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        42*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        37*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        59*mus_qz_642_mvl/mxv
	.byte		N44   , Ds4 , v120 , gtp3
	.byte	W18
	.byte		VOL   , 55*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        49*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        47*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        42*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        37*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        59*mus_qz_642_mvl/mxv
	.byte		N05   , En4 , v124
	.byte	W06
	.byte		        Cs4 , v108
	.byte	W06
	.byte		        As3 , v116
	.byte	W06
	.byte		        Gn3 , v112
	.byte	W06
	.byte		        Fn4 , v124
	.byte	W06
	.byte		        Dn4 , v112
	.byte	W06
	.byte		        Bn3 , v120
	.byte	W06
	.byte		        Gs3 , v112
	.byte	W06
	.byte		        Fs4 , v124
	.byte	W06
	.byte		        Ds4 , v112
	.byte	W06
	.byte		        Cn4 , v120
	.byte	W06
	.byte		        An3 , v112
	.byte	W06
	.byte		        Gn4 , v124
	.byte	W06
	.byte		        En4 , v112
	.byte	W06
	.byte		        Cs4 , v116
	.byte	W06
	.byte		        As3 , v112
	.byte	W06
	.byte		        Gs4 , v124
	.byte	W06
	.byte		        Fn4 , v108
	.byte	W06
	.byte		        Dn4 , v116
	.byte	W06
	.byte		        Bn3 , v108
	.byte	W06
	.byte		        An4 , v124
	.byte	W06
	.byte		        Fs4 , v108
	.byte	W06
	.byte		        Ds4 , v116
	.byte	W06
	.byte		        Cn4 , v108
	.byte	W06
	.byte		        As4 , v124
	.byte	W06
	.byte		        Gn4 , v108
	.byte	W06
	.byte		        En4 , v116
	.byte	W06
	.byte		        Cs4 , v108
	.byte	W06
	.byte		N11   , Bn4 , v124
	.byte	W12
	.byte		N05   , Cn5 , v120
	.byte	W06
	.byte		        Dn5 , v112
	.byte	W06
	.byte	GOTO
	 ptrvalue	mus_qz_642_1_B1
	.byte	FINE

/********************** Track  2 **********************/

mus_qz_642_2:
	.byte		KEYSH , mus_qz_642_key+0
	.byte		VOICE , 29
	.byte		VOL   , 52*mus_qz_642_mvl/mxv
	.byte		PAN   , c_v-14
	.byte		PRIO  , 62
	.byte		VOL   , 52*mus_qz_642_mvl/mxv
	.byte		N05   , Gn3 , v120
	.byte	W06
	.byte		        Gs3
	.byte	W06
mus_qz_642_2_B1:
	.byte		N04   , Cn4 , v124
	.byte	W12
	.byte		        Bn3 , v120
	.byte	W12
	.byte		        Gs3
	.byte	W12
	.byte	N04
	.byte	W24
	.byte		N32   , Fn3 , v127 , gtp3
	.byte	W36
	.byte	W12
	.byte		N03   , Ds3 , v120
	.byte	W36
	.byte		N23   , Ds3 , v116
	.byte	W24
	.byte		N11   , An2 , v120
	.byte	W12
	.byte		N05   , Ds3 , v116
	.byte	W06
	.byte		        Fn3 , v112
	.byte	W06
	.byte		N32   , Fs3 , v120 , gtp3
	.byte	W36
	.byte		N04   , Ds3 , v112
	.byte	W24
	.byte		N03   , Fs3 , v120
	.byte	W12
	.byte		        Ds3 , v116
	.byte	W12
	.byte		        Fs4 , v120
	.byte	W12
	.byte	W12
	.byte		        Ds4
	.byte	W36
	.byte		N44   , An3 , v116 , gtp3
	.byte	W48
	.byte		N32   , As3 , v120 , gtp3
	.byte	W36
	.byte		N03   , An3 , v112
	.byte	W24
	.byte		        As3 , v120
	.byte	W12
	.byte		        An3 , v112
	.byte	W12
	.byte		        Fs4 , v120
	.byte	W12
	.byte		VOL   , 41*mus_qz_642_mvl/mxv
	.byte	W12
	.byte		N03   , Bn2 , v124
	.byte	W48
	.byte		        Cn3 , v120
	.byte	W36
	.byte		VOL   , 46*mus_qz_642_mvl/mxv
	.byte	W12
	.byte		N03   , Bn2 , v124
	.byte	W48
	.byte		        Cn3 , v120
	.byte	W36
	.byte		VOL   , 52*mus_qz_642_mvl/mxv
	.byte	W12
	.byte		N03   , Bn2 , v124
	.byte	W48
	.byte		        Cn3 , v120
	.byte	W36
	.byte		N05   , Fn3 , v124
	.byte	W06
	.byte		        Dn3 , v112
	.byte	W06
	.byte		        Bn2 , v120
	.byte	W06
	.byte		        Gs2 , v112
	.byte	W06
	.byte		        Fs3 , v124
	.byte	W06
	.byte		        Ds3 , v112
	.byte	W06
	.byte		        Cn3 , v120
	.byte	W06
	.byte		        An2 , v112
	.byte	W06
	.byte		        Gn3 , v124
	.byte	W06
	.byte		        En3 , v112
	.byte	W06
	.byte		        Cs3 , v120
	.byte	W06
	.byte		        As2 , v112
	.byte	W06
	.byte		        Gs3 , v124
	.byte	W06
	.byte		        Fn3 , v112
	.byte	W06
	.byte		        Dn3 , v120
	.byte	W06
	.byte		        Bn2 , v112
	.byte	W06
	.byte	W21
	.byte		N02   , Bn3
	.byte	W03
	.byte		N05   , Cn4 , v120
	.byte	W24
	.byte	N05
	.byte	W12
	.byte		        Gn3 , v116
	.byte	W24
	.byte		N32   , Ds4 , v124 , gtp3
	.byte	W12
	.byte	W24
	.byte		N23   , Dn4 , v116
	.byte	W24
	.byte		N05   , Gn3 , v120
	.byte	W24
	.byte		        Gn3 , v116
	.byte	W12
	.byte		        An3 , v112
	.byte	W12
	.byte		        As3 , v120
	.byte	W36
	.byte		N02   , Fn4 , v112
	.byte	W03
	.byte		N08   , Fs4 , v124
	.byte	W09
	.byte		N05   , Fn4 , v120
	.byte	W12
	.byte		        Ds4 , v112
	.byte	W12
	.byte		        Cn4 , v120
	.byte	W06
	.byte		        Dn4 , v108
	.byte	W06
	.byte		        Ds4 , v116
	.byte	W06
	.byte		        Cn4 , v112
	.byte	W06
	.byte		        As3 , v120
	.byte	W06
	.byte		        Gn3 , v112
	.byte	W06
	.byte		        As3 , v116
	.byte	W06
	.byte		        Cn4 , v112
	.byte	W18
	.byte		N11   , Dn4 , v120
	.byte	W24
	.byte		        Ds4 , v116
	.byte	W24
	.byte	N05
	.byte	W06
	.byte		        Gn4 , v112
	.byte	W06
	.byte	W21
	.byte		N02   , Bn3
	.byte	W03
	.byte		N05   , Cn4 , v120
	.byte	W24
	.byte	N05
	.byte	W12
	.byte		        Gn3 , v112
	.byte	W24
	.byte		N32   , Ds4 , v124 , gtp3
	.byte	W12
	.byte	W24
	.byte		N23   , Dn4 , v116
	.byte	W24
	.byte		N05   , As3 , v120
	.byte	W24
	.byte		        Gn3
	.byte	W12
	.byte		        An3 , v116
	.byte	W12
	.byte		        As3 , v120
	.byte	W36
	.byte		N02   , Fs3 , v112
	.byte	W03
	.byte		N08   , Gn3 , v124
	.byte	W09
	.byte		N05   , Fn3 , v120
	.byte	W12
	.byte		        Ds3 , v112
	.byte	W12
	.byte		        Cn3 , v116
	.byte	W06
	.byte		        Dn3 , v112
	.byte	W06
	.byte		        Ds3 , v116
	.byte	W06
	.byte		        Cn3 , v112
	.byte	W06
	.byte		        As2 , v116
	.byte	W06
	.byte		        Gn2 , v108
	.byte	W06
	.byte		        As2 , v112
	.byte	W06
	.byte		        Cn3 , v108
	.byte	W18
	.byte		N11   , Dn3 , v116
	.byte	W24
	.byte		        Ds3 , v120
	.byte	W12
	.byte		N05   , Fn3 , v116
	.byte	W06
	.byte		        En3 , v108
	.byte	W06
	.byte		        Ds3 , v112
	.byte	W06
	.byte		        Dn3 , v108
	.byte	W06
	.byte	W12
	.byte		        Fn3 , v116
	.byte	W48
	.byte		        Fs3 , v112
	.byte	W36
mus_qz_642_2_000:
	.byte	W12
	.byte		N05   , Fn3 , v120
	.byte	W48
	.byte		        Fs3 , v112
	.byte	W36
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_642_2_000
	.byte	W12
	.byte		N05   , Fn3 , v116
	.byte	W48
	.byte		        Fs3 , v120
	.byte	W12
	.byte	N05
	.byte	W12
	.byte		        Ds3 , v112
	.byte	W06
	.byte		        Cn3 , v108
	.byte	W06
	.byte		        Fs3 , v056
	.byte	W06
	.byte		        An3 , v060
	.byte	W06
	.byte		        Fs3 , v064
	.byte	W06
	.byte		        An3 , v068
	.byte	W06
	.byte		        Fs3
	.byte	W06
	.byte		        An3 , v072
	.byte	W06
	.byte		        Fs3 , v076
	.byte	W06
	.byte		        An3 , v080
	.byte	W06
	.byte		        Gn3 , v084
	.byte	W06
	.byte		        As3 , v088
	.byte	W06
	.byte		        Gn3 , v092
	.byte	W06
	.byte		        As3 , v096
	.byte	W06
	.byte		        Gn3 , v100
	.byte	W06
	.byte		        As3 , v104
	.byte	W06
	.byte		        Gn3 , v108
	.byte	W06
	.byte		        As3 , v112
	.byte	W06
	.byte		        Gs3 , v056
	.byte	W06
	.byte		        Bn3 , v060
	.byte	W06
	.byte		        Gs3 , v064
	.byte	W06
	.byte		        Bn3 , v068
	.byte	W06
	.byte		        Gs3
	.byte	W06
	.byte		        Bn3 , v072
	.byte	W06
	.byte		        Gs3 , v076
	.byte	W06
	.byte		        Bn3 , v080
	.byte	W06
	.byte		        An3 , v084
	.byte	W06
	.byte		        Cn4 , v088
	.byte	W06
	.byte		        An3 , v092
	.byte	W06
	.byte		        Cn4 , v096
	.byte	W06
	.byte		        An3 , v100
	.byte	W06
	.byte		        Cn4 , v104
	.byte	W06
	.byte		        An3 , v108
	.byte	W06
	.byte		        Cn4 , v112
	.byte	W06
	.byte		        Cs4 , v116
	.byte	W06
	.byte		        As3 , v100
	.byte	W06
	.byte		        Gn3 , v108
	.byte	W06
	.byte		        En3 , v104
	.byte	W06
	.byte		        Dn4 , v116
	.byte	W06
	.byte		        Bn3 , v104
	.byte	W06
	.byte		        Gs3 , v112
	.byte	W06
	.byte		        Fn3 , v104
	.byte	W06
	.byte		        Ds4 , v116
	.byte	W06
	.byte		        Cn4 , v104
	.byte	W06
	.byte		        An3 , v112
	.byte	W06
	.byte		        Fs3 , v104
	.byte	W06
	.byte		        En4 , v116
	.byte	W06
	.byte		        Cs4 , v104
	.byte	W06
	.byte		        As3 , v108
	.byte	W06
	.byte		        Gn3 , v104
	.byte	W06
	.byte		        Fn4 , v116
	.byte	W06
	.byte		        Dn4 , v100
	.byte	W06
	.byte		        Bn3 , v108
	.byte	W06
	.byte		        Gs3 , v100
	.byte	W06
	.byte		        Fs4 , v116
	.byte	W06
	.byte		        Ds4 , v100
	.byte	W06
	.byte		        Cn4 , v108
	.byte	W06
	.byte		        An3 , v100
	.byte	W06
	.byte		        Gn4 , v116
	.byte	W06
	.byte		        En4 , v100
	.byte	W06
	.byte		        Cs4 , v108
	.byte	W06
	.byte		        As3 , v100
	.byte	W06
	.byte		N11   , Fn4 , v120
	.byte	W12
	.byte		N05   , Cn4
	.byte	W06
	.byte		        Dn4
	.byte	W06
	.byte	GOTO
	 ptrvalue	mus_qz_642_2_B1
	.byte	FINE

/********************** Track  3 **********************/

mus_qz_642_3:
	.byte		KEYSH , mus_qz_642_key+0
	.byte		VOICE , 29
	.byte		VOL   , 49*mus_qz_642_mvl/mxv
	.byte		PAN   , c_v+24
	.byte		PRIO  , 60
	.byte		VOL   , 49*mus_qz_642_mvl/mxv
	.byte		N05   , Ds3 , v100
	.byte	W06
	.byte		        Fn3 , v092
	.byte	W06
mus_qz_642_3_B1:
	.byte		N04   , Gs3 , v104
	.byte	W12
	.byte		        Gs3 , v100
	.byte	W12
	.byte		        Fn3 , v104
	.byte	W12
	.byte		        Fn3 , v100
	.byte	W24
	.byte		N32   , Dn3 , v108 , gtp3
	.byte	W36
	.byte	W12
	.byte		N05   , Cn3 , v112
	.byte	W36
	.byte		N23   , Cn3 , v108
	.byte	W48
	.byte		N32   , Cn3 , v116 , gtp3
	.byte	W36
	.byte		N05   , Cn3 , v108
	.byte	W24
	.byte		        Cn3 , v112
	.byte	W12
	.byte		        Cn3 , v108
	.byte	W12
	.byte		        Cn4 , v112
	.byte	W12
	.byte	W12
	.byte		        Cn3 , v116
	.byte	W36
	.byte	N23
	.byte	W24
	.byte		N05   , Gn3 , v108
	.byte	W06
	.byte		        An3 , v104
	.byte	W06
	.byte		        As3 , v108
	.byte	W06
	.byte		        Cn4 , v104
	.byte	W06
	.byte		N32   , Ds3 , v112 , gtp3
	.byte	W36
	.byte		N05   , Ds3 , v108
	.byte	W24
	.byte		        Ds3 , v112
	.byte	W12
	.byte		        Ds3 , v108
	.byte	W12
	.byte		        As3 , v112
	.byte	W12
	.byte		VOL   , 39*mus_qz_642_mvl/mxv
	.byte	W12
	.byte		N03   , Gs2
	.byte	W48
	.byte		        An2 , v108
	.byte	W36
	.byte		VOL   , 43*mus_qz_642_mvl/mxv
	.byte	W12
	.byte		N03   , Gs2 , v116
	.byte	W48
	.byte		        An2 , v108
	.byte	W36
	.byte		VOL   , 49*mus_qz_642_mvl/mxv
	.byte	W12
	.byte		N03   , Gs2 , v112
	.byte	W48
	.byte		        An2 , v108
	.byte	W36
	.byte		N04   , Dn3 , v112
	.byte	W06
	.byte		        Bn2 , v100
	.byte	W06
	.byte		        Gs2 , v108
	.byte	W06
	.byte		        Fn2 , v100
	.byte	W06
	.byte		        Ds3 , v112
	.byte	W06
	.byte		        Cn3 , v104
	.byte	W06
	.byte		        An2 , v108
	.byte	W06
	.byte		        Fs2 , v100
	.byte	W06
	.byte		        En3 , v112
	.byte	W06
	.byte		        Cs3 , v104
	.byte	W06
	.byte		        As2 , v112
	.byte	W06
	.byte		        Gn2 , v108
	.byte	W06
	.byte		        Fn3 , v112
	.byte	W06
	.byte		        Dn3 , v104
	.byte	W06
	.byte		        Bn2 , v112
	.byte	W06
	.byte		        Gs2 , v108
	.byte	W06
	.byte	W21
	.byte		N02   , Fn3 , v112
	.byte	W03
	.byte		N05   , Gn3 , v120
	.byte	W24
	.byte	N05
	.byte	W12
	.byte		        Ds3 , v116
	.byte	W24
	.byte		N32   , Cn4 , v124 , gtp3
	.byte	W12
	.byte	W24
	.byte		N23   , As3 , v116
	.byte	W24
	.byte		N05   , Ds3 , v120
	.byte	W24
	.byte		        Ds3 , v116
	.byte	W12
	.byte		        Fn3 , v112
	.byte	W12
	.byte		        Gn3 , v120
	.byte	W36
	.byte		N02   , Dn4 , v112
	.byte	W03
	.byte		N08   , Ds4 , v124
	.byte	W09
	.byte		N05   , Cn4 , v120
	.byte	W12
	.byte		        Cn4 , v112
	.byte	W12
	.byte		        Gn3 , v120
	.byte	W06
	.byte		        Gn3 , v108
	.byte	W06
	.byte		        As3 , v116
	.byte	W06
	.byte		        Gn3 , v112
	.byte	W06
	.byte		        Gn3 , v120
	.byte	W06
	.byte		        Ds3 , v112
	.byte	W06
	.byte		        Gn3 , v116
	.byte	W06
	.byte		        Gn3 , v112
	.byte	W18
	.byte		N11   , Gn3 , v120
	.byte	W24
	.byte		        Gn3 , v116
	.byte	W24
	.byte	N05
	.byte	W06
	.byte		        Ds4 , v112
	.byte	W06
	.byte	W21
	.byte		N02   , Fs3
	.byte	W03
	.byte		N05   , Gn3 , v120
	.byte	W24
	.byte	N05
	.byte	W12
	.byte		        Ds3 , v112
	.byte	W24
	.byte		N32   , Cn4 , v124 , gtp3
	.byte	W12
	.byte	W24
	.byte		N23   , As3 , v116
	.byte	W24
	.byte		N05   , Gn3 , v120
	.byte	W24
	.byte		        Ds3
	.byte	W12
	.byte		        Fn3 , v116
	.byte	W12
	.byte		        Gn3 , v120
	.byte	W36
	.byte		N02   , Cs3 , v112
	.byte	W03
	.byte		N08   , Ds3 , v124
	.byte	W09
	.byte		N05   , Cn3 , v120
	.byte	W12
	.byte		        As2 , v112
	.byte	W12
	.byte		        Gn2 , v116
	.byte	W06
	.byte		        Gn2 , v112
	.byte	W06
	.byte		        Cn3 , v116
	.byte	W06
	.byte		        Gn2 , v112
	.byte	W06
	.byte		        Gn2 , v116
	.byte	W06
	.byte		        Dn2 , v108
	.byte	W06
	.byte		        Gn2 , v112
	.byte	W06
	.byte		        Gn2 , v108
	.byte	W18
	.byte		N11   , Gn2 , v116
	.byte	W24
	.byte		        Cn3 , v120
	.byte	W12
	.byte		N05   , Cn3 , v116
	.byte	W06
	.byte		        Cn3 , v108
	.byte	W06
	.byte		        As2 , v112
	.byte	W06
	.byte		        An2 , v108
	.byte	W06
	.byte	W12
	.byte		        Cn3 , v116
	.byte	W48
	.byte		        Cn3 , v112
	.byte	W36
mus_qz_642_3_000:
	.byte	W12
	.byte		N05   , Cn3 , v120
	.byte	W48
	.byte		        Cn3 , v112
	.byte	W36
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_642_3_000
	.byte	W12
	.byte		N05   , Cn3 , v116
	.byte	W48
	.byte		        Cn3 , v120
	.byte	W12
	.byte	N05
	.byte	W12
	.byte		        Cn3 , v112
	.byte	W06
	.byte		        Gn2 , v108
	.byte	W06
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W72
	.byte		N11   , Cn4 , v120
	.byte	W12
	.byte		N05   , Gn3
	.byte	W06
	.byte		        An3
	.byte	W06
	.byte	GOTO
	 ptrvalue	mus_qz_642_3_B1
	.byte	FINE

/********************** Track  4 **********************/

mus_qz_642_4:
	.byte		KEYSH , mus_qz_642_key+0
	.byte		VOICE , 21
	.byte		VOL   , 49*mus_qz_642_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		BENDR , 6
	.byte		PRIO  , 54
	.byte		VOL   , 49*mus_qz_642_mvl/mxv
	.byte		N04   , Bn0 , v100
	.byte	W06
	.byte		        Cn1 , v096
	.byte	W06
mus_qz_642_4_B1:
	.byte		N10   , Fn0 , v120
	.byte	W60
	.byte		N32   , Gn0 , v127 , gtp3
	.byte	W12
	.byte		BEND  , c_v+16
	.byte	W06
	.byte		        c_v+0
	.byte	W06
	.byte		        c_v+17
	.byte	W06
	.byte		        c_v+0
	.byte	W06
	.byte		N11   , Cn1 , v104
	.byte	W12
	.byte		N01   , Cn2 , v096
	.byte	W12
	.byte		N03   , Cn1 , v104
	.byte	W12
	.byte		        Cn1 , v100
	.byte	W12
	.byte		N05   , Cn2 , v096
	.byte	W12
	.byte		N11   , Cn1 , v104
	.byte	W12
	.byte		N05   , Ds1
	.byte	W12
	.byte		        Gn1 , v100
	.byte	W12
	.byte		        Cn1 , v104
	.byte	W12
	.byte		        Cn1 , v096
	.byte	W12
	.byte		        Cn1 , v100
	.byte	W12
	.byte		N11   , Bn0 , v104
	.byte	W12
	.byte		N03   , Cn1
	.byte	W12
	.byte		        Cn1 , v100
	.byte	W12
	.byte		N11   , Fs1 , v104
	.byte	W12
	.byte		N03   , An1 , v100
	.byte	W12
	.byte		N11   , Cn1 , v108
	.byte	W12
	.byte		N05   , Cn2 , v100
	.byte	W12
	.byte		        Cn1 , v104
	.byte	W12
	.byte		        Cn1 , v100
	.byte	W12
	.byte		        Cn2 , v104
	.byte	W12
	.byte		        Cn1
	.byte	W12
	.byte		N11   , Ds1 , v100
	.byte	W12
	.byte		N05   , Gn1 , v104
	.byte	W12
	.byte		        Cn1
	.byte	W12
	.byte		        Cn1 , v100
	.byte	W12
	.byte	N05
	.byte	W12
	.byte		N11   , Bn0 , v104
	.byte	W12
	.byte		N05   , Cn1
	.byte	W12
	.byte		        Cn1 , v100
	.byte	W12
	.byte		N11   , Fs1 , v108
	.byte	W12
	.byte		N05   , An1 , v104
	.byte	W12
	.byte		VOL   , 39*mus_qz_642_mvl/mxv
	.byte		N11   , Gs0 , v108
	.byte	W12
	.byte		N05   , Gs1 , v100
	.byte	W12
	.byte		        Gs0 , v104
	.byte	W12
	.byte		        Gs0 , v100
	.byte	W12
	.byte		N11   , An0 , v104
	.byte	W12
	.byte		N05   , An1 , v108
	.byte	W12
	.byte		        Gs1 , v120
	.byte	W06
	.byte		        Fs1 , v116
	.byte	W06
	.byte		        Ds1 , v120
	.byte	W06
	.byte		        Cn1 , v116
	.byte	W06
	.byte		VOL   , 44*mus_qz_642_mvl/mxv
	.byte		N05   , Gs0 , v104
	.byte	W12
	.byte		N11   , Gs1 , v100
	.byte	W12
	.byte		N05   , Gs0 , v104
	.byte	W12
	.byte		        Gs0 , v100
	.byte	W12
	.byte		N02   , An0 , v108
	.byte	W06
	.byte		        An0 , v104
	.byte	W06
	.byte		N23   , Fs1 , v124
	.byte	W24
	.byte		N11   , Cn1 , v112
	.byte	W12
	.byte		VOL   , 49*mus_qz_642_mvl/mxv
	.byte		N11   , Gs0 , v108
	.byte	W12
	.byte		N05   , Gs1
	.byte	W12
	.byte		        Gs0 , v100
	.byte	W12
	.byte		        Gs0 , v108
	.byte	W12
	.byte		        An0 , v112
	.byte	W12
	.byte		N11   , An1 , v108
	.byte	W12
	.byte		N05   , Gs1
	.byte	W06
	.byte		        Fs1 , v100
	.byte	W06
	.byte		        Ds1 , v104
	.byte	W06
	.byte		        Cn1 , v100
	.byte	W06
	.byte		N10   , Gs0 , v108
	.byte	W12
	.byte		N04   , Fn1 , v104
	.byte	W12
	.byte		N09   , An0 , v108
	.byte	W12
	.byte		N04   , Fs1 , v104
	.byte	W12
	.byte		N09   , As0 , v108
	.byte	W12
	.byte		N04   , Gn1 , v104
	.byte	W12
	.byte		N09   , Bn0 , v108
	.byte	W12
	.byte		N04   , Gs1 , v104
	.byte	W12
	.byte	W24
	.byte		N05   , Cn1 , v088
	.byte	W12
	.byte	N05
	.byte	W12
	.byte		        Cn2
	.byte	W12
	.byte		N06   , Cn1
	.byte	W12
	.byte		        Ds1
	.byte	W12
	.byte		        Gn1
	.byte	W12
mus_qz_642_4_000:
	.byte		N06   , Cn1 , v088
	.byte	W12
	.byte	N06
	.byte	W12
	.byte	N06
	.byte	W12
	.byte	N06
	.byte	W12
	.byte	N06
	.byte	W12
	.byte	N06
	.byte	W12
	.byte		        Fs1
	.byte	W12
	.byte		        An1
	.byte	W12
	.byte	PEND
	.byte		N11   , Cn1
	.byte	W12
	.byte		N06   , Cn2
	.byte	W12
	.byte		        Cn1
	.byte	W12
	.byte	N06
	.byte	W12
	.byte		        Cn2
	.byte	W12
	.byte		        Cn1
	.byte	W12
	.byte		        Ds1
	.byte	W12
	.byte		        Gn1
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_642_4_000
mus_qz_642_4_001:
	.byte		N06   , Cn1 , v088
	.byte	W12
	.byte		        Cn2
	.byte	W12
	.byte		        Cn1
	.byte	W12
	.byte	N06
	.byte	W12
	.byte		        Cn2
	.byte	W12
	.byte		        Cn1
	.byte	W12
	.byte		        Ds1
	.byte	W12
	.byte		        Gn1
	.byte	W12
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_642_4_000
	.byte	PATT
	 ptrvalue	mus_qz_642_4_001
	.byte		N06   , Cn1 , v088
	.byte	W12
	.byte	N06
	.byte	W12
	.byte	N06
	.byte	W12
	.byte	N06
	.byte	W12
	.byte	N06
	.byte	W12
	.byte	N06
	.byte	W12
	.byte		        Dn1
	.byte	W06
	.byte		        Ds1
	.byte	W06
	.byte		        En1
	.byte	W06
	.byte		        Fn1
	.byte	W06
mus_qz_642_4_002:
	.byte		N06   , Gs0 , v088
	.byte	W12
	.byte		        Gs1
	.byte	W12
	.byte		        Gs0
	.byte	W12
	.byte	N06
	.byte	W12
	.byte		        An0
	.byte	W12
	.byte		        An1
	.byte	W12
	.byte		        Gs1
	.byte	W06
	.byte		        Fs1
	.byte	W06
	.byte		        Ds1
	.byte	W06
	.byte		        Cn1
	.byte	W06
	.byte	PEND
mus_qz_642_4_003:
	.byte		N06   , Gs0 , v088
	.byte	W12
	.byte		        Gs1
	.byte	W12
	.byte		        Gs0
	.byte	W12
	.byte	N06
	.byte	W12
	.byte		        An0
	.byte	W12
	.byte		        An1
	.byte	W12
	.byte		        Cn1
	.byte	W12
	.byte		        Fs1
	.byte	W12
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_642_4_002
	.byte	PATT
	 ptrvalue	mus_qz_642_4_003
	.byte		N06   , Cn1 , v088
	.byte	W12
	.byte		        Cn2
	.byte	W12
	.byte		        Cn1
	.byte	W12
	.byte	N06
	.byte	W12
	.byte		        Cs1
	.byte	W12
	.byte		        Cs2
	.byte	W12
	.byte		        As1
	.byte	W06
	.byte		        Gn1
	.byte	W06
	.byte		        En1
	.byte	W06
	.byte		        Cs1
	.byte	W06
	.byte		        Dn1
	.byte	W12
	.byte		        Dn2
	.byte	W12
	.byte		        Dn1
	.byte	W12
	.byte	N06
	.byte	W12
	.byte		        Ds1
	.byte	W12
	.byte		        Ds2
	.byte	W12
	.byte		        Cn2
	.byte	W06
	.byte		        An1
	.byte	W06
	.byte		        Fs1
	.byte	W06
	.byte		        Ds1
	.byte	W06
	.byte		        En1
	.byte	W12
	.byte		        Cs2
	.byte	W12
	.byte		        Fn1
	.byte	W12
	.byte		        Dn2
	.byte	W12
	.byte		        Fs1
	.byte	W12
	.byte		        Ds2
	.byte	W12
	.byte		        Gn1
	.byte	W12
	.byte		        En2
	.byte	W12
	.byte		        Gs1
	.byte	W12
	.byte		        Fn2
	.byte	W12
	.byte		        An1
	.byte	W12
	.byte		        Fs2
	.byte	W12
	.byte		        As1
	.byte	W12
	.byte		        Gn2
	.byte	W12
	.byte		N12   , Bn1
	.byte	W12
	.byte		N06   , Gs2
	.byte	W12
	.byte	GOTO
	 ptrvalue	mus_qz_642_4_B1
	.byte	FINE

/********************** Track  5 **********************/

mus_qz_642_5:
	.byte		KEYSH , mus_qz_642_key+0
	.byte		VOICE , 17
	.byte		VOL   , 40*mus_qz_642_mvl/mxv
	.byte		PAN   , c_v-36
	.byte		BENDR , 6
	.byte		VOL   , 35*mus_qz_642_mvl/mxv
	.byte		PRIO  , 50
	.byte		N05   , Cn2 , v100
	.byte		N05   , Cn3 , v112
	.byte	W06
	.byte		        Dn2 , v096
	.byte		N05   , Dn3 , v108
	.byte	W06
mus_qz_642_5_B1:
	.byte		N11   , Ds2 , v108
	.byte		N11   , Ds3 , v116
	.byte	W12
	.byte		N04   , Dn2 , v100
	.byte		N04   , Dn3 , v108
	.byte	W12
	.byte		N11   , Cs2 , v104
	.byte		N11   , Cs3 , v116
	.byte	W12
	.byte		N04   , Cn2 , v100
	.byte		N04   , Cn3 , v108
	.byte	W24
	.byte		N32   , Gn4 , v127
	.byte	W06
	.byte		BEND  , c_v+14
	.byte	W06
	.byte		        c_v+0
	.byte	W06
	.byte		        c_v+17
	.byte	W06
	.byte		        c_v+0
	.byte	W06
	.byte		        c_v+14
	.byte	W06
	.byte		        c_v+0
	.byte	W12
	.byte		N11   , Cn2 , v088
	.byte		N10   , Cn3 , v080
	.byte	W12
	.byte		N01   , Cn2 , v072
	.byte	W12
	.byte		        Cn2 , v068
	.byte	W12
	.byte		N11   , Cn3 , v080
	.byte	W03
	.byte		BEND  , c_v+8
	.byte	W03
	.byte		        c_v+0
	.byte	W06
	.byte		N11   , Cn2 , v088
	.byte	W12
	.byte		N05   , Ds2
	.byte	W12
	.byte		N04   , Gn2
	.byte	W12
	.byte		N11   , Cn2
	.byte	W12
	.byte		N01   , Cn2 , v072
	.byte	W12
	.byte		        Cn2 , v068
	.byte	W12
	.byte		N11   , Bn1 , v088
	.byte	W12
	.byte		        Cn2
	.byte	W12
	.byte		N02   , Cn2 , v096
	.byte	W12
	.byte		N11   , Fs2 , v092
	.byte	W03
	.byte		BEND  , c_v+5
	.byte	W03
	.byte		        c_v+0
	.byte	W06
	.byte		        c_v+10
	.byte		N04   , An2 , v096
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W09
	.byte		N05   , Cn2 , v088
	.byte	W12
	.byte		N11   , Cn3 , v080
	.byte	W12
	.byte		N01   , Cn2 , v088
	.byte	W12
	.byte		        Cn2 , v076
	.byte	W12
	.byte		BEND  , c_v+14
	.byte		N11   , Gn4 , v064
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W09
	.byte		N02   , Ds4 , v096
	.byte	W12
	.byte		N05   , Ds2 , v088
	.byte	W12
	.byte		        Gn2
	.byte	W12
	.byte		N11   , Cn2
	.byte	W12
	.byte		N01   , Cn3 , v112
	.byte	W12
	.byte		N03   , Cn2 , v088
	.byte	W12
	.byte		N11   , Bn1
	.byte	W12
	.byte		        Cn2
	.byte	W12
	.byte		BEND  , c_v+14
	.byte		N05   , Cn3 , v084
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W09
	.byte		N11   , Fs2 , v088
	.byte	W12
	.byte		N02   , An2
	.byte	W06
	.byte		        An2 , v076
	.byte	W06
	.byte		VOL   , 31*mus_qz_642_mvl/mxv
	.byte		N11   , Gs1 , v100
	.byte	W12
	.byte		N01   , Gs2
	.byte	W12
	.byte		        Gs1
	.byte	W12
	.byte	N01
	.byte	W12
	.byte		N11   , An1
	.byte	W12
	.byte		N32   , An4 , v076 , gtp3
	.byte	W18
	.byte		VOL   , 22*mus_qz_642_mvl/mxv
	.byte	W02
	.byte		        19*mus_qz_642_mvl/mxv
	.byte	W04
	.byte		        16*mus_qz_642_mvl/mxv
	.byte	W02
	.byte		        11*mus_qz_642_mvl/mxv
	.byte	W03
	.byte		        10*mus_qz_642_mvl/mxv
	.byte	W01
	.byte		BEND  , c_v+16
	.byte	W02
	.byte		        c_v+28
	.byte	W04
	.byte		VOL   , 35*mus_qz_642_mvl/mxv
	.byte		BEND  , c_v+0
	.byte		N11   , Gs1 , v100
	.byte	W12
	.byte		N01   , Gs2
	.byte	W12
	.byte		        Gs1
	.byte	W12
	.byte	N01
	.byte	W12
	.byte		BEND  , c_v+0
	.byte		N02   , An1
	.byte	W06
	.byte		        An1 , v088
	.byte	W06
	.byte		N32   , Ds4 , v088 , gtp3
	.byte	W06
	.byte		BEND  , c_v+4
	.byte	W06
	.byte		        c_v+0
	.byte	W06
	.byte		        c_v+4
	.byte	W06
	.byte		        c_v+0
	.byte	W03
	.byte		VOL   , 28*mus_qz_642_mvl/mxv
	.byte	W02
	.byte		        22*mus_qz_642_mvl/mxv
	.byte	W01
	.byte		BEND  , c_v+4
	.byte	W02
	.byte		VOL   , 19*mus_qz_642_mvl/mxv
	.byte	W04
	.byte		PAN   , c_v+29
	.byte		VOL   , 35*mus_qz_642_mvl/mxv
	.byte		BEND  , c_v+0
	.byte		N11   , Gs1 , v100
	.byte	W12
	.byte		N01   , Gs2
	.byte	W12
	.byte		        Gs1
	.byte	W12
	.byte	N02
	.byte	W12
	.byte		        An1
	.byte	W12
	.byte		PAN   , c_v+38
	.byte		N32   , An4 , v092 , gtp3
	.byte	W06
	.byte		BEND  , c_v+3
	.byte	W06
	.byte		        c_v+0
	.byte	W09
	.byte		        c_v-5
	.byte	W03
	.byte		        c_v-13
	.byte	W02
	.byte		        c_v-23
	.byte	W01
	.byte		VOL   , 23*mus_qz_642_mvl/mxv
	.byte		PAN   , c_v+23
	.byte	W02
	.byte		BEND  , c_v-40
	.byte	W01
	.byte		VOL   , 16*mus_qz_642_mvl/mxv
	.byte		PAN   , c_v-22
	.byte	W02
	.byte		BEND  , c_v-47
	.byte	W01
	.byte		VOL   , 9*mus_qz_642_mvl/mxv
	.byte		PAN   , c_v-34
	.byte	W03
	.byte		        c_v-36
	.byte		VOL   , 35*mus_qz_642_mvl/mxv
	.byte		BEND  , c_v+0
	.byte		N01   , Fn1 , v088
	.byte	W06
	.byte		        Fn1 , v076
	.byte	W06
	.byte		N11   , Fn1 , v088
	.byte	W12
	.byte		N01   , Fs1
	.byte	W06
	.byte		        Fs1 , v076
	.byte	W06
	.byte		N10   , Fs1 , v088
	.byte	W12
	.byte		N01   , Gn1
	.byte	W06
	.byte		        Gn1 , v076
	.byte	W06
	.byte		N11   , Gn1 , v088
	.byte	W12
	.byte		N01   , Gs1
	.byte	W06
	.byte		        Gs1 , v076
	.byte	W06
	.byte		N11   , Gs1 , v088
	.byte	W12
	.byte		BEND  , c_v+0
	.byte		N01   , Cn2 , v072
	.byte		N01   , Gn2 , v080
	.byte	W06
	.byte		        Cn2 , v064
	.byte		N01   , Gn2 , v072
	.byte	W06
	.byte		        Cn2
	.byte		N01   , Gn2 , v080
	.byte	W06
	.byte		        Cn2 , v064
	.byte		N01   , Gn2 , v072
	.byte	W06
	.byte		N11   , Cn2 , v088
	.byte		N11   , Cn3 , v080
	.byte	W12
	.byte		N01   , Cn2 , v072
	.byte	W06
	.byte		        Cn2 , v064
	.byte	W06
	.byte		N11   , Cn3 , v080
	.byte	W03
	.byte		BEND  , c_v+8
	.byte	W03
	.byte		        c_v+0
	.byte	W06
	.byte		N11   , Cn2 , v088
	.byte	W12
	.byte		N05   , Ds2
	.byte	W12
	.byte		N04   , Gn2
	.byte	W12
	.byte		N11   , Cn2
	.byte	W12
	.byte		N01   , Cn2 , v072
	.byte	W12
	.byte		        Cn2 , v064
	.byte	W12
	.byte		N11   , Bn1 , v088
	.byte	W12
	.byte		        Cn2
	.byte	W12
	.byte		N02   , Cn2 , v096
	.byte	W12
	.byte		N11   , Fs2 , v092
	.byte	W03
	.byte		BEND  , c_v+5
	.byte	W03
	.byte		        c_v+0
	.byte	W06
	.byte		        c_v+10
	.byte		N04   , An2 , v096
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W09
	.byte		N05   , Cn2 , v088
	.byte	W12
	.byte		N11   , Cn3 , v080
	.byte	W12
	.byte		N01   , Cn2 , v088
	.byte	W12
	.byte		        Cn2 , v080
	.byte	W12
	.byte		BEND  , c_v+14
	.byte		N11   , Gn4 , v064
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W09
	.byte		N02   , Ds4 , v096
	.byte	W12
	.byte		N05   , Ds2 , v088
	.byte	W12
	.byte		        Gn2
	.byte	W12
mus_qz_642_5_000:
	.byte		N11   , Cn2 , v088
	.byte	W12
	.byte		N01   , Cn3 , v112
	.byte	W12
	.byte		N03   , Cn2 , v088
	.byte	W12
	.byte		N11   , Bn1
	.byte	W12
	.byte		        Cn2
	.byte	W12
	.byte		BEND  , c_v+14
	.byte		N05   , Cn3 , v084
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W09
	.byte		N11   , Fs2 , v088
	.byte	W12
	.byte		N02   , An2
	.byte	W06
	.byte		N03   , An2 , v084
	.byte	W06
	.byte	PEND
	.byte		BEND  , c_v+0
	.byte	W12
	.byte		N11   , Cn2 , v088
	.byte		N10   , Cn3 , v080
	.byte	W12
	.byte		N01   , Cn2 , v072
	.byte	W12
	.byte	N01
	.byte	W12
	.byte		N11   , Cn3 , v080
	.byte	W03
	.byte		BEND  , c_v+8
	.byte	W03
	.byte		        c_v+0
	.byte	W06
	.byte		N11   , Cn2 , v088
	.byte	W12
	.byte		N05   , Ds2
	.byte	W12
	.byte		N04   , Gn2
	.byte	W12
	.byte		N11   , Cn2
	.byte	W12
	.byte		N01   , Cn2 , v072
	.byte	W12
	.byte	N01
	.byte	W12
	.byte		N11   , Bn1 , v088
	.byte	W12
	.byte		        Cn2
	.byte	W12
	.byte		N02   , Cn2 , v096
	.byte	W12
	.byte		N11   , Fs2 , v092
	.byte	W03
	.byte		BEND  , c_v+5
	.byte	W03
	.byte		        c_v+0
	.byte	W06
	.byte		        c_v+10
	.byte		N04   , An2 , v096
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W09
	.byte		N05   , Cn2 , v088
	.byte	W12
	.byte		N11   , Cn3 , v080
	.byte	W12
	.byte		N01   , Cn2 , v088
	.byte	W12
	.byte	N01
	.byte	W12
	.byte		BEND  , c_v+14
	.byte		N11   , Gn4 , v064
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W09
	.byte		N02   , Ds4 , v096
	.byte	W12
	.byte		N05   , Ds2 , v088
	.byte	W12
	.byte		        Gn2
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_642_5_000
	.byte		N11   , Gs1 , v100
	.byte	W12
	.byte		N01   , Gs2
	.byte	W12
	.byte		        Gs1
	.byte	W12
	.byte	N01
	.byte	W12
	.byte		N11   , An1
	.byte	W12
	.byte		N32   , An4 , v104 , gtp3
	.byte	W18
	.byte		VOL   , 27*mus_qz_642_mvl/mxv
	.byte	W02
	.byte		        23*mus_qz_642_mvl/mxv
	.byte	W04
	.byte		        20*mus_qz_642_mvl/mxv
	.byte	W02
	.byte		        15*mus_qz_642_mvl/mxv
	.byte	W03
	.byte		        13*mus_qz_642_mvl/mxv
	.byte	W01
	.byte		BEND  , c_v+16
	.byte	W02
	.byte		        c_v+28
	.byte	W04
mus_qz_642_5_001:
	.byte		VOL   , 35*mus_qz_642_mvl/mxv
	.byte		BEND  , c_v+0
	.byte		N11   , Gs1 , v100
	.byte	W12
	.byte		N01   , Gs2
	.byte	W12
	.byte		        Gs1
	.byte	W12
	.byte	N01
	.byte	W12
	.byte		BEND  , c_v+0
	.byte		N02   , An1
	.byte	W06
	.byte	N02
	.byte	W06
	.byte		N32   , Ds4 , v104 , gtp3
	.byte	W06
	.byte		BEND  , c_v+4
	.byte	W06
	.byte		        c_v+0
	.byte	W06
	.byte		        c_v+4
	.byte	W06
	.byte		        c_v+0
	.byte	W03
	.byte		VOL   , 29*mus_qz_642_mvl/mxv
	.byte	W02
	.byte		        23*mus_qz_642_mvl/mxv
	.byte	W01
	.byte		BEND  , c_v+4
	.byte	W02
	.byte		VOL   , 19*mus_qz_642_mvl/mxv
	.byte	W04
	.byte	PEND
	.byte		        35*mus_qz_642_mvl/mxv
	.byte		BEND  , c_v+0
	.byte		N11   , Gs1 , v100
	.byte	W12
	.byte		N01   , Gs2
	.byte	W12
	.byte		        Gs1
	.byte	W12
	.byte	N01
	.byte	W12
	.byte		N11   , An1
	.byte	W12
	.byte		N32   , An4 , v104 , gtp3
	.byte	W18
	.byte		VOL   , 27*mus_qz_642_mvl/mxv
	.byte	W02
	.byte		        23*mus_qz_642_mvl/mxv
	.byte	W04
	.byte		        20*mus_qz_642_mvl/mxv
	.byte	W02
	.byte		        15*mus_qz_642_mvl/mxv
	.byte	W03
	.byte		        13*mus_qz_642_mvl/mxv
	.byte	W01
	.byte		BEND  , c_v+16
	.byte	W02
	.byte		        c_v+28
	.byte	W04
	.byte	PATT
	 ptrvalue	mus_qz_642_5_001
	.byte		VOL   , 35*mus_qz_642_mvl/mxv
	.byte		BEND  , c_v+0
	.byte	W12
	.byte		N11   , Gn2 , v108
	.byte		N10   , Gn3 , v088
	.byte	W12
	.byte		N01   , Gn2 , v092
	.byte	W12
	.byte	N01
	.byte	W24
	.byte		N11   , Gs2 , v108
	.byte		N10   , Gs3 , v088
	.byte	W12
	.byte		N01   , Gs2 , v092
	.byte	W12
	.byte	N01
	.byte	W12
	.byte	W12
	.byte		N11   , An2 , v108
	.byte		N10   , An3 , v088
	.byte	W12
	.byte		N01   , An2 , v092
	.byte	W12
	.byte	N01
	.byte	W24
	.byte		N11   , As2 , v108
	.byte		N10   , As3 , v088
	.byte	W12
	.byte		N01   , As2 , v092
	.byte	W12
	.byte	N01
	.byte	W12
	.byte		N11   , En2 , v108
	.byte		N10   , Cs3 , v088
	.byte	W24
	.byte		N11   , Fn2 , v108
	.byte		N10   , Dn3 , v088
	.byte	W24
	.byte		N11   , Fs2 , v108
	.byte		N10   , Ds3 , v088
	.byte	W24
	.byte		N11   , Gn2 , v108
	.byte		N10   , En3 , v088
	.byte	W24
	.byte		N11   , Gs2 , v108
	.byte		N10   , Fn3 , v088
	.byte	W24
	.byte		N11   , An2 , v108
	.byte		N10   , Fs3 , v088
	.byte	W24
	.byte		N11   , As2 , v108
	.byte		N10   , Gn3 , v088
	.byte	W24
	.byte		N11   , Bn2 , v108
	.byte		N10   , Gs3 , v088
	.byte	W24
	.byte	GOTO
	 ptrvalue	mus_qz_642_5_B1
	.byte	FINE

/********************** Track  6 **********************/

mus_qz_642_6:
	.byte		KEYSH , mus_qz_642_key+0
	.byte		VOICE , 17
	.byte		VOL   , 46*mus_qz_642_mvl/mxv
	.byte		PAN   , c_v-24
	.byte		BENDR , 6
	.byte		PRIO  , 58
	.byte		VOL   , 39*mus_qz_642_mvl/mxv
	.byte	W12
mus_qz_642_6_B1:
	.byte	W96
	.byte		N11   , Gn1 , v104
	.byte	W12
	.byte		        Gn2 , v096
	.byte	W12
	.byte		N02   , Gn1 , v088
	.byte	W12
	.byte		        Gn1 , v080
	.byte	W12
	.byte		N11   , Gn2 , v096
	.byte	W03
	.byte		BEND  , c_v+8
	.byte	W03
	.byte		        c_v+0
	.byte	W06
	.byte		N11   , Gn1 , v104
	.byte	W12
	.byte		N05   , As1
	.byte	W12
	.byte		        Dn2
	.byte	W12
mus_qz_642_6_000:
	.byte		N11   , Gn1 , v104
	.byte	W12
	.byte		N02   , Gn1 , v088
	.byte	W12
	.byte		        Gn1 , v080
	.byte	W12
	.byte		N11   , Fs1 , v104
	.byte	W12
	.byte		        Gn1
	.byte	W12
	.byte		N02   , Gn1 , v127
	.byte	W12
	.byte		N11   , Cs2
	.byte	W03
	.byte		BEND  , c_v+5
	.byte	W03
	.byte		        c_v+0
	.byte	W06
	.byte		        c_v+10
	.byte		N05   , En2
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W09
	.byte	PEND
mus_qz_642_6_001:
	.byte		N02   , Gn1 , v104
	.byte	W12
	.byte		N11   , Gn2 , v096
	.byte	W12
	.byte		N02   , Gn1 , v104
	.byte	W12
	.byte		        Gn1 , v092
	.byte	W12
	.byte		BEND  , c_v+14
	.byte		N11   , Gn4 , v080
	.byte	W03
	.byte		BEND  , c_v+2
	.byte	W09
	.byte		        c_v+0
	.byte		N02   , Ds4 , v112
	.byte	W12
	.byte		N05   , As1 , v104
	.byte	W12
	.byte		        Dn2
	.byte	W12
	.byte	PEND
	.byte		N11   , Gn1
	.byte	W12
	.byte		N02   , Gn2 , v127
	.byte	W12
	.byte		N03   , Gn1 , v104
	.byte	W12
	.byte		N11   , Fs1
	.byte	W12
	.byte		        Gn1
	.byte	W12
	.byte		BEND  , c_v+14
	.byte		N05   , Gn2 , v100
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W09
	.byte		N11   , Cs2 , v104
	.byte	W12
	.byte		N02   , En2
	.byte	W06
	.byte		N03   , En2 , v092
	.byte	W06
	.byte		VOL   , 33*mus_qz_642_mvl/mxv
	.byte		N11   , Ds2 , v104
	.byte	W12
	.byte		N02   , Ds3
	.byte	W12
	.byte		        Ds2
	.byte	W12
	.byte	N01
	.byte	W12
	.byte		N11   , En2
	.byte	W12
	.byte		N32   , Cn5 , v108 , gtp3
	.byte	W18
	.byte		VOL   , 23*mus_qz_642_mvl/mxv
	.byte	W02
	.byte		        20*mus_qz_642_mvl/mxv
	.byte	W04
	.byte		        16*mus_qz_642_mvl/mxv
	.byte	W02
	.byte		        11*mus_qz_642_mvl/mxv
	.byte	W03
	.byte		        10*mus_qz_642_mvl/mxv
	.byte	W01
	.byte		BEND  , c_v+16
	.byte	W02
	.byte		        c_v+28
	.byte	W04
	.byte		PAN   , c_v+50
	.byte		VOL   , 35*mus_qz_642_mvl/mxv
	.byte		BEND  , c_v+0
	.byte		N11   , Ds2 , v104
	.byte	W12
	.byte		N02   , Ds3
	.byte	W12
	.byte		N01   , Ds2
	.byte	W12
	.byte	N02
	.byte	W12
	.byte		BEND  , c_v+0
	.byte		N02   , En2
	.byte	W06
	.byte		        En2 , v092
	.byte	W06
	.byte		BEND  , c_v+1
	.byte		N32   , Ds5 , v108 , gtp3
	.byte	W06
	.byte		BEND  , c_v+8
	.byte	W06
	.byte		        c_v+2
	.byte	W06
	.byte		VOL   , 26*mus_qz_642_mvl/mxv
	.byte		BEND  , c_v+8
	.byte	W02
	.byte		VOL   , 22*mus_qz_642_mvl/mxv
	.byte	W04
	.byte		        19*mus_qz_642_mvl/mxv
	.byte		BEND  , c_v+1
	.byte	W02
	.byte		VOL   , 13*mus_qz_642_mvl/mxv
	.byte	W03
	.byte		        11*mus_qz_642_mvl/mxv
	.byte	W01
	.byte		BEND  , c_v+9
	.byte	W06
	.byte		VOL   , 39*mus_qz_642_mvl/mxv
	.byte		BEND  , c_v+0
	.byte		N11   , Ds2 , v104
	.byte	W12
	.byte		N02   , Ds3
	.byte	W12
	.byte		        Ds2
	.byte	W12
	.byte	N02
	.byte	W12
	.byte		        En2
	.byte	W12
	.byte		BEND  , c_v+2
	.byte		N32   , An4 , v108 , gtp3
	.byte	W06
	.byte		BEND  , c_v+8
	.byte	W06
	.byte		        c_v+3
	.byte	W06
	.byte		        c_v+8
	.byte	W03
	.byte		        c_v+0
	.byte	W03
	.byte		        c_v-7
	.byte	W02
	.byte		        c_v-17
	.byte	W01
	.byte		VOL   , 37*mus_qz_642_mvl/mxv
	.byte		PAN   , c_v+10
	.byte	W02
	.byte		BEND  , c_v-28
	.byte	W01
	.byte		VOL   , 28*mus_qz_642_mvl/mxv
	.byte		PAN   , c_v-22
	.byte	W02
	.byte		BEND  , c_v-42
	.byte	W01
	.byte		VOL   , 21*mus_qz_642_mvl/mxv
	.byte		PAN   , c_v-48
	.byte	W03
	.byte		VOL   , 39*mus_qz_642_mvl/mxv
	.byte		PAN   , c_v+50
	.byte		BEND  , c_v+0
	.byte		N02   , Cn2 , v104
	.byte	W06
	.byte		        Cn2 , v092
	.byte	W06
	.byte		N11   , Cn2 , v104
	.byte	W12
	.byte		N02   , Cs2
	.byte	W06
	.byte		        Cs2 , v092
	.byte	W06
	.byte		N11   , Cs2 , v104
	.byte	W12
	.byte		N02   , Dn2
	.byte	W06
	.byte		        Dn2 , v092
	.byte	W06
	.byte		N11   , Dn2 , v104
	.byte	W12
	.byte		N02   , Ds2
	.byte	W06
	.byte		        Ds2 , v092
	.byte	W06
	.byte		N11   , Ds2 , v104
	.byte	W12
	.byte		N02   , Gn1 , v088
	.byte	W06
	.byte		        Gn1 , v080
	.byte	W06
	.byte		        Gn1 , v088
	.byte	W06
	.byte		        Gn1 , v080
	.byte	W06
	.byte		N12   , Gn2 , v096
	.byte	W12
	.byte		N02   , Gn1 , v088
	.byte	W06
	.byte		        Gn1 , v080
	.byte	W06
	.byte		N11   , Gn2 , v096
	.byte	W03
	.byte		BEND  , c_v+8
	.byte	W03
	.byte		        c_v+0
	.byte	W06
	.byte		N11   , Gn1 , v104
	.byte	W12
	.byte		N05   , As1
	.byte	W12
	.byte		        Dn2
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_642_6_000
	.byte	PATT
	 ptrvalue	mus_qz_642_6_001
mus_qz_642_6_002:
	.byte		N11   , Gn1 , v104
	.byte	W12
	.byte		N02   , Gn2 , v127
	.byte	W12
	.byte		N03   , Gn1 , v104
	.byte	W12
	.byte		N11   , Fs1
	.byte	W12
	.byte		        Gn1
	.byte	W12
	.byte		BEND  , c_v+14
	.byte		N05   , Gn2 , v100
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W09
	.byte		N11   , Cs2 , v104
	.byte	W12
	.byte		N02   , En2
	.byte	W06
	.byte	N04
	.byte	W06
	.byte	PEND
	.byte		N11   , Gn1
	.byte	W12
	.byte		        Gn2 , v096
	.byte	W12
	.byte		N02   , Gn1 , v088
	.byte	W12
	.byte	N02
	.byte	W12
	.byte		N11   , Gn2 , v096
	.byte	W03
	.byte		BEND  , c_v+8
	.byte	W03
	.byte		        c_v+0
	.byte	W06
	.byte		N11   , Gn1 , v104
	.byte	W12
	.byte		N05   , As1
	.byte	W12
	.byte		        Dn2
	.byte	W12
	.byte		N11   , Gn1
	.byte	W12
	.byte		N02   , Gn1 , v088
	.byte	W12
	.byte	N02
	.byte	W12
	.byte		N11   , Fs1 , v104
	.byte	W12
	.byte		        Gn1
	.byte	W12
	.byte		N02   , Gn1 , v127
	.byte	W12
	.byte		N11   , Cs2
	.byte	W03
	.byte		BEND  , c_v+5
	.byte	W03
	.byte		        c_v+0
	.byte	W06
	.byte		        c_v+10
	.byte		N05   , En2
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W09
	.byte		N02   , Gn1 , v104
	.byte	W12
	.byte		N11   , Gn2 , v096
	.byte	W12
	.byte		N02   , Gn1 , v104
	.byte	W12
	.byte	N02
	.byte	W12
	.byte		BEND  , c_v+14
	.byte		N11   , Gn4 , v080
	.byte	W03
	.byte		BEND  , c_v+2
	.byte	W09
	.byte		        c_v+0
	.byte		N02   , Ds4 , v112
	.byte	W12
	.byte		N05   , As1 , v104
	.byte	W12
	.byte		        Dn2
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_642_6_002
	.byte		N11   , Ds2 , v104
	.byte	W12
	.byte		N02   , Ds3
	.byte	W12
	.byte		        Ds2
	.byte	W12
	.byte	N01
	.byte	W12
	.byte		N11   , En2
	.byte	W12
	.byte		N32   , Cn5 , v108 , gtp3
	.byte	W18
	.byte		VOL   , 35*mus_qz_642_mvl/mxv
	.byte	W02
	.byte		        30*mus_qz_642_mvl/mxv
	.byte	W04
	.byte		        26*mus_qz_642_mvl/mxv
	.byte	W02
	.byte		        20*mus_qz_642_mvl/mxv
	.byte	W03
	.byte		        17*mus_qz_642_mvl/mxv
	.byte	W01
	.byte		BEND  , c_v+16
	.byte	W02
	.byte		        c_v+28
	.byte	W04
mus_qz_642_6_003:
	.byte		PAN   , c_v+50
	.byte		VOL   , 39*mus_qz_642_mvl/mxv
	.byte		BEND  , c_v+0
	.byte		N11   , Ds2 , v104
	.byte	W12
	.byte		N02   , Ds3
	.byte	W12
	.byte		N01   , Ds2
	.byte	W12
	.byte	N02
	.byte	W12
	.byte		BEND  , c_v+0
	.byte		N02   , En2
	.byte	W06
	.byte	N02
	.byte	W06
	.byte		BEND  , c_v+1
	.byte		N32   , Ds5 , v112 , gtp3
	.byte	W06
	.byte		BEND  , c_v+8
	.byte	W06
	.byte		        c_v+2
	.byte	W06
	.byte		VOL   , 28*mus_qz_642_mvl/mxv
	.byte		BEND  , c_v+8
	.byte	W02
	.byte		VOL   , 24*mus_qz_642_mvl/mxv
	.byte	W04
	.byte		        20*mus_qz_642_mvl/mxv
	.byte		BEND  , c_v+1
	.byte	W02
	.byte		VOL   , 15*mus_qz_642_mvl/mxv
	.byte	W03
	.byte		        13*mus_qz_642_mvl/mxv
	.byte	W01
	.byte		BEND  , c_v+9
	.byte	W06
	.byte	PEND
	.byte		VOL   , 39*mus_qz_642_mvl/mxv
	.byte		BEND  , c_v+0
	.byte		N11   , Ds2 , v104
	.byte	W12
	.byte		N02   , Ds3
	.byte	W12
	.byte		        Ds2
	.byte	W12
	.byte	N01
	.byte	W12
	.byte		N11   , En2
	.byte	W12
	.byte		N32   , Cn5 , v108 , gtp3
	.byte	W18
	.byte		VOL   , 39*mus_qz_642_mvl/mxv
	.byte	W02
	.byte		        34*mus_qz_642_mvl/mxv
	.byte	W04
	.byte		        29*mus_qz_642_mvl/mxv
	.byte	W02
	.byte		        23*mus_qz_642_mvl/mxv
	.byte	W03
	.byte		        20*mus_qz_642_mvl/mxv
	.byte	W01
	.byte		BEND  , c_v+16
	.byte	W02
	.byte		        c_v+28
	.byte	W04
	.byte	PATT
	 ptrvalue	mus_qz_642_6_003
	.byte		VOL   , 39*mus_qz_642_mvl/mxv
	.byte		BEND  , c_v+0
	.byte		N11   , Cn2 , v120
	.byte	W12
	.byte		        Cn3 , v112
	.byte	W12
	.byte		N02   , Cn2 , v104
	.byte	W12
	.byte	N02
	.byte	W12
	.byte		N11   , Cs2 , v120
	.byte	W12
	.byte		        Cs3 , v112
	.byte	W12
	.byte		N02   , Cs2 , v104
	.byte	W12
	.byte	N02
	.byte	W12
	.byte		N11   , Dn2 , v120
	.byte	W12
	.byte		        Dn3 , v112
	.byte	W12
	.byte		N02   , Dn2 , v104
	.byte	W12
	.byte	N02
	.byte	W12
	.byte		N11   , Ds2 , v120
	.byte	W12
	.byte		        Ds3 , v112
	.byte	W12
	.byte		N02   , Ds2 , v104
	.byte	W12
	.byte	N02
	.byte	W12
	.byte		N11   , En1 , v112
	.byte	W12
	.byte		N02   , En1 , v088
	.byte	W06
	.byte		        En1 , v076
	.byte	W06
	.byte		N11   , Fn1 , v112
	.byte	W12
	.byte		N02   , Fn1 , v088
	.byte	W06
	.byte		        Fn1 , v076
	.byte	W06
	.byte		N11   , Fs1 , v112
	.byte	W12
	.byte		N02   , Fs1 , v088
	.byte	W06
	.byte		        Fs1 , v076
	.byte	W06
	.byte		N11   , Gn1 , v112
	.byte	W12
	.byte		N02   , Gn1 , v088
	.byte	W06
	.byte		        Gn1 , v076
	.byte	W06
	.byte		N11   , Gs1 , v112
	.byte	W12
	.byte		N02   , Gs1 , v088
	.byte	W06
	.byte		        Gs1 , v076
	.byte	W06
	.byte		N11   , An1 , v112
	.byte	W12
	.byte		N02   , An1 , v088
	.byte	W06
	.byte		        An1 , v076
	.byte	W06
	.byte		N11   , As1 , v112
	.byte	W12
	.byte		N02   , As1 , v088
	.byte	W06
	.byte		        As1 , v076
	.byte	W06
	.byte		N11   , Bn1 , v112
	.byte	W12
	.byte		N02   , Bn1 , v088
	.byte	W06
	.byte		        Bn1 , v076
	.byte	W06
	.byte	GOTO
	 ptrvalue	mus_qz_642_6_B1
	.byte	FINE

/********************** Track  7 **********************/

mus_qz_642_7:
	.byte		KEYSH , mus_qz_642_key+0
	.byte		VOICE , 23
	.byte		VOL   , 54*mus_qz_642_mvl/mxv
	.byte		PAN   , c_v-44
	.byte		PRIO  , 60
	.byte		VOL   , 54*mus_qz_642_mvl/mxv
	.byte		N05   , Cn5 , v124
	.byte	W06
	.byte		        Dn5 , v120
	.byte	W06
mus_qz_642_7_B1:
	.byte		N04   , Ds5 , v124
	.byte	W12
	.byte		        Dn5 , v120
	.byte	W12
	.byte		        Cs5 , v124
	.byte	W12
	.byte		        Cn5 , v120
	.byte	W24
	.byte		VOL   , 53*mus_qz_642_mvl/mxv
	.byte		N05   , Fn3 , v076
	.byte		N05   , Fn4 , v096
	.byte	W06
	.byte		        Gs3 , v084
	.byte		N05   , Gs4 , v096
	.byte	W06
	.byte		VOL   , 60*mus_qz_642_mvl/mxv
	.byte		N05   , Bn3 , v088
	.byte		N05   , Bn4 , v096
	.byte	W06
	.byte		        Dn4 , v092
	.byte		N05   , Dn5 , v100
	.byte	W06
	.byte		        Fn4
	.byte		N05   , Fn5
	.byte	W06
	.byte		        Gs4 , v108
	.byte		N05   , Gs5 , v104
	.byte	W06
	.byte		VOL   , 54*mus_qz_642_mvl/mxv
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
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_642_7_B1
	.byte	FINE

/********************** Track  8 **********************/

mus_qz_642_8:
	.byte		KEYSH , mus_qz_642_key+0
	.byte		VOICE , 39
	.byte		VOL   , 64*mus_qz_642_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 40
	.byte		N04   , En1 , v116
	.byte	W06
	.byte		        En1 , v108
	.byte	W06
mus_qz_642_8_B1:
	.byte		N22   , Cn1 , v116
	.byte		N13   , Cs2
	.byte		N13   , As2
	.byte	W48
	.byte		N11   , En1 , v124
	.byte		N23   , Ds2 , v116
	.byte	W12
	.byte		N10   , Cn1
	.byte	W10
	.byte		N01   , En1 , v056
	.byte	W02
	.byte		N07   , En1 , v124
	.byte	W12
	.byte		N04   , Cn1 , v116
	.byte	W06
	.byte		N05   , En1 , v124
	.byte	W06
mus_qz_642_8_000:
	.byte		N10   , Cn1 , v116
	.byte		N23   , Cs2 , v124
	.byte	W12
	.byte		N04   , Cn1 , v092
	.byte	W12
	.byte		N11   , En1 , v120
	.byte	W12
	.byte		N10   , As1 , v092
	.byte	W12
	.byte		        Cn1 , v116
	.byte	W12
	.byte		        Cn1 , v104
	.byte		N10   , As1 , v084
	.byte	W12
	.byte		N11   , En1 , v120
	.byte	W12
	.byte		N10   , As1 , v096
	.byte	W12
	.byte	PEND
mus_qz_642_8_001:
	.byte		N10   , Cn1 , v108
	.byte	W12
	.byte		        As1 , v084
	.byte	W12
	.byte		N11   , En1 , v120
	.byte	W12
	.byte		N10   , Cn1 , v092
	.byte		N10   , As1
	.byte	W12
	.byte		        Cn1 , v116
	.byte	W12
	.byte		N05   , En1 , v100
	.byte		N10   , As1 , v084
	.byte	W06
	.byte		N11   , En1 , v088
	.byte	W06
	.byte		N10   , Cn1 , v116
	.byte	W12
	.byte		N11   , En1 , v120
	.byte		N10   , As1 , v096
	.byte	W12
	.byte	PEND
mus_qz_642_8_002:
	.byte		N10   , Cn1 , v108
	.byte		N23   , Ds2 , v124
	.byte	W24
	.byte		N11   , En1 , v120
	.byte	W12
	.byte		N10   , As1 , v092
	.byte	W12
	.byte		        Cn1
	.byte	W12
	.byte		        Cn1 , v116
	.byte		N10   , As1 , v084
	.byte	W12
	.byte		N11   , En1 , v120
	.byte	W12
	.byte		N10   , As1 , v096
	.byte	W12
	.byte	PEND
mus_qz_642_8_003:
	.byte		N10   , Cn1 , v108
	.byte	W12
	.byte		        As1 , v084
	.byte	W12
	.byte		N11   , En1 , v120
	.byte	W12
	.byte		N10   , Cn1 , v092
	.byte		N10   , As1
	.byte	W12
	.byte		        Cn1 , v116
	.byte	W12
	.byte		N05   , En1 , v120
	.byte		N10   , As1 , v084
	.byte	W06
	.byte		N11   , En1 , v120
	.byte	W06
	.byte		N10   , Cn1 , v116
	.byte	W12
	.byte		N11   , En1 , v120
	.byte		N10   , As1 , v096
	.byte	W12
	.byte	PEND
	.byte		        Cn1 , v092
	.byte	W12
	.byte		N04   , Cn1 , v080
	.byte		N10   , As1 , v068
	.byte	W12
	.byte		N11   , En1 , v108
	.byte	W12
	.byte		N10   , As1 , v076
	.byte	W24
	.byte		        Cn1 , v104
	.byte		N10   , As1 , v068
	.byte	W12
	.byte		N11   , En1 , v108
	.byte	W12
	.byte		N10   , As1 , v080
	.byte	W12
	.byte		        Cn1 , v100
	.byte	W12
	.byte		        As1 , v072
	.byte	W12
	.byte		N11   , En1 , v112
	.byte	W12
	.byte		N10   , Cn1 , v084
	.byte		N10   , As1 , v080
	.byte	W12
	.byte		N04   , Cn1 , v108
	.byte	W06
	.byte	N04
	.byte	W06
	.byte		N10   , En1 , v092
	.byte		N10   , As1 , v072
	.byte	W12
	.byte		        Cn1 , v084
	.byte	W12
	.byte		N05   , En1 , v112
	.byte		N10   , As1 , v084
	.byte	W06
	.byte		N05   , En1 , v112
	.byte	W06
	.byte		N10   , Cn1 , v108
	.byte	W12
	.byte		        As1 , v084
	.byte	W12
	.byte		N11   , En1 , v120
	.byte	W12
	.byte		N10   , As1 , v092
	.byte	W12
	.byte		N04   , Cn1
	.byte	W06
	.byte	N04
	.byte	W06
	.byte		N11   , En1 , v124
	.byte		N10   , As1 , v084
	.byte	W12
	.byte		N11   , En1 , v120
	.byte	W12
	.byte		N10   , As1 , v096
	.byte	W10
	.byte		N01   , En1 , v072
	.byte	W02
	.byte		N07   , En1 , v100
	.byte	W12
	.byte		N10   , Cn1
	.byte		N10   , As1 , v092
	.byte	W10
	.byte		N01   , En1 , v076
	.byte	W02
	.byte		N07   , En1 , v100
	.byte	W12
	.byte		N10   , Cn1 , v108
	.byte		N10   , As1 , v104
	.byte	W12
	.byte		N11   , En1 , v124
	.byte	W12
	.byte		N10   , Cn1 , v116
	.byte		N10   , As1 , v092
	.byte	W12
	.byte		N04   , En1 , v100
	.byte	W06
	.byte		        En1 , v092
	.byte	W06
	.byte		N11   , Fn1 , v120
	.byte		N10   , As1 , v108
	.byte	W12
	.byte		N02   , En1 , v064
	.byte	W03
	.byte		        En1 , v084
	.byte	W03
	.byte		N05   , En1 , v108
	.byte	W06
	.byte		        Gn1 , v116
	.byte	W06
	.byte		        Fn1 , v108
	.byte	W06
	.byte		N10   , Cn1 , v124
	.byte		N23   , Cs2
	.byte	W24
	.byte		N10   , Cn1 , v116
	.byte	W12
	.byte		        Cn1 , v104
	.byte		N10   , As1 , v084
	.byte	W12
	.byte		N11   , En1 , v120
	.byte	W12
	.byte		N10   , As1 , v096
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_642_8_001
	.byte		N10   , Cn1 , v108
	.byte	W12
	.byte		        As1 , v096
	.byte	W12
	.byte		N11   , En1 , v120
	.byte	W12
	.byte		N10   , As1 , v092
	.byte	W12
	.byte		        Cn1
	.byte	W12
	.byte		        Cn1 , v116
	.byte		N10   , As1 , v084
	.byte	W12
	.byte		N11   , En1 , v120
	.byte	W12
	.byte		N10   , As1 , v096
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_642_8_003
	.byte		N10   , Cn1 , v092
	.byte		N23   , Cs2 , v124
	.byte	W12
	.byte		N04   , Cn1 , v080
	.byte	W12
	.byte		N11   , En1 , v108
	.byte	W12
	.byte		N10   , As1 , v084
	.byte	W24
	.byte		        Cn1 , v104
	.byte		N10   , As1 , v076
	.byte	W12
	.byte		N11   , En1 , v108
	.byte	W12
	.byte		N10   , As1 , v084
	.byte	W12
mus_qz_642_8_004:
	.byte		N10   , Cn1 , v100
	.byte	W12
	.byte		        As1 , v076
	.byte	W12
	.byte		N11   , En1 , v112
	.byte	W12
	.byte		N10   , Cn1 , v084
	.byte		N10   , As1 , v088
	.byte	W12
	.byte		N04   , Cn1 , v108
	.byte	W06
	.byte	N04
	.byte	W06
	.byte		N10   , En1 , v092
	.byte		N10   , As1 , v080
	.byte	W12
	.byte		        Cn1 , v084
	.byte	W12
	.byte		N05   , En1 , v112
	.byte		N10   , As1 , v088
	.byte	W06
	.byte		N05   , En1 , v112
	.byte	W06
	.byte	PEND
	.byte		N10   , Cn1 , v092
	.byte	W12
	.byte		N04   , Cn1 , v080
	.byte		N10   , As1 , v072
	.byte	W12
	.byte		N11   , En1 , v108
	.byte	W12
	.byte		N10   , As1 , v084
	.byte	W24
	.byte		        Cn1 , v104
	.byte		N10   , As1 , v076
	.byte	W12
	.byte		N11   , En1 , v108
	.byte	W12
	.byte		N10   , As1 , v084
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_642_8_004
	.byte	PATT
	 ptrvalue	mus_qz_642_8_000
	.byte	PATT
	 ptrvalue	mus_qz_642_8_001
	.byte	PATT
	 ptrvalue	mus_qz_642_8_002
	.byte		N10   , Cn1 , v108
	.byte	W12
	.byte		        As1 , v084
	.byte	W12
	.byte		N11   , En1 , v120
	.byte	W12
	.byte		N10   , Cn1 , v092
	.byte		N10   , As1
	.byte	W12
	.byte		        Cn1 , v116
	.byte	W12
	.byte		N05   , En1 , v120
	.byte		N10   , As1 , v084
	.byte	W12
	.byte		        Cn1 , v116
	.byte		N05   , Bn1 , v120
	.byte	W06
	.byte		        Gs1 , v112
	.byte	W06
	.byte		        Fn1 , v120
	.byte	W06
	.byte		        En1 , v112
	.byte	W06
	.byte		N10   , Cn1 , v092
	.byte		N23   , Ds2 , v124
	.byte	W12
	.byte		N04   , Cn1 , v080
	.byte	W12
	.byte		N11   , En1 , v108
	.byte	W12
	.byte		N10   , As1 , v084
	.byte	W24
	.byte		        Cn1 , v104
	.byte		N10   , As1 , v076
	.byte	W12
	.byte		N11   , En1 , v108
	.byte	W12
	.byte		N10   , As1 , v084
	.byte	W12
	.byte		        Cn1 , v100
	.byte		N23   , En2 , v116
	.byte	W24
	.byte		N11   , En1 , v112
	.byte	W12
	.byte		N10   , Cn1 , v084
	.byte		N10   , As1 , v088
	.byte	W12
	.byte		N04   , Cn1 , v108
	.byte	W06
	.byte	N04
	.byte	W06
	.byte		N10   , En1 , v092
	.byte		N10   , As1 , v080
	.byte	W12
	.byte		        Cn1 , v084
	.byte	W12
	.byte		N05   , En1 , v112
	.byte		N10   , As1 , v088
	.byte	W06
	.byte		N05   , En1 , v112
	.byte	W06
	.byte		N10   , Cn1 , v108
	.byte		N23   , Ds2 , v124
	.byte	W24
	.byte		N11   , En1 , v120
	.byte	W12
	.byte		N10   , As1 , v092
	.byte	W11
	.byte		N23   , En2 , v116
	.byte	W01
	.byte		N04   , Cn1 , v092
	.byte	W06
	.byte	N04
	.byte	W06
	.byte		N11   , En1 , v124
	.byte	W12
	.byte		        En1 , v120
	.byte	W12
	.byte		N10   , As1 , v096
	.byte	W10
	.byte		N01   , En1 , v072
	.byte	W02
	.byte		N07   , En1 , v100
	.byte		N23   , Ds2 , v124
	.byte	W12
	.byte		N10   , Cn1 , v100
	.byte	W10
	.byte		N01   , En1 , v076
	.byte	W02
	.byte		N07   , En1 , v100
	.byte	W12
	.byte		N10   , Cn1 , v108
	.byte		N10   , As1 , v104
	.byte	W12
	.byte		N11   , En1 , v124
	.byte		N23   , En2
	.byte	W12
	.byte		N10   , Cn1 , v116
	.byte	W12
	.byte		N04   , En1 , v100
	.byte	W06
	.byte		        En1 , v092
	.byte	W06
	.byte		N11   , Fn1 , v120
	.byte		N10   , As1 , v108
	.byte	W12
	.byte	GOTO
	 ptrvalue	mus_qz_642_8_B1
	.byte	FINE

/********************** Track  9 **********************/

mus_qz_642_9:
	.byte		KEYSH , mus_qz_642_key+0
	.byte		VOICE , 29
	.byte		VOL   , 22*mus_qz_642_mvl/mxv
	.byte		PAN   , c_v+56
	.byte		VOL   , 17*mus_qz_642_mvl/mxv
	.byte		PRIO  , 38
	.byte	W06
	.byte		N05   , Cn5 , v120
	.byte	W06
mus_qz_642_9_B1:
	.byte		N05   , Dn5 , v116
	.byte	W06
	.byte		N04   , Ds5 , v120
	.byte	W12
	.byte		        Dn5 , v116
	.byte	W12
	.byte		        Cs5 , v120
	.byte	W12
	.byte		        Cn5 , v116
	.byte	W24
	.byte		N32   , Bn4 , v120 , gtp3
	.byte	W30
	.byte	W18
	.byte		N04   , Gn3 , v116
	.byte	W36
	.byte		N44   , An3 , v112 , gtp3
	.byte	W42
	.byte	W06
	.byte		N32   , As3 , v120 , gtp3
	.byte	W36
	.byte		N04   , An3 , v112
	.byte	W24
	.byte		        As3 , v120
	.byte	W12
	.byte		        An3 , v112
	.byte	W12
	.byte		        Cn5 , v124
	.byte	W06
	.byte	W18
	.byte		        Gn3 , v120
	.byte	W36
	.byte		N23   , Fn4 , v112
	.byte	W24
	.byte		N05   , Cn4 , v116
	.byte	W06
	.byte		        Dn4 , v112
	.byte	W06
	.byte		        Ds4 , v116
	.byte	W06
	.byte		        Fn4 , v112
	.byte	W06
	.byte		N32   , Fs4 , v120 , gtp3
	.byte	W36
	.byte		N04   , Ds4 , v112
	.byte	W24
	.byte		        Fs4 , v120
	.byte	W12
	.byte		        Ds4 , v112
	.byte	W12
	.byte		        Ds5 , v120
	.byte	W06
mus_qz_642_9_000:
	.byte	W18
	.byte		N04   , Fn3 , v116
	.byte	W48
	.byte		        Fs3 , v112
	.byte	W30
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_642_9_000
	.byte	PATT
	 ptrvalue	mus_qz_642_9_000
	.byte	W06
	.byte		N05   , Gs3 , v127
	.byte	W06
	.byte		        Fn3
	.byte	W06
	.byte		        Dn3
	.byte	W06
	.byte		        Bn2
	.byte	W06
	.byte		        An3
	.byte	W06
	.byte		        Fs3
	.byte	W06
	.byte		        Ds3
	.byte	W06
	.byte		        Cn3
	.byte	W06
	.byte		        As3
	.byte	W06
	.byte		        Gn3
	.byte	W06
	.byte		        En3
	.byte	W06
	.byte		        Cs3
	.byte	W06
	.byte		        Bn3
	.byte	W06
	.byte		        Gs3
	.byte	W06
	.byte		        Fn3
	.byte	W06
	.byte		        Dn3
	.byte	W24
	.byte	W03
	.byte		N02   , Dn4 , v112
	.byte	W03
	.byte		N05   , Ds4 , v124
	.byte	W24
	.byte	N05
	.byte	W12
	.byte		        Cn4 , v116
	.byte	W24
	.byte		N32   , Fs4 , v124 , gtp3
	.byte	W06
	.byte	W12
	.byte		VOL   , 19*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        17*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        16*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        22*mus_qz_642_mvl/mxv
	.byte		N23   , Fn4 , v116
	.byte	W06
	.byte		VOL   , 20*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        19*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        17*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        22*mus_qz_642_mvl/mxv
	.byte		N05   , Ds4 , v120
	.byte	W24
	.byte		        Cn4
	.byte	W12
	.byte		        Dn4 , v116
	.byte	W06
	.byte	W06
	.byte		        Ds4 , v120
	.byte	W12
	.byte		        Ds3 , v116
	.byte	W12
	.byte		        Ds3 , v112
	.byte	W12
	.byte		N02   , An4 , v116
	.byte	W03
	.byte		TIE   , As4 , v124
	.byte	W44
	.byte	W01
	.byte		VOL   , 21*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        19*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        17*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        16*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        15*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        13*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        11*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        10*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        8*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        7*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        6*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        5*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        4*mus_qz_642_mvl/mxv
	.byte	W02
	.byte	EOT
	.byte	W04
	.byte		VOL   , 3*mus_qz_642_mvl/mxv
	.byte	W12
	.byte		        22*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		N05   , Cn5 , v120
	.byte	W06
	.byte		        Dn5 , v116
	.byte	W24
	.byte	W03
	.byte		N02   , Dn4
	.byte	W03
	.byte		N05   , Ds4 , v124
	.byte	W24
	.byte	N05
	.byte	W12
	.byte		        Cn4 , v120
	.byte	W24
	.byte		N32   , Fs4 , v120 , gtp3
	.byte	W06
	.byte	W12
	.byte		VOL   , 20*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        19*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        17*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        22*mus_qz_642_mvl/mxv
	.byte		N23   , Fn4 , v116
	.byte	W06
	.byte		VOL   , 20*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        19*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        17*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        22*mus_qz_642_mvl/mxv
	.byte		N05   , Ds4 , v120
	.byte	W24
	.byte		        Cn4 , v124
	.byte	W12
	.byte		        Dn4 , v116
	.byte	W06
	.byte	W06
	.byte		        Ds4 , v124
	.byte	W12
	.byte		        Ds3 , v120
	.byte	W12
	.byte		        Ds3 , v116
	.byte	W12
	.byte		N02   , Bn3
	.byte	W03
	.byte		TIE   , Cn4 , v124
	.byte	W44
	.byte	W01
	.byte		VOL   , 21*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        19*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        17*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        16*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        15*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        13*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        11*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        10*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        8*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        7*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        6*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        5*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        4*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        3*mus_qz_642_mvl/mxv
	.byte	W02
	.byte	EOT
	.byte	W04
	.byte		VOL   , 22*mus_qz_642_mvl/mxv
	.byte		N05   , As3 , v120
	.byte	W06
	.byte		        An3 , v112
	.byte	W06
	.byte		        Gs3 , v120
	.byte	W06
	.byte		        Gn3 , v112
	.byte	W06
	.byte		N02   , Bn3
	.byte	W03
	.byte		N08   , Cn4 , v116
	.byte	W09
	.byte		N11   , Ds4 , v124
	.byte	W12
	.byte		        Cn4 , v120
	.byte	W12
	.byte		N23   , Fs4 , v124
	.byte	W06
	.byte		VOL   , 20*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        18*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        16*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        22*mus_qz_642_mvl/mxv
	.byte		N23   , Fn4 , v120
	.byte	W06
	.byte		VOL   , 20*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        19*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        17*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        22*mus_qz_642_mvl/mxv
	.byte		N11   , Cn4 , v124
	.byte	W06
	.byte	W06
	.byte		N02   , Dn4 , v116
	.byte	W03
	.byte		N24   , Ds4 , v124 , gtp2
	.byte	W09
	.byte		VOL   , 20*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        19*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        17*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        22*mus_qz_642_mvl/mxv
	.byte		N02   , Dn4 , v120
	.byte	W03
	.byte		        Cs4 , v112
	.byte	W03
	.byte		N32   , Cn4 , v120 , gtp3
	.byte	W18
	.byte		VOL   , 20*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        19*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        17*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        22*mus_qz_642_mvl/mxv
	.byte		N05   , Fn4 , v124
	.byte	W12
	.byte		        Fs4 , v116
	.byte	W06
	.byte	W06
	.byte		N11   , Cn4 , v120
	.byte	W12
	.byte		        Ds4 , v116
	.byte	W12
	.byte		        Cn4 , v120
	.byte	W12
	.byte		N23   , Fn4 , v124
	.byte	W06
	.byte		VOL   , 20*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        19*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        17*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        22*mus_qz_642_mvl/mxv
	.byte		N23   , Fs4 , v120
	.byte	W06
	.byte		VOL   , 20*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        19*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        17*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        22*mus_qz_642_mvl/mxv
	.byte		N11   , Cn4 , v116
	.byte	W06
	.byte	W06
	.byte		N05   , Bn3 , v124
	.byte	W12
	.byte		        Cn4 , v116
	.byte	W12
	.byte		        Dn4 , v120
	.byte	W12
	.byte		N23   , Ds4
	.byte	W06
	.byte		VOL   , 20*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        19*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        17*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        22*mus_qz_642_mvl/mxv
	.byte	W12
	.byte		N05   , Dn4 , v124
	.byte	W12
	.byte		        Cn4 , v120
	.byte	W06
	.byte		        Gn3 , v116
	.byte	W06
	.byte		N02   , Bn3 , v120
	.byte	W03
	.byte		N44   , Cn4 , v124
	.byte	W15
	.byte		VOL   , 20*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        19*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        17*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        16*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        14*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        22*mus_qz_642_mvl/mxv
	.byte		N44   , Cs4 , v120 , gtp3
	.byte	W42
	.byte	W06
	.byte		        Dn4 , v124 , gtp3
	.byte	W18
	.byte		VOL   , 20*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        19*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        17*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        16*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        14*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        22*mus_qz_642_mvl/mxv
	.byte		N44   , Ds4 , v120 , gtp3
	.byte	W18
	.byte		VOL   , 20*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        19*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        17*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        16*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        14*mus_qz_642_mvl/mxv
	.byte	W06
	.byte		        22*mus_qz_642_mvl/mxv
	.byte		N05   , En4 , v124
	.byte	W06
	.byte		        Cs4 , v108
	.byte	W06
	.byte		        As3 , v116
	.byte	W06
	.byte		        Gn3 , v112
	.byte	W06
	.byte		        Fn4 , v124
	.byte	W06
	.byte		        Dn4 , v112
	.byte	W06
	.byte		        Bn3 , v120
	.byte	W06
	.byte		        Gs3 , v112
	.byte	W06
	.byte		        Fs4 , v124
	.byte	W06
	.byte		        Ds4 , v112
	.byte	W06
	.byte		        Cn4 , v120
	.byte	W06
	.byte		        An3 , v112
	.byte	W06
	.byte		        Gn4 , v124
	.byte	W06
	.byte		        En4 , v112
	.byte	W06
	.byte		        Cs4 , v116
	.byte	W06
	.byte		        As3 , v112
	.byte	W06
	.byte		        Gs4 , v124
	.byte	W06
	.byte		        Fn4 , v108
	.byte	W06
	.byte		        Dn4 , v116
	.byte	W06
	.byte		        Bn3 , v108
	.byte	W06
	.byte		        An4 , v124
	.byte	W06
	.byte		        Fs4 , v108
	.byte	W06
	.byte		        Ds4 , v116
	.byte	W06
	.byte		        Cn4 , v108
	.byte	W06
	.byte		        As4 , v124
	.byte	W06
	.byte		        Gn4 , v108
	.byte	W06
	.byte		        En4 , v116
	.byte	W06
	.byte		        Cs4 , v108
	.byte	W06
	.byte		N11   , Bn4 , v124
	.byte	W12
	.byte		N05   , Cn5 , v120
	.byte	W06
	.byte	GOTO
	 ptrvalue	mus_qz_642_9_B1
	.byte		N05   , Dn5 , v112
	.byte	W05
	.byte	FINE

/******************************************************/
	.p2align	2

mus_qz_642:
	.byte	9	/* NumTrks */
	.byte	0	/* NumBlks */
	.byte	mus_qz_642_pri	/* Priority */
	.byte	mus_qz_642_rev	/* Reverb. */
space64 4
	ptrvalue	mus_qz_642_grp

	ptrvalue	mus_qz_642_1
	ptrvalue	mus_qz_642_2
	ptrvalue	mus_qz_642_3
	ptrvalue	mus_qz_642_4
	ptrvalue	mus_qz_642_5
	ptrvalue	mus_qz_642_6
	ptrvalue	mus_qz_642_7
	ptrvalue	mus_qz_642_8
	ptrvalue	mus_qz_642_9

	.end
