	.include "asm/macros/bit_width.inc"

	.include "MPlayDef.s"

/* Moemon Quetzal song 623, disassembled from the ROM by tools/quetzal_rip/songs.py.
   "org" comments give the ROM address of the bytes that follow. */

	.equ	mus_qz_623_grp, voicegroup_qz_songs
	.equ	mus_qz_623_pri, 0
	.equ	mus_qz_623_rev, reverb_set+0
	.equ	mus_qz_623_mvl, 127
	.equ	mus_qz_623_key, 0
	.equ	mus_qz_623_tbs, 1
	.equ	mus_qz_623_exg, 1
	.equ	mus_qz_623_cmp, 1

	.section .rodata
	.global	mus_qz_623
	.p2align	2
/* org 0x0967B030 */

/********************** Track  1 **********************/

mus_qz_623_1:
	.byte		KEYSH , mus_qz_623_key+0
	.byte		TEMPO , 128*mus_qz_623_tbs/2
	.byte		VOICE , 23
	.byte		VOL   , 66*mus_qz_623_mvl/mxv
	.byte		PAN   , c_v-44
	.byte		VOL   , 51*mus_qz_623_mvl/mxv
	.byte		PRIO  , 64
	.byte		N28   , As3 , v112 , gtp1
	.byte	W36
	.byte		N03   , Fn3 , v092
	.byte	W06
	.byte		        As3 , v104
	.byte	W06
	.byte		N28   , Cn4 , v116 , gtp1
	.byte	W36
	.byte		N03   , Gs3 , v092
	.byte	W06
	.byte		        Cn4 , v104
	.byte	W06
	.byte		N28   , Cs4 , v116 , gtp1
	.byte	W36
	.byte		N03   , As3 , v092
	.byte	W06
	.byte		        Cs4 , v104
	.byte	W06
	.byte		N68   , Ds4 , v120 , gtp3
	.byte	W11
	.byte		VOL   , 46*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        39*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        33*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        28*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        23*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        31*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        36*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        42*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        46*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        50*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        51*mus_qz_623_mvl/mxv
	.byte	W01
	.byte		N06   , Fs4 , v108
	.byte	W08
	.byte		        Fn4 , v100
	.byte	W08
	.byte		        Ds4 , v108
	.byte	W08
mus_qz_623_1_B1:
	.byte		N05   , Cs3 , v116
	.byte	W12
	.byte		        Gs2 , v108
	.byte	W12
	.byte		        Fn3 , v116
	.byte	W12
	.byte		        Gs2 , v108
	.byte	W12
	.byte		        Cs3 , v116
	.byte	W18
	.byte		        Gs2 , v108
	.byte	W06
	.byte		        Fn3 , v116
	.byte	W12
	.byte		        Gs2 , v108
	.byte	W12
	.byte		        Ds3 , v116
	.byte	W12
	.byte		        Fs2 , v108
	.byte	W12
	.byte		        Bn2 , v116
	.byte	W12
	.byte		        Fs2 , v108
	.byte	W12
	.byte		        Ds3 , v120
	.byte	W12
	.byte		        Fs3 , v108
	.byte	W12
	.byte		        Bn2 , v120
	.byte	W12
	.byte		        Fs2 , v112
	.byte	W12
	.byte		        Fn3 , v120
	.byte	W12
	.byte		        Gs2 , v112
	.byte	W12
	.byte		        Cs3 , v116
	.byte	W12
	.byte		        Gs2 , v108
	.byte	W12
	.byte		        Gs3 , v120
	.byte	W18
	.byte		        Gs2 , v112
	.byte	W06
	.byte		        Fn3 , v120
	.byte	W12
	.byte		        Gs2 , v112
	.byte	W12
	.byte		        Cs3 , v116
	.byte	W12
	.byte		        An2 , v108
	.byte	W12
	.byte		        En3 , v116
	.byte	W12
	.byte		        An2 , v108
	.byte	W12
	.byte		        An3 , v116
	.byte	W12
	.byte		        Cs3 , v108
	.byte	W12
	.byte		        Ds3 , v116
	.byte	W12
	.byte		        Fs3 , v108
	.byte	W12
	.byte		        Fn3 , v104
	.byte	W12
	.byte		        Cs3 , v100
	.byte	W06
	.byte		        Fn3 , v088
	.byte	W06
	.byte		N44   , As3 , v100 , gtp3
	.byte	W36
	.byte		VOL   , 47*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        42*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        53*mus_qz_623_mvl/mxv
	.byte		N05   , As3 , v104
	.byte	W12
	.byte		        Cn4 , v096
	.byte	W12
	.byte		N68   , Cs4 , v108 , gtp3
	.byte	W60
	.byte		VOL   , 47*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        42*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        53*mus_qz_623_mvl/mxv
	.byte		N23   , Ds4 , v100
	.byte	W24
	.byte		N44   , Cs4 , v108 , gtp3
	.byte	W36
	.byte		VOL   , 47*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        42*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        53*mus_qz_623_mvl/mxv
	.byte		N44   , Cn4 , v100 , gtp3
	.byte	W36
	.byte		VOL   , 47*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        42*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        53*mus_qz_623_mvl/mxv
	.byte		N44   , As3 , v108 , gtp3
	.byte	W36
	.byte		VOL   , 47*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        42*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        53*mus_qz_623_mvl/mxv
	.byte		N22   , Cn4 , v104
	.byte	W24
	.byte		N05   , Cn4 , v124
	.byte		N05   , Gs4
	.byte	W06
	.byte		        Gs3 , v116
	.byte		N05   , Ds4
	.byte	W06
	.byte		        Ds3 , v112
	.byte		N05   , Cn4
	.byte	W06
	.byte		        Cn3 , v104
	.byte		N05   , Gs3
	.byte	W06
	.byte		N32   , En3 , v092 , gtp3
	.byte		N32   , Cs4 , v120 , gtp3
	.byte	W24
	.byte		VOL   , 49*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        47*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        53*mus_qz_623_mvl/mxv
	.byte		N32   , Fs3 , v084 , gtp3
	.byte		N32   , Ds4 , v108 , gtp3
	.byte	W24
	.byte		VOL   , 49*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        46*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        53*mus_qz_623_mvl/mxv
	.byte		N23   , Gs3 , v092
	.byte		N23   , En4 , v120
	.byte	W24
	.byte		N11   , Bn3 , v092
	.byte		N11   , Fs4 , v120
	.byte	W12
	.byte		N05   , Fs3 , v088
	.byte		N05   , Bn3 , v116
	.byte	W24
	.byte		N23   , Ds4 , v092
	.byte		N23   , Bn4 , v120
	.byte	W24
	.byte		N11   , Cs4 , v092
	.byte		N11   , An4 , v120
	.byte	W12
	.byte		        Bn3 , v088
	.byte		N11   , Gs4 , v112
	.byte	W12
	.byte		        Cs4 , v092
	.byte		N11   , An4 , v120
	.byte	W12
	.byte		N32   , Bn3 , v108 , gtp3
	.byte		N32   , Gs4 , v120 , gtp3
	.byte	W24
	.byte		VOL   , 49*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        46*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        53*mus_qz_623_mvl/mxv
	.byte		N07   , Fs3 , v100
	.byte		N07   , Ds4 , v112
	.byte	W24
	.byte		N11   , Fs3 , v108
	.byte		N11   , Ds4 , v120
	.byte	W12
	.byte		        Bn3 , v100
	.byte		N11   , Gs4 , v112
	.byte	W12
	.byte		        Fs3 , v104
	.byte		N11   , Ds4 , v116
	.byte	W12
	.byte		N08   , Gs3 , v108
	.byte		N08   , Fn4 , v120
	.byte	W24
	.byte		N11   , Gs3 , v096
	.byte		N11   , Fn4 , v108
	.byte	W12
	.byte		N32   , An3 , v108 , gtp3
	.byte		N32   , Fs4 , v120 , gtp3
	.byte	W24
	.byte		VOL   , 49*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        46*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        53*mus_qz_623_mvl/mxv
	.byte		N14   , Bn3 , v108
	.byte		N14   , Gs4 , v120
	.byte	W15
	.byte		N02   , Gs3 , v092
	.byte		N02   , Fn4 , v100
	.byte	W03
	.byte		        An3 , v084
	.byte		N02   , Fs4 , v092
	.byte	W03
	.byte		        Bn3 , v096
	.byte		N02   , Gs4 , v104
	.byte	W03
	.byte		N44   , Cs4 , v108 , gtp3
	.byte		N44   , An4 , v120 , gtp3
	.byte	W36
	.byte		VOL   , 49*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        46*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        53*mus_qz_623_mvl/mxv
	.byte	W12
	.byte		N11   , Cs4 , v108
	.byte		N11   , An4 , v120
	.byte	W12
	.byte		        Bn3 , v100
	.byte		N11   , Gs4 , v112
	.byte	W12
	.byte		        An3 , v108
	.byte		N11   , Fs4 , v120
	.byte	W12
	.byte		N44   , Cn4 , v108 , gtp3
	.byte		N44   , An4 , v120 , gtp3
	.byte	W36
	.byte		VOL   , 49*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        46*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        53*mus_qz_623_mvl/mxv
	.byte	W36
	.byte		N05   , Bn3 , v104
	.byte		N05   , Gs4 , v112
	.byte	W06
	.byte		        Cn4 , v088
	.byte		N05   , An4 , v100
	.byte	W06
	.byte		VOL   , 23*mus_qz_623_mvl/mxv
	.byte		N92   , En4 , v104 , gtp2
	.byte		N92   , Bn4 , v112 , gtp2
	.byte	W02
	.byte		VOL   , 21*mus_qz_623_mvl/mxv
	.byte	W03
	.byte		        20*mus_qz_623_mvl/mxv
	.byte	W03
	.byte		        19*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        20*mus_qz_623_mvl/mxv
	.byte	W03
	.byte		        21*mus_qz_623_mvl/mxv
	.byte	W03
	.byte		        23*mus_qz_623_mvl/mxv
	.byte	W04
	.byte		        27*mus_qz_623_mvl/mxv
	.byte	W02
	.byte		        28*mus_qz_623_mvl/mxv
	.byte	W03
	.byte		        30*mus_qz_623_mvl/mxv
	.byte	W03
	.byte		        32*mus_qz_623_mvl/mxv
	.byte	W04
	.byte		        33*mus_qz_623_mvl/mxv
	.byte	W02
	.byte		        36*mus_qz_623_mvl/mxv
	.byte	W03
	.byte		        39*mus_qz_623_mvl/mxv
	.byte	W03
	.byte		        40*mus_qz_623_mvl/mxv
	.byte	W01
	.byte		        40*mus_qz_623_mvl/mxv
	.byte	W03
	.byte		        44*mus_qz_623_mvl/mxv
	.byte	W02
	.byte		        47*mus_qz_623_mvl/mxv
	.byte	W03
	.byte		        51*mus_qz_623_mvl/mxv
	.byte	W03
	.byte		        55*mus_qz_623_mvl/mxv
	.byte	W04
	.byte		        56*mus_qz_623_mvl/mxv
	.byte	W05
	.byte		        55*mus_qz_623_mvl/mxv
	.byte	W03
	.byte		        49*mus_qz_623_mvl/mxv
	.byte	W04
	.byte		        44*mus_qz_623_mvl/mxv
	.byte	W02
	.byte		        39*mus_qz_623_mvl/mxv
	.byte	W03
	.byte		        35*mus_qz_623_mvl/mxv
	.byte	W03
	.byte		        32*mus_qz_623_mvl/mxv
	.byte	W04
	.byte		        30*mus_qz_623_mvl/mxv
	.byte	W02
	.byte		        27*mus_qz_623_mvl/mxv
	.byte	W03
	.byte		        23*mus_qz_623_mvl/mxv
	.byte	W07
	.byte		        32*mus_qz_623_mvl/mxv
	.byte		N68   , Fs3 , v104 , gtp2
	.byte		N68   , Ds4 , v112 , gtp2
	.byte	W02
	.byte		VOL   , 35*mus_qz_623_mvl/mxv
	.byte	W03
	.byte		        39*mus_qz_623_mvl/mxv
	.byte	W03
	.byte		        40*mus_qz_623_mvl/mxv
	.byte	W04
	.byte		        44*mus_qz_623_mvl/mxv
	.byte	W02
	.byte		        49*mus_qz_623_mvl/mxv
	.byte	W03
	.byte		        51*mus_qz_623_mvl/mxv
	.byte	W03
	.byte		        55*mus_qz_623_mvl/mxv
	.byte	W04
	.byte		        55*mus_qz_623_mvl/mxv
	.byte	W02
	.byte		        57*mus_qz_623_mvl/mxv
	.byte	W22
	.byte		        55*mus_qz_623_mvl/mxv
	.byte	W05
	.byte		        47*mus_qz_623_mvl/mxv
	.byte	W03
	.byte		        42*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        36*mus_qz_623_mvl/mxv
	.byte	W03
	.byte		        32*mus_qz_623_mvl/mxv
	.byte	W03
	.byte		        25*mus_qz_623_mvl/mxv
	.byte	W04
	.byte		        53*mus_qz_623_mvl/mxv
	.byte	N05
	.byte	W06
	.byte		        Cs4
	.byte	W06
	.byte		        Cn4
	.byte	W06
	.byte		        Ds4
	.byte	W06
	.byte	GOTO
	 ptrvalue	mus_qz_623_1_B1
	.byte	FINE

