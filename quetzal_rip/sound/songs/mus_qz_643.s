	.include "asm/macros/bit_width.inc"

	.include "MPlayDef.s"

/* Moemon Quetzal song 643, disassembled from the ROM by tools/quetzal_rip/songs.py.
   "org" comments give the ROM address of the bytes that follow. */

	.equ	mus_qz_643_grp, voicegroup_qz_songs
	.equ	mus_qz_643_pri, 0
	.equ	mus_qz_643_rev, reverb_set+0
	.equ	mus_qz_643_mvl, 127
	.equ	mus_qz_643_key, 0
	.equ	mus_qz_643_tbs, 1
	.equ	mus_qz_643_exg, 1
	.equ	mus_qz_643_cmp, 1

	.section .rodata
	.global	mus_qz_643
	.p2align	2
/* org 0x09690894 */

/********************** Track  1 **********************/

mus_qz_643_1:
	.byte		KEYSH , mus_qz_643_key+0
	.byte		TEMPO , 100*mus_qz_643_tbs/2
	.byte		VOL   , 57*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte		VOL   , 57*mus_qz_643_mvl/mxv
	.byte	W36
	.byte		VOICE , 1
	.byte	W12
	.byte		VOL   , 54*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v+25
	.byte		N05   , Fn5 , v100
	.byte	W06
	.byte		N11   , Fn5 , v048
	.byte	W12
	.byte		PAN   , c_v-17
	.byte		VOL   , 41*mus_qz_643_mvl/mxv
	.byte		N05   , Fn5 , v100
	.byte	W06
	.byte		N11   , Fn5 , v048
	.byte	W12
	.byte		PAN   , c_v+25
	.byte		VOL   , 31*mus_qz_643_mvl/mxv
	.byte		N05   , Fn5 , v100
	.byte	W06
	.byte		N11   , Fn5 , v048
	.byte	W12
	.byte		PAN   , c_v-17
	.byte		VOL   , 21*mus_qz_643_mvl/mxv
	.byte		N05   , Fn5 , v100
	.byte	W06
	.byte		N11   , Fn5 , v048
	.byte	W12
	.byte		PAN   , c_v+25
	.byte		VOL   , 12*mus_qz_643_mvl/mxv
	.byte		N05   , Fn5 , v100
	.byte	W06
	.byte		N11   , Fn5 , v048
	.byte	W12
	.byte		PAN   , c_v-17
	.byte		N05   , Fn5 , v100
	.byte	W06
mus_qz_643_1_B1:
	.byte		PAN   , c_v+25
	.byte		VOL   , 54*mus_qz_643_mvl/mxv
	.byte		N05   , Cn5 , v100
	.byte	W06
	.byte		N11   , Cn5 , v048
	.byte	W12
	.byte		PAN   , c_v-17
	.byte		VOL   , 41*mus_qz_643_mvl/mxv
	.byte		N05   , Cn5 , v100
	.byte	W06
	.byte		N11   , Cn5 , v048
	.byte	W12
	.byte		PAN   , c_v+25
	.byte		VOL   , 31*mus_qz_643_mvl/mxv
	.byte		N05   , Cn5 , v100
	.byte	W06
	.byte		N11   , Cn5 , v048
	.byte	W12
	.byte		PAN   , c_v-17
	.byte		VOL   , 21*mus_qz_643_mvl/mxv
	.byte		N05   , Cn5 , v100
	.byte	W06
	.byte		N11   , Cn5 , v048
	.byte	W12
	.byte		PAN   , c_v+25
	.byte		VOL   , 12*mus_qz_643_mvl/mxv
	.byte		N05   , Cn5 , v100
	.byte	W06
	.byte		N11   , Cn5 , v048
	.byte	W12
	.byte		PAN   , c_v-17
	.byte		N05   , Cn5 , v100
	.byte	W06
	.byte		PAN   , c_v+25
	.byte		VOL   , 54*mus_qz_643_mvl/mxv
	.byte		N05   , Fn5
	.byte	W06
	.byte		N11   , Fn5 , v048
	.byte	W12
	.byte		PAN   , c_v-17
	.byte		VOL   , 41*mus_qz_643_mvl/mxv
	.byte		N05   , Fn5 , v100
	.byte	W06
	.byte		N11   , Fn5 , v048
	.byte	W12
	.byte		PAN   , c_v+25
	.byte		VOL   , 31*mus_qz_643_mvl/mxv
	.byte		N05   , Fn5 , v100
	.byte	W06
	.byte		N11   , Fn5 , v048
	.byte	W12
	.byte		PAN   , c_v-17
	.byte		VOL   , 21*mus_qz_643_mvl/mxv
	.byte		N05   , Fn5 , v100
	.byte	W06
	.byte		N11   , Fn5 , v048
	.byte	W12
	.byte		PAN   , c_v+25
	.byte		VOL   , 12*mus_qz_643_mvl/mxv
	.byte		N05   , Fn5 , v100
	.byte	W06
	.byte		N11   , Fn5 , v048
	.byte	W12
	.byte		PAN   , c_v-17
	.byte		N05   , Fn5 , v100
	.byte	W06
	.byte		PAN   , c_v+25
	.byte		VOL   , 54*mus_qz_643_mvl/mxv
	.byte		N05   , Cn5
	.byte	W06
	.byte		N11   , Cn5 , v048
	.byte	W12
	.byte		PAN   , c_v-17
	.byte		VOL   , 41*mus_qz_643_mvl/mxv
	.byte		N05   , Cn5 , v100
	.byte	W06
	.byte		N11   , Cn5 , v048
	.byte	W12
	.byte		PAN   , c_v+25
	.byte		VOL   , 31*mus_qz_643_mvl/mxv
	.byte		N05   , Cn5 , v100
	.byte	W06
	.byte		N11   , Cn5 , v048
	.byte	W12
	.byte		PAN   , c_v-17
	.byte		VOL   , 21*mus_qz_643_mvl/mxv
	.byte		N05   , Cn5 , v100
	.byte	W06
	.byte		N11   , Cn5 , v048
	.byte	W12
	.byte		PAN   , c_v+25
	.byte		VOL   , 12*mus_qz_643_mvl/mxv
	.byte		N05   , Cn5 , v100
	.byte	W06
	.byte		N11   , Cn5 , v048
	.byte	W12
	.byte		PAN   , c_v-17
	.byte		N05   , Cn5 , v100
	.byte	W06
	.byte		VOL   , 54*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v+25
	.byte		N05   , Fn5
	.byte	W06
	.byte		N11   , Fn5 , v048
	.byte	W12
	.byte		PAN   , c_v-17
	.byte		VOL   , 41*mus_qz_643_mvl/mxv
	.byte		N05   , Fn5 , v100
	.byte	W06
	.byte		N11   , Fn5 , v048
	.byte	W12
	.byte		VOL   , 31*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v+25
	.byte		N05   , Fn5 , v100
	.byte	W06
	.byte		N11   , Fn5 , v048
	.byte	W12
	.byte		PAN   , c_v-17
	.byte		VOL   , 21*mus_qz_643_mvl/mxv
	.byte		N05   , Fn5 , v100
	.byte	W06
	.byte		N11   , Fn5 , v048
	.byte	W12
	.byte		VOL   , 12*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v+25
	.byte		N05   , Fn5 , v100
	.byte	W06
	.byte		N11   , Fn5 , v048
	.byte	W12
	.byte		PAN   , c_v-17
	.byte		N05   , Fn5 , v100
	.byte	W06
	.byte		PAN   , c_v-39
	.byte		VOL   , 23*mus_qz_643_mvl/mxv
	.byte	W24
	.byte		N17   , Cn5
	.byte	W18
	.byte		N02   , Dn5
	.byte	W03
	.byte		        En5
	.byte	W03
	.byte		N17   , Fn5
	.byte	W18
	.byte		        Cn5
	.byte	W18
	.byte		N11   , As4
	.byte	W12
	.byte		N17   , An4
	.byte	W18
	.byte	N02
	.byte	W03
	.byte		        As4
	.byte	W03
	.byte		N11   , Cn5
	.byte	W12
	.byte		        Dn5
	.byte	W12
	.byte		N23   , Cn5
	.byte	W24
	.byte		N32   , Fn4 , v100 , gtp3
	.byte	W24
	.byte	W12
	.byte		N11   , En4
	.byte	W12
	.byte		        Fn4
	.byte	W12
	.byte		        Gn4
	.byte	W12
	.byte		N17   , Cn5
	.byte	W18
	.byte		N02   , An4
	.byte	W03
	.byte		        As4
	.byte	W03
	.byte		N11   , Cn5
	.byte	W12
	.byte		        As4
	.byte	W12
	.byte		N23   , An4
	.byte	W24
	.byte		        Cn5
	.byte	W24
	.byte		N11   , Fn4
	.byte	W12
	.byte		        En4
	.byte	W12
	.byte		        Fn4
	.byte	W12
	.byte		N28   , Cn5 , v100 , gtp1
	.byte	W12
	.byte	W18
	.byte		N02   , An4
	.byte	W03
	.byte		        As4
	.byte	W03
	.byte		N11   , Cn5
	.byte	W12
	.byte		        Dn5
	.byte	W12
	.byte		N23   , Fn5
	.byte	W24
	.byte		        En5
	.byte	W24
	.byte		N28   , Fn5 , v100 , gtp1
	.byte	W30
	.byte		N05   , Cn5
	.byte	W06
	.byte		        As4
	.byte	W06
	.byte		N28   , An4 , v100 , gtp1
	.byte	W30
	.byte		PAN   , c_v+26
	.byte		N05   , Cn5
	.byte	W06
	.byte		        Gn4
	.byte	W06
	.byte		        Fn4
	.byte	W06
	.byte		N28   , Cn4 , v100 , gtp1
	.byte	W06
	.byte	W24
	.byte		PAN   , c_v-22
	.byte		N05   , An4
	.byte	W06
	.byte		        Fn4
	.byte	W06
	.byte		        As3
	.byte	W06
	.byte		N28   , An3 , v100 , gtp1
	.byte	W30
	.byte		VOL   , 63*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v+20
	.byte		N23   , Gn3 , v048
	.byte		N23   , Cn4 , v100
	.byte	W24
	.byte		N17   , An3 , v092
	.byte		N17   , Fn4 , v100
	.byte	W18
	.byte		        An3 , v048
	.byte		N17   , Fn4
	.byte	W18
	.byte		        As3 , v092
	.byte		N17   , Gn4 , v100
	.byte	W18
	.byte		N05   , As3 , v048
	.byte		N05   , Gn4
	.byte	W06
	.byte		N17   , An3 , v092
	.byte		N17   , Fn4 , v100
	.byte	W18
	.byte		N05   , An3 , v048
	.byte		N05   , Fn4
	.byte	W06
	.byte		N11   , An3 , v092
	.byte		N11   , En4 , v100
	.byte	W12
	.byte		N17   , En4 , v092
	.byte		N17   , Cn5 , v100
	.byte	W18
	.byte		N05   , En4 , v048
	.byte		N05   , Cn5
	.byte	W06
	.byte		N17   , Dn4 , v092
	.byte		N17   , As4 , v100
	.byte	W18
	.byte		N05   , Dn4 , v048
	.byte		N05   , As4
	.byte	W06
	.byte		N17   , Cn4 , v092
	.byte		N17   , An4 , v100
	.byte	W18
	.byte		N05   , Cn4 , v048
	.byte		N05   , An4
	.byte	W06
	.byte		N17   , As3 , v092
	.byte		N17   , Gn4 , v100
	.byte	W18
	.byte		N05   , As3 , v048
	.byte		N05   , Gn4
	.byte	W06
	.byte		N17   , An3 , v092
	.byte		N17   , Fn4 , v100
	.byte	W18
	.byte		        An3 , v048
	.byte		N17   , Fn4
	.byte	W18
	.byte		        As3 , v092
	.byte		N17   , Gn4 , v100
	.byte	W18
	.byte		N05   , As3 , v048
	.byte		N05   , Gn4
	.byte	W06
	.byte		N17   , An3 , v092
	.byte		N17   , Fn4 , v100
	.byte	W18
	.byte		N05   , An3 , v048
	.byte		N05   , Fn4
	.byte	W06
	.byte		N11   , Gn3 , v092
	.byte		N11   , En4 , v100
	.byte	W12
	.byte		N17   , Dn4 , v092
	.byte		N17   , An4 , v100
	.byte	W18
	.byte		N05   , Dn4 , v048
	.byte		N05   , An4
	.byte	W06
	.byte		N17   , Bn3 , v092
	.byte		N17   , Gn4 , v100
	.byte	W18
	.byte		N05   , Bn3 , v048
	.byte		N05   , Gn4
	.byte	W06
	.byte		N17   , An3 , v092
	.byte		N17   , Fn4 , v100
	.byte	W18
	.byte		N05   , An3 , v048
	.byte		N05   , Fn4
	.byte	W06
	.byte		N17   , Gn3 , v092
	.byte		N17   , En4 , v100
	.byte	W18
	.byte		N05   , Gn3 , v048
	.byte		N05   , En4
	.byte	W06
	.byte		N17   , Fn3 , v092
	.byte		N17   , Dn4 , v100
	.byte	W18
	.byte		        Fn3 , v048
	.byte		N17   , Dn4
	.byte	W18
	.byte		        Gn3 , v092
	.byte		N17   , Fn4 , v100
	.byte	W18
	.byte		N05   , Gn3 , v048
	.byte		N05   , Fn4
	.byte	W06
	.byte		N17   , Fn3 , v092
	.byte		N17   , En4 , v100
	.byte	W18
	.byte		N05   , Fn3 , v048
	.byte		N05   , En4
	.byte	W06
	.byte		N11   , Gn3 , v092
	.byte		N11   , Fn4 , v100
	.byte	W12
	.byte		N17   , Fn3 , v092
	.byte		N17   , Cn4 , v100
	.byte	W18
	.byte		        Fn3 , v048
	.byte		N17   , Cn4
	.byte	W18
	.byte		        Gs3 , v092
	.byte		N17   , Fn4 , v100
	.byte	W18
	.byte		N05   , Gs3 , v048
	.byte		N05   , Fn4
	.byte	W06
	.byte		N17   , Gn3 , v092
	.byte		N17   , En4 , v100
	.byte	W18
	.byte		N05   , Gn3 , v048
	.byte		N05   , En4
	.byte	W06
	.byte		N11   , Gs3 , v092
	.byte		N11   , Fn4 , v100
	.byte	W12
	.byte		N17   , Cn4 , v092
	.byte		N17   , Gs4 , v100
	.byte	W18
	.byte		N05   , Cn4 , v048
	.byte		N05   , Gs4
	.byte	W06
	.byte		N17   , Dn4 , v092
	.byte		N17   , As4 , v100
	.byte	W18
	.byte		N05   , Dn4 , v048
	.byte		N05   , As4
	.byte	W06
	.byte		N17   , En4 , v092
	.byte		N17   , Cn5 , v100
	.byte	W18
	.byte		N05   , En4 , v048
	.byte		N05   , Cn5
	.byte	W06
	.byte		N17   , Fn3 , v092
	.byte		N17   , Cn4 , v100
	.byte	W18
	.byte		N05   , Fn3 , v048
	.byte		N05   , Cn4
	.byte	W06
	.byte		N17   , As3 , v092
	.byte		N17   , Fn4 , v100
	.byte	W18
	.byte		N05   , As3 , v048
	.byte		N05   , Fn4
	.byte	W06
	.byte		N17   , Gn3 , v092
	.byte		N17   , En4 , v100
	.byte	W18
	.byte		N05   , Gn3 , v048
	.byte		N05   , En4
	.byte	W06
	.byte		N17   , Cn4 , v092
	.byte		N17   , An4 , v100
	.byte	W18
	.byte		N05   , Cn4 , v048
	.byte		N05   , An4
	.byte	W06
	.byte		N17   , As3 , v092
	.byte		N17   , Gn4 , v100
	.byte	W18
	.byte		N05   , As3 , v048
	.byte		N05   , Gn4
	.byte	W06
	.byte		VOL   , 32*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v+25
	.byte		N05   , Fn5 , v100
	.byte	W06
	.byte		N11   , Fn5 , v048
	.byte	W12
	.byte		PAN   , c_v-17
	.byte		VOL   , 31*mus_qz_643_mvl/mxv
	.byte		N05   , Fn5 , v100
	.byte	W06
	.byte		N11   , Fn5 , v048
	.byte	W12
	.byte		PAN   , c_v+25
	.byte		VOL   , 21*mus_qz_643_mvl/mxv
	.byte		N05   , Fn5 , v100
	.byte	W06
	.byte		N11   , Fn5 , v048
	.byte	W12
	.byte		PAN   , c_v-17
	.byte		VOL   , 13*mus_qz_643_mvl/mxv
	.byte		N05   , Fn5 , v100
	.byte	W06
	.byte		N11   , Fn5 , v048
	.byte	W12
	.byte		PAN   , c_v+25
	.byte		VOL   , 7*mus_qz_643_mvl/mxv
	.byte		N05   , Fn5 , v100
	.byte	W06
	.byte		N11   , Fn5 , v048
	.byte	W12
	.byte		PAN   , c_v-17
	.byte		N05   , Fn5 , v100
	.byte	W06
	.byte		PAN   , c_v+25
	.byte		VOL   , 32*mus_qz_643_mvl/mxv
	.byte		N05   , En5
	.byte	W06
	.byte		N11   , En5 , v048
	.byte	W12
	.byte		PAN   , c_v-17
	.byte		VOL   , 31*mus_qz_643_mvl/mxv
	.byte		N05   , En5 , v100
	.byte	W06
	.byte		N11   , En5 , v048
	.byte	W12
	.byte		PAN   , c_v+25
	.byte		VOL   , 21*mus_qz_643_mvl/mxv
	.byte		N05   , En5 , v100
	.byte	W06
	.byte		N11   , En5 , v048
	.byte	W12
	.byte		PAN   , c_v-17
	.byte		VOL   , 13*mus_qz_643_mvl/mxv
	.byte		N05   , En5 , v100
	.byte	W06
	.byte		N11   , En5 , v048
	.byte	W12
	.byte		PAN   , c_v+25
	.byte		VOL   , 7*mus_qz_643_mvl/mxv
	.byte		N05   , En5 , v100
	.byte	W06
	.byte		N11   , En5 , v048
	.byte	W12
	.byte		PAN   , c_v-17
	.byte		N05   , En5 , v100
	.byte	W06
	.byte		PAN   , c_v+25
	.byte		VOL   , 32*mus_qz_643_mvl/mxv
	.byte		N05   , Fn5
	.byte	W06
	.byte		N11   , Fn5 , v048
	.byte	W12
	.byte		PAN   , c_v-17
	.byte		VOL   , 31*mus_qz_643_mvl/mxv
	.byte		N05   , Fn5 , v100
	.byte	W06
	.byte		N11   , Fn5 , v048
	.byte	W12
	.byte		PAN   , c_v+25
	.byte		VOL   , 21*mus_qz_643_mvl/mxv
	.byte		N05   , Fn5 , v100
	.byte	W06
	.byte		N11   , Fn5 , v048
	.byte	W12
	.byte		PAN   , c_v-17
	.byte		VOL   , 13*mus_qz_643_mvl/mxv
	.byte		N05   , Fn5 , v100
	.byte	W06
	.byte		N11   , Fn5 , v048
	.byte	W12
	.byte		PAN   , c_v+25
	.byte		VOL   , 7*mus_qz_643_mvl/mxv
	.byte		N05   , Fn5 , v100
	.byte	W06
	.byte		N11   , Fn5 , v048
	.byte	W12
	.byte		PAN   , c_v-17
	.byte		N05   , Fn5 , v100
	.byte	W06
	.byte		VOL   , 32*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v+25
	.byte		N05   , Gn5
	.byte	W06
	.byte		N11   , Gn5 , v048
	.byte	W12
	.byte		PAN   , c_v-17
	.byte		VOL   , 31*mus_qz_643_mvl/mxv
	.byte		N05   , Gn5 , v100
	.byte	W06
	.byte		N11   , Gn5 , v048
	.byte	W12
	.byte		PAN   , c_v+25
	.byte		VOL   , 21*mus_qz_643_mvl/mxv
	.byte		N05   , Gn5 , v100
	.byte	W06
	.byte		N11   , Gn5 , v048
	.byte	W12
	.byte		PAN   , c_v-17
	.byte		VOL   , 13*mus_qz_643_mvl/mxv
	.byte		N05   , Gn5 , v100
	.byte	W06
	.byte		N11   , Gn5 , v048
	.byte	W12
	.byte		PAN   , c_v+25
	.byte		VOL   , 7*mus_qz_643_mvl/mxv
	.byte		N05   , Gn5 , v100
	.byte	W06
	.byte		N11   , Gn5 , v048
	.byte	W12
	.byte		PAN   , c_v-17
	.byte		N05   , Gn5 , v100
	.byte	W06
	.byte		VOL   , 41*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v-21
	.byte		N17   , En3
	.byte		N17   , Cn4
	.byte	W18
	.byte		PAN   , c_v+11
	.byte		N17   , En3 , v068
	.byte		N17   , Cn4
	.byte	W18
	.byte		PAN   , c_v-21
	.byte		N17   , An3 , v100
	.byte		N17   , Fn4
	.byte	W18
	.byte		PAN   , c_v+11
	.byte		N17   , An3 , v068
	.byte		N17   , Fn4
	.byte	W18
	.byte		PAN   , c_v-21
	.byte		N17   , Cn4 , v100
	.byte		N17   , As4
	.byte	W18
	.byte		PAN   , c_v+11
	.byte		N17   , Cn4 , v068
	.byte		N17   , As4
	.byte	W06
	.byte	W12
	.byte		PAN   , c_v-21
	.byte		N17   , Dn4 , v100
	.byte		N17   , Cn5
	.byte	W18
	.byte		PAN   , c_v+11
	.byte		N17   , Dn4 , v068
	.byte		N17   , Cn5
	.byte	W18
	.byte		PAN   , c_v-21
	.byte		N17   , An4 , v100
	.byte		N17   , Fn5
	.byte	W18
	.byte		PAN   , c_v+11
	.byte		N05   , An4 , v068
	.byte		N05   , Fn5
	.byte	W06
	.byte		PAN   , c_v-21
	.byte		N17   , An3 , v100
	.byte		N17   , Fn4
	.byte	W18
	.byte		PAN   , c_v+11
	.byte		N05   , An3 , v068
	.byte		N05   , Fn4
	.byte	W06
	.byte		PAN   , c_v-28
	.byte		VOL   , 31*mus_qz_643_mvl/mxv
	.byte		N17   , Dn4 , v100
	.byte		N17   , An4
	.byte	W18
	.byte		PAN   , c_v+11
	.byte		N17   , Dn4 , v068
	.byte		N17   , An4
	.byte	W18
	.byte		PAN   , c_v-28
	.byte		N17   , Fn3 , v100
	.byte		N17   , Cn4
	.byte	W18
	.byte		PAN   , c_v+11
	.byte		N40   , Fn3 , v068 , gtp1
	.byte		N40   , Cn4 , v068 , gtp1
	.byte	W42
	.byte		PAN   , c_v-28
	.byte		N17   , Dn4 , v100
	.byte		N17   , Gn4
	.byte	W18
	.byte		PAN   , c_v+11
	.byte		N17   , Dn4 , v068
	.byte		N17   , Gn4
	.byte	W18
	.byte		PAN   , c_v-28
	.byte		N17   , Fn3 , v100
	.byte		N17   , As3
	.byte	W18
	.byte		PAN   , c_v+11
	.byte		N17   , Fn3 , v068
	.byte		N17   , As3
	.byte	W18
	.byte		PAN   , c_v-28
	.byte		N17   , Dn3 , v100
	.byte		N17   , Gn3
	.byte	W18
	.byte		PAN   , c_v+11
	.byte		N05   , Dn3 , v068
	.byte		N05   , Gn3
	.byte	W06
	.byte		PAN   , c_v+25
	.byte		VOL   , 62*mus_qz_643_mvl/mxv
	.byte		N05   , Fn5 , v100
	.byte	W06
	.byte		N11   , Fn5 , v048
	.byte	W12
	.byte		PAN   , c_v-17
	.byte		VOL   , 48*mus_qz_643_mvl/mxv
	.byte		N05   , Fn5 , v100
	.byte	W06
	.byte		N11   , Fn5 , v048
	.byte	W12
	.byte		PAN   , c_v+25
	.byte		VOL   , 37*mus_qz_643_mvl/mxv
	.byte		N05   , Fn5 , v100
	.byte	W06
	.byte		N11   , Fn5 , v048
	.byte	W12
	.byte		PAN   , c_v-17
	.byte		VOL   , 27*mus_qz_643_mvl/mxv
	.byte		N05   , Fn5 , v100
	.byte	W06
	.byte		N11   , Fn5 , v048
	.byte	W12
	.byte		PAN   , c_v+25
	.byte		VOL   , 17*mus_qz_643_mvl/mxv
	.byte		N05   , Fn5 , v100
	.byte	W06
	.byte		N11   , Fn5 , v048
	.byte	W12
	.byte		PAN   , c_v-17
	.byte		N05   , Fn5 , v100
	.byte	W06
	.byte	GOTO
	 ptrvalue	mus_qz_643_1_B1
	.byte	FINE

