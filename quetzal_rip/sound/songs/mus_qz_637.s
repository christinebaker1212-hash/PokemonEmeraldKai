	.include "asm/macros/bit_width.inc"

	.include "MPlayDef.s"

/* Moemon Quetzal song 637, disassembled from the ROM by tools/quetzal_rip/songs.py.
   "org" comments give the ROM address of the bytes that follow. */

	.equ	mus_qz_637_grp, voicegroup_qz_songs
	.equ	mus_qz_637_pri, 0
	.equ	mus_qz_637_rev, reverb_set+0
	.equ	mus_qz_637_mvl, 127
	.equ	mus_qz_637_key, 0
	.equ	mus_qz_637_tbs, 1
	.equ	mus_qz_637_exg, 1
	.equ	mus_qz_637_cmp, 1

	.section .rodata
	.global	mus_qz_637
	.p2align	2
/* org 0x0968A77C */

/********************** Track  1 **********************/

mus_qz_637_1:
	.byte		KEYSH , mus_qz_637_key+0
	.byte		TEMPO , 130*mus_qz_637_tbs/2
	.byte		VOICE , 10
	.byte		VOL   , 73*mus_qz_637_mvl/mxv
	.byte		PAN   , c_v-4
	.byte		PRIO  , 64
	.byte		MOD   , 13
	.byte		LFOS  , 25
	.byte		LFODL , 1
	.byte	W96
	.byte	W96
mus_qz_637_1_B1:
	.byte	W96
	.byte	W96
	.byte		N64   , Ds4 , v100 , gtp1
	.byte	W72
	.byte		N11   , As4 , v088
	.byte	W12
	.byte		        An4 , v080
	.byte	W12
	.byte		N44   , Cs5 , v104 , gtp3
	.byte	W48
	.byte		        Cn5 , v092 , gtp3
	.byte	W48
	.byte		N52   , As4 , v104 , gtp1
	.byte	W72
	.byte		N23   , An4 , v092
	.byte	W24
	.byte		N40   , Fs4 , v104 , gtp1
	.byte	W48
	.byte		        Fn4 , v092 , gtp1
	.byte	W48
	.byte		TEMPO , 130*mus_qz_637_tbs/2
	.byte		N64   , Ds5 , v108 , gtp1
	.byte	W72
	.byte		N11   , As5
	.byte	W12
	.byte		        An5 , v088
	.byte	W12
	.byte		N44   , Fs5 , v100 , gtp3
	.byte	W48
	.byte		        Fn5 , v092 , gtp3
	.byte	W48
	.byte		N52   , Ds5 , v108 , gtp1
	.byte	W72
	.byte		N23   , Dn5 , v096
	.byte	W24
	.byte		N40   , Bn4 , v108 , gtp1
	.byte	W48
	.byte		        As4 , v096 , gtp1
	.byte	W48
	.byte		N64   , Fn5 , v108 , gtp1
	.byte	W72
	.byte		N11   , Cs6
	.byte	W12
	.byte		        Cn6 , v088
	.byte	W12
	.byte		N44   , Gs5 , v100 , gtp2
	.byte	W48
	.byte		        Gn5 , v088 , gtp2
	.byte	W48
	.byte		N52   , Fn5 , v100 , gtp1
	.byte	W72
	.byte		N23   , En5 , v088
	.byte	W24
	.byte		N40   , Cs5 , v100 , gtp1
	.byte	W48
	.byte		        Cn5 , v092 , gtp1
	.byte	W48
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
	 ptrvalue	mus_qz_637_1_B1
	.byte	FINE

/********************** Track  2 **********************/

mus_qz_637_2:
	.byte		KEYSH , mus_qz_637_key+0
	.byte		VOICE , 33
	.byte		VOL   , 54*mus_qz_637_mvl/mxv
	.byte		PAN   , c_v-32
	.byte		PRIO  , 60
	.byte		MOD   , 8
	.byte		LFOS  , 25
	.byte		LFODL , 5
	.byte		VOL   , 42*mus_qz_637_mvl/mxv
	.byte		N05   , Ds2 , v096
	.byte	W24
	.byte		N23   , As2 , v100
	.byte	W24
	.byte		N06   , An2 , v092
	.byte	W48
mus_qz_637_2_000:
	.byte		N11   , Ds2 , v096
	.byte	W12
	.byte		N23   , As2 , v100
	.byte	W24
	.byte		N06   , An2 , v092
	.byte	W60
	.byte	PEND
