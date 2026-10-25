	.include "asm/macros/bit_width.inc"

	.include "MPlayDef.s"

/* Moemon Quetzal song 653, disassembled from the ROM by tools/quetzal_rip/songs.py.
   "org" comments give the ROM address of the bytes that follow. */

	.equ	mus_qz_653_grp, voicegroup_qz_songs
	.equ	mus_qz_653_pri, 0
	.equ	mus_qz_653_rev, reverb_set+0
	.equ	mus_qz_653_mvl, 127
	.equ	mus_qz_653_key, 0
	.equ	mus_qz_653_tbs, 1
	.equ	mus_qz_653_exg, 1
	.equ	mus_qz_653_cmp, 1

	.section .rodata
	.global	mus_qz_653
	.p2align	2
/* org 0x0969BC1C */

/********************** Track  1 **********************/

mus_qz_653_1:
	.byte		KEYSH , mus_qz_653_key+0
	.byte		TEMPO , 188*mus_qz_653_tbs/2
	.byte		VOL   , 82*mus_qz_653_mvl/mxv
	.byte		        55*mus_qz_653_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		BENDR , 6
	.byte		PRIO  , 64
	.byte	W72
	.byte		VOICE , 23
	.byte	W48
	.byte	W48
	.byte		VOL   , 68*mus_qz_653_mvl/mxv
	.byte	W24
	.byte		        4*mus_qz_653_mvl/mxv
	.byte	W05
	.byte		        6*mus_qz_653_mvl/mxv
	.byte	W03
	.byte		TIE   , As1 , v088
	.byte	W04
	.byte		VOL   , 8*mus_qz_653_mvl/mxv
	.byte	W05
	.byte		        11*mus_qz_653_mvl/mxv
	.byte	W07
	.byte		        15*mus_qz_653_mvl/mxv
	.byte	W05
	.byte		        16*mus_qz_653_mvl/mxv
	.byte	W07
	.byte		        20*mus_qz_653_mvl/mxv
	.byte	W05
	.byte		        28*mus_qz_653_mvl/mxv
	.byte	W07
	.byte		        42*mus_qz_653_mvl/mxv
	.byte	W08
	.byte		        52*mus_qz_653_mvl/mxv
	.byte	W08
	.byte		        61*mus_qz_653_mvl/mxv
	.byte	W08
	.byte		        80*mus_qz_653_mvl/mxv
	.byte		        67*mus_qz_653_mvl/mxv
	.byte	W44
	.byte	W03
	.byte	EOT
	.byte	W01
	.byte		N92   , An1 , v088 , gtp3
	.byte	W96
	.byte		        Cs2 , v088 , gtp3
	.byte	W96
	.byte		        Cn2 , v088 , gtp3
	.byte	W96
	.byte		N44   , En2 , v088 , gtp3
	.byte	W48
	.byte		        Ds2 , v088 , gtp3
	.byte	W48
	.byte		        Fs2 , v088 , gtp3
	.byte	W48
	.byte		        Fn2 , v088 , gtp3
	.byte	W48
	.byte		N92   , Gs2 , v088 , gtp3
	.byte	W12
	.byte		BEND  , c_v+24
	.byte	W12
	.byte		        c_v-12
	.byte	W72
	.byte		        c_v+0
	.byte		N44   , Fs2 , v088 , gtp3
	.byte	W48
	.byte		        Fn2 , v088 , gtp3
	.byte	W08
	.byte		VOL   , 70*mus_qz_653_mvl/mxv
	.byte	W07
	.byte		        77*mus_qz_653_mvl/mxv
	.byte	W08
	.byte		        97*mus_qz_653_mvl/mxv
	.byte	W08
	.byte		        102*mus_qz_653_mvl/mxv
	.byte	W17
	.byte		N92   , Dn1 , v088 , gtp3
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W48
	.byte	W96
	.byte	W96
mus_qz_653_1_B1:
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		VOICE , 30
	.byte		VOL   , 100*mus_qz_653_mvl/mxv
	.byte		PAN   , c_v-56
	.byte		N05   , Fn1 , v104
	.byte	W12
	.byte		        Fs1 , v088
	.byte	W24
	.byte	N05
	.byte	W12
	.byte		        Fs1 , v100
	.byte	W12
	.byte		        Fs1 , v056
	.byte	W12
	.byte		N11   , Bn1 , v088
	.byte	W12
	.byte		N32   , An1 , v088 , gtp3
	.byte	W12
	.byte	W24
	.byte		N23   , Fs1
	.byte	W24
	.byte		VOL   , 73*mus_qz_653_mvl/mxv
	.byte		PAN   , c_v+40
	.byte		N11   , Fn1 , v100
	.byte	W12
	.byte		N23   , Fs1 , v104
	.byte	W24
	.byte		N11   , Cn1 , v112
	.byte	W12
	.byte	W96
	.byte		VOICE , 31
	.byte		PAN   , c_v+41
	.byte		VOL   , 102*mus_qz_653_mvl/mxv
	.byte		N09   , Fs1 , v127
	.byte	W12
	.byte		N05   , Gn1 , v088
	.byte	W12
	.byte		N02   , Gn1 , v127
	.byte	W12
	.byte		N05   , Gn1 , v088
	.byte	W12
	.byte		N11   , En1 , v127
	.byte	W12
	.byte		N32   , Cn1 , v127 , gtp3
	.byte	W36
	.byte		VOL   , 78*mus_qz_653_mvl/mxv
	.byte	W18
	.byte		VOICE , 7
	.byte	W06
	.byte		PAN   , c_v-44
	.byte		N05   , An4 , v088
	.byte	W06
	.byte		        Bn4 , v072
	.byte	W06
	.byte		        Fs4 , v068
	.byte	W06
	.byte		        Gn4
	.byte	W06
	.byte		        Ds4 , v064
	.byte	W06
	.byte		        Fs4
	.byte	W06
	.byte		N11   , Bn3 , v068
	.byte	W12
	.byte		N05   , Fs3 , v088
	.byte	W06
	.byte		        Fs3 , v032
	.byte	W06
	.byte		        Bn2 , v088
	.byte	W06
	.byte		        Bn2 , v056
	.byte	W06
	.byte		        Fs3 , v088
	.byte	W06
	.byte		        Fs3 , v048
	.byte	W06
	.byte		        Fs3 , v056
	.byte	W06
	.byte		        As3 , v088
	.byte	W06
	.byte		        Bn3
	.byte	W06
	.byte		        Bn3 , v044
	.byte	W06
	.byte		        Bn3 , v036
	.byte	W06
	.byte		        Bn3 , v028
	.byte	W54
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
	.byte		VOICE , 29
	.byte		PAN   , c_v+13
	.byte		VOL   , 77*mus_qz_653_mvl/mxv
	.byte		        77*mus_qz_653_mvl/mxv
	.byte	W24
	.byte		N03   , Cn3 , v088
	.byte	W08
	.byte		        Cn3 , v072
	.byte	W08
	.byte		        Cn3 , v088
	.byte	W08
	.byte		        Bn2 , v076
	.byte	W08
	.byte		        Bn2 , v088
	.byte	W08
	.byte		        Bn2 , v076
	.byte	W08
	.byte		        Dn3 , v088
	.byte	W08
	.byte		        Dn3 , v072
	.byte	W08
	.byte		        Dn3 , v088
	.byte	W08
	.byte		        Cs3
	.byte	W08
	.byte		        Cs3 , v068
	.byte	W08
	.byte		        Cs3 , v088
	.byte	W08
	.byte		        En3
	.byte	W08
	.byte		        En3 , v072
	.byte	W08
	.byte		        En3 , v088
	.byte	W08
	.byte		        Ds3 , v076
	.byte	W08
	.byte		        Ds3 , v088
	.byte	W08
	.byte		        Ds3 , v076
	.byte	W08
	.byte		        Fs3 , v088
	.byte	W08
	.byte		        Fs3 , v072
	.byte	W08
	.byte		        Fs3 , v088
	.byte	W08
	.byte		N11   , Bn3
	.byte	W12
	.byte		        Bn3 , v040
	.byte	W12
	.byte		N03   , An2 , v088
	.byte	W08
	.byte		        An2 , v072
	.byte	W08
	.byte		        An2 , v088
	.byte	W08
	.byte		        Gs2 , v076
	.byte	W08
	.byte		        Gs2 , v088
	.byte	W08
	.byte		        Gs2 , v076
	.byte	W08
	.byte		        Bn2 , v088
	.byte	W08
	.byte		        Bn2 , v072
	.byte	W08
	.byte		        Bn2 , v088
	.byte	W08
	.byte		        As2
	.byte	W08
	.byte		        As2 , v068
	.byte	W08
	.byte		        As2 , v088
	.byte	W08
	.byte		        Cs3
	.byte	W08
	.byte		        Cs3 , v072
	.byte	W08
	.byte		        Cs3 , v088
	.byte	W08
	.byte		        Cn3 , v076
	.byte	W08
	.byte		        Cn3 , v088
	.byte	W08
	.byte		        Cn3 , v076
	.byte	W08
	.byte		        Ds3 , v088
	.byte	W08
	.byte		        Ds3 , v072
	.byte	W08
	.byte		        Ds3 , v088
	.byte	W08
	.byte		VOL   , 73*mus_qz_653_mvl/mxv
	.byte		PAN   , c_v+44
	.byte		N07   , En3
	.byte	W09
	.byte		        En3 , v040
	.byte	W36
	.byte	W03
	.byte		        En3 , v088
	.byte	W09
	.byte		N10   , En3 , v040
	.byte	W36
	.byte	W03
	.byte	W24
	.byte		N23   , En3 , v088
	.byte	W24
	.byte		N11   , En3 , v036
	.byte	W24
	.byte		N03   , En3 , v076
	.byte	W08
	.byte		        En3 , v088
	.byte	W08
	.byte		        En3 , v076
	.byte	W08
	.byte		N07   , Fs3 , v088
	.byte	W08
	.byte		N15   , Fs3 , v032
	.byte	W40
	.byte		N07   , Fs3 , v084
	.byte	W08
	.byte		N15   , Fs3 , v032
	.byte	W40
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_653_1_B1
	.byte	FINE

/********************** Track  2 **********************/

mus_qz_653_2:
	.byte		KEYSH , mus_qz_653_key+0
	.byte		VOL   , 82*mus_qz_653_mvl/mxv
	.byte		        55*mus_qz_653_mvl/mxv
	.byte		BENDR , 6
	.byte		PRIO  , 64
	.byte	W72
	.byte		PAN   , c_v-45
	.byte	W48
	.byte	W48
	.byte	W48
	.byte	W24
	.byte		VOICE , 25
	.byte		VOL   , 41*mus_qz_653_mvl/mxv
	.byte	W01
	.byte		N68   , Cs1 , v088 , gtp2
	.byte	W07
	.byte		VOL   , 44*mus_qz_653_mvl/mxv
	.byte		        41*mus_qz_653_mvl/mxv
	.byte	W08
	.byte		        48*mus_qz_653_mvl/mxv
	.byte	W08
	.byte		        54*mus_qz_653_mvl/mxv
	.byte	W48
	.byte		N92   , Cn1 , v088 , gtp3
	.byte	W96
	.byte		        En1 , v088 , gtp3
	.byte	W96
	.byte		        Ds1 , v088 , gtp3
	.byte	W96
	.byte		N44   , Gn1 , v088 , gtp3
	.byte	W48
	.byte		        Fs1 , v088 , gtp3
	.byte	W48
	.byte		        An1 , v088 , gtp3
	.byte	W48
	.byte		        Gs1 , v088 , gtp3
	.byte	W48
	.byte		N92   , Bn1 , v088 , gtp3
	.byte	W12
	.byte		BEND  , c_v+24
	.byte	W12
	.byte		        c_v-12
	.byte	W72
	.byte		        c_v+0
	.byte		N44   , An1 , v088 , gtp3
	.byte	W48
	.byte		VOL   , 35*mus_qz_653_mvl/mxv
	.byte		        43*mus_qz_653_mvl/mxv
	.byte		N44   , Gs1 , v088 , gtp3
	.byte	W07
	.byte		VOL   , 80*mus_qz_653_mvl/mxv
	.byte	W16
	.byte		        102*mus_qz_653_mvl/mxv
	.byte	W24
	.byte	W01
	.byte		        82*mus_qz_653_mvl/mxv
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W48
	.byte	W96
	.byte	W72
	.byte		VOICE , 29
	.byte	W24
mus_qz_653_2_B1:
	.byte		VOL   , 102*mus_qz_653_mvl/mxv
	.byte		PAN   , c_v+4
	.byte		VOL   , 102*mus_qz_653_mvl/mxv
	.byte		N05   , Bn2 , v127
	.byte	W06
	.byte		        As2 , v088
	.byte	W06
	.byte		N11   , Bn2 , v104
	.byte	W12
	.byte		N02   , An2 , v088
	.byte	W03
	.byte		        Gn2
	.byte	W03
	.byte		TIE   , Fs2 , v104
	.byte	W66
	.byte	W44
	.byte	W03
	.byte	EOT
	.byte	W13
	.byte		N11   , Bn2 , v112
	.byte	W12
	.byte		N17   , As2 , v104
	.byte	W18
	.byte		N05   , Fs3 , v088
	.byte	W06
	.byte		TIE   , Gn3 , v127
	.byte	W96
	.byte	W11
	.byte	EOT
	.byte	W01
	.byte		N32   , An3 , v120 , gtp3
	.byte	W36
	.byte		N11   , Gn3 , v116
	.byte	W12
	.byte		N32   , Fn3 , v088 , gtp3
	.byte	W36
	.byte		TIE   , Fs3 , v120
	.byte	W96
	.byte	W92
	.byte	W03
	.byte	EOT
	.byte	W01
	.byte	W96
	.byte	W72
	.byte		VOICE , 23
	.byte	W24
	.byte		VOL   , 102*mus_qz_653_mvl/mxv
	.byte		PAN   , c_v-16
	.byte		N03   , Ds4 , v116
	.byte	W06
	.byte		N02   , Dn4 , v088
	.byte	W06
	.byte		N11   , Ds4 , v112
	.byte	W12
	.byte		N02   , Dn4 , v088
	.byte	W03
	.byte		        Cn4
	.byte	W03
	.byte		TIE   , Bn3 , v120
	.byte	W66
	.byte	W44
	.byte	W03
	.byte	EOT
	.byte	W13
	.byte		N11   , Fs3 , v127
	.byte	W12
	.byte		N17   , Bn3 , v088
	.byte	W18
	.byte		N05   , Dn4
	.byte	W06
	.byte		N92   , En4 , v104 , gtp3
	.byte	W96
	.byte		N32   , Gn4
	.byte	W32
	.byte	W01
	.byte		N36   , Fs4 , v088 , gtp2
	.byte	W36
	.byte	W03
	.byte		N23   , Fn4
	.byte	W24
	.byte		VOL   , 39*mus_qz_653_mvl/mxv
	.byte		TIE   , Fs4 , v127
	.byte	W05
	.byte		VOL   , 44*mus_qz_653_mvl/mxv
	.byte	W06
	.byte		        52*mus_qz_653_mvl/mxv
	.byte	W06
	.byte		        61*mus_qz_653_mvl/mxv
	.byte	W06
	.byte		        65*mus_qz_653_mvl/mxv
	.byte	W06
	.byte		        73*mus_qz_653_mvl/mxv
	.byte	W06
	.byte		        78*mus_qz_653_mvl/mxv
	.byte	W06
	.byte		        85*mus_qz_653_mvl/mxv
	.byte	W06
	.byte		        87*mus_qz_653_mvl/mxv
	.byte	W06
	.byte		        96*mus_qz_653_mvl/mxv
	.byte	W06
	.byte		        102*mus_qz_653_mvl/mxv
	.byte	W24
	.byte		        99*mus_qz_653_mvl/mxv
	.byte	W06
	.byte		        93*mus_qz_653_mvl/mxv
	.byte	W06
	.byte		        93*mus_qz_653_mvl/mxv
	.byte	W01
	.byte	W05
	.byte		        85*mus_qz_653_mvl/mxv
	.byte	W06
	.byte		        82*mus_qz_653_mvl/mxv
	.byte	W06
	.byte		        78*mus_qz_653_mvl/mxv
	.byte	W06
	.byte		        75*mus_qz_653_mvl/mxv
	.byte	W06
	.byte		        68*mus_qz_653_mvl/mxv
	.byte	W06
	.byte		        62*mus_qz_653_mvl/mxv
	.byte	W06
	.byte		        57*mus_qz_653_mvl/mxv
	.byte	W06
	.byte		        55*mus_qz_653_mvl/mxv
	.byte	W06
	.byte		        51*mus_qz_653_mvl/mxv
	.byte	W06
	.byte		        46*mus_qz_653_mvl/mxv
	.byte	W06
	.byte		        43*mus_qz_653_mvl/mxv
	.byte	W12
	.byte		        40*mus_qz_653_mvl/mxv
	.byte	W18
	.byte	EOT
	.byte	W01
	.byte	W96
	.byte	W72
	.byte		VOICE , 33
	.byte		VOL   , 75*mus_qz_653_mvl/mxv
	.byte		PAN   , c_v+25
	.byte		N05   , En3 , v088
	.byte	W06
	.byte		        Fn3
	.byte	W06
	.byte		        Fs3
	.byte	W06
	.byte		        Gn3
	.byte	W06
	.byte		TIE   , An3
	.byte	W96
	.byte	W32
	.byte	W03
	.byte	EOT
	.byte	W01
	.byte		N23   , Gn3
	.byte	W24
	.byte		N11   , Ds3
	.byte	W12
	.byte		N23   , En3
	.byte	W24
	.byte		TIE   , Fs3
	.byte	W96
	.byte	W23
	.byte	EOT
	.byte	W01
	.byte		N23   , Gn3
	.byte	W24
	.byte		        Bn2
	.byte	W24
	.byte		        Cn3
	.byte	W24
	.byte		TIE   , Cs3
	.byte	W96
	.byte	W23
	.byte	EOT
	.byte	W01
	.byte		N32   , Dn3 , v088 , gtp3
	.byte	W36
	.byte		N23   , Fs3
	.byte	W24
	.byte		N11   , En3
	.byte	W12
	.byte		N32   , Bn2 , v088 , gtp3
	.byte	W36
	.byte		N80   , An2 , v088 , gtp3
	.byte	W60
	.byte	W24
	.byte		N23   , Cn3
	.byte	W24
	.byte		PAN   , c_v+10
	.byte		N23   , Bn2
	.byte	W24
	.byte		VOICE , 32
	.byte		VOL   , 102*mus_qz_653_mvl/mxv
	.byte		N11   , En2 , v116
	.byte	W12
	.byte		        Fn2 , v112
	.byte	W12
	.byte		TIE   , An2 , v088
	.byte	W96
	.byte	W23
	.byte	EOT
	.byte	W01
	.byte		N23   , As2
	.byte	W24
	.byte		        Dn3
	.byte	W24
	.byte		        Cn3
	.byte	W24
	.byte		VOICE , 34
	.byte		PAN   , c_v-12
	.byte		VOL   , 102*mus_qz_653_mvl/mxv
	.byte		TIE   , Cs3
	.byte	W96
	.byte	W23
	.byte	EOT
	.byte	W01
	.byte		N23   , Dn3
	.byte	W24
	.byte		        Bn2
	.byte	W24
	.byte		        Fs3
	.byte	W24
	.byte		VOICE , 35
	.byte		VOL   , 72*mus_qz_653_mvl/mxv
	.byte		PAN   , c_v+38
	.byte		BEND  , c_v+2
	.byte		TIE   , Gn3
	.byte	W96
	.byte	W23
	.byte	EOT
	.byte	W01
	.byte		N23   , Fn3
	.byte	W24
	.byte		        Cn4
	.byte	W24
	.byte		        Ds4
	.byte	W24
	.byte		        As4 , v104
	.byte	W24
	.byte		N07   , An4 , v088
	.byte	W08
	.byte		        Gs4
	.byte	W08
	.byte		        Gn4
	.byte	W08
	.byte		N23   , Fs4 , v104
	.byte	W24
	.byte		N07   , Fn4 , v088
	.byte	W08
	.byte		        En4
	.byte	W08
	.byte		        Ds4
	.byte	W08
	.byte		N23   , Dn4 , v112
	.byte	W24
	.byte		N07   , Cs4 , v088
	.byte	W08
	.byte		        Cn4
	.byte	W08
	.byte		        Bn3
	.byte	W08
	.byte		N23   , As3 , v104
	.byte	W24
	.byte		N07   , An3 , v088
	.byte	W08
	.byte		        Gs3
	.byte	W08
	.byte		        Gn3
	.byte	W08
	.byte		BEND  , c_v+0
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		PAN   , c_v+32
	.byte	W72
	.byte		VOICE , 29
	.byte	W24
	.byte	GOTO
	 ptrvalue	mus_qz_653_2_B1
	.byte	FINE