/********************** Track  2 **********************/

mus_qz_643_2:
	.byte		KEYSH , mus_qz_643_key+0
	.byte		VOL   , 57*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		BENDR , 6
	.byte		PRIO  , 64
	.byte		VOL   , 57*mus_qz_643_mvl/mxv
	.byte		PRIO  , 61
	.byte	W18
	.byte		PAN   , c_v-32
	.byte		VOL   , 45*mus_qz_643_mvl/mxv
	.byte	W06
	.byte		VOICE , 0
	.byte	W24
	.byte		N05   , Cn2 , v100
	.byte	W06
	.byte		        Fn2
	.byte	W06
	.byte		        Gn2
	.byte	W06
	.byte		        As2
	.byte	W06
	.byte		        An2
	.byte	W06
	.byte		        Cn3
	.byte	W06
	.byte		N40   , Fn3 , v100 , gtp1
	.byte	W42
	.byte		N05   , En3
	.byte	W06
	.byte		        Fn3
	.byte	W06
	.byte		        Gn3
	.byte	W06
mus_qz_643_2_B1:
	.byte		N23   , Fn3 , v100
	.byte	W24
	.byte		N28   , Cn4 , v100 , gtp1
	.byte	W30
	.byte		N05   , As3
	.byte	W06
	.byte		        An3
	.byte	W06
	.byte		        As3
	.byte	W06
	.byte		        An3
	.byte	W06
	.byte		        Gn3
	.byte	W06
	.byte		        Fn3
	.byte	W06
	.byte		        En3
	.byte	W06
	.byte		        Fn3
	.byte	W06
	.byte		        Gn3
	.byte	W06
	.byte		        Fn3
	.byte	W06
	.byte		N28   , Cn3 , v100 , gtp1
	.byte	W30
	.byte		N05   , Fn3
	.byte	W06
	.byte		        En3
	.byte	W06
	.byte		        Dn3
	.byte	W06
	.byte		        En3
	.byte	W06
	.byte		N11   , Fn3
	.byte	W12
	.byte		        Cn3
	.byte	W12
	.byte		N05   , As2
	.byte	W06
	.byte		        Cn3
	.byte	W06
	.byte		N17   , Fn3
	.byte	W18
	.byte		N05   , En3
	.byte	W06
	.byte		        Fn3
	.byte	W06
	.byte		        Gn3
	.byte	W06
	.byte		N23   , Fn3
	.byte	W24
	.byte		        Gn3
	.byte	W24
	.byte		N05   , An3
	.byte	W06
	.byte		        Gn3
	.byte	W06
	.byte		        Fn3
	.byte	W06
	.byte		        En3
	.byte	W06
	.byte		        Fn3
	.byte	W06
	.byte		        Cn3
	.byte	W06
	.byte		        As2
	.byte	W06
	.byte		N32   , An2 , v100 , gtp3
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
	.byte	W84
	.byte		VOICE , 35
	.byte		PAN   , c_v+22
	.byte		VOL   , 73*mus_qz_643_mvl/mxv
	.byte	W06
	.byte		BEND  , c_v+1
	.byte	W06
	.byte		N32   , Fn4 , v100 , gtp3
	.byte	W36
	.byte		N23   , Gn4
	.byte	W24
	.byte		        Fn4
	.byte	W24
	.byte		N11   , Cn4
	.byte	W12
	.byte		N23   , Cn5
	.byte	W24
	.byte		        An4
	.byte	W24
	.byte		        Dn5
	.byte	W24
	.byte		        En5
	.byte	W24
	.byte		N32   , Fn5 , v100 , gtp3
	.byte	W36
	.byte		N23   , Fn4
	.byte	W24
	.byte		        Gn4
	.byte	W24
	.byte		N11   , Fn4
	.byte	W12
	.byte		N23   , Cn5
	.byte	W24
	.byte		        As4
	.byte	W24
	.byte		        Fn4
	.byte	W24
	.byte		        Cs4
	.byte	W24
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W66
	.byte		BEND  , c_v+0
	.byte	W18
	.byte		PAN   , c_v-32
	.byte	W06
	.byte		VOICE , 0
	.byte	W06
	.byte		VOL   , 45*mus_qz_643_mvl/mxv
	.byte		N05   , Cn2
	.byte	W06
	.byte		        Fn2
	.byte	W06
	.byte		        Gn2
	.byte	W06
	.byte		        As2
	.byte	W06
	.byte		        An2
	.byte	W06
	.byte		        Cn3
	.byte	W06
	.byte		N40   , Fn3 , v100 , gtp1
	.byte	W42
	.byte		N05   , En3
	.byte	W06
	.byte		        Fn3
	.byte	W06
	.byte		        Gn3
	.byte	W06
	.byte	GOTO
	 ptrvalue	mus_qz_643_2_B1
	.byte	FINE

/********************** Track  3 **********************/

mus_qz_643_3:
	.byte		KEYSH , mus_qz_643_key+0
	.byte		VOL   , 57*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte		VOL   , 57*mus_qz_643_mvl/mxv
	.byte		PRIO  , 61
	.byte	W36
	.byte		PAN   , c_v+20
	.byte	W03
	.byte		VOICE , 23
	.byte	W03
	.byte		PAN   , c_v+11
	.byte	W03
	.byte		VOL   , 66*mus_qz_643_mvl/mxv
	.byte	W03
mus_qz_643_3_000:
	.byte		N68   , Fn2 , v100 , gtp3
	.byte		N68   , Cn3 , v100 , gtp3
	.byte	W72
	.byte		N23   , An2
	.byte		N23   , En3
	.byte	W24
	.byte	PEND
