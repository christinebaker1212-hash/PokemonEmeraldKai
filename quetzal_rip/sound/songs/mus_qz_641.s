	.include "asm/macros/bit_width.inc"

	.include "MPlayDef.s"

/* Moemon Quetzal song 641, disassembled from the ROM by tools/quetzal_rip/songs.py.
   "org" comments give the ROM address of the bytes that follow. */

	.equ	mus_qz_641_grp, voicegroup_qz_songs
	.equ	mus_qz_641_pri, 0
	.equ	mus_qz_641_rev, reverb_set+0
	.equ	mus_qz_641_mvl, 127
	.equ	mus_qz_641_key, 0
	.equ	mus_qz_641_tbs, 1
	.equ	mus_qz_641_exg, 1
	.equ	mus_qz_641_cmp, 1

	.section .rodata
	.global	mus_qz_641
	.p2align	2
/* org 0x0968E7A8 */

/********************** Track  1 **********************/

mus_qz_641_1:
	.byte		KEYSH , mus_qz_641_key+0
	.byte		TEMPO , 140*mus_qz_641_tbs/2
	.byte		VOICE , 32
	.byte		VOL   , 64*mus_qz_641_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte		        c_v+28
	.byte		VOL   , 82*mus_qz_641_mvl/mxv
	.byte		N23   , An2 , v100
	.byte		N23   , Dn3
	.byte	W24
	.byte		N05   , An2
	.byte		N05   , Dn3
	.byte	W06
	.byte		        An2 , v036
	.byte		N05   , Dn3
	.byte	W06
	.byte		        An2 , v100
	.byte		N05   , Dn3
	.byte	W06
	.byte		        An2 , v036
	.byte		N05   , Dn3
	.byte	W06
	.byte		        An2 , v100
	.byte		N05   , Dn3
	.byte	W06
	.byte		        An2 , v036
	.byte		N05   , Dn3
	.byte	W06
	.byte		        An2 , v100
	.byte		N05   , Dn3
	.byte	W06
	.byte		        An2 , v036
	.byte		N05   , Dn3
	.byte	W06
	.byte		        An2 , v100
	.byte		N05   , Dn3
	.byte	W06
	.byte		        An2 , v036
	.byte		N05   , Dn3
	.byte	W18
	.byte		        An2 , v100
	.byte		N05   , Dn3
	.byte	W06
	.byte		        An2 , v036
	.byte		N05   , Dn3
	.byte	W06
	.byte		        An2 , v100
	.byte		N05   , Dn3
	.byte	W06
	.byte		        An2 , v036
	.byte		N05   , Dn3
	.byte	W06
	.byte		N23   , Dn3 , v100
	.byte		N23   , An3
	.byte	W24
mus_qz_641_1_000:
	.byte		N23   , An2 , v100
	.byte		N23   , Dn3
	.byte	W24
	.byte		N05   , An2
	.byte		N05   , Dn3
	.byte	W06
	.byte		        An2 , v036
	.byte		N05   , Dn3
	.byte	W06
	.byte		        An2 , v100
	.byte		N05   , Dn3
	.byte	W06
	.byte		        An2 , v036
	.byte		N05   , Dn3
	.byte	W06
	.byte		        An2 , v100
	.byte		N05   , Dn3
	.byte	W06
	.byte		        An2 , v036
	.byte		N05   , Dn3
	.byte	W06
	.byte		        An2 , v100
	.byte		N05   , Dn3
	.byte	W06
	.byte		        An2 , v036
	.byte		N05   , Dn3
	.byte	W06
	.byte	PEND
	.byte		        An2 , v100
	.byte		N05   , Dn3
	.byte	W06
	.byte		        An2 , v036
	.byte		N05   , Dn3
	.byte	W18
	.byte		        An2 , v100
	.byte		N05   , Dn3
	.byte	W06
	.byte		        An2 , v036
	.byte		N05   , Dn3
	.byte	W06
	.byte		        An2 , v100
	.byte		N05   , Dn3
	.byte	W06
	.byte		        An2 , v036
	.byte		N05   , Dn3
	.byte	W06
	.byte		N23   , Ds3 , v100
	.byte		N23   , As3
	.byte	W24
	.byte	PATT
	 ptrvalue	mus_qz_641_1_000
	.byte		N05   , An2 , v100
	.byte		N05   , Dn3
	.byte	W06
	.byte		        An2 , v036
	.byte		N05   , Dn3
	.byte	W18
	.byte		        An2 , v100
	.byte		N05   , Dn3
	.byte	W12
	.byte		        An2
	.byte		N05   , Dn3
	.byte	W12
	.byte		N23   , Ds3
	.byte		N23   , An3
	.byte	W24
	.byte		        Dn2
	.byte		N23   , An2
	.byte		N23   , Dn3
	.byte	W24
	.byte		N05   , Dn2
	.byte		N05   , An2
	.byte		N05   , Dn3
	.byte	W12
	.byte		        Dn2
	.byte		N05   , An2
	.byte		N05   , Dn3
	.byte	W12
	.byte		        Dn2
	.byte		N05   , An2
	.byte		N05   , Dn3
	.byte	W12
	.byte		        Dn2
	.byte		N05   , An2
	.byte		N05   , Dn3
	.byte	W12
	.byte		        Dn2
	.byte		N05   , An2
	.byte		N05   , Dn3
	.byte	W24
	.byte		        Dn2
	.byte		N05   , An2
	.byte		N05   , Dn3
	.byte	W12
	.byte		        Dn2
	.byte		N05   , An2
	.byte		N05   , Dn3
	.byte	W12
	.byte		N23   , Fs2
	.byte		N23   , As2
	.byte		N23   , Cs3
	.byte	W24
	.byte	W84
	.byte		TEMPO , 308*mus_qz_641_tbs/2
	.byte	W12
	.byte	W54
	.byte		TEMPO , 140*mus_qz_641_tbs/2
	.byte	W30
	.byte		VOICE , 29
	.byte	W12