/********************** Track  3 **********************/

mus_qz_653_3:
	.byte		KEYSH , mus_qz_653_key+0
	.byte		VOICE , 30
	.byte		VOL   , 82*mus_qz_653_mvl/mxv
	.byte		        55*mus_qz_653_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte	W24
	.byte		VOL   , 69*mus_qz_653_mvl/mxv
	.byte		N07   , Gs1 , v112
	.byte	W08
	.byte		        Cn0 , v088
	.byte	W08
	.byte		        Fn0 , v112
	.byte	W08
	.byte		        As0 , v092
	.byte	W08
	.byte		        Ds1 , v100
	.byte	W08
	.byte		        Gs1 , v116
	.byte	W08
	.byte		VOL   , 81*mus_qz_653_mvl/mxv
	.byte		PAN   , c_v+61
	.byte		N03   , Gn1 , v096
	.byte	W08
	.byte		        Gn1 , v088
	.byte	W08
	.byte	N07
	.byte	W08
	.byte		N23   , Cs1
	.byte	W24
	.byte		N07   , Gn1 , v112
	.byte	W08
	.byte		N11   , Gn1 , v048
	.byte	W16
	.byte		N23   , Cs1 , v088
	.byte	W24
	.byte		N07   , Gn1 , v120
	.byte	W08
	.byte		N11   , Gn1 , v048
	.byte	W16
	.byte		        Cs1 , v112
	.byte	W12
	.byte		N16   , Cs1 , v052
	.byte	W12
	.byte	W48
	.byte		PAN   , c_v+42
	.byte	W48
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W48
	.byte		        c_v-32
	.byte	W24
	.byte		VOICE , 23
	.byte		VOL   , 65*mus_qz_653_mvl/mxv
	.byte	W18
	.byte		N02   , Gs5 , v088
	.byte	W03
	.byte		        An5
	.byte	W03
mus_qz_653_3_000:
	.byte		N11   , As5 , v088
	.byte	W24
	.byte		N02   , An5
	.byte		N11   , As5
	.byte	W24
	.byte		N02   , An5
	.byte		N11   , As5
	.byte	W24
	.byte		N02   , An5
	.byte		N11   , As5
	.byte	W18
	.byte		N02   , Gs5
	.byte	W03
	.byte		        An5
	.byte	W03
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_653_3_000
	.byte		N11   , As5 , v088
	.byte	W24
	.byte		N02   , An5
	.byte		N11   , As5
	.byte	W24
	.byte		N02   , An5
	.byte		N11   , As5
	.byte	W24
	.byte		N02   , An5
	.byte		N11   , As5
	.byte	W24
	.byte		N07   , Gn5 , v092
	.byte	W08
	.byte		        Dn5 , v100
	.byte	W08
	.byte		        Gs4 , v116
	.byte	W08
	.byte		        Ds4
	.byte	W08
	.byte		        As3
	.byte	W08
	.byte		        En3
	.byte	W08
	.byte	W96
	.byte	W48
	.byte		VOICE , 29
	.byte		N07   , En1 , v088
	.byte	W08
	.byte		        Bn1
	.byte	W08
	.byte		        Fs2
	.byte	W08
	.byte		        Cs3
	.byte	W08
	.byte		        Gs3
	.byte	W08
	.byte		        Ds4
	.byte	W08
mus_qz_653_3_B1:
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		VOL   , 25*mus_qz_653_mvl/mxv
	.byte	W24
	.byte		VOICE , 9
	.byte		N11   , Bn3 , v088
	.byte	W12
	.byte		N05   , An3
	.byte	W06
	.byte		        Gs3
	.byte	W06
	.byte		N11   , An3
	.byte	W12
	.byte		N05   , Gn3
	.byte	W06
	.byte		        Fs3
	.byte	W06
	.byte		N11   , En3
	.byte	W12
	.byte		N05   , Ds3
	.byte	W06
	.byte		        Cs3
	.byte	W06
	.byte		N23   , Ds3
	.byte	W24
	.byte	N05
	.byte	W06
	.byte		        En3
	.byte	W06
	.byte		        Ds3
	.byte	W06
	.byte		        En3
	.byte	W06
	.byte		        Fn3
	.byte	W12
	.byte		N23   , Fs3
	.byte	W36
	.byte	W96
	.byte	W96
	.byte		VOL   , 61*mus_qz_653_mvl/mxv
	.byte	W48
	.byte		VOICE , 29
	.byte		PAN   , c_v+50
	.byte		VOL   , 76*mus_qz_653_mvl/mxv
	.byte	W12
	.byte		N11   , Fs2 , v104
	.byte	W12
	.byte		N05   , Bn2 , v127
	.byte	W12
	.byte		N11   , Ds2 , v088
	.byte	W12
	.byte		N17   , Fs2 , v127
	.byte	W18
	.byte		N05   , As2 , v088
	.byte	W06
	.byte		N17   , Bn2 , v127
	.byte	W18
	.byte		N13   , Bn2 , v060
	.byte	W06
	.byte		VOL   , 65*mus_qz_653_mvl/mxv
	.byte	W48
	.byte	W96
	.byte	W48
	.byte		PAN   , c_v+35
	.byte		VOL   , 82*mus_qz_653_mvl/mxv
	.byte		N11   , En3 , v100
	.byte	W12
	.byte		N24   , Fn3 , v116 , gtp2
	.byte	W24
	.byte	W03
	.byte		N02   , Ds3 , v104
	.byte	W03
	.byte		        Dn3 , v092
	.byte	W03
	.byte		        Cn3
	.byte	W03
	.byte		VOICE , 30
	.byte		VOL   , 70*mus_qz_653_mvl/mxv
	.byte		PAN   , c_v+30
	.byte		N11   , Bn0 , v127
	.byte	W12
	.byte		        Bn0 , v088
	.byte	W24
	.byte		N32   , Fs1 , v088 , gtp3
	.byte	W36
	.byte		VOL   , 61*mus_qz_653_mvl/mxv
	.byte		N23   , Fn1 , v124
	.byte	W02
	.byte		VOL   , 64*mus_qz_653_mvl/mxv
	.byte	W03
	.byte		        73*mus_qz_653_mvl/mxv
	.byte	W03
	.byte		        82*mus_qz_653_mvl/mxv
	.byte	W16
mus_qz_653_3_001:
	.byte		N11   , Fs1 , v127
	.byte	W36
	.byte		N23   , Fs1 , v088
	.byte	W60
	.byte	PEND
	.byte		N11   , Bn0 , v127
	.byte	W12
	.byte		        Bn0 , v088
	.byte	W24
	.byte		N32   , Fs1 , v088 , gtp3
	.byte	W36
	.byte		N23   , Fn1 , v124
	.byte	W24
	.byte	PATT
	 ptrvalue	mus_qz_653_3_001
	.byte		VOICE , 23
	.byte		VOL   , 50*mus_qz_653_mvl/mxv
	.byte		PAN   , c_v-19
	.byte		N05   , Gn2 , v104
	.byte		N05   , As2
	.byte	W12
	.byte		N11   , As2 , v088
	.byte	W12
	.byte		N05   , Gn2
	.byte		N05   , As2
	.byte	W12
	.byte		N11   , Gn2
	.byte		N11   , As2
	.byte	W12
	.byte		N05   , Gn2 , v104
	.byte		N05   , As2
	.byte	W12
	.byte		        Gn2 , v088
	.byte		N05   , As2
	.byte	W12
	.byte		N11   , Gn2
	.byte		N11   , As2
	.byte	W12
	.byte		        Gn2 , v104
	.byte		N11   , As2
	.byte	W12
	.byte	N05
	.byte	W12
	.byte		        Gn2 , v088
	.byte		N05   , As2
	.byte	W12
	.byte	N05
	.byte	W12
	.byte		        Gn2
	.byte		N05   , As2
	.byte	W12
	.byte		        Gn2 , v104
	.byte		N05   , As2
	.byte	W12
	.byte		        Gn2 , v088
	.byte		N05   , As2
	.byte	W12
	.byte		        Gn2 , v104
	.byte		N05   , As2
	.byte	W12
	.byte		        Gn2 , v088
	.byte		N05   , As2
	.byte	W12
	.byte		        An2 , v104
	.byte		N05   , Cn3
	.byte	W12
	.byte		        Cn3 , v088
	.byte	W12
	.byte		        An2
	.byte		N05   , Cn3
	.byte	W12
	.byte	N05
	.byte	W12
	.byte		        An2 , v104
	.byte		N05   , Cn3
	.byte	W12
	.byte		        An2 , v088
	.byte		N05   , Cn3
	.byte	W12
	.byte	N05
	.byte	W12
	.byte		N11   , An2
	.byte		N11   , Cn3
	.byte	W12
	.byte		N05   , Cn3 , v104
	.byte	W12
	.byte		        An2 , v088
	.byte		N05   , Cn3
	.byte	W12
	.byte	N05
	.byte	W12
	.byte		        An2
	.byte		N05   , Cn3
	.byte	W12
	.byte		        An2 , v104
	.byte		N05   , Cn3
	.byte	W12
	.byte		        Cn3 , v088
	.byte	W12
	.byte		        An2 , v112
	.byte		N05   , Cn3
	.byte	W12
	.byte		        An2 , v088
	.byte		N05   , Cn3
	.byte	W12
	.byte		        En2 , v104
	.byte		N05   , Gn2
	.byte	W12
	.byte		        Gn2 , v088
	.byte	W12
	.byte		        En2
	.byte		N05   , Gn2
	.byte	W12
	.byte	N05
	.byte	W12
	.byte		        En2 , v104
	.byte		N05   , Gn2
	.byte	W12
	.byte		        En2 , v088
	.byte		N05   , Gn2
	.byte	W12
	.byte		        Gn2 , v104
	.byte	W12
	.byte		N11   , En2 , v112
	.byte		N11   , Gn2
	.byte	W12
	.byte		N05   , Gn2 , v104
	.byte	W12
	.byte		        En2 , v088
	.byte		N05   , Gn2
	.byte	W12
	.byte	N05
	.byte	W12
	.byte		        En2
	.byte		N05   , Gn2
	.byte	W12
	.byte		        En2 , v104
	.byte		N05   , Gn2
	.byte	W12
	.byte		        Gn2 , v088
	.byte	W12
	.byte		        En2 , v104
	.byte		N05   , Gn2
	.byte	W12
	.byte		        Gn2 , v088
	.byte	W12
	.byte		        Dn2 , v104
	.byte		N05   , Fn2
	.byte	W12
	.byte		        Fn2 , v088
	.byte	W12
	.byte		        Dn2
	.byte		N05   , Fn2
	.byte	W12
	.byte	N05
	.byte	W12
	.byte		        Dn2 , v112
	.byte		N05   , Fn2
	.byte	W12
	.byte		        Dn2 , v088
	.byte		N05   , Fn2
	.byte	W12
	.byte	N05
	.byte	W12
	.byte		N11   , Dn2 , v104
	.byte		N11   , Fn2
	.byte	W12
	.byte		N05   , Fn2 , v088
	.byte	W12
	.byte		        Dn2
	.byte		N05   , Fn2
	.byte	W12
	.byte	N05
	.byte	W12
	.byte		        Dn2
	.byte		N05   , Fn2
	.byte	W12
	.byte		        Dn2
	.byte		N05   , Fn2
	.byte	W12
	.byte	N05
	.byte	W12
	.byte		        Dn2
	.byte		N05   , Fn2
	.byte	W12
	.byte	N05
	.byte	W12
	.byte		N11   , Gn2 , v112
	.byte	W12
	.byte		N05   , As2 , v088
	.byte	W12
	.byte		N11   , Gn2
	.byte	W12
	.byte		N05   , As2
	.byte	W12
	.byte		N11   , Gn2
	.byte	W12
	.byte		        Gn2 , v112
	.byte		N05   , As2
	.byte	W12
	.byte		        As2 , v088
	.byte	W12
	.byte		N11   , Gn2
	.byte	W12
	.byte		N05   , As2
	.byte	W12
	.byte		N11   , Gn2 , v112
	.byte	W12
	.byte		N05   , As2 , v088
	.byte	W12
	.byte		N11   , Gn2
	.byte	W12
	.byte		        Gn2 , v104
	.byte	W12
	.byte		N05   , As2 , v088
	.byte	W12
	.byte		N11   , Gn2 , v112
	.byte	W12
	.byte		N05   , As2 , v088
	.byte	W12
	.byte		N11   , Bn2 , v112
	.byte	W12
	.byte		        Dn3 , v088
	.byte	W12
	.byte		        Bn2
	.byte	W12
	.byte		        Dn3
	.byte	W12
	.byte		        Bn2
	.byte	W12
	.byte		        Bn2 , v112
	.byte		N11   , Dn3
	.byte	W12
	.byte		        Dn3 , v088
	.byte	W12
	.byte		        Bn2
	.byte	W12
	.byte		        Dn3
	.byte	W12
	.byte		        Bn2 , v112
	.byte	W12
	.byte		        Dn3 , v088
	.byte	W12
	.byte		        Bn2
	.byte	W12
	.byte		        Bn2 , v116
	.byte	W12
	.byte		        Dn3 , v088
	.byte	W12
	.byte		        Bn2 , v116
	.byte	W12
	.byte		        Dn3 , v088
	.byte	W12
	.byte		        Cn3 , v116
	.byte	W12
	.byte		        Gs2 , v088
	.byte	W12
	.byte		        Cn3
	.byte	W12
	.byte		        Gs2
	.byte	W12
	.byte		        Cn3
	.byte	W12
	.byte		        Gs2 , v120
	.byte		N11   , Cn3
	.byte	W12
	.byte		        Gs2 , v088
	.byte	W12
	.byte		        Cn3
	.byte	W12
	.byte		        Gs2
	.byte	W12
	.byte		        Cn3 , v116
	.byte	W12
	.byte		        Gs2 , v088
	.byte	W12
	.byte		        Cn3
	.byte	W12
	.byte		        Dn3 , v112
	.byte	W12
	.byte		        Cn3 , v088
	.byte	W12
	.byte		        Ds3 , v104
	.byte	W24
	.byte	W96
	.byte	W72
	.byte		VOICE , 29
	.byte		PAN   , c_v+31
	.byte		VOL   , 82*mus_qz_653_mvl/mxv
	.byte		N03   , Fs1 , v088
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		VOL   , 102*mus_qz_653_mvl/mxv
	.byte		        97*mus_qz_653_mvl/mxv
	.byte		N92   , Fn1 , v104 , gtp3
	.byte	W24
	.byte		N07   , Fs3 , v088
	.byte	W08
	.byte		N03   , Fs3 , v072
	.byte	W08
	.byte		        Fs3 , v088
	.byte	W08
	.byte		N07   , Fn3 , v076
	.byte	W08
	.byte		N03   , Fn3 , v088
	.byte	W08
	.byte		        Fn3 , v076
	.byte	W08
	.byte		N07   , Gs3 , v088
	.byte	W08
	.byte		N03   , Gs3 , v072
	.byte	W08
	.byte		        Gs3 , v088
	.byte	W08
	.byte		N11   , Bn0
	.byte		N07   , Gn3
	.byte	W08
	.byte		N03   , Gn3 , v068
	.byte	W08
	.byte		        Gn3 , v088
	.byte	W08
	.byte		N36   , Fn1 , v088 , gtp3
	.byte		N07   , As3
	.byte	W08
	.byte		N03   , As3 , v072
	.byte	W08
	.byte		        As3 , v088
	.byte	W08
	.byte		N07   , An3 , v076
	.byte	W08
	.byte		N03   , An3 , v088
	.byte	W08
	.byte		        An3 , v076
	.byte	W08
	.byte		N23   , Fn1 , v088
	.byte		N07   , Cn4
	.byte	W08
	.byte		N03   , Cn4 , v072
	.byte	W08
	.byte		        Cn4 , v088
	.byte	W08
	.byte		N92   , Dn1 , v088 , gtp3
	.byte		N11   , Dn4
	.byte	W12
	.byte		        Dn4 , v040
	.byte	W12
	.byte		N07   , Ds3 , v088
	.byte	W08
	.byte		N03   , Ds3 , v072
	.byte	W08
	.byte		        Ds3 , v088
	.byte	W08
	.byte		N07   , Dn3 , v076
	.byte	W08
	.byte		N03   , Dn3 , v088
	.byte	W08
	.byte		        Dn3 , v076
	.byte	W08
	.byte		N07   , Fn3 , v088
	.byte	W08
	.byte		N03   , Fn3 , v072
	.byte	W08
	.byte		        Fn3 , v088
	.byte	W08
	.byte		N11   , Gs0
	.byte		N07   , En3
	.byte	W08
	.byte		N03   , En3 , v068
	.byte	W08
	.byte		        En3 , v088
	.byte	W08
	.byte		N36   , Dn1 , v088 , gtp3
	.byte		N07   , Gn3
	.byte	W08
	.byte		N03   , Gn3 , v072
	.byte	W08
	.byte		        Gn3 , v088
	.byte	W08
	.byte		N07   , Fs3 , v076
	.byte	W08
	.byte		N03   , Fs3 , v088
	.byte	W08
	.byte		        Fs3 , v076
	.byte	W08
	.byte		N23   , Dn1 , v088
	.byte		N07   , An3
	.byte	W08
	.byte		N03   , An3 , v072
	.byte	W08
	.byte		        An3 , v088
	.byte	W08
	.byte		VOL   , 89*mus_qz_653_mvl/mxv
	.byte		N07   , As3 , v104
	.byte	W09
	.byte		        As3 , v040
	.byte	W36
	.byte	W03
	.byte		        As3 , v088
	.byte	W09
	.byte		        As3 , v040
	.byte	W36
	.byte	W03
	.byte	W24
	.byte		N23   , As3 , v088
	.byte	W24
	.byte		N11   , As3 , v036
	.byte	W24
	.byte		N07   , As3 , v076
	.byte	W08
	.byte		N03   , As3 , v088
	.byte	W08
	.byte		        As3 , v076
	.byte	W08
	.byte		N07   , Cn4 , v112
	.byte	W08
	.byte		N11   , Cn4 , v040
	.byte	W40
	.byte		N07   , Cn4 , v088
	.byte	W08
	.byte		N11   , Cn4 , v040
	.byte	W40
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_653_3_B1
	.byte	FINE

/********************** Track  4 **********************/

