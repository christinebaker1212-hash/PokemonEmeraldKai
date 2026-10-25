	.include "asm/macros/bit_width.inc"

	.include "MPlayDef.s"

/* Moemon Quetzal song 629, disassembled from the ROM by tools/quetzal_rip/songs.py.
   "org" comments give the ROM address of the bytes that follow. */

	.equ	mus_qz_629_grp, voicegroup_qz_songs
	.equ	mus_qz_629_pri, 0
	.equ	mus_qz_629_rev, reverb_set+0
	.equ	mus_qz_629_mvl, 127
	.equ	mus_qz_629_key, 0
	.equ	mus_qz_629_tbs, 1
	.equ	mus_qz_629_exg, 1
	.equ	mus_qz_629_cmp, 1

	.section .rodata
	.global	mus_qz_629
	.p2align	2
/* org 0x09680CC4 */

/********************** Track  1 **********************/

mus_qz_629_1:
	.byte		KEYSH , mus_qz_629_key+0
	.byte		TEMPO , 130*mus_qz_629_tbs/2
	.byte		VOICE , 0
	.byte		VOL   , 84*mus_qz_629_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte	W48
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
mus_qz_629_1_000:
	.byte	W11
	.byte		N05   , As4 , v100
	.byte	W01
	.byte		        Dn5
	.byte	W17
	.byte		        Gn4
	.byte	W01
	.byte		        As4
	.byte	W17
	.byte		        Ds4
	.byte	W01
	.byte		        Gn4
	.byte	W12
	.byte		        Cn4
	.byte	W12
	.byte		        Dn4
	.byte	W12
	.byte		        Ds4
	.byte	W12
	.byte	PEND
mus_qz_629_1_001:
	.byte		N05   , Fn4 , v100
	.byte	W12
	.byte		        Ds4
	.byte	W12
	.byte		        Fn4
	.byte	W06
	.byte		        Ds4
	.byte	W12
	.byte		N17   , Fn4
	.byte	W18
	.byte		N23   , Gn4
	.byte	W24
	.byte		N05   , Fs4
	.byte	W06
	.byte		        Fn4
	.byte	W06
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_629_1_000
mus_qz_629_1_002:
	.byte	W06
	.byte		N05   , Fn4 , v100
	.byte	W06
	.byte		        Ds4
	.byte	W12
	.byte		        Gn4
	.byte	W06
	.byte		        Ds4
	.byte	W12
	.byte		N17   , As4
	.byte	W18
	.byte		N11   , An4
	.byte	W12
	.byte		N05   , As4
	.byte	W06
	.byte		        Cn5
	.byte	W18
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_629_1_000
	.byte	PATT
	 ptrvalue	mus_qz_629_1_001
	.byte	PATT
	 ptrvalue	mus_qz_629_1_000
	.byte	W96
mus_qz_629_1_B1:
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
mus_qz_629_1_003:
	.byte	W12
	.byte		N05   , As4 , v100
	.byte		N05   , Dn5 , v108
	.byte	W18
	.byte		        Gn4 , v100
	.byte		N05   , As4 , v108
	.byte	W18
	.byte		        Ds4 , v100
	.byte		N05   , Gn4 , v108
	.byte	W12
	.byte		        Cn4
	.byte	W12
	.byte		        Dn4
	.byte	W12
	.byte		        Ds4
	.byte	W12
	.byte	PEND
mus_qz_629_1_004:
	.byte		N05   , Fn4 , v108
	.byte	W12
	.byte		        Ds4
	.byte	W12
	.byte		        Fn4
	.byte	W06
	.byte		        Ds4
	.byte	W12
	.byte		N17   , Fn4
	.byte	W18
	.byte		N23   , Gn4
	.byte	W24
	.byte		N05   , Fs4
	.byte	W06
	.byte		        Fn4
	.byte	W06
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_629_1_003
	.byte	W06
	.byte		N05   , Fn4 , v108
	.byte	W06
	.byte		        Ds4
	.byte	W12
	.byte		        Gn4
	.byte	W06
	.byte		        Ds4
	.byte	W12
	.byte		N17   , As4
	.byte	W18
	.byte		N11   , An4
	.byte	W12
	.byte		N05   , As4
	.byte	W06
	.byte		        Cn5
	.byte	W18
	.byte	PATT
	 ptrvalue	mus_qz_629_1_003
	.byte	PATT
	 ptrvalue	mus_qz_629_1_004
	.byte	PATT
	 ptrvalue	mus_qz_629_1_003
	.byte		N05   , Fn4 , v108
	.byte	W12
	.byte		        Ds4
	.byte	W12
	.byte		        Fn4
	.byte	W06
	.byte		N12   , Ds4
	.byte	W12
	.byte		N17   , Gn4
	.byte	W18
	.byte		N23   , Fn4
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
	.byte	W96
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_629_1_000
	.byte	PATT
	 ptrvalue	mus_qz_629_1_001
	.byte	PATT
	 ptrvalue	mus_qz_629_1_000
	.byte	PATT
	 ptrvalue	mus_qz_629_1_002
	.byte	PATT
	 ptrvalue	mus_qz_629_1_000
	.byte	PATT
	 ptrvalue	mus_qz_629_1_001
	.byte	PATT
	 ptrvalue	mus_qz_629_1_000
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_629_1_B1
	.byte	FINE

/********************** Track  2 **********************/

mus_qz_629_2:
	.byte		KEYSH , mus_qz_629_key+0
	.byte		VOICE , 1
	.byte		VOL   , 72*mus_qz_629_mvl/mxv
	.byte		PAN   , c_v-34
	.byte		PRIO  , 62
	.byte	W48
mus_qz_629_2_000:
	.byte	W12
	.byte		N05   , Ds3 , v076
	.byte		N05   , Gn3 , v088
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W12
	.byte		        Ds3
	.byte		N05   , Gn3 , v088
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W12
	.byte		        Ds3
	.byte		N05   , Gn3 , v088
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W12
	.byte		        Fn3 , v088
	.byte		N05   , An3
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W12
	.byte		N10   , Fn3 , v088
	.byte		N10   , An3
	.byte	W12
	.byte	PEND
mus_qz_629_2_001:
	.byte		PAN   , c_v+46
	.byte	W12
	.byte		N05   , Ds3 , v076
	.byte		N05   , Gn3 , v088
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W12
	.byte		        Ds3
	.byte		N05   , Gn3 , v088
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W12
	.byte		        Fn3 , v088
	.byte		N05   , As3
	.byte	W12
	.byte		        Fn3
	.byte		N05   , An3
	.byte	W12
	.byte		        Ds3 , v076
	.byte		N05   , Gn3 , v088
	.byte	W06
	.byte		N16   , Fn3
	.byte		N16   , An3
	.byte	W18
	.byte	PEND
mus_qz_629_2_002:
	.byte		PAN   , c_v-34
	.byte	W12
	.byte		N05   , Ds3 , v076
	.byte		N05   , Gn3 , v088
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W12
	.byte		        Ds3
	.byte		N05   , Gn3 , v088
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W12
	.byte		        Ds3
	.byte		N05   , Gn3 , v088
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W12
	.byte		        Fn3 , v088
	.byte		N05   , An3
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W12
	.byte		N10   , Fn3 , v088
	.byte		N10   , An3
	.byte	W12
	.byte	PEND
mus_qz_629_2_003:
	.byte		PAN   , c_v+46
	.byte	W12
	.byte		N05   , Ds3 , v076
	.byte		N05   , Gn3 , v088
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W12
	.byte		        Ds3
	.byte		N05   , Gn3 , v088
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W12
	.byte		        Fn3 , v088
	.byte		N05   , As3
	.byte	W12
	.byte		        Fn3
	.byte		N05   , An3
	.byte	W12
	.byte		        Fn3
	.byte		N05   , As3
	.byte	W06
	.byte		N16   , Fn3
	.byte		N16   , Cn4
	.byte	W18
	.byte	PEND
mus_qz_629_2_004:
	.byte		PAN   , c_v-44
	.byte	W12
	.byte		N05   , Ds3 , v076
	.byte		N05   , Gn3 , v088
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W12
	.byte		        Ds3
	.byte		N05   , Gn3 , v088
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W12
	.byte		        Ds3
	.byte		N05   , Gn3 , v088
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W12
	.byte		        Fn3
	.byte		N05   , An3 , v088
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W12
	.byte		N11   , Fn3
	.byte		N11   , An3 , v088
	.byte	W12
	.byte	PEND
mus_qz_629_2_005:
	.byte	W12
	.byte		N05   , Gs3 , v076
	.byte		N05   , Cn4 , v088
	.byte	W06
	.byte		        Fn3
	.byte	W12
	.byte		        Gs3 , v076
	.byte		N05   , Cn4 , v088
	.byte	W06
	.byte		        Fn3
	.byte	W12
	.byte		        As3 , v076
	.byte		N05   , Ds4 , v088
	.byte	W12
	.byte		        As3 , v076
	.byte		N05   , Dn4 , v088
	.byte	W12
	.byte		        Gs3 , v076
	.byte		N05   , Cn4 , v088
	.byte	W06
	.byte		N17   , As3 , v076
	.byte		N17   , Dn4 , v088
	.byte	W18
	.byte	PEND
mus_qz_629_2_006:
	.byte	W12
	.byte		N05   , Ds3 , v076
	.byte		N05   , Gn3 , v088
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W12
	.byte		        Ds3
	.byte		N05   , Gn3 , v088
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W12
	.byte		        Ds3
	.byte		N05   , Gn3 , v088
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W12
	.byte		        Fn3
	.byte		N05   , An3 , v088
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W12
	.byte		N11   , Fn3
	.byte		N11   , An3 , v088
	.byte	W12
	.byte	PEND
mus_qz_629_2_007:
	.byte	W12
	.byte		N05   , Gs3 , v076
	.byte		N05   , Cn4 , v088
	.byte	W06
	.byte		        Fn3
	.byte	W12
	.byte		        Gs3 , v076
	.byte		N05   , Cn4 , v088
	.byte	W06
	.byte		        Fn3
	.byte	W12
	.byte		        As3 , v076
	.byte		N05   , Ds4 , v088
	.byte	W12
	.byte		        As3 , v076
	.byte		N05   , Dn4 , v088
	.byte	W12
	.byte		        Gs3 , v076
	.byte		N05   , Cn4 , v088
	.byte	W06
	.byte		N11   , As3 , v076
	.byte		N11   , Dn4 , v088
	.byte	W18
	.byte	PEND
mus_qz_629_2_008:
	.byte	W12
	.byte		N05   , Ds3 , v076
	.byte		N05   , Gn3 , v088
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W12
	.byte		        Ds3
	.byte		N05   , Gn3 , v088
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W12
	.byte		        Ds3
	.byte		N05   , Gn3 , v088
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W12
	.byte		        Fn3 , v088
	.byte		N05   , An3
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W12
	.byte		N11   , Fn3 , v088
	.byte		N11   , An3
	.byte	W12
	.byte	PEND
mus_qz_629_2_009:
	.byte	W12
	.byte		N05   , Gs3 , v088
	.byte		N05   , Cn4
	.byte	W06
	.byte		        Fn3
	.byte	W12
	.byte		        Gs3
	.byte		N05   , Cn4
	.byte	W06
	.byte		        Fn3
	.byte	W12
	.byte		        As3
	.byte		N05   , Ds4
	.byte	W12
	.byte		        As3
	.byte		N05   , Dn4
	.byte	W12
	.byte		        Gs3
	.byte		N05   , Cn4
	.byte	W06
	.byte		N17   , As3
	.byte		N17   , Dn4
	.byte	W18
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_629_2_008
	.byte	W12
	.byte		N05   , Gs3 , v092
	.byte		N05   , Cn4 , v104
	.byte	W06
	.byte		        Fn3 , v092
	.byte	W12
	.byte		        Gs3
	.byte		N05   , Cn4 , v104
	.byte	W12
	.byte		N17   , Gs3 , v092
	.byte		N17   , Ds4 , v104
	.byte	W18
	.byte		N05   , Gn3 , v092
	.byte		N05   , Dn4 , v104
	.byte	W12
	.byte		        As3 , v092
	.byte		N05   , Ds4 , v104
	.byte	W06
	.byte		N17   , Cn4 , v092
	.byte		N17   , Fn4 , v104
	.byte	W18
mus_qz_629_2_B1:
	.byte	W12
	.byte		N05   , Ds3 , v080
	.byte		N05   , Gn3 , v088
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W12
	.byte		        Ds3 , v080
	.byte		N05   , Gn3 , v088
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W12
	.byte		        Ds3 , v080
	.byte		N05   , Gn3 , v088
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W12
	.byte		        Fn3 , v080
	.byte		N05   , An3 , v088
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W12
	.byte		N11   , Fn3 , v080
	.byte		N11   , An3 , v088
	.byte	W12
	.byte	W12
	.byte		N05   , Ds3 , v076
	.byte		N05   , Gn3 , v088
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W12
	.byte		        Ds3
	.byte		N05   , Gn3 , v088
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W12
	.byte		        Fn3
	.byte		N05   , As3 , v088
	.byte	W12
	.byte		        Fn3 , v076
	.byte		N05   , An3 , v088
	.byte	W12
	.byte		        Ds3 , v076
	.byte		N05   , Gn3 , v088
	.byte	W06
	.byte		N17   , Fn3 , v076
	.byte		N17   , An3 , v088
	.byte	W18
	.byte	PATT
	 ptrvalue	mus_qz_629_2_006
	.byte	W12
	.byte		N05   , Ds3 , v076
	.byte		N05   , Gn3 , v088
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W12
	.byte		        Ds3
	.byte		N05   , Gn3 , v088
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W12
	.byte		        Fn3
	.byte		N05   , As3 , v088
	.byte	W12
	.byte		        Fn3 , v076
	.byte		N05   , An3 , v088
	.byte	W12
	.byte		        Fn3 , v076
	.byte		N05   , As3 , v088
	.byte	W06
	.byte		N17   , Fn3 , v076
	.byte		N17   , Cn4 , v088
	.byte	W18
	.byte	PATT
	 ptrvalue	mus_qz_629_2_004
	.byte	PATT
	 ptrvalue	mus_qz_629_2_005
	.byte	PATT
	 ptrvalue	mus_qz_629_2_006
	.byte	PATT
	 ptrvalue	mus_qz_629_2_007
	.byte	PATT
	 ptrvalue	mus_qz_629_2_008
	.byte	PATT
	 ptrvalue	mus_qz_629_2_009
	.byte	PATT
	 ptrvalue	mus_qz_629_2_008
