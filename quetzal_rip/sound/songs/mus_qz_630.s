	.include "asm/macros/bit_width.inc"

	.include "MPlayDef.s"

/* Moemon Quetzal song 630, disassembled from the ROM by tools/quetzal_rip/songs.py.
   "org" comments give the ROM address of the bytes that follow. */

	.equ	mus_qz_630_grp, voicegroup_qz_songs
	.equ	mus_qz_630_pri, 0
	.equ	mus_qz_630_rev, reverb_set+0
	.equ	mus_qz_630_mvl, 127
	.equ	mus_qz_630_key, 0
	.equ	mus_qz_630_tbs, 1
	.equ	mus_qz_630_exg, 1
	.equ	mus_qz_630_cmp, 1

	.section .rodata
	.global	mus_qz_630
	.p2align	2
/* org 0x0968289C */

/********************** Track  1 **********************/

mus_qz_630_1:
	.byte		KEYSH , mus_qz_630_key+0
	.byte		TEMPO , 108*mus_qz_630_tbs/2
	.byte		TEMPO , 108*mus_qz_630_tbs/2
	.byte		VOICE , 0
	.byte		BENDR , 6
	.byte		VOL   , 48*mus_qz_630_mvl/mxv
	.byte		PAN   , c_v-39
	.byte	W02
	.byte		PRIO  , 64
	.byte	W92
	.byte	W02
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		VOICE , 21
	.byte		VOL   , 62*mus_qz_630_mvl/mxv
	.byte		PAN   , c_v-34
	.byte		BEND  , c_v-18
	.byte		N23   , Fn1 , v032
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W21
	.byte		        c_v-12
	.byte		N23   , Fn3 , v048
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W06
	.byte		VOL   , 53*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        42*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        31*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        62*mus_qz_630_mvl/mxv
	.byte		N11   , Fs3
	.byte	W12
	.byte		BEND  , c_v-9
	.byte		N23   , Ds3
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W06
	.byte		VOL   , 53*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        42*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        31*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        62*mus_qz_630_mvl/mxv
	.byte		N11   , Fs3
	.byte	W12
	.byte		VOL   , 62*mus_qz_630_mvl/mxv
	.byte		BEND  , c_v-18
	.byte		N23   , Fn1 , v028
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W21
	.byte		        c_v-12
	.byte		N23   , Fn3 , v048
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W06
	.byte		VOL   , 53*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        42*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        31*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        62*mus_qz_630_mvl/mxv
	.byte		N11   , Ds3
	.byte	W12
	.byte		BEND  , c_v-9
	.byte		N23   , Cn3
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W06
	.byte		VOL   , 53*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        42*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        31*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        62*mus_qz_630_mvl/mxv
	.byte		N11   , Ds3
	.byte	W12
	.byte		VOICE , 67
	.byte		VOL   , 62*mus_qz_630_mvl/mxv
	.byte		PAN   , c_v-32
	.byte		N23   , Cn4 , v124
	.byte	W09
	.byte		VOL   , 53*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        42*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        31*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        62*mus_qz_630_mvl/mxv
	.byte		N23   , As3
	.byte	W09
	.byte		VOL   , 53*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        42*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        31*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        62*mus_qz_630_mvl/mxv
	.byte		BEND  , c_v-6
	.byte		N23   , Cs4
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W06
	.byte		VOL   , 53*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        42*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        31*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        62*mus_qz_630_mvl/mxv
	.byte		N11   , Cn4
	.byte	W12
	.byte		        As3
	.byte	W12
mus_qz_630_1_000:
	.byte		N11   , Cn4 , v124
	.byte	W12
	.byte		        As3
	.byte	W12
	.byte		N23   , Fs3
	.byte	W06
	.byte		BEND  , c_v+8
	.byte	W03
	.byte		VOL   , 53*mus_qz_630_mvl/mxv
	.byte		BEND  , c_v+0
	.byte	W03
	.byte		VOL   , 42*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        31*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        62*mus_qz_630_mvl/mxv
	.byte		N44   , Fn3 , v124 , gtp3
	.byte	W09
	.byte		VOL   , 53*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        42*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        31*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        20*mus_qz_630_mvl/mxv
	.byte	W24
	.byte	PEND
mus_qz_630_1_001:
	.byte		VOL   , 62*mus_qz_630_mvl/mxv
	.byte		N23   , Fs3 , v124
	.byte	W09
	.byte		VOL   , 53*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        42*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        31*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        62*mus_qz_630_mvl/mxv
	.byte		N23   , As3
	.byte	W09
	.byte		VOL   , 53*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        42*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        31*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        62*mus_qz_630_mvl/mxv
	.byte		BEND  , c_v+2
	.byte		N11   , Cn4
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W09
	.byte		N11   , Cs4
	.byte	W12
	.byte		        Cn4
	.byte	W12
	.byte		        As3
	.byte	W12
	.byte	PEND
	.byte		        Cn4
	.byte	W12
	.byte		        Cs4
	.byte	W12
	.byte		        Ds4
	.byte	W12
	.byte		        Cs4
	.byte	W12
	.byte		N03   , Cn4 , v092
	.byte	W03
	.byte		N02   , Cn4 , v048
	.byte	W04
	.byte		        Cn4 , v072
	.byte	W02
	.byte		        Cn4 , v056
	.byte	W03
	.byte		        Cn4 , v064
	.byte	W02
	.byte		        Cn4 , v060
	.byte	W02
	.byte		        Cn4 , v040
	.byte	W03
	.byte	N02
	.byte	W02
	.byte		        Cn4 , v036
	.byte	W03
	.byte		        Cn4 , v032
	.byte	W02
	.byte		        Cn4 , v028
	.byte	W02
	.byte		        Cn4 , v024
	.byte	W03
	.byte	N02
	.byte	W02
	.byte		        Cn4 , v020
	.byte	W03
	.byte		        Cn4 , v016
	.byte	W02
	.byte	N02
	.byte	W02
	.byte	N02
	.byte	W03
	.byte		        Cn4 , v012
	.byte	W02
	.byte	N02
	.byte	W03
mus_qz_630_1_002:
	.byte		VOICE , 42
	.byte		VOL   , 59*mus_qz_630_mvl/mxv
	.byte		BEND  , c_v-3
	.byte		N05   , Cs4 , v127
	.byte	W06
	.byte		        Cs5 , v060
	.byte	W06
	.byte		        Cn4 , v127
	.byte	W06
	.byte		        Cn5 , v060
	.byte	W06
	.byte		        As3 , v127
	.byte	W06
	.byte		        As4 , v060
	.byte	W06
	.byte		        Fs3 , v127
	.byte	W06
	.byte		        Fs4 , v064
	.byte	W06
	.byte		        As3 , v127
	.byte	W06
	.byte		        As4 , v056
	.byte	W06
	.byte		        As3 , v044
	.byte	W06
	.byte		        As4 , v016
	.byte	W06
	.byte		        As3 , v024
	.byte	W06
	.byte		        As4 , v008
	.byte	W06
	.byte		        As3 , v016
	.byte	W06
	.byte		        As4 , v004
	.byte	W06
	.byte	PEND
mus_qz_630_1_003:
	.byte		N05   , Cn4 , v127
	.byte	W06
	.byte		        Cn5 , v060
	.byte	W06
	.byte		        As3 , v127
	.byte	W06
	.byte		        As4 , v060
	.byte	W06
	.byte		        Fs3 , v127
	.byte	W06
	.byte		        Fs4 , v060
	.byte	W06
	.byte		        Ds3 , v127
	.byte	W06
	.byte		        Ds4 , v064
	.byte	W06
	.byte		        Fn3 , v127
	.byte	W06
	.byte		        Fn4 , v056
	.byte	W06
	.byte		        Fn3 , v044
	.byte	W06
	.byte		        Fn4 , v016
	.byte	W06
	.byte		        Fn3 , v024
	.byte	W06
	.byte		        Fn4 , v008
	.byte	W06
	.byte		        Fn3 , v016
	.byte	W06
	.byte		        Fn4 , v004
	.byte	W06
	.byte	PEND
mus_qz_630_1_004:
	.byte		N05   , Fn3 , v127
	.byte	W06
	.byte		        Fn4 , v060
	.byte	W06
	.byte		        Fn3 , v040
	.byte	W06
	.byte		        Fn4 , v016
	.byte	W06
	.byte		        Fn3 , v127
	.byte	W06
	.byte		        Fn4 , v060
	.byte	W06
	.byte		        Fn3 , v040
	.byte	W06
	.byte		        Fn4 , v016
	.byte	W06
	.byte		        Fs3 , v127
	.byte	W06
	.byte		        Fs4 , v060
	.byte	W06
	.byte		        Fs3 , v040
	.byte	W06
	.byte		        Fs4 , v016
	.byte	W06
	.byte		        As3 , v127
	.byte	W06
	.byte		        As4 , v060
	.byte	W06
	.byte		        Fs3 , v127
	.byte	W06
	.byte		        As3
	.byte	W06
	.byte	PEND
mus_qz_630_1_005:
	.byte		PAN   , c_v-34
	.byte		VOL   , 21*mus_qz_630_mvl/mxv
	.byte		BEND  , c_v-18
	.byte		N05   , Cn4 , v127
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W03
	.byte		N03   , Cn5 , v100
	.byte	W03
	.byte		N01   , Cn5 , v060
	.byte	W04
	.byte		        Cn5 , v056
	.byte	W02
	.byte		        Cn5 , v048
	.byte	W03
	.byte	N01
	.byte	W02
	.byte		        Cn5 , v044
	.byte	W02
	.byte		        Cn5 , v040
	.byte	W03
	.byte	N01
	.byte	W02
	.byte		        Cn5 , v036
	.byte	W03
	.byte		        Cn5 , v032
	.byte	W02
	.byte		        Cn5 , v028
	.byte	W02
	.byte		        Cn5 , v024
	.byte	W03
	.byte	N01
	.byte	W02
	.byte		        Cn5 , v020
	.byte	W03
	.byte		        Cn5 , v016
	.byte	W02
	.byte	N01
	.byte	W02
	.byte		        Cn5 , v012
	.byte	W03
	.byte	N01
	.byte	W02
	.byte		        Cn5 , v008
	.byte	W03
	.byte	N01
	.byte	W03
	.byte	N01
	.byte	W02
	.byte	N01
	.byte	W02
	.byte	N01
	.byte	W03
	.byte	N01
	.byte	W02
	.byte		        Cn5 , v004
	.byte	W03
	.byte	N01
	.byte	W03
	.byte	N01
	.byte	W02
	.byte	N01
	.byte	W02
	.byte	N01
	.byte	W03
	.byte	N01
	.byte	W02
	.byte	N01
	.byte	W15
	.byte	PEND
mus_qz_630_1_006:
	.byte		VOL   , 62*mus_qz_630_mvl/mxv
	.byte		BEND  , c_v-18
	.byte		N23   , Fn1 , v024
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W21
	.byte		        c_v-12
	.byte		N23   , Fn3 , v048
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W06
	.byte		VOL   , 53*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        42*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        31*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        62*mus_qz_630_mvl/mxv
	.byte		N11   , Ds3
	.byte	W12
	.byte		BEND  , c_v-9
	.byte		N23   , Cn3
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W06
	.byte		VOL   , 53*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        42*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        31*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        62*mus_qz_630_mvl/mxv
	.byte		N11   , Ds3
	.byte	W12
	.byte	PEND
