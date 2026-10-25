	.include "asm/macros/bit_width.inc"

	.include "MPlayDef.s"

/* Moemon Quetzal song 620, disassembled from the ROM by tools/quetzal_rip/songs.py.
   "org" comments give the ROM address of the bytes that follow. */

	.equ	mus_qz_620_grp, voicegroup_qz_songs
	.equ	mus_qz_620_pri, 0
	.equ	mus_qz_620_rev, reverb_set+0
	.equ	mus_qz_620_mvl, 127
	.equ	mus_qz_620_key, 0
	.equ	mus_qz_620_tbs, 1
	.equ	mus_qz_620_exg, 1
	.equ	mus_qz_620_cmp, 1

	.section .rodata
	.global	mus_qz_620
	.p2align	2
/* org 0x0967788C */

/********************** Track  1 **********************/

mus_qz_620_1:
	.byte		KEYSH , mus_qz_620_key+0
	.byte		TEMPO , 132*mus_qz_620_tbs/2
	.byte		VOICE , 29
	.byte		VOL   , 78*mus_qz_620_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte		N03   , Fn3 , v116
	.byte	W12
	.byte		        Cn3 , v096
	.byte	W06
	.byte		        Fn3
	.byte	W06
	.byte		N32   , An3 , v096 , gtp3
	.byte	W48
	.byte		N23   , Fn3
	.byte	W24
	.byte		N03   , Cn4 , v108
	.byte	W12
	.byte		        An3 , v096
	.byte	W06
	.byte		        Cn4
	.byte	W06
	.byte		N32   , Fn4 , v096 , gtp3
	.byte	W48
	.byte		N23   , As3
	.byte	W24
mus_qz_620_1_000:
	.byte		N44   , Cn4 , v096 , gtp3
	.byte	W60
	.byte		N11   , An3
	.byte	W12
	.byte		N05   , Cn4
	.byte	W12
	.byte		        An4
	.byte	W12
	.byte	PEND
mus_qz_620_1_001:
	.byte		N17   , Gn4 , v096
	.byte	W18
	.byte		N64   , Ds4 , v092 , gtp1
	.byte	W78
	.byte	PEND
	.byte		TEMPO , 132*mus_qz_620_tbs/2
	.byte		N52   , Cn4 , v096 , gtp1
	.byte	W60
	.byte		N11   , An3
	.byte	W12
	.byte		N05   , Cn4
	.byte	W12
	.byte		        Fn4
	.byte	W12
mus_qz_620_1_002:
	.byte		N05   , Ds4 , v096
	.byte	W12
	.byte		        Ds4 , v092
	.byte	W06
	.byte		        Fn4
	.byte	W06
	.byte		N56   , Gn4 , v096 , gtp3
	.byte	W72
	.byte	PEND
mus_qz_620_1_003:
	.byte		N52   , An4 , v096 , gtp1
	.byte	W60
	.byte		N11   , Fn4
	.byte	W12
	.byte		N05   , An4
	.byte	W12
	.byte		        Cn5
	.byte	W12
	.byte	PEND
mus_qz_620_1_004:
	.byte		N05   , As4 , v096
	.byte	W12
	.byte		        As4 , v088
	.byte	W06
	.byte		        An4
	.byte	W06
	.byte		N44   , Gn4 , v096 , gtp3
	.byte	W48
	.byte		N23   , As4
	.byte	W24
	.byte	PEND
mus_qz_620_1_005:
	.byte		TIE   , An4 , v096
	.byte	W21
	.byte		VOL   , 74*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        70*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        70*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        66*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        63*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        61*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        60*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        59*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        58*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        56*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        53*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        52*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        50*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        49*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        49*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        47*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        46*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        47*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        49*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        49*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        49*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        50*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        55*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        55*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        57*mus_qz_620_mvl/mxv
	.byte	W03
	.byte	PEND
mus_qz_620_1_006:
	.byte		VOL   , 59*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        65*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        65*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        66*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        70*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        76*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        81*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        84*mus_qz_620_mvl/mxv
	.byte	W21
	.byte		        66*mus_qz_620_mvl/mxv
	.byte	W06
	.byte		        58*mus_qz_620_mvl/mxv
	.byte	W06
	.byte		        46*mus_qz_620_mvl/mxv
	.byte	W06
	.byte		        34*mus_qz_620_mvl/mxv
	.byte	W06
	.byte		        26*mus_qz_620_mvl/mxv
	.byte	W05
	.byte	PEND
	.byte		EOT   , An4
	.byte	W01
	.byte		VOL   , 19*mus_qz_620_mvl/mxv
	.byte	W06
	.byte		        17*mus_qz_620_mvl/mxv
	.byte	W06
	.byte		        15*mus_qz_620_mvl/mxv
	.byte	W12
mus_qz_620_1_B1:
	.byte		TEMPO , 132*mus_qz_620_tbs/2
	.byte		VOL   , 76*mus_qz_620_mvl/mxv
	.byte	W96
	.byte	W96
	.byte		TEMPO , 132*mus_qz_620_tbs/2
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		N03   , An3 , v100
	.byte	W12
	.byte		        Fs3 , v092
	.byte	W06
	.byte		        An3 , v100
	.byte	W06
	.byte		N56   , Dn4 , v100 , gtp3
	.byte	W72
	.byte		N03   , Fs3
	.byte	W12
	.byte		        Dn3 , v092
	.byte	W06
	.byte		        Fs3 , v100
	.byte	W06
	.byte		N64   , An3 , v100 , gtp1
	.byte	W72
	.byte		N03   , Gn3
	.byte	W12
	.byte		        En3 , v092
	.byte	W06
	.byte		        Gn3 , v100
	.byte	W06
	.byte		N56   , Cn4 , v100 , gtp3
	.byte	W72
	.byte		N03   , Gn3
	.byte	W12
	.byte		        En3 , v092
	.byte	W06
	.byte		        Gn3 , v100
	.byte	W06
	.byte		N40   , Cn4 , v100 , gtp1
	.byte	W48
	.byte		N23   , Cs4
	.byte	W24
	.byte	W96
	.byte	W72
	.byte		N05   , Fs3
	.byte	W06
	.byte		        Gn3
	.byte	W06
	.byte		        An3
	.byte	W06
	.byte		        Bn3
	.byte	W06
	.byte	PATT
	 ptrvalue	mus_qz_620_1_000
	.byte	PATT
	 ptrvalue	mus_qz_620_1_001
	.byte		N52   , Cn4 , v096 , gtp1
	.byte	W60
	.byte		N11   , An3
	.byte	W12
	.byte		N05   , Cn4
	.byte	W12
	.byte		        Fn4
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_620_1_002
	.byte	PATT
	 ptrvalue	mus_qz_620_1_003
	.byte	PATT
	 ptrvalue	mus_qz_620_1_004
	.byte	PATT
	 ptrvalue	mus_qz_620_1_005
	.byte	PATT
	 ptrvalue	mus_qz_620_1_006
	.byte		EOT   , An4
	.byte	W01
	.byte		VOL   , 19*mus_qz_620_mvl/mxv
	.byte	W06
	.byte		        17*mus_qz_620_mvl/mxv
	.byte	W06
	.byte		        15*mus_qz_620_mvl/mxv
	.byte	W12
	.byte	GOTO
	 ptrvalue	mus_qz_620_1_B1
	.byte	FINE

/********************** Track  2 **********************/

mus_qz_620_2:
	.byte		KEYSH , mus_qz_620_key+0
	.byte		VOICE , 23
	.byte		VOL   , 61*mus_qz_620_mvl/mxv
	.byte		PAN   , c_v-54
	.byte		PRIO  , 62
	.byte		N05   , Fn3 , v100
	.byte		N05   , Cn4
	.byte	W72
	.byte		N23   , Fn3
	.byte		N23   , Cn4
	.byte	W24
	.byte		N05   , Ds3 , v108
	.byte		N05   , As3
	.byte	W72
	.byte		N23   , Gn3 , v100
	.byte		N23   , Ds4
	.byte	W24
mus_qz_620_2_000:
	.byte		N05   , Fn3 , v092
	.byte		N05   , Cn4
	.byte	W24
	.byte		        Cn3 , v084
	.byte		N05   , An3
	.byte	W24
	.byte		        Fn3 , v092
	.byte		N05   , Cn4
	.byte	W24
	.byte		        Cn3 , v084
	.byte		N05   , An3
	.byte	W24
	.byte	PEND