mus_qz_629_2_010:
	.byte	W12
	.byte		N05   , Gs3 , v088
	.byte		N05   , Cn4 , v100
	.byte	W06
	.byte		        Fn3 , v088
	.byte	W12
	.byte		        Gs3
	.byte		N05   , Cn4 , v100
	.byte	W12
	.byte		N17   , Gs3 , v088
	.byte		N17   , Ds4 , v100
	.byte	W18
	.byte		N05   , Gn3 , v088
	.byte		N05   , Dn4 , v100
	.byte	W12
	.byte		        As3 , v088
	.byte		N05   , Ds4 , v100
	.byte	W06
	.byte		N17   , Cn4 , v088
	.byte		N17   , Fn4 , v100
	.byte	W18
	.byte	PEND
	.byte		N04   , Fn5 , v068
	.byte	W06
	.byte		        Cn5 , v052
	.byte	W06
	.byte		        Gs4 , v068
	.byte	W06
	.byte		        Cn5 , v052
	.byte	W06
	.byte		        Gs4 , v068
	.byte	W06
	.byte		        Fn4 , v052
	.byte	W06
	.byte		        Gs4 , v068
	.byte	W06
	.byte		        Cn5 , v052
	.byte	W06
	.byte		        Fn5 , v068
	.byte	W06
	.byte		        Cn5 , v052
	.byte	W06
	.byte		        Gs4 , v068
	.byte	W06
	.byte		        Cn5 , v052
	.byte	W06
	.byte		        Gs4 , v068
	.byte	W06
	.byte		        Fn4 , v052
	.byte	W06
	.byte		        Gs4 , v068
	.byte	W06
	.byte		        Cn5
	.byte	W06
mus_qz_629_2_011:
	.byte		N04   , Gn5 , v068
	.byte	W06
	.byte		        Dn5 , v052
	.byte	W06
	.byte		        As4 , v068
	.byte	W06
	.byte		        Dn5 , v052
	.byte	W06
	.byte		        As4 , v068
	.byte	W06
	.byte		        Gn4 , v052
	.byte	W06
	.byte		        As4 , v068
	.byte	W06
	.byte		        Dn5 , v052
	.byte	W06
	.byte		        Gn5 , v068
	.byte	W06
	.byte		        Dn5 , v052
	.byte	W06
	.byte		        As4 , v068
	.byte	W06
	.byte		        Dn5 , v052
	.byte	W06
	.byte		        As4 , v068
	.byte	W06
	.byte		        Gn4 , v052
	.byte	W06
	.byte		        As4 , v068
	.byte	W06
	.byte		        Dn5 , v052
	.byte	W06
	.byte	PEND
	.byte		        Cn5 , v068
	.byte	W06
	.byte		        Gn4 , v052
	.byte	W06
	.byte		        Ds4 , v068
	.byte	W06
	.byte		        Gn4 , v052
	.byte	W06
	.byte		        Cn4 , v068
	.byte	W06
	.byte		        Ds4 , v052
	.byte	W06
	.byte		        Gn4 , v068
	.byte	W06
	.byte		        Cn5 , v052
	.byte	W06
	.byte		        Gn4 , v068
	.byte	W06
	.byte		        Ds4 , v052
	.byte	W06
	.byte		        Gn4 , v068
	.byte	W06
	.byte		        Cn5 , v052
	.byte	W06
	.byte		        Ds5 , v068
	.byte	W06
	.byte		        Cn5 , v052
	.byte	W06
	.byte		        Ds4 , v068
	.byte	W06
	.byte		        As4 , v052
	.byte	W06
	.byte		        Cn5 , v068
	.byte	W06
	.byte		        An4 , v052
	.byte	W06
	.byte		        Fn4 , v068
	.byte	W06
	.byte		        An4 , v052
	.byte	W06
	.byte		        Fn4 , v068
	.byte	W06
	.byte		        Cn4 , v052
	.byte	W06
	.byte		        Fn4 , v068
	.byte	W06
	.byte		        An4 , v052
	.byte	W06
	.byte		        Fn5 , v068
	.byte	W06
	.byte		        Fn5 , v052
	.byte	W06
	.byte		        Cn5 , v068
	.byte	W06
	.byte		        An4 , v052
	.byte	W06
	.byte		        Cn5 , v068
	.byte	W06
	.byte		        An4 , v052
	.byte	W06
	.byte		        Fn4 , v068
	.byte	W06
	.byte		        Cn4 , v052
	.byte	W06
	.byte		        Fn5 , v068
	.byte	W06
	.byte		        Cn5 , v052
	.byte	W06
	.byte		        Gs4 , v068
	.byte	W06
	.byte		        Cn5 , v052
	.byte	W06
	.byte		        Gs4 , v068
	.byte	W06
	.byte		        Fn4 , v052
	.byte	W06
	.byte		        Gs4 , v068
	.byte	W06
	.byte		        Cn5 , v052
	.byte	W06
	.byte		        Fn5 , v068
	.byte	W06
	.byte		        Cn5 , v052
	.byte	W06
	.byte		        Gs4 , v068
	.byte	W06
	.byte		        Cn5 , v052
	.byte	W06
	.byte		        Gs4 , v068
	.byte	W06
	.byte		        Fn4 , v052
	.byte	W06
	.byte		        Gs4 , v068
	.byte	W06
	.byte		        Cn5 , v052
	.byte	W06
	.byte	PATT
	 ptrvalue	mus_qz_629_2_011
	.byte		N04   , Gn5 , v068
	.byte	W06
	.byte		        Dn5 , v052
	.byte	W06
	.byte		        Bn4 , v068
	.byte	W06
	.byte		        Dn5 , v052
	.byte	W06
	.byte		        Bn4 , v068
	.byte	W06
	.byte		        Gn4 , v052
	.byte	W06
	.byte		        Bn4 , v068
	.byte	W06
	.byte		        Dn5 , v052
	.byte	W06
	.byte		        Bn4 , v068
	.byte	W06
	.byte		        Bn4 , v052
	.byte	W06
	.byte		        Gn4 , v068
	.byte	W06
	.byte		        Bn4 , v052
	.byte	W06
	.byte		        Dn4 , v068
	.byte	W06
	.byte		        Gn4 , v052
	.byte	W06
	.byte		        Bn4 , v068
	.byte	W06
	.byte		        Dn5 , v052
	.byte	W06
	.byte		N05   , Dn4
	.byte		N05   , Gn4 , v076
	.byte	W06
	.byte		        Dn4 , v044
	.byte		N05   , Gn4 , v068
	.byte	W06
	.byte		        Dn4 , v052
	.byte		N05   , Gn4 , v076
	.byte	W18
	.byte		        Fn4 , v052
	.byte		N05   , As4 , v076
	.byte	W12
	.byte		        Fn4 , v064
	.byte		N05   , As4 , v088
	.byte	W54
	.byte	PATT
	 ptrvalue	mus_qz_629_2_000
	.byte	PATT
	 ptrvalue	mus_qz_629_2_001
	.byte	PATT
	 ptrvalue	mus_qz_629_2_002
	.byte	PATT
	 ptrvalue	mus_qz_629_2_003
	.byte	PATT
	 ptrvalue	mus_qz_629_2_004
	.byte	PATT
	 ptrvalue	mus_qz_629_2_005
	.byte	PATT
	 ptrvalue	mus_qz_629_2_006
	.byte	PATT
	 ptrvalue	mus_qz_629_2_007
	.byte	W12
	.byte		N05   , Gn3 , v088
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W12
	.byte		        Gn3 , v088
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W12
	.byte		        Gn3 , v088
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W12
	.byte		        Fn3 , v088
	.byte		N05   , An3
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W12
	.byte		N11   , Fn3 , v088
	.byte		N11   , An3
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_629_2_009
	.byte	PATT
	 ptrvalue	mus_qz_629_2_008
	.byte	PATT
	 ptrvalue	mus_qz_629_2_010
	.byte	GOTO
	 ptrvalue	mus_qz_629_2_B1
	.byte	FINE

/********************** Track  3 **********************/

mus_qz_629_3:
	.byte		KEYSH , mus_qz_629_key+0
	.byte		VOICE , 22
	.byte		VOL   , 89*mus_qz_629_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 60
	.byte	W48
mus_qz_629_3_000:
	.byte		N05   , Cn1 , v100
	.byte	W18
	.byte	N05
	.byte	W18
	.byte	N05
	.byte	W12
	.byte	N05
	.byte	W48
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_629_3_000
	.byte	PATT
	 ptrvalue	mus_qz_629_3_000
mus_qz_629_3_001:
	.byte		N05   , Cn1 , v100
	.byte	W18
	.byte	N05
	.byte	W18
	.byte	N05
	.byte	W12
	.byte		        Fn1
	.byte	W12
	.byte	N05
	.byte	W12
	.byte		        Gn1
	.byte	W12
	.byte	N05
	.byte	W12
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_629_3_000
mus_qz_629_3_002:
	.byte		N05   , Cn1 , v100
	.byte	W18
	.byte	N05
	.byte	W18
	.byte	N05
	.byte	W12
	.byte	N05
	.byte	W12
	.byte	N05
	.byte	W12
	.byte		        Fn1
	.byte	W12
	.byte	N05
	.byte	W12
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_629_3_000
	.byte	PATT
	 ptrvalue	mus_qz_629_3_001
	.byte	PATT
	 ptrvalue	mus_qz_629_3_000
	.byte	PATT
	 ptrvalue	mus_qz_629_3_000
	.byte	PATT
	 ptrvalue	mus_qz_629_3_000
	.byte	W96
mus_qz_629_3_B1:
	.byte		N04   , Cn1 , v100
	.byte	W06
	.byte	N04
	.byte	W06
	.byte	N04
	.byte	W12
	.byte	N04
	.byte	W18
	.byte	N04
	.byte	W18
	.byte	N04
	.byte	W18
	.byte	N11
	.byte	W18
	.byte	N04
	.byte	W06
	.byte	N04
	.byte	W06
	.byte	N04
	.byte	W18
	.byte	N05
	.byte	W12
	.byte	N05
	.byte	W18
	.byte	N05
	.byte	W18
	.byte	N12
	.byte	W18
	.byte	N04
	.byte	W06
	.byte	N04
	.byte	W06
	.byte	N04
	.byte	W12
	.byte	N05
	.byte	W18
	.byte	N05
	.byte	W18
	.byte	N05
	.byte	W18
	.byte	N04
	.byte	W06
	.byte	N04
	.byte	W06
	.byte	N04
	.byte	W06
	.byte	N04
	.byte	W06
	.byte	N04
	.byte	W06
	.byte	N04
	.byte	W18
	.byte	N05
	.byte	W12
	.byte	N05
	.byte	W18
	.byte	N05
	.byte	W24
	.byte	N11
	.byte	W12
	.byte	N04
	.byte	W06
	.byte	N04
	.byte	W06
	.byte	N04
	.byte	W12
	.byte	N05
	.byte	W18
	.byte	N05
	.byte	W18
	.byte	N05
	.byte	W18
	.byte	N05
	.byte	W06
	.byte	N04
	.byte	W06
	.byte	N04
	.byte	W06
mus_qz_629_3_003:
	.byte		N04   , Fn1 , v100
	.byte	W06
	.byte	N04
	.byte	W06
	.byte	N04
	.byte	W12
	.byte	N05
	.byte	W18
	.byte	N05
	.byte	W18
	.byte	N05
	.byte	W24
	.byte	N05
	.byte	W06
	.byte	N04
	.byte	W06
	.byte	PEND
mus_qz_629_3_004:
	.byte		N04   , Cn1 , v100
	.byte	W06
	.byte	N04
	.byte	W06
	.byte	N04
	.byte	W12
	.byte	N05
	.byte	W18
	.byte	N05
	.byte	W18
	.byte	N05
	.byte	W18
	.byte	N05
	.byte	W18
	.byte	PEND
	.byte		N04   , Fn1
	.byte	W06
	.byte	N04
	.byte	W06
	.byte	N04
	.byte	W12
	.byte	N05
	.byte	W18
	.byte		        Fn1 , v092
	.byte	W06
	.byte		        Gn1 , v100
	.byte	W12
	.byte	N05
	.byte	W18
	.byte	N05
	.byte	W12
	.byte		N04   , Gn1 , v088
	.byte	W06
	.byte	PATT
	 ptrvalue	mus_qz_629_3_004
	.byte	PATT
	 ptrvalue	mus_qz_629_3_003
	.byte		N05   , Cn1 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte	N05
	.byte	W12
	.byte	N05
	.byte	W18
	.byte	N05
	.byte	W18
	.byte	N05
	.byte	W18
	.byte	N05
	.byte	W18
	.byte		        Fn1
	.byte	W06
	.byte	N05
	.byte	W06
	.byte	N05
	.byte	W12
	.byte	N05
	.byte	W18
	.byte	N05
	.byte	W18
	.byte	N05
	.byte	W24
	.byte	N05
	.byte	W06
	.byte	N04
	.byte	W06
	.byte	N11
	.byte	W18
	.byte		N05   , Fn1 , v088
	.byte	W06
	.byte		        Fn2
	.byte	W06
	.byte		        Fn1 , v100
	.byte	W12
	.byte		        Fn1 , v088
	.byte	W06
	.byte		        Fn1 , v100
	.byte	W06
	.byte		        Fn2 , v088
	.byte	W06
	.byte		N11   , Fn1 , v100
	.byte	W12
	.byte		N05   , Fn2 , v088
	.byte	W06
	.byte		        Fn1
	.byte	W12
	.byte		        Fn2
	.byte	W06