mus_qz_653_4:
	.byte		KEYSH , mus_qz_653_key+0
	.byte		VOICE , 30
	.byte		VOL   , 68*mus_qz_653_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		BENDR , 6
	.byte		PRIO  , 64
	.byte	W24
	.byte		VOL   , 68*mus_qz_653_mvl/mxv
	.byte	W21
	.byte		VOICE , 25
	.byte	W15
	.byte		VOL   , 87*mus_qz_653_mvl/mxv
	.byte		PAN   , c_v+32
	.byte	W12
	.byte		        c_v+0
	.byte		        c_v-2
	.byte		VOL   , 81*mus_qz_653_mvl/mxv
	.byte		N05   , Gn1 , v108
	.byte	W06
	.byte		        Gn1 , v088
	.byte	W18
	.byte		N17   , Cs1 , v104
	.byte	W24
	.byte		N05   , Gn1 , v100
	.byte	W06
	.byte		        Gn1 , v088
	.byte	W18
	.byte		N17   , Cs1 , v100
	.byte	W24
	.byte		N09   , Gn1 , v104
	.byte	W48
	.byte	W24
	.byte		VOL   , 42*mus_qz_653_mvl/mxv
	.byte		        34*mus_qz_653_mvl/mxv
	.byte	W01
	.byte		N68   , Cs1 , v088 , gtp2
	.byte	W07
	.byte		VOL   , 41*mus_qz_653_mvl/mxv
	.byte	W08
	.byte		        48*mus_qz_653_mvl/mxv
	.byte	W08
	.byte		PAN   , c_v+16
	.byte		VOL   , 54*mus_qz_653_mvl/mxv
	.byte	W48
	.byte		N92   , Cn1 , v088 , gtp3
	.byte	W96
	.byte		        En1 , v088 , gtp3
	.byte	W96
	.byte		        Ds1 , v088 , gtp3
	.byte	W96
	.byte		N44   , Gn1 , v088 , gtp3
	.byte	W48
	.byte		        Fs1 , v088 , gtp3
	.byte	W48
	.byte		        An1 , v088 , gtp3
	.byte	W48
	.byte		        Gs1 , v088 , gtp3
	.byte	W48
	.byte		N92   , Bn1 , v088 , gtp3
	.byte	W12
	.byte		BEND  , c_v+24
	.byte	W12
	.byte		        c_v-12
	.byte	W72
	.byte		        c_v+0
	.byte		N44   , An1 , v088 , gtp3
	.byte	W48
	.byte		        Gs1 , v088 , gtp3
	.byte	W48
	.byte		VOL   , 62*mus_qz_653_mvl/mxv
	.byte		N09   , Gs0
	.byte		TIE   , Gs1
	.byte	W96
	.byte		N11   , Gs0
	.byte	W24
	.byte	N11
	.byte	W24
	.byte	N11
	.byte	W24
	.byte	N11
	.byte	W24
	.byte	N11
	.byte	W24
	.byte	N11
	.byte	W24
	.byte	N11
	.byte	W24
	.byte	N11
	.byte	W23
	.byte		EOT   , Gs1
	.byte	W01
	.byte		VOICE , 30
	.byte		N44   , Dn1 , v100 , gtp3
	.byte	W03
	.byte		VOL   , 37*mus_qz_653_mvl/mxv
	.byte	W02
	.byte		        41*mus_qz_653_mvl/mxv
	.byte	W03
	.byte		        43*mus_qz_653_mvl/mxv
	.byte	W03
	.byte		        48*mus_qz_653_mvl/mxv
	.byte	W03
	.byte		        53*mus_qz_653_mvl/mxv
	.byte	W03
	.byte		        58*mus_qz_653_mvl/mxv
	.byte	W03
	.byte		        65*mus_qz_653_mvl/mxv
	.byte	W03
	.byte		        70*mus_qz_653_mvl/mxv
	.byte	W03
	.byte		        82*mus_qz_653_mvl/mxv
	.byte	W22
	.byte		N11   , Gs1 , v088
	.byte	W48
	.byte	N11
	.byte	W48
mus_qz_653_4_000:
	.byte	W12
	.byte		N32   , Fs1 , v088 , gtp3
	.byte	W84
	.byte	PEND
mus_qz_653_4_B1:
	.byte		VOICE , 25
	.byte		VOL   , 102*mus_qz_653_mvl/mxv
	.byte		PAN   , c_v-4
	.byte		VOL   , 102*mus_qz_653_mvl/mxv
	.byte		N05   , Bn0 , v112
	.byte	W03
	.byte		N11   , Fs2 , v088
	.byte	W09
	.byte		        Bn0
	.byte	W12
	.byte		        Bn0 , v060
	.byte	W12
	.byte		        Fs0 , v088
	.byte	W36
	.byte		        Bn0
	.byte	W12
	.byte		N23   , Bn0 , v040
	.byte	W12
	.byte	W12
	.byte		N11   , Fs0 , v088
	.byte	W36
	.byte		        Bn0
	.byte	W12
	.byte		N32   , As0 , v120 , gtp3
	.byte	W36
	.byte		N11   , Cn1 , v127
	.byte	W36
	.byte		        Gn0 , v088
	.byte	W36
	.byte		N20   , En0
	.byte	W24
	.byte	W12
	.byte		N11   , Gn0
	.byte	W36
	.byte		        Cn1
	.byte	W12
	.byte		N32   , As0 , v127 , gtp3
	.byte	W36
	.byte		N11   , Bn0
	.byte	W12
	.byte		N05   , Fs1 , v088
	.byte	W24
	.byte		        Bn0
	.byte	W12
	.byte		N11   , Fn1
	.byte	W12
	.byte		N05   , Fs1
	.byte	W12
	.byte		N32   , Bn0 , v088 , gtp3
	.byte	W24
	.byte	W12
	.byte		        Cn1 , v088 , gtp3
	.byte	W36
	.byte		N11   , Bn0
	.byte	W12
	.byte		N32   , An0 , v127 , gtp3
	.byte	W36
	.byte		N11   , Cn1
	.byte	W36
	.byte		        Gn0 , v088
	.byte	W36
	.byte		        Cn1
	.byte	W24
	.byte		VOL   , 102*mus_qz_653_mvl/mxv
	.byte		N11   , Fs1 , v120
	.byte	W12
	.byte		        Gn1 , v088
	.byte	W12
	.byte		        Ds1 , v127
	.byte	W12
	.byte		        En1 , v088
	.byte	W12
	.byte		        Cn1 , v127
	.byte	W12
	.byte		N32   , As0 , v127 , gtp3
	.byte	W36
	.byte		VOL   , 82*mus_qz_653_mvl/mxv
	.byte		        81*mus_qz_653_mvl/mxv
	.byte		N11   , Bn0 , v124
	.byte	W36
	.byte		        Fs0 , v088
	.byte	W36
	.byte		        Bn0
	.byte	W24
	.byte	W12
	.byte		        Fs0
	.byte	W36
	.byte		N23   , Bn0
	.byte	W24
	.byte		        Dn1
	.byte	W24
	.byte		N11   , Bn0 , v112
	.byte	W12
	.byte		        Cn1 , v088
	.byte	W24
	.byte		        En1
	.byte	W36
	.byte		        Gn0 , v112
	.byte	W24
	.byte	W12
	.byte		        Cn1 , v088
	.byte	W36
	.byte	N11
	.byte	W12
	.byte		N23   , En1 , v124
	.byte	W24
	.byte		N02   , Fs1 , v088
	.byte	W06
	.byte	N02
	.byte	W06
	.byte		VOL   , 64*mus_qz_653_mvl/mxv
	.byte		N11   , Bn0
	.byte	W12
	.byte		        Fs1
	.byte	W12
	.byte		        Bn1
	.byte	W12
	.byte		N32   , Cn2 , v088 , gtp3
	.byte	W36
	.byte		N05   , Fn1
	.byte	W12
	.byte	N05
	.byte	W12
	.byte		N11   , Bn0
	.byte	W12
	.byte		        Fs1
	.byte	W12
	.byte		        Bn1
	.byte	W12
	.byte		N32   , Cn2 , v088 , gtp3
	.byte	W36
	.byte		N11   , En1 , v112
	.byte	W12
	.byte		N02   , En1 , v100
	.byte	W06
	.byte		        En1 , v116
	.byte	W06
	.byte		N11   , Bn0 , v088
	.byte	W12
	.byte		        Fs1
	.byte	W12
	.byte		        Bn1
	.byte	W12
	.byte		N32   , Cn2 , v088 , gtp3
	.byte	W36
	.byte		N11   , Fn1
	.byte	W12
	.byte		N02   , Fs1
	.byte	W06
	.byte	N02
	.byte	W06
	.byte		N11   , Bn0
	.byte	W12
	.byte		        Fs1
	.byte	W12
	.byte		        Bn1
	.byte	W12
	.byte		N32   , Cn2 , v088 , gtp3
	.byte	W36
	.byte		N11   , Fs2
	.byte	W24
	.byte		VOICE , 19
	.byte		VOL   , 78*mus_qz_653_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		VOL   , 96*mus_qz_653_mvl/mxv
	.byte		N80   , As0 , v112 , gtp3
	.byte	W84
	.byte		N23   , An1
	.byte	W12
	.byte	W12
	.byte		        Gn1 , v088
	.byte	W24
	.byte		        En1 , v104
	.byte	W24
	.byte		N11   , As0
	.byte	W12
	.byte		N23   , As1 , v112
	.byte	W24
	.byte		N11   , An1 , v088
	.byte	W12
	.byte		N23   , Cn1
	.byte	W24
	.byte		        Fn1 , v112
	.byte	W24
	.byte		        Fs1 , v088
	.byte	W24
	.byte		N92   , Cn2 , v112 , gtp3
	.byte	W12
	.byte	W84
	.byte		N11   , Fs1 , v088
	.byte	W12
	.byte		N56   , Cs1 , v120 , gtp3
	.byte	W60
	.byte		N23   , Gn1 , v088
	.byte	W24
	.byte		N44   , En1 , v112 , gtp3
	.byte	W12
	.byte	W36
	.byte		N32   , Bn1 , v116 , gtp3
	.byte	W36
	.byte		N23   , An1 , v104
	.byte	W24
	.byte		N56   , Dn1 , v127 , gtp3
	.byte	W60
	.byte		N23   , En2 , v116
	.byte	W24
	.byte		        Dn2 , v104
	.byte	W12
	.byte	W12
	.byte		N32   , Fn2 , v104 , gtp3
	.byte	W36
	.byte		N11   , An1 , v112
	.byte	W12
	.byte		        Bn1 , v092
	.byte	W12
	.byte		N23   , Cs1 , v104
	.byte	W24
	.byte		N92   , As0 , v127 , gtp3
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		VOICE , 25
	.byte		PAN   , c_v+48
	.byte		VOL   , 87*mus_qz_653_mvl/mxv
	.byte		N92   , Bn1 , v088 , gtp3
	.byte	W96
	.byte		PAN   , c_v+25
	.byte		N23   , Fn1
	.byte	W24
	.byte		N44   , Bn1 , v088 , gtp3
	.byte	W48
	.byte		N23   , Fn1
	.byte	W24
	.byte		N92   , Gs1 , v088 , gtp3
	.byte	W96
	.byte		N23   , Dn1
	.byte	W24
	.byte		N44   , Gs1 , v088 , gtp3
	.byte	W48
	.byte		N23   , Gs0
	.byte	W24
	.byte		N08   , Gn1
	.byte	W48
	.byte	N08
	.byte	W48
	.byte	W24
	.byte	N23
	.byte	W48
	.byte	N08
	.byte	W24
	.byte		        Dn1
	.byte	W48
	.byte	N08
	.byte	W48
	.byte	PATT
	 ptrvalue	mus_qz_653_4_000
	.byte	GOTO
	 ptrvalue	mus_qz_653_4_B1
	.byte	FINE

/********************** Track  5 **********************/

mus_qz_653_5:
	.byte		KEYSH , mus_qz_653_key+0
	.byte		VOICE , 29
	.byte		VOL   , 82*mus_qz_653_mvl/mxv
	.byte		        55*mus_qz_653_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte	W24
	.byte		VOL   , 69*mus_qz_653_mvl/mxv
	.byte		PAN   , c_v+43
	.byte		N07   , Gs3 , v088
	.byte	W08
	.byte		        Cn2
	.byte	W08
	.byte		        Fn2
	.byte	W08
	.byte		        As2 , v092
	.byte	W08
	.byte		        Ds3 , v100
	.byte	W08
	.byte		        Gs3 , v116
	.byte	W08
	.byte		VOL   , 86*mus_qz_653_mvl/mxv
	.byte		        68*mus_qz_653_mvl/mxv
	.byte		N02   , Cs3 , v108
	.byte		N02   , Gn3
	.byte		N02   , Gn4
	.byte	W06
	.byte		        Cs3 , v088
	.byte		N02   , Gn3
	.byte		N02   , Gn4
	.byte	W06
	.byte		N05   , Cs3
	.byte		N05   , Gn3
	.byte		N05   , Gn4
	.byte	W12
	.byte		N07   , Gn2
	.byte		N07   , Cs3
	.byte	W24
	.byte		        Cs3 , v104
	.byte		N07   , Gn3
	.byte		N07   , Gn4
	.byte	W08
	.byte		        Cs3 , v044
	.byte		N07   , Gn3
	.byte		N07   , Gn4
	.byte	W16
	.byte		        Gn2 , v088
	.byte		N07   , Cs3
	.byte	W16
	.byte		        Gn2
	.byte		N07   , Cs3
	.byte	W08
	.byte		        Cs3 , v104
	.byte		N07   , Gn3
	.byte		N07   , Gn4
	.byte	W08
	.byte		        Cs3 , v048
	.byte		N07   , Gn3
	.byte		N07   , Gn4
	.byte	W16
	.byte		        Gn2 , v104
	.byte		N07   , Cs3
	.byte		N07   , Cs4
	.byte	W08
	.byte		N15   , Gn2 , v044
	.byte		N15   , Cs3
	.byte		N15   , Cs4
	.byte	W16
	.byte	W48
	.byte		VOL   , 55*mus_qz_653_mvl/mxv
	.byte	W48
	.byte	W96
	.byte	W48
	.byte		VOICE , 34
	.byte		PAN   , c_v+0
	.byte	W48
	.byte	W24
	.byte		VOL   , 68*mus_qz_653_mvl/mxv
	.byte		N07   , Fn2 , v088
	.byte	W02
	.byte		        An2
	.byte	W06
	.byte		        Cn3 , v068
	.byte	W02
	.byte		        En3 , v056
	.byte	W06
	.byte		        Bn2 , v080
	.byte	W02
	.byte		        Ds3 , v056
	.byte	W06
	.byte		        Ds2 , v076
	.byte	W02
	.byte		        Gn2
	.byte	W06
	.byte		        Bn2 , v068
	.byte	W02
	.byte		        Ds3 , v052
	.byte	W06
	.byte		        As2 , v076
	.byte	W02
	.byte		        Dn3 , v064
	.byte	W06
	.byte		TIE   , Dn2 , v044
	.byte	W02
	.byte		        Fs2 , v048
	.byte	W22
	.byte	W36
	.byte		VOL   , 50*mus_qz_653_mvl/mxv
	.byte	W05
	.byte		        42*mus_qz_653_mvl/mxv
	.byte	W07
	.byte		        37*mus_qz_653_mvl/mxv
	.byte	W05
	.byte		        32*mus_qz_653_mvl/mxv
	.byte	W07
	.byte		        28*mus_qz_653_mvl/mxv
	.byte	W05
	.byte		        22*mus_qz_653_mvl/mxv
	.byte	W07
	.byte		        18*mus_qz_653_mvl/mxv
	.byte	W05
	.byte		        13*mus_qz_653_mvl/mxv
	.byte	W07
	.byte		        8*mus_qz_653_mvl/mxv
	.byte	W05
	.byte		        4*mus_qz_653_mvl/mxv
	.byte	W06
	.byte		EOT   , Dn2
	.byte	W01
	.byte		VOL   , 2*mus_qz_653_mvl/mxv
	.byte	W01
	.byte		EOT   , Fs2
	.byte	W44
	.byte	W03
	.byte		VOL   , 70*mus_qz_653_mvl/mxv
	.byte		        56*mus_qz_653_mvl/mxv
	.byte	W48
	.byte	W96
	.byte	W44
	.byte	W01
	.byte		VOICE , 29
	.byte	W03
	.byte		VOL   , 65*mus_qz_653_mvl/mxv
	.byte		PAN   , c_v+10
	.byte		        c_v-32
	.byte		VOL   , 65*mus_qz_653_mvl/mxv
	.byte	W48
	.byte		        82*mus_qz_653_mvl/mxv
	.byte		PAN   , c_v-11
	.byte	W12
	.byte		N32   , Dn3 , v104 , gtp1
	.byte		N32   , Gs3 , v127 , gtp1
	.byte	W36
	.byte		        Cs3 , v104 , gtp1
	.byte		N32   , Gn3 , v104 , gtp1
	.byte	W36
	.byte		        En3 , v104 , gtp1
	.byte		N32   , As3 , v127 , gtp1
	.byte	W12
	.byte	W24
	.byte		        Ds3 , v104 , gtp1
	.byte		N32   , An3 , v104 , gtp1
	.byte	W36
	.byte		        Fs3 , v104 , gtp1
	.byte		N32   , Cn4 , v127 , gtp1
	.byte	W36
	.byte		        Fn3 , v104 , gtp1
	.byte		N32   , Bn3 , v108 , gtp1
	.byte	W36
	.byte		        Gn3 , v124 , gtp1
	.byte		N32   , Cs4 , v124 , gtp1
	.byte	W36
	.byte		N68   , Gs3 , v127 , gtp1
	.byte		N68   , Dn4 , v127 , gtp1
	.byte	W24
	.byte	W48
	.byte		N09   , En4
	.byte	W12
	.byte		N13   , En4 , v032
	.byte	W36
	.byte		N09   , En4 , v127
	.byte	W12
	.byte		N13   , En4 , v032
	.byte	W36
	.byte		VOL   , 65*mus_qz_653_mvl/mxv
	.byte	W12
	.byte		N44   , Fs4 , v104 , gtp1
	.byte	W84