mus_qz_620_2_001:
	.byte		N05   , Ds3 , v092
	.byte		N05   , As3
	.byte	W24
	.byte		        As2 , v084
	.byte		N05   , Gn3
	.byte	W24
	.byte		        Ds3 , v092
	.byte		N05   , As3
	.byte	W12
	.byte		        As2
	.byte		N05   , Gn3
	.byte	W06
	.byte		        Cn3 , v084
	.byte		N05   , Gs3
	.byte	W06
	.byte		        Ds3 , v092
	.byte		N05   , As3
	.byte	W12
	.byte		        Gn3
	.byte		N05   , Ds4
	.byte	W12
	.byte	PEND
mus_qz_620_2_002:
	.byte		N05   , Fn3 , v092
	.byte		N05   , Cn4
	.byte	W24
	.byte		        Cn3 , v084
	.byte		N05   , An3
	.byte	W24
	.byte		        Fn3 , v092
	.byte		N05   , Cn4
	.byte	W24
	.byte		        An3 , v084
	.byte		N05   , Fn4
	.byte	W24
	.byte	PEND
mus_qz_620_2_003:
	.byte		N05   , Gn3 , v092
	.byte		N05   , Ds4
	.byte	W24
	.byte		        Ds3 , v084
	.byte		N05   , As3
	.byte	W24
	.byte		        As2 , v092
	.byte		N05   , Gn3
	.byte	W12
	.byte	N05
	.byte		N05   , Ds4
	.byte	W06
	.byte		        Gn3
	.byte		N05   , Dn4
	.byte	W06
	.byte		        Ds3
	.byte		N05   , Cn4
	.byte	W12
	.byte		        Ds3
	.byte		N05   , As3
	.byte	W12
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_620_2_000
mus_qz_620_2_004:
	.byte		N05   , Ds3 , v092
	.byte		N05   , As3
	.byte	W24
	.byte		        As2 , v084
	.byte		N05   , Gn3
	.byte	W24
	.byte		        Ds3 , v092
	.byte		N05   , As3
	.byte	W24
	.byte		        Gn3 , v084
	.byte		N05   , Ds4
	.byte	W24
	.byte	PEND
mus_qz_620_2_005:
	.byte		N04   , Fs3 , v084
	.byte		N04   , Dn4
	.byte	W06
	.byte		        Fs3 , v088
	.byte		N04   , Dn4
	.byte	W06
	.byte		        Dn3 , v084
	.byte		N04   , An3
	.byte	W06
	.byte		        Dn3 , v088
	.byte		N04   , An3
	.byte	W06
	.byte		        An2 , v084
	.byte		N04   , Fs3
	.byte	W06
	.byte		        An2 , v088
	.byte		N04   , Fs3
	.byte	W06
	.byte		        Dn3 , v084
	.byte		N04   , An3
	.byte	W06
	.byte		        Dn3 , v088
	.byte		N04   , An3
	.byte	W06
	.byte		        Fs3 , v080
	.byte		N04   , Dn4
	.byte	W06
	.byte		        Fs3 , v084
	.byte		N04   , Dn4
	.byte	W06
	.byte		        Dn3 , v080
	.byte		N04   , An3
	.byte	W06
	.byte		        Dn3 , v084
	.byte		N04   , An3
	.byte	W06
	.byte		        An2 , v080
	.byte		N04   , Fs3
	.byte	W06
	.byte		        An2 , v084
	.byte		N04   , Fs3
	.byte	W06
	.byte		        Dn3 , v080
	.byte		N04   , An3
	.byte	W06
	.byte		        Dn3 , v084
	.byte		N04   , An3
	.byte	W06
	.byte	PEND
mus_qz_620_2_006:
	.byte		N04   , An3 , v088
	.byte		N04   , Fs4
	.byte	W06
	.byte		        An3 , v096
	.byte		N04   , Fs4
	.byte	W06
	.byte		        Fs3 , v088
	.byte		N04   , Dn4
	.byte	W06
	.byte		        Fs3 , v096
	.byte		N04   , Dn4
	.byte	W06
	.byte		        Dn3 , v088
	.byte		N04   , An3
	.byte	W06
	.byte		        Dn3 , v096
	.byte		N04   , An3
	.byte	W06
	.byte		        Fs3 , v088
	.byte		N04   , Dn4
	.byte	W06
	.byte		        Fs3 , v096
	.byte		N04   , Dn4
	.byte	W06
	.byte		        Dn4 , v100
	.byte		N04   , An4
	.byte	W06
	.byte		        Dn4 , v108
	.byte		N04   , An4
	.byte	W06
	.byte		        Dn4 , v100
	.byte		N04   , Bn4
	.byte	W06
	.byte		        Dn4 , v108
	.byte		N04   , Bn4
	.byte	W06
	.byte		        Cs4 , v092
	.byte		N04   , An4
	.byte	W06
	.byte		        Cs4 , v100
	.byte		N04   , An4
	.byte	W06
	.byte		        Bn3 , v088
	.byte		N04   , Gn4
	.byte	W06
	.byte		        Bn3 , v096
	.byte		N04   , Gn4
	.byte	W06
	.byte	PEND
