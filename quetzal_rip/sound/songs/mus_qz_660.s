	.include "asm/macros/bit_width.inc"

	.include "MPlayDef.s"

/* Moemon Quetzal song 660, disassembled from the ROM by tools/quetzal_rip/songs.py.
   "org" comments give the ROM address of the bytes that follow. */

	.equ	mus_qz_660_grp, voicegroup_qz_songs
	.equ	mus_qz_660_pri, 0
	.equ	mus_qz_660_rev, reverb_set+0
	.equ	mus_qz_660_mvl, 127
	.equ	mus_qz_660_key, 0
	.equ	mus_qz_660_tbs, 1
	.equ	mus_qz_660_exg, 1
	.equ	mus_qz_660_cmp, 1

	.section .rodata
	.global	mus_qz_660
	.p2align	2
/* org 0x096A0970 */

/********************** Track  1 **********************/

mus_qz_660_1:
	.byte		KEYSH , mus_qz_660_key+0
	.byte		TEMPO , 132*mus_qz_660_tbs/2
	.byte		VOICE , 42
	.byte		VOL   , 71*mus_qz_660_mvl/mxv
	.byte		PAN   , c_v-44
	.byte		PRIO  , 64
	.byte		MOD   , 3
	.byte		LFOS  , 25
	.byte		LFODL , 14
	.byte	W12
	.byte	W96
	.byte		TEMPO , 134*mus_qz_660_tbs/2
	.byte		N11   , En2 , v100
	.byte	W12
	.byte		        En2 , v064
	.byte	W12
	.byte		        An2 , v100
	.byte	W12
	.byte		        An2 , v064
	.byte	W12
	.byte		        Bn2 , v104
	.byte	W12
	.byte		        Bn2 , v068
	.byte	W12
	.byte		        An2 , v104
	.byte	W12
	.byte		        An2 , v056
	.byte	W12
mus_qz_660_1_000:
	.byte		N11   , Dn3 , v108
	.byte	W12
	.byte		        Dn3 , v060
	.byte	W12
	.byte		        En3 , v108
	.byte	W12
	.byte		        En3 , v068
	.byte	W12
	.byte		        An3 , v112
	.byte	W12
	.byte		        An3 , v072
	.byte	W12
	.byte		        Fn3 , v112
	.byte	W12
	.byte		        Fn3 , v072
	.byte	W12
	.byte	PEND
