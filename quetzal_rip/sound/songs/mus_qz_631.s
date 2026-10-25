	.include "asm/macros/bit_width.inc"

	.include "MPlayDef.s"

/* Moemon Quetzal song 631, disassembled from the ROM by tools/quetzal_rip/songs.py.
   "org" comments give the ROM address of the bytes that follow. */

	.equ	mus_qz_631_grp, voicegroup_qz_songs
	.equ	mus_qz_631_pri, 0
	.equ	mus_qz_631_rev, reverb_set+0
	.equ	mus_qz_631_mvl, 127
	.equ	mus_qz_631_key, 0
	.equ	mus_qz_631_tbs, 1
	.equ	mus_qz_631_exg, 1
	.equ	mus_qz_631_cmp, 1

	.section .rodata
	.global	mus_qz_631
	.p2align	2
/* org 0x09683E54 */

/********************** Track  1 **********************/

mus_qz_631_1:
	.byte		KEYSH , mus_qz_631_key+0
mus_qz_631_1_B1:
	.byte		TEMPO , 108*mus_qz_631_tbs/2
	.byte		VOICE , 3
	.byte		BENDR , 6
	.byte		PAN   , c_v-32
	.byte		VOL   , 50*mus_qz_631_mvl/mxv
	.byte		        50*mus_qz_631_mvl/mxv
	.byte		PAN   , c_v-32
	.byte		VOL   , 45*mus_qz_631_mvl/mxv
	.byte		BEND  , c_v+0
	.byte		        c_v+0
	.byte		N05   , Gs2 , v104
	.byte	W06
	.byte		        Gs2 , v052
	.byte	W06
	.byte		        Cs3 , v104
	.byte	W06
	.byte		        Cs3 , v048
	.byte	W06
	.byte		PAN   , c_v-32
	.byte		N03   , En3 , v116
	.byte	W06
	.byte		PAN   , c_v+48
	.byte		N03   , En3 , v076
	.byte	W06
	.byte		PAN   , c_v-32
	.byte		N03   , En3 , v064
	.byte	W06
	.byte		PAN   , c_v+48
	.byte		N03   , En3 , v052
	.byte	W06
	.byte		PAN   , c_v-32
	.byte		VOL   , 40*mus_qz_631_mvl/mxv
	.byte		N03   , En3 , v044
	.byte	W06
	.byte		PAN   , c_v+48
	.byte		N03   , En3 , v032
	.byte	W06
	.byte		PAN   , c_v-32
	.byte		N03   , En3 , v024
	.byte	W06
	.byte		PAN   , c_v+48
	.byte	N03
	.byte	W06
	.byte		PAN   , c_v-32
	.byte		VOL   , 30*mus_qz_631_mvl/mxv
	.byte		N03   , En3 , v012
	.byte	W06
	.byte		PAN   , c_v+48
	.byte		N03   , En3 , v008
	.byte	W06
	.byte		PAN   , c_v-32
	.byte		N03   , En3 , v004
	.byte	W06
	.byte		PAN   , c_v+48
	.byte	N03
	.byte	W06
mus_qz_631_1_000:
	.byte		PAN   , c_v-32
	.byte		VOL   , 45*mus_qz_631_mvl/mxv
	.byte		N05   , Gs2 , v104
	.byte	W06
	.byte		        Gs2 , v052
	.byte	W06
	.byte		        Cs3 , v104
	.byte	W06
	.byte		        Cs3 , v048
	.byte	W06
	.byte		PAN   , c_v-32
	.byte		N03   , En3 , v116
	.byte	W06
	.byte		PAN   , c_v+48
	.byte		N03   , En3 , v076
	.byte	W06
	.byte		PAN   , c_v-32
	.byte		N03   , En3 , v064
	.byte	W06
	.byte		PAN   , c_v+48
	.byte		N03   , En3 , v052
	.byte	W06
	.byte		PAN   , c_v-32
	.byte		VOL   , 40*mus_qz_631_mvl/mxv
	.byte		N03   , En3 , v044
	.byte	W06
	.byte		PAN   , c_v+48
	.byte		N03   , En3 , v032
	.byte	W06
	.byte		PAN   , c_v-32
	.byte		N03   , En3 , v024
	.byte	W06
	.byte		PAN   , c_v+48
	.byte	N03
	.byte	W06
	.byte		PAN   , c_v-32
	.byte		VOL   , 30*mus_qz_631_mvl/mxv
	.byte		N03   , En3 , v012
	.byte	W06
	.byte		PAN   , c_v+48
	.byte		N03   , En3 , v008
	.byte	W06
	.byte		PAN   , c_v-32
	.byte		N03   , En3 , v004
	.byte	W06
	.byte		PAN   , c_v+48
	.byte	N03
	.byte	W06
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_631_1_000
	.byte		PAN   , c_v-32
	.byte		VOL   , 46*mus_qz_631_mvl/mxv
	.byte		N05   , Gs2 , v104
	.byte	W06
	.byte		        Gs2 , v052
	.byte	W06
	.byte		        Cs3 , v104
	.byte	W06
	.byte		        Cs3 , v048
	.byte	W06
	.byte		PAN   , c_v-32
	.byte		N03   , En3 , v116
	.byte	W06
	.byte		PAN   , c_v+48
	.byte		N03   , En3 , v076
	.byte	W06
	.byte		PAN   , c_v-32
	.byte		N03   , En3 , v064
	.byte	W06
	.byte		PAN   , c_v+48
	.byte		N03   , En3 , v052
	.byte	W06
	.byte		PAN   , c_v-32
	.byte		VOL   , 40*mus_qz_631_mvl/mxv
	.byte		N03   , En3 , v044
	.byte	W06
	.byte		PAN   , c_v+48
	.byte		N03   , En3 , v032
	.byte	W06
	.byte		PAN   , c_v-32
	.byte		N03   , En3 , v024
	.byte	W06
	.byte		PAN   , c_v+48
	.byte	N03
	.byte	W06
	.byte		PAN   , c_v-32
	.byte		VOL   , 30*mus_qz_631_mvl/mxv
	.byte		N03   , En3 , v012
	.byte	W06
	.byte		PAN   , c_v+48
	.byte		N03   , En3 , v008
	.byte	W06
	.byte		PAN   , c_v-32
	.byte		N03   , En3 , v004
	.byte	W06
	.byte		PAN   , c_v+48
	.byte	N03
	.byte	W06
mus_qz_631_1_001:
	.byte		PAN   , c_v-32
	.byte		VOL   , 45*mus_qz_631_mvl/mxv
	.byte		N05   , Gs2 , v104
	.byte	W06
	.byte		        Gs2 , v052
	.byte	W06
	.byte		        Cs3 , v104
	.byte	W06
	.byte		        Cs3 , v048
	.byte	W06
	.byte		PAN   , c_v-32
	.byte		N03   , En3 , v116
	.byte	W06
	.byte		PAN   , c_v+48
	.byte		N03   , En3 , v076
	.byte	W06
	.byte		PAN   , c_v-32
	.byte		N03   , En3 , v064
	.byte	W06
	.byte		PAN   , c_v+48
	.byte		N03   , En3 , v052
	.byte	W06
	.byte		PAN   , c_v-32
	.byte		VOL   , 40*mus_qz_631_mvl/mxv
	.byte		N05   , Cs3 , v104
	.byte	W06
	.byte		        Cs3 , v052
	.byte	W06
	.byte		        En3 , v104
	.byte	W06
	.byte		        En3 , v048
	.byte	W06
	.byte		PAN   , c_v-32
	.byte		VOL   , 30*mus_qz_631_mvl/mxv
	.byte		N03   , Fn3 , v116
	.byte	W06
	.byte		PAN   , c_v+48
	.byte		N03   , Fn3 , v076
	.byte	W06
	.byte		PAN   , c_v-32
	.byte		N03   , Fn3 , v064
	.byte	W06
	.byte		PAN   , c_v+48
	.byte		N03   , Fn3 , v052
	.byte	W06
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_631_1_001
	.byte	PATT
	 ptrvalue	mus_qz_631_1_001
	.byte	PATT
	 ptrvalue	mus_qz_631_1_001
	.byte	PATT
	 ptrvalue	mus_qz_631_1_001
	.byte	PATT
	 ptrvalue	mus_qz_631_1_001
	.byte	PATT
	 ptrvalue	mus_qz_631_1_001
	.byte	PATT
	 ptrvalue	mus_qz_631_1_001