mus_qz_643_3_B1:
	.byte		N68   , Cn3 , v100 , gtp3
	.byte		N68   , Fn3 , v100 , gtp3
	.byte	W72
	.byte		N23   , Gn2
	.byte		N23   , Cn3
	.byte	W24
	.byte		N44   , Cn3 , v100 , gtp3
	.byte		N44   , Fn3 , v100 , gtp3
	.byte	W48
	.byte		N23   , Fn2
	.byte		N23   , An2
	.byte	W24
	.byte		        Cn2
	.byte		N23   , Gn2
	.byte	W24
	.byte		N44   , Cn2 , v100 , gtp3
	.byte		N44   , Gs2 , v100 , gtp3
	.byte	W48
	.byte		        Dn2 , v100 , gtp3
	.byte		N44   , As2 , v100 , gtp3
	.byte	W44
	.byte	W01
	.byte		VOL   , 66*mus_qz_643_mvl/mxv
	.byte	W03
	.byte		N92   , Fn2 , v100 , gtp3
	.byte		N92   , Cn3 , v100 , gtp3
	.byte	W36
	.byte		VOL   , 60*mus_qz_643_mvl/mxv
	.byte	W05
	.byte		        54*mus_qz_643_mvl/mxv
	.byte	W07
	.byte		        43*mus_qz_643_mvl/mxv
	.byte	W05
	.byte		        33*mus_qz_643_mvl/mxv
	.byte	W07
	.byte		        21*mus_qz_643_mvl/mxv
	.byte	W05
	.byte		        10*mus_qz_643_mvl/mxv
	.byte	W07
	.byte		        3*mus_qz_643_mvl/mxv
	.byte	W05
	.byte		        0*mus_qz_643_mvl/mxv
	.byte	W19
	.byte		VOICE , 3
	.byte		PAN   , c_v-12
	.byte	W06
	.byte		VOL   , 54*mus_qz_643_mvl/mxv
	.byte	W06
	.byte		N11   , As2
	.byte	W12
	.byte		N05   , Fn3
	.byte	W06
	.byte	N11
	.byte	W18
	.byte		        Cn3
	.byte	W12
	.byte		N05   , Fn3
	.byte	W06
	.byte	N11
	.byte	W18
	.byte		        En3
	.byte	W12
	.byte		        Fn3
	.byte	W12
	.byte		N05   , Cn3
	.byte	W06
	.byte	N11
	.byte	W18
	.byte		        Dn3
	.byte	W12
	.byte		        Cn3
	.byte	W12
	.byte		N05   , Fn3
	.byte	W06
	.byte	N11
	.byte	W30
	.byte	W12
	.byte		        As2
	.byte	W12
	.byte		N05   , Fn3
	.byte	W06
	.byte	N11
	.byte	W18
	.byte		        Cn3
	.byte	W12
	.byte		N23   , An3
	.byte	W24
	.byte		N11   , As3
	.byte	W12
	.byte		        An3
	.byte	W12
	.byte		N05   , Cn3
	.byte	W06
	.byte	N11
	.byte	W18
	.byte		        En3
	.byte	W12
	.byte		        Fn3
	.byte	W12
	.byte		N05   , An2
	.byte	W06
	.byte	N11
	.byte	W18
	.byte		        Cn3
	.byte	W12
	.byte		        Dn3
	.byte	W12
	.byte		N05   , Fn2
	.byte	W06
	.byte	N11
	.byte	W18
	.byte		        Gn2
	.byte	W12
	.byte		        As2
	.byte	W12
	.byte		N05   , Fn2
	.byte	W06
	.byte	N11
	.byte	W18
	.byte		        Gn2
	.byte	W12
	.byte		        An2
	.byte	W12
	.byte		        Cn3
	.byte	W12
	.byte		N28   , Fn3 , v100 , gtp1
	.byte	W72
	.byte	W96
	.byte		VOICE , 23
	.byte		PAN   , c_v-24
	.byte		VOL   , 25*mus_qz_643_mvl/mxv
	.byte		N05   , Fn4
	.byte	W06
	.byte	N05
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Cn4
	.byte	W06
	.byte		        Fn4
	.byte	W06
	.byte		        Cn4
	.byte	W06
	.byte		        Fn4
	.byte	W06
	.byte		        Gn4
	.byte	W06
	.byte		        Fn4
	.byte	W06
	.byte	N05
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Cn4
	.byte	W06
	.byte		        Fn4
	.byte	W06
	.byte		        Cn4
	.byte	W06
	.byte		        As3
	.byte	W06
	.byte		        Cn4
	.byte	W06
	.byte		        An4
	.byte	W06
	.byte	N05
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Cn4
	.byte	W06
	.byte		        En4
	.byte	W06
	.byte		        Cn4
	.byte	W06
	.byte		        An4
	.byte	W06
	.byte		        As4
	.byte	W06
	.byte		        An4
	.byte	W06
	.byte	N05
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        En4
	.byte	W06
	.byte		        An4
	.byte	W06
	.byte		        Cn4
	.byte	W06
	.byte		        En4
	.byte	W06
	.byte		        Cn4
	.byte	W06
	.byte		        Fn4
	.byte	W06
	.byte	N05
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Cn4
	.byte	W06
	.byte		        Fn4
	.byte	W06
	.byte		        Cn4
	.byte	W06
	.byte		        Fn4
	.byte	W06
	.byte		        Gn4
	.byte	W06
	.byte		        Fn4
	.byte	W06
	.byte	N05
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Cn4
	.byte	W06
	.byte		        Fn4
	.byte	W06
	.byte		        Cn4
	.byte	W06
	.byte		        An3
	.byte	W06
	.byte		        Cn4
	.byte	W06
	.byte		        Fn4
	.byte	W06
	.byte	N05
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Cn4
	.byte	W06
	.byte		        Fn4
	.byte	W06
	.byte		        Cn4
	.byte	W06
	.byte		        Fn4
	.byte	W06
	.byte		        Gn4
	.byte	W06
	.byte		        Fn4
	.byte	W06
	.byte	N05
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Cn4
	.byte	W06
	.byte		        Fn4
	.byte	W06
	.byte		        Cn4
	.byte	W06
	.byte		        As3
	.byte	W06
	.byte		        Cn4
	.byte	W06
	.byte		        Fn4
	.byte	W06
	.byte		        As3
	.byte	W06
	.byte		        Dn4
	.byte	W06
	.byte		        Fn3
	.byte	W06
	.byte		        As3
	.byte	W06
	.byte		        Fn3
	.byte	W06
	.byte		        Fn4
	.byte	W06
	.byte		        Gn4
	.byte	W06
	.byte		        Fn4
	.byte	W06
	.byte		        As3
	.byte	W06
	.byte		        Dn4
	.byte	W06
	.byte		        Fn3
	.byte	W06
	.byte		        As3
	.byte	W06
	.byte		        Fn3
	.byte	W06
	.byte		        Fn4
	.byte	W06
	.byte		        Gn4
	.byte	W06
	.byte		        Fn4
	.byte	W06
	.byte		        As3
	.byte	W06
	.byte		        Cn4
	.byte	W06
	.byte		        Fn3
	.byte	W06
	.byte		        As3
	.byte	W06
	.byte		        Fn3
	.byte	W06
	.byte		        Fn4
	.byte	W06
	.byte		        Gn4
	.byte	W06
	.byte		        Fn4
	.byte	W06
	.byte		        As3
	.byte	W06
	.byte		        Cn4
	.byte	W06
	.byte		        Fn3
	.byte	W06
	.byte		        As3
	.byte	W06
	.byte		        Fn3
	.byte	W06
	.byte		        Fn4
	.byte	W06
	.byte		        Gn4
	.byte	W06
	.byte		        Cn5
	.byte	W06
	.byte		        Gs4
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Cn4
	.byte	W06
	.byte		        Dn5
	.byte	W06
	.byte		        As4
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Dn4
	.byte	W06
	.byte		        En5
	.byte	W06
	.byte		        Cn5
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        En4
	.byte	W06
	.byte		        Fn5
	.byte	W06
	.byte		        Cn5
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Fn4
	.byte	W06
	.byte		        Fn5
	.byte	W06
	.byte		        Cn5
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Fn4
	.byte	W06
	.byte		        Cn5
	.byte	W06
	.byte		        Fn4
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Cn4
	.byte	W06
	.byte		        Cn3
	.byte	W06
	.byte		        Fn3
	.byte	W06
	.byte		        Gn3
	.byte	W06
	.byte		        Cn4
	.byte	W06
	.byte		        Fn4
	.byte	W06
	.byte		        Gn4
	.byte	W06
	.byte		        Cn5
	.byte	W06
	.byte		        Gn5
	.byte	W06
	.byte		VOICE , 4
	.byte		PAN   , c_v-22
	.byte		VOL   , 23*mus_qz_643_mvl/mxv
	.byte		N05   , Fn4
	.byte	W05
	.byte		PAN   , c_v-18
	.byte	W01
	.byte		N05   , Cn4
	.byte	W06
	.byte		PAN   , c_v-14
	.byte		N05   , Fn4
	.byte	W05
	.byte		PAN   , c_v-12
	.byte	W01
	.byte		N05   , Gn4
	.byte	W06
	.byte		        Cn5
	.byte	W05
	.byte		PAN   , c_v-8
	.byte	W01
	.byte		N05   , Fn5
	.byte	W06
	.byte		PAN   , c_v-4
	.byte		N05   , Gn5
	.byte	W05
	.byte		PAN   , c_v+0
	.byte	W01
	.byte		N05   , Cn6
	.byte	W06
	.byte		PAN   , c_v+3
	.byte		N05   , Cn5
	.byte	W05
	.byte		PAN   , c_v+7
	.byte	W01
	.byte		N05   , Fn4
	.byte	W06
	.byte		PAN   , c_v+10
	.byte		N05   , Gn4
	.byte	W05
	.byte		PAN   , c_v+14
	.byte	W01
	.byte		N05   , Cn5
	.byte	W06
	.byte		        Fn5
	.byte	W05
	.byte		PAN   , c_v+18
	.byte	W01
	.byte		N05   , Gn5
	.byte	W06
	.byte		PAN   , c_v+20
	.byte		N05   , Cn6
	.byte	W05
	.byte		PAN   , c_v+24
	.byte	W01
	.byte		N05   , Fn6
	.byte	W06
	.byte		        Fn4
	.byte	W06
	.byte		PAN   , c_v+24
	.byte		N05   , Cn4
	.byte	W06
	.byte		PAN   , c_v+18
	.byte		N05   , Fn4
	.byte	W05
	.byte		PAN   , c_v+10
	.byte	W01
	.byte		N05   , Gn4
	.byte	W06
	.byte		PAN   , c_v+7
	.byte		N05   , Cn5
	.byte	W05
	.byte		PAN   , c_v+0
	.byte	W01
	.byte		N05   , Fn5
	.byte	W06
	.byte		PAN   , c_v-2
	.byte		N05   , Gn5
	.byte	W05
	.byte		PAN   , c_v-6
	.byte	W01
	.byte		N05   , Cn6
	.byte	W06
	.byte		PAN   , c_v-8
	.byte		N05   , Dn5
	.byte	W05
	.byte		PAN   , c_v-10
	.byte	W01
	.byte		N05   , Fn4
	.byte	W06
	.byte		PAN   , c_v-14
	.byte		N05   , Gn4
	.byte	W05
	.byte		PAN   , c_v-18
	.byte	W01
	.byte		N05   , Dn5
	.byte	W06
	.byte		PAN   , c_v-25
	.byte		N05   , En5
	.byte	W05
	.byte		PAN   , c_v-27
	.byte	W01
	.byte		N05   , Fn5
	.byte	W06
	.byte		PAN   , c_v-29
	.byte		N05   , Dn6
	.byte	W06
	.byte		        Fn6
	.byte	W06
	.byte		PAN   , c_v-25
	.byte		N05   , Gn4
	.byte	W05
	.byte		PAN   , c_v-20
	.byte	W01
	.byte		N05   , Bn3
	.byte	W06
	.byte		PAN   , c_v-18
	.byte		N05   , Fn4
	.byte	W05
	.byte		PAN   , c_v-14
	.byte	W01
	.byte		N05   , Gn4
	.byte	W06
	.byte		        Bn4
	.byte	W05
	.byte		PAN   , c_v-10
	.byte	W01
	.byte		N05   , Dn5
	.byte	W06
	.byte		PAN   , c_v-6
	.byte		N05   , Bn5
	.byte	W05
	.byte		PAN   , c_v-4
	.byte	W01
	.byte		N05   , Dn6
	.byte	W06
	.byte		PAN   , c_v+0
	.byte		N05   , Dn5
	.byte	W05
	.byte		PAN   , c_v+3
	.byte	W01
	.byte		N05   , Gn4
	.byte	W06
	.byte		PAN   , c_v+5
	.byte		N05   , Bn4
	.byte	W05
	.byte		PAN   , c_v+7
	.byte	W01
	.byte		N05   , Dn5
	.byte	W06
	.byte		PAN   , c_v+10
	.byte		N05   , Fn5
	.byte	W05
	.byte		PAN   , c_v+14
	.byte	W01
	.byte		N05   , Gn5
	.byte	W06
	.byte		PAN   , c_v+18
	.byte		N05   , Bn5
	.byte	W06
	.byte		        Fn6
	.byte	W06
	.byte		PAN   , c_v+24
	.byte		N05   , Gn4
	.byte	W05
	.byte		PAN   , c_v+20
	.byte	W01
	.byte		N05   , As3
	.byte	W06
	.byte		PAN   , c_v+18
	.byte		N05   , Cs4
	.byte	W05
	.byte		PAN   , c_v+16
	.byte	W01
	.byte		N05   , Gn4
	.byte	W06
	.byte		PAN   , c_v+14
	.byte		N05   , As4
	.byte	W05
	.byte		PAN   , c_v+12
	.byte	W01
	.byte		N05   , Cn5
	.byte	W06
	.byte		PAN   , c_v+10
	.byte		N05   , Fn5
	.byte	W06
	.byte		        As5
	.byte	W06
	.byte		PAN   , c_v+5
	.byte		N05   , Cs5
	.byte	W05
	.byte		PAN   , c_v+3
	.byte	W01
	.byte		N05   , Fn4
	.byte	W06
	.byte		PAN   , c_v+0
	.byte		N05   , As4
	.byte	W06
	.byte		        Cs5
	.byte	W06
	.byte		        Fn5
	.byte	W05
	.byte		PAN   , c_v-2
	.byte	W01
	.byte		N05   , Gn5
	.byte	W06
	.byte		        As5
	.byte	W05
	.byte		PAN   , c_v-4
	.byte	W01
	.byte		N05   , Fn6
	.byte	W06
	.byte		PAN   , c_v-29
	.byte		N05   , Cn4
	.byte	W05
	.byte		PAN   , c_v-25
	.byte	W01
	.byte		N05   , Fn4
	.byte	W06
	.byte		PAN   , c_v-20
	.byte		N05   , Gn4
	.byte	W05
	.byte		PAN   , c_v-16
	.byte	W01
	.byte		N05   , Fn4
	.byte	W06
	.byte		PAN   , c_v-14
	.byte		N05   , As4
	.byte	W05
	.byte		PAN   , c_v-8
	.byte	W01
	.byte		N05   , Cn5
	.byte	W06
	.byte		PAN   , c_v-6
	.byte		N05   , Gn5
	.byte	W05
	.byte		PAN   , c_v-4
	.byte	W01
	.byte		N05   , Fn5
	.byte	W06
	.byte		        Gn5
	.byte	W05
	.byte		PAN   , c_v+0
	.byte	W01
	.byte		N05   , As5
	.byte	W06
	.byte		PAN   , c_v+3
	.byte		N05   , Cn6
	.byte	W05
	.byte		PAN   , c_v+5
	.byte	W01
	.byte		N05   , As5
	.byte	W06
	.byte		PAN   , c_v+10
	.byte		N05   , Cn6
	.byte	W05
	.byte		PAN   , c_v+14
	.byte	W01
	.byte		N05   , Fn6
	.byte	W06
	.byte		PAN   , c_v+16
	.byte		N05   , Gn6
	.byte	W05
	.byte		PAN   , c_v+18
	.byte	W01
	.byte		N05   , Cn7
	.byte	W06
	.byte		PAN   , c_v-14
	.byte		N05   , Cn4
	.byte	W05
	.byte		PAN   , c_v-12
	.byte	W01
	.byte		N05   , Dn4
	.byte	W06
	.byte		PAN   , c_v-8
	.byte		N05   , An4
	.byte	W05
	.byte		PAN   , c_v-2
	.byte	W01
	.byte		N05   , Dn4
	.byte	W06
	.byte		        An4
	.byte	W05
	.byte		PAN   , c_v+5
	.byte	W01
	.byte		N05   , Fn4
	.byte	W06
	.byte		PAN   , c_v+10
	.byte		N05   , An4
	.byte	W05
	.byte		PAN   , c_v+12
	.byte	W01
	.byte		N05   , Cn5
	.byte	W06
	.byte		        Dn5
	.byte	W05
	.byte		PAN   , c_v+14
	.byte	W01
	.byte		N05   , Cn5
	.byte	W06
	.byte		PAN   , c_v+18
	.byte		N05   , Dn5
	.byte	W05
	.byte		PAN   , c_v+20
	.byte	W01
	.byte		N05   , An5
	.byte	W06
	.byte		PAN   , c_v+26
	.byte		N05   , Fn5
	.byte	W05
	.byte		PAN   , c_v+28
	.byte	W01
	.byte		N05   , An5
	.byte	W06
	.byte		PAN   , c_v+35
	.byte		N05   , Cn6
	.byte	W05
	.byte		PAN   , c_v+39
	.byte	W01
	.byte		N05   , Fn6
	.byte	W06
	.byte	W96
	.byte	W78
	.byte		VOICE , 23
	.byte	W12
	.byte		PAN   , c_v+20
	.byte		VOL   , 23*mus_qz_643_mvl/mxv
	.byte		        66*mus_qz_643_mvl/mxv
	.byte	W06
	.byte	PATT
	 ptrvalue	mus_qz_643_3_000
	.byte	GOTO
	 ptrvalue	mus_qz_643_3_B1
	.byte	FINE