mus_qz_653_5_B1:
	.byte		VOICE , 34
	.byte		        31
	.byte		VOL   , 58*mus_qz_653_mvl/mxv
	.byte		PAN   , c_v+63
	.byte		VOL   , 58*mus_qz_653_mvl/mxv
	.byte		N05   , Fs2 , v127
	.byte	W06
	.byte		        Fn2 , v088
	.byte	W06
	.byte		N11   , Fs2
	.byte	W12
	.byte		N32   , Dn2 , v112 , gtp1
	.byte	W36
	.byte		VOICE , 29
	.byte		VOL   , 73*mus_qz_653_mvl/mxv
	.byte		N05   , Bn2 , v104
	.byte		N05   , Dn3
	.byte	W12
	.byte		        As2 , v096
	.byte		N05   , Cs3
	.byte	W12
	.byte		N11   , Bn2 , v104
	.byte		N09   , Dn3
	.byte	W12
	.byte		N17   , Fs3
	.byte	W18
	.byte		N05   , Gs3 , v096
	.byte	W06
	.byte		N17   , Bn3
	.byte	W24
	.byte		VOICE , 34
	.byte		VOL   , 58*mus_qz_653_mvl/mxv
	.byte		        51*mus_qz_653_mvl/mxv
	.byte	W12
	.byte		N11   , Fs2 , v088
	.byte	W12
	.byte		N17   , Fn2
	.byte	W18
	.byte		N05   , Dn3
	.byte	W06
	.byte		N28   , En3 , v092 , gtp1
	.byte	W30
	.byte		N01   , Ds3 , v072
	.byte	W02
	.byte		        Dn3 , v064
	.byte	W02
	.byte		        Cs3 , v060
	.byte	W02
	.byte		N24   , Cn3 , v088 , gtp3
	.byte	W28
	.byte		N01   , Bn2
	.byte	W02
	.byte		        As2
	.byte	W02
	.byte		        An2
	.byte	W02
	.byte		        Gs2
	.byte	W02
	.byte		N32   , Gn2 , v104 , gtp3
	.byte	W24
	.byte	W12
	.byte		        Cn3 , v088 , gtp3
	.byte	W36
	.byte		N11   , En3
	.byte	W12
	.byte		N32   , As2 , v088 , gtp3
	.byte	W36
	.byte		N11   , Dn3
	.byte	W12
	.byte		N44   , Ds3 , v088 , gtp3
	.byte	W84
	.byte	W96
	.byte		PAN   , c_v+0
	.byte		VOL   , 64*mus_qz_653_mvl/mxv
	.byte	W12
	.byte		N11   , Gn2
	.byte	W12
	.byte		        Cn3
	.byte	W12
	.byte		        En3
	.byte	W12
	.byte		        Fs3
	.byte	W12
	.byte		        Gn3
	.byte	W12
	.byte		        En3
	.byte	W12
	.byte		        Cn3
	.byte	W12
	.byte		VOICE , 29
	.byte		VOL   , 64*mus_qz_653_mvl/mxv
	.byte		PAN   , c_v+56
	.byte		N11   , Bn2 , v116
	.byte		N11   , Fs3
	.byte	W12
	.byte		N05   , Cn3 , v088
	.byte		N05   , Gn3
	.byte	W12
	.byte		N02   , Cn3 , v120
	.byte		N02   , Gn3
	.byte	W12
	.byte		N05   , Cn3 , v088
	.byte		N05   , Gn3
	.byte	W12
	.byte		N11   , Dn3 , v104
	.byte		N11   , Gn3
	.byte	W12
	.byte		N24   , En3 , v112 , gtp2
	.byte		N24   , An3 , v112 , gtp2
	.byte	W24
	.byte	W03
	.byte		N02   , Ds3 , v104
	.byte		N02   , Gs3
	.byte	W03
	.byte		        Dn3 , v080
	.byte		N02   , Fs3
	.byte	W03
	.byte		N01   , Cs3 , v092
	.byte		N01   , En3
	.byte	W03
	.byte		VOICE , 23
	.byte		PAN   , c_v-48
	.byte		VOL   , 91*mus_qz_653_mvl/mxv
	.byte		        73*mus_qz_653_mvl/mxv
	.byte		N03   , Bn2 , v120
	.byte	W06
	.byte		N02   , As2 , v088
	.byte	W06
	.byte		N11   , Bn2 , v112
	.byte	W12
	.byte		N32   , Fs3 , v127 , gtp3
	.byte	W36
	.byte		N11   , En3 , v104
	.byte	W12
	.byte		        Ds3 , v088
	.byte	W12
	.byte		        Cs3 , v104
	.byte	W12
	.byte		N17   , Bn2 , v112
	.byte	W18
	.byte		N05   , As2 , v096
	.byte	W06
	.byte		N17   , Gs2 , v112
	.byte	W18
	.byte		N05   , As2 , v092
	.byte	W06
	.byte		N11   , Bn2 , v112
	.byte	W12
	.byte		        Cn3 , v088
	.byte	W12
	.byte		N05   , En3 , v116
	.byte	W06
	.byte		        Fs3 , v127
	.byte	W06
	.byte		        Gn3
	.byte	W06
	.byte		        Bn3
	.byte	W06
	.byte		N32   , Cn4 , v088 , gtp3
	.byte	W36
	.byte		N28   , Dn4 , v088 , gtp1
	.byte	W30
	.byte		N02   , Cs4
	.byte	W03
	.byte		        Bn3
	.byte	W03
	.byte		N28   , An3 , v088 , gtp1
	.byte	W24
	.byte	W06
	.byte		N02   , Dn4
	.byte	W03
	.byte		        Ds4
	.byte	W03
	.byte		N32   , En4 , v088 , gtp3
	.byte	W36
	.byte		N11   , Dn4
	.byte	W12
	.byte		N32   , Cn4 , v088 , gtp3
	.byte	W36
	.byte		VOL   , 91*mus_qz_653_mvl/mxv
	.byte		N92   , Bn3 , v088 , gtp3
	.byte	W96
	.byte		N44   , Cn4 , v088 , gtp3
	.byte	W48
	.byte		        En4 , v088 , gtp3
	.byte	W48
	.byte		N92   , Ds4
	.byte	W96
	.byte		VOL   , 85*mus_qz_653_mvl/mxv
	.byte		N32   , An3 , v112 , gtp3
	.byte	W36
	.byte		N05   , Gs3 , v088
	.byte	W06
	.byte		        Gn3
	.byte	W06
	.byte		N11   , Fs3 , v112
	.byte	W12
	.byte		N05   , Fn3 , v088
	.byte	W06
	.byte		        En3
	.byte	W06
	.byte		N23   , Ds3
	.byte	W24
	.byte		PAN   , c_v-33
	.byte		VOL   , 68*mus_qz_653_mvl/mxv
	.byte		        56*mus_qz_653_mvl/mxv
	.byte		N05   , Dn3 , v104
	.byte		N05   , En3
	.byte	W12
	.byte		N11   , Cn3 , v088
	.byte	W12
	.byte		N05   , Dn3
	.byte		N05   , En3
	.byte	W24
	.byte		        Dn3 , v104
	.byte		N05   , En3
	.byte	W12
	.byte		        Dn3 , v088
	.byte		N05   , En3
	.byte	W24
	.byte		N11   , Dn3 , v104
	.byte		N11   , En3
	.byte	W12
	.byte	W12
	.byte		N05   , Dn3 , v088
	.byte		N05   , En3
	.byte	W24
	.byte		        Dn3
	.byte		N05   , En3
	.byte	W12
	.byte		        Dn3 , v104
	.byte		N05   , En3
	.byte	W24
	.byte		        Dn3
	.byte		N05   , En3
	.byte	W24
	.byte	N05
	.byte		N05   , Fs3
	.byte	W24
	.byte		        En3 , v088
	.byte		N05   , Fs3
	.byte	W24
	.byte		        En3 , v104
	.byte		N05   , Fs3
	.byte	W12
	.byte		        En3 , v088
	.byte		N05   , Fs3
	.byte	W24
	.byte		N11   , En3
	.byte		N11   , Fs3
	.byte	W12
	.byte	W12
	.byte		N05   , En3
	.byte		N05   , Fs3
	.byte	W24
	.byte		        En3
	.byte		N05   , Fs3
	.byte	W12
	.byte		        En3 , v104
	.byte		N05   , Fs3
	.byte	W24
	.byte		        En3 , v112
	.byte		N05   , Fs3
	.byte	W24
	.byte		        Bn2 , v104
	.byte		N05   , Cs3
	.byte	W24
	.byte		        Bn2 , v088
	.byte		N05   , Cs3
	.byte	W24
	.byte		        Bn2 , v104
	.byte		N05   , Cs3
	.byte	W12
	.byte		        Bn2 , v088
	.byte		N05   , Cs3
	.byte	W24
	.byte		N11   , Bn2 , v112
	.byte		N11   , Cs3
	.byte	W12
	.byte	W12
	.byte		N05   , Bn2 , v088
	.byte		N05   , Cs3
	.byte	W24
	.byte		        Bn2
	.byte		N05   , Cs3
	.byte	W12
	.byte		        Bn2 , v104
	.byte		N05   , Cs3
	.byte	W24
	.byte		        Bn2
	.byte		N05   , Cs3
	.byte	W24
	.byte		        An2
	.byte		N05   , Bn2
	.byte	W24
	.byte		        An2 , v088
	.byte		N05   , Bn2
	.byte	W24
	.byte		        An2 , v112
	.byte		N05   , Bn2
	.byte	W12
	.byte		        An2 , v088
	.byte		N05   , Bn2
	.byte	W24
	.byte		N11   , An2 , v104
	.byte		N11   , Bn2
	.byte	W12
	.byte	W12
	.byte		N05   , An2 , v088
	.byte		N05   , Bn2
	.byte	W24
	.byte		        An2
	.byte		N05   , Bn2
	.byte	W12
	.byte		        An2
	.byte		N05   , Bn2
	.byte	W24
	.byte		        An2
	.byte		N05   , Bn2
	.byte	W24
	.byte		VOL   , 54*mus_qz_653_mvl/mxv
	.byte		N11   , Dn3 , v112
	.byte		N11   , En3
	.byte	W24
	.byte		        Dn3 , v088
	.byte		N11   , En3
	.byte	W24
	.byte		        Dn3
	.byte		N11   , En3
	.byte	W12
	.byte		        Dn3 , v112
	.byte		N11   , En3
	.byte	W24
	.byte		        Dn3 , v088
	.byte		N11   , En3
	.byte	W12
	.byte	W12
	.byte		        Dn3 , v112
	.byte		N11   , En3
	.byte	W24
	.byte		        Dn3 , v088
	.byte		N11   , En3
	.byte	W12
	.byte		        Dn3 , v104
	.byte		N11   , En3
	.byte	W24
	.byte		        Dn3 , v112
	.byte		N11   , En3
	.byte	W24
	.byte		        Fs3
	.byte		N11   , Gs3
	.byte	W24
	.byte		        Fs3 , v088
	.byte		N11   , Gs3
	.byte	W24
	.byte		        Fs3
	.byte		N11   , Gs3
	.byte	W12
	.byte		        Fs3 , v112
	.byte		N11   , Gs3
	.byte	W24
	.byte		        Fs3 , v088
	.byte		N11   , Gs3
	.byte	W12
	.byte	W12
	.byte		        Fs3 , v112
	.byte		N11   , Gs3
	.byte	W24
	.byte		        Fs3 , v088
	.byte		N11   , Gs3
	.byte	W12
	.byte		        Fs3 , v116
	.byte		N11   , Gs3
	.byte	W24
	.byte		        Fs3
	.byte		N11   , Gs3
	.byte	W24
	.byte		VOL   , 65*mus_qz_653_mvl/mxv
	.byte		N11   , Dn3
	.byte		N11   , Gn3
	.byte	W24
	.byte		        Dn3 , v088
	.byte		N11   , Gn3
	.byte	W24
	.byte		        Dn3
	.byte		N11   , Gn3
	.byte	W12
	.byte		        Dn3 , v120
	.byte		N11   , Gn3
	.byte	W24
	.byte		        Dn3 , v088
	.byte		N11   , Gn3
	.byte	W12
	.byte	W12
	.byte		        Dn3 , v116
	.byte		N11   , Gn3
	.byte	W24
	.byte		        Dn3 , v088
	.byte		N11   , Gn3
	.byte	W12
	.byte		        Gn3 , v112
	.byte		N11   , As3
	.byte	W24
	.byte		        Gn3 , v104
	.byte		N11   , As3
	.byte	W12
	.byte		        Gn3 , v112
	.byte		N11   , Cn4
	.byte	W12
	.byte		N23   , Dn4 , v104
	.byte		N23   , Fn4
	.byte	W24
	.byte		N07   , Cs4 , v088
	.byte		N07   , En4
	.byte	W08
	.byte		        Cn4
	.byte		N07   , Ds4
	.byte	W08
	.byte		        Bn3
	.byte		N07   , Dn4
	.byte	W08
	.byte		N23   , As3 , v116
	.byte		N23   , Cs4
	.byte	W24
	.byte		N07   , An3 , v088
	.byte		N07   , Cn4
	.byte	W08
	.byte		        Gs3
	.byte		N07   , Bn3
	.byte	W08
	.byte		        Gn3
	.byte		N07   , As3
	.byte	W08
	.byte		N23   , Fs3 , v104
	.byte		N23   , An3
	.byte	W24
	.byte		N07   , Fn3 , v088
	.byte		N07   , Gs3
	.byte	W08
	.byte		        En3
	.byte		N07   , Gn3
	.byte	W08
	.byte		        Ds3
	.byte		N07   , Fs3
	.byte	W08
	.byte		N23   , Dn3 , v104
	.byte		N23   , Fn3
	.byte	W24
	.byte		N07   , Cs3 , v088
	.byte		N07   , En3
	.byte	W08
	.byte		        Cn3
	.byte		N07   , Ds3
	.byte	W08
	.byte		        Bn2
	.byte		N07   , Dn3
	.byte	W08
	.byte		N92   , Dn2 , v104 , gtp3
	.byte	W96
	.byte		N23   , Bn1 , v088
	.byte	W24
	.byte		        Fs2
	.byte	W24
	.byte		        Fn2
	.byte	W24
	.byte		        Bn1
	.byte	W24
	.byte		N92   , Cn2 , v088 , gtp3
	.byte	W96
	.byte		N23   , Bn1
	.byte	W24
	.byte		        Fs2
	.byte	W24
	.byte		        Fn2
	.byte	W24
	.byte		        An2
	.byte	W24
	.byte		VOL   , 76*mus_qz_653_mvl/mxv
	.byte		        95*mus_qz_653_mvl/mxv
	.byte		N08   , Gn2
	.byte	W09
	.byte		N11   , Gn2 , v032
	.byte	W36
	.byte	W03
	.byte		N08   , Gn2 , v088
	.byte	W09
	.byte		N11   , Gn2 , v036
	.byte	W36
	.byte	W03
	.byte	W24
	.byte		N23   , Gn2 , v088
	.byte	W24
	.byte		N11   , Gn2 , v040
	.byte	W48
	.byte		N08   , Dn2 , v088
	.byte	W10
	.byte		        Dn2 , v040
	.byte	W36
	.byte	W02
	.byte		        Dn2 , v084
	.byte	W10
	.byte		        Dn2 , v036
	.byte	W36
	.byte	W02
	.byte		VOICE , 29
	.byte		VOL   , 58*mus_qz_653_mvl/mxv
	.byte		PAN   , c_v+35
	.byte	W12
	.byte		N44   , Bn3 , v112 , gtp1
	.byte		N44   , Fs4 , v112 , gtp1
	.byte	W84
	.byte	GOTO
	 ptrvalue	mus_qz_653_5_B1
	.byte	FINE

/********************** Track  6 **********************/

mus_qz_653_6:
	.byte		KEYSH , mus_qz_653_key+0
	.byte		VOL   , 82*mus_qz_653_mvl/mxv
	.byte		        55*mus_qz_653_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte	W24
	.byte		VOICE , 35
	.byte		VOL   , 82*mus_qz_653_mvl/mxv
	.byte		PAN   , c_v-16
	.byte		        c_v+23
	.byte		N07   , Cs6 , v088
	.byte	W08
	.byte		        Gs5
	.byte	W08
	.byte		        Ds5
	.byte	W08
	.byte		        Fn4
	.byte	W08
	.byte		        As4
	.byte	W08
	.byte		        Ds5
	.byte	W08
	.byte		VOICE , 61
	.byte		VOL   , 48*mus_qz_653_mvl/mxv
	.byte		PAN   , c_v-2
	.byte		N42   , Gn2 , v104 , gtp1
	.byte	W48
	.byte		        Gn2 , v088 , gtp1
	.byte	W48
	.byte	N23
	.byte	W24
	.byte		N68   , Cs3 , v088 , gtp3
	.byte	W24
	.byte	W48
	.byte		VOICE , 0
	.byte	W48
	.byte	W48
	.byte		N44   , Cn1 , v096 , gtp3
	.byte	W02
	.byte		N07   , Cn3 , v048
	.byte	W06
	.byte		        Fs1 , v060
	.byte	W02
	.byte		N60   , Fs3 , v060 , gtp3
	.byte	W06
	.byte		N07   , Bn1 , v068
	.byte	W02
	.byte		N78   , Bn3 , v068 , gtp1
	.byte	W06
	.byte		N07   , Fn2 , v080
	.byte	W02
	.byte		N78   , Fn4 , v080 , gtp1
	.byte	W06
	.byte		N36   , Fn3 , v076 , gtp3
	.byte		N78   , Fn5 , v076 , gtp1
	.byte	W16
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W48
	.byte		VOL   , 61*mus_qz_653_mvl/mxv
	.byte		PAN   , c_v-32
	.byte		VOL   , 65*mus_qz_653_mvl/mxv
	.byte		PAN   , c_v-35
	.byte		N06   , En1 , v112
	.byte		N06   , Gs1
	.byte	W08
	.byte		        As1
	.byte		N06   , Dn2
	.byte	W08
	.byte		        Ds2 , v088
	.byte		N06   , Gn2
	.byte	W08
	.byte		PAN   , c_v-19
	.byte		N07   , An2 , v084
	.byte		N07   , Cs3
	.byte	W08
	.byte		N04   , An3 , v076
	.byte		N06   , Cs4
	.byte	W07
	.byte		PAN   , c_v-20
	.byte	W01
	.byte		        c_v+16
	.byte		N06   , Gs3 , v088
	.byte	W01
	.byte		N05   , En3 , v068
	.byte	W07
	.byte		PAN   , c_v+40
	.byte		N05   , As3 , v088
	.byte		N05   , Dn4
	.byte	W08
	.byte		N06   , Ds4 , v080
	.byte		N06   , Gn4
	.byte	W08
	.byte		N07   , An4
	.byte		N07   , Cs5
	.byte	W08
	.byte		N40   , Fn5 , v044
	.byte		N30   , Cs6 , v060 , gtp1
	.byte	W24
	.byte		VOL   , 53*mus_qz_653_mvl/mxv
	.byte	W48
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W48
	.byte		VOICE , 29
	.byte		PAN   , c_v-25
	.byte		VOL   , 52*mus_qz_653_mvl/mxv
	.byte		N11   , Gn2 , v088
	.byte	W12
	.byte		N15   , Gn2 , v032
	.byte	W36
	.byte		N11   , Gn2 , v088
	.byte	W12
	.byte		N15   , Gn2 , v032
	.byte	W36
	.byte	W12
	.byte		N32   , Fs2 , v088 , gtp2
	.byte	W84
mus_qz_653_6_B1:
	.byte		VOICE , 29
	.byte		VOL   , 64*mus_qz_653_mvl/mxv
	.byte		PAN   , c_v+48
	.byte	W60
	.byte		VOL   , 78*mus_qz_653_mvl/mxv
	.byte		N05   , Fs3 , v096
	.byte	W12
	.byte		        Fn3
	.byte	W12
	.byte		N11   , Fs3
	.byte	W12
	.byte		N17   , Bn3
	.byte	W18
	.byte		N05   , Cs4
	.byte	W06
	.byte		N17   , Dn4
	.byte	W24
	.byte		VOL   , 64*mus_qz_653_mvl/mxv
	.byte	W48
	.byte	W96
	.byte	W96
	.byte		PAN   , c_v+0
	.byte		N05   , Bn4 , v048
	.byte	W06
	.byte		N03   , Ds5 , v052
	.byte	W06
	.byte		        Fs5 , v064
	.byte	W06
	.byte		        An5 , v072
	.byte	W06
	.byte		VOICE , 35
	.byte		N11   , Bn5 , v104
	.byte	W12
	.byte		N05   , An5 , v088
	.byte	W06
	.byte		        Gs5
	.byte	W06
	.byte		N11   , An5 , v104
	.byte	W12
	.byte		N05   , Gn5 , v088
	.byte	W06
	.byte		        Fs5
	.byte	W06
	.byte		N11   , En5
	.byte	W12
	.byte		N05   , Ds5
	.byte	W06
	.byte		        Cs5
	.byte	W06
	.byte		N23   , Ds5 , v104
	.byte	W24
	.byte	N05
	.byte	W06
	.byte		        En5 , v088
	.byte	W06
	.byte		        Ds5
	.byte	W06
	.byte		        En5
	.byte	W06
	.byte		        Fn5
	.byte	W12
	.byte		N23   , Fs5 , v116
	.byte	W24
	.byte		N02   , En5 , v080
	.byte	W03
	.byte		        Ds5 , v076
	.byte	W03
	.byte		        Cn5
	.byte	W03
	.byte		        Bn4 , v072
	.byte	W03
	.byte	W96
	.byte	W96
	.byte		PAN   , c_v+45
	.byte		VOL   , 48*mus_qz_653_mvl/mxv
	.byte	W48
	.byte		VOICE , 29
	.byte		VOL   , 78*mus_qz_653_mvl/mxv
	.byte	W12
	.byte		N11   , Ds2 , v104
	.byte	W12
	.byte		N05   , Fs2 , v127
	.byte	W12
	.byte		N11   , Bn1 , v088
	.byte	W12
	.byte		N17   , Ds2 , v127
	.byte	W18
	.byte		N05   , Fn2 , v088
	.byte	W06
	.byte		N17   , Fs2 , v127
	.byte	W18
	.byte		N13   , Fs2 , v060
	.byte	W06
	.byte		VOL   , 65*mus_qz_653_mvl/mxv
	.byte	W48
	.byte	W96
	.byte	W84
	.byte		N05   , Gn2 , v088
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		PAN   , c_v+32
	.byte		N11   , Fs1 , v120
	.byte	W12
	.byte		        Bn1 , v088
	.byte	W12
	.byte		        Fs2 , v116
	.byte	W12
	.byte		N32   , Gn2 , v112 , gtp3
	.byte	W36
	.byte		N11   , Bn1 , v088
	.byte	W12
	.byte		N05   , Gn2
	.byte	W06
	.byte	N05
	.byte	W06
