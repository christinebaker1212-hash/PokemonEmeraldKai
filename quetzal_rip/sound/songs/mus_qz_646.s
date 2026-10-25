	.include "asm/macros/bit_width.inc"

	.include "MPlayDef.s"

/* Moemon Quetzal song 646, disassembled from the ROM by tools/quetzal_rip/songs.py.
   "org" comments give the ROM address of the bytes that follow. */

	.equ	mus_qz_646_grp, voicegroup_qz_songs
	.equ	mus_qz_646_pri, 0
	.equ	mus_qz_646_rev, reverb_set+0
	.equ	mus_qz_646_mvl, 127
	.equ	mus_qz_646_key, 0
	.equ	mus_qz_646_tbs, 1
	.equ	mus_qz_646_exg, 1
	.equ	mus_qz_646_cmp, 1

	.section .rodata
	.global	mus_qz_646
	.p2align	2
/* org 0x09693B8C */

/********************** Track  1 **********************/

mus_qz_646_1:
	.byte		KEYSH , mus_qz_646_key+0
	.byte		TEMPO , 162*mus_qz_646_tbs/2
	.byte		VOICE , 35
	.byte		VOL   , 78*mus_qz_646_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		VOL   , 61*mus_qz_646_mvl/mxv
	.byte		PRIO  , 64
	.byte		N04   , Bn4 , v092
	.byte	W06
	.byte		        Ds5 , v076
	.byte	W06
	.byte		        Fs5 , v084
	.byte	W06
	.byte		        As5 , v092
	.byte	W06
	.byte		N02   , Bn5 , v088
	.byte	W03
	.byte		        Cs6 , v076
	.byte	W03
	.byte		        Bn5 , v084
	.byte	W03
	.byte		        Cs6 , v076
	.byte	W03
	.byte		        Bn5 , v080
	.byte	W03
	.byte		        Cs6 , v072
	.byte	W03
	.byte		        Bn5 , v080
	.byte	W03
	.byte		        Cs6 , v064
	.byte	W03
	.byte		N03   , Bn5 , v072
	.byte	W04
	.byte		        Cs6 , v056
	.byte	W04
	.byte		        Bn5 , v064
	.byte	W04
	.byte		        Cs6 , v052
	.byte	W04
	.byte		        Bn5 , v056
	.byte	W04
	.byte		        Cs6 , v044
	.byte	W04
	.byte		        Bn5 , v052
	.byte	W04
	.byte		        Cs6 , v044
	.byte	W04
	.byte		        Bn5 , v048
	.byte	W04
	.byte		        Cs6 , v036
	.byte	W04
	.byte		        Bn5 , v044
	.byte	W04
	.byte		        Cs6 , v036
	.byte	W04
	.byte		N05   , Bn5
	.byte	W06
	.byte		        Cs6 , v028
	.byte	W06
	.byte		        Bn5 , v032
	.byte	W06
	.byte		        Cs6 , v028
	.byte	W06
	.byte		TEMPO , 162*mus_qz_646_tbs/2
	.byte	W48
	.byte		        Bn5 , v088
	.byte	W06
	.byte		        Cs6 , v092
	.byte	W06
	.byte		        Bn5 , v084
	.byte	W06
	.byte		        An5 , v080
	.byte	W06
	.byte		        Gs5 , v088
	.byte	W06
	.byte		        An5
	.byte	W06
	.byte		        Gs5 , v080
	.byte	W06
	.byte		        Fs5 , v084
	.byte	W06
	.byte		        En5 , v076
	.byte	W96
	.byte	W96
	.byte	W96
mus_qz_646_1_000:
	.byte	W60
	.byte		N10   , Gs4 , v088
	.byte		N10   , Bn4 , v100
	.byte	W12
	.byte		        An4 , v084
	.byte		N10   , Cs5 , v092
	.byte	W12
	.byte		        Bn4
	.byte		N10   , Ds5 , v104
	.byte	W12
	.byte	PEND
mus_qz_646_1_B1:
	.byte		N10   , Gs4 , v096
	.byte		N10   , En5 , v112
	.byte	W24
	.byte		        Bn4 , v084
	.byte		N10   , Ds5 , v104
	.byte	W12
	.byte		        En4 , v060
	.byte		N10   , Cs5 , v088
	.byte	W60
	.byte	W60
	.byte		        Gs4 , v096
	.byte		N10   , Bn4 , v108
	.byte	W12
	.byte		        Fs4 , v080
	.byte		N10   , An4 , v088
	.byte	W12
	.byte		        En4
	.byte		N10   , Gs4 , v096
	.byte	W12
	.byte		        Fs4
	.byte		N10   , An4 , v108
	.byte	W24
	.byte		N07   , Gs4 , v088
	.byte		N07   , Bn4 , v096
	.byte	W12
	.byte		        Gs4 , v076
	.byte		N07   , Bn4 , v084
	.byte	W60
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		N32   , An4 , v120 , gtp3
	.byte	W36
	.byte		N05   , Fs4 , v092
	.byte	W06
	.byte		        An4 , v100
	.byte	W06
	.byte		N44   , Ds5 , v112 , gtp3
	.byte	W30
	.byte		VOL   , 56*mus_qz_646_mvl/mxv
	.byte	W06
	.byte		        50*mus_qz_646_mvl/mxv
	.byte	W06
	.byte		        43*mus_qz_646_mvl/mxv
	.byte	W06
	.byte		        61*mus_qz_646_mvl/mxv
	.byte		N23   , En5 , v124
	.byte	W24
	.byte		        Ds5 , v116
	.byte	W24
	.byte		        Cs5 , v120
	.byte	W24
	.byte		        Cn5 , v112
	.byte	W24
	.byte		N32   , Bn4 , v116 , gtp3
	.byte	W36
	.byte		N05   , Gs4 , v104
	.byte	W06
	.byte		        Bn4 , v096
	.byte	W06
	.byte		N32   , En5 , v116 , gtp3
	.byte	W36
	.byte		N05   , Bn4
	.byte	W06
	.byte		        En5 , v108
	.byte	W06
	.byte		N64   , Gs5 , v120 , gtp1
	.byte	W48
	.byte		VOL   , 57*mus_qz_646_mvl/mxv
	.byte	W03
	.byte		        54*mus_qz_646_mvl/mxv
	.byte	W03
	.byte		        50*mus_qz_646_mvl/mxv
	.byte	W03
	.byte		        46*mus_qz_646_mvl/mxv
	.byte	W03
	.byte		        43*mus_qz_646_mvl/mxv
	.byte	W03
	.byte		        39*mus_qz_646_mvl/mxv
	.byte	W03
	.byte		        33*mus_qz_646_mvl/mxv
	.byte	W06
	.byte		        61*mus_qz_646_mvl/mxv
	.byte	W24
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		N32   , An4 , v112 , gtp3
	.byte	W36
	.byte		N05   , Fs4 , v108
	.byte	W06
	.byte		        An4 , v100
	.byte	W06
	.byte		N44   , Ds5 , v112 , gtp3
	.byte	W48
	.byte		N23   , En5 , v120
	.byte	W24
	.byte		        Ds5 , v108
	.byte	W24
	.byte		        Cs5 , v116
	.byte	W24
	.byte		        Cn5 , v108
	.byte	W24
	.byte		N32   , Bn4 , v112 , gtp3
	.byte	W36
	.byte		N05   , Gs4 , v104
	.byte	W06
	.byte		        Bn4 , v100
	.byte	W06
	.byte		N32   , En5 , v120 , gtp3
	.byte	W36
	.byte		N05   , Bn4 , v108
	.byte	W06
	.byte		        En5 , v104
	.byte	W06
	.byte		N44   , Gs5 , v124 , gtp3
	.byte	W24
	.byte		VOL   , 57*mus_qz_646_mvl/mxv
	.byte	W03
	.byte		        54*mus_qz_646_mvl/mxv
	.byte	W03
	.byte		        50*mus_qz_646_mvl/mxv
	.byte	W03
	.byte		        46*mus_qz_646_mvl/mxv
	.byte	W03
	.byte		        43*mus_qz_646_mvl/mxv
	.byte	W03
	.byte		        39*mus_qz_646_mvl/mxv
	.byte	W03
	.byte		        36*mus_qz_646_mvl/mxv
	.byte	W06
	.byte		        61*mus_qz_646_mvl/mxv
	.byte	W12
	.byte		N05   , Bn5 , v100
	.byte	W06
	.byte		        Cs6 , v108
	.byte	W06
	.byte		        Bn5 , v096
	.byte	W06
	.byte		        An5 , v092
	.byte	W06
	.byte		        Gs5 , v100
	.byte	W06
	.byte		        Fs5 , v104
	.byte	W06
	.byte		        En5 , v092
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_646_1_000
	.byte	GOTO
	 ptrvalue	mus_qz_646_1_B1
	.byte	FINE

/********************** Track  2 **********************/

mus_qz_646_2:
	.byte		KEYSH , mus_qz_646_key+0
	.byte		VOICE , 7
	.byte		VOL   , 72*mus_qz_646_mvl/mxv
	.byte		PAN   , c_v+46
	.byte		VOL   , 56*mus_qz_646_mvl/mxv
	.byte		PRIO  , 62
	.byte		N04   , Bn3 , v088
	.byte	W06
	.byte		        Ds4 , v072
	.byte	W06
	.byte		        Fs4 , v076
	.byte	W06
	.byte		        As4 , v084
	.byte	W06
	.byte		N02   , Bn4
	.byte	W03
	.byte		        Cs5 , v072
	.byte	W03
	.byte		        Bn4 , v076
	.byte	W03
	.byte		        Cs5 , v072
	.byte	W03
	.byte		        Bn4 , v076
	.byte	W03
	.byte		        Cs5 , v068
	.byte	W03
	.byte		        Bn4 , v072
	.byte	W03
	.byte		        Cs5 , v060
	.byte	W03
	.byte		N03   , Bn4 , v068
	.byte	W04
	.byte		        Cs5 , v052
	.byte	W04
	.byte		        Bn4 , v060
	.byte	W04
	.byte		        Cs5 , v048
	.byte	W04
	.byte		        Bn4 , v052
	.byte	W04
	.byte		        Cs5 , v040
	.byte	W04
	.byte		        Bn4 , v048
	.byte	W04
	.byte		        Cs5 , v036
	.byte	W04
	.byte		        Bn4 , v044
	.byte	W04
	.byte		        Cs5 , v032
	.byte	W04
	.byte		        Bn4 , v036
	.byte	W04
	.byte		        Cs5 , v032
	.byte	W04
	.byte		N05   , Bn4
	.byte	W06
	.byte		        Cs5 , v024
	.byte	W06
	.byte		        Bn4 , v028
	.byte	W06
	.byte		        Cs5 , v024
	.byte	W06
	.byte	W48
	.byte		        Bn4 , v080
	.byte	W06
	.byte		        Cs5 , v088
	.byte	W06
	.byte		        Bn4 , v080
	.byte	W06
	.byte		        An4 , v076
	.byte	W06
	.byte		        Gs4 , v080
	.byte	W06
	.byte		        An4 , v084
	.byte	W06
	.byte		        Gs4 , v076
	.byte	W06
	.byte		        Fs4 , v080
	.byte	W06
	.byte		        En4 , v072
	.byte	W60
	.byte		N04   , Bn3 , v088
	.byte	W12
	.byte		        Cs4 , v080
	.byte	W12
	.byte		        Ds4 , v088
	.byte	W12
	.byte		        En4 , v092
	.byte	W24
	.byte		        Ds4 , v080
	.byte	W12
	.byte		        Cs4 , v088
	.byte	W60
	.byte	W60
	.byte		        Bn3 , v092
	.byte	W12
	.byte		        An3 , v080
	.byte	W12
	.byte		        Gs3 , v084
	.byte	W12
	.byte		        An3 , v092
	.byte	W24
	.byte		        Bn3 , v084
	.byte	W12
	.byte	N04
	.byte	W60