mus_qz_637_2_B1:
	.byte		N05   , Ds2 , v096
	.byte	W24
	.byte		N23   , As2 , v100
	.byte	W24
	.byte		N06   , An2 , v092
	.byte	W48
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_637_2_000
	.byte	PATT
	 ptrvalue	mus_qz_637_2_B1
	.byte	PATT
	 ptrvalue	mus_qz_637_2_000
	.byte	PATT
	 ptrvalue	mus_qz_637_2_B1
	.byte		N11   , Ds2 , v096
	.byte	W12
	.byte		N23   , As2 , v100
	.byte	W24
	.byte		N06   , An2 , v092
	.byte	W48
	.byte		N02   , Fs3 , v048
	.byte	W03
	.byte		        As3 , v052
	.byte	W03
	.byte		        Bn3 , v060
	.byte	W03
	.byte		        Cs4 , v072
	.byte	W03
	.byte		N11   , Ds4 , v076
	.byte	W12
	.byte		        Dn4 , v064
	.byte	W12
	.byte		        Fn3 , v060
	.byte	W12
	.byte		        Fs3
	.byte	W12
	.byte		        Cs4 , v072
	.byte	W12
	.byte		        Cn4 , v060
	.byte	W12
	.byte		        Cs4 , v068
	.byte	W12
	.byte		        Cn4 , v060
	.byte	W12
	.byte		        As3 , v072
	.byte	W12
	.byte		        An3 , v060
	.byte	W12
	.byte		        Fn3 , v068
	.byte	W12
	.byte		        Fs3 , v060
	.byte	W12
	.byte		        Cs4 , v072
	.byte	W12
	.byte		        Cn4 , v064
	.byte	W12
	.byte		        As3 , v068
	.byte	W12
	.byte		        An3 , v060
	.byte	W12
	.byte		        Fs3 , v076
	.byte	W12
	.byte		        Fn3 , v064
	.byte	W12
	.byte		        Ds3 , v072
	.byte	W12
	.byte		        Fn3 , v064
	.byte	W12
	.byte		        Fs3 , v072
	.byte	W12
	.byte		        As3 , v068
	.byte	W12
	.byte		        Fs3 , v076
	.byte	W12
	.byte		        Fn3 , v064
	.byte	W12
	.byte		        Ds3 , v076
	.byte	W12
	.byte		        Fn3 , v068
	.byte	W12
	.byte		        Fs3 , v080
	.byte	W12
	.byte		        An3 , v072
	.byte	W12
	.byte		        As2 , v076
	.byte	W12
	.byte		        Bn2 , v068
	.byte	W12
	.byte		        Ds3 , v076
	.byte	W12
	.byte		        Fn3 , v068
	.byte	W12
	.byte		        Fn4 , v080
	.byte	W12
	.byte		        En4 , v068
	.byte	W12
	.byte		        Gn3 , v076
	.byte	W12
	.byte		        Gs3 , v060
	.byte	W12
	.byte		        Ds4 , v072
	.byte	W12
	.byte		        Dn4 , v064
	.byte	W12
	.byte		        Ds4 , v068
	.byte	W12
	.byte		        Dn4 , v060
	.byte	W12
	.byte		        Cn4 , v076
	.byte	W12
	.byte		        Bn3 , v064
	.byte	W12
	.byte		        Gn3 , v068
	.byte	W12
	.byte		        Gs3 , v064
	.byte	W12
	.byte		        Ds4 , v072
	.byte	W12
	.byte		        Dn4 , v064
	.byte	W12
	.byte		        Cn4 , v068
	.byte	W12
	.byte		        Bn3 , v064
	.byte	W12
	.byte		        Gs3 , v072
	.byte	W12
	.byte		        Gn3 , v064
	.byte	W12
	.byte		        Fn3 , v068
	.byte	W12
	.byte		        Gn3 , v056
	.byte	W12
	.byte		        Gs3 , v072
	.byte	W12
	.byte		        Cn4 , v060
	.byte	W12
	.byte		        Gs3 , v076
	.byte	W12
	.byte		        Gn3 , v068
	.byte	W12
	.byte		        Fn3 , v076
	.byte	W12
	.byte		        Gn3 , v060
	.byte	W12
	.byte		        Gs3 , v068
	.byte	W12
	.byte		        Bn3 , v076
	.byte	W12
	.byte		        Cn3 , v080
	.byte	W12
	.byte		        Cs3 , v068
	.byte	W12
	.byte		        Fn3 , v076
	.byte	W12
	.byte		        Gn3 , v068
	.byte	W12
	.byte		N05   , Cs2 , v092
	.byte	W24
	.byte		N23   , Gs2 , v088
	.byte	W24
	.byte		N05   , Gn2
	.byte	W48
	.byte		VOL   , 17*mus_qz_637_mvl/mxv
	.byte		N84   , Gs2 , v088 , gtp2
	.byte	W06
	.byte		VOL   , 19*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        21*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        22*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        26*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        28*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        30*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        33*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        35*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        38*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        41*mus_qz_637_mvl/mxv
	.byte	W24
	.byte	W03
	.byte		N02   , Fs2
	.byte	W03
	.byte		        En2 , v072
	.byte	W03
	.byte		        Ds2 , v060
	.byte	W03
	.byte		N05   , Cs3 , v088
	.byte	W24
	.byte		N23   , Gs3
	.byte	W24
	.byte		N05   , Gn3
	.byte	W48
	.byte		VOL   , 13*mus_qz_637_mvl/mxv
	.byte		N80   , An2 , v088 , gtp3
	.byte	W06
	.byte		VOL   , 15*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        17*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        19*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        22*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        26*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        28*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        31*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        34*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        38*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        41*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        44*mus_qz_637_mvl/mxv
	.byte	W18
	.byte		N02   , Gs2
	.byte	W03
	.byte		        Fs2 , v072
	.byte	W03
	.byte		        En2 , v060
	.byte	W03
	.byte		        Ds2 , v052
	.byte	W03
mus_qz_637_2_001:
	.byte		N05   , Cs2 , v100
	.byte	W24
	.byte		N23   , Gs2 , v096
	.byte	W24
	.byte		N05   , Gn2 , v092
	.byte	W48
	.byte	PEND
mus_qz_637_2_002:
	.byte		N11   , Cs2 , v100
	.byte	W12
	.byte		N23   , Gs2 , v096
	.byte	W24
	.byte		N05   , Gn2 , v092
	.byte	W60
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_637_2_001
	.byte	PATT
	 ptrvalue	mus_qz_637_2_002
	.byte	PATT
	 ptrvalue	mus_qz_637_2_B1
	.byte	PATT
	 ptrvalue	mus_qz_637_2_000
	.byte	GOTO
	 ptrvalue	mus_qz_637_2_B1
	.byte	FINE

/********************** Track  3 **********************/

mus_qz_637_3:
	.byte		KEYSH , mus_qz_637_key+0
	.byte		VOICE , 22
	.byte		VOL   , 73*mus_qz_637_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 58
	.byte		MOD   , 1
	.byte		LFOS  , 25
	.byte		VOL   , 66*mus_qz_637_mvl/mxv
	.byte		LFODL , 2
	.byte		N05   , Ds0 , v092
	.byte	W24
	.byte		N23   , As0 , v100
	.byte	W24
	.byte		N05   , An0 , v088
	.byte	W48
mus_qz_637_3_000:
	.byte		N11   , Ds0 , v092
	.byte	W12
	.byte		N23   , As0 , v100
	.byte	W24
	.byte		N05   , An0 , v088
	.byte	W60
	.byte	PEND
mus_qz_637_3_001:
	.byte		N05   , Ds0 , v092
	.byte	W24
	.byte		N23   , As0 , v100
	.byte	W24
	.byte		N05   , An0 , v088
	.byte	W48
	.byte	PEND
	.byte		N11   , Ds0 , v092
	.byte	W12
	.byte		N23   , As0 , v100
	.byte	W24
	.byte		N05   , An0 , v088
	.byte	W48
	.byte		N03   , Ds0 , v076
	.byte	W06
	.byte		        Ds0 , v064
	.byte	W06
	.byte	PATT
	 ptrvalue	mus_qz_637_3_001
	.byte	PATT
	 ptrvalue	mus_qz_637_3_000
	.byte	PATT
	 ptrvalue	mus_qz_637_3_001
	.byte	PATT
	 ptrvalue	mus_qz_637_3_000
	.byte	PATT
	 ptrvalue	mus_qz_637_3_001
	.byte	PATT
	 ptrvalue	mus_qz_637_3_000
	.byte	PATT
	 ptrvalue	mus_qz_637_3_001
	.byte	PATT
	 ptrvalue	mus_qz_637_3_000