/********************** Track  4 **********************/

mus_qz_643_4:
	.byte		KEYSH , mus_qz_643_key+0
	.byte		VOICE , 27
	.byte		VOL   , 57*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte		VOL   , 57*mus_qz_643_mvl/mxv
	.byte		PRIO  , 61
	.byte		PAN   , c_v+7
	.byte		VOL   , 50*mus_qz_643_mvl/mxv
	.byte		N03   , Cn6 , v100
	.byte	W04
	.byte		        Gn5
	.byte	W04
	.byte		        Fn5
	.byte	W04
	.byte		        Gn5
	.byte	W04
	.byte		        Fn5
	.byte	W04
	.byte		        Cn5
	.byte	W04
	.byte		        Fn5
	.byte	W04
	.byte		        Cn5
	.byte	W04
	.byte		        As4
	.byte	W04
	.byte		        Cn5
	.byte	W04
	.byte		        Gn4
	.byte	W04
	.byte		        Cn4
	.byte	W04
	.byte		VOL   , 44*mus_qz_643_mvl/mxv
	.byte		N03   , An3
	.byte	W04
	.byte		        Cn4
	.byte	W04
	.byte		        Fn4
	.byte	W04
	.byte		        Gn4
	.byte	W04
	.byte		        An4
	.byte	W04
	.byte		        Cn5
	.byte	W04
	.byte		        An3
	.byte	W04
	.byte		        Gn3
	.byte	W04
	.byte		        Fn3
	.byte	W04
	.byte		        Cn3
	.byte	W04
	.byte		        As2
	.byte	W04
	.byte		        An2
	.byte	W04
	.byte		        Cn3
	.byte	W04
	.byte		        Fn3
	.byte	W04
	.byte		        Gn3
	.byte	W04
	.byte		        An3
	.byte	W04
	.byte		        Cn4
	.byte	W04
	.byte		        Fn4
	.byte	W04
	.byte		        An3
	.byte	W04
	.byte		        Cn4
	.byte	W04
	.byte		        Fn4
	.byte	W04
	.byte		        Gn4
	.byte	W04
	.byte		        An4
	.byte	W04
	.byte		        Cn5
	.byte	W04
