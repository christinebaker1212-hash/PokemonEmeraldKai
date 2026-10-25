	.include "asm/macros/bit_width.inc"

	.include "MPlayDef.s"

/* Moemon Quetzal song 664, disassembled from the ROM by tools/quetzal_rip/songs.py.
   "org" comments give the ROM address of the bytes that follow. */

	.equ	mus_qz_664_grp, voicegroup_qz_songs
	.equ	mus_qz_664_pri, 0
	.equ	mus_qz_664_rev, reverb_set+0
	.equ	mus_qz_664_mvl, 127
	.equ	mus_qz_664_key, 0
	.equ	mus_qz_664_tbs, 1
	.equ	mus_qz_664_exg, 1
	.equ	mus_qz_664_cmp, 1

	.section .rodata
	.global	mus_qz_664
	.p2align	2
/* org 0x096A2A44 */

/********************** Track  1 **********************/

mus_qz_664_1:
	.byte		KEYSH , mus_qz_664_key+0
	.byte		TEMPO , 144*mus_qz_664_tbs/2
	.byte		VOICE , 11
	.byte		VOL   , 73*mus_qz_664_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte		MOD   , 10
	.byte		LFOS  , 23
	.byte		MODT  , mod_pan
	.byte		LFODL , 12
	.byte		N10   , Bn3 , v088
	.byte	W12
	.byte		        As3 , v076
	.byte	W12
	.byte		N44   , Bn3 , v088 , gtp2
	.byte	W48
	.byte		N10   , Ds4
	.byte	W12
	.byte		        Dn4 , v080
	.byte	W12
	.byte		N01   , Dn2 , v020
	.byte		N10   , Cs4 , v088
	.byte	W02
	.byte		N01   , En2 , v024
	.byte	W02
	.byte		        Fn2 , v020
	.byte	W02
	.byte		        Gn2 , v036
	.byte	W02
	.byte		        As2 , v032
	.byte	W02
	.byte		        Bn2 , v044
	.byte	W02
	.byte		        Cn3
	.byte		N10   , Cn4 , v076
	.byte	W02
	.byte		N01   , Dn3 , v056
	.byte	W02
	.byte		        En3 , v048
	.byte	W02
	.byte		        Fn3 , v060
	.byte	W02
	.byte		        Gn3 , v056
	.byte	W02
	.byte		        An3 , v068
	.byte	W02
	.byte		N04   , Bn3 , v088
	.byte	W36
	.byte		        Bn3 , v084
	.byte	W36
	.byte		        Bn3 , v088
	.byte	W24
	.byte		        Bn3 , v084
	.byte	W48
	.byte		N10   , Gn3 , v088
	.byte	W12
	.byte		        Gs3 , v076
	.byte	W12
	.byte		        An3 , v088
	.byte	W12
	.byte		        As3 , v080
	.byte	W12
	.byte		N04   , Bn3 , v088
	.byte	W36
	.byte		        Bn3 , v080
	.byte	W36
	.byte		        Bn3 , v084
	.byte	W24
	.byte		        Bn3 , v088
	.byte	W48
	.byte		N10   , Ds4
	.byte	W12
	.byte		        Dn4 , v080
	.byte	W12
	.byte		        Cs4 , v084
	.byte	W12
	.byte		        Cn4 , v076
	.byte	W12