mus_qz_637_3_002:
	.byte		N05   , Fn0 , v092
	.byte	W24
	.byte		N23   , Cn1 , v100
	.byte	W24
	.byte		N05   , Bn0 , v088
	.byte	W48
	.byte	PEND
mus_qz_637_3_003:
	.byte		N11   , Fn0 , v092
	.byte	W12
	.byte		N23   , Cn1 , v100
	.byte	W24
	.byte		N05   , Bn0 , v088
	.byte	W60
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_637_3_002
	.byte	PATT
	 ptrvalue	mus_qz_637_3_003
	.byte		N05   , Cs1 , v108
	.byte	W24
	.byte		N23   , Gs1 , v100
	.byte	W24
	.byte		N05   , Gn1
	.byte	W48
	.byte		VOL   , 15*mus_qz_637_mvl/mxv
	.byte		N84   , Cs2 , v088 , gtp2
	.byte	W06
	.byte		VOL   , 19*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        22*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        26*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        30*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        35*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        40*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        44*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        49*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        56*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        60*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        67*mus_qz_637_mvl/mxv
	.byte	W21
	.byte		N02   , Fs1 , v100
	.byte	W03
	.byte		        En1 , v084
	.byte	W03
	.byte		        Ds1 , v072
	.byte	W03
	.byte		N05   , Cs2 , v100
	.byte	W24
	.byte		N23   , Gs2
	.byte	W24
	.byte		N05   , Gn2
	.byte	W48
	.byte		VOL   , 12*mus_qz_637_mvl/mxv
	.byte		N80   , Cs2 , v088 , gtp3
	.byte	W06
	.byte		VOL   , 14*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        18*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        22*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        28*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        31*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        37*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        42*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        48*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        55*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        60*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        67*mus_qz_637_mvl/mxv
	.byte	W18
	.byte		N02   , Gs1 , v100
	.byte	W03
	.byte		        Fs1 , v084
	.byte	W03
	.byte		        En1 , v072
	.byte	W03
	.byte		        Ds1 , v064
	.byte	W03
	.byte		N05   , Cs1 , v108
	.byte	W24
	.byte		N23   , Gs1 , v100
	.byte	W24
	.byte		N05   , Gn1 , v088
	.byte	W48
mus_qz_637_3_004:
	.byte		N11   , Cs1 , v092
	.byte	W12
	.byte		N23   , Gs1 , v100
	.byte	W24
	.byte		N05   , Gn1 , v088
	.byte	W60
	.byte	PEND
	.byte		        Cs1 , v092
	.byte	W24
	.byte		N23   , Gs1 , v100
	.byte	W24
	.byte		N05   , Gn1 , v088
	.byte	W48
	.byte	PATT
	 ptrvalue	mus_qz_637_3_004
	.byte	PATT
	 ptrvalue	mus_qz_637_3_001
	.byte	PATT
	 ptrvalue	mus_qz_637_3_000
	.byte	GOTO
	 ptrvalue	mus_qz_637_3_001
	.byte	FINE

/********************** Track  4 **********************/

mus_qz_637_4:
	.byte		KEYSH , mus_qz_637_key+0
	.byte		VOICE , 39
	.byte		VOL   , 68*mus_qz_637_mvl/mxv
	.byte		PAN   , c_v-14
	.byte		PRIO  , 42
	.byte	W96
	.byte	W96
mus_qz_637_4_B1:
	.byte		N01   , Fs1 , v080
	.byte	W06
	.byte		        Fs1 , v068
	.byte	W06
	.byte		        Fs1 , v084
	.byte	W06
	.byte		        Fs1 , v060
	.byte	W06
	.byte		        Fs1 , v076
	.byte	W06
	.byte		        Fs1 , v064
	.byte	W06
	.byte		N11   , As1 , v100
	.byte	W12
	.byte		N05   , As0 , v092
	.byte	W12
	.byte		        As0 , v080
	.byte		N01   , Fs1 , v072
	.byte	W06
	.byte		        Fs1 , v080
	.byte	W06
	.byte		N11   , As1 , v088
	.byte		N05   , Gs4 , v072
	.byte	W06
	.byte		        Gs4 , v060
	.byte	W06
	.byte		N11   , An4 , v072
	.byte	W12
	.byte	PEND
mus_qz_637_4_000:
	.byte		N01   , Fs1 , v088
	.byte	W06
	.byte	N01
	.byte	W06
	.byte		N11   , As1
	.byte	W24
	.byte		N01   , Fs1 , v080
	.byte	W06
	.byte		        Fs1 , v060
	.byte	W06
	.byte		N05   , As0 , v092
	.byte		N11   , As1 , v088
	.byte		N05   , Gs4 , v072
	.byte	W06
	.byte		        As0 , v060
	.byte		N05   , Gs4
	.byte	W06
	.byte		        As0 , v040
	.byte		N05   , Gs4 , v072
	.byte	W06
	.byte		        Gs4 , v060
	.byte	W06
	.byte		        As0 , v092
	.byte		N05   , Gs4 , v072
	.byte	W06
	.byte		        Gs4 , v060
	.byte	W06
	.byte		N11   , An4 , v072
	.byte	W12
	.byte	PEND
	.byte		N01   , Fs1
	.byte	W06
	.byte		        Fs1 , v080
	.byte	W06
	.byte		N11   , As1 , v088
	.byte	W24
	.byte		N01   , Fs1 , v080
	.byte	W06
	.byte		        Fs1 , v064
	.byte	W06
	.byte		N05   , As0 , v092
	.byte		N11   , As1 , v088
	.byte	W18
	.byte		N05   , As0 , v080
	.byte	W06
	.byte		        Gs4 , v072
	.byte	W06
	.byte		        Gs4 , v060
	.byte	W06
	.byte		N11   , An4 , v072
	.byte	W12
	.byte		N01   , Fs1 , v088
	.byte	W06
	.byte	N01
	.byte	W06
	.byte		N11   , As1
	.byte	W24
	.byte		N01   , Fs1 , v080
	.byte	W06
	.byte		        Fs1 , v064
	.byte	W06
	.byte		N05   , As0 , v092
	.byte		N11   , As1 , v088
	.byte	W06
	.byte		N05   , As0 , v060
	.byte	W06
	.byte		        As0 , v040
	.byte		N05   , Gs4 , v072
	.byte	W06
	.byte		        Gs4 , v060
	.byte	W06
	.byte		N11   , An4 , v072
	.byte	W12
	.byte		N05   , Gs4
	.byte	W06
	.byte		        Gs4 , v060
	.byte	W06