mus_qz_653_6_000:
	.byte		N11   , Fs1 , v088
	.byte	W12
	.byte		        Bn1
	.byte	W12
	.byte		        Fs2
	.byte	W12
	.byte		N32   , Gn2 , v088 , gtp3
	.byte	W36
	.byte		N11   , An2
	.byte	W24
	.byte	PEND
	.byte		        Fs1
	.byte	W12
	.byte		        Bn1
	.byte	W12
	.byte		        Fs2
	.byte	W12
	.byte		N32   , Gn2 , v088 , gtp3
	.byte	W36
	.byte		N11   , Bn1
	.byte	W12
	.byte		N05   , Gn2
	.byte	W06
	.byte	N05
	.byte	W06
	.byte	PATT
	 ptrvalue	mus_qz_653_6_000
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		VOICE , 27
	.byte	W96
	.byte		VOL   , 34*mus_qz_653_mvl/mxv
	.byte		PAN   , c_v-48
	.byte		VOL   , 37*mus_qz_653_mvl/mxv
	.byte		N05   , Bn3 , v104
	.byte	W06
	.byte		        Gn3 , v088
	.byte	W06
	.byte		        En3 , v104
	.byte	W06
	.byte		        Cs3 , v080
	.byte	W06
	.byte		N08   , Bn2
	.byte	W06
	.byte		N05   , Cs3 , v088
	.byte	W03
	.byte		        En3
	.byte	W03
	.byte		        Gn3
	.byte	W03
	.byte		        Bn3
	.byte	W03
	.byte		        Cs4
	.byte	W03
	.byte		N02   , En4 , v092
	.byte	W03
	.byte		PAN   , c_v+26
	.byte		N44   , Gn4 , v104
	.byte	W03
	.byte		N02   , Bn4 , v088
	.byte	W03
	.byte		N36   , Cs5 , v104 , gtp2
	.byte	W42
	.byte	W24
	.byte		N05   , Cs5 , v112
	.byte	W06
	.byte		        Bn4 , v088
	.byte	W06
	.byte		        Gn4 , v096
	.byte	W06
	.byte		        En4 , v088
	.byte	W06
	.byte		PAN   , c_v-34
	.byte		N05   , Cs4 , v104
	.byte	W06
	.byte		        Bn3 , v088
	.byte	W06
	.byte		        Gn3
	.byte	W06
	.byte		        En3
	.byte	W06
	.byte		        Gn3 , v104
	.byte	W06
	.byte		        Bn3 , v088
	.byte	W06
	.byte		        Cs4
	.byte	W06
	.byte		        En4
	.byte	W06
	.byte		        An4 , v104
	.byte	W06
	.byte		        Fn4 , v088
	.byte	W06
	.byte		        Dn4
	.byte	W06
	.byte		        Bn3
	.byte	W06
	.byte		        An3
	.byte	W06
	.byte		N08   , Fn3 , v127
	.byte	W06
	.byte		N05   , Dn3 , v084
	.byte	W06
	.byte		        Bn2 , v120
	.byte	W06
	.byte		N08   , An2 , v080
	.byte	W06
	.byte		N05   , Bn2 , v088
	.byte	W03
	.byte		        Dn3
	.byte	W03
	.byte		        Fn3
	.byte	W03
	.byte		        An3
	.byte	W03
	.byte		        Bn3
	.byte	W03
	.byte		N02   , Dn4
	.byte	W03
	.byte		PAN   , c_v+32
	.byte		N44   , Fn4 , v104
	.byte	W03
	.byte		N02   , An4 , v088
	.byte	W03
	.byte		N36   , Bn4 , v116 , gtp2
	.byte	W18
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		VOICE , 8
	.byte		PAN   , c_v-36
	.byte		VOL   , 52*mus_qz_653_mvl/mxv
	.byte		N44   , Fs2 , v092 , gtp3
	.byte		N44   , Bn2 , v092 , gtp3
	.byte		N44   , Fs3 , v092 , gtp3
	.byte	W48
	.byte		PAN   , c_v+51
	.byte		N44   , Fs2 , v068 , gtp3
	.byte		N44   , Bn2 , v068 , gtp3
	.byte		N44   , Fs3 , v068 , gtp3
	.byte	W48
	.byte		PAN   , c_v-24
	.byte		N23   , Dn3 , v092
	.byte		N23   , Gn3
	.byte		N23   , Dn4
	.byte	W24
	.byte		N44   , Bn2 , v088 , gtp3
	.byte		N44   , En3 , v088 , gtp3
	.byte		N44   , Bn3 , v088 , gtp3
	.byte	W48
	.byte		PAN   , c_v+51
	.byte		N23   , Bn2 , v080
	.byte		N23   , En3
	.byte		N23   , Bn3
	.byte	W24
	.byte		PAN   , c_v-36
	.byte		        c_v-36
	.byte		VOL   , 52*mus_qz_653_mvl/mxv
	.byte		N44   , Ds2 , v092 , gtp3
	.byte		N44   , Gs2 , v092 , gtp3
	.byte		N44   , Ds3 , v092 , gtp3
	.byte	W48
	.byte		PAN   , c_v+51
	.byte		        c_v+51
	.byte		N44   , Ds2 , v068 , gtp3
	.byte		N44   , Gs2 , v068 , gtp3
	.byte		N44   , Ds3 , v068 , gtp3
	.byte	W48
	.byte		PAN   , c_v-24
	.byte		        c_v-24
	.byte		N23   , Bn2 , v092
	.byte		N23   , En3
	.byte		N23   , Bn3
	.byte	W24
	.byte		N44   , Gs2 , v088 , gtp3
	.byte		N44   , Cs3 , v088 , gtp3
	.byte		N44   , Gs3 , v088 , gtp3
	.byte	W48
	.byte		PAN   , c_v+51
	.byte		N23   , Gs2 , v080
	.byte		N23   , Cs3
	.byte		N23   , Gs3
	.byte	W24
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		VOICE , 23
	.byte		VOL   , 69*mus_qz_653_mvl/mxv
	.byte		PAN   , c_v-48
	.byte	W12
	.byte		N32   , Fs2 , v100 , gtp2
	.byte	W84
	.byte	GOTO
	 ptrvalue	mus_qz_653_6_B1
	.byte	FINE

/********************** Track  7 **********************/

mus_qz_653_7:
	.byte		KEYSH , mus_qz_653_key+0
	.byte		VOICE , 23
	.byte		VOL   , 82*mus_qz_653_mvl/mxv
	.byte		        55*mus_qz_653_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte	W24
	.byte		VOL   , 68*mus_qz_653_mvl/mxv
	.byte		        100*mus_qz_653_mvl/mxv
	.byte		PAN   , c_v-29
	.byte		N07   , Fn2 , v120
	.byte		N07   , Cs4
	.byte	W08
	.byte		        As2 , v088
	.byte		N07   , Gs3
	.byte	W08
	.byte		        Fn2
	.byte		N07   , Ds3
	.byte	W08
	.byte		        As2
	.byte		N07   , Gs3
	.byte	W08
	.byte		        Ds3
	.byte		N07   , Cs4
	.byte	W04
	.byte		PAN   , c_v-32
	.byte	W04
	.byte		N07   , Gs3
	.byte		N07   , Fs4
	.byte	W08
	.byte		VOL   , 93*mus_qz_653_mvl/mxv
	.byte		N07   , Fn3 , v104
	.byte		N07   , Gn4
	.byte	W08
	.byte		N11   , Fn3 , v060
	.byte		N11   , Gn4
	.byte	W40
	.byte		N07   , Fn3 , v104
	.byte		N07   , Gn4
	.byte	W08
	.byte		N11   , Fn3 , v056
	.byte		N11   , Gn4
	.byte	W40
	.byte		PAN   , c_v-34
	.byte		N07   , Fn3 , v104
	.byte		N07   , Gn4
	.byte	W08
	.byte		        Fn3 , v088
	.byte		N07   , Gn4
	.byte	W08
	.byte	N07
	.byte	W08
	.byte		        Bn2 , v096
	.byte		N07   , Cs5
	.byte	W08
	.byte		N15   , Bn2 , v028
	.byte		N15   , Cs5
	.byte	W16
	.byte	W48
	.byte		VOL   , 73*mus_qz_653_mvl/mxv
	.byte	W48
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W48
	.byte		PAN   , c_v-28
	.byte	W48
	.byte	W48
	.byte		        c_v-25
	.byte	W48
	.byte	W96
	.byte	W96
	.byte		N07   , En2 , v092
	.byte	W08
	.byte		        Dn3 , v100
	.byte	W08
	.byte		        Fs3 , v116
	.byte	W08
	.byte		        Bn3
	.byte	W08
	.byte		        Cs4
	.byte	W08
	.byte		        An3
	.byte	W06
	.byte		PAN   , c_v-56
	.byte	W02
	.byte		N11   , As3
	.byte	W12
	.byte		        As3 , v036
	.byte	W36
	.byte		        As3 , v116
	.byte	W12
	.byte		        As3 , v036
	.byte	W36
	.byte	W12
	.byte		N42   , Cs4 , v116 , gtp1
	.byte	W36
	.byte		PAN   , c_v+54
	.byte		N07   , Bn4 , v096
	.byte		N07   , En5
	.byte	W08
	.byte		PAN   , c_v+16
	.byte		N07   , Fs4 , v104
	.byte		N07   , Bn4
	.byte	W08
	.byte		PAN   , c_v-24
	.byte		N07   , Cs4 , v096
	.byte		N07   , Fs4
	.byte	W08
	.byte		VOL   , 93*mus_qz_653_mvl/mxv
	.byte		PAN   , c_v-48
	.byte		VOL   , 102*mus_qz_653_mvl/mxv
	.byte		N07   , Gs3 , v112
	.byte		N07   , Cs4
	.byte	W08
	.byte		        Ds3 , v127
	.byte		N07   , Gs3
	.byte	W04
	.byte		PAN   , c_v-56
	.byte	W04
	.byte		N07   , As2
	.byte		N07   , Ds3
	.byte	W08
mus_qz_653_7_B1:
	.byte		PAN   , c_v-39
	.byte		VOL   , 81*mus_qz_653_mvl/mxv
	.byte		        65*mus_qz_653_mvl/mxv
	.byte	W12
	.byte		N11   , Cs4 , v104
	.byte		N11   , Fn4
	.byte	W12
	.byte		N05   , Dn4 , v088
	.byte		N05   , Fs4
	.byte	W12
	.byte		        Dn3 , v104
	.byte	W12
	.byte		N11   , As3
	.byte		N11   , Cs4
	.byte	W12
	.byte		N05   , Bn3 , v088
	.byte		N05   , Dn4
	.byte	W12
	.byte		        Bn2 , v092
	.byte	W12
	.byte		N11   , Fn3 , v112
	.byte		N11   , As3
	.byte	W12
	.byte		VOL   , 57*mus_qz_653_mvl/mxv
	.byte		N05   , Fs3 , v088
	.byte		N05   , Bn3
	.byte	W24
	.byte		VOICE , 26
	.byte		VOL   , 72*mus_qz_653_mvl/mxv
	.byte		N11   , Fs1 , v127
	.byte		N11   , Fs2
	.byte	W12
	.byte		N05   , Bn2 , v104
	.byte		N05   , Fs3
	.byte	W06
	.byte		        Bn2 , v044
	.byte		N05   , Fs3
	.byte	W06
	.byte		        Fs2 , v104
	.byte		N05   , Bn2
	.byte	W06
	.byte		        Fs2 , v044
	.byte		N05   , Bn2
	.byte	W06
	.byte		        Fs3 , v112
	.byte		N05   , Bn3
	.byte	W06
	.byte		        Fs3 , v044
	.byte		N05   , Bn3
	.byte	W06
	.byte		        Bn3 , v112
	.byte		N05   , Dn4
	.byte	W06
	.byte		        Bn3 , v048
	.byte		N05   , Dn4
	.byte	W06
	.byte		        Dn4 , v120
	.byte		N05   , Fs4
	.byte	W06
	.byte		VOICE , 23
	.byte		N05   , Fs3 , v088
	.byte		N05   , Cs4
	.byte	W06
	.byte		VOL   , 64*mus_qz_653_mvl/mxv
	.byte		        51*mus_qz_653_mvl/mxv
	.byte		N32   , Gn3 , v116 , gtp3
	.byte		N32   , Dn4 , v116 , gtp3
	.byte	W36
	.byte		        Fs3 , v096 , gtp3
	.byte		N32   , Cn4 , v096 , gtp3
	.byte	W36
	.byte		        An3 , v112 , gtp3
	.byte		N32   , En4 , v112 , gtp3
	.byte	W24
	.byte	W12
	.byte		        Fs3 , v104 , gtp3
	.byte		N32   , Cn4 , v104 , gtp3
	.byte	W36
	.byte		N11   , Cn3 , v112
	.byte		N11   , Bn3
	.byte	W12
	.byte		N32   , Fn3 , v104 , gtp3
	.byte		N32   , Cn4 , v104 , gtp3
	.byte	W36
	.byte		VOICE , 29
	.byte		VOL   , 78*mus_qz_653_mvl/mxv
	.byte		PAN   , c_v+40
	.byte		N11   , As2 , v088
	.byte	W12
	.byte		N02   , Bn2
	.byte	W24
	.byte	N05
	.byte	W12
	.byte		        Bn2 , v056
	.byte	W12
	.byte		        As2 , v088
	.byte	W06
	.byte		        Bn2
	.byte	W06
	.byte		N32   , Ds3 , v088 , gtp3
	.byte	W24
	.byte	W12
	.byte		        Cn3 , v088 , gtp3
	.byte	W36
	.byte		VOICE , 23
	.byte		PAN   , c_v-44
	.byte		VOL   , 68*mus_qz_653_mvl/mxv
	.byte		N11   , Bn2
	.byte		N11   , En3
	.byte	W12
	.byte		N23   , An2
	.byte		N23   , Fs3
	.byte	W24
	.byte		N02   , Gn2
	.byte		N02   , En3
	.byte	W03
	.byte		        Fs2
	.byte		N02   , Dn3
	.byte	W03
	.byte		        En2
	.byte		N02   , Cn3
	.byte	W03
	.byte		        Dn2
	.byte		N02   , An2
	.byte	W03
	.byte		VOICE , 26
	.byte		PAN   , c_v-52
	.byte	W12
	.byte		N11   , En2
	.byte		N11   , Gn2
	.byte	W12
	.byte	N11
	.byte		N11   , Cn3
	.byte	W12
	.byte	N11
	.byte		N11   , En3
	.byte	W12
	.byte		VOICE , 23
	.byte	W12
	.byte		N11   , Gn2
	.byte		N11   , Cn3
	.byte	W12
	.byte	N11
	.byte		N11   , En3
	.byte	W12
	.byte	N11
	.byte		N11   , Gn3
	.byte	W12
	.byte		        Gn2 , v120
	.byte		N11   , Cn3
	.byte	W12
	.byte		        Cn3 , v088
	.byte		N11   , En3
	.byte	W12
	.byte		        En3 , v127
	.byte		N11   , Gn3
	.byte	W12
	.byte		        Cn4 , v100
	.byte	W12
	.byte		        Fs3 , v127
	.byte		N11   , An3
	.byte	W12
	.byte		N32   , Gn3 , v127 , gtp3
	.byte		N32   , As3 , v127 , gtp3
	.byte	W36
	.byte		VOL   , 56*mus_qz_653_mvl/mxv
	.byte	W96
	.byte	W24
	.byte		VOICE , 26
	.byte	W66
	.byte		        23
	.byte	W06
	.byte		        32
	.byte		PAN   , c_v-12
	.byte		VOL   , 73*mus_qz_653_mvl/mxv
	.byte		N32   , Gn2 , v088 , gtp3
	.byte		N32   , Dn3 , v088 , gtp3
	.byte	W36
	.byte		        Cn3 , v088 , gtp3
	.byte		N32   , Gn3 , v088 , gtp3
	.byte	W36
	.byte		        Cn2 , v104 , gtp3
	.byte		N32   , Gn2 , v104 , gtp3
	.byte	W24
	.byte	W12
	.byte		        Dn2 , v088 , gtp3
	.byte		N32   , Cn3 , v088 , gtp3
	.byte	W32
	.byte	W01
	.byte		VOICE , 29
	.byte	W03
	.byte		PAN   , c_v+51
	.byte		VOL   , 76*mus_qz_653_mvl/mxv
	.byte		N11   , Gn2 , v100
	.byte	W12
	.byte		N24   , As2 , v116 , gtp2
	.byte	W24
	.byte	W03
	.byte		N02   , An2 , v088
	.byte	W03
	.byte		        Gs2
	.byte	W03
	.byte		        Fs2
	.byte	W03
	.byte		VOICE , 26
	.byte		PAN   , c_v-44
	.byte		VOL   , 82*mus_qz_653_mvl/mxv
	.byte		N11   , Fs4 , v127
	.byte	W12
	.byte		        Bn3
	.byte	W12
	.byte		        Fs3
	.byte	W12
	.byte		        Gn4
	.byte	W12
	.byte		        Gn4 , v044
	.byte	W12
	.byte		        En4 , v104
	.byte	W12
	.byte		        Cn5 , v120
	.byte	W12
	.byte		        Cn5 , v040
	.byte	W12
	.byte	W96
	.byte		VOICE , 23
	.byte		PAN   , c_v-40
	.byte		VOL   , 102*mus_qz_653_mvl/mxv
	.byte		N11   , Bn3 , v104
	.byte		N11   , Fs4
	.byte	W12
	.byte		        Fs3 , v088
	.byte		N11   , Bn3
	.byte	W12
	.byte		        Fs3
	.byte	W12
	.byte		        Cn4 , v104
	.byte		N11   , Gn4
	.byte	W12
	.byte		        Cn4 , v036
	.byte		N11   , Gn4
	.byte	W12
	.byte		        An3 , v088
	.byte		N11   , En4
	.byte	W12
	.byte		        En4 , v112
	.byte		N11   , Cn5
	.byte	W12
	.byte		        En4 , v036
	.byte		N11   , Cn5
	.byte	W12
	.byte		PAN   , c_v-12
	.byte		VOL   , 78*mus_qz_653_mvl/mxv
	.byte		        64*mus_qz_653_mvl/mxv
	.byte		N32   , En4 , v112 , gtp3
	.byte	W36
	.byte		N05   , Ds4 , v088
	.byte	W06
	.byte		        Dn4
	.byte	W06
	.byte		N11   , Cs4 , v112
	.byte	W12
	.byte		N05   , Cn4 , v088
	.byte	W06
	.byte		        Bn3
	.byte	W06
	.byte		N23   , As3
	.byte	W24
	.byte		VOL   , 62*mus_qz_653_mvl/mxv
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
	.byte	W72
	.byte		        73*mus_qz_653_mvl/mxv
	.byte		N03   , Dn3 , v100
	.byte	W04
	.byte		        En3
	.byte	W04
	.byte		        Gn3
	.byte	W04
	.byte		        An3
	.byte	W04
	.byte		        As3
	.byte	W04
	.byte		        Cn4
	.byte	W04
	.byte		PAN   , c_v-32
	.byte		VOL   , 91*mus_qz_653_mvl/mxv
	.byte		N07   , Dn4 , v088
	.byte	W08
	.byte		N11   , Dn4 , v036
	.byte	W40
	.byte		N07   , Dn4 , v088
	.byte	W08
	.byte		N11   , Dn4 , v036
	.byte	W40
	.byte	W24
	.byte		N23   , Dn4 , v088
	.byte	W24
	.byte		N11   , Dn4 , v036
	.byte	W24
	.byte		N03   , As3 , v088
	.byte		N07   , Dn4
	.byte	W08
	.byte		N03   , As3
	.byte		N03   , Dn4
	.byte	W08
	.byte		        As3 , v076
	.byte		N03   , Dn4
	.byte	W08
	.byte		N07   , Cn4 , v104
	.byte		N07   , En4
	.byte	W08
	.byte		N11   , Cn4 , v036
	.byte		N11   , En4
	.byte	W40
	.byte		N07   , Cn4 , v104
	.byte		N07   , En4
	.byte	W08
	.byte		N11   , Cn4 , v040
	.byte		N11   , En4
	.byte	W40
	.byte	W48
	.byte		N07   , Bn4 , v088
	.byte		N07   , En5
	.byte	W08
	.byte		        Fs4
	.byte		N07   , Bn4
	.byte	W08
	.byte		        Cs4
	.byte		N07   , Fs4
	.byte	W08
	.byte		        Gs3
	.byte		N07   , Cs4
	.byte	W08
	.byte		        Ds3
	.byte		N07   , Gs3
	.byte	W08
	.byte		        As2
	.byte		N07   , Ds3
	.byte	W08
	.byte	GOTO
	 ptrvalue	mus_qz_653_7_B1
	.byte	FINE

/********************** Track  8 **********************/