mus_qz_629_3_005:
	.byte		N11   , Gn1 , v100
	.byte	W18
	.byte		N05   , Gn1 , v088
	.byte	W06
	.byte		        Gn2
	.byte	W06
	.byte		        Gn1 , v100
	.byte	W12
	.byte		        Gn1 , v088
	.byte	W06
	.byte		        Gn1 , v100
	.byte	W06
	.byte		        Gn2 , v088
	.byte	W06
	.byte		        Fn1 , v100
	.byte	W06
	.byte		        Fn2 , v088
	.byte	W06
	.byte		        Ds1 , v100
	.byte	W06
	.byte		        Ds2 , v088
	.byte	W06
	.byte		        Dn1 , v100
	.byte	W06
	.byte		        Dn2 , v088
	.byte	W06
	.byte	PEND
	.byte		N11   , Cn1 , v100
	.byte	W18
	.byte		N05   , Cn1 , v088
	.byte	W06
	.byte		        Cn2
	.byte	W06
	.byte		        Cn1 , v100
	.byte	W12
	.byte		        Cn1 , v088
	.byte	W06
	.byte		        Cn1 , v100
	.byte	W06
	.byte		        Cn2 , v088
	.byte	W06
	.byte		N11   , Cn1 , v100
	.byte	W12
	.byte		N05   , Cn2 , v088
	.byte	W06
	.byte		        Cn1 , v100
	.byte	W12
	.byte		        Cn2 , v088
	.byte	W06
mus_qz_629_3_006:
	.byte		N11   , Fn1 , v100
	.byte	W18
	.byte		N05   , Fn1 , v088
	.byte	W06
	.byte		        Fn2
	.byte	W06
	.byte		        Fn1 , v100
	.byte	W12
	.byte		        Fn1 , v088
	.byte	W06
	.byte		        Fn1 , v100
	.byte	W06
	.byte		        Fn2 , v088
	.byte	W06
	.byte		N11   , Fn1 , v100
	.byte	W12
	.byte		N05   , Fn2 , v088
	.byte	W06
	.byte		        Fn1 , v100
	.byte	W12
	.byte		        Fn2 , v088
	.byte	W06
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_629_3_006
	.byte	PATT
	 ptrvalue	mus_qz_629_3_005
	.byte		N11   , Gn1 , v100
	.byte	W18
	.byte		N05   , Gn1 , v088
	.byte	W06
	.byte		        Gn2
	.byte	W06
	.byte		        Gn1
	.byte	W12
	.byte	N05
	.byte	W06
	.byte		        Gn1 , v100
	.byte	W06
	.byte		        Gn2 , v088
	.byte	W06
	.byte		N11   , Gn1 , v100
	.byte	W12
	.byte		N05   , Gn2 , v088
	.byte	W06
	.byte		        Gn1
	.byte	W12
	.byte		        Gn2
	.byte	W06
	.byte		        Bn0 , v100
	.byte	W06
	.byte		        Bn0 , v088
	.byte	W06
	.byte		        Bn0 , v100
	.byte	W18
	.byte		        Gn0
	.byte	W12
	.byte	N05
	.byte	W30
	.byte		N02   , Dn2
	.byte	W03
	.byte		        Cn2 , v088
	.byte	W03
	.byte		        Bn1 , v100
	.byte	W03
	.byte		        An1 , v088
	.byte	W03
	.byte		        Gn1 , v100
	.byte	W03
	.byte		        Fn1 , v088
	.byte	W03
	.byte		        En1
	.byte	W03
	.byte		        Dn1 , v076
	.byte	W03
	.byte	PATT
	 ptrvalue	mus_qz_629_3_000
	.byte	PATT
	 ptrvalue	mus_qz_629_3_000
	.byte	PATT
	 ptrvalue	mus_qz_629_3_000
	.byte	PATT
	 ptrvalue	mus_qz_629_3_001
	.byte	PATT
	 ptrvalue	mus_qz_629_3_000
	.byte	PATT
	 ptrvalue	mus_qz_629_3_002
	.byte	PATT
	 ptrvalue	mus_qz_629_3_000
	.byte	PATT
	 ptrvalue	mus_qz_629_3_001
	.byte	PATT
	 ptrvalue	mus_qz_629_3_000
	.byte	PATT
	 ptrvalue	mus_qz_629_3_000
	.byte	PATT
	 ptrvalue	mus_qz_629_3_000
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_629_3_B1
	.byte	FINE

/********************** Track  4 **********************/

mus_qz_629_4:
	.byte		KEYSH , mus_qz_629_key+0
	.byte		VOICE , 5
	.byte		VOL   , 61*mus_qz_629_mvl/mxv
	.byte		PAN   , c_v-54
	.byte		PRIO  , 58
	.byte	W48
mus_qz_629_4_000:
	.byte		N05   , Cn3 , v068
	.byte	W06
	.byte		        Cn4 , v064
	.byte	W06
	.byte		        Gn4 , v072
	.byte	W06
	.byte		        Cn3 , v068
	.byte	W06
	.byte		        Cn4 , v064
	.byte	W06
	.byte		        Gn4 , v072
	.byte	W06
	.byte		        Cn3 , v068
	.byte	W06
	.byte		        Cn4 , v064
	.byte	W06
	.byte		        Gn4 , v072
	.byte	W06
	.byte		        Cn3 , v068
	.byte	W06
	.byte		        Cn4 , v064
	.byte	W06
	.byte		        An4 , v072
	.byte	W06
	.byte		        Cn3 , v068
	.byte	W06
	.byte		        Cn4 , v064
	.byte	W06
	.byte		N11   , An4 , v072
	.byte	W12
	.byte	PEND
mus_qz_629_4_001:
	.byte		N05   , Cn3 , v068
	.byte	W06
	.byte		        Cn4 , v064
	.byte	W06
	.byte		        Gn4 , v072
	.byte	W06
	.byte		        Cn3 , v068
	.byte	W06
	.byte		        Cn4 , v064
	.byte	W06
	.byte		        Gn4 , v072
	.byte	W06
	.byte		        Cn3 , v068
	.byte	W06
	.byte		        Cn4 , v064
	.byte	W06
	.byte		        As4 , v072
	.byte	W06
	.byte		        Cn4 , v068
	.byte	W06
	.byte		        An4 , v072
	.byte	W06
	.byte		        Cn4 , v064
	.byte	W06
	.byte		        Gn4 , v072
	.byte	W06
	.byte		N17   , An4
	.byte	W18
	.byte	PEND
mus_qz_629_4_002:
	.byte		N05   , Cn3 , v068
	.byte	W06
	.byte		        Cn4 , v064
	.byte	W06
	.byte		        Gn4 , v072
	.byte	W06
	.byte		        Cn3 , v068
	.byte	W06
	.byte		        Cn4 , v064
	.byte	W06
	.byte		        Gn4 , v072
	.byte	W06
	.byte		        Cn3 , v068
	.byte	W06
	.byte		        Cn4 , v064
	.byte	W06
	.byte		        Gn4 , v072
	.byte	W06
	.byte		        Cn3 , v068
	.byte	W06
	.byte		        Cn4 , v064
	.byte	W06
	.byte		        An4 , v072
	.byte	W06
	.byte		        Cn3 , v068
	.byte	W06
	.byte		        Cn4 , v064
	.byte	W06
	.byte		        An4 , v072
	.byte	W06
	.byte		        Cn4 , v064
	.byte	W06
	.byte	PEND
mus_qz_629_4_003:
	.byte		N05   , Cn3 , v068
	.byte	W06
	.byte		        Cn4 , v064
	.byte	W06
	.byte		        Gn4 , v072
	.byte	W06
	.byte		        Cn3 , v068
	.byte	W06
	.byte		        Cn4 , v064
	.byte	W06
	.byte		        Gn4 , v072
	.byte	W06
	.byte		        Cn3 , v068
	.byte	W06
	.byte		        Cn4 , v064
	.byte	W06
	.byte		        As4 , v072
	.byte	W06
	.byte		        Cn4 , v068
	.byte	W06
	.byte		        An4 , v072
	.byte	W06
	.byte		        Cn4 , v064
	.byte	W06
	.byte		        Gn4 , v072
	.byte	W06
	.byte		N11   , An4
	.byte	W12
	.byte		N05   , Cn4 , v064
	.byte	W06
	.byte	PEND
mus_qz_629_4_004:
	.byte		N05   , Cn3 , v068
	.byte	W06
	.byte		        Cn4 , v064
	.byte	W06
	.byte		        Gn4 , v072
	.byte	W06
	.byte		        Cn3 , v068
	.byte	W06
	.byte		        Cn4 , v064
	.byte	W06
	.byte		        Gn4 , v072
	.byte	W06
	.byte		        Cn3 , v068
	.byte	W06
	.byte		        Cn4 , v064
	.byte	W06
	.byte		        Gn4 , v072
	.byte	W06
	.byte		        Cn3 , v068
	.byte	W06
	.byte		        Cn4 , v064
	.byte	W06
	.byte		        An4 , v072
	.byte	W06
	.byte		        Cn3 , v068
	.byte	W06
	.byte		        Cn4 , v064
	.byte	W06
	.byte		        An4 , v072
	.byte	W12
	.byte	PEND
mus_qz_629_4_005:
	.byte		N05   , Cn3 , v068
	.byte	W06
	.byte		        Cn4 , v064
	.byte	W06
	.byte		        Gn4 , v072
	.byte	W06
	.byte		        Cn3 , v068
	.byte	W06
	.byte		        Cn4 , v064
	.byte	W06
	.byte		        Gn4 , v072
	.byte	W06
	.byte		        Cn3 , v068
	.byte	W06
	.byte		        Cn4 , v064
	.byte	W06
	.byte		        As4 , v072
	.byte	W06
	.byte		        Cn3 , v068
	.byte	W06
	.byte		        Cn4 , v064
	.byte	W06
	.byte		        Cn5 , v072
	.byte	W06
	.byte		        Cn3 , v068
	.byte	W06
	.byte		        An4 , v072
	.byte	W06
	.byte		        Cn4 , v064
	.byte	W12
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_629_4_004
mus_qz_629_4_006:
	.byte		N05   , Cn3 , v068
	.byte	W06
	.byte		        Cn4 , v064
	.byte	W06
	.byte		        Gn4 , v072
	.byte	W06
	.byte		        Cn3 , v068
	.byte	W06
	.byte		        Cn4 , v064
	.byte	W06
	.byte		        Gn4 , v072
	.byte	W06
	.byte		        Cn3 , v068
	.byte	W06
	.byte		        As4 , v072
	.byte	W06
	.byte		        Cn4 , v064
	.byte	W06
	.byte		        Cn3 , v068
	.byte	W06
	.byte		        Cn5 , v072
	.byte	W06
	.byte		        Cn4 , v064
	.byte	W06
	.byte		        Cn3 , v068
	.byte	W06
	.byte		        An4 , v072
	.byte	W06
	.byte		N11   , Cn4 , v064
	.byte	W12
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_629_4_004
	.byte	PATT
	 ptrvalue	mus_qz_629_4_005
	.byte	PATT
	 ptrvalue	mus_qz_629_4_004
	.byte	W96
mus_qz_629_4_B1:
	.byte	PATT
	 ptrvalue	mus_qz_629_4_004
	.byte	PATT
	 ptrvalue	mus_qz_629_4_001
	.byte	PATT
	 ptrvalue	mus_qz_629_4_002
	.byte		N05   , Cn3 , v068
	.byte	W06
	.byte		        Cn4 , v064
	.byte	W06
	.byte		        Gn4 , v072
	.byte	W06
	.byte		        Cn3 , v068
	.byte	W06
	.byte		        Cn4 , v064
	.byte	W06
	.byte		        Gn4 , v072
	.byte	W06
	.byte		        Cn3 , v068
	.byte	W06
	.byte		        Cn4 , v064
	.byte	W06
	.byte		        As4 , v072
	.byte	W06
	.byte		        Cn4 , v068
	.byte	W06
	.byte		        An4 , v072
	.byte	W06
	.byte		        Cn4 , v064
	.byte	W06
	.byte		        Cn3 , v068
	.byte		N05   , Gn4 , v072
	.byte	W06
	.byte		N11   , An4
	.byte	W12
	.byte		N05   , Cn4 , v064
	.byte	W06
	.byte	PATT
	 ptrvalue	mus_qz_629_4_004
	.byte	PATT
	 ptrvalue	mus_qz_629_4_005
	.byte	PATT
	 ptrvalue	mus_qz_629_4_004
	.byte		N05   , Cn3 , v068
	.byte	W06
	.byte		        Cn4 , v064
	.byte	W06
	.byte		        Gn4 , v072
	.byte	W06
	.byte		        Cn3 , v068
	.byte	W06
	.byte		        Cn4 , v064
	.byte	W06
	.byte		        Gn4 , v072
	.byte	W06
	.byte		        Cn3 , v068
	.byte	W06
	.byte		        Gn4 , v072
	.byte	W06
	.byte		        Cn4 , v064
	.byte	W06
	.byte		        Cn3 , v068
	.byte	W06
	.byte		        Fn4 , v072
	.byte	W06
	.byte		        Cn4 , v064
	.byte	W06
	.byte		        Gn4 , v072
	.byte	W06
	.byte		N11   , Gn4 , v064
	.byte	W12
	.byte		N05   , Cn4
	.byte	W06
	.byte	PATT
	 ptrvalue	mus_qz_629_4_004
	.byte		N05   , Cn3 , v068
	.byte	W06
	.byte		        Cn4 , v064
	.byte	W06
	.byte		        Gn4 , v072
	.byte	W06
	.byte		        Cn3 , v068
	.byte	W06
	.byte		        Cn4 , v064
	.byte	W06
	.byte		        Gn4 , v072
	.byte	W06
	.byte		        Cn3 , v068
	.byte	W06
	.byte		        Cn4 , v064
	.byte	W06
	.byte		        As4 , v072
	.byte	W06
	.byte		        Cn3 , v068
	.byte	W06
	.byte		        Cn4 , v064
	.byte	W06
	.byte		        Cn5 , v072
	.byte	W06
	.byte		        Cn3 , v068
	.byte	W06
	.byte		        An4 , v072
	.byte	W06
	.byte		N11   , Cn4 , v064
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_629_4_004
	.byte		N05   , Cn3 , v068
	.byte	W06
	.byte		        Cn4 , v064
	.byte	W06
	.byte		        Gn4 , v072
	.byte	W06
	.byte		        Cn3 , v068
	.byte	W06
	.byte		        Cn4 , v064
	.byte	W06
	.byte		        Gn4 , v072
	.byte	W06
	.byte		        Cn3 , v068
	.byte	W06
	.byte		        As4 , v072
	.byte	W06
	.byte		        Cn4 , v064
	.byte	W06
	.byte		        Cn3 , v068
	.byte	W06
	.byte		        Cn5 , v072
	.byte	W06
	.byte		        Cn4 , v064
	.byte	W06
	.byte		        Cn3 , v068
	.byte	W06
	.byte		        An4 , v072
	.byte	W06
	.byte		        Ds4 , v112
	.byte	W06
	.byte		        Fn4 , v104
	.byte	W06
	.byte		N32   , Gn4 , v127 , gtp3
	.byte	W36
	.byte		        Fn4 , v124 , gtp3
	.byte	W36
	.byte		N23   , Cn4
	.byte	W24
	.byte		N32   , Dn4 , v124 , gtp3
	.byte	W36
	.byte		        Ds4 , v120 , gtp3
	.byte	W36
	.byte		N23   , Fn4 , v124
	.byte	W24
	.byte		N32   , Dn4 , v124 , gtp3
	.byte	W36
	.byte		        Cn4 , v120 , gtp3
	.byte	W36
	.byte		N23   , As3 , v112
	.byte	W24
	.byte		N44   , An3 , v120 , gtp3
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_629_4_000
	.byte	PATT
	 ptrvalue	mus_qz_629_4_001
	.byte	PATT
	 ptrvalue	mus_qz_629_4_002
	.byte	PATT
	 ptrvalue	mus_qz_629_4_003
	.byte	PATT
	 ptrvalue	mus_qz_629_4_004
	.byte	PATT
	 ptrvalue	mus_qz_629_4_005
	.byte	PATT
	 ptrvalue	mus_qz_629_4_004
	.byte	PATT
	 ptrvalue	mus_qz_629_4_006
	.byte	PATT
	 ptrvalue	mus_qz_629_4_004
	.byte	PATT
	 ptrvalue	mus_qz_629_4_005
	.byte	PATT
	 ptrvalue	mus_qz_629_4_004
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_629_4_B1
	.byte	FINE