mus_qz_630_1_B1:
	.byte		VOICE , 67
	.byte		N23   , Cn4 , v124
	.byte	W09
	.byte		VOL   , 53*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        42*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        31*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        62*mus_qz_630_mvl/mxv
	.byte		N23   , As3
	.byte	W09
	.byte		VOL   , 53*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        42*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        31*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        62*mus_qz_630_mvl/mxv
	.byte		BEND  , c_v-6
	.byte		N23   , Cs4
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W06
	.byte		VOL   , 53*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        42*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        31*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        62*mus_qz_630_mvl/mxv
	.byte		N11   , Cn4
	.byte	W12
	.byte		        As3
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_630_1_000
	.byte	PATT
	 ptrvalue	mus_qz_630_1_001
	.byte		N11   , Cn4 , v124
	.byte	W12
	.byte		        Cs4
	.byte	W12
	.byte		        Ds4
	.byte	W12
	.byte		        Cs4
	.byte	W12
	.byte		N03   , Cn4 , v048
	.byte	W03
	.byte	N02
	.byte	W04
	.byte	N02
	.byte	W02
	.byte	N02
	.byte	W03
	.byte	N02
	.byte	W02
	.byte		        Cn4 , v044
	.byte	W02
	.byte		        Cn4 , v040
	.byte	W03
	.byte	N02
	.byte	W02
	.byte		        Cn4 , v036
	.byte	W03
	.byte		        Cn4 , v032
	.byte	W02
	.byte		        Cn4 , v028
	.byte	W02
	.byte		        Cn4 , v024
	.byte	W03
	.byte	N02
	.byte	W02
	.byte		        Cn4 , v020
	.byte	W03
	.byte		        Cn4 , v016
	.byte	W02
	.byte	N02
	.byte	W02
	.byte	N02
	.byte	W03
	.byte		        Cn4 , v012
	.byte	W02
	.byte	N02
	.byte	W03
	.byte	PATT
	 ptrvalue	mus_qz_630_1_002
	.byte	PATT
	 ptrvalue	mus_qz_630_1_003
	.byte	PATT
	 ptrvalue	mus_qz_630_1_004
	.byte	PATT
	 ptrvalue	mus_qz_630_1_005
	.byte	PATT
	 ptrvalue	mus_qz_630_1_006
	.byte	GOTO
	 ptrvalue	mus_qz_630_1_B1
	.byte	FINE

/********************** Track  2 **********************/

mus_qz_630_2:
	.byte		KEYSH , mus_qz_630_key+0
	.byte		VOICE , 34
	.byte		BENDR , 6
	.byte		PAN   , c_v-32
	.byte		VOL   , 1*mus_qz_630_mvl/mxv
	.byte		BEND  , c_v-16
	.byte		TIE   , As4 , v100
	.byte	W02
	.byte		VOL   , 2*mus_qz_630_mvl/mxv
	.byte		PRIO  , 64
	.byte	W03
	.byte		        2*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        2*mus_qz_630_mvl/mxv
	.byte	W01
	.byte		BEND  , c_v+0
	.byte	W05
	.byte		VOL   , 3*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        3*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        4*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        5*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        5*mus_qz_630_mvl/mxv
	.byte	W09
	.byte		        6*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        7*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        7*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        7*mus_qz_630_mvl/mxv
	.byte	W05
	.byte		        9*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        9*mus_qz_630_mvl/mxv
	.byte	W06
	.byte		        11*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        11*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        13*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        13*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        15*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        15*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        17*mus_qz_630_mvl/mxv
	.byte	W10
	.byte		        17*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        20*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        21*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        21*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        22*mus_qz_630_mvl/mxv
	.byte	W10
	.byte		        24*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        25*mus_qz_630_mvl/mxv
	.byte	W06
	.byte		        27*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        28*mus_qz_630_mvl/mxv
	.byte	W05
	.byte		        30*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        31*mus_qz_630_mvl/mxv
	.byte	W06
	.byte		        33*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        34*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        37*mus_qz_630_mvl/mxv
	.byte	W06
	.byte		        37*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        39*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        41*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        41*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        43*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        44*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        45*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        47*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        47*mus_qz_630_mvl/mxv
	.byte	W10
	.byte	W44
	.byte	W03
	.byte	EOT
	.byte	W01
	.byte		BEND  , c_v-16
	.byte		TIE   , Ds4
	.byte	W06
	.byte		BEND  , c_v+0
	.byte	W42
	.byte	W48
	.byte		VOL   , 21*mus_qz_630_mvl/mxv
	.byte		BEND  , c_v-8
	.byte	W02
	.byte		VOL   , 26*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        31*mus_qz_630_mvl/mxv
	.byte	W01
	.byte		BEND  , c_v+0
	.byte	W02
	.byte		VOL   , 35*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        39*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        42*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        47*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        48*mus_qz_630_mvl/mxv
	.byte	W28
	.byte	EOT
	.byte		VOL   , 22*mus_qz_630_mvl/mxv
	.byte		BEND  , c_v-8
	.byte		N44   , As4 , v100 , gtp3
	.byte	W02
	.byte		VOL   , 26*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        30*mus_qz_630_mvl/mxv
	.byte	W01
	.byte		BEND  , c_v+0
	.byte	W02
	.byte		VOL   , 37*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        41*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        45*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        47*mus_qz_630_mvl/mxv
	.byte	W30
	.byte	W01
	.byte		N44   , Cs4 , v056 , gtp3
	.byte	W15
	.byte		VOL   , 45*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        42*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        38*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        37*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        31*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        27*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        23*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        21*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        15*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        11*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        9*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		VOICE , 23
	.byte		VOL   , 52*mus_qz_630_mvl/mxv
	.byte		N02   , En1 , v100
	.byte	W03
	.byte		N20   , Fn1
	.byte	W21
	.byte		N11   , Cn2
	.byte	W12
	.byte		        Fn2
	.byte	W12
	.byte		        Fs2
	.byte	W12
	.byte		N23   , Ds2
	.byte	W24
	.byte		N11   , Fs2
	.byte	W12
mus_qz_630_2_000:
	.byte		N23   , Fn1 , v100
	.byte	W24
	.byte		N11   , Cn2
	.byte	W12
	.byte		N02   , Fs2
	.byte	W03
	.byte		N08   , Fn2
	.byte	W09
	.byte		N11   , Ds2
	.byte	W12
	.byte		N23   , Cn2
	.byte	W24
	.byte		N11   , Ds2
	.byte	W12
	.byte	PEND
mus_qz_630_2_001:
	.byte		VOL   , 62*mus_qz_630_mvl/mxv
	.byte		        53*mus_qz_630_mvl/mxv
	.byte		N11   , Fn2 , v100
	.byte	W12
	.byte		        Fn2 , v040
	.byte	W12
	.byte		        Cn2 , v100
	.byte	W12
	.byte		        Fn2
	.byte	W12
	.byte		        Fs2
	.byte	W12
	.byte		        Ds2
	.byte	W12
	.byte		        Ds2 , v040
	.byte	W12
	.byte		BEND  , c_v-32
	.byte		N23   , Fs2 , v100
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W09
	.byte	PEND
mus_qz_630_2_002:
	.byte	W12
	.byte		N11   , Fn1 , v100
	.byte	W12
	.byte		        Cn2
	.byte	W12
	.byte		        Fn2
	.byte	W12
	.byte		        Fs2
	.byte	W12
	.byte		        Fs2 , v048
	.byte	W12
	.byte		        Ds2 , v100
	.byte	W12
	.byte		        As1
	.byte	W12
	.byte	PEND
mus_qz_630_2_003:
	.byte		N11   , Ds2 , v100
	.byte	W12
	.byte		        Ds2 , v040
	.byte	W12
	.byte		        As1 , v100
	.byte	W12
	.byte		        Ds2
	.byte	W12
	.byte		        Fs2
	.byte	W12
	.byte		        Fs2 , v040
	.byte	W12
	.byte		        Ds2 , v100
	.byte	W12
	.byte		        Fs2
	.byte	W12
	.byte	PEND
mus_qz_630_2_004:
	.byte		N11   , Fn2 , v100
	.byte	W12
	.byte		        Fn2 , v040
	.byte	W12
	.byte		        Cn2 , v100
	.byte	W12
	.byte		        Fn2
	.byte	W12
	.byte		        Fs2
	.byte	W12
	.byte		        Ds2
	.byte	W12
	.byte		        Ds2 , v040
	.byte	W12
	.byte		        Fs2 , v100
	.byte	W12
	.byte	PEND
mus_qz_630_2_005:
	.byte		N44   , Fn2 , v100 , gtp3
	.byte	W48
	.byte		N23   , Fs2
	.byte	W24
	.byte		        As2
	.byte	W24
	.byte	PEND
mus_qz_630_2_006:
	.byte		N44   , Ds2 , v100 , gtp3
	.byte	W48
	.byte		N23   , Fn2
	.byte	W24
	.byte		        Fs2
	.byte	W24
	.byte	PEND
mus_qz_630_2_007:
	.byte		N92   , Fn2 , v100 , gtp3
	.byte	W03
	.byte		VOL   , 51*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        39*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        18*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        9*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        6*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        6*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        9*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        11*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        14*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        16*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        18*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        25*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        28*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        31*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        33*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        37*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        41*mus_qz_630_mvl/mxv
	.byte	W06
	.byte		        54*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        57*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        62*mus_qz_630_mvl/mxv
	.byte	W28
	.byte	PEND
	.byte		        62*mus_qz_630_mvl/mxv
	.byte		        47*mus_qz_630_mvl/mxv
	.byte		N02   , En1
	.byte	W03
	.byte		N20   , Fn1
	.byte	W21
	.byte		N11   , Cn2
	.byte	W12
	.byte		        Fn2
	.byte	W12
	.byte		        Fs2
	.byte	W12
	.byte		N23   , Ds2
	.byte	W24
	.byte		N11   , Fs2
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_630_2_000
mus_qz_630_2_B1:
	.byte	PATT
	 ptrvalue	mus_qz_630_2_001
	.byte	PATT
	 ptrvalue	mus_qz_630_2_002
	.byte	PATT
	 ptrvalue	mus_qz_630_2_003
	.byte	PATT
	 ptrvalue	mus_qz_630_2_004
	.byte	PATT
	 ptrvalue	mus_qz_630_2_005
	.byte	PATT
	 ptrvalue	mus_qz_630_2_006
	.byte	PATT
	 ptrvalue	mus_qz_630_2_007
	.byte		VOL   , 47*mus_qz_630_mvl/mxv
	.byte		N02   , En1 , v100
	.byte	W03
	.byte		N20   , Fn1
	.byte	W21
	.byte		N11   , Cn2
	.byte	W12
	.byte		        Fn2
	.byte	W12
	.byte		        Fs2
	.byte	W12
	.byte		N23   , Ds2
	.byte	W24
	.byte		N11   , Fs2
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_630_2_000
	.byte	GOTO
	 ptrvalue	mus_qz_630_2_B1
	.byte	FINE

/********************** Track  3 **********************/

mus_qz_630_3:
	.byte		KEYSH , mus_qz_630_key+0
	.byte		VOICE , 0
	.byte		PAN   , c_v-22
	.byte		VOL   , 49*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		PRIO  , 64
	.byte	W10
	.byte		        45*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        40*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        36*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        31*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        26*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        23*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        20*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        16*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        14*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        11*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        8*mus_qz_630_mvl/mxv
	.byte	W30
	.byte		        50*mus_qz_630_mvl/mxv
	.byte	W18
	.byte		        50*mus_qz_630_mvl/mxv
	.byte	W06
	.byte	W96
	.byte	W88
	.byte		N01   , Fn1 , v064
	.byte	W03
	.byte		        Fn1 , v060
	.byte	W02
	.byte		VOL   , 50*mus_qz_630_mvl/mxv
	.byte		N01   , Fn1 , v064
	.byte	W03
	.byte		N48   , Fn1 , v100 , gtp3
	.byte	W15
	.byte		VOL   , 45*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        40*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        36*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        31*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        26*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        23*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        20*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        16*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        14*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        11*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        8*mus_qz_630_mvl/mxv
	.byte	W48
	.byte	W03
	.byte	W72
	.byte		        50*mus_qz_630_mvl/mxv
	.byte	W16
	.byte		N01   , Fn1 , v064
	.byte	W02
	.byte		VOL   , 50*mus_qz_630_mvl/mxv
	.byte	W01
	.byte		N01   , Fn1 , v060
	.byte	W02
	.byte		        Fn1 , v064
	.byte	W03
