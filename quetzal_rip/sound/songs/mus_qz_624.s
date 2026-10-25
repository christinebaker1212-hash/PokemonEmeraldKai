	.include "asm/macros/bit_width.inc"

	.include "MPlayDef.s"

/* Moemon Quetzal song 624, disassembled from the ROM by tools/quetzal_rip/songs.py.
   "org" comments give the ROM address of the bytes that follow. */

	.equ	mus_qz_624_grp, voicegroup_qz_songs
	.equ	mus_qz_624_pri, 0
	.equ	mus_qz_624_rev, reverb_set+0
	.equ	mus_qz_624_mvl, 127
	.equ	mus_qz_624_key, 0
	.equ	mus_qz_624_tbs, 1
	.equ	mus_qz_624_exg, 1
	.equ	mus_qz_624_cmp, 1

	.section .rodata
	.global	mus_qz_624
	.p2align	2
/* org 0x0967C16C */

/********************** Track  1 **********************/

mus_qz_624_1:
	.byte		KEYSH , mus_qz_624_key+0
mus_qz_624_1_B1:
	.byte		TEMPO , 114*mus_qz_624_tbs/2
	.byte		VOICE , 5
	.byte		VOL   , 54*mus_qz_624_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte		N11   , Fs3 , v088
	.byte		N11   , Dn4 , v108
	.byte	W12
	.byte		N05   , Dn3 , v080
	.byte		N05   , An3 , v100
	.byte	W12
	.byte		        Fs3 , v080
	.byte		N05   , Dn4 , v100
	.byte	W12
	.byte		N06   , Dn4 , v080
	.byte		N06   , An4 , v100
	.byte	W24
	.byte		N23   , Bn3 , v084
	.byte		N23   , Gn4 , v108
	.byte	W24
	.byte		N05   , An3 , v092
	.byte		N05   , Fs4 , v100
	.byte	W12
	.byte		N11   , Gn3 , v080
	.byte		N11   , En4 , v100
	.byte	W12
	.byte		N09   , Gn3 , v056
	.byte		N09   , Cs4 , v100
	.byte	W84
mus_qz_624_1_000:
	.byte		N11   , En3 , v088
	.byte		N11   , Cs4 , v108
	.byte	W12
	.byte		N05   , Cs3 , v080
	.byte		N05   , An3 , v100
	.byte	W12
	.byte		N11   , En3 , v080
	.byte		N11   , Cs4 , v100
	.byte	W12
	.byte		        An3 , v080
	.byte		N11   , Fs4 , v100
	.byte	W24
	.byte		        Gn3 , v080
	.byte		N11   , En4 , v100
	.byte	W24
	.byte		        En3 , v080
	.byte		N11   , Cs4 , v100
	.byte	W12
	.byte	PEND
	.byte		        Fs3 , v080
	.byte		N11   , Dn4 , v100
	.byte	W12
	.byte		        An3 , v080
	.byte		N11   , Fs4 , v100
	.byte	W84
	.byte		        Fs3 , v088
	.byte		N11   , Dn4 , v108
	.byte	W12
	.byte		N05   , Dn3 , v080
	.byte		N05   , An3 , v100
	.byte	W12
	.byte		N11   , Fs3 , v080
	.byte		N11   , Dn4 , v100
	.byte	W12
	.byte		N09   , Dn4 , v080
	.byte		N09   , An4 , v100
	.byte	W24
	.byte		N17   , Bn3 , v080
	.byte		N17   , Gn4 , v100
	.byte	W24
	.byte		N11   , An3 , v080
	.byte		N11   , Fs4 , v100
	.byte	W12
	.byte		        Gn3 , v080
	.byte		N11   , En4 , v100
	.byte	W12
	.byte		        En3 , v056
	.byte		N11   , Cs4 , v100
	.byte	W84
	.byte	PATT
	 ptrvalue	mus_qz_624_1_000
	.byte		N32   , Fs3 , v080 , gtp3
	.byte		N32   , Dn4 , v100 , gtp3
	.byte	W92
	.byte	W03
	.byte		PAN   , c_v-44
	.byte	W01
	.byte		N40   , Fs4 , v100 , gtp1
	.byte	W48
	.byte		        An4 , v104 , gtp1
	.byte	W48
	.byte		N11   , Gn4 , v100
	.byte	W12
	.byte		        An4 , v092
	.byte	W12
	.byte		N05   , Gn4
	.byte	W06
	.byte		        Fs4 , v084
	.byte	W12
	.byte		N44   , En4 , v100 , gtp3
	.byte	W54
	.byte		N40   , Cs4 , v100 , gtp1
	.byte	W48
	.byte		        En4 , v108 , gtp1
	.byte	W48
	.byte		N11   , Fs4 , v100
	.byte	W12
	.byte		        Gn4
	.byte	W12
	.byte		N05   , Fs4 , v092
	.byte	W06
	.byte		        En4 , v088
	.byte	W12
	.byte		N44   , Dn4 , v104 , gtp3
	.byte	W54
	.byte		        Fs4 , v100 , gtp3
	.byte	W48
	.byte		N40   , An4 , v100 , gtp1
	.byte	W48
	.byte		N11   , Gn4
	.byte	W12
	.byte		        Fs4 , v096
	.byte	W12
	.byte		N05   , Gn4 , v100
	.byte	W06
	.byte		        An4 , v092
	.byte	W12
	.byte		N40   , Bn4 , v100 , gtp1
	.byte	W54
	.byte		N23   , An4
	.byte	W24
	.byte		N05   , Gn4
	.byte	W12
	.byte		        Fs4
	.byte	W06
	.byte		N44   , Gn4 , v100 , gtp3
	.byte	W54
	.byte		N11   , Fs4
	.byte	W12
	.byte		        Gn4 , v096
	.byte	W12
	.byte		N05   , Fs4 , v100
	.byte	W06
	.byte		        En4 , v092
	.byte	W12
	.byte		N32   , Dn4 , v104 , gtp3
	.byte	W48
	.byte	W03
	.byte		PAN   , c_v+0
	.byte	W03
	.byte	GOTO
	 ptrvalue	mus_qz_624_1_B1
	.byte	FINE

/********************** Track  2 **********************/

mus_qz_624_2:
	.byte		KEYSH , mus_qz_624_key+0
