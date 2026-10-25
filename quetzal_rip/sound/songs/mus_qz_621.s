	.include "asm/macros/bit_width.inc"

	.include "MPlayDef.s"

/* Moemon Quetzal song 621, disassembled from the ROM by tools/quetzal_rip/songs.py.
   "org" comments give the ROM address of the bytes that follow. */

	.equ	mus_qz_621_grp, voicegroup_qz_songs
	.equ	mus_qz_621_pri, 0
	.equ	mus_qz_621_rev, reverb_set+0
	.equ	mus_qz_621_mvl, 127
	.equ	mus_qz_621_key, 0
	.equ	mus_qz_621_tbs, 1
	.equ	mus_qz_621_exg, 1
	.equ	mus_qz_621_cmp, 1

	.section .rodata
	.global	mus_qz_621
	.p2align	2
/* org 0x0967872C */

/********************** Track  1 **********************/

mus_qz_621_1:
	.byte		KEYSH , mus_qz_621_key+0
	.byte		TEMPO , 130*mus_qz_621_tbs/2
	.byte		VOICE , 29
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte		VOL   , 83*mus_qz_621_mvl/mxv
	.byte	W96
	.byte		PAN   , c_v+8
	.byte	W72
	.byte		VOL   , 76*mus_qz_621_mvl/mxv
	.byte		N23   , Fs3 , v100
	.byte	W24
mus_qz_621_1_000:
	.byte		N44   , Bn3 , v100 , gtp3
	.byte	W48
	.byte		N05   , Bn3 , v048
	.byte	W12
	.byte		        Fs3 , v100
	.byte	W06
	.byte		        Fs3 , v048
	.byte	W06
	.byte		        Fs3 , v100
	.byte	W06
	.byte		        Fs3 , v048
	.byte	W06
	.byte		        Bn3 , v100
	.byte	W06
	.byte		        Bn3 , v048
	.byte	W06
	.byte	PEND
mus_qz_621_1_B1:
	.byte		VOL   , 76*mus_qz_621_mvl/mxv
	.byte		N23   , Cs4 , v100
	.byte	W24
	.byte		        Bn3
	.byte	W24
	.byte		        An3
	.byte	W18
	.byte		VOL   , 69*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        64*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        60*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        54*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        53*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        65*mus_qz_621_mvl/mxv
	.byte		N23   , En3
	.byte	W01
	.byte		VOL   , 73*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        75*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        76*mus_qz_621_mvl/mxv
	.byte	W16
	.byte		        75*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        72*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        67*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        61*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        68*mus_qz_621_mvl/mxv
	.byte		TIE   , Fs3
	.byte	W01
	.byte		VOL   , 71*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        72*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        73*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        75*mus_qz_621_mvl/mxv
	.byte	W03
	.byte		        76*mus_qz_621_mvl/mxv
	.byte	W11
	.byte		        76*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        75*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        74*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        73*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        72*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        71*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        70*mus_qz_621_mvl/mxv
	.byte	W04
	.byte		        69*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        68*mus_qz_621_mvl/mxv
	.byte	W03
	.byte		        67*mus_qz_621_mvl/mxv
	.byte	W06
	.byte		        66*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        67*mus_qz_621_mvl/mxv
	.byte	W06
	.byte		        68*mus_qz_621_mvl/mxv
	.byte	W08
	.byte		        69*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        70*mus_qz_621_mvl/mxv
	.byte	W07
	.byte		        71*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        72*mus_qz_621_mvl/mxv
	.byte	W04
	.byte		        73*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        74*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        75*mus_qz_621_mvl/mxv
	.byte	W03
	.byte		        76*mus_qz_621_mvl/mxv
	.byte	W18
	.byte	W08
	.byte		        75*mus_qz_621_mvl/mxv
	.byte	W06
	.byte		        73*mus_qz_621_mvl/mxv
	.byte	W03
	.byte		        67*mus_qz_621_mvl/mxv
	.byte	W03
	.byte		        57*mus_qz_621_mvl/mxv
	.byte	W04
	.byte		        50*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        50*mus_qz_621_mvl/mxv
	.byte		        37*mus_qz_621_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        24*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        20*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        18*mus_qz_621_mvl/mxv
	.byte		        8*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        7*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        5*mus_qz_621_mvl/mxv
	.byte	W01
	.byte	EOT
	.byte	W01
	.byte		VOL   , 3*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        2*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        0*mus_qz_621_mvl/mxv
	.byte	W28
	.byte	W01
	.byte		        83*mus_qz_621_mvl/mxv
	.byte	W05
	.byte	N23
	.byte	W24
	.byte	PATT
	 ptrvalue	mus_qz_621_1_000
	.byte		N23   , Cs4 , v100
	.byte	W24
	.byte		        Bn3
	.byte	W24
	.byte		        Cs4
	.byte	W16
	.byte		VOL   , 81*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        73*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        59*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        54*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        41*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        64*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        77*mus_qz_621_mvl/mxv
	.byte		N23   , En4
	.byte	W01
	.byte		VOL   , 81*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        83*mus_qz_621_mvl/mxv
	.byte	W22
	.byte		        59*mus_qz_621_mvl/mxv
	.byte		TIE   , Ds4
	.byte	W01
	.byte		VOL   , 66*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        73*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        79*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        83*mus_qz_621_mvl/mxv
	.byte	W21
	.byte		        81*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        81*mus_qz_621_mvl/mxv
	.byte	W04
	.byte		        79*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        78*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        77*mus_qz_621_mvl/mxv
	.byte	W03
	.byte		        76*mus_qz_621_mvl/mxv
	.byte	W05
	.byte		        75*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        74*mus_qz_621_mvl/mxv
	.byte	W05
	.byte		        73*mus_qz_621_mvl/mxv
	.byte	W14
	.byte		        72*mus_qz_621_mvl/mxv
	.byte	W09
	.byte		        73*mus_qz_621_mvl/mxv
	.byte	W09
	.byte		        74*mus_qz_621_mvl/mxv
	.byte	W05
	.byte		        75*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        76*mus_qz_621_mvl/mxv
	.byte	W04
	.byte		        77*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        78*mus_qz_621_mvl/mxv
	.byte	W02
	.byte	W02
	.byte		        79*mus_qz_621_mvl/mxv
	.byte	W04
	.byte		        81*mus_qz_621_mvl/mxv
	.byte	W03
	.byte		        81*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        83*mus_qz_621_mvl/mxv
	.byte	W10
	.byte		        81*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        81*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        78*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        77*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        76*mus_qz_621_mvl/mxv
	.byte	W03
	.byte		        74*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        71*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        70*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        69*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        66*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        65*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        64*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        60*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        59*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        58*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        55*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        52*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        52*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        50*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        47*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        45*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        44*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        41*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        39*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        38*mus_qz_621_mvl/mxv
	.byte	W03
	.byte		        34*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        32*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        30*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        28*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        26*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        23*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        22*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        20*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        18*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        16*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        13*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        12*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        9*mus_qz_621_mvl/mxv
	.byte	W01
	.byte	EOT
	.byte		VOL   , 8*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        5*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        3*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        2*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        1*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        0*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        0*mus_qz_621_mvl/mxv
	.byte	W18
	.byte		        83*mus_qz_621_mvl/mxv
	.byte	W24
	.byte		N05   , En3 , v092
	.byte		N05   , An3
	.byte	W18
	.byte	N05
	.byte		N05   , En4
	.byte	W06
	.byte		N23   , Cs4
	.byte		N23   , An4
	.byte	W24
	.byte		        Bn3
	.byte		N23   , Gs4
	.byte	W12
	.byte		VOL   , 81*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        79*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        78*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        74*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        71*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        70*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        67*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        65*mus_qz_621_mvl/mxv
	.byte	W03
	.byte		        83*mus_qz_621_mvl/mxv
	.byte		N32   , En3 , v092 , gtp3
	.byte		N32   , Cs4 , v092 , gtp3
	.byte	W36
	.byte		        Fs3 , v092 , gtp3
	.byte		N32   , Ds4 , v092 , gtp3
	.byte	W36
	.byte		N23   , Gs3
	.byte		N23   , En4
	.byte	W24
	.byte		VOL   , 83*mus_qz_621_mvl/mxv
	.byte		N05   , Bn3
	.byte		N05   , Fs4
	.byte	W18
	.byte		        An3
	.byte		N05   , En4
	.byte	W06
	.byte		N56   , Fs3 , v092 , gtp3
	.byte		N56   , Ds4 , v092 , gtp3
	.byte	W44
	.byte		VOL   , 81*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        81*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        77*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        76*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        72*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        70*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        68*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        67*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        63*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        61*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        53*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        47*mus_qz_621_mvl/mxv
	.byte	W03
	.byte		        43*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        38*mus_qz_621_mvl/mxv
	.byte	W10
	.byte		        32*mus_qz_621_mvl/mxv
	.byte		        71*mus_qz_621_mvl/mxv
	.byte		N05   , Ds3
	.byte		N05   , Bn3
	.byte	W18
	.byte		        Bn2
	.byte		N05   , Fs3
	.byte	W06
	.byte		N56   , Fs3 , v092 , gtp3
	.byte		N56   , Ds4 , v092 , gtp3
	.byte	W30
	.byte		VOL   , 75*mus_qz_621_mvl/mxv
	.byte	W06
	.byte		        76*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        78*mus_qz_621_mvl/mxv
	.byte	W03
	.byte		        81*mus_qz_621_mvl/mxv
	.byte	W04
	.byte		        79*mus_qz_621_mvl/mxv
	.byte	W03
	.byte		        78*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        75*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        74*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        70*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        68*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        66*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        65*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        61*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        59*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        52*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        45*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        41*mus_qz_621_mvl/mxv
	.byte	W08
	.byte		        81*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        83*mus_qz_621_mvl/mxv
	.byte	W24
	.byte		N05   , En3
	.byte		N05   , An3
	.byte	W18
	.byte	N05
	.byte		N05   , En4
	.byte	W06
	.byte		N23   , Cs4
	.byte		N23   , An4
	.byte	W24
	.byte		        Bn3
	.byte		N23   , Gs4
	.byte	W13
	.byte		VOL   , 76*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        74*mus_qz_621_mvl/mxv
	.byte	W03
	.byte		        71*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        67*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        62*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        56*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        73*mus_qz_621_mvl/mxv
	.byte		N32   , En3 , v092 , gtp3
	.byte		N32   , Cs4 , v092 , gtp3
	.byte	W01
	.byte		VOL   , 75*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        77*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        81*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        83*mus_qz_621_mvl/mxv
	.byte	W32
	.byte		N32   , Fs3 , v092 , gtp3
	.byte		N32   , Ds4 , v092 , gtp3
	.byte	W36
	.byte		N23   , En3
	.byte		N23   , En4
	.byte	W24
	.byte		N05   , Bn3
	.byte		N05   , Fs4
	.byte	W18
	.byte		        Fs3
	.byte		N05   , Ds4
	.byte	W06
	.byte		N56   , Ds4 , v092 , gtp3
	.byte		N56   , Bn4 , v092 , gtp3
	.byte	W08
	.byte		VOL   , 81*mus_qz_621_mvl/mxv
	.byte	W06
	.byte		        79*mus_qz_621_mvl/mxv
	.byte	W10
	.byte		        77*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        79*mus_qz_621_mvl/mxv
	.byte	W06
	.byte		        81*mus_qz_621_mvl/mxv
	.byte	W06
	.byte		        83*mus_qz_621_mvl/mxv
	.byte	W11
	.byte		        81*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        78*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        72*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        68*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        64*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        53*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        45*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        34*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        22*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        0*mus_qz_621_mvl/mxv
	.byte	W08
	.byte		        83*mus_qz_621_mvl/mxv
	.byte	W03
	.byte		N05   , Ds4
	.byte		N05   , Bn4
	.byte	W18
	.byte		        Bn3
	.byte		N05   , Fs4
	.byte	W06
	.byte		N56   , Fs4 , v092 , gtp3
	.byte		N56   , Ds5 , v092 , gtp3
	.byte	W08
	.byte		VOL   , 81*mus_qz_621_mvl/mxv
	.byte	W04
	.byte		        79*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        77*mus_qz_621_mvl/mxv
	.byte	W15
	.byte		        74*mus_qz_621_mvl/mxv
	.byte	W09
	.byte		        77*mus_qz_621_mvl/mxv
	.byte	W03
	.byte		        79*mus_qz_621_mvl/mxv
	.byte	W03
	.byte		        83*mus_qz_621_mvl/mxv
	.byte	W04
	.byte		        81*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        74*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        68*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        64*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        55*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        50*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        41*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        32*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        19*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        2*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        0*mus_qz_621_mvl/mxv
	.byte	W12
	.byte	W56
	.byte	W01
	.byte		        83*mus_qz_621_mvl/mxv
	.byte	W36
	.byte	W03
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W72
	.byte		N23   , Fs3 , v100
	.byte	W24
	.byte	PATT
	 ptrvalue	mus_qz_621_1_000
	.byte	GOTO
	 ptrvalue	mus_qz_621_1_B1
	.byte	FINE