mus_qz_630_3_000:
	.byte		N48   , Fn1 , v100
	.byte	W12
	.byte		VOL   , 45*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        40*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        36*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        31*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        26*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        23*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        20*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        16*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        14*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        11*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        8*mus_qz_630_mvl/mxv
	.byte	W44
	.byte	W02
	.byte		N01   , Fn1 , v064
	.byte	W02
	.byte		VOL   , 50*mus_qz_630_mvl/mxv
	.byte	W01
	.byte		N01   , Fn1 , v060
	.byte	W02
	.byte		        Fn1 , v064
	.byte	W03
	.byte	PEND
mus_qz_630_3_001:
	.byte		N48   , Fn1 , v100
	.byte	W12
	.byte		VOL   , 45*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        40*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        36*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        31*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        26*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        23*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        20*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        16*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        14*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        11*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        8*mus_qz_630_mvl/mxv
	.byte	W54
	.byte	PEND
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
mus_qz_630_3_002:
	.byte	W88
	.byte		N01   , Fn1 , v064
	.byte	W03
	.byte		        Fn1 , v060
	.byte	W02
	.byte		        Fn1 , v064
	.byte	W03
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_630_3_000
	.byte	PATT
	 ptrvalue	mus_qz_630_3_001
mus_qz_630_3_B1:
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_630_3_002
	.byte	PATT
	 ptrvalue	mus_qz_630_3_000
	.byte	PATT
	 ptrvalue	mus_qz_630_3_001
	.byte	GOTO
	 ptrvalue	mus_qz_630_3_B1
	.byte	FINE

/********************** Track  4 **********************/

mus_qz_630_4:
	.byte		KEYSH , mus_qz_630_key+0
	.byte		VOICE , 34
	.byte		BENDR , 6
	.byte		PAN   , c_v+24
	.byte		VOL   , 49*mus_qz_630_mvl/mxv
	.byte		        1*mus_qz_630_mvl/mxv
	.byte		BEND  , c_v-24
	.byte		TIE   , Gs3 , v127
	.byte	W02
	.byte		VOL   , 2*mus_qz_630_mvl/mxv
	.byte		PRIO  , 64
	.byte	W03
	.byte		        2*mus_qz_630_mvl/mxv
	.byte	W01
	.byte		BEND  , c_v-6
	.byte	W02
	.byte		VOL   , 3*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        5*mus_qz_630_mvl/mxv
	.byte		BEND  , c_v-3
	.byte	W02
	.byte		VOL   , 5*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        7*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        9*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        9*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        12*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        14*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        16*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        18*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        20*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        22*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        23*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        26*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        28*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        31*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        33*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        37*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        39*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        44*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        47*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        48*mus_qz_630_mvl/mxv
	.byte	W24
	.byte	W96
	.byte	W44
	.byte	W01
	.byte		        45*mus_qz_630_mvl/mxv
	.byte	W11
	.byte		        44*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        42*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        41*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        39*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        37*mus_qz_630_mvl/mxv
	.byte	W12
	.byte		        37*mus_qz_630_mvl/mxv
	.byte	W06
	.byte		        35*mus_qz_630_mvl/mxv
	.byte	W10
	.byte		        34*mus_qz_630_mvl/mxv
	.byte	W05
	.byte		        33*mus_qz_630_mvl/mxv
	.byte	W19
	.byte		        31*mus_qz_630_mvl/mxv
	.byte	W05
	.byte		        31*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        30*mus_qz_630_mvl/mxv
	.byte	W05
	.byte		        28*mus_qz_630_mvl/mxv
	.byte	W09
	.byte		        28*mus_qz_630_mvl/mxv
	.byte	W06
	.byte		        27*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        26*mus_qz_630_mvl/mxv
	.byte	W05
	.byte		        25*mus_qz_630_mvl/mxv
	.byte	W09
	.byte		        24*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        23*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        22*mus_qz_630_mvl/mxv
	.byte	W06
	.byte		        21*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        20*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        19*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        17*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        15*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        15*mus_qz_630_mvl/mxv
	.byte	W09
	.byte		        15*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        13*mus_qz_630_mvl/mxv
	.byte	W05
	.byte		        13*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        11*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        11*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        11*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        10*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        9*mus_qz_630_mvl/mxv
	.byte	W06
	.byte		        9*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        9*mus_qz_630_mvl/mxv
	.byte	W12
	.byte		PAN   , c_v+28
	.byte	W02
	.byte		VOL   , 7*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        7*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        6*mus_qz_630_mvl/mxv
	.byte	W02
	.byte	EOT
	.byte		VOL   , 5*mus_qz_630_mvl/mxv
	.byte	W01
	.byte		VOICE , 67
	.byte		VOL   , 61*mus_qz_630_mvl/mxv
	.byte	W01
	.byte		N01   , Fn2 , v088
	.byte	W02
	.byte		VOL   , 62*mus_qz_630_mvl/mxv
	.byte	W01
	.byte	N01
	.byte	W02
	.byte		        Fn2 , v064
	.byte	W03
	.byte		VOL   , 59*mus_qz_630_mvl/mxv
	.byte		BEND  , c_v-12
	.byte		N23   , Fn2 , v100
	.byte	W06
	.byte		BEND  , c_v+0
	.byte	W18
	.byte		N24   , Fn2 , v032 , gtp2
	.byte	W64
	.byte		N01   , Fn2 , v064
	.byte	W03
	.byte		        Fn2 , v060
	.byte	W02
	.byte		        Fn2 , v064
	.byte	W03
mus_qz_630_4_000:
	.byte		VOL   , 59*mus_qz_630_mvl/mxv
	.byte		BEND  , c_v-12
	.byte		N23   , Fn2 , v100
	.byte	W06
	.byte		BEND  , c_v+0
	.byte	W18
	.byte		N24   , Fn2 , v036 , gtp2
	.byte	W72
	.byte	PEND
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
mus_qz_630_4_001:
	.byte	W90
	.byte		N01   , Fn2 , v100
	.byte	W02
	.byte	N01
	.byte	W02
	.byte	N01
	.byte	W02
	.byte	PEND
mus_qz_630_4_002:
	.byte		VOL   , 59*mus_qz_630_mvl/mxv
	.byte		PAN   , c_v+28
	.byte		BEND  , c_v-12
	.byte		N23   , Fn2 , v108
	.byte	W06
	.byte		BEND  , c_v+0
	.byte	W18
	.byte		N24   , Fn2 , v032 , gtp2
	.byte	W64
	.byte		N01   , Fn2 , v080
	.byte	W03
	.byte		        Fn2 , v060
	.byte	W02
	.byte		        Fn2 , v064
	.byte	W03
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_630_4_000
mus_qz_630_4_B1:
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_630_4_001
	.byte	PATT
	 ptrvalue	mus_qz_630_4_002
	.byte	PATT
	 ptrvalue	mus_qz_630_4_000
	.byte	GOTO
	 ptrvalue	mus_qz_630_4_B1
	.byte	FINE

/********************** Track  5 **********************/

mus_qz_630_5:
	.byte		KEYSH , mus_qz_630_key+0
	.byte		VOICE , 67
	.byte		BENDR , 6
	.byte		VOL   , 48*mus_qz_630_mvl/mxv
	.byte		PAN   , c_v+32
	.byte	W02
	.byte		PRIO  , 64
	.byte	W10
	.byte		VOL   , 39*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        36*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        31*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        29*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        26*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        23*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        19*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        16*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        14*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        12*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        9*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        6*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        4*mus_qz_630_mvl/mxv
	.byte	W24
	.byte		        49*mus_qz_630_mvl/mxv
	.byte	W18
	.byte		        49*mus_qz_630_mvl/mxv
	.byte	W06
	.byte	W96
	.byte	W84
	.byte	W03
	.byte		        62*mus_qz_630_mvl/mxv
	.byte	W01
	.byte		N01   , Fn2 , v064
	.byte	W03
	.byte		        Fn2 , v060
	.byte	W02
	.byte		VOL   , 62*mus_qz_630_mvl/mxv
	.byte		N01   , Fn2 , v064
	.byte	W03
	.byte		N48   , Fn2 , v100 , gtp3
	.byte	W15
	.byte		VOL   , 39*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        36*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        31*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        29*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        26*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        23*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        19*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        16*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        14*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        12*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        9*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        6*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        4*mus_qz_630_mvl/mxv
	.byte	W44
	.byte	W01
	.byte	W68
	.byte	W01
	.byte		PAN   , c_v-32
	.byte	W03
	.byte		VOL   , 49*mus_qz_630_mvl/mxv
	.byte	W16
	.byte		N01   , Fn1 , v064
	.byte	W02
	.byte		VOL   , 49*mus_qz_630_mvl/mxv
	.byte	W01
	.byte		N01   , Fn1 , v060
	.byte	W02
	.byte		        Fn1 , v064
	.byte	W03
	.byte		N48   , Fn1 , v100
	.byte	W12
	.byte		VOL   , 39*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        36*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        31*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        29*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        26*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        23*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        19*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        16*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        14*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        12*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        9*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        6*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        4*mus_qz_630_mvl/mxv
	.byte	W40
	.byte		N01   , Fn1 , v064
	.byte	W02
	.byte		VOL   , 49*mus_qz_630_mvl/mxv
	.byte	W01
	.byte		N01   , Fn1 , v060
	.byte	W02
	.byte		        Fn1 , v064
	.byte	W03
	.byte		N96   , Fn1 , v100
	.byte	W12
	.byte		VOL   , 39*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        36*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        31*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        29*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        26*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        23*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        19*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        16*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        14*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        12*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        9*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        6*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        4*mus_qz_630_mvl/mxv
	.byte	W48
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
mus_qz_630_5_000:
	.byte	W72
	.byte		PAN   , c_v-39
	.byte	W18
	.byte		VOL   , 62*mus_qz_630_mvl/mxv
	.byte		N01   , Fn1 , v100
	.byte	W02
	.byte	N01
	.byte	W02
	.byte	N01
	.byte	W02
	.byte	PEND
mus_qz_630_5_001:
	.byte		PAN   , c_v-35
	.byte		VOL   , 62*mus_qz_630_mvl/mxv
	.byte		BEND  , c_v-10
	.byte		N48   , Fn1 , v100
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W84
	.byte	W01
	.byte		N01   , Fn2 , v064
	.byte	W02
	.byte		VOL   , 49*mus_qz_630_mvl/mxv
	.byte	W01
	.byte		N01   , Fn2 , v060
	.byte	W02
	.byte		        Fn2 , v064
	.byte	W03
	.byte	PEND
mus_qz_630_5_002:
	.byte		N48   , Fn2 , v100
	.byte	W12
	.byte		VOL   , 39*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        36*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        31*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        29*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        26*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        23*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        19*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        16*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        14*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        12*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        9*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        6*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        4*mus_qz_630_mvl/mxv
	.byte	W48
	.byte	PEND
mus_qz_630_5_B1:
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_630_5_000
	.byte	PATT
	 ptrvalue	mus_qz_630_5_001
	.byte	PATT
	 ptrvalue	mus_qz_630_5_002
	.byte	GOTO
	 ptrvalue	mus_qz_630_5_B1
	.byte	FINE

/********************** Track  6 **********************/

