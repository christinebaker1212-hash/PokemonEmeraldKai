	.include "asm/macros/bit_width.inc"

	.include "MPlayDef.s"

/* Moemon Quetzal song 617, disassembled from the ROM by tools/quetzal_rip/songs.py.
   "org" comments give the ROM address of the bytes that follow. */

	.equ	mus_qz_617_grp, voicegroup_qz_songs
	.equ	mus_qz_617_pri, 0
	.equ	mus_qz_617_rev, reverb_set+0
	.equ	mus_qz_617_mvl, 127
	.equ	mus_qz_617_key, 0
	.equ	mus_qz_617_tbs, 1
	.equ	mus_qz_617_exg, 1
	.equ	mus_qz_617_cmp, 1

	.section .rodata
	.global	mus_qz_617
	.p2align	2
/* org 0x0967411C */

/********************** Track  1 **********************/

mus_qz_617_1:
	.byte		KEYSH , mus_qz_617_key+0
mus_qz_617_1_B1:
	.byte		TEMPO , 88*mus_qz_617_tbs/2
	.byte		VOICE , 13
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte		LFOS  , 38
	.byte		LFODL , 1
	.byte		PAN   , c_v+32
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W60
	.byte		MOD   , 3
	.byte	W03
	.byte		VOL   , 39*mus_qz_617_mvl/mxv
	.byte	W09
	.byte		N23   , Ds3 , v072
	.byte	W21
	.byte		N02   , An3 , v080
	.byte	W03
	.byte		N32   , As3 , v100 , gtp3
	.byte	W36
	.byte		N08   , Gs3 , v092
	.byte	W09
	.byte		N02   , Gs3 , v020
	.byte	W03
	.byte		N44   , Gs3 , v084 , gtp3
	.byte	W22
	.byte		VOL   , 37*mus_qz_617_mvl/mxv
	.byte	W11
	.byte		        33*mus_qz_617_mvl/mxv
	.byte	W03
	.byte		        31*mus_qz_617_mvl/mxv
	.byte	W02
	.byte		        25*mus_qz_617_mvl/mxv
	.byte	W04
	.byte		        20*mus_qz_617_mvl/mxv
	.byte	W02
	.byte		        15*mus_qz_617_mvl/mxv
	.byte	W04
	.byte		        39*mus_qz_617_mvl/mxv
	.byte		N32   , Cs4 , v100 , gtp3
	.byte	W36
	.byte		N11   , Cn4 , v084
	.byte	W12
	.byte		        Cn4 , v020
	.byte	W12
	.byte		        Cn4 , v088
	.byte	W12
	.byte		        Cs4 , v096
	.byte	W12
	.byte		        Ds4 , v100
	.byte	W12
	.byte		        Cn4
	.byte	W12
	.byte		N05   , Gs3 , v088
	.byte	W06
	.byte		        Gs3 , v020
	.byte	W06
	.byte		N11   , Gs3 , v100
	.byte	W12
	.byte		N05   , Fn3 , v064
	.byte	W06
	.byte		N02   , Fn3 , v020
	.byte	W03
	.byte		        Fn3 , v060
	.byte	W03
	.byte		VOL   , 39*mus_qz_617_mvl/mxv
	.byte		N44   , Gs3 , v100 , gtp3
	.byte	W02
	.byte		VOL   , 35*mus_qz_617_mvl/mxv
	.byte	W01
	.byte		        33*mus_qz_617_mvl/mxv
	.byte	W01
	.byte		        33*mus_qz_617_mvl/mxv
	.byte	W03
	.byte		        32*mus_qz_617_mvl/mxv
	.byte	W02
	.byte		        31*mus_qz_617_mvl/mxv
	.byte	W10
	.byte		        32*mus_qz_617_mvl/mxv
	.byte	W02
	.byte		        33*mus_qz_617_mvl/mxv
	.byte	W01
	.byte		        33*mus_qz_617_mvl/mxv
	.byte	W24
	.byte	W02
	.byte		        39*mus_qz_617_mvl/mxv
	.byte		N11   , Cn4 , v084
	.byte	W12
	.byte		        Ds4 , v100
	.byte	W12
	.byte		        Cn4
	.byte	W12
	.byte		N28   , As3 , v080 , gtp1
	.byte	W15
	.byte		VOL   , 33*mus_qz_617_mvl/mxv
	.byte	W03
	.byte		        31*mus_qz_617_mvl/mxv
	.byte	W02
	.byte		        27*mus_qz_617_mvl/mxv
	.byte	W04
	.byte		        23*mus_qz_617_mvl/mxv
	.byte	W02
	.byte		        21*mus_qz_617_mvl/mxv
	.byte	W04
	.byte		N05   , As3 , v020
	.byte	W02
	.byte		VOL   , 28*mus_qz_617_mvl/mxv
	.byte	W04
	.byte		        39*mus_qz_617_mvl/mxv
	.byte		N23   , Gn4 , v100
	.byte	W24
	.byte		N02   , An4 , v076
	.byte	W03
	.byte		N32   , Fn4 , v100
	.byte		N32   , As4
	.byte	W32
	.byte	W01
	.byte		N08   , Ds4 , v076
	.byte		N08   , Gs4
	.byte	W09
	.byte		N02   , Ds4 , v020
	.byte		N02   , Gs4
	.byte	W03
	.byte		N44   , Ds4 , v072 , gtp3
	.byte		N44   , Gs4 , v072 , gtp3
	.byte	W36
	.byte	W01
	.byte		VOL   , 37*mus_qz_617_mvl/mxv
	.byte	W03
	.byte		        36*mus_qz_617_mvl/mxv
	.byte	W02
	.byte		        33*mus_qz_617_mvl/mxv
	.byte	W01
	.byte		        32*mus_qz_617_mvl/mxv
	.byte	W01
	.byte		        31*mus_qz_617_mvl/mxv
	.byte	W04
	.byte		        39*mus_qz_617_mvl/mxv
	.byte		N32   , Fs4 , v092 , gtp3
	.byte		N32   , Cs5 , v092 , gtp3
	.byte	W36
	.byte		N11   , Ds4 , v080
	.byte		N11   , Cn5
	.byte	W12
	.byte		        Ds4 , v020
	.byte		N11   , Cn5
	.byte	W12
	.byte		        Fs4 , v092
	.byte		N11   , Cn5
	.byte	W12
	.byte		        An4
	.byte		N11   , Cs5
	.byte	W12
	.byte		N05   , Cn5 , v100
	.byte		N05   , Ds5
	.byte	W06
	.byte		N02   , Fn4 , v080
	.byte	W03
	.byte		        Gs4
	.byte	W03
	.byte		        Cn5
	.byte	W03
	.byte		N08   , Cn5 , v100
	.byte		N08   , Fn5
	.byte	W09
	.byte		N11   , Gs4 , v076
	.byte		N11   , Cn5
	.byte	W12
	.byte		        Gs4 , v080
	.byte		N11   , Cn5
	.byte	W12
	.byte		N08   , Cs4 , v068
	.byte		N08   , Gs4
	.byte	W09
	.byte		N02   , Cs4 , v020
	.byte		N02   , Gs4
	.byte	W03
	.byte		N11   , Cs4 , v100
	.byte		N11   , Gs4
	.byte	W12
	.byte		        Gs3 , v080
	.byte		N11   , Fn4
	.byte	W12
	.byte		        As3 , v092
	.byte		N11   , Gn4
	.byte	W12
	.byte		N05   , Cn4 , v100
	.byte		N05   , Gs4
	.byte	W06
	.byte		N02   , Ds4 , v080
	.byte	W03
	.byte		        Gn4
	.byte	W03
	.byte		        Bn4
	.byte	W03
	.byte		N32   , Gn4 , v100
	.byte		N32   , Cn5
	.byte	W32
	.byte	W01
	.byte		N08   , Ds4
	.byte		N08   , As4
	.byte	W09
	.byte		N02   , Ds4 , v020
	.byte		N02   , As4
	.byte	W03
	.byte		N40   , Ds4 , v100 , gtp1
	.byte		N40   , As4 , v100 , gtp1
	.byte	W18
	.byte		VOL   , 36*mus_qz_617_mvl/mxv
	.byte	W02
	.byte		        35*mus_qz_617_mvl/mxv
	.byte	W04
	.byte		        33*mus_qz_617_mvl/mxv
	.byte	W02
	.byte		        31*mus_qz_617_mvl/mxv
	.byte	W04
	.byte		        30*mus_qz_617_mvl/mxv
	.byte	W02
	.byte		        27*mus_qz_617_mvl/mxv
	.byte	W04
	.byte		        25*mus_qz_617_mvl/mxv
	.byte	W02
	.byte		        23*mus_qz_617_mvl/mxv
	.byte	W04
	.byte		        21*mus_qz_617_mvl/mxv
	.byte		N11   , Ds4 , v020
	.byte		N11   , As4
	.byte	W06
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W84
	.byte		VOL   , 39*mus_qz_617_mvl/mxv
	.byte	W12
	.byte		N23   , As3 , v100
	.byte	W24
	.byte		        An3
	.byte	W24
	.byte		        Cs4
	.byte	W24
	.byte		        Ds4
	.byte	W24
	.byte		N32   , Fn4 , v100 , gtp3
	.byte	W36
	.byte		N05   , Cn4 , v088
	.byte	W06
	.byte		        Cn4 , v020
	.byte	W06
	.byte		        Cn4 , v088
	.byte	W06
	.byte		        Cn4 , v020
	.byte	W06
	.byte		N08   , Cn4 , v084
	.byte	W09
	.byte		N02   , En4 , v100
	.byte	W03
	.byte		N11   , Fn4
	.byte	W12
	.byte		N05   , Cn4 , v084
	.byte	W06
	.byte		N02   , Cn4 , v020
	.byte	W03
	.byte		        Bn3 , v100
	.byte	W03
	.byte		N11   , Cn4
	.byte	W12
	.byte		N08   , As3
	.byte	W09
	.byte		N02   , As3 , v020
	.byte	W03
	.byte		N11   , An3 , v088
	.byte	W12
	.byte		N32   , As3 , v100 , gtp3
	.byte	W18
	.byte		VOL   , 36*mus_qz_617_mvl/mxv
	.byte	W02
	.byte		        33*mus_qz_617_mvl/mxv
	.byte	W06
	.byte		        31*mus_qz_617_mvl/mxv
	.byte	W04
	.byte		        28*mus_qz_617_mvl/mxv
	.byte	W02
	.byte		        27*mus_qz_617_mvl/mxv
	.byte	W04
	.byte		        39*mus_qz_617_mvl/mxv
	.byte		N11   , Gn3 , v068
	.byte	W12
	.byte		        Cs4 , v100
	.byte	W12
	.byte		N32   , Ds4 , v100 , gtp3
	.byte	W36
	.byte		N05   , As3 , v088
	.byte	W06
	.byte		        As3 , v020
	.byte	W06
	.byte		        As3 , v088
	.byte	W06
	.byte		        As3 , v020
	.byte	W06
	.byte		N08   , As3 , v088
	.byte	W09
	.byte		N02   , Dn4 , v100
	.byte	W03
	.byte		N11   , Ds4
	.byte	W12
	.byte		N05   , As3
	.byte	W06
	.byte		N02   , As3 , v020
	.byte	W03
	.byte		        Fs4 , v068
	.byte	W03
	.byte		N32   , Gn4 , v092 , gtp3
	.byte	W36
	.byte		N11   , Gs4
	.byte	W12
	.byte		N23   , Fn4 , v084
	.byte	W24
	.byte		N11   , Fn3 , v076
	.byte	W12
	.byte		N08   , Gs3 , v084
	.byte	W09
	.byte		N02   , Cn4 , v072
	.byte	W03
	.byte		N56   , Cs4 , v100 , gtp3
	.byte	W36
	.byte		VOL   , 35*mus_qz_617_mvl/mxv
	.byte	W06
	.byte		        33*mus_qz_617_mvl/mxv
	.byte	W02
	.byte		        30*mus_qz_617_mvl/mxv
	.byte	W04
	.byte		        28*mus_qz_617_mvl/mxv
	.byte	W02
	.byte		        25*mus_qz_617_mvl/mxv
	.byte	W04
	.byte		        21*mus_qz_617_mvl/mxv
	.byte	W06
	.byte		        39*mus_qz_617_mvl/mxv
	.byte		N11   , Ds4
	.byte	W12
	.byte		        Cs4 , v096
	.byte	W12
	.byte		        Cn4 , v088
	.byte	W12
	.byte		N44   , As3 , v100 , gtp3
	.byte	W30
	.byte		VOL   , 35*mus_qz_617_mvl/mxv
	.byte	W02
	.byte		        33*mus_qz_617_mvl/mxv
	.byte	W04
	.byte		        31*mus_qz_617_mvl/mxv
	.byte	W02
	.byte		        28*mus_qz_617_mvl/mxv
	.byte	W04
	.byte		        27*mus_qz_617_mvl/mxv
	.byte	W02
	.byte		        21*mus_qz_617_mvl/mxv
	.byte	W04
	.byte		        18*mus_qz_617_mvl/mxv
	.byte		N11   , As3 , v020
	.byte	W02
	.byte		VOL   , 13*mus_qz_617_mvl/mxv
	.byte	W10
	.byte		        39*mus_qz_617_mvl/mxv
	.byte		N11   , Gn3 , v068
	.byte	W12
	.byte		        Gs3 , v076
	.byte	W12
	.byte		        As3 , v080
	.byte	W12
	.byte		N92   , Cn4 , v080 , gtp3
	.byte	W56
	.byte	W01
	.byte		VOL   , 35*mus_qz_617_mvl/mxv
	.byte	W05
	.byte		        33*mus_qz_617_mvl/mxv
	.byte	W04
	.byte		        30*mus_qz_617_mvl/mxv
	.byte	W02
	.byte		        23*mus_qz_617_mvl/mxv
	.byte	W04
	.byte		        18*mus_qz_617_mvl/mxv
	.byte	W06
	.byte		        14*mus_qz_617_mvl/mxv
	.byte	W02
	.byte		        10*mus_qz_617_mvl/mxv
	.byte	W06
	.byte		        6*mus_qz_617_mvl/mxv
	.byte	W04
	.byte		        4*mus_qz_617_mvl/mxv
	.byte	W02
	.byte		        2*mus_qz_617_mvl/mxv
	.byte	W04
	.byte	GOTO
	 ptrvalue	mus_qz_617_1_B1
	.byte	FINE