mus_qz_620_2_B1:
	.byte		N28   , Dn3 , v060 , gtp1
	.byte		N28   , An3 , v060 , gtp1
	.byte	W36
	.byte		N32   , Dn3 , v060 , gtp3
	.byte		N32   , Gn3 , v060 , gtp3
	.byte	W36
	.byte		N23   , An2
	.byte		N23   , Fs3
	.byte	W24
	.byte		N28   , An2 , v060 , gtp1
	.byte		N28   , En3 , v060 , gtp1
	.byte	W36
	.byte		N32   , An2 , v060 , gtp3
	.byte		N32   , Fs3 , v060 , gtp3
	.byte	W36
	.byte		N23   , Cs3
	.byte		N23   , Gn3
	.byte	W24
	.byte		N05   , Fs3
	.byte		N05   , Dn4
	.byte	W12
	.byte		        Dn3
	.byte		N05   , An3
	.byte	W06
	.byte		        Fs3
	.byte		N05   , Dn4
	.byte	W06
	.byte		N11   , An3
	.byte		N11   , Fs4
	.byte	W12
	.byte		N05   , An3 , v076
	.byte		N05   , Fs4
	.byte	W06
	.byte		        An3 , v056
	.byte		N05   , Fs4
	.byte	W06
	.byte		        An3 , v076
	.byte		N05   , Fs4
	.byte	W06
	.byte		        An3 , v056
	.byte		N05   , Fs4
	.byte	W06
	.byte		        Bn3 , v076
	.byte		N05   , Gn4
	.byte	W06
	.byte		        Bn3 , v056
	.byte		N05   , Gn4
	.byte	W06
	.byte		        An3 , v072
	.byte		N05   , Fs4
	.byte	W06
	.byte		        An3 , v052
	.byte		N05   , Fs4
	.byte	W06
	.byte		        Gn3 , v064
	.byte		N05   , En4
	.byte	W06
	.byte		        Gn3 , v044
	.byte		N05   , En4
	.byte	W06
	.byte		        Fs3 , v060
	.byte		N05   , Dn4
	.byte	W12
	.byte		        An2
	.byte		N05   , Fs3
	.byte	W06
	.byte	N05
	.byte		N05   , An3
	.byte	W06
	.byte		N23   , Fs3
	.byte		N23   , Dn4
	.byte	W24
	.byte		N05   , Fs3 , v072
	.byte		N05   , Dn4
	.byte	W06
	.byte		        Fs3 , v056
	.byte		N05   , Dn4
	.byte	W06
	.byte		        Gn3 , v072
	.byte		N05   , En4
	.byte	W06
	.byte		        Gn3 , v056
	.byte		N05   , En4
	.byte	W06
	.byte		        Fs3 , v072
	.byte		N05   , Dn4
	.byte	W06
	.byte		        Fs3 , v056
	.byte		N05   , Dn4
	.byte	W06
	.byte		        En3 , v072
	.byte		N05   , Cs4
	.byte	W06
	.byte		        En3 , v056
	.byte		N05   , Cs4
	.byte	W06
	.byte		N28   , Dn3 , v072 , gtp1
	.byte		N28   , An3 , v072 , gtp1
	.byte	W36
	.byte		N32   , Dn3 , v060 , gtp3
	.byte		N32   , Gn3 , v060 , gtp3
	.byte	W36
	.byte		N23   , Fs3 , v068
	.byte		N23   , An3
	.byte	W24
	.byte		N28   , En3 , v072 , gtp1
	.byte		N28   , Bn3 , v072 , gtp1
	.byte	W36
	.byte		N32   , Cs3 , v060 , gtp3
	.byte		N32   , An3 , v060 , gtp3
	.byte	W36
	.byte		N23   , En3 , v068
	.byte		N23   , Cs4
	.byte	W24
	.byte		N05   , Fs3 , v060
	.byte		N05   , Dn4
	.byte	W12
	.byte		        Dn3
	.byte		N05   , An3
	.byte	W06
	.byte		        Fs3
	.byte		N05   , Dn4
	.byte	W06
	.byte		N23   , An3
	.byte		N23   , Fs4
	.byte	W24
	.byte		N05   , An3 , v068
	.byte		N05   , Fs4
	.byte	W06
	.byte		        An3 , v056
	.byte		N05   , Fs4
	.byte	W06
	.byte		        Bn3 , v068
	.byte		N05   , Gn4
	.byte	W06
	.byte		        Bn3 , v056
	.byte		N05   , Gn4
	.byte	W06
	.byte		        An3 , v068
	.byte		N05   , Fs4
	.byte	W06
	.byte		        An3 , v056
	.byte		N05   , Fs4
	.byte	W06
	.byte		        Gn3 , v068
	.byte		N05   , En4
	.byte	W06
	.byte		        Gn3 , v056
	.byte		N05   , En4
	.byte	W06
	.byte		        Fs3 , v060
	.byte		N05   , Dn4
	.byte	W12
	.byte		N11   , Dn3
	.byte		N11   , An3
	.byte	W12
	.byte		N23   , Fs3
	.byte		N23   , Dn4
	.byte	W24
	.byte		N05   , Fs3 , v068
	.byte		N05   , Dn4
	.byte	W06
	.byte		        Fs3 , v056
	.byte		N05   , Dn4
	.byte	W06
	.byte		        Gn3 , v068
	.byte		N05   , En4
	.byte	W06
	.byte		        Gn3 , v056
	.byte		N05   , En4
	.byte	W06
	.byte		        Fs3 , v068
	.byte		N05   , Dn4
	.byte	W06
	.byte		        Fs3 , v056
	.byte		N05   , Dn4
	.byte	W06
	.byte		        Dn3 , v068
	.byte		N05   , An3
	.byte	W06
	.byte		        Dn3 , v056
	.byte		N05   , An3
	.byte	W06
	.byte		        En3 , v060
	.byte		N05   , Cn4
	.byte	W12
	.byte		        Cn3
	.byte		N05   , Gn3
	.byte	W06
	.byte		        En3
	.byte		N05   , Cn4
	.byte	W06
	.byte		N23   , Gn3
	.byte		N23   , En4
	.byte	W24
	.byte		N05   , En3 , v068
	.byte		N05   , Cn4
	.byte	W06
	.byte		        En3 , v056
	.byte		N05   , Cn4
	.byte	W06
	.byte		        Cn3 , v068
	.byte		N05   , Gn3
	.byte	W06
	.byte		        Cn3 , v056
	.byte		N05   , Gn3
	.byte	W06
	.byte		        En3 , v068
	.byte		N05   , Cn4
	.byte	W06
	.byte		        En3 , v056
	.byte		N05   , Cn4
	.byte	W06
	.byte		        Gn3 , v068
	.byte		N05   , En4
	.byte	W06
	.byte		        Gn3 , v056
	.byte		N05   , En4
	.byte	W06
	.byte		        En3 , v072
	.byte		N05   , Cn4
	.byte	W06
	.byte		        En3 , v060
	.byte		N05   , Cn4
	.byte	W06
	.byte		        Cn3 , v072
	.byte		N05   , Gn3
	.byte	W06
	.byte		        Cn3 , v060
	.byte		N05   , Gn3
	.byte	W06
	.byte		        En3 , v072
	.byte		N05   , Cn4
	.byte	W06
	.byte		        En3 , v060
	.byte		N05   , Cn4
	.byte	W06
	.byte		        Gn3 , v072
	.byte		N05   , En4
	.byte	W06
	.byte		        Gn3 , v060
	.byte		N05   , En4
	.byte	W06
	.byte		        En4 , v088
	.byte		N05   , Cn5
	.byte	W06
	.byte		        En4 , v076
	.byte		N05   , Cn5
	.byte	W06
	.byte		        Cn4 , v088
	.byte		N05   , Gn4
	.byte	W06
	.byte		        Cn4 , v076
	.byte		N05   , En4
	.byte	W06
	.byte		        Cs4
	.byte		N05   , Gn4
	.byte	W06
	.byte		        Cs4 , v064
	.byte		N05   , Gn4
	.byte	W06
	.byte		        An3 , v076
	.byte		N05   , En4
	.byte	W06
	.byte		        En3 , v064
	.byte		N05   , Cs4
	.byte	W06
	.byte		N32   , Gn3 , v076 , gtp3
	.byte		N68   , An3 , v052 , gtp3
	.byte		N68   , Dn4 , v064 , gtp3
	.byte	W36
	.byte		N32   , Fs3 , v076 , gtp3
	.byte		N32   , An4 , v072 , gtp3
	.byte	W36
	.byte		N23   , An3 , v060
	.byte		N23   , Dn4 , v064
	.byte		N11   , Gn4 , v072
	.byte	W12
	.byte		        Fs4
	.byte	W12
	.byte		        An2 , v076
	.byte		N92   , An3 , v060 , gtp3
	.byte		N92   , Dn4 , v064 , gtp3
	.byte	W18
	.byte		N05   , Cn3
	.byte	W06
	.byte		N28   , Dn3 , v076 , gtp1
	.byte	W36
	.byte		N11   , Dn3 , v084
	.byte	W12
	.byte		        Ds3 , v080
	.byte	W12
	.byte		        En3 , v088
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_620_2_000
	.byte	PATT
	 ptrvalue	mus_qz_620_2_001
	.byte	PATT
	 ptrvalue	mus_qz_620_2_002
	.byte	PATT
	 ptrvalue	mus_qz_620_2_003
	.byte	PATT
	 ptrvalue	mus_qz_620_2_000
	.byte	PATT
	 ptrvalue	mus_qz_620_2_004
	.byte	PATT
	 ptrvalue	mus_qz_620_2_005
	.byte	PATT
	 ptrvalue	mus_qz_620_2_006
	.byte	GOTO
	 ptrvalue	mus_qz_620_2_B1
	.byte	FINE

/********************** Track  3 **********************/

mus_qz_620_3:
	.byte		KEYSH , mus_qz_620_key+0
	.byte		VOICE , 30
	.byte		VOL   , 89*mus_qz_620_mvl/mxv
	.byte		PAN   , c_v+36
	.byte		PRIO  , 60
	.byte		N05   , Fn1 , v124
	.byte	W72
	.byte	N23
	.byte	W24
	.byte		N05   , Ds1
	.byte	W72
	.byte	N23
	.byte	W24
mus_qz_620_3_000:
	.byte		N05   , Fn1 , v124
	.byte	W24
	.byte		        Cn1
	.byte	W24
	.byte		        Fn1
	.byte	W24
	.byte		        Cn1
	.byte	W24
	.byte	PEND
mus_qz_620_3_001:
	.byte		N05   , Ds1 , v124
	.byte	W24
	.byte		        As0
	.byte	W24
	.byte		        Ds1
	.byte	W24
	.byte		        As0
	.byte	W24
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_620_3_000
mus_qz_620_3_002:
	.byte		N05   , Ds1 , v124
	.byte	W24
	.byte		        As0
	.byte	W24
	.byte		        Ds1
	.byte	W24
	.byte		        Gn1
	.byte	W24
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_620_3_000
mus_qz_620_3_003:
	.byte		N05   , Gn1 , v124
	.byte	W24
	.byte		        Ds1
	.byte	W24
	.byte		        Gn1
	.byte	W24
	.byte		        Ds1
	.byte	W24
	.byte	PEND