mus_qz_664_1_B1:
	.byte		N04   , Bn3 , v088
	.byte	W36
	.byte		        Bn3 , v080
	.byte	W36
	.byte		        Bn3 , v084
	.byte	W24
	.byte		        Bn3 , v088
	.byte	W48
	.byte		N10   , Gn3 , v080
	.byte	W12
	.byte		        Gs3 , v076
	.byte	W12
	.byte		        An3 , v088
	.byte	W12
	.byte		        As3 , v080
	.byte	W12
	.byte		N05   , Bn3 , v088
	.byte	W36
	.byte		N04   , Bn3 , v080
	.byte	W36
	.byte		        Bn3 , v088
	.byte	W24
	.byte	N04
	.byte	W48
	.byte		N10   , Ds4
	.byte	W12
	.byte		        Dn4 , v080
	.byte	W12
	.byte		        Cs4 , v088
	.byte	W12
	.byte		        Cn4 , v084
	.byte	W12
	.byte		N04   , Bn3 , v092
	.byte	W36
	.byte		        Bn3 , v084
	.byte	W36
	.byte		        Bn3 , v088
	.byte	W24
	.byte		        Bn3 , v092
	.byte	W48
	.byte		N10   , Gn3 , v084
	.byte	W12
	.byte		        Gs3 , v076
	.byte	W12
	.byte		        An3 , v088
	.byte	W12
	.byte		        As3 , v084
	.byte	W12
	.byte		N05   , Bn3 , v092
	.byte	W36
	.byte		N04   , Bn3 , v080
	.byte	W36
	.byte		        Bn3 , v084
	.byte	W24
	.byte		        Bn3 , v092
	.byte	W48
	.byte		N11   , Ds4 , v084
	.byte	W12
	.byte		        Dn4 , v076
	.byte	W12
	.byte		        Cs4 , v088
	.byte	W12
	.byte		        Cn4 , v080
	.byte	W12
	.byte	GOTO
	 ptrvalue	mus_qz_664_1_B1
	.byte	FINE

/********************** Track  2 **********************/

mus_qz_664_2:
	.byte		KEYSH , mus_qz_664_key+0
	.byte		VOICE , 22
	.byte		VOL   , 67*mus_qz_664_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 62
	.byte		MOD   , 5
	.byte		LFOS  , 23
	.byte	W24
	.byte		N04   , Bn0 , v100
	.byte	W06
	.byte		        Bn0 , v072
	.byte	W30
	.byte		        Fs1 , v100
	.byte	W06
	.byte		        Fs1 , v072
	.byte	W30
	.byte		        Bn0 , v100
	.byte	W06
	.byte		        Bn0 , v072
	.byte	W06
	.byte		        Fs1 , v100
	.byte	W06
	.byte		        Dn1 , v072
	.byte	W06
mus_qz_664_2_000:
	.byte		N04   , Bn0 , v100
	.byte	W06
	.byte		        Bn0 , v088
	.byte	W12
	.byte		        Dn1 , v100
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W12
	.byte		        Fs1 , v100
	.byte	W06
	.byte		        Fs1 , v088
	.byte	W12
	.byte		        As1 , v100
	.byte	W06
	.byte		        As1 , v088
	.byte	W12
	.byte		        Bn0 , v100
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte		        Fs1 , v100
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W06
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_664_2_000
	.byte	PATT
	 ptrvalue	mus_qz_664_2_000
	.byte	PATT
	 ptrvalue	mus_qz_664_2_000
mus_qz_664_2_B1:
	.byte	PATT
	 ptrvalue	mus_qz_664_2_000
	.byte	PATT
	 ptrvalue	mus_qz_664_2_000
	.byte	PATT
	 ptrvalue	mus_qz_664_2_000
	.byte	PATT
	 ptrvalue	mus_qz_664_2_000
	.byte	PATT
	 ptrvalue	mus_qz_664_2_000
	.byte	PATT
	 ptrvalue	mus_qz_664_2_000
	.byte	PATT
	 ptrvalue	mus_qz_664_2_000
	.byte	PATT
	 ptrvalue	mus_qz_664_2_000
	.byte	GOTO
	 ptrvalue	mus_qz_664_2_B1
	.byte	FINE

/********************** Track  3 **********************/