/********************** Track  2 **********************/

mus_qz_617_2:
	.byte		KEYSH , mus_qz_617_key+0
mus_qz_617_2_B1:
	.byte		VOICE , 13
	.byte		PAN   , c_v+0
	.byte		LFOS  , 38
	.byte		PRIO  , 55
	.byte		LFODL , 1
	.byte		PAN   , c_v-32
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W56
	.byte	W01
	.byte		VOL   , 19*mus_qz_617_mvl/mxv
	.byte	W03
	.byte		MOD   , 9
	.byte	W15
	.byte		N23   , Ds3 , v100
	.byte	W21
	.byte		N02   , An3
	.byte	W03
	.byte		N32   , As3 , v100 , gtp3
	.byte	W36
	.byte		N08   , Gs3
	.byte	W09
	.byte		N02   , Gs3 , v020
	.byte	W03
	.byte		N44   , Gs3 , v100 , gtp3
	.byte	W36
	.byte		VOL   , 17*mus_qz_617_mvl/mxv
	.byte	W03
	.byte		        13*mus_qz_617_mvl/mxv
	.byte	W02
	.byte		        5*mus_qz_617_mvl/mxv
	.byte	W04
	.byte		        3*mus_qz_617_mvl/mxv
	.byte	W03
	.byte		        19*mus_qz_617_mvl/mxv
	.byte		N32   , Cs4 , v100 , gtp3
	.byte	W36
	.byte		N11   , Cn4
	.byte	W12
	.byte		        Cn4 , v020
	.byte	W12
	.byte		        Cn4 , v100
	.byte	W12
	.byte		        Cs4
	.byte	W12
	.byte		        Ds4
	.byte	W09
	.byte	W03
	.byte		        Cn4
	.byte	W12
	.byte		N05   , Gs3
	.byte	W06
	.byte		        Gs3 , v020
	.byte	W06
	.byte		N11   , Gs3 , v100
	.byte	W12
	.byte		N05   , Fn3 , v080
	.byte	W06
	.byte		N02   , Fn3 , v020
	.byte	W03
	.byte		        Fn3 , v060
	.byte	W03
	.byte		VOL   , 9*mus_qz_617_mvl/mxv
	.byte		N32   , Gs3 , v100 , gtp1
	.byte	W05
	.byte		VOL   , 5*mus_qz_617_mvl/mxv
	.byte	W07
	.byte		        4*mus_qz_617_mvl/mxv
	.byte	W05
	.byte		        1*mus_qz_617_mvl/mxv
	.byte	W07
	.byte		        0*mus_qz_617_mvl/mxv
	.byte	W05
	.byte		        0*mus_qz_617_mvl/mxv
	.byte	W16
	.byte	W03
	.byte		        19*mus_qz_617_mvl/mxv
	.byte		N11   , Cn4
	.byte	W12
	.byte		        Ds4
	.byte	W12
	.byte		        Cn4
	.byte	W12
	.byte		N28   , As3 , v100 , gtp1
	.byte	W12
	.byte		VOL   , 8*mus_qz_617_mvl/mxv
	.byte	W05
	.byte		        5*mus_qz_617_mvl/mxv
	.byte	W07
	.byte		        2*mus_qz_617_mvl/mxv
	.byte	W05
	.byte		        1*mus_qz_617_mvl/mxv
	.byte	W01
	.byte		N05   , As3 , v020
	.byte	W06
	.byte		VOL   , 16*mus_qz_617_mvl/mxv
	.byte		N23   , Gn4 , v100
	.byte	W21
	.byte	W03
	.byte		N02   , An4
	.byte	W03
	.byte		N32   , Fn4
	.byte		N32   , As4
	.byte	W32
	.byte	W01
	.byte		N08   , Ds4
	.byte		N08   , Gs4
	.byte	W09
	.byte		N02   , Ds4 , v020
	.byte		N02   , Gs4
	.byte	W03
	.byte		N44   , Ds4 , v100 , gtp3
	.byte		N44   , Gs4 , v100 , gtp3
	.byte	W36
	.byte		VOL   , 5*mus_qz_617_mvl/mxv
	.byte	W05
	.byte		        1*mus_qz_617_mvl/mxv
	.byte	W04
	.byte	W03
	.byte		        16*mus_qz_617_mvl/mxv
	.byte		N32   , Fs4 , v100 , gtp3
	.byte		N32   , Cs5 , v100 , gtp3
	.byte	W36
	.byte		N11   , Ds4
	.byte		N11   , Cn5
	.byte	W12
	.byte		        Ds4 , v020
	.byte		N11   , Cn5
	.byte	W12
	.byte		        Fs4 , v100
	.byte		N11   , Cn5
	.byte	W12
	.byte		        An4
	.byte		N11   , Cs5
	.byte	W12
	.byte		N05   , Cn5
	.byte		N05   , Ds5
	.byte	W06
	.byte		N02   , Fn4
	.byte	W03
	.byte		        Gs4
	.byte	W03
	.byte		        Cn5
	.byte	W03
	.byte	N08
	.byte		N08   , Fn5
	.byte	W09
	.byte		N11   , Gs4
	.byte		N11   , Cn5
	.byte	W12
	.byte		        Gs4
	.byte		N11   , Cn5
	.byte	W12
	.byte		N08   , Cs4
	.byte		N08   , Gs4
	.byte	W09
	.byte		N02   , Cs4 , v020
	.byte		N02   , Gs4
	.byte	W03
	.byte		N11   , Cs4 , v100
	.byte		N11   , Gs4
	.byte	W12
	.byte		        Gs3
	.byte		N11   , Fn4
	.byte	W12
	.byte		        As3
	.byte		N11   , Gn4
	.byte	W12
	.byte		N05   , Cn4
	.byte		N05   , Gs4
	.byte	W06
	.byte		N02   , Ds4
	.byte	W03
	.byte		        Gn4
	.byte	W03
	.byte		        Bn4
	.byte	W03
	.byte		N32   , Gn4
	.byte		N32   , Cn5
	.byte	W32
	.byte	W01
	.byte		N08   , Ds4
	.byte		N08   , As4
	.byte	W09
	.byte		N02   , Ds4 , v020
	.byte		N02   , As4
	.byte	W03
	.byte		N40   , Ds4 , v100 , gtp1
	.byte		N40   , As4 , v100 , gtp1
	.byte	W12
	.byte		VOL   , 13*mus_qz_617_mvl/mxv
	.byte	W05
	.byte		        8*mus_qz_617_mvl/mxv
	.byte	W07
	.byte		        5*mus_qz_617_mvl/mxv
	.byte	W05
	.byte		        3*mus_qz_617_mvl/mxv
	.byte	W07
	.byte		        1*mus_qz_617_mvl/mxv
	.byte	W05
	.byte		        0*mus_qz_617_mvl/mxv
	.byte	W01
	.byte		N11   , Ds4 , v020
	.byte		N11   , As4
	.byte	W03
	.byte	W03
	.byte		VOL   , 0*mus_qz_617_mvl/mxv
	.byte	W92
	.byte	W01
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W68
	.byte	W01
	.byte		        19*mus_qz_617_mvl/mxv
	.byte	W24
	.byte	W03
	.byte	W03
	.byte		N23   , As3 , v100
	.byte	W24
	.byte		        An3
	.byte	W24
	.byte		        Cs4
	.byte	W24
	.byte		        Ds4
	.byte	W21
	.byte	W03
	.byte		N32   , Fn4 , v100 , gtp3
	.byte	W36
	.byte		N05   , Cn4
	.byte	W06
	.byte		        Cn4 , v020
	.byte	W06
	.byte		        Cn4 , v100
	.byte	W06
	.byte		        Cn4 , v020
	.byte	W06
	.byte		N08   , Cn4 , v100
	.byte	W09
	.byte		N02   , En4
	.byte	W03
	.byte		N11   , Fn4
	.byte	W12
	.byte		N05   , Cn4
	.byte	W06
	.byte		N02   , Cn4 , v020
	.byte	W03
	.byte		        Bn3 , v100
	.byte	W03
	.byte		N11   , Cn4
	.byte	W12
	.byte		N08   , As3
	.byte	W09
	.byte		N02   , As3 , v020
	.byte	W03
	.byte		N11   , An3 , v100
	.byte	W12
	.byte		N32   , As3 , v100 , gtp3
	.byte	W18
	.byte		VOL   , 10*mus_qz_617_mvl/mxv
	.byte	W06
	.byte		        4*mus_qz_617_mvl/mxv
	.byte	W05
	.byte		        1*mus_qz_617_mvl/mxv
	.byte	W07
	.byte		        19*mus_qz_617_mvl/mxv
	.byte		N11   , Gn3 , v068
	.byte	W12
	.byte		        Cs4 , v100
	.byte	W09
	.byte	W03
	.byte		N32   , Ds4 , v100 , gtp3
	.byte	W36
	.byte		N05   , As3
	.byte	W06
	.byte		        As3 , v020
	.byte	W06
	.byte		        As3 , v100
	.byte	W06
	.byte		        As3 , v020
	.byte	W06
	.byte		N08   , As3 , v100
	.byte	W09
	.byte		N02   , Dn4
	.byte	W03
	.byte		N11   , Ds4
	.byte	W12
	.byte		N05   , As3
	.byte	W06
	.byte		N02   , As3 , v020
	.byte	W03
	.byte		        Fs4 , v100
	.byte	W03
	.byte		N32   , Gn4 , v100 , gtp3
	.byte	W36
	.byte		N11   , Gs4
	.byte	W12
	.byte		N23   , Fn4
	.byte	W24
	.byte		N11   , Fn3
	.byte	W12
	.byte		N08   , Gs3
	.byte	W09
	.byte		N02   , Cn4
	.byte	W03
	.byte		N56   , Cs4 , v100 , gtp3
	.byte	W36
	.byte		VOL   , 12*mus_qz_617_mvl/mxv
	.byte	W05
	.byte		        7*mus_qz_617_mvl/mxv
	.byte	W07
	.byte		        5*mus_qz_617_mvl/mxv
	.byte	W05
	.byte		        6*mus_qz_617_mvl/mxv
	.byte	W07
	.byte		        19*mus_qz_617_mvl/mxv
	.byte		N11   , Ds4
	.byte	W12
	.byte		        Cs4
	.byte	W12
	.byte		        Cn4
	.byte	W09
	.byte	W03
	.byte		N44   , As3 , v100 , gtp3
	.byte	W30
	.byte		VOL   , 10*mus_qz_617_mvl/mxv
	.byte	W06
	.byte		        7*mus_qz_617_mvl/mxv
	.byte	W05
	.byte		        4*mus_qz_617_mvl/mxv
	.byte	W07
	.byte		N11   , As3 , v020
	.byte	W12
	.byte		VOL   , 19*mus_qz_617_mvl/mxv
	.byte		N11   , Gn3 , v060
	.byte	W12
	.byte		        Gs3 , v100
	.byte	W12
	.byte		        As3
	.byte	W09
	.byte	W03
	.byte		N92   , Cn4
	.byte	W54
	.byte		VOL   , 11*mus_qz_617_mvl/mxv
	.byte	W06
	.byte		        9*mus_qz_617_mvl/mxv
	.byte	W05
	.byte		        4*mus_qz_617_mvl/mxv
	.byte	W07
	.byte		        2*mus_qz_617_mvl/mxv
	.byte	W05
	.byte		        0*mus_qz_617_mvl/mxv
	.byte	W07
	.byte		        0*mus_qz_617_mvl/mxv
	.byte	W09
	.byte	GOTO
	 ptrvalue	mus_qz_617_2_B1
	.byte	FINE