/********************** Track  5 **********************/

mus_qz_629_5:
	.byte		KEYSH , mus_qz_629_key+0
	.byte		VOICE , 23
	.byte		VOL   , 63*mus_qz_629_mvl/mxv
	.byte		PAN   , c_v+46
	.byte		PRIO  , 56
	.byte	W48
mus_qz_629_5_000:
	.byte		N05   , Ds3 , v076
	.byte		N05   , Cn4 , v088
	.byte	W18
	.byte		        Ds3 , v064
	.byte		N05   , Cn4 , v072
	.byte	W18
	.byte		        Ds3
	.byte		N05   , Cn4 , v080
	.byte	W12
	.byte		        Ds3 , v076
	.byte		N05   , Cn4 , v088
	.byte	W48
	.byte	PEND
mus_qz_629_5_001:
	.byte		N05   , Ds3 , v076
	.byte		N05   , Cn4 , v088
	.byte	W18
	.byte		        Ds3 , v076
	.byte		N05   , Cn4 , v088
	.byte	W18
	.byte		        Ds3 , v076
	.byte		N05   , Cn4 , v088
	.byte	W12
	.byte		        Cn3
	.byte		N05   , Cn4 , v092
	.byte	W06
	.byte		        Gn2 , v088
	.byte		N05   , Gn3 , v092
	.byte	W06
	.byte		        Cn3 , v088
	.byte		N05   , Cn4 , v092
	.byte	W06
	.byte		        Fn3 , v088
	.byte		N05   , Fn4 , v092
	.byte	W06
	.byte		        Cn3 , v088
	.byte		N05   , Cn4 , v092
	.byte	W06
	.byte		        An3 , v088
	.byte		N05   , An4 , v092
	.byte	W06
	.byte		        Cn4
	.byte		N05   , Cn5 , v100
	.byte	W06
	.byte		        Fn4 , v092
	.byte		N05   , Fn5 , v100
	.byte	W06
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_629_5_000
mus_qz_629_5_002:
	.byte		N05   , Ds3 , v076
	.byte		N05   , Cn4 , v088
	.byte	W18
	.byte		        Ds3 , v064
	.byte		N05   , Cn4 , v072
	.byte	W18
	.byte		        Ds3
	.byte		N05   , Cn4 , v080
	.byte	W12
	.byte		        Cn3 , v088
	.byte		N05   , Cn4 , v092
	.byte	W06
	.byte		        Gn2 , v088
	.byte		N05   , Gn3 , v092
	.byte	W06
	.byte		        Cn3 , v088
	.byte		N05   , Cn4 , v092
	.byte	W06
	.byte		        Fn3 , v088
	.byte		N05   , Fn4 , v092
	.byte	W06
	.byte		        Cn3 , v088
	.byte		N05   , Cn4 , v092
	.byte	W06
	.byte		        An3 , v088
	.byte		N05   , An4 , v092
	.byte	W06
	.byte		        Cn4
	.byte		N05   , Cn5 , v100
	.byte	W06
	.byte		        Fn4 , v092
	.byte		N05   , Fn5 , v100
	.byte	W06
	.byte	PEND
mus_qz_629_5_003:
	.byte		N05   , Ds3 , v076
	.byte		N05   , Cn4 , v088
	.byte	W18
	.byte		        Ds3 , v064
	.byte		N05   , Cn4 , v072
	.byte	W18
	.byte		        Ds3
	.byte		N05   , Cn4 , v080
	.byte	W12
	.byte		        Ds3 , v076
	.byte		N05   , Cn4 , v088
	.byte	W12
	.byte		        Ds3 , v076
	.byte		N05   , Cn4 , v088
	.byte	W12
	.byte		        Ds3 , v076
	.byte		N05   , Dn4 , v088
	.byte	W12
	.byte		        Ds3 , v076
	.byte		N05   , Ds4 , v088
	.byte	W12
	.byte	PEND
mus_qz_629_5_004:
	.byte		N05   , Ds3 , v076
	.byte		N05   , Cn4 , v088
	.byte	W18
	.byte		        Ds3 , v076
	.byte		N05   , Cn4 , v088
	.byte	W18
	.byte		        Ds3 , v076
	.byte		N05   , Cn4 , v088
	.byte	W12
	.byte		        Ds3 , v076
	.byte		N05   , Cn4 , v088
	.byte	W12
	.byte		        Ds3 , v076
	.byte		N05   , Cn4 , v088
	.byte	W12
	.byte		        Ds3 , v076
	.byte		N05   , Dn4 , v088
	.byte	W12
	.byte		        Ds3 , v076
	.byte		N05   , As3 , v088
	.byte	W12
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_629_5_003
	.byte		N05   , Ds3 , v076
	.byte		N05   , Cn4 , v088
	.byte	W18
	.byte		        Ds3 , v064
	.byte		N05   , Cn4 , v072
	.byte	W18
	.byte		        Ds3
	.byte		N05   , Cn4 , v080
	.byte	W12
	.byte		        Ds3 , v076
	.byte		N05   , Cn4 , v088
	.byte	W12
	.byte		        Ds3 , v076
	.byte		N05   , Cn4 , v088
	.byte	W12
	.byte		        Fn3 , v076
	.byte		N05   , Dn4 , v088
	.byte	W12
	.byte		        Ds3 , v076
	.byte		N05   , As3 , v088
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_629_5_003
mus_qz_629_5_005:
	.byte		N05   , Ds3 , v076
	.byte		N05   , Cn4 , v088
	.byte	W18
	.byte		        Ds3 , v076
	.byte		N05   , Cn4 , v088
	.byte	W18
	.byte		        Ds3 , v076
	.byte		N05   , Cn4 , v088
	.byte	W12
	.byte		        Ds3 , v076
	.byte		N05   , Cn4 , v088
	.byte	W12
	.byte		        Ds3 , v076
	.byte		N05   , Cn4 , v088
	.byte	W12
	.byte		        Fn3 , v076
	.byte		N05   , Dn4 , v088
	.byte	W12
	.byte		        Dn3 , v076
	.byte		N05   , As3 , v088
	.byte	W12
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_629_5_003
	.byte	W96
mus_qz_629_5_B1:
	.byte	PATT
	 ptrvalue	mus_qz_629_5_000
	.byte		N05   , Ds3 , v076
	.byte		N05   , Cn4 , v088
	.byte	W18
	.byte		        Ds3 , v076
	.byte		N05   , Cn4 , v088
	.byte	W18
	.byte		        Ds3 , v076
	.byte		N05   , Cn4 , v088
	.byte	W12
	.byte		        Cn3
	.byte		N05   , Cn4 , v092
	.byte	W06
	.byte		        Gn2 , v088
	.byte		N05   , Gn3 , v092
	.byte	W06
	.byte		        Cn3 , v088
	.byte		N05   , Cn4 , v092
	.byte	W06
	.byte		        Fn3 , v088
	.byte		N05   , Fn4 , v092
	.byte	W06
	.byte		        Cn3 , v088
	.byte		N05   , Cn4 , v092
	.byte	W06
	.byte		        An3 , v088
	.byte		N05   , An4 , v092
	.byte	W06
	.byte		        Cn4 , v088
	.byte		N05   , Cn5 , v092
	.byte	W06
	.byte		        Fn4 , v088
	.byte		N05   , Fn5 , v092
	.byte	W06
	.byte	PATT
	 ptrvalue	mus_qz_629_5_000
	.byte		N05   , Ds3 , v076
	.byte		N05   , Cn4 , v088
	.byte	W18
	.byte		        Ds3 , v064
	.byte		N05   , Cn4 , v072
	.byte	W18
	.byte		        Ds3
	.byte		N05   , Cn4 , v080
	.byte	W12
	.byte		        Cn3 , v088
	.byte		N05   , Cn4 , v092
	.byte	W06
	.byte		        Gn2 , v088
	.byte		N05   , Gn3 , v092
	.byte	W06
	.byte		        Cn3 , v088
	.byte		N05   , Cn4 , v092
	.byte	W06
	.byte		        Fn3 , v088
	.byte		N05   , Fn4 , v092
	.byte	W06
	.byte		        Cn3 , v088
	.byte		N05   , Cn4 , v092
	.byte	W06
	.byte		        An3 , v088
	.byte		N05   , An4 , v092
	.byte	W06
	.byte		        Cn4 , v088
	.byte		N05   , Cn5 , v092
	.byte	W06
	.byte		        Fn4 , v088
	.byte		N05   , Fn5 , v092
	.byte	W06
	.byte	PATT
	 ptrvalue	mus_qz_629_5_000
	.byte		N05   , Ds3 , v076
	.byte		N05   , Cn4 , v088
	.byte	W18
	.byte		        Ds3 , v076
	.byte		N05   , Cn4 , v088
	.byte	W18
	.byte		        Ds3 , v076
	.byte		N05   , Cn4 , v088
	.byte	W12
	.byte		        Gn3 , v076
	.byte		N05   , Ds4 , v088
	.byte	W12
	.byte		        Fn3 , v076
	.byte		N05   , Dn4 , v088
	.byte	W12
	.byte		        Ds3 , v076
	.byte		N05   , Cn4 , v088
	.byte	W06
	.byte		N14   , Fn3 , v076
	.byte		N14   , Dn4 , v088
	.byte	W18
	.byte	PATT
	 ptrvalue	mus_qz_629_5_000
	.byte		N05   , Ds3 , v076
	.byte		N05   , Cn4 , v088
	.byte	W18
	.byte		        Ds3 , v064
	.byte		N05   , Cn4 , v072
	.byte	W18
	.byte		        Ds3
	.byte		N05   , Cn4 , v080
	.byte	W12
	.byte		        Gn3 , v076
	.byte		N05   , Ds4 , v088
	.byte	W12
	.byte		        Fn3 , v076
	.byte		N05   , Dn4 , v088
	.byte	W12
	.byte		        Gn3 , v076
	.byte		N05   , Ds4 , v088
	.byte	W06
	.byte		N14   , As3 , v068
	.byte		N14   , Fn4 , v088
	.byte	W18
	.byte	PATT
	 ptrvalue	mus_qz_629_5_000
	.byte		N05   , Ds3 , v076
	.byte		N05   , Cn4 , v088
	.byte	W18
	.byte		        Ds3 , v076
	.byte		N05   , Cn4 , v088
	.byte	W18
	.byte		        Gn3 , v072
	.byte		N05   , Ds4 , v084
	.byte	W12
	.byte		        Gn3 , v076
	.byte		N05   , Ds4 , v088
	.byte	W12
	.byte		        Fn3 , v076
	.byte		N05   , Dn4 , v088
	.byte	W12
	.byte		        Ds3 , v076
	.byte		N05   , Cn4 , v088
	.byte	W06
	.byte		N11   , Fn3 , v076
	.byte		N11   , Dn4 , v088
	.byte	W18
	.byte	PATT
	 ptrvalue	mus_qz_629_5_000
	.byte		N05   , Ds3 , v076
	.byte		N05   , Cn4 , v088
	.byte	W18
	.byte		        Ds3 , v064
	.byte		N05   , Cn4 , v072
	.byte	W12
	.byte		        Gn3 , v060
	.byte		N05   , Ds4 , v072
	.byte	W12
	.byte		N02   , Gn3 , v020
	.byte		N02   , Ds4 , v028
	.byte	W06
	.byte		N11   , Gn3 , v072
	.byte		N11   , Ds4 , v080
	.byte	W12
	.byte		        Fn3 , v072
	.byte		N11   , Dn4 , v080
	.byte	W12
	.byte		N03   , As2 , v068
	.byte		N03   , Gn3 , v080
	.byte	W03
	.byte		        Cn3 , v068
	.byte		N03   , Gs3 , v080
	.byte	W03
	.byte		        Dn3
	.byte		N03   , As3 , v092
	.byte	W04
	.byte		        Ds3 , v080
	.byte		N03   , Cn4 , v092
	.byte	W03
	.byte		        Fn3
	.byte		N03   , Dn4 , v108
	.byte	W04
	.byte		        Gn3 , v092
	.byte		N03   , Ds4 , v108
	.byte	W03
	.byte		        An3
	.byte		N03   , Fn4 , v124
	.byte	W04
	.byte		N32   , As3 , v092 , gtp3
	.byte		N32   , Gn4 , v108 , gtp3
	.byte	W36
	.byte		        Gs3 , v092 , gtp3
	.byte		N32   , Fn4 , v108 , gtp3
	.byte	W36
	.byte		N23   , Fn3 , v092
	.byte		N23   , Cn4 , v108
	.byte	W24
	.byte		N32   , Fn3 , v092 , gtp3
	.byte		N32   , Dn4 , v108 , gtp3
	.byte	W36
	.byte		        Gn3 , v092 , gtp3
	.byte		N32   , Ds4 , v108 , gtp3
	.byte	W36
	.byte		N23   , As3 , v092
	.byte		N23   , Fn4 , v108
	.byte	W24
	.byte		N32   , Fn3 , v092 , gtp3
	.byte		N32   , Dn4 , v108 , gtp3
	.byte	W36
	.byte		        Ds3 , v092 , gtp3
	.byte		N32   , Cn4 , v108 , gtp3
	.byte	W36
	.byte		N23   , Dn3 , v092
	.byte		N23   , As3 , v108
	.byte	W24
	.byte		N56   , Cn3 , v092 , gtp3
	.byte		N56   , An3 , v108 , gtp3
	.byte	W36
	.byte		VOL   , 46*mus_qz_629_mvl/mxv
	.byte	W03
	.byte		        35*mus_qz_629_mvl/mxv
	.byte	W03
	.byte		        30*mus_qz_629_mvl/mxv
	.byte	W03
	.byte		        22*mus_qz_629_mvl/mxv
	.byte	W03
	.byte		        18*mus_qz_629_mvl/mxv
	.byte	W03
	.byte		        13*mus_qz_629_mvl/mxv
	.byte	W03
	.byte		        9*mus_qz_629_mvl/mxv
	.byte	W42
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W90
	.byte		        63*mus_qz_629_mvl/mxv
	.byte	W06
	.byte	PATT
	 ptrvalue	mus_qz_629_5_000
	.byte	PATT
	 ptrvalue	mus_qz_629_5_001
	.byte	PATT
	 ptrvalue	mus_qz_629_5_000
	.byte	PATT
	 ptrvalue	mus_qz_629_5_002
	.byte	PATT
	 ptrvalue	mus_qz_629_5_003
	.byte	PATT
	 ptrvalue	mus_qz_629_5_004
	.byte	PATT
	 ptrvalue	mus_qz_629_5_003
	.byte		N05   , Ds3 , v076
	.byte		N05   , Cn4 , v088
	.byte	W18
	.byte		        Ds3 , v064
	.byte		N05   , Cn4 , v072
	.byte	W18
	.byte		        Ds3
	.byte		N05   , Cn4 , v080
	.byte	W12
	.byte		        Ds3 , v076
	.byte		N05   , Cn4 , v088
	.byte	W12
	.byte		        Ds3 , v076
	.byte		N05   , Cn4 , v088
	.byte	W12
	.byte		        En3 , v076
	.byte		N05   , Dn4 , v088
	.byte	W12
	.byte		        Ds3 , v076
	.byte		N05   , As3 , v088
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_629_5_003
	.byte	PATT
	 ptrvalue	mus_qz_629_5_005
	.byte	PATT
	 ptrvalue	mus_qz_629_5_003
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_629_5_B1
	.byte	FINE

