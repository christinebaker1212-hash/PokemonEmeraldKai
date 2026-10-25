	.include "asm/macros/bit_width.inc"

	.include "MPlayDef.s"

/* Moemon Quetzal song 638, disassembled from the ROM by tools/quetzal_rip/songs.py.
   "org" comments give the ROM address of the bytes that follow. */

	.equ	mus_qz_638_grp, voicegroup_qz_songs
	.equ	mus_qz_638_pri, 0
	.equ	mus_qz_638_rev, reverb_set+0
	.equ	mus_qz_638_mvl, 127
	.equ	mus_qz_638_key, 0
	.equ	mus_qz_638_tbs, 1
	.equ	mus_qz_638_exg, 1
	.equ	mus_qz_638_cmp, 1

	.section .rodata
	.global	mus_qz_638
	.p2align	2
/* org 0x0968B37C */

/********************** Track  1 **********************/

mus_qz_638_1:
	.byte		KEYSH , mus_qz_638_key+0
	.byte		TEMPO , 160*mus_qz_638_tbs/2
	.byte		PAN   , c_v+0
	.byte		BENDR , 6
	.byte		LFOS  , 21
	.byte		VOL   , 18*mus_qz_638_mvl/mxv
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W72
	.byte		VOICE , 33
	.byte	W24
	.byte		VOL   , 5*mus_qz_638_mvl/mxv
	.byte		PAN   , c_v-20
	.byte		N92   , Ds4 , v100 , gtp3
	.byte		N92   , Gn4 , v100 , gtp3
	.byte	W17
	.byte		VOL   , 6*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        7*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        8*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        9*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        11*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        13*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        15*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        17*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        19*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        21*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        26*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        29*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        38*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		PAN   , c_v+12
	.byte		VOL   , 5*mus_qz_638_mvl/mxv
	.byte		PAN   , c_v-2
	.byte		N92   , Dn4 , v100 , gtp3
	.byte		N92   , As4 , v100 , gtp3
	.byte	W17
	.byte		VOL   , 6*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        7*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        8*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        9*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        11*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        13*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        15*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        17*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        19*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        21*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        26*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        29*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        38*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		PAN   , c_v-16
	.byte		VOL   , 5*mus_qz_638_mvl/mxv
	.byte		N92   , Cs4 , v100 , gtp3
	.byte		N92   , Fn4 , v100 , gtp3
	.byte	W17
	.byte		VOL   , 6*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        7*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        8*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        9*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        11*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        13*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        15*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        17*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        19*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        21*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        26*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        29*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        38*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		PAN   , c_v+12
	.byte		VOL   , 5*mus_qz_638_mvl/mxv
	.byte		PAN   , c_v-11
	.byte		N92   , Cn4 , v100 , gtp3
	.byte		N92   , Gn4 , v100 , gtp3
	.byte	W17
	.byte		VOL   , 6*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        7*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        8*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        9*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        11*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        13*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        15*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        17*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        19*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        21*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        26*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        29*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        38*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		PAN   , c_v-25
	.byte		VOL   , 5*mus_qz_638_mvl/mxv
	.byte		N15   , Cn4
	.byte		N15   , Gn4
	.byte	W02
	.byte		VOL   , 6*mus_qz_638_mvl/mxv
	.byte	W01
	.byte		        9*mus_qz_638_mvl/mxv
	.byte	W03
	.byte		        13*mus_qz_638_mvl/mxv
	.byte	W10
	.byte		PAN   , c_v+3
	.byte		N15   , Cn4 , v080
	.byte		N15   , Gn4
	.byte	W16
	.byte		PAN   , c_v-25
	.byte		N15   , Cn4 , v072
	.byte		N15   , Gn4
	.byte	W16
	.byte		PAN   , c_v+3
	.byte		N15   , Cn4 , v052
	.byte		N15   , Gn4
	.byte	W16
	.byte		PAN   , c_v-25
	.byte		N15   , Cn4 , v032
	.byte		N15   , Gn4
	.byte	W16
	.byte		PAN   , c_v+3
	.byte		N15   , Cn4 , v020
	.byte		N15   , Gn4
	.byte	W16
	.byte	W96
mus_qz_638_1_B1:
	.byte	W96
	.byte	W96
	.byte		VOICE , 33
	.byte		PAN   , c_v-13
	.byte		VOL   , 45*mus_qz_638_mvl/mxv
	.byte		        51*mus_qz_638_mvl/mxv
	.byte	W48
	.byte		N44   , Cn4 , v100 , gtp3
	.byte	W48
	.byte		N11   , Dn4
	.byte	W12
	.byte		        Cn4
	.byte	W12
	.byte		        As3
	.byte	W12
	.byte		N23   , Gn3
	.byte	W24
	.byte		N11   , As3
	.byte	W12
	.byte		N23   , Dn4
	.byte	W24
	.byte		N92   , Cs4 , v100 , gtp3
	.byte	W96
	.byte		TIE   , Cn4
	.byte	W60
	.byte	W03
	.byte		VOL   , 41*mus_qz_638_mvl/mxv
	.byte	W02
	.byte		        40*mus_qz_638_mvl/mxv
	.byte	W03
	.byte		        38*mus_qz_638_mvl/mxv
	.byte	W04
	.byte		        36*mus_qz_638_mvl/mxv
	.byte	W02
	.byte		        35*mus_qz_638_mvl/mxv
	.byte	W03
	.byte		        33*mus_qz_638_mvl/mxv
	.byte	W03
	.byte		        30*mus_qz_638_mvl/mxv
	.byte	W04
	.byte		        29*mus_qz_638_mvl/mxv
	.byte	W02
	.byte		        27*mus_qz_638_mvl/mxv
	.byte	W03
	.byte		        24*mus_qz_638_mvl/mxv
	.byte	W03
	.byte		        20*mus_qz_638_mvl/mxv
	.byte	W04
	.byte		        18*mus_qz_638_mvl/mxv
	.byte	W02
	.byte		        14*mus_qz_638_mvl/mxv
	.byte	W03
	.byte		        10*mus_qz_638_mvl/mxv
	.byte	W03
	.byte		        2*mus_qz_638_mvl/mxv
	.byte	W04
	.byte		        0*mus_qz_638_mvl/mxv
	.byte	W02
	.byte	EOT
	.byte	W22
	.byte		VOL   , 51*mus_qz_638_mvl/mxv
	.byte	W12
	.byte		N44   , Ds3 , v100 , gtp3
	.byte		N44   , Cn4 , v100 , gtp3
	.byte	W48
	.byte		N11   , Fn3
	.byte		N11   , Dn4
	.byte	W12
	.byte		        Ds3
	.byte		N11   , Cn4
	.byte	W12
	.byte		        Dn3
	.byte		N11   , As3
	.byte	W12
	.byte		N23   , As2
	.byte		N11   , Gn3
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		        Dn3
	.byte		N11   , As3
	.byte	W12
	.byte		N23   , Gn3
	.byte		N23   , Dn4
	.byte	W24
	.byte		N68   , Fn3 , v100 , gtp3
	.byte		N68   , Cs4 , v100 , gtp3
	.byte	W72
	.byte		N11   , Ds3
	.byte		N11   , Cn4
	.byte	W12
	.byte		        Fn3
	.byte		N11   , Cs4
	.byte	W12
	.byte		N92   , Gn3 , v100 , gtp3
	.byte		N92   , Ds4 , v100 , gtp3
	.byte	W68
	.byte	W01
	.byte		VOL   , 45*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        46*mus_qz_638_mvl/mxv
	.byte	W03
	.byte		        48*mus_qz_638_mvl/mxv
	.byte	W03
	.byte		        49*mus_qz_638_mvl/mxv
	.byte	W04
	.byte		        51*mus_qz_638_mvl/mxv
	.byte	W02
	.byte		        53*mus_qz_638_mvl/mxv
	.byte	W03
	.byte		        54*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		VOICE , 18
	.byte		PAN   , c_v+10
	.byte		VOL   , 63*mus_qz_638_mvl/mxv
	.byte		MOD   , 3
	.byte		VOL   , 48*mus_qz_638_mvl/mxv
	.byte		N23   , Fn4
	.byte	W24
	.byte		N11   , Ds4
	.byte	W12
	.byte		        Dn4
	.byte	W12
	.byte		N23   , Ds4
	.byte	W24
	.byte		N11   , Dn4
	.byte	W12
	.byte		        Cn4
	.byte	W12
	.byte		        Dn4
	.byte	W12
	.byte		        Cn4
	.byte	W12
	.byte		        As3
	.byte	W12
	.byte		        Cn4
	.byte	W12
	.byte		N23   , Dn4
	.byte	W24
	.byte		N11   , Cn4
	.byte	W12
	.byte		        As3
	.byte	W12
	.byte		N44   , Cs4 , v100 , gtp3
	.byte	W48
	.byte		        As3 , v100 , gtp3
	.byte	W48
	.byte		N11   , Gn3
	.byte		N11   , Ds4
	.byte	W12
	.byte		        Fn3
	.byte		N11   , Dn4
	.byte	W12
	.byte		        Ds3
	.byte		N11   , Cn4
	.byte	W12
	.byte		        Dn3
	.byte		N11   , As3
	.byte	W12
	.byte		N44   , Ds3 , v100 , gtp3
	.byte		N44   , Cn4 , v100 , gtp3
	.byte	W48
	.byte		N23   , Gs3
	.byte		N23   , Fn4
	.byte	W24
	.byte		N11   , Gn3
	.byte		N11   , Ds4
	.byte	W12
	.byte		        Fn3
	.byte		N11   , Dn4
	.byte	W12
	.byte		N23   , Gn3
	.byte		N23   , Ds4
	.byte	W24
	.byte		N11   , Fn3
	.byte		N11   , Dn4
	.byte	W12
	.byte		        Ds3
	.byte		N11   , Cn4
	.byte	W12
	.byte		        Fn3
	.byte		N11   , Dn4
	.byte	W12
	.byte		        Ds3
	.byte		N11   , Cn4
	.byte	W12
	.byte		        Dn3
	.byte		N11   , As3
	.byte	W12
	.byte		        Ds3
	.byte		N11   , Cn4
	.byte	W12
	.byte		N23   , Fn3
	.byte		N23   , Dn4
	.byte	W24
	.byte		N11   , Ds3
	.byte		N11   , Cn4
	.byte	W12
	.byte		        Dn3
	.byte		N11   , As3
	.byte	W12
	.byte		N23   , Fn3
	.byte		N23   , Cs4
	.byte	W24
	.byte		N11   , Ds3
	.byte		N11   , Cn4
	.byte	W12
	.byte		        Fn3
	.byte		N11   , Cs4
	.byte	W12
	.byte		        Ds3
	.byte		N11   , Cn4
	.byte	W12
	.byte		        Fn3
	.byte		N11   , Cs4
	.byte	W12
	.byte		        Cs3
	.byte		N11   , As3
	.byte	W12
	.byte		        Fn3
	.byte		N11   , Cs4
	.byte	W12
	.byte		N92   , Ds3 , v100 , gtp3
	.byte		N92   , Cn4 , v100 , gtp3
	.byte	W66
	.byte		VOL   , 53*mus_qz_638_mvl/mxv
	.byte	W02
	.byte		        48*mus_qz_638_mvl/mxv
	.byte	W04
	.byte		        45*mus_qz_638_mvl/mxv
	.byte	W02
	.byte		        41*mus_qz_638_mvl/mxv
	.byte	W03
	.byte		        36*mus_qz_638_mvl/mxv
	.byte	W03
	.byte		        28*mus_qz_638_mvl/mxv
	.byte	W04
	.byte		        21*mus_qz_638_mvl/mxv
	.byte	W02
	.byte		        17*mus_qz_638_mvl/mxv
	.byte	W03
	.byte		        9*mus_qz_638_mvl/mxv
	.byte	W03
	.byte		        6*mus_qz_638_mvl/mxv
	.byte	W04
	.byte		        2*mus_qz_638_mvl/mxv
	.byte	W02
	.byte		        0*mus_qz_638_mvl/mxv
	.byte	W10
	.byte		VOICE , 3
	.byte	W12
	.byte		VOL   , 58*mus_qz_638_mvl/mxv
	.byte		MOD   , 0
	.byte		VOL   , 53*mus_qz_638_mvl/mxv
	.byte	W24
	.byte		N11   , Cn5
	.byte	W12
	.byte		        Cn5 , v076
	.byte	W12
	.byte		        Cn5 , v060
	.byte	W12
	.byte		        Cn5 , v048
	.byte	W12