/********************** Track  2 **********************/

mus_qz_623_2:
	.byte		KEYSH , mus_qz_623_key+0
	.byte		VOICE , 29
	.byte		VOL   , 73*mus_qz_623_mvl/mxv
	.byte		PAN   , c_v-6
	.byte		VOL   , 66*mus_qz_623_mvl/mxv
	.byte		PRIO  , 64
	.byte		N23   , Fn3 , v088
	.byte		N23   , As3 , v104
	.byte	W36
	.byte		N02   , As2 , v100
	.byte		N02   , Fn3
	.byte	W06
	.byte		        Fn3 , v096
	.byte		N02   , As3
	.byte	W06
	.byte		N23   , Ds3 , v088
	.byte		N23   , Cn4 , v104
	.byte	W36
	.byte		N02   , Ds3 , v092
	.byte		N02   , Gs3 , v100
	.byte	W06
	.byte		        Gs3 , v088
	.byte		N02   , Cn4 , v096
	.byte	W06
	.byte		N23   , Fs3 , v088
	.byte		N23   , Cs4 , v104
	.byte	W36
	.byte		N02   , Fs3 , v088
	.byte		N02   , As3 , v096
	.byte	W06
	.byte		        Fs3 , v084
	.byte		N02   , Cs4 , v092
	.byte	W06
	.byte		N68   , Gs3 , v088 , gtp3
	.byte		N68   , Ds4 , v104 , gtp3
	.byte	W11
	.byte		VOL   , 59*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        51*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        44*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        38*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        33*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        42*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        49*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        55*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        59*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        65*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        66*mus_qz_623_mvl/mxv
	.byte	W01
	.byte		N06   , Cn4 , v096
	.byte		N06   , Fs4
	.byte	W08
	.byte		        Gs3 , v088
	.byte		N06   , Fn4
	.byte	W08
	.byte		        Fs3 , v096
	.byte		N06   , Ds4
	.byte	W08
mus_qz_623_2_B1:
	.byte		N04   , Cs3 , v124
	.byte	W18
	.byte		N03   , Gs2 , v112
	.byte	W06
	.byte		N40   , Fn3 , v124 , gtp1
	.byte	W48
	.byte		N11   , Cs3 , v120
	.byte	W12
	.byte		N03   , Fn3 , v108
	.byte	W12
	.byte		N21   , Gs3 , v124
	.byte	W24
	.byte		        Fs3 , v112
	.byte	W24
	.byte		        Cs3 , v120
	.byte	W24
	.byte		        Ds3 , v112
	.byte	W24
	.byte		N03   , Cs3 , v124
	.byte	W12
	.byte		        Gs2 , v112
	.byte	W06
	.byte		        Cs3 , v120
	.byte	W06
	.byte		N40   , Fn3 , v120 , gtp1
	.byte	W48
	.byte		N11   , Cs3 , v116
	.byte	W12
	.byte		N05   , Fn3 , v108
	.byte	W12
	.byte		N22   , Gs3 , v120
	.byte	W24
	.byte		        Fs3 , v108
	.byte	W24
	.byte		        Cs3 , v120
	.byte	W24
	.byte		        Ds3 , v112
	.byte	W24
	.byte	W36
	.byte		N02   , Fn3 , v100
	.byte	W06
	.byte		        Fn3 , v092
	.byte	W06
	.byte		N05   , Fn3 , v108
	.byte	W12
	.byte		        Cs3 , v112
	.byte	W12
	.byte		        Fn3 , v100
	.byte	W12
	.byte		        Gs3
	.byte	W12
	.byte		N24   , As3 , v108 , gtp3
	.byte	W36
	.byte		N02   , As3 , v092
	.byte	W06
	.byte		        As3 , v104
	.byte	W06
	.byte	N05
	.byte	W12
	.byte		        Fs3 , v092
	.byte	W12
	.byte		        As3 , v100
	.byte	W12
	.byte		        Cn4 , v096
	.byte	W12
	.byte		N04   , Cs4 , v108
	.byte	W08
	.byte		N02   , Cs4 , v104
	.byte	W08
	.byte		        Ds4 , v116
	.byte	W08
	.byte		N23   , Gs4 , v108
	.byte	W24
	.byte		        Fs4 , v100
	.byte	W24
	.byte		        Ds4 , v108
	.byte	W24
	.byte		        Cs4
	.byte	W24
	.byte		        As3 , v100
	.byte	W24
	.byte		        Cn4 , v112
	.byte	W24
	.byte		        Gs3 , v104
	.byte	W24
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		        Cs4 , v092
	.byte	W24
	.byte		N05   , Cs3 , v084
	.byte	W08
	.byte		        Cs3 , v080
	.byte	W08
	.byte		        Cs3 , v084
	.byte	W08
	.byte		N23   , Cs3 , v088
	.byte	W44
	.byte	W01
	.byte		N03   , Ds4 , v060
	.byte	W03
	.byte		N23   , En4 , v092
	.byte	W24
	.byte		N05   , En3 , v084
	.byte	W08
	.byte		        En3 , v080
	.byte	W08
	.byte		        En3 , v084
	.byte	W08
	.byte		N23   , En3 , v092
	.byte	W48
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_623_2_B1
	.byte	FINE

/********************** Track  3 **********************/

mus_qz_623_3:
	.byte		KEYSH , mus_qz_623_key+0
	.byte		VOICE , 30
	.byte		VOL   , 71*mus_qz_623_mvl/mxv
	.byte		PAN   , c_v+21
	.byte		VOL   , 64*mus_qz_623_mvl/mxv
	.byte		PRIO  , 62
	.byte		N23   , As2 , v116
	.byte	W36
	.byte		N02   , As2 , v104
	.byte	W06
	.byte		        As2 , v112
	.byte	W06
	.byte		N23   , Gs2 , v116
	.byte	W36
	.byte		N02   , Gs2 , v108
	.byte	W06
	.byte		        Gs2 , v112
	.byte	W06
	.byte		N23   , Fs2 , v116
	.byte	W36
	.byte		N02   , Fs2 , v104
	.byte	W06
	.byte		        Fs2 , v108
	.byte	W06
	.byte		N68   , Gs1 , v120
	.byte	W12
	.byte		VOL   , 57*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        50*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        42*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        37*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        32*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        40*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        47*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        54*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        57*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        63*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        64*mus_qz_623_mvl/mxv
	.byte		N06   , Gs1 , v116
	.byte	W08
	.byte		        Cn2 , v104
	.byte	W08
	.byte		        Ds2
	.byte	W08