mus_qz_637_4_001:
	.byte		N01   , Fs1 , v072
	.byte	W06
	.byte		        Fs1 , v080
	.byte	W06
	.byte		N11   , As1 , v088
	.byte	W24
	.byte		N01   , Fs1 , v080
	.byte	W06
	.byte		        Fs1 , v064
	.byte	W06
	.byte		N05   , As0 , v092
	.byte		N11   , As1 , v088
	.byte	W12
	.byte		N05   , As0 , v080
	.byte		N05   , Gs4 , v072
	.byte	W06
	.byte		        Gs4 , v060
	.byte	W06
	.byte		N11   , An4 , v072
	.byte	W24
	.byte	PEND
mus_qz_637_4_002:
	.byte		N01   , Fs1 , v088
	.byte	W06
	.byte	N01
	.byte	W06
	.byte		N11   , As1
	.byte	W24
	.byte		N01   , Fs1 , v080
	.byte	W06
	.byte		        Fs1 , v064
	.byte	W06
	.byte		N05   , As0 , v092
	.byte		N11   , As1 , v088
	.byte		N05   , Gs4 , v072
	.byte	W06
	.byte		        As0 , v060
	.byte		N05   , Gs4
	.byte	W06
	.byte		        As0 , v040
	.byte		N05   , Gs4 , v072
	.byte	W06
	.byte		        Gs4 , v060
	.byte	W06
	.byte		        Gs4 , v072
	.byte	W06
	.byte		        As0 , v060
	.byte		N05   , Gs4
	.byte	W06
	.byte		N11   , An4 , v072
	.byte	W12
	.byte	PEND
	.byte		N01   , Fs1
	.byte	W06
	.byte		        Fs1 , v080
	.byte	W06
	.byte		N11   , As1 , v088
	.byte	W24
	.byte		N01   , Fs1 , v080
	.byte	W06
	.byte		        Fs1 , v064
	.byte	W06
	.byte		N05   , As0 , v092
	.byte		N11   , As1 , v088
	.byte	W12
	.byte		N05   , As0 , v080
	.byte	W12
	.byte		        Gs4 , v072
	.byte	W06
	.byte		        Gs4 , v060
	.byte	W06
	.byte		N11   , An4 , v072
	.byte	W12
	.byte		N01   , Fs1 , v088
	.byte	W06
	.byte	N01
	.byte	W06
	.byte		N11   , As1
	.byte	W24
	.byte		N01   , Fs1 , v080
	.byte	W06
	.byte		        Fs1 , v064
	.byte	W06
	.byte		N05   , As0 , v092
	.byte		N11   , As1 , v088
	.byte		N05   , Gs4 , v072
	.byte	W06
	.byte		        As0 , v060
	.byte		N05   , Gs4
	.byte	W06
	.byte		        As0 , v040
	.byte		N11   , An4 , v072
	.byte	W12
	.byte		N05   , As0 , v040
	.byte		N05   , Gs4 , v072
	.byte	W06
	.byte		        Gs4 , v060
	.byte	W06
	.byte		        Gs4 , v072
	.byte	W06
	.byte		        Gs4 , v060
	.byte	W06
	.byte	PATT
	 ptrvalue	mus_qz_637_4_001
	.byte		N01   , Fs1 , v088
	.byte	W06
	.byte	N01
	.byte	W06
	.byte		N11   , As1
	.byte	W24
	.byte		N01   , Fs1 , v080
	.byte	W06
	.byte		        Fs1 , v064
	.byte	W06
	.byte		N05   , As0 , v092
	.byte		N11   , As1 , v088
	.byte		N05   , Gs4 , v072
	.byte	W06
	.byte		        As0 , v060
	.byte		N05   , Gs4
	.byte	W06
	.byte		        As0 , v040
	.byte		N05   , Gs4 , v072
	.byte	W06
	.byte		        As0 , v056
	.byte		N05   , Gs4 , v060
	.byte	W06
	.byte		        Gs4 , v072
	.byte	W06
	.byte		        As0
	.byte		N05   , Gs4 , v060
	.byte	W06
	.byte		N11   , An4 , v072
	.byte	W12
	.byte		N01   , Fs1
	.byte	W06
	.byte		        Fs1 , v080
	.byte	W06
	.byte		N11   , As1 , v088
	.byte	W24
	.byte		N01   , Fs1 , v080
	.byte	W06
	.byte		        Fs1 , v064
	.byte	W06
	.byte		N05   , As0 , v092
	.byte		N11   , As1 , v088
	.byte	W06
	.byte		N05   , As0 , v080
	.byte	W06
	.byte		        Gs4 , v072
	.byte	W06
	.byte		        Gs4 , v060
	.byte	W06
	.byte		        As0 , v040
	.byte		N11   , An4 , v072
	.byte	W24
	.byte		N01   , Fs1 , v088
	.byte	W06
	.byte	N01
	.byte	W06
	.byte		N11   , As1
	.byte	W24
	.byte		N01   , Fs1 , v080
	.byte	W06
	.byte		        Fs1 , v064
	.byte	W06
	.byte		N05   , As0 , v092
	.byte		N11   , As1 , v088
	.byte		N05   , Gs4 , v072
	.byte	W06
	.byte		        As0 , v060
	.byte		N05   , Gs4
	.byte	W06
	.byte		        As0 , v040
	.byte		N05   , Gs4 , v072
	.byte	W06
	.byte		        Gs4 , v060
	.byte	W06
	.byte		        Gs4 , v072
	.byte	W06
	.byte		        Gs4 , v060
	.byte	W06
	.byte		N11   , An4 , v072
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_637_4_001
	.byte	PATT
	 ptrvalue	mus_qz_637_4_002