mus_qz_643_4_B1:
	.byte		N03   , An3 , v100
	.byte	W04
	.byte		        As3
	.byte	W04
	.byte		        Dn4
	.byte	W04
	.byte		        Fn4
	.byte	W04
	.byte		        An4
	.byte	W04
	.byte		        Cn5
	.byte	W04
	.byte		        As3
	.byte	W04
	.byte		        An3
	.byte	W04
	.byte		        Fn3
	.byte	W04
	.byte		        Dn3
	.byte	W04
	.byte		        As2
	.byte	W04
	.byte		        An2
	.byte	W04
	.byte		        As2
	.byte	W04
	.byte		        Cn3
	.byte	W04
	.byte		        Dn3
	.byte	W04
	.byte		        Fn3
	.byte	W04
	.byte		        An3
	.byte	W04
	.byte		        Cn4
	.byte	W04
	.byte		        An3
	.byte	W04
	.byte		        As3
	.byte	W04
	.byte		        Dn4
	.byte	W04
	.byte		        Fn4
	.byte	W04
	.byte		        An4
	.byte	W04
	.byte		        Cn5
	.byte	W04
	.byte		        An3
	.byte	W04
	.byte		        Dn4
	.byte	W04
	.byte		        En4
	.byte	W04
	.byte		        Fn4
	.byte	W04
	.byte		        An4
	.byte	W04
	.byte		        Cn5
	.byte	W04
	.byte		        Dn4
	.byte	W04
	.byte		        An3
	.byte	W04
	.byte		        Fn3
	.byte	W04
	.byte		        En3
	.byte	W04
	.byte		        Dn3
	.byte	W04
	.byte		        An2
	.byte	W04
	.byte		        Cn3
	.byte	W04
	.byte		        Dn3
	.byte	W04
	.byte		        En3
	.byte	W04
	.byte		        Fn3
	.byte	W04
	.byte		        An3
	.byte	W04
	.byte		        Cn4
	.byte	W04
	.byte		        An3
	.byte	W04
	.byte		        Dn4
	.byte	W04
	.byte		        En4
	.byte	W04
	.byte		        Fn4
	.byte	W04
	.byte		        An4
	.byte	W04
	.byte		        Cn5
	.byte	W04
	.byte		        Gs2
	.byte	W04
	.byte		        As2
	.byte	W04
	.byte		        Cn3
	.byte	W04
	.byte		        Fn3
	.byte	W04
	.byte		        Gs3
	.byte	W04
	.byte		        Cn4
	.byte	W04
	.byte		        Gs3
	.byte	W04
	.byte		        As3
	.byte	W04
	.byte		        Cn4
	.byte	W04
	.byte		        Fn4
	.byte	W04
	.byte		        Gs4
	.byte	W04
	.byte		        Cn5
	.byte	W04
	.byte		        As2
	.byte	W04
	.byte		        Dn3
	.byte	W04
	.byte		        Fn3
	.byte	W04
	.byte		        An3
	.byte	W04
	.byte		        As3
	.byte	W04
	.byte		        Cn4
	.byte	W04
	.byte		        As3
	.byte	W04
	.byte		        Cn4
	.byte	W04
	.byte		        En4
	.byte	W04
	.byte		        Gn4
	.byte	W04
	.byte		        As4
	.byte	W04
	.byte		        Cn5
	.byte	W04
	.byte		N92   , Fn4 , v100 , gtp3
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W48
	.byte		VOL   , 33*mus_qz_643_mvl/mxv
	.byte		N05   , Cn2
	.byte	W02
	.byte		VOL   , 36*mus_qz_643_mvl/mxv
	.byte	W01
	.byte		N05   , Fn2
	.byte	W03
	.byte		        Gn2
	.byte	W02
	.byte		VOL   , 39*mus_qz_643_mvl/mxv
	.byte	W01
	.byte		N05   , As2
	.byte	W03
	.byte		VOL   , 40*mus_qz_643_mvl/mxv
	.byte		N05   , An2
	.byte	W02
	.byte		VOL   , 42*mus_qz_643_mvl/mxv
	.byte	W01
	.byte		N05   , Cn3
	.byte	W03
	.byte		        Fn3
	.byte	W02
	.byte		VOL   , 44*mus_qz_643_mvl/mxv
	.byte	W01
	.byte		N05   , Cn3
	.byte	W03
	.byte		VOL   , 48*mus_qz_643_mvl/mxv
	.byte		N05   , Gn3
	.byte	W02
	.byte		VOL   , 52*mus_qz_643_mvl/mxv
	.byte	W01
	.byte		N05   , As3
	.byte	W03
	.byte		VOL   , 55*mus_qz_643_mvl/mxv
	.byte		N05   , An3
	.byte	W02
	.byte		VOL   , 60*mus_qz_643_mvl/mxv
	.byte	W01
	.byte		N05   , Cn4
	.byte	W03
	.byte		VOL   , 68*mus_qz_643_mvl/mxv
	.byte		N05   , Fn4
	.byte	W03
	.byte		        Gn4
	.byte	W03
	.byte		        Cn4
	.byte	W03
	.byte		        As4
	.byte	W03
	.byte		VOL   , 60*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v+12
	.byte		N02   , An4
	.byte	W03
	.byte		        Gn4
	.byte	W02
	.byte		PAN   , c_v+9
	.byte	W01
	.byte		N02   , Fn4
	.byte	W03
	.byte		        Cn4
	.byte	W03
	.byte		VOL   , 54*mus_qz_643_mvl/mxv
	.byte		N02   , Fn4
	.byte	W03
	.byte		        Cn4
	.byte	W03
	.byte		        As3
	.byte	W03
	.byte		        An3
	.byte	W03
	.byte		VOL   , 51*mus_qz_643_mvl/mxv
	.byte		N02   , Fn4
	.byte	W03
	.byte		        Cn4
	.byte	W02
	.byte		PAN   , c_v+6
	.byte	W01
	.byte		N02   , As3
	.byte	W03
	.byte		        An3
	.byte	W03
	.byte		VOL   , 48*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v+3
	.byte		N02   , As3
	.byte	W03
	.byte		        An3
	.byte	W02
	.byte		VOL   , 47*mus_qz_643_mvl/mxv
	.byte	W01
	.byte		N02   , Fn3
	.byte	W03
	.byte		        Cn3
	.byte	W03
	.byte		VOL   , 45*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		N02   , An3
	.byte	W03
	.byte		        Gn3
	.byte	W02
	.byte		PAN   , c_v-2
	.byte		VOL   , 44*mus_qz_643_mvl/mxv
	.byte	W01
	.byte		N02   , Fn3
	.byte	W03
	.byte		        Cn3
	.byte	W03
	.byte		        Fn3
	.byte	W03
	.byte		        Cn3
	.byte	W02
	.byte		PAN   , c_v-5
	.byte		VOL   , 42*mus_qz_643_mvl/mxv
	.byte	W01
	.byte		N02   , As2
	.byte	W03
	.byte		        An2
	.byte	W03
	.byte		VOL   , 41*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v-8
	.byte		N02   , Fn3
	.byte	W03
	.byte		        Cn3
	.byte	W03
	.byte		        As2
	.byte	W03
	.byte		        An2
	.byte	W03
	.byte		VOL   , 39*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v-10
	.byte		N02   , As2
	.byte	W03
	.byte		        An2
	.byte	W02
	.byte		PAN   , c_v-8
	.byte	W01
	.byte		N02   , Fn2
	.byte	W03
	.byte		        Cn2
	.byte	W03
	.byte		VOL   , 60*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v+12
	.byte		N02   , An4
	.byte	W03
	.byte		        En4
	.byte	W02
	.byte		PAN   , c_v+9
	.byte	W01
	.byte		N02   , Cn4
	.byte	W03
	.byte		        An3
	.byte	W03
	.byte		VOL   , 54*mus_qz_643_mvl/mxv
	.byte		N02   , En4
	.byte	W03
	.byte		        Cn4
	.byte	W03
	.byte		        Gn3
	.byte	W03
	.byte		        En3
	.byte	W03
	.byte		VOL   , 51*mus_qz_643_mvl/mxv
	.byte		N02   , En4
	.byte	W03
	.byte		        Cn4
	.byte	W02
	.byte		PAN   , c_v+6
	.byte	W01
	.byte		N02   , Gn3
	.byte	W03
	.byte		        En3
	.byte	W03
	.byte		VOL   , 48*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v+3
	.byte		N02   , An3
	.byte	W03
	.byte		        Gn3
	.byte	W02
	.byte		VOL   , 47*mus_qz_643_mvl/mxv
	.byte	W01
	.byte		N02   , En3
	.byte	W03
	.byte		        Cn3
	.byte	W03
	.byte		VOL   , 45*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		N02   , An3
	.byte	W03
	.byte		        En3
	.byte	W02
	.byte		PAN   , c_v-2
	.byte		VOL   , 44*mus_qz_643_mvl/mxv
	.byte	W01
	.byte		N02   , Cn3
	.byte	W03
	.byte		        An2
	.byte	W03
	.byte		        En3
	.byte	W03
	.byte		        Cn3
	.byte	W02
	.byte		PAN   , c_v-5
	.byte		VOL   , 42*mus_qz_643_mvl/mxv
	.byte	W01
	.byte		N02   , Gn2
	.byte	W03
	.byte		        En2
	.byte	W03
	.byte		VOL   , 41*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v-8
	.byte		N02   , En3
	.byte	W03
	.byte		        Cn3
	.byte	W03
	.byte		        Gn2
	.byte	W03
	.byte		        En2
	.byte	W03
	.byte		VOL   , 39*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v-10
	.byte		N02   , An2
	.byte	W03
	.byte		        Gn2
	.byte	W02
	.byte		PAN   , c_v-8
	.byte	W01
	.byte		N02   , En2
	.byte	W03
	.byte		        Cn2
	.byte	W03
	.byte		VOL   , 60*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v+12
	.byte		N02   , An4
	.byte	W03
	.byte		        Fn4
	.byte	W02
	.byte		PAN   , c_v+9
	.byte	W01
	.byte		N02   , Dn4
	.byte	W03
	.byte		        Cn4
	.byte	W03
	.byte		VOL   , 54*mus_qz_643_mvl/mxv
	.byte		N02   , Dn4
	.byte	W03
	.byte		        Cn4
	.byte	W03
	.byte		        An3
	.byte	W03
	.byte		        Fn3
	.byte	W03
	.byte		VOL   , 51*mus_qz_643_mvl/mxv
	.byte		N02   , Dn4
	.byte	W03
	.byte		        Cn4
	.byte	W02
	.byte		PAN   , c_v+6
	.byte	W01
	.byte		N02   , An3
	.byte	W03
	.byte		        Fn3
	.byte	W03
	.byte		VOL   , 48*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v+3
	.byte		N02   , An3
	.byte	W03
	.byte		        Fn3
	.byte	W02
	.byte		VOL   , 47*mus_qz_643_mvl/mxv
	.byte	W01
	.byte		N02   , Dn3
	.byte	W03
	.byte		        Cn3
	.byte	W03
	.byte		VOL   , 45*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		N02   , An3
	.byte	W03
	.byte		        Fn3
	.byte	W02
	.byte		PAN   , c_v-2
	.byte		VOL   , 44*mus_qz_643_mvl/mxv
	.byte	W01
	.byte		N02   , Dn3
	.byte	W03
	.byte		        Cn3
	.byte	W03
	.byte		        Dn3
	.byte	W03
	.byte		        Cn3
	.byte	W02
	.byte		PAN   , c_v-5
	.byte		VOL   , 42*mus_qz_643_mvl/mxv
	.byte	W01
	.byte		N02   , An2
	.byte	W03
	.byte		        Fn2
	.byte	W03
	.byte		VOL   , 41*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v-8
	.byte		N02   , Dn3
	.byte	W03
	.byte		        Cn3
	.byte	W03
	.byte		        An2
	.byte	W03
	.byte		        Fn2
	.byte	W03
	.byte		VOL   , 39*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v-10
	.byte		N02   , An2
	.byte	W03
	.byte		        Fn2
	.byte	W02
	.byte		PAN   , c_v-8
	.byte	W01
	.byte		N02   , Dn2
	.byte	W03
	.byte		        Cn2
	.byte	W03
	.byte		VOL   , 60*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v+12
	.byte		N02   , An4
	.byte	W03
	.byte		        Gn4
	.byte	W02
	.byte		PAN   , c_v+9
	.byte	W01
	.byte		N02   , Fn4
	.byte	W03
	.byte		        Dn4
	.byte	W03
	.byte		VOL   , 54*mus_qz_643_mvl/mxv
	.byte		N02   , Fn4
	.byte	W03
	.byte		        Dn4
	.byte	W03
	.byte		        An3
	.byte	W03
	.byte		        Gn3
	.byte	W03
	.byte		VOL   , 51*mus_qz_643_mvl/mxv
	.byte		N02   , Fn4
	.byte	W03
	.byte		        Dn4
	.byte	W02
	.byte		PAN   , c_v+6
	.byte	W01
	.byte		N02   , An3
	.byte	W03
	.byte		        Gn3
	.byte	W03
	.byte		VOL   , 48*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v+3
	.byte		N02   , An3
	.byte	W03
	.byte		        Gn3
	.byte	W02
	.byte		VOL   , 47*mus_qz_643_mvl/mxv
	.byte	W01
	.byte		N02   , Fn3
	.byte	W03
	.byte		        Dn3
	.byte	W03
	.byte		VOL   , 45*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		N02   , An3
	.byte	W03
	.byte		        Gn3
	.byte	W02
	.byte		PAN   , c_v-2
	.byte		VOL   , 44*mus_qz_643_mvl/mxv
	.byte	W01
	.byte		N02   , Fn3
	.byte	W03
	.byte		        Dn3
	.byte	W03
	.byte		        Fn3
	.byte	W03
	.byte		        Dn3
	.byte	W02
	.byte		PAN   , c_v-5
	.byte		VOL   , 42*mus_qz_643_mvl/mxv
	.byte	W01
	.byte		N02   , An2
	.byte	W03
	.byte		        Gn2
	.byte	W03
	.byte		VOL   , 41*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v-8
	.byte		N02   , Fn3
	.byte	W03
	.byte		        Dn3
	.byte	W03
	.byte		        An2
	.byte	W03
	.byte		        Gn2
	.byte	W03
	.byte		VOL   , 39*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v-10
	.byte		N02   , An2
	.byte	W03
	.byte		        Gn2
	.byte	W02
	.byte		PAN   , c_v-8
	.byte	W01
	.byte		N02   , Fn2
	.byte	W03
	.byte		        Dn2
	.byte	W03
	.byte		VOL   , 60*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v+12
	.byte		N02   , As4
	.byte	W03
	.byte		        Gn4
	.byte	W02
	.byte		PAN   , c_v+9
	.byte	W01
	.byte		N02   , Fn4
	.byte	W03
	.byte		        Dn4
	.byte	W03
	.byte		VOL   , 54*mus_qz_643_mvl/mxv
	.byte		N02   , Fn4
	.byte	W03
	.byte		        Dn4
	.byte	W03
	.byte		        As3
	.byte	W03
	.byte		        Gn3
	.byte	W03
	.byte		VOL   , 51*mus_qz_643_mvl/mxv
	.byte		N02   , Fn4
	.byte	W03
	.byte		        Dn4
	.byte	W02
	.byte		PAN   , c_v+6
	.byte	W01
	.byte		N02   , As3
	.byte	W03
	.byte		        Gn3
	.byte	W03
	.byte		VOL   , 48*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v+3
	.byte		N02   , As3
	.byte	W03
	.byte		        Gn3
	.byte	W02
	.byte		VOL   , 47*mus_qz_643_mvl/mxv
	.byte	W01
	.byte		N02   , Fn3
	.byte	W03
	.byte		        Dn3
	.byte	W03
	.byte		VOL   , 45*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		N02   , As3
	.byte	W03
	.byte		        Gn3
	.byte	W02
	.byte		PAN   , c_v-2
	.byte		VOL   , 44*mus_qz_643_mvl/mxv
	.byte	W01
	.byte		N02   , Fn3
	.byte	W03
	.byte		        Dn3
	.byte	W03
	.byte		        Fn3
	.byte	W03
	.byte		        Dn3
	.byte	W02
	.byte		PAN   , c_v-5
	.byte		VOL   , 42*mus_qz_643_mvl/mxv
	.byte	W01
	.byte		N02   , As2
	.byte	W03
	.byte		        Gn2
	.byte	W03
	.byte		VOL   , 41*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v-8
	.byte		N02   , Fn3
	.byte	W03
	.byte		        Dn3
	.byte	W03
	.byte		        As2
	.byte	W03
	.byte		        Gn2
	.byte	W03
	.byte		VOL   , 39*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v-10
	.byte		N02   , As2
	.byte	W03
	.byte		        Gn2
	.byte	W02
	.byte		PAN   , c_v-8
	.byte	W01
	.byte		N02   , Fn2
	.byte	W03
	.byte		        Dn2
	.byte	W03
	.byte		VOL   , 60*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v+12
	.byte		N02   , Gs4
	.byte	W03
	.byte		        Gn4
	.byte	W02
	.byte		PAN   , c_v+9
	.byte	W01
	.byte		N02   , Fn4
	.byte	W03
	.byte		        Cn4
	.byte	W03
	.byte		VOL   , 54*mus_qz_643_mvl/mxv
	.byte		N02   , Fn4
	.byte	W03
	.byte		        Cn4
	.byte	W03
	.byte		        Gs3
	.byte	W03
	.byte		        Gn3
	.byte	W03
	.byte		VOL   , 51*mus_qz_643_mvl/mxv
	.byte		N02   , Fn4
	.byte	W03
	.byte		        Cn4
	.byte	W02
	.byte		PAN   , c_v+6
	.byte	W01
	.byte		N02   , Gs3
	.byte	W03
	.byte		        Gn3
	.byte	W03
	.byte		VOL   , 48*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v+3
	.byte		N02   , Gs3
	.byte	W03
	.byte		        Gn3
	.byte	W02
	.byte		VOL   , 47*mus_qz_643_mvl/mxv
	.byte	W01
	.byte		N02   , Fn3
	.byte	W03
	.byte		        Cn3
	.byte	W03
	.byte		VOL   , 45*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		N02   , Gs3
	.byte	W03
	.byte		        Gn3
	.byte	W02
	.byte		PAN   , c_v-2
	.byte		VOL   , 44*mus_qz_643_mvl/mxv
	.byte	W01
	.byte		N02   , Fn3
	.byte	W03
	.byte		        Cn3
	.byte	W03
	.byte		        Fn3
	.byte	W03
	.byte		        Cn3
	.byte	W02
	.byte		PAN   , c_v-5
	.byte		VOL   , 42*mus_qz_643_mvl/mxv
	.byte	W01
	.byte		N02   , Gs2
	.byte	W03
	.byte		        Gn2
	.byte	W03
	.byte		VOL   , 41*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v-8
	.byte		N02   , Fn3
	.byte	W03
	.byte		        Cn3
	.byte	W03
	.byte		        Gs2
	.byte	W03
	.byte		        Gn2
	.byte	W03
	.byte		VOL   , 39*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v-10
	.byte		N02   , Gs2
	.byte	W03
	.byte		        Gn2
	.byte	W02
	.byte		PAN   , c_v-8
	.byte	W01
	.byte		N02   , Fn2
	.byte	W03
	.byte		        Cn2
	.byte	W03
	.byte		VOL   , 60*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v+21
	.byte		N02   , Gn2
	.byte	W03
	.byte		        As2
	.byte	W03
	.byte		        Cn3
	.byte	W03
	.byte		        As2
	.byte	W03
	.byte		VOL   , 54*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v+18
	.byte		N02   , Cn3
	.byte	W03
	.byte		        Fn3
	.byte	W02
	.byte		PAN   , c_v+15
	.byte	W01
	.byte		N02   , Cn3
	.byte	W03
	.byte		        Fn3
	.byte	W03
	.byte		VOL   , 51*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v+12
	.byte		N02   , Cn3
	.byte	W03
	.byte		        Fn3
	.byte	W03
	.byte		        Gn3
	.byte	W03
	.byte		        Fn3
	.byte	W03
	.byte		VOL   , 48*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v+9
	.byte		N02   , Gn3
	.byte	W03
	.byte		        Cn4
	.byte	W02
	.byte		VOL   , 47*mus_qz_643_mvl/mxv
	.byte	W01
	.byte		N02   , Gn3
	.byte	W03
	.byte		        Cn4
	.byte	W03
	.byte		VOL   , 45*mus_qz_643_mvl/mxv
	.byte		N02   , Gn3
	.byte	W03
	.byte		        As3
	.byte	W02
	.byte		VOL   , 44*mus_qz_643_mvl/mxv
	.byte	W01
	.byte		N02   , Cn4
	.byte	W03
	.byte		        As3
	.byte	W03
	.byte		PAN   , c_v+6
	.byte		N02   , Cn4
	.byte	W03
	.byte		        Fn4
	.byte	W02
	.byte		VOL   , 42*mus_qz_643_mvl/mxv
	.byte	W01
	.byte		N02   , Cn4
	.byte	W03
	.byte		        Fn4
	.byte	W03
	.byte		VOL   , 41*mus_qz_643_mvl/mxv
	.byte		N02   , Cn4
	.byte	W03
	.byte		        Fn4
	.byte	W02
	.byte		PAN   , c_v+3
	.byte	W01
	.byte		N02   , Gn4
	.byte	W03
	.byte		        Fn4
	.byte	W03
	.byte		VOL   , 39*mus_qz_643_mvl/mxv
	.byte		N02   , Gn4
	.byte	W03
	.byte		        Cn5
	.byte	W02
	.byte		PAN   , c_v-2
	.byte	W01
	.byte		N02   , Gn4
	.byte	W03
	.byte		        Cn5
	.byte	W03
	.byte		        Gn3
	.byte	W03
	.byte		        As3
	.byte	W02
	.byte		PAN   , c_v-5
	.byte	W01
	.byte		N02   , Cn4
	.byte	W03
	.byte		        As3
	.byte	W03
	.byte		PAN   , c_v-8
	.byte		N02   , Cn4
	.byte	W03
	.byte		        Fn4
	.byte	W03
	.byte		        Cn4
	.byte	W03
	.byte		        Fn4
	.byte	W03
	.byte		VOL   , 33*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v-10
	.byte		N02   , Cn4
	.byte	W03
	.byte		        Fn4
	.byte	W02
	.byte		PAN   , c_v-13
	.byte	W01
	.byte		N02   , Gn4
	.byte	W03
	.byte		        Fn4
	.byte	W03
	.byte		VOL   , 31*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v-16
	.byte		N02   , Gn4
	.byte	W03
	.byte		        Cn5
	.byte	W02
	.byte		VOL   , 32*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v-18
	.byte	W01
	.byte		N02   , Gn4
	.byte	W03
	.byte		        Cn5
	.byte	W03
	.byte		VOL   , 36*mus_qz_643_mvl/mxv
	.byte		        36*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v-21
	.byte		N02   , Gn4
	.byte	W03
	.byte		        As4
	.byte	W02
	.byte		PAN   , c_v-26
	.byte		VOL   , 37*mus_qz_643_mvl/mxv
	.byte	W01
	.byte		N02   , Cn5
	.byte	W03
	.byte		        As4
	.byte	W03
	.byte		VOL   , 39*mus_qz_643_mvl/mxv
	.byte		        37*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v-29
	.byte		N02   , Cn5
	.byte	W03
	.byte		        Fn5
	.byte	W02
	.byte		VOL   , 41*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v-32
	.byte		VOL   , 41*mus_qz_643_mvl/mxv
	.byte	W01
	.byte		N02   , Cn5
	.byte	W03
	.byte		        Fn5
	.byte	W03
	.byte		VOL   , 43*mus_qz_643_mvl/mxv
	.byte		        42*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v-34
	.byte		N02   , Cn5
	.byte	W03
	.byte		        Fn5
	.byte	W02
	.byte		VOL   , 45*mus_qz_643_mvl/mxv
	.byte		        44*mus_qz_643_mvl/mxv
	.byte	W01
	.byte		N02   , Gn5
	.byte	W03
	.byte		        Fn5
	.byte	W03
	.byte		VOL   , 48*mus_qz_643_mvl/mxv
	.byte		N02   , Gn5
	.byte	W03
	.byte		        Cn6
	.byte	W02
	.byte		PAN   , c_v-32
	.byte	W01
	.byte		N02   , Gn5
	.byte	W03
	.byte		        Cn6
	.byte	W03
	.byte		PAN   , c_v-19
	.byte	W72
	.byte		VOICE , 23
	.byte	W12
	.byte		VOL   , 1*mus_qz_643_mvl/mxv
	.byte	W05
	.byte		        1*mus_qz_643_mvl/mxv
	.byte	W01
	.byte		        2*mus_qz_643_mvl/mxv
	.byte	W06
	.byte		        3*mus_qz_643_mvl/mxv
	.byte		N44   , An2 , v100 , gtp3
	.byte		N44   , Fn3 , v100 , gtp3
	.byte	W12
	.byte		VOL   , 4*mus_qz_643_mvl/mxv
	.byte	W05
	.byte		        4*mus_qz_643_mvl/mxv
	.byte	W07
	.byte		        5*mus_qz_643_mvl/mxv
	.byte	W17
	.byte		        6*mus_qz_643_mvl/mxv
	.byte	W07
	.byte		N44   , En2 , v100 , gtp3
	.byte		N44   , Cn3 , v100 , gtp3
	.byte	W12
	.byte		VOL   , 8*mus_qz_643_mvl/mxv
	.byte	W12
	.byte		        9*mus_qz_643_mvl/mxv
	.byte	W05
	.byte		        10*mus_qz_643_mvl/mxv
	.byte	W12
	.byte		        12*mus_qz_643_mvl/mxv
	.byte	W07
	.byte		        13*mus_qz_643_mvl/mxv
	.byte		N92   , Dn2 , v100 , gtp3
	.byte		N92   , Bn2 , v100 , gtp3
	.byte	W05
	.byte		VOL   , 14*mus_qz_643_mvl/mxv
	.byte	W07
	.byte		        16*mus_qz_643_mvl/mxv
	.byte	W05
	.byte		        17*mus_qz_643_mvl/mxv
	.byte	W07
	.byte		        18*mus_qz_643_mvl/mxv
	.byte	W05
	.byte		        19*mus_qz_643_mvl/mxv
	.byte	W07
	.byte		        20*mus_qz_643_mvl/mxv
	.byte	W05
	.byte		        21*mus_qz_643_mvl/mxv
	.byte	W19
	.byte		        25*mus_qz_643_mvl/mxv
	.byte	W12
	.byte		        25*mus_qz_643_mvl/mxv
	.byte	W05
	.byte		        27*mus_qz_643_mvl/mxv
	.byte	W12
	.byte		        28*mus_qz_643_mvl/mxv
	.byte	W07
	.byte		        31*mus_qz_643_mvl/mxv
	.byte		N44   , Cs2 , v100 , gtp3
	.byte		N44   , As2 , v100 , gtp3
	.byte	W48
	.byte		        As2 , v100 , gtp3
	.byte		N44   , Fn3 , v100 , gtp3
	.byte	W48
	.byte		N32   , Gn2 , v100 , gtp3
	.byte	W36
	.byte		        Cn3 , v100 , gtp3
	.byte	W36
	.byte		        Gn3 , v100 , gtp3
	.byte	W24
	.byte	W12
	.byte		        An3 , v100 , gtp3
	.byte	W36
	.byte		N44   , Cn4 , v100 , gtp3
	.byte	W48
	.byte		N32   , Dn2 , v100 , gtp3
	.byte		N32   , An2 , v100 , gtp3
	.byte	W36
	.byte		N56   , Fn2 , v100 , gtp3
	.byte		N56   , Cn3 , v100 , gtp3
	.byte	W60
	.byte		N32   , Gn2 , v100 , gtp3
	.byte		N32   , Cn3 , v100 , gtp3
	.byte	W36
	.byte		N56   , Cn2 , v100 , gtp3
	.byte		N56   , Gn2 , v100 , gtp3
	.byte	W48
	.byte		VOICE , 27
	.byte	W12
	.byte		VOL   , 48*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v+7
	.byte		VOL   , 44*mus_qz_643_mvl/mxv
	.byte		N03   , An3
	.byte	W04
	.byte		        Cn4
	.byte	W04
	.byte		        Fn4
	.byte	W04
	.byte		        Gn4
	.byte	W04
	.byte		        An4
	.byte	W04
	.byte		        Cn5
	.byte	W04
	.byte		        An3
	.byte	W04
	.byte		        Gn3
	.byte	W04
	.byte		        Fn3
	.byte	W04
	.byte		        Cn3
	.byte	W04
	.byte		        As2
	.byte	W04
	.byte		        An2
	.byte	W04
	.byte		        Cn3
	.byte	W04
	.byte		        Fn3
	.byte	W04
	.byte		        Gn3
	.byte	W04
	.byte		        An3
	.byte	W04
	.byte		        Cn4
	.byte	W04
	.byte		        Fn4
	.byte	W04
	.byte		        An3
	.byte	W04
	.byte		        Cn4
	.byte	W04
	.byte		        Fn4
	.byte	W04
	.byte		        Gn4
	.byte	W04
	.byte		        An4
	.byte	W04
	.byte		        Cn5
	.byte	W04
	.byte	GOTO
	 ptrvalue	mus_qz_643_4_B1
	.byte	FINE