/********************** Track  6 **********************/

mus_qz_629_6:
	.byte		KEYSH , mus_qz_629_key+0
	.byte		VOICE , 39
	.byte		VOL   , 84*mus_qz_629_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 40
	.byte		N05   , Cn1 , v088
	.byte		N03   , En1 , v056
	.byte		N23   , As2 , v100
	.byte	W04
	.byte		N03   , En1 , v068
	.byte	W04
	.byte		        En1 , v080
	.byte	W04
	.byte		N05   , En1 , v092
	.byte	W06
	.byte		        En1 , v108
	.byte	W06
	.byte		        Cn1 , v116
	.byte		N02   , Bn1 , v100
	.byte	W03
	.byte		        Bn1 , v068
	.byte	W03
	.byte		N05   , Bn1 , v092
	.byte	W06
	.byte		N03   , Cn1 , v116
	.byte		N05   , An1 , v072
	.byte	W06
	.byte		N03   , Cn1 , v116
	.byte		N05   , Fn1 , v092
	.byte	W06
mus_qz_629_6_000:
	.byte		N05   , Cn1 , v116
	.byte		N23   , Cs2 , v100
	.byte	W18
	.byte		N05   , Cn1 , v084
	.byte	W18
	.byte		        Cn1 , v104
	.byte	W12
	.byte		        Cn1 , v116
	.byte	W24
	.byte		N11   , En1 , v100
	.byte	W24
	.byte	PEND
mus_qz_629_6_001:
	.byte		N05   , Cn1 , v116
	.byte	W18
	.byte		        Cn1 , v084
	.byte	W18
	.byte		        Cn1 , v116
	.byte	W12
	.byte	N05
	.byte	W24
	.byte		N11   , En1 , v100
	.byte	W18
	.byte		N05   , Cn1 , v084
	.byte	W06
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_629_6_000
	.byte		N05   , Cn1 , v116
	.byte	W06
	.byte		        Cn1 , v084
	.byte	W06
	.byte		        En1
	.byte	W06
	.byte		        Cn1
	.byte	W12
	.byte		N11   , En1
	.byte	W12
	.byte		N05   , Cn1
	.byte	W06
	.byte		        Cn1 , v116
	.byte	W06
	.byte		        Cn1 , v084
	.byte	W06
	.byte		N11   , En1 , v100
	.byte	W12
	.byte		N05   , Cn1 , v116
	.byte	W06
	.byte		        Cn1 , v084
	.byte	W06
	.byte		        An1 , v092
	.byte	W06
	.byte		        Fn1 , v084
	.byte	W06
	.byte	PATT
	 ptrvalue	mus_qz_629_6_000
	.byte		N05   , Cn1 , v116
	.byte	W06
	.byte		        Cn1 , v084
	.byte	W06
	.byte		N11   , En1 , v072
	.byte	W12
	.byte		N05   , Cn1 , v088
	.byte	W06
	.byte		        Cn1 , v084
	.byte	W06
	.byte		N11   , En1 , v072
	.byte	W12
	.byte		N05   , Cn1 , v116
	.byte	W06
	.byte		N11   , En1 , v084
	.byte	W12
	.byte		N05   , Cn1
	.byte	W06
	.byte		        Cn1 , v116
	.byte	W06
	.byte		N11   , En1 , v100
	.byte	W12
	.byte		N05   , Cn1 , v084
	.byte	W06
	.byte		        Cn1 , v116
	.byte	W18
	.byte		        Cn1 , v084
	.byte	W18
	.byte		        Cn1 , v104
	.byte	W12
	.byte		        Cn1 , v116
	.byte	W24
	.byte		N11   , En1 , v100
	.byte	W18
	.byte		N05   , Cn1 , v084
	.byte	W06
	.byte		        Cn1 , v116
	.byte	W06
	.byte		N11   , En1 , v072
	.byte	W12
	.byte		N05   , Cn1 , v084
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		N11   , En1 , v068
	.byte	W12
	.byte		N05   , Cn1 , v084
	.byte	W06
	.byte		        Cn1 , v116
	.byte	W06
	.byte		N11   , En1 , v080
	.byte	W12
	.byte		N05   , Cn1 , v084
	.byte	W06
	.byte		        Cn1 , v116
	.byte	W06
	.byte		        Cn1 , v084
	.byte	W06
	.byte		        Cn1 , v092
	.byte	W06
	.byte		        Cn1 , v084
	.byte	W06
	.byte	PATT
	 ptrvalue	mus_qz_629_6_000
	.byte		N05   , Cn1 , v116
	.byte	W06
	.byte		        Cn1 , v084
	.byte	W06
	.byte		N11   , En1 , v068
	.byte	W12
	.byte		N05   , Cn1 , v084
	.byte	W06
	.byte		N11   , En1 , v068
	.byte	W12
	.byte		N05   , Cn1 , v084
	.byte	W06
	.byte		        Cn1 , v116
	.byte	W06
	.byte		N11   , En1 , v080
	.byte	W12
	.byte		N05   , Cn1 , v084
	.byte	W06
	.byte		        Cn1 , v116
	.byte	W12
	.byte		        Gn1 , v100
	.byte	W06
	.byte		        Cn1 , v084
	.byte		N05   , Fn1 , v100
	.byte	W06
mus_qz_629_6_002:
	.byte		N05   , Cn1 , v116
	.byte	W18
	.byte		        Cn1 , v084
	.byte	W18
	.byte		        Cn1 , v104
	.byte	W12
	.byte		        Cn1 , v116
	.byte		N03   , En1 , v044
	.byte	W04
	.byte		        En1 , v056
	.byte	W04
	.byte		        En1 , v068
	.byte	W04
	.byte		N05   , En1 , v088
	.byte	W06
	.byte		        En1 , v100
	.byte	W06
	.byte		N02   , Bn1
	.byte	W03
	.byte		        Bn1 , v068
	.byte	W03
	.byte		N05   , Bn1 , v100
	.byte	W06
	.byte		        An1 , v072
	.byte	W06
	.byte		        Fn1 , v100
	.byte	W06
	.byte	PEND
	.byte		N11   , Cn1 , v116
	.byte		N23   , Cs2 , v100
	.byte	W72
	.byte		N02   , En1
	.byte	W03
	.byte		        En1 , v068
	.byte	W03
	.byte		N05   , En1 , v092
	.byte	W05
	.byte		        An1
	.byte	W07
	.byte		        Gn1
	.byte	W06
mus_qz_629_6_B1:
	.byte		N05   , Cn1 , v116
	.byte		N23   , Cs2 , v100
	.byte	W18
	.byte		N05   , Cn1 , v088
	.byte	W06
	.byte		N11   , En1 , v100
	.byte	W12
	.byte		        As1 , v072
	.byte	W06
	.byte		N05   , Cn1 , v088
	.byte	W06
	.byte		        Cn1 , v116
	.byte	W12
	.byte		N11   , As1 , v072
	.byte	W12
	.byte		        En1 , v100
	.byte	W12
	.byte		        As1 , v072
	.byte	W06
	.byte		N05   , Cn1 , v088
	.byte	W06
	.byte	PEND
mus_qz_629_6_003:
	.byte		N05   , Cn1 , v116
	.byte	W12
	.byte		N11   , As1 , v072
	.byte	W06
	.byte		N05   , Cn1 , v088
	.byte	W06
	.byte		N11   , En1 , v100
	.byte	W12
	.byte		        As1 , v072
	.byte	W06
	.byte		N05   , Cn1 , v088
	.byte	W06
	.byte		        Cn1 , v116
	.byte	W12
	.byte		N11   , As1 , v072
	.byte	W12
	.byte		        En1 , v100
	.byte	W12
	.byte		        As1 , v072
	.byte	W06
	.byte		N05   , Cn1 , v088
	.byte	W06
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_629_6_003
	.byte		N05   , Cn1 , v116
	.byte	W12
	.byte		N11   , As1 , v072
	.byte	W06
	.byte		N05   , Cn1 , v088
	.byte	W06
	.byte		N11   , En1 , v100
	.byte	W12
	.byte		        As1 , v072
	.byte	W06
	.byte		N05   , Cn1 , v088
	.byte	W06
	.byte		        Cn1 , v116
	.byte	W12
	.byte		N11   , As1 , v072
	.byte	W12
	.byte		N05   , En1 , v100
	.byte	W06
	.byte		        En1 , v076
	.byte	W06
	.byte		        En1 , v088
	.byte	W06
	.byte		        Cn1
	.byte		N05   , En1 , v076
	.byte	W06
	.byte	PATT
	 ptrvalue	mus_qz_629_6_B1
	.byte	PATT
	 ptrvalue	mus_qz_629_6_003
	.byte	PATT
	 ptrvalue	mus_qz_629_6_003
	.byte		N05   , Cn1 , v116
	.byte	W12
	.byte		N11   , As1 , v072
	.byte	W06
	.byte		N05   , Cn1 , v088
	.byte	W06
	.byte		N11   , En1 , v100
	.byte	W12
	.byte		        As1 , v072
	.byte	W06
	.byte		N05   , Cn1 , v088
	.byte	W06
	.byte		        Cn1 , v116
	.byte	W12
	.byte		N11   , As1 , v072
	.byte	W12
	.byte		        En1 , v100
	.byte	W12
	.byte		N05   , Gn1
	.byte	W06
	.byte		        Cn1 , v088
	.byte		N05   , Fn1 , v100
	.byte	W06
	.byte	PATT
	 ptrvalue	mus_qz_629_6_B1
	.byte	PATT
	 ptrvalue	mus_qz_629_6_003
	.byte	PATT
	 ptrvalue	mus_qz_629_6_003
	.byte		N05   , Cn1 , v116
	.byte	W12
	.byte		N11   , As1 , v072
	.byte	W06
	.byte		N05   , Cn1 , v088
	.byte	W06
	.byte		N11   , En1 , v100
	.byte	W12
	.byte		        As1 , v072
	.byte	W06
	.byte		N05   , Cn1 , v088
	.byte	W06
	.byte		        Cn1 , v116
	.byte	W06
	.byte		N02   , En1 , v060
	.byte	W03
	.byte		        En1 , v088
	.byte	W03
	.byte		N05   , En1 , v100
	.byte		N11   , As1 , v072
	.byte	W06
	.byte		N05   , En1 , v088
	.byte	W06
	.byte		        An1 , v100
	.byte	W06
	.byte		        An1 , v084
	.byte	W06
	.byte		        Fn1 , v100
	.byte	W06
	.byte		        Cn1 , v088
	.byte		N05   , Fn1 , v072
	.byte	W06
	.byte		        Cn1 , v116
	.byte		N23   , Cs2 , v100
	.byte	W18
	.byte		N05   , Cn1
	.byte	W06
	.byte		N11   , En1
	.byte	W06
	.byte		N05   , Cn1 , v116
	.byte	W06
	.byte		N11   , As1 , v080
	.byte	W06
	.byte		N05   , Cn1 , v100
	.byte	W06
	.byte		        Cn1 , v116
	.byte	W06
	.byte		        En1 , v084
	.byte	W06
	.byte		N11   , As1 , v080
	.byte	W06
	.byte		N05   , Cn1 , v100
	.byte	W06
	.byte		N11   , En1
	.byte	W12
	.byte		        As1 , v080
	.byte	W12