mus_qz_638_1_000:
	.byte		N11   , Dn5 , v100
	.byte	W12
	.byte		        Cn5
	.byte	W12
	.byte		        As4
	.byte	W12
	.byte		        Gn4
	.byte	W12
	.byte		        Gn4 , v076
	.byte	W12
	.byte		        As4 , v100
	.byte	W12
	.byte		        Dn5
	.byte	W12
	.byte	N11
	.byte	W12
	.byte	PEND
mus_qz_638_1_001:
	.byte		N11   , Cs5 , v100
	.byte	W12
	.byte		        Cs5 , v076
	.byte	W12
	.byte		        Cs5 , v060
	.byte	W12
	.byte		        Cs5 , v048
	.byte	W12
	.byte		        Cs5 , v036
	.byte	W12
	.byte		        Cs5 , v032
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		        Cs5 , v024
	.byte	W12
	.byte	PEND
mus_qz_638_1_002:
	.byte		N11   , Cn5 , v100
	.byte	W12
	.byte		        Cn5 , v076
	.byte	W12
	.byte		        Cn5 , v060
	.byte	W12
	.byte		        Cn5 , v048
	.byte	W12
	.byte		        Cn5 , v036
	.byte	W12
	.byte		        Cn5 , v032
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		        Cn5 , v024
	.byte	W12
	.byte	PEND
	.byte	W48
	.byte		        Cn5 , v100
	.byte	W12
	.byte		        Cn5 , v076
	.byte	W12
	.byte		        Cn5 , v060
	.byte	W12
	.byte		        Cn5 , v048
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_638_1_000
	.byte	PATT
	 ptrvalue	mus_qz_638_1_001
	.byte	PATT
	 ptrvalue	mus_qz_638_1_002
	.byte		N11   , Cn5 , v016
	.byte	W12
	.byte		        Cn5 , v012
	.byte	W12
	.byte		        Cn5 , v008
	.byte	W12
	.byte		        Cn5 , v004
	.byte	W60
	.byte		VOICE , 2
	.byte		PAN   , c_v-20
	.byte	W24
	.byte		MOD   , 1
	.byte		VOL   , 61*mus_qz_638_mvl/mxv
	.byte	W24
	.byte		N11   , Fn2 , v100
	.byte		N11   , As3
	.byte	W12
	.byte		        Gn2
	.byte		N11   , Cn4
	.byte	W12
	.byte		N23   , Gs2
	.byte		N23   , Cs4
	.byte	W24
mus_qz_638_1_003:
	.byte		N92   , Ds3 , v100 , gtp3
	.byte		N92   , Cn4 , v100 , gtp3
	.byte	W96
	.byte	PEND
	.byte	W24
	.byte		VOICE , 5
	.byte	W24
	.byte		N11   , Gs2
	.byte		N11   , Cs4
	.byte	W12
	.byte		        Gn2
	.byte		N11   , Cn4
	.byte	W12
	.byte		N23   , Gs2
	.byte		N23   , Cs4
	.byte	W24
	.byte	PATT
	 ptrvalue	mus_qz_638_1_003
	.byte		N11   , Gn3 , v100
	.byte		N11   , Ds4
	.byte	W36
	.byte		VOICE , 18
	.byte	W12
	.byte		N11   , Fn3
	.byte		N11   , As3
	.byte	W12
	.byte		        Gn3
	.byte		N11   , Cn4
	.byte	W12
	.byte		N23   , Gs3
	.byte		N23   , Cs4
	.byte	W24
	.byte		N92   , Gn3 , v100 , gtp3
	.byte		N92   , Cn4 , v100 , gtp3
	.byte	W96
	.byte		N11   , Fn3
	.byte		N11   , As3
	.byte	W24
	.byte		VOICE , 2
	.byte	W24
	.byte		N11   , Gs3
	.byte		N11   , Cs4
	.byte	W12
	.byte		        Gn3
	.byte		N11   , Cn4
	.byte	W12
	.byte		N23   , Gs3
	.byte		N23   , Cs4
	.byte	W24
	.byte		PAN   , c_v+13
	.byte		VOL   , 43*mus_qz_638_mvl/mxv
	.byte		N44   , Gs3 , v100 , gtp3
	.byte		N44   , Ds4 , v100 , gtp3
	.byte	W48
	.byte		PAN   , c_v-22
	.byte		VOL   , 30*mus_qz_638_mvl/mxv
	.byte		N92   , Gs3 , v100 , gtp3
	.byte		N92   , Ds4 , v100 , gtp3
	.byte	W48
	.byte	W96
	.byte		MOD   , 0
	.byte		PAN   , c_v+13
	.byte		VOL   , 43*mus_qz_638_mvl/mxv
	.byte		N44   , Fn3 , v100 , gtp3
	.byte		N44   , Cs4 , v100 , gtp3
	.byte	W48
	.byte		PAN   , c_v-22
	.byte		VOL   , 30*mus_qz_638_mvl/mxv
	.byte		N92   , Fn3 , v100 , gtp3
	.byte		N92   , Cs4 , v100 , gtp3
	.byte	W48
	.byte	W48
	.byte		VOICE , 18
	.byte		VOL   , 64*mus_qz_638_mvl/mxv
	.byte		PAN   , c_v+14
	.byte		N15   , Fn3 , v080
	.byte		N15   , An3
	.byte		N15   , Cn4
	.byte	W16
	.byte		        Fs3
	.byte		N15   , As3
	.byte		N15   , Cs4
	.byte	W16
	.byte		        Gn3
	.byte		N15   , Bn3
	.byte		N15   , Dn4
	.byte	W16
	.byte		BEND  , c_v-12
	.byte		N92   , Gs3 , v100 , gtp3
	.byte		N92   , Cn4 , v100 , gtp3
	.byte		N92   , Ds4 , v100 , gtp3
	.byte	W02
	.byte		BEND  , c_v-7
	.byte	W03
	.byte		        c_v-5
	.byte	W03
	.byte		        c_v-3
	.byte	W04
	.byte		        c_v+0
	.byte	W60
	.byte		PAN   , c_v-2
	.byte	W24
	.byte		BEND  , c_v-12
	.byte		N92   , Gn3 , v100 , gtp3
	.byte		N92   , As3 , v100 , gtp3
	.byte		N92   , Dn4 , v100 , gtp3
	.byte	W02
	.byte		BEND  , c_v-7
	.byte	W03
	.byte		        c_v-5
	.byte	W03
	.byte		        c_v-3
	.byte	W04
	.byte		        c_v+0
	.byte	W60
	.byte		PAN   , c_v+13
	.byte	W24
	.byte		N92   , As3 , v100 , gtp3
	.byte		N92   , Cs4 , v100 , gtp3
	.byte		N92   , Fn4 , v100 , gtp3
	.byte	W05
	.byte		BEND  , c_v-6
	.byte	W04
	.byte		        c_v-13
	.byte	W15
	.byte		        c_v-6
	.byte	W02
	.byte		        c_v-3
	.byte	W03
	.byte		        c_v+0
	.byte	W42
	.byte	W01
	.byte		PAN   , c_v-11
	.byte	W24
	.byte		BEND  , c_v-12
	.byte		N92   , Gs3 , v100 , gtp3
	.byte		N92   , Cn4 , v100 , gtp3
	.byte		N92   , Ds4 , v100 , gtp3
	.byte	W02
	.byte		BEND  , c_v-10
	.byte	W03
	.byte		        c_v-5
	.byte	W03
	.byte		        c_v+0
	.byte	W88
	.byte		VOL   , 17*mus_qz_638_mvl/mxv
	.byte		N15   , Gs3 , v080
	.byte		N15   , Cn4
	.byte		N15   , Ds4
	.byte	W12
	.byte		VOL   , 20*mus_qz_638_mvl/mxv
	.byte	W04
	.byte		PAN   , c_v+3
	.byte		N15   , Gs3
	.byte		N15   , Cn4
	.byte		N15   , Ds4
	.byte	W16
	.byte		PAN   , c_v-25
	.byte		N15   , Gs3 , v072
	.byte		N15   , Cn4
	.byte		N15   , Ds4
	.byte	W16
	.byte		PAN   , c_v+3
	.byte		N15   , Gs3 , v052
	.byte		N15   , Cn4
	.byte		N15   , Ds4
	.byte	W16
	.byte		PAN   , c_v-25
	.byte		N15   , Gs3 , v032
	.byte		N15   , Cn4
	.byte		N15   , Ds4
	.byte	W16
	.byte		PAN   , c_v+3
	.byte		N15   , Gs3 , v020
	.byte		N15   , Cn4
	.byte		N15   , Ds4
	.byte	W16
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_638_1_B1
	.byte	FINE