mus_qz_623_3_B1:
	.byte		N06   , Cs2 , v108
	.byte	W12
	.byte		        Cs2 , v100
	.byte	W24
	.byte		N11   , Gs1 , v108
	.byte	W12
	.byte		N06   , Cs2
	.byte	W24
	.byte		        Gs1
	.byte	W12
	.byte		        Cs2 , v100
	.byte	W12
	.byte		        Bn1 , v108
	.byte	W12
	.byte		        Bn1 , v100
	.byte	W24
	.byte		N11   , Fs1
	.byte	W12
	.byte		N06   , Bn1 , v108
	.byte	W24
	.byte		N23   , Ds2 , v100
	.byte	W24
	.byte		N06   , As1 , v108
	.byte	W12
	.byte		        As1 , v100
	.byte	W24
	.byte		N11   , Fn1 , v108
	.byte	W12
	.byte		N06   , As1
	.byte	W24
	.byte		        Fn1
	.byte	W12
	.byte		        As1 , v100
	.byte	W12
	.byte		        An1 , v108
	.byte	W12
	.byte		        An1 , v100
	.byte	W24
	.byte		N11   , En1 , v108
	.byte	W12
	.byte		        An1
	.byte	W24
	.byte		N06   , Bn1 , v100
	.byte	W08
	.byte		        Bn1 , v092
	.byte	W08
	.byte		        Bn1 , v100
	.byte	W08
	.byte		N05   , Fn2 , v116
	.byte	W12
	.byte		        Cs2
	.byte	W06
	.byte		        Fn2 , v108
	.byte	W06
	.byte		N42   , As2 , v120 , gtp1
	.byte	W48
	.byte		N05   , As2 , v124
	.byte	W12
	.byte		        Cn3 , v116
	.byte	W12
	.byte		N66   , Cs3 , v124 , gtp1
	.byte	W72
	.byte		N23   , Ds3 , v120
	.byte	W24
mus_qz_623_3_000:
	.byte		N05   , Gs2 , v116
	.byte	W08
	.byte		        Gs2 , v112
	.byte	W08
	.byte		        Gs2 , v116
	.byte	W08
	.byte		N23   , Gs1 , v124
	.byte	W24
	.byte		N05   , Gs2 , v116
	.byte	W08
	.byte		        Gs2 , v112
	.byte	W08
	.byte		        Gs2 , v116
	.byte	W08
	.byte		N23   , Gs1 , v124
	.byte	W24
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_623_3_000
	.byte		N05   , An1 , v116
	.byte	W12
	.byte		        An1 , v108
	.byte	W24
	.byte		N02   , An1 , v116
	.byte	W06
	.byte		        An1 , v108
	.byte	W06
	.byte		N05   , An1 , v116
	.byte	W12
	.byte		        An1 , v108
	.byte	W12
	.byte		N11   , En2 , v124
	.byte	W12
	.byte		        An1 , v112
	.byte	W12
	.byte		N05   , Bn1 , v116
	.byte	W12
	.byte		        Bn1 , v108
	.byte	W24
	.byte		N02   , Bn1 , v116
	.byte	W06
	.byte		        Bn1 , v108
	.byte	W06
	.byte		N05   , Bn1 , v116
	.byte	W12
	.byte		        Bn1 , v108
	.byte	W12
	.byte		N11   , Fs2 , v120
	.byte	W12
	.byte		        Bn1 , v108
	.byte	W12
	.byte		N05   , Gs1 , v116
	.byte	W12
	.byte		        Gs1 , v108
	.byte	W24
	.byte		N02   , Gs1 , v116
	.byte	W06
	.byte		        Gs1 , v108
	.byte	W06
	.byte		N05   , Gs1 , v120
	.byte	W12
	.byte		        Gs1 , v112
	.byte	W12
	.byte		N11   , Ds2 , v120
	.byte	W12
	.byte		N05   , Gs1 , v108
	.byte	W12
	.byte		        Cs2 , v116
	.byte	W12
	.byte		        Cs2 , v108
	.byte	W24
	.byte		N02   , Cs2 , v116
	.byte	W06
	.byte		        Cs2 , v108
	.byte	W06
	.byte		N05   , Cs2 , v116
	.byte	W12
	.byte	N05
	.byte	W12
	.byte		        Gs1 , v108
	.byte	W12
	.byte		N11   , Cs2 , v116
	.byte	W12
	.byte		N23   , Fs2 , v124
	.byte	W24
	.byte		N03   , Fs1 , v116
	.byte	W08
	.byte		        Fs1 , v112
	.byte	W08
	.byte		        Fs1 , v116
	.byte	W08
	.byte	N23
	.byte	W48
	.byte		        En2 , v120
	.byte	W24
	.byte		N03   , Cn2 , v116
	.byte	W08
	.byte		        Cn2 , v108
	.byte	W08
	.byte		        Cn2 , v112
	.byte	W08
	.byte		N23   , Cn2 , v120
	.byte	W48
	.byte		N88   , Bn1 , v120 , gtp1
	.byte	W96
	.byte	N22
	.byte	W24
	.byte		        An1 , v112
	.byte	W24
	.byte		        Gs1
	.byte	W24
	.byte		N05   , Ds2 , v116
	.byte	W12
	.byte		        Gs1 , v108
	.byte	W06
	.byte		        Cn2 , v112
	.byte	W06
	.byte	GOTO
	 ptrvalue	mus_qz_623_3_B1
	.byte	FINE

/********************** Track  4 **********************/

mus_qz_623_4:
	.byte		KEYSH , mus_qz_623_key+0
	.byte		VOICE , 23
	.byte		VOL   , 76*mus_qz_623_mvl/mxv
	.byte		PAN   , c_v+51
	.byte		VOL   , 68*mus_qz_623_mvl/mxv
	.byte		PRIO  , 60
	.byte		N23   , As1 , v116
	.byte	W36
	.byte		N02   , As1 , v104
	.byte	W06
	.byte		        As1 , v108
	.byte	W06
	.byte		N23   , Gs1 , v116
	.byte	W36
	.byte		N02   , Gs1 , v108
	.byte	W06
	.byte		        Gs1 , v104
	.byte	W06
	.byte		N23   , Fs1 , v112
	.byte	W36
	.byte		N02   , Fs1 , v108
	.byte	W06
	.byte		        Fs1 , v100
	.byte	W06
	.byte		N68   , Gs1 , v116
	.byte	W12
	.byte		VOL   , 62*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        54*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        46*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        39*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        34*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        44*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        50*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        57*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        62*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        68*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        68*mus_qz_623_mvl/mxv
	.byte	N05
	.byte	W08
	.byte		        Gs1 , v108
	.byte	W08
	.byte		        Gs1 , v112
	.byte	W08
mus_qz_623_4_B1:
	.byte		N04   , Cs2 , v104
	.byte	W12
	.byte		        Cs2 , v096
	.byte	W24
	.byte		N05   , Gs1 , v104
	.byte	W06
	.byte		        Gs1 , v096
	.byte	W06
	.byte		        Cs2 , v100
	.byte	W24
	.byte		N23   , Gs1 , v104
	.byte	W24
	.byte		N04   , Bn1
	.byte	W12
	.byte		        Bn1 , v096
	.byte	W24
	.byte		N05   , Fs1 , v104
	.byte	W06
	.byte		        Fs1 , v096
	.byte	W06
	.byte		        Bn1 , v100
	.byte	W24
	.byte		N11   , Fs1 , v104
	.byte	W12
	.byte		        Bn1 , v100
	.byte	W12
	.byte		N04   , As1 , v104
	.byte	W12
	.byte		        As1 , v096
	.byte	W24
	.byte		N05   , Fn1 , v104
	.byte	W06
	.byte		        Fn1 , v096
	.byte	W05
	.byte		        As1 , v100
	.byte	W24
	.byte		N23   , Fn1 , v104
	.byte	W24
	.byte		N04   , An1
	.byte	W01
	.byte	W11
	.byte		        An1 , v096
	.byte	W24
	.byte	W01
	.byte		N05   , En1 , v104
	.byte	W06
	.byte		        En1 , v096
	.byte	W05
	.byte		        An1 , v100
	.byte	W13
	.byte		        En1
	.byte	W12
	.byte		        An1 , v104
	.byte	W12
	.byte		        Bn1 , v100
	.byte	W12
	.byte		        As1 , v116
	.byte	W36
	.byte		N02   , As1 , v108
	.byte	W06
	.byte		        As1 , v100
	.byte	W06
	.byte		N05   , As1 , v108
	.byte	W12
	.byte		        As1 , v104
	.byte	W12
	.byte		N23   , As1 , v112
	.byte	W24
	.byte		N05   , Fs1 , v116
	.byte	W36
	.byte		N02   , Fs1 , v112
	.byte	W06
	.byte		        Fs1 , v100
	.byte	W06
	.byte		N05   , Fs1 , v116
	.byte	W12
	.byte		        Fs1 , v108
	.byte	W12
	.byte		N23   , Ds1 , v116
	.byte	W24
	.byte	W23
	.byte		        Gs0 , v112
	.byte	W48
	.byte		        Gs0 , v116
	.byte	W24
	.byte	W01
	.byte	W23
	.byte	N23
	.byte	W48
	.byte	N23
	.byte	W24
	.byte	W01
	.byte		N05   , An0
	.byte	W12
	.byte		        An0 , v108
	.byte	W36
	.byte		        An0 , v116
	.byte	W24
	.byte		N11   , En1 , v112
	.byte	W12
	.byte		        An0 , v116
	.byte	W12
	.byte		N05   , Bn0
	.byte	W12
	.byte		        Bn0 , v108
	.byte	W36
	.byte		        Bn0 , v116
	.byte	W24
	.byte		N11   , Fs1 , v112
	.byte	W12
	.byte		        Bn0 , v116
	.byte	W12
	.byte		N05   , Gs0
	.byte	W12
	.byte		        Gs0 , v108
	.byte	W36
	.byte		        Gs0 , v116
	.byte	W24
	.byte		N11   , Ds1 , v112
	.byte	W12
	.byte		        Gs0 , v116
	.byte	W12
	.byte		N05   , Cs1
	.byte	W12
	.byte		        Cs1 , v108
	.byte	W36
	.byte		        Cs1 , v116
	.byte	W24
	.byte		N11   , Gs0
	.byte	W12
	.byte		        Cs1
	.byte	W12
	.byte		N23   , Fs1 , v112
	.byte	W24
	.byte		N03   , Fs0 , v116
	.byte	W08
	.byte		        Fs0 , v112
	.byte	W08
	.byte		        Fs0 , v116
	.byte	W08
	.byte	N23
	.byte	W48
	.byte		        En1
	.byte	W24
	.byte		N03   , Cn1
	.byte	W08
	.byte		        Cn1 , v112
	.byte	W08
	.byte		        Cn1 , v116
	.byte	W08
	.byte	N23
	.byte	W48
	.byte		N92   , Bn0 , v116 , gtp3
	.byte	W96
	.byte		N23   , Bn1
	.byte	W24
	.byte		        An1
	.byte	W24
	.byte		        Gs1
	.byte	W24
	.byte		N05   , Ds1
	.byte	W12
	.byte		        Gs1
	.byte	W06
	.byte		        Cn2
	.byte	W06
	.byte	GOTO
	 ptrvalue	mus_qz_623_4_B1
	.byte	FINE