mus_qz_653_8:
	.byte		KEYSH , mus_qz_653_key+0
	.byte		VOL   , 82*mus_qz_653_mvl/mxv
	.byte		        102*mus_qz_653_mvl/mxv
	.byte		        68*mus_qz_653_mvl/mxv
	.byte		PRIO  , 64
	.byte		PAN   , c_v-48
	.byte	W24
	.byte		VOICE , 63
	.byte		N07   , Cs3 , v116
	.byte	W08
	.byte		        Gn2 , v100
	.byte	W08
	.byte	N07
	.byte	W08
	.byte		        Gn2 , v127
	.byte	W08
	.byte		        Gn2 , v072
	.byte	W08
	.byte		        Gn2 , v104
	.byte	W08
	.byte		VOL   , 78*mus_qz_653_mvl/mxv
	.byte		N05   , Gn2 , v127
	.byte	W06
	.byte		        Gn2 , v088
	.byte	W06
	.byte	N11
	.byte	W12
	.byte		N07   , Gn1 , v120
	.byte	W08
	.byte		        Gn1 , v068
	.byte	W08
	.byte		        Gn1 , v088
	.byte	W08
	.byte		N05   , Gn2 , v127
	.byte	W06
	.byte		        Gn2 , v088
	.byte	W06
	.byte	N11
	.byte	W12
	.byte		N15   , Gn1
	.byte	W16
	.byte	N07
	.byte	W08
	.byte		        Gn2
	.byte	W16
	.byte	N07
	.byte	W08
	.byte		N23   , Gn1
	.byte	W24
	.byte	W48
	.byte		N15   , Gs1 , v016
	.byte	W16
	.byte		        Gs1 , v028
	.byte	W16
	.byte		        Gs1 , v048
	.byte	W16
	.byte	N15
	.byte	W16
	.byte		N60   , Ds1 , v088 , gtp3
	.byte	W80
	.byte	W96
	.byte	W72
	.byte		N15   , Gs1 , v016
	.byte	W16
	.byte		        Gs1 , v028
	.byte	W08
	.byte	W08
	.byte		        Gs1 , v048
	.byte	W16
	.byte	N15
	.byte	W16
	.byte		N60   , Ds1 , v088 , gtp3
	.byte	W56
	.byte	W24
	.byte		N15   , Gs1 , v016
	.byte	W16
	.byte		        Gs1 , v028
	.byte	W16
	.byte		        Gs1 , v048
	.byte	W16
	.byte	N15
	.byte	W16
	.byte		        Gs1 , v044
	.byte	W08
	.byte	W08
	.byte		        Gs1 , v064
	.byte	W16
	.byte		        Gs1 , v080
	.byte	W16
	.byte		        Gs1 , v088
	.byte	W56
	.byte	W48
	.byte		N05   , Gs1 , v032
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Gs1 , v036
	.byte	W06
	.byte		        Gs1 , v044
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Gs1 , v048
	.byte	W06
	.byte		        Gs1 , v056
	.byte	W06
	.byte		        Gs1 , v076
	.byte	W06
	.byte		        Gs1 , v127
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		VOL   , 102*mus_qz_653_mvl/mxv
	.byte		N56   , Dn1
	.byte	W60
	.byte		VOL   , 78*mus_qz_653_mvl/mxv
	.byte		N20   , Dn1 , v096
	.byte	W24
	.byte		        Dn1 , v060
	.byte	W24
	.byte		        Dn1 , v100
	.byte	W24
	.byte		        Dn1 , v056
	.byte	W24
	.byte		        Dn1 , v104
	.byte	W24
	.byte		        Dn1 , v068
	.byte	W24
	.byte		        Dn1 , v100
	.byte	W24
	.byte		        Dn1 , v112
	.byte	W24
	.byte		VOL   , 97*mus_qz_653_mvl/mxv
	.byte		N20   , Dn1 , v127
	.byte	W24
	.byte		VOL   , 77*mus_qz_653_mvl/mxv
	.byte		N05   , Gs1 , v032
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Gs1 , v036
	.byte	W06
	.byte		        Gs1 , v044
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Gs1 , v048
	.byte	W06
	.byte		        Gs1 , v056
	.byte	W06
	.byte		        Gs1 , v076
	.byte	W06
	.byte		N42   , Gs1 , v127 , gtp1
	.byte	W48
	.byte		        Gs1 , v127 , gtp1
	.byte	W48
	.byte	W12
	.byte		N32   , Gs1 , v127 , gtp3
	.byte	W36
	.byte		VOL   , 93*mus_qz_653_mvl/mxv
	.byte		N07   , Fs1 , v112
	.byte	W08
	.byte		        En2 , v104
	.byte	W08
	.byte		        Gn1 , v116
	.byte	W08
	.byte		        Fs1 , v104
	.byte	W08
	.byte		        Fs2 , v116
	.byte	W08
	.byte		        Fs1 , v088
	.byte	W08
mus_qz_653_8_B1:
	.byte		VOL   , 82*mus_qz_653_mvl/mxv
	.byte		PAN   , c_v-48
	.byte		N11   , Bn1 , v127
	.byte	W12
	.byte		N23   , Bn1 , v088
	.byte	W12
	.byte		VOL   , 82*mus_qz_653_mvl/mxv
	.byte	W12
	.byte		N11   , Fs1
	.byte	W12
	.byte	N11
	.byte	W24
	.byte		N23   , Bn1
	.byte	W24
	.byte	W12
	.byte		        Fs1
	.byte	W36
	.byte		N11   , Bn1
	.byte	W12
	.byte		N32   , Fs1 , v088 , gtp3
	.byte	W36
	.byte		N23   , Cn2 , v127
	.byte	W36
	.byte		        Gn1 , v088
	.byte	W36
	.byte		        En1
	.byte	W24
	.byte		N11   , Dn2
	.byte	W12
	.byte		N23   , Gn1
	.byte	W36
	.byte		N11   , Cn2
	.byte	W12
	.byte		N32   , As1 , v088 , gtp3
	.byte	W36
	.byte		N11   , Bn1
	.byte	W12
	.byte		N23   , Fs1
	.byte	W24
	.byte		N11   , Bn1
	.byte	W24
	.byte	N11
	.byte	W12
	.byte		N20   , Ds2
	.byte	W24
	.byte	N11
	.byte	W12
	.byte		N23   , Fs1
	.byte	W36
	.byte		N11   , Bn1
	.byte	W12
	.byte		N32   , An1 , v088 , gtp3
	.byte	W36
	.byte		N23   , Cn2
	.byte	W36
	.byte		        Gn1
	.byte	W36
	.byte		N11   , Cn2
	.byte	W12
	.byte		N05   , Gn1
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		N11   , En2 , v127
	.byte	W12
	.byte		        Cn2 , v088
	.byte	W12
	.byte		        Gn2 , v127
	.byte	W12
	.byte		        Cn2 , v088
	.byte	W12
	.byte		        An1 , v127
	.byte	W12
	.byte		N32   , As1 , v127 , gtp3
	.byte	W36
	.byte		N11   , En2 , v088
	.byte	W12
	.byte		N23   , Bn1
	.byte	W24
	.byte		N11   , Fs1
	.byte	W12
	.byte	N11
	.byte	W24
	.byte		N23   , Bn1
	.byte	W24
	.byte	W12
	.byte		        Fs1
	.byte	W36
	.byte		        Bn1
	.byte	W24
	.byte		N20   , Dn2
	.byte	W24
	.byte		N23   , Cn2
	.byte	W36
	.byte		N32   , En2
	.byte	W36
	.byte		N23   , Gn1
	.byte	W24
	.byte		N11   , Dn2
	.byte	W12
	.byte		N23   , Cn2
	.byte	W36
	.byte	N11
	.byte	W12
	.byte		N32   , As1 , v088 , gtp3
	.byte	W36
	.byte		N11   , Fs2
	.byte	W12
	.byte		N23   , Bn1
	.byte	W24
	.byte		        Fs1
	.byte	W24
	.byte	N05
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		N11   , Bn1
	.byte	W12
	.byte		        Cn2
	.byte	W12
mus_qz_653_8_000:
	.byte		N11   , Fs2 , v088
	.byte	W12
	.byte		N23   , Bn1
	.byte	W24
	.byte		        Fs1
	.byte	W24
	.byte	N05
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		N11   , En2
	.byte	W12
	.byte	N05
	.byte	W06
	.byte	N05
	.byte	W06
	.byte	PEND
	.byte		N11   , Fs2
	.byte	W12
	.byte		N23   , Bn1
	.byte	W24
	.byte		        Fs1
	.byte	W24
	.byte	N05
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		N23   , Bn1
	.byte	W24
	.byte	PATT
	 ptrvalue	mus_qz_653_8_000
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
mus_qz_653_8_001:
	.byte		N44   , As1 , v088 , gtp3
	.byte	W60
	.byte		        Dn1 , v088 , gtp3
	.byte	W36
	.byte	PEND
	.byte	W12
	.byte		N23   , As1
	.byte	W24
	.byte		N32   , Dn1 , v088 , gtp3
	.byte	W60
	.byte		N44   , Bn1 , v088 , gtp3
	.byte	W60
	.byte		        En1 , v088 , gtp3
	.byte	W36
	.byte	W12
	.byte		N23   , Bn1
	.byte	W24
	.byte		        En1
	.byte	W24
	.byte	N11
	.byte	W12
	.byte		N23   , Bn1
	.byte	W24
	.byte	PATT
	 ptrvalue	mus_qz_653_8_001
	.byte	W12
	.byte		N23   , As1 , v088
	.byte	W24
	.byte		N32   , Dn1 , v088 , gtp3
	.byte	W36
	.byte		N05   , Fs1 , v056
	.byte	W06
	.byte		        Fs1 , v068
	.byte	W06
	.byte		        Fs1 , v080
	.byte	W06
	.byte		        Fs1 , v104
	.byte	W06
	.byte		N92   , As1 , v116 , gtp3
	.byte	W96
	.byte		N68   , Fn1 , v060 , gtp3
	.byte	W72
	.byte		N07   , En1 , v088
	.byte	W08
	.byte	N07
	.byte	W08
	.byte	N07
	.byte	W08
	.byte		N44   , Bn1 , v088 , gtp3
	.byte	W96
	.byte		N23   , Fn1
	.byte	W24
	.byte		N44   , Bn1 , v088 , gtp3
	.byte	W48
	.byte		N23   , Fn1
	.byte	W24
	.byte		N44   , Gs1 , v088 , gtp3
	.byte	W96
	.byte		N23   , Dn1
	.byte	W24
	.byte		N44   , Gs1 , v088 , gtp3
	.byte	W48
	.byte		N23   , Dn1
	.byte	W24
	.byte		N44   , Dn2 , v088 , gtp3
	.byte	W48
	.byte		        Gn1 , v088 , gtp3
	.byte	W48
	.byte		N23   , Dn2
	.byte	W24
	.byte		        Gn1
	.byte	W48
	.byte		N07   , Dn2 , v076
	.byte	W08
	.byte		        Dn2 , v088
	.byte	W08
	.byte		        Dn2 , v076
	.byte	W08
	.byte		N44   , Cn2 , v088 , gtp3
	.byte	W48
	.byte		        Fs1 , v088 , gtp3
	.byte	W48
	.byte	W12
	.byte		N32   , Gs1 , v127 , gtp3
	.byte	W36
	.byte		VOL   , 80*mus_qz_653_mvl/mxv
	.byte		N07   , Fs1 , v112
	.byte	W08
	.byte		        En2 , v104
	.byte	W08
	.byte		        Gn1 , v116
	.byte	W08
	.byte		        Fs1 , v104
	.byte	W08
	.byte		        Fs2 , v116
	.byte	W08
	.byte		        Fs1 , v088
	.byte	W08
	.byte	GOTO
	 ptrvalue	mus_qz_653_8_B1
	.byte	FINE

/********************** Track  9 **********************/

mus_qz_653_9:
	.byte		KEYSH , mus_qz_653_key+0
	.byte		VOL   , 82*mus_qz_653_mvl/mxv
	.byte		        102*mus_qz_653_mvl/mxv
	.byte		        68*mus_qz_653_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		BENDR , 6
	.byte		PRIO  , 64
	.byte	W60
	.byte		PAN   , c_v-52
	.byte	W12
	.byte		        c_v-50
	.byte	W48
	.byte	W48
	.byte		VOICE , 23
	.byte	W24
	.byte		VOL   , 5*mus_qz_653_mvl/mxv
	.byte	W05
	.byte		        8*mus_qz_653_mvl/mxv
	.byte	W07
	.byte		        11*mus_qz_653_mvl/mxv
	.byte		TIE   , As1 , v088
	.byte	W05
	.byte		VOL   , 12*mus_qz_653_mvl/mxv
	.byte	W07
	.byte		        15*mus_qz_653_mvl/mxv
	.byte	W05
	.byte		        16*mus_qz_653_mvl/mxv
	.byte	W07
	.byte		        18*mus_qz_653_mvl/mxv
	.byte	W05
	.byte		        21*mus_qz_653_mvl/mxv
	.byte	W07
	.byte		        24*mus_qz_653_mvl/mxv
	.byte	W05
	.byte		        27*mus_qz_653_mvl/mxv
	.byte	W07
	.byte		        30*mus_qz_653_mvl/mxv
	.byte	W05
	.byte		        34*mus_qz_653_mvl/mxv
	.byte	W12
	.byte		        36*mus_qz_653_mvl/mxv
	.byte	W42
	.byte	W01
	.byte	W11
	.byte	EOT
	.byte	W01
	.byte		N92   , An1 , v088 , gtp3
	.byte	W84
	.byte	W12
	.byte		        Cs2 , v088 , gtp3
	.byte	W84
	.byte	W12
	.byte		        Cn2 , v088 , gtp3
	.byte	W84
	.byte	W12
	.byte		N44   , En2 , v088 , gtp3
	.byte	W48
	.byte		        Ds2 , v088 , gtp3
	.byte	W36
	.byte	W12
	.byte		        Fs2 , v088 , gtp3
	.byte	W48
	.byte		        Fn2 , v088 , gtp3
	.byte	W36
	.byte	W12
	.byte		N92   , Gs2 , v088 , gtp3
	.byte	W12
	.byte		BEND  , c_v+24
	.byte	W12
	.byte		        c_v-12
	.byte	W60
	.byte	W12
	.byte		        c_v+0
	.byte		N32   , Fs2 , v088 , gtp3
	.byte	W36
	.byte		VOICE , 28
	.byte		PAN   , c_v+48
	.byte		VOL   , 80*mus_qz_653_mvl/mxv
	.byte		N05   , En2 , v032
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        En2 , v036
	.byte	W06
	.byte		        En2 , v044
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        En2 , v048
	.byte	W06
	.byte		        En2 , v056
	.byte	W06
	.byte		        En2 , v076
	.byte	W06
	.byte		        En2 , v127
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		N56   , Cs2
	.byte	W60
	.byte		N20   , Cs2 , v096
	.byte	W24
	.byte		        Cs2 , v060
	.byte	W24
	.byte		        Cs2 , v100
	.byte	W24
	.byte		        Cs2 , v056
	.byte	W24
	.byte		        Cs2 , v104
	.byte	W24
	.byte		        Cs2 , v068
	.byte	W24
	.byte		VOL   , 97*mus_qz_653_mvl/mxv
	.byte		N20   , Cs2 , v100
	.byte	W24
	.byte		        Cs2 , v112
	.byte	W24
	.byte		        Cs2 , v127
	.byte	W24
	.byte		N05   , Gs2 , v032
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Gs2 , v036
	.byte	W06
	.byte		        Gs2 , v044
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Gs2 , v048
	.byte	W06
	.byte		        Gs2 , v056
	.byte	W06
	.byte		        Gs2 , v076
	.byte	W06
	.byte		N11   , Gs2 , v127
	.byte	W48
	.byte		        Gs2 , v088
	.byte	W48
	.byte	W12
	.byte		N32   , Fs2 , v088 , gtp3
	.byte	W36
	.byte		VOL   , 82*mus_qz_653_mvl/mxv
	.byte		PAN   , c_v+55
	.byte		N07   , Fn2 , v112
	.byte	W08
	.byte		        Fs3 , v104
	.byte	W08
	.byte		        Cs3 , v116
	.byte	W08
	.byte		        Fn2 , v104
	.byte	W08
	.byte		        Fs3 , v116
	.byte	W08
	.byte		        Cs3 , v088
	.byte	W08
mus_qz_653_9_B1:
	.byte		VOL   , 80*mus_qz_653_mvl/mxv
	.byte		PAN   , c_v+46
	.byte		N32   , Bn2 , v088
	.byte	W36
	.byte		        Fn2
	.byte	W36
	.byte		        Bn2
	.byte	W24
	.byte	W12
	.byte		        Fn2 , v088 , gtp3
	.byte	W36
	.byte		N11   , Bn2
	.byte	W12
	.byte		N32   , Fn2
	.byte	W36
	.byte		        Cn3
	.byte	W36
	.byte		        Fs2
	.byte	W36
	.byte		        Dn2
	.byte	W24
	.byte	W12
	.byte		        Fs2
	.byte	W36
	.byte		N11   , Bn2
	.byte	W12
	.byte	N11
	.byte	W24
	.byte		N05   , Bn2 , v100
	.byte	W06
	.byte		        Bn2 , v088
	.byte	W06
	.byte		N32   , Bn2 , v100
	.byte	W36
	.byte		        Fn2 , v088
	.byte	W36
	.byte		        Bn2
	.byte	W24
	.byte	W12
	.byte		        Fn2
	.byte	W36
	.byte		N11   , Bn2
	.byte	W12
	.byte		N32   , Fn2
	.byte	W36
	.byte		        Cn3
	.byte	W36
	.byte		        Fs2
	.byte	W36
	.byte		N08   , Cn3
	.byte	W12
	.byte		N05   , Cn3 , v100
	.byte	W06
	.byte		        Cn3 , v096
	.byte	W06
	.byte		N20   , Cn3 , v120
	.byte	W24
	.byte		N11   , Cn3 , v088
	.byte	W12
	.byte		N08   , En2
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		N32   , Cn3 , v088 , gtp3
	.byte	W36
	.byte		N08   , Bn2 , v116
	.byte	W12
	.byte		N20   , Bn2 , v088
	.byte	W24
	.byte		N28   , Fn2 , v088 , gtp1
	.byte	W36
	.byte		N32   , Bn2
	.byte	W24
	.byte	W12
	.byte		        Fn2 , v088 , gtp3
	.byte	W36
	.byte		N23   , Bn2
	.byte	W24
	.byte		        Fn2
	.byte	W24
	.byte		N32   , Cn3
	.byte	W36
	.byte		        En3
	.byte	W36
	.byte		        Gn2 , v100
	.byte	W24
	.byte	W12
	.byte		        Cn3 , v088
	.byte	W36
	.byte		N11   , Bn2
	.byte	W12
	.byte	N11
	.byte	W24
	.byte	N05
	.byte	W06
	.byte		        Bn2 , v056
	.byte	W06
	.byte		N11   , Bn2 , v100
	.byte	W12
	.byte		N20   , Fn2 , v088
	.byte	W24
	.byte		N28   , Bn2 , v100 , gtp1
	.byte	W36
	.byte		N05   , Bn2 , v088
	.byte	W12
	.byte		        Bn2 , v056
	.byte	W12
	.byte		N11   , Bn2 , v100
	.byte	W12
	.byte		        Bn2 , v088
	.byte	W24
	.byte		N28   , Bn2 , v100 , gtp1
	.byte	W36
	.byte		N11   , Bn2 , v088
	.byte	W12
	.byte	N05
	.byte	W06
	.byte		        Bn2 , v048
	.byte	W06
	.byte		N11   , Bn2 , v100
	.byte	W12
	.byte		        Bn2 , v088
	.byte	W24
	.byte		N36   , Bn2 , v100 , gtp2
	.byte	W48
	.byte		N05   , Bn2 , v088
	.byte	W06
	.byte		        Bn2 , v056
	.byte	W06
	.byte		N11   , Bn2 , v100
	.byte	W12
	.byte		        Bn2 , v088
	.byte	W24
	.byte		N52   , Bn2 , v100 , gtp1
	.byte	W60
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		PAN   , c_v+21
	.byte		N44   , Bn2 , v088 , gtp3
	.byte	W60
	.byte		        Dn2
	.byte	W36
	.byte	W12
	.byte		N23   , Bn2
	.byte	W24
	.byte		        Dn2
	.byte	W24
	.byte	N11
	.byte	W12
	.byte		N23   , Bn2
	.byte	W24
	.byte	N56
	.byte	W60
	.byte		N44   , En2
	.byte	W36
	.byte	W12
	.byte		N23   , Bn2
	.byte	W24
	.byte		N08   , En2
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		        Bn2
	.byte	W12
	.byte	N23
	.byte	W24
	.byte		N56   , Bn2 , v088 , gtp3
	.byte	W60
	.byte		N44   , En2 , v088 , gtp3
	.byte	W36
	.byte	W12
	.byte		N23   , Bn2
	.byte	W24
	.byte		N32   , En2 , v088 , gtp3
	.byte	W36
	.byte		N05   , En2 , v056
	.byte	W06
	.byte		        En2 , v068
	.byte	W06
	.byte		        En2 , v080
	.byte	W06
	.byte		        En2 , v104
	.byte	W06
	.byte		N92   , As2 , v116 , gtp3
	.byte	W96
	.byte		N68   , En2 , v104 , gtp3
	.byte	W72
	.byte		N07   , En2 , v088
	.byte	W08
	.byte	N07
	.byte	W08
	.byte	N07
	.byte	W08
	.byte		N44   , Bn2 , v088 , gtp3
	.byte	W96
	.byte	W24
	.byte		PAN   , c_v+48
	.byte	W72
	.byte	W96
	.byte	W96
	.byte		N44   , Dn3 , v088 , gtp3
	.byte	W48
	.byte		        Gn2 , v088 , gtp3
	.byte	W48
	.byte		PAN   , c_v+24
	.byte		VOL   , 61*mus_qz_653_mvl/mxv
	.byte		        95*mus_qz_653_mvl/mxv
	.byte		N23   , Dn3
	.byte	W24
	.byte		N44   , Fs2 , v088 , gtp3
	.byte	W48
	.byte		N07   , Dn3 , v076
	.byte	W08
	.byte		        Dn3 , v088
	.byte	W08
	.byte		        Dn3 , v076
	.byte	W08
	.byte		N44   , Cn3 , v088 , gtp3
	.byte	W48
	.byte		        Gs2 , v088 , gtp3
	.byte	W48
	.byte	W48
	.byte		VOL   , 64*mus_qz_653_mvl/mxv
	.byte		PAN   , c_v+42
	.byte		N07   , Fn2 , v112
	.byte	W08
	.byte		        Fs3 , v104
	.byte	W08
	.byte		        Cs3 , v116
	.byte	W08
	.byte		        Fn2 , v104
	.byte	W08
	.byte		        Fs3 , v116
	.byte	W08
	.byte		        Cs3 , v088
	.byte	W08
	.byte	GOTO
	 ptrvalue	mus_qz_653_9_B1
	.byte	FINE