/********************** Track  2 **********************/

mus_qz_638_2:
	.byte		KEYSH , mus_qz_638_key+0
	.byte		PAN   , c_v+0
	.byte		BENDR , 6
	.byte		LFOS  , 21
	.byte		VOL   , 18*mus_qz_638_mvl/mxv
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W78
	.byte		VOICE , 33
	.byte	W18
	.byte	W06
	.byte		PAN   , c_v+31
	.byte		VOL   , 1*mus_qz_638_mvl/mxv
	.byte		PAN   , c_v-20
	.byte		N92   , Gn4 , v100 , gtp3
	.byte	W36
	.byte		VOL   , 1*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        2*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        2*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        2*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        3*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        4*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        4*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        6*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        7*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        9*mus_qz_638_mvl/mxv
	.byte	W01
	.byte	W06
	.byte		        1*mus_qz_638_mvl/mxv
	.byte		PAN   , c_v-32
	.byte		        c_v-2
	.byte		N92   , As4 , v100 , gtp3
	.byte	W36
	.byte		VOL   , 1*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        2*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        2*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        2*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        3*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        4*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        4*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        6*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        7*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        9*mus_qz_638_mvl/mxv
	.byte	W01
	.byte	W06
	.byte		        1*mus_qz_638_mvl/mxv
	.byte		PAN   , c_v+32
	.byte		N92   , Fn4 , v100 , gtp3
	.byte	W36
	.byte		VOL   , 1*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        2*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        2*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        2*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        3*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        4*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        4*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        6*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        7*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        9*mus_qz_638_mvl/mxv
	.byte	W01
	.byte	W06
	.byte		        1*mus_qz_638_mvl/mxv
	.byte		PAN   , c_v-36
	.byte		        c_v-11
	.byte		N92   , Gn4 , v100 , gtp3
	.byte	W36
	.byte		VOL   , 1*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        2*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        2*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        2*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        3*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        4*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        4*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        6*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        7*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        9*mus_qz_638_mvl/mxv
	.byte	W01
	.byte	W06
	.byte		        3*mus_qz_638_mvl/mxv
	.byte		PAN   , c_v+22
	.byte		VOL   , 5*mus_qz_638_mvl/mxv
	.byte		PAN   , c_v-25
	.byte	N15
	.byte	W06
	.byte		VOL   , 4*mus_qz_638_mvl/mxv
	.byte	W10
	.byte		PAN   , c_v-39
	.byte		        c_v+3
	.byte		N15   , Gn4 , v080
	.byte	W16
	.byte		PAN   , c_v+28
	.byte		        c_v-25
	.byte		N15   , Gn4 , v072
	.byte	W16
	.byte		PAN   , c_v-33
	.byte		        c_v+3
	.byte		N15   , Gn4 , v052
	.byte	W16
	.byte		PAN   , c_v+15
	.byte		        c_v-25
	.byte		N15   , Gn4 , v032
	.byte	W16
	.byte		PAN   , c_v+3
	.byte		N15   , Gn4 , v020
	.byte	W10
	.byte	W96
mus_qz_638_2_B1:
	.byte	W96
	.byte	W96
	.byte	W06
	.byte		VOICE , 33
	.byte		PAN   , c_v-13
	.byte		        c_v+37
	.byte		VOL   , 18*mus_qz_638_mvl/mxv
	.byte	W48
	.byte		N44   , Cn4 , v100 , gtp3
	.byte	W42
	.byte	W06
	.byte		N11   , Dn4
	.byte	W12
	.byte		        Cn4
	.byte	W12
	.byte		        As3
	.byte	W12
	.byte		N23   , Gn3
	.byte	W24
	.byte		N11   , As3
	.byte	W12
	.byte		N23   , Dn4
	.byte	W18
	.byte	W06
	.byte		N92   , Cs4 , v100 , gtp3
	.byte	W90
	.byte	W06
	.byte		TIE   , Cn4
	.byte	W66
	.byte		VOL   , 13*mus_qz_638_mvl/mxv
	.byte	W06
	.byte		        10*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        9*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        6*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        4*mus_qz_638_mvl/mxv
	.byte	W01
	.byte	W06
	.byte		        2*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        0*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        0*mus_qz_638_mvl/mxv
	.byte	W02
	.byte	EOT
	.byte	W03
	.byte		VOL   , 0*mus_qz_638_mvl/mxv
	.byte	W24
	.byte	W01
	.byte		        18*mus_qz_638_mvl/mxv
	.byte	W06
	.byte		N44   , Cn4 , v100 , gtp3
	.byte	W42
	.byte	W06
	.byte		N11   , Dn4
	.byte	W12
	.byte		        Cn4
	.byte	W12
	.byte		        As3
	.byte	W12
	.byte		        Gn3
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		        As3
	.byte	W12
	.byte		N23   , Dn4
	.byte	W18
	.byte	W06
	.byte		N68   , Cs4 , v100 , gtp3
	.byte	W72
	.byte		N11   , Cn4
	.byte	W12
	.byte		        Cs4
	.byte	W06
	.byte	W06
	.byte		N88   , Ds4 , v100 , gtp1
	.byte	W68
	.byte	W01
	.byte		VOL   , 18*mus_qz_638_mvl/mxv
	.byte	W11
	.byte		        19*mus_qz_638_mvl/mxv
	.byte	W06
	.byte		        20*mus_qz_638_mvl/mxv
	.byte	W04
	.byte	W02
	.byte		        22*mus_qz_638_mvl/mxv
	.byte	W04
	.byte		VOICE , 18
	.byte		VOL   , 17*mus_qz_638_mvl/mxv
	.byte		PAN   , c_v-39
	.byte		        c_v+10
	.byte		MOD   , 3
	.byte		N23   , Fn4
	.byte	W24
	.byte		N11   , Ds4
	.byte	W12
	.byte		        Dn4
	.byte	W12
	.byte		N23   , Ds4
	.byte	W24
	.byte		N11   , Dn4
	.byte	W12
	.byte		        Cn4
	.byte	W06
mus_qz_638_2_000:
	.byte	W06
	.byte		N11   , Dn4 , v100
	.byte	W12
	.byte		        Cn4
	.byte	W12
	.byte		        As3
	.byte	W12
	.byte		        Cn4
	.byte	W12
	.byte		N23   , Dn4
	.byte	W24
	.byte		N11   , Cn4
	.byte	W12
	.byte		        As3
	.byte	W06
	.byte	PEND
	.byte	W06
	.byte		N44   , Cs4 , v100 , gtp3
	.byte	W48
	.byte		        As3 , v100 , gtp3
	.byte	W42
	.byte	W06
	.byte		N11   , Ds4
	.byte	W12
	.byte		        Dn4
	.byte	W12
	.byte		        Cn4
	.byte	W12
	.byte		        As3
	.byte	W12
	.byte		N44   , Cn4 , v100 , gtp3
	.byte	W42
	.byte	W06
	.byte		N23   , Fn4
	.byte	W24
	.byte		N11   , Ds4
	.byte	W12
	.byte		        Dn4
	.byte	W12
	.byte		N23   , Ds4
	.byte	W24
	.byte		N11   , Dn4
	.byte	W12
	.byte		        Cn4
	.byte	W06
	.byte	PATT
	 ptrvalue	mus_qz_638_2_000
	.byte	W06
	.byte		N23   , Cs4 , v100
	.byte	W24
	.byte		N11   , Cn4
	.byte	W12
	.byte		        Cs4
	.byte	W12
	.byte		        Cn4
	.byte	W12
	.byte		        Cs4
	.byte	W12
	.byte		        As3
	.byte	W12
	.byte		        Cs4
	.byte	W06
	.byte	W06
	.byte		N92   , Cn4 , v100 , gtp3
	.byte	W66
	.byte		VOL   , 11*mus_qz_638_mvl/mxv
	.byte	W02
	.byte		        10*mus_qz_638_mvl/mxv
	.byte	W06
	.byte		        9*mus_qz_638_mvl/mxv
	.byte	W03
	.byte		        7*mus_qz_638_mvl/mxv
	.byte	W03
	.byte		        5*mus_qz_638_mvl/mxv
	.byte	W04
	.byte		        4*mus_qz_638_mvl/mxv
	.byte	W02
	.byte		        3*mus_qz_638_mvl/mxv
	.byte	W03
	.byte		        2*mus_qz_638_mvl/mxv
	.byte	W01
	.byte	W02
	.byte		        1*mus_qz_638_mvl/mxv
	.byte	W04
	.byte		        0*mus_qz_638_mvl/mxv
	.byte	W02
	.byte		        0*mus_qz_638_mvl/mxv
	.byte	W03
	.byte		        0*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		VOICE , 3
	.byte		PAN   , c_v-33
	.byte	W12
	.byte		MOD   , 0
	.byte		VOL   , 15*mus_qz_638_mvl/mxv
	.byte	W24
	.byte		N11   , Cn5
	.byte	W12
	.byte		        Cn5 , v076
	.byte	W12
	.byte		        Cn5 , v060
	.byte	W12
	.byte		        Cn5 , v048
	.byte	W06
mus_qz_638_2_001:
	.byte	W06
	.byte		N11   , Dn5 , v100
	.byte	W12
	.byte		        Cn5
	.byte	W12
	.byte		        As4
	.byte	W12
	.byte		        Gn4
	.byte	W12
	.byte		        Gn4 , v076
	.byte	W12
	.byte		        As4 , v100
	.byte	W12
	.byte		        Dn5
	.byte	W12
	.byte	N11
	.byte	W06
	.byte	PEND