mus_qz_664_3:
	.byte		KEYSH , mus_qz_664_key+0
	.byte		VOICE , 39
	.byte		VOL   , 52*mus_qz_664_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 40
	.byte	W24
	.byte		N05   , Cn1 , v100
	.byte		N03   , Fs1 , v080
	.byte	W06
	.byte		N05   , Cn1 , v100
	.byte		N03   , Fs1 , v052
	.byte	W30
	.byte		N05   , Cn1 , v100
	.byte		N03   , Fs1 , v080
	.byte	W06
	.byte		N05   , Cn1 , v100
	.byte		N03   , Fs1 , v052
	.byte	W30
	.byte		N05   , Cn1 , v100
	.byte		N03   , Fs1 , v080
	.byte	W06
	.byte		N05   , Cn1 , v100
	.byte		N03   , Fs1 , v052
	.byte	W06
	.byte		N05   , Cn1 , v100
	.byte		N01   , Fs1 , v064
	.byte	W03
	.byte		        Fs1 , v044
	.byte	W03
	.byte		N05   , Cn1 , v100
	.byte		N05   , As1 , v072
	.byte	W06
	.byte		        Cn1 , v100
	.byte		N03   , Fs1 , v080
	.byte	W06
	.byte		        Fs1 , v052
	.byte	W18
	.byte		N05   , As0 , v104
	.byte	W12
	.byte		N03   , Fs1 , v080
	.byte	W06
	.byte		N05   , Cn1 , v072
	.byte		N03   , Fs1 , v052
	.byte	W06
	.byte		N05   , Cn1 , v100
	.byte	W24
	.byte		        As0 , v104
	.byte		N03   , Fs1 , v080
	.byte	W06
	.byte		        Fs1 , v052
	.byte	W06
	.byte		N05   , Cn1 , v100
	.byte		N11   , As1 , v072
	.byte	W06
	.byte		N05   , Cn1 , v064
	.byte	W06
	.byte		        Cn1 , v100
	.byte		N03   , Fs1 , v080
	.byte	W06
	.byte		        Fs1 , v052
	.byte	W18
	.byte		N05   , As0 , v104
	.byte	W12
	.byte		N03   , Fs1 , v080
	.byte	W06
	.byte		N05   , Cn1 , v100
	.byte		N03   , Fs1 , v052
	.byte	W06
	.byte		N05   , Cn1 , v100
	.byte	W24
	.byte		        As0 , v104
	.byte		N03   , Fs1 , v080
	.byte	W06
	.byte		        Fs1 , v052
	.byte	W06
	.byte		N05   , Cn1 , v100
	.byte		N11   , As1 , v072
	.byte	W06
	.byte		N05   , Cn1 , v060
	.byte	W06
	.byte		        Cn1 , v100
	.byte		N03   , Fs1 , v080
	.byte	W06
	.byte		        Fs1 , v052
	.byte	W18
	.byte		N05   , As0 , v104
	.byte	W12
	.byte		N03   , Fs1 , v080
	.byte	W06
	.byte		N05   , Cn1 , v072
	.byte		N03   , Fs1 , v052
	.byte	W06
	.byte		N05   , Cn1 , v100
	.byte	W06
	.byte		        As0 , v076
	.byte	W18
	.byte		        As0 , v104
	.byte		N03   , Fs1 , v080
	.byte	W06
	.byte		        Fs1 , v052
	.byte	W06
	.byte		N05   , Cn1 , v100
	.byte		N11   , As1 , v072
	.byte	W06
	.byte		N05   , Cn1 , v064
	.byte	W06
	.byte		        Cn1 , v100
	.byte		N03   , Fs1 , v080
	.byte	W06
	.byte		        Fs1 , v052
	.byte	W18
	.byte		N05   , As0 , v104
	.byte	W12
	.byte		N03   , Fs1 , v080
	.byte	W06
	.byte		N05   , Cn1 , v100
	.byte		N03   , Fs1 , v052
	.byte	W06
	.byte		N05   , Cn1 , v100
	.byte	W06
	.byte		        As0 , v076
	.byte	W18
	.byte		        As0 , v104
	.byte		N03   , Fs1 , v080
	.byte	W06
	.byte		N05   , As0 , v064
	.byte		N03   , Fs1 , v052
	.byte	W06
	.byte		N05   , As0 , v104
	.byte		N11   , As1 , v072
	.byte	W06
	.byte		N05   , As0 , v064
	.byte	W06