mus_qz_624_2_B1:
	.byte		VOICE , 19
	.byte		VOL   , 73*mus_qz_624_mvl/mxv
	.byte		PAN   , c_v-4
	.byte		PRIO  , 60
	.byte		N09   , Dn1 , v108
	.byte	W24
	.byte		        An1
	.byte	W18
	.byte		N05   , Ds1
	.byte	W18
	.byte	N11
	.byte	W12
	.byte		        Bn1
	.byte	W12
	.byte		        Ds1
	.byte	W12
	.byte		N09   , En1
	.byte	W24
	.byte		        Bn1
	.byte	W18
	.byte		N05   , Dn1
	.byte	W18
	.byte	N11
	.byte	W12
	.byte		        Bn1
	.byte	W12
	.byte		        Dn1
	.byte	W12
	.byte		        Cs1
	.byte	W24
	.byte		N09   , An1
	.byte	W18
	.byte		N17   , An0
	.byte	W18
	.byte		N05   , En1
	.byte	W12
	.byte		N11   , An1
	.byte	W12
	.byte		        An0
	.byte	W12
	.byte		        Dn1
	.byte	W24
	.byte		N09   , An1
	.byte	W18
	.byte		N05   , Dn1
	.byte	W18
	.byte		N11   , An0
	.byte	W12
	.byte		        Bn0
	.byte	W12
	.byte		        Cs1
	.byte	W12
	.byte		N09   , Dn1
	.byte	W24
	.byte		        An1
	.byte	W18
	.byte		N05   , Ds1
	.byte	W18
	.byte	N11
	.byte	W12
	.byte		        Bn1
	.byte	W12
	.byte		        Ds1
	.byte	W12
	.byte		N09   , En1
	.byte	W24
	.byte		        Bn1
	.byte	W18
	.byte		N05   , En1
	.byte	W18
	.byte	N11
	.byte	W12
	.byte		        Bn1
	.byte	W12
	.byte		        En1
	.byte	W12
	.byte		N09   , An0
	.byte	W24
	.byte		        An1
	.byte	W18
	.byte		N17   , Cs1
	.byte	W18
	.byte		N05   , En1
	.byte	W12
	.byte		N11   , An1
	.byte	W12
	.byte		        An0
	.byte	W12
	.byte		N09   , Dn1
	.byte	W24
	.byte		        An1
	.byte	W18
	.byte		N05   , Dn1
	.byte	W18
	.byte		N11   , An0
	.byte	W12
	.byte		        Bn0
	.byte	W12
	.byte		        Cs1
	.byte	W12
	.byte		N09   , Dn1
	.byte	W24
	.byte		        An1
	.byte	W18
	.byte		N05   , Ds1
	.byte	W18
	.byte	N11
	.byte	W12
	.byte		        An1
	.byte	W12
	.byte		        Ds1
	.byte	W12
	.byte		N09   , En1
	.byte	W24
	.byte		        Bn1
	.byte	W18
	.byte		N11   , Dn1
	.byte	W18
	.byte	N11
	.byte	W12
	.byte		N05   , Bn1
	.byte	W06
	.byte		N14   , Dn1 , v100
	.byte	W18
	.byte		N09   , An0 , v108
	.byte	W24
	.byte		        En1
	.byte	W18
	.byte		N17   , As0
	.byte	W18
	.byte		N05   , Cs1
	.byte	W12
	.byte		N11   , Fs1
	.byte	W12
	.byte		        As0
	.byte	W12
	.byte		N09   , Bn0
	.byte	W24
	.byte		N15   , Fs1
	.byte	W24
	.byte		N05   , Dn1
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		N05   , Cn2 , v072
	.byte	W06
	.byte		N17   , Dn1 , v108
	.byte	W18
	.byte		N09   , Gn1
	.byte	W24
	.byte		        Dn2
	.byte	W18
	.byte		N17   , Fs1
	.byte	W18
	.byte		N05   , An1
	.byte	W12
	.byte		N11   , Dn2
	.byte	W12
	.byte		        Fs1
	.byte	W12
	.byte		N09   , En1
	.byte	W24
	.byte		        Bn1
	.byte	W18
	.byte		N17   , Fn1
	.byte	W18
	.byte		N05   , Bn1
	.byte	W12
	.byte		N11   , Dn2
	.byte	W12
	.byte		        Fn1
	.byte	W12
	.byte		N09   , Fs1
	.byte	W24
	.byte		        Cs2
	.byte	W18
	.byte		N05   , Fs1 , v080
	.byte	W06
	.byte		        Gn1 , v108
	.byte	W12
	.byte	N05
	.byte	W12
	.byte		N11   , Bn1
	.byte	W12
	.byte		        Gn1
	.byte	W12
	.byte		N09   , An1
	.byte	W24
	.byte		N11   , En1
	.byte	W12
	.byte		        An0
	.byte	W12
	.byte		N05   , Dn1
	.byte	W12
	.byte		N11   , An0
	.byte	W12
	.byte		        Bn0
	.byte	W12
	.byte		        Cs1
	.byte	W12
	.byte	GOTO
	 ptrvalue	mus_qz_624_2_B1
	.byte	FINE

/********************** Track  3 **********************/

mus_qz_624_3:
	.byte		KEYSH , mus_qz_624_key+0