mus_qz_638_2_002:
	.byte	W06
	.byte		N11   , Cs5 , v100
	.byte	W12
	.byte		        Cs5 , v076
	.byte	W12
	.byte		        Cs5 , v060
	.byte	W12
	.byte		        Cs5 , v048
	.byte	W12
	.byte		        Cs5 , v036
	.byte	W12
	.byte		        Cs5 , v032
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		        Cs5 , v024
	.byte	W06
	.byte	PEND
mus_qz_638_2_003:
	.byte	W06
	.byte		N11   , Cn5 , v100
	.byte	W12
	.byte		        Cn5 , v076
	.byte	W12
	.byte		        Cn5 , v060
	.byte	W12
	.byte		        Cn5 , v048
	.byte	W12
	.byte		        Cn5 , v036
	.byte	W12
	.byte		        Cn5 , v032
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		        Cn5 , v024
	.byte	W06
	.byte	PEND
	.byte	W54
	.byte		        Cn5 , v100
	.byte	W12
	.byte		        Cn5 , v076
	.byte	W12
	.byte		        Cn5 , v060
	.byte	W12
	.byte		        Cn5 , v048
	.byte	W06
	.byte	PATT
	 ptrvalue	mus_qz_638_2_001
	.byte	PATT
	 ptrvalue	mus_qz_638_2_002
	.byte	PATT
	 ptrvalue	mus_qz_638_2_003
	.byte	W06
	.byte		N11   , Cn5 , v016
	.byte	W12
	.byte		        Cn5 , v012
	.byte	W12
	.byte		        Cn5 , v008
	.byte	W12
	.byte		        Cn5 , v004
	.byte	W54
	.byte	W06
	.byte		VOICE , 2
	.byte		PAN   , c_v+42
	.byte	W24
	.byte		MOD   , 1
	.byte	W24
	.byte		N11   , As3 , v100
	.byte	W12
	.byte		        Cn4
	.byte	W12
	.byte		N23   , Cs4
	.byte	W18
mus_qz_638_2_004:
	.byte	W06
	.byte		N92   , Cn4 , v100 , gtp3
	.byte	W90
	.byte	PEND
	.byte	W30
	.byte		VOICE , 5
	.byte	W24
	.byte		N11   , Cs4
	.byte	W12
	.byte		        Cn4
	.byte	W12
	.byte		N23   , Cs4
	.byte	W18
	.byte	PATT
	 ptrvalue	mus_qz_638_2_004
	.byte	W06
	.byte		N11   , Ds4 , v100
	.byte	W36
	.byte		VOICE , 18
	.byte	W12
	.byte		N11   , As3
	.byte	W12
	.byte		        Cn4
	.byte	W12
	.byte		N23   , Cs4
	.byte	W18
	.byte	PATT
	 ptrvalue	mus_qz_638_2_004
	.byte	W06
	.byte		N11   , As3 , v100
	.byte	W24
	.byte		VOICE , 2
	.byte	W24
	.byte		N11   , Cs4
	.byte	W12
	.byte		        Cn4
	.byte	W12
	.byte		N23   , Cs4
	.byte	W18
	.byte	W06
	.byte		PAN   , c_v-48
	.byte		N44   , Ds4 , v100 , gtp3
	.byte	W48
	.byte		PAN   , c_v+39
	.byte		N92   , Ds4 , v100 , gtp3
	.byte	W42
	.byte	W96
	.byte	W06
	.byte		MOD   , 0
	.byte		PAN   , c_v+13
	.byte		        c_v-48
	.byte		N44   , Cs4 , v100 , gtp3
	.byte	W48
	.byte		PAN   , c_v-22
	.byte		        c_v+39
	.byte		N92   , Cs4 , v100 , gtp3
	.byte	W42
	.byte	W48
	.byte		VOL   , 29*mus_qz_638_mvl/mxv
	.byte	W06
	.byte		VOICE , 18
	.byte		PAN   , c_v-42
	.byte		N15   , Cn4 , v080
	.byte	W16
	.byte		        Cs4
	.byte	W16
	.byte		        Dn4
	.byte	W10
	.byte	W06
	.byte		BEND  , c_v-12
	.byte		N92   , Ds4 , v100 , gtp3
	.byte	W02
	.byte		BEND  , c_v-7
	.byte	W03
	.byte		        c_v-5
	.byte	W03
	.byte		        c_v-3
	.byte	W04
	.byte		        c_v+0
	.byte	W60
	.byte		PAN   , c_v+42
	.byte		        c_v-2
	.byte	W18
	.byte	W06
	.byte		BEND  , c_v-12
	.byte		N92   , Dn4 , v100 , gtp3
	.byte	W02
	.byte		BEND  , c_v-7
	.byte	W03
	.byte		        c_v-5
	.byte	W03
	.byte		        c_v-3
	.byte	W04
	.byte		        c_v+0
	.byte	W60
	.byte		PAN   , c_v-42
	.byte		        c_v+13
	.byte	W18
	.byte	W06
	.byte		N92   , Fn4 , v100 , gtp3
	.byte	W05
	.byte		BEND  , c_v-6
	.byte	W04
	.byte		        c_v-13
	.byte	W15
	.byte		        c_v-6
	.byte	W02
	.byte		        c_v-3
	.byte	W03
	.byte		        c_v+0
	.byte	W42
	.byte	W01
	.byte		PAN   , c_v+42
	.byte		        c_v-11
	.byte	W18
	.byte	W06
	.byte		BEND  , c_v-12
	.byte		TIE   , Ds4
	.byte	W02
	.byte		BEND  , c_v-10
	.byte	W03
	.byte		        c_v-5
	.byte	W03
	.byte		        c_v+0
	.byte	W80
	.byte	W02
	.byte	W21
	.byte	EOT
	.byte	W01
	.byte		PAN   , c_v-42
	.byte		        c_v+3
	.byte		N15   , Ds4 , v080
	.byte	W16
	.byte		PAN   , c_v+31
	.byte		        c_v-25
	.byte		N15   , Ds4 , v072
	.byte	W16
	.byte		PAN   , c_v-36
	.byte		        c_v+3
	.byte		N15   , Ds4 , v052
	.byte	W16
	.byte		PAN   , c_v+12
	.byte		        c_v-25
	.byte		N15   , Ds4 , v032
	.byte	W16
	.byte		PAN   , c_v-23
	.byte		        c_v+3
	.byte		N15   , Ds4 , v020
	.byte	W10
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_638_2_B1
	.byte	FINE

/********************** Track  3 **********************/

mus_qz_638_3:
	.byte		KEYSH , mus_qz_638_key+0
	.byte		VOICE , 6
	.byte		PAN   , c_v+0
	.byte		        c_v+28
	.byte		VOL   , 23*mus_qz_638_mvl/mxv
	.byte		N11   , Ds7 , v100
	.byte	W12
	.byte		        Gs6
	.byte	W12
	.byte		        Cn7
	.byte	W12
	.byte		        Ds7
	.byte	W12
	.byte		        Gs6
	.byte	W12
	.byte		        Cn7
	.byte	W12
	.byte		        Ds7
	.byte	W12
	.byte		        Cn7
	.byte	W12
mus_qz_638_3_000:
	.byte		N11   , Dn7 , v100
	.byte	W12
	.byte		        Gn6
	.byte	W12
	.byte		        As6
	.byte	W12
	.byte		        Dn7
	.byte	W12
	.byte		        Gn6
	.byte	W12
	.byte		        As6
	.byte	W12
	.byte		        Dn7
	.byte	W12
	.byte		        Gn6
	.byte	W12
	.byte	PEND
mus_qz_638_3_001:
	.byte		N11   , Cs7 , v100
	.byte	W12
	.byte		        Fn6
	.byte	W12
	.byte		        As6
	.byte	W12
	.byte		        Cs7
	.byte	W12
	.byte		        Fn6
	.byte	W12
	.byte		        As6
	.byte	W12
	.byte		        Cs7
	.byte	W12
	.byte		        As6
	.byte	W12
	.byte	PEND
mus_qz_638_3_002:
	.byte		N11   , Cn7 , v100
	.byte	W12
	.byte		        Gn6
	.byte	W12
	.byte		        As6
	.byte	W12
	.byte		        Cn7
	.byte	W12
	.byte		        Gn6
	.byte	W12
	.byte		        As6
	.byte	W12
	.byte		        Cn7
	.byte	W12
	.byte		        As6
	.byte	W12
	.byte	PEND
mus_qz_638_3_003:
	.byte		N11   , Ds7 , v100
	.byte	W12
	.byte		        Gs6
	.byte	W12
	.byte		        Cn7
	.byte	W12
	.byte		        Ds7
	.byte	W12
	.byte		        Gs6
	.byte	W12
	.byte		        Cn7
	.byte	W12
	.byte		        Ds7
	.byte	W12
	.byte		        Cn7
	.byte	W12
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_638_3_000
	.byte	PATT
	 ptrvalue	mus_qz_638_3_001
	.byte	PATT
	 ptrvalue	mus_qz_638_3_002
	.byte	PATT
	 ptrvalue	mus_qz_638_3_003
	.byte	PATT
	 ptrvalue	mus_qz_638_3_000
mus_qz_638_3_B1:
	.byte	PATT
	 ptrvalue	mus_qz_638_3_001
	.byte	PATT
	 ptrvalue	mus_qz_638_3_002
	.byte	PATT
	 ptrvalue	mus_qz_638_3_003
	.byte	PATT
	 ptrvalue	mus_qz_638_3_000
	.byte	PATT
	 ptrvalue	mus_qz_638_3_001
	.byte	PATT
	 ptrvalue	mus_qz_638_3_002
	.byte	PATT
	 ptrvalue	mus_qz_638_3_003
	.byte	PATT
	 ptrvalue	mus_qz_638_3_000
	.byte	PATT
	 ptrvalue	mus_qz_638_3_001
	.byte	PATT
	 ptrvalue	mus_qz_638_3_002
	.byte	PATT
	 ptrvalue	mus_qz_638_3_003
	.byte	PATT
	 ptrvalue	mus_qz_638_3_000
	.byte	PATT
	 ptrvalue	mus_qz_638_3_001
	.byte	PATT
	 ptrvalue	mus_qz_638_3_002
	.byte	PATT
	 ptrvalue	mus_qz_638_3_003
	.byte	PATT
	 ptrvalue	mus_qz_638_3_000
	.byte	PATT
	 ptrvalue	mus_qz_638_3_001
	.byte	PATT
	 ptrvalue	mus_qz_638_3_002
	.byte	PATT
	 ptrvalue	mus_qz_638_3_003