/********************** Track  5 **********************/

mus_qz_623_5:
	.byte		KEYSH , mus_qz_623_key+0
	.byte		VOICE , 35
	.byte		VOL   , 56*mus_qz_623_mvl/mxv
	.byte		PAN   , c_v-14
	.byte		VOL   , 44*mus_qz_623_mvl/mxv
	.byte		PRIO  , 58
	.byte		N28   , As4 , v100 , gtp1
	.byte	W36
	.byte		N03   , Fn4 , v084
	.byte	W06
	.byte		        As4 , v096
	.byte	W06
	.byte		N28   , Cn5 , v100 , gtp1
	.byte	W36
	.byte		N03   , Gs4 , v084
	.byte	W06
	.byte		        Cn5 , v096
	.byte	W06
	.byte		N28   , Cs5 , v100 , gtp1
	.byte	W36
	.byte		N03   , As4 , v084
	.byte	W06
	.byte		        Cs5 , v096
	.byte	W06
	.byte		N02   , Ds5 , v064
	.byte	W03
	.byte		        Fn5 , v068
	.byte	W03
	.byte		        Ds5 , v064
	.byte	W03
	.byte		        Fn5 , v068
	.byte	W03
	.byte		VOL   , 40*mus_qz_623_mvl/mxv
	.byte		N02   , Ds5
	.byte	W03
	.byte		        Fn5 , v076
	.byte	W03
	.byte		VOL   , 36*mus_qz_623_mvl/mxv
	.byte		N02   , Ds5 , v068
	.byte	W03
	.byte		        Fn5 , v076
	.byte	W03
	.byte		VOL   , 33*mus_qz_623_mvl/mxv
	.byte		N02   , Ds5
	.byte	W03
	.byte		        Fn5 , v084
	.byte	W03
	.byte		VOL   , 29*mus_qz_623_mvl/mxv
	.byte		N02   , Ds5 , v076
	.byte	W03
	.byte		        Fn5 , v084
	.byte	W03
	.byte		VOL   , 25*mus_qz_623_mvl/mxv
	.byte		N02   , Ds5
	.byte	W03
	.byte		        Fn5 , v088
	.byte	W03
	.byte		VOL   , 28*mus_qz_623_mvl/mxv
	.byte		N02   , Ds5 , v084
	.byte	W03
	.byte		        Fn5 , v088
	.byte	W03
	.byte		VOL   , 30*mus_qz_623_mvl/mxv
	.byte		N02   , Ds5
	.byte	W03
	.byte		        Fn5 , v096
	.byte	W03
	.byte		VOL   , 36*mus_qz_623_mvl/mxv
	.byte		N02   , Ds5 , v088
	.byte	W03
	.byte		        Fn5 , v096
	.byte	W03
	.byte		VOL   , 39*mus_qz_623_mvl/mxv
	.byte		N02   , Ds5 , v088
	.byte	W03
	.byte		        Fn5 , v096
	.byte	W03
	.byte		VOL   , 44*mus_qz_623_mvl/mxv
	.byte		N02   , Ds5 , v088
	.byte	W03
	.byte		        Fn5 , v096
	.byte	W03
	.byte		VOL   , 44*mus_qz_623_mvl/mxv
	.byte		N06   , Fs5 , v088
	.byte	W08
	.byte		        Fn5 , v084
	.byte	W08
	.byte		        Ds5 , v088
	.byte	W08
mus_qz_623_5_B1:
	.byte	W48
	.byte		N02   , Gs5 , v080
	.byte	W03
	.byte		        As5 , v088
	.byte	W03
	.byte		        Gs5 , v080
	.byte	W03
	.byte		        As5 , v088
	.byte	W03
	.byte		        Gs5
	.byte	W03
	.byte		        As5 , v092
	.byte	W03
	.byte		        Gs5 , v088
	.byte	W03
	.byte		        As5 , v092
	.byte	W03
	.byte		        Gs5 , v088
	.byte	W24
	.byte	PEND
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_623_5_B1
	.byte	W48
	.byte		N02   , En5 , v080
	.byte	W06
	.byte		        An4 , v088
	.byte	W06
	.byte		        Cs5 , v080
	.byte	W06
	.byte		        En5 , v088
	.byte	W06
	.byte		N03   , Ds5
	.byte	W06
	.byte		N02   , Fs4 , v092
	.byte	W06
	.byte		        Bn4 , v088
	.byte	W06
	.byte		N03   , Ds5
	.byte	W06
	.byte	W48
	.byte		N02   , As5 , v080
	.byte	W03
	.byte		        Cn6 , v088
	.byte	W03
	.byte		        As5 , v080
	.byte	W03
	.byte		        Cn6 , v088
	.byte	W03
	.byte		        As5
	.byte	W03
	.byte		        Cn6 , v092
	.byte	W03
	.byte		        As5 , v088
	.byte	W03
	.byte		        Cn6 , v092
	.byte	W03
	.byte		        As5 , v088
	.byte	W24
	.byte	W36
	.byte		        Fs5 , v092
	.byte	W06
	.byte		        Fs5 , v084
	.byte	W06
	.byte		        Fs5 , v096
	.byte	W12
	.byte		N03   , Ds5 , v084
	.byte	W12
	.byte		        Fs5 , v092
	.byte	W12
	.byte		        As5 , v084
	.byte	W12
	.byte		N02   , Gs5 , v092
	.byte	W08
	.byte		        Gs5 , v104
	.byte	W08
	.byte		        Gs5 , v092
	.byte	W08
	.byte		        Gs5 , v076
	.byte	W03
	.byte		        As5 , v068
	.byte	W03
	.byte		        Gs5 , v060
	.byte	W03
	.byte		        As5 , v068
	.byte	W03
	.byte		        Gs5 , v052
	.byte	W03
	.byte		        As5 , v060
	.byte	W03
	.byte		        Gs5 , v044
	.byte	W03
	.byte		        As5 , v056
	.byte	W02
	.byte		        Gs5 , v100
	.byte	W08
	.byte		        Gs5 , v108
	.byte	W08
	.byte		        Gs5 , v100
	.byte	W09
	.byte		        Gs5 , v068
	.byte	W03
	.byte		        As5 , v072
	.byte	W03
	.byte		        Gs5 , v068
	.byte	W03
	.byte		        As5 , v072
	.byte	W03
	.byte		        Gs5 , v056
	.byte	W03
	.byte		        As5 , v064
	.byte	W03
	.byte		        Gs5 , v048
	.byte	W03
	.byte		        As5 , v060
	.byte	W03
	.byte		        Gs5 , v100
	.byte	W08
	.byte		        Gs5 , v108
	.byte	W08
	.byte		        Gs5 , v100
	.byte	W08
	.byte		        Gs5 , v080
	.byte	W03
	.byte		        As5 , v088
	.byte	W03
	.byte		        Gs5 , v080
	.byte	W03
	.byte		        As5 , v088
	.byte	W03
	.byte		        Gs5 , v068
	.byte	W03
	.byte		        As5 , v080
	.byte	W03
	.byte		        Gs5 , v060
	.byte	W03
	.byte		        As5 , v072
	.byte	W02
	.byte		        Gs5 , v100
	.byte	W08
	.byte		        Gs5 , v108
	.byte	W08
	.byte		        Gs5 , v100
	.byte	W09
	.byte		        Gs5 , v080
	.byte	W03
	.byte		        As5 , v088
	.byte	W03
	.byte		        Gs5 , v064
	.byte	W03
	.byte		        As5 , v072
	.byte	W03
	.byte		        Gs5 , v060
	.byte	W03
	.byte		        As5 , v068
	.byte	W03
	.byte		        Gs5 , v044
	.byte	W03
	.byte		        As5 , v056
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
	 ptrvalue	mus_qz_623_5_B1
	.byte	FINE

/********************** Track  6 **********************/