/********************** Track  2 **********************/

mus_qz_621_2:
	.byte		KEYSH , mus_qz_621_key+0
	.byte		VOICE , 29
	.byte		PAN   , c_v+0
	.byte		BENDR , 6
	.byte		PRIO  , 50
	.byte		VOL   , 39*mus_qz_621_mvl/mxv
	.byte		BEND  , c_v+1
	.byte	W96
	.byte		PAN   , c_v-34
	.byte	W78
	.byte		N23   , Fs3 , v100
	.byte	W18
mus_qz_621_2_000:
	.byte	W06
	.byte		N44   , Bn3 , v100 , gtp3
	.byte	W60
	.byte		N05   , Fs3
	.byte	W12
	.byte	N05
	.byte	W12
	.byte		        Bn3
	.byte	W06
	.byte	PEND
mus_qz_621_2_B1:
	.byte	W06
	.byte		VOL   , 39*mus_qz_621_mvl/mxv
	.byte		N23   , Cs4 , v100
	.byte	W24
	.byte		        Bn3
	.byte	W24
	.byte		        An3
	.byte	W18
	.byte		VOL   , 38*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        36*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        35*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        35*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        33*mus_qz_621_mvl/mxv
	.byte		N23   , En3
	.byte	W01
	.byte		VOL   , 38*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        39*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        39*mus_qz_621_mvl/mxv
	.byte	W15
	.byte	W06
	.byte		TIE   , Fs3
	.byte	W10
	.byte		VOL   , 38*mus_qz_621_mvl/mxv
	.byte	W04
	.byte		        37*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        36*mus_qz_621_mvl/mxv
	.byte	W05
	.byte		        35*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        35*mus_qz_621_mvl/mxv
	.byte	W24
	.byte	W02
	.byte		        35*mus_qz_621_mvl/mxv
	.byte	W03
	.byte		        36*mus_qz_621_mvl/mxv
	.byte	W07
	.byte		        37*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        38*mus_qz_621_mvl/mxv
	.byte	W07
	.byte		        39*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        39*mus_qz_621_mvl/mxv
	.byte	W21
	.byte	W36
	.byte		        37*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        32*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        27*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        22*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        18*mus_qz_621_mvl/mxv
	.byte	W01
	.byte	EOT
	.byte	W02
	.byte		VOL   , 14*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        12*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        8*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        6*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        3*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        2*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        0*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        0*mus_qz_621_mvl/mxv
	.byte	W22
	.byte		        39*mus_qz_621_mvl/mxv
	.byte	W05
	.byte	N23
	.byte	W18
	.byte	PATT
	 ptrvalue	mus_qz_621_2_000
	.byte	W06
	.byte		N23   , Cs4 , v100
	.byte	W24
	.byte		        Bn3
	.byte	W24
	.byte		        Cs4
	.byte	W16
	.byte		VOL   , 39*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        38*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        36*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        35*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        32*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        30*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        36*mus_qz_621_mvl/mxv
	.byte		N23   , En4
	.byte	W01
	.byte		VOL   , 38*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        39*mus_qz_621_mvl/mxv
	.byte	W16
	.byte		        26*mus_qz_621_mvl/mxv
	.byte	W06
	.byte		        24*mus_qz_621_mvl/mxv
	.byte		        22*mus_qz_621_mvl/mxv
	.byte		TIE   , Ds4
	.byte	W01
	.byte		VOL   , 23*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        24*mus_qz_621_mvl/mxv
	.byte		        20*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        21*mus_qz_621_mvl/mxv
	.byte	W22
	.byte		        20*mus_qz_621_mvl/mxv
	.byte	W05
	.byte		        20*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        17*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        16*mus_qz_621_mvl/mxv
	.byte	W10
	.byte		        16*mus_qz_621_mvl/mxv
	.byte	W19
	.byte		        15*mus_qz_621_mvl/mxv
	.byte	W03
	.byte		        13*mus_qz_621_mvl/mxv
	.byte	W15
	.byte		        14*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        16*mus_qz_621_mvl/mxv
	.byte	W03
	.byte		        16*mus_qz_621_mvl/mxv
	.byte	W04
	.byte	W04
	.byte		        17*mus_qz_621_mvl/mxv
	.byte	W08
	.byte		        18*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        20*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        21*mus_qz_621_mvl/mxv
	.byte	W06
	.byte		        20*mus_qz_621_mvl/mxv
	.byte	W03
	.byte		        20*mus_qz_621_mvl/mxv
	.byte	W03
	.byte		        19*mus_qz_621_mvl/mxv
	.byte	W03
	.byte		        18*mus_qz_621_mvl/mxv
	.byte	W05
	.byte		        20*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        20*mus_qz_621_mvl/mxv
	.byte	W04
	.byte		        19*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        18*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        17*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        16*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        15*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        15*mus_qz_621_mvl/mxv
	.byte	W03
	.byte		        13*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        12*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        11*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        10*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        9*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        8*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        7*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        7*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        6*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        5*mus_qz_621_mvl/mxv
	.byte	W03
	.byte		        4*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        3*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        3*mus_qz_621_mvl/mxv
	.byte	W02
	.byte	EOT
	.byte		VOL   , 2*mus_qz_621_mvl/mxv
	.byte	W03
	.byte		        1*mus_qz_621_mvl/mxv
	.byte	W03
	.byte		        0*mus_qz_621_mvl/mxv
	.byte	W03
	.byte		        0*mus_qz_621_mvl/mxv
	.byte	W12
	.byte	W06
	.byte		        26*mus_qz_621_mvl/mxv
	.byte	W24
	.byte		N05   , An3
	.byte	W18
	.byte		        En4
	.byte	W06
	.byte		N23   , An4
	.byte	W24
	.byte		        Gs4
	.byte	W12
	.byte		VOL   , 24*mus_qz_621_mvl/mxv
	.byte	W03
	.byte		        24*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        22*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        22*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        20*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        20*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        17*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        16*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        26*mus_qz_621_mvl/mxv
	.byte		N32   , Cs4 , v100 , gtp3
	.byte	W36
	.byte		        Ds4 , v100 , gtp3
	.byte	W36
	.byte		N23   , En4
	.byte	W18
	.byte	W06
	.byte		N05   , Fs4
	.byte	W18
	.byte		        En4
	.byte	W06
	.byte		N56   , Ds4 , v100 , gtp3
	.byte	W44
	.byte		VOL   , 25*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        24*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        24*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        23*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        22*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        21*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        20*mus_qz_621_mvl/mxv
	.byte	W03
	.byte		        20*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        19*mus_qz_621_mvl/mxv
	.byte	W10
	.byte	W06
	.byte		        26*mus_qz_621_mvl/mxv
	.byte		N05   , Bn3
	.byte	W18
	.byte		        Fs3
	.byte	W06
	.byte		N56   , Ds4 , v100 , gtp3
	.byte	W44
	.byte	W01
	.byte		VOL   , 25*mus_qz_621_mvl/mxv
	.byte	W03
	.byte		        24*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        24*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        23*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        22*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        21*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        20*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        20*mus_qz_621_mvl/mxv
	.byte	W10
	.byte	W04
	.byte		        26*mus_qz_621_mvl/mxv
	.byte	W24
	.byte	W02
	.byte		N05   , An3
	.byte	W18
	.byte		        En4
	.byte	W06
	.byte		N23   , An4
	.byte	W24
	.byte		        Gs4
	.byte	W13
	.byte		VOL   , 23*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        22*mus_qz_621_mvl/mxv
	.byte	W03
	.byte		        22*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        20*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        19*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        17*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        22*mus_qz_621_mvl/mxv
	.byte		N32   , Cs4 , v100 , gtp3
	.byte	W01
	.byte		VOL   , 23*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        24*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        24*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        26*mus_qz_621_mvl/mxv
	.byte	W32
	.byte		N32   , Ds4 , v100 , gtp3
	.byte	W36
	.byte		N23   , En4
	.byte	W18
	.byte	W06
	.byte		N05   , Fs4
	.byte	W18
	.byte		        Ds4
	.byte	W06
	.byte		N56   , Bn4 , v100 , gtp3
	.byte	W48
	.byte	W01
	.byte		VOL   , 24*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        24*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        22*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        20*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        20*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        16*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        14*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        10*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        7*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        0*mus_qz_621_mvl/mxv
	.byte	W05
	.byte	W03
	.byte		        26*mus_qz_621_mvl/mxv
	.byte	W03
	.byte	N05
	.byte	W18
	.byte		        Fs4
	.byte	W06
	.byte		N56   , Ds5 , v100 , gtp3
	.byte	W48
	.byte		VOL   , 24*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        22*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        20*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        20*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        17*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        15*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        12*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        10*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        5*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        0*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        0*mus_qz_621_mvl/mxv
	.byte	W06
	.byte	W60
	.byte	W03
	.byte		        26*mus_qz_621_mvl/mxv
	.byte	W32
	.byte	W01
	.byte		VOICE , 23
	.byte	W78
	.byte		N23   , Fs2 , v127
	.byte	W18
	.byte	W06
	.byte		N44   , Bn2 , v127 , gtp3
	.byte	W36
	.byte	W02
	.byte		VOL   , 24*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        22*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        20*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        20*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        18*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        16*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        11*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        8*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        7*mus_qz_621_mvl/mxv
	.byte	W07
	.byte		        26*mus_qz_621_mvl/mxv
	.byte	W05
	.byte		N05   , Fs2
	.byte	W12
	.byte	N05
	.byte	W12
	.byte		        Bn2
	.byte	W06
	.byte	W06
	.byte		N23   , Cs3
	.byte	W24
	.byte		        Bn2
	.byte	W24
	.byte		        Cs3
	.byte	W14
	.byte		VOL   , 25*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        24*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        23*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        22*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        21*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        20*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        20*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        19*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        24*mus_qz_621_mvl/mxv
	.byte		N23   , En3
	.byte	W01
	.byte		VOL   , 25*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        26*mus_qz_621_mvl/mxv
	.byte	W16
	.byte	W06
	.byte		N80   , Ds3 , v127 , gtp3
	.byte	W03
	.byte		VOL   , 24*mus_qz_621_mvl/mxv
	.byte		        24*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        23*mus_qz_621_mvl/mxv
	.byte		        21*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        22*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        20*mus_qz_621_mvl/mxv
	.byte		        18*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        18*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        16*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        16*mus_qz_621_mvl/mxv
	.byte		        14*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        13*mus_qz_621_mvl/mxv
	.byte	W04
	.byte		        13*mus_qz_621_mvl/mxv
	.byte	W07
	.byte		        11*mus_qz_621_mvl/mxv
	.byte	W05
	.byte		        10*mus_qz_621_mvl/mxv
	.byte	W13
	.byte		        11*mus_qz_621_mvl/mxv
	.byte	W06
	.byte		        9*mus_qz_621_mvl/mxv
	.byte	W03
	.byte		        9*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        10*mus_qz_621_mvl/mxv
	.byte	W05
	.byte		        12*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        13*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        13*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        16*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        16*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        17*mus_qz_621_mvl/mxv
	.byte	W03
	.byte		        20*mus_qz_621_mvl/mxv
	.byte		        20*mus_qz_621_mvl/mxv
	.byte	W03
	.byte		        24*mus_qz_621_mvl/mxv
	.byte	W07
	.byte		        25*mus_qz_621_mvl/mxv
	.byte	W05
	.byte		        32*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        33*mus_qz_621_mvl/mxv
	.byte		N05   , En3
	.byte	W06
	.byte		        Fs3
	.byte	W06
	.byte		N23   , Bn3
	.byte	W24
	.byte		        An3
	.byte	W24
	.byte		        Bn3
	.byte	W24
	.byte		        Cs4
	.byte	W18
	.byte	W06
	.byte		        Bn3
	.byte	W24
	.byte		N32   , Fs3 , v127 , gtp3
	.byte	W18
	.byte		VOL   , 32*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        32*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        32*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        32*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        31*mus_qz_621_mvl/mxv
	.byte	W04
	.byte		        30*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        28*mus_qz_621_mvl/mxv
	.byte	W04
	.byte		        27*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        26*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        33*mus_qz_621_mvl/mxv
	.byte		N11   , En3
	.byte	W12
	.byte		N05   , Ds3
	.byte	W06
	.byte		        En3
	.byte	W06
	.byte		N11   , Fs3
	.byte	W06
	.byte	W06
	.byte		N44   , Gn3 , v127 , gtp3
	.byte	W24
	.byte	W02
	.byte		VOL   , 35*mus_qz_621_mvl/mxv
	.byte	W06
	.byte		        37*mus_qz_621_mvl/mxv
	.byte	W03
	.byte		        39*mus_qz_621_mvl/mxv
	.byte	W03
	.byte		        38*mus_qz_621_mvl/mxv
	.byte		        42*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        41*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        39*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        36*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        37*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        36*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        35*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        25*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        33*mus_qz_621_mvl/mxv
	.byte		N44   , An3 , v127 , gtp3
	.byte	W24
	.byte		VOL   , 35*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        37*mus_qz_621_mvl/mxv
	.byte	W09
	.byte		        39*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        38*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        37*mus_qz_621_mvl/mxv
	.byte		        39*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        37*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        35*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        33*mus_qz_621_mvl/mxv
	.byte		        35*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        32*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        30*mus_qz_621_mvl/mxv
	.byte		        32*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        27*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        22*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        24*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        48*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        50*mus_qz_621_mvl/mxv
	.byte	W88
	.byte		VOICE , 29
	.byte	W72
	.byte		VOL   , 32*mus_qz_621_mvl/mxv
	.byte	W06
	.byte		N23   , Fs3 , v100
	.byte	W18
	.byte	PATT
	 ptrvalue	mus_qz_621_2_000
	.byte	GOTO
	 ptrvalue	mus_qz_621_2_B1
	.byte	FINE