mus_qz_638_3_004:
	.byte		N11   , Dn7 , v100
	.byte	W12
	.byte		        Gn6
	.byte	W12
	.byte		        As6
	.byte	W12
	.byte		        Dn7
	.byte	W12
	.byte		        Gn6
	.byte	W12
	.byte		        As6
	.byte	W12
	.byte		        Dn7
	.byte	W12
	.byte		        As6
	.byte	W12
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_638_3_001
	.byte	PATT
	 ptrvalue	mus_qz_638_3_002
	.byte	PATT
	 ptrvalue	mus_qz_638_3_003
	.byte	PATT
	 ptrvalue	mus_qz_638_3_004
	.byte		N11   , Cs7 , v100
	.byte	W12
	.byte		        Gn6
	.byte	W12
	.byte		        As6
	.byte	W12
	.byte		        Cs7
	.byte	W12
	.byte		        Gn6
	.byte	W12
	.byte		        As6
	.byte	W12
	.byte		        Cs7
	.byte	W12
	.byte		        As6
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_638_3_002
	.byte	PATT
	 ptrvalue	mus_qz_638_3_003
	.byte	PATT
	 ptrvalue	mus_qz_638_3_000
	.byte	PATT
	 ptrvalue	mus_qz_638_3_001
	.byte	PATT
	 ptrvalue	mus_qz_638_3_002
	.byte	PATT
	 ptrvalue	mus_qz_638_3_003
	.byte	PATT
	 ptrvalue	mus_qz_638_3_000
	.byte	PATT
	 ptrvalue	mus_qz_638_3_001
	.byte	PATT
	 ptrvalue	mus_qz_638_3_002
	.byte	PATT
	 ptrvalue	mus_qz_638_3_003
	.byte	PATT
	 ptrvalue	mus_qz_638_3_004
	.byte	PATT
	 ptrvalue	mus_qz_638_3_001
	.byte	PATT
	 ptrvalue	mus_qz_638_3_002
	.byte	PATT
	 ptrvalue	mus_qz_638_3_003
	.byte	PATT
	 ptrvalue	mus_qz_638_3_004
	.byte	PATT
	 ptrvalue	mus_qz_638_3_001
	.byte	PATT
	 ptrvalue	mus_qz_638_3_002
	.byte	PATT
	 ptrvalue	mus_qz_638_3_003
	.byte	PATT
	 ptrvalue	mus_qz_638_3_000
	.byte	GOTO
	 ptrvalue	mus_qz_638_3_B1
	.byte	FINE

/********************** Track  4 **********************/

mus_qz_638_4:
	.byte		KEYSH , mus_qz_638_key+0
	.byte		VOICE , 6
	.byte		PAN   , c_v+0
	.byte		BENDR , 6
	.byte		VOL   , 34*mus_qz_638_mvl/mxv
	.byte		PAN   , c_v-12
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W72
	.byte		VOL   , 34*mus_qz_638_mvl/mxv
	.byte	W12
	.byte		PAN   , c_v+25
	.byte	W12
mus_qz_638_4_000:
	.byte		N05   , Ds5 , v100
	.byte	W12
	.byte		        Gs4
	.byte	W12
	.byte		        Cn5
	.byte	W12
	.byte		        Ds5
	.byte	W12
	.byte		        Gs4
	.byte	W12
	.byte		        Cn5
	.byte	W12
	.byte		        Ds5
	.byte	W12
	.byte		        Cn5
	.byte	W12
	.byte	PEND
mus_qz_638_4_001:
	.byte		N05   , Dn5 , v100
	.byte	W12
	.byte		        Gn4
	.byte	W12
	.byte		        As4
	.byte	W12
	.byte		        Dn5
	.byte	W12
	.byte		        Gn4
	.byte	W12
	.byte		        As4
	.byte	W12
	.byte		        Dn5
	.byte	W12
	.byte		        Gn4
	.byte	W12
	.byte	PEND
mus_qz_638_4_B1:
	.byte		N05   , Cs5 , v100
	.byte	W12
	.byte		        Fn4
	.byte	W12
	.byte		        As4
	.byte	W12
	.byte		        Cs5
	.byte	W12
	.byte		        Fn4
	.byte	W12
	.byte		        As4
	.byte	W12
	.byte		        Cs5
	.byte	W12
	.byte		        As4
	.byte	W12
	.byte	PEND
mus_qz_638_4_002:
	.byte		N05   , Cn5 , v100
	.byte	W12
	.byte		        Gn4
	.byte	W12
	.byte		        As4
	.byte	W12
	.byte		        Cn5
	.byte	W12
	.byte		        Gn4
	.byte	W12
	.byte		        As4
	.byte	W12
	.byte		        Cn5
	.byte	W12
	.byte		        As4
	.byte	W12
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_638_4_000
	.byte	PATT
	 ptrvalue	mus_qz_638_4_001
	.byte	PATT
	 ptrvalue	mus_qz_638_4_B1
	.byte	PATT
	 ptrvalue	mus_qz_638_4_002
	.byte	PATT
	 ptrvalue	mus_qz_638_4_000
	.byte	PATT
	 ptrvalue	mus_qz_638_4_001
	.byte	PATT
	 ptrvalue	mus_qz_638_4_B1
	.byte	PATT
	 ptrvalue	mus_qz_638_4_002
mus_qz_638_4_003:
	.byte		N23   , Fn5 , v100
	.byte	W24
	.byte		N11   , Ds5
	.byte	W12
	.byte		        Dn5
	.byte	W12
	.byte		N23   , Ds5
	.byte	W24
	.byte		N11   , Dn5
	.byte	W12
	.byte		        Cn5
	.byte	W12
	.byte	PEND
mus_qz_638_4_004:
	.byte		N11   , Dn5 , v100
	.byte	W12
	.byte		        Cn5
	.byte	W12
	.byte		        As4
	.byte	W12
	.byte		        Cn5
	.byte	W12
	.byte		N23   , Dn5
	.byte	W24
	.byte		N11   , Cn5
	.byte	W12
	.byte		        As4
	.byte	W12
	.byte	PEND
	.byte		N44   , Cs5 , v100 , gtp3
	.byte	W48
	.byte		        As4 , v100 , gtp3
	.byte	W48
	.byte		N11   , Ds5
	.byte	W12
	.byte		        Dn5
	.byte	W12
	.byte		        Cn5
	.byte	W12
	.byte		        As4
	.byte	W12
	.byte		N44   , Cn5 , v100 , gtp3
	.byte	W48
	.byte	PATT
	 ptrvalue	mus_qz_638_4_003
	.byte	PATT
	 ptrvalue	mus_qz_638_4_004
	.byte		N23   , Cs5 , v100
	.byte	W24
	.byte		N11   , Cn5
	.byte	W12
	.byte		        Cs5
	.byte	W12
	.byte		        Cn5
	.byte	W12
	.byte		        Cs5
	.byte	W12
	.byte		        As4
	.byte	W12
	.byte		        Cs5
	.byte	W12
	.byte		VOL   , 56*mus_qz_638_mvl/mxv
	.byte		PAN   , c_v-21
	.byte		N15   , Ds2
	.byte		N15   , Fn2
	.byte		N15   , Cn3
	.byte	W16
	.byte		BEND  , c_v-6
	.byte		N15   , Ds2 , v080
	.byte		N15   , Fn2
	.byte		N15   , Cn3
	.byte	W16
	.byte		BEND  , c_v-11
	.byte		N15   , Ds2 , v072
	.byte		N15   , Fn2
	.byte		N15   , Cn3
	.byte	W16
	.byte		BEND  , c_v-24
	.byte		N15   , Ds2 , v052
	.byte		N15   , Fn2
	.byte		N15   , Cn3
	.byte	W16
	.byte		BEND  , c_v-38
	.byte		N15   , Ds2 , v032
	.byte		N15   , Fn2
	.byte		N15   , Cn3
	.byte	W16
	.byte		BEND  , c_v-50
	.byte		N15   , Ds2 , v020
	.byte		N15   , Fn2
	.byte		N15   , Cn3
	.byte	W16
	.byte		PAN   , c_v+25
	.byte		VOL   , 34*mus_qz_638_mvl/mxv
	.byte		BEND  , c_v+0
	.byte		N05   , Ds5 , v100
	.byte	W12
	.byte		        Gs4
	.byte	W12
	.byte		        Cn5
	.byte	W12
	.byte		        Ds5
	.byte	W12
	.byte		        Gs4
	.byte	W12
	.byte		        Cn5
	.byte	W12
	.byte		        Ds5
	.byte	W12
	.byte		        Cn5
	.byte	W12