mus_qz_646_2_B1:
	.byte	W60
	.byte		N04   , Bn3 , v080
	.byte	W12
	.byte		        Cs4 , v072
	.byte	W12
	.byte		        Ds4 , v080
	.byte	W12
	.byte		        En4 , v084
	.byte	W24
	.byte		        Ds4 , v064
	.byte	W12
	.byte		        Cs4 , v076
	.byte	W60
	.byte	W60
	.byte		        Bn3 , v080
	.byte	W12
	.byte		        An3 , v068
	.byte	W12
	.byte		        Gs3 , v080
	.byte	W12
	.byte		        An3 , v088
	.byte	W24
	.byte		        Bn3 , v072
	.byte	W12
	.byte		        Bn3 , v084
	.byte	W60
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		N05   , Bn3 , v072
	.byte	W06
	.byte		        Bn3 , v052
	.byte	W06
	.byte		N03   , Bn3 , v060
	.byte	W04
	.byte		        Bn3 , v048
	.byte	W04
	.byte		        Bn3 , v052
	.byte	W04
	.byte		        Bn3 , v032
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		N01   , Bn3 , v036
	.byte	W04
	.byte		N05   , Gs3 , v072
	.byte	W06
	.byte		        Bn3 , v056
	.byte	W06
	.byte		        En4 , v072
	.byte	W06
	.byte		        En4 , v052
	.byte	W06
	.byte		N03   , En4 , v060
	.byte	W04
	.byte		        En4 , v048
	.byte	W04
	.byte		        En4 , v052
	.byte	W04
	.byte		        En4 , v032
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		N01   , En4 , v036
	.byte	W02
	.byte		        En4 , v028
	.byte	W02
	.byte		        En4 , v032
	.byte	W02
	.byte		        En4 , v020
	.byte	W10
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		N05   , Bn3 , v072
	.byte	W06
	.byte		        Bn3 , v052
	.byte	W06
	.byte		N03   , Bn3 , v060
	.byte	W04
	.byte		        Bn3 , v048
	.byte	W04
	.byte		        Bn3 , v052
	.byte	W04
	.byte		        Bn3 , v032
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		N01   , Bn3 , v036
	.byte	W04
	.byte		N05   , Gn3 , v072
	.byte	W06
	.byte		        Bn3 , v056
	.byte	W06
	.byte		        En4 , v072
	.byte	W06
	.byte		        En4 , v052
	.byte	W06
	.byte		N03   , En4 , v060
	.byte	W04
	.byte		        En4 , v048
	.byte	W04
	.byte		        En4 , v052
	.byte	W04
	.byte		        En4 , v032
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		N01   , En4 , v036
	.byte	W02
	.byte		        En4 , v028
	.byte	W02
	.byte		        En4 , v032
	.byte	W02
	.byte		        En4 , v020
	.byte	W10
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W60
	.byte		N04   , Bn2 , v088
	.byte	W12
	.byte		        Cs3 , v080
	.byte	W12
	.byte		        Ds3 , v088
	.byte	W12
	.byte		        En3 , v092
	.byte	W24
	.byte		        Ds3 , v080
	.byte	W12
	.byte		        Cs3 , v088
	.byte	W60
	.byte	W60
	.byte		        Bn2 , v092
	.byte	W12
	.byte		        An2 , v080
	.byte	W12
	.byte		        Gs2 , v084
	.byte	W12
	.byte		        An2 , v092
	.byte	W24
	.byte		        Bn2 , v084
	.byte	W12
	.byte	N04
	.byte	W60
	.byte	GOTO
	 ptrvalue	mus_qz_646_2_B1
	.byte	FINE

/********************** Track  3 **********************/

mus_qz_646_3:
	.byte		KEYSH , mus_qz_646_key+0
	.byte		VOICE , 19
	.byte		VOL   , 97*mus_qz_646_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		VOL   , 76*mus_qz_646_mvl/mxv
	.byte		PRIO  , 60
	.byte	W24
	.byte		N11   , En1 , v116
	.byte	W24
	.byte		N05   , En2 , v108
	.byte	W12
	.byte		N11   , An1
	.byte	W24
	.byte		        Bn0 , v120
	.byte	W12
	.byte		N05   , An1 , v096
	.byte	W12
	.byte		N11   , En1 , v108
	.byte	W12
	.byte		        En1 , v112
	.byte	W24
	.byte		N05   , En2 , v100
	.byte	W12
	.byte		N11   , Gs1 , v108
	.byte	W24
	.byte		        Bn0 , v112
	.byte	W12
	.byte		N05   , Gs1 , v100
	.byte	W12
	.byte		N11   , En1 , v108
	.byte	W12
mus_qz_646_3_000:
	.byte		N11   , En1 , v120
	.byte	W24
	.byte		N05   , En2 , v100
	.byte	W12
	.byte		N11   , Gs1 , v108
	.byte	W24
	.byte		        Bn0 , v112
	.byte	W12
	.byte		N05   , Bn1 , v104
	.byte	W12
	.byte		N11   , En1 , v112
	.byte	W12
	.byte	PEND
mus_qz_646_3_001:
	.byte		N11   , En1 , v120
	.byte	W24
	.byte		N05   , En2 , v100
	.byte	W12
	.byte		N11   , Gs1 , v112
	.byte	W24
	.byte		        Bn0 , v100
	.byte	W12
	.byte		N05   , Bn1 , v112
	.byte	W12
	.byte		N11   , En1 , v108
	.byte	W12
	.byte	PEND
mus_qz_646_3_002:
	.byte		N11   , Bn0 , v116
	.byte	W24
	.byte		N05   , Ds2 , v104
	.byte	W12
	.byte		N11   , Ds1 , v108
	.byte	W24
	.byte		        Fs1 , v112
	.byte	W12
	.byte		N05   , Bn1 , v100
	.byte	W12
	.byte		N11   , Ds1 , v108
	.byte	W12
	.byte	PEND
mus_qz_646_3_003:
	.byte		N11   , Bn0 , v116
	.byte	W24
	.byte		N05   , Ds2 , v100
	.byte	W12
	.byte		N11   , Ds1 , v108
	.byte	W24
	.byte		        Fs1 , v100
	.byte	W12
	.byte		N05   , Bn1 , v112
	.byte	W12
	.byte		N11   , Ds1 , v104
	.byte	W12
	.byte	PEND
mus_qz_646_3_B1:
	.byte		N11   , En1 , v116
	.byte	W24
	.byte		N05   , En2 , v100
	.byte	W12
	.byte		N11   , Gs1 , v108
	.byte	W24
	.byte		        Bn0 , v100
	.byte	W12
	.byte		N05   , Bn1 , v112
	.byte	W12
	.byte		N11   , En1 , v108
	.byte	W12
	.byte		        En1 , v116
	.byte	W24
	.byte		N05   , En2 , v104
	.byte	W12
	.byte		N11   , Gs1 , v112
	.byte	W24
	.byte		        Bn0 , v108
	.byte	W12
	.byte		N05   , Bn1 , v100
	.byte	W12
	.byte		N11   , En1 , v108
	.byte	W12
	.byte		        Bn0 , v116
	.byte	W24
	.byte		N05   , Ds2 , v104
	.byte	W12
	.byte		N11   , Ds1 , v112
	.byte	W24
	.byte		        Fs1 , v108
	.byte	W12
	.byte		N05   , Bn1 , v112
	.byte	W12
	.byte		N11   , Ds1 , v104
	.byte	W12
	.byte		        Bn0 , v120
	.byte	W24
	.byte		N05   , Ds2 , v108
	.byte	W12
	.byte		N11   , Ds1 , v116
	.byte	W24
	.byte		        Fs1 , v104
	.byte	W12
	.byte		N05   , Bn1 , v112
	.byte	W12
	.byte		N11   , Ds1 , v104
	.byte	W12
	.byte		        An0 , v112
	.byte	W24
	.byte		N05   , An1 , v104
	.byte	W12
	.byte		N11   , An0 , v112
	.byte	W24
	.byte		        En1
	.byte	W12
	.byte		N05   , An1 , v104
	.byte	W12
	.byte		N11   , Cs1 , v112
	.byte	W12
	.byte		        An0 , v116
	.byte	W24
	.byte		N05   , An1 , v100
	.byte	W12
	.byte		N11   , An0 , v112
	.byte	W24
	.byte		        Fs1
	.byte	W12
	.byte		N05   , An1 , v104
	.byte	W12
	.byte		N11   , Cs1 , v116
	.byte	W12
	.byte		        Gs0 , v124
	.byte	W24
	.byte		N05   , Gs1 , v100
	.byte	W12
	.byte		N11   , Gs0 , v112
	.byte	W24
	.byte		        Ds1 , v100
	.byte	W12
	.byte		        Gs1 , v116
	.byte	W12
	.byte		        Bn0 , v104
	.byte	W12
	.byte		        Cs1 , v112
	.byte	W24
	.byte		N05   , Gs1 , v104
	.byte	W12
	.byte		N11   , Cs1 , v112
	.byte	W36
	.byte		N23   , Gn1
	.byte	W24
	.byte		N11   , Bn0 , v124
	.byte	W24
	.byte		N05   , Bn1 , v100
	.byte	W12
	.byte		N11   , Bn0 , v112
	.byte	W24
	.byte		        Fs1 , v100
	.byte	W12
	.byte		N05   , Bn1 , v112
	.byte	W12
	.byte		N11   , Ds1 , v100
	.byte	W12
	.byte		        Bn0 , v112
	.byte	W24
	.byte		N05   , Bn1 , v096
	.byte	W12
	.byte		N11   , Bn0 , v108
	.byte	W24
	.byte		        Fs1 , v104
	.byte	W12
	.byte		N05   , Bn1 , v112
	.byte	W12
	.byte		N11   , Ds1 , v100
	.byte	W12
	.byte		        Gs0 , v124
	.byte	W24
	.byte		N05   , Gs1 , v100
	.byte	W12
	.byte		N11   , Gs0 , v112
	.byte	W24
	.byte		        En1 , v100
	.byte	W12
	.byte		N05   , Gs1 , v116
	.byte	W12
	.byte		N11   , Bn0 , v104
	.byte	W12
	.byte		        En1 , v120
	.byte	W24
	.byte		N05   , En1 , v104
	.byte	W12
	.byte		N11   , Bn0 , v112
	.byte	W24
	.byte		        En1 , v116
	.byte	W12
	.byte		N05   , Gs1 , v120
	.byte	W12
	.byte		N11   , Bn0 , v100
	.byte	W12
	.byte		        An0 , v124
	.byte	W24
	.byte		N05   , An1 , v108
	.byte	W12
	.byte		N11   , An0 , v112
	.byte	W24
	.byte		        En1 , v100
	.byte	W12
	.byte		N05   , An1 , v112
	.byte	W12
	.byte		N11   , Cs1 , v100
	.byte	W12
	.byte		        An0 , v124
	.byte	W24
	.byte		N05   , An1 , v100
	.byte	W12
	.byte		N11   , An0 , v108
	.byte	W24
	.byte		        En1 , v112
	.byte	W12
	.byte		N05   , An1 , v100
	.byte	W12
	.byte		N11   , Cs1 , v112
	.byte	W12
	.byte		        Gs0 , v120
	.byte	W24
	.byte		N05   , Gs1 , v100
	.byte	W12
	.byte		N11   , Gs0 , v112
	.byte	W24
	.byte		        En1 , v100
	.byte	W12
	.byte		N05   , Gs1 , v112
	.byte	W12
	.byte		N11   , Bn0 , v100
	.byte	W12
	.byte		        Gn0 , v124
	.byte	W24
	.byte		N05   , En1 , v096
	.byte	W12
	.byte		N11   , Gn0 , v108
	.byte	W24
	.byte		        Gn0 , v100
	.byte	W12
	.byte		N05   , En1 , v112
	.byte	W12
	.byte		N11   , Bn0 , v104
	.byte	W12
	.byte		        Fs0 , v112
	.byte	W24
	.byte		N05   , Fs1 , v100
	.byte	W12
	.byte		N11   , Cs1 , v112
	.byte	W24
	.byte		        Fs1 , v096
	.byte	W12
	.byte		N05   , An1 , v108
	.byte	W12
	.byte		N11   , Cs1 , v096
	.byte	W12
	.byte		        Bn0 , v120
	.byte	W24
	.byte		N05   , Bn1 , v096
	.byte	W12
	.byte		N11   , Bn0 , v108
	.byte	W24
	.byte		        Fs1 , v100
	.byte	W12
	.byte		N05   , Bn1 , v112
	.byte	W12
	.byte		N11   , Ds1 , v100
	.byte	W12
	.byte		        En1 , v120
	.byte	W24
	.byte		N05   , Gs1 , v096
	.byte	W12
	.byte		N11   , En1 , v112
	.byte	W24
	.byte		        En1 , v120
	.byte	W12
	.byte		N05   , Gs1 , v100
	.byte	W12
	.byte		N11   , Bn0 , v108
	.byte	W12
	.byte		        En0 , v120
	.byte	W24
	.byte		N05   , Gs1 , v096
	.byte	W12
	.byte		N11   , En0 , v124
	.byte	W24
	.byte		        En1 , v100
	.byte	W12
	.byte		N05   , Gs1 , v116
	.byte	W12
	.byte		N11   , Bn0 , v100
	.byte	W12
	.byte		N10   , En1 , v120
	.byte	W24
	.byte		        En1 , v100
	.byte	W36
	.byte		        En1 , v120
	.byte	W12
	.byte		        En1 , v108
	.byte	W24
	.byte		        En1 , v112
	.byte	W12
	.byte		        En1 , v104
	.byte	W12
	.byte		        En1 , v112
	.byte	W12
	.byte		        En1 , v120
	.byte	W60
	.byte	PATT
	 ptrvalue	mus_qz_646_3_000
	.byte	PATT
	 ptrvalue	mus_qz_646_3_001
	.byte	PATT
	 ptrvalue	mus_qz_646_3_002
	.byte	PATT
	 ptrvalue	mus_qz_646_3_003
	.byte	GOTO
	 ptrvalue	mus_qz_646_3_B1
	.byte	FINE

