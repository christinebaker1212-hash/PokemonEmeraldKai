	.include "asm/macros/bit_width.inc"

	.include "MPlayDef.s"

/* Moemon Quetzal song 610, disassembled from the ROM by tools/quetzal_rip/songs.py.
   "org" comments give the ROM address of the bytes that follow. */

	.equ	mus_qz_610_grp, voicegroup_qz_610
	.equ	mus_qz_610_pri, 0
	.equ	mus_qz_610_rev, reverb_set+0
	.equ	mus_qz_610_mvl, 127
	.equ	mus_qz_610_key, 0
	.equ	mus_qz_610_tbs, 1
	.equ	mus_qz_610_exg, 1
	.equ	mus_qz_610_cmp, 1

	.section .rodata
	.global	mus_qz_610
	.p2align	2
/* org 0x09669A14 */

/********************** Track  1 **********************/

mus_qz_610_1:
	.byte		KEYSH , mus_qz_610_key+0
	.byte		TEMPO , 152*mus_qz_610_tbs/2
	.byte		VOICE , 56
	.byte		PAN   , c_v+0
	.byte		VOL   , 66*mus_qz_610_mvl/mxv
	.byte	W24
	.byte		N05   , Cn4 , v127
	.byte	W06
	.byte		        Cn4 , v020
	.byte	W06
	.byte		        Fn4 , v127
	.byte	W06
	.byte		        Fn4 , v020
	.byte	W18
	.byte		        Cn4 , v127
	.byte	W06
	.byte		        Cn4 , v020
	.byte	W06
	.byte		        Cn5 , v127
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Cn4
	.byte	W06
	.byte		        Cn4 , v020
	.byte	W06
	.byte		N23   , Cn5 , v127
	.byte	W24
	.byte	N05
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Cn5 , v020
	.byte	W24
	.byte		        Cn4 , v127
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		VOL   , 61*mus_qz_610_mvl/mxv
	.byte		N05   , Cn4 , v020
	.byte	W12
	.byte		VOICE , 60
	.byte		VOL   , 56*mus_qz_610_mvl/mxv
	.byte		N11   , Cn3 , v127
	.byte	W12
mus_qz_610_1_B1:
	.byte		VOICE , 60
	.byte		N05   , Fn3 , v127
	.byte	W06
	.byte		        Fn3 , v020
	.byte	W06
	.byte		        Cn3 , v127
	.byte	W06
	.byte		        Cn3 , v020
	.byte	W06
	.byte		        Gn3 , v127
	.byte	W06
	.byte		        Gn3 , v020
	.byte	W06
	.byte		        An3 , v127
	.byte	W06
	.byte		        An3 , v020
	.byte	W18
	.byte		N28   , Cn4 , v127 , gtp1
	.byte	W30
	.byte		N05   , Cn4 , v020
	.byte	W06
	.byte	W12
	.byte		        As3 , v127
	.byte	W06
	.byte		        As3 , v020
	.byte	W06
	.byte		        Dn4 , v127
	.byte	W06
	.byte		        Dn4 , v020
	.byte	W06
	.byte		        Fn4 , v127
	.byte	W06
	.byte		        Fn4 , v020
	.byte	W06
	.byte		        Gn4 , v127
	.byte	W06
	.byte		        Gn4 , v020
	.byte	W06
	.byte		        Fn4 , v127
	.byte	W06
	.byte		        Fn4 , v020
	.byte	W06
	.byte		        As4 , v127
	.byte	W06
	.byte		        As4 , v020
	.byte	W06
	.byte		N44   , An4 , v127 , gtp3
	.byte	W12
	.byte		VOL   , 55*mus_qz_610_mvl/mxv
	.byte	W02
	.byte		        43*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        38*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        36*mus_qz_610_mvl/mxv
	.byte	W06
	.byte		        40*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        46*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        50*mus_qz_610_mvl/mxv
	.byte	W04
	.byte		        56*mus_qz_610_mvl/mxv
	.byte	W12
	.byte	N05
	.byte	W06
	.byte		        As4
	.byte	W06
	.byte		N23   , Cn5
	.byte	W24
	.byte		        Cn4
	.byte	W24
	.byte	W12
	.byte		N05   , As3
	.byte	W06
	.byte		        As3 , v020
	.byte	W06
	.byte		        Cs4 , v127
	.byte	W06
	.byte		        Cs4 , v020
	.byte	W06
	.byte		        Fn4 , v127
	.byte	W06
	.byte		        Fn4 , v020
	.byte	W06
	.byte		        Gn4 , v127
	.byte	W06
	.byte		        Gn4 , v020
	.byte	W06
	.byte		        Fn4 , v127
	.byte	W06
	.byte		        Fn4 , v020
	.byte	W06
	.byte		        As4 , v127
	.byte	W06
	.byte		        As4 , v020
	.byte	W06
	.byte		N44   , An4 , v127 , gtp3
	.byte	W12
	.byte		VOL   , 51*mus_qz_610_mvl/mxv
	.byte	W02
	.byte		        46*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        40*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        36*mus_qz_610_mvl/mxv
	.byte	W04
	.byte		        33*mus_qz_610_mvl/mxv
	.byte	W02
	.byte		        34*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        37*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        45*mus_qz_610_mvl/mxv
	.byte	W04
	.byte		        57*mus_qz_610_mvl/mxv
	.byte	W12
	.byte	N05
	.byte	W06
	.byte		        As4
	.byte	W06
	.byte		N23   , Cn5
	.byte	W24
	.byte		        Cn4
	.byte	W24
	.byte		N05   , Dn4
	.byte	W06
	.byte		        Dn4 , v020
	.byte	W06
	.byte		        An3 , v127
	.byte	W06
	.byte		        An3 , v020
	.byte	W06
	.byte		        En4 , v127
	.byte	W06
	.byte		        En4 , v020
	.byte	W06
	.byte		N23   , Fn4 , v127
	.byte	W24
	.byte		        Dn4
	.byte	W24
	.byte		N05   , Fn4
	.byte	W06
	.byte		        An4
	.byte	W06
	.byte		VOL   , 37*mus_qz_610_mvl/mxv
	.byte		N44   , Fn4 , v127 , gtp3
	.byte	W02
	.byte		VOL   , 40*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        43*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        46*mus_qz_610_mvl/mxv
	.byte	W04
	.byte		        50*mus_qz_610_mvl/mxv
	.byte	W02
	.byte		        53*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        57*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        59*mus_qz_610_mvl/mxv
	.byte	W28
	.byte		N32   , En4 , v127 , gtp3
	.byte	W36
	.byte		N05   , Fn4
	.byte	W06
	.byte		        Gn4
	.byte	W06
	.byte		N11   , An4
	.byte	W12
	.byte		N05   , An4 , v020
	.byte	W12
	.byte		N11   , Gn4 , v127
	.byte	W12
	.byte		N05   , Gn4 , v020
	.byte	W12
	.byte		N11   , Fn4 , v127
	.byte	W12
	.byte		N05   , Fn4 , v020
	.byte	W12
	.byte		N11   , Cn4 , v127
	.byte	W12
	.byte		N05   , Dn4
	.byte	W06
	.byte		        En4
	.byte	W06
	.byte		N44   , Fn4 , v127 , gtp3
	.byte	W48
	.byte		N32   , Gn4 , v127 , gtp3
	.byte	W36
	.byte		N05   , Dn4
	.byte	W06
	.byte		        En4
	.byte	W06
	.byte		N92   , Fn4 , v127 , gtp3
	.byte	W09
	.byte		VOL   , 57*mus_qz_610_mvl/mxv
	.byte	W02
	.byte		        53*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        43*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        36*mus_qz_610_mvl/mxv
	.byte	W09
	.byte		        40*mus_qz_610_mvl/mxv
	.byte	W01
	.byte		        33*mus_qz_610_mvl/mxv
	.byte	W02
	.byte		        34*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        37*mus_qz_610_mvl/mxv
	.byte	W04
	.byte		        40*mus_qz_610_mvl/mxv
	.byte	W02
	.byte		        43*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        53*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        57*mus_qz_610_mvl/mxv
	.byte	W01
	.byte		        59*mus_qz_610_mvl/mxv
	.byte	W48
	.byte	W03
	.byte		N05   , Fn4 , v020
	.byte	W12
	.byte		        Fn4 , v127
	.byte	W06
	.byte		        Fn4 , v020
	.byte	W06
	.byte		        Fn4 , v127
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Fn4 , v020
	.byte	W06
	.byte		        Fn4 , v127
	.byte	W06
	.byte		VOICE , 56
	.byte		N05   , Fn4 , v020
	.byte	W12
	.byte		VOL   , 80*mus_qz_610_mvl/mxv
	.byte		N11   , Cn4 , v127
	.byte	W12
	.byte		N05   , Dn4
	.byte	W06
	.byte		        Cs4
	.byte	W06
	.byte		N11   , Cn4
	.byte	W12
	.byte		        Cn5
	.byte	W12
	.byte		N05   , Cn5 , v020
	.byte	W12
	.byte		N11   , Fn4 , v127
	.byte	W12
	.byte		N05   , Fn4 , v020
	.byte	W12
	.byte		N11   , Cn4 , v127
	.byte	W12
	.byte		N23   , Gn4
	.byte	W24
	.byte		N05   , Fn4
	.byte	W06
	.byte		        Gn4
	.byte	W06
	.byte		N11   , En4
	.byte	W12
	.byte		        Cn4
	.byte	W12
	.byte		        Dn4
	.byte	W12
	.byte		        An3
	.byte	W12
	.byte		N05   , An3 , v020
	.byte	W12
	.byte		N11   , Cn4 , v127
	.byte	W12
	.byte		N05   , Dn4
	.byte	W06
	.byte		        Cs4
	.byte	W06
	.byte		N11   , Cn4
	.byte	W12
	.byte		        Cn5
	.byte	W12
	.byte		N05   , Cn5 , v020
	.byte	W12
	.byte		N11   , Fn4 , v127
	.byte	W12
	.byte		N05   , Fn4 , v020
	.byte	W12
	.byte		N11   , En4 , v127
	.byte	W12
	.byte		N23   , Fn4
	.byte	W24
	.byte		N05   , Gn4
	.byte	W06
	.byte		        Fn4
	.byte	W06
	.byte		N11   , An4
	.byte	W12
	.byte		        As4
	.byte	W12
	.byte		        An4
	.byte	W12
	.byte		        Gn4
	.byte	W12
	.byte		N05   , Gn4 , v020
	.byte	W12
	.byte		N23   , Fn4 , v127
	.byte	W24
	.byte		N05   , En4
	.byte	W06
	.byte		        Fn4
	.byte	W06
	.byte		N23   , Cn4
	.byte	W24
	.byte		N11   , Fn4
	.byte	W12
	.byte		N23   , En4
	.byte	W24
	.byte		N11   , Fn4
	.byte	W12
	.byte		        Gn4
	.byte	W12
	.byte		        As4
	.byte	W12
	.byte		        An4
	.byte	W12
	.byte		        Gs4
	.byte	W12
	.byte		        Gn4
	.byte	W12
	.byte		N23   , Fn4
	.byte	W24
	.byte		N05   , En4
	.byte	W06
	.byte		        Fn4
	.byte	W06
	.byte		N11   , Dn5
	.byte	W12
	.byte		TIE   , Cn5
	.byte	W12
	.byte	W21
	.byte		VOL   , 71*mus_qz_610_mvl/mxv
	.byte	W02
	.byte		        63*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        57*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        50*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        45*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        40*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        34*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        31*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        29*mus_qz_610_mvl/mxv
	.byte	W01
	.byte		        27*mus_qz_610_mvl/mxv
	.byte	W02
	.byte		        25*mus_qz_610_mvl/mxv
	.byte	W48
	.byte	W01
	.byte		        27*mus_qz_610_mvl/mxv
	.byte	W02
	.byte		        28*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        31*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        36*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        41*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        48*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        55*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        59*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        63*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        69*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        76*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        80*mus_qz_610_mvl/mxv
	.byte	W15
	.byte	EOT
	.byte	W13
	.byte	N11
	.byte	W18
	.byte	N11
	.byte	W18
	.byte		VOICE , 60
	.byte		VOL   , 57*mus_qz_610_mvl/mxv
	.byte	W24
	.byte		N23   , Cn3
	.byte	W24
	.byte		        Fn3
	.byte	W24
	.byte		        An3
	.byte	W24