mus_qz_638_4_005:
	.byte		N05   , Dn5 , v100
	.byte	W12
	.byte		        Gn4
	.byte	W12
	.byte		        As4
	.byte	W12
	.byte		        Dn5
	.byte	W12
	.byte		        Gn4
	.byte	W12
	.byte		        As4
	.byte	W12
	.byte		        Dn5
	.byte	W12
	.byte		        As4
	.byte	W12
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_638_4_B1
	.byte	PATT
	 ptrvalue	mus_qz_638_4_002
	.byte	PATT
	 ptrvalue	mus_qz_638_4_000
	.byte	PATT
	 ptrvalue	mus_qz_638_4_005
	.byte		N05   , Cs5 , v100
	.byte	W12
	.byte		        Gn4
	.byte	W12
	.byte		        As4
	.byte	W12
	.byte		        Cs5
	.byte	W12
	.byte		        Gn4
	.byte	W12
	.byte		        As4
	.byte	W12
	.byte		        Cs5
	.byte	W12
	.byte		        As4
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_638_4_002
	.byte	W96
	.byte		PAN   , c_v+18
	.byte	W96
	.byte		VOICE , 6
	.byte		VOL   , 41*mus_qz_638_mvl/mxv
	.byte	W48
	.byte		N15   , Ds4 , v100
	.byte		N15   , Fn4
	.byte		N15   , Cn5
	.byte	W16
	.byte		BEND  , c_v-6
	.byte		N15   , Ds4 , v080
	.byte		N15   , Fn4
	.byte		N15   , Cn5
	.byte	W16
	.byte		BEND  , c_v-11
	.byte		N15   , Ds4 , v072
	.byte		N15   , Fn4
	.byte		N15   , Cn5
	.byte	W16
	.byte		BEND  , c_v-24
	.byte		N15   , Ds4 , v052
	.byte		N15   , Fn4
	.byte		N15   , Cn5
	.byte	W16
	.byte		BEND  , c_v-38
	.byte		N15   , Ds4 , v032
	.byte		N15   , Fn4
	.byte		N15   , Cn5
	.byte	W16
	.byte		BEND  , c_v-50
	.byte		N15   , Ds4 , v020
	.byte		N15   , Fn4
	.byte		N15   , Cn5
	.byte	W40
	.byte		BEND  , c_v+0
	.byte	W24
	.byte	W36
	.byte		VOICE , 2
	.byte	W12
	.byte		N15   , As3 , v100
	.byte		N15   , Cn4
	.byte		N15   , Gn4
	.byte	W16
	.byte		BEND  , c_v-6
	.byte		N15   , As3 , v080
	.byte		N15   , Cn4
	.byte		N15   , Gn4
	.byte	W16
	.byte		BEND  , c_v-11
	.byte		N15   , As3 , v072
	.byte		N15   , Cn4
	.byte		N15   , Gn4
	.byte	W16
	.byte		BEND  , c_v-24
	.byte		N15   , As3 , v052
	.byte		N15   , Cn4
	.byte		N15   , Gn4
	.byte	W16
	.byte		BEND  , c_v-38
	.byte		N15   , As3 , v032
	.byte		N15   , Cn4
	.byte		N15   , Gn4
	.byte	W16
	.byte		BEND  , c_v-50
	.byte		N15   , As3 , v020
	.byte		N15   , Cn4
	.byte		N15   , Gn4
	.byte	W40
	.byte		BEND  , c_v+0
	.byte	W24
	.byte	W96
	.byte	W72
	.byte		VOICE , 6
	.byte	W24
	.byte		PAN   , c_v+25
	.byte		VOL   , 34*mus_qz_638_mvl/mxv
	.byte		N05   , Ds5 , v100
	.byte	W12
	.byte		        Gs4
	.byte	W12
	.byte		        Cn5
	.byte	W12
	.byte		        Ds5
	.byte	W12
	.byte		        Gs4
	.byte	W12
	.byte		        Cn5
	.byte	W12
	.byte		        Ds5
	.byte	W12
	.byte		        Cn5
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_638_4_005
	.byte	PATT
	 ptrvalue	mus_qz_638_4_B1
	.byte	PATT
	 ptrvalue	mus_qz_638_4_002
	.byte	PATT
	 ptrvalue	mus_qz_638_4_000
	.byte	PATT
	 ptrvalue	mus_qz_638_4_005
	.byte	PATT
	 ptrvalue	mus_qz_638_4_B1
	.byte	PATT
	 ptrvalue	mus_qz_638_4_002
	.byte	PATT
	 ptrvalue	mus_qz_638_4_000
	.byte	PATT
	 ptrvalue	mus_qz_638_4_001
	.byte	GOTO
	 ptrvalue	mus_qz_638_4_B1
	.byte	FINE

/********************** Track  5 **********************/

mus_qz_638_5:
	.byte		KEYSH , mus_qz_638_key+0
	.byte		VOICE , 23
	.byte		PAN   , c_v+0
	.byte		LFOS  , 21
	.byte		VOL   , 27*mus_qz_638_mvl/mxv
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
mus_qz_638_5_B1:
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
	.byte		VOICE , 23
	.byte	W72
	.byte		PAN   , c_v+0
	.byte	W24
	.byte		VOL   , 21*mus_qz_638_mvl/mxv
	.byte		        3*mus_qz_638_mvl/mxv
	.byte		N92   , Ds5 , v108 , gtp3
	.byte		N92   , Fn5 , v108 , gtp3
	.byte	W24
	.byte		VOL   , 3*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        4*mus_qz_638_mvl/mxv
	.byte	W12
	.byte		        5*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        5*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        7*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        8*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        11*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        12*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        14*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        15*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        20*mus_qz_638_mvl/mxv
	.byte	W07
	.byte	W12
	.byte		PAN   , c_v-32
	.byte	W12
	.byte		VOICE , 18
	.byte		MOD   , 3
	.byte		VOL   , 12*mus_qz_638_mvl/mxv
	.byte	W24
	.byte		N44   , Cn6 , v100 , gtp3
	.byte	W48
mus_qz_638_5_000:
	.byte		N11   , Dn6 , v100
	.byte	W12
	.byte		        Cn6
	.byte	W12
	.byte		        As5
	.byte	W12
	.byte		N23   , Gn5
	.byte	W24
	.byte		N11   , As5
	.byte	W12
	.byte		N23   , Dn6
	.byte	W24
	.byte	PEND
	.byte		N92   , Cs6 , v100 , gtp3
	.byte	W96
	.byte		        Cn6 , v100 , gtp3
	.byte	W96
	.byte	W48
	.byte		N44   , Cn6 , v100 , gtp3
	.byte	W48
	.byte	PATT
	 ptrvalue	mus_qz_638_5_000
	.byte		N92   , Cs6 , v100 , gtp3
	.byte	W96
	.byte		TIE   , Cn6
	.byte	W96
	.byte		VOL   , 9*mus_qz_638_mvl/mxv
	.byte	W12
	.byte		        7*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        6*mus_qz_638_mvl/mxv
	.byte	W12
	.byte		        5*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        3*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        1*mus_qz_638_mvl/mxv
	.byte	W06
	.byte	EOT
	.byte	W01
	.byte		VOL   , 0*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        0*mus_qz_638_mvl/mxv
	.byte	W19
	.byte		MOD   , 0
	.byte	W24
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
	 ptrvalue	mus_qz_638_5_B1
	.byte	FINE

/********************** Track  6 **********************/

mus_qz_638_6:
	.byte		KEYSH , mus_qz_638_key+0
	.byte		PAN   , c_v+0
	.byte		VOL   , 45*mus_qz_638_mvl/mxv
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
mus_qz_638_6_B1:
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		PAN   , c_v-28
	.byte	W72
	.byte		VOICE , 23
	.byte		VOL   , 31*mus_qz_638_mvl/mxv
	.byte		PAN   , c_v-17
	.byte	W24
mus_qz_638_6_000:
	.byte		N44   , Gs3 , v100 , gtp3
	.byte	W48
	.byte		        Ds3 , v100 , gtp3
	.byte	W48
	.byte	PEND
mus_qz_638_6_001:
	.byte		N44   , Gn3 , v100 , gtp3
	.byte	W48
	.byte		        Dn3 , v100 , gtp3
	.byte	W48
	.byte	PEND
mus_qz_638_6_002:
	.byte		N44   , Fn3 , v100 , gtp3
	.byte	W48
	.byte		        Cs3 , v100 , gtp3
	.byte	W48
	.byte	PEND
	.byte		        Ds3 , v100 , gtp3
	.byte	W48
	.byte		VOICE , 8
	.byte		N44   , Gn3 , v100 , gtp3
	.byte	W48
	.byte	PATT
	 ptrvalue	mus_qz_638_6_000
	.byte	PATT
	 ptrvalue	mus_qz_638_6_001
	.byte	PATT
	 ptrvalue	mus_qz_638_6_002
	.byte	W72
	.byte		VOICE , 36
	.byte	W24
	.byte		PAN   , c_v-19
	.byte		VOL   , 31*mus_qz_638_mvl/mxv
	.byte		N11   , Cn5 , v100
	.byte	W12
	.byte		PAN   , c_v-23
	.byte		N11   , Bn4
	.byte	W12
	.byte		PAN   , c_v-24
	.byte		N11   , As4
	.byte	W12
	.byte		PAN   , c_v-26
	.byte		N11   , An4
	.byte	W12
	.byte		PAN   , c_v-28
	.byte		N11   , Gs4
	.byte	W12
	.byte		        Gn4
	.byte	W12
	.byte		PAN   , c_v-32
	.byte		N11   , Fs4
	.byte	W12
	.byte		        Fn4
	.byte	W12
mus_qz_638_6_003:
	.byte		PAN   , c_v-19
	.byte		N11   , Cs5 , v100
	.byte	W12
	.byte		PAN   , c_v-23
	.byte		N11   , Cn5
	.byte	W12
	.byte		PAN   , c_v-24
	.byte		N11   , Bn4
	.byte	W12
	.byte		PAN   , c_v-26
	.byte		N11   , As4
	.byte	W12
	.byte		PAN   , c_v-28
	.byte		N11   , An4
	.byte	W12
	.byte		        Gs4
	.byte	W12
	.byte		PAN   , c_v-32
	.byte		N11   , Gn4
	.byte	W12
	.byte		        Fs4
	.byte	W12
	.byte	PEND
mus_qz_638_6_004:
	.byte		PAN   , c_v-19
	.byte		N11   , Cn5 , v100
	.byte	W12
	.byte		PAN   , c_v-23
	.byte		N11   , Bn4
	.byte	W12
	.byte		PAN   , c_v-24
	.byte		N11   , As4
	.byte	W12
	.byte		PAN   , c_v-26
	.byte		N11   , An4
	.byte	W12
	.byte		PAN   , c_v-28
	.byte		N11   , Gs4
	.byte	W12
	.byte		        Gn4
	.byte	W12
	.byte		PAN   , c_v-32
	.byte		N11   , Fs4
	.byte	W12
	.byte		        Fn4
	.byte	W12
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_638_6_003
	.byte	PATT
	 ptrvalue	mus_qz_638_6_004
	.byte	PATT
	 ptrvalue	mus_qz_638_6_003
	.byte	PATT
	 ptrvalue	mus_qz_638_6_004
	.byte	PATT
	 ptrvalue	mus_qz_638_6_003
	.byte		VOL   , 33*mus_qz_638_mvl/mxv
	.byte		N11   , Fn4 , v100
	.byte	W12
	.byte		VOL   , 27*mus_qz_638_mvl/mxv
	.byte		N11   , En4
	.byte	W12
	.byte		VOL   , 26*mus_qz_638_mvl/mxv
	.byte		N11   , Ds4
	.byte	W12
	.byte		VOL   , 20*mus_qz_638_mvl/mxv
	.byte		N11   , Dn4
	.byte	W12
	.byte		VOL   , 17*mus_qz_638_mvl/mxv
	.byte		N11   , Cs4
	.byte	W12
	.byte		VOL   , 13*mus_qz_638_mvl/mxv
	.byte		N11   , Cn4
	.byte	W12
	.byte		VOL   , 6*mus_qz_638_mvl/mxv
	.byte		N11   , Bn3
	.byte	W12
	.byte		VOL   , 2*mus_qz_638_mvl/mxv
	.byte		N11   , As3
	.byte	W12
	.byte		PAN   , c_v+29
	.byte		VOL   , 0*mus_qz_638_mvl/mxv
	.byte	W18
	.byte		        51*mus_qz_638_mvl/mxv
	.byte	W78
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
	 ptrvalue	mus_qz_638_6_B1
	.byte	FINE