mus_qz_624_3_B1:
	.byte		VOICE , 3
	.byte		VOL   , 45*mus_qz_624_mvl/mxv
	.byte		PAN   , c_v+26
	.byte		PRIO  , 58
	.byte		MOD   , 10
	.byte		LFOS  , 29
	.byte		MODT  , mod_tre
	.byte		LFODL , 9
	.byte		N44   , Dn3 , v092 , gtp3
	.byte		N44   , Fs3 , v068 , gtp3
	.byte		N44   , An3 , v100 , gtp3
	.byte	W60
	.byte		N04   , Bn2 , v088
	.byte		N04   , Ds3 , v056
	.byte		N04   , Fs3 , v060
	.byte		N04   , An3 , v084
	.byte	W18
	.byte		N16   , Bn2 , v092
	.byte		N16   , Ds3 , v068
	.byte		N16   , Fs3
	.byte		N16   , An3 , v080
	.byte	W18
	.byte		N44   , Bn2 , v092 , gtp3
	.byte		N44   , Dn3 , v068 , gtp3
	.byte		N44   , En3 , v072 , gtp3
	.byte		N44   , Gn3 , v100 , gtp3
	.byte	W60
	.byte		N01   , Dn4 , v096
	.byte	W01
	.byte		        Ds4 , v092
	.byte	W02
	.byte		N08   , En4 , v112
	.byte	W09
	.byte		N05   , Bn3 , v068
	.byte		N05   , Dn4 , v092
	.byte	W06
	.byte		N17   , Gn3 , v068
	.byte		N17   , Bn3 , v092
	.byte	W18
	.byte		N44   , Cs3 , v092 , gtp3
	.byte		N44   , En3 , v064 , gtp3
	.byte		N44   , Gn3 , v068 , gtp3
	.byte		N44   , An3 , v100 , gtp3
	.byte	W60
	.byte		N05   , An2 , v092
	.byte		N05   , Bn2 , v064
	.byte		N05   , Cs3 , v068
	.byte		N05   , En3 , v100
	.byte	W18
	.byte		N17   , Cs3 , v092
	.byte		N17   , En3 , v068
	.byte		N17   , An3 , v100
	.byte	W18
	.byte		N68   , An2 , v092 , gtp3
	.byte		N68   , Dn3 , v068 , gtp3
	.byte	W01
	.byte		        Fs3 , v100 , gtp3
	.byte	W92
	.byte	W03
	.byte		N44   , Dn3 , v092 , gtp3
	.byte		N44   , En3 , v064 , gtp3
	.byte		N44   , Fs3 , v068 , gtp3
	.byte	W01
	.byte		        An3 , v100 , gtp3
	.byte	W56
	.byte	W03
	.byte		N05   , Bn2 , v092
	.byte		N05   , Ds3 , v068
	.byte		N05   , Fs3 , v064
	.byte		N05   , An3 , v100
	.byte	W18
	.byte		N17   , An2 , v068
	.byte		N17   , Bn2 , v092
	.byte		N17   , Ds3 , v068
	.byte		N17   , Fs3 , v100
	.byte	W18
	.byte		N52   , Bn2 , v092 , gtp1
	.byte		N52   , Dn3 , v068 , gtp1
	.byte		N52   , En3 , v068 , gtp1
	.byte	W01
	.byte		        Gn3 , v100 , gtp1
	.byte	W56
	.byte	W03
	.byte		N01   , Fn4 , v080
	.byte	W01
	.byte		        Fs4 , v076
	.byte	W02
	.byte		N08   , Gn4 , v088
	.byte	W08
	.byte		N04   , Bn3 , v092
	.byte	W01
	.byte		        Gn3 , v080
	.byte	W05
	.byte		N17   , Dn4 , v092
	.byte	W01
	.byte		        Bn3 , v080
	.byte	W18
	.byte		N44   , En3 , v092 , gtp3
	.byte		N44   , Gn3 , v064 , gtp3
	.byte		N44   , An3 , v068 , gtp3
	.byte		N44   , Cs4 , v100 , gtp3
	.byte	W60
	.byte		N05   , An2 , v092
	.byte		N05   , Bn2 , v060
	.byte		N05   , Cs3 , v068
	.byte		N05   , En3 , v100
	.byte	W18
	.byte		N17   , Cs3 , v092
	.byte		N17   , En3 , v068
	.byte		N17   , An3 , v100
	.byte	W18
	.byte		N44   , An2 , v092 , gtp3
	.byte		N44   , Dn3 , v068 , gtp3
	.byte	W01
	.byte		        Fs3 , v100 , gtp3
	.byte	W92
	.byte	W03
	.byte		        Dn3 , v100 , gtp3
	.byte		N44   , En3 , v068 , gtp3
	.byte		N44   , Fs3 , v072 , gtp3
	.byte		N44   , An3 , v124 , gtp3
	.byte	W60
	.byte		N05   , Bn2 , v100
	.byte		N05   , Ds3 , v072
	.byte		N05   , Fs3 , v112
	.byte	W18
	.byte		N17   , Ds3 , v100
	.byte		N17   , Fs3 , v072
	.byte		N17   , An3 , v120
	.byte	W18
	.byte		N44   , Dn3 , v100 , gtp3
	.byte		N44   , En3 , v068 , gtp3
	.byte		N44   , Gn3 , v072 , gtp3
	.byte		N44   , Bn3 , v116 , gtp3
	.byte	W56
	.byte	W03
	.byte		N02   , Bn3 , v048
	.byte	W01
	.byte		N01   , Dn4 , v112
	.byte	W01
	.byte		N02   , Ds4 , v124
	.byte	W01
	.byte		N07   , En4
	.byte	W10
	.byte		N05   , Dn4 , v096
	.byte	W06
	.byte		N17   , Bn3 , v104
	.byte	W18
	.byte		N56   , En3 , v100
	.byte		N56   , Gn3 , v068
	.byte		N56   , An3 , v072
	.byte		N56   , Cs4 , v108
	.byte	W60
	.byte		N05   , As2 , v100
	.byte		N05   , Cs3 , v072
	.byte		N05   , En3 , v068
	.byte		N05   , Fs3 , v084
	.byte	W18
	.byte		N17   , Cs3 , v100
	.byte		N17   , En3 , v068
	.byte		N17   , Fs3 , v072
	.byte		N17   , As3 , v084
	.byte	W18
	.byte		N44   , Dn3 , v100 , gtp3
	.byte		N44   , Fs3 , v072 , gtp3
	.byte		N44   , An3 , v068 , gtp3
	.byte		N44   , Bn3 , v120 , gtp3
	.byte	W56
	.byte	W03
	.byte		N01   , Bn3 , v048
	.byte	W01
	.byte		N05   , Cn3 , v100
	.byte		N05   , An3 , v072
	.byte	W01
	.byte		N01   , Cs4 , v084
	.byte	W01
	.byte		N14   , Dn4 , v108
	.byte	W16
	.byte		N18   , Cn3 , v100
	.byte		N17   , Fs3 , v072
	.byte		N17   , An3 , v068
	.byte		N17   , Fs4 , v104
	.byte	W18
	.byte		N44   , Dn3 , v100 , gtp3
	.byte		N44   , Fs3 , v072 , gtp3
	.byte		N44   , An3 , v068 , gtp3
	.byte		N44   , Dn4 , v104 , gtp3
	.byte	W60
	.byte		N04   , Dn3 , v100
	.byte		N04   , Fs3 , v072
	.byte		N04   , Bn3 , v096
	.byte	W18
	.byte		N17   , Dn3 , v100
	.byte		N17   , Fs3 , v072
	.byte		N17   , An3 , v088
	.byte	W18
	.byte		N44   , Dn3 , v100 , gtp3
	.byte		N44   , En3 , v068 , gtp3
	.byte		N44   , Gn3 , v072 , gtp3
	.byte		N44   , Bn3 , v116 , gtp3
	.byte	W60
	.byte		N05   , Dn3 , v100
	.byte		N05   , Fn3 , v068
	.byte		N05   , Gs3 , v072
	.byte		N05   , Bn3 , v100
	.byte	W18
	.byte		N17   , Fn3
	.byte		N17   , Gs3 , v068
	.byte		N17   , Bn3 , v072
	.byte		N17   , Dn4 , v112
	.byte	W18
	.byte		N44   , En3 , v100 , gtp3
	.byte		N44   , Fs3 , v068 , gtp3
	.byte		N44   , An3 , v072 , gtp3
	.byte		N44   , Cs4 , v120 , gtp3
	.byte	W60
	.byte		N05   , Dn3 , v072
	.byte		N05   , En3 , v100
	.byte		N05   , Gn3 , v072
	.byte		N05   , Bn3 , v088
	.byte	W18
	.byte		N17   , Bn2 , v100
	.byte		N17   , Dn3 , v088
	.byte		N17   , En3 , v092
	.byte		N17   , Gn3 , v120
	.byte		N17   , Gn3 , v072
	.byte	W18
	.byte		N23   , Dn3 , v092
	.byte		N44   , En3 , v072 , gtp3
	.byte		N44   , Fs3 , v068 , gtp3
	.byte		N44   , An3 , v092 , gtp3
	.byte	W24
	.byte		N23   , Cs3
	.byte	W24
	.byte		N44   , Dn3 , v092 , gtp3
	.byte		N44   , Fs3 , v072 , gtp3
	.byte		N44   , An3 , v088 , gtp3
	.byte	W48
	.byte	GOTO
	 ptrvalue	mus_qz_624_3_B1
	.byte	FINE