/********************** Track  3 **********************/

mus_qz_617_3:
	.byte		KEYSH , mus_qz_617_key+0
mus_qz_617_3_B1:
	.byte		VOICE , 14
	.byte		VOL   , 57*mus_qz_617_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte		        c_v-16
	.byte		VOL   , 62*mus_qz_617_mvl/mxv
	.byte		N92   , Gs1 , v100 , gtp3
	.byte	W12
	.byte		N23   , Ds2
	.byte	W12
	.byte		        As2
	.byte	W12
	.byte		N56   , Cn3 , v100 , gtp3
	.byte	W60
	.byte		N92   , Gs1 , v100 , gtp3
	.byte	W12
	.byte		N23   , Fn2
	.byte	W12
	.byte		        Cn3
	.byte	W12
	.byte		N56   , Cs3 , v100 , gtp3
	.byte	W60
mus_qz_617_3_000:
	.byte		N92   , Gs1 , v100 , gtp3
	.byte	W12
	.byte		N23   , Ds2
	.byte	W12
	.byte		        As2
	.byte	W12
	.byte		N56   , Cn3 , v100 , gtp3
	.byte	W60
	.byte	PEND
	.byte		N23   , Ds3
	.byte	W03
	.byte		N20   , Gn3
	.byte	W21
	.byte		N23   , Dn3
	.byte	W03
	.byte		N20   , Gs3
	.byte	W21
	.byte		N44   , Cs3 , v100 , gtp3
	.byte	W03
	.byte		N20   , As3
	.byte	W21
	.byte		N23   , Gn2
	.byte	W12
	.byte		N11   , Ds2
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_617_3_000
	.byte		N32   , An1 , v100 , gtp3
	.byte	W12
	.byte		        Ds2 , v100 , gtp3
	.byte	W12
	.byte		        Fs2 , v100 , gtp3
	.byte	W12
	.byte		N23   , As2
	.byte	W24
	.byte		N11   , An2
	.byte	W12
	.byte		        As2
	.byte	W12
	.byte		        Cn3
	.byte	W12
	.byte		N32   , As1 , v100 , gtp3
	.byte	W12
	.byte		N23   , Fn2
	.byte	W12
	.byte		        Cn3
	.byte	W12
	.byte		N32   , Cs3 , v100 , gtp3
	.byte	W12
	.byte		N44   , En3 , v100 , gtp3
	.byte	W48
	.byte		N32   , Ds2 , v100 , gtp3
	.byte	W12
	.byte		N23   , As2
	.byte	W12
	.byte		        Cs3
	.byte	W12
	.byte		        En3
	.byte	W12
	.byte		N11   , Gn3
	.byte	W12
	.byte		N05   , Ds3
	.byte	W06
	.byte		N28   , En3 , v100 , gtp1
	.byte	W06
	.byte		N23   , Bn3
	.byte	W24
	.byte		        Gs1
	.byte		N02   , Fs3
	.byte	W03
	.byte		N32   , Gn3
	.byte	W09
	.byte		N23   , Ds2
	.byte	W12
	.byte		        As2
	.byte	W12
	.byte		        Cn3
	.byte		N11   , Gs3
	.byte	W12
	.byte		N23   , Ds3
	.byte		N11   , Cn4
	.byte	W12
	.byte		N23   , Gs3
	.byte		N32   , Ds4 , v100 , gtp3
	.byte	W12
	.byte		N23   , As3
	.byte	W12
	.byte		N11   , Cn4
	.byte	W12
	.byte		N23   , An1
	.byte	W12
	.byte		        Fs2
	.byte	W12
	.byte		        As2
	.byte	W12
	.byte		        An2
	.byte	W12
	.byte		        Cn3
	.byte	W12
	.byte		        As2
	.byte	W12
	.byte		        Cs3
	.byte	W12
	.byte		N11   , Ds3
	.byte	W12
	.byte		N44   , Cs2 , v100 , gtp3
	.byte		N23   , Fn3
	.byte	W24
	.byte		        Gs3
	.byte	W24
	.byte		        Fn3
	.byte	W12
	.byte		        Gs2
	.byte	W12
	.byte		        Cs3
	.byte	W12
	.byte		N11   , Fn3
	.byte	W12
	.byte		N23   , Ds1
	.byte		N02   , Cn4
	.byte	W03
	.byte		N32   , Cs4
	.byte	W09
	.byte		N23   , As1
	.byte	W12
	.byte		        Ds2
	.byte	W12
	.byte		        Dn2
	.byte	W12
	.byte		        Fn2
	.byte	W12
	.byte		        Ds2
	.byte	W12
	.byte		        Gn2
	.byte	W12
	.byte		N11   , As2
	.byte	W12
	.byte		N23   , Cs2
	.byte	W12
	.byte		N11   , Gs2
	.byte	W12
	.byte		        Fn3
	.byte	W12
	.byte		N05   , Cs2
	.byte		N23   , Cn4
	.byte	W12
	.byte		        Cs2
	.byte	W12
	.byte		N11   , Gs2
	.byte	W12
	.byte		        Fn3
	.byte	W12
	.byte		N05   , Gs1
	.byte		N23   , Cs4
	.byte	W12
	.byte		        Cs2
	.byte	W12
	.byte		N11   , Gn2
	.byte	W12
	.byte		        Ds3
	.byte	W12
	.byte		N05   , Cs2
	.byte		N23   , As3
	.byte	W12
	.byte		N05   , Cs2
	.byte	W12
	.byte	N11
	.byte		N11   , Gn2
	.byte	W12
	.byte		        Ds3
	.byte	W12
	.byte		N05   , Cs2
	.byte		N23   , As3
	.byte	W12
	.byte		        Cn2
	.byte	W12
	.byte		N11   , Gn2
	.byte	W12
	.byte		        Ds3
	.byte	W12
	.byte		N05   , Gn2
	.byte		N23   , Gs3
	.byte	W12
	.byte		N05   , Cn2
	.byte	W12
	.byte	N11
	.byte		N11   , Gn2
	.byte	W12
	.byte		        Ds3
	.byte	W12
	.byte		N05   , Gn2
	.byte		N23   , Cn4
	.byte	W12
	.byte		        Fn2
	.byte	W12
	.byte		N11   , Ds3
	.byte	W12
	.byte		        Cn4
	.byte	W12
	.byte		N08   , Fn2
	.byte		N23   , Fn4
	.byte	W12
	.byte		N08   , En2
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		        Ds2
	.byte	W12
	.byte		        Dn2
	.byte	W12
	.byte		N23   , As1
	.byte	W12
	.byte		N11   , Gs2
	.byte	W12
	.byte		        Fn3
	.byte	W12
	.byte		N05   , Fn2
	.byte		N11   , As3
	.byte	W12
	.byte		N08   , As1
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		        Cn2
	.byte	W12
	.byte		        Cs2
	.byte	W12
	.byte		N23   , Ds2
	.byte	W12
	.byte		N11   , As2
	.byte	W12
	.byte		        Ds3
	.byte	W12
	.byte		N05   , As2
	.byte		N11   , Gn3
	.byte	W12
	.byte		N08   , Ds2
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		        Fn2
	.byte	W12
	.byte		N05   , Fs2
	.byte	W06
	.byte		        Gn2
	.byte	W06
	.byte		N23   , Gs2
	.byte	W24
	.byte		        Cn3
	.byte	W24
	.byte		        Ds3
	.byte	W24
	.byte		        Gn3
	.byte	W24
	.byte		        Fs3
	.byte	W24
	.byte		        Ds3
	.byte	W24
	.byte		        Cn3
	.byte	W24
	.byte		        An2
	.byte	W24
	.byte		VOL   , 50*mus_qz_617_mvl/mxv
	.byte		N23   , Cs1
	.byte	W12
	.byte		N11   , Gs2
	.byte	W12
	.byte		        Cs3
	.byte		N02   , Fn3
	.byte	W03
	.byte		        Gs3
	.byte	W03
	.byte		N05   , Fn3
	.byte	W06
	.byte		        Cs1
	.byte		N05   , Cs3
	.byte	W06
	.byte		        Gs2
	.byte	W06
	.byte		N23   , Cs1
	.byte	W12
	.byte		N11   , Gs2
	.byte	W12
	.byte		        Cs3
	.byte		N02   , Fn3
	.byte	W03
	.byte		        Gs3
	.byte	W03
	.byte		N05   , Fn3
	.byte	W06
	.byte		        Gs1
	.byte		N05   , Cs3
	.byte	W06
	.byte		        Fn2
	.byte	W06
	.byte		N23   , Cs1
	.byte	W12
	.byte		N11   , Gs2
	.byte	W12
	.byte		        Cs3
	.byte		N02   , En3
	.byte	W03
	.byte		        Gs3
	.byte	W03
	.byte		N05   , En3
	.byte	W06
	.byte		        Cs1
	.byte		N05   , Cs3
	.byte	W06
	.byte		        Gs2
	.byte	W06
	.byte		        Cs1
	.byte	W06
	.byte		        Gs2
	.byte	W06
	.byte		N11   , Cs1
	.byte		N05   , Cs3
	.byte	W06
	.byte		        Gs2
	.byte	W06
	.byte		        En3
	.byte	W06
	.byte		        Gs2
	.byte	W06
	.byte		        Cs1
	.byte		N11   , En3
	.byte	W12
	.byte		N23   , Cn1
	.byte	W06
	.byte		N05   , Gn2
	.byte	W06
	.byte		        As2
	.byte	W06
	.byte		        Gn2
	.byte	W06
	.byte		N11   , Ds3
	.byte		N02   , Gn3
	.byte	W03
	.byte		        As3
	.byte	W03
	.byte		N05   , Gn3
	.byte	W06
	.byte		        Gn1
	.byte		N05   , Ds3
	.byte	W06
	.byte		        As2
	.byte	W06
	.byte		        Cn1
	.byte	W06
	.byte		        Gn2
	.byte	W06
	.byte		N11   , Cn1
	.byte		N05   , As2
	.byte	W06
	.byte		        Gn2
	.byte	W06
	.byte		N11   , Ds3
	.byte		N02   , Gn3
	.byte	W03
	.byte		        As3
	.byte	W03
	.byte		N05   , Gn3
	.byte	W06
	.byte		        Gn1
	.byte		N05   , Ds3
	.byte	W06
	.byte		        En3
	.byte	W06
	.byte		N23   , Fn1
	.byte		N11   , Fn3
	.byte	W06
	.byte		        Cn3
	.byte	W06
	.byte		        Gs2
	.byte	W06
	.byte		        Fn2
	.byte	W06
	.byte		        Cn3
	.byte	W06
	.byte		        Gs2
	.byte	W06
	.byte		N05   , Fn1
	.byte		N11   , Fn2
	.byte	W06
	.byte		        Cn3
	.byte	W06
	.byte		N05   , En1
	.byte		N11   , Gs3
	.byte	W06
	.byte		        Fn3
	.byte	W06
	.byte		N05   , En1
	.byte		N11   , Cn3
	.byte	W06
	.byte		        Gs2
	.byte	W06
	.byte		N05   , Ds1
	.byte		N11   , Fn3
	.byte	W06
	.byte		        Cn3
	.byte	W06
	.byte		N05   , Dn1
	.byte		N11   , Gs2
	.byte	W06
	.byte		        Fn2
	.byte	W06
	.byte		N23   , As0
	.byte	W12
	.byte		N17   , Fn2
	.byte	W12
	.byte		N11   , Cs3
	.byte	W06
	.byte		        Fn2
	.byte	W06
	.byte		N05   , Fn1
	.byte		N05   , Gs2
	.byte	W06
	.byte		N02   , Cn3
	.byte	W03
	.byte		        Cs3
	.byte	W03
	.byte		N05   , As0
	.byte		N11   , Ds3
	.byte	W06
	.byte		        Fn2
	.byte	W06
	.byte		N05   , As0
	.byte		N11   , Gn3
	.byte	W12
	.byte		N05   , Cn1
	.byte		N11   , Fn3
	.byte	W12
	.byte		N05   , Cs1
	.byte		N11   , Ds3
	.byte	W12
	.byte		N23   , Ds1
	.byte		N11   , Cs3
	.byte	W12
	.byte		        Gn2
	.byte	W06
	.byte		N02   , Ds3
	.byte	W03
	.byte		        Fn3
	.byte	W03
	.byte		N11   , Gn3
	.byte	W12
	.byte		N05   , As1
	.byte		N11   , Gs3
	.byte	W12
	.byte		N05   , Ds1
	.byte		N23   , Gn3
	.byte	W12
	.byte		N05   , Ds1
	.byte	W12
	.byte		        Fn1
	.byte		N11   , Cn4
	.byte	W12
	.byte		N05   , Gn1
	.byte		N11   , Cs4
	.byte	W09
	.byte		N02   , Gn1
	.byte	W03
	.byte		N23   , Gs1
	.byte		N92   , Ds4 , v100 , gtp3
	.byte	W24
	.byte		N23   , Ds1
	.byte	W24
	.byte		        Cn2
	.byte	W24
	.byte		        As1
	.byte	W24
	.byte	GOTO
	 ptrvalue	mus_qz_617_3_B1
	.byte	FINE