mus_qz_660_1_B1:
	.byte		N11   , Dn3 , v108
	.byte	W12
	.byte		        Dn3 , v060
	.byte	W12
	.byte		        Bn2 , v108
	.byte	W12
	.byte		        Bn2 , v060
	.byte	W12
	.byte		        An2 , v100
	.byte	W12
	.byte		        An2 , v060
	.byte	W12
	.byte		        Bn2 , v100
	.byte	W12
	.byte		        Bn2 , v060
	.byte	W12
	.byte		        Dn3 , v104
	.byte	W12
	.byte		        Dn3 , v064
	.byte	W12
	.byte		        Bn2 , v104
	.byte	W12
	.byte		        Bn2 , v064
	.byte	W12
	.byte		        Dn3 , v108
	.byte	W12
	.byte		        Dn3 , v060
	.byte	W12
	.byte		        En3 , v108
	.byte	W12
	.byte		        En3 , v060
	.byte	W12
	.byte		        Fn3 , v112
	.byte	W12
	.byte		        Fn3 , v068
	.byte	W12
	.byte		        Dn3 , v112
	.byte	W12
	.byte		        Dn3 , v068
	.byte	W12
	.byte		        En3 , v104
	.byte	W12
	.byte		        En3 , v064
	.byte	W12
	.byte		        An3 , v104
	.byte	W12
	.byte		        An3 , v064
	.byte	W12
	.byte		TEMPO , 132*mus_qz_660_tbs/2
	.byte		        Dn3 , v108
	.byte	W12
	.byte		        Dn3 , v060
	.byte	W12
	.byte		        Bn2 , v108
	.byte	W12
	.byte		        Bn2 , v060
	.byte	W12
	.byte		        An2 , v112
	.byte	W12
	.byte		        An2 , v072
	.byte	W12
	.byte		        Bn2 , v112
	.byte	W12
	.byte		        Bn2 , v072
	.byte	W12
	.byte		TEMPO , 132*mus_qz_660_tbs/2
	.byte		        Dn3 , v108
	.byte	W12
	.byte		        Dn3 , v060
	.byte	W12
	.byte		        En3 , v108
	.byte	W12
	.byte		        En3 , v068
	.byte	W12
	.byte		        An3 , v108
	.byte	W12
	.byte		        An3 , v068
	.byte	W12
	.byte		        Fn3 , v108
	.byte	W12
	.byte		        Fn3 , v068
	.byte	W12
	.byte		        Dn3 , v104
	.byte	W12
	.byte		        Dn3 , v056
	.byte	W12
	.byte		        Bn2 , v104
	.byte	W12
	.byte		        Bn2 , v056
	.byte	W12
	.byte		        An2 , v100
	.byte	W12
	.byte		        An2 , v060
	.byte	W12
	.byte		        Bn2 , v100
	.byte	W12
	.byte		        Bn2 , v060
	.byte	W12
	.byte		        Dn3 , v100
	.byte	W12
	.byte		        Dn3 , v060
	.byte	W12
	.byte		        Bn2 , v100
	.byte	W12
	.byte		        Bn2 , v060
	.byte	W12
	.byte		        Dn3 , v108
	.byte	W12
	.byte		        Dn3 , v060
	.byte	W12
	.byte		        En3 , v108
	.byte	W12
	.byte		        En3 , v060
	.byte	W12
	.byte		        Fn3 , v112
	.byte	W12
	.byte		        Fn3 , v072
	.byte	W12
	.byte		        Dn3 , v112
	.byte	W12
	.byte		        Dn3 , v072
	.byte	W12
	.byte		        En3 , v112
	.byte	W12
	.byte		        En3 , v072
	.byte	W12
	.byte		        An3 , v112
	.byte	W12
	.byte		        An3 , v072
	.byte	W12
	.byte		        Dn3 , v108
	.byte	W12
	.byte		        Dn3 , v060
	.byte	W12
	.byte		        Bn2 , v108
	.byte	W12
	.byte		        Bn2 , v060
	.byte	W12
	.byte		        An2 , v100
	.byte	W12
	.byte		        An2 , v060
	.byte	W12
	.byte		        Bn2 , v100
	.byte	W12
	.byte		        Bn2 , v060
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_660_1_000
	.byte	GOTO
	 ptrvalue	mus_qz_660_1_B1
	.byte	FINE

/********************** Track  2 **********************/

mus_qz_660_2:
	.byte		KEYSH , mus_qz_660_key+0
	.byte		VOICE , 42
	.byte		VOL   , 23*mus_qz_660_mvl/mxv
	.byte		PAN   , c_v+56
	.byte		PRIO  , 40
	.byte		MOD   , 3
	.byte		LFOS  , 25
	.byte		LFODL , 14
	.byte	W12
	.byte	W96
	.byte	W12
	.byte		N11   , En2 , v100
	.byte	W12
	.byte		        En2 , v064
	.byte	W12
	.byte		        An2 , v100
	.byte	W12
	.byte		        An2 , v064
	.byte	W12
	.byte		        Bn2 , v104
	.byte	W12
	.byte		        Bn2 , v068
	.byte	W12
	.byte		        An2 , v104
	.byte	W12
	.byte		        An2 , v056
	.byte	W12
	.byte		        Dn3 , v108
	.byte	W12
	.byte		        Dn3 , v060
	.byte	W12
	.byte		        En3 , v108
	.byte	W12
	.byte		        En3 , v068
	.byte	W12
	.byte		        An3 , v112
	.byte	W12
	.byte		        An3 , v072
	.byte	W12
	.byte		        Fn3 , v112
	.byte	W12