mus_qz_637_4_003:
	.byte		N01   , Fs1 , v072
	.byte	W06
	.byte		        Fs1 , v080
	.byte	W06
	.byte		N11   , As1 , v088
	.byte	W24
	.byte		N01   , Fs1 , v080
	.byte	W06
	.byte		        Fs1 , v064
	.byte	W06
	.byte		N05   , As0 , v092
	.byte		N11   , As1 , v088
	.byte	W24
	.byte		N05   , Gs4 , v072
	.byte	W06
	.byte		        Gs4 , v060
	.byte	W06
	.byte		N11   , An4 , v072
	.byte	W12
	.byte	PEND
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_637_4_003
	.byte	W48
	.byte		N05   , Gs4 , v072
	.byte	W06
	.byte		        Gs4 , v060
	.byte	W06
	.byte		        Gs4 , v072
	.byte	W06
	.byte		        Gs4 , v060
	.byte	W06
	.byte		        Gs4 , v072
	.byte	W06
	.byte		        Gs4 , v060
	.byte	W06
	.byte		N11   , An4 , v072
	.byte	W12
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_637_4_B1
	.byte	PATT
	 ptrvalue	mus_qz_637_4_000
	.byte	GOTO
	 ptrvalue	mus_qz_637_4_B1
	.byte	FINE

/********************** Track  5 **********************/

mus_qz_637_5:
	.byte		KEYSH , mus_qz_637_key+0
	.byte		VOICE , 39
	.byte		VOL   , 63*mus_qz_637_mvl/mxv
	.byte		PAN   , c_v-24
	.byte		PRIO  , 40
	.byte	W96
	.byte	W96
mus_qz_637_5_B1:
	.byte		N05   , Bn0 , v112
	.byte	W36
	.byte		        Bn0 , v092
	.byte	W48
	.byte		        Bn0 , v096
	.byte	W12
	.byte	PEND
mus_qz_637_5_000:
	.byte		N05   , Bn0 , v112
	.byte	W18
	.byte		        Bn0 , v072
	.byte	W18
	.byte		        Bn0 , v112
	.byte	W48
	.byte		        Bn0 , v088
	.byte	W12
	.byte	PEND
mus_qz_637_5_001:
	.byte		N05   , Bn0 , v112
	.byte	W36
	.byte	N05
	.byte	W48
	.byte		        Bn0 , v096
	.byte	W12
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_637_5_001
	.byte	PATT
	 ptrvalue	mus_qz_637_5_001
	.byte		N05   , Bn0 , v112
	.byte	W17
	.byte		        Bn0 , v072
	.byte	W19
	.byte		        Bn0 , v112
	.byte	W48
	.byte		        Bn0 , v096
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_637_5_001
	.byte	PATT
	 ptrvalue	mus_qz_637_5_001
	.byte	PATT
	 ptrvalue	mus_qz_637_5_001
mus_qz_637_5_002:
	.byte		N05   , Bn0 , v112
	.byte	W18
	.byte		        Bn0 , v072
	.byte	W18
	.byte		        Bn0 , v112
	.byte	W48
	.byte		        Bn0 , v096
	.byte	W12
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_637_5_001
	.byte	PATT
	 ptrvalue	mus_qz_637_5_001
	.byte	PATT
	 ptrvalue	mus_qz_637_5_001
	.byte	PATT
	 ptrvalue	mus_qz_637_5_002
	.byte		N05   , Bn0 , v112
	.byte	W36
	.byte	N05
	.byte	W48
	.byte		        Bn0 , v092
	.byte	W12
	.byte		        Bn0 , v112
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_637_5_001
	.byte		N05   , Bn0 , v112
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_637_5_B1
	.byte	PATT
	 ptrvalue	mus_qz_637_5_000
	.byte	GOTO
	 ptrvalue	mus_qz_637_5_B1
	.byte	FINE

/********************** Track  6 **********************/

mus_qz_637_6:
	.byte		KEYSH , mus_qz_637_key+0
	.byte		VOICE , 23
	.byte		VOL   , 68*mus_qz_637_mvl/mxv
	.byte		PAN   , c_v-54
	.byte		PRIO  , 52
	.byte		MOD   , 3
	.byte		LFOS  , 25
	.byte		LFODL , 11
	.byte		VOL   , 53*mus_qz_637_mvl/mxv
	.byte		N05   , Ds2 , v088
	.byte	W24
	.byte		N23   , As2
	.byte	W24
	.byte		N05   , An2
	.byte	W48
mus_qz_637_6_000:
	.byte		N11   , Ds2 , v088
	.byte	W12
	.byte		N23   , As2
	.byte	W24
	.byte		N05   , An2
	.byte	W60
	.byte	PEND
mus_qz_637_6_001:
	.byte		N05   , Ds2 , v088
	.byte	W24
	.byte		N23   , As2
	.byte	W24
	.byte		N05   , An2
	.byte	W48
	.byte	PEND
	.byte		N11   , Ds2
	.byte	W12
	.byte		N23   , As2
	.byte	W24
	.byte		N05   , An2
	.byte	W12
	.byte		N04   , As2 , v100
	.byte	W06
	.byte		        Bn2 , v084
	.byte	W06
	.byte		        As2 , v088
	.byte	W06
	.byte		        Bn2 , v080
	.byte	W06
	.byte		        As2 , v088
	.byte	W06
	.byte		        Gs2 , v100
	.byte	W06
	.byte		        Fs2 , v088
	.byte	W06
	.byte		        Fn2 , v100
	.byte	W06
	.byte	PATT
	 ptrvalue	mus_qz_637_6_001
	.byte	PATT
	 ptrvalue	mus_qz_637_6_000
	.byte	PATT
	 ptrvalue	mus_qz_637_6_001
	.byte	PATT
	 ptrvalue	mus_qz_637_6_000
mus_qz_637_6_002:
	.byte		N05   , Ds1 , v092
	.byte	W24
	.byte		N23   , As1 , v100
	.byte	W24
	.byte		N05   , An1 , v088
	.byte	W48
	.byte	PEND
mus_qz_637_6_003:
	.byte		N11   , Ds1 , v092
	.byte	W12
	.byte		N23   , As1 , v100
	.byte	W24
	.byte		N05   , An1 , v088
	.byte	W60
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_637_6_002
	.byte	PATT
	 ptrvalue	mus_qz_637_6_003
mus_qz_637_6_004:
	.byte		N05   , Fn1 , v092
	.byte	W24
	.byte		N23   , Cn2 , v100
	.byte	W24
	.byte		N05   , Bn1 , v088
	.byte	W48
	.byte	PEND