/********************** Track  4 **********************/

mus_qz_617_4:
	.byte		KEYSH , mus_qz_617_key+0
mus_qz_617_4_B1:
	.byte		VOL   , 57*mus_qz_617_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		BENDR , 6
	.byte		PRIO  , 64
	.byte		VOL   , 33*mus_qz_617_mvl/mxv
	.byte		PAN   , c_v-34
	.byte		BEND  , c_v+1
	.byte	W03
	.byte		VOICE , 35
	.byte	W44
	.byte	W01
	.byte		VOL   , 33*mus_qz_617_mvl/mxv
	.byte	W12
	.byte		N02   , Gs3 , v100
	.byte	W03
	.byte		        Ds4
	.byte	W03
	.byte		        As4
	.byte	W03
	.byte		N68   , Cn5 , v100 , gtp3
	.byte	W24
	.byte	W03
	.byte		VOL   , 33*mus_qz_617_mvl/mxv
	.byte	W06
	.byte		        33*mus_qz_617_mvl/mxv
	.byte	W02
	.byte		        33*mus_qz_617_mvl/mxv
	.byte	W04
	.byte		        28*mus_qz_617_mvl/mxv
	.byte	W02
	.byte		        24*mus_qz_617_mvl/mxv
	.byte	W04
	.byte		        19*mus_qz_617_mvl/mxv
	.byte	W02
	.byte		        13*mus_qz_617_mvl/mxv
	.byte	W04
	.byte		        7*mus_qz_617_mvl/mxv
	.byte	W02
	.byte		        1*mus_qz_617_mvl/mxv
	.byte	W04
	.byte		        0*mus_qz_617_mvl/mxv
	.byte	W24
	.byte		        33*mus_qz_617_mvl/mxv
	.byte	W06
	.byte		N02   , Gs3
	.byte	W03
	.byte		        Fn4
	.byte	W03
	.byte		        Cn5
	.byte	W03
	.byte		N68   , Cs5
	.byte	W24
	.byte	W03
	.byte		VOL   , 33*mus_qz_617_mvl/mxv
	.byte	W06
	.byte		        33*mus_qz_617_mvl/mxv
	.byte	W02
	.byte		        33*mus_qz_617_mvl/mxv
	.byte	W04
	.byte		        28*mus_qz_617_mvl/mxv
	.byte	W02
	.byte		        24*mus_qz_617_mvl/mxv
	.byte	W04
	.byte		        19*mus_qz_617_mvl/mxv
	.byte	W02
	.byte		        13*mus_qz_617_mvl/mxv
	.byte	W04
	.byte		        7*mus_qz_617_mvl/mxv
	.byte	W02
	.byte		        1*mus_qz_617_mvl/mxv
	.byte	W04
	.byte		        0*mus_qz_617_mvl/mxv
	.byte	W24
	.byte	W03
	.byte		        48*mus_qz_617_mvl/mxv
	.byte	W03
	.byte		N11   , As4
	.byte	W12
	.byte		        Gs4
	.byte	W12
	.byte		        En4
	.byte	W12
	.byte		N23   , Ds4
	.byte	W24
	.byte		        Fn4
	.byte	W24
	.byte		N40   , Gn4 , v100 , gtp1
	.byte	W18
	.byte		VOL   , 36*mus_qz_617_mvl/mxv
	.byte	W02
	.byte		        28*mus_qz_617_mvl/mxv
	.byte	W04
	.byte		        25*mus_qz_617_mvl/mxv
	.byte	W02
	.byte		        19*mus_qz_617_mvl/mxv
	.byte	W04
	.byte		        12*mus_qz_617_mvl/mxv
	.byte	W02
	.byte		        8*mus_qz_617_mvl/mxv
	.byte	W04
	.byte		        1*mus_qz_617_mvl/mxv
	.byte	W02
	.byte		        0*mus_qz_617_mvl/mxv
	.byte	W10
	.byte	W96
	.byte		        33*mus_qz_617_mvl/mxv
	.byte	W60
	.byte		N11   , Ds4
	.byte	W12
	.byte		        Fn4
	.byte	W12
	.byte		        Fs4
	.byte	W12
	.byte		N44   , Fn4
	.byte	W48
	.byte		N11   , En4
	.byte	W12
	.byte		        Ds4
	.byte	W12
	.byte		        Cs4
	.byte	W12
	.byte		N32   , Gs4 , v100 , gtp3
	.byte	W12
	.byte	W24
	.byte		N11   , An4
	.byte	W12
	.byte		N44   , As4 , v100 , gtp3
	.byte	W12
	.byte		VOL   , 33*mus_qz_617_mvl/mxv
	.byte	W06
	.byte		        33*mus_qz_617_mvl/mxv
	.byte	W02
	.byte		        33*mus_qz_617_mvl/mxv
	.byte	W04
	.byte		        28*mus_qz_617_mvl/mxv
	.byte	W02
	.byte		        24*mus_qz_617_mvl/mxv
	.byte	W04
	.byte		        19*mus_qz_617_mvl/mxv
	.byte	W02
	.byte		        13*mus_qz_617_mvl/mxv
	.byte	W04
	.byte		        7*mus_qz_617_mvl/mxv
	.byte	W02
	.byte		        1*mus_qz_617_mvl/mxv
	.byte	W04
	.byte		        0*mus_qz_617_mvl/mxv
	.byte	W18
	.byte		        33*mus_qz_617_mvl/mxv
	.byte	W24
	.byte		N23   , Ds4
	.byte	W24
	.byte		N11   , Cn4
	.byte	W12
	.byte		        Gs3
	.byte	W12
	.byte		        As3
	.byte	W12
	.byte		        Gs3
	.byte	W12
	.byte		N32   , Fs3 , v100 , gtp3
	.byte	W36
	.byte		N11   , Fn3
	.byte	W24
	.byte		        Ds3
	.byte	W12
	.byte		        Fn3
	.byte	W12
	.byte		        Fs3
	.byte	W12
	.byte		N23   , Gs3
	.byte	W24
	.byte		N32   , Fn3 , v100 , gtp3
	.byte	W36
	.byte		N11   , Cs3
	.byte	W12
	.byte		        Fn3
	.byte	W12
	.byte		        Gs3
	.byte	W12
	.byte		N32   , Ds3 , v100 , gtp3
	.byte	W18
	.byte		VOL   , 31*mus_qz_617_mvl/mxv
	.byte	W02
	.byte		        29*mus_qz_617_mvl/mxv
	.byte	W04
	.byte		        26*mus_qz_617_mvl/mxv
	.byte	W02
	.byte		        20*mus_qz_617_mvl/mxv
	.byte	W04
	.byte		        14*mus_qz_617_mvl/mxv
	.byte	W02
	.byte		        10*mus_qz_617_mvl/mxv
	.byte	W04
	.byte		        5*mus_qz_617_mvl/mxv
	.byte	W02
	.byte		        0*mus_qz_617_mvl/mxv
	.byte	W10
	.byte		        62*mus_qz_617_mvl/mxv
	.byte		N11   , Fn4 , v084
	.byte	W12
	.byte		        Ds4
	.byte	W12
	.byte		        Cn4
	.byte	W12
	.byte		N08   , Ds4 , v088
	.byte	W09
	.byte		N02   , En4 , v056
	.byte	W03
	.byte		N44   , Fn4 , v100 , gtp3
	.byte	W32
	.byte	W01
	.byte		VOL   , 58*mus_qz_617_mvl/mxv
	.byte	W03
	.byte		        54*mus_qz_617_mvl/mxv
	.byte	W02
	.byte		        50*mus_qz_617_mvl/mxv
	.byte	W04
	.byte		        45*mus_qz_617_mvl/mxv
	.byte	W02
	.byte		        43*mus_qz_617_mvl/mxv
	.byte	W04
	.byte		        62*mus_qz_617_mvl/mxv
	.byte		N11   , Fn3 , v084
	.byte	W12
	.byte		        Gs3 , v092
	.byte	W12
	.byte		        Cn4 , v100
	.byte	W12
	.byte		N32   , As3 , v100 , gtp3
	.byte	W12
	.byte	W24
	.byte		N23   , Gs3 , v092
	.byte	W24
	.byte		N17   , Gn3 , v080
	.byte	W18
	.byte		N05   , Gn3 , v020
	.byte	W03
	.byte		N02   , Cn4 , v072
	.byte	W03
	.byte		N23   , Gs3 , v100
	.byte		N23   , Cs4
	.byte	W24
	.byte		N32   , As3 , v100 , gtp3
	.byte		N32   , Ds4 , v100 , gtp3
	.byte	W36
	.byte		        Gs3 , v092 , gtp3
	.byte		N32   , Cs4 , v092 , gtp3
	.byte	W36
	.byte		N23   , Gn3 , v088
	.byte		N23   , Cn4
	.byte	W24
	.byte		N11   , Fn3 , v100
	.byte		N11   , As3
	.byte	W12
	.byte		        Cn3 , v088
	.byte		N11   , Gs3
	.byte	W12
	.byte		        En3 , v080
	.byte		N11   , Gn3
	.byte	W12
	.byte		        Fn3 , v076
	.byte		N11   , Gs3
	.byte	W12
	.byte		        Fn3 , v020
	.byte		N11   , Gs3
	.byte	W12
	.byte		        En3 , v100
	.byte		N11   , Gn3
	.byte	W12
	.byte		        En3 , v020
	.byte		N11   , Gn3
	.byte	W12
	.byte		        Cn3 , v100
	.byte		N08   , Gs3
	.byte	W09
	.byte		N02   , En3
	.byte	W03
	.byte		N44   , Gs2 , v100 , gtp3
	.byte		N44   , Fn3 , v100 , gtp3
	.byte	W30
	.byte		VOL   , 59*mus_qz_617_mvl/mxv
	.byte	W02
	.byte		        52*mus_qz_617_mvl/mxv
	.byte	W06
	.byte		        47*mus_qz_617_mvl/mxv
	.byte	W04
	.byte		        42*mus_qz_617_mvl/mxv
	.byte	W02
	.byte		        36*mus_qz_617_mvl/mxv
	.byte	W04
	.byte		        30*mus_qz_617_mvl/mxv
	.byte		N08   , Gs2 , v020
	.byte		N08   , Fn3
	.byte	W02
	.byte		VOL   , 28*mus_qz_617_mvl/mxv
	.byte	W10
	.byte		        62*mus_qz_617_mvl/mxv
	.byte		N11   , Fn3 , v100
	.byte		N11   , Cs4
	.byte	W12
	.byte		        Ds3 , v088
	.byte		N11   , Cn4
	.byte	W12
	.byte		        Cs3 , v080
	.byte		N08   , As3
	.byte	W09
	.byte		N02   , Gn3 , v064
	.byte	W03
	.byte		N32   , Ds3 , v100 , gtp3
	.byte		N32   , Gs3 , v100 , gtp3
	.byte	W36
	.byte		N11   , As2
	.byte		N11   , Gn3
	.byte	W12
	.byte		        As2 , v020
	.byte		N11   , Gn3
	.byte	W12
	.byte		        As2 , v076
	.byte		N11   , Gs3
	.byte	W12
	.byte		        Ds3 , v084
	.byte		N11   , As3
	.byte	W12
	.byte		        Gn3 , v092
	.byte		N11   , Bn3
	.byte	W12
	.byte		TIE   , Gs3 , v100
	.byte		TIE   , Cn4
	.byte	W60
	.byte		VOL   , 52*mus_qz_617_mvl/mxv
	.byte	W05
	.byte		        48*mus_qz_617_mvl/mxv
	.byte	W03
	.byte		        41*mus_qz_617_mvl/mxv
	.byte	W06
	.byte		        37*mus_qz_617_mvl/mxv
	.byte	W03
	.byte		        32*mus_qz_617_mvl/mxv
	.byte	W03
	.byte		        28*mus_qz_617_mvl/mxv
	.byte	W04
	.byte		        27*mus_qz_617_mvl/mxv
	.byte	W02
	.byte		        23*mus_qz_617_mvl/mxv
	.byte	W03
	.byte		        18*mus_qz_617_mvl/mxv
	.byte	W03
	.byte		        14*mus_qz_617_mvl/mxv
	.byte	W04
	.byte		        8*mus_qz_617_mvl/mxv
	.byte	W02
	.byte		        4*mus_qz_617_mvl/mxv
	.byte	W03
	.byte		        2*mus_qz_617_mvl/mxv
	.byte	W03
	.byte		        0*mus_qz_617_mvl/mxv
	.byte	W06
	.byte		EOT   , Gs3
	.byte		        Cn4
	.byte	W80
	.byte	W02
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_617_4_B1
	.byte	FINE