mus_qz_610_1_000:
	.byte		N23   , As3 , v127
	.byte	W24
	.byte		        An3
	.byte	W24
	.byte		        Fn3
	.byte	W24
	.byte		        Dn4
	.byte	W24
	.byte	PEND
	.byte		        Cn4
	.byte	W24
	.byte		N05   , Cn4 , v020
	.byte	W12
	.byte		N11   , Fn3 , v127
	.byte	W12
	.byte	N23
	.byte	W24
	.byte		N11   , Cn3
	.byte	W12
	.byte		        Gn3
	.byte	W12
	.byte		N05   , Gn3 , v020
	.byte	W12
	.byte		N11   , Gn3 , v127
	.byte	W12
	.byte		N23   , Fn3
	.byte	W24
	.byte		        Cn4
	.byte	W24
	.byte		        Dn4
	.byte	W24
	.byte		        Ds4
	.byte	W24
	.byte		        An3
	.byte	W24
	.byte		        Cn4
	.byte	W24
	.byte		N11   , Dn4
	.byte	W12
	.byte		        Cn4
	.byte	W12
	.byte		N05   , Cn4 , v020
	.byte	W12
	.byte		N11   , Cn4 , v127
	.byte	W12
	.byte		N23   , As3
	.byte	W24
	.byte		        An3
	.byte	W24
	.byte		        As3
	.byte	W24
	.byte		        Fn4
	.byte	W24
	.byte		        Cs4
	.byte	W24
	.byte		        Cn4
	.byte	W24
	.byte		        As3
	.byte	W24
	.byte		        An3
	.byte	W24
	.byte		        Gn3
	.byte	W24
	.byte		        Fn3
	.byte	W24
	.byte		N11   , Gn3
	.byte	W12
	.byte		N32   , Fn3 , v127 , gtp3
	.byte	W12
	.byte	W24
	.byte		VOICE , 56
	.byte		VOL   , 80*mus_qz_610_mvl/mxv
	.byte		N23   , Cn3
	.byte	W24
	.byte		        Gn3
	.byte	W24
	.byte		        An3
	.byte	W24
	.byte	PATT
	 ptrvalue	mus_qz_610_1_000
	.byte		N23   , Cn4 , v127
	.byte	W24
	.byte		N05   , Cn4 , v020
	.byte	W12
	.byte		N11   , Fn4 , v127
	.byte	W12
	.byte	N23
	.byte	W24
	.byte		N11   , Cn4
	.byte	W12
	.byte		        Gn4
	.byte	W12
	.byte		N05   , Gn4 , v020
	.byte	W12
	.byte		N11   , Gn4 , v127
	.byte	W12
	.byte		N23   , Fn4
	.byte	W24
	.byte		        En4
	.byte	W24
	.byte		        Fn4
	.byte	W24
	.byte		N11   , Cn5
	.byte	W12
	.byte		N05   , Cn5 , v020
	.byte	W12
	.byte		N11   , Fn4 , v127
	.byte	W12
	.byte		N05   , Fn4 , v020
	.byte	W12
	.byte		N11   , Gn4 , v127
	.byte	W12
	.byte		N05   , Gn4 , v020
	.byte	W12
	.byte		        Cn4 , v127
	.byte	W06
	.byte		        Cn4 , v020
	.byte	W06
	.byte		N23   , Fn4 , v127
	.byte	W12
	.byte	W12
	.byte		N11   , Gn4
	.byte	W12
	.byte		        An4
	.byte	W12
	.byte		        As4
	.byte	W12
	.byte		        An4
	.byte	W12
	.byte		        Cn4
	.byte	W12
	.byte		        Fn4
	.byte	W12
	.byte		N05   , Gn4
	.byte	W06
	.byte		TIE   , Fn4
	.byte	W06
	.byte	W09
	.byte		VOL   , 80*mus_qz_610_mvl/mxv
	.byte	W02
	.byte		        78*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        76*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        68*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        64*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        61*mus_qz_610_mvl/mxv
	.byte	W06
	.byte		        57*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        53*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        51*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        48*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        45*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        40*mus_qz_610_mvl/mxv
	.byte	W06
	.byte		        37*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        34*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        31*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        29*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        27*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        25*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        23*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        22*mus_qz_610_mvl/mxv
	.byte	W12
	.byte		        21*mus_qz_610_mvl/mxv
	.byte	W06
	.byte		        20*mus_qz_610_mvl/mxv
	.byte	W07
	.byte	W02
	.byte		        19*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        20*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        21*mus_qz_610_mvl/mxv
	.byte	W06
	.byte		        22*mus_qz_610_mvl/mxv
	.byte	W12
	.byte		        25*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        28*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        30*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        34*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        40*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        43*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        48*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        59*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		        76*mus_qz_610_mvl/mxv
	.byte	W06
	.byte		        80*mus_qz_610_mvl/mxv
	.byte	W36
	.byte	W03
	.byte	EOT
	.byte	W01
	.byte	W24
	.byte		N05   , Cn4
	.byte	W06
	.byte		        Cn4 , v020
	.byte	W06
	.byte		        Fn4 , v127
	.byte	W06
	.byte		        Fn4 , v020
	.byte	W18
	.byte		        Cn4 , v127
	.byte	W06
	.byte		        Cn4 , v020
	.byte	W06
	.byte		        Cn5 , v127
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Cn4
	.byte	W06
	.byte		        Cn4 , v020
	.byte	W06
	.byte	W12
	.byte		        Cn4 , v127
	.byte	W06
	.byte		        Cn4 , v020
	.byte	W06
	.byte		        Fn4 , v127
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Fn4 , v020
	.byte	W48
	.byte		VOICE , 60
	.byte		N11   , Cn3 , v080
	.byte	W12
	.byte	GOTO
	 ptrvalue	mus_qz_610_1_B1
	.byte	FINE

/********************** Track  2 **********************/

mus_qz_610_2:
	.byte		KEYSH , mus_qz_610_key+0
	.byte		VOICE , 56
	.byte		BENDR , 12
	.byte		VOL   , 15*mus_qz_610_mvl/mxv
	.byte		PAN   , c_v+34
	.byte		BEND  , c_v+1
	.byte	W24
	.byte	W03
	.byte		N05   , Cn4 , v127
	.byte	W06
	.byte		        Cn4 , v020
	.byte	W06
	.byte		        Fn4 , v127
	.byte	W06
	.byte		        Fn4 , v020
	.byte	W18
	.byte		        Cn4 , v127
	.byte	W06
	.byte		        Cn4 , v020
	.byte	W06
	.byte		        Cn5 , v127
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Cn4
	.byte	W06
	.byte		        Cn4 , v020
	.byte	W03
	.byte	W03
	.byte		N23   , Cn5 , v127
	.byte	W24
	.byte	N05
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Cn5 , v020
	.byte	W24
	.byte		        Cn4 , v127
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Cn4 , v020
	.byte	W12
	.byte		VOICE , 60
	.byte		N08   , Cn3 , v127
	.byte	W09
mus_qz_610_2_B1:
	.byte	W03
	.byte		VOICE , 60
	.byte		N05   , Fn3 , v127
	.byte	W06
	.byte		        Fn3 , v020
	.byte	W06
	.byte		        Cn3 , v127
	.byte	W06
	.byte		        Cn3 , v020
	.byte	W06
	.byte		        Gn3 , v127
	.byte	W06
	.byte		        Gn3 , v020
	.byte	W06
	.byte		        An3 , v127
	.byte	W06
	.byte		        An3 , v020
	.byte	W18
	.byte		N28   , Cn4 , v127 , gtp1
	.byte	W30
	.byte		N05   , Cn4 , v020
	.byte	W03
	.byte	W15
	.byte		        As3 , v127
	.byte	W06
	.byte		        As3 , v020
	.byte	W06
	.byte		        Dn4 , v127
	.byte	W06
	.byte		        Dn4 , v020
	.byte	W06
	.byte		        Fn4 , v127
	.byte	W06
	.byte		        Fn4 , v020
	.byte	W06
	.byte		        Gn4 , v127
	.byte	W06
	.byte		        Gn4 , v020
	.byte	W06
	.byte		        Fn4 , v127
	.byte	W06
	.byte		        Fn4 , v020
	.byte	W06
	.byte		        As4 , v127
	.byte	W06
	.byte		        As4 , v020
	.byte	W06
	.byte		N44   , An4 , v127 , gtp3
	.byte	W09
	.byte	W06
	.byte		VOL   , 8*mus_qz_610_mvl/mxv
	.byte	W06
	.byte		        3*mus_qz_610_mvl/mxv
	.byte	W12
	.byte		        8*mus_qz_610_mvl/mxv
	.byte	W06
	.byte		        17*mus_qz_610_mvl/mxv
	.byte	W09
	.byte	N05
	.byte	W06
	.byte		        As4
	.byte	W06
	.byte		N23   , Cn5
	.byte	W24
	.byte		        Cn4
	.byte	W21
	.byte	W15
	.byte		N05   , As3
	.byte	W06
	.byte		        As3 , v020
	.byte	W06
	.byte		        Cs4 , v127
	.byte	W06
	.byte		        Cs4 , v020
	.byte	W06
	.byte		        Fn4 , v127
	.byte	W06
	.byte		        Fn4 , v020
	.byte	W06
	.byte		        Gn4 , v127
	.byte	W06
	.byte		        Gn4 , v020
	.byte	W06
	.byte		        Fn4 , v127
	.byte	W06
	.byte		        Fn4 , v020
	.byte	W06
	.byte		        As4 , v127
	.byte	W06
	.byte		        As4 , v020
	.byte	W06
	.byte		N44   , An4 , v127 , gtp3
	.byte	W09
	.byte	W06
	.byte		VOL   , 10*mus_qz_610_mvl/mxv
	.byte	W06
	.byte		        3*mus_qz_610_mvl/mxv
	.byte	W12
	.byte		        8*mus_qz_610_mvl/mxv
	.byte	W06
	.byte		        15*mus_qz_610_mvl/mxv
	.byte	W09
	.byte	N05
	.byte	W06
	.byte		        As4
	.byte	W06
	.byte		N23   , Cn5
	.byte	W24
	.byte		        Cn4
	.byte	W21
	.byte	W03
	.byte		N05   , Dn4
	.byte	W06
	.byte		        Dn4 , v020
	.byte	W06
	.byte		        An3 , v127
	.byte	W06
	.byte		        An3 , v020
	.byte	W06
	.byte		        En4 , v127
	.byte	W06
	.byte		        En4 , v020
	.byte	W06
	.byte		N23   , Fn4 , v127
	.byte	W24
	.byte		        Dn4
	.byte	W24
	.byte		N05   , Fn4
	.byte	W06
	.byte		        An4
	.byte	W03
	.byte		VOL   , 4*mus_qz_610_mvl/mxv
	.byte	W03
	.byte		N44   , Fn4 , v127 , gtp3
	.byte	W09
	.byte		VOL   , 8*mus_qz_610_mvl/mxv
	.byte	W06
	.byte		        12*mus_qz_610_mvl/mxv
	.byte	W12
	.byte		        15*mus_qz_610_mvl/mxv
	.byte	W21
	.byte		N32   , En4 , v127 , gtp3
	.byte	W36
	.byte		N05   , Fn4
	.byte	W06
	.byte		        Gn4
	.byte	W03
	.byte	W03
	.byte		N11   , An4
	.byte	W12
	.byte		N05   , An4 , v020
	.byte	W12
	.byte		N11   , Gn4 , v127
	.byte	W12
	.byte		N05   , Gn4 , v020
	.byte	W12
	.byte		N11   , Fn4 , v127
	.byte	W12
	.byte		N05   , Fn4 , v020
	.byte	W12
	.byte		N11   , Cn4 , v127
	.byte	W12
	.byte		N05   , Dn4
	.byte	W06
	.byte		        En4
	.byte	W03
	.byte	W03
	.byte		N44   , Fn4 , v127 , gtp3
	.byte	W48
	.byte		N32   , Gn4 , v127 , gtp3
	.byte	W36
	.byte		N05   , Dn4
	.byte	W06
	.byte		        En4
	.byte	W03
	.byte	W03
	.byte		N92   , Fn4 , v127 , gtp3
	.byte	W09
	.byte		VOL   , 11*mus_qz_610_mvl/mxv
	.byte	W06
	.byte		        10*mus_qz_610_mvl/mxv
	.byte	W06
	.byte		        6*mus_qz_610_mvl/mxv
	.byte	W06
	.byte		        10*mus_qz_610_mvl/mxv
	.byte	W06
	.byte		        13*mus_qz_610_mvl/mxv
	.byte	W06
	.byte		        15*mus_qz_610_mvl/mxv
	.byte	W06
	.byte		        17*mus_qz_610_mvl/mxv
	.byte	W48
	.byte	W03
	.byte		N05   , Fn4 , v020
	.byte	W12
	.byte		        Fn4 , v127
	.byte	W06
	.byte		        Fn4 , v020
	.byte	W06
	.byte		        Fn4 , v127
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Fn4 , v020
	.byte	W06
	.byte		        Fn4 , v127
	.byte	W06
	.byte		VOICE , 56
	.byte		N05   , Fn4 , v020
	.byte	W12
	.byte		N11   , Cn4 , v127
	.byte	W12
	.byte		N05   , Dn4
	.byte	W06
	.byte		        Cs4
	.byte	W06
	.byte		N11   , Cn4
	.byte	W09
	.byte	W03
	.byte		        Cn5
	.byte	W12
	.byte		N05   , Cn5 , v020
	.byte	W12
	.byte		N11   , Fn4 , v127
	.byte	W12
	.byte		N05   , Fn4 , v020
	.byte	W12
	.byte		N11   , Cn4 , v127
	.byte	W12
	.byte		N23   , Gn4
	.byte	W24
	.byte		N05   , Fn4
	.byte	W06
	.byte		        Gn4
	.byte	W03
	.byte	W03
	.byte		N11   , En4
	.byte	W12
	.byte		        Cn4
	.byte	W12
	.byte		        Dn4
	.byte	W12
	.byte		        An3
	.byte	W12
	.byte		N05   , An3 , v020
	.byte	W12
	.byte		N11   , Cn4 , v127
	.byte	W12
	.byte		N05   , Dn4
	.byte	W06
	.byte		        Cs4
	.byte	W06
	.byte		N11   , Cn4
	.byte	W09
	.byte	W03
	.byte		        Cn5
	.byte	W12
	.byte		N05   , Cn5 , v020
	.byte	W12
	.byte		N11   , Fn4 , v127
	.byte	W12
	.byte		N05   , Fn4 , v020
	.byte	W12
	.byte		N11   , En4 , v127
	.byte	W12
	.byte		N23   , Fn4
	.byte	W24
	.byte		N05   , Gn4
	.byte	W06
	.byte		        Fn4
	.byte	W03
	.byte	W03
	.byte		N11   , An4
	.byte	W12
	.byte		        As4
	.byte	W12
	.byte		        An4
	.byte	W12
	.byte		        Gn4
	.byte	W12
	.byte		N05   , Gn4 , v020
	.byte	W12
	.byte		N23   , Fn4 , v127
	.byte	W24
	.byte		N05   , En4
	.byte	W06
	.byte		        Fn4
	.byte	W03
	.byte	W03
	.byte		N23   , Cn4
	.byte	W24
	.byte		N11   , Fn4
	.byte	W12
	.byte		N23   , En4
	.byte	W24
	.byte		N11   , Fn4
	.byte	W12
	.byte		        Gn4
	.byte	W12
	.byte		        As4
	.byte	W09
	.byte	W03
	.byte		        An4
	.byte	W12
	.byte		        Gs4
	.byte	W12
	.byte		        Gn4
	.byte	W12
	.byte		N23   , Fn4
	.byte	W24
	.byte		N05   , En4
	.byte	W06
	.byte		        Fn4
	.byte	W06
	.byte		N11   , Dn5
	.byte	W12
	.byte		TIE   , Cn5
	.byte	W09
	.byte	W24
	.byte		VOL   , 13*mus_qz_610_mvl/mxv
	.byte	W06
	.byte		        7*mus_qz_610_mvl/mxv
	.byte	W06
	.byte		        3*mus_qz_610_mvl/mxv
	.byte	W06
	.byte		        5*mus_qz_610_mvl/mxv
	.byte	W54
	.byte	W18
	.byte		        5*mus_qz_610_mvl/mxv
	.byte	W06
	.byte		        8*mus_qz_610_mvl/mxv
	.byte	W02
	.byte		        10*mus_qz_610_mvl/mxv
	.byte	W04
	.byte		        14*mus_qz_610_mvl/mxv
	.byte	W06
	.byte		        15*mus_qz_610_mvl/mxv
	.byte	W14
	.byte	EOT
	.byte	W13
	.byte	N11
	.byte	W18
	.byte	N11
	.byte	W15
	.byte	W03
	.byte		VOICE , 60
	.byte	W24
	.byte		N23   , Cn3
	.byte	W24
	.byte		        Fn3
	.byte	W24
	.byte		        An3
	.byte	W21