mus_qz_623_6:
	.byte		KEYSH , mus_qz_623_key+0
	.byte		VOICE , 32
	.byte		VOL   , 56*mus_qz_623_mvl/mxv
	.byte		PAN   , c_v-26
	.byte		VOL   , 47*mus_qz_623_mvl/mxv
	.byte		PRIO  , 56
	.byte		N24   , Dn3 , v092 , gtp3
	.byte		N24   , Fn3 , v112 , gtp3
	.byte	W36
	.byte		N02   , Dn3 , v088
	.byte		N02   , Fn3 , v108
	.byte	W06
	.byte		        Dn3 , v080
	.byte		N02   , Fn3 , v100
	.byte	W06
	.byte		N23   , Cn3 , v104
	.byte		N23   , Ds3 , v112
	.byte	W36
	.byte		N02   , Cn3 , v100
	.byte		N02   , Ds3 , v092
	.byte	W06
	.byte		        Cn3 , v088
	.byte		N02   , Ds3 , v080
	.byte	W06
	.byte		N30   , As2 , v100 , gtp1
	.byte		N30   , Cs3 , v112 , gtp1
	.byte	W36
	.byte		N02   , As2 , v104
	.byte		N02   , Cs3 , v092
	.byte	W06
	.byte		        As2
	.byte		N02   , Cs3 , v080
	.byte	W06
	.byte		N66   , Gs2 , v096 , gtp1
	.byte		N66   , Ds3 , v108 , gtp1
	.byte	W12
	.byte		VOL   , 46*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        39*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        34*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        29*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        26*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        33*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        37*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        42*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        46*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        50*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        51*mus_qz_623_mvl/mxv
	.byte		N03   , Gs2 , v100
	.byte		N03   , Fs3 , v116
	.byte	W08
	.byte		        Gs2 , v084
	.byte		N03   , Fn3 , v112
	.byte	W08
	.byte		        Gs2 , v100
	.byte		N03   , Ds3 , v112
	.byte	W08
mus_qz_623_6_B1:
	.byte		N32   , Cs3 , v092
	.byte		N32   , Fn3 , v108
	.byte	W36
	.byte		N02   , Gs2 , v088
	.byte		N02   , Cs3 , v100
	.byte	W06
	.byte		        Gs2 , v084
	.byte		N02   , Cs3 , v096
	.byte	W06
	.byte		VOL   , 64*mus_qz_623_mvl/mxv
	.byte		N05   , Gs2 , v124
	.byte		N05   , Cs3 , v108
	.byte	W18
	.byte		        Fn2 , v116
	.byte		N05   , Gs2 , v108
	.byte	W06
	.byte		N23   , Cs3
	.byte		N23   , Fn3 , v124
	.byte	W24
	.byte		VOL   , 54*mus_qz_623_mvl/mxv
	.byte		N44   , Bn2 , v092 , gtp1
	.byte		N44   , Ds3 , v108 , gtp1
	.byte	W48
	.byte		N21   , Fs2 , v088
	.byte		N21   , Bn2 , v100
	.byte	W24
	.byte		        Bn2 , v088
	.byte		N21   , Ds3 , v108
	.byte	W24
	.byte		N05   , As2 , v112
	.byte		N05   , Cs3
	.byte	W12
	.byte		        Fn2 , v104
	.byte		N05   , As2 , v112
	.byte	W06
	.byte		        As2 , v104
	.byte		N05   , Cs3 , v112
	.byte	W06
	.byte	N22
	.byte		N22   , Fn3
	.byte	W24
	.byte		N05   , Cs3
	.byte		N05   , Fn3
	.byte	W12
	.byte		        As2 , v104
	.byte		N05   , Cs3 , v112
	.byte	W06
	.byte		        Cs3 , v104
	.byte		N05   , Fn3 , v112
	.byte	W06
	.byte		N22   , Cs3
	.byte		N22   , As3
	.byte	W24
	.byte		N30   , Cs3 , v112 , gtp1
	.byte		N30   , Gs3 , v112 , gtp1
	.byte	W36
	.byte		N02   , Cs3 , v092
	.byte		N02   , Gs3
	.byte	W06
	.byte		        Cs3 , v088
	.byte		N02   , Gs3
	.byte	W06
	.byte		        Cs3 , v092
	.byte		N02   , En3
	.byte	W12
	.byte		        Cs3 , v088
	.byte		N02   , En3
	.byte	W12
	.byte		        Bn2 , v092
	.byte		N02   , Fs3
	.byte	W08
	.byte		        Bn2 , v100
	.byte		N02   , Fs3
	.byte	W08
	.byte		        Bn2 , v092
	.byte		N02   , Fs3
	.byte	W08
	.byte		VOL   , 64*mus_qz_623_mvl/mxv
	.byte		N04   , As2 , v112
	.byte		N04   , Cs3 , v127
	.byte	W12
	.byte		N03   , Fn2 , v108
	.byte		N03   , As2 , v124
	.byte	W06
	.byte		        As2 , v104
	.byte		N03   , Cs3 , v120
	.byte	W06
	.byte		N04   , Cs3 , v112
	.byte		N04   , Fn3 , v127
	.byte	W12
	.byte		N03   , As2 , v108
	.byte		N03   , Cs3 , v124
	.byte	W06
	.byte		        Cs3 , v104
	.byte		N03   , Fn3 , v120
	.byte	W06
	.byte		N23   , Fn3 , v100
	.byte		N23   , As3 , v112
	.byte	W24
	.byte		N05   , Fn3 , v116
	.byte		N05   , As3
	.byte	W12
	.byte		        Fn3 , v108
	.byte		N05   , Cn4 , v116
	.byte	W12
	.byte		N68   , Fs3 , v100 , gtp1
	.byte		N68   , Cs4 , v124 , gtp1
	.byte	W72
	.byte		N21   , Fs3 , v096
	.byte		N21   , Ds4 , v120
	.byte	W24
	.byte		N44   , Gs3 , v100 , gtp1
	.byte		N44   , Cs4 , v124 , gtp1
	.byte	W48
	.byte		        Fs3 , v096 , gtp1
	.byte		N44   , Cn4 , v120 , gtp1
	.byte	W48
	.byte		        Fs3 , v100 , gtp1
	.byte		N44   , As3 , v124 , gtp1
	.byte	W48
	.byte		        Fs3 , v096 , gtp1
	.byte		N44   , Cn4 , v120 , gtp1
	.byte	W48
	.byte		VOL   , 51*mus_qz_623_mvl/mxv
	.byte		N32   , An2 , v100 , gtp2
	.byte		N32   , Cs3 , v100 , gtp2
	.byte	W36
	.byte		        Bn2 , v092 , gtp2
	.byte		N32   , Ds3 , v092 , gtp2
	.byte	W36
	.byte		N22   , Cs3 , v104
	.byte		N22   , En3
	.byte	W24
	.byte		N32   , Bn2 , v100 , gtp2
	.byte		N32   , Ds3 , v100 , gtp2
	.byte	W36
	.byte		        Cs3 , v092 , gtp3
	.byte		N32   , En3 , v092 , gtp3
	.byte	W36
	.byte		N22   , Ds3 , v100
	.byte		N22   , Fs3
	.byte	W24
	.byte		N32   , Gs2 , v104 , gtp2
	.byte		N32   , Ds3 , v104 , gtp2
	.byte	W36
	.byte		        Ds3 , v092 , gtp2
	.byte		N32   , Fs3 , v092 , gtp2
	.byte	W36
	.byte		N22   , Gs2 , v104
	.byte		N22   , Ds3
	.byte	W24
	.byte		N32   , Cs3 , v104 , gtp2
	.byte		N32   , Fn3 , v104 , gtp2
	.byte	W36
	.byte		        Ds3 , v092 , gtp2
	.byte		N32   , Fs3 , v092 , gtp2
	.byte	W36
	.byte		N22   , Fn3 , v104
	.byte		N22   , Gs3
	.byte	W24
	.byte		N19   , En3 , v100
	.byte		N19   , An3 , v108
	.byte	W24
	.byte		N03   , An2 , v092
	.byte		N03   , En3 , v100
	.byte	W08
	.byte		        An2 , v088
	.byte		N03   , En3 , v096
	.byte	W08
	.byte		        An2 , v092
	.byte		N03   , En3 , v100
	.byte	W08
	.byte		N24   , An2 , v092 , gtp3
	.byte		N24   , En3 , v100 , gtp3
	.byte	W48
	.byte		N19   , En3 , v092
	.byte		N19   , An3 , v100
	.byte	W24
	.byte		N03   , Cn3
	.byte		N03   , En3
	.byte	W08
	.byte		        Cn3 , v088
	.byte		N03   , En3 , v096
	.byte	W08
	.byte		        Cn3 , v092
	.byte		N03   , En3 , v100
	.byte	W08
	.byte		N24   , Cn3 , v092 , gtp3
	.byte		N24   , En3 , v100 , gtp3
	.byte	W48
	.byte		TIE   , Bn2 , v092
	.byte		N92   , En3 , v108 , gtp3
	.byte	W96
	.byte		N68   , Fs3 , v100 , gtp3
	.byte	W68
	.byte	W03
	.byte		EOT   , Bn2
	.byte	W01
	.byte		N23   , Cn3 , v092
	.byte		N23   , Gs3 , v100
	.byte	W24
	.byte	GOTO
	 ptrvalue	mus_qz_623_6_B1
	.byte	FINE

/********************** Track  7 **********************/

mus_qz_623_7:
	.byte		KEYSH , mus_qz_623_key+0
	.byte		VOICE , 28
	.byte		VOL   , 76*mus_qz_623_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		VOL   , 59*mus_qz_623_mvl/mxv
	.byte		PRIO  , 42
	.byte		N32   , As1 , v112
	.byte	W48
	.byte		N28   , As1 , v108 , gtp1
	.byte	W48
	.byte		        As1 , v112 , gtp1
	.byte	W48
	.byte		N02   , Gn2 , v100
	.byte	W03
	.byte		        Gn2 , v076
	.byte	W03
	.byte		        Gn2 , v080
	.byte	W03
	.byte		        Gn2 , v056
	.byte	W03
	.byte		        Gn2 , v064
	.byte	W03
	.byte		        Gn2 , v044
	.byte	W03
	.byte		        Gn2 , v032
	.byte	W03
	.byte		        Gn2 , v028
	.byte	W03
	.byte		        Gn2 , v024
	.byte	W03
	.byte		        Gn2 , v020
	.byte	W03
	.byte		        Gn2 , v028
	.byte	W03
	.byte		        Gn2 , v024
	.byte	W03
	.byte		        Gn2 , v036
	.byte	W03
	.byte		        Gn2 , v028
	.byte	W03
	.byte		        Gn2 , v040
	.byte	W03
	.byte		        Gn2 , v032
	.byte	W03
	.byte		        Gn2 , v044
	.byte	W03
	.byte		        Gn2 , v036
	.byte	W03
	.byte		        Gn2 , v044
	.byte	W03
	.byte	N02
	.byte	W03
	.byte		        Gn2 , v056
	.byte	W03
	.byte		        Gn2 , v052
	.byte	W03
	.byte		        Gn2 , v068
	.byte	W03
	.byte		        Gn2 , v064
	.byte	W03
	.byte		N07   , Gn2 , v088
	.byte	W08
	.byte		N06   , Cs2 , v080
	.byte	W08
	.byte		N07   , Gn2 , v108
	.byte	W08