mus_qz_660_2_B1:
	.byte		N11   , Fn3 , v072
	.byte	W12
	.byte		        Dn3 , v108
	.byte	W12
	.byte		        Dn3 , v060
	.byte	W12
	.byte		        Bn2 , v108
	.byte	W12
	.byte		        Bn2 , v060
	.byte	W12
	.byte		        An2 , v100
	.byte	W12
	.byte		        An2 , v060
	.byte	W12
	.byte		        Bn2 , v100
	.byte	W12
	.byte		        Bn2 , v060
	.byte	W12
	.byte		        Dn3 , v104
	.byte	W12
	.byte		        Dn3 , v064
	.byte	W12
	.byte		        Bn2 , v104
	.byte	W12
	.byte		        Bn2 , v064
	.byte	W12
	.byte		        Dn3 , v108
	.byte	W12
	.byte		        Dn3 , v060
	.byte	W12
	.byte		        En3 , v108
	.byte	W12
	.byte		        En3 , v060
	.byte	W12
	.byte		        Fn3 , v112
	.byte	W12
	.byte		        Fn3 , v068
	.byte	W12
	.byte		        Dn3 , v112
	.byte	W12
	.byte		        Dn3 , v068
	.byte	W12
	.byte		        En3 , v104
	.byte	W12
	.byte		        En3 , v064
	.byte	W12
	.byte		        An3 , v104
	.byte	W12
	.byte		        An3 , v064
	.byte	W12
	.byte		        Dn3 , v108
	.byte	W12
	.byte		        Dn3 , v060
	.byte	W12
	.byte		        Bn2 , v108
	.byte	W12
	.byte		        Bn2 , v060
	.byte	W12
	.byte		        An2 , v112
	.byte	W12
	.byte		        An2 , v072
	.byte	W12
	.byte		        Bn2 , v112
	.byte	W12
	.byte		        Bn2 , v072
	.byte	W12
	.byte		        Dn3 , v108
	.byte	W12
	.byte		        Dn3 , v060
	.byte	W12
	.byte		        En3 , v108
	.byte	W12
	.byte		        En3 , v068
	.byte	W12
	.byte		        An3 , v108
	.byte	W12
	.byte		        An3 , v068
	.byte	W12
	.byte		        Fn3 , v108
	.byte	W12
	.byte		        Fn3 , v068
	.byte	W12
	.byte		        Dn3 , v104
	.byte	W12
	.byte		        Dn3 , v056
	.byte	W12
	.byte		        Bn2 , v104
	.byte	W12
	.byte		        Bn2 , v056
	.byte	W12
	.byte		        An2 , v100
	.byte	W12
	.byte		        An2 , v060
	.byte	W12
	.byte		        Bn2 , v100
	.byte	W12
	.byte		        Bn2 , v060
	.byte	W12
	.byte		        Dn3 , v100
	.byte	W12
	.byte		        Dn3 , v060
	.byte	W12
	.byte		        Bn2 , v100
	.byte	W12
	.byte		        Bn2 , v060
	.byte	W12
	.byte		        Dn3 , v108
	.byte	W12
	.byte		        Dn3 , v060
	.byte	W12
	.byte		        En3 , v108
	.byte	W12
	.byte		        En3 , v060
	.byte	W12
	.byte		        Fn3 , v112
	.byte	W12
	.byte		        Fn3 , v072
	.byte	W12
	.byte		        Dn3 , v112
	.byte	W12
	.byte		        Dn3 , v072
	.byte	W12
	.byte		        En3 , v112
	.byte	W12
	.byte		        En3 , v072
	.byte	W12
	.byte		        An3 , v112
	.byte	W12
	.byte		        An3 , v072
	.byte	W12
	.byte		        Dn3 , v108
	.byte	W12
	.byte		        Dn3 , v060
	.byte	W12
	.byte		        Bn2 , v108
	.byte	W12
	.byte		        Bn2 , v060
	.byte	W12
	.byte		        An2 , v100
	.byte	W12
	.byte		        An2 , v060
	.byte	W12
	.byte		        Bn2 , v100
	.byte	W12
	.byte		        An2 , v056
	.byte		N11   , Bn2 , v060
	.byte	W12
	.byte		        Dn3 , v108
	.byte	W12
	.byte		        Dn3 , v060
	.byte	W12
	.byte		        En3 , v108
	.byte	W12
	.byte		        En3 , v068
	.byte	W12
	.byte		        An3 , v112
	.byte	W12
	.byte		        An3 , v072
	.byte	W12
	.byte		        Fn3 , v112
	.byte	W12
	.byte	GOTO
	 ptrvalue	mus_qz_660_2_B1
	.byte	FINE