mus_qz_610_2_000:
	.byte	W03
	.byte		N23   , As3 , v127
	.byte	W24
	.byte		        An3
	.byte	W24
	.byte		        Fn3
	.byte	W24
	.byte		        Dn4
	.byte	W21
	.byte	PEND
	.byte	W03
	.byte		        Cn4
	.byte	W24
	.byte		N05   , Cn4 , v020
	.byte	W12
	.byte		N11   , Fn3 , v127
	.byte	W12
	.byte	N23
	.byte	W24
	.byte		N11   , Cn3
	.byte	W12
	.byte		        Gn3
	.byte	W09
	.byte	W03
	.byte		N05   , Gn3 , v020
	.byte	W12
	.byte		N11   , Gn3 , v127
	.byte	W12
	.byte		N23   , Fn3
	.byte	W24
	.byte		        Cn4
	.byte	W24
	.byte		        Dn4
	.byte	W21
	.byte	W03
	.byte		        Ds4
	.byte	W24
	.byte		        An3
	.byte	W24
	.byte		        Cn4
	.byte	W24
	.byte		N11   , Dn4
	.byte	W12
	.byte		        Cn4
	.byte	W09
	.byte	W03
	.byte		N05   , Cn4 , v020
	.byte	W12
	.byte		N11   , Cn4 , v127
	.byte	W12
	.byte		N23   , As3
	.byte	W24
	.byte		        An3
	.byte	W24
	.byte		        As3
	.byte	W21
	.byte	W03
	.byte		        Fn4
	.byte	W24
	.byte		        Cs4
	.byte	W24
	.byte		        Cn4
	.byte	W24
	.byte		        As3
	.byte	W21
	.byte	W03
	.byte		        An3
	.byte	W24
	.byte		        Gn3
	.byte	W24
	.byte		        Fn3
	.byte	W24
	.byte		N11   , Gn3
	.byte	W12
	.byte		N32   , Fn3 , v127 , gtp3
	.byte	W09
	.byte	W24
	.byte	W03
	.byte		VOICE , 56
	.byte		N23   , Cn3
	.byte	W24
	.byte		        Gn3
	.byte	W24
	.byte		        An3
	.byte	W21
	.byte	PATT
	 ptrvalue	mus_qz_610_2_000
	.byte	W03
	.byte		N23   , Cn4 , v127
	.byte	W24
	.byte		N05   , Cn4 , v020
	.byte	W12
	.byte		N11   , Fn4 , v127
	.byte	W12
	.byte	N23
	.byte	W24
	.byte		N11   , Cn4
	.byte	W12
	.byte		        Gn4
	.byte	W09
	.byte	W03
	.byte		N05   , Gn4 , v020
	.byte	W12
	.byte		N11   , Gn4 , v127
	.byte	W12
	.byte		N23   , Fn4
	.byte	W24
	.byte		        En4
	.byte	W24
	.byte		        Fn4
	.byte	W21
	.byte	W03
	.byte		N11   , Cn5
	.byte	W12
	.byte		N05   , Cn5 , v020
	.byte	W12
	.byte		N11   , Fn4 , v127
	.byte	W12
	.byte		N05   , Fn4 , v020
	.byte	W12
	.byte		N11   , Gn4 , v127
	.byte	W12
	.byte		N05   , Gn4 , v020
	.byte	W12
	.byte		        Cn4 , v127
	.byte	W06
	.byte		        Cn4 , v020
	.byte	W06
	.byte		N23   , Fn4 , v127
	.byte	W09
	.byte	W15
	.byte		N11   , Gn4
	.byte	W12
	.byte		        An4
	.byte	W12
	.byte		        As4
	.byte	W12
	.byte		        An4
	.byte	W12
	.byte		        Cn4
	.byte	W12
	.byte		        Fn4
	.byte	W12
	.byte		N05   , Gn4
	.byte	W06
	.byte		TIE   , Fn4
	.byte	W03
	.byte	W18
	.byte		VOL   , 10*mus_qz_610_mvl/mxv
	.byte	W12
	.byte		        7*mus_qz_610_mvl/mxv
	.byte	W06
	.byte		        3*mus_qz_610_mvl/mxv
	.byte	W06
	.byte		        2*mus_qz_610_mvl/mxv
	.byte	W06
	.byte		        2*mus_qz_610_mvl/mxv
	.byte	W48
	.byte	W30
	.byte		        5*mus_qz_610_mvl/mxv
	.byte	W06
	.byte		        8*mus_qz_610_mvl/mxv
	.byte	W12
	.byte		        12*mus_qz_610_mvl/mxv
	.byte	W06
	.byte		        17*mus_qz_610_mvl/mxv
	.byte	W06
	.byte		        15*mus_qz_610_mvl/mxv
	.byte	W36
	.byte	W02
	.byte	EOT
	.byte	W24
	.byte	W01
	.byte		N05   , Cn4
	.byte	W06
	.byte		        Cn4 , v020
	.byte	W06
	.byte		        Fn4 , v127
	.byte	W06
	.byte		        Fn4 , v020
	.byte	W18
	.byte		        Cn4 , v127
	.byte	W06
	.byte		        Cn4 , v020
	.byte	W06
	.byte		        Cn5 , v127
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Cn4
	.byte	W06
	.byte		        Cn4 , v020
	.byte	W03
	.byte	W15
	.byte		        Cn4 , v127
	.byte	W06
	.byte		        Cn4 , v020
	.byte	W06
	.byte		        Fn4 , v127
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Fn4 , v020
	.byte	W32
	.byte	W01
	.byte		VOICE , 60
	.byte	W15
	.byte		N08   , Cn3 , v127
	.byte	W09
	.byte	GOTO
	 ptrvalue	mus_qz_610_2_B1
	.byte	FINE

/********************** Track  3 **********************/

mus_qz_610_3:
	.byte		KEYSH , mus_qz_610_key+0
	.byte		VOICE , 56
	.byte		PAN   , c_v+0
	.byte		VOL   , 62*mus_qz_610_mvl/mxv
	.byte		N05   , Cn3 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Cn3 , v020
	.byte	W12
	.byte		        Cn3 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Cn3 , v020
	.byte	W06
	.byte		        Fn3 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Fn3 , v020
	.byte	W06
	.byte		        Cn3 , v100
	.byte	W06
	.byte		        Fn3
	.byte	W06
	.byte		        Gn3
	.byte	W06
	.byte		        Gn3 , v020
	.byte	W06
	.byte		N28   , Cn4 , v100 , gtp1
	.byte	W06
	.byte	W24
	.byte		N05   , Cn3
	.byte	W06
	.byte	N05
	.byte	W18
	.byte	N05
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Cn3 , v020
	.byte	W36
mus_qz_610_3_B1:
	.byte		VOICE , 57
	.byte		PAN   , c_v+23
	.byte		VOL   , 76*mus_qz_610_mvl/mxv
	.byte	W24
	.byte		N05   , Cn3 , v100
	.byte	W06
	.byte		        Cn3 , v020
	.byte	W30
	.byte		N23   , Cn3 , v100
	.byte	W24
	.byte		N05   , Cn3 , v020
	.byte	W12
	.byte	W24
	.byte		        Dn3 , v100
	.byte	W06
	.byte		        Dn3 , v020
	.byte	W30
	.byte		N23   , Dn3 , v100
	.byte	W24
	.byte		N05   , Dn3 , v020
	.byte	W12
	.byte	W24
	.byte		        Cn3 , v100
	.byte	W06
	.byte		        Cn3 , v020
	.byte	W30
	.byte		N23   , Cn3 , v100
	.byte	W24
	.byte		N05   , Cn3 , v020
	.byte	W12
	.byte	W24
	.byte		        Cs3 , v100
	.byte	W06
	.byte		        Cs3 , v020
	.byte	W30
	.byte		N23   , Cs3 , v100
	.byte	W24
	.byte		N05   , Cs3 , v020
	.byte	W12
	.byte	W24
	.byte		N11   , Fn3 , v100
	.byte	W12
	.byte		N23   , Cn3
	.byte	W24
	.byte	N11
	.byte	W12
	.byte		        Fn3
	.byte	W12
	.byte		        Cn3
	.byte	W12
	.byte		        Dn3
	.byte	W12
	.byte		        An2
	.byte	W12
	.byte		        Fn3
	.byte	W12
	.byte		N23   , Dn3
	.byte	W24
	.byte	N11
	.byte	W12
	.byte		        Fn3
	.byte	W12
	.byte		        An3
	.byte	W12
	.byte		VOICE , 56
	.byte		N11   , As3
	.byte	W12
	.byte		N05   , As3 , v020
	.byte	W06
	.byte		        As3 , v100
	.byte	W06
	.byte		        As3 , v020
	.byte	W24
	.byte		N11   , Cn4 , v100
	.byte	W12
	.byte		N05   , Cn4 , v020
	.byte	W06
	.byte		        Cn4 , v100
	.byte	W06
	.byte		        Cn4 , v020
	.byte	W24
	.byte		N11   , An3 , v100
	.byte	W12
	.byte		N05   , An3 , v020
	.byte	W12
	.byte		N11   , Gn3 , v100
	.byte	W12
	.byte		N05   , Gn3 , v020
	.byte	W24
	.byte		        Fn3 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        En3
	.byte	W06
	.byte		        Fn3
	.byte	W06
	.byte		        Fn3 , v020
	.byte	W12
	.byte	W12
	.byte		N11   , As2 , v100
	.byte	W12
	.byte		        Dn3
	.byte	W12
	.byte		        Fn3
	.byte	W12
	.byte		        En3
	.byte	W12
	.byte		        As2
	.byte	W12
	.byte		        Cn3
	.byte	W12
	.byte		        En3
	.byte	W12
	.byte		N23   , Fn3
	.byte	W24
	.byte		        Fn2
	.byte	W24
	.byte		N05   , An2
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        An2 , v020
	.byte	W12
	.byte		        Cn3 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Cn3 , v020
	.byte	W12
	.byte	W12
	.byte		        Fn3 , v100
	.byte	W06
	.byte		        Fn3 , v020
	.byte	W06
	.byte		        Fn3 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Fn3 , v020
	.byte	W06
	.byte		N11   , Fn3 , v100
	.byte	W12
	.byte		N05   , Fn3 , v020
	.byte	W06
	.byte		        Ds3 , v100
	.byte	W06
	.byte		        Ds3 , v020
	.byte	W06
	.byte		        Ds3 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Ds3 , v020
	.byte	W06
	.byte		        Ds3 , v100
	.byte	W06
	.byte		VOICE , 27
	.byte		PAN   , c_v+45
	.byte		VOL   , 64*mus_qz_610_mvl/mxv
	.byte	W12
	.byte		N01   , Fn3 , v060
	.byte	W12
	.byte		N23   , Fn3 , v100
	.byte	W24
	.byte		N01   , Fn3 , v060
	.byte	W12
	.byte		N23   , Gn3 , v100
	.byte	W24
	.byte		N05   , An3
	.byte	W06
	.byte		        As3
	.byte	W06
	.byte		N11   , Cn4
	.byte	W12
	.byte		        An3
	.byte	W12
	.byte		N05   , Gn3
	.byte	W06
	.byte		        Fs3
	.byte	W06
	.byte		        Fn3
	.byte	W06
	.byte		N01   , Fn3 , v060
	.byte	W06
	.byte	N01
	.byte	W12
	.byte		N11   , Fn3 , v100
	.byte	W12
	.byte		N01   , Fn3 , v060
	.byte	W06
	.byte		N17   , Fn3 , v100
	.byte	W18
	.byte		N11   , Cn4
	.byte	W12
	.byte		N01   , Cn4 , v060
	.byte	W12
	.byte		N23   , An3 , v100
	.byte	W24
	.byte		N01   , An3 , v060
	.byte	W12
	.byte		N23   , Gn3 , v100
	.byte	W24
	.byte		N05   , An3
	.byte	W06
	.byte		        As3
	.byte	W06
	.byte		N11   , Cn4
	.byte	W12
	.byte		N01   , Cn4 , v060
	.byte	W06
	.byte		N11   , Cn4 , v100
	.byte	W12
	.byte		N01   , Cn4 , v060
	.byte	W18
	.byte		N11   , En4 , v100
	.byte	W12
	.byte		N01   , En4 , v060
	.byte	W06
	.byte		N11   , En4 , v100
	.byte	W12
	.byte		        Cn4
	.byte	W12
	.byte		N05   , As3
	.byte	W06
	.byte		N11   , An3
	.byte	W12
	.byte		        As3
	.byte	W12
	.byte		        An3
	.byte	W12
	.byte		N23   , As3
	.byte	W24
	.byte		N11   , An3
	.byte	W12
	.byte		        As3
	.byte	W12
	.byte		        Gn3
	.byte	W12
	.byte		        Cn4
	.byte	W12
	.byte		        As3
	.byte	W12
	.byte		        Cn4
	.byte	W12
	.byte		N23   , Gn3
	.byte	W24
	.byte		N11   , Bn3
	.byte	W12
	.byte		        Dn4
	.byte	W12
	.byte		        Bn3
	.byte	W12
	.byte		        Cn4
	.byte	W12
	.byte		N05   , Cn3
	.byte	W06
	.byte		N01   , Cn3 , v060
	.byte	W06
	.byte		N11   , Cn3 , v100
	.byte	W12
	.byte		N05   , Fn3
	.byte	W06
	.byte		N01   , Fn3 , v060
	.byte	W06
	.byte		N11   , Fn3 , v100
	.byte	W12
	.byte		N05   , Cn3
	.byte	W06
	.byte		N01   , Cn3 , v060
	.byte	W06
	.byte		N05   , Fn3 , v100
	.byte	W06
	.byte		        Gn3
	.byte	W06
	.byte		N01   , Gn3 , v060
	.byte	W06
	.byte		N52   , Cn4 , v100 , gtp1
	.byte	W06
	.byte	W48
	.byte		N01   , Cn4 , v060
	.byte	W12
	.byte		N11   , Cn4 , v100
	.byte	W12
	.byte		N01   , Cn4 , v060
	.byte	W06
	.byte		N11   , Cn4 , v100
	.byte	W12
	.byte		N01   , Cn4 , v060
	.byte	W06
	.byte		VOICE , 48
	.byte		PAN   , c_v+39
	.byte		VOL   , 51*mus_qz_610_mvl/mxv
	.byte		N44   , Cn3 , v100 , gtp3
	.byte	W48
	.byte		        Fn3 , v100 , gtp3
	.byte	W48
	.byte		        As3 , v100 , gtp3
	.byte	W48
	.byte		        Dn4 , v100 , gtp3
	.byte	W48
	.byte		        Cn3 , v100 , gtp3
	.byte	W48
	.byte		        Fn3 , v100 , gtp3
	.byte	W48
	.byte		        Fn3 , v100 , gtp3
	.byte	W48
	.byte		        An3 , v100 , gtp3
	.byte	W48
	.byte		        Fs3 , v100 , gtp3
	.byte	W48
	.byte		        Dn3 , v100 , gtp3
	.byte	W48
	.byte		        Dn3 , v100 , gtp3
	.byte	W48
	.byte		        Fn3 , v100 , gtp3
	.byte	W48
	.byte		        Cs3 , v100 , gtp3
	.byte	W48
	.byte		N23   , Ds3
	.byte	W24
	.byte		N68   , Fn3 , v100 , gtp3
	.byte	W24
	.byte	W48
	.byte		N44   , Gn2 , v100 , gtp3
	.byte	W48
	.byte		        An2 , v100 , gtp3
	.byte	W48
	.byte		        Cn3 , v100 , gtp3
	.byte	W48
	.byte		        Cs3 , v100 , gtp3
	.byte	W48
	.byte		        Dn3 , v100 , gtp3
	.byte	W48
	.byte		        Cn3 , v100 , gtp3
	.byte	W48
	.byte		        An2 , v100 , gtp3
	.byte	W48
	.byte		N23   , Cn4
	.byte	W24
	.byte		        An3
	.byte	W24
	.byte		        Fn3
	.byte	W24
	.byte		        An3
	.byte	W24
	.byte	N05
	.byte	W06
	.byte		        An3 , v020
	.byte	W06
	.byte		        An3 , v100
	.byte	W06
	.byte		        An3 , v020
	.byte	W30
	.byte		        En3 , v100
	.byte	W06
	.byte		        En3 , v020
	.byte	W06
	.byte		        En3 , v100
	.byte	W06
	.byte		        En3 , v020
	.byte	W30
	.byte		N23   , An3 , v100
	.byte	W24
	.byte		N11   , Fn3
	.byte	W12
	.byte		        An3
	.byte	W12
	.byte		N05   , An3 , v020
	.byte	W12
	.byte		N32   , Gn3 , v100 , gtp3
	.byte	W36
	.byte		N92   , En3 , v100 , gtp3
	.byte	W96
	.byte		        Fn3 , v100 , gtp3
	.byte	W96
	.byte		VOICE , 57
	.byte		N23   , En3
	.byte	W24
	.byte		N05   , En3 , v020
	.byte	W12
	.byte		N11   , En3 , v100
	.byte	W12
	.byte		N05   , En3 , v020
	.byte	W12
	.byte		        En3 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        En3 , v020
	.byte	W24
	.byte		N23   , Fn3 , v100
	.byte	W24
	.byte		N05   , Fn3 , v020
	.byte	W12
	.byte		N11   , Fn3 , v100
	.byte	W12
	.byte		N05   , Fn3 , v020
	.byte	W12
	.byte		        Fn3 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Fn3 , v020
	.byte	W24
	.byte	GOTO
	 ptrvalue	mus_qz_610_3_B1
	.byte	FINE