/********************** Track  4 **********************/

mus_qz_646_4:
	.byte		KEYSH , mus_qz_646_key+0
	.byte		VOICE , 39
	.byte		VOL   , 91*mus_qz_646_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		VOL   , 70*mus_qz_646_mvl/mxv
	.byte		PRIO  , 40
	.byte	W24
	.byte		N11   , Cn1 , v112
	.byte		N28   , Dn2 , v108 , gtp1
	.byte	W24
	.byte		N11   , Ds1 , v084
	.byte	W12
	.byte		        Cn1 , v100
	.byte		N11   , As1 , v072
	.byte	W24
	.byte		        Ds1 , v088
	.byte		N02   , Fs1 , v060
	.byte	W12
	.byte		N11   , Cn1 , v112
	.byte		N02   , Fs1 , v060
	.byte	W12
	.byte		N11   , Cn1 , v100
	.byte		N02   , Fs1 , v060
	.byte	W11
	.byte		N11   , Cn1 , v112
	.byte	W01
	.byte	W11
	.byte		N02   , Fs1 , v060
	.byte	W12
	.byte		N11   , Ds1 , v084
	.byte		N02   , Fs1 , v060
	.byte	W12
	.byte		N11   , Cn1 , v100
	.byte		N11   , As1 , v072
	.byte	W24
	.byte		        Cn1 , v112
	.byte	W01
	.byte		        Ds1 , v076
	.byte	W11
	.byte		        Ds1 , v088
	.byte		N02   , Fs1 , v060
	.byte	W12
	.byte		N11   , Cn1 , v100
	.byte	W01
	.byte		        Ds1 , v072
	.byte	W12
	.byte		        Cn1 , v112
	.byte		N28   , Ds2 , v088 , gtp1
	.byte	W24
	.byte		N11   , Ds1 , v084
	.byte	W12
	.byte		        Cn1 , v100
	.byte		N11   , As1 , v072
	.byte	W24
	.byte		        Cn1 , v112
	.byte		N02   , Fs1 , v060
	.byte	W12
	.byte		N11   , Ds1 , v088
	.byte		N02   , Fs1 , v060
	.byte	W12
	.byte		N11   , Cn1 , v100
	.byte		N02   , Fs1 , v060
	.byte	W11
	.byte		N11   , Cn1 , v112
	.byte	W01
mus_qz_646_4_000:
	.byte	W11
	.byte		N02   , Fs1 , v060
	.byte	W12
	.byte		N11   , Ds1 , v084
	.byte		N02   , Fs1 , v060
	.byte	W12
	.byte		N11   , Cn1 , v100
	.byte		N11   , As1 , v072
	.byte	W24
	.byte		        Cn1 , v112
	.byte	W01
	.byte		        Ds1 , v088
	.byte	W11
	.byte		N02   , Fs1 , v060
	.byte	W12
	.byte		N11   , Cn1 , v100
	.byte	W01
	.byte		        Ds1 , v072
	.byte	W12
	.byte	PEND
mus_qz_646_4_001:
	.byte		N11   , Cn1 , v112
	.byte	W24
	.byte		        Ds1 , v084
	.byte		N02   , Fs1 , v060
	.byte	W12
	.byte		N11   , Cn1 , v100
	.byte		N11   , As1 , v072
	.byte	W24
	.byte		        Cn1 , v112
	.byte		N02   , Fs1 , v060
	.byte	W12
	.byte		N11   , Ds1 , v088
	.byte		N02   , Fs1 , v060
	.byte	W12
	.byte		N11   , Cn1 , v100
	.byte		N02   , Fs1 , v060
	.byte	W11
	.byte		N11   , Cn1 , v112
	.byte	W01
	.byte	PEND
mus_qz_646_4_002:
	.byte	W11
	.byte		N02   , Fs1 , v060
	.byte	W12
	.byte		N11   , Ds1 , v084
	.byte		N02   , Fs1 , v060
	.byte	W12
	.byte		N11   , Cn1 , v100
	.byte		N11   , As1 , v072
	.byte	W24
	.byte		        Cn1 , v112
	.byte	W01
	.byte		        Ds1 , v088
	.byte	W11
	.byte		N02   , Fs1 , v060
	.byte	W01
	.byte		N11   , Ds1 , v072
	.byte	W11
	.byte		        Cn1 , v100
	.byte	W01
	.byte		N02   , Fs1 , v060
	.byte	W12
	.byte	PEND
mus_qz_646_4_B1:
	.byte		N11   , Cn1 , v112
	.byte		N28   , As2 , v076 , gtp1
	.byte	W24
	.byte		N11   , Ds1 , v084
	.byte	W12
	.byte		        Cn1 , v100
	.byte		N11   , As1 , v072
	.byte	W24
	.byte		        Cn1 , v112
	.byte		N02   , Fs1 , v060
	.byte	W12
	.byte		N11   , Ds1 , v088
	.byte		N02   , Fs1 , v060
	.byte	W12
	.byte		N11   , Cn1 , v100
	.byte		N02   , Fs1 , v060
	.byte	W11
	.byte		N11   , Cn1 , v112
	.byte	W01
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_646_4_000
	.byte		N11   , Cn1 , v112
	.byte		N28   , En2 , v076 , gtp1
	.byte	W24
	.byte		N11   , Ds1 , v084
	.byte	W12
	.byte		        Cn1 , v100
	.byte		N11   , As1 , v072
	.byte	W24
	.byte		        Cn1 , v112
	.byte		N02   , Fs1 , v060
	.byte	W12
	.byte		N11   , Ds1 , v088
	.byte		N02   , Fs1 , v060
	.byte	W12
	.byte		N11   , Cn1 , v100
	.byte		N02   , Fs1 , v060
	.byte	W11
	.byte		N11   , Cn1 , v112
	.byte	W01
	.byte	W11
	.byte		N02   , Fs1 , v060
	.byte	W12
	.byte		N11   , Ds1 , v084
	.byte		N02   , Fs1 , v060
	.byte	W12
	.byte		N11   , Cn1 , v100
	.byte		N11   , As1 , v072
	.byte	W13
	.byte		N05   , Ds1 , v088
	.byte	W06
	.byte		        Ds1 , v064
	.byte	W05
	.byte		N11   , Cn1 , v112
	.byte	W01
	.byte		N05   , Ds1 , v072
	.byte	W06
	.byte		        Ds1 , v052
	.byte	W05
	.byte		N11   , Ds1 , v088
	.byte	W12
	.byte		        Cn1 , v100
	.byte	W01
	.byte		        Ds1 , v072
	.byte	W11
	.byte		N28   , As2 , v076 , gtp1
	.byte	W01
mus_qz_646_4_003:
	.byte		N11   , Cn1 , v112
	.byte	W24
	.byte		        Ds1 , v084
	.byte	W12
	.byte		        Cn1 , v100
	.byte		N11   , As1 , v072
	.byte	W24
	.byte		        Cn1 , v112
	.byte		N02   , Fs1 , v060
	.byte	W12
	.byte		N11   , Ds1 , v088
	.byte		N02   , Fs1 , v060
	.byte	W12
	.byte		N11   , Cn1 , v100
	.byte		N02   , Fs1 , v060
	.byte	W11
	.byte		N11   , Cn1 , v112
	.byte	W01
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_646_4_000
	.byte	PATT
	 ptrvalue	mus_qz_646_4_001
mus_qz_646_4_004:
	.byte	W11
	.byte		N02   , Fs1 , v060
	.byte	W12
	.byte		N11   , Ds1 , v084
	.byte		N02   , Fs1 , v060
	.byte	W12
	.byte		N11   , Cn1 , v100
	.byte		N11   , As1 , v072
	.byte	W24
	.byte		        Cn1 , v112
	.byte		N02   , Fs1 , v060
	.byte	W12
	.byte		N11   , Ds1 , v088
	.byte		N02   , Fs1 , v060
	.byte	W12
	.byte		N11   , Cn1 , v100
	.byte	W01
	.byte		        Ds1 , v072
	.byte	W12
	.byte	PEND
mus_qz_646_4_005:
	.byte		N11   , Cn1 , v112
	.byte	W12
	.byte		N02   , Fs1 , v060
	.byte	W12
	.byte		N11   , Ds1 , v084
	.byte		N02   , Fs1 , v060
	.byte	W12
	.byte		N11   , Cn1 , v100
	.byte		N11   , As1 , v072
	.byte	W24
	.byte		        Cn1 , v112
	.byte		N02   , Fs1 , v060
	.byte	W12
	.byte		N11   , Ds1 , v088
	.byte		N02   , Fs1 , v060
	.byte	W12
	.byte		N11   , Cn1 , v100
	.byte		N02   , Fs1 , v060
	.byte	W11
	.byte		N11   , Cn1 , v112
	.byte	W01
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_646_4_000
	.byte	PATT
	 ptrvalue	mus_qz_646_4_B1
	.byte		N28   , En2 , v076 , gtp1
	.byte	W23
	.byte		N11   , Ds1 , v084
	.byte	W12
	.byte		        Cn1 , v100
	.byte		N11   , As1 , v072
	.byte	W13
	.byte		N05   , Ds1 , v088
	.byte	W06
	.byte		        Ds1 , v064
	.byte	W05
	.byte		N11   , Cn1 , v112
	.byte		N02   , Fs1 , v060
	.byte	W01
	.byte		N05   , Ds1 , v072
	.byte	W06
	.byte		        Ds1 , v052
	.byte	W05
	.byte		N11   , Ds1 , v088
	.byte		N02   , Fs1 , v060
	.byte	W12
	.byte		N11   , Cn1 , v100
	.byte	W01
	.byte		        Ds1 , v072
	.byte	W11
	.byte		N28   , As2 , v076 , gtp1
	.byte	W01
	.byte	PATT
	 ptrvalue	mus_qz_646_4_003
	.byte	PATT
	 ptrvalue	mus_qz_646_4_000
	.byte	PATT
	 ptrvalue	mus_qz_646_4_001
	.byte	PATT
	 ptrvalue	mus_qz_646_4_004