mus_qz_631_1_002:
	.byte		PAN   , c_v-32
	.byte		VOL   , 45*mus_qz_631_mvl/mxv
	.byte		N05   , An2 , v104
	.byte	W06
	.byte		        An2 , v052
	.byte	W06
	.byte		        Dn3 , v104
	.byte	W06
	.byte		        Dn3 , v048
	.byte	W06
	.byte		PAN   , c_v-32
	.byte		N03   , Fn3 , v116
	.byte	W06
	.byte		PAN   , c_v+48
	.byte		N03   , Fn3 , v076
	.byte	W06
	.byte		PAN   , c_v-32
	.byte		N03   , Fn3 , v064
	.byte	W06
	.byte		PAN   , c_v+48
	.byte		N03   , Fn3 , v052
	.byte	W06
	.byte		PAN   , c_v-32
	.byte		VOL   , 40*mus_qz_631_mvl/mxv
	.byte		N05   , Dn3 , v104
	.byte	W06
	.byte		        Dn3 , v052
	.byte	W06
	.byte		        Fn3 , v104
	.byte	W06
	.byte		        Fn3 , v048
	.byte	W06
	.byte		PAN   , c_v-32
	.byte		VOL   , 30*mus_qz_631_mvl/mxv
	.byte		N03   , Fs3 , v116
	.byte	W06
	.byte		PAN   , c_v+48
	.byte		N03   , Fs3 , v076
	.byte	W06
	.byte		PAN   , c_v-32
	.byte		N03   , Fs3 , v064
	.byte	W06
	.byte		PAN   , c_v+48
	.byte		N03   , Fs3 , v052
	.byte	W06
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_631_1_002
	.byte	PATT
	 ptrvalue	mus_qz_631_1_002
	.byte		PAN   , c_v-32
	.byte		VOL   , 45*mus_qz_631_mvl/mxv
	.byte		N05   , An2 , v104
	.byte	W06
	.byte		        An2 , v052
	.byte	W06
	.byte		        Dn3 , v104
	.byte	W06
	.byte		        Dn3 , v048
	.byte	W06
	.byte		PAN   , c_v-32
	.byte		N03   , Fn3 , v116
	.byte	W06
	.byte		PAN   , c_v+48
	.byte		N03   , Fn3 , v076
	.byte	W06
	.byte		PAN   , c_v-32
	.byte		N03   , Fn3 , v064
	.byte	W06
	.byte		PAN   , c_v+48
	.byte		N03   , Fn3 , v052
	.byte	W06
	.byte		PAN   , c_v-32
	.byte		VOL   , 40*mus_qz_631_mvl/mxv
	.byte		N05   , Dn3 , v104
	.byte	W06
	.byte		        Dn3 , v052
	.byte	W06
	.byte		        Fn3 , v104
	.byte	W06
	.byte		        Fn3 , v048
	.byte	W06
	.byte		PAN   , c_v-32
	.byte		VOL   , 30*mus_qz_631_mvl/mxv
	.byte		N03   , Fs3 , v116
	.byte	W06
	.byte		TEMPO , 102*mus_qz_631_tbs/2
	.byte		TEMPO , 102*mus_qz_631_tbs/2
	.byte		PAN   , c_v+48
	.byte		N03   , Fs3 , v076
	.byte	W06
	.byte		PAN   , c_v-32
	.byte		N03   , Fs3 , v064
	.byte	W06
	.byte		PAN   , c_v+48
	.byte		N03   , Fs3 , v052
	.byte	W06
	.byte		PAN   , c_v-32
	.byte		VOL   , 40*mus_qz_631_mvl/mxv
	.byte		N03   , Fs3 , v044
	.byte	W06
	.byte		TEMPO , 90*mus_qz_631_tbs/2
	.byte		TEMPO , 90*mus_qz_631_tbs/2
	.byte		PAN   , c_v+48
	.byte		N03   , Fs3 , v032
	.byte	W06
	.byte		PAN   , c_v-32
	.byte		N03   , Fs3 , v024
	.byte	W06
	.byte		TEMPO , 62*mus_qz_631_tbs/2
	.byte		TEMPO , 62*mus_qz_631_tbs/2
	.byte		PAN   , c_v+48
	.byte	N03
	.byte	W06
	.byte		VOICE , 23
	.byte		VOL   , 16*mus_qz_631_mvl/mxv
	.byte		PAN   , c_v-32
	.byte		N92   , En3 , v100 , gtp3
	.byte	W03
	.byte		VOL   , 15*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        16*mus_qz_631_mvl/mxv
	.byte	W01
	.byte		TEMPO , 104*mus_qz_631_tbs/2
	.byte		TEMPO , 104*mus_qz_631_tbs/2
	.byte	W02
	.byte		        17*mus_qz_631_mvl/mxv
	.byte	W09
	.byte		        17*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        18*mus_qz_631_mvl/mxv
	.byte	W06
	.byte		        19*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        19*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        19*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        21*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        21*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        22*mus_qz_631_mvl/mxv
	.byte	W06
	.byte		        22*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        23*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        24*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        25*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        26*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        28*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        29*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        30*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        30*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        32*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        33*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        34*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        35*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        36*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        22*mus_qz_631_mvl/mxv
	.byte		N92   , Fn3 , v100 , gtp3
	.byte	W05
	.byte		VOL   , 22*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        23*mus_qz_631_mvl/mxv
	.byte	W06
	.byte		        24*mus_qz_631_mvl/mxv
	.byte	W06
	.byte		        25*mus_qz_631_mvl/mxv
	.byte	W06
	.byte		        26*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        28*mus_qz_631_mvl/mxv
	.byte	W09
	.byte		        29*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        30*mus_qz_631_mvl/mxv
	.byte	W06
	.byte		        32*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        34*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        35*mus_qz_631_mvl/mxv
	.byte	W06
	.byte		        37*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        38*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        40*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        44*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        47*mus_qz_631_mvl/mxv
	.byte	W22
	.byte		        10*mus_qz_631_mvl/mxv
	.byte		N92   , Fs3 , v100 , gtp3
	.byte	W05
	.byte		VOL   , 10*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        12*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        14*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        15*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        17*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        19*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        22*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        27*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        31*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        34*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        40*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        43*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        49*mus_qz_631_mvl/mxv
	.byte	W19
	.byte		        10*mus_qz_631_mvl/mxv
	.byte		N92   , Fn3 , v100 , gtp3
	.byte	W05
	.byte		VOL   , 11*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        12*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        14*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        16*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        19*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        22*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        26*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        30*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        34*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        39*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        46*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        48*mus_qz_631_mvl/mxv
	.byte	W24
mus_qz_631_1_003:
	.byte		VOL   , 5*mus_qz_631_mvl/mxv
	.byte		N44   , Gn3 , v100 , gtp3
	.byte	W05
	.byte		VOL   , 6*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        10*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        14*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        18*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        21*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        24*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        28*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        8*mus_qz_631_mvl/mxv
	.byte		N44   , Fs3 , v100 , gtp3
	.byte	W05
	.byte		VOL   , 11*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        14*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        19*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        22*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        28*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        34*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        37*mus_qz_631_mvl/mxv
	.byte	W07
	.byte	PEND
	.byte		        18*mus_qz_631_mvl/mxv
	.byte		N44   , Fn3 , v100 , gtp3
	.byte	W05
	.byte		VOL   , 22*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        27*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        31*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        36*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        40*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        44*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        50*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        30*mus_qz_631_mvl/mxv
	.byte		N44   , En3 , v116 , gtp3
	.byte	W05
	.byte		VOL   , 35*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        40*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        46*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        50*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        50*mus_qz_631_mvl/mxv
	.byte	W19
	.byte	PATT
	 ptrvalue	mus_qz_631_1_003
	.byte		VOL   , 18*mus_qz_631_mvl/mxv
	.byte		N44   , Cn4 , v100 , gtp3
	.byte	W05
	.byte		VOL   , 22*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        27*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        31*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        36*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        40*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        44*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        50*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        30*mus_qz_631_mvl/mxv
	.byte		N44   , An3 , v127 , gtp3
	.byte	W05
	.byte		VOL   , 35*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        40*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        46*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        50*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        50*mus_qz_631_mvl/mxv
	.byte	W19
	.byte		VOICE , 3
	.byte		VOL   , 37*mus_qz_631_mvl/mxv
	.byte		BEND  , c_v+1
	.byte		N11   , Gs2 , v100
	.byte	W12
	.byte		        Cs3
	.byte	W12
	.byte		N68   , En3 , v100 , gtp3
	.byte	W72
	.byte		BEND  , c_v+3
	.byte		N11   , Gs2
	.byte	W12
	.byte		        Cs3
	.byte	W12
	.byte		BEND  , c_v+4
	.byte		N68   , En3 , v100 , gtp3
	.byte	W72
	.byte		N11   , Gs2
	.byte	W12
	.byte		        Cs3
	.byte	W12
	.byte		BEND  , c_v+6
	.byte		N68   , En3 , v100 , gtp3
	.byte	W72
	.byte		BEND  , c_v+1
	.byte		N11   , Gs2
	.byte	W12
	.byte		BEND  , c_v+6
	.byte		N11   , Cs3
	.byte	W12
	.byte		BEND  , c_v+11
	.byte		N68   , En3 , v100 , gtp3
	.byte	W72
	.byte	GOTO
	 ptrvalue	mus_qz_631_1_B1
	.byte	FINE

/********************** Track  2 **********************/

mus_qz_631_2:
	.byte		KEYSH , mus_qz_631_key+0
mus_qz_631_2_B1:
	.byte		VOICE , 3
	.byte		VOL   , 62*mus_qz_631_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		VOL   , 62*mus_qz_631_mvl/mxv
	.byte		        57*mus_qz_631_mvl/mxv
	.byte		N23   , Cn4 , v100
	.byte	W24
	.byte		        As3
	.byte	W24
	.byte		N11   , Cs4
	.byte	W12
	.byte		        Ds4
	.byte	W12
	.byte		        Cn4
	.byte	W12
	.byte		        As3
	.byte	W12
mus_qz_631_2_000:
	.byte		N23   , Cn4 , v100
	.byte	W24
	.byte		        As3
	.byte	W24
	.byte		N44   , Cs4 , v100 , gtp3
	.byte	W48
	.byte	PEND
mus_qz_631_2_001:
	.byte		N23   , As3 , v100
	.byte	W24
	.byte		        Cs4
	.byte	W24
	.byte		N11   , Fn4
	.byte	W12
	.byte		        En4
	.byte	W12
	.byte		        Cn4
	.byte	W12
	.byte		        As3
	.byte	W12
	.byte	PEND