/********************** Track  4 **********************/

mus_qz_610_4:
	.byte		KEYSH , mus_qz_610_key+0
	.byte		VOICE , 56
	.byte		PAN   , c_v+0
	.byte		VOL   , 62*mus_qz_610_mvl/mxv
	.byte		N05   , Gn2 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Gn2 , v020
	.byte	W12
	.byte		        Gn2 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Gn2 , v020
	.byte	W06
	.byte		        Cn3 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Cn3 , v020
	.byte	W06
	.byte		        Gn2 , v100
	.byte	W06
	.byte		        Cn3
	.byte	W06
	.byte		        Fn3
	.byte	W06
	.byte		        Fn3 , v020
	.byte	W06
	.byte		N28   , Gn3 , v100 , gtp1
	.byte	W06
	.byte	W24
	.byte	N05
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Gn3 , v020
	.byte	W12
	.byte		        Gn2 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Gn2 , v020
	.byte	W36
mus_qz_610_4_B1:
	.byte		VOICE , 57
	.byte		PAN   , c_v+23
	.byte		VOL   , 76*mus_qz_610_mvl/mxv
	.byte	W24
	.byte		N05   , An2 , v100
	.byte	W06
	.byte		        An2 , v020
	.byte	W30
	.byte		N23   , An2 , v100
	.byte	W24
	.byte		N05   , An2 , v020
	.byte	W12
mus_qz_610_4_000:
	.byte	W24
	.byte		N05   , As2 , v100
	.byte	W06
	.byte		        As2 , v020
	.byte	W30
	.byte		N23   , As2 , v100
	.byte	W24
	.byte		N05   , As2 , v020
	.byte	W12
	.byte	PEND
	.byte	W24
	.byte		        An2 , v100
	.byte	W06
	.byte		        An2 , v020
	.byte	W30
	.byte		N23   , An2 , v100
	.byte	W24
	.byte		N05   , An2 , v020
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_610_4_000
	.byte	W24
	.byte		N11   , Cn3 , v100
	.byte	W12
	.byte		N23   , An2
	.byte	W24
	.byte	N11
	.byte	W12
	.byte		        Cn3
	.byte	W12
	.byte		        An2
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		        Fn2
	.byte	W12
	.byte		        Dn3
	.byte	W12
	.byte		N23   , An2
	.byte	W24
	.byte	N11
	.byte	W12
	.byte		        Dn3
	.byte	W12
	.byte		        Fn3
	.byte	W12
	.byte		VOICE , 56
	.byte		N11   , Gn3
	.byte	W12
	.byte		N05   , Gn3 , v020
	.byte	W06
	.byte		        Gn3 , v100
	.byte	W06
	.byte		        Gn3 , v020
	.byte	W24
	.byte		N11   , As3 , v100
	.byte	W12
	.byte		N05   , As3 , v020
	.byte	W06
	.byte		        As3 , v100
	.byte	W06
	.byte		        As3 , v020
	.byte	W24
	.byte		N11   , Fn3 , v100
	.byte	W12
	.byte		N05   , Fn3 , v020
	.byte	W12
	.byte		N11   , En3 , v100
	.byte	W12
	.byte		N05   , En3 , v020
	.byte	W24
	.byte		        Cn3 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        As2
	.byte	W06
	.byte		        Cn3
	.byte	W06
	.byte		        Cn3 , v020
	.byte	W12
	.byte	W12
	.byte		N11   , Fn2 , v100
	.byte	W12
	.byte		        As2
	.byte	W12
	.byte		        Dn3
	.byte	W12
	.byte		        Cn3
	.byte	W12
	.byte		        Gn2
	.byte	W12
	.byte		        As2
	.byte	W12
	.byte		        Cn3
	.byte	W12
	.byte	N23
	.byte	W24
	.byte		        Cn2
	.byte	W24
	.byte		N05   , Fn2
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Fn2 , v020
	.byte	W12
	.byte		        An2 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        An2 , v020
	.byte	W12
	.byte	W12
	.byte		        Cn3 , v100
	.byte	W06
	.byte		        Cn3 , v020
	.byte	W06
	.byte		        Cn3 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Cn3 , v020
	.byte	W06
	.byte		N11   , Cn3 , v100
	.byte	W12
	.byte		N05   , Cn3 , v020
	.byte	W06
	.byte		        Cn3 , v100
	.byte	W06
	.byte		        Cn3 , v020
	.byte	W06
	.byte		        Cn3 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Cn3 , v020
	.byte	W06
	.byte		        Cn3 , v100
	.byte	W06
	.byte		VOICE , 27
	.byte	W12
	.byte		N01   , Dn3 , v060
	.byte	W12
	.byte		N23   , Dn3 , v100
	.byte	W24
	.byte		N01   , Dn3 , v060
	.byte	W12
	.byte		N28   , En3 , v100 , gtp1
	.byte	W36
	.byte	W12
	.byte		N11   , Fn3
	.byte	W24
	.byte		N05   , Cn3
	.byte	W06
	.byte		N01   , Cn3 , v060
	.byte	W06
	.byte	N01
	.byte	W12
	.byte		N11   , Cn3 , v100
	.byte	W12
	.byte		N01   , Cn3 , v060
	.byte	W06
	.byte		N17   , Ds3 , v100
	.byte	W18
	.byte		N11   , An3
	.byte	W12
	.byte		N01   , An3 , v060
	.byte	W12
	.byte		N23   , Fn3 , v100
	.byte	W24
	.byte		N01   , Fn3 , v060
	.byte	W12
	.byte		N28   , En3 , v100 , gtp1
	.byte	W36
	.byte		N11   , An3
	.byte	W12
	.byte		N01   , An3 , v060
	.byte	W06
	.byte		N11   , An3 , v100
	.byte	W12
	.byte		N01   , An3 , v060
	.byte	W18
	.byte		N11   , Cn4 , v100
	.byte	W12
	.byte		N01   , Cn4 , v060
	.byte	W06
	.byte		N11   , Cn4 , v100
	.byte	W12
	.byte		N17   , An3
	.byte	W18
	.byte		N11   , Fn3
	.byte	W24
	.byte	N11
	.byte	W12
	.byte		N23   , Gn3
	.byte	W36
	.byte	N11
	.byte	W12
	.byte		        En3
	.byte	W12
	.byte		        An3
	.byte	W24
	.byte	N11
	.byte	W12
	.byte		N23   , Fn3
	.byte	W36
	.byte		N11   , Gn3
	.byte	W12
	.byte	N11
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		N05   , Gn2
	.byte	W06
	.byte		N01   , Gn2 , v060
	.byte	W06
	.byte		N11   , Gn2 , v100
	.byte	W12
	.byte		N05   , Cn3
	.byte	W06
	.byte		N01   , Cn3 , v060
	.byte	W06
	.byte		N11   , Cn3 , v100
	.byte	W12
	.byte		N05   , Gn2
	.byte	W06
	.byte		N01   , Gn2 , v060
	.byte	W06
	.byte		N05   , Cn3 , v100
	.byte	W06
	.byte		        Fn3
	.byte	W06
	.byte		N01   , Fn3 , v060
	.byte	W06
	.byte		N52   , Gn3 , v100 , gtp1
	.byte	W06
	.byte	W48
	.byte		N01   , Gn3 , v060
	.byte	W12
	.byte		N11   , Gn3 , v100
	.byte	W12
	.byte		N01   , Gn3 , v060
	.byte	W06
	.byte		N11   , Gn3 , v100
	.byte	W12
	.byte		N01   , Gn3 , v060
	.byte	W06
	.byte		VOICE , 48
	.byte		PAN   , c_v-40
	.byte		VOL   , 59*mus_qz_610_mvl/mxv
	.byte		N44   , An2 , v100 , gtp3
	.byte	W48
	.byte		        Cn3 , v100 , gtp3
	.byte	W48
	.byte		        Fn3 , v100 , gtp3
	.byte	W48
	.byte		        As3 , v100 , gtp3
	.byte	W48
	.byte		        An2 , v100 , gtp3
	.byte	W48
	.byte		        Cn3 , v100 , gtp3
	.byte	W48
	.byte		        Dn3 , v100 , gtp3
	.byte	W48
	.byte		        Fn3 , v100 , gtp3
	.byte	W48
	.byte		        Dn3 , v100 , gtp3
	.byte	W48
	.byte		        Cn3 , v100 , gtp3
	.byte	W48
	.byte		        As2 , v100 , gtp3
	.byte	W48
	.byte		        Dn3 , v100 , gtp3
	.byte	W48
	.byte		N92   , As2 , v100 , gtp3
	.byte	W96
	.byte		N44   , Fn2 , v100 , gtp3
	.byte	W48
	.byte		        Fn2 , v100 , gtp3
	.byte	W48
	.byte		        Gn2 , v100 , gtp3
	.byte	W48
	.byte		        An2 , v100 , gtp3
	.byte	W48
	.byte		        As2 , v100 , gtp3
	.byte	W48
	.byte		        As2 , v100 , gtp3
	.byte	W48
	.byte		        An2 , v100 , gtp3
	.byte	W48
	.byte		        Gn2 , v100 , gtp3
	.byte	W48
	.byte		N23   , An3
	.byte	W24
	.byte		        Fn3
	.byte	W24
	.byte		        Dn3
	.byte	W24
	.byte		        Fn3
	.byte	W24
	.byte	N05
	.byte	W06
	.byte		        Fn3 , v020
	.byte	W06
	.byte		        Fn3 , v100
	.byte	W06
	.byte		        Fn3 , v020
	.byte	W30
	.byte		        Cn3 , v100
	.byte	W06
	.byte		        Cn3 , v020
	.byte	W06
	.byte		        Cn3 , v100
	.byte	W06
	.byte		        Cn3 , v020
	.byte	W30
	.byte		N23   , Fn3 , v100
	.byte	W24
	.byte		N11   , Dn3
	.byte	W12
	.byte		        Fn3
	.byte	W12
	.byte		N05   , Fn3 , v020
	.byte	W12
	.byte		N32   , En3 , v100 , gtp3
	.byte	W36
	.byte		N92   , Cn3 , v100 , gtp3
	.byte	W96
	.byte		        Dn3 , v100 , gtp3
	.byte	W96
	.byte		VOICE , 57
	.byte		N23   , Cn3
	.byte	W24
	.byte		N05   , Cn3 , v020
	.byte	W12
	.byte		N11   , Cn3 , v100
	.byte	W12
	.byte		N05   , Cn3 , v020
	.byte	W12
	.byte		        Cn3 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Cn3 , v020
	.byte	W24
	.byte		N23   , Dn3 , v100
	.byte	W24
	.byte		N05   , Dn3 , v020
	.byte	W12
	.byte		N11   , Dn3 , v100
	.byte	W12
	.byte		N05   , Dn3 , v020
	.byte	W12
	.byte		        Dn3 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Dn3 , v020
	.byte	W24
	.byte	GOTO
	 ptrvalue	mus_qz_610_4_B1
	.byte	FINE

/********************** Track  5 **********************/