mus_qz_623_7_000:
	.byte		N23   , As1 , v124
	.byte	W72
	.byte		N22   , As1 , v108
	.byte	W24
	.byte	PEND
mus_qz_623_7_001:
	.byte		N23   , As1 , v120
	.byte	W72
	.byte		N02   , Fs2 , v044
	.byte	W03
	.byte		        Fs2 , v028
	.byte	W03
	.byte		        Fs2 , v048
	.byte	W03
	.byte		        Fs2 , v036
	.byte	W03
	.byte		        Fs2 , v068
	.byte	W03
	.byte		        Fs2 , v056
	.byte	W03
	.byte		        Fs2 , v088
	.byte	W03
	.byte		        Fs2 , v072
	.byte	W03
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_623_7_000
	.byte	PATT
	 ptrvalue	mus_qz_623_7_001
	.byte		N23   , As1 , v124
	.byte	W96
	.byte	N23
	.byte	W96
	.byte		N07   , An2 , v108
	.byte	W08
	.byte		        An2 , v100
	.byte	W08
	.byte		        An2 , v108
	.byte	W08
	.byte		N17   , Bn1 , v096
	.byte	W24
	.byte		N07   , An2 , v108
	.byte	W08
	.byte		        An2 , v100
	.byte	W08
	.byte		        An2 , v108
	.byte	W08
	.byte		N17   , Bn1 , v096
	.byte	W24
	.byte		N07   , An2 , v108
	.byte	W08
	.byte		        An2 , v100
	.byte	W08
	.byte		        An2 , v108
	.byte	W08
	.byte		N17   , Bn1 , v096
	.byte	W24
	.byte		N07   , An2 , v108
	.byte	W08
	.byte		        An2 , v100
	.byte	W08
	.byte		        An2 , v108
	.byte	W08
	.byte		N17   , Bn1 , v092
	.byte	W24
mus_qz_623_7_002:
	.byte		N23   , As1 , v124
	.byte	W72
	.byte		        As1 , v108
	.byte	W24
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_623_7_002
	.byte		N23   , As1 , v124
	.byte	W96
	.byte	N23
	.byte	W68
	.byte	W03
	.byte		N02   , Fs2 , v044
	.byte	W03
	.byte		        Fs2 , v028
	.byte	W03
	.byte		        Fs2 , v048
	.byte	W03
	.byte		        Fs2 , v036
	.byte	W03
	.byte		        Fs2 , v068
	.byte	W03
	.byte		        Fs2 , v056
	.byte	W03
	.byte		        Fs2 , v088
	.byte	W03
	.byte		        Fs2 , v072
	.byte	W04
	.byte		N11   , Cs2 , v088
	.byte	W24
	.byte	N07
	.byte	W08
	.byte		        Cs2 , v080
	.byte	W08
	.byte		        Cs2 , v088
	.byte	W08
	.byte		N23   , As1 , v124
	.byte	W48
	.byte	W24
	.byte		N07   , An2 , v088
	.byte	W08
	.byte		        An2 , v080
	.byte	W08
	.byte		        An2 , v088
	.byte	W08
	.byte		N23   , As1 , v124
	.byte	W48
	.byte	N23
	.byte	W96
	.byte	W68
	.byte	W03
	.byte	N23
	.byte	W24
	.byte	W01
	.byte	GOTO
	 ptrvalue	mus_qz_623_7_000
	.byte	FINE

/********************** Track  8 **********************/

mus_qz_623_8:
	.byte		KEYSH , mus_qz_623_key+0
	.byte		VOICE , 39
	.byte		VOL   , 71*mus_qz_623_mvl/mxv
	.byte		PAN   , c_v-22
	.byte		VOL   , 55*mus_qz_623_mvl/mxv
	.byte		PRIO  , 40
	.byte		N28   , Cn2 , v108
	.byte	W48
	.byte		        Cn2 , v100
	.byte	W48
	.byte		        Cn2 , v108
	.byte	W48
	.byte	W96
mus_qz_623_8_B1:
	.byte		N28   , Cn2 , v108
	.byte	W96
	.byte	W96
	.byte		        Cn2 , v100
	.byte	W96
	.byte	W96
	.byte		        Cn2 , v104
	.byte	W96
	.byte		        Cn2 , v108
	.byte	W96
mus_qz_623_8_000:
	.byte	W24
	.byte		N22   , Cn2 , v096
	.byte	W48
	.byte		        Cn2 , v088
	.byte	W24
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_623_8_000
	.byte		N22   , Cn2 , v096
	.byte	W96
	.byte	W72
	.byte	N22
	.byte	W24
	.byte	N22
	.byte	W96
	.byte	W96
	.byte	W48
	.byte		        Cn2 , v092
	.byte	W48
	.byte	W48
	.byte		        Cn2 , v088
	.byte	W48
	.byte	N23
	.byte	W96
	.byte	W68
	.byte	W03
	.byte	N22
	.byte	W24
	.byte	W01
	.byte	GOTO
	 ptrvalue	mus_qz_623_8_B1
	.byte	FINE

/********************** Track  9 **********************/

mus_qz_623_9:
	.byte		KEYSH , mus_qz_623_key+0
	.byte		VOICE , 39
	.byte		VOL   , 71*mus_qz_623_mvl/mxv
	.byte		PAN   , c_v-29
	.byte		VOL   , 59*mus_qz_623_mvl/mxv
	.byte		PRIO  , 40
	.byte	W36
	.byte		N02   , Dn1 , v048
	.byte	W03
	.byte		        Dn1 , v064
	.byte	W03
	.byte		        Dn1 , v076
	.byte	W03
	.byte		        Dn1 , v092
	.byte	W03
	.byte		N05   , Dn1 , v112
	.byte	W36
	.byte		        Dn1 , v108
	.byte	W06
	.byte		        Dn1 , v096
	.byte	W06
	.byte	W36
	.byte		N02   , Dn1 , v048
	.byte	W03
	.byte		        Dn1 , v064
	.byte	W03
	.byte		        Dn1 , v076
	.byte	W03
	.byte		        Dn1 , v092
	.byte	W03
	.byte		N05   , Dn1 , v112
	.byte	W56
	.byte	W01
	.byte		N02   , Dn1 , v048
	.byte	W03
	.byte		        Dn1 , v064
	.byte	W03
	.byte		        Dn1 , v080
	.byte	W03
	.byte		        Dn1 , v100
	.byte	W03
	.byte		        Dn1 , v120
	.byte	W03
	.byte		N05   , Dn1 , v108
	.byte	W08
	.byte		        Dn1 , v096
	.byte	W08
	.byte		        Dn1 , v104
	.byte	W08