mus_qz_646_4_006:
	.byte		N11   , Cn1 , v112
	.byte		N28   , Ds2 , v076 , gtp1
	.byte	W24
	.byte		N11   , Ds1 , v084
	.byte	W12
	.byte		        Cn1 , v100
	.byte		N11   , As1 , v072
	.byte	W24
	.byte		        Cn1 , v112
	.byte		N02   , Fs1 , v060
	.byte	W12
	.byte		N11   , Ds1 , v088
	.byte		N02   , Fs1 , v060
	.byte	W12
	.byte		N11   , Cn1 , v100
	.byte		N02   , Fs1 , v060
	.byte	W11
	.byte		N11   , Cn1 , v112
	.byte	W01
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_646_4_000
	.byte	PATT
	 ptrvalue	mus_qz_646_4_005
	.byte	W11
	.byte		N02   , Fs1 , v060
	.byte	W12
	.byte		N11   , Ds1 , v084
	.byte		N02   , Fs1 , v060
	.byte	W12
	.byte		N11   , Cn1 , v100
	.byte		N11   , As1 , v072
	.byte	W24
	.byte		        Cn1 , v112
	.byte	W01
	.byte		        Ds1 , v088
	.byte	W12
	.byte		        Cn1 , v100
	.byte		N11   , Ds1 , v076
	.byte	W11
	.byte		        Cn1 , v100
	.byte	W01
	.byte		        Ds1 , v072
	.byte	W12
	.byte		        Cn1 , v120
	.byte		N11   , Ds1 , v092
	.byte	W24
	.byte		        Cn1 , v112
	.byte		N11   , Ds1 , v088
	.byte	W36
	.byte		        Cn1 , v120
	.byte		N11   , Ds1 , v092
	.byte	W12
	.byte		        Cn1 , v112
	.byte		N11   , Ds1 , v088
	.byte	W24
	.byte		        Cn1 , v120
	.byte		N11   , Ds1 , v092
	.byte	W12
	.byte		        Cn1 , v112
	.byte		N11   , Ds1 , v088
	.byte	W12
	.byte		        Cn1 , v120
	.byte		N11   , Ds1 , v092
	.byte	W12
	.byte		        Cn1 , v112
	.byte		N11   , Ds1 , v088
	.byte	W60
	.byte	PATT
	 ptrvalue	mus_qz_646_4_006
	.byte	PATT
	 ptrvalue	mus_qz_646_4_000
	.byte	PATT
	 ptrvalue	mus_qz_646_4_001
	.byte	PATT
	 ptrvalue	mus_qz_646_4_002
	.byte	GOTO
	 ptrvalue	mus_qz_646_4_B1
	.byte	FINE

/********************** Track  5 **********************/

mus_qz_646_5:
	.byte		KEYSH , mus_qz_646_key+0
	.byte		VOICE , 34
	.byte		VOL   , 85*mus_qz_646_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		VOL   , 66*mus_qz_646_mvl/mxv
	.byte		PRIO  , 58
	.byte	W24
	.byte	W96
mus_qz_646_5_000:
	.byte	W60
	.byte		N10   , En3 , v088
	.byte		N10   , Bn3 , v100
	.byte	W12
	.byte		        En3 , v076
	.byte		N10   , Cs4 , v084
	.byte	W12
	.byte		        Fs3
	.byte		N10   , Ds4 , v092
	.byte	W12
	.byte	PEND
mus_qz_646_5_001:
	.byte		N10   , Gs3 , v092
	.byte		N10   , En4 , v104
	.byte	W24
	.byte		        Fs3 , v088
	.byte		N10   , Ds4 , v096
	.byte	W12
	.byte		        En3 , v076
	.byte		N10   , Cs4 , v084
	.byte	W60
	.byte	PEND
mus_qz_646_5_002:
	.byte	W60
	.byte		N10   , En3 , v088
	.byte		N10   , Bn3 , v100
	.byte	W12
	.byte		        Ds3 , v076
	.byte		N10   , An3 , v084
	.byte	W12
	.byte		        Dn3
	.byte		N10   , Gs3 , v092
	.byte	W12
	.byte	PEND
mus_qz_646_5_003:
	.byte		N10   , Ds3 , v092
	.byte		N10   , An3 , v104
	.byte	W24
	.byte		N06   , Ds3 , v088
	.byte		N06   , Bn3 , v096
	.byte	W12
	.byte		        Ds3 , v076
	.byte		N06   , Bn3 , v084
	.byte	W60
	.byte	PEND
	.byte	W96
mus_qz_646_5_B1:
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		N32   , Cs4 , v108 , gtp3
	.byte	W36
	.byte		N05   , An3 , v076
	.byte	W06
	.byte		        Cs4 , v092
	.byte	W06
	.byte		N28   , En4 , v116 , gtp1
	.byte	W36
	.byte		N05   , Cs4 , v084
	.byte	W06
	.byte		        En4 , v096
	.byte	W06
	.byte		N23   , Fs4 , v116
	.byte	W24
	.byte		        En4 , v104
	.byte	W24
	.byte		        Ds4 , v112
	.byte	W24
	.byte		        Cs4 , v104
	.byte	W24
	.byte		N32   , Bn3 , v112 , gtp3
	.byte	W36
	.byte		N05   , Gs3 , v100
	.byte	W06
	.byte		        Bn3 , v088
	.byte	W06
	.byte		N40   , En4 , v112 , gtp1
	.byte	W30
	.byte		VOL   , 61*mus_qz_646_mvl/mxv
	.byte	W03
	.byte		        56*mus_qz_646_mvl/mxv
	.byte	W03
	.byte		        51*mus_qz_646_mvl/mxv
	.byte	W03
	.byte		        47*mus_qz_646_mvl/mxv
	.byte	W03
	.byte		        42*mus_qz_646_mvl/mxv
	.byte	W06
	.byte		        66*mus_qz_646_mvl/mxv
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		N32   , Cs4 , v112 , gtp3
	.byte	W36
	.byte		N05   , An3 , v092
	.byte	W06
	.byte		        Cs4 , v100
	.byte	W06
	.byte		N28   , En4 , v116 , gtp1
	.byte	W36
	.byte		N05   , Cs4 , v096
	.byte	W06
	.byte		        En4 , v100
	.byte	W06
	.byte		N23   , Fs4 , v120
	.byte	W24
	.byte		        En4 , v108
	.byte	W24
	.byte		        Ds4 , v120
	.byte	W24
	.byte		        Cs4 , v112
	.byte	W24
	.byte		N32   , Bn3 , v120 , gtp3
	.byte	W36
	.byte		N05   , Gs3 , v108
	.byte	W06
	.byte		        Bn3 , v100
	.byte	W06
	.byte		N40   , En4 , v116 , gtp1
	.byte	W24
	.byte		VOL   , 61*mus_qz_646_mvl/mxv
	.byte	W03
	.byte		        58*mus_qz_646_mvl/mxv
	.byte	W03
	.byte		        53*mus_qz_646_mvl/mxv
	.byte	W03
	.byte		        50*mus_qz_646_mvl/mxv
	.byte	W03
	.byte		        46*mus_qz_646_mvl/mxv
	.byte	W03
	.byte		        42*mus_qz_646_mvl/mxv
	.byte	W03
	.byte		        38*mus_qz_646_mvl/mxv
	.byte	W06
	.byte		        66*mus_qz_646_mvl/mxv
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_646_5_000
	.byte	PATT
	 ptrvalue	mus_qz_646_5_001
	.byte	PATT
	 ptrvalue	mus_qz_646_5_002
	.byte	PATT
	 ptrvalue	mus_qz_646_5_003
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_646_5_B1
	.byte	FINE

/********************** Track  6 **********************/

mus_qz_646_6:
	.byte		KEYSH , mus_qz_646_key+0
	.byte		VOICE , 23
	.byte		VOL   , 64*mus_qz_646_mvl/mxv
	.byte		PAN   , c_v-38
	.byte		VOL   , 50*mus_qz_646_mvl/mxv
	.byte		PRIO  , 56
	.byte		N05   , Fs2 , v080
	.byte	W06
	.byte		        Bn2 , v064
	.byte	W06
	.byte		        Ds3 , v072
	.byte	W06
	.byte		        Fs3 , v064
	.byte	W06
	.byte		N92   , En3 , v076 , gtp2
	.byte		N92   , An3 , v084 , gtp2
	.byte	W78
	.byte		VOL   , 46*mus_qz_646_mvl/mxv
	.byte	W03
	.byte		        43*mus_qz_646_mvl/mxv
	.byte	W03
	.byte		        39*mus_qz_646_mvl/mxv
	.byte	W03
	.byte		        37*mus_qz_646_mvl/mxv
	.byte	W03
	.byte		        34*mus_qz_646_mvl/mxv
	.byte	W03
	.byte		        25*mus_qz_646_mvl/mxv
	.byte	W03
	.byte		        50*mus_qz_646_mvl/mxv
	.byte		N92   , Bn2 , v068 , gtp2
	.byte		N92   , Gs3 , v072 , gtp2
	.byte	W84
	.byte		VOL   , 42*mus_qz_646_mvl/mxv
	.byte	W03
	.byte		        35*mus_qz_646_mvl/mxv
	.byte	W03
	.byte		        28*mus_qz_646_mvl/mxv
	.byte	W03
	.byte		        21*mus_qz_646_mvl/mxv
	.byte	W03
	.byte		        50*mus_qz_646_mvl/mxv
	.byte		N05   , Bn2 , v096
	.byte		N05   , Gs3 , v100
	.byte	W36
	.byte		        Ds3 , v088
	.byte		N05   , Bn3 , v096
	.byte	W60
mus_qz_646_6_000:
	.byte		N05   , Ds3 , v096
	.byte		N05   , Gs3 , v100
	.byte	W36
	.byte		        Ds3 , v088
	.byte		N05   , Bn3 , v096
	.byte	W60
	.byte	PEND
mus_qz_646_6_001:
	.byte		N05   , Bn2 , v096
	.byte		N05   , Fs3 , v100
	.byte	W36
	.byte		        Ds3 , v088
	.byte		N05   , Bn3 , v096
	.byte	W60
	.byte	PEND
mus_qz_646_6_002:
	.byte		N05   , Ds3 , v096
	.byte		N05   , An3 , v100
	.byte	W36
	.byte		        Ds3 , v088
	.byte		N05   , Bn3 , v096
	.byte	W60
	.byte	PEND