mus_qz_610_5:
	.byte		KEYSH , mus_qz_610_key+0
	.byte		VOICE , 56
	.byte		PAN   , c_v+0
	.byte		VOL   , 62*mus_qz_610_mvl/mxv
	.byte		N05   , Fn2 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Fn2 , v020
	.byte	W12
	.byte		        Fn2 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Fn2 , v020
	.byte	W06
	.byte		        Gn2 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Gn2 , v020
	.byte	W06
	.byte		        Fn2 , v100
	.byte	W06
	.byte		        Gn2
	.byte	W06
	.byte		        Cn3
	.byte	W06
	.byte		        Cn3 , v020
	.byte	W06
	.byte		N28   , Fn3 , v100 , gtp1
	.byte	W06
	.byte	W24
	.byte	N05
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Fn3 , v020
	.byte	W12
	.byte		        En3 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        En3 , v020
	.byte	W36
mus_qz_610_5_B1:
	.byte		VOICE , 27
	.byte		VOL   , 50*mus_qz_610_mvl/mxv
	.byte		PAN   , c_v-22
	.byte	W21
	.byte		N02   , Gs2 , v100
	.byte	W03
	.byte		N11   , An2
	.byte	W12
	.byte		N05   , Fn2
	.byte	W06
	.byte		        Fn2 , v020
	.byte	W18
	.byte		        Cn2 , v100
	.byte	W06
	.byte		        Cn2 , v020
	.byte	W06
	.byte		        An2 , v100
	.byte	W06
	.byte		        An2 , v020
	.byte	W06
	.byte		        Fn3 , v100
	.byte	W06
	.byte		        Fn3 , v020
	.byte	W06
	.byte		        Gn3 , v100
	.byte	W06
	.byte		        Gn3 , v020
	.byte	W06
	.byte		        Fn3 , v100
	.byte	W06
	.byte		        Fn3 , v020
	.byte	W06
	.byte		        As2 , v100
	.byte	W06
	.byte		        As2 , v020
	.byte	W06
	.byte		        Fn2 , v100
	.byte	W06
	.byte		        Fn2 , v020
	.byte	W18
	.byte		        Dn2 , v100
	.byte	W06
	.byte		        Dn2 , v020
	.byte	W06
	.byte		        As2 , v100
	.byte	W06
	.byte		        As2 , v020
	.byte	W06
	.byte		        Fn3 , v100
	.byte	W06
	.byte		        Fn3 , v020
	.byte	W06
	.byte		        Gn3 , v100
	.byte	W06
	.byte		        Gn3 , v020
	.byte	W06
	.byte		        Fn3 , v100
	.byte	W09
	.byte		N02   , Gs2
	.byte	W03
	.byte		N11   , An2
	.byte	W12
	.byte		N05   , Fn2
	.byte	W06
	.byte		        Fn2 , v020
	.byte	W18
	.byte		        Cn2 , v100
	.byte	W06
	.byte		        Cn2 , v020
	.byte	W06
	.byte		        An2 , v100
	.byte	W06
	.byte		        An2 , v020
	.byte	W06
	.byte		        Fn3 , v100
	.byte	W06
	.byte		        Fn3 , v020
	.byte	W06
	.byte		        Gn3 , v100
	.byte	W06
	.byte		        Gn3 , v020
	.byte	W06
	.byte		        Fn3 , v100
	.byte	W06
	.byte		        Fn3 , v020
	.byte	W06
	.byte		        As3 , v100
	.byte	W06
	.byte		        As3 , v020
	.byte	W06
	.byte		        Cs4 , v100
	.byte	W06
	.byte		        Cs4 , v020
	.byte	W06
	.byte		        Ds4 , v100
	.byte	W06
	.byte		        Ds4 , v020
	.byte	W06
	.byte		        As3 , v100
	.byte	W06
	.byte		        As3 , v020
	.byte	W06
	.byte		        Cs4 , v100
	.byte	W06
	.byte		        Cs4 , v020
	.byte	W06
	.byte		N44   , Cn4 , v100 , gtp3
	.byte	W12
	.byte	W36
	.byte		N05   , Fn3
	.byte	W06
	.byte		        Gn3
	.byte	W06
	.byte		        An3
	.byte	W06
	.byte		        An3 , v020
	.byte	W06
	.byte		        Gn3 , v100
	.byte	W06
	.byte		        Gn3 , v020
	.byte	W06
	.byte		        Fn3 , v100
	.byte	W06
	.byte		        Fn3 , v020
	.byte	W06
	.byte		        Cn3 , v100
	.byte	W06
	.byte		        Cn3 , v020
	.byte	W06
	.byte		        Fn3 , v100
	.byte	W06
	.byte		        Fn3 , v020
	.byte	W06
	.byte		        En3 , v100
	.byte	W06
	.byte		        En3 , v020
	.byte	W06
	.byte		        Dn3 , v100
	.byte	W06
	.byte		        Dn3 , v020
	.byte	W06
	.byte		N32   , An2 , v100 , gtp3
	.byte	W60
	.byte		VOICE , 56
	.byte		PAN   , c_v+23
	.byte		VOL   , 76*mus_qz_610_mvl/mxv
	.byte		N11   , Fn3
	.byte	W12
	.byte		N05   , Fn3 , v020
	.byte	W06
	.byte		        Fn3 , v100
	.byte	W06
	.byte		        Fn3 , v020
	.byte	W24
	.byte		N11   , Gn3 , v100
	.byte	W12
	.byte		N05   , Gn3 , v020
	.byte	W06
	.byte		        Gn3 , v100
	.byte	W06
	.byte		        Gn3 , v020
	.byte	W24
	.byte		N11   , Dn3 , v100
	.byte	W12
	.byte		N05   , Dn3 , v020
	.byte	W12
	.byte		N11   , Cn3 , v100
	.byte	W12
	.byte		N05   , Cn3 , v020
	.byte	W24
	.byte		        An2 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Gn2
	.byte	W06
	.byte		        An2
	.byte	W06
	.byte		        An2 , v020
	.byte	W12
	.byte		VOICE , 27
	.byte		PAN   , c_v-22
	.byte		VOL   , 50*mus_qz_610_mvl/mxv
	.byte	W12
	.byte		N11   , As3 , v100
	.byte	W12
	.byte		        Dn4
	.byte	W12
	.byte		        Fn4
	.byte	W12
	.byte		        En4
	.byte	W12
	.byte		        Gn3
	.byte	W12
	.byte		N05   , Cn4
	.byte	W06
	.byte		        As3
	.byte	W06
	.byte		        An3
	.byte	W06
	.byte		        Gn3
	.byte	W06
	.byte		N11   , An3
	.byte	W12
	.byte		        As3
	.byte	W12
	.byte		        An3
	.byte	W12
	.byte		N23   , Fn3
	.byte	W24
	.byte		        Cn3
	.byte	W24
	.byte		N11   , Gn3
	.byte	W12
	.byte		        An3
	.byte	W12
	.byte		VOICE , 56
	.byte		PAN   , c_v+23
	.byte		VOL   , 76*mus_qz_610_mvl/mxv
	.byte		N05   , An2
	.byte	W06
	.byte		        An2 , v020
	.byte	W06
	.byte		        An2 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        An2 , v020
	.byte	W06
	.byte		N11   , An2 , v100
	.byte	W12
	.byte		N05   , An2 , v020
	.byte	W06
	.byte		        An2 , v100
	.byte	W06
	.byte		        An2 , v020
	.byte	W06
	.byte		        An2 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        An2 , v020
	.byte	W06
	.byte		        An2 , v100
	.byte	W06
	.byte		VOICE , 27
	.byte	W12
	.byte		N01   , As2 , v060
	.byte	W12
	.byte		N23   , As2 , v100
	.byte	W24
	.byte		N01   , As2 , v060
	.byte	W12
	.byte		N28   , Cn3 , v100 , gtp1
	.byte	W36
	.byte	W12
	.byte	N11
	.byte	W24
	.byte		N05   , An2
	.byte	W06
	.byte		N01   , An2 , v060
	.byte	W06
	.byte	N01
	.byte	W12
	.byte		N11   , An2 , v100
	.byte	W12
	.byte		N01   , An2 , v060
	.byte	W06
	.byte		N17   , Cn3 , v100
	.byte	W18
	.byte		N11   , Fn3
	.byte	W12
	.byte		N01   , Fn3 , v060
	.byte	W12
	.byte		N23   , Dn3 , v100
	.byte	W24
	.byte		N01   , Dn3 , v060
	.byte	W12
	.byte		N28   , Cn3 , v100 , gtp1
	.byte	W36
	.byte		N11   , Fn3
	.byte	W12
	.byte		N01   , Fn3 , v060
	.byte	W06
	.byte		N11   , Fn3 , v100
	.byte	W12
	.byte		N01   , Fn3 , v060
	.byte	W06
	.byte		N05   , Fn3 , v100
	.byte	W06
	.byte		        Gn3
	.byte	W06
	.byte		N11   , An3
	.byte	W12
	.byte		N01   , An3 , v060
	.byte	W06
	.byte		N11   , An3 , v100
	.byte	W12
	.byte		N17   , Fn3
	.byte	W18
	.byte		N11   , Dn3
	.byte	W24
	.byte	N11
	.byte	W12
	.byte		N23   , En3
	.byte	W36
	.byte	N11
	.byte	W12
	.byte		        Cs3
	.byte	W12
	.byte		        Fn3
	.byte	W24
	.byte	N11
	.byte	W12
	.byte		N23   , Dn3
	.byte	W36
	.byte		N11   , Fn3
	.byte	W12
	.byte	N11
	.byte	W12
	.byte	N11
	.byte	W90
	.byte		N52   , Fn3 , v100 , gtp1
	.byte	W06
	.byte	W48
	.byte		N01   , Fn3 , v060
	.byte	W12
	.byte		N11   , En3 , v100
	.byte	W12
	.byte		N01   , En3 , v060
	.byte	W06
	.byte		N11   , En3 , v100
	.byte	W12
	.byte		N01   , En3 , v060
	.byte	W06
	.byte		VOICE , 48
	.byte		PAN   , c_v-40
	.byte		VOL   , 49*mus_qz_610_mvl/mxv
	.byte		N44   , Gn2 , v100 , gtp3
	.byte	W48
	.byte		        An2 , v100 , gtp3
	.byte	W48
	.byte		        Dn3 , v100 , gtp3
	.byte	W48
	.byte		        Fn3 , v100 , gtp3
	.byte	W48
	.byte		        Gn2 , v100 , gtp3
	.byte	W48
	.byte		        An2 , v100 , gtp3
	.byte	W48
	.byte		        As2 , v100 , gtp3
	.byte	W48
	.byte		        Dn3 , v100 , gtp3
	.byte	W48
	.byte		        Cn3 , v100 , gtp3
	.byte	W48
	.byte		        An2 , v100 , gtp3
	.byte	W48
	.byte		        Gn2 , v100 , gtp3
	.byte	W48
	.byte		        As2 , v100 , gtp3
	.byte	W48
	.byte		N92   , Gn2 , v100 , gtp3
	.byte	W96
	.byte		N44   , Cn2 , v100 , gtp3
	.byte	W48
	.byte		        Cn2 , v100 , gtp3
	.byte	W48
	.byte		        Cn2 , v100 , gtp3
	.byte	W48
	.byte		        Gn2 , v100 , gtp3
	.byte	W48
	.byte		        Fn2 , v100 , gtp3
	.byte	W48
	.byte		        Fn2 , v100 , gtp3
	.byte	W48
	.byte		        Gn2 , v100 , gtp3
	.byte	W48
	.byte		        Cn2 , v100 , gtp3
	.byte	W48
	.byte		N23   , Fn3
	.byte	W24
	.byte		        Dn3
	.byte	W24
	.byte		        As2
	.byte	W24
	.byte		        Dn3
	.byte	W24
	.byte	N05
	.byte	W06
	.byte		        Dn3 , v020
	.byte	W06
	.byte		        Dn3 , v100
	.byte	W06
	.byte		        Dn3 , v020
	.byte	W30
	.byte		        An2 , v100
	.byte	W06
	.byte		        An2 , v020
	.byte	W06
	.byte		        An2 , v100
	.byte	W06
	.byte		        An2 , v020
	.byte	W30
	.byte		N23   , Dn3 , v100
	.byte	W24
	.byte		N11   , As2
	.byte	W12
	.byte		        Dn3
	.byte	W12
	.byte		N05   , Dn3 , v020
	.byte	W12
	.byte		N32   , As2 , v100 , gtp3
	.byte	W36
	.byte		N92   , An2 , v100 , gtp3
	.byte	W96
	.byte		        As2 , v100 , gtp3
	.byte	W96
	.byte		VOICE , 57
	.byte		N23   , An2
	.byte	W24
	.byte		N05   , An2 , v020
	.byte	W12
	.byte		N11   , An2 , v100
	.byte	W12
	.byte		N05   , An2 , v020
	.byte	W12
	.byte		        An2 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        An2 , v020
	.byte	W24
	.byte		N23   , As2 , v100
	.byte	W24
	.byte		N05   , As2 , v020
	.byte	W12
	.byte		N11   , As2 , v100
	.byte	W12
	.byte		N05   , As2 , v020
	.byte	W12
	.byte		        As2 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        As2 , v020
	.byte	W24
	.byte	GOTO
	 ptrvalue	mus_qz_610_5_B1
	.byte	FINE

/********************** Track  6 **********************/

mus_qz_610_6:
	.byte		KEYSH , mus_qz_610_key+0
	.byte		VOICE , 14
	.byte		PAN   , c_v+0
	.byte		VOL   , 62*mus_qz_610_mvl/mxv
	.byte		PAN   , c_v-49
	.byte		N23   , Gn2 , v100
	.byte	W24
	.byte		        Cn3
	.byte	W24
	.byte		        Fn3
	.byte	W24
	.byte		        Gn3
	.byte	W24
	.byte		N44   , Cn4 , v100 , gtp3
	.byte	W48
	.byte		        Gn3 , v100 , gtp3
	.byte	W48