/********************** Track  5 **********************/

mus_qz_643_5:
	.byte		KEYSH , mus_qz_643_key+0
	.byte		VOICE , 8
	.byte		VOL   , 57*mus_qz_643_mvl/mxv
	.byte		        57*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte		PRIO  , 61
	.byte	W48
mus_qz_643_5_000:
	.byte		VOL   , 39*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v+12
	.byte	W24
	.byte		N68   , Fn4 , v100 , gtp3
	.byte	W72
	.byte	PEND
mus_qz_643_5_B1:
	.byte	W24
	.byte		N44   , Fn4 , v100 , gtp3
	.byte	W48
	.byte		N23   , En4
	.byte	W24
	.byte		N44   , An4 , v100 , gtp3
	.byte	W48
	.byte		        Cn4 , v100 , gtp3
	.byte	W48
	.byte		        Gs3 , v100 , gtp3
	.byte	W48
	.byte		        As3 , v100 , gtp3
	.byte	W48
	.byte		PAN   , c_v+14
	.byte		VOL   , 39*mus_qz_643_mvl/mxv
	.byte		        52*mus_qz_643_mvl/mxv
	.byte	W06
	.byte		VOICE , 32
	.byte	W18
	.byte		VOL   , 21*mus_qz_643_mvl/mxv
	.byte		N23   , Cn3
	.byte		N23   , Gn3
	.byte	W02
	.byte		VOL   , 26*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        29*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        33*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        36*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        39*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        40*mus_qz_643_mvl/mxv
	.byte	W10
	.byte		        35*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        30*mus_qz_643_mvl/mxv
	.byte		N23   , Fn3
	.byte		N23   , An3
	.byte	W02
	.byte		VOL   , 33*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        34*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        36*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        37*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        41*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        42*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        43*mus_qz_643_mvl/mxv
	.byte	W08
	.byte		        39*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        28*mus_qz_643_mvl/mxv
	.byte	N23
	.byte		N23   , Cn4
	.byte	W02
	.byte		VOL   , 31*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        32*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        35*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        39*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        41*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        44*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        42*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        41*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        39*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        36*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        31*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        31*mus_qz_643_mvl/mxv
	.byte		N44   , Cn4 , v100 , gtp3
	.byte		N44   , Fn4 , v100 , gtp3
	.byte	W02
	.byte		VOL   , 33*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        34*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        36*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        39*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        41*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        44*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        45*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        47*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        48*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        50*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        51*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        52*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        52*mus_qz_643_mvl/mxv
	.byte	W22
	.byte		N11   , An3
	.byte		N11   , Cn4
	.byte	W12
	.byte		        Gn3
	.byte		N11   , As3
	.byte	W12
	.byte		        Fn3
	.byte		N11   , An3
	.byte	W12
	.byte		        Gn3
	.byte		N11   , As3
	.byte	W12
	.byte		VOL   , 39*mus_qz_643_mvl/mxv
	.byte		N23   , An3
	.byte		N23   , Cn4
	.byte	W02
	.byte		VOL   , 37*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        41*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        44*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        47*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        52*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        52*mus_qz_643_mvl/mxv
	.byte	W08
	.byte		        44*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        30*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        52*mus_qz_643_mvl/mxv
	.byte		N44   , Cn3 , v100 , gtp3
	.byte		N44   , Fn3 , v100 , gtp3
	.byte	W20
	.byte		VOL   , 50*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        48*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        47*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        44*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        41*mus_qz_643_mvl/mxv
	.byte	W04
	.byte		        39*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        37*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        35*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        33*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        31*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        28*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        25*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        24*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        52*mus_qz_643_mvl/mxv
	.byte		N11   , Cn3
	.byte		N11   , En3
	.byte	W12
	.byte		        Cn3
	.byte		N11   , Fn3
	.byte	W12
	.byte		VOL   , 46*mus_qz_643_mvl/mxv
	.byte		N23   , Dn3
	.byte		N23   , Gn3
	.byte	W02
	.byte		VOL   , 50*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        52*mus_qz_643_mvl/mxv
	.byte	W20
	.byte		        45*mus_qz_643_mvl/mxv
	.byte		N23   , Gn2
	.byte		N23   , Dn3
	.byte	W09
	.byte		VOL   , 45*mus_qz_643_mvl/mxv
	.byte	W03
	.byte		        44*mus_qz_643_mvl/mxv
	.byte	W03
	.byte		        42*mus_qz_643_mvl/mxv
	.byte	W03
	.byte		        41*mus_qz_643_mvl/mxv
	.byte	W03
	.byte		        40*mus_qz_643_mvl/mxv
	.byte	W03
	.byte		        52*mus_qz_643_mvl/mxv
	.byte		N11   , Cn3
	.byte		N11   , Fn3
	.byte	W12
	.byte		        As2
	.byte		N11   , En3
	.byte	W12
	.byte		        Cn3
	.byte		N11   , Fn3
	.byte	W12
	.byte		        Dn3
	.byte		N11   , Gn3
	.byte	W12
	.byte		VOL   , 30*mus_qz_643_mvl/mxv
	.byte		N44   , Gn3 , v100 , gtp3
	.byte		N44   , Cn4 , v100 , gtp3
	.byte	W02
	.byte		VOL   , 34*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        37*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        41*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        44*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        47*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        50*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        52*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        52*mus_qz_643_mvl/mxv
	.byte	W28
	.byte		        52*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        48*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        41*mus_qz_643_mvl/mxv
	.byte	N23
	.byte		N23   , Fn4
	.byte	W02
	.byte		VOL   , 44*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        47*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        50*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        52*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        52*mus_qz_643_mvl/mxv
	.byte	W14
	.byte		N23   , An3
	.byte		N23   , Cn4
	.byte	W12
	.byte		VOL   , 52*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        50*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        48*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        44*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        39*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        36*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        44*mus_qz_643_mvl/mxv
	.byte		N11   , Gn3
	.byte		N11   , As3
	.byte	W02
	.byte		VOL   , 48*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        51*mus_qz_643_mvl/mxv
	.byte	W08
	.byte		N11   , Fn3
	.byte		N11   , An3
	.byte	W12
	.byte		        Dn3
	.byte		N11   , Gn3
	.byte	W12
	.byte		        Cn3
	.byte		N11   , Fn3
	.byte	W12
	.byte		VOL   , 41*mus_qz_643_mvl/mxv
	.byte		N44   , Cs3 , v100 , gtp3
	.byte		N44   , Gn3 , v100 , gtp3
	.byte	W02
	.byte		VOL   , 47*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        48*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        52*mus_qz_643_mvl/mxv
	.byte	W32
	.byte	W02
	.byte		        48*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        46*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        44*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        40*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        39*mus_qz_643_mvl/mxv
	.byte		TIE   , Cn3
	.byte		TIE   , Fn3
	.byte	W02
	.byte		VOL   , 31*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        35*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        37*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        40*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        42*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        44*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        48*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        50*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        52*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        52*mus_qz_643_mvl/mxv
	.byte	W64
	.byte		        50*mus_qz_643_mvl/mxv
	.byte	W05
	.byte		        41*mus_qz_643_mvl/mxv
	.byte	W07
	.byte		        35*mus_qz_643_mvl/mxv
	.byte	W05
	.byte		        23*mus_qz_643_mvl/mxv
	.byte	W07
	.byte		        14*mus_qz_643_mvl/mxv
	.byte	W05
	.byte		        4*mus_qz_643_mvl/mxv
	.byte	W06
	.byte		EOT   , Cn3
	.byte		        Fn3
	.byte	W01
	.byte		VOL   , 2*mus_qz_643_mvl/mxv
	.byte	W05
	.byte		        0*mus_qz_643_mvl/mxv
	.byte	W60
	.byte	W01
	.byte		        59*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v-28
	.byte	W06
	.byte		N32   , Fn2 , v100 , gtp3
	.byte		N92   , Cn3 , v100 , gtp3
	.byte	W36
	.byte		N32   , Cn2 , v100 , gtp3
	.byte	W36
	.byte		N23   , An2
	.byte	W24
	.byte	N23
	.byte		N92   , En3 , v100 , gtp3
	.byte	W24
	.byte		N44   , Cn3 , v100 , gtp3
	.byte	W48
	.byte		N23   , As2
	.byte	W24
	.byte		N92   , An2 , v100 , gtp3
	.byte		N44   , Fn3 , v100 , gtp3
	.byte	W48
	.byte		N23   , Dn3
	.byte	W24
	.byte		        En3
	.byte	W24
	.byte		N44   , Bn2 , v100 , gtp3
	.byte		N44   , Fn3 , v100 , gtp3
	.byte	W48
	.byte		        Gn2 , v100 , gtp3
	.byte		N23   , Dn3
	.byte	W24
	.byte		        En3
	.byte	W24
	.byte		N92   , As2 , v100 , gtp3
	.byte		N44   , Dn3 , v100 , gtp3
	.byte	W48
	.byte		N23   , An3
	.byte	W24
	.byte		        Gn3
	.byte	W24
	.byte		        Gs2
	.byte		N44   , Fn3 , v100 , gtp3
	.byte	W24
	.byte		N68   , Cn3 , v100 , gtp3
	.byte	W24
	.byte		N44   , Gs3 , v100 , gtp3
	.byte	W48
	.byte		N68   , Dn3 , v100 , gtp3
	.byte		N92   , Cn4 , v100 , gtp3
	.byte	W72
	.byte		N23   , Fn3
	.byte	W24
	.byte		N92   , Gn3 , v100 , gtp3
	.byte		N44   , Cs4 , v100 , gtp3
	.byte	W48
	.byte		        Cn4 , v100 , gtp3
	.byte	W48
	.byte		VOICE , 32
	.byte		PAN   , c_v+15
	.byte		VOL   , 35*mus_qz_643_mvl/mxv
	.byte		N92   , En3 , v100 , gtp3
	.byte		N92   , Cn4 , v100 , gtp3
	.byte	W96
	.byte		N44   , Fn3 , v100 , gtp3
	.byte		N44   , Dn4 , v100 , gtp3
	.byte	W48
	.byte		        En3 , v100 , gtp3
	.byte		N44   , Cn4 , v100 , gtp3
	.byte	W48
	.byte		TIE   , Fn3
	.byte		N92   , Dn4 , v100 , gtp3
	.byte	W96
	.byte		        Cs4 , v100 , gtp3
	.byte	W92
	.byte	W03
	.byte		EOT   , Fn3
	.byte	W01
	.byte		VOL   , 23*mus_qz_643_mvl/mxv
	.byte		N32   , Cn3 , v100 , gtp3
	.byte		N32   , Gn3 , v100 , gtp3
	.byte	W36
	.byte		        Fn3 , v100 , gtp3
	.byte		N32   , Cn4 , v100 , gtp3
	.byte	W36
	.byte		        Gn3 , v100 , gtp3
	.byte		N32   , Gn4 , v100 , gtp3
	.byte	W24
	.byte	W12
	.byte		        Cn4 , v100 , gtp3
	.byte		N32   , An4 , v100 , gtp3
	.byte	W36
	.byte		N44   , Fn4 , v100 , gtp3
	.byte		N44   , Cn5 , v100 , gtp3
	.byte	W42
	.byte		PAN   , c_v-10
	.byte	W06
	.byte		VOICE , 23
	.byte		VOL   , 27*mus_qz_643_mvl/mxv
	.byte		N44   , An3 , v100 , gtp3
	.byte		N44   , Dn4 , v100 , gtp3
	.byte	W24
	.byte		VOL   , 31*mus_qz_643_mvl/mxv
	.byte	W06
	.byte		        33*mus_qz_643_mvl/mxv
	.byte	W18
	.byte		        27*mus_qz_643_mvl/mxv
	.byte		N44   , Fn3 , v100 , gtp3
	.byte		N44   , An3 , v100 , gtp3
	.byte	W24
	.byte		VOL   , 31*mus_qz_643_mvl/mxv
	.byte	W06
	.byte		        33*mus_qz_643_mvl/mxv
	.byte	W18
	.byte		        27*mus_qz_643_mvl/mxv
	.byte		N44   , Gn3 , v100 , gtp3
	.byte		N44   , Dn4 , v100 , gtp3
	.byte	W24
	.byte		VOL   , 31*mus_qz_643_mvl/mxv
	.byte	W06
	.byte		        33*mus_qz_643_mvl/mxv
	.byte	W18
	.byte		        27*mus_qz_643_mvl/mxv
	.byte		N44   , Dn3 , v100 , gtp3
	.byte		N44   , As3 , v100 , gtp3
	.byte	W24
	.byte		VOL   , 31*mus_qz_643_mvl/mxv
	.byte	W06
	.byte		        33*mus_qz_643_mvl/mxv
	.byte	W12
	.byte		VOICE , 8
	.byte	W06
	.byte	PATT
	 ptrvalue	mus_qz_643_5_000
	.byte	GOTO
	 ptrvalue	mus_qz_643_5_B1
	.byte	FINE

/********************** Track  6 **********************/

mus_qz_643_6:
	.byte		KEYSH , mus_qz_643_key+0
	.byte		VOL   , 57*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte		VOL   , 57*mus_qz_643_mvl/mxv
	.byte		PRIO  , 61
	.byte	W48
	.byte	W96