mus_qz_620_3_004:
	.byte		N05   , Fs1 , v124
	.byte	W24
	.byte		        Dn1
	.byte	W24
	.byte		        Fs1
	.byte	W24
	.byte		        Dn1
	.byte	W24
	.byte	PEND
mus_qz_620_3_005:
	.byte		N05   , Fs1 , v124
	.byte	W24
	.byte		        An0
	.byte	W24
	.byte		        Bn0
	.byte	W24
	.byte		        Cs1
	.byte	W24
	.byte	PEND
mus_qz_620_3_B1:
	.byte		N05   , Dn1 , v124
	.byte	W24
	.byte		        An0
	.byte	W24
	.byte		        Dn1
	.byte	W24
	.byte		        An0
	.byte	W24
	.byte	PEND
	.byte		        En1
	.byte	W24
	.byte		        An0
	.byte	W24
	.byte		        Cs1
	.byte	W24
	.byte		        An0
	.byte	W24
	.byte	PATT
	 ptrvalue	mus_qz_620_3_B1
	.byte		N05   , Dn1 , v124
	.byte	W24
	.byte		        An0
	.byte	W24
	.byte		        Bn0
	.byte	W24
	.byte		        Cs1
	.byte	W24
	.byte	PATT
	 ptrvalue	mus_qz_620_3_B1
	.byte		N05   , En1 , v124
	.byte	W24
	.byte		        An0
	.byte	W24
	.byte		        En1
	.byte	W24
	.byte		        An0
	.byte	W24
	.byte	PATT
	 ptrvalue	mus_qz_620_3_B1
	.byte	PATT
	 ptrvalue	mus_qz_620_3_B1
	.byte		N05   , Cn1 , v124
	.byte	W24
	.byte		        Gn0
	.byte	W24
	.byte		        Cn1
	.byte	W24
	.byte		        Gn0
	.byte	W24
	.byte		        Cn1
	.byte	W24
	.byte		        Gn0
	.byte	W24
	.byte		        Cn1
	.byte	W24
	.byte		        Cs1
	.byte	W24
	.byte	W96
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_620_3_000
	.byte	PATT
	 ptrvalue	mus_qz_620_3_001
	.byte	PATT
	 ptrvalue	mus_qz_620_3_000
	.byte	PATT
	 ptrvalue	mus_qz_620_3_002
	.byte	PATT
	 ptrvalue	mus_qz_620_3_000
	.byte	PATT
	 ptrvalue	mus_qz_620_3_003
	.byte	PATT
	 ptrvalue	mus_qz_620_3_004
	.byte	PATT
	 ptrvalue	mus_qz_620_3_005
	.byte	GOTO
	 ptrvalue	mus_qz_620_3_B1
	.byte	FINE

/********************** Track  4 **********************/

mus_qz_620_4:
	.byte		KEYSH , mus_qz_620_key+0
	.byte		VOICE , 39
	.byte		VOL   , 61*mus_qz_620_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 40
	.byte		N05   , Dn1 , v092
	.byte		N32   , Cn2 , v092 , gtp3
	.byte	W12
	.byte		N05   , Dn1 , v064
	.byte	W06
	.byte		        Dn1 , v080
	.byte	W06
	.byte		N02   , Dn1 , v076
	.byte	W03
	.byte		        Dn1 , v052
	.byte	W03
	.byte		        Dn1 , v068
	.byte	W03
	.byte		        Dn1 , v076
	.byte	W03
	.byte		N05   , Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v068
	.byte	W06
	.byte		        Dn1 , v100
	.byte	W12
	.byte		        Dn1 , v068
	.byte	W06
	.byte		        Dn1 , v084
	.byte	W06
	.byte		        Dn1 , v100
	.byte		N23   , Cn2
	.byte	W06
	.byte		N05   , Dn1 , v080
	.byte	W06
	.byte		        Dn1 , v100
	.byte	W12
	.byte		        Dn1 , v092
	.byte		N32   , Cn2 , v092 , gtp3
	.byte	W12
	.byte		N05   , Dn1 , v064
	.byte	W06
	.byte		        Dn1 , v080
	.byte	W06
	.byte		N02   , Dn1 , v076
	.byte	W03
	.byte		        Dn1 , v052
	.byte	W03
	.byte		        Dn1 , v068
	.byte	W03
	.byte		        Dn1 , v076
	.byte	W03
	.byte		N05   , Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v068
	.byte	W06
	.byte		        Dn1 , v100
	.byte	W12
	.byte		        Dn1 , v068
	.byte	W06
	.byte		        Dn1 , v084
	.byte	W06
	.byte		        Dn1 , v100
	.byte		N23   , Cn2 , v092
	.byte	W06
	.byte		N05   , Dn1 , v080
	.byte	W06
	.byte		        Dn1 , v100
	.byte	W12
mus_qz_620_4_000:
	.byte		N05   , Dn1 , v092
	.byte		N32   , Cn2 , v092 , gtp3
	.byte	W12
	.byte		N05   , Dn1 , v064
	.byte	W06
	.byte		        Dn1 , v080
	.byte	W06
	.byte		N02   , Dn1 , v076
	.byte	W03
	.byte		        Dn1 , v052
	.byte	W03
	.byte		        Dn1 , v068
	.byte	W03
	.byte		        Dn1 , v076
	.byte	W03
	.byte		N05   , Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v068
	.byte	W06
	.byte		        Dn1 , v100
	.byte	W12
	.byte		        Dn1 , v068
	.byte	W06
	.byte		        Dn1 , v084
	.byte	W06
	.byte		        Dn1 , v100
	.byte	W06
	.byte		        Dn1 , v080
	.byte	W06
	.byte		        Dn1 , v100
	.byte	W12
	.byte	PEND
mus_qz_620_4_001:
	.byte		N05   , Dn1 , v092
	.byte	W12
	.byte		        Dn1 , v064
	.byte	W06
	.byte		        Dn1 , v080
	.byte	W06
	.byte		N02   , Dn1 , v076
	.byte	W03
	.byte		        Dn1 , v052
	.byte	W03
	.byte		        Dn1 , v068
	.byte	W03
	.byte		        Dn1 , v076
	.byte	W03
	.byte		N05   , Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v068
	.byte	W06
	.byte		        Dn1 , v100
	.byte	W12
	.byte		        Dn1 , v068
	.byte	W06
	.byte		        Dn1 , v084
	.byte	W06
	.byte		        Dn1 , v100
	.byte	W06
	.byte		        Dn1 , v080
	.byte	W06
	.byte		        Dn1 , v100
	.byte	W12
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_620_4_001
	.byte	PATT
	 ptrvalue	mus_qz_620_4_001
	.byte	PATT
	 ptrvalue	mus_qz_620_4_000
	.byte	PATT
	 ptrvalue	mus_qz_620_4_001
	.byte	PATT
	 ptrvalue	mus_qz_620_4_001
	.byte	PATT
	 ptrvalue	mus_qz_620_4_001
mus_qz_620_4_B1:
	.byte	PATT
	 ptrvalue	mus_qz_620_4_000
	.byte	PATT
	 ptrvalue	mus_qz_620_4_001
	.byte	PATT
	 ptrvalue	mus_qz_620_4_000
	.byte	PATT
	 ptrvalue	mus_qz_620_4_001
	.byte	PATT
	 ptrvalue	mus_qz_620_4_000
	.byte	PATT
	 ptrvalue	mus_qz_620_4_001
	.byte	PATT
	 ptrvalue	mus_qz_620_4_000
	.byte	PATT
	 ptrvalue	mus_qz_620_4_001
	.byte	PATT
	 ptrvalue	mus_qz_620_4_001
	.byte	PATT
	 ptrvalue	mus_qz_620_4_001
	.byte		N32   , Cn2 , v088 , gtp3
	.byte	W96
	.byte	W72
	.byte	N20
	.byte	W24
	.byte	PATT
	 ptrvalue	mus_qz_620_4_000
	.byte	PATT
	 ptrvalue	mus_qz_620_4_001
	.byte	PATT
	 ptrvalue	mus_qz_620_4_001
	.byte	PATT
	 ptrvalue	mus_qz_620_4_001
	.byte	PATT
	 ptrvalue	mus_qz_620_4_000
	.byte	PATT
	 ptrvalue	mus_qz_620_4_001
	.byte	PATT
	 ptrvalue	mus_qz_620_4_001
	.byte	PATT
	 ptrvalue	mus_qz_620_4_001
	.byte	GOTO
	 ptrvalue	mus_qz_620_4_B1
	.byte	FINE