mus_qz_630_6:
	.byte		KEYSH , mus_qz_630_key+0
	.byte		VOICE , 34
	.byte		BENDR , 6
	.byte		VOL   , 48*mus_qz_630_mvl/mxv
	.byte		PAN   , c_v+40
	.byte		VOL   , 1*mus_qz_630_mvl/mxv
	.byte		BEND  , c_v-21
	.byte		TIE   , Cs4 , v127
	.byte	W02
	.byte		VOL   , 2*mus_qz_630_mvl/mxv
	.byte		PRIO  , 64
	.byte	W03
	.byte		        3*mus_qz_630_mvl/mxv
	.byte	W01
	.byte		BEND  , c_v-6
	.byte	W02
	.byte		VOL   , 4*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        5*mus_qz_630_mvl/mxv
	.byte		BEND  , c_v+1
	.byte		        c_v+0
	.byte	W02
	.byte		VOL   , 7*mus_qz_630_mvl/mxv
	.byte	W06
	.byte		        9*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        9*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        11*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        14*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        15*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        16*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        21*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        23*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        29*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        33*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        37*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        41*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        42*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        47*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        49*mus_qz_630_mvl/mxv
	.byte	W30
	.byte	W01
	.byte	W92
	.byte	W03
	.byte	EOT
	.byte	W01
	.byte		N96   , As3 , v100
	.byte	W48
	.byte		VOL   , 49*mus_qz_630_mvl/mxv
	.byte	W05
	.byte		        48*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        47*mus_qz_630_mvl/mxv
	.byte	W09
	.byte		        45*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        44*mus_qz_630_mvl/mxv
	.byte	W06
	.byte		        43*mus_qz_630_mvl/mxv
	.byte	W06
	.byte		        41*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        40*mus_qz_630_mvl/mxv
	.byte	W05
	.byte		        38*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        37*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        25*mus_qz_630_mvl/mxv
	.byte		BEND  , c_v-12
	.byte		N96   , Gs4
	.byte	W02
	.byte		VOL   , 27*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        30*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        33*mus_qz_630_mvl/mxv
	.byte	W01
	.byte		BEND  , c_v+1
	.byte		        c_v+0
	.byte	W03
	.byte		VOL   , 34*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        37*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        39*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        41*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        43*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        44*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        45*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        47*mus_qz_630_mvl/mxv
	.byte	W06
	.byte		        43*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        41*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        40*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        38*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        37*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        37*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        35*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        34*mus_qz_630_mvl/mxv
	.byte	W06
	.byte		        33*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        31*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        31*mus_qz_630_mvl/mxv
	.byte	W06
	.byte		        30*mus_qz_630_mvl/mxv
	.byte	W06
	.byte		        28*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        28*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		N96   , Ds4
	.byte	W02
	.byte		VOL   , 26*mus_qz_630_mvl/mxv
	.byte	W06
	.byte		        25*mus_qz_630_mvl/mxv
	.byte	W09
	.byte		        24*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        23*mus_qz_630_mvl/mxv
	.byte	W06
	.byte		        22*mus_qz_630_mvl/mxv
	.byte	W06
	.byte		        21*mus_qz_630_mvl/mxv
	.byte	W28
	.byte		        21*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        20*mus_qz_630_mvl/mxv
	.byte	W32
	.byte	W02
	.byte	W05
	.byte		        21*mus_qz_630_mvl/mxv
	.byte	W60
	.byte		        6*mus_qz_630_mvl/mxv
	.byte	W30
	.byte	W01
	.byte	W96
	.byte		VOICE , 35
	.byte		VOL   , 49*mus_qz_630_mvl/mxv
	.byte		BEND  , c_v+1
	.byte	W24
	.byte		N11   , As2
	.byte	W12
	.byte		        Ds3
	.byte	W12
	.byte		N23   , Fs3
	.byte	W24
	.byte		N02   , As3
	.byte	W03
	.byte		        Cn4
	.byte	W03
	.byte		N05   , As3
	.byte	W06
	.byte		N11   , Ds3
	.byte	W12
mus_qz_630_6_000:
	.byte		N23   , Fs3 , v100
	.byte	W24
	.byte		        Ds3
	.byte	W24
	.byte		N02   , Cn3
	.byte	W03
	.byte		        Cs3
	.byte	W03
	.byte		N17   , Cn3
	.byte	W18
	.byte		N02   , As2
	.byte	W03
	.byte		        Cn3
	.byte	W03
	.byte		N17   , As2
	.byte	W18
	.byte	PEND
mus_qz_630_6_001:
	.byte		N23   , Ds3 , v100
	.byte	W24
	.byte		N11   , Cn3
	.byte	W12
	.byte		        Ds3
	.byte	W12
	.byte		        Fs3
	.byte	W12
	.byte		N02   , As3
	.byte	W03
	.byte		        Cn4
	.byte	W03
	.byte		N05   , As3
	.byte	W06
	.byte		N11   , Ds3
	.byte	W12
	.byte		        Fs3
	.byte	W12
	.byte	PEND
mus_qz_630_6_002:
	.byte		N11   , Cn4 , v100
	.byte	W12
	.byte		        Fs3
	.byte	W12
	.byte		        Fn3
	.byte	W12
	.byte		        Ds3
	.byte	W12
	.byte		N02   , Fn3 , v116
	.byte	W03
	.byte		        Fs3 , v088
	.byte	W03
	.byte		        Fn3 , v068
	.byte	W03
	.byte		        Fs3 , v056
	.byte	W03
	.byte		        Fn3 , v048
	.byte	W03
	.byte		        Fs3 , v044
	.byte	W03
	.byte		        Fn3 , v032
	.byte	W03
	.byte		        Fs3
	.byte	W03
	.byte		        Fn3 , v056
	.byte	W03
	.byte		        Fs3 , v068
	.byte	W03
	.byte		        Fn3 , v076
	.byte	W03
	.byte		        Fs3 , v084
	.byte	W03
	.byte		        Fn3 , v092
	.byte	W03
	.byte		        Fs3 , v108
	.byte	W03
	.byte		        Fn3 , v116
	.byte	W03
	.byte		        Fs3 , v127
	.byte	W03
	.byte	PEND
mus_qz_630_6_003:
	.byte		N02   , Cn4 , v100
	.byte	W03
	.byte		        As3
	.byte	W03
	.byte		N17   , Cn4
	.byte	W18
	.byte		N02   , As3
	.byte	W03
	.byte		N20   , Cn4
	.byte	W21
	.byte		N23   , Fn4
	.byte	W24
	.byte		        Fs4
	.byte	W24
	.byte	PEND
mus_qz_630_6_004:
	.byte		N02   , Fn5 , v100
	.byte	W03
	.byte		        Fs5
	.byte	W03
	.byte		N40   , Fn5 , v100 , gtp1
	.byte	W42
	.byte		N02   , As4
	.byte	W03
	.byte		        Cn5
	.byte	W03
	.byte		N17   , As4
	.byte	W18
	.byte		N23   , Cn5
	.byte	W24
	.byte	PEND
mus_qz_630_6_005:
	.byte		VOL   , 5*mus_qz_630_mvl/mxv
	.byte		N23   , Fn4 , v100
	.byte	W02
	.byte		VOL   , 10*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        22*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        35*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        47*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        54*mus_qz_630_mvl/mxv
	.byte	W10
	.byte		N02   , Ds4
	.byte	W03
	.byte		N20   , Fn4
	.byte	W21
	.byte		N23   , Fs4
	.byte	W24
	.byte		N02   , As4
	.byte	W03
	.byte		        Cn5
	.byte	W03
	.byte		N05   , As4
	.byte	W06
	.byte		        Fs4
	.byte	W06
	.byte		        As4
	.byte	W06
	.byte	PEND
mus_qz_630_6_006:
	.byte		N44   , Cn5 , v100 , gtp3
	.byte	W48
	.byte		N02   , Ds5
	.byte	W03
	.byte		        Fn5
	.byte	W03
	.byte		N05   , Ds5
	.byte	W06
	.byte		        Cn5
	.byte	W06
	.byte		        Fn4
	.byte	W06
	.byte		        As4
	.byte	W06
	.byte		        Fs4
	.byte	W06
	.byte		        Ds4
	.byte	W06
	.byte		        As4
	.byte	W06
	.byte	PEND
mus_qz_630_6_007:
	.byte		N02   , Cn4 , v100
	.byte	W03
	.byte		        Cs4
	.byte	W03
	.byte		N28   , Cn4 , v100 , gtp1
	.byte	W30
	.byte		N02   , Cs4
	.byte	W03
	.byte		N56   , Cn4
	.byte	W09
	.byte		VOL   , 49*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        44*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        39*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        33*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        31*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        28*mus_qz_630_mvl/mxv
	.byte	W06
	.byte		        24*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        21*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        18*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        14*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        11*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        8*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        7*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        5*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        3*mus_qz_630_mvl/mxv
	.byte	W04
	.byte	PEND
mus_qz_630_6_B1:
	.byte		VOL   , 49*mus_qz_630_mvl/mxv
	.byte	W24
	.byte		N11   , As2 , v100
	.byte	W12
	.byte		        Ds3
	.byte	W12
	.byte		N23   , Fs3
	.byte	W24
	.byte		N02   , As3
	.byte	W03
	.byte		        Cn4
	.byte	W03
	.byte		N05   , As3
	.byte	W06
	.byte		N11   , Ds3
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_630_6_000
	.byte	PATT
	 ptrvalue	mus_qz_630_6_001
	.byte	PATT
	 ptrvalue	mus_qz_630_6_002
	.byte	PATT
	 ptrvalue	mus_qz_630_6_003
	.byte	PATT
	 ptrvalue	mus_qz_630_6_004
	.byte	PATT
	 ptrvalue	mus_qz_630_6_005
	.byte	PATT
	 ptrvalue	mus_qz_630_6_006
	.byte	PATT
	 ptrvalue	mus_qz_630_6_007
	.byte	GOTO
	 ptrvalue	mus_qz_630_6_B1
	.byte	FINE

/********************** Track  7 **********************/