mus_qz_643_6_B1:
	.byte		BENDR , 12
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		VOL   , 57*mus_qz_643_mvl/mxv
	.byte		PRIO  , 52
	.byte	W03
	.byte		PAN   , c_v-28
	.byte		VOL   , 21*mus_qz_643_mvl/mxv
	.byte	W03
	.byte		VOICE , 32
	.byte		VOL   , 37*mus_qz_643_mvl/mxv
	.byte		BEND  , c_v+0
	.byte	W19
	.byte		VOL   , 14*mus_qz_643_mvl/mxv
	.byte		N23   , Cn3 , v100
	.byte		N23   , Gn3
	.byte	W02
	.byte		VOL   , 18*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        20*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        23*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        25*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        27*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        28*mus_qz_643_mvl/mxv
	.byte	W11
	.byte		        25*mus_qz_643_mvl/mxv
	.byte	W01
	.byte		        20*mus_qz_643_mvl/mxv
	.byte		N23   , Fn3
	.byte		N23   , An3
	.byte	W02
	.byte		VOL   , 23*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        24*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        25*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        27*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        28*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        29*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        30*mus_qz_643_mvl/mxv
	.byte	W09
	.byte		        27*mus_qz_643_mvl/mxv
	.byte	W01
	.byte		        20*mus_qz_643_mvl/mxv
	.byte	N23
	.byte		N23   , Cn4
	.byte	W02
	.byte		VOL   , 21*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        22*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        24*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        27*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        28*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        31*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        29*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        28*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        27*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        25*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        21*mus_qz_643_mvl/mxv
	.byte	W01
	.byte	W01
	.byte		        21*mus_qz_643_mvl/mxv
	.byte		N44   , Cn4 , v100 , gtp3
	.byte		N44   , Fn4 , v100 , gtp3
	.byte	W02
	.byte		VOL   , 23*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        24*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        25*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        27*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        29*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        31*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        32*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        33*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        33*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        35*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        36*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        36*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        37*mus_qz_643_mvl/mxv
	.byte	W22
	.byte		N11   , An3
	.byte		N11   , Cn4
	.byte	W12
	.byte		        Gn3
	.byte		N11   , As3
	.byte	W12
	.byte		        Fn3
	.byte		N11   , An3
	.byte	W12
	.byte		        Gn3
	.byte		N11   , As3
	.byte	W11
	.byte	W01
	.byte		VOL   , 27*mus_qz_643_mvl/mxv
	.byte		N23   , An3
	.byte		N23   , Cn4
	.byte	W02
	.byte		VOL   , 26*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        28*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        31*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        33*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        36*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        37*mus_qz_643_mvl/mxv
	.byte	W08
	.byte		        31*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        20*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        37*mus_qz_643_mvl/mxv
	.byte		N44   , Cn3 , v100 , gtp3
	.byte		N44   , Fn3 , v100 , gtp3
	.byte	W20
	.byte		VOL   , 35*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        33*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        33*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        31*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        28*mus_qz_643_mvl/mxv
	.byte	W04
	.byte		        27*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        26*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        24*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        23*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        21*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        19*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        17*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        17*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        37*mus_qz_643_mvl/mxv
	.byte		N11   , Cn3
	.byte		N11   , En3
	.byte	W12
	.byte		        Cn3
	.byte		N11   , Fn3
	.byte	W11
	.byte	W01
	.byte		VOL   , 32*mus_qz_643_mvl/mxv
	.byte		N23   , Dn3
	.byte		N23   , Gn3
	.byte	W02
	.byte		VOL   , 35*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        37*mus_qz_643_mvl/mxv
	.byte	W20
	.byte		        32*mus_qz_643_mvl/mxv
	.byte		N23   , Gn2
	.byte		N23   , Dn3
	.byte	W09
	.byte		VOL   , 32*mus_qz_643_mvl/mxv
	.byte	W03
	.byte		        31*mus_qz_643_mvl/mxv
	.byte	W03
	.byte		        29*mus_qz_643_mvl/mxv
	.byte	W03
	.byte		        29*mus_qz_643_mvl/mxv
	.byte	W03
	.byte		        28*mus_qz_643_mvl/mxv
	.byte	W03
	.byte		        37*mus_qz_643_mvl/mxv
	.byte		N11   , Cn3
	.byte		N11   , Fn3
	.byte	W12
	.byte		        As2
	.byte		N11   , En3
	.byte	W12
	.byte		        Cn3
	.byte		N11   , Fn3
	.byte	W12
	.byte		        Dn3
	.byte		N11   , Gn3
	.byte	W11
	.byte	W01
	.byte		VOL   , 20*mus_qz_643_mvl/mxv
	.byte		N44   , Gn3 , v100 , gtp3
	.byte		N44   , Cn4 , v100 , gtp3
	.byte	W02
	.byte		VOL   , 24*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        27*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        29*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        31*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        33*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        35*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        36*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        37*mus_qz_643_mvl/mxv
	.byte	W28
	.byte		        36*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        33*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        28*mus_qz_643_mvl/mxv
	.byte	N23
	.byte		N23   , Fn4
	.byte	W02
	.byte		VOL   , 31*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        33*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        35*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        36*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        37*mus_qz_643_mvl/mxv
	.byte	W14
	.byte		N23   , An3
	.byte		N23   , Cn4
	.byte	W12
	.byte		VOL   , 36*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        35*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        33*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        31*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        27*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        25*mus_qz_643_mvl/mxv
	.byte	W01
	.byte	W01
	.byte		        31*mus_qz_643_mvl/mxv
	.byte		N11   , Gn3
	.byte		N11   , As3
	.byte	W02
	.byte		VOL   , 33*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        36*mus_qz_643_mvl/mxv
	.byte	W08
	.byte		N11   , Fn3
	.byte		N11   , An3
	.byte	W12
	.byte		        Dn3
	.byte		N11   , Gn3
	.byte	W12
	.byte		        Cn3
	.byte		N11   , Fn3
	.byte	W12
	.byte		VOL   , 28*mus_qz_643_mvl/mxv
	.byte		N44   , Cs3 , v100 , gtp3
	.byte		N44   , Gn3 , v100 , gtp3
	.byte	W02
	.byte		VOL   , 33*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        34*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        37*mus_qz_643_mvl/mxv
	.byte	W32
	.byte	W02
	.byte		        33*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        32*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        31*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        28*mus_qz_643_mvl/mxv
	.byte	W01
	.byte	W01
	.byte		        27*mus_qz_643_mvl/mxv
	.byte		TIE   , Cn3
	.byte		TIE   , Fn3
	.byte	W02
	.byte		VOL   , 21*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        24*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        25*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        28*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        29*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        31*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        33*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        35*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        36*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        37*mus_qz_643_mvl/mxv
	.byte	W68
	.byte	W01
	.byte		        17*mus_qz_643_mvl/mxv
	.byte	W06
	.byte		        12*mus_qz_643_mvl/mxv
	.byte	W05
	.byte		        7*mus_qz_643_mvl/mxv
	.byte	W07
	.byte		        5*mus_qz_643_mvl/mxv
	.byte	W05
	.byte		        2*mus_qz_643_mvl/mxv
	.byte	W07
	.byte		EOT   , Cn3
	.byte		        Fn3
	.byte		VOL   , 0*mus_qz_643_mvl/mxv
	.byte	W72
	.byte		PRIO  , 61
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W90
	.byte		VOICE , 8
	.byte		VOL   , 42*mus_qz_643_mvl/mxv
	.byte	W06
	.byte	W24
	.byte		N44   , Dn4 , v100 , gtp3
	.byte	W48
	.byte		N23   , En4
	.byte	W24
	.byte		        Fn4
	.byte	W24
	.byte		N44   , Cn4 , v100 , gtp3
	.byte	W48
	.byte		N23   , Gs3
	.byte	W24
	.byte		N44   , As3 , v100 , gtp3
	.byte	W48
	.byte		        Fn4 , v100 , gtp3
	.byte	W48
	.byte		        Cs4 , v100 , gtp3
	.byte	W48
	.byte		        Cn4 , v100 , gtp3
	.byte	W48
	.byte		VOICE , 3
	.byte		VOL   , 54*mus_qz_643_mvl/mxv
	.byte	W96
	.byte		        36*mus_qz_643_mvl/mxv
	.byte	W21
	.byte		BEND  , c_v+57
	.byte	W03
	.byte		PAN   , c_v-7
	.byte		BEND  , c_v+54
	.byte		N80   , Fn5 , v100 , gtp3
	.byte	W02
	.byte		BEND  , c_v+51
	.byte	W03
	.byte		        c_v+46
	.byte	W03
	.byte		        c_v+40
	.byte	W04
	.byte		PAN   , c_v-12
	.byte		BEND  , c_v+36
	.byte	W02
	.byte		        c_v+32
	.byte	W03
	.byte		        c_v+28
	.byte	W03
	.byte		        c_v+24
	.byte	W04
	.byte		        c_v+20
	.byte	W02
	.byte		        c_v+16
	.byte	W03
	.byte		PAN   , c_v-17
	.byte		BEND  , c_v+12
	.byte	W03
	.byte		        c_v+6
	.byte	W04
	.byte		PAN   , c_v-22
	.byte		BEND  , c_v+4
	.byte	W02
	.byte		        c_v+1
	.byte	W03
	.byte		        c_v-5
	.byte	W03
	.byte		        c_v-9
	.byte	W04
	.byte		PAN   , c_v-27
	.byte	W02
	.byte		BEND  , c_v-12
	.byte	W03
	.byte		        c_v-20
	.byte	W07
	.byte		PAN   , c_v-32
	.byte		BEND  , c_v-28
	.byte	W02
	.byte		        c_v-32
	.byte	W03
	.byte		        c_v-37
	.byte	W03
	.byte		        c_v-42
	.byte	W04
	.byte		        c_v-47
	.byte	W05
	.byte		        c_v-50
	.byte	W03
	.byte		        c_v-54
	.byte	W04
	.byte		        c_v-57
	.byte	W02
	.byte		        c_v-63
	.byte	W07
	.byte		        c_v+57
	.byte	W03
	.byte		PAN   , c_v-12
	.byte		BEND  , c_v+54
	.byte		N80   , Cn5 , v100 , gtp3
	.byte	W02
	.byte		BEND  , c_v+51
	.byte	W03
	.byte		PAN   , c_v-17
	.byte		BEND  , c_v+46
	.byte	W03
	.byte		        c_v+40
	.byte	W04
	.byte		PAN   , c_v-22
	.byte		BEND  , c_v+36
	.byte	W02
	.byte		        c_v+32
	.byte	W03
	.byte		        c_v+28
	.byte	W03
	.byte		        c_v+24
	.byte	W04
	.byte		        c_v+20
	.byte	W02
	.byte		        c_v+16
	.byte	W03
	.byte		        c_v+12
	.byte	W03
	.byte		        c_v+6
	.byte	W04
	.byte		PAN   , c_v-27
	.byte		BEND  , c_v+4
	.byte	W02
	.byte		        c_v+1
	.byte	W03
	.byte		PAN   , c_v-34
	.byte		BEND  , c_v-5
	.byte	W03
	.byte		        c_v-9
	.byte	W06
	.byte		        c_v-12
	.byte	W03
	.byte		PAN   , c_v-39
	.byte		BEND  , c_v-20
	.byte	W07
	.byte		PAN   , c_v-41
	.byte		BEND  , c_v-28
	.byte	W02
	.byte		        c_v-32
	.byte	W03
	.byte		PAN   , c_v-44
	.byte		BEND  , c_v-37
	.byte	W03
	.byte		        c_v-42
	.byte	W04
	.byte		PAN   , c_v-46
	.byte		BEND  , c_v-47
	.byte	W05
	.byte		        c_v-50
	.byte	W03
	.byte		        c_v-54
	.byte	W04
	.byte		        c_v-57
	.byte	W02
	.byte		        c_v-63
	.byte	W56
	.byte	W02
	.byte		BENDR , 6
	.byte		BEND  , c_v+0
	.byte	W06
	.byte		        c_v+1
	.byte	W12
	.byte		VOICE , 35
	.byte		PAN   , c_v+22
	.byte		VOL   , 73*mus_qz_643_mvl/mxv
	.byte	W06
	.byte		N32   , Cn4 , v092 , gtp3
	.byte	W36
	.byte		        Fn4 , v092 , gtp3
	.byte	W36
	.byte		        As4 , v092 , gtp3
	.byte	W24
	.byte	W12
	.byte		        Cn5 , v092 , gtp3
	.byte	W36
	.byte		N44   , Fn5 , v092 , gtp3
	.byte	W48
	.byte		N23   , Dn5
	.byte	W24
	.byte		        Cn5
	.byte	W24
	.byte		        Fn4
	.byte	W24
	.byte		        Gn4
	.byte	W24
	.byte		        Cs5
	.byte	W24
	.byte		        As4
	.byte	W24
	.byte		        Dn4
	.byte	W24
	.byte		        En4
	.byte	W24
	.byte		VOL   , 73*mus_qz_643_mvl/mxv
	.byte		N92   , Fn4 , v092 , gtp3
	.byte	W36
	.byte		VOL   , 71*mus_qz_643_mvl/mxv
	.byte	W05
	.byte		        65*mus_qz_643_mvl/mxv
	.byte	W07
	.byte		        54*mus_qz_643_mvl/mxv
	.byte	W05
	.byte		        45*mus_qz_643_mvl/mxv
	.byte	W07
	.byte		        37*mus_qz_643_mvl/mxv
	.byte	W05
	.byte		        28*mus_qz_643_mvl/mxv
	.byte	W07
	.byte		        20*mus_qz_643_mvl/mxv
	.byte	W05
	.byte		        6*mus_qz_643_mvl/mxv
	.byte	W07
	.byte		        1*mus_qz_643_mvl/mxv
	.byte	W05
	.byte		        0*mus_qz_643_mvl/mxv
	.byte	W07
	.byte	GOTO
	 ptrvalue	mus_qz_643_6_B1
	.byte	FINE

/********************** Track  7 **********************/

mus_qz_643_7:
	.byte		KEYSH , mus_qz_643_key+0
	.byte		VOICE , 28
	.byte		VOL   , 57*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte		VOL   , 57*mus_qz_643_mvl/mxv
	.byte		PRIO  , 61
	.byte		        12*mus_qz_643_mvl/mxv
	.byte		N02   , Cn2 , v100
	.byte	W03
	.byte	N02
	.byte	W01
	.byte		VOL   , 14*mus_qz_643_mvl/mxv
	.byte	W02
	.byte	N02
	.byte	W02
	.byte		VOL   , 17*mus_qz_643_mvl/mxv
	.byte	W01
	.byte	N02
	.byte	W03
	.byte		VOL   , 19*mus_qz_643_mvl/mxv
	.byte	N02
	.byte	W03
	.byte	N02
	.byte	W01
	.byte		VOL   , 21*mus_qz_643_mvl/mxv
	.byte	W02
	.byte	N02
	.byte	W02
	.byte		VOL   , 24*mus_qz_643_mvl/mxv
	.byte	W01
	.byte	N02
	.byte	W03
	.byte		VOL   , 29*mus_qz_643_mvl/mxv
	.byte	N02
	.byte	W03
	.byte	N02
	.byte	W01
	.byte		VOL   , 31*mus_qz_643_mvl/mxv
	.byte	W02
	.byte	N02
	.byte	W02
	.byte		VOL   , 36*mus_qz_643_mvl/mxv
	.byte	W01
	.byte	N02
	.byte	W03
	.byte		VOL   , 41*mus_qz_643_mvl/mxv
	.byte	N02
	.byte	W03
	.byte	N02
	.byte	W01
	.byte		VOL   , 45*mus_qz_643_mvl/mxv
	.byte	W02
	.byte	N02
	.byte	W02
	.byte		VOL   , 57*mus_qz_643_mvl/mxv
	.byte	W01
	.byte	N02
	.byte	W03
	.byte	W96