mus_qz_641_1_B1:
	.byte		VOL   , 82*mus_qz_641_mvl/mxv
	.byte		N32   , An3 , v100 , gtp3
	.byte	W36
	.byte		N11   , Dn3
	.byte	W12
	.byte		        Gn3
	.byte	W12
	.byte		        As3
	.byte	W12
	.byte		N92   , An3 , v100 , gtp3
	.byte	W24
	.byte		VOL   , 82*mus_qz_641_mvl/mxv
	.byte	W06
	.byte		        78*mus_qz_641_mvl/mxv
	.byte	W06
	.byte		        68*mus_qz_641_mvl/mxv
	.byte	W05
	.byte		        52*mus_qz_641_mvl/mxv
	.byte	W07
	.byte		        45*mus_qz_641_mvl/mxv
	.byte	W05
	.byte		        22*mus_qz_641_mvl/mxv
	.byte	W07
	.byte		        12*mus_qz_641_mvl/mxv
	.byte	W05
	.byte		        2*mus_qz_641_mvl/mxv
	.byte	W07
	.byte		        0*mus_qz_641_mvl/mxv
	.byte	W72
	.byte	W48
	.byte		        82*mus_qz_641_mvl/mxv
	.byte	W24
	.byte		N32   , Dn4 , v100 , gtp3
	.byte	W36
	.byte		N11   , Ds4
	.byte	W12
	.byte		N23   , Cn4
	.byte	W24
	.byte		N92   , Dn4 , v100 , gtp3
	.byte	W24
	.byte		VOL   , 82*mus_qz_641_mvl/mxv
	.byte	W06
	.byte		        78*mus_qz_641_mvl/mxv
	.byte	W06
	.byte		        68*mus_qz_641_mvl/mxv
	.byte	W05
	.byte		        52*mus_qz_641_mvl/mxv
	.byte	W07
	.byte		        45*mus_qz_641_mvl/mxv
	.byte	W05
	.byte		        22*mus_qz_641_mvl/mxv
	.byte	W07
	.byte		        12*mus_qz_641_mvl/mxv
	.byte	W05
	.byte		        2*mus_qz_641_mvl/mxv
	.byte	W07
	.byte		        0*mus_qz_641_mvl/mxv
	.byte	W72
	.byte	W48
	.byte		        82*mus_qz_641_mvl/mxv
	.byte	W24
	.byte		N32   , Cn4 , v100 , gtp3
	.byte	W36
	.byte		N11   , As3
	.byte	W12
	.byte		        An3
	.byte	W12
	.byte		        Gn3
	.byte	W12
	.byte		N92   , Fs3 , v100 , gtp3
	.byte	W24
	.byte		VOL   , 82*mus_qz_641_mvl/mxv
	.byte	W06
	.byte		        78*mus_qz_641_mvl/mxv
	.byte	W06
	.byte		        68*mus_qz_641_mvl/mxv
	.byte	W05
	.byte		        52*mus_qz_641_mvl/mxv
	.byte	W07
	.byte		        45*mus_qz_641_mvl/mxv
	.byte	W05
	.byte		        22*mus_qz_641_mvl/mxv
	.byte	W07
	.byte		        12*mus_qz_641_mvl/mxv
	.byte	W05
	.byte		        2*mus_qz_641_mvl/mxv
	.byte	W07
	.byte		        0*mus_qz_641_mvl/mxv
	.byte	W72
	.byte	W48
	.byte		        82*mus_qz_641_mvl/mxv
	.byte	W24
	.byte		N32   , Dn4 , v100 , gtp3
	.byte	W36
	.byte		N11   , Ds4
	.byte	W12
	.byte		N23   , Fn4
	.byte	W24
	.byte		TIE   , Fs4
	.byte	W24
	.byte		VOL   , 82*mus_qz_641_mvl/mxv
	.byte	W06
	.byte		        78*mus_qz_641_mvl/mxv
	.byte	W06
	.byte		        68*mus_qz_641_mvl/mxv
	.byte	W05
	.byte		        52*mus_qz_641_mvl/mxv
	.byte	W07
	.byte		        45*mus_qz_641_mvl/mxv
	.byte	W05
	.byte		        22*mus_qz_641_mvl/mxv
	.byte	W07
	.byte		        12*mus_qz_641_mvl/mxv
	.byte	W05
	.byte		        2*mus_qz_641_mvl/mxv
	.byte	W07
	.byte		        0*mus_qz_641_mvl/mxv
	.byte	W32
	.byte	EOT
	.byte	W40
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W84
	.byte		TEMPO , 308*mus_qz_641_tbs/2
	.byte	W12
	.byte		VOL   , 82*mus_qz_641_mvl/mxv
	.byte	W54
	.byte		TEMPO , 140*mus_qz_641_tbs/2
	.byte	W24
	.byte		VOICE , 29
	.byte	W18
	.byte	GOTO
	 ptrvalue	mus_qz_641_1_B1
	.byte	FINE

/********************** Track  2 **********************/

mus_qz_641_2:
	.byte		KEYSH , mus_qz_641_key+0
	.byte		VOICE , 32
	.byte		VOL   , 64*mus_qz_641_mvl/mxv
	.byte		PRIO  , 56
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte		PRIO  , 54
	.byte		        c_v-20
	.byte		VOL   , 23*mus_qz_641_mvl/mxv
	.byte	W06
	.byte		N23   , An2 , v100
	.byte		N23   , Dn3
	.byte	W24
	.byte		N05   , An2
	.byte		N05   , Dn3
	.byte	W12
	.byte		        An2
	.byte		N05   , Dn3
	.byte	W12
	.byte		        An2
	.byte		N05   , Dn3
	.byte	W12
	.byte		        An2
	.byte		N05   , Dn3
	.byte	W06
	.byte	W06
	.byte		        An2
	.byte		N05   , Dn3
	.byte	W06
	.byte		        An2 , v048
	.byte		N05   , Dn3
	.byte	W18
	.byte		        An2 , v100
	.byte		N05   , Dn3
	.byte	W12
	.byte		        An2
	.byte		N05   , Dn3
	.byte	W12
	.byte	N23
	.byte		N23   , An3
	.byte	W18
mus_qz_641_2_000:
	.byte	W06
	.byte		N23   , An2 , v100
	.byte		N23   , Dn3
	.byte	W24
	.byte		N05   , An2
	.byte		N05   , Dn3
	.byte	W12
	.byte		        An2
	.byte		N05   , Dn3
	.byte	W12
	.byte		        An2
	.byte		N05   , Dn3
	.byte	W12
	.byte		        An2
	.byte		N05   , Dn3
	.byte	W06
	.byte	PEND
	.byte	W06
	.byte		        An2
	.byte		N05   , Dn3
	.byte	W06
	.byte		        An2 , v048
	.byte		N05   , Dn3
	.byte	W18
	.byte		        An2 , v100
	.byte		N05   , Dn3
	.byte	W12
	.byte		        An2
	.byte		N05   , Dn3
	.byte	W12
	.byte		N23   , Ds3
	.byte		N23   , As3
	.byte	W18
	.byte	PATT
	 ptrvalue	mus_qz_641_2_000
	.byte	W06
	.byte		N05   , An2 , v100
	.byte		N05   , Dn3
	.byte	W24
	.byte		        An2
	.byte		N05   , Dn3
	.byte	W12
	.byte		        An2
	.byte		N05   , Dn3
	.byte	W12
	.byte		N17   , Ds3
	.byte		N17   , An3
	.byte	W18
	.byte	W06
	.byte		N23   , Dn3
	.byte	W24
	.byte	N05
	.byte	W12
	.byte	N05
	.byte	W12
	.byte	N05
	.byte	W12
	.byte	N05
	.byte	W06
	.byte	W06
	.byte	N05
	.byte	W24
	.byte	N05
	.byte	W12
	.byte	N05
	.byte	W12
	.byte		N23   , Cs3
	.byte	W18
	.byte	W96
	.byte	W90
	.byte		VOICE , 29
	.byte	W06