mus_qz_610_6_B1:
	.byte		VOICE , 17
	.byte		PAN   , c_v+41
	.byte	W36
	.byte		N05   , Fn3 , v100
	.byte	W06
	.byte		        Fn3 , v020
	.byte	W06
	.byte		        En3 , v100
	.byte	W06
	.byte		        En3 , v020
	.byte	W06
	.byte		        Fn3 , v100
	.byte	W06
	.byte		        Fn3 , v020
	.byte	W18
	.byte		        Cn4 , v100
	.byte	W06
	.byte		        Cn4 , v020
	.byte	W06
	.byte	W36
	.byte		        Fn3 , v100
	.byte	W06
	.byte		        Fn3 , v020
	.byte	W06
	.byte		        En3 , v100
	.byte	W06
	.byte		        En3 , v020
	.byte	W06
	.byte		        Fn3 , v100
	.byte	W06
	.byte		        Fn3 , v020
	.byte	W18
	.byte		        Cn3 , v100
	.byte	W06
	.byte		        Cn3 , v020
	.byte	W06
	.byte	W12
	.byte		        Fn3 , v100
	.byte	W06
	.byte		        Fn3 , v020
	.byte	W06
	.byte		        En3 , v100
	.byte	W06
	.byte		        En3 , v020
	.byte	W06
	.byte		        Fn3 , v100
	.byte	W06
	.byte		        Fn3 , v020
	.byte	W18
	.byte		        Cn4 , v100
	.byte	W06
	.byte		        Cn4 , v020
	.byte	W06
	.byte		        Cn3 , v100
	.byte	W06
	.byte		        Cn3 , v020
	.byte	W06
	.byte		        Fn3 , v100
	.byte	W06
	.byte		        Fn3 , v020
	.byte	W06
	.byte		        Gn3 , v100
	.byte	W06
	.byte		        Gn3 , v020
	.byte	W06
	.byte		        Fn3 , v100
	.byte	W06
	.byte		        Fn3 , v020
	.byte	W06
	.byte		        As3 , v100
	.byte	W06
	.byte		        As3 , v020
	.byte	W06
	.byte		        Cs4 , v100
	.byte	W06
	.byte		        Cs4 , v020
	.byte	W18
	.byte		        As3 , v100
	.byte	W06
	.byte		        As3 , v020
	.byte	W06
	.byte		        Cs4 , v100
	.byte	W06
	.byte		        Cs4 , v020
	.byte	W06
	.byte		        Cn4 , v100
	.byte	W06
	.byte		        Cn4 , v020
	.byte	W06
	.byte	W12
	.byte		        Fn3 , v100
	.byte	W06
	.byte		        Fn3 , v020
	.byte	W06
	.byte		        Cn4 , v100
	.byte	W06
	.byte		        Cn4 , v020
	.byte	W06
	.byte		        Fn3 , v100
	.byte	W06
	.byte		        Fn3 , v020
	.byte	W06
	.byte		        Cn3 , v100
	.byte	W06
	.byte		        Cn3 , v020
	.byte	W06
	.byte		        Fn3 , v100
	.byte	W06
	.byte		        Fn3 , v020
	.byte	W06
	.byte		        En3 , v100
	.byte	W06
	.byte		        En3 , v020
	.byte	W06
	.byte		        Fn3 , v100
	.byte	W06
	.byte		        Fn3 , v020
	.byte	W06
	.byte		        An3 , v100
	.byte	W06
	.byte		        An3 , v020
	.byte	W06
	.byte		        En3 , v100
	.byte	W06
	.byte		        En3 , v020
	.byte	W06
	.byte		        An3 , v100
	.byte	W06
	.byte		        An3 , v020
	.byte	W06
	.byte		        Dn4 , v100
	.byte	W06
	.byte		        Dn4 , v020
	.byte	W18
	.byte		        An3 , v100
	.byte	W06
	.byte		        An3 , v020
	.byte	W18
	.byte		        Dn4 , v100
	.byte	W06
	.byte		        Fn4
	.byte	W06
	.byte		        Dn4
	.byte	W06
	.byte		        Dn4 , v020
	.byte	W06
	.byte		        As3 , v100
	.byte	W06
	.byte		        As3 , v020
	.byte	W06
	.byte		        Fn4 , v100
	.byte	W06
	.byte		        Fn4 , v020
	.byte	W06
	.byte		        As4 , v100
	.byte	W06
	.byte		        As4 , v020
	.byte	W06
	.byte		        En4 , v100
	.byte	W06
	.byte		        En4 , v020
	.byte	W06
	.byte		        As3 , v100
	.byte	W06
	.byte		        As3 , v020
	.byte	W06
	.byte		        Gn4 , v100
	.byte	W06
	.byte		        Gn4 , v020
	.byte	W06
	.byte		        As4 , v100
	.byte	W06
	.byte		        As4 , v020
	.byte	W06
	.byte		VOICE , 14
	.byte		PAN   , c_v-40
	.byte		VOL   , 62*mus_qz_610_mvl/mxv
	.byte		N23   , An4 , v100
	.byte	W24
	.byte		PAN   , c_v-24
	.byte		N23   , Gn4
	.byte	W24
	.byte		PAN   , c_v+23
	.byte		N23   , Fn4
	.byte	W24
	.byte		PAN   , c_v+40
	.byte		N23   , An3
	.byte	W24
	.byte		VOICE , 17
	.byte		VOL   , 46*mus_qz_610_mvl/mxv
	.byte		N05   , Dn3
	.byte	W06
	.byte		        Dn3 , v020
	.byte	W06
	.byte		        Fn2 , v100
	.byte	W06
	.byte		        Fn2 , v020
	.byte	W06
	.byte		        As2 , v100
	.byte	W06
	.byte		        As2 , v020
	.byte	W06
	.byte		N11   , Cn3 , v100
	.byte	W12
	.byte		N05   , Cn3 , v020
	.byte	W12
	.byte		        En3 , v100
	.byte	W06
	.byte		        Ds3
	.byte	W06
	.byte		        En3
	.byte	W06
	.byte		        Fn3
	.byte	W06
	.byte		        Gn3
	.byte	W06
	.byte		N17   , An3
	.byte	W06
	.byte	W12
	.byte		N11   , Gs3
	.byte	W12
	.byte		        An3
	.byte	W12
	.byte		N05   , En3
	.byte	W06
	.byte		N11   , Fn3
	.byte	W12
	.byte		N17   , Cn3
	.byte	W30
	.byte		N05   , As2
	.byte	W06
	.byte		        Gs2
	.byte	W06
	.byte		N11   , An2
	.byte	W12
	.byte		N23   , Fn3
	.byte	W24
	.byte		N05   , Cn3
	.byte	W06
	.byte		N11   , Fn3
	.byte	W12
	.byte	N05
	.byte	W06
	.byte		        Cn4
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Gn4
	.byte	W06
	.byte		        An4
	.byte	W06
	.byte		        Cn5
	.byte	W06
	.byte		        Fn5
	.byte	W06
	.byte		PAN   , c_v+0
	.byte		VOL   , 46*mus_qz_610_mvl/mxv
	.byte	N11
	.byte	W12
	.byte		N05   , Fn5 , v020
	.byte	W12
	.byte		N11   , Cn5 , v100
	.byte	W12
	.byte		N05   , Cn5 , v020
	.byte	W12
	.byte		N11   , Gn4 , v100
	.byte	W12
	.byte		N23   , Cn5
	.byte	W24
	.byte		N05   , An4
	.byte	W06
	.byte		        As4
	.byte	W06
	.byte		N23   , Cn5
	.byte	W24
	.byte		N05   , Dn5
	.byte	W06
	.byte		        Cs5
	.byte	W06
	.byte		N11   , Cn5
	.byte	W12
	.byte		N05   , Cn5 , v020
	.byte	W12
	.byte		N11   , An4 , v100
	.byte	W12
	.byte		N05   , Gn5
	.byte	W06
	.byte		        Fs5
	.byte	W06
	.byte		N11   , Fn5
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		N05   , Fn5 , v020
	.byte	W12
	.byte		N11   , Cn5 , v100
	.byte	W12
	.byte		N05   , Cn5 , v020
	.byte	W12
	.byte		N11   , Gn4 , v100
	.byte	W12
	.byte		N23   , Cn5
	.byte	W24
	.byte		N05   , Dn5
	.byte	W06
	.byte		        Cs5
	.byte	W06
	.byte		N11   , Cn5
	.byte	W12
	.byte		        Dn5
	.byte	W12
	.byte		        Cn5
	.byte	W12
	.byte		        As4
	.byte	W12
	.byte		N05   , As4 , v020
	.byte	W12
	.byte		N23   , An4 , v100
	.byte	W24
	.byte		N05   , Gs4
	.byte	W06
	.byte		        An4
	.byte	W06
	.byte		N23   , Fn5
	.byte	W24
	.byte		N11   , Cn5
	.byte	W12
	.byte		N23   , Gn4
	.byte	W24
	.byte		N11   , En4
	.byte	W12
	.byte		        As4
	.byte	W12
	.byte		N05   , An4
	.byte	W06
	.byte		        Gn4
	.byte	W06
	.byte		        Fn4
	.byte	W06
	.byte		        Fn4 , v020
	.byte	W06
	.byte		        En4 , v100
	.byte	W06
	.byte		        En4 , v020
	.byte	W06
	.byte		        Fn4 , v100
	.byte	W06
	.byte		        Fn4 , v020
	.byte	W06
	.byte		        Gn4 , v100
	.byte	W06
	.byte		        Gn4 , v020
	.byte	W18
	.byte		        Bn4 , v100
	.byte	W06
	.byte		        Bn4 , v020
	.byte	W18
	.byte		        An4 , v100
	.byte	W06
	.byte		        Bn4
	.byte	W06
	.byte		N11   , Fn4
	.byte	W12
	.byte		        En4
	.byte	W12
	.byte	N05
	.byte	W06
	.byte		        Fn4
	.byte	W12
	.byte		N17   , Cn4
	.byte	W18
	.byte		N11   , Gn3
	.byte	W12
	.byte		N05   , Fn3
	.byte	W06
	.byte		        Gn3
	.byte	W06
	.byte		        Gn3 , v020
	.byte	W06
	.byte		N11   , Cn3 , v100
	.byte	W06
	.byte	W06
	.byte		N05   , Fn3
	.byte	W06
	.byte		        Gn3
	.byte	W06
	.byte		        Fn3
	.byte	W06
	.byte		        Gn3
	.byte	W06
	.byte		        Cn4
	.byte	W06
	.byte		        Dn4
	.byte	W06
	.byte		        Cs4
	.byte	W06
	.byte		        Cn4
	.byte	W06
	.byte		        En4
	.byte	W06
	.byte		        Gn4
	.byte	W06
	.byte		        As4
	.byte	W06
	.byte		        Cn5
	.byte	W06
	.byte		        En5
	.byte	W06
	.byte		        As5
	.byte	W06
	.byte		        Cn6
	.byte	W06
	.byte		VOICE , 60
	.byte		PAN   , c_v+20
	.byte		VOL   , 47*mus_qz_610_mvl/mxv
	.byte	W24
	.byte		N23   , An2
	.byte	W24
	.byte		        Cn3
	.byte	W24
	.byte		        Fn3
	.byte	W24
	.byte		        Gn3
	.byte	W24
	.byte		        Fn3
	.byte	W24
	.byte		        Dn3
	.byte	W24
	.byte		        As3
	.byte	W24
	.byte		        An3
	.byte	W24
	.byte		N05   , An3 , v020
	.byte	W12
	.byte		N11   , Cn3 , v100
	.byte	W12
	.byte	N23
	.byte	W24
	.byte		N11   , An2
	.byte	W12
	.byte		        Dn3
	.byte	W12
	.byte		N05   , Dn3 , v020
	.byte	W12
	.byte		N11   , Dn3 , v100
	.byte	W12
	.byte		N23   , Cn3
	.byte	W24
	.byte		        Fn3
	.byte	W24
	.byte		        Gn3
	.byte	W24
	.byte		        An3
	.byte	W24
	.byte		        Dn3
	.byte	W24
	.byte		        An3
	.byte	W24
	.byte		N11   , As3
	.byte	W12
	.byte		        An3
	.byte	W12
	.byte		N05   , An3 , v020
	.byte	W12
	.byte		N11   , An3 , v100
	.byte	W12
	.byte		N23   , Gn3
	.byte	W24
	.byte		        Fn3
	.byte	W24
	.byte		        Gn3
	.byte	W24
	.byte		VOICE , 14
	.byte		PAN   , c_v-40
	.byte		VOL   , 62*mus_qz_610_mvl/mxv
	.byte		N23   , Fn4
	.byte	W24
	.byte		PAN   , c_v-25
	.byte		N23   , Cs4
	.byte	W24
	.byte		PAN   , c_v+24
	.byte		N23   , Cn4
	.byte	W24
	.byte		PAN   , c_v+45
	.byte		N23   , As3
	.byte	W24
	.byte		PAN   , c_v-44
	.byte		N23   , Cn4
	.byte	W24
	.byte		PAN   , c_v+39
	.byte		N23   , Fn4
	.byte	W24
	.byte		PAN   , c_v-43
	.byte		N23   , Gn4
	.byte	W24
	.byte		PAN   , c_v+40
	.byte		N44   , Fn4 , v100 , gtp3
	.byte	W24
	.byte	W24
	.byte		VOICE , 17
	.byte		PAN   , c_v+41
	.byte		VOL   , 46*mus_qz_610_mvl/mxv
	.byte		N11   , Cn3
	.byte	W12
	.byte		        Fn3
	.byte	W12
	.byte		N05   , Fn3 , v020
	.byte	W12
	.byte		N11   , Cn3 , v100
	.byte	W12
	.byte		N05   , An3
	.byte	W06
	.byte		        Gn3
	.byte	W06
	.byte		N11   , Fn3
	.byte	W12
	.byte		N05   , Fn3 , v020
	.byte	W12
	.byte		N11   , As2 , v100
	.byte	W12
	.byte		        Cs3
	.byte	W12
	.byte		        Fn3
	.byte	W12
	.byte		N05   , An3
	.byte	W06
	.byte		        Gn3
	.byte	W06
	.byte		N11   , Fn3
	.byte	W12
	.byte		        As3
	.byte	W12
	.byte		        Cn4
	.byte	W12
	.byte		        Fn3
	.byte	W12
	.byte		        Cn3
	.byte	W12
	.byte		N05   , An3
	.byte	W06
	.byte		        As3
	.byte	W06
	.byte		N23   , Cn4
	.byte	W24
	.byte		N11   , Fn3
	.byte	W12
	.byte		N05   , An3
	.byte	W06
	.byte		        As3
	.byte	W06
	.byte		N11   , Cn4
	.byte	W12
	.byte		        Fn4
	.byte	W12
	.byte		        Cn4
	.byte	W12
	.byte		        Fn4
	.byte	W12
	.byte		N23   , Gn4
	.byte	W24
	.byte	N11
	.byte	W12
	.byte		        An4
	.byte	W12
	.byte		        As4
	.byte	W12
	.byte		VOICE , 14
	.byte		PAN   , c_v-26
	.byte		VOL   , 62*mus_qz_610_mvl/mxv
	.byte		N23   , Cn4
	.byte	W24
	.byte		PAN   , c_v+39
	.byte		N23   , Fn3
	.byte	W24
	.byte		PAN   , c_v-27
	.byte		N23   , Gn3
	.byte	W24
	.byte		PAN   , c_v+36
	.byte		N23   , Cn3
	.byte	W24
	.byte		        Fn3
	.byte	W24
	.byte		        An3
	.byte	W24
	.byte		        Cn4
	.byte	W24
	.byte	N23
	.byte	W24
	.byte		N92   , An3 , v100 , gtp3
	.byte	W96
	.byte		        As3 , v100 , gtp3
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_610_6_B1
	.byte	FINE