/********************** Track  3 **********************/

mus_qz_660_3:
	.byte		KEYSH , mus_qz_660_key+0
	.byte		VOICE , 35
	.byte		VOL   , 82*mus_qz_660_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte		MOD   , 3
	.byte		LFOS  , 25
	.byte		LFODL , 19
	.byte		N05   , Bn4 , v124
	.byte	W06
	.byte		        Cn5
	.byte	W06
	.byte		TIE   , Bn4 , v108
	.byte	W96
	.byte	W44
	.byte	W03
	.byte	EOT
	.byte	W01
	.byte		N32   , Dn5 , v108 , gtp3
	.byte	W36
	.byte		N05   , En5
	.byte	W06
	.byte		        Fn5
	.byte	W06
mus_qz_660_3_000:
	.byte		N80   , En5 , v108 , gtp3
	.byte	W84
	.byte		N05   , Gn5
	.byte	W06
	.byte		        Gs5
	.byte	W06
	.byte	PEND
mus_qz_660_3_B1:
	.byte		N68   , An5 , v108 , gtp3
	.byte	W72
	.byte		N23   , Bn5
	.byte	W24
	.byte	PEND
mus_qz_660_3_001:
	.byte		N52   , Dn5 , v108 , gtp1
	.byte	W54
	.byte		N05   , En5
	.byte	W06
	.byte		        Fn5
	.byte	W06
	.byte		        Dn5
	.byte	W06
	.byte		N68   , En5 , v108 , gtp3
	.byte	W24
	.byte	PEND
mus_qz_660_3_002:
	.byte	W48
	.byte		N44   , Bn4 , v108 , gtp3
	.byte	W48
	.byte	PEND
	.byte		        Fn5 , v108 , gtp3
	.byte	W48
	.byte		N02   , En5 , v068
	.byte	W03
	.byte		        Ds5
	.byte	W03
	.byte		        Dn5
	.byte	W03
	.byte		        Cs5
	.byte	W03
	.byte		        Cn5
	.byte	W03
	.byte		        Bn4
	.byte	W03
	.byte		        As4
	.byte	W03
	.byte		        An4
	.byte	W15
	.byte		N05   , En5 , v108
	.byte	W06
	.byte		        Fn5
	.byte	W06
	.byte	PATT
	 ptrvalue	mus_qz_660_3_000
	.byte	PATT
	 ptrvalue	mus_qz_660_3_B1
	.byte	PATT
	 ptrvalue	mus_qz_660_3_001
	.byte	PATT
	 ptrvalue	mus_qz_660_3_002
	.byte		N44   , Fn5 , v108 , gtp3
	.byte	W48
	.byte		N02   , En5 , v060
	.byte	W03
	.byte		        Ds5
	.byte	W03
	.byte		        Dn5
	.byte	W03
	.byte		        Cs5
	.byte	W03
	.byte		        Cn5
	.byte	W03
	.byte		        Bn4
	.byte	W03
	.byte		        As4
	.byte	W03
	.byte		        An4
	.byte	W15
	.byte		N05   , En5 , v108
	.byte	W06
	.byte		        Fn5
	.byte	W06
	.byte	PATT
	 ptrvalue	mus_qz_660_3_000
	.byte	GOTO
	 ptrvalue	mus_qz_660_3_B1
	.byte	FINE

/********************** Track  4 **********************/

mus_qz_660_4:
	.byte		KEYSH , mus_qz_660_key+0
	.byte		VOICE , 23
	.byte		PAN   , c_v+46
	.byte		PRIO  , 38
	.byte		VOL   , 33*mus_qz_660_mvl/mxv
	.byte	W12
	.byte	W96
	.byte		        15*mus_qz_660_mvl/mxv
	.byte		TIE   , En2 , v080
	.byte		TIE   , Bn2 , v088
	.byte		TIE   , En3 , v108
	.byte	W06
	.byte		VOL   , 16*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        17*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        18*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        19*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        20*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        20*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        21*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        22*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        23*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        24*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        25*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        27*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        28*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        29*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        29*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        31*mus_qz_660_mvl/mxv
	.byte	W48
	.byte		        29*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        27*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        25*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        23*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        21*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        20*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        17*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        15*mus_qz_660_mvl/mxv
	.byte	W05
	.byte		EOT   , En2
	.byte		        Bn2
	.byte		        En3
	.byte	W01