mus_qz_629_6_004:
	.byte		N05   , Cn1 , v116
	.byte	W12
	.byte		N11   , As1 , v080
	.byte	W06
	.byte		N05   , Cn1 , v100
	.byte	W06
	.byte		N11   , En1
	.byte	W06
	.byte		N05   , Cn1 , v116
	.byte	W06
	.byte		N11   , As1 , v080
	.byte	W06
	.byte		N05   , Cn1 , v100
	.byte	W06
	.byte		        Cn1 , v116
	.byte	W06
	.byte		        En1 , v084
	.byte	W06
	.byte		N11   , As1 , v080
	.byte	W06
	.byte		N05   , Cn1 , v100
	.byte	W06
	.byte		N11   , En1
	.byte	W12
	.byte		        As1 , v080
	.byte	W12
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_629_6_004
	.byte		N05   , Cn1 , v116
	.byte	W12
	.byte		N11   , As1 , v080
	.byte	W06
	.byte		N05   , Cn1 , v100
	.byte	W06
	.byte		N11   , En1
	.byte	W06
	.byte		N05   , Cn1 , v116
	.byte	W06
	.byte		N11   , As1 , v080
	.byte	W06
	.byte		N05   , Cn1 , v100
	.byte	W06
	.byte		        Cn1 , v116
	.byte	W06
	.byte		        En1 , v084
	.byte	W06
	.byte		N11   , As1 , v080
	.byte	W06
	.byte		N05   , Cn1 , v100
	.byte	W06
	.byte		        En1
	.byte	W12
	.byte		        En1 , v084
	.byte		N11   , As1 , v080
	.byte	W06
	.byte		N05   , En1 , v100
	.byte	W06
	.byte		        Cn1 , v116
	.byte	W12
	.byte		N11   , As1 , v080
	.byte	W06
	.byte		N05   , Cn1 , v100
	.byte	W06
	.byte		N11   , En1
	.byte	W06
	.byte		N05   , Cn1 , v116
	.byte	W06
	.byte		N11   , As1 , v080
	.byte	W06
	.byte		N05   , Cn1 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        En1 , v084
	.byte	W06
	.byte		N11   , As1 , v080
	.byte	W06
	.byte		N05   , Cn1 , v100
	.byte	W06
	.byte		N11   , En1
	.byte	W12
	.byte		        As1 , v080
	.byte	W12
	.byte		N05   , Cn1 , v100
	.byte	W12
	.byte		N11   , As1 , v080
	.byte	W06
	.byte		N05   , Cn1 , v100
	.byte	W06
	.byte		N11   , En1
	.byte	W06
	.byte		N05   , Cn1
	.byte	W06
	.byte		N11   , As1 , v080
	.byte	W06
	.byte		N05   , Cn1 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        En1 , v084
	.byte	W06
	.byte		N11   , As1 , v080
	.byte	W06
	.byte		N05   , Cn1 , v100
	.byte	W06
	.byte		N11   , En1
	.byte	W12
	.byte		        As1 , v080
	.byte	W12
	.byte		N05   , Cn1 , v100
	.byte	W12
	.byte		N11   , As1 , v080
	.byte	W06
	.byte		N05   , Cn1 , v100
	.byte	W06
	.byte		N11   , En1
	.byte	W06
	.byte		N05   , Cn1
	.byte	W06
	.byte		N11   , As1 , v080
	.byte	W06
	.byte		N05   , Cn1 , v100
	.byte	W06
	.byte		        Cn1 , v088
	.byte		N03   , En1 , v060
	.byte	W04
	.byte		        En1 , v072
	.byte	W04
	.byte		        En1 , v088
	.byte	W04
	.byte		N05   , En1 , v100
	.byte	W06
	.byte		        En1 , v116
	.byte	W06
	.byte		        Cn1
	.byte		N02   , Bn1 , v100
	.byte	W03
	.byte		        Bn1 , v068
	.byte	W03
	.byte		N05   , Bn1 , v100
	.byte	W06
	.byte		N03   , Cn1 , v116
	.byte		N05   , An1 , v072
	.byte	W06
	.byte		N03   , Cn1 , v116
	.byte		N05   , Fn1 , v100
	.byte	W06
	.byte		        En1
	.byte		N17   , Cs2
	.byte	W06
	.byte		N05   , En1
	.byte	W06
	.byte	N05
	.byte	W18
	.byte		        Cn1 , v116
	.byte		N11   , As1 , v100
	.byte	W12
	.byte		N05   , Cn1 , v116
	.byte		N05   , As1 , v100
	.byte	W30
	.byte		        En1
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Cn1 , v116
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_629_6_000
	.byte	PATT
	 ptrvalue	mus_qz_629_6_001
	.byte	PATT
	 ptrvalue	mus_qz_629_6_000
mus_qz_629_6_005:
	.byte		N05   , Cn1 , v116
	.byte	W06
	.byte		        Cn1 , v084
	.byte	W06
	.byte		N11   , En1 , v072
	.byte	W12
	.byte		N05   , Cn1 , v088
	.byte	W06
	.byte		N11   , En1 , v084
	.byte	W12
	.byte		N05   , Cn1
	.byte	W06
	.byte		        Cn1 , v116
	.byte	W06
	.byte		        Cn1 , v084
	.byte	W06
	.byte		N11   , En1 , v100
	.byte	W12
	.byte		N05   , Cn1 , v084
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		N11   , En1 , v100
	.byte	W06
	.byte		N05   , Cn1 , v084
	.byte	W06
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_629_6_000
	.byte		N05   , Cn1 , v116
	.byte	W06
	.byte		        Cn1 , v084
	.byte	W06
	.byte		N11   , En1 , v072
	.byte	W12
	.byte		N05   , Cn1 , v084
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		N11   , En1 , v072
	.byte	W12
	.byte		N05   , Cn1 , v116
	.byte	W06
	.byte		N11   , En1 , v084
	.byte	W12
	.byte		N05   , Cn1
	.byte	W06
	.byte		        Cn1 , v116
	.byte	W06
	.byte		N11   , En1 , v100
	.byte	W12
	.byte		N05   , Cn1 , v084
	.byte	W06
	.byte		        Cn1 , v116
	.byte	W18
	.byte		        Cn1 , v084
	.byte	W18
	.byte		        Cn1 , v104
	.byte	W12
	.byte		        Cn1 , v116
	.byte	W24
	.byte		N11   , En1 , v100
	.byte	W24
	.byte	PATT
	 ptrvalue	mus_qz_629_6_005
	.byte	PATT
	 ptrvalue	mus_qz_629_6_000
	.byte		N05   , Cn1 , v116
	.byte	W06
	.byte		        Cn1 , v084
	.byte	W06
	.byte		N11   , En1 , v072
	.byte	W12
	.byte		N05   , Cn1 , v084
	.byte	W06
	.byte		N11   , En1 , v072
	.byte	W12
	.byte		N05   , Cn1 , v084
	.byte	W06
	.byte		        Cn1 , v116
	.byte	W06
	.byte		N11   , En1 , v092
	.byte	W12
	.byte		N05   , Cn1 , v084
	.byte	W06
	.byte		        Cn1 , v116
	.byte	W12
	.byte		        Gn1 , v100
	.byte	W06
	.byte		        Cn1 , v084
	.byte		N05   , Fn1 , v100
	.byte	W06
	.byte	PATT
	 ptrvalue	mus_qz_629_6_002
	.byte		N11   , Cn1 , v116
	.byte		N23   , Cs2 , v100
	.byte	W72
	.byte		N02   , En1
	.byte	W03
	.byte		        En1 , v068
	.byte	W03
	.byte		N05   , En1 , v100
	.byte	W06
	.byte		        An1 , v072
	.byte	W06
	.byte		        Gn1 , v100
	.byte	W06
	.byte	GOTO
	 ptrvalue	mus_qz_629_6_B1
	.byte	FINE

/********************** Track  7 **********************/

mus_qz_629_7:
	.byte		KEYSH , mus_qz_629_key+0
	.byte		VOICE , 5
	.byte		VOL   , 19*mus_qz_629_mvl/mxv
	.byte		PAN   , c_v+56
	.byte		PRIO  , 40
	.byte	W48
mus_qz_629_7_000:
	.byte	W24
	.byte		N05   , Cn3 , v076
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        Gn4 , v080
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        Gn4 , v080
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        Gn4 , v080
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        An4 , v080
	.byte	W06
	.byte	PEND
mus_qz_629_7_001:
	.byte		N05   , Cn3 , v076
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		N11   , An4 , v080
	.byte	W12
	.byte		N05   , Cn3 , v076
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        Gn4 , v080
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        Gn4 , v080
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        As4 , v080
	.byte	W06
	.byte		        Cn4 , v076
	.byte	W06
	.byte		        An4 , v080
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte	PEND
mus_qz_629_7_002:
	.byte		N05   , Gn4 , v080
	.byte	W06
	.byte		N17   , An4
	.byte	W18
	.byte		N05   , Cn3 , v076
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        Gn4 , v080
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        Gn4 , v080
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        Gn4 , v080
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        An4 , v080
	.byte	W06
	.byte	PEND
mus_qz_629_7_003:
	.byte		N05   , Cn3 , v076
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        An4 , v080
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        Gn4 , v080
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        Gn4 , v080
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        As4 , v080
	.byte	W06
	.byte		        Cn4 , v076
	.byte	W06
	.byte		        An4 , v080
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte	PEND
mus_qz_629_7_004:
	.byte		N05   , Gn4 , v080
	.byte	W06
	.byte		N11   , An4
	.byte	W12
	.byte		N05   , Cn4 , v072
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        Gn4 , v080
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        Gn4 , v080
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        Gn4 , v080
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        An4 , v080
	.byte	W06
	.byte	PEND
mus_qz_629_7_005:
	.byte		N05   , Cn3 , v076
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        An4 , v080
	.byte	W12
	.byte		        Cn3 , v076
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        Gn4 , v080
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        Gn4 , v080
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        As4 , v080
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        Cn5 , v080
	.byte	W06
	.byte	PEND
mus_qz_629_7_006:
	.byte		N05   , Cn3 , v076
	.byte	W06
	.byte		        An4 , v080
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W12
	.byte		        Cn3 , v076
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        Gn4 , v080
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        Gn4 , v080
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        Gn4 , v080
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        An4 , v080
	.byte	W06
	.byte	PEND
mus_qz_629_7_007:
	.byte		N05   , Cn3 , v076
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        An4 , v080
	.byte	W12
	.byte		        Cn3 , v076
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        Gn4 , v080
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        Gn4 , v080
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W06
	.byte		        As4 , v080
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W06
	.byte		        Cn5 , v080
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte	PEND
mus_qz_629_7_008:
	.byte		N05   , Cn3 , v076
	.byte	W06
	.byte		        An4 , v080
	.byte	W06
	.byte		N11   , Cn4 , v072
	.byte	W12
	.byte		N05   , Cn3 , v076
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        Gn4 , v080
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        Gn4 , v080
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        Gn4 , v080
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        An4 , v080
	.byte	W06
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_629_7_005
	.byte	PATT
	 ptrvalue	mus_qz_629_7_006
	.byte	W96