/********************** Track  4 **********************/

mus_qz_624_4:
	.byte		KEYSH , mus_qz_624_key+0
mus_qz_624_4_B1:
	.byte		VOICE , 39
	.byte		VOL   , 63*mus_qz_624_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 40
	.byte		N05   , Cn1 , v108
	.byte		N23   , As2 , v080
	.byte	W24
	.byte		N05   , Ds1 , v076
	.byte		N11   , As1 , v072
	.byte	W12
	.byte		N01   , Fs1 , v040
	.byte	W06
	.byte		N05   , Cn1 , v088
	.byte	W06
	.byte		N01   , Fs1 , v040
	.byte	W06
	.byte		        Fs1 , v016
	.byte	W06
	.byte		N05   , Cn1 , v108
	.byte		N01   , Fs1 , v040
	.byte	W12
	.byte		N05   , Ds1 , v076
	.byte		N01   , Fs1 , v040
	.byte	W06
	.byte		        Fs1 , v024
	.byte	W06
	.byte		N11   , As1 , v072
	.byte	W12
	.byte		N05   , Cn1 , v108
	.byte		N01   , Fs1 , v040
	.byte	W06
	.byte		        Fs1 , v016
	.byte	W06
	.byte		        Fs1 , v040
	.byte	W12
	.byte		N05   , Ds1 , v076
	.byte		N11   , As1 , v072
	.byte	W12
	.byte		N01   , Fs1 , v040
	.byte	W06
	.byte		N05   , Cn1 , v088
	.byte	W06
	.byte		N01   , Fs1 , v044
	.byte	W03
	.byte		        Fs1 , v024
	.byte	W03
	.byte		        Fs1 , v032
	.byte	W06
	.byte		N05   , Ds1 , v076
	.byte		N01   , Fs1 , v040
	.byte	W06
	.byte	N01
	.byte	W06
	.byte		N11   , As1 , v072
	.byte	W06
	.byte		N05   , Ds1 , v076
	.byte	W06
	.byte		        Cn1 , v088
	.byte		N01   , Fs1 , v040
	.byte	W12
	.byte		N05   , Cn1 , v108
	.byte		N01   , Fs1 , v040
	.byte	W06
	.byte		        Fs1 , v016
	.byte	W06
	.byte		        Fs1 , v040
	.byte	W12
	.byte		N05   , Ds1 , v076
	.byte		N11   , As1 , v072
	.byte	W12
	.byte		N01   , Fs1 , v040
	.byte	W06
	.byte		N05   , Cn1 , v108
	.byte	W06
	.byte		N01   , Fs1 , v040
	.byte	W06
	.byte		        Fs1 , v016
	.byte	W06
	.byte		        Fs1 , v040
	.byte	W12
	.byte		N05   , Ds1 , v076
	.byte		N11   , As1 , v072
	.byte	W12
	.byte		N05   , Cn1 , v088
	.byte		N01   , Fs1 , v040
	.byte	W12
	.byte		N05   , Cn1 , v108
	.byte		N01   , Fs1 , v040
	.byte	W06
	.byte		        Fs1 , v016
	.byte	W06
	.byte		        Fs1 , v040
	.byte	W12
	.byte		N05   , Ds1 , v076
	.byte		N11   , As1 , v072
	.byte	W12
	.byte		N01   , Fs1 , v040
	.byte	W06
	.byte		N05   , Cn1 , v108
	.byte	W06
	.byte		N01   , Fs1 , v040
	.byte	W06
	.byte		        Fs1 , v016
	.byte	W06
	.byte		N05   , Ds1 , v076
	.byte		N01   , Fs1 , v040
	.byte	W06
	.byte	N01
	.byte	W06
	.byte	N01
	.byte	W06
	.byte		N05   , Ds1 , v076
	.byte		N01   , Fs1 , v016
	.byte	W06
	.byte		N05   , Cn1 , v108
	.byte		N11   , As1 , v072
	.byte	W12
	.byte		N05   , Cn1 , v108
	.byte		N23   , An2 , v080
	.byte	W24
	.byte		N05   , Ds1 , v076
	.byte		N11   , As1 , v072
	.byte	W12
	.byte		N01   , Fs1 , v044
	.byte	W06
	.byte		N05   , Cn1 , v108
	.byte	W06
	.byte		N01   , Fs1 , v044
	.byte	W06
	.byte		        Fs1 , v020
	.byte	W06
	.byte		        Fs1 , v044
	.byte	W06
	.byte		        Fs1 , v020
	.byte	W06
	.byte		N05   , Ds1 , v076
	.byte		N11   , As1 , v072
	.byte	W12
	.byte		N05   , Cn1 , v088
	.byte		N01   , Fs1 , v044
	.byte	W12
	.byte		N05   , Cn1 , v108
	.byte		N01   , Fs1 , v044
	.byte	W06
	.byte		        Fs1 , v020
	.byte	W06
	.byte		        Fs1 , v044
	.byte	W12
	.byte		N05   , Ds1 , v076
	.byte		N11   , As1 , v072
	.byte	W12
	.byte		N01   , Fs1 , v044
	.byte	W06
	.byte		N05   , Cn1 , v108
	.byte	W06
	.byte		N01   , Fs1 , v044
	.byte	W03
	.byte		        Fs1 , v028
	.byte	W03
	.byte		        Fs1 , v032
	.byte	W06
	.byte		N05   , Cn1 , v108
	.byte		N05   , Ds1 , v076
	.byte	W06
	.byte		N01   , Fs1 , v044
	.byte	W06
	.byte	N01
	.byte	W06
	.byte		N05   , Ds1 , v076
	.byte		N01   , Fs1 , v020
	.byte	W06
	.byte		N11   , As1 , v072
	.byte	W12
	.byte		N05   , Cn1 , v108
	.byte		N01   , Fs1 , v044
	.byte	W06
	.byte		        Fs1 , v020
	.byte	W06
	.byte		        Fs1 , v044
	.byte	W12
	.byte		N05   , Ds1 , v076
	.byte		N11   , As1 , v072
	.byte	W12
	.byte		N01   , Fs1 , v024
	.byte	W03
	.byte		        Fs1 , v032
	.byte	W03
	.byte		N05   , Cn1 , v108
	.byte		N01   , Fs1 , v048
	.byte	W06
	.byte		        Fs1 , v044
	.byte	W06
	.byte		        Fs1 , v020
	.byte	W12
	.byte		        Fs1 , v044
	.byte	W06
	.byte		N05   , Ds1 , v076
	.byte		N11   , As1 , v072
	.byte	W12
	.byte		N05   , Cn1 , v088
	.byte		N01   , Fs1 , v044
	.byte	W12
	.byte		N05   , Cn1 , v108
	.byte		N23   , En2 , v068
	.byte	W24
	.byte		N05   , Ds1 , v076
	.byte		N01   , Fs1 , v044
	.byte	W06
	.byte		        Fs1 , v032
	.byte	W06
	.byte		N11   , As1 , v072
	.byte	W06
	.byte		N05   , Cn1 , v108
	.byte	W06
	.byte		N11   , Bn1 , v072
	.byte	W12
	.byte		N05   , Cn1 , v108
	.byte		N11   , An1 , v084
	.byte	W12
	.byte		N05   , Ds1 , v064
	.byte	W06
	.byte	N05
	.byte		N17   , Dn2 , v080
	.byte	W06
	.byte		N11   , Gn1 , v084
	.byte	W12
	.byte		N05   , Cn1 , v108
	.byte		N23   , An2 , v080
	.byte	W24
	.byte		N11   , En1
	.byte		N01   , Fs1 , v048
	.byte	W12
	.byte		        Fs1 , v060
	.byte	W06
	.byte		N05   , Cn1 , v080
	.byte		N01   , Fs1 , v032
	.byte	W06
	.byte		        Fs1 , v048
	.byte	W06
	.byte		        Fs1 , v024
	.byte	W06
	.byte		N05   , Cn1 , v108
	.byte		N01   , Fs1 , v048
	.byte	W06
	.byte		        Fs1 , v024
	.byte	W06
	.byte		N11   , En1 , v080
	.byte		N01   , Fs1 , v048
	.byte	W12
	.byte		N11   , As1 , v072
	.byte	W12
	.byte		N05   , Cn1 , v108
	.byte		N01   , Fs1 , v056
	.byte	W06
	.byte		        Fs1 , v028
	.byte	W06
	.byte		        Fs1 , v056
	.byte	W06
	.byte		        Fs1 , v028
	.byte	W06
	.byte		N11   , En1 , v080
	.byte		N01   , Fs1 , v056
	.byte	W06
	.byte		        Fs1 , v028
	.byte	W06
	.byte		        Fs1 , v048
	.byte	W06
	.byte		N05   , Cn1 , v080
	.byte		N01   , Fs1 , v032
	.byte	W06
	.byte		        Fs1 , v048
	.byte	W06
	.byte		        Fs1 , v036
	.byte	W06
	.byte		N05   , Cn1 , v108
	.byte		N01   , Fs1 , v048
	.byte	W06
	.byte		        Fs1 , v060
	.byte	W03
	.byte		        Fs1 , v036
	.byte	W03
	.byte		N03   , Cn1 , v088
	.byte		N01   , Fs1 , v036
	.byte	W06
	.byte		N05   , Cn1 , v108
	.byte		N17   , As1 , v072
	.byte	W18
	.byte		N05   , Cn1 , v108
	.byte		N20   , En2 , v072
	.byte	W24
	.byte		N11   , En1 , v080
	.byte		N01   , Fs1 , v048
	.byte	W12
	.byte		        Fs1 , v036
	.byte	W06
	.byte		N05   , Cn1 , v080
	.byte	W06
	.byte		N01   , Fs1 , v048
	.byte	W06
	.byte		        Fs1 , v024
	.byte	W06
	.byte		N05   , Cn1 , v108
	.byte		N11   , As1 , v072
	.byte	W12
	.byte		        En1 , v080
	.byte		N01   , Fs1 , v064
	.byte	W12
	.byte		        Fs1 , v044
	.byte	W12
	.byte		N05   , Cn1 , v108
	.byte		N01   , Fs1 , v048
	.byte		N23   , An2 , v080
	.byte	W06
	.byte		N01   , Fs1 , v024
	.byte	W06
	.byte		        Fs1 , v048
	.byte	W12
	.byte		N11   , En1 , v080
	.byte	W12
	.byte		N01   , Fs1 , v060
	.byte	W03
	.byte		        Fs1 , v036
	.byte	W03
	.byte		N05   , Cn1 , v080
	.byte		N01   , Fs1 , v048
	.byte	W06
	.byte	N01
	.byte	W12
	.byte		N05   , Cn1 , v108
	.byte		N17   , An2 , v080
	.byte	W18
	.byte		N05   , Cn1 , v084
	.byte		N17   , Dn2 , v068
	.byte	W18
	.byte		N05   , Cn1 , v108
	.byte		N23   , An2 , v080
	.byte	W24
	.byte		N11   , En1
	.byte		N01   , Fs1 , v060
	.byte	W12
	.byte		        Fs1 , v048
	.byte	W06
	.byte		N05   , Cn1 , v080
	.byte	W06
	.byte		N01   , Fs1 , v060
	.byte	W03
	.byte		        Fs1 , v040
	.byte	W03
	.byte		        Fs1 , v056
	.byte	W06
	.byte		N05   , Cn1 , v108
	.byte		N01   , Fs1 , v048
	.byte	W06
	.byte	N01
	.byte	W06
	.byte		N11   , En1 , v080
	.byte		N01   , Fs1 , v048
	.byte	W06
	.byte	N01
	.byte	W06
	.byte		N11   , As1 , v072
	.byte	W12
	.byte		N05   , Cn1 , v108
	.byte		N01   , Fs1 , v048
	.byte	W06
	.byte		        Fs1 , v024
	.byte	W06
	.byte		        Fs1 , v048
	.byte	W06
	.byte		        Fs1 , v036
	.byte	W06
	.byte		N11   , En1 , v080
	.byte		N01   , Fs1 , v048
	.byte	W12
	.byte	N01
	.byte	W06
	.byte		N05   , Cn1 , v080
	.byte	W06
	.byte		N01   , Fs1 , v048
	.byte	W06
	.byte		        Fs1 , v024
	.byte	W06
	.byte		N05   , Cn1 , v108
	.byte		N11   , As1 , v072
	.byte	W12
	.byte		        En1 , v080
	.byte		N01   , Fs1 , v048
	.byte	W12
	.byte		        Fs1 , v036
	.byte	W12
	.byte		N05   , Cn1 , v108
	.byte		N23   , An2 , v072
	.byte	W24
	.byte		N11   , En1 , v080
	.byte	W12
	.byte		N01   , Fs1 , v048
	.byte	W06
	.byte		N05   , Cn1 , v080
	.byte	W06
	.byte		N01   , Fs1 , v048
	.byte	W06
	.byte		        Fs1 , v024
	.byte	W06
	.byte		N05   , Cn1 , v108
	.byte		N11   , As1 , v072
	.byte	W12
	.byte		        En1 , v080
	.byte		N01   , Fs1 , v036
	.byte	W05
	.byte		        Fs1 , v024
	.byte	W07
	.byte		        Fs1 , v036
	.byte	W12
	.byte		N05   , Cn1 , v108
	.byte		N32   , An2 , v072 , gtp3
	.byte	W24
	.byte		N11   , En1 , v080
	.byte	W12
	.byte		N03   , En1 , v024
	.byte	W04
	.byte		        En1 , v044
	.byte	W04
	.byte		        En1 , v076
	.byte	W04
	.byte		N11   , En1 , v108
	.byte	W12
	.byte		        An1 , v084
	.byte	W12
	.byte		N05   , En1 , v108
	.byte	W06
	.byte		        Cn1 , v100
	.byte		N05   , En1 , v096
	.byte		N17   , En2 , v060
	.byte	W06
	.byte		N11   , Gn1 , v084
	.byte	W12
	.byte	GOTO
	 ptrvalue	mus_qz_624_4_B1
	.byte	FINE