mus_qz_646_6_B1:
	.byte		N05   , Bn2 , v096
	.byte		N05   , Gs3 , v100
	.byte	W36
	.byte		        Ds3 , v088
	.byte		N05   , Bn3 , v096
	.byte	W36
	.byte		        Bn2 , v088
	.byte		N05   , An3 , v096
	.byte	W24
	.byte		        Bn2
	.byte		N05   , Gs3 , v100
	.byte	W36
	.byte		        Ds3 , v088
	.byte		N05   , Bn3 , v096
	.byte	W36
	.byte		        Ds3 , v088
	.byte		N05   , An3 , v096
	.byte	W24
	.byte		        Bn2
	.byte		N05   , Fs3 , v100
	.byte	W36
	.byte		        Ds3 , v088
	.byte		N05   , Bn3 , v096
	.byte	W36
	.byte		        Ds3 , v088
	.byte		N05   , An3 , v096
	.byte	W24
	.byte		        Bn2
	.byte		N05   , Fs3 , v100
	.byte	W36
	.byte		        Ds3 , v088
	.byte		N05   , Bn3 , v096
	.byte	W36
	.byte		        Bn2 , v088
	.byte		N05   , An3 , v096
	.byte	W24
	.byte		N68   , En3 , v080 , gtp3
	.byte	W72
	.byte		N23   , Gs3 , v072
	.byte	W24
	.byte		N68   , Fs3 , v084 , gtp3
	.byte	W72
	.byte		N23   , Bn3 , v072
	.byte	W24
	.byte		N68   , Gs3 , v088 , gtp3
	.byte	W72
	.byte		N23   , En4 , v080
	.byte	W24
	.byte		        Ds4 , v072
	.byte	W24
	.byte		        Bn3 , v068
	.byte	W24
	.byte		        Gn3 , v072
	.byte	W24
	.byte		        Bn3 , v064
	.byte	W24
	.byte		N56   , An3 , v072 , gtp3
	.byte	W60
	.byte		N11   , Fs3 , v068
	.byte	W12
	.byte		        Gs3 , v060
	.byte	W12
	.byte		        An3 , v084
	.byte	W12
	.byte		N44   , Ds3 , v076 , gtp3
	.byte	W48
	.byte		N23   , Fs3 , v084
	.byte	W24
	.byte		        An3 , v072
	.byte	W24
	.byte		N32   , En3 , v072 , gtp3
	.byte	W36
	.byte		N05   , En3 , v076
	.byte	W06
	.byte		        Fs3 , v064
	.byte	W06
	.byte		N32   , Gs3 , v068 , gtp3
	.byte	W36
	.byte		N05   , An3 , v072
	.byte	W06
	.byte		N17   , Bn3 , v056
	.byte	W06
	.byte	W12
	.byte		N11   , En3 , v060
	.byte	W12
	.byte		        Ds3 , v052
	.byte	W12
	.byte		        En3 , v068
	.byte	W12
	.byte		        Fs3 , v056
	.byte	W12
	.byte		        Gs3 , v072
	.byte	W12
	.byte		        An3 , v084
	.byte	W12
	.byte		        Bn3 , v072
	.byte	W12
	.byte		N56   , An3 , v076 , gtp3
	.byte	W60
	.byte		N17   , Bn3
	.byte	W18
	.byte		        Cs4 , v072
	.byte	W18
	.byte		N68   , Ds4 , v076 , gtp3
	.byte	W48
	.byte		VOL   , 46*mus_qz_646_mvl/mxv
	.byte	W06
	.byte		        42*mus_qz_646_mvl/mxv
	.byte	W06
	.byte		        37*mus_qz_646_mvl/mxv
	.byte	W06
	.byte		        33*mus_qz_646_mvl/mxv
	.byte	W06
	.byte		        50*mus_qz_646_mvl/mxv
	.byte		N23   , Fs4 , v072
	.byte	W24
	.byte		N44   , Ds4 , v076 , gtp3
	.byte	W48
	.byte		N23   , Bn3
	.byte	W24
	.byte		        En4
	.byte	W24
	.byte		        Ds4 , v084
	.byte	W24
	.byte		        Bn3 , v068
	.byte	W24
	.byte		        Gn3 , v076
	.byte	W24
	.byte		        Bn3 , v068
	.byte	W24
	.byte		N44   , An3 , v084 , gtp3
	.byte	W48
	.byte		N15   , Fs3 , v080
	.byte	W16
	.byte		        Gs3 , v072
	.byte	W16
	.byte		        An3 , v080
	.byte	W16
	.byte		N44   , Ds3 , v088 , gtp3
	.byte	W48
	.byte		N23   , Fs3 , v080
	.byte	W24
	.byte		        Cn4 , v072
	.byte	W24
	.byte		N32   , Bn3 , v088 , gtp3
	.byte	W36
	.byte		N05   , Bn3 , v076
	.byte	W06
	.byte		        An3 , v068
	.byte	W06
	.byte		N32   , Gs3 , v084 , gtp3
	.byte	W36
	.byte		N05   , Gs3 , v080
	.byte	W06
	.byte		        Fs3 , v068
	.byte	W06
	.byte		N88   , En3 , v088 , gtp1
	.byte	W72
	.byte		VOL   , 46*mus_qz_646_mvl/mxv
	.byte	W06
	.byte		        42*mus_qz_646_mvl/mxv
	.byte	W06
	.byte		        37*mus_qz_646_mvl/mxv
	.byte	W06
	.byte		        33*mus_qz_646_mvl/mxv
	.byte	W06
	.byte		        50*mus_qz_646_mvl/mxv
	.byte		N04   , Bn2 , v100
	.byte		N04   , Gs3 , v104
	.byte	W24
	.byte		        Bn2
	.byte		N04   , Gs3 , v112
	.byte	W36
	.byte		        Bn2 , v092
	.byte		N04   , Gs3 , v096
	.byte	W12
	.byte		        Bn2
	.byte		N04   , Gs3 , v100
	.byte	W24
	.byte		        Bn2
	.byte		N04   , Gs3 , v108
	.byte	W12
	.byte		        Bn2 , v096
	.byte		N04   , Gs3 , v100
	.byte	W12
	.byte		        Bn2 , v096
	.byte		N04   , Gs3 , v100
	.byte	W12
	.byte		N05   , Bn2
	.byte		N04   , Gs3 , v108
	.byte	W60
	.byte		N05   , Bn2 , v096
	.byte		N05   , Gs3 , v100
	.byte	W36
	.byte		        Ds3 , v088
	.byte		N05   , Bn3 , v096
	.byte	W60
	.byte	PATT
	 ptrvalue	mus_qz_646_6_000
	.byte	PATT
	 ptrvalue	mus_qz_646_6_001
	.byte	PATT
	 ptrvalue	mus_qz_646_6_002
	.byte	GOTO
	 ptrvalue	mus_qz_646_6_B1
	.byte	FINE

/********************** Track  7 **********************/

mus_qz_646_7:
	.byte		KEYSH , mus_qz_646_key+0
	.byte		VOICE , 6
	.byte		VOL   , 61*mus_qz_646_mvl/mxv
	.byte		PAN   , c_v-34
	.byte		VOL   , 48*mus_qz_646_mvl/mxv
	.byte		PRIO  , 54
	.byte	W24
	.byte	W96
	.byte	W96
mus_qz_646_7_000:
	.byte		N05   , Bn2 , v100
	.byte	W12
	.byte		        En3 , v080
	.byte	W12
	.byte		        Gs3 , v088
	.byte	W12
	.byte		        En3 , v080
	.byte	W12
	.byte		        Bn3 , v088
	.byte	W12
	.byte		        En3 , v080
	.byte	W12
	.byte		        Gs3 , v088
	.byte	W12
	.byte		        En3 , v080
	.byte	W12
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_646_7_000
mus_qz_646_7_001:
	.byte		N05   , Bn2 , v100
	.byte	W12
	.byte		        Ds3 , v080
	.byte	W12
	.byte		        Fs3 , v088
	.byte	W12
	.byte		        Ds3 , v080
	.byte	W12
	.byte		        Bn3 , v088
	.byte	W12
	.byte		        Ds3 , v080
	.byte	W12
	.byte		        Fs3 , v088
	.byte	W12
	.byte		        Ds3 , v080
	.byte	W12
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_646_7_001
mus_qz_646_7_B1:
	.byte	PATT
	 ptrvalue	mus_qz_646_7_000
	.byte	PATT
	 ptrvalue	mus_qz_646_7_000
	.byte	PATT
	 ptrvalue	mus_qz_646_7_001
	.byte	PATT
	 ptrvalue	mus_qz_646_7_001
	.byte		N05   , An2 , v096
	.byte	W12
	.byte		        Cs3 , v092
	.byte	W12
	.byte		        En3 , v088
	.byte	W12
	.byte		        Cs3 , v092
	.byte	W06
	.byte		        En3 , v088
	.byte	W06
	.byte		        An2 , v096
	.byte	W12
	.byte		        An3 , v088
	.byte	W12
	.byte		        Cs3 , v092
	.byte	W12
	.byte		        En3 , v088
	.byte	W12
	.byte		        Bn2 , v096
	.byte	W12
	.byte		        Ds3 , v092
	.byte	W12
	.byte		        Fs3 , v088
	.byte	W12
	.byte		        Bn2 , v096
	.byte	W06
	.byte		        Fs3 , v092
	.byte	W06
	.byte		        Bn2 , v096
	.byte	W12
	.byte		        Bn3 , v088
	.byte	W12
	.byte		        Ds3
	.byte	W12
	.byte		        Fs3 , v092
	.byte	W12
	.byte		        Bn2 , v096
	.byte	W06
	.byte		        En3 , v092
	.byte	W06
	.byte		        Gs3 , v088
	.byte	W24
	.byte		        En3 , v092
	.byte	W24
	.byte		        Bn2 , v096
	.byte	W06
	.byte		        Gs3 , v088
	.byte	W06
	.byte		        Bn3
	.byte	W24
	.byte		        Bn2 , v096
	.byte	W06
	.byte		        Ds3 , v092
	.byte	W06
	.byte		        Gn3 , v088
	.byte	W24
	.byte		        Ds3 , v092
	.byte	W24
	.byte		        Bn2 , v096
	.byte	W06
	.byte		        Gn3 , v088
	.byte	W06
	.byte		        Bn3
	.byte	W12
	.byte		        Ds3 , v092
	.byte	W12
	.byte		        Bn2 , v096
	.byte	W12
	.byte		        Ds3 , v092
	.byte	W12
	.byte		        Fs3 , v088
	.byte	W12
	.byte		        Ds3 , v092
	.byte	W06
	.byte		        Fs3 , v088
	.byte	W06
	.byte		        Ds3 , v092
	.byte	W12
	.byte		        Bn3 , v088
	.byte	W12
	.byte		        Bn2
	.byte	W12
	.byte		        Fs3
	.byte	W12
	.byte		        Bn2 , v096
	.byte	W12
	.byte		        Ds3 , v092
	.byte	W12
	.byte		        Fs3 , v088
	.byte	W12
	.byte		        Ds3 , v092
	.byte	W06
	.byte		        Fs3 , v088
	.byte	W06
	.byte		        Ds3 , v092
	.byte	W12
	.byte		        Bn3 , v088
	.byte	W12
	.byte		        Ds3 , v092
	.byte	W12
	.byte		        Gn3 , v088
	.byte	W12
	.byte		        Bn2 , v100
	.byte	W06
	.byte		        En3 , v096
	.byte	W06
	.byte		        Gs3 , v092
	.byte	W24
	.byte		        En3 , v096
	.byte	W24
	.byte	N05
	.byte	W06
	.byte		        Gs3 , v092
	.byte	W06
	.byte		        Bn3
	.byte	W24
	.byte		        Bn2 , v096
	.byte	W06
	.byte		        Dn3 , v092
	.byte	W06
	.byte		        Gs3 , v088
	.byte	W24
	.byte		        En3 , v092
	.byte	W24
	.byte		        Bn2 , v088
	.byte	W06
	.byte		        Gs3
	.byte	W06
	.byte		        Bn3
	.byte	W12
	.byte		        En3 , v084
	.byte	W12
	.byte		        An2 , v096
	.byte	W12
	.byte		        Cs3 , v092
	.byte	W12
	.byte		        En3 , v088
	.byte	W12
	.byte		        Cs3 , v092
	.byte	W06
	.byte		        En3 , v088
	.byte	W06
	.byte		        Cs3 , v092
	.byte	W12
	.byte		        An3 , v088
	.byte	W12
	.byte		        Cs3 , v092
	.byte	W12
	.byte		        En3 , v088
	.byte	W12
	.byte		        Bn2 , v096
	.byte	W12
	.byte		        Ds3 , v092
	.byte	W12
	.byte		        Fs3 , v088
	.byte	W12
	.byte		        Ds3 , v092
	.byte	W06
	.byte		        Fs3 , v088
	.byte	W06
	.byte		        Bn2
	.byte	W12
	.byte		        Bn3
	.byte	W12
	.byte		        Cs3
	.byte	W12
	.byte		        En3
	.byte	W12
	.byte		        Bn2 , v096
	.byte	W06
	.byte		        En3 , v092
	.byte	W06
	.byte		        Gs3 , v088
	.byte	W24
	.byte		        En3 , v092
	.byte	W24
	.byte	N05
	.byte	W06
	.byte		        Gs3 , v088
	.byte	W06
	.byte		        Bn3
	.byte	W12
	.byte		        En3 , v092
	.byte	W12
	.byte		        Bn2 , v096
	.byte	W06
	.byte		        Ds3 , v092
	.byte	W06
	.byte		        Gn3 , v088
	.byte	W24
	.byte		        En3 , v092
	.byte	W24
	.byte	N05
	.byte	W06
	.byte		        Gs3 , v088
	.byte	W06
	.byte		        Bn3
	.byte	W12
	.byte		        Ds3 , v092
	.byte	W12
mus_qz_646_7_002:
	.byte		N05   , Bn2 , v096
	.byte	W12
	.byte		        Ds3 , v092
	.byte	W12
	.byte		        Fs3 , v088
	.byte	W12
	.byte		        Ds3 , v092
	.byte	W06
	.byte		        Fs3 , v088
	.byte	W06
	.byte		        Ds3 , v092
	.byte	W12
	.byte		        Bn3 , v088
	.byte	W12
	.byte		        Ds3 , v092
	.byte	W12
	.byte		        Fs3 , v088
	.byte	W12
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_646_7_002
mus_qz_646_7_003:
	.byte		N05   , Bn2 , v096
	.byte	W06
	.byte		        En3 , v092
	.byte	W06
	.byte		        Gs3 , v088
	.byte	W24
	.byte		        En3 , v092
	.byte	W24
	.byte	N05
	.byte	W06
	.byte		        Gs3 , v088
	.byte	W06
	.byte		        Bn3
	.byte	W24
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_646_7_003
	.byte	W96
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_646_7_000
	.byte	PATT
	 ptrvalue	mus_qz_646_7_000
	.byte	PATT
	 ptrvalue	mus_qz_646_7_001
	.byte	PATT
	 ptrvalue	mus_qz_646_7_001
	.byte	GOTO
	 ptrvalue	mus_qz_646_7_B1
	.byte	FINE