mus_qz_629_7_B1:
	.byte	PATT
	 ptrvalue	mus_qz_629_7_000
	.byte		N05   , Cn3 , v076
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        An4 , v080
	.byte	W12
	.byte		        Cn3 , v076
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        Gn4 , v080
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        Gn4 , v080
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        As4 , v080
	.byte	W06
	.byte		        Cn4 , v076
	.byte	W06
	.byte		        An4 , v080
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte	PATT
	 ptrvalue	mus_qz_629_7_002
	.byte	PATT
	 ptrvalue	mus_qz_629_7_003
	.byte		N05   , Cn3 , v076
	.byte		N05   , Gn4 , v080
	.byte	W06
	.byte		N11   , An4
	.byte	W12
	.byte		N05   , Cn4 , v072
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        Gn4 , v080
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        Gn4 , v080
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        Gn4 , v080
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        An4 , v080
	.byte	W06
	.byte	PATT
	 ptrvalue	mus_qz_629_7_005
	.byte	PATT
	 ptrvalue	mus_qz_629_7_006
	.byte		N05   , Cn3 , v076
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        An4 , v080
	.byte	W12
	.byte		        Cn3 , v076
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        Gn4 , v080
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        Gn4 , v080
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W06
	.byte		        Gn4 , v080
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W06
	.byte		        Fn4 , v080
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        Gn4 , v080
	.byte	W06
	.byte		N11   , Gn4 , v072
	.byte	W12
	.byte		N05   , Cn4
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        Gn4 , v080
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        Gn4 , v080
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        Gn4 , v080
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        An4 , v080
	.byte	W06
	.byte	PATT
	 ptrvalue	mus_qz_629_7_005
	.byte	PATT
	 ptrvalue	mus_qz_629_7_008
	.byte		N05   , Cn3 , v076
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        An4 , v080
	.byte	W12
	.byte		        Cn3 , v076
	.byte	W06
	.byte		N12   , Cn4 , v072
	.byte	W06
	.byte		N05   , Gn4 , v080
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        Gn4 , v080
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W06
	.byte		        As4 , v080
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W06
	.byte		        Cn5 , v080
	.byte	W06
	.byte		        Cn4 , v072
	.byte	W06
	.byte		        Cn3 , v076
	.byte	W06
	.byte		        An4 , v080
	.byte	W06
	.byte		        Ds4 , v120
	.byte	W06
	.byte		        Fn4 , v112
	.byte	W06
	.byte		N32   , Gn4 , v127 , gtp3
	.byte	W36
	.byte		        Fn4 , v124 , gtp3
	.byte	W36
	.byte		N23   , Cn4
	.byte	W24
	.byte		N32   , Dn4 , v124 , gtp3
	.byte	W36
	.byte		        Ds4 , v120 , gtp3
	.byte	W36
	.byte		N23   , Fn4 , v124
	.byte	W24
	.byte		N32   , Dn4 , v124 , gtp3
	.byte	W36
	.byte		        Cn4 , v120 , gtp3
	.byte	W36
	.byte		N23   , As3 , v112
	.byte	W24
	.byte		N44   , An3 , v120 , gtp3
	.byte	W72
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_629_7_000
	.byte	PATT
	 ptrvalue	mus_qz_629_7_001
	.byte	PATT
	 ptrvalue	mus_qz_629_7_002
	.byte	PATT
	 ptrvalue	mus_qz_629_7_003
	.byte	PATT
	 ptrvalue	mus_qz_629_7_004
	.byte	PATT
	 ptrvalue	mus_qz_629_7_005
	.byte	PATT
	 ptrvalue	mus_qz_629_7_006
	.byte	PATT
	 ptrvalue	mus_qz_629_7_007
	.byte	PATT
	 ptrvalue	mus_qz_629_7_008
	.byte	PATT
	 ptrvalue	mus_qz_629_7_005
	.byte	PATT
	 ptrvalue	mus_qz_629_7_006
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_629_7_B1
	.byte	FINE

/********************** Track  8 **********************/

mus_qz_629_8:
	.byte		KEYSH , mus_qz_629_key+0
	.byte		VOICE , 37
	.byte		VOL   , 81*mus_qz_629_mvl/mxv
	.byte		PAN   , c_v-54
	.byte		PRIO  , 48
	.byte	W48
	.byte		N23   , Cn4 , v116
	.byte	W96
	.byte	W96
	.byte	N23
	.byte	W96
	.byte	W96
	.byte	N23
	.byte	W96
	.byte	W96
	.byte	N23
	.byte	W96
	.byte	W96
	.byte	N23
	.byte	W96
	.byte	W96
	.byte	N23
	.byte	W96
	.byte	N23
	.byte	W96
mus_qz_629_8_B1:
	.byte		N23   , Cn4 , v116
	.byte	W96
	.byte	W96
	.byte	N23
	.byte	W96
	.byte	W96
	.byte	N23
	.byte	W96
	.byte	W96
	.byte	N23
	.byte	W96
	.byte	W96
	.byte	N23
	.byte	W96
	.byte	W96
	.byte	N23
	.byte	W96
	.byte	W96
	.byte	N23
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	N23
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		N05   , Gn3
	.byte	W06
	.byte	N05
	.byte	W06
	.byte	N11
	.byte	W18
	.byte		N05   , As3
	.byte	W12
	.byte	N11
	.byte	W54
	.byte		N23   , Cn4
	.byte	W96
	.byte	W96
	.byte	N23
	.byte	W96
	.byte	W96
	.byte	N23
	.byte	W96
	.byte	W96
	.byte	N23
	.byte	W96
	.byte	W96
	.byte	N23
	.byte	W96
	.byte	W96
	.byte	N23
	.byte	W96
	.byte	N23
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_629_8_B1
	.byte	FINE

/********************** Track  9 **********************/

mus_qz_629_9:
	.byte		KEYSH , mus_qz_629_key+0
	.byte		VOICE , 39
	.byte		VOL   , 49*mus_qz_629_mvl/mxv
	.byte		PAN   , c_v+46
	.byte		PRIO  , 42
	.byte	W48
mus_qz_629_9_000:
	.byte		N05   , Gn3 , v036
	.byte	W06
	.byte		        Gn3 , v020
	.byte	W06
	.byte		        Gn3 , v028
	.byte	W06
	.byte		        Gn3 , v036
	.byte	W06
	.byte		        Gn3 , v020
	.byte	W06
	.byte		        Gn3 , v028
	.byte	W06
	.byte		        Gn3 , v036
	.byte	W06
	.byte		        Gn3 , v020
	.byte	W06
	.byte		        Gn3 , v028
	.byte	W06
	.byte		        Gn3 , v036
	.byte	W06
	.byte		        Gn3 , v020
	.byte	W06
	.byte		        Gn3 , v028
	.byte	W06
	.byte		        Gn3 , v036
	.byte	W06
	.byte		        Gn3 , v020
	.byte	W06
	.byte		        Gn3 , v028
	.byte	W06
	.byte		        Gn3 , v016
	.byte	W06
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_629_9_000
	.byte	PATT
	 ptrvalue	mus_qz_629_9_000
	.byte	PATT
	 ptrvalue	mus_qz_629_9_000
	.byte	PATT
	 ptrvalue	mus_qz_629_9_000
	.byte	PATT
	 ptrvalue	mus_qz_629_9_000
	.byte	PATT
	 ptrvalue	mus_qz_629_9_000
	.byte	PATT
	 ptrvalue	mus_qz_629_9_000
	.byte	PATT
	 ptrvalue	mus_qz_629_9_000
	.byte	PATT
	 ptrvalue	mus_qz_629_9_000
	.byte	PATT
	 ptrvalue	mus_qz_629_9_000
	.byte		N05   , Gn3 , v036
	.byte	W96
mus_qz_629_9_B1:
	.byte		N05   , Gn3 , v048
	.byte	W06
	.byte		        Gn3 , v028
	.byte	W06
	.byte		        Gn3 , v036
	.byte	W06
	.byte		        Gn3 , v048
	.byte	W06
	.byte		        Gn3 , v028
	.byte	W06
	.byte		        Gn3 , v036
	.byte	W06
	.byte		        Gn3 , v048
	.byte	W06
	.byte		        Gn3 , v028
	.byte	W06
	.byte		        Gn3 , v036
	.byte	W06
	.byte		        Gn3 , v048
	.byte	W06
	.byte		        Gn3 , v028
	.byte	W06
	.byte		        Gn3 , v036
	.byte	W06
	.byte		        Gn3 , v048
	.byte	W06
	.byte		        Gn3 , v028
	.byte	W06
	.byte		        Gn3 , v036
	.byte	W06
	.byte		        Gn3 , v024
	.byte	W06
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_629_9_B1
	.byte	PATT
	 ptrvalue	mus_qz_629_9_B1
	.byte	PATT
	 ptrvalue	mus_qz_629_9_B1
	.byte	PATT
	 ptrvalue	mus_qz_629_9_B1
	.byte	PATT
	 ptrvalue	mus_qz_629_9_B1
	.byte	PATT
	 ptrvalue	mus_qz_629_9_B1
	.byte	PATT
	 ptrvalue	mus_qz_629_9_B1
	.byte	PATT
	 ptrvalue	mus_qz_629_9_B1
	.byte	PATT
	 ptrvalue	mus_qz_629_9_B1
	.byte	PATT
	 ptrvalue	mus_qz_629_9_B1
	.byte		N05   , Gn3 , v048
	.byte	W06
	.byte		        Gn3 , v028
	.byte	W06
	.byte		        Gn3 , v036
	.byte	W06
	.byte		        Gn3 , v048
	.byte	W06
	.byte		        Gn3 , v028
	.byte	W06
	.byte		N12   , Gn3 , v036
	.byte		N12   , Gn3 , v048
	.byte	W12
	.byte		N01   , Gn3 , v028
	.byte	W06
	.byte		N05   , Gn3 , v036
	.byte	W06
	.byte		        Gn3 , v048
	.byte	W06
	.byte		        Gn3 , v028
	.byte	W06
	.byte		        Gn3 , v036
	.byte	W06
	.byte		        Gn3 , v048
	.byte	W06
	.byte		        Gn3 , v028
	.byte	W06
	.byte		        Gn3 , v036
	.byte	W06
	.byte		        Gn3 , v024
	.byte	W06
	.byte	PATT
	 ptrvalue	mus_qz_629_9_B1
	.byte	PATT
	 ptrvalue	mus_qz_629_9_B1
	.byte	PATT
	 ptrvalue	mus_qz_629_9_B1
	.byte	PATT
	 ptrvalue	mus_qz_629_9_B1
	.byte	PATT
	 ptrvalue	mus_qz_629_9_B1
	.byte	PATT
	 ptrvalue	mus_qz_629_9_B1
	.byte	PATT
	 ptrvalue	mus_qz_629_9_B1
	.byte		N05   , Gn3 , v048
	.byte	W06
	.byte		        Gn3 , v028
	.byte	W06
	.byte		        Gn3 , v036
	.byte	W18
	.byte	N05
	.byte	W12
	.byte	N05
	.byte	W54
	.byte	PATT
	 ptrvalue	mus_qz_629_9_000
	.byte	PATT
	 ptrvalue	mus_qz_629_9_000
	.byte	PATT
	 ptrvalue	mus_qz_629_9_000
	.byte	PATT
	 ptrvalue	mus_qz_629_9_000
	.byte	PATT
	 ptrvalue	mus_qz_629_9_000
	.byte	PATT
	 ptrvalue	mus_qz_629_9_000
	.byte	PATT
	 ptrvalue	mus_qz_629_9_000
	.byte	PATT
	 ptrvalue	mus_qz_629_9_000
	.byte	PATT
	 ptrvalue	mus_qz_629_9_000
	.byte	PATT
	 ptrvalue	mus_qz_629_9_000
	.byte	PATT
	 ptrvalue	mus_qz_629_9_000
	.byte		N05   , Gn3 , v036
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_629_9_B1
	.byte	FINE

/********************** Track 10 **********************/

mus_qz_629_10:
	.byte		KEYSH , mus_qz_629_key+0
	.byte		VOICE , 1
	.byte		VOL   , 16*mus_qz_629_mvl/mxv
	.byte		PAN   , c_v+56
	.byte		PRIO  , 36
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
mus_qz_629_10_000:
	.byte	W24
	.byte		N05   , Gs3 , v072
	.byte		N05   , Cn4 , v084
	.byte	W06
	.byte		        Fn3 , v072
	.byte	W12
	.byte		        Gs3
	.byte		N05   , Cn4 , v084
	.byte	W12
	.byte		N17   , Gs3 , v072
	.byte		N17   , Ds4 , v084
	.byte	W18
	.byte		N05   , Gn3 , v072
	.byte		N05   , Dn4 , v084
	.byte	W12
	.byte		        As3 , v072
	.byte		N05   , Ds4 , v084
	.byte	W06
	.byte		N17   , Cn4 , v072
	.byte		N17   , Fn4 , v084
	.byte	W06
	.byte	PEND