mus_qz_641_2_B1:
	.byte	W06
	.byte		VOL   , 23*mus_qz_641_mvl/mxv
	.byte		N32   , An3 , v100 , gtp3
	.byte	W36
	.byte		N11   , Dn3
	.byte	W12
	.byte		        Gn3
	.byte	W12
	.byte		        As3
	.byte	W06
	.byte	W06
	.byte		N92   , An3
	.byte	W18
	.byte		VOL   , 26*mus_qz_641_mvl/mxv
	.byte		        23*mus_qz_641_mvl/mxv
	.byte	W05
	.byte		        21*mus_qz_641_mvl/mxv
	.byte	W07
	.byte		        16*mus_qz_641_mvl/mxv
	.byte	W05
	.byte		        12*mus_qz_641_mvl/mxv
	.byte	W07
	.byte		        8*mus_qz_641_mvl/mxv
	.byte	W05
	.byte		        2*mus_qz_641_mvl/mxv
	.byte	W07
	.byte		        0*mus_qz_641_mvl/mxv
	.byte	W05
	.byte		        0*mus_qz_641_mvl/mxv
	.byte	W07
	.byte	W72
	.byte	W72
	.byte	W06
	.byte		        23*mus_qz_641_mvl/mxv
	.byte		N32   , Dn4 , v100 , gtp3
	.byte	W36
	.byte		N11   , Ds4
	.byte	W12
	.byte		N23   , Cn4
	.byte	W18
	.byte	W06
	.byte		N92   , Dn4 , v100 , gtp3
	.byte	W18
	.byte		VOL   , 26*mus_qz_641_mvl/mxv
	.byte		        23*mus_qz_641_mvl/mxv
	.byte	W05
	.byte		        21*mus_qz_641_mvl/mxv
	.byte	W07
	.byte		        16*mus_qz_641_mvl/mxv
	.byte	W05
	.byte		        12*mus_qz_641_mvl/mxv
	.byte	W07
	.byte		        8*mus_qz_641_mvl/mxv
	.byte	W05
	.byte		        2*mus_qz_641_mvl/mxv
	.byte	W07
	.byte		        0*mus_qz_641_mvl/mxv
	.byte	W05
	.byte		        0*mus_qz_641_mvl/mxv
	.byte	W07
	.byte	W72
	.byte	W72
	.byte	W06
	.byte		        23*mus_qz_641_mvl/mxv
	.byte		N32   , Cn4 , v100 , gtp3
	.byte	W36
	.byte		N11   , As3
	.byte	W12
	.byte		        An3
	.byte	W12
	.byte		        Gn3
	.byte	W06
	.byte	W06
	.byte		N92   , Fs3 , v100 , gtp3
	.byte	W18
	.byte		VOL   , 26*mus_qz_641_mvl/mxv
	.byte		        23*mus_qz_641_mvl/mxv
	.byte	W05
	.byte		        21*mus_qz_641_mvl/mxv
	.byte	W07
	.byte		        16*mus_qz_641_mvl/mxv
	.byte	W05
	.byte		        12*mus_qz_641_mvl/mxv
	.byte	W07
	.byte		        8*mus_qz_641_mvl/mxv
	.byte	W05
	.byte		        2*mus_qz_641_mvl/mxv
	.byte	W07
	.byte		        0*mus_qz_641_mvl/mxv
	.byte	W05
	.byte		        0*mus_qz_641_mvl/mxv
	.byte	W07
	.byte	W72
	.byte	W72
	.byte	W06
	.byte		        23*mus_qz_641_mvl/mxv
	.byte		N32   , Dn4 , v100 , gtp3
	.byte	W36
	.byte		N11   , Ds4
	.byte	W12
	.byte		N23   , Fn4
	.byte	W18
	.byte	W06
	.byte		TIE   , Fs4
	.byte	W18
	.byte		VOL   , 26*mus_qz_641_mvl/mxv
	.byte		        23*mus_qz_641_mvl/mxv
	.byte	W05
	.byte		        21*mus_qz_641_mvl/mxv
	.byte	W07
	.byte		        16*mus_qz_641_mvl/mxv
	.byte	W05
	.byte		        12*mus_qz_641_mvl/mxv
	.byte	W07
	.byte		        8*mus_qz_641_mvl/mxv
	.byte	W05
	.byte		        2*mus_qz_641_mvl/mxv
	.byte	W07
	.byte		        0*mus_qz_641_mvl/mxv
	.byte	W05
	.byte		        0*mus_qz_641_mvl/mxv
	.byte	W07
	.byte	W36
	.byte	W02
	.byte	EOT
	.byte	W32
	.byte	W02
	.byte	W72
	.byte	W06
	.byte		VOICE , 33
	.byte		VOL   , 12*mus_qz_641_mvl/mxv
	.byte		PAN   , c_v-39
	.byte		N23   , An2
	.byte		N23   , Dn3
	.byte	W24
	.byte		N05   , An2
	.byte		N05   , Dn3
	.byte	W12
	.byte		        An2
	.byte		N05   , Dn3
	.byte	W12
	.byte		        An2
	.byte		N05   , Dn3
	.byte	W12
	.byte		        An2
	.byte		N05   , Dn3
	.byte	W06
	.byte	W06
	.byte		        An2
	.byte		N05   , Dn3
	.byte	W24
	.byte		        An2
	.byte		N05   , Dn3
	.byte	W12
	.byte		        An2
	.byte		N05   , Dn3
	.byte	W12
	.byte	N23
	.byte		N23   , An3
	.byte	W18
	.byte	PATT
	 ptrvalue	mus_qz_641_2_000
	.byte	W06
	.byte		N05   , An2 , v100
	.byte		N05   , Dn3
	.byte	W24
	.byte		        An2
	.byte		N05   , Dn3
	.byte	W12
	.byte		        An2
	.byte		N05   , Dn3
	.byte	W12
	.byte		N23   , Ds3
	.byte		N23   , As3
	.byte	W18
	.byte	PATT
	 ptrvalue	mus_qz_641_2_000
	.byte	W06
	.byte		N05   , An2 , v100
	.byte		N05   , Dn3
	.byte	W24
	.byte		        An2
	.byte		N05   , Dn3
	.byte	W12
	.byte		        An2
	.byte		N05   , Dn3
	.byte	W12
	.byte		N23   , Ds3
	.byte		N23   , An3
	.byte	W18
	.byte	PATT
	 ptrvalue	mus_qz_641_2_000
	.byte	W06
	.byte		N05   , An2 , v100
	.byte		N05   , Dn3
	.byte	W24
	.byte		        An2
	.byte		N05   , Dn3
	.byte	W12
	.byte		        An2
	.byte		N05   , Dn3
	.byte	W12
	.byte		N23   , Fs2
	.byte		N23   , As2
	.byte		N23   , Cs3
	.byte	W18
	.byte	W06
	.byte		N05   , Fs2 , v044
	.byte		N05   , As2
	.byte		N05   , Cs3
	.byte	W90
	.byte	W06
	.byte		VOL   , 82*mus_qz_641_mvl/mxv
	.byte	W78
	.byte		VOICE , 29
	.byte	W12
	.byte	GOTO
	 ptrvalue	mus_qz_641_2_B1
	.byte	FINE