/********************** Track  5 **********************/

mus_qz_620_5:
	.byte		KEYSH , mus_qz_620_key+0
	.byte		VOICE , 35
	.byte		VOL   , 72*mus_qz_620_mvl/mxv
	.byte		PAN   , c_v+6
	.byte		PRIO  , 55
	.byte	W24
	.byte		N01   , Cn3 , v036
	.byte	W01
	.byte		        Dn3
	.byte	W02
	.byte		        En3 , v040
	.byte	W01
	.byte		        Fn3
	.byte	W02
	.byte		        Gn3
	.byte	W01
	.byte		        An3 , v044
	.byte	W02
	.byte		        Bn3 , v048
	.byte	W01
	.byte		        Cn4
	.byte	W02
	.byte		        Dn4 , v056
	.byte	W01
	.byte		        En4 , v064
	.byte	W02
	.byte		        Fn4 , v068
	.byte	W01
	.byte		        Gn4 , v076
	.byte	W02
	.byte		        An4 , v088
	.byte	W01
	.byte		        Bn4 , v096
	.byte	W02
	.byte		        Cn5 , v104
	.byte	W03
	.byte		N02   , Cn5 , v096
	.byte	W03
	.byte		        Dn5 , v080
	.byte	W03
	.byte		        Cn5 , v096
	.byte	W03
	.byte		        Dn5 , v076
	.byte	W03
	.byte		        Cn5 , v088
	.byte	W03
	.byte		        Dn5 , v064
	.byte	W03
	.byte		        Cn5 , v076
	.byte	W03
	.byte		        Dn5 , v056
	.byte	W03
	.byte		        Cn5 , v072
	.byte	W03
	.byte		        Dn5 , v052
	.byte	W03
	.byte		        Cn5 , v064
	.byte	W18
	.byte	W24
	.byte		N01   , As2 , v036
	.byte	W01
	.byte		        Cn3
	.byte	W02
	.byte		        Dn3
	.byte	W01
	.byte		        Ds3 , v040
	.byte	W02
	.byte		        Fn3
	.byte	W01
	.byte		        Gn3 , v044
	.byte	W02
	.byte		        An3 , v048
	.byte	W01
	.byte		        As3 , v052
	.byte	W02
	.byte		        Cn4
	.byte	W01
	.byte		        Dn4 , v060
	.byte	W02
	.byte		        Ds4 , v068
	.byte	W01
	.byte		        Fn4 , v080
	.byte	W02
	.byte		        Gn4 , v092
	.byte	W01
	.byte		        An4 , v104
	.byte	W02
	.byte		        As4 , v112
	.byte	W03
	.byte		N02   , As4 , v096
	.byte	W03
	.byte		        Cn5 , v072
	.byte	W03
	.byte		        As4 , v088
	.byte	W03
	.byte		        Cn5 , v072
	.byte	W03
	.byte		        As4 , v084
	.byte	W03
	.byte		        Cn5 , v064
	.byte	W03
	.byte		        As4 , v076
	.byte	W03
	.byte		        Cn5 , v064
	.byte	W03
	.byte		        As4 , v072
	.byte	W03
	.byte		        Cn5 , v064
	.byte	W03
	.byte		        As4 , v052
	.byte	W18
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
mus_qz_620_5_B1:
	.byte	W96
	.byte	W72
	.byte		N01   , An2 , v040
	.byte	W01
	.byte		        Bn2 , v044
	.byte	W02
	.byte		        Cs3
	.byte	W01
	.byte		        Dn3
	.byte	W02
	.byte		        En3
	.byte	W01
	.byte		        Fs3 , v048
	.byte	W02
	.byte		        Gs3 , v052
	.byte	W01
	.byte		        An3
	.byte	W02
	.byte		        Bn3 , v060
	.byte	W01
	.byte		        Cs4 , v068
	.byte	W02
	.byte		        Dn4 , v072
	.byte	W01
	.byte		        En4 , v084
	.byte	W02
	.byte		        Fs4 , v096
	.byte	W01
	.byte		        Gs4 , v104
	.byte	W02
	.byte		        An4 , v112
	.byte	W03
	.byte		N05   , An4 , v104
	.byte	W12
	.byte		        Fs4 , v096
	.byte	W06
	.byte		        An4 , v100
	.byte	W06
	.byte		N02   , Dn5 , v096
	.byte	W03
	.byte		        En5 , v080
	.byte	W03
	.byte		        Dn5 , v092
	.byte	W03
	.byte		        En5 , v076
	.byte	W03
	.byte		        Dn5 , v088
	.byte	W03
	.byte		        En5 , v072
	.byte	W03
	.byte		        Dn5 , v088
	.byte	W03
	.byte		        En5 , v072
	.byte	W03
	.byte		        Dn5 , v084
	.byte	W03
	.byte		        En5 , v068
	.byte	W03
	.byte		        Dn5 , v080
	.byte	W03
	.byte		        En5 , v068
	.byte	W03
	.byte		        Dn5 , v080
	.byte	W03
	.byte		        En5 , v064
	.byte	W03
	.byte		        Dn5 , v068
	.byte	W03
	.byte		        En5 , v052
	.byte	W24
	.byte	W03
	.byte		N05   , Fs4 , v104
	.byte	W12
	.byte		        Dn4 , v096
	.byte	W06
	.byte		        Fs4 , v100
	.byte	W06
	.byte		N02   , An4
	.byte	W03
	.byte		        Bn4 , v084
	.byte	W03
	.byte		        An4 , v096
	.byte	W03
	.byte		        Bn4 , v080
	.byte	W03
	.byte		        An4 , v092
	.byte	W03
	.byte		        Bn4 , v076
	.byte	W03
	.byte		        An4 , v088
	.byte	W03
	.byte		        Bn4 , v072
	.byte	W03
	.byte		        An4 , v088
	.byte	W03
	.byte		        Bn4 , v072
	.byte	W03
	.byte		        An4 , v084
	.byte	W03
	.byte		        Bn4 , v068
	.byte	W03
	.byte		        An4 , v080
	.byte	W03
	.byte		        Bn4 , v064
	.byte	W03
	.byte		        An4 , v068
	.byte	W03
	.byte		        Bn4 , v052
	.byte	W24
	.byte	W03
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		N32   , Dn4 , v100 , gtp3
	.byte	W36
	.byte		        An4 , v096 , gtp3
	.byte	W36
	.byte		N11   , Gn4
	.byte	W12
	.byte		        Fs4 , v088
	.byte	W12
	.byte		N68   , Dn4 , v100 , gtp3
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
	 ptrvalue	mus_qz_620_5_B1
	.byte	FINE

/********************** Track  6 **********************/

mus_qz_620_6:
	.byte		KEYSH , mus_qz_620_key+0
	.byte		VOICE , 34
	.byte		VOL   , 61*mus_qz_620_mvl/mxv
	.byte		PAN   , c_v-8
	.byte		PRIO  , 58
	.byte		N11   , Fn3 , v100
	.byte	W24
	.byte	N03
	.byte	W04
	.byte		        Cn3 , v084
	.byte	W04
	.byte		        Fn3 , v100
	.byte	W04
	.byte		        An3 , v108
	.byte	W04
	.byte		        Fn3 , v084
	.byte	W04
	.byte		        An3 , v104
	.byte	W04
	.byte		        Cn4 , v120
	.byte	W24
	.byte		N19   , Cn4 , v100
	.byte	W24
	.byte		N11   , Gn3
	.byte	W24
	.byte		N03   , Gn3 , v080
	.byte	W04
	.byte		        As2 , v072
	.byte	W04
	.byte		        Ds3 , v080
	.byte	W04
	.byte		        Gn3 , v088
	.byte	W04
	.byte		        Ds3 , v076
	.byte	W04
	.byte		        Gn3 , v084
	.byte	W04
	.byte		        As3 , v104
	.byte	W24
	.byte		N19   , Gn3 , v100
	.byte	W24
mus_qz_620_6_000:
	.byte		N44   , An3 , v100 , gtp3
	.byte	W56
	.byte	W03
	.byte		N12   , Cn3
	.byte	W13
	.byte		N03   , Fn3 , v088
	.byte	W11
	.byte		N04   , Cn4 , v092
	.byte	W13
	.byte	PEND