/********************** Track 10 **********************/

mus_qz_653_10:
	.byte		KEYSH , mus_qz_653_key+0
	.byte		VOICE , 39
	.byte		VOL   , 82*mus_qz_653_mvl/mxv
	.byte		        65*mus_qz_653_mvl/mxv
	.byte		        82*mus_qz_653_mvl/mxv
	.byte		        68*mus_qz_653_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte		N68   , Gn0 , v088 , gtp3
	.byte	W72
	.byte		VOL   , 91*mus_qz_653_mvl/mxv
	.byte		        81*mus_qz_653_mvl/mxv
	.byte		N05   , Dn1 , v104
	.byte		N23   , An2 , v088
	.byte	W06
	.byte		N05   , Dn1 , v028
	.byte	W06
	.byte		        Dn1 , v080
	.byte	W12
	.byte		N11   , En2 , v088
	.byte	W24
	.byte		N05   , Dn1 , v104
	.byte		N23   , An2 , v088
	.byte	W06
	.byte		N05   , Dn1 , v028
	.byte	W06
	.byte		        Dn1 , v080
	.byte	W12
	.byte		N11   , En2 , v088
	.byte	W24
	.byte	N11
	.byte	W24
	.byte		N64   , Cs2 , v088 , gtp1
	.byte	W24
	.byte	W96
	.byte	W96
	.byte	W48
	.byte		N21   , Dn1 , v052
	.byte	W24
	.byte		N13   , Dn1 , v032
	.byte	W24
	.byte	N11
	.byte	W16
	.byte		        Dn1 , v048
	.byte	W16
	.byte		        Dn1 , v052
	.byte	W16
	.byte		N01   , Dn1 , v080
	.byte	W04
	.byte		N03   , Dn1 , v048
	.byte	W04
	.byte		        Dn1 , v032
	.byte	W04
	.byte	N03
	.byte	W04
	.byte	N03
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		N23   , Dn1 , v088
	.byte	W24
	.byte	W96
	.byte	W48
	.byte		N13   , Dn1 , v032
	.byte	W24
	.byte	N13
	.byte	W24
	.byte	N11
	.byte	W16
	.byte		        Dn1 , v048
	.byte	W16
	.byte		        Dn1 , v052
	.byte	W16
	.byte		N01   , Dn1 , v080
	.byte	W04
	.byte		N03   , Dn1 , v048
	.byte	W04
	.byte		        Dn1 , v032
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		        Dn1 , v040
	.byte	W04
	.byte		        Dn1 , v036
	.byte	W04
	.byte		N01   , Dn1 , v040
	.byte	W04
	.byte	N03
	.byte	W04
	.byte		        Dn1 , v048
	.byte	W04
	.byte		        Dn1 , v052
	.byte	W04
	.byte		        Dn1 , v060
	.byte	W04
	.byte		        Dn1 , v064
	.byte	W04
	.byte	N11
	.byte	W16
	.byte		        Dn1 , v076
	.byte	W16
	.byte		        Dn1 , v092
	.byte	W64
	.byte	W12
	.byte		N28   , Ds2 , v088 , gtp1
	.byte	W36
	.byte		N40   , An2 , v088 , gtp1
	.byte	W48
	.byte	W96
	.byte	W96
	.byte	W48
	.byte		N11   , Ds2
	.byte	W48
	.byte		N14   , As2 , v072
	.byte	W48
	.byte	W12
	.byte		N32   , Cs2 , v088 , gtp2
	.byte	W21
	.byte		N60   , Gn0 , v088 , gtp2
	.byte	W15
	.byte		VOL   , 89*mus_qz_653_mvl/mxv
	.byte	W48
mus_qz_653_10_B1:
	.byte		VOL   , 102*mus_qz_653_mvl/mxv
	.byte		N32   , Cs2 , v088 , gtp3
	.byte	W12
	.byte		N05   , Dn1 , v056
	.byte	W06
	.byte		        Dn1 , v044
	.byte	W06
	.byte		N11   , Dn1 , v088
	.byte	W12
	.byte		N44   , Cn2 , v116 , gtp3
	.byte	W12
	.byte		N05   , Dn1 , v056
	.byte	W06
	.byte		        Dn1 , v044
	.byte	W06
	.byte		N11   , Dn1 , v088
	.byte	W24
	.byte		N02   , Dn1 , v056
	.byte	W03
	.byte		        Dn1 , v032
	.byte	W03
	.byte		        Dn1 , v048
	.byte	W03
	.byte		        Dn1 , v032
	.byte	W03
	.byte		N11   , Dn1 , v100
	.byte	W24
	.byte		N05   , Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v044
	.byte	W06
	.byte		        Dn1 , v056
	.byte	W06
	.byte		        Dn1 , v044
	.byte	W06
	.byte		N02   , Dn1 , v104
	.byte	W03
	.byte		        Dn1 , v048
	.byte	W03
	.byte		        Dn1 , v064
	.byte	W03
	.byte		        Dn1 , v048
	.byte	W03
	.byte		        Dn1 , v064
	.byte	W03
	.byte		        Dn1 , v048
	.byte	W03
	.byte		        Dn1 , v064
	.byte	W03
	.byte		        Dn1 , v048
	.byte	W03
	.byte		N11   , Dn1 , v112
	.byte	W12
	.byte		N05   , Dn1 , v092
	.byte	W06
	.byte		        Dn1 , v116
	.byte	W06
	.byte		N32   , Cs2 , v088 , gtp3
	.byte	W12
	.byte		N05   , Dn1 , v056
	.byte	W06
	.byte		        Dn1 , v044
	.byte	W06
	.byte		N11   , Dn1 , v088
	.byte	W12
	.byte		N56   , Dn2 , v088 , gtp3
	.byte	W12
	.byte		N05   , Dn1 , v056
	.byte	W06
	.byte		        Dn1 , v044
	.byte	W06
	.byte		N11   , Dn1 , v088
	.byte	W24
	.byte	N02
	.byte	W03
	.byte		        Dn1 , v048
	.byte	W03
	.byte		        Dn1 , v060
	.byte	W03
	.byte		        Dn1 , v048
	.byte	W03
	.byte		N11   , Dn1 , v088
	.byte	W24
	.byte	N05
	.byte	W06
	.byte		        Dn1 , v044
	.byte	W06
	.byte		        Dn1 , v056
	.byte	W06
	.byte		        Dn1 , v044
	.byte	W06
	.byte		N02   , Dn1 , v068
	.byte	W03
	.byte		        Dn1 , v032
	.byte	W03
	.byte		        Dn1 , v048
	.byte	W03
	.byte		        Dn1 , v036
	.byte	W03
	.byte		        Dn1 , v056
	.byte	W03
	.byte		        Dn1 , v028
	.byte	W03
	.byte		        Dn1 , v044
	.byte	W03
	.byte		        Dn1 , v028
	.byte	W03
	.byte		N11   , Dn1 , v088
	.byte	W12
	.byte		N05   , Dn1 , v072
	.byte	W06
	.byte		        Dn1 , v056
	.byte	W06
	.byte	W12
	.byte	N05
	.byte	W06
	.byte		        Dn1 , v044
	.byte	W06
	.byte		N11   , Dn1 , v088
	.byte	W24
	.byte		        Dn1 , v100
	.byte	W12
	.byte	N05
	.byte	W06
	.byte	N05
	.byte	W18
	.byte		N02   , Dn1 , v056
	.byte	W03
	.byte		        Dn1 , v032
	.byte	W03
	.byte		        Dn1 , v048
	.byte	W03
	.byte		        Dn1 , v032
	.byte	W03
	.byte		N11   , Dn1 , v088
	.byte	W36
	.byte		N05   , Dn1 , v096
	.byte	W06
	.byte		        Dn1 , v088
	.byte	W18
	.byte	N02
	.byte	W03
	.byte		        Dn1 , v032
	.byte	W03
	.byte		        Dn1 , v060
	.byte	W03
	.byte		        Dn1 , v036
	.byte	W03
	.byte		N11   , Dn1 , v100
	.byte	W12
	.byte		N05   , Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v104
	.byte	W06
	.byte	W12
	.byte		        Dn1 , v056
	.byte	W06
	.byte		        Dn1 , v044
	.byte	W06
	.byte		N11   , Dn1 , v088
	.byte	W24
	.byte		N05   , Dn1 , v056
	.byte	W06
	.byte		        Dn1 , v044
	.byte	W06
	.byte		N11   , Dn1 , v088
	.byte	W36
	.byte		        Dn1 , v092
	.byte	W12
	.byte		        Dn1 , v112
	.byte	W24
	.byte		N05   , Dn1 , v056
	.byte	W06
	.byte		        Dn1 , v044
	.byte	W06
	.byte		N02   , Dn1 , v088
	.byte	W03
	.byte		        Dn1 , v052
	.byte	W03
	.byte		        Dn1 , v068
	.byte	W03
	.byte		        Dn1 , v044
	.byte	W03
	.byte		        Dn1 , v068
	.byte	W03
	.byte		        Dn1 , v044
	.byte	W03
	.byte		        Dn1 , v068
	.byte	W03
	.byte		        Dn1 , v028
	.byte	W03
	.byte		N11   , Dn1 , v120
	.byte	W12
	.byte		N05   , Dn1 , v092
	.byte	W06
	.byte		        Dn1 , v104
	.byte	W06
	.byte		N32   , Cs2 , v088 , gtp3
	.byte	W12
	.byte		N05   , Dn1 , v056
	.byte	W06
	.byte		        Dn1 , v044
	.byte	W06
	.byte		N11   , Dn1 , v088
	.byte	W12
	.byte		N56   , Cn2 , v088 , gtp3
	.byte	W12
	.byte		N05   , Dn1 , v056
	.byte	W06
	.byte		        Dn1 , v044
	.byte	W06
	.byte		N11   , Dn1 , v088
	.byte	W24
	.byte		N02   , Dn1 , v056
	.byte	W03
	.byte		        Dn1 , v032
	.byte	W03
	.byte		        Dn1 , v048
	.byte	W03
	.byte		        Dn1 , v032
	.byte	W03
	.byte		N11   , Dn1 , v100
	.byte	W24
	.byte		N05   , Dn1 , v088
	.byte	W06
	.byte		        Dn1 , v044
	.byte	W06
	.byte		        Dn1 , v056
	.byte	W06
	.byte		        Dn1 , v044
	.byte	W06
	.byte		N02   , Dn1 , v100
	.byte	W03
	.byte		        Dn1 , v064
	.byte	W03
	.byte		        Dn1 , v072
	.byte	W03
	.byte		        Dn1 , v052
	.byte	W03
	.byte		        Dn1 , v068
	.byte	W03
	.byte		        Dn1 , v048
	.byte	W03
	.byte		        Dn1 , v068
	.byte	W03
	.byte		        Dn1 , v044
	.byte	W03
	.byte		N11   , Dn1 , v112
	.byte	W12
	.byte		N05   , Dn1 , v080
	.byte	W06
	.byte		        Dn1 , v127
	.byte	W06
	.byte		N32   , Cs2 , v088 , gtp3
	.byte	W12
	.byte		N05   , Dn1 , v056
	.byte	W06
	.byte		        Dn1 , v044
	.byte	W06
	.byte		N11   , Dn1 , v088
	.byte	W12
	.byte		N56   , Dn2 , v088 , gtp3
	.byte	W12
	.byte		N05   , Dn1 , v056
	.byte	W06
	.byte		        Dn1 , v044
	.byte	W06
	.byte		N11   , Dn1 , v088
	.byte	W24
	.byte		N02   , Dn1 , v080
	.byte	W03
	.byte		        Dn1 , v044
	.byte	W03
	.byte		        Dn1 , v068
	.byte	W03
	.byte		        Dn1 , v044
	.byte	W03
	.byte		N11   , Dn1 , v088
	.byte	W24
	.byte	N05
	.byte	W06
	.byte		        Dn1 , v044
	.byte	W06
	.byte		        Dn1 , v056
	.byte	W06
	.byte		        Dn1 , v044
	.byte	W06
	.byte		N02   , Dn1 , v068
	.byte	W03
	.byte		        Dn1 , v032
	.byte	W03
	.byte		        Dn1 , v048
	.byte	W03
	.byte		        Dn1 , v036
	.byte	W03
	.byte		        Dn1 , v056
	.byte	W03
	.byte		        Dn1 , v028
	.byte	W03
	.byte		        Dn1 , v044
	.byte	W03
	.byte		        Dn1 , v028
	.byte	W03
	.byte		N11   , Dn1 , v088
	.byte	W12
	.byte		N44   , Gn0 , v127 , gtp3
	.byte	W12
	.byte	W36
	.byte		N32   , Dn2 , v100 , gtp3
	.byte	W36
	.byte		N11   , An2 , v088
	.byte	W12
	.byte		N44   , Gn0 , v127 , gtp3
	.byte	W12
	.byte	W36
	.byte		N32   , An2 , v100 , gtp3
	.byte	W36
	.byte		N44   , As2 , v084 , gtp3
	.byte	W24
	.byte	W36
	.byte		N32   , Cn2 , v100 , gtp3
	.byte	W36
	.byte		N23   , As2 , v080
	.byte	W12
	.byte		N44   , Gn0 , v127 , gtp3
	.byte	W12
	.byte	W36
	.byte		N32   , Cs2 , v100 , gtp3
	.byte	W12
	.byte		N44   , Gn0 , v124 , gtp3
	.byte	W48
	.byte		        Ds2 , v064 , gtp3
	.byte	W24
	.byte		N02   , Dn1 , v056
	.byte	W03
	.byte		        Dn1 , v032
	.byte	W03
	.byte		        Dn1 , v044
	.byte	W03
	.byte		        Dn1 , v028
	.byte	W03
	.byte		        Dn1 , v036
	.byte	W03
	.byte		        Dn1 , v044
	.byte	W03
	.byte		        Dn1 , v020
	.byte	W03
	.byte		        Dn1 , v040
	.byte	W03
	.byte		        Dn1 , v036
	.byte	W03
	.byte		N05   , Dn1 , v076
	.byte	W06
	.byte		N02   , Dn1 , v036
	.byte	W03
	.byte		N05   , Dn1 , v080
	.byte	W12
	.byte	N05
	.byte	W21
	.byte		N02   , Dn1 , v044
	.byte	W03
	.byte		N11   , Dn1 , v088
	.byte	W32
	.byte	W01
	.byte		N24   , Fn2 , v028 , gtp2
	.byte	W24
	.byte	W03
	.byte		N11   , Ds2 , v064
	.byte	W12
	.byte		N44   , As2 , v040 , gtp3
	.byte	W24
	.byte	W24
	.byte		N23   , Ds2 , v032
	.byte	W36
	.byte		        Ds2 , v036
	.byte	W36
	.byte		N05   , Dn1 , v056
	.byte	W06
	.byte		        Dn1 , v032
	.byte	W06
	.byte		        Dn1 , v048
	.byte	W06
	.byte		        Dn1 , v028
	.byte	W06
	.byte		N02   , Dn1 , v068
	.byte	W03
	.byte		        Dn1 , v024
	.byte	W03
	.byte		        Dn1 , v036
	.byte	W03
	.byte		        Dn1 , v020
	.byte	W03
	.byte		        Dn1 , v044
	.byte	W03
	.byte		        Dn1 , v016
	.byte	W03
	.byte		        Dn1 , v044
	.byte	W03
	.byte		        Dn1 , v020
	.byte	W03
	.byte		N05   , Dn1 , v048
	.byte	W12
	.byte	N11
	.byte		N11   , Ds2 , v008
	.byte	W12
	.byte		        Ds2 , v012
	.byte	W12
	.byte		        Ds2 , v020
	.byte	W12
	.byte		        Ds2 , v044
	.byte	W12
	.byte		N32   , Dn2 , v056 , gtp3
	.byte	W84
	.byte	W96
	.byte	W72
	.byte		N02   , Dn1 , v088
	.byte	W03
	.byte		        Dn1 , v028
	.byte	W03
	.byte		        Dn1 , v044
	.byte	W03
	.byte		        Dn1 , v028
	.byte	W03
	.byte		        Dn1 , v044
	.byte	W03
	.byte		        Dn1 , v020
	.byte	W03
	.byte		        Dn1 , v044
	.byte	W03
	.byte		        Dn1 , v020
	.byte	W03
	.byte		N05   , Dn1 , v068
	.byte	W12
	.byte		        Dn1 , v028
	.byte	W12
	.byte		        Dn1 , v032
	.byte	W09
	.byte		N02   , Dn1 , v020
	.byte	W03
	.byte		N05   , Dn1 , v112
	.byte	W12
	.byte		        Dn1 , v028
	.byte	W12
	.byte		        Dn1 , v100
	.byte	W12
	.byte		        Dn1 , v088
	.byte	W06
	.byte		N02   , Dn1 , v036
	.byte	W03
	.byte		        Dn1 , v056
	.byte	W03
	.byte		        Dn1 , v032
	.byte	W03
	.byte		        Dn1 , v056
	.byte	W03
	.byte		        Dn1 , v032
	.byte	W03
	.byte		        Dn1 , v056
	.byte	W03
	.byte		N11   , Dn1 , v088
	.byte	W24
	.byte	N23
	.byte	W24
	.byte	N02
	.byte	W03
	.byte		        Dn1 , v028
	.byte	W03
	.byte		        Dn1 , v044
	.byte	W03
	.byte		        Dn1 , v028
	.byte	W03
	.byte		        Dn1 , v044
	.byte	W03
	.byte		        Dn1 , v020
	.byte	W03
	.byte		        Dn1 , v044
	.byte	W03
	.byte		        Dn1 , v020
	.byte	W15
	.byte		N05   , Dn1 , v088
	.byte	W12
	.byte	W12
	.byte	N07
	.byte	W12
	.byte	N11
	.byte	W24
	.byte	N02
	.byte	W03
	.byte		        Dn1 , v028
	.byte	W03
	.byte		        Dn1 , v044
	.byte	W03
	.byte		        Dn1 , v028
	.byte	W03
	.byte		        Dn1 , v044
	.byte	W03
	.byte		        Dn1 , v020
	.byte	W03
	.byte		        Dn1 , v044
	.byte	W03
	.byte		        Dn1 , v020
	.byte	W03
	.byte		N05   , Dn1 , v088
	.byte	W12
	.byte	N11
	.byte	W12
	.byte	N07
	.byte	W24
	.byte	N11
	.byte	W24
	.byte	N02
	.byte	W03
	.byte		        Dn1 , v028
	.byte	W03
	.byte		        Dn1 , v068
	.byte	W03
	.byte		        Dn1 , v028
	.byte	W03
	.byte		        Dn1 , v060
	.byte	W03
	.byte		        Dn1 , v020
	.byte	W03
	.byte		        Dn1 , v064
	.byte	W03
	.byte		        Dn1 , v020
	.byte	W03
	.byte		N07   , Dn1 , v088
	.byte	W12
	.byte	N11
	.byte	W12
	.byte	W24
	.byte	N11
	.byte	W12
	.byte	N02
	.byte	W03
	.byte		        Dn1 , v028
	.byte	W03
	.byte		        Dn1 , v044
	.byte	W03
	.byte		        Dn1 , v028
	.byte	W03
	.byte		        Dn1 , v044
	.byte	W03
	.byte		        Dn1 , v028
	.byte	W03
	.byte		        Dn1 , v044
	.byte	W03
	.byte		        Dn1 , v028
	.byte	W03
	.byte		N11   , Dn1 , v088
	.byte	W12
	.byte	N05
	.byte	W12
	.byte	N11
	.byte	W12
	.byte	N11
	.byte	W24
	.byte	N05
	.byte	W24
	.byte	N02
	.byte	W03
	.byte		        Dn1 , v028
	.byte	W03
	.byte		        Dn1 , v044
	.byte	W03
	.byte		        Dn1 , v028
	.byte	W03
	.byte		        Dn1 , v044
	.byte	W03
	.byte		        Dn1 , v020
	.byte	W03
	.byte		        Dn1 , v044
	.byte	W03
	.byte		        Dn1 , v020
	.byte	W15
	.byte		N05   , Dn1 , v088
	.byte	W12
	.byte	W24
	.byte	N05
	.byte	W24
	.byte	N02
	.byte	W03
	.byte		        Dn1 , v028
	.byte	W03
	.byte		        Dn1 , v044
	.byte	W03
	.byte		        Dn1 , v028
	.byte	W03
	.byte		        Dn1 , v044
	.byte	W03
	.byte		        Dn1 , v020
	.byte	W03
	.byte		        Dn1 , v044
	.byte	W03
	.byte		        Dn1 , v020
	.byte	W03
	.byte		        Dn1 , v088
	.byte	W03
	.byte		        Dn1 , v028
	.byte	W03
	.byte		        Dn1 , v044
	.byte	W03
	.byte		        Dn1 , v028
	.byte	W03
	.byte		        Dn1 , v044
	.byte	W03
	.byte		        Dn1 , v020
	.byte	W03
	.byte		        Dn1 , v044
	.byte	W03
	.byte		        Dn1 , v020
	.byte	W03
	.byte		N23   , Dn1 , v088
	.byte		N44   , As2 , v088 , gtp3
	.byte	W48
	.byte		        En2 , v088 , gtp3
	.byte	W48
	.byte		N23   , Dn1
	.byte	W24
	.byte	N07
	.byte	W08
	.byte		        Dn1 , v072
	.byte	W08
	.byte		        Dn1 , v088
	.byte	W08
	.byte		        Dn1 , v072
	.byte		N44   , An2 , v088 , gtp3
	.byte	W08
	.byte		N07   , Dn1
	.byte	W08
	.byte		        Dn1 , v072
	.byte	W08
	.byte		N02   , Dn1 , v088
	.byte	W03
	.byte		        Dn1 , v052
	.byte	W03
	.byte		        Dn1 , v048
	.byte	W03
	.byte	N02
	.byte	W03
	.byte	N02
	.byte	W03
	.byte		        Dn1 , v052
	.byte	W03
	.byte	N02
	.byte	W03
	.byte		        Dn1 , v048
	.byte	W03
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		N44   , Dn2 , v088 , gtp3
	.byte	W48
	.byte		        As2 , v088 , gtp3
	.byte	W48
	.byte	W96
	.byte	W96
	.byte	W12
	.byte		N32   , Cs2 , v088 , gtp2
	.byte	W84
	.byte	GOTO
	 ptrvalue	mus_qz_653_10_B1
	.byte	FINE