mus_qz_631_2_002:
	.byte		N44   , Cn4 , v100 , gtp3
	.byte	W48
	.byte		N23   , Cs4
	.byte	W24
	.byte		N44   , Cn4 , v100 , gtp3
	.byte	W24
	.byte	PEND
mus_qz_631_2_003:
	.byte	W24
	.byte		N23   , As3 , v100
	.byte	W24
	.byte		N11   , Cs4
	.byte	W12
	.byte		        Ds4
	.byte	W12
	.byte		        Cn4
	.byte	W12
	.byte		        As3
	.byte	W12
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_631_2_000
	.byte	PATT
	 ptrvalue	mus_qz_631_2_001
	.byte	PATT
	 ptrvalue	mus_qz_631_2_002
	.byte	PATT
	 ptrvalue	mus_qz_631_2_003
	.byte	PATT
	 ptrvalue	mus_qz_631_2_000
	.byte	PATT
	 ptrvalue	mus_qz_631_2_001
	.byte		N44   , Cn4 , v100 , gtp3
	.byte	W48
	.byte		N23   , Cs4
	.byte	W24
	.byte		        Cn4
	.byte	W24
	.byte		        Cs4
	.byte	W24
	.byte		        Bn3
	.byte	W24
	.byte		N11   , Dn4
	.byte	W12
	.byte		        En4
	.byte	W12
	.byte		        Cs4
	.byte	W12
	.byte		        Bn3
	.byte	W12
	.byte		N23   , Cs4
	.byte	W24
	.byte		        Bn3
	.byte	W24
	.byte		N44   , Dn4 , v100 , gtp3
	.byte	W48
	.byte		N23   , Bn3
	.byte	W24
	.byte		        Dn4
	.byte	W24
	.byte		N11   , Fs4
	.byte	W12
	.byte		        Fn4
	.byte	W12
	.byte		        Cs4
	.byte	W12
	.byte		        Bn3
	.byte	W12
	.byte		N44   , Cs4 , v100 , gtp3
	.byte	W48
	.byte		N23   , Dn4
	.byte	W24
	.byte		N44   , Cs4 , v100 , gtp3
	.byte	W24
	.byte	W24
	.byte		VOICE , 23
	.byte		VOL   , 3*mus_qz_631_mvl/mxv
	.byte		N92   , Cn4 , v100 , gtp3
	.byte	W02
	.byte		VOL   , 4*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        5*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        6*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        7*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        10*mus_qz_631_mvl/mxv
	.byte	W06
	.byte		        10*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        11*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        14*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        14*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        16*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        19*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        19*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        22*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        24*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        24*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        27*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        28*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        30*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        32*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        34*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        36*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        38*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        40*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        41*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        44*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        45*mus_qz_631_mvl/mxv
	.byte	W06
	.byte		        47*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        48*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        6*mus_qz_631_mvl/mxv
	.byte		N92   , Cs4 , v100 , gtp3
	.byte	W02
	.byte		VOL   , 8*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        10*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        10*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        11*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        12*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        14*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        16*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        16*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        19*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        22*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        22*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        25*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        26*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        28*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        29*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        32*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        34*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        34*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        37*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        37*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        40*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        40*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        40*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        44*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        45*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        48*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        51*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        53*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        55*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        11*mus_qz_631_mvl/mxv
	.byte		N92   , Dn4 , v100 , gtp3
	.byte	W05
	.byte		VOL   , 13*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        14*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        17*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        19*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        23*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        28*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        33*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        37*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        41*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        45*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        51*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        55*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        56*mus_qz_631_mvl/mxv
	.byte	W19
	.byte		        10*mus_qz_631_mvl/mxv
	.byte		N92   , Cs4 , v100 , gtp3
	.byte	W05
	.byte		VOL   , 11*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        14*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        17*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        20*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        24*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        28*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        34*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        37*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        42*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        46*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        52*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        57*mus_qz_631_mvl/mxv
	.byte	W24
	.byte		        27*mus_qz_631_mvl/mxv
	.byte		N92   , En4 , v100 , gtp3
	.byte	W05
	.byte		VOL   , 28*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        28*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        29*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        30*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        31*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        34*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        37*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        40*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        42*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        45*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        51*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        57*mus_qz_631_mvl/mxv
	.byte	W24
	.byte		        20*mus_qz_631_mvl/mxv
	.byte		N92   , Ds4 , v100 , gtp3
	.byte	W05
	.byte		VOL   , 22*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        23*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        26*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        28*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        30*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        33*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        35*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        41*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        43*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        48*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        57*mus_qz_631_mvl/mxv
	.byte	W30
	.byte	W01
	.byte		        16*mus_qz_631_mvl/mxv
	.byte		N92   , As4 , v100 , gtp3
	.byte	W05
	.byte		VOL   , 17*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        20*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        23*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        28*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        34*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        39*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        46*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        52*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        57*mus_qz_631_mvl/mxv
	.byte	W42
	.byte	W01
	.byte		        13*mus_qz_631_mvl/mxv
	.byte		N92   , An4 , v100 , gtp3
	.byte	W05
	.byte		VOL   , 19*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        24*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        30*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        37*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        45*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        56*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        57*mus_qz_631_mvl/mxv
	.byte	W54
	.byte	W01
	.byte		VOICE , 3
	.byte		PAN   , c_v+14
	.byte		N11   , Gs2
	.byte	W12
	.byte		        Cs3
	.byte	W12
	.byte		N68   , En3 , v100 , gtp3
	.byte	W72
mus_qz_631_2_004:
	.byte		N11   , Gs2 , v100
	.byte	W12
	.byte		        Cs3
	.byte	W12
	.byte		N68   , En3 , v100 , gtp3
	.byte	W72
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_631_2_004
	.byte	PATT
	 ptrvalue	mus_qz_631_2_004
	.byte	GOTO
	 ptrvalue	mus_qz_631_2_B1
	.byte	FINE

/********************** Track  3 **********************/

mus_qz_631_3:
	.byte		KEYSH , mus_qz_631_key+0
mus_qz_631_3_B1:
	.byte		VOICE , 3
	.byte		PAN   , c_v+32
	.byte		VOL   , 50*mus_qz_631_mvl/mxv
	.byte		        50*mus_qz_631_mvl/mxv
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		        73*mus_qz_631_mvl/mxv
	.byte		N09   , Cn3 , v100
	.byte	W12
	.byte		        En2
	.byte	W12
	.byte		PAN   , c_v-45
	.byte		N09   , En2 , v032
	.byte	W12
	.byte		PAN   , c_v+32
	.byte		N09   , En2 , v100
	.byte	W12
	.byte		PAN   , c_v+32
	.byte		N09   , Gs2
	.byte	W12
	.byte		        En2
	.byte	W12
	.byte		PAN   , c_v-50
	.byte		N09   , En2 , v032
	.byte	W12
	.byte		PAN   , c_v+32
	.byte		N09   , Cn3 , v100
	.byte	W12
mus_qz_631_3_000:
	.byte		PAN   , c_v-48
	.byte		N09   , Cn3 , v036
	.byte	W12
	.byte		PAN   , c_v+32
	.byte		N09   , En2 , v100
	.byte	W12
	.byte		PAN   , c_v-45
	.byte		N09   , En2 , v032
	.byte	W12
	.byte		PAN   , c_v+32
	.byte		N09   , En2 , v100
	.byte	W12
	.byte		PAN   , c_v+32
	.byte		N09   , Gs2
	.byte	W12
	.byte		        En2
	.byte	W12
	.byte		PAN   , c_v-50
	.byte		N09   , En2 , v032
	.byte	W12
	.byte		PAN   , c_v+32
	.byte		N09   , Cn3 , v100
	.byte	W12
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_631_3_000
	.byte	PATT
	 ptrvalue	mus_qz_631_3_000
	.byte		VOL   , 64*mus_qz_631_mvl/mxv
	.byte		N09   , Cs3 , v100
	.byte	W12
	.byte		        Fn2
	.byte	W12
	.byte		PAN   , c_v-45
	.byte		N09   , Fn2 , v032
	.byte	W12
	.byte		PAN   , c_v+32
	.byte		N09   , Fn2 , v100
	.byte	W12
	.byte		PAN   , c_v+32
	.byte		N09   , An2
	.byte	W12
	.byte		        Fn2
	.byte	W12
	.byte		PAN   , c_v-50
	.byte		N09   , Fn2 , v032
	.byte	W12
	.byte		PAN   , c_v+32
	.byte		N09   , Cs3 , v100
	.byte	W12