/********************** Track  3 **********************/

mus_qz_621_3:
	.byte		KEYSH , mus_qz_621_key+0
	.byte		VOICE , 30
	.byte		PAN   , c_v+0
	.byte		BENDR , 6
	.byte		LFOS  , 25
	.byte		PRIO  , 61
	.byte		VOL   , 64*mus_qz_621_mvl/mxv
	.byte		N02   , Ds2 , v100
	.byte		N02   , Fs2
	.byte		N02   , Bn2
	.byte	W12
	.byte		        Ds2
	.byte		N02   , Fs2
	.byte		N02   , Bn2
	.byte	W06
	.byte		        Ds2
	.byte		N02   , Fs2
	.byte		N02   , Bn2
	.byte	W06
	.byte		        Ds2
	.byte		N02   , Fs2
	.byte		N02   , Bn2
	.byte	W12
	.byte		        Ds2
	.byte		N02   , Fs2
	.byte		N02   , Bn2
	.byte	W06
	.byte		        Ds2
	.byte		N02   , Fs2
	.byte		N02   , Bn2
	.byte	W06
	.byte		        Ds2
	.byte		N02   , Fs2
	.byte		N02   , Bn2
	.byte	W06
	.byte		        Ds2
	.byte		N02   , Fs2
	.byte		N02   , Bn2
	.byte	W06
	.byte		        Ds2
	.byte		N02   , Fs2
	.byte		N02   , Bn2
	.byte	W36
	.byte		        Ds2
	.byte		N02   , Fs2
	.byte		N02   , Bn2
	.byte	W12
	.byte		        Ds2
	.byte		N02   , Fs2
	.byte		N02   , Bn2
	.byte	W06
	.byte		        Ds2
	.byte		N02   , Fs2
	.byte		N02   , Bn2
	.byte	W06
	.byte		        Ds2
	.byte		N02   , Fs2
	.byte		N02   , Bn2
	.byte	W12
	.byte		        Ds2
	.byte		N02   , Fs2
	.byte		N02   , Bn2
	.byte	W06
	.byte		        Ds2
	.byte		N02   , Fs2
	.byte		N02   , Bn2
	.byte	W06
	.byte		        Ds2
	.byte		N02   , Fs2
	.byte		N02   , Bn2
	.byte	W06
	.byte		        Ds2
	.byte		N02   , Fs2
	.byte		N02   , Bn2
	.byte	W06
	.byte		        Ds2
	.byte		N02   , Fs2
	.byte		N02   , Bn2
	.byte	W36
	.byte		VOICE , 35
	.byte		VOL   , 49*mus_qz_621_mvl/mxv
	.byte		PAN   , c_v-7
	.byte	W05
	.byte		        c_v-8
	.byte	W02
	.byte		BEND  , c_v+1
	.byte	W05
	.byte		MOD   , 1
	.byte		PAN   , c_v-14
	.byte		VOL   , 35*mus_qz_621_mvl/mxv
	.byte	W05
	.byte		PAN   , c_v-16
	.byte	W07
	.byte		N68   , Fs5 , v100 , gtp3
	.byte	W05
	.byte		PAN   , c_v-17
	.byte	W01
	.byte		VOL   , 39*mus_qz_621_mvl/mxv
	.byte	W06
	.byte		        49*mus_qz_621_mvl/mxv
	.byte	W05
	.byte		PAN   , c_v-19
	.byte	W12
	.byte		        c_v-23
	.byte	W19
	.byte		        c_v-24
	.byte	W12
	.byte		        c_v-26
	.byte	W12
mus_qz_621_3_B1:
	.byte		N44   , An5 , v100 , gtp3
	.byte	W05
	.byte		PAN   , c_v-28
	.byte	W30
	.byte	W01
	.byte		        c_v-26
	.byte	W05
	.byte		        c_v-24
	.byte	W07
	.byte		        c_v-23
	.byte		N44   , Cs6 , v100 , gtp3
	.byte	W17
	.byte		PAN   , c_v-19
	.byte	W12
	.byte		        c_v-17
	.byte	W07
	.byte		        c_v-16
	.byte	W12
	.byte		        c_v-14
	.byte		N23   , Bn5
	.byte	W05
	.byte		PAN   , c_v-8
	.byte	W07
	.byte		        c_v-7
	.byte	W05
	.byte		        c_v-5
	.byte	W07
	.byte		        c_v-1
	.byte		N44   , Fs5 , v100 , gtp3
	.byte	W05
	.byte		PAN   , c_v+0
	.byte	W12
	.byte		        c_v+6
	.byte	W07
	.byte		        c_v+9
	.byte	W05
	.byte		        c_v+11
	.byte	W07
	.byte		        c_v+15
	.byte	W05
	.byte		        c_v+16
	.byte	W07
	.byte		N23   , En5
	.byte	W05
	.byte		PAN   , c_v+18
	.byte	W19
	.byte		N17   , Ds5
	.byte	W05
	.byte		PAN   , c_v+20
	.byte	W12
	.byte		        c_v+24
	.byte	W01
	.byte		N05   , En5
	.byte	W06
	.byte		N23   , Fs5
	.byte	W12
	.byte		PAN   , c_v+20
	.byte	W12
	.byte		N68   , Bn4 , v100 , gtp3
	.byte	W05
	.byte		PAN   , c_v+18
	.byte	W12
	.byte		        c_v+16
	.byte	W07
	.byte		        c_v+15
	.byte	W12
	.byte		        c_v+11
	.byte	W05
	.byte		        c_v+9
	.byte	W07
	.byte	W12
	.byte		        c_v+6
	.byte	W05
	.byte		        c_v+2
	.byte	W07
	.byte		        c_v+0
	.byte	W12
	.byte		        c_v-5
	.byte	W05
	.byte		        c_v-7
	.byte	W07
	.byte		        c_v-8
	.byte	W05
	.byte		        c_v-10
	.byte	W07
	.byte		        c_v-14
	.byte	W05
	.byte		        c_v-16
	.byte	W07
	.byte		        c_v-17
	.byte	W05
	.byte		        c_v-19
	.byte	W07
	.byte		        c_v-23
	.byte	W12
	.byte	W05
	.byte		        c_v-19
	.byte	W07
	.byte		        c_v-17
	.byte	W84
	.byte	W96
	.byte	W96
	.byte	W72
	.byte		N23   , En4
	.byte	W24
	.byte		N32   , Cs5 , v100 , gtp3
	.byte	W36
	.byte		        An4 , v100 , gtp3
	.byte	W36
	.byte		N23   , Gs5
	.byte	W24
	.byte		N68   , Fs5 , v100 , gtp3
	.byte	W72
	.byte		N23   , Fn5
	.byte	W24
	.byte		N05   , Fs5
	.byte	W18
	.byte		        En5
	.byte	W06
	.byte		N11   , Ds5
	.byte	W12
	.byte		        En5
	.byte	W12
	.byte		N23   , Fs5
	.byte	W24
	.byte		        Bn4
	.byte	W24
	.byte		N56   , Cs5 , v100 , gtp3
	.byte	W60
	.byte		N05   , En5
	.byte	W06
	.byte		        An5
	.byte	W06
	.byte		N23   , Bn5
	.byte	W24
	.byte		N32   , An5 , v100 , gtp3
	.byte	W36
	.byte		        Gs5 , v100 , gtp3
	.byte	W36
	.byte		N23   , An5
	.byte	W24
	.byte		N56   , Bn5 , v100 , gtp3
	.byte	W60
	.byte		N11   , Ds5
	.byte	W12
	.byte		N05   , Fs5
	.byte	W06
	.byte		        Bn5
	.byte	W06
	.byte		N11   , Cs6
	.byte	W12
	.byte		VOL   , 47*mus_qz_621_mvl/mxv
	.byte		N05   , Bn5
	.byte	W06
	.byte		        Cs6
	.byte	W06
	.byte		        Ds6
	.byte	W06
	.byte		        En6
	.byte	W06
	.byte		N44   , Fs6 , v100 , gtp3
	.byte	W48
	.byte		N23   , Fn5
	.byte	W24
	.byte		BEND  , c_v+3
	.byte		N02   , Ds3
	.byte		N02   , Fs3
	.byte		N02   , Bn3
	.byte	W12
	.byte		        Ds3
	.byte		N02   , Fs3
	.byte		N02   , Bn3
	.byte	W06
	.byte		        Ds3
	.byte		N02   , Fs3
	.byte		N02   , Bn3
	.byte	W06
	.byte		        Ds3
	.byte		N02   , Fs3
	.byte		N02   , Bn3
	.byte	W12
	.byte		        Ds3
	.byte		N02   , Fs3
	.byte		N02   , Bn3
	.byte	W06
	.byte		        Ds3
	.byte		N02   , Fs3
	.byte		N02   , Bn3
	.byte	W06
	.byte		        Ds3
	.byte		N02   , Fs3
	.byte		N02   , Bn3
	.byte	W06
	.byte		        Ds3
	.byte		N02   , Fs3
	.byte		N02   , Bn3
	.byte	W06
	.byte		        Ds3
	.byte		N02   , Fs3
	.byte		N02   , Bn3
	.byte	W36
	.byte		VOL   , 45*mus_qz_621_mvl/mxv
	.byte		N02   , Ds3
	.byte		N02   , Fs3
	.byte		N02   , Bn3
	.byte	W12
	.byte		        Ds3
	.byte		N02   , Fs3
	.byte		N02   , Bn3
	.byte	W06
	.byte		        Ds3
	.byte		N02   , Fs3
	.byte		N02   , Bn3
	.byte	W06
	.byte		        Ds3
	.byte		N02   , Fs3
	.byte		N02   , Bn3
	.byte	W12
	.byte		        Ds3
	.byte		N02   , Fs3
	.byte		N02   , Bn3
	.byte	W06
	.byte		        Ds3
	.byte		N02   , Fs3
	.byte		N02   , Bn3
	.byte	W06
	.byte		        Ds3
	.byte		N02   , Fs3
	.byte		N02   , Bn3
	.byte	W06
	.byte		        Ds3
	.byte		N02   , Fs3
	.byte		N02   , Bn3
	.byte	W06
	.byte		        Ds3
	.byte		N02   , Fs3
	.byte		N02   , Bn3
	.byte	W36
	.byte		VOL   , 56*mus_qz_621_mvl/mxv
	.byte		PAN   , c_v-24
	.byte	W36
	.byte		N05   , Ds4
	.byte	W06
	.byte		        En4
	.byte	W06
	.byte		N23   , Fs4
	.byte	W24
	.byte		        Bn4
	.byte	W24
	.byte		        An4
	.byte	W24
	.byte		        Gs4
	.byte	W24
	.byte		N05   , Fs4
	.byte	W12
	.byte		        Gs4
	.byte	W12
	.byte		        An4
	.byte	W12
	.byte		        En4
	.byte	W12
	.byte		        Ds4
	.byte	W18
	.byte		        En4
	.byte	W06
	.byte		N23   , Fs4
	.byte	W24
	.byte		        Bn3
	.byte	W24
	.byte		N92   , Bn4 , v100 , gtp3
	.byte	W24
	.byte	W72
	.byte		N05   , Cs4
	.byte	W06
	.byte		        Dn4
	.byte	W06
	.byte		        En4
	.byte	W06
	.byte		        Fs4
	.byte	W06
	.byte		N88   , Bn4 , v100 , gtp1
	.byte	W56
	.byte	W01
	.byte		VOL   , 55*mus_qz_621_mvl/mxv
	.byte	W03
	.byte		        52*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        45*mus_qz_621_mvl/mxv
	.byte	W03
	.byte		        39*mus_qz_621_mvl/mxv
	.byte	W03
	.byte		        32*mus_qz_621_mvl/mxv
	.byte	W04
	.byte		        28*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        16*mus_qz_621_mvl/mxv
	.byte	W03
	.byte		        4*mus_qz_621_mvl/mxv
	.byte	W03
	.byte		        0*mus_qz_621_mvl/mxv
	.byte	W16
	.byte	W48
	.byte		        56*mus_qz_621_mvl/mxv
	.byte	W48
	.byte	W96
	.byte		BEND  , c_v+1
	.byte	W96
	.byte		VOICE , 35
	.byte		VOL   , 49*mus_qz_621_mvl/mxv
	.byte		PAN   , c_v-7
	.byte	W05
	.byte		        c_v-8
	.byte	W07
	.byte		        c_v-14
	.byte		VOL   , 35*mus_qz_621_mvl/mxv
	.byte	W05
	.byte		PAN   , c_v-16
	.byte	W07
	.byte		N68   , Fs5 , v100 , gtp3
	.byte	W05
	.byte		PAN   , c_v-17
	.byte	W01
	.byte		VOL   , 39*mus_qz_621_mvl/mxv
	.byte	W06
	.byte		        49*mus_qz_621_mvl/mxv
	.byte	W05
	.byte		PAN   , c_v-19
	.byte	W12
	.byte		        c_v-23
	.byte	W19
	.byte		        c_v-24
	.byte	W12
	.byte		        c_v-26
	.byte	W12
	.byte	GOTO
	 ptrvalue	mus_qz_621_3_B1
	.byte	FINE