/********************** Track  5 **********************/

mus_qz_624_5:
	.byte		KEYSH , mus_qz_624_key+0
mus_qz_624_5_B1:
	.byte		VOICE , 26
	.byte		VOL   , 38*mus_qz_624_mvl/mxv
	.byte		PAN   , c_v-44
	.byte		PRIO  , 52
	.byte	W96
	.byte	W28
	.byte	W01
	.byte		N05   , Bn2 , v080
	.byte	W01
	.byte		        En3 , v100
	.byte	W05
	.byte		        Dn3 , v076
	.byte	W01
	.byte		        Gn3 , v096
	.byte	W05
	.byte		        Gn3 , v080
	.byte	W01
	.byte		        Bn3 , v100
	.byte	W17
	.byte		        Bn3 , v080
	.byte	W01
	.byte		        En4 , v100
	.byte	W11
	.byte		        Gn3 , v080
	.byte	W01
	.byte		        Bn3 , v100
	.byte	W05
	.byte		        Bn3 , v076
	.byte	W01
	.byte		        Dn4 , v096
	.byte	W18
	.byte	W96
mus_qz_624_5_000:
	.byte	W32
	.byte	W03
	.byte		N05   , Dn3 , v080
	.byte	W01
	.byte		        Fs3 , v100
	.byte	W05
	.byte		        En3 , v076
	.byte	W01
	.byte		        Gn3 , v096
	.byte	W17
	.byte		        Fs3 , v080
	.byte	W01
	.byte		        An3 , v100
	.byte	W11
	.byte		        Dn3 , v080
	.byte	W01
	.byte		        Fs3 , v100
	.byte	W05
	.byte		        Fs3 , v076
	.byte	W01
	.byte		        An3 , v096
	.byte	W18
	.byte	PEND
	.byte	W96
	.byte	W28
	.byte	W01
	.byte		        Bn2 , v080
	.byte	W01
	.byte		        En3 , v100
	.byte	W05
	.byte		        Dn3 , v076
	.byte	W01
	.byte		        Gn3 , v096
	.byte	W05
	.byte		        Gn3 , v080
	.byte	W01
	.byte		        Bn3 , v100
	.byte	W17
	.byte		        En4 , v080
	.byte	W01
	.byte		        Gn4 , v100
	.byte	W11
	.byte		        Gn3 , v080
	.byte	W01
	.byte		        Bn3 , v100
	.byte	W05
	.byte		        Bn3 , v076
	.byte	W01
	.byte		        Dn4 , v096
	.byte	W18
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_624_5_000
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_624_5_B1
	.byte	FINE