mus_qz_631_3_001:
	.byte		PAN   , c_v-48
	.byte		N09   , Cs3 , v036
	.byte	W12
	.byte		PAN   , c_v+32
	.byte		N09   , Fn2 , v100
	.byte	W12
	.byte		PAN   , c_v-45
	.byte		N09   , Fn2 , v032
	.byte	W12
	.byte		PAN   , c_v+32
	.byte		N09   , Fn2 , v100
	.byte	W12
	.byte		PAN   , c_v+32
	.byte		N09   , An2
	.byte	W12
	.byte		        Fn2
	.byte	W12
	.byte		PAN   , c_v-50
	.byte		N09   , Fn2 , v032
	.byte	W12
	.byte		PAN   , c_v+32
	.byte		N09   , Cs3 , v100
	.byte	W12
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_631_3_001
	.byte		PAN   , c_v-48
	.byte		N09   , Cs3 , v036
	.byte	W12
	.byte		PAN   , c_v+32
	.byte		N09   , Fn2 , v100
	.byte	W12
	.byte		PAN   , c_v-45
	.byte		N09   , Fn2 , v032
	.byte	W12
	.byte		PAN   , c_v+32
	.byte		N09   , Fn2 , v100
	.byte	W12
	.byte		PAN   , c_v+32
	.byte		N09   , An2
	.byte	W12
	.byte		        Fn2
	.byte	W12
	.byte		PAN   , c_v-50
	.byte		N09   , Fn2 , v032
	.byte	W12
	.byte		PAN   , c_v+32
	.byte		N08   , Cs3 , v100
	.byte	W12
	.byte		PAN   , c_v-52
	.byte		N08   , Cs3 , v036
	.byte	W12
	.byte		PAN   , c_v+48
	.byte		N08   , Cs3 , v024
	.byte	W12
	.byte		VOICE , 23
	.byte		VOL   , 10*mus_qz_631_mvl/mxv
	.byte		PAN   , c_v+32
	.byte		N44   , Cn3 , v100 , gtp3
	.byte	W02
	.byte		VOL   , 11*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        11*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        13*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        14*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        15*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        17*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        18*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        19*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        22*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        22*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        25*mus_qz_631_mvl/mxv
	.byte	W16
	.byte		        11*mus_qz_631_mvl/mxv
	.byte		N44   , Bn2 , v100 , gtp3
	.byte	W02
	.byte		VOL   , 13*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        14*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        17*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        18*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        20*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        22*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        25*mus_qz_631_mvl/mxv
	.byte	W01
	.byte		        26*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        28*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        28*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        31*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        32*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        34*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        36*mus_qz_631_mvl/mxv
	.byte	W10
	.byte		        14*mus_qz_631_mvl/mxv
	.byte		N44   , As2 , v100 , gtp3
	.byte	W02
	.byte		VOL   , 16*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        18*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        21*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        22*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        25*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        28*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        30*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        33*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        37*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        38*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        40*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        41*mus_qz_631_mvl/mxv
	.byte	W12
	.byte		        23*mus_qz_631_mvl/mxv
	.byte		N44   , An2 , v100 , gtp3
	.byte	W02
	.byte		VOL   , 25*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        28*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        32*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        37*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        38*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        45*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        47*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        51*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        54*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        55*mus_qz_631_mvl/mxv
	.byte	W19
	.byte		        10*mus_qz_631_mvl/mxv
	.byte		N44   , Cs3 , v100 , gtp3
	.byte	W02
	.byte		VOL   , 11*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        11*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        13*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        14*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        15*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        17*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        18*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        19*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        22*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        22*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        25*mus_qz_631_mvl/mxv
	.byte	W16
	.byte		        11*mus_qz_631_mvl/mxv
	.byte		N44   , Cn3 , v100 , gtp3
	.byte	W02
	.byte		VOL   , 13*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        14*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        17*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        18*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        20*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        22*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        25*mus_qz_631_mvl/mxv
	.byte	W01
	.byte		        26*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        28*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        28*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        31*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        32*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        34*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        36*mus_qz_631_mvl/mxv
	.byte	W10
	.byte		        14*mus_qz_631_mvl/mxv
	.byte		N44   , Bn2 , v100 , gtp3
	.byte	W02
	.byte		VOL   , 16*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        18*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        21*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        22*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        25*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        28*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        30*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        33*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        37*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        38*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        40*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        41*mus_qz_631_mvl/mxv
	.byte	W12
	.byte		        23*mus_qz_631_mvl/mxv
	.byte		N44   , As2 , v100 , gtp3
	.byte	W02
	.byte		VOL   , 25*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        28*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        32*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        37*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        38*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        45*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        47*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        51*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        54*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        55*mus_qz_631_mvl/mxv
	.byte	W19
	.byte		        40*mus_qz_631_mvl/mxv
	.byte		N23   , An2
	.byte	W24
	.byte		        Gn2
	.byte	W24
	.byte		N11   , As2
	.byte	W12
	.byte		        Cn3
	.byte	W12
	.byte		        An2
	.byte	W12
	.byte		        Gn2
	.byte	W12
	.byte		VOL   , 51*mus_qz_631_mvl/mxv
	.byte		N23   , An2
	.byte	W24
	.byte		        Gn2
	.byte	W24
	.byte		N11   , As2
	.byte	W12
	.byte		        Cn3
	.byte	W12
	.byte		        An2
	.byte	W12
	.byte		        Gn2
	.byte	W12
	.byte		VOL   , 64*mus_qz_631_mvl/mxv
	.byte		N23   , An2
	.byte	W24
	.byte		        Gn2
	.byte	W24
	.byte		N11   , As2
	.byte	W12
	.byte		        Cn3
	.byte	W12
	.byte		        An2
	.byte	W12
	.byte		        Gn2
	.byte	W12
	.byte		N23   , An2
	.byte	W24
	.byte		        Gn2
	.byte	W24
	.byte		VOL   , 18*mus_qz_631_mvl/mxv
	.byte		TIE   , As2
	.byte	W05
	.byte		VOL   , 23*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        27*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        34*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        39*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        47*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        51*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        57*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        61*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        62*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        61*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        56*mus_qz_631_mvl/mxv
	.byte	W12
	.byte		        52*mus_qz_631_mvl/mxv
	.byte	W12
	.byte		        51*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        50*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        45*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        44*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        40*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        38*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        34*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        33*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        29*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        28*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        22*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        22*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        18*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        14*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        11*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        8*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        6*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        5*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        4*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        3*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        1*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        1*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        0*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        0*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        0*mus_qz_631_mvl/mxv
	.byte	W06
	.byte	EOT
	.byte	W01
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_631_3_B1
	.byte	FINE

/********************** Track  4 **********************/

mus_qz_631_4:
	.byte		KEYSH , mus_qz_631_key+0
mus_qz_631_4_B1:
	.byte		VOICE , 5
	.byte		BENDR , 6
	.byte		PAN   , c_v+24
	.byte		VOL   , 63*mus_qz_631_mvl/mxv
	.byte		PAN   , c_v-49
	.byte		VOL   , 17*mus_qz_631_mvl/mxv
	.byte		BEND  , c_v+0
	.byte		        c_v+1
	.byte	W06
	.byte		N23   , Cn4 , v100
	.byte	W24
	.byte		        As3
	.byte	W24
	.byte		N11   , Cs4
	.byte	W12
	.byte		        Ds4
	.byte	W12
	.byte		        Cn4
	.byte	W12
	.byte		        As3
	.byte	W06
mus_qz_631_4_000:
	.byte	W06
	.byte		N23   , Cn4 , v100
	.byte	W24
	.byte		        As3
	.byte	W24
	.byte		N44   , Cs4 , v100 , gtp3
	.byte	W42
	.byte	PEND
mus_qz_631_4_001:
	.byte	W06
	.byte		N23   , As3 , v100
	.byte	W24
	.byte		        Cs4
	.byte	W24
	.byte		N11   , Fn4
	.byte	W12
	.byte		        En4
	.byte	W12
	.byte		        Cn4
	.byte	W12
	.byte		        As3
	.byte	W06
	.byte	PEND
mus_qz_631_4_002:
	.byte	W06
	.byte		N44   , Cn4 , v100 , gtp3
	.byte	W48
	.byte		N23   , Cs4
	.byte	W24
	.byte		N44   , Cn4 , v100 , gtp3
	.byte	W18
	.byte	PEND