/********************** Track  4 **********************/

mus_qz_621_4:
	.byte		KEYSH , mus_qz_621_key+0
	.byte		VOICE , 32
	.byte		PAN   , c_v+0
	.byte		PRIO  , 61
	.byte		VOL   , 61*mus_qz_621_mvl/mxv
	.byte	W96
	.byte		PAN   , c_v-10
	.byte	W96
mus_qz_621_4_000:
	.byte	W12
	.byte		N11   , Fs2 , v100
	.byte	W12
	.byte		        Bn2
	.byte	W12
	.byte		        En3
	.byte	W12
	.byte		N44   , Ds3 , v100 , gtp3
	.byte	W48
	.byte	PEND
mus_qz_621_4_B1:
	.byte	W12
	.byte		N11   , En3 , v100
	.byte	W24
	.byte		        Ds3
	.byte	W24
	.byte		        Cs3
	.byte	W24
	.byte		        Bn2
	.byte	W12
	.byte		N05   , Ds3
	.byte	W18
	.byte		        Bn2
	.byte	W06
	.byte		N44   , Fs2 , v100 , gtp3
	.byte	W48
	.byte		N23   , Bn2
	.byte	W24
	.byte		N56   , Ds3 , v100 , gtp3
	.byte	W96
	.byte		VOL   , 83*mus_qz_621_mvl/mxv
	.byte		PAN   , c_v-12
	.byte	W12
	.byte		N11   , Fs2
	.byte	W12
	.byte		        Bn2
	.byte	W12
	.byte		        En3
	.byte	W12
	.byte		N44   , Ds3 , v100 , gtp3
	.byte	W48
	.byte	W12
	.byte		N11   , An2
	.byte	W24
	.byte		        Cs3
	.byte	W24
	.byte		        En3
	.byte	W24
	.byte		        An3
	.byte	W12
	.byte		N05   , Fs3
	.byte	W18
	.byte		        An3
	.byte	W06
	.byte		N32   , Bn3 , v100 , gtp3
	.byte	W36
	.byte		N11   , Bn2
	.byte	W12
	.byte		        Ds3
	.byte	W12
	.byte		        Fs3
	.byte	W12
	.byte		        Bn3
	.byte	W12
	.byte		N23   , An3
	.byte	W24
	.byte		        Fs3
	.byte	W24
	.byte		N11   , En3
	.byte	W12
	.byte		N23   , Ds3
	.byte	W24
	.byte		PAN   , c_v-8
	.byte		N05   , En2
	.byte		N05   , An2
	.byte	W06
	.byte		        En2 , v036
	.byte		N05   , An2
	.byte	W06
	.byte		        En2 , v100
	.byte		N05   , An2
	.byte	W06
	.byte		        En2
	.byte		N05   , An2
	.byte	W06
	.byte		        En2
	.byte		N05   , An2
	.byte	W06
	.byte		        En2 , v036
	.byte		N05   , An2
	.byte	W06
	.byte		        En2 , v100
	.byte		N05   , An2
	.byte	W06
	.byte		        En2
	.byte		N05   , An2
	.byte	W06
	.byte		        En2
	.byte		N05   , An2
	.byte	W06
	.byte		        En2 , v036
	.byte		N05   , An2
	.byte	W06
	.byte		        En2 , v100
	.byte		N05   , An2
	.byte	W06
	.byte		        En2
	.byte		N05   , An2
	.byte	W06
	.byte		N23   , En2
	.byte		N23   , Cs3
	.byte	W24
	.byte		N05   , En2
	.byte		N05   , An2
	.byte	W06
	.byte		        En2 , v036
	.byte		N05   , An2
	.byte	W06
	.byte		        En2 , v100
	.byte		N05   , An2
	.byte	W06
	.byte		        En2
	.byte		N05   , An2
	.byte	W06
	.byte		        En2
	.byte		N05   , An2
	.byte	W06
	.byte		        En2 , v036
	.byte		N05   , An2
	.byte	W06
	.byte		        En2 , v100
	.byte		N05   , An2
	.byte	W06
	.byte		        En2
	.byte		N05   , An2
	.byte	W06
	.byte		        En2
	.byte		N05   , An2
	.byte	W06
	.byte		        En2 , v036
	.byte		N05   , An2
	.byte	W06
	.byte		        En2 , v100
	.byte		N05   , An2
	.byte	W06
	.byte		        En2
	.byte		N05   , An2
	.byte	W06
	.byte		N23   , En2
	.byte		N23   , Cs3
	.byte	W24
mus_qz_621_4_001:
	.byte		N05   , Fs2 , v100
	.byte		N05   , Bn2
	.byte	W06
	.byte		        Fs2 , v036
	.byte		N05   , Bn2
	.byte	W06
	.byte		        Fs2 , v100
	.byte		N05   , Bn2
	.byte	W06
	.byte		        Fs2
	.byte		N05   , Bn2
	.byte	W06
	.byte		        Fs2
	.byte		N05   , Bn2
	.byte	W06
	.byte		        Fs2 , v036
	.byte		N05   , Bn2
	.byte	W06
	.byte		        Fs2 , v100
	.byte		N05   , Bn2
	.byte	W06
	.byte		        Fs2
	.byte		N05   , Bn2
	.byte	W06
	.byte		        Fs2
	.byte		N05   , Bn2
	.byte	W06
	.byte		        Fs2 , v036
	.byte		N05   , Bn2
	.byte	W06
	.byte		        Fs2 , v100
	.byte		N05   , Bn2
	.byte	W06
	.byte		        Fs2
	.byte		N05   , Bn2
	.byte	W06
	.byte	N23
	.byte		N23   , Fs3
	.byte	W24
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_621_4_001
	.byte		N05   , En2 , v100
	.byte		N05   , An2
	.byte	W06
	.byte		        En2 , v036
	.byte		N05   , An2
	.byte	W06
	.byte		        En2 , v100
	.byte		N05   , An2
	.byte	W06
	.byte		        En2
	.byte		N05   , An2
	.byte	W06
	.byte		        En2
	.byte		N05   , An2
	.byte	W06
	.byte		        En2 , v036
	.byte		N05   , An2
	.byte	W06
	.byte		        En2 , v100
	.byte		N05   , An2
	.byte	W06
	.byte		        En2
	.byte		N05   , An2
	.byte	W06
	.byte		        En2
	.byte		N05   , An2
	.byte	W06
	.byte		        En2 , v036
	.byte		N05   , An2
	.byte	W06
	.byte		        En2 , v100
	.byte		N05   , An2
	.byte	W06
	.byte		        En2
	.byte		N05   , An2
	.byte	W06
	.byte	N23
	.byte		N23   , En3
	.byte	W24
	.byte		N05   , En2
	.byte		N05   , An2
	.byte	W06
	.byte		        En2 , v036
	.byte		N05   , An2
	.byte	W06
	.byte		        En2 , v100
	.byte		N05   , An2
	.byte	W06
	.byte		        En2
	.byte		N05   , An2
	.byte	W06
	.byte		        En2
	.byte		N05   , An2
	.byte	W06
	.byte		        En2 , v036
	.byte		N05   , An2
	.byte	W06
	.byte		        En2 , v100
	.byte		N05   , An2
	.byte	W06
	.byte		        En2
	.byte		N05   , An2
	.byte	W06
	.byte		        En2
	.byte		N05   , An2
	.byte	W06
	.byte		        En2 , v036
	.byte		N05   , An2
	.byte	W06
	.byte		        En2 , v100
	.byte		N05   , An2
	.byte	W06
	.byte		        En2
	.byte		N05   , An2
	.byte	W06
	.byte		N23   , Cs3
	.byte		N23   , Gs3
	.byte	W24
	.byte		N05   , Ds2
	.byte		N05   , Bn2
	.byte	W06
	.byte		        Ds2 , v036
	.byte		N05   , Bn2
	.byte	W06
	.byte		        Ds2 , v100
	.byte		N05   , Bn2
	.byte	W06
	.byte		        Ds2
	.byte		N05   , Bn2
	.byte	W06
	.byte		        Ds2
	.byte		N05   , Bn2
	.byte	W06
	.byte		        Ds2 , v036
	.byte		N05   , Bn2
	.byte	W06
	.byte		        Ds2 , v100
	.byte		N05   , Bn2
	.byte	W06
	.byte		        Ds2
	.byte		N05   , Bn2
	.byte	W06
	.byte		        Ds2
	.byte		N05   , Bn2
	.byte	W06
	.byte		        Ds2 , v036
	.byte		N05   , Bn2
	.byte	W06
	.byte		        Ds2 , v100
	.byte		N05   , Bn2
	.byte	W06
	.byte		        Ds2
	.byte		N05   , Bn2
	.byte	W06
	.byte		N23   , Fs2
	.byte		N23   , Ds3
	.byte	W24
	.byte		N05   , Ds2
	.byte		N05   , Bn2
	.byte	W06
	.byte		        Ds2 , v036
	.byte		N05   , Bn2
	.byte	W06
	.byte		        Ds2 , v100
	.byte		N05   , Bn2
	.byte	W06
	.byte		        Ds2
	.byte		N05   , Bn2
	.byte	W06
	.byte		        Ds2
	.byte		N05   , Bn2
	.byte	W06
	.byte		        Ds2 , v036
	.byte		N05   , Bn2
	.byte	W06
	.byte		        Ds2 , v100
	.byte		N05   , Bn2
	.byte	W06
	.byte		        Ds2
	.byte		N05   , Bn2
	.byte	W06
	.byte		        Ds2
	.byte		N05   , Bn2
	.byte	W06
	.byte		        Ds2 , v036
	.byte		N05   , Bn2
	.byte	W06
	.byte		        Ds2 , v100
	.byte		N05   , Bn2
	.byte	W06
	.byte		        Ds2
	.byte		N05   , Bn2
	.byte	W06
	.byte	N23
	.byte		N23   , Fn3
	.byte	W24
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W48
	.byte		VOICE , 28
	.byte	W48
	.byte		N23   , Gn3
	.byte	W24
	.byte		        Fs3
	.byte	W24
	.byte		        Gn3
	.byte	W24
	.byte		        An3
	.byte	W24
	.byte		        Bn3
	.byte	W96
	.byte		        Gn3
	.byte	W24
	.byte		        Gn2
	.byte	W24
	.byte		        An3
	.byte	W24
	.byte		N11   , An2
	.byte	W12
	.byte	N03
	.byte	W04
	.byte	N03
	.byte	W04
	.byte	N03
	.byte	W04
	.byte	W96
	.byte	W72
	.byte		VOICE , 32
	.byte	W24
	.byte	PATT
	 ptrvalue	mus_qz_621_4_000
	.byte	GOTO
	 ptrvalue	mus_qz_621_4_B1
	.byte	FINE