mus_qz_664_3_B1:
	.byte		N05   , Cn1 , v100
	.byte		N28   , An2 , v096 , gtp1
	.byte	W24
	.byte		N05   , As0 , v104
	.byte	W12
	.byte		N03   , Fs1 , v080
	.byte	W06
	.byte		N05   , Cn1 , v072
	.byte		N03   , Fs1 , v052
	.byte	W06
	.byte		N05   , Cn1 , v100
	.byte	W06
	.byte		        As0 , v056
	.byte	W18
	.byte		        As0 , v104
	.byte		N03   , Fs1 , v080
	.byte	W06
	.byte		        Fs1 , v052
	.byte	W06
	.byte		N05   , Cn1 , v100
	.byte		N11   , As1 , v072
	.byte	W06
	.byte		N05   , Cn1 , v064
	.byte	W06
mus_qz_664_3_000:
	.byte		N05   , Cn1 , v100
	.byte		N03   , Fs1 , v080
	.byte	W06
	.byte		        Fs1 , v052
	.byte	W18
	.byte		N05   , As0 , v104
	.byte	W12
	.byte		N03   , Fs1 , v080
	.byte	W06
	.byte		N05   , Cn1 , v100
	.byte		N03   , Fs1 , v052
	.byte	W06
	.byte		N05   , Cn1 , v100
	.byte	W06
	.byte		        As0 , v056
	.byte	W18
	.byte		        As0 , v104
	.byte		N03   , Fs1 , v080
	.byte	W06
	.byte		        Fs1 , v052
	.byte	W06
	.byte		N05   , Cn1 , v100
	.byte		N11   , As1 , v072
	.byte	W06
	.byte		N05   , Cn1 , v060
	.byte	W06
	.byte	PEND
mus_qz_664_3_001:
	.byte		N05   , Cn1 , v100
	.byte		N03   , Fs1 , v080
	.byte	W06
	.byte		        Fs1 , v052
	.byte	W18
	.byte		N05   , As0 , v104
	.byte	W12
	.byte		N03   , Fs1 , v080
	.byte	W06
	.byte		N05   , Cn1 , v072
	.byte		N03   , Fs1 , v052
	.byte	W06
	.byte		N05   , Cn1 , v100
	.byte	W06
	.byte		        As0 , v056
	.byte	W18
	.byte		        As0 , v104
	.byte		N03   , Fs1 , v080
	.byte	W06
	.byte		        Fs1 , v052
	.byte	W06
	.byte		N05   , Cn1 , v100
	.byte		N11   , As1 , v072
	.byte	W06
	.byte		N05   , Cn1 , v064
	.byte	W06
	.byte	PEND
mus_qz_664_3_002:
	.byte		N05   , Cn1 , v100
	.byte		N03   , Fs1 , v080
	.byte	W06
	.byte		        Fs1 , v052
	.byte	W18
	.byte		N05   , As0 , v104
	.byte	W12
	.byte		N03   , Fs1 , v080
	.byte	W06
	.byte		N05   , Cn1 , v100
	.byte		N03   , Fs1 , v052
	.byte	W06
	.byte		N05   , Cn1 , v100
	.byte	W06
	.byte		        As0 , v056
	.byte	W06
	.byte		N03   , Fs1 , v080
	.byte	W06
	.byte		        Fs1 , v052
	.byte	W06
	.byte		N05   , As0 , v104
	.byte		N11   , As1 , v072
	.byte	W12
	.byte		N05   , Cn1 , v100
	.byte		N03   , Fs1 , v080
	.byte	W06
	.byte		N05   , Cn1 , v060
	.byte		N03   , Fs1 , v052
	.byte	W06
	.byte	PEND
	.byte		N05   , Cn1 , v100
	.byte		N32   , As2 , v096 , gtp3
	.byte	W24
	.byte		N05   , As0 , v104
	.byte	W12
	.byte		N03   , Fs1 , v080
	.byte	W06
	.byte		N05   , Cn1 , v072
	.byte		N03   , Fs1 , v052
	.byte	W06
	.byte		N05   , Cn1 , v100
	.byte	W06
	.byte		        As0 , v056
	.byte	W18
	.byte		        As0 , v104
	.byte		N03   , Fs1 , v080
	.byte	W06
	.byte		        Fs1 , v052
	.byte	W06
	.byte		N05   , Cn1 , v100
	.byte		N11   , As1 , v072
	.byte	W06
	.byte		N05   , Cn1 , v064
	.byte	W06
	.byte	PATT
	 ptrvalue	mus_qz_664_3_000
	.byte	PATT
	 ptrvalue	mus_qz_664_3_001
	.byte	PATT
	 ptrvalue	mus_qz_664_3_002
	.byte	GOTO
	 ptrvalue	mus_qz_664_3_B1
	.byte	FINE