mus_qz_631_4_003:
	.byte	W30
	.byte		N23   , As3 , v100
	.byte	W24
	.byte		N11   , Cs4
	.byte	W12
	.byte		        Ds4
	.byte	W12
	.byte		        Cn4
	.byte	W12
	.byte		        As3
	.byte	W06
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_631_4_000
	.byte	PATT
	 ptrvalue	mus_qz_631_4_001
	.byte	PATT
	 ptrvalue	mus_qz_631_4_002
	.byte	PATT
	 ptrvalue	mus_qz_631_4_003
	.byte	PATT
	 ptrvalue	mus_qz_631_4_000
	.byte	PATT
	 ptrvalue	mus_qz_631_4_001
	.byte	W06
	.byte		N44   , Cn4 , v100 , gtp3
	.byte	W48
	.byte		N23   , Cs4
	.byte	W24
	.byte		        Cn4
	.byte	W18
	.byte	W06
	.byte		        Cs4
	.byte	W24
	.byte		        Bn3
	.byte	W24
	.byte		N11   , Dn4
	.byte	W12
	.byte		        En4
	.byte	W12
	.byte		        Cs4
	.byte	W12
	.byte		        Bn3
	.byte	W06
	.byte	W06
	.byte		N23   , Cs4
	.byte	W24
	.byte		        Bn3
	.byte	W24
	.byte		N44   , Dn4 , v100 , gtp3
	.byte	W42
	.byte	W06
	.byte		N23   , Bn3
	.byte	W24
	.byte		        Dn4
	.byte	W24
	.byte		N11   , Fs4
	.byte	W12
	.byte		        Fn4
	.byte	W12
	.byte		        Cs4
	.byte	W12
	.byte		        Bn3
	.byte	W06
	.byte	W06
	.byte		N44   , Cs4 , v100 , gtp3
	.byte	W48
	.byte		N23   , Dn4
	.byte	W24
	.byte		N44   , Cs4 , v100 , gtp3
	.byte	W18
	.byte	W24
	.byte	W06
	.byte		VOICE , 23
	.byte		VOL   , 1*mus_qz_631_mvl/mxv
	.byte		N92   , Cn4 , v100 , gtp3
	.byte	W02
	.byte		VOL   , 1*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        1*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        1*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        2*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        2*mus_qz_631_mvl/mxv
	.byte	W06
	.byte		        3*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        3*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        4*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        4*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        5*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        5*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        5*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        6*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        6*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        6*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        8*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        8*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        8*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        9*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        10*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        10*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        11*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        11*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        11*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        12*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        13*mus_qz_631_mvl/mxv
	.byte	W06
	.byte		        13*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        14*mus_qz_631_mvl/mxv
	.byte	W01
	.byte	W06
	.byte		        1*mus_qz_631_mvl/mxv
	.byte		N92   , Cs4 , v100 , gtp3
	.byte	W02
	.byte		VOL   , 2*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        2*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        2*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        3*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        3*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        3*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        4*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        5*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        5*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        6*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        6*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        7*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        7*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        8*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        8*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        9*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        10*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        10*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        10*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        11*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        11*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        11*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        11*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        12*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        13*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        14*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        14*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        15*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        16*mus_qz_631_mvl/mxv
	.byte	W01
	.byte	W06
	.byte		        3*mus_qz_631_mvl/mxv
	.byte		N92   , Dn4 , v100 , gtp3
	.byte	W05
	.byte		VOL   , 3*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        4*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        5*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        5*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        6*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        8*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        10*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        10*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        11*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        13*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        14*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        16*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        16*mus_qz_631_mvl/mxv
	.byte	W13
	.byte	W06
	.byte		        3*mus_qz_631_mvl/mxv
	.byte		N92   , Cs4 , v100 , gtp3
	.byte	W05
	.byte		VOL   , 3*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        4*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        5*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        6*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        6*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        8*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        10*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        11*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        12*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        13*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        15*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        17*mus_qz_631_mvl/mxv
	.byte	W18
	.byte	W06
	.byte		        8*mus_qz_631_mvl/mxv
	.byte		N92   , En4 , v100 , gtp3
	.byte	W05
	.byte		VOL   , 8*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        8*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        8*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        8*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        9*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        10*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        10*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        11*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        12*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        13*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        14*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        17*mus_qz_631_mvl/mxv
	.byte	W18
	.byte	W06
	.byte		        6*mus_qz_631_mvl/mxv
	.byte		N92   , Ds4 , v100 , gtp3
	.byte	W05
	.byte		VOL   , 6*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        6*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        7*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        8*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        8*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        10*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        10*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        11*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        12*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        14*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        17*mus_qz_631_mvl/mxv
	.byte	W24
	.byte	W01
	.byte	W06
	.byte		        4*mus_qz_631_mvl/mxv
	.byte		N92   , As4 , v100 , gtp3
	.byte	W05
	.byte		VOL   , 5*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        6*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        6*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        8*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        10*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        11*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        13*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        15*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        17*mus_qz_631_mvl/mxv
	.byte	W36
	.byte	W01
	.byte	W06
	.byte		        3*mus_qz_631_mvl/mxv
	.byte		N92   , An4 , v100 , gtp3
	.byte	W05
	.byte		VOL   , 5*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        6*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        8*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        10*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        13*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        16*mus_qz_631_mvl/mxv
	.byte	W05
	.byte		        17*mus_qz_631_mvl/mxv
	.byte	W48
	.byte	W01
	.byte	W06
	.byte		VOICE , 3
	.byte		VOL   , 8*mus_qz_631_mvl/mxv
	.byte		PAN   , c_v+23
	.byte		BEND  , c_v+0
	.byte		N11   , Gs4
	.byte	W12
	.byte		        Cs5
	.byte	W12
	.byte		N68   , En5 , v100 , gtp3
	.byte	W66
mus_qz_631_4_004:
	.byte	W06
	.byte		N11   , Gs4 , v100
	.byte	W12
	.byte		        Cs5
	.byte	W12
	.byte		N68   , En5 , v100 , gtp3
	.byte	W66
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_631_4_004
	.byte	W06
	.byte		N11   , Gs4 , v100
	.byte	W12
	.byte		        Cs5
	.byte	W12
	.byte		N64   , En5 , v100 , gtp1
	.byte	W66
	.byte	GOTO
	 ptrvalue	mus_qz_631_4_B1
	.byte	FINE

/********************** Track  5 **********************/

mus_qz_631_5:
	.byte		KEYSH , mus_qz_631_key+0
mus_qz_631_5_B1:
	.byte		VOICE , 10
	.byte		BENDR , 6
	.byte		PAN   , c_v+15
	.byte		        c_v+28
	.byte		VOL   , 30*mus_qz_631_mvl/mxv
	.byte		BEND  , c_v+0
	.byte		        c_v+0
	.byte	W01
	.byte		N23   , Cn5 , v100
	.byte	W24
	.byte		        As4
	.byte	W24
	.byte		N11   , Cs5
	.byte	W12
	.byte		        Ds5
	.byte	W12
	.byte		        Cn5
	.byte	W12
	.byte		        As4
	.byte	W11
mus_qz_631_5_000:
	.byte	W01
	.byte		N23   , Cn5 , v100
	.byte	W24
	.byte		        As4
	.byte	W24
	.byte		N44   , Cs5 , v100 , gtp3
	.byte	W44
	.byte	W03
	.byte	PEND
mus_qz_631_5_001:
	.byte	W01
	.byte		N23   , As4 , v100
	.byte	W24
	.byte		        Cs5
	.byte	W24
	.byte		N11   , Fn5
	.byte	W12
	.byte		        En5
	.byte	W12
	.byte		        Cn5
	.byte	W12
	.byte		        As4
	.byte	W11
	.byte	PEND
mus_qz_631_5_002:
	.byte	W01
	.byte		N44   , Cn5 , v100 , gtp3
	.byte	W48
	.byte		N23   , Cs5
	.byte	W24
	.byte		N44   , Cn5 , v100 , gtp3
	.byte	W23
	.byte	PEND
mus_qz_631_5_003:
	.byte	W24
	.byte	W01
	.byte		N23   , As4 , v100
	.byte	W24
	.byte		N11   , Cs5
	.byte	W12
	.byte		        Ds5
	.byte	W12
	.byte		        Cn5
	.byte	W12
	.byte		        As4
	.byte	W11
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_631_5_000
	.byte	PATT
	 ptrvalue	mus_qz_631_5_001
	.byte	PATT
	 ptrvalue	mus_qz_631_5_002
	.byte	PATT
	 ptrvalue	mus_qz_631_5_003
	.byte	PATT
	 ptrvalue	mus_qz_631_5_000
	.byte	PATT
	 ptrvalue	mus_qz_631_5_001
	.byte	W01
	.byte		N44   , Cn5 , v100 , gtp3
	.byte	W48
	.byte		N23   , Cs5
	.byte	W24
	.byte		        Cn5
	.byte	W23
	.byte	W01
	.byte		        Cs5
	.byte	W24
	.byte		        Bn4
	.byte	W24
	.byte		N11   , Dn5
	.byte	W12
	.byte		        En5
	.byte	W12
	.byte		        Cs5
	.byte	W12
	.byte		        Bn4
	.byte	W11
	.byte	W01
	.byte		N23   , Cs5
	.byte	W24
	.byte		        Bn4
	.byte	W24
	.byte		N44   , Dn5 , v100 , gtp3
	.byte	W44
	.byte	W03
	.byte	W01
	.byte		N23   , Bn4
	.byte	W24
	.byte		        Dn5
	.byte	W24
	.byte		N11   , Fs5
	.byte	W12
	.byte		        Fn5
	.byte	W12
	.byte		        Cs5
	.byte	W12
	.byte		        Bn4
	.byte	W11
	.byte	W01
	.byte		N44   , Cs5 , v100 , gtp3
	.byte	W48
	.byte		N23   , Dn5
	.byte	W24
	.byte		N44   , Cs5 , v100 , gtp2
	.byte	W23
	.byte	W24
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		VOL   , 15*mus_qz_631_mvl/mxv
	.byte	W01
	.byte		N11   , Gs4
	.byte	W12
	.byte		        Cs5
	.byte	W12
	.byte		N68   , En5 , v100 , gtp3
	.byte	W68
	.byte	W03
	.byte	W01
	.byte		N11   , Gs4
	.byte	W12
	.byte		        Cs5
	.byte	W12
	.byte		N68   , En5 , v100 , gtp3
	.byte	W68
	.byte	W03
	.byte		BEND  , c_v+1
	.byte	W01
	.byte		N11   , Gs4
	.byte	W12
	.byte		        Cs5
	.byte	W12
	.byte		N68   , En5 , v100 , gtp3
	.byte	W68
	.byte	W03
	.byte		BEND  , c_v+3
	.byte	W01
	.byte		N11   , Gs4
	.byte	W12
	.byte		        Cs5
	.byte	W12
	.byte		N68   , En5 , v100 , gtp1
	.byte	W68
	.byte	W03
	.byte	GOTO
	 ptrvalue	mus_qz_631_5_B1
	.byte		BEND  , c_v+0
	.byte	FINE

/********************** Track  6 **********************/

mus_qz_631_6:
	.byte		KEYSH , mus_qz_631_key+0