/********************** Track  6 **********************/

mus_qz_624_6:
	.byte		KEYSH , mus_qz_624_key+0
mus_qz_624_6_B1:
	.byte		VOICE , 23
	.byte		PAN   , c_v+0
	.byte		PRIO  , 50
	.byte		VOL   , 48*mus_qz_624_mvl/mxv
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W84
	.byte		        48*mus_qz_624_mvl/mxv
	.byte	W09
	.byte		        45*mus_qz_624_mvl/mxv
	.byte	W03
	.byte		N44   , An3 , v072 , gtp2
	.byte		N44   , Fs4 , v120 , gtp2
	.byte	W36
	.byte		VOL   , 38*mus_qz_624_mvl/mxv
	.byte	W06
	.byte		        35*mus_qz_624_mvl/mxv
	.byte	W06
	.byte		        46*mus_qz_624_mvl/mxv
	.byte		N44   , Ds4 , v072 , gtp2
	.byte		N44   , An4 , v120 , gtp2
	.byte	W30
	.byte		VOL   , 43*mus_qz_624_mvl/mxv
	.byte	W06
	.byte		        36*mus_qz_624_mvl/mxv
	.byte	W06
	.byte		        31*mus_qz_624_mvl/mxv
	.byte	W06
	.byte		        46*mus_qz_624_mvl/mxv
	.byte		N10   , Bn3 , v072
	.byte		N10   , Gn4 , v120
	.byte	W12
	.byte		        Cs4 , v072
	.byte		N10   , An4 , v120
	.byte	W12
	.byte		N04   , Bn3 , v072
	.byte		N04   , Gn4 , v120
	.byte	W06
	.byte		        An3 , v072
	.byte		N04   , Fs4 , v120
	.byte	W12
	.byte		N48   , Gn3 , v072 , gtp2
	.byte		N48   , En4 , v120 , gtp2
	.byte	W30
	.byte		VOL   , 42*mus_qz_624_mvl/mxv
	.byte	W06
	.byte		        38*mus_qz_624_mvl/mxv
	.byte	W06
	.byte		        35*mus_qz_624_mvl/mxv
	.byte	W06
	.byte		        29*mus_qz_624_mvl/mxv
	.byte	W06
	.byte		        47*mus_qz_624_mvl/mxv
	.byte		N44   , En3 , v072 , gtp2
	.byte		N44   , Cs4 , v120 , gtp2
	.byte	W30
	.byte		VOL   , 43*mus_qz_624_mvl/mxv
	.byte	W06
	.byte		        36*mus_qz_624_mvl/mxv
	.byte	W06
	.byte		        31*mus_qz_624_mvl/mxv
	.byte	W06
	.byte		        46*mus_qz_624_mvl/mxv
	.byte		N22   , As3 , v072
	.byte		N44   , En4 , v120 , gtp2
	.byte	W24
	.byte		N22   , Fs3 , v072
	.byte	W06
	.byte		VOL   , 43*mus_qz_624_mvl/mxv
	.byte	W06
	.byte		        38*mus_qz_624_mvl/mxv
	.byte	W06
	.byte		        33*mus_qz_624_mvl/mxv
	.byte	W06
	.byte		        46*mus_qz_624_mvl/mxv
	.byte		N10   , An3
	.byte		N10   , Fs4 , v120
	.byte	W12
	.byte		        Bn3 , v072
	.byte		N10   , Gn4 , v120
	.byte	W12
	.byte		N04   , An3 , v072
	.byte		N04   , Fs4 , v120
	.byte	W06
	.byte		        Gn3 , v072
	.byte		N04   , En4 , v120
	.byte	W12
	.byte		N48   , Fs3 , v072 , gtp2
	.byte		N48   , Dn4 , v120 , gtp2
	.byte	W30
	.byte		VOL   , 42*mus_qz_624_mvl/mxv
	.byte	W06
	.byte		        37*mus_qz_624_mvl/mxv
	.byte	W06
	.byte		        33*mus_qz_624_mvl/mxv
	.byte	W06
	.byte		        29*mus_qz_624_mvl/mxv
	.byte	W06
	.byte		        47*mus_qz_624_mvl/mxv
	.byte		N44   , An3 , v072 , gtp2
	.byte		N44   , Fs4 , v120 , gtp2
	.byte	W30
	.byte		VOL   , 42*mus_qz_624_mvl/mxv
	.byte	W06
	.byte		        38*mus_qz_624_mvl/mxv
	.byte	W06
	.byte		        32*mus_qz_624_mvl/mxv
	.byte	W06
	.byte		        46*mus_qz_624_mvl/mxv
	.byte		N44   , Dn4 , v072 , gtp2
	.byte		N44   , An4 , v120 , gtp2
	.byte	W30
	.byte		VOL   , 43*mus_qz_624_mvl/mxv
	.byte	W06
	.byte		        37*mus_qz_624_mvl/mxv
	.byte	W06
	.byte		        32*mus_qz_624_mvl/mxv
	.byte	W06
	.byte		        46*mus_qz_624_mvl/mxv
	.byte		N10   , Bn3 , v072
	.byte		N10   , Gn4 , v120
	.byte	W12
	.byte		        An3 , v072
	.byte		N10   , Fs4 , v120
	.byte	W12
	.byte		N04   , Bn3 , v072
	.byte		N04   , Gn4 , v120
	.byte	W06
	.byte		        Cs4 , v072
	.byte		N04   , An4 , v120
	.byte	W12
	.byte		N52   , Dn4 , v072
	.byte		N52   , Bn4 , v120
	.byte	W36
	.byte		VOL   , 40*mus_qz_624_mvl/mxv
	.byte	W06
	.byte		        35*mus_qz_624_mvl/mxv
	.byte	W06
	.byte		        31*mus_qz_624_mvl/mxv
	.byte	W06
	.byte		        45*mus_qz_624_mvl/mxv
	.byte		N22   , Cs4 , v072
	.byte		N22   , An4 , v120
	.byte	W24
	.byte		N04   , Bn3 , v072
	.byte		N04   , Gn4 , v120
	.byte	W12
	.byte		        An3 , v072
	.byte		N04   , Fs4 , v120
	.byte	W06
	.byte		N44   , Bn3 , v072 , gtp2
	.byte		N44   , Gn4 , v120 , gtp2
	.byte	W30
	.byte		VOL   , 41*mus_qz_624_mvl/mxv
	.byte	W06
	.byte		        35*mus_qz_624_mvl/mxv
	.byte	W06
	.byte		        31*mus_qz_624_mvl/mxv
	.byte	W06
	.byte		        27*mus_qz_624_mvl/mxv
	.byte	W06
	.byte		        47*mus_qz_624_mvl/mxv
	.byte		N10   , An3 , v072
	.byte		N10   , Fs4 , v120
	.byte	W12
	.byte		        Bn3 , v072
	.byte		N10   , Gn4 , v120
	.byte	W12
	.byte		N04   , An3 , v072
	.byte		N04   , Fs4 , v120
	.byte	W06
	.byte		        Gn3 , v072
	.byte		N04   , En4 , v120
	.byte	W12
	.byte		N52   , Fs3 , v072
	.byte		N52   , Dn4 , v120
	.byte	W30
	.byte		VOL   , 42*mus_qz_624_mvl/mxv
	.byte	W06
	.byte		        35*mus_qz_624_mvl/mxv
	.byte	W06
	.byte		        28*mus_qz_624_mvl/mxv
	.byte	W06
	.byte		        15*mus_qz_624_mvl/mxv
	.byte		        25*mus_qz_624_mvl/mxv
	.byte	W05
	.byte		        46*mus_qz_624_mvl/mxv
	.byte	W01
	.byte	GOTO
	 ptrvalue	mus_qz_624_6_B1
	.byte	FINE