/********************** Track  4 **********************/

mus_qz_664_4:
	.byte		KEYSH , mus_qz_664_key+0
	.byte		VOICE , 11
	.byte		VOL   , 39*mus_qz_664_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 56
	.byte		MOD   , 12
	.byte		LFOS  , 23
	.byte		MODT  , mod_pan
	.byte		LFODL , 9
	.byte		N11   , Fs3 , v088
	.byte	W12
	.byte		        Fn3
	.byte	W12
	.byte		N44   , Fs3 , v088 , gtp3
	.byte	W48
	.byte		N11   , Dn3
	.byte	W12
	.byte		        Ds3
	.byte	W12
	.byte		        En3
	.byte	W12
	.byte		        Fn3
	.byte	W12
mus_qz_664_4_000:
	.byte		N10   , Fs3 , v088
	.byte	W36
	.byte	N10
	.byte	W36
	.byte	N10
	.byte	W24
	.byte	PEND
mus_qz_664_4_001:
	.byte		N10   , Fs3 , v088
	.byte	W48
	.byte		N11   , Dn3
	.byte	W12
	.byte		        Ds3
	.byte	W12
	.byte		        En3
	.byte	W12
	.byte		        Fn3
	.byte	W12
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_664_4_000
	.byte	PATT
	 ptrvalue	mus_qz_664_4_001
mus_qz_664_4_B1:
	.byte	PATT
	 ptrvalue	mus_qz_664_4_000
	.byte	PATT
	 ptrvalue	mus_qz_664_4_001
	.byte	PATT
	 ptrvalue	mus_qz_664_4_000
	.byte	PATT
	 ptrvalue	mus_qz_664_4_001
	.byte	PATT
	 ptrvalue	mus_qz_664_4_000
	.byte	PATT
	 ptrvalue	mus_qz_664_4_001
	.byte	PATT
	 ptrvalue	mus_qz_664_4_000
	.byte	PATT
	 ptrvalue	mus_qz_664_4_001
	.byte	GOTO
	 ptrvalue	mus_qz_664_4_B1
	.byte	FINE

/********************** Track  5 **********************/

mus_qz_664_5:
	.byte		KEYSH , mus_qz_664_key+0
	.byte		VOICE , 23
	.byte		VOL   , 73*mus_qz_664_mvl/mxv
	.byte		PAN   , c_v+16
	.byte		PRIO  , 60
	.byte		VOL   , 56*mus_qz_664_mvl/mxv
	.byte		N11   , Bn3 , v088
	.byte	W12
	.byte		        As3
	.byte	W12
	.byte		N44   , Bn3 , v088 , gtp3
	.byte	W48
	.byte		N11   , Ds4
	.byte	W12
	.byte		        Dn4
	.byte	W12
	.byte		        Cs4
	.byte	W12
	.byte		        Cn4
	.byte	W12