/********************** Track  7 **********************/

mus_qz_610_7:
	.byte		KEYSH , mus_qz_610_key+0
	.byte		VOICE , 32
	.byte		PAN   , c_v+0
	.byte		VOL   , 73*mus_qz_610_mvl/mxv
	.byte		N11   , Cn1 , v127
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		N05   , Cn1 , v020
	.byte	W12
	.byte		N11   , Cn1 , v127
	.byte	W12
	.byte		N05   , Cn1 , v020
	.byte	W12
	.byte		N11   , Cn1 , v127
	.byte	W12
	.byte		N05   , Cn1 , v020
	.byte	W12
	.byte		N11   , Cn1 , v127
	.byte	W12
	.byte		        Gn1
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		        Cn1
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		N05   , Cn2
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		N11   , Cn1
	.byte	W12
	.byte		N05   , Cn1 , v020
	.byte	W12
	.byte		N11   , Cn1 , v127
	.byte	W12
mus_qz_610_7_000:
	.byte		N11   , Fn1 , v127
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		N05   , Fn1 , v020
	.byte	W72
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_610_7_000
	.byte	PATT
	 ptrvalue	mus_qz_610_7_000
mus_qz_610_7_001:
	.byte		N11   , Fn1 , v127
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		N05   , Fn1 , v020
	.byte	W36
	.byte		N11   , Fn1 , v127
	.byte	W12
	.byte		N05   , Fn1 , v020
	.byte	W12
	.byte		N11   , Fn1 , v127
	.byte	W12
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_610_7_001
	.byte		N11   , Dn1 , v127
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		N05   , Dn1 , v020
	.byte	W12
	.byte		N11   , Dn1 , v127
	.byte	W12
	.byte		N05   , Dn1 , v020
	.byte	W12
	.byte		N11   , Dn1 , v127
	.byte	W12
	.byte		N05   , Dn1 , v020
	.byte	W12
	.byte		N11   , Dn1 , v127
	.byte	W12
	.byte		        Gn1
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		N05   , Gn1 , v020
	.byte	W12
	.byte		N11   , Gn1 , v127
	.byte	W12
	.byte		        Cn1
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		N05   , Cn1 , v020
	.byte	W12
	.byte		N11   , Cn1 , v127
	.byte	W12
	.byte		        Dn1
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		N05   , Dn1 , v020
	.byte	W12
	.byte		N11   , Dn1 , v127
	.byte	W12
	.byte		        Cn1
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		N05   , Cn1 , v020
	.byte	W12
	.byte		N11   , An0 , v127
	.byte	W12
	.byte		        As0
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		N05   , As0 , v020
	.byte	W12
	.byte		N11   , As0 , v127
	.byte	W12
	.byte		        Cn1
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		N05   , Cn1 , v020
	.byte	W12
	.byte		N11   , Cn1 , v127
	.byte	W12
	.byte		        Fn1
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		N05   , Fn1 , v020
	.byte	W12
	.byte		N11   , Cn1 , v127
	.byte	W12
	.byte		N05   , Cn1 , v020
	.byte	W12
	.byte		N11   , Fn1 , v127
	.byte	W12
	.byte		        Cn2
	.byte	W12
	.byte		        Cn1
	.byte	W12
	.byte		N05   , Cn1 , v020
	.byte	W12
	.byte		        Fn1 , v127
	.byte	W06
	.byte		        Fn1 , v020
	.byte	W06
	.byte		        Fn1 , v127
	.byte	W06
	.byte		        Fn1 , v020
	.byte	W06
	.byte		        Fn1 , v127
	.byte	W06
	.byte		        Fn1 , v020
	.byte	W06
	.byte		N11   , Fn1 , v127
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		N05   , Gn1
	.byte	W06
	.byte		        Fn1
	.byte	W06
	.byte		        Cn1
	.byte	W06
	.byte		        Fn1
	.byte	W06
mus_qz_610_7_002:
	.byte		N11   , As1 , v127
	.byte	W12
	.byte		N05   , As1 , v020
	.byte	W12
	.byte		N17   , As0 , v127
	.byte	W18
	.byte	N05
	.byte	W06
	.byte		N11   , Cn1
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		N05   , Gn1
	.byte	W06
	.byte		N11   , Cn1
	.byte	W12
	.byte	N05
	.byte	W06
	.byte	PEND
	.byte		N23   , Fn1
	.byte	W24
	.byte		N05   , Fn1 , v020
	.byte	W06
	.byte		N11   , Fn1 , v127
	.byte	W12
	.byte		N05   , Cn1
	.byte	W06
	.byte		        Cn1 , v020
	.byte	W12
	.byte		N11   , Cn1 , v127
	.byte	W12
	.byte		N05   , Fn1
	.byte	W06
	.byte		N11   , Cn1
	.byte	W12
	.byte		N05   , An1
	.byte	W06
	.byte	PATT
	 ptrvalue	mus_qz_610_7_002
	.byte		N17   , Dn1 , v127
	.byte	W18
	.byte	N05
	.byte	W06
	.byte		        Dn1 , v020
	.byte	W12
	.byte		N11   , Dn1 , v127
	.byte	W12
	.byte		N05   , Dn1 , v020
	.byte	W06
	.byte		N11   , Fn1 , v127
	.byte	W12
	.byte	N05
	.byte	W06
	.byte		        An1
	.byte	W06
	.byte		N11   , Cn1
	.byte	W12
	.byte		N05   , Fn1
	.byte	W06
	.byte		N17   , As0
	.byte	W18
	.byte	N05
	.byte	W06
	.byte		        As0 , v020
	.byte	W12
	.byte		N11   , As0 , v127
	.byte	W12
	.byte		        An0
	.byte	W12
	.byte		N05   , Cs1
	.byte	W06
	.byte		N11   , En1
	.byte	W12
	.byte		        An1
	.byte	W12
	.byte		N05   , Gn1
	.byte	W06
	.byte		N23   , Dn1
	.byte	W24
	.byte		N05   , Dn1 , v020
	.byte	W12
	.byte		N11   , Dn1 , v127
	.byte	W12
	.byte		        Gn1
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		N05   , Bn1
	.byte	W06
	.byte		N11   , Dn2
	.byte	W12
	.byte		N05   , Gn1
	.byte	W06
	.byte		N11   , Cn2
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		N05   , Cn2 , v020
	.byte	W12
	.byte		        Cn2 , v127
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Cn2 , v020
	.byte	W12
	.byte		N11   , Cn2 , v127
	.byte	W12
	.byte	N05
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Cn2 , v020
	.byte	W06
	.byte		        Cn2 , v127
	.byte	W06
	.byte		        Cn1
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Cn1 , v020
	.byte	W24
	.byte		        Cn1 , v127
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Cn1 , v020
	.byte	W12
	.byte		N11   , Cn1 , v127
	.byte	W12
	.byte		N05   , Cn1 , v020
	.byte	W06
	.byte		N11   , Cn1 , v127
	.byte	W12
	.byte	N05
	.byte	W06
mus_qz_610_7_003:
	.byte		N23   , Fn1 , v127
	.byte	W24
	.byte		N05   , Fn1 , v020
	.byte	W12
	.byte		N11   , Fn1 , v127
	.byte	W12
	.byte		N05   , Fn1 , v020
	.byte	W12
	.byte		N11   , Fn1 , v127
	.byte	W12
	.byte		N05   , Fn1 , v020
	.byte	W12
	.byte		N11   , Fn1 , v127
	.byte	W12
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_610_7_003
	.byte	PATT
	 ptrvalue	mus_qz_610_7_003
	.byte		N05   , Fn1 , v020
	.byte	W12
	.byte		N11   , Fn1 , v127
	.byte	W12
	.byte	N23
	.byte	W24
	.byte		N05   , Fn1 , v020
	.byte	W12
	.byte		N11   , Fn1 , v127
	.byte	W12
	.byte		N05   , Fn1 , v020
	.byte	W12
	.byte		N11   , Fn1 , v127
	.byte	W12
	.byte		N23   , Dn1
	.byte	W24
	.byte		N05   , Dn1 , v020
	.byte	W12
	.byte		N11   , Dn1 , v127
	.byte	W12
	.byte		N05   , Dn1 , v020
	.byte	W12
	.byte		N11   , Fs1 , v127
	.byte	W12
	.byte		N05   , Fs1 , v020
	.byte	W12
	.byte		N11   , Fs1 , v127
	.byte	W12
	.byte		N23   , Gn1
	.byte	W24
	.byte		N05   , Gn1 , v020
	.byte	W12
	.byte		N11   , Gn1 , v127
	.byte	W12
	.byte		N05   , Gn1 , v020
	.byte	W12
	.byte		N11   , Gn1 , v127
	.byte	W12
	.byte		N05   , Gn1 , v020
	.byte	W12
	.byte		N11   , Dn1 , v127
	.byte	W12
	.byte		        Ds1
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		        Gn1
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		        As1
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		        Cs2
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		N32   , Cn2 , v127 , gtp3
	.byte	W36
	.byte		N23   , Cn1
	.byte	W24
	.byte	N23
	.byte	W24
	.byte		N11   , Gn1
	.byte	W12
	.byte		N32   , Fn1 , v127 , gtp3
	.byte	W36
	.byte		N11   , Cn1
	.byte	W12
	.byte		N23   , Fn1
	.byte	W24
	.byte	N11
	.byte	W12
	.byte		        Cn2
	.byte	W12
	.byte		N32   , Fn1 , v127 , gtp3
	.byte	W36
	.byte		N23   , As0
	.byte	W24
	.byte		        Fn1
	.byte	W24
	.byte		N11   , As1
	.byte	W12
	.byte		        An1
	.byte	W12
	.byte		        Cn2
	.byte	W12
	.byte		        Fn1
	.byte	W12
	.byte		N23   , Cn2
	.byte	W24
	.byte	N11
	.byte	W12
	.byte		        Fn1
	.byte	W12
	.byte		        An1
	.byte	W12
	.byte		N23   , As1
	.byte	W24
	.byte		N11   , Dn2
	.byte	W12
	.byte		N23   , Fn1
	.byte	W24
	.byte	N11
	.byte	W12
	.byte		        Cn1
	.byte	W12
	.byte		        Fn1
	.byte	W12
	.byte		        As1
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		N05   , As1 , v020
	.byte	W24
	.byte		N11   , An1 , v127
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		N05   , An1 , v020
	.byte	W24
	.byte		N23   , Gn1 , v127
	.byte	W24
	.byte		N11   , An1
	.byte	W12
	.byte		        As1
	.byte	W12
	.byte		        Cn2
	.byte	W12
	.byte		        Cn1
	.byte	W12
	.byte		        Gn1
	.byte	W12
	.byte		        Cn1
	.byte	W12
	.byte		N23   , Fn1
	.byte	W24
	.byte		N05   , Fn1 , v020
	.byte	W12
	.byte		N11   , Fn1 , v127
	.byte	W12
	.byte		N05   , Fn1 , v020
	.byte	W12
	.byte		N11   , Fn1 , v127
	.byte	W12
	.byte		N05   , Fn1 , v020
	.byte	W12
	.byte		N11   , Cn1 , v127
	.byte	W12
	.byte		N23   , Fn1
	.byte	W24
	.byte		N05   , Fn1 , v020
	.byte	W12
	.byte		N11   , Fn1 , v127
	.byte	W12
	.byte		N05   , Fn1 , v020
	.byte	W12
	.byte		N11   , Fn1 , v127
	.byte	W12
	.byte		N05   , Fn1 , v020
	.byte	W12
	.byte		N11   , As0 , v127
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_610_7_003
	.byte	PATT
	 ptrvalue	mus_qz_610_7_003
	.byte	GOTO
	 ptrvalue	mus_qz_610_7_000
	.byte	FINE

/********************** Track  8 **********************/

mus_qz_610_8:
	.byte		KEYSH , mus_qz_610_key+0
	.byte		VOICE , 1
	.byte		PAN   , c_v+0
	.byte		VOL   , 70*mus_qz_610_mvl/mxv
	.byte		N11   , En1 , v100
	.byte		N92   , Cs2 , v100 , gtp3
	.byte	W12
	.byte		N11   , Cn1 , v127
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		        En1
	.byte	W12
	.byte		        Cn1
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		        En1 , v100
	.byte	W12
	.byte		        Cn1 , v127
	.byte	W12
	.byte		        En1 , v100
	.byte	W12
	.byte		        Cn1
	.byte	W12
	.byte		        En1
	.byte	W12
	.byte		        Cn1
	.byte		N11   , Gs1
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		        Gs1 , v068
	.byte	W12
	.byte		        Cn1 , v100
	.byte		N23   , As1
	.byte	W12
	.byte		N11   , Gs1
	.byte	W12
mus_qz_610_8_B1:
	.byte		N11   , Cn1 , v127
	.byte	W12
	.byte	N23
	.byte		N23   , Gs1 , v100
	.byte	W24
	.byte		        Cs1 , v080
	.byte		N23   , Gs1 , v068
	.byte	W24
	.byte		        Gs1 , v100
	.byte	W12
	.byte		        Fs2 , v080
	.byte	W12
	.byte		        Gs1 , v068
	.byte	W12
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_610_8_B1
	.byte	PATT
	 ptrvalue	mus_qz_610_8_B1
	.byte	PATT
	 ptrvalue	mus_qz_610_8_B1