mus_qz_631_6_B1:
	.byte		VOICE , 0
	.byte		BENDR , 6
	.byte		PAN   , c_v+40
	.byte		VOL   , 51*mus_qz_631_mvl/mxv
	.byte		BEND  , c_v+0
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
	.byte	W24
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W92
	.byte	W02
	.byte		PAN   , c_v+38
	.byte		BEND  , c_v+4
	.byte	W02
	.byte		VOL   , 33*mus_qz_631_mvl/mxv
	.byte	W01
	.byte		VOICE , 9
	.byte		N11   , Gs4 , v100
	.byte	W12
	.byte		        Cs5
	.byte	W12
	.byte		N68   , En5 , v100 , gtp3
	.byte	W68
	.byte	W03
	.byte	W01
	.byte		N11   , Gs4
	.byte	W12
	.byte		        Cs5
	.byte	W12
	.byte		N68   , En5 , v100 , gtp3
	.byte	W68
	.byte	W01
	.byte		BEND  , c_v+8
	.byte	W02
	.byte	W01
	.byte		N11   , Gs4
	.byte	W12
	.byte		        Cs5
	.byte	W12
	.byte		N68   , En5 , v100 , gtp3
	.byte	W68
	.byte	W01
	.byte		BEND  , c_v+9
	.byte	W02
	.byte	W01
	.byte		N11   , Gs4
	.byte	W12
	.byte		        Cs5
	.byte	W12
	.byte		N68   , En5 , v100 , gtp1
	.byte	W68
	.byte	W03
	.byte	GOTO
	 ptrvalue	mus_qz_631_6_B1
	.byte	FINE

/********************** Track  7 **********************/

mus_qz_631_7:
	.byte		KEYSH , mus_qz_631_key+0