/********************** Track  5 **********************/

mus_qz_617_5:
	.byte		KEYSH , mus_qz_617_key+0
mus_qz_617_5_B1:
	.byte		VOL   , 57*mus_qz_617_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		BENDR , 6
	.byte		PRIO  , 55
	.byte		VOL   , 15*mus_qz_617_mvl/mxv
	.byte		PAN   , c_v+43
	.byte		BEND  , c_v+0
	.byte	W03
	.byte		VOICE , 35
	.byte	W60
	.byte		N02   , Gs3 , v100
	.byte	W03
	.byte		        Ds4
	.byte	W03
	.byte		        As4
	.byte	W03
	.byte		N14   , Cn5
	.byte	W15
	.byte		PAN   , c_v-35
	.byte	N02
	.byte	W03
	.byte		        Cs5
	.byte	W03
	.byte		        Ds5
	.byte	W03
	.byte		N60   , Gs5
	.byte	W32
	.byte	W01
	.byte		VOL   , 10*mus_qz_617_mvl/mxv
	.byte	W06
	.byte		        6*mus_qz_617_mvl/mxv
	.byte	W05
	.byte		        1*mus_qz_617_mvl/mxv
	.byte	W07
	.byte		        0*mus_qz_617_mvl/mxv
	.byte	W12
	.byte		PAN   , c_v+43
	.byte		VOL   , 15*mus_qz_617_mvl/mxv
	.byte		N02   , Gs3
	.byte	W03
	.byte		        Fn4
	.byte	W03
	.byte		        Cn5
	.byte	W03
	.byte		N14   , Cs5
	.byte	W15
	.byte		PAN   , c_v-35
	.byte	N02
	.byte	W03
	.byte		        Ds5
	.byte	W03
	.byte		        Gs5
	.byte	W03
	.byte		N54   , Cn6
	.byte	W21
	.byte		VOL   , 13*mus_qz_617_mvl/mxv
	.byte	W06
	.byte		        10*mus_qz_617_mvl/mxv
	.byte	W05
	.byte		        5*mus_qz_617_mvl/mxv
	.byte	W07
	.byte		        4*mus_qz_617_mvl/mxv
	.byte	W05
	.byte		        0*mus_qz_617_mvl/mxv
	.byte	W07
	.byte		        0*mus_qz_617_mvl/mxv
	.byte	W09
	.byte		        15*mus_qz_617_mvl/mxv
	.byte		BEND  , c_v+1
	.byte	W03
	.byte		PAN   , c_v+43
	.byte		N11   , As4
	.byte	W12
	.byte		        Gs4
	.byte	W12
	.byte		        En4
	.byte	W09
	.byte	W03
	.byte		N23   , Ds4
	.byte	W24
	.byte		        Fn4
	.byte	W24
	.byte		N40   , Gn4 , v100 , gtp1
	.byte	W18
	.byte		VOL   , 9*mus_qz_617_mvl/mxv
	.byte	W06
	.byte		        5*mus_qz_617_mvl/mxv
	.byte	W05
	.byte		        2*mus_qz_617_mvl/mxv
	.byte	W07
	.byte		        0*mus_qz_617_mvl/mxv
	.byte	W05
	.byte		        0*mus_qz_617_mvl/mxv
	.byte	W04
	.byte	W96
	.byte	W03
	.byte		        15*mus_qz_617_mvl/mxv
	.byte	W60
	.byte		N11   , Ds4
	.byte	W12
	.byte		        Fn4
	.byte	W12
	.byte		        Fs4
	.byte	W09
	.byte	W03
	.byte		N44   , Fn4
	.byte	W48
	.byte		N11   , En4
	.byte	W12
	.byte		        Ds4
	.byte	W12
	.byte		        Cs4
	.byte	W12
	.byte		N32   , Gs4 , v100 , gtp3
	.byte	W09
	.byte	W24
	.byte	W03
	.byte		N11   , An4
	.byte	W12
	.byte		N44   , As4 , v100 , gtp3
	.byte	W12
	.byte		VOL   , 10*mus_qz_617_mvl/mxv
	.byte	W05
	.byte		        7*mus_qz_617_mvl/mxv
	.byte	W07
	.byte		        4*mus_qz_617_mvl/mxv
	.byte	W05
	.byte		        2*mus_qz_617_mvl/mxv
	.byte	W07
	.byte		        1*mus_qz_617_mvl/mxv
	.byte	W05
	.byte		        0*mus_qz_617_mvl/mxv
	.byte	W16
	.byte	W03
	.byte		        15*mus_qz_617_mvl/mxv
	.byte	W24
	.byte		N23   , Ds4
	.byte	W24
	.byte		N11   , Cn4
	.byte	W12
	.byte		        Gs3
	.byte	W12
	.byte		        As3
	.byte	W12
	.byte		        Gs3
	.byte	W09
	.byte	W03
	.byte		N32   , Fs3 , v100 , gtp3
	.byte	W36
	.byte		N11   , Fn3
	.byte	W24
	.byte		        Ds3
	.byte	W12
	.byte		        Fn3
	.byte	W12
	.byte		        Fs3
	.byte	W09
	.byte	W03
	.byte		N23   , Gs3
	.byte	W24
	.byte		N32   , Fn3 , v100 , gtp3
	.byte	W36
	.byte		N11   , Cs3
	.byte	W12
	.byte		        Fn3
	.byte	W12
	.byte		        Gs3
	.byte	W09
	.byte	W03
	.byte		N32   , Ds3 , v100 , gtp3
	.byte	W18
	.byte		VOL   , 7*mus_qz_617_mvl/mxv
	.byte	W06
	.byte		        5*mus_qz_617_mvl/mxv
	.byte	W05
	.byte		        2*mus_qz_617_mvl/mxv
	.byte	W07
	.byte		        1*mus_qz_617_mvl/mxv
	.byte	W05
	.byte		        0*mus_qz_617_mvl/mxv
	.byte	W07
	.byte		        18*mus_qz_617_mvl/mxv
	.byte		N11   , Fn4
	.byte	W12
	.byte		        Ds4
	.byte	W12
	.byte		        Cn4
	.byte	W12
	.byte		N08   , Ds4
	.byte	W09
	.byte		N02   , En4
	.byte	W03
	.byte		N44   , Fn4 , v100 , gtp3
	.byte	W42
	.byte		VOL   , 10*mus_qz_617_mvl/mxv
	.byte	W06
	.byte		        18*mus_qz_617_mvl/mxv
	.byte		N11   , Fn3
	.byte	W12
	.byte		        Gs3
	.byte	W12
	.byte		        Cn4
	.byte	W12
	.byte		N32   , As3 , v100 , gtp3
	.byte	W09
	.byte	W24
	.byte	W03
	.byte		N23   , Gs3
	.byte	W24
	.byte		N17   , Gn3
	.byte	W18
	.byte		N05   , Gn3 , v020
	.byte	W03
	.byte		N02   , Cn4 , v100
	.byte	W03
	.byte		N23   , Gs3
	.byte		N23   , Cs4
	.byte	W21
	.byte	W03
	.byte		N32   , As3 , v100 , gtp3
	.byte		N32   , Ds4 , v100 , gtp3
	.byte	W36
	.byte		        Gs3 , v100 , gtp3
	.byte		N32   , Cs4 , v100 , gtp3
	.byte	W36
	.byte		N23   , Gn3
	.byte		N23   , Cn4
	.byte	W21
	.byte	W03
	.byte		N11   , Fn3
	.byte		N11   , As3
	.byte	W12
	.byte		        Cn3
	.byte		N11   , Gs3
	.byte	W12
	.byte		        En3
	.byte		N11   , Gn3
	.byte	W12
	.byte		        Fn3
	.byte		N11   , Gs3
	.byte	W12
	.byte		        Fn3 , v020
	.byte		N11   , Gs3
	.byte	W12
	.byte		        En3 , v100
	.byte		N11   , Gn3
	.byte	W12
	.byte		        En3 , v020
	.byte		N11   , Gn3
	.byte	W12
	.byte		        Cn3 , v100
	.byte		N08   , Gs3
	.byte	W09
	.byte		N02   , En3
	.byte	W03
	.byte		N44   , Gs2 , v100 , gtp3
	.byte		N44   , Fn3 , v100 , gtp3
	.byte	W30
	.byte		VOL   , 7*mus_qz_617_mvl/mxv
	.byte	W06
	.byte		        4*mus_qz_617_mvl/mxv
	.byte	W05
	.byte		        1*mus_qz_617_mvl/mxv
	.byte	W07
	.byte		        0*mus_qz_617_mvl/mxv
	.byte		N08   , Gs2 , v020
	.byte		N08   , Fn3
	.byte	W05
	.byte		VOL   , 0*mus_qz_617_mvl/mxv
	.byte	W07
	.byte		        18*mus_qz_617_mvl/mxv
	.byte		N11   , Fn3 , v100
	.byte		N11   , Cs4
	.byte	W12
	.byte		        Ds3
	.byte		N11   , Cn4
	.byte	W12
	.byte		        Cs3
	.byte		N08   , As3
	.byte	W09
	.byte		N02   , Gn3
	.byte	W03
	.byte		N32   , Ds3 , v100 , gtp3
	.byte		N32   , Gs3 , v100 , gtp3
	.byte	W36
	.byte		N11   , As2
	.byte		N11   , Gn3
	.byte	W12
	.byte		        As2 , v020
	.byte		N11   , Gn3
	.byte	W12
	.byte		        As2 , v100
	.byte		N11   , Gs3
	.byte	W12
	.byte		        Ds3
	.byte		N11   , As3
	.byte	W12
	.byte		        Gn3
	.byte		N11   , Bn3
	.byte	W09
	.byte	W03
	.byte		TIE   , Gs3
	.byte		TIE   , Cn4
	.byte	W60
	.byte		VOL   , 7*mus_qz_617_mvl/mxv
	.byte	W05
	.byte		        4*mus_qz_617_mvl/mxv
	.byte	W07
	.byte		        2*mus_qz_617_mvl/mxv
	.byte	W05
	.byte		        0*mus_qz_617_mvl/mxv
	.byte	W12
	.byte		        0*mus_qz_617_mvl/mxv
	.byte	W04
	.byte	W17
	.byte		EOT   , Gs3
	.byte		        Cn4
	.byte	W78
	.byte	W01
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_617_5_B1
	.byte	FINE