/********************** Track  7 **********************/

mus_qz_638_7:
	.byte		KEYSH , mus_qz_638_key+0
	.byte		PAN   , c_v+0
	.byte		LFOS  , 21
	.byte		VOL   , 29*mus_qz_638_mvl/mxv
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
mus_qz_638_7_B1:
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
	.byte	W72
	.byte		VOICE , 12
	.byte	W24
	.byte		MOD   , 2
	.byte		PAN   , c_v-16
	.byte		VOL   , 35*mus_qz_638_mvl/mxv
	.byte		N92   , Ds2 , v100 , gtp3
	.byte	W48
	.byte		VOL   , 40*mus_qz_638_mvl/mxv
	.byte	W12
	.byte		        41*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        45*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        48*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        51*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        54*mus_qz_638_mvl/mxv
	.byte	W12
	.byte		        35*mus_qz_638_mvl/mxv
	.byte		N92   , Dn2 , v100 , gtp3
	.byte	W48
	.byte		VOL   , 40*mus_qz_638_mvl/mxv
	.byte	W12
	.byte		        41*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        45*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        48*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        51*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        54*mus_qz_638_mvl/mxv
	.byte	W12
	.byte		        35*mus_qz_638_mvl/mxv
	.byte		N92   , Fn2 , v100 , gtp3
	.byte	W48
	.byte		VOL   , 40*mus_qz_638_mvl/mxv
	.byte	W12
	.byte		        41*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        45*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        48*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        51*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        54*mus_qz_638_mvl/mxv
	.byte	W12
	.byte		        35*mus_qz_638_mvl/mxv
	.byte		N92   , Ds2 , v100 , gtp3
	.byte	W48
	.byte		VOL   , 40*mus_qz_638_mvl/mxv
	.byte	W12
	.byte		        41*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        45*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        48*mus_qz_638_mvl/mxv
	.byte	W05
	.byte		        51*mus_qz_638_mvl/mxv
	.byte	W07
	.byte		        54*mus_qz_638_mvl/mxv
	.byte	W12
	.byte	W12
	.byte		MOD   , 0
	.byte	W84
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		VOL   , 28*mus_qz_638_mvl/mxv
	.byte	W40
	.byte		PAN   , c_v+32
	.byte	W08
	.byte		N07   , Cs6
	.byte	W08
	.byte		        Cs6 , v072
	.byte	W08
	.byte		        Cs6 , v044
	.byte	W08
	.byte		PAN   , c_v-25
	.byte		N07   , Cs6 , v068
	.byte	W08
	.byte		        Cs6 , v044
	.byte	W08
	.byte		N23   , Cs6 , v016
	.byte	W08
	.byte	W40
	.byte		PAN   , c_v+32
	.byte	W32
	.byte		        c_v-25
	.byte	W24
	.byte	W96
	.byte	W96
	.byte	W40
	.byte		        c_v+32
	.byte	W08
	.byte		N07   , Cs6 , v100
	.byte	W08
	.byte		        Cs6 , v072
	.byte	W08
	.byte		        Cs6 , v044
	.byte	W08
	.byte		PAN   , c_v-25
	.byte		N07   , Cs6 , v068
	.byte	W08
	.byte		        Cs6 , v044
	.byte	W08
	.byte		N23   , Cs6 , v016
	.byte	W08
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_638_7_B1
	.byte	FINE

/********************** Track  8 **********************/

mus_qz_638_8:
	.byte		KEYSH , mus_qz_638_key+0
	.byte		VOICE , 12
	.byte		PAN   , c_v+0
	.byte		VOL   , 72*mus_qz_638_mvl/mxv
	.byte		PAN   , c_v-23
	.byte		N23   , Cn1 , v127
	.byte	W24
	.byte		N11   , Gn1
	.byte	W12
	.byte		N23   , Ds1
	.byte	W24
	.byte		N11   , Gn1
	.byte	W12
	.byte		N23   , Fn1
	.byte	W24
mus_qz_638_8_000:
	.byte		N11   , Ds1 , v127
	.byte	W24
	.byte		N23   , As0
	.byte	W24
	.byte	N11
	.byte	W12
	.byte		        Cn1
	.byte	W12
	.byte		N23   , Cs1
	.byte	W24
	.byte	PEND
mus_qz_638_8_001:
	.byte		N23   , Cn1 , v127
	.byte	W24
	.byte		N11   , Gn1
	.byte	W12
	.byte		N23   , Ds1
	.byte	W24
	.byte		N11   , Gn1
	.byte	W12
	.byte		N23   , Ds1
	.byte	W24
	.byte	PEND
mus_qz_638_8_002:
	.byte		N11   , Fn1 , v127
	.byte	W24
	.byte		N23   , As0
	.byte	W24
	.byte		N11   , Cs1
	.byte	W12
	.byte		        Cn1
	.byte	W12
	.byte		N23   , Cs1
	.byte	W24
	.byte	PEND
mus_qz_638_8_003:
	.byte		N23   , Cn1 , v127
	.byte	W24
	.byte		N11   , Gn1
	.byte	W12
	.byte		N23   , Ds1
	.byte	W24
	.byte		N11   , Gn1
	.byte	W12
	.byte		N23   , Fn1
	.byte	W24
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_638_8_000
	.byte	PATT
	 ptrvalue	mus_qz_638_8_001
	.byte	PATT
	 ptrvalue	mus_qz_638_8_002
	.byte	PATT
	 ptrvalue	mus_qz_638_8_003
	.byte	PATT
	 ptrvalue	mus_qz_638_8_000
mus_qz_638_8_B1:
	.byte	PATT
	 ptrvalue	mus_qz_638_8_001
	.byte	PATT
	 ptrvalue	mus_qz_638_8_002
	.byte	PATT
	 ptrvalue	mus_qz_638_8_003
	.byte	PATT
	 ptrvalue	mus_qz_638_8_000
	.byte	PATT
	 ptrvalue	mus_qz_638_8_001
	.byte	PATT
	 ptrvalue	mus_qz_638_8_002
	.byte	PATT
	 ptrvalue	mus_qz_638_8_003
	.byte	PATT
	 ptrvalue	mus_qz_638_8_000
	.byte	PATT
	 ptrvalue	mus_qz_638_8_001
	.byte	PATT
	 ptrvalue	mus_qz_638_8_002
mus_qz_638_8_004:
	.byte		N05   , Fn0 , v127
	.byte	W12
	.byte		        Cn1
	.byte	W12
	.byte		        Ds1
	.byte	W12
	.byte		        Gs1
	.byte	W12
	.byte		        Fn1
	.byte	W12
	.byte		        Gs1
	.byte	W12
	.byte		        Ds1
	.byte	W12
	.byte		        Cn1
	.byte	W12
	.byte	PEND
mus_qz_638_8_005:
	.byte		N05   , Fn1 , v127
	.byte	W12
	.byte		        As0
	.byte	W12
	.byte		        Cs1
	.byte	W12
	.byte		        Cn1
	.byte	W12
	.byte		        Ds1
	.byte	W12
	.byte		        As0
	.byte	W12
	.byte		        Cn1
	.byte	W12
	.byte		        Fn1
	.byte	W12
	.byte	PEND
	.byte		        Cn1
	.byte	W12
	.byte		        Gn1
	.byte	W12
	.byte		        Ds1
	.byte	W12
	.byte		        Cn1
	.byte	W12
	.byte		        As0
	.byte	W12
	.byte		        Fn0
	.byte	W12
	.byte		        Cn1
	.byte	W12
	.byte		        Ds1
	.byte	W12
	.byte		        Fn1
	.byte	W12
	.byte		        As0
	.byte	W12
	.byte		        Cn1
	.byte	W12
	.byte		        Fn0
	.byte	W12
	.byte		        As0
	.byte	W12
	.byte		        Cn1
	.byte	W12
	.byte		        Ds1
	.byte	W12
	.byte		        Fn1
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_638_8_004
	.byte	PATT
	 ptrvalue	mus_qz_638_8_005
	.byte	W96
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_638_8_003
	.byte	PATT
	 ptrvalue	mus_qz_638_8_000
	.byte	PATT
	 ptrvalue	mus_qz_638_8_001
	.byte	PATT
	 ptrvalue	mus_qz_638_8_002
	.byte	PATT
	 ptrvalue	mus_qz_638_8_003
	.byte	PATT
	 ptrvalue	mus_qz_638_8_000
	.byte	PATT
	 ptrvalue	mus_qz_638_8_001
	.byte	PATT
	 ptrvalue	mus_qz_638_8_002
	.byte		VOICE , 12
	.byte		N05   , Cn1 , v127
	.byte	W24
	.byte		        Gn1
	.byte	W12
	.byte		        Ds1
	.byte	W24
	.byte		        Gn1
	.byte	W12
	.byte		        Fn1
	.byte	W24