mus_qz_664_5_000:
	.byte		N05   , Bn3 , v088
	.byte	W06
	.byte		        Bn3 , v028
	.byte	W30
	.byte		        Bn3 , v088
	.byte	W06
	.byte		        Bn3 , v028
	.byte	W30
	.byte		        Bn3 , v088
	.byte	W06
	.byte		        Bn3 , v028
	.byte	W18
	.byte	PEND
	.byte		        Bn3 , v088
	.byte	W48
	.byte		N11   , Gn3
	.byte	W12
	.byte		        Gs3
	.byte	W12
	.byte		        An3
	.byte	W12
	.byte		        As3
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_664_5_000
	.byte		N05   , Bn3 , v088
	.byte	W96
mus_qz_664_5_B1:
	.byte		VOL   , 35*mus_qz_664_mvl/mxv
	.byte		TIE   , Bn2 , v100
	.byte		N92   , Fs3 , v100 , gtp3
	.byte	W12
	.byte		VOL   , 36*mus_qz_664_mvl/mxv
	.byte	W12
	.byte		        36*mus_qz_664_mvl/mxv
	.byte	W12
	.byte		        37*mus_qz_664_mvl/mxv
	.byte	W12
	.byte		        37*mus_qz_664_mvl/mxv
	.byte	W12
	.byte		        39*mus_qz_664_mvl/mxv
	.byte	W12
	.byte		        39*mus_qz_664_mvl/mxv
	.byte	W12
	.byte		        40*mus_qz_664_mvl/mxv
	.byte	W12
	.byte	PEND
mus_qz_664_5_001:
	.byte		VOL   , 42*mus_qz_664_mvl/mxv
	.byte		N92   , Gn3 , v100 , gtp3
	.byte	W12
	.byte		VOL   , 43*mus_qz_664_mvl/mxv
	.byte	W12
	.byte		        44*mus_qz_664_mvl/mxv
	.byte	W12
	.byte		        44*mus_qz_664_mvl/mxv
	.byte	W12
	.byte		        44*mus_qz_664_mvl/mxv
	.byte	W12
	.byte		        46*mus_qz_664_mvl/mxv
	.byte	W12
	.byte		        46*mus_qz_664_mvl/mxv
	.byte	W12
	.byte		        47*mus_qz_664_mvl/mxv
	.byte	W12
	.byte	PEND
mus_qz_664_5_002:
	.byte		N92   , Gs3 , v100 , gtp3
	.byte	W12
	.byte		VOL   , 48*mus_qz_664_mvl/mxv
	.byte	W12
	.byte		        49*mus_qz_664_mvl/mxv
	.byte	W12
	.byte		        50*mus_qz_664_mvl/mxv
	.byte	W12
	.byte		        51*mus_qz_664_mvl/mxv
	.byte	W12
	.byte		        52*mus_qz_664_mvl/mxv
	.byte	W12
	.byte		        52*mus_qz_664_mvl/mxv
	.byte	W12
	.byte		        52*mus_qz_664_mvl/mxv
	.byte	W12
	.byte	PEND
	.byte		        54*mus_qz_664_mvl/mxv
	.byte		N92   , An3 , v100 , gtp3
	.byte	W12
	.byte		VOL   , 52*mus_qz_664_mvl/mxv
	.byte	W12
	.byte		        50*mus_qz_664_mvl/mxv
	.byte	W12
	.byte		        48*mus_qz_664_mvl/mxv
	.byte	W12
	.byte		        46*mus_qz_664_mvl/mxv
	.byte	W12
	.byte		        44*mus_qz_664_mvl/mxv
	.byte	W12
	.byte		        41*mus_qz_664_mvl/mxv
	.byte	W12
	.byte		        39*mus_qz_664_mvl/mxv
	.byte	W11
	.byte		EOT   , Bn2
	.byte	W01
	.byte	PATT
	 ptrvalue	mus_qz_664_5_B1
	.byte	PATT
	 ptrvalue	mus_qz_664_5_001
	.byte	PATT
	 ptrvalue	mus_qz_664_5_002
	.byte		VOL   , 54*mus_qz_664_mvl/mxv
	.byte		N92   , An3 , v100 , gtp3
	.byte	W12
	.byte		VOL   , 52*mus_qz_664_mvl/mxv
	.byte	W12
	.byte		        50*mus_qz_664_mvl/mxv
	.byte	W12
	.byte		        48*mus_qz_664_mvl/mxv
	.byte	W12
	.byte		        46*mus_qz_664_mvl/mxv
	.byte	W12
	.byte		        44*mus_qz_664_mvl/mxv
	.byte	W12
	.byte		        42*mus_qz_664_mvl/mxv
	.byte	W12
	.byte		        39*mus_qz_664_mvl/mxv
	.byte	W11
	.byte		EOT   , Bn2
	.byte	W01
	.byte	GOTO
	 ptrvalue	mus_qz_664_5_B1
	.byte	FINE