mus_qz_623_9_B1:
	.byte		N05   , Dn1 , v104
	.byte	W36
	.byte		        Dn1 , v096
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v096
	.byte	W18
	.byte		        Dn1 , v080
	.byte	W06
	.byte		N02   , Dn1 , v092
	.byte	W03
	.byte		        Dn1 , v076
	.byte	W03
	.byte		        Dn1 , v084
	.byte	W03
	.byte		        Dn1 , v068
	.byte	W03
	.byte		        Dn1 , v072
	.byte	W03
	.byte		        Dn1 , v060
	.byte	W03
	.byte		        Dn1 , v068
	.byte	W03
	.byte		        Dn1 , v056
	.byte	W03
	.byte		N05   , Dn1 , v104
	.byte	W12
	.byte		        Dn1 , v084
	.byte	W24
	.byte		        Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v076
	.byte	W06
	.byte		        Dn1 , v092
	.byte	W12
	.byte		        Dn1 , v084
	.byte	W12
	.byte		N02   , Dn1 , v092
	.byte	W03
	.byte		        Dn1 , v076
	.byte	W03
	.byte		        Dn1 , v084
	.byte	W03
	.byte		        Dn1 , v068
	.byte	W03
	.byte		        Dn1 , v072
	.byte	W03
	.byte		        Dn1 , v060
	.byte	W03
	.byte		        Dn1 , v068
	.byte	W03
	.byte		        Dn1 , v056
	.byte	W03
	.byte		N05   , Dn1 , v104
	.byte	W12
	.byte		        Dn1 , v080
	.byte	W24
	.byte		        Dn1 , v096
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v096
	.byte	W12
	.byte		        Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v080
	.byte	W06
	.byte		N02   , Dn1 , v092
	.byte	W03
	.byte		        Dn1 , v076
	.byte	W03
	.byte		        Dn1 , v084
	.byte	W03
	.byte		        Dn1 , v068
	.byte	W03
	.byte		        Dn1 , v072
	.byte	W03
	.byte		        Dn1 , v060
	.byte	W03
	.byte		        Dn1 , v068
	.byte	W03
	.byte		        Dn1 , v056
	.byte	W03
	.byte		N05   , Dn1 , v096
	.byte	W12
	.byte		N06   , Dn1 , v084
	.byte	W24
	.byte		N05   , Dn1 , v092
	.byte	W06
	.byte		        Dn1 , v084
	.byte	W06
	.byte		        Dn1 , v100
	.byte	W08
	.byte		        Dn1 , v088
	.byte	W08
	.byte		        Dn1 , v100
	.byte	W08
	.byte	N05
	.byte	W08
	.byte		        Dn1 , v092
	.byte	W08
	.byte		        Dn1 , v100
	.byte	W08
	.byte		        Dn1 , v104
	.byte	W12
	.byte		        Dn1 , v084
	.byte	W06
	.byte		        Dn1 , v092
	.byte	W06
	.byte		        Dn1 , v104
	.byte	W12
	.byte		        Dn1 , v084
	.byte	W06
	.byte		        Dn1 , v092
	.byte	W06
	.byte	N02
	.byte	W03
	.byte		        Dn1 , v076
	.byte	W03
	.byte		        Dn1 , v084
	.byte	W03
	.byte		        Dn1 , v068
	.byte	W03
	.byte		N03   , Dn1 , v092
	.byte	W06
	.byte		        Dn1 , v084
	.byte	W06
	.byte		N05   , Dn1 , v100
	.byte	W12
	.byte		        Dn1 , v092
	.byte	W12
	.byte		N04   , Dn1 , v100
	.byte	W12
	.byte		N05   , Dn1 , v092
	.byte	W06
	.byte		        Dn1 , v084
	.byte	W06
	.byte		        Dn1 , v100
	.byte	W12
	.byte		        Dn1 , v092
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v100
	.byte	W12
	.byte	N05
	.byte	W11
	.byte		N02   , Dn1 , v092
	.byte	W03
	.byte		        Dn1 , v076
	.byte	W03
	.byte		        Dn1 , v084
	.byte	W03
	.byte		        Dn1 , v068
	.byte	W03
	.byte		        Dn1 , v072
	.byte	W03
	.byte		        Dn1 , v060
	.byte	W03
	.byte		        Dn1 , v068
	.byte	W03
	.byte		        Dn1 , v056
	.byte	W04
	.byte		N05   , Dn1 , v100
	.byte	W08
	.byte		        Dn1 , v108
	.byte	W08
	.byte		        Dn1 , v100
	.byte	W32
	.byte	N05
	.byte	W08
	.byte		        Dn1 , v112
	.byte	W08
	.byte		        Dn1 , v100
	.byte	W32
	.byte	N05
	.byte	W08
	.byte		        Dn1 , v112
	.byte	W08
	.byte		        Dn1 , v100
	.byte	W32
	.byte	N05
	.byte	W08
	.byte		        Dn1 , v108
	.byte	W08
	.byte		        Dn1 , v104
	.byte	W32
	.byte		        Dn1 , v112
	.byte	W12
	.byte		        Dn1 , v100
	.byte	W24
	.byte		        Dn1 , v108
	.byte	W06
	.byte		        Dn1 , v100
	.byte	W06
	.byte		        Dn1 , v108
	.byte	W12
	.byte		        Dn1 , v096
	.byte	W12
	.byte		        Dn1 , v108
	.byte	W12
	.byte		        Dn1 , v096
	.byte	W12
	.byte		        Dn1 , v108
	.byte	W12
	.byte		        Dn1 , v096
	.byte	W24
	.byte		        Dn1 , v104
	.byte	W06
	.byte		        Dn1 , v092
	.byte	W06
	.byte		        Dn1 , v108
	.byte	W12
	.byte		        Dn1 , v096
	.byte	W12
	.byte		N02   , Dn1 , v092
	.byte	W03
	.byte		        Dn1 , v076
	.byte	W03
	.byte		        Dn1 , v084
	.byte	W03
	.byte		        Dn1 , v068
	.byte	W03
	.byte		        Dn1 , v072
	.byte	W03
	.byte		        Dn1 , v060
	.byte	W03
	.byte		        Dn1 , v068
	.byte	W03
	.byte		        Dn1 , v056
	.byte	W03
	.byte		N05   , Dn1 , v108
	.byte	W12
	.byte		        Dn1 , v096
	.byte	W24
	.byte		        Dn1 , v108
	.byte	W06
	.byte		        Dn1 , v100
	.byte	W06
	.byte		        Dn1 , v108
	.byte	W12
	.byte		        Dn1 , v100
	.byte	W12
	.byte		        Dn1 , v104
	.byte	W12
	.byte		        Dn1 , v096
	.byte	W12
	.byte		        Dn1 , v108
	.byte	W12
	.byte		        Dn1 , v096
	.byte	W24
	.byte		        Dn1 , v104
	.byte	W06
	.byte		        Dn1 , v096
	.byte	W06
	.byte		        Dn1 , v108
	.byte	W12
	.byte		        Dn1 , v096
	.byte	W12
	.byte		N02   , Dn1 , v092
	.byte	W03
	.byte		        Dn1 , v076
	.byte	W03
	.byte		        Dn1 , v084
	.byte	W03
	.byte		        Dn1 , v068
	.byte	W03
	.byte		N05   , Dn1 , v104
	.byte	W06
	.byte		        Dn1 , v096
	.byte	W06
	.byte		N02   , Dn1 , v092
	.byte	W03
	.byte		        Dn1 , v076
	.byte	W03
	.byte		        Dn1 , v084
	.byte	W03
	.byte		        Dn1 , v068
	.byte	W03
	.byte		        Dn1 , v072
	.byte	W03
	.byte		        Dn1 , v060
	.byte	W03
	.byte		        Dn1 , v068
	.byte	W03
	.byte		        Dn1 , v056
	.byte	W03
	.byte		N05   , Dn1 , v104
	.byte	W08
	.byte		        Dn1 , v092
	.byte	W08
	.byte		        Dn1 , v104
	.byte	W56
	.byte		N02   , Dn1 , v092
	.byte	W03
	.byte		        Dn1 , v076
	.byte	W03
	.byte		        Dn1 , v084
	.byte	W03
	.byte		        Dn1 , v068
	.byte	W03
	.byte		        Dn1 , v072
	.byte	W03
	.byte		        Dn1 , v060
	.byte	W03
	.byte		        Dn1 , v068
	.byte	W03
	.byte		        Dn1 , v056
	.byte	W03
	.byte		N05   , Dn1 , v104
	.byte	W08
	.byte		        Dn1 , v096
	.byte	W08
	.byte		        Dn1 , v104
	.byte	W56
	.byte	W96
	.byte		N02   , Dn1 , v100
	.byte	W03
	.byte		        Dn1 , v072
	.byte	W03
	.byte		        Dn1 , v056
	.byte	W03
	.byte		        Dn1 , v044
	.byte	W03
	.byte		        Dn1 , v036
	.byte	W03
	.byte		        Dn1 , v028
	.byte	W03
	.byte		        Dn1 , v036
	.byte	W03
	.byte		        Dn1 , v032
	.byte	W03
	.byte		        Dn1 , v044
	.byte	W03
	.byte		        Dn1 , v036
	.byte	W03
	.byte		        Dn1 , v048
	.byte	W03
	.byte		        Dn1 , v040
	.byte	W03
	.byte		        Dn1 , v052
	.byte	W03
	.byte		        Dn1 , v048
	.byte	W03
	.byte		        Dn1 , v056
	.byte	W03
	.byte		        Dn1 , v048
	.byte	W03
	.byte		        Dn1 , v060
	.byte	W03
	.byte		        Dn1 , v052
	.byte	W03
	.byte		        Dn1 , v064
	.byte	W03
	.byte		        Dn1 , v052
	.byte	W03
	.byte		        Dn1 , v068
	.byte	W03
	.byte		        Dn1 , v056
	.byte	W03
	.byte		        Dn1 , v072
	.byte	W03
	.byte		        Dn1 , v056
	.byte	W03
	.byte		N05   , Dn1 , v112
	.byte	W06
	.byte		        Dn1 , v100
	.byte	W06
	.byte		        Dn1 , v120
	.byte	W06
	.byte		        Dn1 , v100
	.byte	W06
	.byte	GOTO
	 ptrvalue	mus_qz_623_9_B1
	.byte	FINE

/********************** Track 10 **********************/

mus_qz_623_10:
	.byte		KEYSH , mus_qz_623_key+0
	.byte		VOICE , 34
	.byte		VOL   , 56*mus_qz_623_mvl/mxv
	.byte		PAN   , c_v+6
	.byte		VOL   , 44*mus_qz_623_mvl/mxv
	.byte		PRIO  , 50
	.byte	W48
	.byte	W48
	.byte	W48
	.byte	W96
mus_qz_623_10_B1:
	.byte		N32   , Cs3 , v112 , gtp2
	.byte	W36
	.byte		N04   , Cs3 , v100
	.byte	W06
	.byte		        Ds3 , v104
	.byte	W06
	.byte		        Fn3 , v112
	.byte	W18
	.byte		        Cs3 , v104
	.byte	W06
	.byte		        Gs2 , v116
	.byte	W12
	.byte		        Fn3 , v108
	.byte	W12
	.byte		N32   , Ds3 , v112 , gtp2
	.byte	W36
	.byte		N04   , En3 , v096
	.byte	W06
	.byte		        Ds3 , v084
	.byte	W06
	.byte		N22   , Bn2 , v104
	.byte	W24
	.byte		        Ds3 , v112
	.byte	W24
	.byte		N28   , Cs3
	.byte	W36
	.byte		N04   , Cs3 , v108
	.byte	W06
	.byte		        Ds3 , v100
	.byte	W06
	.byte		        Fn3 , v112
	.byte	W18
	.byte		        Cs3 , v104
	.byte	W06
	.byte		        As2 , v112
	.byte	W12
	.byte		        Fn3 , v108
	.byte	W12
	.byte		N22   , En3 , v112
	.byte	W24
	.byte		        Ds3 , v104
	.byte	W24
	.byte		        An2 , v108
	.byte	W24
	.byte		        Bn2 , v104
	.byte	W24
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		N16   , An3 , v100
	.byte	W18
	.byte		        Gs3 , v088
	.byte	W18
	.byte		N22   , Fs3 , v100
	.byte	W24
	.byte		N10   , Cs3 , v088
	.byte	W12
	.byte		        Ds3 , v104
	.byte	W12
	.byte		        En3 , v092
	.byte	W12
	.byte		        Fs3 , v096
	.byte	W12
	.byte		N22   , Bn3 , v100
	.byte	W24
	.byte		N10   , An3 , v092
	.byte	W12
	.byte		        Gs3 , v100
	.byte	W12
	.byte		        Fs3 , v092
	.byte	W12
	.byte		        Gs3 , v104
	.byte	W12
	.byte		        An3 , v096
	.byte	W12
	.byte		N22   , Gs3 , v100
	.byte	W24
	.byte		        Fs3 , v092
	.byte	W24
	.byte		        En3 , v100
	.byte	W24
	.byte		        Ds3 , v092
	.byte	W24
	.byte		N10   , Cs4 , v100
	.byte	W12
	.byte		        Bn3 , v092
	.byte	W12
	.byte		        Gs3 , v100
	.byte	W12
	.byte		N22   , Fn3 , v092
	.byte	W24
	.byte		N10   , Fs3 , v104
	.byte	W12
	.byte		N16   , Gs3 , v092
	.byte	W24
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_623_10_B1
	.byte	FINE