mus_qz_630_7:
	.byte		KEYSH , mus_qz_630_key+0
	.byte		VOICE , 34
	.byte		BENDR , 6
	.byte		PAN   , c_v-8
	.byte		VOL   , 3*mus_qz_630_mvl/mxv
	.byte		BEND  , c_v-24
	.byte		TIE   , Cs5 , v127
	.byte	W02
	.byte		VOL   , 5*mus_qz_630_mvl/mxv
	.byte		PRIO  , 60
	.byte	W04
	.byte		BEND  , c_v-6
	.byte	W02
	.byte		VOL   , 6*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        7*mus_qz_630_mvl/mxv
	.byte		BEND  , c_v-3
	.byte	W02
	.byte		VOL   , 9*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        9*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        10*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        11*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        13*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        15*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        15*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        17*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        20*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        22*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        23*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        25*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        28*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        30*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        31*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        33*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        35*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        37*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        40*mus_qz_630_mvl/mxv
	.byte	W06
	.byte		        42*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        45*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        48*mus_qz_630_mvl/mxv
	.byte	W16
	.byte	W96
	.byte	W44
	.byte	W01
	.byte		        44*mus_qz_630_mvl/mxv
	.byte	W11
	.byte		        44*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        43*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        42*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        40*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        39*mus_qz_630_mvl/mxv
	.byte	W05
	.byte		        38*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        37*mus_qz_630_mvl/mxv
	.byte	W06
	.byte		        37*mus_qz_630_mvl/mxv
	.byte	W06
	.byte		        35*mus_qz_630_mvl/mxv
	.byte	W04
	.byte	W02
	.byte		        33*mus_qz_630_mvl/mxv
	.byte	W06
	.byte		        31*mus_qz_630_mvl/mxv
	.byte	W06
	.byte		        31*mus_qz_630_mvl/mxv
	.byte	W22
	.byte		        28*mus_qz_630_mvl/mxv
	.byte	W05
	.byte		        28*mus_qz_630_mvl/mxv
	.byte	W09
	.byte		        27*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        25*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        23*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        22*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        21*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        21*mus_qz_630_mvl/mxv
	.byte	W09
	.byte		        20*mus_qz_630_mvl/mxv
	.byte	W15
	.byte		        19*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        17*mus_qz_630_mvl/mxv
	.byte	W08
	.byte		        15*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        15*mus_qz_630_mvl/mxv
	.byte	W05
	.byte		        13*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        13*mus_qz_630_mvl/mxv
	.byte	W08
	.byte		        12*mus_qz_630_mvl/mxv
	.byte	W06
	.byte		        11*mus_qz_630_mvl/mxv
	.byte	W06
	.byte		        11*mus_qz_630_mvl/mxv
	.byte	W06
	.byte		        10*mus_qz_630_mvl/mxv
	.byte	W06
	.byte		        9*mus_qz_630_mvl/mxv
	.byte	W06
	.byte		        9*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        9*mus_qz_630_mvl/mxv
	.byte	W09
	.byte		        7*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        7*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        7*mus_qz_630_mvl/mxv
	.byte	W06
	.byte		        6*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        5*mus_qz_630_mvl/mxv
	.byte	W01
	.byte		N01   , Fn1 , v092
	.byte	W05
	.byte		EOT   , Cs5
	.byte	W01
	.byte		VOICE , 0
	.byte		VOL   , 62*mus_qz_630_mvl/mxv
	.byte		PAN   , c_v+32
	.byte		BEND  , c_v-18
	.byte		N23   , Fn1
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W21
	.byte		        c_v-12
	.byte		N23   , Fn3
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W06
	.byte		VOL   , 53*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        42*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        31*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        62*mus_qz_630_mvl/mxv
	.byte		N11   , Fs3
	.byte	W12
	.byte		BEND  , c_v-9
	.byte		N23   , Ds3
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W06
	.byte		VOL   , 53*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        42*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        31*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        62*mus_qz_630_mvl/mxv
	.byte		N11   , Fs3
	.byte	W12
	.byte		VOL   , 62*mus_qz_630_mvl/mxv
	.byte		BEND  , c_v-18
	.byte		N23   , Fn1
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W21
	.byte		        c_v-12
	.byte		N23   , Fn3
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W06
	.byte		VOL   , 53*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        42*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        31*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        62*mus_qz_630_mvl/mxv
	.byte		N11   , Ds3
	.byte	W12
	.byte		BEND  , c_v-9
	.byte		N23   , Cn3
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W06
	.byte		VOL   , 53*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        42*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        31*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        62*mus_qz_630_mvl/mxv
	.byte		N11   , Ds3
	.byte	W12
mus_qz_630_7_000:
	.byte		VOL   , 62*mus_qz_630_mvl/mxv
	.byte		PAN   , c_v+32
	.byte		N23   , Cn4 , v092
	.byte	W09
	.byte		VOL   , 53*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        42*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        31*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        62*mus_qz_630_mvl/mxv
	.byte		N23   , As3
	.byte	W09
	.byte		VOL   , 53*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        42*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        31*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        62*mus_qz_630_mvl/mxv
	.byte		BEND  , c_v-6
	.byte		N23   , Cs4
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W06
	.byte		VOL   , 53*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        42*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        31*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        62*mus_qz_630_mvl/mxv
	.byte		N11   , Cn4
	.byte	W12
	.byte		        As3
	.byte	W12
	.byte	PEND
mus_qz_630_7_001:
	.byte		N11   , Cn4 , v092
	.byte	W12
	.byte		        As3
	.byte	W12
	.byte		N23   , Fs3
	.byte	W06
	.byte		BEND  , c_v+8
	.byte	W03
	.byte		VOL   , 53*mus_qz_630_mvl/mxv
	.byte		BEND  , c_v+0
	.byte	W03
	.byte		VOL   , 42*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        31*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        62*mus_qz_630_mvl/mxv
	.byte		N44   , Fn3 , v092 , gtp3
	.byte	W09
	.byte		VOL   , 53*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        42*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        31*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        20*mus_qz_630_mvl/mxv
	.byte	W24
	.byte	PEND
mus_qz_630_7_002:
	.byte		VOL   , 62*mus_qz_630_mvl/mxv
	.byte		N23   , Fs3 , v092
	.byte	W09
	.byte		VOL   , 53*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        42*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        31*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        62*mus_qz_630_mvl/mxv
	.byte		N23   , As3
	.byte	W09
	.byte		VOL   , 53*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        42*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        31*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        62*mus_qz_630_mvl/mxv
	.byte		BEND  , c_v+2
	.byte		N23   , Cn4
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W09
	.byte		        c_v+11
	.byte	W12
	.byte		        c_v+0
	.byte	N23
	.byte	W12
	.byte		BEND  , c_v-21
	.byte	W12
	.byte	PEND
mus_qz_630_7_003:
	.byte		BEND  , c_v+0
	.byte		N11   , Cn4 , v092
	.byte	W12
	.byte		        Cs4
	.byte	W12
	.byte		        Ds4
	.byte	W12
	.byte		        Cs4
	.byte	W12
	.byte		N03   , Cn4
	.byte	W03
	.byte		N02   , Cn4 , v060
	.byte	W04
	.byte		        Cn4 , v056
	.byte	W02
	.byte		        Cn4 , v048
	.byte	W03
	.byte	N02
	.byte	W02
	.byte		        Cn4 , v044
	.byte	W02
	.byte		        Cn4 , v040
	.byte	W03
	.byte	N02
	.byte	W02
	.byte		        Cn4 , v036
	.byte	W03
	.byte		        Cn4 , v032
	.byte	W02
	.byte		        Cn4 , v028
	.byte	W02
	.byte		        Cn4 , v024
	.byte	W03
	.byte	N02
	.byte	W02
	.byte		        Cn4 , v020
	.byte	W03
	.byte		        Cn4 , v016
	.byte	W02
	.byte	N02
	.byte	W02
	.byte	N02
	.byte	W03
	.byte		        Cn4 , v012
	.byte	W02
	.byte	N02
	.byte	W03
	.byte	PEND
mus_qz_630_7_004:
	.byte		VOL   , 41*mus_qz_630_mvl/mxv
	.byte		N05   , Cs4 , v127
	.byte	W06
	.byte		        Cs5 , v060
	.byte	W06
	.byte		        Cn4 , v127
	.byte	W06
	.byte		        Cn5 , v060
	.byte	W06
	.byte		        As3 , v127
	.byte	W06
	.byte		        As4 , v060
	.byte	W06
	.byte		        Fs3 , v127
	.byte	W06
	.byte		        Fs4 , v064
	.byte	W06
	.byte		        As3 , v127
	.byte	W06
	.byte		        As4 , v056
	.byte	W06
	.byte		        As3 , v044
	.byte	W06
	.byte		        As4 , v016
	.byte	W06
	.byte		        As3 , v024
	.byte	W06
	.byte		        As4 , v008
	.byte	W06
	.byte		        As3 , v016
	.byte	W06
	.byte		        As4 , v004
	.byte	W06
	.byte	PEND
mus_qz_630_7_005:
	.byte		N05   , Cn4 , v127
	.byte	W06
	.byte		        Cn5 , v060
	.byte	W06
	.byte		        As3 , v127
	.byte	W06
	.byte		        As4 , v060
	.byte	W06
	.byte		        Fs3 , v127
	.byte	W06
	.byte		        Fs4 , v060
	.byte	W06
	.byte		        Ds3 , v127
	.byte	W06
	.byte		        Ds4 , v064
	.byte	W06
	.byte		        Fn3 , v127
	.byte	W06
	.byte		        Fn4 , v056
	.byte	W06
	.byte		        Fn3 , v044
	.byte	W06
	.byte		        Fn4 , v016
	.byte	W06
	.byte		        Fn3 , v024
	.byte	W06
	.byte		        Fn4 , v008
	.byte	W06
	.byte		        Fn3 , v016
	.byte	W06
	.byte		        Fn4 , v004
	.byte	W06
	.byte	PEND
mus_qz_630_7_006:
	.byte		N05   , Fn3 , v127
	.byte	W06
	.byte		        Fn4 , v060
	.byte	W06
	.byte		        Fn3 , v040
	.byte	W06
	.byte		        Fn4 , v016
	.byte	W06
	.byte		        Fn3 , v127
	.byte	W06
	.byte		        Fn4 , v060
	.byte	W06
	.byte		        Fn3 , v040
	.byte	W06
	.byte		        Fn4 , v016
	.byte	W06
	.byte		        Fs3 , v127
	.byte	W06
	.byte		        Fs4 , v060
	.byte	W06
	.byte		        Fs3 , v040
	.byte	W06
	.byte		        Fs4 , v016
	.byte	W06
	.byte		        As3 , v127
	.byte	W06
	.byte		        As4 , v060
	.byte	W06
	.byte		        Fs3 , v127
	.byte	W06
	.byte		        As3
	.byte	W06
	.byte	PEND
mus_qz_630_7_007:
	.byte		VOL   , 40*mus_qz_630_mvl/mxv
	.byte		BEND  , c_v-18
	.byte		N05   , Cn4 , v127
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W03
	.byte		N03   , Cn5 , v100
	.byte	W03
	.byte		N02   , Cn5 , v060
	.byte	W04
	.byte		        Cn5 , v056
	.byte	W02
	.byte		        Cn5 , v048
	.byte	W03
	.byte	N02
	.byte	W02
	.byte		        Cn5 , v044
	.byte	W02
	.byte		        Cn5 , v040
	.byte	W02
	.byte		VOL   , 59*mus_qz_630_mvl/mxv
	.byte		BEND  , c_v-12
	.byte		N23   , Fn3 , v084
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W06
	.byte		VOL   , 53*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        42*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        31*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        62*mus_qz_630_mvl/mxv
	.byte		N11   , Fs3
	.byte	W12
	.byte		BEND  , c_v-9
	.byte		N23   , Ds3
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W06
	.byte		VOL   , 53*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        42*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        31*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        62*mus_qz_630_mvl/mxv
	.byte		N11   , Fs3
	.byte	W12
	.byte	PEND
mus_qz_630_7_008:
	.byte		VOL   , 62*mus_qz_630_mvl/mxv
	.byte		BEND  , c_v-18
	.byte		N23   , Fn1 , v084
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W21
	.byte		        c_v-12
	.byte		N23   , Fn3
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W06
	.byte		VOL   , 53*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        42*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        31*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        62*mus_qz_630_mvl/mxv
	.byte		N11   , Ds3
	.byte	W12
	.byte		BEND  , c_v-9
	.byte		N23   , Cn3
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W06
	.byte		VOL   , 53*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        42*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        31*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        62*mus_qz_630_mvl/mxv
	.byte		N11   , Ds3
	.byte	W12
	.byte	PEND
mus_qz_630_7_B1:
	.byte	PATT
	 ptrvalue	mus_qz_630_7_000
	.byte	PATT
	 ptrvalue	mus_qz_630_7_001
	.byte	PATT
	 ptrvalue	mus_qz_630_7_002
	.byte	PATT
	 ptrvalue	mus_qz_630_7_003
	.byte	PATT
	 ptrvalue	mus_qz_630_7_004
	.byte	PATT
	 ptrvalue	mus_qz_630_7_005
	.byte	PATT
	 ptrvalue	mus_qz_630_7_006
	.byte	PATT
	 ptrvalue	mus_qz_630_7_007
	.byte	PATT
	 ptrvalue	mus_qz_630_7_008
	.byte	GOTO
	 ptrvalue	mus_qz_630_7_B1
	.byte	FINE

/********************** Track  8 **********************/