mus_qz_620_6_001:
	.byte		N14   , As3 , v100
	.byte	W18
	.byte		N28   , Gn3
	.byte	W40
	.byte	W01
	.byte		N05   , Ds3 , v080
	.byte	W06
	.byte		        Fn3 , v076
	.byte	W04
	.byte		N04   , Gn3 , v088
	.byte	W14
	.byte		N03   , Ds3 , v100
	.byte	W13
	.byte	PEND
mus_qz_620_6_002:
	.byte		N48   , An3 , v100 , gtp3
	.byte	W60
	.byte		N11   , Fn3
	.byte	W12
	.byte		        An3
	.byte	W12
	.byte		N05   , Cn4
	.byte	W12
	.byte	PEND
mus_qz_620_6_003:
	.byte		N03   , As3 , v100
	.byte	W12
	.byte	N07
	.byte	W08
	.byte		N03   , An3
	.byte	W04
	.byte		N30   , Gn3 , v100 , gtp1
	.byte	W36
	.byte		N03   , Ds3 , v088
	.byte	W06
	.byte		        Fn3 , v080
	.byte	W06
	.byte		        Gn3 , v088
	.byte	W12
	.byte		        Ds3
	.byte	W12
	.byte	PEND
mus_qz_620_6_004:
	.byte		N44   , Fn3 , v100 , gtp3
	.byte	W60
	.byte		N11   , Cn3
	.byte	W12
	.byte		        Fn3
	.byte	W12
	.byte		N05   , An3
	.byte	W12
	.byte	PEND
mus_qz_620_6_005:
	.byte		N03   , Gn3 , v100
	.byte	W12
	.byte	N03
	.byte	W04
	.byte		N07   , Fn3
	.byte	W08
	.byte		N28   , Ds3 , v100 , gtp1
	.byte	W36
	.byte		N05   , Ds3 , v088
	.byte	W06
	.byte		        Fn3 , v080
	.byte	W06
	.byte		N03   , Gn3 , v088
	.byte	W12
	.byte		        Ds3
	.byte	W12
	.byte	PEND
mus_qz_620_6_006:
	.byte		N30   , Fs3 , v100 , gtp1
	.byte	W36
	.byte		N07   , Fs3 , v088
	.byte	W08
	.byte		N03   , Gn3 , v084
	.byte	W04
	.byte		        An3 , v092
	.byte	W12
	.byte		        Gn3 , v088
	.byte	W12
	.byte		        Fs3
	.byte	W12
	.byte		        Gn3
	.byte	W12
	.byte	PEND
mus_qz_620_6_007:
	.byte		N03   , An3 , v088
	.byte	W12
	.byte		        Dn3
	.byte	W12
	.byte		        Fs3
	.byte	W12
	.byte		        Dn3
	.byte	W12
	.byte		        An3
	.byte	W12
	.byte		        An2 , v100
	.byte	W12
	.byte		N02   , An2 , v052
	.byte	W03
	.byte		        Bn2 , v056
	.byte	W03
	.byte		        Cs3 , v064
	.byte	W03
	.byte		        Dn3 , v072
	.byte	W03
	.byte		        En3 , v080
	.byte	W03
	.byte		        Fs3 , v092
	.byte	W03
	.byte		N04   , Gn3 , v112
	.byte	W06
	.byte	PEND
mus_qz_620_6_B1:
	.byte		N28   , Fs3 , v084 , gtp1
	.byte		N28   , An3 , v112 , gtp1
	.byte	W36
	.byte		N32   , An2 , v068 , gtp1
	.byte		N32   , Gn3 , v092 , gtp1
	.byte	W36
	.byte		N21   , Dn3 , v084
	.byte		N21   , Fs3 , v112
	.byte	W24
	.byte		N28   , Cs3 , v084 , gtp1
	.byte		N28   , En3 , v112 , gtp1
	.byte	W36
	.byte		N32   , Dn3 , v068 , gtp1
	.byte		N32   , Fs3 , v092 , gtp1
	.byte	W36
	.byte		N21   , An2 , v084
	.byte		N21   , Gn3 , v112
	.byte	W24
	.byte	W96
	.byte	W96
	.byte		N28   , Fs3 , v084 , gtp1
	.byte		N28   , An3 , v112 , gtp1
	.byte	W36
	.byte		N32   , An2 , v076 , gtp1
	.byte		N32   , Gn3 , v104 , gtp1
	.byte	W36
	.byte		N21   , Dn3 , v084
	.byte		N21   , Fs3 , v112
	.byte	W24
	.byte		N28   , Cs3 , v084 , gtp1
	.byte		N28   , Bn3 , v112 , gtp1
	.byte	W36
	.byte		N32   , En3 , v076 , gtp1
	.byte		N32   , An3 , v104 , gtp1
	.byte	W36
	.byte		N21   , An2 , v084
	.byte		N23   , Gn3 , v112
	.byte	W24
	.byte		N03   , Fs3 , v092
	.byte	W12
	.byte		        Dn3
	.byte	W06
	.byte		        Fs3
	.byte	W06
	.byte		N64   , An3 , v088 , gtp1
	.byte	W72
	.byte		N03   , Dn3 , v092
	.byte	W12
	.byte		        An2
	.byte	W06
	.byte		        Dn3
	.byte	W06
	.byte		N64   , Fs3 , v088 , gtp1
	.byte	W72
	.byte		N05   , En3
	.byte	W12
	.byte		        Cn3
	.byte	W06
	.byte		        En3
	.byte	W06
	.byte		N56   , Gn3 , v088 , gtp3
	.byte	W72
	.byte		N05   , Cn3
	.byte	W12
	.byte		        Gn2
	.byte	W06
	.byte		        Cn3
	.byte	W06
	.byte		N36   , En3 , v088 , gtp2
	.byte	W42
	.byte	N02
	.byte	W03
	.byte		        Fs3 , v080
	.byte	W03
	.byte		N23   , Gn3 , v088
	.byte	W24
	.byte		N32   , An2 , v060 , gtp2
	.byte		N32   , An3 , v072 , gtp3
	.byte	W36
	.byte		N22   , Fs3 , v068
	.byte		N32   , Dn4 , v072 , gtp3
	.byte	W24
	.byte		N11   , Dn2 , v068
	.byte	W12
	.byte		        An2
	.byte		N11   , Cn4 , v072
	.byte	W12
	.byte		        Dn3 , v068
	.byte		N11   , Bn3 , v072
	.byte	W12
	.byte		N16   , Cn3
	.byte		N23   , An3
	.byte	W18
	.byte		N04   , En3
	.byte	W06
	.byte		N32   , Fs3 , v072 , gtp2
	.byte	W12
	.byte		N23   , Dn3
	.byte	W24
	.byte	N11
	.byte	W12
	.byte		        Ds3
	.byte	W12
	.byte		        En3
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_620_6_000
	.byte	PATT
	 ptrvalue	mus_qz_620_6_001
	.byte	PATT
	 ptrvalue	mus_qz_620_6_002
	.byte	PATT
	 ptrvalue	mus_qz_620_6_003
	.byte	PATT
	 ptrvalue	mus_qz_620_6_004
	.byte	PATT
	 ptrvalue	mus_qz_620_6_005
	.byte	PATT
	 ptrvalue	mus_qz_620_6_006
	.byte	PATT
	 ptrvalue	mus_qz_620_6_007
	.byte	GOTO
	 ptrvalue	mus_qz_620_6_B1
	.byte	FINE

/********************** Track  7 **********************/