mus_qz_631_7_B1:
	.byte		VOICE , 34
	.byte		BENDR , 12
	.byte		        12
	.byte		PAN   , c_v+13
	.byte		VOL   , 2*mus_qz_631_mvl/mxv
	.byte		        1*mus_qz_631_mvl/mxv
	.byte		BEND  , c_v+0
	.byte		N23   , Cn2 , v100
	.byte	W02
	.byte		VOL   , 5*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        11*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        18*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        23*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        32*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        40*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        1*mus_qz_631_mvl/mxv
	.byte		PAN   , c_v-23
	.byte		N23   , As1
	.byte	W02
	.byte		VOL   , 4*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        9*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        13*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        18*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        24*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        32*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        39*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        7*mus_qz_631_mvl/mxv
	.byte		PAN   , c_v+31
	.byte		N11   , Cs2
	.byte	W02
	.byte		VOL   , 14*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        19*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        6*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        10*mus_qz_631_mvl/mxv
	.byte		PAN   , c_v-32
	.byte		N11   , Ds2
	.byte	W02
	.byte		VOL   , 19*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        29*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        5*mus_qz_631_mvl/mxv
	.byte		PAN   , c_v+41
	.byte		N11   , Cn2
	.byte	W02
	.byte		VOL   , 8*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        11*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        18*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        5*mus_qz_631_mvl/mxv
	.byte		PAN   , c_v-49
	.byte		N11   , As1
	.byte	W02
	.byte		VOL   , 11*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        23*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        24*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        1*mus_qz_631_mvl/mxv
	.byte		N23   , Cn2
	.byte	W02
	.byte		VOL   , 5*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        11*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        18*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        23*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        32*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        40*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        1*mus_qz_631_mvl/mxv
	.byte		N23   , As1
	.byte	W02
	.byte		VOL   , 4*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        9*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        13*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        18*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        24*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        32*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        39*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        9*mus_qz_631_mvl/mxv
	.byte		N44   , Cs2 , v100 , gtp3
	.byte	W02
	.byte		VOL   , 11*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        12*mus_qz_631_mvl/mxv
	.byte	W01
	.byte		PAN   , c_v-45
	.byte	W02
	.byte		        c_v-40
	.byte		VOL   , 13*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		PAN   , c_v-38
	.byte		VOL   , 14*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		PAN   , c_v-34
	.byte		VOL   , 17*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		PAN   , c_v-29
	.byte		VOL   , 18*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		PAN   , c_v-22
	.byte		VOL   , 18*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		PAN   , c_v-12
	.byte		VOL   , 20*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		PAN   , c_v-8
	.byte		VOL   , 22*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		PAN   , c_v+0
	.byte		VOL   , 27*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		PAN   , c_v+6
	.byte		VOL   , 30*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		PAN   , c_v+13
	.byte		VOL   , 36*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		PAN   , c_v+35
	.byte		VOL   , 46*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		PAN   , c_v+52
	.byte		VOL   , 61*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		PAN   , c_v+55
	.byte		VOL   , 70*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        1*mus_qz_631_mvl/mxv
	.byte		N23   , As1
	.byte	W02
	.byte		VOL   , 5*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        11*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        18*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        23*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        32*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        40*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        1*mus_qz_631_mvl/mxv
	.byte		N23   , Cs2
	.byte	W02
	.byte		VOL   , 4*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        9*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        13*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        18*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        24*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        32*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        39*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        7*mus_qz_631_mvl/mxv
	.byte		N11   , Fn2
	.byte	W02
	.byte		VOL   , 14*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        19*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        6*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        10*mus_qz_631_mvl/mxv
	.byte		N11   , En2
	.byte	W02
	.byte		VOL   , 19*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        29*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        5*mus_qz_631_mvl/mxv
	.byte		N11   , Cn2
	.byte	W02
	.byte		VOL   , 8*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        11*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        18*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        5*mus_qz_631_mvl/mxv
	.byte		N11   , As1
	.byte	W02
	.byte		VOL   , 11*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        23*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        24*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        4*mus_qz_631_mvl/mxv
	.byte		PAN   , c_v+48
	.byte		N44   , Cn2 , v100 , gtp3
	.byte	W02
	.byte		VOL   , 5*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		PAN   , c_v+46
	.byte		VOL   , 6*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		PAN   , c_v+45
	.byte		VOL   , 9*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		PAN   , c_v+40
	.byte		VOL   , 10*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		PAN   , c_v+35
	.byte		VOL   , 12*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		PAN   , c_v+29
	.byte		VOL   , 18*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		PAN   , c_v+18
	.byte		VOL   , 21*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		PAN   , c_v+13
	.byte		VOL   , 26*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		PAN   , c_v+6
	.byte		VOL   , 34*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		PAN   , c_v+0
	.byte		VOL   , 42*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		PAN   , c_v-11
	.byte		VOL   , 51*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		PAN   , c_v-17
	.byte		VOL   , 52*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		PAN   , c_v-29
	.byte	W03
	.byte		        c_v-40
	.byte	W07
	.byte		VOL   , 9*mus_qz_631_mvl/mxv
	.byte		PAN   , c_v-52
	.byte		N23   , Cs2
	.byte	W02
	.byte		VOL   , 17*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        27*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        35*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        40*mus_qz_631_mvl/mxv
	.byte	W12
	.byte		VOICE , 73
	.byte		VOL   , 7*mus_qz_631_mvl/mxv
	.byte		PAN   , c_v-50
	.byte		BEND  , c_v-64
	.byte		N44   , Cn3 , v068 , gtp3
	.byte	W02
	.byte		PAN   , c_v-45
	.byte	W01
	.byte		VOL   , 27*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        26*mus_qz_631_mvl/mxv
	.byte		PAN   , c_v-39
	.byte	W03
	.byte		VOL   , 24*mus_qz_631_mvl/mxv
	.byte		PAN   , c_v-34
	.byte	W01
	.byte		BEND  , c_v-60
	.byte	W03
	.byte		PAN   , c_v-29
	.byte		BEND  , c_v-56
	.byte	W02
	.byte		VOL   , 26*mus_qz_631_mvl/mxv
	.byte		PAN   , c_v-24
	.byte		BEND  , c_v-48
	.byte	W03
	.byte		PAN   , c_v-18
	.byte		BEND  , c_v-34
	.byte	W03
	.byte		PAN   , c_v-12
	.byte		BEND  , c_v-26
	.byte	W04
	.byte		VOL   , 27*mus_qz_631_mvl/mxv
	.byte		PAN   , c_v-6
	.byte		BEND  , c_v-20
	.byte	W02
	.byte		VOL   , 29*mus_qz_631_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		BEND  , c_v-12
	.byte	W03
	.byte		VOL   , 32*mus_qz_631_mvl/mxv
	.byte		PAN   , c_v+12
	.byte		BEND  , c_v-3
	.byte	W03
	.byte		VOL   , 35*mus_qz_631_mvl/mxv
	.byte		PAN   , c_v+16
	.byte		BEND  , c_v+0
	.byte	W04
	.byte		VOL   , 35*mus_qz_631_mvl/mxv
	.byte		PAN   , c_v+23
	.byte	W02
	.byte		VOL   , 39*mus_qz_631_mvl/mxv
	.byte		PAN   , c_v+30
	.byte	W10
	.byte		VOL   , 8*mus_qz_631_mvl/mxv
	.byte		PAN   , c_v+46
	.byte		N23   , As2 , v084
	.byte	W02
	.byte		VOL   , 12*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        18*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        23*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        28*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        37*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        42*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        12*mus_qz_631_mvl/mxv
	.byte		PAN   , c_v-22
	.byte		N11   , Cs3
	.byte	W02
	.byte		VOL   , 18*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        22*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		PAN   , c_v+25
	.byte		N11   , Ds3
	.byte	W03
	.byte		VOL   , 18*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        23*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        27*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        11*mus_qz_631_mvl/mxv
	.byte		PAN   , c_v-19
	.byte		N11   , Cn3
	.byte	W02
	.byte		VOL   , 17*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        20*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        23*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        11*mus_qz_631_mvl/mxv
	.byte		PAN   , c_v+47
	.byte		N11   , As2
	.byte	W02
	.byte		VOL   , 18*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        29*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        11*mus_qz_631_mvl/mxv
	.byte		N23   , Cn3
	.byte	W02
	.byte		VOL   , 15*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        18*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        23*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        35*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        42*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        45*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        50*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        16*mus_qz_631_mvl/mxv
	.byte		N23   , As2
	.byte	W02
	.byte		VOL   , 18*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        31*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        39*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        43*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        47*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        13*mus_qz_631_mvl/mxv
	.byte		N44   , Cs3 , v084 , gtp3
	.byte	W02
	.byte		VOL   , 15*mus_qz_631_mvl/mxv
	.byte	W01
	.byte		PAN   , c_v+34
	.byte	W02
	.byte		        c_v+29
	.byte		VOL   , 18*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		PAN   , c_v+21
	.byte		VOL   , 23*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		PAN   , c_v+16
	.byte		VOL   , 29*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		PAN   , c_v+8
	.byte		VOL   , 34*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		PAN   , c_v+2
	.byte		VOL   , 39*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		PAN   , c_v-6
	.byte		VOL   , 43*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		PAN   , c_v-14
	.byte		VOL   , 46*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		PAN   , c_v-25
	.byte		VOL   , 51*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		PAN   , c_v-35
	.byte		VOL   , 56*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		PAN   , c_v-43
	.byte	W16
	.byte		VOL   , 15*mus_qz_631_mvl/mxv
	.byte		N23   , As2
	.byte	W02
	.byte		VOL   , 18*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        23*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        29*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        35*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        38*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        39*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		PAN   , c_v-17
	.byte		VOL   , 11*mus_qz_631_mvl/mxv
	.byte		N23   , Cs3
	.byte	W02
	.byte		VOL   , 13*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        18*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        20*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        24*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        35*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        42*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        43*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        13*mus_qz_631_mvl/mxv
	.byte		PAN   , c_v+23
	.byte		N11   , Fn3
	.byte	W02
	.byte		VOL   , 18*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        29*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        34*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        13*mus_qz_631_mvl/mxv
	.byte		PAN   , c_v+47
	.byte		N11   , En3
	.byte	W02
	.byte		VOL   , 18*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        34*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        39*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        18*mus_qz_631_mvl/mxv
	.byte		PAN   , c_v+8
	.byte		N11   , Cn3
	.byte	W02
	.byte		VOL   , 31*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        35*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        39*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        18*mus_qz_631_mvl/mxv
	.byte		PAN   , c_v+32
	.byte		N11   , As2
	.byte	W02
	.byte		VOL   , 23*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        31*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        37*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        4*mus_qz_631_mvl/mxv
	.byte		N44   , Cn3 , v084 , gtp3
	.byte	W02
	.byte		VOL   , 6*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        8*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        11*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        13*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        16*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        20*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        24*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        28*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        32*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        37*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        43*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        50*mus_qz_631_mvl/mxv
	.byte	W12
	.byte		        11*mus_qz_631_mvl/mxv
	.byte		N23   , Cs3
	.byte	W02
	.byte		VOL   , 16*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        22*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        29*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        35*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        38*mus_qz_631_mvl/mxv
	.byte	W10
	.byte		N44   , Cn3 , v084 , gtp3
	.byte	W03
	.byte		VOL   , 34*mus_qz_631_mvl/mxv
	.byte		PAN   , c_v+20
	.byte	W02
	.byte		VOL   , 29*mus_qz_631_mvl/mxv
	.byte		PAN   , c_v+2
	.byte	W03
	.byte		VOL   , 20*mus_qz_631_mvl/mxv
	.byte		PAN   , c_v-22
	.byte	W04
	.byte		VOL   , 13*mus_qz_631_mvl/mxv
	.byte		PAN   , c_v-30
	.byte		BEND  , c_v-8
	.byte	W02
	.byte		VOL   , 8*mus_qz_631_mvl/mxv
	.byte		PAN   , c_v-43
	.byte		BEND  , c_v-16
	.byte	W03
	.byte		VOL   , 5*mus_qz_631_mvl/mxv
	.byte		PAN   , c_v-44
	.byte		BEND  , c_v-37
	.byte	W03
	.byte		VOL   , 4*mus_qz_631_mvl/mxv
	.byte		BEND  , c_v-55
	.byte	W04
	.byte		VOL   , 30*mus_qz_631_mvl/mxv
	.byte		BEND  , c_v-63
	.byte	W24
	.byte		VOL   , 22*mus_qz_631_mvl/mxv
	.byte		BEND  , c_v+0
	.byte		N23   , As1 , v100
	.byte	W24
	.byte		PAN   , c_v+34
	.byte		N11   , Cs2
	.byte	W12
	.byte		        Ds2
	.byte	W12
	.byte		PAN   , c_v-33
	.byte		N11   , Cn2
	.byte	W12
	.byte		        As1
	.byte	W12
	.byte		PAN   , c_v+31
	.byte		N23   , Cn2
	.byte	W24
	.byte		PAN   , c_v-17
	.byte		N23   , As1
	.byte	W24
	.byte		PAN   , c_v-41
	.byte		N44   , Cs2 , v100 , gtp3
	.byte	W48
	.byte		N23   , As1
	.byte	W24
	.byte		        Cs2
	.byte	W24
	.byte		PAN   , c_v+36
	.byte		N11   , Fn2
	.byte	W12
	.byte		PAN   , c_v+15
	.byte		N11   , En2
	.byte	W12
	.byte		PAN   , c_v-18
	.byte		N11   , Cn2
	.byte	W12
	.byte		PAN   , c_v-44
	.byte		N11   , As1
	.byte	W12
	.byte		N44   , Cn2 , v100 , gtp3
	.byte	W03
	.byte		PAN   , c_v-44
	.byte	W02
	.byte		        c_v-40
	.byte	W03
	.byte		        c_v-34
	.byte	W04
	.byte		        c_v-29
	.byte	W02
	.byte		        c_v-18
	.byte	W03
	.byte		        c_v+0
	.byte	W03
	.byte		        c_v+35
	.byte	W04
	.byte		        c_v+47
	.byte	W24
	.byte		N23   , Cs2
	.byte	W12
	.byte		PAN   , c_v+43
	.byte	W02
	.byte		        c_v+39
	.byte	W03
	.byte		        c_v+31
	.byte	W03
	.byte		        c_v+16
	.byte	W04
	.byte		        c_v+7
	.byte		N23   , Cn2
	.byte	W02
	.byte		PAN   , c_v+4
	.byte	W01
	.byte		        c_v-4
	.byte	W02
	.byte		        c_v-11
	.byte	W03
	.byte		        c_v-18
	.byte	W04
	.byte		        c_v-24
	.byte	W02
	.byte		        c_v-32
	.byte	W03
	.byte		        c_v-34
	.byte	W07
	.byte		        c_v+40
	.byte		N23   , Cs2
	.byte	W09
	.byte		PAN   , c_v+31
	.byte	W03
	.byte		        c_v+26
	.byte	W02
	.byte		        c_v+12
	.byte	W03
	.byte		        c_v+3
	.byte	W03
	.byte		        c_v-4
	.byte	W04
	.byte		        c_v-11
	.byte		N23   , Bn1
	.byte	W02
	.byte		PAN   , c_v-18
	.byte	W03
	.byte		        c_v-24
	.byte	W03
	.byte		        c_v-28
	.byte	W04
	.byte		        c_v-30
	.byte	W02
	.byte		        c_v-33
	.byte	W03
	.byte		        c_v-37
	.byte	W03
	.byte		        c_v-36
	.byte	W04
	.byte		        c_v-49
	.byte		N11   , Dn2
	.byte	W02
	.byte		PAN   , c_v-45
	.byte	W03
	.byte		        c_v-40
	.byte	W03
	.byte		        c_v-29
	.byte	W04
	.byte		        c_v-21
	.byte		N11   , En2
	.byte	W02
	.byte		PAN   , c_v-10
	.byte	W03
	.byte		        c_v+7
	.byte	W03
	.byte		        c_v+19
	.byte	W04
	.byte		        c_v+23
	.byte		N11   , Cs2
	.byte	W02
	.byte		PAN   , c_v+35
	.byte	W03
	.byte		        c_v+38
	.byte	W03
	.byte		        c_v+43
	.byte	W04
	.byte		        c_v+44
	.byte		N11   , Bn1
	.byte	W02
	.byte		PAN   , c_v+43
	.byte	W10
	.byte		        c_v-48
	.byte		N23   , Cs2
	.byte	W02
	.byte		PAN   , c_v-45
	.byte	W03
	.byte		        c_v-41
	.byte	W03
	.byte		        c_v-36
	.byte	W04
	.byte		        c_v-28
	.byte	W02
	.byte		        c_v-17
	.byte	W03
	.byte		        c_v-7
	.byte	W03
	.byte		        c_v+7
	.byte	W04
	.byte		        c_v+20
	.byte		N23   , Bn1
	.byte	W02
	.byte		PAN   , c_v+36
	.byte	W06
	.byte		        c_v+52
	.byte	W16
	.byte		N44   , Dn2 , v100 , gtp3
	.byte	W03
	.byte		PAN   , c_v+48
	.byte	W02
	.byte		        c_v+47
	.byte	W03
	.byte		        c_v+43
	.byte	W04
	.byte		        c_v+42
	.byte	W02
	.byte		        c_v+34
	.byte	W03
	.byte		        c_v+27
	.byte	W03
	.byte		        c_v+12
	.byte	W04
	.byte		        c_v+4
	.byte	W02
	.byte		        c_v-7
	.byte	W03
	.byte		        c_v-18
	.byte	W03
	.byte		        c_v-26
	.byte	W04
	.byte		        c_v-33
	.byte	W02
	.byte		        c_v-37
	.byte	W10
	.byte		        c_v+47
	.byte		N23   , Bn1
	.byte	W02
	.byte		PAN   , c_v+43
	.byte	W03
	.byte		        c_v+40
	.byte	W03
	.byte		        c_v+35
	.byte	W04
	.byte		        c_v+31
	.byte	W02
	.byte		        c_v+24
	.byte	W03
	.byte		        c_v+19
	.byte	W03
	.byte		        c_v+15
	.byte	W04
	.byte		        c_v+11
	.byte		N23   , Dn2
	.byte	W02
	.byte		PAN   , c_v+7
	.byte	W03
	.byte		        c_v+3
	.byte	W03
	.byte		        c_v-1
	.byte	W04
	.byte		        c_v-7
	.byte	W02
	.byte		        c_v-14
	.byte	W03
	.byte		        c_v-22
	.byte	W03
	.byte		        c_v-29
	.byte	W04
	.byte		        c_v-33
	.byte		N11   , Fs2
	.byte	W02
	.byte		PAN   , c_v-37
	.byte	W03
	.byte		        c_v-44
	.byte	W03
	.byte		        c_v-45
	.byte	W04
	.byte		N11   , Fn2
	.byte	W02
	.byte		PAN   , c_v-44
	.byte	W03
	.byte		        c_v-41
	.byte	W03
	.byte		        c_v-34
	.byte	W04
	.byte		        c_v-29
	.byte		N11   , Cs2
	.byte	W02
	.byte		PAN   , c_v-25
	.byte	W03
	.byte		        c_v-18
	.byte	W03
	.byte		        c_v-14
	.byte	W04
	.byte		N11   , Bn1
	.byte	W12
	.byte		PAN   , c_v+48
	.byte		N44   , Cs2 , v100 , gtp3
	.byte	W02
	.byte		PAN   , c_v+47
	.byte	W03
	.byte		        c_v+44
	.byte	W03
	.byte		        c_v+42
	.byte	W04
	.byte		        c_v+39
	.byte	W02
	.byte		        c_v+36
	.byte	W03
	.byte		        c_v+32
	.byte	W03
	.byte		        c_v+24
	.byte	W04
	.byte		        c_v+19
	.byte	W02
	.byte		        c_v+11
	.byte	W03
	.byte		        c_v+0
	.byte	W03
	.byte		        c_v-14
	.byte	W04
	.byte		        c_v-20
	.byte	W02
	.byte		        c_v-26
	.byte	W03
	.byte		        c_v-30
	.byte	W03
	.byte		        c_v-33
	.byte	W04
	.byte		        c_v-34
	.byte		N23   , Dn2
	.byte	W02
	.byte		PAN   , c_v-36
	.byte	W06
	.byte		        c_v-32
	.byte	W04
	.byte		        c_v-26
	.byte	W02
	.byte		        c_v-20
	.byte	W03
	.byte		        c_v-10
	.byte	W03
	.byte		        c_v-4
	.byte	W04
	.byte		        c_v+8
	.byte		N44   , Cs2 , v100 , gtp3
	.byte	W02
	.byte		PAN   , c_v+15
	.byte	W03
	.byte		        c_v+24
	.byte	W03
	.byte		        c_v+31
	.byte	W04
	.byte		        c_v+36
	.byte	W02
	.byte		        c_v+40
	.byte	W03
	.byte		        c_v+44
	.byte	W03
	.byte		        c_v+47
	.byte	W04
	.byte	W03
	.byte		        c_v+39
	.byte	W02
	.byte		        c_v+32
	.byte	W03
	.byte		        c_v+26
	.byte	W04
	.byte		        c_v+13
	.byte	W02
	.byte		        c_v+0
	.byte	W03
	.byte		        c_v-11
	.byte	W07
	.byte		VOL   , 6*mus_qz_631_mvl/mxv
	.byte		PAN   , c_v-11
	.byte		N92   , Cn2
	.byte	W02
	.byte		VOL   , 6*mus_qz_631_mvl/mxv
	.byte	W01
	.byte		        9*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        10*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        11*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        11*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        12*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        14*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        14*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        15*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        17*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        18*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        20*mus_qz_631_mvl/mxv
	.byte	W06
	.byte		        21*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        23*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        25*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        28*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        30*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        30*mus_qz_631_mvl/mxv
	.byte	W06
	.byte		        30*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        27*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        24*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        21*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        19*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        17*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        15*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        14*mus_qz_631_mvl/mxv
	.byte	W10
	.byte		        10*mus_qz_631_mvl/mxv
	.byte		N92   , Cs2
	.byte	W02
	.byte		VOL   , 10*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        11*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        11*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        12*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        13*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        14*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        15*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        16*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        18*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        19*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        19*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        20*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        21*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        23*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        25*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        27*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        30*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        32*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        33*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        34*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        34*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        32*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        30*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        30*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        27*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        24*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        22*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        21*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		        12*mus_qz_631_mvl/mxv
	.byte		N92   , Dn2
	.byte	W05
	.byte		VOL   , 13*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        14*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        14*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        15*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        18*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        19*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        20*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        21*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        22*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        24*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        27*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        28*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        30*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        32*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        33*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        35*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        35*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        37*mus_qz_631_mvl/mxv
	.byte	W06
	.byte		        40*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        37*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        35*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        34*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        30*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        30*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        28*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        24*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        20*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        11*mus_qz_631_mvl/mxv
	.byte		N92   , Cs2
	.byte	W02
	.byte		VOL   , 12*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        13*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        14*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        16*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        19*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        20*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        22*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        25*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        28*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        30*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        30*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        32*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        34*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        37*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        40*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        40*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        42*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        41*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        40*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        37*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        35*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        34*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        32*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        30*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        30*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        28*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        27*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        25*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        12*mus_qz_631_mvl/mxv
	.byte		N92   , En2 , v100 , gtp3
	.byte	W02
	.byte		VOL   , 13*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        14*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        14*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        15*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        16*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        17*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        18*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        19*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        20*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        21*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        22*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        23*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        28*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        30*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        30*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        30*mus_qz_631_mvl/mxv
	.byte	W12
	.byte		        30*mus_qz_631_mvl/mxv
	.byte	W06
	.byte		        28*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        23*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        22*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        21*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        20*mus_qz_631_mvl/mxv
	.byte	W04
	.byte		        19*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		N92   , Ds2 , v100 , gtp3
	.byte	W02
	.byte		VOL   , 19*mus_qz_631_mvl/mxv
	.byte	W06
	.byte		        19*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        20*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        21*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        22*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        24*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        27*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        28*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        30*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        30*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        32*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        33*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        34*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        35*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        35*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        37*mus_qz_631_mvl/mxv
	.byte	W06
	.byte		        38*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        40*mus_qz_631_mvl/mxv
	.byte	W06
	.byte		        38*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        37*mus_qz_631_mvl/mxv
	.byte	W06
	.byte		        35*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        34*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        32*mus_qz_631_mvl/mxv
	.byte	W06
	.byte		        30*mus_qz_631_mvl/mxv
	.byte	W07
	.byte		N92   , As2 , v100 , gtp3
	.byte	W02
	.byte		VOL   , 28*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        30*mus_qz_631_mvl/mxv
	.byte	W12
	.byte		        30*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        30*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        32*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        33*mus_qz_631_mvl/mxv
	.byte	W01
	.byte		        34*mus_qz_631_mvl/mxv
	.byte	W02
	.byte		        35*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        37*mus_qz_631_mvl/mxv
	.byte	W06
	.byte		        38*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        41*mus_qz_631_mvl/mxv
	.byte	W09
	.byte		        40*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        38*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        37*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        35*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        33*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        30*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        30*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        28*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        27*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        23*mus_qz_631_mvl/mxv
	.byte	W09
	.byte		        22*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        22*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        21*mus_qz_631_mvl/mxv
	.byte	W01
	.byte		N92   , An2 , v100 , gtp3
	.byte	W05
	.byte		VOL   , 20*mus_qz_631_mvl/mxv
	.byte	W12
	.byte		        21*mus_qz_631_mvl/mxv
	.byte	W09
	.byte		        22*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        23*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        24*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_631_mvl/mxv
	.byte	W06
	.byte		        27*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        28*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        28*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        30*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        30*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        32*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        33*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        35*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        35*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        38*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        40*mus_qz_631_mvl/mxv
	.byte	W03
	.byte		        42*mus_qz_631_mvl/mxv
	.byte	W22
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_631_7_B1
	.byte	FINE

/******************************************************/
	.p2align	2

mus_qz_631:
	.byte	7	/* NumTrks */
	.byte	0	/* NumBlks */
	.byte	mus_qz_631_pri	/* Priority */
	.byte	mus_qz_631_rev	/* Reverb. */
space64 4
	ptrvalue	mus_qz_631_grp

	ptrvalue	mus_qz_631_1
	ptrvalue	mus_qz_631_2
	ptrvalue	mus_qz_631_3
	ptrvalue	mus_qz_631_4
	ptrvalue	mus_qz_631_5
	ptrvalue	mus_qz_631_6
	ptrvalue	mus_qz_631_7

	.end