/********************** Track  3 **********************/

mus_qz_641_3:
	.byte		KEYSH , mus_qz_641_key+0
	.byte		VOICE , 6
	.byte		VOL   , 64*mus_qz_641_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W96
	.byte	W78
	.byte		VOICE , 23
	.byte	W18
mus_qz_641_3_B1:
	.byte		PAN   , c_v-17
	.byte		VOL   , 64*mus_qz_641_mvl/mxv
	.byte	W72
	.byte		N44   , Dn3 , v100 , gtp3
	.byte		N44   , An3 , v100 , gtp3
	.byte	W48
	.byte		N23   , Gn3
	.byte		N23   , Cn4
	.byte	W24
	.byte		N05   , An3
	.byte		N05   , Dn4
	.byte	W06
	.byte		        An3 , v040
	.byte		N05   , Dn4
	.byte	W06
	.byte		        Dn2 , v100
	.byte		N05   , An2
	.byte	W06
	.byte		        Dn2 , v040
	.byte		N05   , An2
	.byte	W06
	.byte		N44   , An2 , v100 , gtp3
	.byte		N44   , En3 , v100 , gtp3
	.byte	W48
	.byte		        Cn3 , v100 , gtp3
	.byte		N44   , Gn3 , v100 , gtp3
	.byte	W48
	.byte		        Dn3 , v100 , gtp3
	.byte		N44   , An3 , v100 , gtp3
	.byte	W24
	.byte	W24
	.byte		N68   , Gn3 , v100 , gtp3
	.byte		N68   , Cn4 , v100 , gtp3
	.byte	W48
	.byte	W24
	.byte		N23   , Dn3
	.byte		N23   , An3
	.byte	W24
	.byte		        Cn3
	.byte		N23   , Gn3
	.byte	W24
	.byte	N23
	.byte		N23   , Cn4
	.byte	W24
	.byte		N05   , An3
	.byte		N05   , Dn4
	.byte	W06
	.byte		        An3 , v040
	.byte		N05   , Dn4
	.byte	W06
	.byte		        Gn2 , v100
	.byte		N05   , Dn3
	.byte	W06
	.byte		        Gn2 , v040
	.byte		N05   , Dn3
	.byte	W06
	.byte		N23   , Cn3 , v100
	.byte		N23   , Gn3
	.byte	W24
	.byte		N11   , An2
	.byte		N11   , En3
	.byte	W12
	.byte		        Gn2
	.byte		N11   , Cn3
	.byte	W12
	.byte		N23   , Dn3
	.byte		N23   , Gn3
	.byte	W24
	.byte		        Cs3
	.byte		N23   , As3
	.byte	W24
	.byte		N56   , Dn3 , v100 , gtp3
	.byte		N56   , An3 , v100 , gtp3
	.byte	W60
	.byte		N11   , Gn2
	.byte		N11   , Cn3
	.byte	W12
	.byte		N23   , An2
	.byte		N23   , Dn3
	.byte	W24
	.byte		        Dn2
	.byte		N23   , An2
	.byte	W24
	.byte	N05
	.byte		N05   , Fs3
	.byte	W06
	.byte		        An2 , v040
	.byte		N05   , Fs3
	.byte	W06
	.byte		        As2 , v100
	.byte		N05   , Gn3
	.byte	W06
	.byte		        As2 , v040
	.byte		N05   , Gn3
	.byte	W06
	.byte		N23   , Dn3 , v100
	.byte		N23   , An3
	.byte	W24
	.byte		N05   , An2
	.byte		N05   , Dn3
	.byte	W06
	.byte		        An2 , v040
	.byte		N05   , Dn3
	.byte	W06
	.byte		        Gn2 , v100
	.byte		N05   , Cn3
	.byte	W06
	.byte		        Gn2 , v040
	.byte		N05   , Cn3
	.byte	W06
	.byte		N23   , As2 , v100
	.byte		N23   , Ds3
	.byte	W24
	.byte		        An2
	.byte		N23   , Dn3
	.byte	W24
	.byte	N23
	.byte		N23   , Gn3
	.byte	W24
	.byte		        Ds3
	.byte		N23   , As3
	.byte	W24
	.byte		        Dn3
	.byte		N23   , An3
	.byte	W24
	.byte		N05   , Cn3
	.byte		N05   , Gn3
	.byte	W06
	.byte		        Cn3 , v040
	.byte		N05   , Gn3
	.byte	W06
	.byte		        Gn2 , v100
	.byte		N05   , Cn3
	.byte	W06
	.byte		        Gn2 , v040
	.byte		N05   , Cn3
	.byte	W06
	.byte		N23   , Gn3 , v100
	.byte		N23   , Cn4
	.byte	W24
	.byte		N44   , Dn3 , v100 , gtp3
	.byte		N44   , An3 , v100 , gtp3
	.byte	W48
	.byte		N23   , Ds3
	.byte		N23   , As3
	.byte	W24
	.byte		N44   , Fs3 , v100 , gtp3
	.byte		N44   , Cn4 , v100 , gtp3
	.byte	W48
	.byte		N23   , Gn3
	.byte		N23   , Cs4
	.byte	W24
	.byte		N68   , An3 , v100 , gtp3
	.byte		N68   , Dn4 , v100 , gtp3
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W66
	.byte		PAN   , c_v+26
	.byte	W06
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W96
	.byte	W72
	.byte		        c_v-17
	.byte	W06
	.byte		VOICE , 23
	.byte	W18
	.byte	GOTO
	 ptrvalue	mus_qz_641_3_B1
	.byte	FINE

/********************** Track  4 **********************/

mus_qz_641_4:
	.byte		KEYSH , mus_qz_641_key+0
	.byte		VOICE , 28
	.byte		VOL   , 64*mus_qz_641_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte		VOL   , 82*mus_qz_641_mvl/mxv
	.byte		PAN   , c_v+39
	.byte	W72
	.byte	W72
	.byte		N32   , Dn2 , v100 , gtp3
	.byte	W72
	.byte	W72
	.byte		        Dn2 , v100 , gtp3
	.byte	W72
	.byte	W72
	.byte		        Dn2 , v100 , gtp3
	.byte	W72
	.byte	W72
	.byte	W96
	.byte	W96