mus_qz_637_6_005:
	.byte		N11   , Fn1 , v092
	.byte	W12
	.byte		N23   , Cn2 , v100
	.byte	W24
	.byte		N05   , Bn1 , v088
	.byte	W60
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_637_6_004
	.byte	PATT
	 ptrvalue	mus_qz_637_6_005
	.byte		N05   , Cs2 , v092
	.byte	W24
	.byte		N23   , Gs2 , v088
	.byte	W24
	.byte		N05   , Gn2
	.byte	W48
	.byte		VOL   , 12*mus_qz_637_mvl/mxv
	.byte		N88   , Cs3 , v092 , gtp1
	.byte	W06
	.byte		VOL   , 14*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        17*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        21*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        24*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        28*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        31*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        35*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        40*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        44*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        49*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        57*mus_qz_637_mvl/mxv
	.byte	W30
	.byte	N05
	.byte	W24
	.byte		N23   , Gs3
	.byte	W24
	.byte		N05   , Gn3
	.byte	W48
	.byte		VOL   , 13*mus_qz_637_mvl/mxv
	.byte		N88   , Cs3 , v092 , gtp1
	.byte	W06
	.byte		VOL   , 15*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        19*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        22*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        28*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        31*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        35*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        40*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        44*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        49*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        55*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        60*mus_qz_637_mvl/mxv
	.byte	W30
	.byte		        53*mus_qz_637_mvl/mxv
	.byte		N05   , Cs2
	.byte	W24
	.byte		N23   , Gs2 , v100
	.byte	W24
	.byte		N05   , Gn2 , v088
	.byte	W48
mus_qz_637_6_006:
	.byte		N11   , Cs2 , v092
	.byte	W12
	.byte		N23   , Gs2 , v100
	.byte	W24
	.byte		N05   , Gn2 , v088
	.byte	W60
	.byte	PEND
	.byte		        Cs2 , v092
	.byte	W24
	.byte		N23   , Gs2 , v100
	.byte	W24
	.byte		N05   , Gn2 , v088
	.byte	W48
	.byte	PATT
	 ptrvalue	mus_qz_637_6_006
	.byte	PATT
	 ptrvalue	mus_qz_637_6_001
	.byte	PATT
	 ptrvalue	mus_qz_637_6_000
	.byte	GOTO
	 ptrvalue	mus_qz_637_6_001
	.byte	FINE

/********************** Track  7 **********************/

mus_qz_637_7:
	.byte		KEYSH , mus_qz_637_key+0
	.byte		VOICE , 25
	.byte		VOL   , 73*mus_qz_637_mvl/mxv
	.byte		PAN   , c_v+51
	.byte		PRIO  , 54
	.byte		VOL   , 66*mus_qz_637_mvl/mxv
	.byte		N05   , Ds1 , v088
	.byte	W24
	.byte		N23   , As1
	.byte	W24
	.byte		N05   , An1
	.byte	W48
mus_qz_637_7_000:
	.byte		N11   , Ds1 , v088
	.byte	W12
	.byte		N23   , As1
	.byte	W24
	.byte		N05   , An1
	.byte	W60
	.byte	PEND
mus_qz_637_7_001:
	.byte		N05   , Ds1 , v088
	.byte	W24
	.byte		N23   , As1
	.byte	W24
	.byte		N05   , An1
	.byte	W48
	.byte	PEND
	.byte		N11   , Ds1
	.byte	W12
	.byte		N23   , As1
	.byte	W24
	.byte		N05   , An1
	.byte	W12
	.byte		N04   , As1
	.byte	W06
	.byte		        Bn1 , v072
	.byte	W06
	.byte		        As1 , v076
	.byte	W06
	.byte		        Bn1 , v068
	.byte	W06
	.byte		        As1 , v076
	.byte	W06
	.byte		        Gs1 , v088
	.byte	W06
	.byte		        Fs1 , v076
	.byte	W06
	.byte		        Fn1 , v088
	.byte	W06
	.byte	PATT
	 ptrvalue	mus_qz_637_7_001
	.byte	PATT
	 ptrvalue	mus_qz_637_7_000
	.byte	PATT
	 ptrvalue	mus_qz_637_7_001
	.byte	PATT
	 ptrvalue	mus_qz_637_7_000
mus_qz_637_7_002:
	.byte		N05   , Ds1 , v092
	.byte	W24
	.byte		N23   , As1 , v100
	.byte	W24
	.byte		N05   , An1 , v088
	.byte	W48
	.byte	PEND
mus_qz_637_7_003:
	.byte		N11   , Ds1 , v092
	.byte	W12
	.byte		N23   , As1 , v100
	.byte	W24
	.byte		N05   , An1 , v088
	.byte	W60
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_637_7_002
	.byte	PATT
	 ptrvalue	mus_qz_637_7_003
mus_qz_637_7_004:
	.byte		N05   , Fn1 , v092
	.byte	W24
	.byte		N23   , Cn2 , v100
	.byte	W24
	.byte		N05   , Bn1 , v088
	.byte	W48
	.byte	PEND
mus_qz_637_7_005:
	.byte		N11   , Fn1 , v092
	.byte	W12
	.byte		N23   , Cn2 , v100
	.byte	W24
	.byte		N05   , Bn1 , v088
	.byte	W60
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_637_7_004
	.byte	PATT
	 ptrvalue	mus_qz_637_7_005
	.byte		N05   , Cs1 , v092
	.byte	W24
	.byte		N23   , Gs1 , v088
	.byte	W24
	.byte		N05   , Gn1
	.byte	W48
	.byte		VOL   , 10*mus_qz_637_mvl/mxv
	.byte		N88   , Cs1 , v072 , gtp1
	.byte	W06
	.byte		VOL   , 12*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        16*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        19*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        24*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        28*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        33*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        38*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        44*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        49*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        56*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        66*mus_qz_637_mvl/mxv
	.byte	W30
	.byte		N05   , Cs1 , v088
	.byte	W24
	.byte		N23   , Gs1
	.byte	W24
	.byte		N05   , Gn1
	.byte	W48
	.byte		VOL   , 12*mus_qz_637_mvl/mxv
	.byte		N88   , Cs1 , v088 , gtp1
	.byte	W06
	.byte		VOL   , 15*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        19*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        23*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        28*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        33*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        38*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        43*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        49*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        55*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        60*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        66*mus_qz_637_mvl/mxv
	.byte	W30
mus_qz_637_7_006:
	.byte		N05   , Cs1 , v092
	.byte	W24
	.byte		N23   , Gs1 , v100
	.byte	W24
	.byte		N05   , Gn1 , v088
	.byte	W48
	.byte	PEND