mus_qz_660_4_B1:
	.byte		VOL   , 13*mus_qz_660_mvl/mxv
	.byte		TIE   , Dn2 , v084
	.byte		TIE   , An2 , v088
	.byte		TIE   , Dn3 , v108
	.byte	W06
	.byte		VOL   , 14*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        15*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        15*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        16*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        17*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        17*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        19*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        19*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        20*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        20*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        21*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        22*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        23*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        23*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        25*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        25*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        27*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        28*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        28*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        29*mus_qz_660_mvl/mxv
	.byte	W44
	.byte	W03
	.byte		EOT   , Dn2
	.byte		        An2
	.byte		        Dn3
	.byte	W01
	.byte		TIE   , En2 , v080
	.byte		TIE   , Bn2 , v092
	.byte		TIE   , En3 , v108
	.byte	W24
	.byte	W30
	.byte		VOL   , 28*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        27*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        25*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        23*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        22*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        21*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        20*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        19*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        17*mus_qz_660_mvl/mxv
	.byte	W17
	.byte		EOT   , En2
	.byte		        Bn2
	.byte		        En3
	.byte	W01
	.byte		N92   , Dn2 , v080 , gtp3
	.byte		N92   , An2 , v088 , gtp3
	.byte		N92   , Dn3 , v108 , gtp3
	.byte	W06
	.byte		VOL   , 19*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        19*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        20*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        21*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        22*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        23*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        24*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        25*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        27*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        28*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        27*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        25*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        23*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        21*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        20*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        16*mus_qz_660_mvl/mxv
	.byte		N92   , En2 , v080 , gtp2
	.byte		N92   , Bn2 , v088 , gtp2
	.byte		N92   , En3 , v108 , gtp2
	.byte	W06
	.byte		VOL   , 17*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        18*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        19*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        20*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        20*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        21*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        21*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        22*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        23*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        23*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        25*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        25*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        27*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        27*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        28*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        29*mus_qz_660_mvl/mxv
	.byte		TIE   , Dn2 , v080
	.byte		TIE   , An2 , v088
	.byte		TIE   , Dn3 , v108
	.byte	W06
	.byte		VOL   , 29*mus_qz_660_mvl/mxv
	.byte	W30
	.byte		        29*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        28*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        27*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        25*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        25*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        23*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        23*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        22*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        21*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        21*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        20*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        19*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        19*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        18*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        17*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        19*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        20*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        20*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        21*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        21*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        22*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        23*mus_qz_660_mvl/mxv
	.byte	W04
	.byte		EOT   , Dn2
	.byte		        An2
	.byte		        Dn3
	.byte	W02
	.byte		VOL   , 24*mus_qz_660_mvl/mxv
	.byte		TIE   , En2 , v080
	.byte		TIE   , Bn2 , v088
	.byte		TIE   , En3 , v108
	.byte	W06
	.byte		VOL   , 25*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        25*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        27*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        28*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        28*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        29*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        30*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        31*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        31*mus_qz_660_mvl/mxv
	.byte	W64
	.byte	W01
	.byte		EOT   , En2
	.byte		        Bn2
	.byte		        En3
	.byte	W01
	.byte		N92   , Dn2 , v080 , gtp3
	.byte		N92   , An2 , v088 , gtp3
	.byte		N92   , Dn3 , v108 , gtp3
	.byte	W06
	.byte		VOL   , 31*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        30*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        29*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        29*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        28*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        27*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        25*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        25*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        23*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        23*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        22*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        21*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        20*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        20*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        19*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        20*mus_qz_660_mvl/mxv
	.byte		N92   , En2 , v080 , gtp3
	.byte		N92   , Bn2 , v088 , gtp3
	.byte		N92   , En3 , v108 , gtp3
	.byte	W06
	.byte		VOL   , 21*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        22*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        24*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        27*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        28*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        29*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        31*mus_qz_660_mvl/mxv
	.byte	W30
	.byte		        28*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        25*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        23*mus_qz_660_mvl/mxv
	.byte	W06
	.byte		        21*mus_qz_660_mvl/mxv
	.byte		        19*mus_qz_660_mvl/mxv
	.byte	W06
	.byte	GOTO
	 ptrvalue	mus_qz_660_4_B1
	.byte	FINE