mus_qz_641_4_B1:
	.byte		VOICE , 28
	.byte		PAN   , c_v+39
	.byte		VOL   , 82*mus_qz_641_mvl/mxv
	.byte		N11   , Dn2 , v100
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		        Dn1
	.byte	W12
	.byte		        An1
	.byte	W12
	.byte		        An2
	.byte	W12
	.byte	N03
	.byte	W04
	.byte	N03
	.byte	W04
	.byte	N03
	.byte	W04
mus_qz_641_4_000:
	.byte		N11   , Dn2 , v100
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		N23   , Dn1
	.byte	W24
	.byte		N11   , An2
	.byte	W12
	.byte	N03
	.byte	W04
	.byte	N03
	.byte	W04
	.byte	N03
	.byte	W04
	.byte	PEND
mus_qz_641_4_001:
	.byte		N23   , Dn2 , v100
	.byte	W24
	.byte		N11   , An1
	.byte	W12
	.byte	N03
	.byte	W04
	.byte	N03
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		N11   , Cn1
	.byte	W12
	.byte		        Cn2
	.byte	W12
	.byte	PEND
mus_qz_641_4_002:
	.byte		N11   , Dn1 , v100
	.byte	W12
	.byte		        Dn2
	.byte	W12
	.byte		N23   , An1
	.byte	W24
	.byte		N11   , An2
	.byte	W12
	.byte	N03
	.byte	W04
	.byte	N03
	.byte	W04
	.byte	N03
	.byte	W04
	.byte	PEND
mus_qz_641_4_003:
	.byte		N11   , Dn2 , v100
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		        Dn1
	.byte	W12
	.byte		        An1
	.byte	W12
	.byte		        An2
	.byte	W12
	.byte	N03
	.byte	W04
	.byte	N03
	.byte	W04
	.byte	N03
	.byte	W04
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_641_4_000
	.byte	PATT
	 ptrvalue	mus_qz_641_4_001
mus_qz_641_4_004:
	.byte		N11   , Dn1 , v100
	.byte	W12
	.byte		        Dn2
	.byte	W12
	.byte		N23   , An1
	.byte	W24
	.byte		N11   , As2
	.byte	W12
	.byte	N03
	.byte	W04
	.byte	N03
	.byte	W04
	.byte	N03
	.byte	W04
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_641_4_003
	.byte	PATT
	 ptrvalue	mus_qz_641_4_000
	.byte	PATT
	 ptrvalue	mus_qz_641_4_001
	.byte	PATT
	 ptrvalue	mus_qz_641_4_002
	.byte	PATT
	 ptrvalue	mus_qz_641_4_003
	.byte	PATT
	 ptrvalue	mus_qz_641_4_000
	.byte	PATT
	 ptrvalue	mus_qz_641_4_001
	.byte	PATT
	 ptrvalue	mus_qz_641_4_004
	.byte		VOICE , 33
	.byte		PAN   , c_v+24
	.byte		VOL   , 51*mus_qz_641_mvl/mxv
	.byte		N23   , An2 , v100
	.byte		N23   , Dn3
	.byte	W24
	.byte		N05   , An2
	.byte		N05   , Dn3
	.byte	W12
	.byte		        An2
	.byte		N05   , Dn3
	.byte	W12
	.byte		        An2
	.byte		N05   , Dn3
	.byte	W12
	.byte		        An2
	.byte		N05   , Dn3
	.byte	W12
	.byte		        An2
	.byte		N05   , Dn3
	.byte	W24
	.byte		        An2
	.byte		N05   , Dn3
	.byte	W12
	.byte		        An2
	.byte		N05   , Dn3
	.byte	W12
	.byte	N23
	.byte		N23   , An3
	.byte	W24
mus_qz_641_4_005:
	.byte		N23   , An2 , v100
	.byte		N23   , Dn3
	.byte	W24
	.byte		N05   , An2
	.byte		N05   , Dn3
	.byte	W12
	.byte		        An2
	.byte		N05   , Dn3
	.byte	W12
	.byte		        An2
	.byte		N05   , Dn3
	.byte	W12
	.byte		        An2
	.byte		N05   , Dn3
	.byte	W12
	.byte	PEND
	.byte		        An2
	.byte		N05   , Dn3
	.byte	W24
	.byte		        An2
	.byte		N05   , Dn3
	.byte	W12
	.byte		        An2
	.byte		N05   , Dn3
	.byte	W12
	.byte		N23   , Ds3
	.byte		N23   , As3
	.byte	W24
	.byte	PATT
	 ptrvalue	mus_qz_641_4_005
	.byte		N05   , An2 , v100
	.byte		N05   , Dn3
	.byte	W24
	.byte		        An2
	.byte		N05   , Dn3
	.byte	W12
	.byte		        An2
	.byte		N05   , Dn3
	.byte	W12
	.byte		N23   , Ds3
	.byte		N23   , An3
	.byte	W24
	.byte	PATT
	 ptrvalue	mus_qz_641_4_005
	.byte		N05   , An2 , v100
	.byte		N05   , Dn3
	.byte	W24
	.byte		        An2
	.byte		N05   , Dn3
	.byte	W12
	.byte		        An2
	.byte		N05   , Dn3
	.byte	W12
	.byte		N23   , Fs2
	.byte		N23   , As2
	.byte		N23   , Cs3
	.byte	W24
	.byte		N05   , Fs2 , v044
	.byte		N05   , As2
	.byte		N05   , Cs3
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_641_4_B1
	.byte	FINE

/********************** Track  5 **********************/

mus_qz_641_5:
	.byte		KEYSH , mus_qz_641_key+0
	.byte		VOICE , 23
	.byte		VOL   , 64*mus_qz_641_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte		        c_v-30
	.byte		VOL   , 23*mus_qz_641_mvl/mxv
	.byte	W72
	.byte	W72
	.byte	W48
	.byte		        23*mus_qz_641_mvl/mxv
	.byte		PAN   , c_v-30
	.byte	W24