/********************** Track  5 **********************/

mus_qz_621_5:
	.byte		KEYSH , mus_qz_621_key+0
	.byte		PAN   , c_v+0
	.byte		PRIO  , 61
	.byte		VOL   , 21*mus_qz_621_mvl/mxv
	.byte	W96
	.byte	W96
	.byte	W24
	.byte		        55*mus_qz_621_mvl/mxv
	.byte	W72
mus_qz_621_5_B1:
	.byte	W96
	.byte	W96
	.byte	W24
	.byte		VOICE , 23
	.byte	W48
	.byte		VOL   , 47*mus_qz_621_mvl/mxv
	.byte	W24
	.byte		PAN   , c_v-12
	.byte	W24
	.byte		N23   , Gs3 , v100
	.byte		N23   , En4
	.byte	W24
	.byte		N44   , Fs3 , v100 , gtp3
	.byte		N44   , Ds4 , v100 , gtp3
	.byte	W48
	.byte		        An3 , v100 , gtp3
	.byte		N44   , En4 , v100 , gtp3
	.byte	W48
	.byte		        Cs4 , v100 , gtp3
	.byte		N44   , Gs4 , v100 , gtp3
	.byte	W48
	.byte		N68   , Bn3 , v100 , gtp3
	.byte		N68   , Fs4 , v100 , gtp3
	.byte	W72
	.byte		N23   , Ds4
	.byte		N23   , An4
	.byte	W24
	.byte		N92   , Fs4 , v100 , gtp3
	.byte		N92   , Bn4 , v100 , gtp3
	.byte	W12
	.byte		VOL   , 47*mus_qz_621_mvl/mxv
	.byte	W05
	.byte		        45*mus_qz_621_mvl/mxv
	.byte	W07
	.byte		        43*mus_qz_621_mvl/mxv
	.byte	W36
	.byte	W03
	.byte		        47*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        48*mus_qz_621_mvl/mxv
	.byte	W07
	.byte		        50*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        52*mus_qz_621_mvl/mxv
	.byte	W06
	.byte		        53*mus_qz_621_mvl/mxv
	.byte	W06
	.byte		        57*mus_qz_621_mvl/mxv
	.byte	W10
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W60
	.byte		        78*mus_qz_621_mvl/mxv
	.byte		PAN   , c_v+12
	.byte	W12
	.byte		N23   , Fs2 , v120
	.byte	W24
	.byte		N44   , Ds2 , v120 , gtp3
	.byte		N44   , Bn2 , v120 , gtp3
	.byte	W36
	.byte	W02
	.byte		VOL   , 74*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        70*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        64*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        61*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        55*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        50*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        32*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        24*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        24*mus_qz_621_mvl/mxv
	.byte	W07
	.byte		        78*mus_qz_621_mvl/mxv
	.byte	W05
	.byte		N05   , Bn1
	.byte		N05   , Fs2
	.byte	W06
	.byte		        Bn1 , v020
	.byte		N05   , Fs2
	.byte	W06
	.byte		        Bn1 , v120
	.byte		N05   , Fs2
	.byte	W06
	.byte		        Bn1 , v020
	.byte		N05   , Fs2
	.byte	W06
	.byte		        Fs2 , v120
	.byte		N05   , Bn2
	.byte	W06
	.byte		        Fs2 , v020
	.byte		N05   , Bn2
	.byte	W06
	.byte		N23   , An2 , v120
	.byte		N23   , Cs3
	.byte	W24
	.byte		        En2
	.byte		N23   , Bn2
	.byte	W24
	.byte		        An2
	.byte		N23   , Cs3
	.byte	W14
	.byte		VOL   , 77*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        76*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        72*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        70*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        66*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        63*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        61*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        58*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        73*mus_qz_621_mvl/mxv
	.byte		N23   , Gn2
	.byte		N23   , En3
	.byte	W01
	.byte		VOL   , 77*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        78*mus_qz_621_mvl/mxv
	.byte	W22
	.byte		N80   , Fs2 , v120 , gtp3
	.byte		N80   , Ds3 , v120 , gtp3
	.byte	W06
	.byte		VOL   , 77*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        76*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        75*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        74*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        73*mus_qz_621_mvl/mxv
	.byte		        71*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        69*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        67*mus_qz_621_mvl/mxv
	.byte	W04
	.byte		        66*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        65*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        63*mus_qz_621_mvl/mxv
	.byte	W04
	.byte		        65*mus_qz_621_mvl/mxv
	.byte	W06
	.byte		        66*mus_qz_621_mvl/mxv
	.byte	W08
	.byte		        68*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        69*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        70*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        71*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        72*mus_qz_621_mvl/mxv
	.byte	W03
	.byte		        74*mus_qz_621_mvl/mxv
	.byte	W03
	.byte		        76*mus_qz_621_mvl/mxv
	.byte	W03
	.byte		        77*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        78*mus_qz_621_mvl/mxv
	.byte	W28
	.byte		        83*mus_qz_621_mvl/mxv
	.byte		N05   , Gn2
	.byte		N05   , En3
	.byte	W06
	.byte		        An2
	.byte		N05   , Fs3
	.byte	W06
	.byte		N23   , Dn3
	.byte		N23   , Bn3
	.byte	W24
	.byte		        Cs3
	.byte		N23   , An3
	.byte	W24
	.byte		        Dn3
	.byte		N23   , Bn3
	.byte	W24
	.byte		        En3
	.byte		N23   , Cs4
	.byte	W24
	.byte		        Ds3
	.byte		N23   , Bn3
	.byte	W24
	.byte		N32   , Bn2 , v120 , gtp3
	.byte		N32   , Fs3 , v120 , gtp3
	.byte	W18
	.byte		VOL   , 81*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        83*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        81*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        79*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        78*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        77*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        74*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        71*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        70*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        67*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        66*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        65*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        64*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        63*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        83*mus_qz_621_mvl/mxv
	.byte		N11   , Gn2
	.byte		N11   , En3
	.byte	W12
	.byte		N05   , Fs2
	.byte		N05   , Ds3
	.byte	W06
	.byte		        Gn2
	.byte		N05   , En3
	.byte	W06
	.byte		N11   , An2
	.byte		N11   , Fs3
	.byte	W12
	.byte		N44   , Bn2 , v120 , gtp3
	.byte		N44   , Gn3 , v120 , gtp3
	.byte	W08
	.byte		VOL   , 79*mus_qz_621_mvl/mxv
	.byte	W04
	.byte		        78*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        73*mus_qz_621_mvl/mxv
	.byte	W15
	.byte		        76*mus_qz_621_mvl/mxv
	.byte	W09
	.byte		        74*mus_qz_621_mvl/mxv
	.byte		        77*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        74*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        71*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        67*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        66*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        64*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        61*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        64*mus_qz_621_mvl/mxv
	.byte		        83*mus_qz_621_mvl/mxv
	.byte		N44   , Cs3 , v120 , gtp3
	.byte		N44   , An3 , v120 , gtp3
	.byte	W08
	.byte		VOL   , 79*mus_qz_621_mvl/mxv
	.byte	W04
	.byte		        78*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        73*mus_qz_621_mvl/mxv
	.byte	W10
	.byte		        79*mus_qz_621_mvl/mxv
	.byte	W03
	.byte		        83*mus_qz_621_mvl/mxv
	.byte	W10
	.byte		        81*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        78*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        75*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        72*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        67*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        63*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        58*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        50*mus_qz_621_mvl/mxv
	.byte	W01
	.byte		        42*mus_qz_621_mvl/mxv
	.byte	W02
	.byte		        83*mus_qz_621_mvl/mxv
	.byte		PAN   , c_v-5
	.byte		VOL   , 61*mus_qz_621_mvl/mxv
	.byte		N02   , Ds3 , v100
	.byte		N02   , Fs3
	.byte		N02   , Bn3
	.byte	W12
	.byte		        Ds3
	.byte		N02   , Fs3
	.byte		N02   , Bn3
	.byte	W06
	.byte		        Ds3
	.byte		N02   , Fs3
	.byte		N02   , Bn3
	.byte	W06
	.byte		        Ds3
	.byte		N02   , Fs3
	.byte		N02   , Bn3
	.byte	W12
	.byte		        Ds3
	.byte		N02   , Fs3
	.byte		N02   , Bn3
	.byte	W06
	.byte		        Ds3
	.byte		N02   , Fs3
	.byte		N02   , Bn3
	.byte	W06
	.byte		        Ds3
	.byte		N02   , Fs3
	.byte		N02   , Bn3
	.byte	W06
	.byte		        Ds3
	.byte		N02   , Fs3
	.byte		N02   , Bn3
	.byte	W06
	.byte		        Ds3
	.byte		N02   , Fs3
	.byte		N02   , Bn3
	.byte	W36
	.byte		        Ds3
	.byte		N02   , Fs3
	.byte		N02   , Bn3
	.byte	W12
	.byte		        Ds3
	.byte		N02   , Fs3
	.byte		N02   , Bn3
	.byte	W06
	.byte		        Ds3
	.byte		N02   , Fs3
	.byte		N02   , Bn3
	.byte	W06
	.byte		        Ds3
	.byte		N02   , Fs3
	.byte		N02   , Bn3
	.byte	W12
	.byte		        Ds3
	.byte		N02   , Fs3
	.byte		N02   , Bn3
	.byte	W06
	.byte		        Ds3
	.byte		N02   , Fs3
	.byte		N02   , Bn3
	.byte	W06
	.byte		        Ds3
	.byte		N02   , Fs3
	.byte		N02   , Bn3
	.byte	W06
	.byte		        Ds3
	.byte		N02   , Fs3
	.byte		N02   , Bn3
	.byte	W06
	.byte		        Ds3
	.byte		N02   , Fs3
	.byte		N02   , Bn3
	.byte	W36
	.byte	W24
	.byte		VOL   , 55*mus_qz_621_mvl/mxv
	.byte	W72
	.byte	GOTO
	 ptrvalue	mus_qz_621_5_B1
	.byte	FINE