/********************** Track  8 **********************/

mus_qz_646_8:
	.byte		KEYSH , mus_qz_646_key+0
	.byte		VOICE , 35
	.byte		VOL   , 21*mus_qz_646_mvl/mxv
	.byte		PAN   , c_v+56
	.byte		VOL   , 16*mus_qz_646_mvl/mxv
	.byte		PRIO  , 38
	.byte	W12
	.byte		N04   , Bn4 , v092
	.byte	W06
	.byte		        Ds5 , v076
	.byte	W06
	.byte		        Fs5 , v084
	.byte	W06
	.byte		        As5 , v092
	.byte	W06
	.byte		N02   , Bn5 , v088
	.byte	W03
	.byte		        Cs6 , v076
	.byte	W03
	.byte		        Bn5 , v084
	.byte	W03
	.byte		        Cs6 , v076
	.byte	W03
	.byte		        Bn5 , v080
	.byte	W03
	.byte		        Cs6 , v072
	.byte	W03
	.byte		        Bn5 , v080
	.byte	W03
	.byte		        Cs6 , v064
	.byte	W03
	.byte		N03   , Bn5 , v072
	.byte	W04
	.byte		        Cs6 , v056
	.byte	W04
	.byte		        Bn5 , v064
	.byte	W04
	.byte		        Cs6 , v052
	.byte	W04
	.byte		        Bn5 , v056
	.byte	W04
	.byte		        Cs6 , v044
	.byte	W04
	.byte		        Bn5 , v052
	.byte	W04
	.byte		        Cs6 , v044
	.byte	W04
	.byte		        Bn5 , v048
	.byte	W04
	.byte		        Cs6 , v036
	.byte	W04
	.byte		        Bn5 , v044
	.byte	W04
	.byte		        Cs6 , v036
	.byte	W04
	.byte		N05   , Bn5
	.byte	W06
	.byte		        Cs6 , v028
	.byte	W06
	.byte		        Bn5 , v032
	.byte	W06
	.byte		        Cs6 , v028
	.byte	W54
	.byte		        Bn5 , v088
	.byte	W06
	.byte		        Cs6 , v092
	.byte	W06
	.byte		        Bn5 , v084
	.byte	W06
	.byte		        An5 , v080
	.byte	W06
	.byte		        Gs5 , v088
	.byte	W06
	.byte		        An5
	.byte	W06
	.byte		        Gs5 , v080
	.byte	W06
	.byte		        Fs5 , v084
	.byte	W06
	.byte		        En5 , v076
	.byte	W84
	.byte	W96
	.byte	W96
mus_qz_646_8_000:
	.byte	W72
	.byte		N10   , Bn4 , v100
	.byte	W12
	.byte		        Cs5 , v092
	.byte	W12
	.byte	PEND
mus_qz_646_8_B1:
	.byte		N10   , Ds5 , v104
	.byte	W12
	.byte		        En5 , v112
	.byte	W24
	.byte		        Ds5 , v104
	.byte	W12
	.byte		        Cs5 , v088
	.byte	W48
	.byte	W72
	.byte		        Bn4 , v108
	.byte	W12
	.byte		        An4 , v088
	.byte	W12
	.byte		        Gs4 , v096
	.byte	W12
	.byte		        An4 , v108
	.byte	W24
	.byte		N06   , Bn4 , v096
	.byte	W12
	.byte		        Bn4 , v084
	.byte	W48
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W12
	.byte		N32   , An4 , v100 , gtp3
	.byte	W36
	.byte		N05   , Fs4 , v076
	.byte	W06
	.byte		        An4 , v084
	.byte	W06
	.byte		N44   , Ds5 , v096 , gtp3
	.byte	W36
	.byte	W12
	.byte		N23   , En5 , v104
	.byte	W24
	.byte		        Ds5 , v096
	.byte	W24
	.byte		        Cs5 , v104
	.byte	W24
	.byte		        Cn5 , v096
	.byte	W12
	.byte	W12
	.byte		N32   , Bn4 , v100 , gtp3
	.byte	W36
	.byte		N05   , Gs4 , v084
	.byte	W06
	.byte		        Bn4 , v080
	.byte	W06
	.byte		N32   , En5 , v096 , gtp3
	.byte	W36
	.byte		N05   , Bn4
	.byte	W06
	.byte		        En5 , v088
	.byte	W06
	.byte		N68   , Gs5 , v104 , gtp3
	.byte	W84
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W12
	.byte		N32   , An4 , v092 , gtp3
	.byte	W36
	.byte		N05   , Fs4
	.byte	W06
	.byte		        An4
	.byte	W06
	.byte		N44   , Ds5 , v092 , gtp3
	.byte	W36
	.byte	W12
	.byte		N23   , En5
	.byte	W24
	.byte		        Ds5
	.byte	W24
	.byte		        Cs5
	.byte	W24
	.byte		        Cn5
	.byte	W12
	.byte	W12
	.byte		N32   , Bn4 , v092 , gtp3
	.byte	W36
	.byte		N05   , Gs4
	.byte	W06
	.byte		        Bn4
	.byte	W06
	.byte		N32   , En5 , v092 , gtp3
	.byte	W36
	.byte		N05   , Bn4
	.byte	W06
	.byte		        En5
	.byte	W06
	.byte		N52   , Gs5 , v092 , gtp1
	.byte	W60
	.byte		N05   , Bn5 , v080
	.byte	W06
	.byte		        Cs6 , v084
	.byte	W06
	.byte		        Bn5 , v076
	.byte	W06
	.byte		        An5
	.byte	W06
	.byte		        Gs5 , v080
	.byte	W06
	.byte		        Fs5 , v084
	.byte	W06
	.byte		        En5 , v072
	.byte	W84
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_646_8_000
	.byte	GOTO
	 ptrvalue	mus_qz_646_8_B1
	.byte	FINE

/********************** Track  9 **********************/

mus_qz_646_9:
	.byte		KEYSH , mus_qz_646_key+0
	.byte		VOICE , 6
	.byte		VOL   , 28*mus_qz_646_mvl/mxv
	.byte		PAN   , c_v+56
	.byte		VOL   , 22*mus_qz_646_mvl/mxv
	.byte		PRIO  , 36
	.byte	W24
	.byte	W96
	.byte	W96
mus_qz_646_9_000:
	.byte	W24
	.byte		N04   , Bn2 , v096
	.byte	W12
	.byte		        En3 , v080
	.byte	W12
	.byte		        Gs3 , v088
	.byte	W12
	.byte		        En3 , v080
	.byte	W12
	.byte		        Bn3 , v088
	.byte	W12
	.byte		        En3 , v080
	.byte	W12
	.byte	PEND
mus_qz_646_9_001:
	.byte		N04   , Gs3 , v088
	.byte	W12
	.byte		        En3 , v080
	.byte	W12
	.byte		        Bn2 , v096
	.byte	W12
	.byte		        En3 , v080
	.byte	W12
	.byte		        Gs3 , v088
	.byte	W12
	.byte		        En3 , v080
	.byte	W12
	.byte		        Bn3 , v088
	.byte	W12
	.byte		        En3 , v080
	.byte	W12
	.byte	PEND
mus_qz_646_9_002:
	.byte		N04   , Gs3 , v088
	.byte	W12
	.byte		        En3 , v080
	.byte	W12
	.byte		        Bn2 , v096
	.byte	W12
	.byte		        Ds3 , v080
	.byte	W12
	.byte		        Fs3 , v088
	.byte	W12
	.byte		        Ds3 , v080
	.byte	W12
	.byte		        Bn3 , v088
	.byte	W12
	.byte		        Ds3 , v080
	.byte	W12
	.byte	PEND
mus_qz_646_9_003:
	.byte		N04   , Fs3 , v088
	.byte	W12
	.byte		        Ds3 , v080
	.byte	W12
	.byte		        Bn2 , v096
	.byte	W12
	.byte		        Ds3 , v080
	.byte	W12
	.byte		        Fs3 , v088
	.byte	W12
	.byte		        Ds3 , v080
	.byte	W12
	.byte		        Bn3 , v088
	.byte	W12
	.byte		        Ds3 , v080
	.byte	W12
	.byte	PEND
mus_qz_646_9_B1:
	.byte		N04   , Fs3 , v088
	.byte	W12
	.byte		        Ds3 , v080
	.byte	W12
	.byte		        Bn2 , v096
	.byte	W12
	.byte		        En3 , v080
	.byte	W12
	.byte		        Gs3 , v088
	.byte	W12
	.byte		        En3 , v080
	.byte	W12
	.byte		        Bn3 , v088
	.byte	W12
	.byte		        En3 , v080
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_646_9_001
	.byte	PATT
	 ptrvalue	mus_qz_646_9_002
	.byte	PATT
	 ptrvalue	mus_qz_646_9_003
	.byte		N04   , Fs3 , v088
	.byte	W12
	.byte		        Ds3 , v080
	.byte	W84
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
	.byte	PATT
	 ptrvalue	mus_qz_646_9_000
	.byte	PATT
	 ptrvalue	mus_qz_646_9_001
	.byte	PATT
	 ptrvalue	mus_qz_646_9_002
	.byte	PATT
	 ptrvalue	mus_qz_646_9_003
	.byte	GOTO
	 ptrvalue	mus_qz_646_9_B1
	.byte	FINE

/********************** Track 10 **********************/

mus_qz_646_10:
	.byte		KEYSH , mus_qz_646_key+0
	.byte		VOICE , 14
	.byte		VOL   , 91*mus_qz_646_mvl/mxv
	.byte		PAN   , c_v+32
	.byte		VOL   , 70*mus_qz_646_mvl/mxv
	.byte		BENDR , 6
	.byte		PRIO  , 50
	.byte	W24
	.byte		N05   , En2 , v104
	.byte	W24
	.byte		        En3 , v100
	.byte		N05   , An3 , v112
	.byte	W12
	.byte		        An1 , v104
	.byte	W24
	.byte		        En3 , v092
	.byte		N05   , An3 , v104
	.byte	W12
	.byte		        En2
	.byte	W12
	.byte		        En3 , v092
	.byte		N05   , An3 , v104
	.byte	W12
mus_qz_646_10_000:
	.byte		N05   , En2 , v104
	.byte	W24
	.byte		        Bn2 , v100
	.byte		N05   , Gs3 , v112
	.byte	W12
	.byte		        Bn1 , v104
	.byte	W24
	.byte		        Bn2 , v092
	.byte		N05   , Gs3 , v104
	.byte	W12
	.byte		        En2
	.byte	W12
	.byte		        Bn2 , v092
	.byte		N05   , Gs3 , v104
	.byte	W12
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_646_10_000
mus_qz_646_10_001:
	.byte		N05   , En2 , v104
	.byte	W12
	.byte		N03   , Gs3 , v120
	.byte	W06
	.byte		        Gs3 , v076
	.byte	W06
	.byte		N05   , Bn2 , v100
	.byte		N05   , Gs3 , v112
	.byte	W12
	.byte		        Bn1 , v104
	.byte	W24
	.byte		        Bn2 , v092
	.byte		N05   , Gs3 , v104
	.byte	W12
	.byte		        En2
	.byte	W12
	.byte		        Bn2 , v092
	.byte		N05   , Gs3 , v104
	.byte	W12
	.byte	PEND
mus_qz_646_10_002:
	.byte		N05   , Bn1 , v104
	.byte	W24
	.byte		        Bn2 , v100
	.byte		N05   , Fs3 , v112
	.byte	W12
	.byte		        Fs1 , v104
	.byte	W24
	.byte		        Bn2 , v092
	.byte		N05   , Fs3 , v104
	.byte	W12
	.byte		        Bn1
	.byte	W12
	.byte		        Bn2 , v092
	.byte		N05   , Fs3 , v104
	.byte	W12
	.byte	PEND
mus_qz_646_10_003:
	.byte		N05   , Bn1 , v104
	.byte	W12
	.byte		N03   , Fs3 , v120
	.byte	W06
	.byte		        Fs3 , v076
	.byte	W06
	.byte		N05   , Bn2 , v100
	.byte		N05   , Fs3 , v112
	.byte	W12
	.byte		        Fs1 , v104
	.byte	W24
	.byte		        Bn2 , v092
	.byte		N05   , Fs3 , v104
	.byte	W12
	.byte		        Bn1
	.byte	W12
	.byte		        Bn2 , v092
	.byte		N05   , Fs3 , v104
	.byte	W12
	.byte	PEND