mus_qz_641_5_000:
	.byte		N05   , An4 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte	N05
	.byte	W06
	.byte	N05
	.byte	W06
	.byte	N05
	.byte	W06
	.byte	N05
	.byte	W06
	.byte	N05
	.byte	W06
	.byte	N05
	.byte	W06
	.byte	N05
	.byte	W06
	.byte	N05
	.byte	W06
	.byte	N05
	.byte	W06
	.byte	N05
	.byte	W06
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_641_5_000
	.byte		N05   , An4 , v100
	.byte		N05   , Cn5
	.byte	W06
	.byte		        An4
	.byte		N05   , Cn5
	.byte	W06
	.byte		        An4
	.byte		N05   , Cn5
	.byte	W06
	.byte		        An4
	.byte		N05   , Cn5
	.byte	W06
	.byte		        An4
	.byte		N05   , Cn5
	.byte	W06
	.byte		        An4
	.byte		N05   , Cn5
	.byte	W06
	.byte		        An4
	.byte		N05   , Cn5
	.byte	W06
	.byte		        An4
	.byte		N05   , Cn5
	.byte	W06
	.byte		        An4
	.byte		N05   , Cn5
	.byte	W06
	.byte		        An4
	.byte		N05   , Cn5
	.byte	W06
	.byte		        An4
	.byte		N05   , Cn5
	.byte	W06
	.byte		        An4
	.byte		N05   , Cn5
	.byte	W06
	.byte		        An4
	.byte		N05   , Cn5
	.byte	W06
	.byte		        An4
	.byte		N05   , Cn5
	.byte	W06
	.byte		        An4
	.byte		N05   , Cn5
	.byte	W06
	.byte		        An4
	.byte		N05   , Cn5
	.byte	W06
	.byte		        An4
	.byte		N05   , Cn5
	.byte	W06
	.byte		        An4
	.byte		N05   , Cn5
	.byte	W06
	.byte		        An4
	.byte		N05   , Cn5
	.byte	W06
	.byte		        An4
	.byte		N05   , Cn5
	.byte	W06
	.byte		VOL   , 23*mus_qz_641_mvl/mxv
	.byte		N05   , An4
	.byte		N05   , Cn5
	.byte	W06
	.byte		        An4
	.byte		N05   , Cn5
	.byte	W06
	.byte		        An4
	.byte		N05   , Cn5
	.byte	W06
	.byte		VOL   , 26*mus_qz_641_mvl/mxv
	.byte		N05   , An4
	.byte		N05   , Cn5
	.byte	W06
	.byte		        An4
	.byte		N05   , Dn5
	.byte	W05
	.byte		VOL   , 27*mus_qz_641_mvl/mxv
	.byte	W01
	.byte		N05   , An4
	.byte		N05   , Dn5
	.byte	W06
	.byte		VOL   , 29*mus_qz_641_mvl/mxv
	.byte		N05   , An4
	.byte		N05   , Dn5
	.byte	W05
	.byte		VOL   , 31*mus_qz_641_mvl/mxv
	.byte	W01
	.byte		N05   , An4
	.byte		N05   , Dn5
	.byte	W06
	.byte		VOL   , 33*mus_qz_641_mvl/mxv
	.byte		N05   , An4
	.byte		N05   , Dn5
	.byte	W05
	.byte		VOL   , 34*mus_qz_641_mvl/mxv
	.byte	W01
	.byte		N05   , An4
	.byte		N05   , Dn5
	.byte	W06
	.byte		VOL   , 37*mus_qz_641_mvl/mxv
	.byte		N05   , An4
	.byte		N05   , Dn5
	.byte	W06
	.byte		VOL   , 39*mus_qz_641_mvl/mxv
	.byte		N05   , An4
	.byte		N05   , Dn5
	.byte	W06
	.byte		VOL   , 42*mus_qz_641_mvl/mxv
	.byte		N05   , An4
	.byte		N05   , Dn5
	.byte	W05
	.byte		VOL   , 45*mus_qz_641_mvl/mxv
	.byte	W01
	.byte		N05   , An4
	.byte		N05   , Dn5
	.byte	W06
	.byte		VOL   , 51*mus_qz_641_mvl/mxv
	.byte		N05   , An4
	.byte		N05   , Dn5
	.byte	W05
	.byte		VOL   , 52*mus_qz_641_mvl/mxv
	.byte	W01
	.byte		N05   , An4
	.byte		N05   , Dn5
	.byte	W06
	.byte		VOL   , 56*mus_qz_641_mvl/mxv
	.byte	W96
	.byte	W90
	.byte		VOICE , 11
	.byte	W06
mus_qz_641_5_B1:
	.byte		PAN   , c_v-30
	.byte		VOL   , 64*mus_qz_641_mvl/mxv
	.byte		N11   , Dn1 , v100
	.byte	W12
	.byte		        Dn2
	.byte	W12
	.byte		N23   , An1
	.byte	W24
	.byte		        Cn2
	.byte	W24
	.byte		        Dn2
	.byte	W24
	.byte		        An1
	.byte	W24
	.byte		        An2
	.byte	W24
	.byte		N11   , Dn3
	.byte	W12
	.byte		        Dn2
	.byte	W12
	.byte		        An1
	.byte	W12
	.byte		        An0
	.byte	W12
	.byte		N23   , Cn1
	.byte	W24
	.byte		N11   , Dn1
	.byte	W12
	.byte		        Dn2
	.byte	W12
	.byte		N23   , An1
	.byte	W24
	.byte		        An2
	.byte	W24
	.byte		        Dn3
	.byte	W24
	.byte		N11   , An2
	.byte	W12
	.byte		        An1
	.byte	W12
	.byte		        Cn2
	.byte	W12
	.byte		        Cn1
	.byte	W12
	.byte		N23   , Dn1
	.byte	W24
	.byte		        An1
	.byte	W24
	.byte		        Cn2
	.byte	W24
	.byte		        Dn2
	.byte	W24
	.byte		N11   , An1
	.byte	W12
	.byte		        An0
	.byte	W12
	.byte		N23   , Gn1
	.byte	W24
	.byte		        Cn2
	.byte	W24
	.byte		N11   , An1
	.byte	W12
	.byte		        Dn2
	.byte	W12
	.byte		N23   , Fs1
	.byte	W24
	.byte		        Dn1
	.byte	W24
	.byte		        Dn2
	.byte	W24
	.byte		N11   , Dn1
	.byte	W12
	.byte		        Cn2
	.byte	W12
	.byte		N23   , An1
	.byte	W24
	.byte		N11   , Cn2
	.byte	W12
	.byte		        Dn2
	.byte	W12
	.byte		N23   , An1
	.byte	W24
	.byte		        Dn1
	.byte	W24
	.byte		        Cn2
	.byte	W24
	.byte		        Dn2
	.byte	W24
	.byte		        An1
	.byte	W24
	.byte		N11   , Cn2
	.byte	W12
	.byte		        Cn1
	.byte	W12
	.byte		N23   , Dn2
	.byte	W24
	.byte		N11   , An1
	.byte	W12
	.byte		        Dn1
	.byte	W12
	.byte		        Dn2
	.byte	W12
	.byte		        An1
	.byte	W12
	.byte		N23   , Cn2
	.byte	W24
	.byte		        An1
	.byte	W24
	.byte		N11   , Dn1
	.byte	W12
	.byte		        Dn2
	.byte	W12
	.byte		N23   , An1
	.byte	W24
	.byte		        Dn1
	.byte	W24
	.byte		N11   , An1
	.byte	W12
	.byte		        Dn1
	.byte	W12
	.byte		N23   , Cn2
	.byte	W24
	.byte		N11   , Dn2
	.byte	W12
	.byte		        Gn2
	.byte	W12
	.byte		        An2
	.byte	W12
	.byte		        An1
	.byte	W12
	.byte		        Gn2
	.byte	W12
	.byte		        Cs3
	.byte	W12
	.byte		        Dn3
	.byte	W12
	.byte		        Cn3
	.byte	W12
	.byte		        An2
	.byte	W12
	.byte		        Dn1
	.byte	W12
	.byte		VOL   , 17*mus_qz_641_mvl/mxv
	.byte		PAN   , c_v+32
	.byte		N11   , Dn3
	.byte	W12
	.byte		        Cn3
	.byte	W12
	.byte		        An2
	.byte	W12
	.byte		        Dn1
	.byte	W12
	.byte		VOL   , 11*mus_qz_641_mvl/mxv
	.byte		PAN   , c_v-28
	.byte		N11   , Dn3
	.byte	W12
	.byte		        Cn3
	.byte	W12
	.byte		        An2
	.byte	W12
	.byte		        Dn1
	.byte	W12
	.byte	W72
	.byte	W72
	.byte		VOL   , 82*mus_qz_641_mvl/mxv
	.byte	W06
	.byte		VOICE , 25
	.byte	W66