/********************** Track  7 **********************/

mus_qz_624_7:
	.byte		KEYSH , mus_qz_624_key+0
mus_qz_624_7_B1:
	.byte		VOICE , 5
	.byte		VOL   , 13*mus_qz_624_mvl/mxv
	.byte		PAN   , c_v+56
	.byte		PRIO  , 40
	.byte	W12
	.byte		N10   , Dn4 , v108
	.byte	W12
	.byte		N04   , An3 , v100
	.byte	W12
	.byte		        Dn4
	.byte	W12
	.byte		N06   , An4
	.byte	W24
	.byte		N22   , Gn4 , v108
	.byte	W24
	.byte		N04   , Fs4 , v100
	.byte	W12
	.byte		N10   , En4
	.byte	W12
	.byte		N09   , Cs4
	.byte	W72
mus_qz_624_7_000:
	.byte	W12
	.byte		N10   , Cs4 , v108
	.byte	W12
	.byte		N04   , An3 , v100
	.byte	W12
	.byte		N10   , Cs4
	.byte	W12
	.byte		        Fs4
	.byte	W24
	.byte		        En4
	.byte	W24
	.byte	PEND
	.byte		        Cs4
	.byte	W12
	.byte		        Dn4
	.byte	W12
	.byte		        Fs4
	.byte	W72
	.byte	W12
	.byte		        Dn4 , v108
	.byte	W12
	.byte		N04   , An3 , v100
	.byte	W12
	.byte		N10   , Dn4
	.byte	W12
	.byte		N08   , An4
	.byte	W24
	.byte		N16   , Gn4
	.byte	W24
	.byte		N10   , Fs4
	.byte	W12
	.byte		        En4
	.byte	W12
	.byte		        Cs4
	.byte	W72
	.byte	PATT
	 ptrvalue	mus_qz_624_7_000
	.byte		N10   , Cs4 , v100
	.byte	W12
	.byte		N32   , Dn4 , v100 , gtp2
	.byte	W80
	.byte	W03
	.byte		PAN   , c_v-44
	.byte	W01
	.byte	W12
	.byte		N40   , Fs4 , v108
	.byte	W48
	.byte		        An4 , v112 , gtp1
	.byte	W36
	.byte	W12
	.byte		N11   , Gn4 , v108
	.byte	W12
	.byte		        An4 , v100
	.byte	W12
	.byte		N05   , Gn4
	.byte	W06
	.byte		        Fs4 , v092
	.byte	W12
	.byte		N40   , En4 , v108 , gtp1
	.byte	W42
	.byte	W12
	.byte		        Cs4 , v108 , gtp1
	.byte	W48
	.byte		        En4 , v112 , gtp1
	.byte	W36
	.byte	W12
	.byte		N11   , Fs4 , v108
	.byte	W12
	.byte		        Gn4
	.byte	W12
	.byte		N05   , Fs4 , v100
	.byte	W06
	.byte		        En4 , v096
	.byte	W12
	.byte		N44   , Dn4 , v112 , gtp3
	.byte	W42
	.byte	W12
	.byte		        Fs4 , v108 , gtp3
	.byte	W48
	.byte		N40   , An4 , v108 , gtp1
	.byte	W36
	.byte	W12
	.byte		N11   , Gn4
	.byte	W12
	.byte		        Fs4 , v104
	.byte	W12
	.byte		N05   , Gn4 , v108
	.byte	W06
	.byte		        An4 , v100
	.byte	W12
	.byte		N40   , Bn4 , v108 , gtp1
	.byte	W42
	.byte	W12
	.byte		N23   , An4
	.byte	W24
	.byte		N05   , Gn4
	.byte	W12
	.byte		        Fs4
	.byte	W06
	.byte		N44   , Gn4 , v108 , gtp3
	.byte	W42
	.byte	W12
	.byte		N11   , Fs4
	.byte	W12
	.byte		        Gn4 , v104
	.byte	W12
	.byte		N05   , Fs4 , v108
	.byte	W06
	.byte		        En4 , v100
	.byte	W12
	.byte		N40   , Dn4 , v112 , gtp1
	.byte	W40
	.byte	W01
	.byte		PAN   , c_v-44
	.byte	W01
	.byte	GOTO
	 ptrvalue	mus_qz_624_7_B1
	.byte	FINE