/********************** Track  6 **********************/

mus_qz_664_6:
	.byte		KEYSH , mus_qz_664_key+0
	.byte		VOICE , 11
	.byte		VOL   , 20*mus_qz_664_mvl/mxv
	.byte		PAN   , c_v+56
	.byte		PRIO  , 38
	.byte		MOD   , 10
	.byte		LFOS  , 23
	.byte		MODT  , mod_pan
	.byte		LFODL , 12
	.byte	W12
	.byte		N11   , Bn3 , v088
	.byte	W12
	.byte		        As3
	.byte	W12
	.byte		N44   , Bn3 , v088 , gtp3
	.byte	W48
	.byte		N11   , Ds4
	.byte	W12
	.byte		        Dn4
	.byte	W12
	.byte		        Cs4
	.byte	W12
mus_qz_664_6_000:
	.byte		N11   , Cn4 , v088
	.byte	W12
	.byte		N05   , Bn3
	.byte	W36
	.byte	N05
	.byte	W36
	.byte	N05
	.byte	W12
	.byte	PEND
mus_qz_664_6_001:
	.byte	W12
	.byte		N05   , Bn3 , v088
	.byte	W48
	.byte		N11   , Gn3
	.byte	W12
	.byte		        Gs3
	.byte	W12
	.byte		        An3
	.byte	W12
	.byte	PEND
	.byte		        As3
	.byte	W12
	.byte		N05   , Bn3
	.byte	W36
	.byte	N05
	.byte	W36
	.byte	N05
	.byte	W12
mus_qz_664_6_002:
	.byte	W12
	.byte		N05   , Bn3 , v088
	.byte	W48
	.byte		N11   , Ds4
	.byte	W12
	.byte		        Dn4
	.byte	W12
	.byte		        Cs4
	.byte	W12
	.byte	PEND
mus_qz_664_6_B1:
	.byte	PATT
	 ptrvalue	mus_qz_664_6_000
	.byte	PATT
	 ptrvalue	mus_qz_664_6_001
mus_qz_664_6_003:
	.byte		N11   , As3 , v088
	.byte	W12
	.byte		        Bn3
	.byte	W36
	.byte	N05
	.byte	W36
	.byte	N05
	.byte	W12
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_664_6_002
	.byte	PATT
	 ptrvalue	mus_qz_664_6_000
	.byte	PATT
	 ptrvalue	mus_qz_664_6_001
	.byte	PATT
	 ptrvalue	mus_qz_664_6_003
	.byte	PATT
	 ptrvalue	mus_qz_664_6_002
	.byte	GOTO
	 ptrvalue	mus_qz_664_6_B1
	.byte	FINE

/******************************************************/
	.p2align	2

mus_qz_664:
	.byte	6	/* NumTrks */
	.byte	0	/* NumBlks */
	.byte	mus_qz_664_pri	/* Priority */
	.byte	mus_qz_664_rev	/* Reverb. */
space64 4
	ptrvalue	mus_qz_664_grp

	ptrvalue	mus_qz_664_1
	ptrvalue	mus_qz_664_2
	ptrvalue	mus_qz_664_3
	ptrvalue	mus_qz_664_4
	ptrvalue	mus_qz_664_5
	ptrvalue	mus_qz_664_6

	.end