mus_qz_641_5_001:
	.byte		N05   , Dn0 , v112
	.byte		N05   , Dn1
	.byte	W06
	.byte		        Dn0 , v048
	.byte		N05   , Dn1
	.byte	W06
	.byte		        Dn0 , v112
	.byte		N05   , Dn1
	.byte	W06
	.byte		        Dn0 , v048
	.byte		N05   , Dn1
	.byte	W54
	.byte	PEND
	.byte	W72
	.byte	PATT
	 ptrvalue	mus_qz_641_5_001
	.byte		VOL   , 56*mus_qz_641_mvl/mxv
	.byte	W96
	.byte	W90
	.byte		VOICE , 11
	.byte	W06
	.byte	GOTO
	 ptrvalue	mus_qz_641_5_B1
	.byte	FINE

/********************** Track  6 **********************/

mus_qz_641_6:
	.byte		KEYSH , mus_qz_641_key+0
	.byte		VOICE , 8
	.byte		VOL   , 64*mus_qz_641_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte		        c_v+10
	.byte	W72
	.byte		N68   , An2 , v100 , gtp3
	.byte	W72
	.byte	W72
	.byte		        An2 , v100 , gtp3
	.byte	W72
	.byte	W72
	.byte		        An2 , v100 , gtp3
	.byte	W72
	.byte	W72
	.byte		        An2 , v100 , gtp3
	.byte	W72
	.byte	W96
	.byte	W96
mus_qz_641_6_B1:
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_641_6_B1
	.byte	FINE

/********************** Track  7 **********************/

mus_qz_641_7:
	.byte		KEYSH , mus_qz_641_key+0
	.byte		VOICE , 25
	.byte		VOL   , 64*mus_qz_641_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte		        c_v-11
	.byte		VOL   , 82*mus_qz_641_mvl/mxv
	.byte	W72
	.byte	W72
	.byte		N11   , Dn0 , v127
	.byte		N11   , Dn1
	.byte	W12
	.byte		N05   , Dn0 , v040
	.byte		N05   , Dn1
	.byte	W60
	.byte	W72
	.byte		        Dn0 , v127
	.byte		N05   , Dn1
	.byte	W06
	.byte		        Dn0 , v040
	.byte		N05   , Dn1
	.byte	W06
	.byte		        Dn0 , v127
	.byte		N05   , Dn1
	.byte	W06
	.byte		        Dn0 , v040
	.byte		N05   , Dn1
	.byte	W54
	.byte	W72
	.byte		        Dn0 , v127
	.byte		N05   , Dn1
	.byte	W12
	.byte		        Dn0
	.byte		N05   , Dn1
	.byte	W60
	.byte	W72
	.byte	W96
	.byte	W96
mus_qz_641_7_B1:
	.byte		VOICE , 25
	.byte		VOL   , 82*mus_qz_641_mvl/mxv
	.byte		N11   , Dn1 , v127
	.byte	W24
	.byte		        An0
	.byte	W24
	.byte		        Cn1
	.byte	W24
mus_qz_641_7_000:
	.byte		N11   , Dn1 , v127
	.byte	W24
	.byte		        An0
	.byte	W24
	.byte		        An1
	.byte	W24
	.byte	PEND
mus_qz_641_7_001:
	.byte		N11   , Dn1 , v127
	.byte	W24
	.byte		        An0
	.byte	W24
	.byte		        Cn1
	.byte	W24
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_641_7_000
	.byte	PATT
	 ptrvalue	mus_qz_641_7_001
	.byte	PATT
	 ptrvalue	mus_qz_641_7_000
	.byte	PATT
	 ptrvalue	mus_qz_641_7_001