mus_qz_646_10_B1:
	.byte		N05   , En2 , v104
	.byte	W24
	.byte		        Ds3 , v100
	.byte		N05   , Gs3 , v112
	.byte	W12
	.byte		        Bn1 , v104
	.byte	W24
	.byte		        Ds3 , v092
	.byte		N05   , Gs3 , v104
	.byte	W12
	.byte		        En2
	.byte	W12
	.byte		        Ds3 , v092
	.byte		N05   , Gs3 , v104
	.byte	W12
	.byte		        Bn1
	.byte	W12
	.byte		N03   , Gs3 , v120
	.byte	W06
	.byte		        Gs3 , v076
	.byte	W06
	.byte		N05   , Ds3 , v100
	.byte		N05   , Gs3 , v112
	.byte	W12
	.byte		        En2 , v104
	.byte	W24
	.byte		        Ds3 , v092
	.byte		N05   , Gs3 , v104
	.byte	W12
	.byte		        Bn1
	.byte	W12
	.byte		        Ds3 , v092
	.byte		N05   , Gs3 , v104
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_646_10_002
	.byte	PATT
	 ptrvalue	mus_qz_646_10_003
	.byte		N17   , An1 , v100
	.byte	W12
	.byte		N23   , Gs2 , v092
	.byte	W12
	.byte		        Cs3
	.byte	W12
	.byte		N32   , En3 , v100 , gtp3
	.byte	W24
	.byte		N23   , En2 , v092
	.byte	W12
	.byte		        Gs2
	.byte	W12
	.byte		N11   , Cs3 , v100
	.byte	W12
	.byte		N17   , An1
	.byte	W12
	.byte		N32   , Bn2 , v092 , gtp3
	.byte	W12
	.byte		        Ds3 , v092 , gtp3
	.byte	W12
	.byte		N28   , Fs3 , v100 , gtp1
	.byte	W24
	.byte		N17   , Fs2 , v092
	.byte	W12
	.byte		N11   , Bn2 , v088
	.byte	W06
	.byte		        Ds3 , v072
	.byte	W06
	.byte		        Fs3 , v100
	.byte	W12
	.byte		N17   , Gs1
	.byte	W12
	.byte		N23   , Gs2 , v092
	.byte	W12
	.byte		        Bn2
	.byte	W12
	.byte		        En3 , v100
	.byte	W24
	.byte		N11   , Bn2 , v092
	.byte	W12
	.byte		        Ds3
	.byte	W12
	.byte		        Fs3 , v100
	.byte	W12
	.byte		N17   , Cs2
	.byte	W12
	.byte		N28   , Bn2 , v092 , gtp1
	.byte	W12
	.byte		        Ds3 , v092 , gtp1
	.byte	W12
	.byte		N23   , Gn3 , v100
	.byte	W24
	.byte	N11
	.byte	W04
	.byte		BEND  , c_v+32
	.byte	W04
	.byte		        c_v+0
	.byte	W04
	.byte		N11   , Bn2 , v092
	.byte		N11   , Ds3 , v100
	.byte	W12
	.byte		        Bn2 , v092
	.byte	W12
	.byte		N17   , Fs1 , v100
	.byte	W12
	.byte		        An2 , v092
	.byte	W12
	.byte		        Cs3
	.byte	W12
	.byte		        En3 , v096
	.byte	W24
	.byte		        Fs2 , v092
	.byte	W12
	.byte		        An2
	.byte	W12
	.byte		N11   , En3 , v096
	.byte	W12
	.byte		N17   , Bn1 , v100
	.byte	W12
	.byte		N23   , An2 , v092
	.byte	W12
	.byte		        Ds3
	.byte	W12
	.byte		N28   , Fs3 , v096 , gtp1
	.byte	W24
	.byte		N23   , Bn2 , v092
	.byte	W12
	.byte		        Ds3
	.byte	W12
	.byte		N17   , Gn3 , v096
	.byte	W12
	.byte		        En1 , v100
	.byte	W12
	.byte		N28   , Gs2 , v092 , gtp1
	.byte	W12
	.byte		N23   , Bn2
	.byte	W12
	.byte		N28   , Ds3 , v096 , gtp1
	.byte	W24
	.byte		N23   , En2 , v092
	.byte	W12
	.byte		N17   , Bn2
	.byte	W12
	.byte		N11   , Ds3 , v096
	.byte	W12
	.byte		N17   , En1 , v100
	.byte	W12
	.byte		N20   , Gs2 , v092
	.byte		N20   , Dn3 , v080
	.byte	W01
	.byte		N21   , Fs3 , v088
	.byte	W32
	.byte	W03
	.byte		N08   , Gs2
	.byte		N08   , Dn3 , v092
	.byte	W01
	.byte		        Gs3 , v096
	.byte	W23
	.byte		        Gs2 , v088
	.byte		N08   , Dn3 , v092
	.byte	W01
	.byte		        Fs3 , v096
	.byte	W23
	.byte		N17   , An1 , v100
	.byte	W12
	.byte		        Gs2 , v092
	.byte	W12
	.byte		        Cs3
	.byte	W12
	.byte		N23   , En3 , v096
	.byte	W24
	.byte		        Gs2 , v092
	.byte	W12
	.byte		N17   , Cs3
	.byte	W12
	.byte		N11   , En3 , v096
	.byte	W12
	.byte		N17   , An1 , v100
	.byte	W12
	.byte		        Bn2 , v092
	.byte	W12
	.byte		        Ds3
	.byte	W12
	.byte		N23   , Fs3 , v104
	.byte	W24
	.byte		        Fs2 , v092
	.byte	W12
	.byte		N17   , Ds3
	.byte	W12
	.byte		N11   , Fs3 , v104
	.byte	W12
	.byte		N17   , Gs1 , v100
	.byte	W12
	.byte		        Gs2 , v092
	.byte	W12
	.byte		        Bn2
	.byte	W12
	.byte		N23   , En3
	.byte	W24
	.byte		N17   , Bn2
	.byte	W12
	.byte		        Ds3
	.byte	W12
	.byte		N11   , Fs3 , v100
	.byte	W12
	.byte		N17   , Gn1
	.byte	W12
	.byte		N23   , Gn2 , v092
	.byte	W12
	.byte		        Bn2
	.byte	W12
	.byte		        En3 , v100
	.byte	W24
	.byte		N11   , Gn3 , v088
	.byte	W04
	.byte		BEND  , c_v+42
	.byte	W04
	.byte		        c_v+0
	.byte	W04
	.byte		N11   , En3 , v100
	.byte	W12
	.byte		        Cs3 , v092
	.byte	W12
	.byte		N17   , Fs1 , v100
	.byte	W12
	.byte		N23   , An2 , v092
	.byte	W12
	.byte		N17   , Cs3
	.byte	W12
	.byte		N23   , En3
	.byte	W24
	.byte		N17   , An2
	.byte	W12
	.byte		        Cs3
	.byte	W12
	.byte		N11   , En3
	.byte	W12
	.byte		N17   , Bn1 , v100
	.byte	W12
	.byte		N23   , An2 , v092
	.byte	W12
	.byte		        Ds3 , v088
	.byte	W12
	.byte		        Fs3 , v092
	.byte	W24
	.byte		N17   , An2
	.byte	W12
	.byte		        Ds3 , v088
	.byte	W12
	.byte		N23   , Fs3 , v092
	.byte	W12
	.byte		N17   , En1 , v100
	.byte	W12
	.byte		N23   , Bn2 , v092
	.byte	W12
	.byte		        Ds3 , v088
	.byte	W12
	.byte		        Gs3 , v104
	.byte	W24
	.byte		        Bn2 , v092
	.byte	W12
	.byte		N17   , Ds3 , v088
	.byte	W12
	.byte		N11   , Gs3 , v104
	.byte	W12
	.byte		N17   , En2 , v092
	.byte	W12
	.byte		N23   , Bn2
	.byte	W12
	.byte		        Ds3 , v088
	.byte	W12
	.byte		N21   , Gs3 , v104
	.byte	W24
	.byte		N11   , Gs3 , v100
	.byte	W04
	.byte		BEND  , c_v+32
	.byte	W04
	.byte		        c_v+0
	.byte	W04
	.byte		N20   , En3
	.byte	W12
	.byte		BEND  , c_v-54
	.byte	W11
	.byte		        c_v+0
	.byte	W01
	.byte		N05   , En1
	.byte		N05   , Bn2 , v096
	.byte		N05   , Ds3 , v092
	.byte		N05   , Gs3
	.byte	W24
	.byte		        En1 , v100
	.byte		N05   , Bn2 , v096
	.byte		N05   , Ds3 , v092
	.byte		N05   , Gs3
	.byte	W36
	.byte		        En1 , v100
	.byte		N05   , Bn2 , v096
	.byte		N05   , Ds3 , v092
	.byte		N05   , Gs3
	.byte	W12
	.byte		        En1 , v100
	.byte		N05   , Bn2 , v096
	.byte		N05   , Ds3 , v092
	.byte		N05   , Gs3
	.byte	W24
	.byte		        En1 , v100
	.byte		N05   , Bn2 , v096
	.byte		N05   , Ds3 , v092
	.byte		N05   , Gs3
	.byte	W12
	.byte		        En1 , v100
	.byte		N05   , Bn2 , v096
	.byte		N05   , Ds3 , v092
	.byte		N05   , Gs3
	.byte	W12
	.byte		        En1 , v100
	.byte		N05   , Bn2 , v096
	.byte		N05   , Ds3 , v092
	.byte		N05   , Gs3
	.byte	W12
	.byte		N09   , En1 , v100
	.byte		N09   , Bn2 , v096
	.byte		N09   , Ds3 , v092
	.byte		N09   , Gs3
	.byte	W60
	.byte	PATT
	 ptrvalue	mus_qz_646_10_000
	.byte	PATT
	 ptrvalue	mus_qz_646_10_001
	.byte	PATT
	 ptrvalue	mus_qz_646_10_002
	.byte	PATT
	 ptrvalue	mus_qz_646_10_003
	.byte	GOTO
	 ptrvalue	mus_qz_646_10_B1
	.byte	FINE

/********************** Track 11 **********************/

mus_qz_646_11:
	.byte		KEYSH , mus_qz_646_key+0
	.byte		VOICE , 34
	.byte		VOL   , 21*mus_qz_646_mvl/mxv
	.byte		PAN   , c_v-54
	.byte		VOL   , 16*mus_qz_646_mvl/mxv
	.byte		PRIO  , 34
	.byte	W24
	.byte	W96
mus_qz_646_11_000:
	.byte	W72
	.byte		N10   , Bn3 , v100
	.byte	W12
	.byte		        Cs4 , v084
	.byte	W12
	.byte	PEND
mus_qz_646_11_001:
	.byte		N10   , Ds4 , v092
	.byte	W12
	.byte		        En4 , v104
	.byte	W24
	.byte		        Ds4 , v096
	.byte	W12
	.byte		        Cs4 , v084
	.byte	W48
	.byte	PEND
mus_qz_646_11_002:
	.byte	W72
	.byte		N10   , Bn3 , v100
	.byte	W12
	.byte		        An3 , v084
	.byte	W12
	.byte	PEND
mus_qz_646_11_003:
	.byte		N10   , Gs3 , v092
	.byte	W12
	.byte		        An3 , v104
	.byte	W24
	.byte		N05   , Bn3 , v096
	.byte	W12
	.byte		        Bn3 , v084
	.byte	W48
	.byte	PEND
	.byte	W96