/********************** Track 11 **********************/

mus_qz_653_11:
	.byte		KEYSH , mus_qz_653_key+0
	.byte		VOL   , 82*mus_qz_653_mvl/mxv
	.byte		        55*mus_qz_653_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		BENDR , 6
	.byte		PRIO  , 60
	.byte	W72
	.byte		VOICE , 47
	.byte	W48
	.byte	W48
	.byte	W48
	.byte	W03
	.byte		N20   , Bn1 , v088
	.byte	W21
	.byte		N54   , Gn1 , v088 , gtp1
	.byte	W56
	.byte		N60   , Ds1 , v088 , gtp3
	.byte	W16
	.byte	W48
	.byte		N68   , Dn1 , v088 , gtp3
	.byte	W48
	.byte	W24
	.byte		TIE   , Bn0
	.byte	W72
	.byte	W36
	.byte	W03
	.byte	EOT
	.byte	W56
	.byte	W01
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W48
	.byte		VOICE , 28
	.byte		PAN   , c_v+45
	.byte		VOL   , 73*mus_qz_653_mvl/mxv
	.byte		N05   , Gs2 , v032
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Gs2 , v036
	.byte	W06
	.byte		        Gs2 , v044
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Gs2 , v048
	.byte	W06
	.byte		        Gs2 , v056
	.byte	W06
	.byte		        Gs2 , v076
	.byte	W06
	.byte		        Gs2 , v127
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		N56   , Dn2
	.byte	W60
	.byte		N20   , Dn2 , v096
	.byte	W24
	.byte		        Dn2 , v060
	.byte	W24
	.byte		        Dn2 , v100
	.byte	W24
	.byte		        Dn2 , v056
	.byte	W24
	.byte		        Dn2 , v104
	.byte	W24
	.byte		        Dn2 , v068
	.byte	W24
	.byte		        Dn2 , v100
	.byte	W24
	.byte		        Dn2 , v112
	.byte	W24
	.byte		        Dn2 , v127
	.byte	W24
	.byte		N05   , Gs2 , v032
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Gs2 , v036
	.byte	W06
	.byte		        Gs2 , v044
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Gs2 , v048
	.byte	W06
	.byte		        Gs2 , v056
	.byte	W06
	.byte		        Gs2 , v076
	.byte	W06
	.byte		N42   , Gs2 , v127 , gtp1
	.byte	W48
	.byte		        Gs2 , v127 , gtp1
	.byte	W48
	.byte	W12
	.byte		N32   , Gs2 , v127 , gtp3
	.byte	W36
	.byte		N07   , Fs2 , v112
	.byte	W08
	.byte		        En3 , v104
	.byte	W08
	.byte		        Gn2 , v116
	.byte	W08
	.byte		        Fs2 , v104
	.byte	W08
	.byte		        Fs3 , v116
	.byte	W08
	.byte		        Fs2 , v088
	.byte	W08
mus_qz_653_11_B1:
	.byte		VOICE , 29
	.byte		VOL   , 44*mus_qz_653_mvl/mxv
	.byte		PAN   , c_v+59
	.byte		BEND  , c_v+1
	.byte	W12
	.byte		N05   , Bn2 , v088
	.byte	W06
	.byte		        As2
	.byte	W06
	.byte		N11   , Bn2
	.byte	W12
	.byte		N02   , An2
	.byte	W03
	.byte		        Gn2
	.byte	W03
	.byte		TIE   , Fs2
	.byte	W54
	.byte	W56
	.byte	W03
	.byte	EOT
	.byte	W13
	.byte		N11   , Bn2
	.byte	W12
	.byte		N17   , As2
	.byte	W12
	.byte	W06
	.byte		N05   , Fs3
	.byte	W06
	.byte		TIE   , Gn3
	.byte	W84
	.byte	W23
	.byte	EOT
	.byte	W01
	.byte		N32   , An3 , v088 , gtp3
	.byte	W36
	.byte		N11   , Gn3
	.byte	W12
	.byte		N32   , Fn3 , v088 , gtp3
	.byte	W24
mus_qz_653_11_000:
	.byte	W12
	.byte		TIE   , Fs3 , v088
	.byte	W84
	.byte	PEND
	.byte	W96
	.byte	W11
	.byte	EOT
	.byte	W01
	.byte		VOICE , 34
	.byte	W12
	.byte		N11   , Gn2
	.byte	W12
	.byte		        Cn3
	.byte	W12
	.byte		        En3
	.byte	W12
	.byte		        Fs3
	.byte	W12
	.byte		        Gn3
	.byte	W12
	.byte		        En3
	.byte	W12
	.byte		        Cn3
	.byte	W12
	.byte		VOICE , 29
	.byte		N11   , Fs3
	.byte	W12
	.byte		N05   , Gn3
	.byte	W12
	.byte	N02
	.byte	W12
	.byte	N05
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		N20   , An3
	.byte	W24
	.byte	W12
	.byte		VOICE , 23
	.byte		PAN   , c_v+50
	.byte		        c_v+42
	.byte		N05   , Ds4
	.byte	W06
	.byte		N02   , Dn4
	.byte	W06
	.byte		N11   , Ds4
	.byte	W12
	.byte		N02   , Dn4
	.byte	W03
	.byte		        Cn4
	.byte	W03
	.byte		TIE   , Bn3
	.byte	W54
	.byte	W56
	.byte	W03
	.byte	EOT
	.byte	W13
	.byte		N11   , Fs3
	.byte	W12
	.byte		N17   , Bn3
	.byte	W12
	.byte	W06
	.byte		N05   , Dn4
	.byte	W06
	.byte		N92   , En4 , v088 , gtp3
	.byte	W84
	.byte	W12
	.byte		N32   , Gn4
	.byte	W32
	.byte	W01
	.byte		N36   , Fs4 , v088 , gtp2
	.byte	W36
	.byte	W03
	.byte		N23   , Fn4
	.byte	W12
	.byte	W12
	.byte		VOL   , 28*mus_qz_653_mvl/mxv
	.byte		TIE   , Fs4
	.byte	W17
	.byte		VOL   , 29*mus_qz_653_mvl/mxv
	.byte	W18
	.byte		        31*mus_qz_653_mvl/mxv
	.byte	W06
	.byte		        34*mus_qz_653_mvl/mxv
	.byte	W36
	.byte		        36*mus_qz_653_mvl/mxv
	.byte	W07
	.byte	W11
	.byte		        37*mus_qz_653_mvl/mxv
	.byte	W06
	.byte		        40*mus_qz_653_mvl/mxv
	.byte	W42
	.byte		        37*mus_qz_653_mvl/mxv
	.byte	W12
	.byte		        36*mus_qz_653_mvl/mxv
	.byte	W06
	.byte		        34*mus_qz_653_mvl/mxv
	.byte	W06
	.byte		        25*mus_qz_653_mvl/mxv
	.byte	W06
	.byte		        20*mus_qz_653_mvl/mxv
	.byte	W04
	.byte	EOT
	.byte	W03
	.byte		VOICE , 33
	.byte		VOL   , 36*mus_qz_653_mvl/mxv
	.byte		PAN   , c_v-25
	.byte		N01   , Fn5 , v112
	.byte	W02
	.byte		N09   , Fs5 , v116
	.byte	W10
	.byte		N11   , Bn4 , v088
	.byte	W12
	.byte		        Fs4 , v104
	.byte	W12
	.byte		        Gn5
	.byte	W24
	.byte		        En5 , v088
	.byte	W12
	.byte		        Cn6 , v112
	.byte	W24
	.byte	W84
	.byte		VOL   , 46*mus_qz_653_mvl/mxv
	.byte		PAN   , c_v-24
	.byte		VOL   , 29*mus_qz_653_mvl/mxv
	.byte		N05   , En3 , v088
	.byte	W06
	.byte		        Fn3
	.byte	W06
	.byte		        Fs3
	.byte	W06
	.byte		        Gn3
	.byte	W06
	.byte		TIE   , An3
	.byte	W84
	.byte	W44
	.byte	W03
	.byte	EOT
	.byte	W01
	.byte		N23   , Gn3
	.byte	W24
	.byte		N11   , Ds3
	.byte	W12
	.byte		N23   , En3
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_653_11_000
	.byte	W32
	.byte	W03
	.byte		EOT   , Fs3
	.byte	W01
	.byte		N23   , Gn3 , v088
	.byte	W24
	.byte		        Bn2
	.byte	W24
	.byte		        Cn3
	.byte	W12
	.byte	W12
	.byte		TIE   , Cs3
	.byte	W84
	.byte	W32
	.byte	W03
	.byte	EOT
	.byte	W01
	.byte		N32   , Dn3 , v088 , gtp3
	.byte	W36
	.byte		N23   , Fs3
	.byte	W24
	.byte		N11   , En3
	.byte	W12
	.byte		N32   , Bn2 , v088 , gtp3
	.byte	W36
	.byte		N80   , An2 , v088 , gtp3
	.byte	W48
	.byte	W36
	.byte		N23   , Cn3
	.byte	W24
	.byte		        Bn2
	.byte	W12
	.byte		VOL   , 46*mus_qz_653_mvl/mxv
	.byte		PAN   , c_v+38
	.byte	W12
	.byte		VOICE , 32
	.byte		N11   , En2 , v112
	.byte	W12
	.byte		        Fn2 , v088
	.byte	W12
	.byte		TIE   , An2
	.byte	W84
	.byte	W32
	.byte	W03
	.byte	EOT
	.byte	W01
	.byte		N23   , As2
	.byte	W24
	.byte		        Dn3
	.byte	W24
	.byte		        Cn3
	.byte	W12
	.byte	W12
	.byte		VOICE , 34
	.byte		PAN   , c_v+30
	.byte		TIE   , Cs3
	.byte	W84
	.byte	W32
	.byte	W03
	.byte	EOT
	.byte	W01
	.byte		N23   , Dn3
	.byte	W24
	.byte		        Bn2
	.byte	W24
	.byte		        Fs3
	.byte	W12
	.byte	W12
	.byte		VOICE , 35
	.byte		PAN   , c_v-34
	.byte		TIE   , Gn3
	.byte	W84
	.byte	W32
	.byte	W03
	.byte	EOT
	.byte	W01
	.byte		N23   , Fn3
	.byte	W24
	.byte		        Cn4
	.byte	W24
	.byte		        Ds4
	.byte	W12
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
	.byte	GOTO
	 ptrvalue	mus_qz_653_11_B1
	.byte	FINE

/********************** Track 12 **********************/

mus_qz_653_12:
	.byte		KEYSH , mus_qz_653_key+0
	.byte		VOL   , 46*mus_qz_653_mvl/mxv
	.byte		        89*mus_qz_653_mvl/mxv
	.byte		        59*mus_qz_653_mvl/mxv
	.byte		PRIO  , 62
	.byte		PAN   , c_v-48
	.byte		        c_v+42
	.byte	W24
	.byte		VOICE , 35
	.byte	W08
	.byte		N07   , Cs6 , v088
	.byte	W08
	.byte		        Gs5
	.byte	W08
	.byte		        Ds5
	.byte	W08
	.byte		        Fn4
	.byte	W08
	.byte		        As4
	.byte	W08
	.byte		VOL   , 20*mus_qz_653_mvl/mxv
	.byte		PAN   , c_v+48
	.byte		N07   , Ds5
	.byte	W08
	.byte		VOICE , 23
	.byte	W08
	.byte		N07   , Gn4
	.byte	W32
	.byte	W16
	.byte	N07
	.byte	W32
	.byte	W16
	.byte	N07
	.byte	W16
	.byte	N07
	.byte	W08
	.byte		N05   , Cs5
	.byte	W08
	.byte	W48
	.byte		VOICE , 28
	.byte		VOL   , 68*mus_qz_653_mvl/mxv
	.byte		PAN   , c_v+24
	.byte		VOL   , 100*mus_qz_653_mvl/mxv
	.byte		N15   , Gs2 , v016
	.byte	W16
	.byte		        Gs2 , v028
	.byte	W16
	.byte		        Gs2 , v048
	.byte	W16
	.byte	N15
	.byte	W16
	.byte		N30   , Ds2 , v088 , gtp1
	.byte	W80
	.byte	W96
	.byte	W72
	.byte		N15   , Gs2 , v016
	.byte	W16
	.byte		        Gs2 , v028
	.byte	W08
	.byte	W08
	.byte		        Gs2 , v048
	.byte	W16
	.byte	N15
	.byte	W16
	.byte		N30   , Ds2 , v088 , gtp1
	.byte	W56
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		VOICE , 32
	.byte		VOL   , 91*mus_qz_653_mvl/mxv
	.byte		PAN   , c_v+52
	.byte		VOL   , 65*mus_qz_653_mvl/mxv
	.byte	W12
	.byte		N32   , Gs2 , v112 , gtp1
	.byte		N32   , Dn3 , v112 , gtp1
	.byte	W36
	.byte		        Gn2 , v112 , gtp1
	.byte		N32   , Cs3 , v112 , gtp1
	.byte	W36
	.byte		        As2 , v112 , gtp1
	.byte		N32   , En3 , v112 , gtp1
	.byte	W12
	.byte	W24
	.byte		        An2 , v112 , gtp1
	.byte		N32   , Ds3 , v112 , gtp1
	.byte	W36
	.byte		        Cn3 , v112 , gtp1
	.byte		N32   , Fs3 , v112 , gtp1
	.byte	W36
	.byte		        Bn2 , v112 , gtp1
	.byte		N32   , Fn3 , v112 , gtp1
	.byte	W36
	.byte		        Cs3 , v112 , gtp1
	.byte	W36
	.byte		N68   , Dn3 , v112 , gtp1
	.byte	W24
	.byte	W48
	.byte		VOICE , 29
	.byte		N09   , En3 , v100
	.byte		N09   , As3
	.byte	W12
	.byte		N13   , En3 , v024
	.byte		N13   , As3
	.byte	W36
	.byte		N09   , En3 , v100
	.byte		N09   , As3
	.byte	W12
	.byte		N13   , En3 , v024
	.byte		N13   , As3
	.byte	W36
	.byte	W12
	.byte		N44   , En3 , v104 , gtp3
	.byte		N44   , Bn3 , v104 , gtp1
	.byte	W84
mus_qz_653_12_B1:
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
	.byte		PAN   , c_v+34
	.byte		VOL   , 82*mus_qz_653_mvl/mxv
	.byte	W12
	.byte		N32   , En3 , v088 , gtp3
	.byte		N32   , Cs4 , v088 , gtp3
	.byte	W84
	.byte	GOTO
	 ptrvalue	mus_qz_653_12_B1
	.byte	FINE

/******************************************************/
	.p2align	2

mus_qz_653:
	.byte	12	/* NumTrks */
	.byte	0	/* NumBlks */
	.byte	mus_qz_653_pri	/* Priority */
	.byte	mus_qz_653_rev	/* Reverb. */
space64 4
	ptrvalue	mus_qz_653_grp

	ptrvalue	mus_qz_653_1
	ptrvalue	mus_qz_653_2
	ptrvalue	mus_qz_653_3
	ptrvalue	mus_qz_653_4
	ptrvalue	mus_qz_653_5
	ptrvalue	mus_qz_653_6
	ptrvalue	mus_qz_653_7
	ptrvalue	mus_qz_653_8
	ptrvalue	mus_qz_653_9
	ptrvalue	mus_qz_653_10
	ptrvalue	mus_qz_653_11
	ptrvalue	mus_qz_653_12

	.end