mus_qz_641_7_002:
	.byte		N11   , Dn1 , v127
	.byte	W24
	.byte		        An0
	.byte	W24
	.byte		        Fs1
	.byte	W24
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_641_7_001
	.byte	PATT
	 ptrvalue	mus_qz_641_7_000
	.byte	PATT
	 ptrvalue	mus_qz_641_7_001
	.byte	PATT
	 ptrvalue	mus_qz_641_7_000
	.byte	PATT
	 ptrvalue	mus_qz_641_7_001
	.byte	PATT
	 ptrvalue	mus_qz_641_7_000
	.byte	PATT
	 ptrvalue	mus_qz_641_7_001
	.byte	PATT
	 ptrvalue	mus_qz_641_7_002
	.byte		VOICE , 8
	.byte		VOL   , 29*mus_qz_641_mvl/mxv
	.byte		PAN   , c_v-26
	.byte		N23   , Cn5 , v100
	.byte	W04
	.byte		PAN   , c_v-18
	.byte	W07
	.byte		        c_v-2
	.byte	W01
	.byte		N23   , Cn4
	.byte	W12
	.byte		        An4
	.byte	W04
	.byte		PAN   , c_v+6
	.byte	W07
	.byte		        c_v+8
	.byte	W01
	.byte		N23   , Cn5
	.byte	W05
	.byte		PAN   , c_v+14
	.byte	W07
	.byte		        c_v+19
	.byte		N23   , Dn5
	.byte	W04
	.byte		PAN   , c_v+24
	.byte	W07
	.byte		        c_v+27
	.byte	W01
	.byte		N23   , Dn4
	.byte	W12
	.byte		        Cn5
	.byte	W11
	.byte		PAN   , c_v+30
	.byte	W01
	.byte		N23   , Dn4
	.byte	W12
	.byte		        Dn5
	.byte	W04
	.byte		PAN   , c_v+27
	.byte	W08
	.byte		N23   , Dn4
	.byte	W05
	.byte		PAN   , c_v+24
	.byte	W07
	.byte		        c_v+22
	.byte		N23   , An4
	.byte	W04
	.byte		PAN   , c_v+16
	.byte	W07
	.byte		        c_v+14
	.byte	W01
	.byte		N23   , Cn5
	.byte	W05
	.byte		PAN   , c_v+6
	.byte	W07
	.byte		N23   , Dn5
	.byte	W04
	.byte		PAN   , c_v+3
	.byte	W07
	.byte		        c_v-5
	.byte	W01
	.byte		N23   , Dn4
	.byte	W12
	.byte		PAN   , c_v-8
	.byte		N23   , An4
	.byte	W04
	.byte		PAN   , c_v-10
	.byte	W08
	.byte		N23   , Dn4
	.byte	W12
	.byte		PAN   , c_v-13
	.byte		N23   , Gn4
	.byte	W04
	.byte		PAN   , c_v-16
	.byte	W08
	.byte		N23   , An4
	.byte	W12
	.byte		PAN   , c_v-18
	.byte		N23   , Dn4
	.byte	W04
	.byte		PAN   , c_v-21
	.byte	W08
	.byte		N23   , Gn4
	.byte	W12
	.byte		        An4
	.byte	W11
	.byte		PAN   , c_v-18
	.byte	W01
	.byte		N23   , Cn5
	.byte	W05
	.byte		PAN   , c_v-16
	.byte	W07
	.byte		N23   , Cn4
	.byte	W12
	.byte		PAN   , c_v-16
	.byte		N23   , Dn4
	.byte	W05
	.byte		PAN   , c_v-10
	.byte	W07
	.byte		N23   , Cn5
	.byte	W04
	.byte		PAN   , c_v-8
	.byte	W07
	.byte		        c_v-5
	.byte	W01
	.byte		N23   , Cn4
	.byte	W05
	.byte		PAN   , c_v-2
	.byte	W07
	.byte		        c_v+0
	.byte		N23   , An4
	.byte	W04
	.byte		PAN   , c_v+3
	.byte	W07
	.byte		        c_v+6
	.byte	W01
	.byte		N23   , Cn5
	.byte	W05
	.byte		PAN   , c_v+8
	.byte	W07
	.byte		N23   , Dn5
	.byte	W04
	.byte		PAN   , c_v+11
	.byte	W08
	.byte		N23   , Dn4
	.byte	W12
	.byte		        Cn5
	.byte	W12
	.byte		        Dn4
	.byte	W12
	.byte		        Dn5
	.byte	W12
	.byte		        Dn4
	.byte	W12
	.byte		        An4
	.byte	W04
	.byte		PAN   , c_v+6
	.byte	W08
	.byte		N23   , Cn5
	.byte	W12
	.byte		PAN   , c_v+0
	.byte		N23   , Dn5
	.byte	W11
	.byte		PAN   , c_v-2
	.byte	W01
	.byte		N23   , Dn4
	.byte	W12
	.byte		PAN   , c_v-8
	.byte		N23   , An4
	.byte	W04
	.byte		PAN   , c_v-13
	.byte	W07
	.byte		        c_v-16
	.byte	W01
	.byte		N23   , Dn4
	.byte	W05
	.byte		PAN   , c_v-18
	.byte	W07
	.byte		N23   , Gn4
	.byte	W11
	.byte		PAN   , c_v-21
	.byte	W01
	.byte		N23   , An4
	.byte	W05
	.byte		PAN   , c_v-24
	.byte	W07
	.byte		N23   , Dn4
	.byte	W12
	.byte		        Gn4
	.byte	W12
	.byte		        An4
	.byte	W12
	.byte		        Cn5
	.byte	W12
	.byte		        Cn4
	.byte	W04
	.byte		PAN   , c_v-18
	.byte	W07
	.byte		        c_v-16
	.byte	W01
	.byte		N88   , Dn4 , v100 , gtp1
	.byte	W12
	.byte	W04
	.byte		PAN   , c_v-10
	.byte	W13
	.byte		        c_v-8
	.byte	W24
	.byte	W01
	.byte		VOL   , 26*mus_qz_641_mvl/mxv
	.byte	W06
	.byte		        23*mus_qz_641_mvl/mxv
	.byte	W04
	.byte		        21*mus_qz_641_mvl/mxv
	.byte	W13
	.byte		        20*mus_qz_641_mvl/mxv
	.byte	W07
	.byte		        12*mus_qz_641_mvl/mxv
	.byte	W04
	.byte		        11*mus_qz_641_mvl/mxv
	.byte	W07
	.byte		        5*mus_qz_641_mvl/mxv
	.byte	W06
	.byte		        1*mus_qz_641_mvl/mxv
	.byte	W07
	.byte		        0*mus_qz_641_mvl/mxv
	.byte	W04
	.byte		        0*mus_qz_641_mvl/mxv
	.byte	W32
	.byte		PAN   , c_v-11
	.byte	W60
	.byte	GOTO
	 ptrvalue	mus_qz_641_7_B1
	.byte	FINE

/********************** Track  8 **********************/

mus_qz_641_8:
	.byte		KEYSH , mus_qz_641_key+0
	.byte		VOICE , 39
	.byte		VOL   , 64*mus_qz_641_mvl/mxv
	.byte		PRIO  , 64
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W72
mus_qz_641_8_000:
	.byte	W48
	.byte		TIE   , As2 , v100
	.byte	W24
	.byte	PEND
	.byte	W96
mus_qz_641_8_001:
	.byte	W48
	.byte		N44   , Gn0 , v100 , gtp3
	.byte	W17
	.byte	PEND
	.byte		EOT   , As2
	.byte	W30
	.byte	W01
mus_qz_641_8_B1:
	.byte		N92   , As2 , v100 , gtp3
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W72
	.byte		        As2 , v100 , gtp3
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W72
	.byte		        As2 , v100 , gtp3
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W72
	.byte		        As2 , v100 , gtp3
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W24
	.byte		N44   , Gn0 , v100 , gtp3
	.byte	W48
	.byte		N92   , As2 , v100 , gtp3
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	W72
	.byte	PATT
	 ptrvalue	mus_qz_641_8_000
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_641_8_001
	.byte		EOT   , As2
	.byte	W30
	.byte	W01
	.byte	GOTO
	 ptrvalue	mus_qz_641_8_B1
	.byte	FINE

/******************************************************/
	.p2align	2

mus_qz_641:
	.byte	8	/* NumTrks */
	.byte	0	/* NumBlks */
	.byte	mus_qz_641_pri	/* Priority */
	.byte	mus_qz_641_rev	/* Reverb. */
space64 4
	ptrvalue	mus_qz_641_grp

	ptrvalue	mus_qz_641_1
	ptrvalue	mus_qz_641_2
	ptrvalue	mus_qz_641_3
	ptrvalue	mus_qz_641_4
	ptrvalue	mus_qz_641_5
	ptrvalue	mus_qz_641_6
	ptrvalue	mus_qz_641_7
	ptrvalue	mus_qz_641_8

	.end