mus_qz_646_11_B1:
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W12
	.byte		N32   , Cs4 , v100 , gtp3
	.byte	W36
	.byte		N05   , An3 , v072
	.byte	W06
	.byte		        Cs4 , v084
	.byte	W06
	.byte		N28   , En4 , v108 , gtp1
	.byte	W36
	.byte		N05   , Cs4 , v076
	.byte	W06
	.byte		        En4 , v088
	.byte	W06
	.byte		N23   , Fs4 , v108
	.byte	W24
	.byte		        En4 , v096
	.byte	W24
	.byte		        Ds4 , v104
	.byte	W24
	.byte		        Cs4 , v096
	.byte	W12
	.byte	W12
	.byte		N32   , Bn3 , v104 , gtp3
	.byte	W36
	.byte		N05   , Gs3 , v092
	.byte	W06
	.byte		        Bn3 , v084
	.byte	W06
	.byte		N40   , En4 , v108 , gtp1
	.byte	W36
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W12
	.byte		N32   , Cs4 , v100 , gtp3
	.byte	W36
	.byte		N05   , An3 , v080
	.byte	W06
	.byte		        Cs4 , v088
	.byte	W06
	.byte		N28   , En4 , v104 , gtp1
	.byte	W36
	.byte		N05   , Cs4 , v084
	.byte	W06
	.byte		        En4 , v088
	.byte	W06
	.byte		N23   , Fs4 , v108
	.byte	W24
	.byte		        En4 , v096
	.byte	W24
	.byte		        Ds4 , v108
	.byte	W24
	.byte		        Cs4 , v100
	.byte	W12
	.byte	W12
	.byte		N32   , Bn3 , v108 , gtp3
	.byte	W36
	.byte		N05   , Gs3 , v096
	.byte	W06
	.byte		        Bn3 , v088
	.byte	W06
	.byte		N40   , En4 , v104 , gtp1
	.byte	W36
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_646_11_000
	.byte	PATT
	 ptrvalue	mus_qz_646_11_001
	.byte	PATT
	 ptrvalue	mus_qz_646_11_002
	.byte	PATT
	 ptrvalue	mus_qz_646_11_003
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_646_11_B1
	.byte	FINE

/********************** Track 12 **********************/

mus_qz_646_12:
	.byte		KEYSH , mus_qz_646_key+0
	.byte		VOICE , 7
	.byte		VOL   , 14*mus_qz_646_mvl/mxv
	.byte		PAN   , c_v-54
	.byte		VOL   , 11*mus_qz_646_mvl/mxv
	.byte		PRIO  , 34
	.byte	W24
	.byte		N04   , Bn3 , v092
	.byte	W06
	.byte		        Ds4 , v076
	.byte	W06
	.byte		        Fs4 , v084
	.byte	W06
	.byte		        As4 , v092
	.byte	W06
	.byte		N02   , Bn4 , v088
	.byte	W03
	.byte		        Cs5 , v076
	.byte	W03
	.byte		        Bn4 , v084
	.byte	W03
	.byte		        Cs5 , v076
	.byte	W03
	.byte		        Bn4 , v080
	.byte	W03
	.byte		        Cs5 , v072
	.byte	W03
	.byte		        Bn4 , v080
	.byte	W03
	.byte		        Cs5 , v064
	.byte	W03
	.byte		N03   , Bn4 , v072
	.byte	W04
	.byte		        Cs5 , v056
	.byte	W04
	.byte		        Bn4 , v064
	.byte	W04
	.byte		        Cs5 , v052
	.byte	W04
	.byte		        Bn4 , v056
	.byte	W04
	.byte		        Cs5 , v044
	.byte	W04
	.byte		        Bn4 , v052
	.byte	W04
	.byte		        Cs5 , v044
	.byte	W04
	.byte		        Bn4 , v048
	.byte	W04
	.byte		        Cs5 , v036
	.byte	W04
	.byte		        Bn4 , v044
	.byte	W04
	.byte		        Cs5 , v036
	.byte	W04
	.byte		N05   , Bn4
	.byte	W06
	.byte		        Cs5 , v028
	.byte	W06
	.byte		        Bn4 , v032
	.byte	W06
	.byte		        Cs5 , v028
	.byte	W54
	.byte		        Bn4 , v088
	.byte	W06
	.byte		        Cs5 , v092
	.byte	W06
	.byte		        Bn4 , v084
	.byte	W06
	.byte		        An4 , v080
	.byte	W06
	.byte		        Gs4 , v088
	.byte	W06
	.byte		        An4
	.byte	W06
	.byte		        Gs4 , v080
	.byte	W06
	.byte		        Fs4 , v084
	.byte	W06
	.byte		        En4 , v076
	.byte	W42
	.byte		N04   , Bn2 , v088
	.byte	W12
	.byte		        Cs3 , v080
	.byte	W12
	.byte		        Ds3 , v088
	.byte	W06
mus_qz_646_12_000:
	.byte	W06
	.byte		N04   , En3 , v092
	.byte	W24
	.byte		        Ds3 , v080
	.byte	W12
	.byte		        Cs3 , v088
	.byte	W54
	.byte	PEND
mus_qz_646_12_001:
	.byte	W66
	.byte		N04   , Bn2 , v092
	.byte	W12
	.byte		        An2 , v080
	.byte	W12
	.byte		        Gs2 , v084
	.byte	W06
	.byte	PEND
mus_qz_646_12_002:
	.byte	W06
	.byte		N04   , An2 , v092
	.byte	W24
	.byte		        Bn2 , v084
	.byte	W12
	.byte	N04
	.byte	W54
	.byte	PEND
mus_qz_646_12_B1:
	.byte	W66
	.byte		N04   , Bn3 , v080
	.byte	W12
	.byte		        Cs4 , v072
	.byte	W12
	.byte		        Ds4 , v080
	.byte	W06
	.byte	W06
	.byte		        En4 , v084
	.byte	W24
	.byte		        Ds4 , v064
	.byte	W12
	.byte		        Cs4 , v076
	.byte	W54
	.byte	W66
	.byte		        Bn3 , v080
	.byte	W12
	.byte		        An3 , v068
	.byte	W12
	.byte		        Gs3 , v080
	.byte	W06
	.byte	W06
	.byte		        An3 , v088
	.byte	W24
	.byte		        Bn3 , v072
	.byte	W12
	.byte		        Bn3 , v084
	.byte	W54
	.byte	W96
	.byte	W96
	.byte	W96
mus_qz_646_12_003:
	.byte	W06
	.byte		N05   , Bn3 , v072
	.byte	W06
	.byte		        Bn3 , v052
	.byte	W06
	.byte		N03   , Bn3 , v060
	.byte	W04
	.byte		        Bn3 , v048
	.byte	W04
	.byte		        Bn3 , v052
	.byte	W04
	.byte		        Bn3 , v032
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		N01   , Bn3 , v036
	.byte	W04
	.byte		N05   , Gs3 , v072
	.byte	W06
	.byte		        Bn3 , v056
	.byte	W06
	.byte		        En4 , v072
	.byte	W06
	.byte		        En4 , v052
	.byte	W06
	.byte		N03   , En4 , v060
	.byte	W04
	.byte		        En4 , v048
	.byte	W04
	.byte		        En4 , v052
	.byte	W04
	.byte		        En4 , v032
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		N01   , En4 , v036
	.byte	W02
	.byte		        En4 , v028
	.byte	W02
	.byte		        En4 , v032
	.byte	W02
	.byte		        En4 , v020
	.byte	W04
	.byte	PEND
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_646_12_003
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W66
	.byte		N04   , Bn2 , v088
	.byte	W12
	.byte		        Cs3 , v080
	.byte	W12
	.byte		        Ds3 , v088
	.byte	W06
	.byte	PATT
	 ptrvalue	mus_qz_646_12_000
	.byte	PATT
	 ptrvalue	mus_qz_646_12_001
	.byte	PATT
	 ptrvalue	mus_qz_646_12_002
	.byte	GOTO
	 ptrvalue	mus_qz_646_12_B1
	.byte	FINE

/********************** Track 13 **********************/

mus_qz_646_13:
	.byte		KEYSH , mus_qz_646_key+0
	.byte		VOICE , 23
	.byte		VOL   , 21*mus_qz_646_mvl/mxv
	.byte		PAN   , c_v+56
	.byte		VOL   , 16*mus_qz_646_mvl/mxv
	.byte		PRIO  , 36
	.byte	W24
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
mus_qz_646_13_B1:
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W12
	.byte		N68   , En3 , v080 , gtp3
	.byte	W72
	.byte		N23   , Gs3 , v072
	.byte	W12
	.byte	W12
	.byte		N68   , Fs3 , v084 , gtp3
	.byte	W72
	.byte		N23   , Bn3 , v072
	.byte	W12
	.byte	W12
	.byte		N68   , Gs3 , v088 , gtp3
	.byte	W72
	.byte		N23   , En4 , v080
	.byte	W12
	.byte	W12
	.byte		        Ds4 , v072
	.byte	W24
	.byte		        Bn3 , v068
	.byte	W24
	.byte		        Gn3 , v072
	.byte	W24
	.byte		        Bn3 , v064
	.byte	W12
	.byte	W12
	.byte		N56   , An3 , v072 , gtp3
	.byte	W60
	.byte		N11   , Fs3 , v068
	.byte	W12
	.byte		        Gs3 , v060
	.byte	W12
	.byte		        An3 , v084
	.byte	W12
	.byte		N44   , Ds3 , v076 , gtp3
	.byte	W48
	.byte		N23   , Fs3 , v084
	.byte	W24
	.byte		        An3 , v072
	.byte	W12
	.byte	W12
	.byte		N32   , En3 , v072 , gtp3
	.byte	W36
	.byte		N05   , En3 , v076
	.byte	W06
	.byte		        Fs3 , v064
	.byte	W06
	.byte		N32   , Gs3 , v068 , gtp3
	.byte	W36
	.byte		N05   , An3 , v072
	.byte	W06
	.byte		N17   , Bn3 , v056
	.byte	W18
	.byte		N11   , En3 , v060
	.byte	W12
	.byte		        Ds3 , v052
	.byte	W12
	.byte		        En3 , v068
	.byte	W12
	.byte		        Fs3 , v056
	.byte	W12
	.byte		        Gs3 , v072
	.byte	W12
	.byte		        An3 , v084
	.byte	W12
	.byte		        Bn3 , v072
	.byte	W12
	.byte		N56   , An3 , v076 , gtp3
	.byte	W60
	.byte		N17   , Bn3
	.byte	W18
	.byte		        Cs4 , v072
	.byte	W06
	.byte	W12
	.byte		N68   , Ds4 , v076 , gtp3
	.byte	W72
	.byte		N23   , Fs4 , v072
	.byte	W12
	.byte	W12
	.byte		N44   , Ds4 , v076 , gtp3
	.byte	W48
	.byte		N23   , Bn3
	.byte	W24
	.byte		        En4
	.byte	W12
	.byte	W12
	.byte		        Ds4 , v084
	.byte	W24
	.byte		        Bn3 , v068
	.byte	W24
	.byte		        Gn3 , v076
	.byte	W24
	.byte		        Bn3 , v068
	.byte	W12
	.byte	W12
	.byte		N44   , An3 , v084 , gtp3
	.byte	W48
	.byte		N15   , Fs3 , v080
	.byte	W16
	.byte		        Gs3 , v072
	.byte	W16
	.byte		        An3 , v080
	.byte	W04
	.byte	W12
	.byte		N44   , Ds3 , v088 , gtp3
	.byte	W48
	.byte		N23   , Fs3 , v080
	.byte	W24
	.byte		        Cn4 , v072
	.byte	W12
	.byte	W12
	.byte		N32   , Bn3 , v088 , gtp3
	.byte	W36
	.byte		N05   , Bn3 , v076
	.byte	W06
	.byte		        An3 , v068
	.byte	W06
	.byte		N32   , Gs3 , v084 , gtp3
	.byte	W36
	.byte		N05   , Gs3 , v080
	.byte	W06
	.byte		        Fs3 , v068
	.byte	W06
	.byte		N88   , En3 , v088 , gtp1
	.byte	W84
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_646_13_B1
	.byte	FINE

/******************************************************/
	.p2align	2

mus_qz_646:
	.byte	13	/* NumTrks */
	.byte	0	/* NumBlks */
	.byte	mus_qz_646_pri	/* Priority */
	.byte	mus_qz_646_rev	/* Reverb. */
space64 4
	ptrvalue	mus_qz_646_grp

	ptrvalue	mus_qz_646_1
	ptrvalue	mus_qz_646_2
	ptrvalue	mus_qz_646_3
	ptrvalue	mus_qz_646_4
	ptrvalue	mus_qz_646_5
	ptrvalue	mus_qz_646_6
	ptrvalue	mus_qz_646_7
	ptrvalue	mus_qz_646_8
	ptrvalue	mus_qz_646_9
	ptrvalue	mus_qz_646_10
	ptrvalue	mus_qz_646_11
	ptrvalue	mus_qz_646_12
	ptrvalue	mus_qz_646_13

	.end