mus_qz_643_7_B1:
	.byte	W96
	.byte	W96
	.byte	W48
	.byte		N02   , Gn1 , v100
	.byte	W03
	.byte	N02
	.byte	W03
	.byte	N02
	.byte	W03
	.byte	N02
	.byte	W03
	.byte	N02
	.byte	W03
	.byte	N02
	.byte	W03
	.byte	N02
	.byte	W03
	.byte	N02
	.byte	W03
	.byte	N02
	.byte	W03
	.byte	N02
	.byte	W03
	.byte	N02
	.byte	W03
	.byte	N02
	.byte	W03
	.byte	N02
	.byte	W03
	.byte	N02
	.byte	W03
	.byte	N02
	.byte	W03
	.byte	N02
	.byte	W03
	.byte		N23   , Fn1
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
	.byte	W48
	.byte		VOL   , 12*mus_qz_643_mvl/mxv
	.byte		N02   , Cn2
	.byte	W03
	.byte	N02
	.byte	W01
	.byte		VOL   , 14*mus_qz_643_mvl/mxv
	.byte	W02
	.byte	N02
	.byte	W02
	.byte		VOL   , 17*mus_qz_643_mvl/mxv
	.byte	W01
	.byte	N02
	.byte	W03
	.byte		VOL   , 19*mus_qz_643_mvl/mxv
	.byte	N02
	.byte	W03
	.byte	N02
	.byte	W01
	.byte		VOL   , 21*mus_qz_643_mvl/mxv
	.byte	W02
	.byte	N02
	.byte	W02
	.byte		VOL   , 24*mus_qz_643_mvl/mxv
	.byte	W01
	.byte	N02
	.byte	W03
	.byte		VOL   , 29*mus_qz_643_mvl/mxv
	.byte	N02
	.byte	W03
	.byte	N02
	.byte	W01
	.byte		VOL   , 31*mus_qz_643_mvl/mxv
	.byte	W02
	.byte	N02
	.byte	W02
	.byte		VOL   , 36*mus_qz_643_mvl/mxv
	.byte	W01
	.byte	N02
	.byte	W03
	.byte		VOL   , 41*mus_qz_643_mvl/mxv
	.byte	N02
	.byte	W03
	.byte	N02
	.byte	W01
	.byte		VOL   , 45*mus_qz_643_mvl/mxv
	.byte	W02
	.byte	N02
	.byte	W02
	.byte		VOL   , 57*mus_qz_643_mvl/mxv
	.byte	W01
	.byte	N02
	.byte	W03
	.byte		N23   , Cn2 , v048
	.byte	W96
	.byte	W96
	.byte	W84
	.byte		VOICE , 8
	.byte	W06
	.byte		PAN   , c_v-17
	.byte	W06
	.byte		VOL   , 41*mus_qz_643_mvl/mxv
	.byte		N23   , Gn4 , v100
	.byte	W24
	.byte		        Fn4
	.byte	W24
	.byte		        Cs4
	.byte	W24
	.byte		        Gn3
	.byte	W24
	.byte		N44   , An3 , v100 , gtp3
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W56
	.byte		VOL   , 57*mus_qz_643_mvl/mxv
	.byte	W04
	.byte		VOICE , 28
	.byte	W36
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_643_7_B1
	.byte	FINE

/********************** Track  8 **********************/

mus_qz_643_8:
	.byte		KEYSH , mus_qz_643_key+0
	.byte		VOL   , 57*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte		VOL   , 57*mus_qz_643_mvl/mxv
	.byte		PAN   , c_v+31
	.byte		VOL   , 67*mus_qz_643_mvl/mxv
	.byte	W36
	.byte		VOICE , 25
	.byte	W12
	.byte		N68   , Fn0 , v127 , gtp3
	.byte	W72
	.byte		N23   , An0
	.byte	W24
mus_qz_643_8_B1:
	.byte		N92   , As0 , v127 , gtp3
	.byte	W96
	.byte		N68   , Dn1 , v127 , gtp3
	.byte	W72
	.byte		N23   , Cn1
	.byte	W24
	.byte		N44   , Gs0 , v127 , gtp3
	.byte	W48
	.byte		N23   , As0
	.byte	W24
	.byte		        Cn1
	.byte	W24
	.byte		VOL   , 59*mus_qz_643_mvl/mxv
	.byte		N92   , Fn0 , v127 , gtp3
	.byte	W96
	.byte		N44   , As0 , v127 , gtp3
	.byte	W48
	.byte		        Cn1 , v127 , gtp3
	.byte	W48
	.byte		        Dn1 , v127 , gtp3
	.byte	W48
	.byte		        An0 , v127 , gtp3
	.byte	W48
	.byte		N23   , As0
	.byte	W24
	.byte		        Gn0
	.byte	W24
	.byte		        Cn1
	.byte	W24
	.byte		        As0
	.byte	W24
	.byte		N44   , An0 , v127 , gtp3
	.byte	W48
	.byte		        Dn1 , v127 , gtp3
	.byte	W48
	.byte		        Gn0 , v127 , gtp3
	.byte	W48
	.byte		        As0 , v127 , gtp3
	.byte	W48
	.byte		TIE   , Fn0
	.byte	W78
	.byte		VOL   , 54*mus_qz_643_mvl/mxv
	.byte	W06
	.byte		        50*mus_qz_643_mvl/mxv
	.byte	W05
	.byte		        41*mus_qz_643_mvl/mxv
	.byte	W07
	.byte		        35*mus_qz_643_mvl/mxv
	.byte	W05
	.byte		        23*mus_qz_643_mvl/mxv
	.byte	W07
	.byte		        14*mus_qz_643_mvl/mxv
	.byte	W05
	.byte		        4*mus_qz_643_mvl/mxv
	.byte	W06
	.byte	EOT
	.byte	W01
	.byte		VOL   , 2*mus_qz_643_mvl/mxv
	.byte	W05
	.byte		        0*mus_qz_643_mvl/mxv
	.byte	W36
	.byte	W01
	.byte		        63*mus_qz_643_mvl/mxv
	.byte	W06
	.byte		        52*mus_qz_643_mvl/mxv
	.byte		N05   , Cn1
	.byte	W02
	.byte		VOL   , 54*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        56*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        60*mus_qz_643_mvl/mxv
	.byte	N17
	.byte	W04
	.byte		VOL   , 63*mus_qz_643_mvl/mxv
	.byte	W14
	.byte		        48*mus_qz_643_mvl/mxv
	.byte		N92   , Fn0 , v127 , gtp3
	.byte	W02
	.byte		VOL   , 54*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        58*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        60*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        63*mus_qz_643_mvl/mxv
	.byte	W72
	.byte		        63*mus_qz_643_mvl/mxv
	.byte	W04
	.byte		        62*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        60*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        57*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        54*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        51*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        50*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        63*mus_qz_643_mvl/mxv
	.byte		N92   , Fn0 , v127 , gtp3
	.byte	W80
	.byte	W02
	.byte		VOL   , 63*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        62*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        60*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        59*mus_qz_643_mvl/mxv
	.byte	W04
	.byte		        57*mus_qz_643_mvl/mxv
	.byte	W04
	.byte		        46*mus_qz_643_mvl/mxv
	.byte		N92   , Dn1 , v116 , gtp3
	.byte	W02
	.byte		VOL   , 46*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        50*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        52*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        56*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        59*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        63*mus_qz_643_mvl/mxv
	.byte	W56
	.byte	W02
	.byte		        63*mus_qz_643_mvl/mxv
	.byte	W14
	.byte		        60*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        59*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        56*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        54*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        50*mus_qz_643_mvl/mxv
	.byte	W04
	.byte		        44*mus_qz_643_mvl/mxv
	.byte		N92   , Bn0 , v127 , gtp3
	.byte	W02
	.byte		VOL   , 50*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        51*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        54*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        57*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        60*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        63*mus_qz_643_mvl/mxv
	.byte	W84
	.byte		        46*mus_qz_643_mvl/mxv
	.byte		N92   , As0 , v127 , gtp3
	.byte	W02
	.byte		VOL   , 51*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        54*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        56*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        60*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        63*mus_qz_643_mvl/mxv
	.byte	W80
	.byte		        63*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        60*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        57*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        51*mus_qz_643_mvl/mxv
	.byte		N92   , Gs0 , v127 , gtp3
	.byte	W02
	.byte		VOL   , 57*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        58*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        63*mus_qz_643_mvl/mxv
	.byte	W84
	.byte	W02
	.byte		        62*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        57*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        57*mus_qz_643_mvl/mxv
	.byte		N92   , As0 , v127 , gtp3
	.byte	W02
	.byte		VOL   , 62*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        63*mus_qz_643_mvl/mxv
	.byte	W88
	.byte		        62*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        58*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        57*mus_qz_643_mvl/mxv
	.byte		N92   , Cn1 , v127 , gtp3
	.byte	W02
	.byte		VOL   , 60*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        62*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        63*mus_qz_643_mvl/mxv
	.byte	W90
	.byte		        48*mus_qz_643_mvl/mxv
	.byte		        42*mus_qz_643_mvl/mxv
	.byte		N92   , Fn0 , v127 , gtp3
	.byte	W02
	.byte		VOL   , 44*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        48*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        52*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        54*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        55*mus_qz_643_mvl/mxv
	.byte	W78
	.byte		        55*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        50*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        41*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        39*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        21*mus_qz_643_mvl/mxv
	.byte		N44   , Dn1 , v127 , gtp3
	.byte	W02
	.byte		VOL   , 28*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        33*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        37*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        50*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        51*mus_qz_643_mvl/mxv
	.byte	W24
	.byte	W02
	.byte		        54*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        50*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        44*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        41*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        37*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        34*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        43*mus_qz_643_mvl/mxv
	.byte		N44   , Cn1 , v127 , gtp3
	.byte	W02
	.byte		VOL   , 51*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        54*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        55*mus_qz_643_mvl/mxv
	.byte	W36
	.byte	W02
	.byte		        51*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        41*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		N92   , Bn0 , v127 , gtp3
	.byte	W02
	.byte		VOL   , 46*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        48*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        51*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        54*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        55*mus_qz_643_mvl/mxv
	.byte	W68
	.byte		        55*mus_qz_643_mvl/mxv
	.byte	W06
	.byte		        54*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        51*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        50*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        44*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        41*mus_qz_643_mvl/mxv
	.byte	W04
	.byte		        27*mus_qz_643_mvl/mxv
	.byte		N92   , As0 , v127 , gtp3
	.byte	W02
	.byte		VOL   , 33*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        39*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        44*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        54*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        55*mus_qz_643_mvl/mxv
	.byte	W84
	.byte	W02
	.byte		        42*mus_qz_643_mvl/mxv
	.byte		N92   , An0 , v127 , gtp3
	.byte	W02
	.byte		VOL   , 51*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        54*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        55*mus_qz_643_mvl/mxv
	.byte	W90
	.byte		        29*mus_qz_643_mvl/mxv
	.byte		N92   , Dn1 , v127 , gtp3
	.byte	W02
	.byte		VOL   , 41*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        50*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        55*mus_qz_643_mvl/mxv
	.byte	W84
	.byte		        52*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        50*mus_qz_643_mvl/mxv
	.byte	W04
	.byte		        41*mus_qz_643_mvl/mxv
	.byte		N92   , Gn0 , v127 , gtp3
	.byte	W02
	.byte		VOL   , 48*mus_qz_643_mvl/mxv
	.byte	W04
	.byte		        54*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        55*mus_qz_643_mvl/mxv
	.byte	W80
	.byte	W02
	.byte		        55*mus_qz_643_mvl/mxv
	.byte	W02
	.byte		        51*mus_qz_643_mvl/mxv
	.byte	W04
	.byte		        44*mus_qz_643_mvl/mxv
	.byte		N92   , Cn1 , v127 , gtp3
	.byte	W02
	.byte		VOL   , 50*mus_qz_643_mvl/mxv
	.byte	W04
	.byte		        55*mus_qz_643_mvl/mxv
	.byte	W90
	.byte		        67*mus_qz_643_mvl/mxv
	.byte		N68   , Fn0 , v127 , gtp3
	.byte	W72
	.byte		N23   , An0
	.byte	W24
	.byte	GOTO
	 ptrvalue	mus_qz_643_8_B1
	.byte	FINE

/********************** Track  9 **********************/

mus_qz_643_9:
	.byte		KEYSH , mus_qz_643_key+0
	.byte		VOICE , 39
	.byte		VOL   , 57*mus_qz_643_mvl/mxv
	.byte		PRIO  , 61
	.byte		        57*mus_qz_643_mvl/mxv
	.byte		N44   , Gn0 , v120 , gtp3
	.byte	W48
	.byte		N92   , An2 , v108 , gtp3
	.byte	W96
mus_qz_643_9_B1:
	.byte	W96
	.byte		N92   , An2 , v108 , gtp3
	.byte	W96
	.byte	W96
	.byte		N11   , Cn1 , v100
	.byte		N44   , An2 , v108 , gtp2
	.byte	W12
	.byte		N11   , Cn1 , v100
	.byte	W12
	.byte		N05   , Ds4
	.byte	W06
	.byte		        Cn4
	.byte	W12
	.byte		        An3
	.byte	W18
	.byte		N11   , Cn1
	.byte	W12
	.byte		N05   , Ds4
	.byte	W06
	.byte		        An3
	.byte	W12
	.byte		        Ds3
	.byte	W06
	.byte		N11   , Cn1
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		N05   , Cn4
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        An3
	.byte	W12
	.byte		N11   , Dn4
	.byte	W12
	.byte		        Cn1
	.byte	W12
	.byte		N05   , Dn4
	.byte	W06
	.byte		        An3
	.byte	W12
	.byte	N05
	.byte	W06
mus_qz_643_9_000:
	.byte		N11   , Cn1 , v100
	.byte	W12
	.byte		        An3
	.byte	W12
	.byte		N05   , Cn4
	.byte	W06
	.byte	N05
	.byte	W18
	.byte		N11   , Dn4
	.byte	W12
	.byte		        Cn1
	.byte	W12
	.byte		N05   , Dn4
	.byte	W06
	.byte	N11
	.byte	W18
	.byte	PEND
	.byte		        Cn1
	.byte	W12
	.byte		        As3
	.byte	W12
	.byte		N05   , An3
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Cn4
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		        Cn1
	.byte	W12
	.byte		N05   , Dn4
	.byte	W06
	.byte	N05
	.byte	W12
	.byte		        An3
	.byte	W06
	.byte		N11   , Cn1
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		N05   , Ds4
	.byte	W06
	.byte		        Cn4
	.byte	W12
	.byte		        An3
	.byte	W18
	.byte		N11   , Cn1
	.byte	W12
	.byte		N05   , Ds4
	.byte	W06
	.byte		        An3
	.byte	W12
	.byte		        Ds3
	.byte	W06
	.byte		N11   , Cn1
	.byte	W12
	.byte		        As3
	.byte	W12
	.byte		N05   , Cn4
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        An3
	.byte	W12
	.byte		N11   , Dn4
	.byte	W12
	.byte		        Cn1
	.byte	W12
	.byte		N05   , Dn4
	.byte	W06
	.byte		        An3
	.byte	W12
	.byte	N05
	.byte	W06
	.byte	PATT
	 ptrvalue	mus_qz_643_9_000
	.byte		N11   , Cn1 , v100
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		N05   , An3
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Cn4
	.byte	W12
	.byte		N44   , Gn0 , v100 , gtp3
	.byte		N11   , Cn4
	.byte	W12
	.byte		        Cn1
	.byte	W12
	.byte		N05   , Bn3
	.byte	W06
	.byte	N05
	.byte	W18
	.byte		N92   , An2 , v108 , gtp3
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		        An2 , v108 , gtp3
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		        An2 , v108 , gtp3
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		        An2 , v108 , gtp3
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		        An2 , v108 , gtp3
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_643_9_B1
	.byte	FINE

/******************************************************/
	.p2align	2

mus_qz_643:
	.byte	9	/* NumTrks */
	.byte	0	/* NumBlks */
	.byte	mus_qz_643_pri	/* Priority */
	.byte	mus_qz_643_rev	/* Reverb. */
space64 4
	ptrvalue	mus_qz_643_grp

	ptrvalue	mus_qz_643_1
	ptrvalue	mus_qz_643_2
	ptrvalue	mus_qz_643_3
	ptrvalue	mus_qz_643_4
	ptrvalue	mus_qz_643_5
	ptrvalue	mus_qz_643_6
	ptrvalue	mus_qz_643_7
	ptrvalue	mus_qz_643_8
	ptrvalue	mus_qz_643_9

	.end