mus_qz_630_8:
	.byte		KEYSH , mus_qz_630_key+0
	.byte		VOICE , 39
	.byte		PAN   , c_v+0
	.byte		VOL   , 62*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		PRIO  , 64
	.byte	W22
	.byte		PAN   , c_v+60
	.byte	W12
	.byte		        c_v+0
	.byte	W36
	.byte		        c_v+62
	.byte	W24
	.byte		        c_v+0
	.byte		VOL   , 48*mus_qz_630_mvl/mxv
	.byte		N11   , Cs3 , v127
	.byte	W12
	.byte		        Fn3 , v080
	.byte	W12
	.byte		        Fn2 , v076
	.byte	W12
	.byte		        Fn4
	.byte	W36
	.byte		        An1 , v100
	.byte	W12
	.byte		N05   , Fn1 , v088
	.byte		N11   , Cn4 , v124
	.byte	W12
mus_qz_630_8_000:
	.byte		PAN   , c_v+0
	.byte		N11   , Cs3 , v127
	.byte	W12
	.byte		        Fn3 , v080
	.byte	W12
	.byte		        Fn2 , v076
	.byte	W12
	.byte		        Fn4
	.byte	W36
	.byte		        An1 , v100
	.byte	W12
	.byte		        Cn4 , v124
	.byte	W12
	.byte	PEND
mus_qz_630_8_001:
	.byte		PAN   , c_v+0
	.byte		VOL   , 48*mus_qz_630_mvl/mxv
	.byte		N11   , Cs3 , v127
	.byte	W12
	.byte		        Fn3 , v080
	.byte	W12
	.byte		        Fn2 , v076
	.byte	W12
	.byte		        Fn4
	.byte	W36
	.byte		        An1 , v100
	.byte	W12
	.byte		        Cn4 , v124
	.byte	W12
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_630_8_000
	.byte	PATT
	 ptrvalue	mus_qz_630_8_001
	.byte	PATT
	 ptrvalue	mus_qz_630_8_000
	.byte	PATT
	 ptrvalue	mus_qz_630_8_001
	.byte	PATT
	 ptrvalue	mus_qz_630_8_000
	.byte	PATT
	 ptrvalue	mus_qz_630_8_001
	.byte	PATT
	 ptrvalue	mus_qz_630_8_000
	.byte	PATT
	 ptrvalue	mus_qz_630_8_001
	.byte	PATT
	 ptrvalue	mus_qz_630_8_000
	.byte	PATT
	 ptrvalue	mus_qz_630_8_001
mus_qz_630_8_002:
	.byte		PAN   , c_v+0
	.byte		        c_v+0
	.byte		VOL   , 48*mus_qz_630_mvl/mxv
	.byte		N11   , Cs3 , v127
	.byte	W12
	.byte		        Fn3 , v080
	.byte	W12
	.byte		        Fn2 , v076
	.byte	W12
	.byte		        Fn4
	.byte	W36
	.byte		        An1 , v100
	.byte	W12
	.byte		        Cn4 , v124
	.byte	W12
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_630_8_000
mus_qz_630_8_B1:
	.byte	PATT
	 ptrvalue	mus_qz_630_8_001
	.byte	PATT
	 ptrvalue	mus_qz_630_8_000
	.byte	PATT
	 ptrvalue	mus_qz_630_8_001
	.byte	PATT
	 ptrvalue	mus_qz_630_8_000
	.byte	PATT
	 ptrvalue	mus_qz_630_8_001
	.byte	PATT
	 ptrvalue	mus_qz_630_8_000
	.byte	PATT
	 ptrvalue	mus_qz_630_8_001
	.byte	PATT
	 ptrvalue	mus_qz_630_8_002
	.byte	PATT
	 ptrvalue	mus_qz_630_8_000
	.byte	GOTO
	 ptrvalue	mus_qz_630_8_B1
	.byte	FINE

/********************** Track  9 **********************/

mus_qz_630_9:
	.byte		KEYSH , mus_qz_630_key+0
	.byte		VOICE , 33
	.byte		BENDR , 6
	.byte		VOL   , 1*mus_qz_630_mvl/mxv
	.byte		PAN   , c_v+48
	.byte		VOL   , 0*mus_qz_630_mvl/mxv
	.byte		        18*mus_qz_630_mvl/mxv
	.byte		BEND  , c_v-16
	.byte	W01
	.byte	W01
	.byte		PRIO  , 64
	.byte		VOL   , 0*mus_qz_630_mvl/mxv
	.byte		PRIO  , 64
	.byte	W01
	.byte		TIE   , As4 , v088
	.byte	W01
	.byte	W01
	.byte		VOL   , 0*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        1*mus_qz_630_mvl/mxv
	.byte	W01
	.byte		BEND  , c_v+3
	.byte		        c_v+0
	.byte	W05
	.byte		VOL   , 1*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        1*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        1*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        2*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        2*mus_qz_630_mvl/mxv
	.byte	W09
	.byte		        2*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        2*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        3*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        3*mus_qz_630_mvl/mxv
	.byte	W05
	.byte		        3*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        4*mus_qz_630_mvl/mxv
	.byte	W06
	.byte		        4*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        5*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        5*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        5*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        6*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        6*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        7*mus_qz_630_mvl/mxv
	.byte	W10
	.byte		        7*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        8*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        9*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        9*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        9*mus_qz_630_mvl/mxv
	.byte	W10
	.byte		        11*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        11*mus_qz_630_mvl/mxv
	.byte	W06
	.byte		        12*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        12*mus_qz_630_mvl/mxv
	.byte	W05
	.byte		        13*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        14*mus_qz_630_mvl/mxv
	.byte	W06
	.byte		        15*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        15*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        16*mus_qz_630_mvl/mxv
	.byte	W06
	.byte		        16*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        17*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        18*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        18*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        20*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        20*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        20*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        21*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        21*mus_qz_630_mvl/mxv
	.byte	W10
	.byte	W44
	.byte	W03
	.byte	EOT
	.byte	W01
	.byte		BEND  , c_v-16
	.byte		TIE   , Ds4 , v100
	.byte	W06
	.byte		BEND  , c_v+3
	.byte		        c_v+0
	.byte	W42
	.byte	W48
	.byte		VOL   , 9*mus_qz_630_mvl/mxv
	.byte		BEND  , c_v-8
	.byte	W02
	.byte		VOL   , 11*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        14*mus_qz_630_mvl/mxv
	.byte	W01
	.byte		BEND  , c_v+3
	.byte		        c_v+0
	.byte	W02
	.byte		VOL   , 15*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        17*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        18*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        21*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        21*mus_qz_630_mvl/mxv
	.byte	W28
	.byte	EOT
	.byte		VOL   , 9*mus_qz_630_mvl/mxv
	.byte		BEND  , c_v-8
	.byte		N44   , As4 , v100 , gtp3
	.byte	W02
	.byte		VOL   , 11*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        13*mus_qz_630_mvl/mxv
	.byte	W01
	.byte		BEND  , c_v+3
	.byte		        c_v+0
	.byte	W02
	.byte		VOL   , 16*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        18*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        20*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        21*mus_qz_630_mvl/mxv
	.byte	W30
	.byte	W01
	.byte		N44   , Cs4 , v056 , gtp3
	.byte	W15
	.byte		VOL   , 20*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        18*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        17*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        16*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        14*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        12*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        10*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		        9*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        6*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        4*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        3*mus_qz_630_mvl/mxv
	.byte	W04
	.byte		BEND  , c_v+3
	.byte	W96
	.byte	W96
	.byte		VOL   , 1*mus_qz_630_mvl/mxv
	.byte	W06
	.byte		VOICE , 67
	.byte		PAN   , c_v+31
	.byte		        c_v-32
	.byte		N23   , Cn4 , v124
	.byte	W09
	.byte		VOL   , 9*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        7*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        5*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        4*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        11*mus_qz_630_mvl/mxv
	.byte		N23   , As3
	.byte	W09
	.byte		VOL   , 9*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        7*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        5*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        4*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        11*mus_qz_630_mvl/mxv
	.byte		BEND  , c_v-6
	.byte		N23   , Cs4
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W06
	.byte		VOL   , 9*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        7*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        5*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        4*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        11*mus_qz_630_mvl/mxv
	.byte		N11   , Cn4
	.byte	W12
	.byte		        As3
	.byte	W06
	.byte	W06
	.byte		        Cn4
	.byte	W12
	.byte		        As3
	.byte	W12
	.byte		N23   , Fs3
	.byte	W06
	.byte		BEND  , c_v+8
	.byte	W03
	.byte		VOL   , 9*mus_qz_630_mvl/mxv
	.byte		BEND  , c_v+0
	.byte	W03
	.byte		VOL   , 7*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        5*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        4*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        11*mus_qz_630_mvl/mxv
	.byte		N44   , Fn3 , v124 , gtp3
	.byte	W09
	.byte		VOL   , 9*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        7*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        5*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        4*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        3*mus_qz_630_mvl/mxv
	.byte	W18
	.byte	W06
	.byte		        11*mus_qz_630_mvl/mxv
	.byte		N23   , Fs3
	.byte	W09
	.byte		VOL   , 9*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        7*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        5*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        4*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        11*mus_qz_630_mvl/mxv
	.byte		N23   , As3
	.byte	W09
	.byte		VOL   , 9*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        7*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        5*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        4*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        11*mus_qz_630_mvl/mxv
	.byte		BEND  , c_v+2
	.byte		N11   , Cn4
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W09
	.byte		N11   , Cs4
	.byte	W12
	.byte		        Cn4
	.byte	W12
	.byte		        As3
	.byte	W06
mus_qz_630_9_000:
	.byte	W06
	.byte		N11   , Cn4 , v124
	.byte	W12
	.byte		        Cs4
	.byte	W12
	.byte		        Ds4
	.byte	W12
	.byte		        Cs4
	.byte	W12
	.byte		N03   , Cn4 , v092
	.byte	W03
	.byte		N02   , Cn4 , v048
	.byte	W04
	.byte		        Cn4 , v072
	.byte	W02
	.byte		        Cn4 , v056
	.byte	W03
	.byte		        Cn4 , v064
	.byte	W02
	.byte		        Cn4 , v060
	.byte	W02
	.byte		        Cn4 , v040
	.byte	W03
	.byte	N02
	.byte	W02
	.byte		        Cn4 , v036
	.byte	W03
	.byte		        Cn4 , v032
	.byte	W02
	.byte		        Cn4 , v028
	.byte	W02
	.byte		        Cn4 , v024
	.byte	W03
	.byte	N02
	.byte	W02
	.byte		        Cn4 , v020
	.byte	W03
	.byte		        Cn4 , v016
	.byte	W02
	.byte	N02
	.byte	W02
	.byte	N02
	.byte	W02
	.byte	PEND
	.byte	W01
	.byte		        Cn4 , v012
	.byte	W02
	.byte	N02
	.byte	W03
	.byte		VOICE , 42
	.byte		VOL   , 11*mus_qz_630_mvl/mxv
	.byte		BEND  , c_v-3
	.byte		N05   , Cs4 , v127
	.byte	W06
	.byte		        Cs5 , v060
	.byte	W06
	.byte		        Cn4 , v127
	.byte	W06
	.byte		        Cn5 , v060
	.byte	W06
	.byte		        As3 , v127
	.byte	W06
	.byte		        As4 , v060
	.byte	W06
	.byte		        Fs3 , v127
	.byte	W06
	.byte		        Fs4 , v064
	.byte	W06
	.byte		        As3 , v127
	.byte	W06
	.byte		        As4 , v056
	.byte	W06
	.byte		        As3 , v044
	.byte	W06
	.byte		        As4 , v016
	.byte	W06
	.byte		        As3 , v024
	.byte	W06
	.byte		        As4 , v008
	.byte	W06
	.byte		        As3 , v016
	.byte	W06