mus_qz_620_7:
	.byte		KEYSH , mus_qz_620_key+0
	.byte		VOICE , 7
	.byte		VOL   , 32*mus_qz_620_mvl/mxv
	.byte		PAN   , c_v-54
	.byte		PRIO  , 54
	.byte		N03   , Fn3 , v116
	.byte	W12
	.byte		        Cn3 , v096
	.byte	W06
	.byte		        Fn3
	.byte	W06
	.byte		N01   , An3 , v104
	.byte	W03
	.byte		        An3 , v084
	.byte	W03
	.byte		        An3 , v092
	.byte	W03
	.byte		N02   , An3 , v076
	.byte	W03
	.byte		        An3 , v088
	.byte	W03
	.byte		        An3 , v044
	.byte	W03
	.byte		N01   , An3 , v060
	.byte	W02
	.byte		N02   , An3 , v044
	.byte	W04
	.byte		        An3 , v056
	.byte	W04
	.byte		        An3 , v032
	.byte	W20
	.byte		N23   , Fn3 , v096
	.byte	W24
	.byte		N03   , Cn4 , v108
	.byte	W12
	.byte		        An3 , v096
	.byte	W06
	.byte		        Cn4
	.byte	W06
	.byte		N01   , Fn4 , v104
	.byte	W03
	.byte		        Fn4 , v064
	.byte	W03
	.byte		        Fn4 , v084
	.byte	W02
	.byte		N02   , Fn4 , v060
	.byte	W04
	.byte		        Fn4 , v068
	.byte	W03
	.byte		        Fn4 , v052
	.byte	W03
	.byte		        Fn4 , v064
	.byte	W03
	.byte		        Fn4 , v044
	.byte	W03
	.byte		        Fn4 , v056
	.byte	W04
	.byte		        Fn4 , v044
	.byte	W04
	.byte		        Fn4 , v016
	.byte	W16
	.byte		N03   , As3 , v096
	.byte	W24
	.byte		N01   , Cn4 , v104
	.byte	W03
	.byte		        Cn4 , v064
	.byte	W03
	.byte		        Cn4 , v084
	.byte	W02
	.byte		N02   , Cn4 , v060
	.byte	W04
	.byte		        Cn4 , v068
	.byte	W03
	.byte		N01   , Cn4 , v052
	.byte	W03
	.byte		        Cn4 , v064
	.byte	W03
	.byte		N02   , Cn4 , v044
	.byte	W03
	.byte		        Cn4 , v056
	.byte	W04
	.byte		        Cn4 , v044
	.byte	W04
	.byte		        Cn4 , v032
	.byte	W04
	.byte		        Cn4 , v024
	.byte	W04
	.byte		        Cn4 , v032
	.byte	W04
	.byte		        Cn4 , v020
	.byte	W52
mus_qz_620_7_000:
	.byte	W60
	.byte		N05   , Ds4 , v092
	.byte	W06
	.byte		        As3
	.byte	W06
	.byte		        Ds4
	.byte	W12
	.byte		        Gn4
	.byte	W12
	.byte	PEND
	.byte		        Fn4
	.byte	W96
mus_qz_620_7_001:
	.byte	W60
	.byte		N05   , Ds4 , v092
	.byte	W06
	.byte		        Gn3
	.byte	W06
	.byte		        Ds4
	.byte	W12
	.byte		        As4
	.byte	W12
	.byte	PEND
	.byte		        An4
	.byte	W96
mus_qz_620_7_002:
	.byte	W60
	.byte		N05   , Gn4 , v092
	.byte	W06
	.byte		        Ds4
	.byte	W06
	.byte		        As3
	.byte	W12
	.byte		        Ds4
	.byte	W12
	.byte	PEND
mus_qz_620_7_003:
	.byte		N05   , An2 , v060
	.byte	W06
	.byte		        An3
	.byte	W06
	.byte		        Dn3 , v072
	.byte	W06
	.byte		        Dn4 , v064
	.byte	W06
	.byte		        Fs2 , v060
	.byte	W06
	.byte		        Fs3 , v076
	.byte	W06
	.byte		        An2 , v068
	.byte	W06
	.byte		        An3 , v060
	.byte	W06
	.byte		        Dn3 , v072
	.byte	W06
	.byte		        Dn4 , v064
	.byte	W06
	.byte		        Fs2 , v060
	.byte	W06
	.byte		        Fs3 , v076
	.byte	W06
	.byte		        An2 , v068
	.byte	W06
	.byte		        An3 , v060
	.byte	W06
	.byte		        Dn3 , v072
	.byte	W06
	.byte		        Dn4 , v064
	.byte	W06
	.byte	PEND
mus_qz_620_7_004:
	.byte		N05   , An2 , v068
	.byte	W06
	.byte		        An3 , v080
	.byte	W06
	.byte		        Dn3 , v076
	.byte	W06
	.byte		        Dn4 , v068
	.byte	W06
	.byte		        Fs3 , v080
	.byte	W06
	.byte		        Fs4 , v072
	.byte	W06
	.byte		        An2 , v068
	.byte	W06
	.byte		        An3 , v080
	.byte	W06
	.byte		        Dn3 , v076
	.byte	W06
	.byte		        Dn4 , v068
	.byte	W06
	.byte		        Fs3 , v084
	.byte	W06
	.byte		        Fs4 , v072
	.byte	W06
	.byte		N03   , Cs3 , v056
	.byte		N03   , An3
	.byte	W03
	.byte		        Dn3 , v064
	.byte		N03   , Bn3
	.byte	W03
	.byte		        En3 , v072
	.byte		N03   , Cs4
	.byte	W04
	.byte		        Fs3 , v080
	.byte		N03   , Dn4
	.byte	W03
	.byte		        Gn3 , v088
	.byte		N03   , En4
	.byte	W04
	.byte		        An3 , v096
	.byte		N03   , Fs4
	.byte	W03
	.byte		        Bn3 , v108
	.byte		N03   , Gn4
	.byte	W04
	.byte	PEND
mus_qz_620_7_B1:
	.byte	W96
	.byte	W72
	.byte		N01   , An2 , v032
	.byte	W01
	.byte		        Bn2 , v036
	.byte	W02
	.byte		        Cs3
	.byte	W01
	.byte		        Dn3
	.byte	W02
	.byte		        En3
	.byte	W01
	.byte		        Fs3 , v040
	.byte	W02
	.byte		        Gs3 , v044
	.byte	W01
	.byte		        An3
	.byte	W02
	.byte		        Bn3 , v052
	.byte	W01
	.byte		        Cs4 , v060
	.byte	W02
	.byte		        Dn4 , v064
	.byte	W01
	.byte		        En4 , v072
	.byte	W02
	.byte		        Fs4 , v084
	.byte	W01
	.byte		        Gs4 , v092
	.byte	W02
	.byte		        An4 , v100
	.byte	W03
	.byte		N05   , An4 , v080
	.byte	W06
	.byte		        Dn4 , v060
	.byte	W06
	.byte		        Fs4 , v072
	.byte	W06
	.byte		        An4 , v080
	.byte	W06
	.byte		        Fs4 , v072
	.byte	W06
	.byte		        Dn4 , v060
	.byte	W06
	.byte		        Fs4 , v072
	.byte	W06
	.byte		        An3 , v068
	.byte	W06
	.byte		        An4 , v080
	.byte	W06
	.byte		        Dn4 , v060
	.byte	W06
	.byte		        Bn4 , v080
	.byte	W06
	.byte		        An3 , v068
	.byte	W06
	.byte		        An4 , v080
	.byte	W06
	.byte		        Dn4 , v060
	.byte	W06
	.byte		        Gn4 , v080
	.byte	W06
	.byte		        An3 , v068
	.byte	W06
	.byte		        Fs4 , v080
	.byte	W06
	.byte		        Dn4 , v060
	.byte	W06
	.byte		        An3 , v068
	.byte	W06
	.byte		        Dn4 , v060
	.byte	W06
	.byte		        An4 , v080
	.byte	W06
	.byte		        An3 , v060
	.byte	W06
	.byte		        En4 , v080
	.byte	W06
	.byte		        An3 , v068
	.byte	W06
	.byte		        Fs4 , v080
	.byte	W06
	.byte		        Dn4 , v060
	.byte	W06
	.byte		        Gn4 , v080
	.byte	W06
	.byte		        An3 , v068
	.byte	W06
	.byte		        Fs4 , v080
	.byte	W06
	.byte		        Dn4 , v060
	.byte	W06
	.byte		        En4 , v080
	.byte	W06
	.byte		        An3 , v068
	.byte	W06
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_620_7_000
	.byte		N05   , Fn4 , v092
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_620_7_001
	.byte		N05   , An4 , v092
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_620_7_002
	.byte	PATT
	 ptrvalue	mus_qz_620_7_003
	.byte	PATT
	 ptrvalue	mus_qz_620_7_004
	.byte	GOTO
	 ptrvalue	mus_qz_620_7_B1
	.byte	FINE

/********************** Track  8 **********************/