/********************** Track  6 **********************/

mus_qz_621_6:
	.byte		KEYSH , mus_qz_621_key+0
	.byte		PAN   , c_v+0
	.byte		VOL   , 83*mus_qz_621_mvl/mxv
	.byte		PRIO  , 61
	.byte	W96
	.byte	W96
	.byte	W48
	.byte		PAN   , c_v+0
	.byte	W48
mus_qz_621_6_B1:
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
	.byte	W60
	.byte		VOL   , 61*mus_qz_621_mvl/mxv
	.byte	W12
	.byte		VOICE , 27
	.byte	W24
	.byte		PAN   , c_v-18
	.byte	W48
	.byte		N03   , Gn3 , v100
	.byte	W04
	.byte		        Dn3
	.byte	W04
	.byte		        Gn3
	.byte	W04
	.byte		        Dn4
	.byte	W04
	.byte		        Gn4
	.byte	W04
	.byte		        Bn4
	.byte	W04
	.byte		        An2
	.byte	W04
	.byte		        En2
	.byte	W04
	.byte		        Cs3
	.byte	W04
	.byte		        En3
	.byte	W04
	.byte		        Cs4
	.byte	W04
	.byte		        As4
	.byte	W04
	.byte	W96
	.byte		        Gn3
	.byte	W04
	.byte		        Dn3
	.byte	W04
	.byte		        Gn3
	.byte	W04
	.byte		        Bn3
	.byte	W04
	.byte		        Dn4
	.byte	W04
	.byte		        Gn4
	.byte	W04
	.byte		        Bn3
	.byte	W04
	.byte		        Dn4
	.byte	W04
	.byte		        Gn4
	.byte	W04
	.byte		        Bn4
	.byte	W04
	.byte		        Dn5
	.byte	W04
	.byte		        Gn5
	.byte	W04
	.byte		        An3
	.byte	W04
	.byte		        En3
	.byte	W04
	.byte		        An3
	.byte	W04
	.byte		        Cs4
	.byte	W04
	.byte		        En4
	.byte	W04
	.byte		        An4
	.byte	W04
	.byte		        Cs4
	.byte	W04
	.byte		        En4
	.byte	W04
	.byte		        An4
	.byte	W04
	.byte		        Cs5
	.byte	W04
	.byte		        En5
	.byte	W04
	.byte		        An5
	.byte	W04
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_621_6_B1
	.byte	FINE

/********************** Track  7 **********************/

mus_qz_621_7:
	.byte		KEYSH , mus_qz_621_key+0
	.byte		VOICE , 36
	.byte		PAN   , c_v+0
	.byte		PRIO  , 61
	.byte		VOL   , 70*mus_qz_621_mvl/mxv
	.byte		N11   , Bn0 , v127
	.byte	W12
	.byte		N05   , Bn0 , v020
	.byte	W60
	.byte		N17   , Fs0 , v127
	.byte	W18
	.byte		N05   , Fs0 , v020
	.byte	W06
	.byte		N11   , Bn0 , v127
	.byte	W12
	.byte		N05   , Bn0 , v020
	.byte	W60
	.byte		N17   , Fs0 , v127
	.byte	W18
	.byte		N05   , Fs0 , v020
	.byte	W06
mus_qz_621_7_000:
	.byte		N05   , Bn0 , v127
	.byte	W06
	.byte		        Bn0 , v020
	.byte	W30
	.byte		N11   , Fs1 , v127
	.byte	W12
	.byte		N05   , Bn0
	.byte	W06
	.byte		        Bn0 , v020
	.byte	W30
	.byte		N11   , Fs1 , v127
	.byte	W12
	.byte	PEND
mus_qz_621_7_B1:
	.byte		N05   , An1 , v127
	.byte	W06
	.byte		        An1 , v020
	.byte	W30
	.byte		N11   , En1 , v127
	.byte	W12
	.byte		N05   , An0
	.byte	W06
	.byte		        An0 , v020
	.byte	W30
	.byte		N11   , En1 , v127
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_621_7_000
	.byte		N05   , Bn1 , v127
	.byte	W06
	.byte		        Bn1 , v020
	.byte	W30
	.byte		N11   , Fs1 , v127
	.byte	W12
	.byte		N05   , Bn1
	.byte	W06
	.byte		        Bn1 , v020
	.byte	W06
	.byte		VOICE , 22
	.byte		VOL   , 67*mus_qz_621_mvl/mxv
	.byte		N05   , Fs0 , v127
	.byte	W06
	.byte		        Fs0 , v020
	.byte	W06
	.byte		        Gs0 , v127
	.byte	W06
	.byte		        Gs0 , v020
	.byte	W06
	.byte		        As0 , v127
	.byte	W06
	.byte		        As0 , v020
	.byte	W06
	.byte		VOL   , 64*mus_qz_621_mvl/mxv
	.byte		N05   , Bn0 , v127
	.byte	W06
	.byte		        Bn0 , v020
	.byte	W06
	.byte		        Bn2 , v068
	.byte	W12
	.byte		        Fs1 , v127
	.byte	W06
	.byte		        Fs1 , v020
	.byte	W06
	.byte		        Bn2 , v068
	.byte	W12
	.byte		        Bn0 , v127
	.byte	W06
	.byte		        Bn0 , v020
	.byte	W06
	.byte		        Bn2 , v068
	.byte	W12
	.byte		        Fs1 , v127
	.byte	W06
	.byte		        Fs1 , v020
	.byte	W06
	.byte		        Bn2 , v068
	.byte	W12
	.byte		        An1 , v127
	.byte	W06
	.byte		        An1 , v020
	.byte	W06
	.byte		        An2 , v068
	.byte	W12
	.byte		        En1 , v127
	.byte	W06
	.byte		        En1 , v020
	.byte	W06
	.byte		        An2 , v068
	.byte	W12
	.byte		        An0 , v127
	.byte	W06
	.byte		        An0 , v020
	.byte	W06
	.byte		        Cs3 , v068
	.byte	W12
	.byte		        En1 , v127
	.byte	W06
	.byte		        En1 , v020
	.byte	W06
	.byte		        Cs3 , v068
	.byte	W12
	.byte		        Bn0 , v127
	.byte	W06
	.byte		        Bn0 , v020
	.byte	W06
	.byte		        Ds3 , v068
	.byte	W12
	.byte		        Fs1 , v127
	.byte	W06
	.byte		        Fs1 , v020
	.byte	W06
	.byte		        Ds3 , v068
	.byte	W12
	.byte		        Bn0 , v127
	.byte	W06
	.byte		        Bn0 , v020
	.byte	W06
	.byte		        Ds3 , v068
	.byte	W12
	.byte		        Fs1 , v127
	.byte	W06
	.byte		        Fs1 , v020
	.byte	W06
	.byte		        Ds3 , v068
	.byte	W12
	.byte		        Bn1 , v127
	.byte	W06
	.byte		        Bn1 , v020
	.byte	W06
	.byte		        Bn2 , v068
	.byte	W12
	.byte		        Fs1 , v127
	.byte	W06
	.byte		        Fs1 , v020
	.byte	W06
	.byte		        Bn2 , v068
	.byte	W12
	.byte		        Bn0 , v127
	.byte	W06
	.byte		        Bn0 , v020
	.byte	W06
	.byte		        Bn2 , v068
	.byte	W12
	.byte		        Fs1 , v127
	.byte	W06
	.byte		        Fs1 , v020
	.byte	W06
	.byte		        Bn2 , v068
	.byte	W12
mus_qz_621_7_001:
	.byte		N05   , An1 , v127
	.byte	W06
	.byte		        An1 , v020
	.byte	W06
	.byte		        An1 , v127
	.byte	W06
	.byte		        An1 , v020
	.byte	W66
	.byte		N11   , En1 , v127
	.byte	W12
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_621_7_001
	.byte		N05   , Bn0 , v127
	.byte	W06
	.byte		        Bn0 , v020
	.byte	W06
	.byte		        Ds2 , v108
	.byte	W06
	.byte		        Ds2 , v020
	.byte	W06
	.byte		        Fs1 , v127
	.byte	W06
	.byte		        Fs1 , v020
	.byte	W06
	.byte		        Ds2 , v108
	.byte	W06
	.byte		        Ds2 , v020
	.byte	W06
	.byte		        Bn1 , v127
	.byte	W06
	.byte		        Bn1 , v020
	.byte	W06
	.byte		        Ds2 , v108
	.byte	W06
	.byte		        Ds2 , v020
	.byte	W06
	.byte		        Fs1 , v127
	.byte	W06
	.byte		        Fs1 , v020
	.byte	W06
	.byte		        Ds2 , v108
	.byte	W06
	.byte		        Ds2 , v020
	.byte	W06
	.byte		        Bn1 , v127
	.byte	W06
	.byte		        Bn1 , v020
	.byte	W06
	.byte		        Fs2 , v108
	.byte	W06
	.byte		        Fs2 , v020
	.byte	W06
	.byte		        Fs1 , v127
	.byte	W06
	.byte		        Fs1 , v020
	.byte	W06
	.byte		        Fs2 , v108
	.byte	W06
	.byte		        Fs2 , v020
	.byte	W06
	.byte		        Bn1 , v127
	.byte	W06
	.byte		        Bn1 , v020
	.byte	W06
	.byte		        Fs1 , v127
	.byte	W06
	.byte		        Fs1 , v020
	.byte	W06
	.byte		        Cs2 , v108
	.byte	W06
	.byte		        Cs2 , v020
	.byte	W06
	.byte		        Bn1 , v127
	.byte	W06
	.byte		        Bn1 , v020
	.byte	W06
	.byte		        An1 , v127
	.byte	W06
	.byte		        An1 , v020
	.byte	W06
	.byte		        En2 , v108
	.byte	W06
	.byte		        En2 , v020
	.byte	W06
	.byte		        En1 , v127
	.byte	W06
	.byte		        En1 , v020
	.byte	W06
	.byte		        En2 , v108
	.byte	W06
	.byte		        En2 , v020
	.byte	W06
	.byte		        An1 , v127
	.byte	W06
	.byte		        An1 , v020
	.byte	W06
	.byte		        En2 , v108
	.byte	W06
	.byte		        En2 , v020
	.byte	W06
	.byte		        En1 , v127
	.byte	W06
	.byte		        En1 , v020
	.byte	W06
	.byte		        En2 , v108
	.byte	W06
	.byte		        En2 , v020
	.byte	W06
	.byte		        An1 , v127
	.byte	W06
	.byte		        An1 , v020
	.byte	W06
	.byte		        En2 , v108
	.byte	W06
	.byte		        En2 , v020
	.byte	W06
	.byte		        En1 , v127
	.byte	W06
	.byte		        En1 , v020
	.byte	W06
	.byte		        En2 , v108
	.byte	W06
	.byte		        En2 , v020
	.byte	W06
	.byte		        An1 , v127
	.byte	W06
	.byte		        An1 , v020
	.byte	W06
	.byte		        An2 , v108
	.byte	W06
	.byte		        An2 , v020
	.byte	W06
	.byte		        En2 , v108
	.byte	W06
	.byte		        En2 , v020
	.byte	W06
	.byte		        Cs3 , v108
	.byte	W06
	.byte		        Cs3 , v020
	.byte	W06
	.byte		        Bn1 , v127
	.byte	W06
	.byte		        Bn1 , v020
	.byte	W06
	.byte		        Fs2 , v108
	.byte	W06
	.byte		        Fs2 , v020
	.byte	W06
	.byte		        Fs1 , v127
	.byte	W06
	.byte		        Fs1 , v020
	.byte	W06
	.byte		        Fs2 , v108
	.byte	W06
	.byte		        Fs2 , v020
	.byte	W06
	.byte		        Bn1 , v127
	.byte	W06
	.byte		        Bn1 , v020
	.byte	W06
	.byte		        Bn2 , v108
	.byte	W06
	.byte		        Bn2 , v020
	.byte	W06
	.byte		        Fs2 , v108
	.byte	W06
	.byte		        Fs2 , v020
	.byte	W06
	.byte		        Ds3 , v108
	.byte	W06
	.byte		        Ds3 , v020
	.byte	W06
	.byte		        Bn1 , v127
	.byte	W06
	.byte		        Bn1 , v020
	.byte	W06
	.byte		        Fs2 , v108
	.byte	W06
	.byte		        Fs2 , v020
	.byte	W06
	.byte		        Fs1 , v127
	.byte	W06
	.byte		        Fs1 , v020
	.byte	W06
	.byte		        Fs2 , v108
	.byte	W06
	.byte		        Fs2 , v020
	.byte	W06
	.byte		        Bn1 , v127
	.byte	W06
	.byte		        Bn1 , v020
	.byte	W06
	.byte		        Fs2 , v108
	.byte	W06
	.byte		        Fs2 , v020
	.byte	W06
	.byte		        Bn2 , v096
	.byte	W06
	.byte		        Bn2 , v020
	.byte	W06
	.byte		        Fn3 , v080
	.byte	W06
	.byte		        Fn3 , v020
	.byte	W06
	.byte		VOL   , 70*mus_qz_621_mvl/mxv
	.byte		N05   , Bn0 , v127
	.byte	W06
	.byte		        Bn0 , v020
	.byte	W06
	.byte		        Bn0 , v127
	.byte	W06
	.byte		        Bn0 , v020
	.byte	W54
	.byte		N17   , Fs1 , v127
	.byte	W18
	.byte		N05   , Fs1 , v020
	.byte	W06