/********************** Track  6 **********************/

mus_qz_617_6:
	.byte		KEYSH , mus_qz_617_key+0
mus_qz_617_6_B1:
	.byte		VOL   , 57*mus_qz_617_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte		VOL   , 17*mus_qz_617_mvl/mxv
	.byte		PAN   , c_v+40
	.byte	W12
	.byte		VOICE , 35
	.byte	W72
	.byte		N02   , Cn5 , v100
	.byte	W03
	.byte		        Cs5
	.byte	W03
	.byte		        Ds5
	.byte	W03
	.byte		N66   , Gs5 , v100 , gtp1
	.byte	W03
	.byte	W32
	.byte	W01
	.byte		VOL   , 16*mus_qz_617_mvl/mxv
	.byte	W03
	.byte		        12*mus_qz_617_mvl/mxv
	.byte	W02
	.byte		        11*mus_qz_617_mvl/mxv
	.byte	W03
	.byte		        8*mus_qz_617_mvl/mxv
	.byte	W03
	.byte		        7*mus_qz_617_mvl/mxv
	.byte	W04
	.byte		        6*mus_qz_617_mvl/mxv
	.byte	W02
	.byte		        5*mus_qz_617_mvl/mxv
	.byte	W03
	.byte		        3*mus_qz_617_mvl/mxv
	.byte	W03
	.byte		        1*mus_qz_617_mvl/mxv
	.byte	W04
	.byte		        0*mus_qz_617_mvl/mxv
	.byte	W02
	.byte		        0*mus_qz_617_mvl/mxv
	.byte	W10
	.byte		        17*mus_qz_617_mvl/mxv
	.byte	W12
	.byte		N02   , Cs5
	.byte	W03
	.byte		        Ds5
	.byte	W03
	.byte		        Gs5
	.byte	W03
	.byte		N54   , Cn6 , v100 , gtp1
	.byte	W03
	.byte	W15
	.byte		VOL   , 17*mus_qz_617_mvl/mxv
	.byte	W02
	.byte		        13*mus_qz_617_mvl/mxv
	.byte	W03
	.byte		        11*mus_qz_617_mvl/mxv
	.byte	W04
	.byte		        10*mus_qz_617_mvl/mxv
	.byte	W02
	.byte		        8*mus_qz_617_mvl/mxv
	.byte	W03
	.byte		        6*mus_qz_617_mvl/mxv
	.byte	W03
	.byte		        5*mus_qz_617_mvl/mxv
	.byte	W04
	.byte		        3*mus_qz_617_mvl/mxv
	.byte	W02
	.byte		        2*mus_qz_617_mvl/mxv
	.byte	W03
	.byte		        1*mus_qz_617_mvl/mxv
	.byte	W03
	.byte		        0*mus_qz_617_mvl/mxv
	.byte	W06
	.byte		        0*mus_qz_617_mvl/mxv
	.byte	W10
	.byte		        17*mus_qz_617_mvl/mxv
	.byte		N11   , Fn4
	.byte	W12
	.byte		        Cn4
	.byte	W12
	.byte		        Gs3
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
	.byte	W84
	.byte	W03
	.byte		VOICE , 23
	.byte	W03
	.byte		VOL   , 18*mus_qz_617_mvl/mxv
	.byte	W06
	.byte		PAN   , c_v+6
	.byte		N96   , Ds5 , v044
	.byte	W96
	.byte		N48   , Fn5
	.byte	W48
	.byte		        Cn5
	.byte	W48
	.byte		        Cs5
	.byte	W48
	.byte		        Fn4
	.byte	W48
	.byte		        Gn4
	.byte	W48
	.byte		        Cs5
	.byte	W48
	.byte		PAN   , c_v-14
	.byte		N44   , Cn5 , v100 , gtp3
	.byte	W48
	.byte		PAN   , c_v-28
	.byte		N44   , Ds5 , v100 , gtp3
	.byte	W48
	.byte		VOL   , 23*mus_qz_617_mvl/mxv
	.byte		N11   , Fs5
	.byte	W06
	.byte		        Ds5
	.byte	W06
	.byte		        Cn5
	.byte	W06
	.byte		        An4
	.byte	W06
	.byte		        Ds5
	.byte	W06
	.byte		        Cn5
	.byte	W06
	.byte		        An4
	.byte	W06
	.byte		        Fs4
	.byte	W06
	.byte		N05   , Cn5
	.byte	W03
	.byte		        An4
	.byte	W03
	.byte		        Fs4
	.byte	W03
	.byte		        Ds4
	.byte	W03
	.byte		        Cn4
	.byte	W03
	.byte		        An3
	.byte	W03
	.byte		        Fs3
	.byte	W03
	.byte		        Ds3
	.byte	W03
	.byte		        Fs4
	.byte	W03
	.byte		        Ds4
	.byte	W03
	.byte		        Cn4
	.byte	W03
	.byte		        An3
	.byte	W03
	.byte		        Fs3
	.byte	W03
	.byte		        Ds3
	.byte	W03
	.byte		        Cn3
	.byte	W03
	.byte		        En3
	.byte	W03
	.byte		N11   , Fn3
	.byte	W12
	.byte		N56   , Gs2 , v100 , gtp3
	.byte		N56   , Cs3 , v100 , gtp3
	.byte	W60
	.byte	N23
	.byte		N23   , Gs3
	.byte	W24
	.byte		N44   , Gs2 , v100 , gtp3
	.byte		N44   , En3 , v100 , gtp3
	.byte	W48
	.byte		        Cs3 , v100 , gtp3
	.byte		N11   , As3
	.byte	W12
	.byte		N23   , Gs3
	.byte	W24
	.byte		N11   , En3
	.byte	W12
	.byte		N44   , As2 , v100 , gtp3
	.byte		N68   , Ds3 , v100 , gtp3
	.byte	W48
	.byte		N44   , Gn3 , v100 , gtp3
	.byte	W24
	.byte		N23   , As3
	.byte	W24
	.byte		        Fn3
	.byte		N23   , Cn4
	.byte	W24
	.byte		        Cn3
	.byte		N23   , Gs3
	.byte	W24
	.byte		N44   , Gs2 , v100 , gtp3
	.byte		N23   , En3
	.byte	W24
	.byte		        Fn3
	.byte	W24
	.byte		N44   , As2 , v100 , gtp3
	.byte		N44   , Gs3 , v100 , gtp3
	.byte	W48
	.byte		        Gs2 , v100 , gtp3
	.byte		N44   , Fn3 , v100 , gtp3
	.byte	W48
	.byte		N23   , Gn2
	.byte		N23   , Ds3
	.byte	W24
	.byte		        As1
	.byte		N23   , Gn2
	.byte	W24
	.byte		        Ds2
	.byte		N23   , Gs2
	.byte	W24
	.byte		        Gn2
	.byte		N23   , As2
	.byte	W24
	.byte		N92   , Gs2 , v100 , gtp3
	.byte		N92   , Ds3 , v100 , gtp3
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_617_6_B1
	.byte	FINE