mus_qz_630_9_001:
	.byte		N05   , As4 , v004
	.byte	W06
	.byte		        Cn4 , v127
	.byte	W06
	.byte		        Cn5 , v060
	.byte	W06
	.byte		        As3 , v127
	.byte	W06
	.byte		        As4 , v060
	.byte	W06
	.byte		        Fs3 , v127
	.byte	W06
	.byte		        Fs4 , v060
	.byte	W06
	.byte		        Ds3 , v127
	.byte	W06
	.byte		        Ds4 , v064
	.byte	W06
	.byte		        Fn3 , v127
	.byte	W06
	.byte		        Fn4 , v056
	.byte	W06
	.byte		        Fn3 , v044
	.byte	W06
	.byte		        Fn4 , v016
	.byte	W06
	.byte		        Fn3 , v024
	.byte	W06
	.byte		        Fn4 , v008
	.byte	W06
	.byte		        Fn3 , v016
	.byte	W06
	.byte	PEND
mus_qz_630_9_002:
	.byte		N05   , Fn4 , v004
	.byte	W06
	.byte		        Fn3 , v127
	.byte	W06
	.byte		        Fn4 , v060
	.byte	W06
	.byte		        Fn3 , v040
	.byte	W06
	.byte		        Fn4 , v016
	.byte	W06
	.byte		        Fn3 , v127
	.byte	W06
	.byte		        Fn4 , v060
	.byte	W06
	.byte		        Fn3 , v040
	.byte	W06
	.byte		        Fn4 , v016
	.byte	W06
	.byte		        Fs3 , v127
	.byte	W06
	.byte		        Fs4 , v060
	.byte	W06
	.byte		        Fs3 , v040
	.byte	W06
	.byte		        Fs4 , v016
	.byte	W06
	.byte		        As3 , v127
	.byte	W06
	.byte		        As4 , v060
	.byte	W06
	.byte		        Fs3 , v127
	.byte	W06
	.byte	PEND
	.byte		        As3
	.byte	W06
	.byte		PAN   , c_v-34
	.byte		VOL   , 3*mus_qz_630_mvl/mxv
	.byte		BEND  , c_v-18
	.byte		N05   , Cn4
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W03
	.byte		N03   , Cn5 , v100
	.byte	W03
	.byte		N01   , Cn5 , v060
	.byte	W04
	.byte		        Cn5 , v056
	.byte	W02
	.byte		        Cn5 , v048
	.byte	W03
	.byte	N01
	.byte	W02
	.byte		        Cn5 , v044
	.byte	W02
	.byte		        Cn5 , v040
	.byte	W03
	.byte	N01
	.byte	W02
	.byte		        Cn5 , v036
	.byte	W03
	.byte		        Cn5 , v032
	.byte	W02
	.byte		        Cn5 , v028
	.byte	W02
	.byte		        Cn5 , v024
	.byte	W03
	.byte	N01
	.byte	W02
	.byte		        Cn5 , v020
	.byte	W03
	.byte		        Cn5 , v016
	.byte	W02
	.byte	N01
	.byte	W02
	.byte		        Cn5 , v012
	.byte	W03
	.byte	N01
	.byte	W02
	.byte		        Cn5 , v008
	.byte	W03
	.byte	N01
	.byte	W03
	.byte	N01
	.byte	W02
	.byte	N01
	.byte	W02
	.byte	N01
	.byte	W03
	.byte	N01
	.byte	W02
	.byte		        Cn5 , v004
	.byte	W03
	.byte	N01
	.byte	W03
	.byte	N01
	.byte	W02
	.byte	N01
	.byte	W02
	.byte	N01
	.byte	W03
	.byte	N01
	.byte	W02
	.byte	N01
	.byte	W09
	.byte	W96
mus_qz_630_9_B1:
	.byte		VOL   , 3*mus_qz_630_mvl/mxv
	.byte	W06
	.byte		VOICE , 67
	.byte		PAN   , c_v+31
	.byte		        c_v-32
	.byte		N23   , Cn4 , v124
	.byte	W09
	.byte		VOL   , 9*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        7*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        5*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        4*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        11*mus_qz_630_mvl/mxv
	.byte		N23   , As3
	.byte	W09
	.byte		VOL   , 9*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        7*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        5*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        4*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        11*mus_qz_630_mvl/mxv
	.byte		BEND  , c_v-6
	.byte		N23   , Cs4
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W06
	.byte		VOL   , 9*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        7*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        5*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        4*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        11*mus_qz_630_mvl/mxv
	.byte		N11   , Cn4
	.byte	W12
	.byte		        As3
	.byte	W06
	.byte	W06
	.byte		        Cn4
	.byte	W12
	.byte		        As3
	.byte	W12
	.byte		N23   , Fs3
	.byte	W06
	.byte		BEND  , c_v+8
	.byte	W03
	.byte		VOL   , 9*mus_qz_630_mvl/mxv
	.byte		BEND  , c_v+0
	.byte	W03
	.byte		VOL   , 7*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        5*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        4*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        11*mus_qz_630_mvl/mxv
	.byte		N44   , Fn3 , v124 , gtp3
	.byte	W09
	.byte		VOL   , 9*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        7*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        5*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        4*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        3*mus_qz_630_mvl/mxv
	.byte	W18
	.byte	W06
	.byte		        11*mus_qz_630_mvl/mxv
	.byte		N23   , Fs3
	.byte	W09
	.byte		VOL   , 9*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        7*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        5*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        4*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        11*mus_qz_630_mvl/mxv
	.byte		N23   , As3
	.byte	W09
	.byte		VOL   , 9*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        7*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		        5*mus_qz_630_mvl/mxv
	.byte	W03
	.byte		        4*mus_qz_630_mvl/mxv
	.byte	W07
	.byte		        11*mus_qz_630_mvl/mxv
	.byte		BEND  , c_v+2
	.byte		N11   , Cn4
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W09
	.byte		N11   , Cs4
	.byte	W12
	.byte		        Cn4
	.byte	W12
	.byte		        As3
	.byte	W06
	.byte	PATT
	 ptrvalue	mus_qz_630_9_000
	.byte	W01
	.byte		N02   , Cn4 , v012
	.byte	W02
	.byte	N02
	.byte	W03
	.byte		VOICE , 42
	.byte		VOL   , 10*mus_qz_630_mvl/mxv
	.byte		BEND  , c_v-3
	.byte		N05   , Cs4 , v127
	.byte	W06
	.byte		        Cs5 , v060
	.byte	W06
	.byte		        Cn4 , v127
	.byte	W06
	.byte		        Cn5 , v060
	.byte	W06
	.byte		        As3 , v127
	.byte	W06
	.byte		        As4 , v060
	.byte	W06
	.byte		        Fs3 , v127
	.byte	W06
	.byte		        Fs4 , v064
	.byte	W06
	.byte		        As3 , v127
	.byte	W06
	.byte		        As4 , v056
	.byte	W06
	.byte		        As3 , v044
	.byte	W06
	.byte		        As4 , v016
	.byte	W06
	.byte		        As3 , v024
	.byte	W06
	.byte		        As4 , v008
	.byte	W06
	.byte		        As3 , v016
	.byte	W06
	.byte	PATT
	 ptrvalue	mus_qz_630_9_001
	.byte	PATT
	 ptrvalue	mus_qz_630_9_002
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_630_9_B1
	.byte	FINE

/********************** Track 10 **********************/

mus_qz_630_10:
	.byte		KEYSH , mus_qz_630_key+0
	.byte		VOICE , 39
	.byte		PAN   , c_v+0
	.byte		VOL   , 62*mus_qz_630_mvl/mxv
	.byte	W02
	.byte		PRIO  , 64
	.byte	W22
	.byte		PAN   , c_v+48
	.byte		BEND  , c_v-63
	.byte	W12
	.byte		PAN   , c_v+0
	.byte	W60
	.byte		        c_v+34
	.byte		VOL   , 58*mus_qz_630_mvl/mxv
	.byte		PAN   , c_v+26
	.byte		N11   , Dn3 , v127
	.byte	W12
	.byte		        Fs3
	.byte	W12
	.byte		        Fn2 , v076
	.byte	W12
	.byte		        Fn4
	.byte	W12
	.byte		        Bn2 , v127
	.byte	W12
	.byte		        Fs2 , v100
	.byte	W12
	.byte		        En2
	.byte	W12
	.byte		        Bn3 , v124
	.byte	W12
	.byte		        Dn3 , v127
	.byte	W12
	.byte		        Fs3 , v080
	.byte	W12
	.byte		        Fn2 , v076
	.byte	W12
	.byte		        Fn4
	.byte	W12
	.byte		        Bn2 , v127
	.byte	W12
	.byte		        Fn2 , v112
	.byte	W12
	.byte		        En2 , v100
	.byte	W12
	.byte		        Bn3 , v124
	.byte	W12
	.byte		        Dn3 , v127
	.byte	W12
	.byte		        Fs3 , v080
	.byte	W12
	.byte		        Fn2 , v076
	.byte	W12
	.byte		        Fn4
	.byte	W12
	.byte		        Bn2 , v127
	.byte	W12
	.byte		        Fs2 , v100
	.byte	W12
	.byte		        En2
	.byte	W12
	.byte		        Bn3 , v124
	.byte	W12