mus_qz_621_7_002:
	.byte		N05   , Bn0 , v127
	.byte	W06
	.byte		        Bn0 , v020
	.byte	W06
	.byte		        Bn0 , v127
	.byte	W06
	.byte		        Bn0 , v020
	.byte	W54
	.byte		N17   , Fs1 , v127
	.byte	W18
	.byte		N05   , Fs1 , v020
	.byte	W06
	.byte	PEND
	.byte		        Bn0 , v127
	.byte	W06
	.byte		        Bn0 , v020
	.byte	W06
	.byte		        Bn0 , v127
	.byte	W06
	.byte		        Bn0 , v020
	.byte	W30
	.byte		        Bn0 , v127
	.byte	W06
	.byte		        Bn0 , v020
	.byte	W06
	.byte		        Bn0 , v127
	.byte	W06
	.byte		        Bn0 , v020
	.byte	W30
	.byte		        An0 , v127
	.byte	W06
	.byte		        An0 , v020
	.byte	W06
	.byte		        An0 , v127
	.byte	W06
	.byte		        An0 , v020
	.byte	W30
	.byte		        An0 , v127
	.byte	W06
	.byte		        An0 , v020
	.byte	W06
	.byte		        An0 , v127
	.byte	W06
	.byte		        An0 , v020
	.byte	W30
	.byte		        Bn0 , v127
	.byte	W06
	.byte		        Bn0 , v020
	.byte	W06
	.byte		        Bn0 , v127
	.byte	W06
	.byte		        Bn0 , v020
	.byte	W30
	.byte		        An0 , v127
	.byte	W06
	.byte		        An0 , v020
	.byte	W06
	.byte		        An0 , v127
	.byte	W06
	.byte		        An0 , v020
	.byte	W30
	.byte		VOL   , 75*mus_qz_621_mvl/mxv
	.byte		N17   , Gn0 , v127
	.byte	W18
	.byte		N05   , Gn0 , v020
	.byte	W06
	.byte		N17   , Fs0 , v127
	.byte	W18
	.byte		N05   , Fs0 , v020
	.byte	W06
	.byte		N17   , Gn0 , v127
	.byte	W18
	.byte		N05   , Gn0 , v020
	.byte	W06
	.byte		N17   , An0 , v127
	.byte	W18
	.byte		N05   , An0 , v020
	.byte	W06
	.byte		VOL   , 70*mus_qz_621_mvl/mxv
	.byte		N11   , Bn0 , v127
	.byte	W12
	.byte		N05   , Bn0 , v020
	.byte	W12
	.byte		N11   , Fs0 , v127
	.byte	W12
	.byte		N05   , Fs0 , v020
	.byte	W12
	.byte		N11   , Bn0 , v127
	.byte	W12
	.byte		N05   , Bn0 , v020
	.byte	W12
	.byte		N11   , Fs0 , v127
	.byte	W12
	.byte		N05   , Fs0 , v020
	.byte	W12
	.byte		        Dn1 , v127
	.byte	W06
	.byte		        Dn1 , v020
	.byte	W06
	.byte		        Dn1 , v127
	.byte	W06
	.byte		        Dn1 , v020
	.byte	W06
	.byte		        Gn0 , v127
	.byte	W06
	.byte		        Gn0 , v020
	.byte	W18
	.byte		        En1 , v127
	.byte	W06
	.byte		        En1 , v020
	.byte	W06
	.byte		        En1 , v127
	.byte	W06
	.byte		        En1 , v020
	.byte	W06
	.byte		        An0 , v127
	.byte	W06
	.byte		        An0 , v020
	.byte	W18
	.byte		VOICE , 36
	.byte		N05   , Bn0 , v127
	.byte	W06
	.byte		        Bn0 , v020
	.byte	W06
	.byte		        Bn0 , v127
	.byte	W06
	.byte		        Bn0 , v020
	.byte	W54
	.byte		N17   , Fs1 , v127
	.byte	W18
	.byte		N05   , Fs1 , v020
	.byte	W06
	.byte	PATT
	 ptrvalue	mus_qz_621_7_002
	.byte	PATT
	 ptrvalue	mus_qz_621_7_000
	.byte	GOTO
	 ptrvalue	mus_qz_621_7_B1
	.byte	FINE

/********************** Track  8 **********************/

mus_qz_621_8:
	.byte		KEYSH , mus_qz_621_key+0
	.byte		VOICE , 28
	.byte		PAN   , c_v+0
	.byte		PRIO  , 34
	.byte		VOL   , 52*mus_qz_621_mvl/mxv
	.byte		PAN   , c_v-10
	.byte		N20   , Cn2 , v100
	.byte	W72
	.byte	N20
	.byte	W24
	.byte	N20
	.byte	W96
	.byte	N20
	.byte	W96
mus_qz_621_8_B1:
	.byte		VOL   , 52*mus_qz_621_mvl/mxv
	.byte	W96
	.byte		N20   , Cn2 , v100
	.byte	W96
	.byte	W72
	.byte	N20
	.byte	W24
	.byte	N20
	.byte	W96
	.byte	W96
	.byte	N20
	.byte	W96
	.byte	W96
	.byte		VOL   , 63*mus_qz_621_mvl/mxv
	.byte	N20
	.byte	W96
	.byte	W72
	.byte	N20
	.byte	W24
	.byte	N20
	.byte	W96
	.byte	N20
	.byte	W96
	.byte	N20
	.byte	W96
	.byte	W72
	.byte	N20
	.byte	W24
	.byte	N20
	.byte	W96
	.byte	N20
	.byte	W72
	.byte	N20
	.byte	W24
	.byte	N20
	.byte	W96
	.byte	N20
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		        Cn2 , v088
	.byte	W24
	.byte		        Cn2 , v092
	.byte	W24
	.byte		        Cn2 , v100
	.byte	W24
	.byte		        Cn2 , v108
	.byte	W24
	.byte		        Cn2 , v100
	.byte	W96
	.byte	W24
	.byte	N20
	.byte	W48
	.byte	N20
	.byte	W24
	.byte	N20
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_621_8_B1
	.byte	FINE

/********************** Track  9 **********************/

mus_qz_621_9:
	.byte		KEYSH , mus_qz_621_key+0
	.byte		VOICE , 39
	.byte		PAN   , c_v+0
	.byte		PRIO  , 61
	.byte		VOL   , 50*mus_qz_621_mvl/mxv
	.byte		N05   , Dn1 , v100
	.byte	W12
	.byte		        Dn1 , v088
	.byte	W12
	.byte		        Dn1 , v100
	.byte	W12
	.byte		N03   , Dn1 , v080
	.byte	W04
	.byte		        Dn1 , v072
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		N05   , Dn1 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W12
	.byte		N03   , Dn1 , v080
	.byte	W04
	.byte		        Dn1 , v060
	.byte	W04
	.byte	N03
	.byte	W04
	.byte	N03
	.byte	W04
	.byte	N03
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		N05   , Dn1 , v100
	.byte	W12
	.byte		        Dn1 , v088
	.byte	W12
	.byte		        Dn1 , v100
	.byte	W12
	.byte		N03   , Dn1 , v080
	.byte	W04
	.byte		        Dn1 , v072
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		N05   , Dn1 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W12
	.byte		N03   , Dn1 , v080
	.byte	W04
	.byte		        Dn1 , v060
	.byte	W04
	.byte	N03
	.byte	W04
	.byte	N03
	.byte	W04
	.byte	N03
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		VOL   , 66*mus_qz_621_mvl/mxv
	.byte		N05   , Dn1 , v100
	.byte	W12
	.byte		        Dn1 , v088
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		N11   , Cn2 , v068
	.byte	W12
	.byte		N03   , Dn1 , v080
	.byte	W04
	.byte		        Dn1 , v060
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		N05   , Dn1 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W12
	.byte		N11   , Cn2 , v068
	.byte	W12
	.byte		N03   , Dn1 , v080
	.byte	W04
	.byte		        Dn1 , v060
	.byte	W04
	.byte	N03
	.byte	W04