mus_qz_620_8:
	.byte		KEYSH , mus_qz_620_key+0
	.byte		VOICE , 29
	.byte		VOL   , 47*mus_qz_620_mvl/mxv
	.byte		PAN   , c_v+56
	.byte		PRIO  , 38
	.byte	W12
	.byte		N03   , Fn3 , v036
	.byte	W12
	.byte		        Cn3 , v024
	.byte	W06
	.byte		        Fn3
	.byte	W06
	.byte		N32   , An3 , v024 , gtp3
	.byte	W48
	.byte		N23   , Fn3
	.byte	W12
	.byte	W12
	.byte		N03   , Cn4 , v032
	.byte	W12
	.byte		        An3 , v024
	.byte	W06
	.byte		        Cn4
	.byte	W06
	.byte		N32   , Fn4 , v024 , gtp3
	.byte	W48
	.byte		N23   , As3
	.byte	W12
mus_qz_620_8_000:
	.byte	W12
	.byte		N44   , Cn4 , v024 , gtp3
	.byte	W60
	.byte		N11   , An3
	.byte	W12
	.byte		N05   , Cn4
	.byte	W12
	.byte	PEND
mus_qz_620_8_001:
	.byte		N05   , An4 , v024
	.byte	W12
	.byte		N17   , Gn4
	.byte	W18
	.byte		N64   , Ds4 , v024 , gtp1
	.byte	W66
	.byte	PEND
mus_qz_620_8_002:
	.byte	W12
	.byte		N52   , Cn4 , v024 , gtp1
	.byte	W60
	.byte		N11   , An3
	.byte	W12
	.byte		N05   , Cn4
	.byte	W12
	.byte	PEND
mus_qz_620_8_003:
	.byte		N05   , Fn4 , v024
	.byte	W12
	.byte		        Ds4
	.byte	W12
	.byte	N05
	.byte	W06
	.byte		        Fn4
	.byte	W06
	.byte		N56   , Gn4 , v024 , gtp3
	.byte	W60
	.byte	PEND
mus_qz_620_8_004:
	.byte	W12
	.byte		N52   , An4 , v024 , gtp1
	.byte	W60
	.byte		N11   , Fn4
	.byte	W12
	.byte		N05   , An4
	.byte	W12
	.byte	PEND
mus_qz_620_8_005:
	.byte		N05   , Cn5 , v024
	.byte	W12
	.byte		        As4
	.byte	W12
	.byte		        As4 , v020
	.byte	W06
	.byte		        An4
	.byte	W06
	.byte		N44   , Gn4 , v024 , gtp3
	.byte	W48
	.byte		N23   , As4
	.byte	W12
	.byte	PEND
mus_qz_620_8_006:
	.byte	W12
	.byte		TIE   , An4 , v024
	.byte	W21
	.byte		VOL   , 42*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        42*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        40*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        38*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        34*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        34*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        32*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        32*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        31*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        30*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        27*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        26*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        25*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        24*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        23*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        23*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        23*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        23*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        23*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        24*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        24*mus_qz_620_mvl/mxv
	.byte	W03
	.byte	PEND
mus_qz_620_8_007:
	.byte		VOL   , 25*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        28*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        29*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        31*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        32*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        36*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        36*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        38*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        42*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        46*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        52*mus_qz_620_mvl/mxv
	.byte	W03
	.byte		        52*mus_qz_620_mvl/mxv
	.byte	W21
	.byte		        38*mus_qz_620_mvl/mxv
	.byte	W06
	.byte		        31*mus_qz_620_mvl/mxv
	.byte	W06
	.byte		        22*mus_qz_620_mvl/mxv
	.byte	W06
	.byte		        13*mus_qz_620_mvl/mxv
	.byte	W06
	.byte		        7*mus_qz_620_mvl/mxv
	.byte	W05
	.byte	PEND
	.byte		EOT   , An4
	.byte	W01
	.byte		VOL   , 4*mus_qz_620_mvl/mxv
	.byte	W06
	.byte		        2*mus_qz_620_mvl/mxv
	.byte	W06
mus_qz_620_8_B1:
	.byte		VOL   , 2*mus_qz_620_mvl/mxv
	.byte	W12
	.byte		        46*mus_qz_620_mvl/mxv
	.byte	W84
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W12
	.byte		N03   , An3 , v028
	.byte	W12
	.byte		        Fs3
	.byte	W06
	.byte		        An3
	.byte	W06
	.byte		N56   , Dn4 , v028 , gtp3
	.byte	W60
	.byte	W12
	.byte		N03   , Fs3
	.byte	W12
	.byte		        Dn3
	.byte	W06
	.byte		        Fs3
	.byte	W06
	.byte		N64   , An3 , v028 , gtp1
	.byte	W60
	.byte	W12
	.byte		N03   , Gn3
	.byte	W12
	.byte		        En3
	.byte	W06
	.byte		        Gn3
	.byte	W06
	.byte		N56   , Cn4 , v028 , gtp3
	.byte	W60
	.byte	W12
	.byte		N03   , Gn3
	.byte	W12
	.byte		        En3
	.byte	W06
	.byte		        Gn3
	.byte	W06
	.byte		N40   , Cn4 , v028 , gtp1
	.byte	W48
	.byte		N23   , Cs4
	.byte	W12
	.byte	W96
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_620_8_000
	.byte	PATT
	 ptrvalue	mus_qz_620_8_001
	.byte	PATT
	 ptrvalue	mus_qz_620_8_002
	.byte	PATT
	 ptrvalue	mus_qz_620_8_003
	.byte	PATT
	 ptrvalue	mus_qz_620_8_004
	.byte	PATT
	 ptrvalue	mus_qz_620_8_005
	.byte	PATT
	 ptrvalue	mus_qz_620_8_006
	.byte	PATT
	 ptrvalue	mus_qz_620_8_007
	.byte		EOT   , An4
	.byte	W01
	.byte		VOL   , 4*mus_qz_620_mvl/mxv
	.byte	W06
	.byte		        2*mus_qz_620_mvl/mxv
	.byte	W06
	.byte	GOTO
	 ptrvalue	mus_qz_620_8_B1
	.byte	FINE

/********************** Track  9 **********************/

mus_qz_620_9:
	.byte		KEYSH , mus_qz_620_key+0
	.byte		VOICE , 28
	.byte		VOL   , 61*mus_qz_620_mvl/mxv
	.byte		PAN   , c_v-24
	.byte		PRIO  , 42
	.byte		N23   , Fn1 , v088
	.byte	W72
	.byte	N23
	.byte	W24
	.byte	N17
	.byte	W72
	.byte	N23
	.byte	W24
	.byte	N23
	.byte	W96
	.byte	W96
	.byte	N23
	.byte	W96
	.byte	W96
	.byte	N23
	.byte	W96
	.byte	W72
	.byte	N23
	.byte	W24
	.byte	N23
	.byte	W96
	.byte	W96
mus_qz_620_9_B1:
	.byte		N23   , Fn1 , v088
	.byte	W96
	.byte	W96
	.byte	N23
	.byte	W96
	.byte	W72
	.byte	N23
	.byte	W24
	.byte	N23
	.byte	W96
	.byte	W72
	.byte	N23
	.byte	W24
	.byte	N23
	.byte	W96
	.byte	W96
	.byte	N23
	.byte	W96
	.byte	W96
	.byte	N23
	.byte	W96
	.byte	W72
	.byte	N23
	.byte	W24
	.byte	N23
	.byte	W96
	.byte	W96
	.byte	N23
	.byte	W96
	.byte	W96
	.byte	N23
	.byte	W96
	.byte	W72
	.byte	N23
	.byte	W24
	.byte	N23
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_620_9_B1
	.byte	FINE

/******************************************************/
	.p2align	2

mus_qz_620:
	.byte	9	/* NumTrks */
	.byte	0	/* NumBlks */
	.byte	mus_qz_620_pri	/* Priority */
	.byte	mus_qz_620_rev	/* Reverb. */
space64 4
	ptrvalue	mus_qz_620_grp

	ptrvalue	mus_qz_620_1
	ptrvalue	mus_qz_620_2
	ptrvalue	mus_qz_620_3
	ptrvalue	mus_qz_620_4
	ptrvalue	mus_qz_620_5
	ptrvalue	mus_qz_620_6
	ptrvalue	mus_qz_620_7
	ptrvalue	mus_qz_620_8
	ptrvalue	mus_qz_620_9

	.end