/********************** Track  8 **********************/

mus_qz_624_8:
	.byte		KEYSH , mus_qz_624_key+0
mus_qz_624_8_B1:
	.byte		VOICE , 26
	.byte		VOL   , 13*mus_qz_624_mvl/mxv
	.byte		PAN   , c_v+56
	.byte		PRIO  , 38
	.byte	W96
	.byte	W36
	.byte		N05   , En3 , v100
	.byte	W06
	.byte		        Gn3
	.byte	W06
	.byte		        Bn3
	.byte	W18
	.byte		        En4
	.byte	W12
	.byte		        Bn3
	.byte	W06
	.byte		        Dn4
	.byte	W12
	.byte	W96
mus_qz_624_8_000:
	.byte	W42
	.byte		N05   , Fs3 , v100
	.byte	W06
	.byte		        Gn3
	.byte	W18
	.byte		        An3
	.byte	W12
	.byte		        Fs3
	.byte	W06
	.byte		        An3
	.byte	W12
	.byte	PEND
	.byte	W96
	.byte	W36
	.byte		        En3
	.byte	W06
	.byte		        Gn3
	.byte	W06
	.byte		        Bn3
	.byte	W18
	.byte		        Gn4
	.byte	W12
	.byte		        Bn3
	.byte	W06
	.byte		        Dn4
	.byte	W12
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_624_8_000
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_624_8_B1
	.byte	FINE

/******************************************************/
	.p2align	2

mus_qz_624:
	.byte	8	/* NumTrks */
	.byte	0	/* NumBlks */
	.byte	mus_qz_624_pri	/* Priority */
	.byte	mus_qz_624_rev	/* Reverb. */
space64 4
	ptrvalue	mus_qz_624_grp

	ptrvalue	mus_qz_624_1
	ptrvalue	mus_qz_624_2
	ptrvalue	mus_qz_624_3
	ptrvalue	mus_qz_624_4
	ptrvalue	mus_qz_624_5
	ptrvalue	mus_qz_624_6
	ptrvalue	mus_qz_624_7
	ptrvalue	mus_qz_624_8

	.end