/********************** Track 11 **********************/

mus_qz_623_11:
	.byte		KEYSH , mus_qz_623_key+0
	.byte		VOICE , 29
	.byte		VOL   , 22*mus_qz_623_mvl/mxv
	.byte		PAN   , c_v+56
	.byte		VOL   , 17*mus_qz_623_mvl/mxv
	.byte		PRIO  , 38
	.byte	W12
	.byte		N23   , As3 , v104
	.byte	W36
	.byte		N02   , Fn3
	.byte	W06
	.byte		        As3
	.byte	W06
	.byte		N23   , Cn4
	.byte	W36
	.byte		N02   , Gs3
	.byte	W06
	.byte		        Cn4
	.byte	W06
	.byte		N23   , Cs4
	.byte	W36
	.byte		N02   , As3
	.byte	W06
	.byte		        Cs4
	.byte	W06
	.byte		N56   , Ds4 , v104 , gtp3
	.byte	W12
	.byte		VOL   , 15*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        13*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        11*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        9*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        7*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        10*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        12*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        14*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        15*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        17*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        17*mus_qz_623_mvl/mxv
	.byte	W12
mus_qz_623_11_B1:
	.byte	W12
	.byte		N04   , Cs3 , v116
	.byte	W18
	.byte		        Gs2 , v108
	.byte	W06
	.byte		N44   , Fn3 , v120 , gtp1
	.byte	W48
	.byte		N11   , Cs3 , v112
	.byte	W12
	.byte		N03   , Fn3 , v104
	.byte	W12
	.byte		N21   , Gs3 , v116
	.byte	W24
	.byte		        Fs3 , v108
	.byte	W24
	.byte		        Cs3 , v116
	.byte	W24
	.byte		        Ds3 , v108
	.byte	W12
	.byte	W12
	.byte		N03   , Cs3 , v116
	.byte	W12
	.byte		N04   , Gs2 , v108
	.byte	W06
	.byte		        Cs3 , v116
	.byte	W06
	.byte		N44   , Fn3 , v116 , gtp1
	.byte	W48
	.byte		N11   , Cs3 , v108
	.byte	W12
	.byte		N05   , Fn3 , v100
	.byte	W12
	.byte		N21   , Gs3 , v116
	.byte	W24
	.byte		        Fs3 , v104
	.byte	W24
	.byte		        Cs3 , v112
	.byte	W24
	.byte		        Ds3 , v108
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
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_623_11_B1
	.byte	FINE

/********************** Track 12 **********************/

mus_qz_623_12:
	.byte		KEYSH , mus_qz_623_key+0
	.byte		VOICE , 23
	.byte		VOL   , 14*mus_qz_623_mvl/mxv
	.byte		PAN   , c_v+56
	.byte		VOL   , 10*mus_qz_623_mvl/mxv
	.byte		PRIO  , 36
	.byte	W48
	.byte	W48
	.byte	W48
	.byte	W96
mus_qz_623_12_B1:
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W24
	.byte		N32   , Cs4 , v120 , gtp2
	.byte	W24
	.byte		VOL   , 10*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        9*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        11*mus_qz_623_mvl/mxv
	.byte		N32   , Ds4 , v108 , gtp2
	.byte	W24
	.byte		VOL   , 10*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        9*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        11*mus_qz_623_mvl/mxv
	.byte		N22   , En4 , v120
	.byte	W24
	.byte		N10   , Fs4
	.byte	W36
	.byte		N22   , Bn4
	.byte	W24
	.byte		N10   , An4
	.byte	W12
	.byte		        Gs4 , v112
	.byte	W12
	.byte		        An4 , v120
	.byte	W12
	.byte		N32   , Gs4 , v120 , gtp2
	.byte	W24
	.byte		VOL   , 10*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        9*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        11*mus_qz_623_mvl/mxv
	.byte		N08   , Ds4 , v112
	.byte	W24
	.byte		N10   , Ds4 , v120
	.byte	W12
	.byte		        Gs4 , v112
	.byte	W12
	.byte		        Ds4 , v116
	.byte	W12
	.byte		        Fn4 , v120
	.byte	W24
	.byte		        Fn4 , v116
	.byte	W12
	.byte		N32   , Fs4 , v120 , gtp2
	.byte	W24
	.byte		VOL   , 10*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        9*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        11*mus_qz_623_mvl/mxv
	.byte		N22   , Gs4
	.byte	W24
	.byte		N44   , An4 , v120 , gtp2
	.byte	W36
	.byte		VOL   , 10*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        9*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        11*mus_qz_623_mvl/mxv
	.byte	W12
	.byte	N10
	.byte	W12
	.byte		        Gs4 , v112
	.byte	W12
	.byte		        Fs4 , v120
	.byte	W12
	.byte		N44   , An4 , v120 , gtp2
	.byte	W36
	.byte		VOL   , 10*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        9*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        11*mus_qz_623_mvl/mxv
	.byte	W24
	.byte	W12
	.byte		N04   , Gs4 , v112
	.byte	W06
	.byte		        An4 , v108
	.byte	W06
	.byte		VOL   , 4*mus_qz_623_mvl/mxv
	.byte		N92   , Bn4 , v112 , gtp2
	.byte	W02
	.byte		VOL   , 3*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        3*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        3*mus_qz_623_mvl/mxv
	.byte	W06
	.byte		        4*mus_qz_623_mvl/mxv
	.byte	W04
	.byte		        4*mus_qz_623_mvl/mxv
	.byte	W05
	.byte		        5*mus_qz_623_mvl/mxv
	.byte	W07
	.byte		        6*mus_qz_623_mvl/mxv
	.byte	W05
	.byte		        7*mus_qz_623_mvl/mxv
	.byte	W04
	.byte		        7*mus_qz_623_mvl/mxv
	.byte	W05
	.byte		        9*mus_qz_623_mvl/mxv
	.byte	W03
	.byte		        9*mus_qz_623_mvl/mxv
	.byte	W03
	.byte		        10*mus_qz_623_mvl/mxv
	.byte	W04
	.byte		        11*mus_qz_623_mvl/mxv
	.byte	W05
	.byte		        10*mus_qz_623_mvl/mxv
	.byte	W03
	.byte		        9*mus_qz_623_mvl/mxv
	.byte	W04
	.byte		        7*mus_qz_623_mvl/mxv
	.byte	W02
	.byte		        7*mus_qz_623_mvl/mxv
	.byte	W03
	.byte		        6*mus_qz_623_mvl/mxv
	.byte	W07
	.byte		        5*mus_qz_623_mvl/mxv
	.byte	W05
	.byte		        4*mus_qz_623_mvl/mxv
	.byte	W07
	.byte		        5*mus_qz_623_mvl/mxv
	.byte		N68   , Ds4 , v112 , gtp2
	.byte	W02
	.byte		VOL   , 7*mus_qz_623_mvl/mxv
	.byte	W03
	.byte		        7*mus_qz_623_mvl/mxv
	.byte	W03
	.byte		        8*mus_qz_623_mvl/mxv
	.byte	W04
	.byte		        9*mus_qz_623_mvl/mxv
	.byte	W02
	.byte		        10*mus_qz_623_mvl/mxv
	.byte	W03
	.byte		        10*mus_qz_623_mvl/mxv
	.byte	W03
	.byte		        11*mus_qz_623_mvl/mxv
	.byte	W04
	.byte		        11*mus_qz_623_mvl/mxv
	.byte	W02
	.byte		        11*mus_qz_623_mvl/mxv
	.byte	W22
	.byte		        11*mus_qz_623_mvl/mxv
	.byte	W02
	.byte		        10*mus_qz_623_mvl/mxv
	.byte	W03
	.byte		        9*mus_qz_623_mvl/mxv
	.byte	W03
	.byte		        9*mus_qz_623_mvl/mxv
	.byte	W04
	.byte		        7*mus_qz_623_mvl/mxv
	.byte	W02
	.byte		        6*mus_qz_623_mvl/mxv
	.byte	W03
	.byte		        5*mus_qz_623_mvl/mxv
	.byte	W03
	.byte		        4*mus_qz_623_mvl/mxv
	.byte	W03
	.byte		        11*mus_qz_623_mvl/mxv
	.byte	W01
	.byte	GOTO
	 ptrvalue	mus_qz_623_12_B1
	.byte		N04   , Ds4 , v112
	.byte	W06
	.byte		        Cs4
	.byte	W06
	.byte		        Cn4
	.byte	W06
	.byte		        Ds4
	.byte	W04
	.byte	FINE

/******************************************************/
	.p2align	2

mus_qz_623:
	.byte	12	/* NumTrks */
	.byte	0	/* NumBlks */
	.byte	mus_qz_623_pri	/* Priority */
	.byte	mus_qz_623_rev	/* Reverb. */
space64 4
	ptrvalue	mus_qz_623_grp

	ptrvalue	mus_qz_623_1
	ptrvalue	mus_qz_623_2
	ptrvalue	mus_qz_623_3
	ptrvalue	mus_qz_623_4
	ptrvalue	mus_qz_623_5
	ptrvalue	mus_qz_623_6
	ptrvalue	mus_qz_623_7
	ptrvalue	mus_qz_623_8
	ptrvalue	mus_qz_623_9
	ptrvalue	mus_qz_623_10
	ptrvalue	mus_qz_623_11
	ptrvalue	mus_qz_623_12

	.end