/********************** Track  5 **********************/

mus_qz_660_5:
	.byte		KEYSH , mus_qz_660_key+0
	.byte		VOICE , 39
	.byte		VOL   , 56*mus_qz_660_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 40
	.byte	W12
	.byte	W96
	.byte		N05   , Bn0 , v108
	.byte		N23   , En2 , v092
	.byte	W24
	.byte		N11   , Dn3 , v088
	.byte	W12
	.byte		        Fs3
	.byte	W12
	.byte		N05   , As0
	.byte	W24
	.byte		N11   , Gs3
	.byte	W12
	.byte		N05   , Bn0
	.byte		N11   , En3
	.byte	W12
mus_qz_660_5_000:
	.byte		N05   , Bn0 , v108
	.byte	W24
	.byte		N11   , Dn3 , v088
	.byte	W12
	.byte		        Fs3
	.byte	W12
	.byte		N05   , As0
	.byte	W18
	.byte	N05
	.byte	W06
	.byte		N11   , Gs3
	.byte	W12
	.byte		N05   , Bn0
	.byte		N11   , En3
	.byte	W12
	.byte	PEND
mus_qz_660_5_B1:
	.byte		N05   , Bn0 , v108
	.byte	W24
	.byte		N11   , Dn3 , v088
	.byte	W12
	.byte		        Fs3
	.byte	W12
	.byte		N05   , As0
	.byte	W24
	.byte		N11   , Gs3
	.byte	W12
	.byte		N05   , Bn0
	.byte		N11   , En3
	.byte	W12
	.byte		N05   , Bn0 , v108
	.byte	W12
	.byte		        As0 , v088
	.byte	W12
	.byte		N11   , Dn3
	.byte	W06
	.byte		N05   , As0
	.byte	W06
	.byte		N11   , Fs3
	.byte	W12
	.byte		N05   , As0
	.byte		N11   , Gs3
	.byte	W12
	.byte		N05   , Bn0
	.byte		N11   , En3
	.byte	W12
	.byte		N05   , Bn0 , v108
	.byte	W24
	.byte	W12
	.byte		        As0 , v072
	.byte	W12
	.byte		N11   , Dn3 , v088
	.byte	W12
	.byte		N05   , Bn0 , v080
	.byte		N11   , Fs3 , v088
	.byte	W12
	.byte		N05   , Bn0
	.byte	W24
	.byte		N11   , Gs3
	.byte	W12
	.byte		N05   , Bn0
	.byte		N11   , En3
	.byte	W12
mus_qz_660_5_001:
	.byte		N05   , Bn0 , v108
	.byte	W24
	.byte		N11   , Dn3 , v088
	.byte	W12
	.byte		N05   , As0
	.byte		N11   , Fs3
	.byte	W36
	.byte		        Gs3
	.byte	W12
	.byte		N05   , Bn0
	.byte		N11   , En3
	.byte	W12
	.byte	PEND
	.byte		N05   , Bn0 , v108
	.byte		N23   , En2 , v092
	.byte	W24
	.byte		N11   , Dn3 , v088
	.byte	W12
	.byte		        Fs3
	.byte	W12
	.byte		N05   , As0 , v072
	.byte	W24
	.byte		N11   , Gs3 , v088
	.byte	W12
	.byte		N05   , Bn0
	.byte		N11   , En3
	.byte	W12
	.byte		N05   , Bn0 , v108
	.byte	W24
	.byte		N11   , Dn3 , v088
	.byte	W12
	.byte		N05   , As0
	.byte		N11   , Fs3
	.byte	W18
	.byte		N05   , As0 , v076
	.byte	W18
	.byte		        As0 , v088
	.byte		N11   , Gs3
	.byte	W12
	.byte		N05   , Bn0
	.byte		N11   , En3
	.byte	W12
	.byte		N05   , Bn0 , v108
	.byte	W24
	.byte		N11   , Dn3 , v088
	.byte	W12
	.byte		N05   , As0
	.byte		N11   , Fs3
	.byte	W24
	.byte		N05   , Bn0
	.byte	W12
	.byte	N05
	.byte		N11   , Gs3
	.byte	W12
	.byte		        En3
	.byte	W12
	.byte	W12
	.byte		N05   , Bn0 , v072
	.byte	W12
	.byte		        As0 , v088
	.byte		N11   , Dn3
	.byte	W12
	.byte		        Fs3
	.byte	W24
	.byte		N05   , Bn0
	.byte	W12
	.byte	N05
	.byte		N11   , Gs3
	.byte	W12
	.byte		        En3
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_660_5_001
	.byte	PATT
	 ptrvalue	mus_qz_660_5_000
	.byte	GOTO
	 ptrvalue	mus_qz_660_5_B1
	.byte	FINE