/********************** Track  7 **********************/

mus_qz_617_7:
	.byte		KEYSH , mus_qz_617_key+0
mus_qz_617_7_B1:
	.byte		VOICE , 39
	.byte		VOL   , 57*mus_qz_617_mvl/mxv
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
	.byte		        44*mus_qz_617_mvl/mxv
	.byte	W12
	.byte		N05   , Gs4 , v100
	.byte	W12
	.byte		N23   , An4
	.byte	W36
	.byte		N05   , Gs4
	.byte	W12
	.byte		N23   , An4
	.byte	W24
mus_qz_617_7_000:
	.byte	W12
	.byte		N05   , Gs4 , v100
	.byte	W12
	.byte		N23   , An4
	.byte	W36
	.byte		N05   , Gs4
	.byte	W12
	.byte		N23   , An4
	.byte	W24
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_617_7_000
	.byte	PATT
	 ptrvalue	mus_qz_617_7_000
	.byte	PATT
	 ptrvalue	mus_qz_617_7_000
	.byte	PATT
	 ptrvalue	mus_qz_617_7_000
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_617_7_B1
	.byte	FINE

/******************************************************/
	.p2align	2

mus_qz_617:
	.byte	7	/* NumTrks */
	.byte	0	/* NumBlks */
	.byte	mus_qz_617_pri	/* Priority */
	.byte	mus_qz_617_rev	/* Reverb. */
space64 4
	ptrvalue	mus_qz_617_grp

	ptrvalue	mus_qz_617_1
	ptrvalue	mus_qz_617_2
	ptrvalue	mus_qz_617_3
	ptrvalue	mus_qz_617_4
	ptrvalue	mus_qz_617_5
	ptrvalue	mus_qz_617_6
	ptrvalue	mus_qz_617_7

	.end