mus_qz_630_10_000:
	.byte		N11   , Dn3 , v127
	.byte	W12
	.byte		        Fs3
	.byte	W12
	.byte		        Fn2 , v076
	.byte	W12
	.byte		        Fn4
	.byte	W12
	.byte		        Bn2 , v127
	.byte	W12
	.byte		        Fn2 , v104
	.byte	W12
	.byte		        En2 , v100
	.byte	W12
	.byte		        Bn3 , v124
	.byte	W12
	.byte	PEND
	.byte		        Dn3 , v127
	.byte	W12
	.byte		        Fs3
	.byte	W12
	.byte		        Fn2 , v076
	.byte	W12
	.byte		        Fn4
	.byte	W12
	.byte		        Bn2 , v127
	.byte	W12
	.byte		        Fs2 , v080
	.byte	W12
	.byte		        En2 , v100
	.byte	W12
	.byte		        Bn3 , v124
	.byte	W12
	.byte		        Dn3 , v127
	.byte	W12
	.byte		        Fs3
	.byte	W12
	.byte		        Fn2 , v112
	.byte	W12
	.byte		        Fn4 , v080
	.byte	W12
	.byte		        Bn2 , v127
	.byte	W12
	.byte		        Fs2 , v100
	.byte	W12
	.byte		        En2
	.byte	W12
	.byte		        Bn3 , v124
	.byte	W12
	.byte		        Dn3 , v127
	.byte	W12
	.byte		        Fs3 , v116
	.byte	W12
	.byte		        Fn2 , v076
	.byte	W12
	.byte		        Fn4
	.byte	W12
	.byte		        Bn2 , v127
	.byte	W12
	.byte		        Fs2 , v104
	.byte	W12
	.byte		        En2 , v100
	.byte	W12
	.byte		        Bn3 , v124
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_630_10_000
	.byte		N11   , Dn3 , v127
	.byte	W12
	.byte		        Fs3 , v120
	.byte	W12
	.byte		        Fn2 , v076
	.byte	W12
	.byte		        Fn4 , v088
	.byte	W12
	.byte		        Bn2 , v127
	.byte	W12
	.byte		        Fs2 , v104
	.byte	W12
	.byte		        En2 , v100
	.byte	W12
	.byte		        Bn3 , v124
	.byte	W12
	.byte		        Dn3 , v127
	.byte	W12
	.byte		        Fs3 , v120
	.byte	W12
	.byte		        Fn2 , v076
	.byte	W12
	.byte		        Fn4 , v084
	.byte	W12
	.byte		        Bn2 , v127
	.byte	W12
	.byte		        Fs2 , v088
	.byte	W12
	.byte		        En2 , v100
	.byte	W12
	.byte		        Bn3 , v124
	.byte	W12
	.byte		        Dn3 , v127
	.byte	W12
	.byte		        Fs3
	.byte	W12
	.byte		        Fn2 , v076
	.byte	W12
	.byte		        Fn4
	.byte	W12
	.byte		        Bn2 , v127
	.byte	W12
	.byte		        Fn2 , v080
	.byte	W12
	.byte		        En2 , v100
	.byte	W12
	.byte		        Bn3 , v124
	.byte	W12
	.byte		        Dn3 , v127
	.byte	W12
	.byte		        Fs3
	.byte	W12
	.byte		        Fn2 , v076
	.byte	W12
	.byte		        Fn4
	.byte	W12
	.byte		        Bn2 , v127
	.byte	W12
	.byte		        Fs2 , v088
	.byte	W12
	.byte		        En2 , v100
	.byte	W12
	.byte		        Bn3 , v124
	.byte	W12
	.byte		PAN   , c_v+0
	.byte		        c_v+16
	.byte		BEND  , c_v-31
	.byte		N11   , An1 , v127
	.byte	W12
	.byte		        En1
	.byte	W12
	.byte		        Gn1
	.byte	W12
	.byte		        As2 , v100
	.byte	W12
	.byte		        Bn1
	.byte		N11   , Bn2 , v127
	.byte	W12
	.byte		        Fn1 , v088
	.byte	W12
	.byte		N23   , En1 , v127
	.byte	W24
	.byte		N11   , Bn0 , v100
	.byte		N11   , Dn3 , v127
	.byte	W12
	.byte		        Bn0 , v100
	.byte		N11   , Fs3 , v080
	.byte	W12
	.byte		        As0 , v100
	.byte		N11   , Fn2 , v076
	.byte	W12
	.byte		        Cn1 , v100
	.byte		N11   , Fn4 , v076
	.byte	W12
	.byte		        Bn2 , v127
	.byte	W12
	.byte		        Bn0 , v100
	.byte		N11   , Bn2 , v040
	.byte	W12
	.byte		N23   , En2 , v100
	.byte	W12
	.byte		N05   , Bn0 , v060
	.byte		N11   , Bn3 , v124
	.byte	W06
	.byte		N05   , Bn0 , v044
	.byte	W06
	.byte		        Bn0 , v100
	.byte		N11   , Dn3 , v127
	.byte	W06
	.byte		N05   , Cn1 , v060
	.byte	W06
	.byte		        Bn0 , v100
	.byte		N11   , Fs3 , v080
	.byte	W06
	.byte		N05   , Cn1 , v068
	.byte	W06
	.byte		N11   , As0 , v100
	.byte		N11   , Fn2 , v076
	.byte	W12
	.byte		        Cn1 , v100
	.byte		N11   , Fn4 , v076
	.byte	W12
	.byte		        Bn2 , v127
	.byte	W12
	.byte		        Bn0 , v100
	.byte		N11   , Bn2 , v040
	.byte	W12
	.byte		N23   , En1 , v100
	.byte		N11   , Fn2
	.byte	W12
	.byte		        Bn3 , v124
	.byte	W12
mus_qz_630_10_B1:
	.byte		PAN   , c_v+0
	.byte		N23   , Bn0 , v100
	.byte		N11   , Dn3 , v127
	.byte	W12
	.byte		        Fs3 , v080
	.byte	W12
	.byte		        Bn0 , v100
	.byte		N11   , Fn2 , v076
	.byte	W12
	.byte		        Fs1 , v100
	.byte		N11   , Fn4 , v076
	.byte	W09
	.byte		N23   , Ds1 , v127
	.byte	W03
	.byte		N11   , Bn2
	.byte	W12
	.byte		        Bn0 , v100
	.byte		N11   , Fs2 , v104
	.byte	W12
	.byte		        En2 , v100
	.byte	W12
	.byte		N05   , Bn0 , v060
	.byte		N11   , Bn3 , v124
	.byte	W06
	.byte		N05   , Bn0 , v044
	.byte	W06
	.byte		N11   , Bn0 , v100
	.byte		N11   , Dn3 , v127
	.byte	W12
	.byte		        Bn0 , v100
	.byte		N11   , Fs3 , v080
	.byte	W12
	.byte		        As0 , v100
	.byte		N11   , Fn2 , v076
	.byte	W12
	.byte		        Cn1 , v100
	.byte		N11   , Fn4 , v076
	.byte	W12
	.byte		        Bn2 , v127
	.byte	W12
	.byte		        Bn0 , v100
	.byte		N11   , Fn2 , v104
	.byte	W12
	.byte		N23   , En1 , v100
	.byte		N11   , En2
	.byte	W12
	.byte		        Bn3 , v124
	.byte	W12
	.byte		N23   , Bn0 , v100
	.byte		N11   , Dn3 , v127
	.byte	W12
	.byte		        Fs3 , v080
	.byte	W12
	.byte		        Bn0 , v100
	.byte		N11   , Fn2 , v076
	.byte	W12
	.byte		        As0 , v100
	.byte		N11   , Fn4 , v088
	.byte	W09
	.byte		N14   , Ds1 , v127
	.byte	W03
	.byte		N11   , Bn2
	.byte	W12
	.byte		        Bn0 , v100
	.byte		N11   , Fs2 , v104
	.byte	W12
	.byte		        As0 , v100
	.byte		N11   , En2
	.byte	W12
	.byte		N05   , Bn0 , v060
	.byte		N11   , Bn3 , v124
	.byte	W06
	.byte		N05   , Bn0 , v044
	.byte	W06
	.byte		        Bn0 , v100
	.byte		N11   , Dn3 , v127
	.byte	W06
	.byte		N05   , Dn1 , v100
	.byte	W06
	.byte		        Bn0
	.byte		N11   , Fs3 , v080
	.byte	W12
	.byte		        Dn1 , v100
	.byte		N11   , Fn2 , v076
	.byte	W12
	.byte		N05   , Bn0 , v100
	.byte		N11   , Fn4 , v084
	.byte	W06
	.byte		N05   , Bn0 , v100
	.byte	W06
	.byte		N11   , Cn1
	.byte		N11   , Bn2 , v127
	.byte	W12
	.byte		        As0 , v100
	.byte		N11   , Fs2 , v088
	.byte	W12
	.byte		        En1 , v100
	.byte		N11   , En2
	.byte	W12
	.byte		N05   , Dn1
	.byte		N11   , Bn3 , v124
	.byte	W06
	.byte		N05   , Bn0 , v080
	.byte	W06
	.byte		N11   , Bn0 , v100
	.byte		N11   , Dn3 , v127
	.byte	W12
	.byte		        Bn0 , v100
	.byte		N11   , Fs3 , v080
	.byte	W12
	.byte		        As0 , v100
	.byte		N11   , Fn2 , v076
	.byte	W12
	.byte		        Bn0 , v100
	.byte		N11   , Fn4 , v076
	.byte	W12
	.byte		N05   , Bn0 , v100
	.byte		N11   , Bn2 , v127
	.byte	W06
	.byte		N05   , As0 , v100
	.byte	W06
	.byte		N11   , Bn0
	.byte		N11   , Fn2 , v080
	.byte	W12
	.byte		N05   , As0 , v100
	.byte		N11   , En2
	.byte	W12
	.byte		N05   , Bn0 , v060
	.byte		N11   , Bn3 , v124
	.byte	W06
	.byte		N05   , Bn0 , v044
	.byte	W06
	.byte		N11   , Bn0 , v100
	.byte		N11   , Dn3 , v127
	.byte	W12
	.byte		        Bn0 , v100
	.byte		N11   , Fs3 , v080
	.byte	W12
	.byte		        As0 , v100
	.byte		N11   , Fn2 , v076
	.byte	W12
	.byte		        Cn1 , v100
	.byte		N11   , Fn4 , v076
	.byte	W12
	.byte		        Bn2 , v127
	.byte	W12
	.byte		        Bn0 , v100
	.byte		N11   , Fs2 , v088
	.byte	W12
	.byte		N23   , En1 , v100
	.byte		N11   , En2
	.byte	W12
	.byte		        Bn3 , v124
	.byte	W12
	.byte		        Bn0 , v100
	.byte		N11   , An1 , v127
	.byte	W12
	.byte		        Bn0 , v100
	.byte		N11   , En1
	.byte	W12
	.byte		        As0
	.byte		N11   , Gn1 , v127
	.byte	W12
	.byte		        Cn1 , v100
	.byte		N11   , As2
	.byte	W12
	.byte		        Bn1
	.byte		N11   , Bn2 , v127
	.byte	W12
	.byte		        Bn0 , v100
	.byte		N11   , Fn1 , v088
	.byte	W12
	.byte		N23   , An2 , v076
	.byte		N23   , As2
	.byte	W18
	.byte		N02   , As0 , v088
	.byte	W03
	.byte		        As0 , v060
	.byte	W03
	.byte		N11   , Bn0 , v100
	.byte		N11   , Dn3 , v127
	.byte	W12
	.byte		        Bn0 , v100
	.byte		N11   , Fs3 , v080
	.byte	W12
	.byte		        As0 , v100
	.byte		N11   , Fn2 , v076
	.byte	W12
	.byte		        Cn1 , v100
	.byte		N11   , Fn4 , v076
	.byte	W12
	.byte		        As0 , v100
	.byte		N11   , Bn2 , v127
	.byte	W12
	.byte		N05   , Bn0 , v100
	.byte		N11   , Bn2 , v040
	.byte	W06
	.byte		N05   , As0 , v100
	.byte	W06
	.byte		N11   , Bn0
	.byte		N11   , En2
	.byte	W12
	.byte		N05   , As0 , v127
	.byte		N11   , Bn3
	.byte	W06
	.byte		N05   , As0
	.byte	W06
	.byte		N11   , Bn0 , v100
	.byte		N11   , Dn3 , v127
	.byte	W12
	.byte		N05   , Bn0 , v100
	.byte		N11   , Fs3 , v080
	.byte	W06
	.byte		N02   , As0 , v068
	.byte	W03
	.byte		        As0 , v072
	.byte	W03
	.byte		N11   , As0 , v116
	.byte		N11   , Fn2
	.byte	W12
	.byte		N05   , Cn1 , v100
	.byte		N11   , Fn4 , v076
	.byte	W06
	.byte		N05   , As0 , v100
	.byte	W06
	.byte		N11   , Bn2 , v127
	.byte	W12
	.byte		        Bn0 , v100
	.byte		N11   , Bn2 , v040
	.byte	W12
	.byte		        As0 , v116
	.byte		N23   , En1 , v100
	.byte		N11   , Fn2
	.byte	W12
	.byte		N05   , As0 , v116
	.byte		N11   , Bn3 , v124
	.byte	W06
	.byte		N05   , As0 , v116
	.byte	W06
	.byte	GOTO
	 ptrvalue	mus_qz_630_10_B1
	.byte	FINE

/******************************************************/
	.p2align	2

mus_qz_630:
	.byte	10	/* NumTrks */
	.byte	0	/* NumBlks */
	.byte	mus_qz_630_pri	/* Priority */
	.byte	mus_qz_630_rev	/* Reverb. */
space64 4
	ptrvalue	mus_qz_630_grp

	ptrvalue	mus_qz_630_1
	ptrvalue	mus_qz_630_2
	ptrvalue	mus_qz_630_3
	ptrvalue	mus_qz_630_4
	ptrvalue	mus_qz_630_5
	ptrvalue	mus_qz_630_6
	ptrvalue	mus_qz_630_7
	ptrvalue	mus_qz_630_8
	ptrvalue	mus_qz_630_9
	ptrvalue	mus_qz_630_10

	.end