mus_qz_637_7_007:
	.byte		N11   , Cs1 , v092
	.byte	W12
	.byte		N23   , Gs1 , v100
	.byte	W24
	.byte		N05   , Gn1 , v088
	.byte	W60
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_637_7_006
	.byte	PATT
	 ptrvalue	mus_qz_637_7_007
	.byte	PATT
	 ptrvalue	mus_qz_637_7_001
	.byte	PATT
	 ptrvalue	mus_qz_637_7_000
	.byte	GOTO
	 ptrvalue	mus_qz_637_7_001
	.byte	FINE

/********************** Track  8 **********************/

mus_qz_637_8:
	.byte		KEYSH , mus_qz_637_key+0
	.byte		VOICE , 10
	.byte		VOL   , 19*mus_qz_637_mvl/mxv
	.byte		PAN   , c_v+56
	.byte		PRIO  , 38
	.byte		MOD   , 17
	.byte		LFOS  , 25
	.byte		LFODL , 1
	.byte	W96
	.byte	W96
mus_qz_637_8_B1:
	.byte	W96
	.byte	W96
	.byte	W12
	.byte		N64   , Ds4 , v092 , gtp1
	.byte	W72
	.byte		N11   , As4 , v080
	.byte	W12
	.byte		        An4 , v072
	.byte	W12
	.byte		N44   , Cs5 , v096 , gtp3
	.byte	W48
	.byte		        Cn5 , v084 , gtp3
	.byte	W36
	.byte	W12
	.byte		N52   , As4 , v096 , gtp1
	.byte	W72
	.byte		N23   , An4 , v084
	.byte	W12
	.byte	W12
	.byte		N40   , Fs4 , v096 , gtp1
	.byte	W48
	.byte		        Fn4 , v084 , gtp1
	.byte	W36
	.byte	W12
	.byte		N64   , Ds5 , v096 , gtp1
	.byte	W72
	.byte		N11   , As5 , v088
	.byte	W12
	.byte		        An5 , v080
	.byte	W12
	.byte		N44   , Fs5 , v092 , gtp3
	.byte	W48
	.byte		        Fn5 , v084 , gtp3
	.byte	W36
	.byte	W12
	.byte		N52   , Ds5 , v100 , gtp1
	.byte	W72
	.byte		N23   , Dn5 , v092
	.byte	W12
	.byte	W12
	.byte		N40   , Bn4 , v096 , gtp1
	.byte	W48
	.byte		        As4 , v088 , gtp1
	.byte	W36
	.byte	W12
	.byte		N64   , Fn5 , v100 , gtp1
	.byte	W72
	.byte		N11   , Cs6 , v092
	.byte	W12
	.byte		        Cn6 , v080
	.byte	W12
	.byte		N44   , Gs5 , v092 , gtp2
	.byte	W48
	.byte		        Gn5 , v080 , gtp2
	.byte	W36
	.byte	W12
	.byte		N52   , Fn5 , v092 , gtp1
	.byte	W72
	.byte		N23   , En5 , v080
	.byte	W12
	.byte	W12
	.byte		N40   , Cs5 , v088 , gtp1
	.byte	W48
	.byte		        Cn5 , v084 , gtp1
	.byte	W36
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
	 ptrvalue	mus_qz_637_8_B1
	.byte	FINE

/********************** Track  9 **********************/

mus_qz_637_9:
	.byte		KEYSH , mus_qz_637_key+0
	.byte		VOICE , 27
	.byte		VOL   , 41*mus_qz_637_mvl/mxv
	.byte		PAN   , c_v-32
	.byte		PRIO  , 54
	.byte	W96
	.byte	W96
mus_qz_637_9_B1:
	.byte		N14   , Ds4 , v092
	.byte	W12
	.byte		        Fn4 , v076
	.byte	W12
	.byte		        Fs4 , v088
	.byte	W12
	.byte		        As4 , v100
	.byte	W12
	.byte		        As3 , v088
	.byte	W12
	.byte		        Fn4 , v072
	.byte	W12
	.byte		        Fs4 , v076
	.byte	W12
	.byte		        As4 , v096
	.byte	W12
	.byte		        Ds4 , v092
	.byte	W12
	.byte		        Fn4 , v076
	.byte	W12
	.byte		        Fs4 , v088
	.byte	W12
	.byte		N11   , As4 , v096
	.byte	W12
	.byte		N05   , As3 , v092
	.byte	W06
	.byte		        Bn3 , v080
	.byte	W06
	.byte		        As3 , v084
	.byte	W06
	.byte		        Bn3 , v076
	.byte	W06
	.byte		        As3 , v084
	.byte	W06
	.byte		        Gs3 , v092
	.byte	W06
	.byte		        Fs3 , v084
	.byte	W06
	.byte		        Fn3 , v092
	.byte	W06
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
mus_qz_637_9_000:
	.byte		N11   , Ds4 , v092
	.byte	W12
	.byte		        Fn4 , v076
	.byte	W12
	.byte		        Fs4 , v088
	.byte	W12
	.byte		N32   , As4 , v100 , gtp3
	.byte	W60
	.byte	PEND
	.byte		N11   , As3 , v088
	.byte	W12
	.byte		        Fn4 , v072
	.byte	W12
	.byte		        Fs4 , v076
	.byte	W12
	.byte		N32   , As4 , v096 , gtp3
	.byte	W60
	.byte	PATT
	 ptrvalue	mus_qz_637_9_000
	.byte		N11   , As3 , v088
	.byte	W12
	.byte		        Fn4 , v072
	.byte	W12
	.byte		        Fs4 , v076
	.byte	W12
	.byte		        As4 , v096
	.byte	W12
	.byte		N05   , As3 , v088
	.byte	W06
	.byte		        Bn3 , v072
	.byte	W06
	.byte		        As3 , v076
	.byte	W06
	.byte		        Bn3 , v068
	.byte	W06
	.byte		        As3 , v076
	.byte	W06
	.byte		        Gs3 , v088
	.byte	W06
	.byte		        Fs3 , v076
	.byte	W06
	.byte		        Fn3 , v088
	.byte	W06