mus_qz_629_10_B1:
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
	.byte	W48
	.byte		N04   , Fn5 , v088
	.byte	W06
	.byte		        Cn5 , v068
	.byte	W06
	.byte		        Gs4 , v088
	.byte	W06
	.byte		        Cn5 , v068
	.byte	W06
	.byte		        Gs4 , v088
	.byte	W06
	.byte		        Fn4 , v068
	.byte	W06
	.byte		        Gs4 , v088
	.byte	W06
	.byte		        Cn5 , v068
	.byte	W06
	.byte		        Fn5 , v088
	.byte	W06
	.byte		        Cn5 , v068
	.byte	W06
	.byte		        Gs4 , v088
	.byte	W06
	.byte		        Cn5 , v068
	.byte	W06
	.byte		        Gs4 , v088
	.byte	W06
	.byte		        Fn4 , v068
	.byte	W06
	.byte		        Gs4 , v088
	.byte	W06
	.byte		        Cn5
	.byte	W06
	.byte		        Gn5
	.byte	W06
	.byte		        Dn5 , v068
	.byte	W06
	.byte		        As4 , v088
	.byte	W06
	.byte		        Dn5 , v068
	.byte	W06
	.byte		        As4 , v088
	.byte	W06
	.byte		        Gn4 , v068
	.byte	W06
	.byte		        As4 , v088
	.byte	W06
	.byte		        Dn5 , v068
	.byte	W06
	.byte		        Gn5 , v088
	.byte	W06
	.byte		        Dn5 , v068
	.byte	W06
	.byte		        As4 , v088
	.byte	W06
	.byte		        Dn5 , v068
	.byte	W06
	.byte		        As4 , v088
	.byte	W06
	.byte		        Gn4 , v068
	.byte	W06
	.byte		        As4 , v088
	.byte	W06
	.byte		        Dn5 , v068
	.byte	W06
	.byte		        Cn5 , v088
	.byte	W06
	.byte		        Gn4 , v068
	.byte	W06
	.byte		        Ds4 , v088
	.byte	W06
	.byte		        Gn4 , v068
	.byte	W06
	.byte		        Cn4 , v088
	.byte	W06
	.byte		        Ds4 , v068
	.byte	W06
	.byte		        Gn4 , v088
	.byte	W06
	.byte		        Cn5 , v068
	.byte	W06
	.byte		        Gn4 , v088
	.byte	W06
	.byte		        Ds4 , v068
	.byte	W06
	.byte		        Gn4 , v088
	.byte	W06
	.byte		        Cn5 , v068
	.byte	W06
	.byte		        Ds5 , v088
	.byte	W06
	.byte		        Cn5 , v068
	.byte	W06
	.byte		        Ds4 , v088
	.byte	W06
	.byte		        As4 , v068
	.byte	W06
	.byte		        Cn5 , v088
	.byte	W06
	.byte		        An4 , v068
	.byte	W06
	.byte		        Fn4 , v088
	.byte	W06
	.byte		        An4 , v068
	.byte	W06
	.byte		        Fn4 , v088
	.byte	W06
	.byte		        Cn4 , v068
	.byte	W06
	.byte		        Fn4 , v088
	.byte	W06
	.byte		        An4 , v068
	.byte	W06
	.byte		        Fn5 , v088
	.byte	W06
	.byte		        Fn5 , v068
	.byte	W06
	.byte		        Cn5 , v088
	.byte	W06
	.byte		        An4 , v068
	.byte	W06
	.byte		        Cn5 , v088
	.byte	W06
	.byte		        An4 , v068
	.byte	W06
	.byte		        Fn4 , v088
	.byte	W06
	.byte		        Cn4 , v068
	.byte	W06
	.byte		        Fn5 , v088
	.byte	W06
	.byte		        Cn5 , v068
	.byte	W06
	.byte		        Gs4 , v088
	.byte	W06
	.byte		        Cn5 , v068
	.byte	W06
	.byte		        Gs4 , v088
	.byte	W06
	.byte		        Fn4 , v068
	.byte	W06
	.byte		        Gs4 , v088
	.byte	W06
	.byte		        Cn5 , v068
	.byte	W06
	.byte		        Fn5 , v088
	.byte	W06
	.byte		        Cn5 , v068
	.byte	W06
	.byte		        Gs4 , v088
	.byte	W06
	.byte		        Cn5 , v068
	.byte	W06
	.byte		        Gs4 , v088
	.byte	W06
	.byte		        Fn4 , v068
	.byte	W06
	.byte		        Gs4 , v088
	.byte	W06
	.byte		        Cn5 , v068
	.byte	W06
	.byte		        Gn5 , v088
	.byte	W06
	.byte		        Dn5 , v068
	.byte	W06
	.byte		        As4 , v088
	.byte	W06
	.byte		        Dn5 , v068
	.byte	W06
	.byte		        As4 , v088
	.byte	W06
	.byte		        Gn4 , v068
	.byte	W06
	.byte		        As4 , v088
	.byte	W06
	.byte		        Dn5 , v068
	.byte	W06
	.byte		        Gn5 , v088
	.byte	W06
	.byte		        Dn5 , v068
	.byte	W06
	.byte		        As4 , v088
	.byte	W06
	.byte		        Dn5 , v068
	.byte	W06
	.byte		        As4 , v088
	.byte	W06
	.byte		        Gn4 , v068
	.byte	W06
	.byte		        As4 , v088
	.byte	W06
	.byte		        Dn5 , v068
	.byte	W06
	.byte		        Gn5 , v088
	.byte	W06
	.byte		        Dn5 , v068
	.byte	W06
	.byte		        Bn4 , v088
	.byte	W06
	.byte		        Dn5 , v068
	.byte	W06
	.byte		        Bn4 , v088
	.byte	W06
	.byte		        Gn4 , v068
	.byte	W06
	.byte		        Bn4 , v088
	.byte	W06
	.byte		        Dn5 , v068
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
	.byte	W96
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_629_10_000
	.byte	GOTO
	 ptrvalue	mus_qz_629_10_B1
	.byte	W11
	.byte	FINE

/********************** Track 11 **********************/

mus_qz_629_11:
	.byte		KEYSH , mus_qz_629_key+0
	.byte		VOICE , 17
	.byte		PAN   , c_v+0
	.byte		BENDR , 12
	.byte		PRIO  , 64
	.byte		VOL   , 72*mus_qz_629_mvl/mxv
	.byte		        52*mus_qz_629_mvl/mxv
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
mus_qz_629_11_B1:
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
	.byte	W36
	.byte		N01   , Fn4 , v072
	.byte		N01   , Fn5
	.byte	W06
	.byte		        Fn4
	.byte		N01   , Fn5
	.byte	W06
	.byte		N32   , Fn2 , v108 , gtp3
	.byte		N32   , Fn3 , v108 , gtp3
	.byte	W07
	.byte		BEND  , c_v-5
	.byte	W02
	.byte		        c_v-9
	.byte	W01
	.byte		        c_v-12
	.byte	W02
	.byte		        c_v-14
	.byte	W01
	.byte		        c_v-17
	.byte	W02
	.byte		        c_v-21
	.byte	W01
	.byte		        c_v-24
	.byte	W02
	.byte		        c_v-26
	.byte	W01
	.byte		        c_v-29
	.byte	W02
	.byte		        c_v-33
	.byte	W01
	.byte		        c_v-35
	.byte	W02
	.byte		        c_v-38
	.byte	W01
	.byte		        c_v-42
	.byte	W02
	.byte		        c_v-45
	.byte	W01
	.byte		        c_v-50
	.byte	W02
	.byte		        c_v-55
	.byte	W01
	.byte		        c_v-60
	.byte	W02
	.byte		        c_v-64
	.byte	W03
	.byte		        c_v+0
	.byte		N04   , Cn4 , v088
	.byte	W01
	.byte		        Ds4 , v108
	.byte	W05
	.byte		        Dn4 , v080
	.byte	W01
	.byte		        Fn4 , v100
	.byte	W05
	.byte		N32   , Ds4 , v088 , gtp2
	.byte	W01
	.byte		        Gn4 , v108 , gtp2
	.byte	W32
	.byte	W03
	.byte		        Dn4 , v088 , gtp2
	.byte	W01
	.byte		        Fn4 , v108 , gtp2
	.byte	W32
	.byte	W03
	.byte		N22   , Gs3 , v088
	.byte	W01
	.byte		        Cn4 , v108
	.byte	W23
	.byte		N32   , Bn3 , v088 , gtp2
	.byte	W01
	.byte		        Dn4 , v108 , gtp2
	.byte	W32
	.byte	W03
	.byte		        Bn3 , v088 , gtp2
	.byte	W01
	.byte		        Ds4 , v108 , gtp2
	.byte	W32
	.byte	W03
	.byte		N10   , Bn3 , v088
	.byte	W01
	.byte		        Fn4 , v108
	.byte	W11
	.byte		N11   , An3 , v096
	.byte	W02
	.byte		BEND  , c_v+5
	.byte	W02
	.byte		        c_v+16
	.byte	W02
	.byte		        c_v+21
	.byte	W02
	.byte		        c_v+32
	.byte	W02
	.byte		        c_v+37
	.byte	W02
	.byte		        c_v+0
	.byte		N84   , Bn3 , v088 , gtp1
	.byte	W01
	.byte		        Gn4 , v108 , gtp1
	.byte	W76
	.byte		VOL   , 47*mus_qz_629_mvl/mxv
	.byte	W03
	.byte		        45*mus_qz_629_mvl/mxv
	.byte	W03
	.byte		        42*mus_qz_629_mvl/mxv
	.byte	W03
	.byte		        39*mus_qz_629_mvl/mxv
	.byte	W03
	.byte		        34*mus_qz_629_mvl/mxv
	.byte	W03
	.byte		        32*mus_qz_629_mvl/mxv
	.byte	W03
	.byte		        27*mus_qz_629_mvl/mxv
	.byte	W01
	.byte		N03   , Bn3 , v092
	.byte	W01
	.byte		        Gn4 , v108
	.byte	W01
	.byte		VOL   , 52*mus_qz_629_mvl/mxv
	.byte	W04
	.byte		N03   , Bn3 , v092
	.byte	W01
	.byte		        Gn4 , v108
	.byte	W05
	.byte		N04   , Bn3 , v092
	.byte	W01
	.byte		        Gn4 , v108
	.byte	W17
	.byte		        Dn4 , v092
	.byte	W01
	.byte		        As4 , v108
	.byte	W11
	.byte		        Dn4 , v092
	.byte	W01
	.byte		        As4 , v108
	.byte	W28
	.byte	W01
	.byte		N22   , Gn2 , v100
	.byte		N22   , Gn3
	.byte	W03
	.byte		BEND  , c_v-9
	.byte	W01
	.byte		        c_v-13
	.byte	W02
	.byte		        c_v-19
	.byte	W01
	.byte		        c_v-23
	.byte	W02
	.byte		        c_v-29
	.byte	W01
	.byte		        c_v-33
	.byte	W02
	.byte		        c_v-39
	.byte	W01
	.byte		        c_v-44
	.byte	W02
	.byte		        c_v-47
	.byte	W01
	.byte		        c_v-53
	.byte	W02
	.byte		        c_v-59
	.byte	W01
	.byte		        c_v-62
	.byte	W02
	.byte		        c_v-64
	.byte	W02
	.byte		        c_v+0
	.byte	W01
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
	 ptrvalue	mus_qz_629_11_B1
	.byte	FINE

/********************** Track 12 **********************/

mus_qz_629_12:
	.byte		KEYSH , mus_qz_629_key+0
	.byte		VOICE , 17
	.byte		VOL   , 16*mus_qz_629_mvl/mxv
	.byte		PAN   , c_v+56
	.byte		BENDR , 12
	.byte		PRIO  , 34
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
mus_qz_629_12_B1:
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
	.byte		N32   , Fn2 , v100 , gtp3
	.byte	W07
	.byte		BEND  , c_v-5
	.byte	W02
	.byte		        c_v-9
	.byte	W01
	.byte		        c_v-12
	.byte	W02
	.byte		        c_v-14
	.byte	W01
	.byte		        c_v-17
	.byte	W02
	.byte		        c_v-21
	.byte	W01
	.byte		        c_v-24
	.byte	W02
	.byte		        c_v-26
	.byte	W01
	.byte		        c_v-29
	.byte	W02
	.byte		        c_v-33
	.byte	W01
	.byte		        c_v-35
	.byte	W02
	.byte		        c_v-38
	.byte	W01
	.byte		        c_v-44
	.byte	W02
	.byte		        c_v-47
	.byte	W01
	.byte		        c_v-50
	.byte	W02
	.byte		        c_v-55
	.byte	W01
	.byte		        c_v-59
	.byte	W02
	.byte		        c_v-64
	.byte	W02
	.byte		        c_v-1
	.byte	W01
	.byte		N04   , Cn4 , v092
	.byte	W01
	.byte		        Ds4 , v108
	.byte	W01
	.byte		BEND  , c_v+0
	.byte	W04
	.byte		N04   , Dn4 , v092
	.byte	W01
	.byte		        Fn4 , v108
	.byte	W05
	.byte		N32   , Ds4 , v092 , gtp2
	.byte	W01
	.byte		        Gn4 , v108 , gtp2
	.byte	W32
	.byte	W03
	.byte		        Dn4 , v092 , gtp2
	.byte	W01
	.byte		        Fn4 , v108 , gtp2
	.byte	W32
	.byte	W03
	.byte		N22   , Gs3 , v092
	.byte	W01
	.byte		        Cn4 , v108
	.byte	W11
	.byte	W12
	.byte		N32   , Bn3 , v092 , gtp2
	.byte	W01
	.byte		        Dn4 , v108 , gtp2
	.byte	W32
	.byte	W03
	.byte		        Bn3 , v092 , gtp2
	.byte	W01
	.byte		        Ds4 , v108 , gtp2
	.byte	W32
	.byte	W03
	.byte		N10   , Bn3 , v092
	.byte	W01
	.byte		        Fn4 , v108
	.byte	W11
	.byte		N11   , An3 , v100
	.byte	W02
	.byte		BEND  , c_v+5
	.byte	W02
	.byte		        c_v+12
	.byte	W02
	.byte		        c_v+21
	.byte	W02
	.byte		        c_v+30
	.byte	W02
	.byte		        c_v+41
	.byte	W02
	.byte		        c_v+0
	.byte		N84   , Bn3 , v092 , gtp1
	.byte	W01
	.byte		        Gn4 , v108 , gtp1
	.byte	W80
	.byte	W03
	.byte	W05
	.byte		N03   , Bn3 , v092
	.byte	W01
	.byte		        Gn4 , v108
	.byte	W05
	.byte		        Bn3 , v092
	.byte	W01
	.byte		        Gn4 , v108
	.byte	W05
	.byte		N04   , Bn3 , v092
	.byte	W01
	.byte		        Gn4 , v108
	.byte	W17
	.byte		        Dn4 , v092
	.byte	W01
	.byte		        As4 , v108
	.byte	W11
	.byte		        Dn4 , v092
	.byte	W01
	.byte		        As4 , v108
	.byte	W36
	.byte		N22   , Gn2 , v100
	.byte	W04
	.byte		BEND  , c_v-9
	.byte	W02
	.byte		        c_v-16
	.byte	W01
	.byte		        c_v-21
	.byte	W02
	.byte		        c_v-26
	.byte	W01
	.byte		        c_v-32
	.byte	W02
	.byte		        c_v-38
	.byte	W01
	.byte		        c_v-42
	.byte	W02
	.byte		        c_v-45
	.byte	W01
	.byte		        c_v-50
	.byte	W02
	.byte		        c_v-55
	.byte	W01
	.byte		        c_v-59
	.byte	W02
	.byte		        c_v-64
	.byte	W02
	.byte		        c_v+0
	.byte	W84
	.byte	W01
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
	 ptrvalue	mus_qz_629_12_B1
	.byte	FINE

/******************************************************/
	.p2align	2

mus_qz_629:
	.byte	12	/* NumTrks */
	.byte	0	/* NumBlks */
	.byte	mus_qz_629_pri	/* Priority */
	.byte	mus_qz_629_rev	/* Reverb. */
space64 4
	ptrvalue	mus_qz_629_grp

	ptrvalue	mus_qz_629_1
	ptrvalue	mus_qz_629_2
	ptrvalue	mus_qz_629_3
	ptrvalue	mus_qz_629_4
	ptrvalue	mus_qz_629_5
	ptrvalue	mus_qz_629_6
	ptrvalue	mus_qz_629_7
	ptrvalue	mus_qz_629_8
	ptrvalue	mus_qz_629_9
	ptrvalue	mus_qz_629_10
	ptrvalue	mus_qz_629_11
	ptrvalue	mus_qz_629_12

	.end