mus_qz_610_8_000:
	.byte		N11   , Cn1 , v127
	.byte	W12
	.byte	N11
	.byte		N23   , Gs1 , v100
	.byte	W12
	.byte		        Cs1 , v080
	.byte	W12
	.byte		        Gs1 , v068
	.byte	W12
	.byte		N11   , Cs1 , v080
	.byte	W12
	.byte		        Cn1 , v127
	.byte		N11   , Gs1 , v100
	.byte	W12
	.byte		N05   , Cs1 , v080
	.byte		N11   , Fs2
	.byte	W06
	.byte		        Cs1
	.byte	W06
	.byte		N23   , Gs1 , v068
	.byte	W06
	.byte		N05   , Cn1 , v127
	.byte	W06
	.byte	PEND
	.byte	N11
	.byte	W12
	.byte	N11
	.byte		N23   , Gs1 , v100
	.byte	W12
	.byte		        Cs1 , v080
	.byte	W12
	.byte		        Gs1 , v068
	.byte	W12
	.byte		N11   , Cs1 , v080
	.byte	W12
	.byte		        Cn1 , v127
	.byte		N11   , Gs1 , v100
	.byte	W12
	.byte		        Cs1 , v080
	.byte		N11   , Fs2
	.byte	W12
	.byte		N05   , Cn1 , v127
	.byte		N23   , Gs1 , v068
	.byte	W06
	.byte		N05   , Cs1 , v080
	.byte	W06
	.byte		N11   , Cn1 , v127
	.byte	W12
	.byte	N11
	.byte		N23   , Gs1 , v100
	.byte	W12
	.byte		        Cs1 , v080
	.byte	W12
	.byte		        Gs1 , v068
	.byte	W12
	.byte		N05   , Cs1 , v080
	.byte	W06
	.byte	N11
	.byte	W06
	.byte		        Gs1 , v100
	.byte	W06
	.byte		N05   , Cn1 , v127
	.byte	W06
	.byte		N11   , Cs1 , v080
	.byte		N11   , Fs2
	.byte	W12
	.byte		        Cs1
	.byte		N23   , Gs1 , v068
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_610_8_000
	.byte		N11   , Cn1 , v127
	.byte	W12
	.byte	N11
	.byte		N23   , Gs1 , v100
	.byte	W12
	.byte		        Cs1 , v080
	.byte	W12
	.byte		        Gs1 , v068
	.byte	W12
	.byte		N11   , Cs1 , v080
	.byte	W12
	.byte		        Cn1 , v127
	.byte		N11   , Gs1 , v100
	.byte	W12
	.byte		        Cs1 , v080
	.byte		N11   , Fs2
	.byte	W12
	.byte		        Cs1
	.byte		N11   , Gs1 , v068
	.byte	W12
	.byte		        Cn1 , v127
	.byte		N11   , Fs2 , v080
	.byte	W12
	.byte		        Cn1 , v127
	.byte		N11   , Gs1 , v100
	.byte	W12
	.byte		N23   , Cs1 , v080
	.byte		N11   , Fs2
	.byte	W12
	.byte		        Gs1 , v100
	.byte	W12
	.byte		        Cs1 , v080
	.byte		N11   , Fs2
	.byte	W12
	.byte		        Cn1 , v127
	.byte		N11   , Gs1 , v100
	.byte	W12
	.byte		N05   , En1
	.byte		N11   , Fs2 , v080
	.byte	W06
	.byte		N05   , En1 , v100
	.byte	W06
	.byte		N11   , Cs1 , v080
	.byte		N11   , Gs1 , v100
	.byte	W12
	.byte		        En1
	.byte		N05   , Fs2
	.byte	W06
	.byte		        Fs2 , v080
	.byte	W06
	.byte		N11   , Cn1 , v127
	.byte		N11   , En1 , v100
	.byte	W12
	.byte		        Cn1 , v127
	.byte		N11   , En1 , v100
	.byte	W12
	.byte		        Cn1 , v127
	.byte		N11   , En1 , v100
	.byte	W12
	.byte	N05
	.byte	W06
	.byte		N11   , Cn1 , v127
	.byte	W06
	.byte		        Dn2 , v100
	.byte	W06
	.byte		N17   , Cn1 , v127
	.byte	W06
	.byte		N05   , Cn2 , v100
	.byte	W06
	.byte	N17
	.byte	W06
	.byte		N05   , En1
	.byte	W06
	.byte		        Cn1 , v127
	.byte	W06
mus_qz_610_8_001:
	.byte		N23   , Cn1 , v127
	.byte	W24
	.byte		N11   , En1 , v100
	.byte		N68   , Cs2 , v100 , gtp3
	.byte	W12
	.byte		N11   , Cn1 , v127
	.byte	W12
	.byte	N11
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		N05   , En1 , v100
	.byte	W06
	.byte		        Cn1 , v127
	.byte	W06
	.byte		        Fn1 , v100
	.byte	W06
	.byte		        Cn1 , v127
	.byte	W06
	.byte	PEND
	.byte	N11
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		        En1 , v100
	.byte	W12
	.byte		        Cn1 , v127
	.byte	W12
	.byte		N23   , As1 , v100
	.byte		N05   , Cn2
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Cn1 , v127
	.byte	W06
	.byte		        En1 , v100
	.byte	W06
	.byte	N05
	.byte		N23   , As1
	.byte	W06
	.byte		N05   , Cn1 , v127
	.byte	W06
	.byte		        An1 , v100
	.byte	W06
	.byte		        Cn1 , v127
	.byte	W06
	.byte	PATT
	 ptrvalue	mus_qz_610_8_001
	.byte		N11   , Cn1 , v127
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		N05   , En1 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Fn1
	.byte	W06
	.byte		        Cn1 , v127
	.byte	W06
	.byte		        En1 , v100
	.byte	W06
	.byte		N11   , Cn1 , v127
	.byte	W12
	.byte	N05
	.byte	W06
	.byte		N11   , En1 , v100
	.byte	W12
	.byte		        Cn1 , v127
	.byte	W12
	.byte	N11
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		N05   , En1 , v100
	.byte	W06
	.byte		        Cn1 , v127
	.byte	W06
	.byte		        Fn1 , v100
	.byte	W06
	.byte		        Cn1 , v127
	.byte	W06
	.byte	N11
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		N05   , En1 , v100
	.byte	W06
	.byte		        Cn1 , v127
	.byte	W06
	.byte		        Fn1 , v100
	.byte	W06
	.byte		        Cn1 , v127
	.byte	W06
	.byte	N11
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		        En1 , v100
	.byte	W12
	.byte		        Cn1 , v127
	.byte	W12
	.byte		N05   , En1 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Cn1 , v127
	.byte	W06
	.byte		        An1 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Cn1 , v127
	.byte	W06
	.byte		        Fn1 , v100
	.byte	W06
	.byte		        En1
	.byte	W06
	.byte		N11   , Cn1 , v127
	.byte		N92   , Cs2 , v100 , gtp3
	.byte	W12
	.byte		N11   , Cn1 , v127
	.byte	W12
	.byte		N05   , Cn2 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Cn1 , v127
	.byte	W06
	.byte		        An1 , v100
	.byte	W06
	.byte		N11   , Cn1 , v127
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		N05   , En1 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		N11   , Cn1 , v127
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		N05   , Gn1 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Fn1
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		N11   , Cn1 , v127
	.byte	W12
	.byte		N05   , En1 , v100
	.byte	W06
	.byte		        Cn1 , v127
	.byte	W06
	.byte		        Gn1 , v100
	.byte		N17   , As1
	.byte	W06
	.byte		N05   , Gn1
	.byte	W06
	.byte		        Cn1 , v127
	.byte	W06
	.byte		        En1 , v100
	.byte		N17   , As1
	.byte	W06
	.byte		N05   , Fn1
	.byte	W06
	.byte		        Cn1 , v127
	.byte	W06
	.byte	N11
	.byte		N92   , Cs2 , v100 , gtp3
	.byte	W12
	.byte		N11   , Cn1 , v127
	.byte	W12
	.byte		        En1 , v100
	.byte	W12
	.byte		        Cn1 , v127
	.byte	W12
	.byte		        Cs1 , v100
	.byte	W12
	.byte		        Cn1 , v127
	.byte	W12
	.byte		        En1 , v100
	.byte	W12
	.byte		        Cn1 , v127
	.byte	W12
	.byte		N92   , An4 , v100 , gtp3
	.byte	W12
	.byte		N11   , Cn1 , v127
	.byte	W12
	.byte		        En1 , v100
	.byte	W12
	.byte		        Cn1 , v127
	.byte	W12
	.byte	N11
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		        En1 , v100
	.byte	W12
	.byte		        Cn1 , v127
	.byte	W12
mus_qz_610_8_002:
	.byte		N11   , Cn1 , v127
	.byte		N92   , Cs2 , v100 , gtp3
	.byte	W12
	.byte		N11   , Cn1 , v127
	.byte	W12
	.byte		        En1 , v100
	.byte	W12
	.byte		        Cn1 , v127
	.byte	W12
	.byte	N11
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		        En1 , v100
	.byte	W12
	.byte		        Gn1
	.byte	W12
	.byte	PEND
	.byte		        Fn1
	.byte		N92   , An4 , v100 , gtp3
	.byte	W12
	.byte		N11   , Gn1
	.byte	W12
	.byte		        Cn1 , v127
	.byte	W12
	.byte		        En1 , v100
	.byte	W12
	.byte		        Cn1 , v127
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		        En1 , v100
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		        Cn1 , v127
	.byte		N23   , Gs1 , v056
	.byte	W12
	.byte		N11   , Cn1 , v127
	.byte	W12
	.byte		        En1 , v100
	.byte		N23   , Gs1 , v084
	.byte	W12
	.byte		N11   , Cn1 , v127
	.byte	W12
	.byte		        En1 , v100
	.byte		N23   , Gs1 , v056
	.byte	W12
	.byte		N11   , Cn1 , v127
	.byte	W12
	.byte		        En1 , v100
	.byte		N23   , Gs1 , v084
	.byte	W12
	.byte		N11   , Gn1 , v100
	.byte	W12
	.byte		        Fn1
	.byte		N23   , Gs1 , v056
	.byte	W12
	.byte		N11   , Gn1 , v100
	.byte	W12
	.byte		        En1
	.byte		N23   , Gs1 , v084
	.byte	W12
	.byte		N11   , Fn1 , v100
	.byte	W12
	.byte		        En1
	.byte		N23   , Gs1 , v056
	.byte	W12
	.byte		N11   , Cn1 , v127
	.byte	W12
	.byte		        En1 , v100
	.byte		N23   , Gs1 , v084
	.byte	W12
	.byte		N11   , Cn1 , v127
	.byte	W12
	.byte	N11
	.byte		N23   , Gs1 , v056
	.byte	W12
	.byte		N11   , Cn1 , v127
	.byte	W12
	.byte		        En1 , v100
	.byte		N23   , Gs1 , v084
	.byte	W12
	.byte		N11   , Cn1 , v127
	.byte	W12
	.byte	N11
	.byte		N23   , Gs1 , v056
	.byte	W12
	.byte		N11   , Cn1 , v127
	.byte	W12
	.byte		        En1 , v100
	.byte		N23   , Gn1 , v084
	.byte	W12
	.byte		N11   , Cn1 , v127
	.byte	W12
	.byte	N11
	.byte		N11   , Gs1 , v084
	.byte	W12
	.byte		        Cn1 , v127
	.byte		N11   , Gs1 , v084
	.byte	W12
	.byte		        Bn1 , v100
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		N05   , En1
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		N11   , Cn1 , v127
	.byte	W12
	.byte		N05   , Gn1 , v100
	.byte		N23   , As1
	.byte	W06
	.byte		N05   , Gn1
	.byte	W06
	.byte		N11   , En1
	.byte	W12
	.byte		        Cn1 , v127
	.byte		N92   , Cs2 , v100 , gtp3
	.byte	W12
	.byte		N11   , Cn1 , v127
	.byte	W12
	.byte		        En1 , v100
	.byte	W12
	.byte		        Cn1 , v127
	.byte	W12
	.byte	N11
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		        En1 , v100
	.byte	W12
	.byte		        Cn1 , v127
	.byte	W12
	.byte	N11
	.byte		N92   , An4 , v100 , gtp3
	.byte	W12
	.byte		N11   , Cn1 , v127
	.byte	W12
	.byte		        En1 , v100
	.byte	W12
	.byte		        Cn1 , v127
	.byte	W12
	.byte	N11
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		        En1 , v100
	.byte	W12
	.byte		        Cn1 , v127
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_610_8_002
	.byte		N11   , Fn1 , v100
	.byte		N92   , An4 , v100 , gtp3
	.byte	W12
	.byte		N11   , Gn1
	.byte	W12
	.byte		        En1
	.byte	W12
	.byte		        Cn1 , v127
	.byte	W12
	.byte	N11
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		        En1 , v100
	.byte	W12
	.byte		        Cn1 , v127
	.byte	W12
	.byte	N11
	.byte		N11   , En1 , v100
	.byte	W12
	.byte		        Cn1 , v127
	.byte		N11   , En1 , v100
	.byte	W12
	.byte	N23
	.byte		N23   , As1
	.byte	W24
	.byte		N11   , Cn1 , v127
	.byte		N11   , En1 , v100
	.byte	W12
	.byte		        Cn1 , v127
	.byte		N11   , En1 , v100
	.byte	W12
	.byte	N11
	.byte		N23   , As1
	.byte	W12
	.byte		N11   , Cn1 , v127
	.byte	W12
	.byte		        En1 , v100
	.byte	W12
	.byte		N23   , Cn1 , v127
	.byte		N80   , Cs2 , v100 , gtp3
	.byte	W24
	.byte		N23   , Dn2
	.byte	W24
	.byte	N11
	.byte	W12
	.byte		N05   , An1
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		N11   , Fn1
	.byte	W12
mus_qz_610_8_003:
	.byte		N11   , Cn1 , v127
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		        En1 , v100
	.byte		N44   , Ds2 , v100 , gtp3
	.byte	W12
	.byte		N11   , Cn1 , v127
	.byte	W12
	.byte	N11
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		        En1 , v100
	.byte		N44   , Ds2 , v100 , gtp3
	.byte	W12
	.byte		N11   , Cn1 , v127
	.byte	W12
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_610_8_003
mus_qz_610_8_004:
	.byte		N11   , Cn1 , v127
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		        En1 , v100
	.byte	W12
	.byte		        Cn1 , v127
	.byte	W12
	.byte	N11
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		N05   , En1 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		N11   , Cn1 , v127
	.byte	W12
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_610_8_004
	.byte	GOTO
	 ptrvalue	mus_qz_610_8_B1
	.byte	FINE

/******************************************************/
	.p2align	2

mus_qz_610:
	.byte	8	/* NumTrks */
	.byte	0	/* NumBlks */
	.byte	mus_qz_610_pri	/* Priority */
	.byte	mus_qz_610_rev	/* Reverb. */
space64 4
	ptrvalue	mus_qz_610_grp

	ptrvalue	mus_qz_610_1
	ptrvalue	mus_qz_610_2
	ptrvalue	mus_qz_610_3
	ptrvalue	mus_qz_610_4
	ptrvalue	mus_qz_610_5
	ptrvalue	mus_qz_610_6
	ptrvalue	mus_qz_610_7
	ptrvalue	mus_qz_610_8

	.end