mus_qz_637_9_001:
	.byte		N11   , Fn4 , v092
	.byte	W12
	.byte		        Gn4 , v076
	.byte	W12
	.byte		        Gs4 , v088
	.byte	W12
	.byte		        Cn5 , v100
	.byte	W12
	.byte		        Cn4 , v088
	.byte	W12
	.byte		        Gn4 , v072
	.byte	W12
	.byte		        Gs4 , v076
	.byte	W12
	.byte		        Cn5 , v092
	.byte	W12
	.byte	PEND
	.byte		        Fn4 , v096
	.byte	W12
	.byte		        Gn4 , v080
	.byte	W12
	.byte		        Gs4 , v088
	.byte	W12
	.byte		        Cn5 , v100
	.byte	W12
	.byte		        Cn4 , v088
	.byte	W12
	.byte		        Gn4 , v072
	.byte	W12
	.byte		        Gs4 , v080
	.byte	W12
	.byte		        Cn5 , v096
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_637_9_001
	.byte		N11   , Fn4 , v088
	.byte	W12
	.byte		        Gn4 , v072
	.byte	W12
	.byte		        Gs4 , v076
	.byte	W12
	.byte		        Cn5 , v092
	.byte	W12
	.byte		N05   , Cn4 , v088
	.byte	W06
	.byte		        Cs4 , v072
	.byte	W06
	.byte		        Cn4 , v076
	.byte	W06
	.byte		        Cs4 , v068
	.byte	W06
	.byte		        Cn4 , v076
	.byte	W06
	.byte		        As3 , v088
	.byte	W06
	.byte		        Gs3 , v076
	.byte	W06
	.byte		        Gn3 , v088
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
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_637_9_B1
	.byte	FINE

/********************** Track 10 **********************/

mus_qz_637_10:
	.byte		KEYSH , mus_qz_637_key+0
	.byte		VOICE , 27
	.byte		VOL   , 14*mus_qz_637_mvl/mxv
	.byte		PAN   , c_v+61
	.byte		PRIO  , 38
	.byte	W96
	.byte	W96
mus_qz_637_10_B1:
	.byte	W12
	.byte		N14   , Ds4 , v092
	.byte	W12
	.byte		        Fn4 , v076
	.byte	W12
	.byte		        Fs4 , v088
	.byte	W12
	.byte		        As4 , v100
	.byte	W12
	.byte		        As3 , v088
	.byte	W12
	.byte		        Fn4 , v072
	.byte	W12
	.byte		        Fs4 , v076
	.byte	W12
	.byte		        As4 , v096
	.byte	W12
	.byte		        Ds4 , v092
	.byte	W12
	.byte		        Fn4 , v076
	.byte	W12
	.byte		        Fs4 , v088
	.byte	W12
	.byte		N11   , As4 , v096
	.byte	W48
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
	 ptrvalue	mus_qz_637_10_B1
	.byte	FINE

/********************** Track 11 **********************/

mus_qz_637_11:
	.byte		KEYSH , mus_qz_637_key+0
	.byte		VOICE , 23
	.byte		VOL   , 14*mus_qz_637_mvl/mxv
	.byte		PAN   , c_v+56
	.byte		PRIO  , 34
	.byte		MOD   , 3
	.byte		LFOS  , 25
	.byte		LFODL , 11
	.byte		VOL   , 12*mus_qz_637_mvl/mxv
	.byte	W12
	.byte		N05   , Ds2 , v088
	.byte	W24
	.byte		N23   , As2
	.byte	W24
	.byte		N05   , An2
	.byte	W36
mus_qz_637_11_000:
	.byte	W12
	.byte		N11   , Ds2 , v088
	.byte	W12
	.byte		N23   , As2
	.byte	W24
	.byte		N05   , An2
	.byte	W48
	.byte	PEND
mus_qz_637_11_B1:
	.byte	W12
	.byte		N05   , Ds2 , v088
	.byte	W24
	.byte		N23   , As2
	.byte	W24
	.byte		N05   , An2
	.byte	W36
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_637_11_000
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
	.byte	W12
	.byte		VOL   , 2*mus_qz_637_mvl/mxv
	.byte		N88   , Cs3 , v092 , gtp1
	.byte	W06
	.byte		VOL   , 3*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        3*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        4*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        5*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        5*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        7*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        7*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        8*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        9*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        10*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        12*mus_qz_637_mvl/mxv
	.byte	W18
	.byte	W12
	.byte	N05
	.byte	W24
	.byte		N23   , Gs3
	.byte	W24
	.byte		N05   , Gn3
	.byte	W36
	.byte	W12
	.byte		VOL   , 2*mus_qz_637_mvl/mxv
	.byte		N88   , Cs3 , v092 , gtp1
	.byte	W06
	.byte		VOL   , 3*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        3*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        4*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        5*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        6*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        7*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        7*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        8*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        10*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        10*mus_qz_637_mvl/mxv
	.byte	W06
	.byte		        12*mus_qz_637_mvl/mxv
	.byte	W18
mus_qz_637_11_001:
	.byte	W12
	.byte		N05   , Cs2 , v092
	.byte	W24
	.byte		N23   , Gs2 , v100
	.byte	W24
	.byte		N05   , Gn2 , v088
	.byte	W36
	.byte	PEND
mus_qz_637_11_002:
	.byte	W12
	.byte		N11   , Cs2 , v092
	.byte	W12
	.byte		N23   , Gs2 , v100
	.byte	W24
	.byte		N05   , Gn2 , v088
	.byte	W48
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_637_11_001
	.byte	PATT
	 ptrvalue	mus_qz_637_11_002
	.byte	PATT
	 ptrvalue	mus_qz_637_11_B1
	.byte	PATT
	 ptrvalue	mus_qz_637_11_000
	.byte	GOTO
	 ptrvalue	mus_qz_637_11_B1
	.byte	FINE

/******************************************************/
	.p2align	2

mus_qz_637:
	.byte	11	/* NumTrks */
	.byte	0	/* NumBlks */
	.byte	mus_qz_637_pri	/* Priority */
	.byte	mus_qz_637_rev	/* Reverb. */
space64 4
	ptrvalue	mus_qz_637_grp

	ptrvalue	mus_qz_637_1
	ptrvalue	mus_qz_637_2
	ptrvalue	mus_qz_637_3
	ptrvalue	mus_qz_637_4
	ptrvalue	mus_qz_637_5
	ptrvalue	mus_qz_637_6
	ptrvalue	mus_qz_637_7
	ptrvalue	mus_qz_637_8
	ptrvalue	mus_qz_637_9
	ptrvalue	mus_qz_637_10
	ptrvalue	mus_qz_637_11

	.end