/********************** Track  6 **********************/

mus_qz_660_6:
	.byte		KEYSH , mus_qz_660_key+0
	.byte		VOICE , 35
	.byte		VOL   , 17*mus_qz_660_mvl/mxv
	.byte		PAN   , c_v+56
	.byte		PRIO  , 38
	.byte		MOD   , 3
	.byte		LFOS  , 25
	.byte		LFODL , 19
	.byte	W12
	.byte		N05   , Bn4 , v124
	.byte	W06
	.byte		        Cn5
	.byte	W06
	.byte		TIE   , Bn4 , v108
	.byte	W84
	.byte	W44
	.byte	W03
	.byte	EOT
	.byte	W13
	.byte		N32   , Dn5 , v108 , gtp3
	.byte	W36
mus_qz_660_6_000:
	.byte		N05   , En5 , v108
	.byte	W06
	.byte		        Fn5
	.byte	W06
	.byte		N80   , En5 , v108 , gtp3
	.byte	W84
	.byte	PEND
mus_qz_660_6_B1:
	.byte		N05   , Gn5 , v108
	.byte	W06
	.byte		        Gs5
	.byte	W06
	.byte		N68   , An5 , v108 , gtp3
	.byte	W72
	.byte		N23   , Bn5
	.byte	W12
	.byte	PEND
mus_qz_660_6_001:
	.byte	W12
	.byte		N52   , Dn5 , v108 , gtp1
	.byte	W54
	.byte		N05   , En5
	.byte	W06
	.byte		        Fn5
	.byte	W06
	.byte		        Dn5
	.byte	W06
	.byte		N68   , En5 , v108 , gtp3
	.byte	W12
	.byte	PEND
mus_qz_660_6_002:
	.byte	W60
	.byte		N44   , Bn4 , v108 , gtp3
	.byte	W36
	.byte	PEND
	.byte	W12
	.byte		        Fn5 , v108 , gtp3
	.byte	W48
	.byte		N02   , En5 , v068
	.byte	W03
	.byte		        Ds5 , v056
	.byte	W03
	.byte		        Dn5 , v068
	.byte	W03
	.byte		        Cs5 , v056
	.byte	W03
	.byte		        Cn5 , v068
	.byte	W03
	.byte		        Bn4 , v056
	.byte	W03
	.byte		        As4 , v068
	.byte	W03
	.byte		        An4 , v056
	.byte	W15
	.byte	PATT
	 ptrvalue	mus_qz_660_6_000
	.byte	PATT
	 ptrvalue	mus_qz_660_6_B1
	.byte	PATT
	 ptrvalue	mus_qz_660_6_001
	.byte	PATT
	 ptrvalue	mus_qz_660_6_002
	.byte	W12
	.byte		N44   , Fn5 , v108 , gtp3
	.byte	W48
	.byte		N02   , En5 , v068
	.byte	W03
	.byte		        Ds5 , v052
	.byte	W03
	.byte		        Dn5 , v068
	.byte	W03
	.byte		        Cs5 , v052
	.byte	W03
	.byte		        Cn5 , v068
	.byte	W03
	.byte		        Bn4 , v052
	.byte	W03
	.byte		        As4 , v068
	.byte	W03
	.byte		        An4 , v048
	.byte	W15
	.byte	PATT
	 ptrvalue	mus_qz_660_6_000
	.byte	GOTO
	 ptrvalue	mus_qz_660_6_B1
	.byte	FINE

/********************** Track  7 **********************/