mus_qz_621_9_B1:
	.byte		N05   , Dn1 , v100
	.byte	W12
	.byte		        Dn1 , v088
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		N11   , Cn2 , v068
	.byte	W12
	.byte		N03   , Dn1 , v080
	.byte	W04
	.byte		        Dn1 , v060
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		N05   , Dn1 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W12
	.byte		N11   , Cn2 , v068
	.byte	W12
	.byte		N03   , Dn1 , v080
	.byte	W04
	.byte		        Dn1 , v060
	.byte	W04
	.byte	N03
	.byte	W04
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_621_9_B1
	.byte		N05   , Dn1 , v100
	.byte	W12
	.byte		        Dn1 , v088
	.byte	W12
	.byte		N11   , Cn2 , v068
	.byte	W12
	.byte		N03   , Dn1 , v080
	.byte	W04
	.byte		        Dn1 , v060
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		N05   , Dn1 , v100
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte		N03   , Dn1 , v080
	.byte	W04
	.byte		        Dn1 , v060
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		N05   , Dn1 , v100
	.byte		N17   , Cn2
	.byte	W12
	.byte		N03   , Dn1 , v080
	.byte	W04
	.byte		        Dn1 , v060
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		N05   , Dn1 , v100
	.byte		N23   , Cn2
	.byte	W12
	.byte		N05   , Dn1 , v088
	.byte	W12
	.byte		        Dn1 , v100
	.byte	W12
	.byte		N03   , Dn1 , v080
	.byte	W04
	.byte		        Dn1 , v060
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		N05   , Dn1 , v100
	.byte	W12
	.byte	N05
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v100
	.byte	W12
	.byte		N03   , Dn1 , v080
	.byte	W04
	.byte		        Dn1 , v060
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		N05   , Dn1 , v100
	.byte	W12
	.byte		        Dn1 , v088
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Dn1 , v100
	.byte	W12
	.byte		        Dn1 , v088
	.byte	W12
	.byte		        Dn1 , v100
	.byte	W12
	.byte		N03   , Dn1 , v080
	.byte	W04
	.byte		        Dn1 , v060
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		N05   , Dn1 , v100
	.byte	W12
	.byte		        Dn1 , v088
	.byte	W12
	.byte		        Dn1 , v100
	.byte	W12
	.byte		        Dn1 , v088
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Dn1 , v100
	.byte	W12
	.byte		        Dn1 , v088
	.byte	W12
	.byte		        Dn1 , v100
	.byte	W12
	.byte		        Dn1 , v088
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Dn1 , v100
	.byte	W12
	.byte		N03   , Dn1 , v080
	.byte	W04
	.byte		        Dn1 , v060
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		N05   , Dn1 , v100
	.byte	W12
	.byte		        Dn1 , v088
	.byte	W12
	.byte		        Dn1 , v100
	.byte	W12
	.byte		N03   , Dn1 , v080
	.byte	W04
	.byte		        Dn1 , v060
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		N05   , Dn1 , v100
	.byte	W12
	.byte		        Dn1 , v088
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Dn1 , v100
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte	N05
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Dn1 , v100
	.byte		N44   , Cn2 , v108 , gtp3
	.byte	W12
	.byte		N05   , Dn1 , v088
	.byte	W12
	.byte		        Dn1 , v100
	.byte	W12
	.byte		N03   , Dn1 , v080
	.byte	W04
	.byte		        Dn1 , v072
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		N05   , Dn1 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W12
	.byte		N03   , Dn1 , v080
	.byte	W04
	.byte		        Dn1 , v060
	.byte	W04
	.byte	N03
	.byte	W04
	.byte	N03
	.byte	W04
	.byte	N03
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		N05   , Dn1 , v100
	.byte	W12
	.byte		        Dn1 , v088
	.byte	W12
	.byte		        Dn1 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W12
	.byte		        Dn1 , v100
	.byte	W12
	.byte		        Dn1 , v088
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		N03   , Dn1 , v080
	.byte		N23   , Cn2 , v108
	.byte	W04
	.byte		N03   , Dn1 , v060
	.byte	W04
	.byte	N03
	.byte	W04
	.byte	N03
	.byte	W04
	.byte	N03
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		N05   , Dn1 , v100
	.byte		N44   , Cn2 , v108 , gtp3
	.byte	W12
	.byte		N05   , Dn1 , v088
	.byte	W12
	.byte		        Dn1 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		N03   , Dn1 , v080
	.byte	W04
	.byte		        Dn1 , v060
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		N05   , Dn1 , v100
	.byte	W12
	.byte		        Dn1 , v088
	.byte	W12
	.byte		        Dn1 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W12
	.byte		        Dn1 , v100
	.byte	W12
	.byte		N03   , Dn1 , v080
	.byte	W04
	.byte		        Dn1 , v060
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		N05   , Dn1 , v100
	.byte	W12
	.byte		N03   , Dn1 , v080
	.byte	W04
	.byte		        Dn1 , v060
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		N05   , Dn1 , v100
	.byte	W12
	.byte		        Dn1 , v088
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Dn1 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W12
	.byte		        Dn1 , v100
	.byte		N44   , Cn2 , v108 , gtp3
	.byte	W12
	.byte		N05   , Dn1 , v088
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Dn1 , v100
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte	N05
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		N03   , Dn1 , v080
	.byte	W04
	.byte		        Dn1 , v060
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		N05   , Dn1 , v088
	.byte	W12
	.byte		        Dn1 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W12
	.byte		        Dn1 , v100
	.byte	W12
	.byte		        Dn1 , v088
	.byte	W12
	.byte		        Dn1 , v100
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte	N05
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Dn1 , v100
	.byte	W12
	.byte		N03   , Dn1 , v080
	.byte	W04
	.byte		        Dn1 , v060
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		N05   , Dn1 , v100
	.byte		N23   , Cn2 , v108
	.byte	W12
	.byte		N05   , Dn1 , v088
	.byte	W12
	.byte		        Dn1 , v100
	.byte		N44   , Cn2 , v108 , gtp3
	.byte	W12
	.byte		N05   , Dn1 , v088
	.byte	W12
	.byte		        Dn1 , v100
	.byte	W12
	.byte		N03   , Dn1 , v080
	.byte	W04
	.byte		        Dn1 , v060
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		N05   , Dn1 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W12
	.byte		        Dn1 , v100
	.byte	W12
	.byte		N03   , Dn1 , v080
	.byte	W04
	.byte		        Dn1 , v060
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		        Dn1 , v080
	.byte		N44   , Cn2 , v108 , gtp3
	.byte	W04
	.byte		N03   , Dn1 , v060
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		N05   , Dn1 , v088
	.byte	W12
	.byte		        Dn1 , v100
	.byte	W12
	.byte		        Dn1 , v088
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Dn1 , v100
	.byte	W12
	.byte		        Dn1 , v088
	.byte	W12
	.byte		N03   , Dn1 , v080
	.byte		N23   , Cn2 , v108
	.byte	W04
	.byte		N03   , Dn1 , v060
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		        Dn1 , v080
	.byte	W04
	.byte		        Dn1 , v060
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		N05   , Dn1 , v100
	.byte		N44   , Cn2 , v108 , gtp3
	.byte	W12
	.byte		N05   , Dn1 , v088
	.byte	W12
	.byte		        Dn1 , v100
	.byte	W12
	.byte		N03   , Dn1 , v080
	.byte	W04
	.byte		        Dn1 , v060
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		N05   , Dn1 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W12
	.byte		N03   , Dn1 , v080
	.byte	W04
	.byte		        Dn1 , v060
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		        Dn1 , v080
	.byte	W04
	.byte		        Dn1 , v060
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		N05   , Dn1 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W12
	.byte		        Dn1 , v100
	.byte	W12
	.byte		N03   , Dn1 , v080
	.byte	W04
	.byte		        Dn1 , v060
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		N05   , Dn1 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W12
	.byte		N03   , Dn1 , v080
	.byte	W04
	.byte		        Dn1 , v060
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		        Dn1 , v080
	.byte	W04
	.byte		        Dn1 , v060
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		N05   , Dn1 , v100
	.byte	W12
	.byte		        Dn1 , v088
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Dn1 , v100
	.byte		N23   , Cn2 , v072
	.byte	W12
	.byte		N05   , Dn1 , v088
	.byte	W12
	.byte		        Dn1 , v100
	.byte	W12
	.byte		N03   , Dn1 , v080
	.byte	W04
	.byte		        Dn1 , v060
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		N05   , Dn1 , v100
	.byte		N23   , Cn2 , v072
	.byte	W12
	.byte		N05   , Dn1 , v088
	.byte	W12
	.byte		        Dn1 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W12
	.byte		        Dn1 , v100
	.byte		N23   , Cn2 , v072
	.byte	W12
	.byte		N05   , Dn1 , v088
	.byte	W12
	.byte		        Dn1 , v100
	.byte	W12
	.byte		N03   , Dn1 , v080
	.byte	W04
	.byte		        Dn1 , v060
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		N05   , Dn1 , v100
	.byte		N23   , Cn2 , v072
	.byte	W12
	.byte		N05   , Dn1 , v088
	.byte	W12
	.byte		        Dn1 , v100
	.byte	W12
	.byte		        Dn1 , v088
	.byte	W12
	.byte		        Dn1 , v100
	.byte		N23   , Cn2 , v072
	.byte	W24
	.byte		N05   , Dn1 , v100
	.byte	W12
	.byte		        Dn1 , v088
	.byte	W12
	.byte		N03   , Dn1 , v080
	.byte		N23   , Cn2
	.byte	W04
	.byte		N03   , Dn1 , v060
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		        Dn1 , v080
	.byte	W04
	.byte		        Dn1 , v060
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		N05   , Dn1 , v100
	.byte		N23   , Cn2 , v088
	.byte	W12
	.byte		N03   , Dn1 , v080
	.byte	W04
	.byte		        Dn1 , v060
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		N05   , Dn1 , v100
	.byte		N23   , Cn2 , v096
	.byte	W12
	.byte		N03   , Dn1 , v080
	.byte	W04
	.byte		        Dn1 , v060
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		N05   , Dn1 , v100
	.byte		N23   , Cn2 , v096
	.byte	W06
	.byte		N05   , Dn1 , v088
	.byte	W06
	.byte	N05
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Dn1 , v100
	.byte		N23   , Cn2 , v108
	.byte	W12
	.byte		N03   , Dn1 , v080
	.byte	W04
	.byte		        Dn1 , v060
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		N05   , Dn1 , v100
	.byte		N44   , Cn2 , v108 , gtp3
	.byte	W12
	.byte		N05   , Dn1 , v088
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Dn1 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W12
	.byte		        Dn1 , v100
	.byte	W12
	.byte		        Dn1 , v088
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Dn1 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W12
	.byte		        Dn1 , v100
	.byte		N11   , Cn2
	.byte	W12
	.byte		N05   , Dn1 , v088
	.byte		N11   , Cn2 , v100
	.byte	W12
	.byte		N03   , Dn1 , v080
	.byte		N23   , Cn2 , v100
	.byte	W04
	.byte		N03   , Dn1 , v060
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		        Dn1 , v080
	.byte	W04
	.byte		        Dn1 , v060
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		N44   , Gn0 , v100 , gtp3
	.byte		N05   , Dn1
	.byte		N11   , Cn2
	.byte	W12
	.byte		N05   , Dn1 , v088
	.byte		N11   , Cn2 , v100
	.byte	W12
	.byte		N03   , Dn1 , v080
	.byte		N23   , Cn2 , v100
	.byte	W04
	.byte		N03   , Dn1 , v060
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		        Dn1 , v080
	.byte	W04
	.byte		        Dn1 , v060
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		N05   , Dn1 , v100
	.byte		N52   , Cn2 , v100 , gtp1
	.byte	W12
	.byte		N05   , Dn1 , v088
	.byte	W12
	.byte		        Dn1 , v100
	.byte	W12
	.byte		N03   , Dn1 , v080
	.byte	W04
	.byte		        Dn1 , v060
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		N05   , Dn1 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W12
	.byte		N03   , Dn1 , v080
	.byte	W04
	.byte		        Dn1 , v060
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		        Dn1 , v080
	.byte	W04
	.byte		        Dn1 , v060
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		N05   , Dn1 , v100
	.byte	W12
	.byte		        Dn1 , v088
	.byte	W12
	.byte		        Dn1 , v100
	.byte	W12
	.byte		N03   , Dn1 , v080
	.byte	W04
	.byte		        Dn1 , v060
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		N05   , Dn1 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W12
	.byte		N03   , Dn1 , v080
	.byte	W04
	.byte		        Dn1 , v060
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		        Dn1 , v080
	.byte	W04
	.byte		        Dn1 , v060
	.byte	W04
	.byte	N03
	.byte	W04
	.byte	PATT
	 ptrvalue	mus_qz_621_9_B1
	.byte	GOTO
	 ptrvalue	mus_qz_621_9_B1
	.byte	FINE

/******************************************************/
	.p2align	2

mus_qz_621:
	.byte	9	/* NumTrks */
	.byte	0	/* NumBlks */
	.byte	mus_qz_621_pri	/* Priority */
	.byte	mus_qz_621_rev	/* Reverb. */
space64 4
	ptrvalue	mus_qz_621_grp

	ptrvalue	mus_qz_621_1
	ptrvalue	mus_qz_621_2
	ptrvalue	mus_qz_621_3
	ptrvalue	mus_qz_621_4
	ptrvalue	mus_qz_621_5
	ptrvalue	mus_qz_621_6
	ptrvalue	mus_qz_621_7
	ptrvalue	mus_qz_621_8
	ptrvalue	mus_qz_621_9

	.end