mus_qz_638_8_006:
	.byte		N05   , Ds1 , v127
	.byte	W24
	.byte		        As0
	.byte	W72
	.byte	PEND
	.byte		VOICE , 20
	.byte		N05   , Cn1
	.byte	W24
	.byte		        Gn1
	.byte	W12
	.byte		        Ds1
	.byte	W24
	.byte		        Gn1
	.byte	W12
	.byte		        Ds1
	.byte	W24
	.byte		        Fn1
	.byte	W24
	.byte		        As0
	.byte	W72
	.byte		VOICE , 19
	.byte		VOL   , 61*mus_qz_638_mvl/mxv
	.byte		N05   , Cn1
	.byte	W24
	.byte		        Gn1
	.byte	W12
	.byte		        Ds1
	.byte	W24
	.byte		        Gn1
	.byte	W12
	.byte		        Fn1
	.byte	W24
	.byte	PATT
	 ptrvalue	mus_qz_638_8_006
	.byte		VOICE , 20
	.byte		VOL   , 72*mus_qz_638_mvl/mxv
	.byte		N05   , Cn1 , v127
	.byte	W24
	.byte		        Gn1
	.byte	W12
	.byte		        Ds1
	.byte	W24
	.byte		        Gn1
	.byte	W12
	.byte		        Ds1
	.byte	W24
	.byte		        Fn1
	.byte	W24
	.byte		        As0
	.byte	W24
	.byte		VOICE , 12
	.byte		N05   , Cs1
	.byte	W12
	.byte		        Cn1
	.byte	W12
	.byte		N23   , Cs1
	.byte	W24
	.byte	PATT
	 ptrvalue	mus_qz_638_8_003
	.byte	PATT
	 ptrvalue	mus_qz_638_8_000
	.byte	PATT
	 ptrvalue	mus_qz_638_8_001
	.byte	PATT
	 ptrvalue	mus_qz_638_8_002
	.byte	PATT
	 ptrvalue	mus_qz_638_8_003
	.byte	PATT
	 ptrvalue	mus_qz_638_8_000
	.byte	PATT
	 ptrvalue	mus_qz_638_8_001
	.byte	PATT
	 ptrvalue	mus_qz_638_8_002
	.byte	PATT
	 ptrvalue	mus_qz_638_8_003
	.byte	PATT
	 ptrvalue	mus_qz_638_8_000
	.byte	GOTO
	 ptrvalue	mus_qz_638_8_B1
	.byte	FINE

/********************** Track  9 **********************/

mus_qz_638_9:
	.byte		KEYSH , mus_qz_638_key+0
	.byte		VOICE , 39
	.byte		VOL   , 53*mus_qz_638_mvl/mxv
	.byte		N23   , Cn1 , v127
	.byte	W48
	.byte	N23
	.byte	W48
mus_qz_638_9_000:
	.byte		N23   , Cn1 , v127
	.byte	W24
	.byte		        Bn0 , v072
	.byte	W24
	.byte		N02   , Fs1 , v052
	.byte	W12
	.byte	N02
	.byte	W12
	.byte		N23   , As1 , v044
	.byte	W24
	.byte	PEND
mus_qz_638_9_001:
	.byte		N23   , Cn1 , v127
	.byte	W48
	.byte	N23
	.byte	W48
	.byte	PEND
mus_qz_638_9_002:
	.byte		N23   , Cn1 , v127
	.byte	W24
	.byte		        Bn0 , v072
	.byte	W24
	.byte		N02   , Fs1 , v052
	.byte	W12
	.byte	N02
	.byte	W12
	.byte		N11   , Dn1 , v076
	.byte	W12
	.byte	N11
	.byte	W12
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_638_9_001
	.byte	PATT
	 ptrvalue	mus_qz_638_9_000
	.byte	PATT
	 ptrvalue	mus_qz_638_9_001
	.byte	PATT
	 ptrvalue	mus_qz_638_9_002
	.byte	PATT
	 ptrvalue	mus_qz_638_9_001
	.byte	PATT
	 ptrvalue	mus_qz_638_9_000
mus_qz_638_9_B1:
	.byte	PATT
	 ptrvalue	mus_qz_638_9_001
	.byte	PATT
	 ptrvalue	mus_qz_638_9_002
	.byte	PATT
	 ptrvalue	mus_qz_638_9_001
	.byte	PATT
	 ptrvalue	mus_qz_638_9_000
	.byte	PATT
	 ptrvalue	mus_qz_638_9_001
	.byte	PATT
	 ptrvalue	mus_qz_638_9_002
	.byte	PATT
	 ptrvalue	mus_qz_638_9_001
	.byte	PATT
	 ptrvalue	mus_qz_638_9_000
	.byte	PATT
	 ptrvalue	mus_qz_638_9_001
	.byte		N23   , Cn1 , v127
	.byte	W24
	.byte		        Bn0 , v072
	.byte	W24
	.byte		N44   , Gn0 , v100 , gtp3
	.byte		N02   , Fs1 , v052
	.byte	W12
	.byte	N02
	.byte	W12
	.byte		N05   , Dn1 , v076
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		N11   , Bn0 , v100
	.byte	W12
mus_qz_638_9_003:
	.byte		N05   , Cn1 , v100
	.byte	W12
	.byte	N05
	.byte	W12
	.byte		        Dn1
	.byte	W12
	.byte	N05
	.byte	W12
	.byte		        Cn1
	.byte	W12
	.byte	N05
	.byte	W12
	.byte		        Dn1
	.byte	W12
	.byte	N05
	.byte	W12
	.byte	PEND
mus_qz_638_9_004:
	.byte		N05   , Cn1 , v100
	.byte	W12
	.byte	N05
	.byte	W12
	.byte		        Dn1
	.byte	W12
	.byte		        Cn1
	.byte	W12
	.byte	N05
	.byte	W12
	.byte	N05
	.byte	W12
	.byte		        Dn1
	.byte	W12
	.byte	N05
	.byte	W12
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_638_9_003
	.byte		N05   , Cn1 , v100
	.byte	W12
	.byte	N05
	.byte	W12
	.byte		        Dn1
	.byte	W12
	.byte		        Cn1
	.byte	W12
	.byte		N02   , Fs1 , v052
	.byte	W12
	.byte	N02
	.byte	W12
	.byte		N05   , Dn1 , v076
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		N11   , Bn0 , v100
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_638_9_003
	.byte	PATT
	 ptrvalue	mus_qz_638_9_004
	.byte		N92   , En2 , v100 , gtp3
	.byte	W96
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_638_9_001
	.byte	PATT
	 ptrvalue	mus_qz_638_9_000
	.byte	PATT
	 ptrvalue	mus_qz_638_9_001
	.byte	PATT
	 ptrvalue	mus_qz_638_9_002
	.byte	PATT
	 ptrvalue	mus_qz_638_9_001
	.byte	PATT
	 ptrvalue	mus_qz_638_9_000
	.byte	PATT
	 ptrvalue	mus_qz_638_9_001
	.byte		N23   , Cn1 , v127
	.byte	W24
	.byte		        Bn0 , v072
	.byte	W24
	.byte		N02   , Fs1 , v052
	.byte	W12
	.byte	N02
	.byte	W12
	.byte		N05   , Dn1 , v076
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		N11   , Dn1 , v100
	.byte	W12
mus_qz_638_9_005:
	.byte		N23   , Bn0 , v100
	.byte		N44   , As1 , v100 , gtp3
	.byte	W24
	.byte		N23   , Bn0
	.byte	W24
	.byte		N11   , En1
	.byte		N02   , Fs1 , v052
	.byte	W12
	.byte		N23   , Cn1 , v100
	.byte	W24
	.byte	N23
	.byte	W12
	.byte	PEND
	.byte		N02   , Fs1 , v052
	.byte	W12
	.byte		N11   , Cn1 , v100
	.byte	W12
	.byte		N05   , En1
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		N11   , Cn1
	.byte	W12
	.byte		        En1
	.byte		N02   , Fs1 , v052
	.byte	W12
	.byte		N23   , En1 , v100
	.byte	W24
	.byte		N11   , Bn0
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_638_9_005
	.byte		N02   , Fs1 , v052
	.byte	W12
	.byte		N11   , Cn1 , v100
	.byte	W12
	.byte		N05   , En1
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		N11   , Cn1
	.byte	W12
	.byte		        En1
	.byte		N02   , Fs1 , v052
	.byte	W12
	.byte		N23   , Bn0 , v100
	.byte	W24
	.byte	N11
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_638_9_005
	.byte		N02   , Fs1 , v052
	.byte	W12
	.byte		N11   , Cn1 , v100
	.byte	W12
	.byte		        En1
	.byte	W12
	.byte		        Cn1
	.byte	W12
	.byte		        En1
	.byte		N02   , Fs1 , v052
	.byte	W12
	.byte		N23   , En1 , v100
	.byte	W24
	.byte		N11   , Bn0
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_638_9_005
	.byte		N02   , Fs1 , v052
	.byte	W12
	.byte		N11   , Cn1 , v100
	.byte	W12
	.byte		N05   , En1
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		N11   , Cn1
	.byte	W12
	.byte		N15   , En1
	.byte		N02   , Fs1 , v052
	.byte	W16
	.byte		N15   , En1 , v100
	.byte	W16
	.byte		        Cn1
	.byte		N15   , En1
	.byte	W16
	.byte		N23   , Bn0 , v127
	.byte	W48
	.byte		        Cn1
	.byte	W48
	.byte	PATT
	 ptrvalue	mus_qz_638_9_000
	.byte	PATT
	 ptrvalue	mus_qz_638_9_001
	.byte	PATT
	 ptrvalue	mus_qz_638_9_002
	.byte	PATT
	 ptrvalue	mus_qz_638_9_001
	.byte	PATT
	 ptrvalue	mus_qz_638_9_000
	.byte	PATT
	 ptrvalue	mus_qz_638_9_001
	.byte	PATT
	 ptrvalue	mus_qz_638_9_002
	.byte	PATT
	 ptrvalue	mus_qz_638_9_001
	.byte	PATT
	 ptrvalue	mus_qz_638_9_000
	.byte	GOTO
	 ptrvalue	mus_qz_638_9_B1
	.byte	FINE

/******************************************************/
	.p2align	2

mus_qz_638:
	.byte	9	/* NumTrks */
	.byte	0	/* NumBlks */
	.byte	mus_qz_638_pri	/* Priority */
	.byte	mus_qz_638_rev	/* Reverb. */
space64 4
	ptrvalue	mus_qz_638_grp

	ptrvalue	mus_qz_638_1
	ptrvalue	mus_qz_638_2
	ptrvalue	mus_qz_638_3
	ptrvalue	mus_qz_638_4
	ptrvalue	mus_qz_638_5
	ptrvalue	mus_qz_638_6
	ptrvalue	mus_qz_638_7
	ptrvalue	mus_qz_638_8
	ptrvalue	mus_qz_638_9

	.end