mus_qz_660_7:
	.byte		KEYSH , mus_qz_660_key+0
	.byte		VOICE , 20
	.byte		VOL   , 66*mus_qz_660_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 50
	.byte	W12
	.byte	W96
mus_qz_660_7_000:
	.byte		N68   , En2 , v108 , gtp3
	.byte	W84
	.byte		N03   , En2 , v088
	.byte	W12
	.byte	PEND
mus_qz_660_7_001:
	.byte		N68   , En2 , v108 , gtp3
	.byte	W84
	.byte		N03   , En2 , v092
	.byte	W12
	.byte	PEND
mus_qz_660_7_B1:
	.byte		N68   , Dn2 , v108 , gtp3
	.byte	W84
	.byte		N03   , Dn2 , v088
	.byte	W12
	.byte		N52   , Bn1 , v108 , gtp1
	.byte	W60
	.byte		N03   , Dn2 , v092
	.byte	W12
	.byte		N52   , En2 , v100 , gtp1
	.byte	W24
	.byte	W36
	.byte		N03   , En2 , v092
	.byte	W12
	.byte		N32   , En2 , v100 , gtp3
	.byte	W36
	.byte		N03   , Ds2 , v092
	.byte	W12
	.byte		N32   , Dn2 , v100 , gtp3
	.byte	W36
	.byte		N04   , An1 , v092
	.byte	W12
	.byte		N32   , Dn2 , v100 , gtp3
	.byte	W36
	.byte		N04   , En2 , v092
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_660_7_000
	.byte		N52   , Dn2 , v108 , gtp1
	.byte	W60
	.byte		N05   , Dn2 , v092
	.byte	W12
	.byte		N03   , Fn2
	.byte	W12
	.byte		        Dn2
	.byte	W12
	.byte		N32   , Bn1 , v100 , gtp3
	.byte	W36
	.byte		N03   , Fs2 , v092
	.byte	W12
	.byte		N11   , Bn1 , v100
	.byte	W12
	.byte		N03   , Dn2 , v096
	.byte	W12
	.byte		N52   , En2 , v100 , gtp1
	.byte	W24
	.byte	W36
	.byte		N04   , Bn1 , v092
	.byte	W12
	.byte		N32   , En2 , v100 , gtp3
	.byte	W36
	.byte		N04   , Fn2 , v092
	.byte	W12
	.byte		N32   , Dn2 , v108 , gtp3
	.byte	W36
	.byte		N05   , An1 , v092
	.byte	W12
	.byte		N11   , Dn2 , v108
	.byte	W12
	.byte		N02   , An1 , v092
	.byte	W24
	.byte		N05   , Dn2 , v104
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_660_7_001
	.byte	GOTO
	 ptrvalue	mus_qz_660_7_B1
	.byte	FINE

/********************** Track  8 **********************/

mus_qz_660_8:
	.byte		KEYSH , mus_qz_660_key+0
	.byte		VOICE , 35
	.byte		VOL   , 39*mus_qz_660_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		BENDR , 12
	.byte		PRIO  , 38
	.byte		BEND  , c_v-4
	.byte		N05   , Bn4 , v048
	.byte	W06
	.byte		        Cn5
	.byte	W06
	.byte		TIE   , Bn4 , v036
	.byte	W96
	.byte	W32
	.byte	W03
	.byte	EOT
	.byte	W60
	.byte	W01
	.byte	W96
mus_qz_660_8_B1:
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
	 ptrvalue	mus_qz_660_8_B1
	.byte	FINE

/******************************************************/
	.p2align	2

mus_qz_660:
	.byte	8	/* NumTrks */
	.byte	0	/* NumBlks */
	.byte	mus_qz_660_pri	/* Priority */
	.byte	mus_qz_660_rev	/* Reverb. */
space64 4
	ptrvalue	mus_qz_660_grp

	ptrvalue	mus_qz_660_1
	ptrvalue	mus_qz_660_2
	ptrvalue	mus_qz_660_3
	ptrvalue	mus_qz_660_4
	ptrvalue	mus_qz_660_5
	ptrvalue	mus_qz_660_6
	ptrvalue	mus_qz_660_7
	ptrvalue	mus_qz_660_8

	.end
