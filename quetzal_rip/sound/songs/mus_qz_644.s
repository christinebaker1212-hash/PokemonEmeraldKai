	.include "asm/macros/bit_width.inc"

	.include "MPlayDef.s"

/* Moemon Quetzal song 644, disassembled from the ROM by tools/quetzal_rip/songs.py.
   "org" comments give the ROM address of the bytes that follow. */

	.equ	mus_qz_644_grp, voicegroup_qz_songs
	.equ	mus_qz_644_pri, 0
	.equ	mus_qz_644_rev, reverb_set+0
	.equ	mus_qz_644_mvl, 127
	.equ	mus_qz_644_key, 0
	.equ	mus_qz_644_tbs, 1
	.equ	mus_qz_644_exg, 1
	.equ	mus_qz_644_cmp, 1

	.section .rodata
	.global	mus_qz_644
	.p2align	2
/* org 0x096922E8 */

/********************** Track  1 **********************/

mus_qz_644_1:
	.byte		KEYSH , mus_qz_644_key+0
	.byte		TEMPO , 120*mus_qz_644_tbs/2
	.byte		VOICE , 34
	.byte		PAN   , c_v+24
	.byte		VOL   , 62*mus_qz_644_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte		VOL   , 60*mus_qz_644_mvl/mxv
	.byte		N23   , An2 , v100
	.byte		N23   , Cs3
	.byte		N23   , En3
	.byte	W24
	.byte		N05   , An2
	.byte		N05   , Dn3
	.byte		N05   , Fs3
	.byte	W06
	.byte		        An2
	.byte		N05   , Cs3
	.byte		N05   , En3
	.byte	W12
	.byte		N11   , An2
	.byte		N11   , Cs3
	.byte		N11   , En3
	.byte	W12
	.byte		N05   , Gs2 , v080
	.byte		N05   , Bn2
	.byte		N05   , Dn3
	.byte	W06
	.byte		        En2 , v052
	.byte		N05   , An2
	.byte		N05   , Cs3
	.byte	W24
	.byte		        En2 , v100
	.byte		N05   , Gs2
	.byte		N05   , Bn2
	.byte	W06
	.byte		        En2
	.byte		N05   , An2
	.byte		N05   , Cs3
	.byte	W06
mus_qz_644_1_000:
	.byte		N05   , En2 , v080
	.byte		N05   , An2
	.byte		N05   , Cs3
	.byte	W06
	.byte		        Fs2 , v100
	.byte		N05   , An2
	.byte		N05   , Dn3
	.byte	W06
	.byte		        Fs2 , v072
	.byte		N05   , An2
	.byte		N05   , Dn3
	.byte	W12
	.byte		        Fs2 , v100
	.byte		N05   , An2
	.byte		N05   , Dn3
	.byte	W12
	.byte		        En2 , v080
	.byte		N05   , An2
	.byte		N05   , Cs3
	.byte	W06
	.byte		        Fs2 , v088
	.byte		N05   , An2
	.byte		N05   , Dn3
	.byte	W06
	.byte		        En2 , v096
	.byte		N05   , An2
	.byte		N05   , Cs3
	.byte	W06
	.byte		        Fs2 , v092
	.byte		N05   , An2
	.byte		N05   , Dn3
	.byte	W06
	.byte		        Gs2 , v100
	.byte		N05   , Bn2
	.byte		N05   , En3
	.byte	W12
	.byte		        Bn2
	.byte		N05   , Dn3
	.byte		N05   , Fs3
	.byte	W06
	.byte		        Gs2 , v080
	.byte		N05   , Bn2
	.byte		N05   , En3
	.byte	W12
	.byte		N28   , An2 , v100 , gtp1
	.byte		N28   , Cs3 , v100 , gtp1
	.byte		N28   , En3 , v100 , gtp1
	.byte	W06
	.byte	PEND
	.byte	W30
	.byte		VOL   , 79*mus_qz_644_mvl/mxv
	.byte	W06
	.byte		N05   , Bn2
	.byte		N05   , Dn3
	.byte		N05   , Fs3
	.byte	W06
	.byte		N11   , Cs3
	.byte		N11   , En3
	.byte		N11   , An3
	.byte	W18
	.byte		        Fs3
	.byte		N11   , An3
	.byte		N11   , Dn4
	.byte	W12
	.byte		N05   , En3
	.byte		N05   , An3
	.byte		N05   , Cs4
	.byte	W06
	.byte		        Fs3
	.byte		N05   , An3
	.byte		N05   , Dn4
	.byte	W06
	.byte		N11   , En3
	.byte		N11   , An3
	.byte		N11   , Cs4
	.byte	W12
	.byte		        Dn3
	.byte		N11   , Fs3
	.byte		N11   , An3
	.byte	W12
	.byte		N05   , Dn3
	.byte		N05   , Fs3
	.byte		N05   , Bn3
	.byte	W06
	.byte		        Dn3 , v072
	.byte		N05   , Fs3
	.byte		N05   , An3
	.byte	W18
	.byte		N11   , Gs2 , v100
	.byte		N11   , Bn2
	.byte		N11   , En3
	.byte	W12
	.byte		N05   , Bn2
	.byte		N05   , Dn3
	.byte		N05   , Fs3
	.byte	W06
	.byte		        Gs2 , v080
	.byte		N05   , Bn2
	.byte		N05   , En3
	.byte	W12
	.byte		        Gs2 , v100
	.byte		N05   , Bn2
	.byte		N05   , En3
	.byte	W06
	.byte		        Bn2
	.byte		N05   , Dn3
	.byte		N05   , Fs3
	.byte	W06
	.byte		        Cs3
	.byte		N05   , En3
	.byte		N05   , An3
	.byte	W06
	.byte		        Bn3
	.byte		N05   , Dn4
	.byte		N05   , Fs4
	.byte	W06
	.byte		N28   , An3 , v100 , gtp1
	.byte		N28   , Cs4 , v100 , gtp1
	.byte		N28   , En4 , v100 , gtp1
	.byte	W06
mus_qz_644_1_001:
	.byte	W36
	.byte		N05   , Gs3 , v100
	.byte		N05   , Bn3
	.byte		N05   , Dn4
	.byte	W06
	.byte		N11   , En3
	.byte		N11   , An3
	.byte		N11   , Cs4
	.byte	W18
	.byte		        En3
	.byte		N11   , An3
	.byte		N11   , Dn4
	.byte	W12
	.byte		N05   , An3
	.byte		N05   , Cs4
	.byte		N05   , En4
	.byte	W06
	.byte		        An3
	.byte		N05   , Dn4
	.byte		N05   , Fs4
	.byte	W12
	.byte		N17   , An3
	.byte		N17   , Cs4
	.byte		N17   , En4
	.byte	W06
	.byte	PEND
mus_qz_644_1_002:
	.byte	W12
	.byte		N11   , An3 , v100
	.byte		N11   , Dn4
	.byte		N11   , Fs4
	.byte	W12
	.byte		N05   , Dn3
	.byte		N05   , Fs3
	.byte		N05   , An3
	.byte	W06
	.byte		        Dn3
	.byte		N05   , Fs3
	.byte		N05   , Bn3
	.byte	W06
	.byte		N11   , En3
	.byte		N11   , An3
	.byte		N11   , Cs4
	.byte	W12
	.byte		        Fs3
	.byte		N11   , An3
	.byte		N11   , Dn4
	.byte	W12
	.byte		        Fs3
	.byte		N11   , An3
	.byte		N11   , Cs4
	.byte	W12
	.byte		N05   , Dn3
	.byte		N05   , Fs3
	.byte		N05   , An3
	.byte	W12
	.byte		        En3
	.byte		N05   , Gs3
	.byte		N05   , Bn3
	.byte	W06
	.byte		        Cs3
	.byte		N05   , En3
	.byte		N05   , An3
	.byte	W06
	.byte	PEND
mus_qz_644_1_B1:
	.byte	W12
	.byte		N05   , Cs3 , v100
	.byte		N05   , En3
	.byte		N05   , An3
	.byte	W12
	.byte		        Cs3
	.byte		N05   , En3
	.byte		N05   , An3
	.byte	W12
	.byte		        An2
	.byte		N05   , Cs3
	.byte		N05   , En3
	.byte	W12
	.byte		        An3
	.byte		N05   , Cs4
	.byte		N05   , En4
	.byte	W06
	.byte		        An3
	.byte		N05   , Cs4
	.byte		N05   , En4
	.byte	W12
	.byte		N17   , An3
	.byte		N17   , Cs4
	.byte		N17   , En4
	.byte	W30
	.byte	W12
	.byte		N05   , Dn3
	.byte		N05   , Fs3
	.byte		N05   , An3
	.byte	W12
	.byte		        Dn3
	.byte		N05   , Fs3
	.byte		N05   , An3
	.byte	W12
	.byte		        An2
	.byte		N05   , Cs3
	.byte		N05   , En3
	.byte	W12
	.byte		        Gs3
	.byte		N05   , Bn3
	.byte		N05   , En4
	.byte	W06
	.byte		        Gs3
	.byte		N05   , Bn3
	.byte		N05   , En4
	.byte	W12
	.byte		N11   , Gs3
	.byte		N11   , Bn3
	.byte		N11   , En4
	.byte	W12
	.byte		N05   , Bn3
	.byte		N05   , Dn4
	.byte		N05   , Fs4
	.byte	W06
	.byte		N11   , Gs3
	.byte		N11   , Bn3
	.byte		N11   , En4
	.byte	W12
	.byte		N17   , An3
	.byte		N17   , Cs4
	.byte		N17   , Fs4
	.byte	W18
	.byte		N05   , An3
	.byte		N05   , Cs4
	.byte		N05   , En4
	.byte	W18
	.byte		N23   , Cs3
	.byte		N23   , En3
	.byte		N23   , An3
	.byte	W24
	.byte		N11   , En3
	.byte		N11   , An3
	.byte		N11   , Dn4
	.byte	W12
	.byte		        En3
	.byte		N11   , An3
	.byte		N11   , Cs4
	.byte	W12
	.byte		        En3
	.byte		N11   , Gs3
	.byte		N11   , Bn3
	.byte	W12
	.byte		N17   , Dn3
	.byte		N17   , Fs3
	.byte		N17   , An3
	.byte	W18
	.byte		N05   , An2
	.byte		N05   , Cs3
	.byte		N05   , En3
	.byte	W18
	.byte		N23   , Fs3
	.byte		N23   , An3
	.byte		N23   , Dn4
	.byte	W24
	.byte		N11   , En3
	.byte		N11   , An3
	.byte		N11   , Cs4
	.byte	W12
	.byte		        En3
	.byte		N11   , Gs3
	.byte		N11   , Bn3
	.byte	W12
	.byte		        En3
	.byte		N11   , Gs3
	.byte		N11   , Cs4
	.byte	W12
	.byte		VOICE , 7
	.byte		VOL   , 79*mus_qz_644_mvl/mxv
	.byte		PAN   , c_v+25
	.byte		N05   , An2
	.byte	W18
	.byte		        Gs2
	.byte	W18
	.byte		        An2
	.byte	W24
	.byte		        En2
	.byte	W12
	.byte		        An2
	.byte	W12
	.byte		        Bn2
	.byte	W12
	.byte		        Cs3
	.byte	W18
	.byte		        Dn3
	.byte	W18
	.byte		        Fs2
	.byte	W24
	.byte		        An2
	.byte	W12
	.byte		        Gs2
	.byte	W06
	.byte		        An2
	.byte	W06
	.byte		        Bn2
	.byte	W12
	.byte		        Cs3
	.byte	W12
	.byte		        Dn3
	.byte	W12
	.byte		        En3
	.byte	W12
	.byte		        Fs3
	.byte	W06
	.byte		N23   , En3
	.byte	W54
	.byte		N05   , Cs3
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Dn3
	.byte	W12
	.byte		        En3
	.byte	W12
	.byte		        Fs3
	.byte	W06
	.byte		N23   , En3
	.byte	W54
	.byte	N05
	.byte	W12
	.byte		        An2
	.byte	W12
	.byte		        An3
	.byte	W18
	.byte		        Gs3
	.byte	W12
	.byte		        An3
	.byte	W06
	.byte		        Bn3
	.byte	W12
	.byte		        An3
	.byte	W24
	.byte	N05
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        En3
	.byte	W12
	.byte		        An3
	.byte	W18
	.byte		        Gs3
	.byte	W12
	.byte		        An3
	.byte	W06
	.byte		        Bn3
	.byte	W12
	.byte		        An3
	.byte	W06
	.byte		        Gs3
	.byte	W06
	.byte		        Fs3
	.byte	W06
	.byte		        Gs3
	.byte	W06
	.byte		        An3
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		N05   , Dn4
	.byte	W06
	.byte		        Cs4
	.byte	W06
	.byte		        Bn3
	.byte	W06
	.byte		N11   , An3
	.byte	W18
	.byte	N11
	.byte	W12
	.byte		N05   , Cs3
	.byte	W06
	.byte		        Dn3
	.byte	W06
	.byte		        En3
	.byte	W06
	.byte		N17   , Fs3
	.byte	W06
	.byte	W12
	.byte		N11   , An3
	.byte	W12
	.byte		        Dn4
	.byte	W12
	.byte		N05   , Fs3
	.byte	W06
	.byte		N11   , An3
	.byte	W12
	.byte	N05
	.byte	W06
	.byte		N11   , Gs3
	.byte	W12
	.byte		N05   , Fs3
	.byte	W12
	.byte		        Gs3
	.byte	W12
	.byte		N44   , An3 , v100 , gtp3
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W78
	.byte		VOICE , 34
	.byte	W18
	.byte		VOL   , 74*mus_qz_644_mvl/mxv
	.byte		PAN   , c_v+24
	.byte		N23   , An2
	.byte		N23   , Cs3
	.byte		N23   , En3
	.byte	W24
	.byte		N05   , An2
	.byte		N05   , Dn3
	.byte		N05   , Fs3
	.byte	W06
	.byte		        An2
	.byte		N05   , Cs3
	.byte		N05   , En3
	.byte	W12
	.byte		N11   , An2
	.byte		N11   , Cs3
	.byte		N11   , En3
	.byte	W12
	.byte		N05   , Gs2 , v080
	.byte		N05   , Bn2
	.byte		N05   , Dn3
	.byte	W06
	.byte		        En2 , v052
	.byte		N05   , An2
	.byte		N05   , Cs3
	.byte	W24
	.byte		        En2 , v100
	.byte		N05   , Gs2
	.byte		N05   , Bn2
	.byte	W06
	.byte		        En2
	.byte		N05   , An2
	.byte		N05   , Cs3
	.byte	W06
	.byte	PATT
	 ptrvalue	mus_qz_644_1_000
	.byte	W36
	.byte		N05   , Bn2 , v100
	.byte		N05   , Dn3
	.byte		N05   , Fs3
	.byte	W06
	.byte		N11   , Cs3
	.byte		N11   , En3
	.byte		N11   , An3
	.byte	W18
	.byte		        Fs3
	.byte		N11   , An3
	.byte		N11   , Dn4
	.byte	W12
	.byte		N05   , En3
	.byte		N05   , An3
	.byte		N05   , Cs4
	.byte	W06
	.byte		        Fs3
	.byte		N05   , An3
	.byte		N05   , Dn4
	.byte	W06
	.byte		N11   , En3
	.byte		N11   , An3
	.byte		N11   , Cs4
	.byte	W12
	.byte		        Dn3
	.byte		N11   , Fs3
	.byte		N11   , An3
	.byte	W12
	.byte		N05   , Dn3
	.byte		N05   , Fs3
	.byte		N05   , Bn3
	.byte	W06
	.byte		        Dn3
	.byte		N05   , Fs3
	.byte		N05   , An3
	.byte	W18
	.byte		N11   , Gs2
	.byte		N11   , Bn2
	.byte		N11   , En3
	.byte	W12
	.byte		N05   , Bn2
	.byte		N05   , Dn3
	.byte		N05   , Fs3
	.byte	W06
	.byte		        Gs2
	.byte		N05   , Bn2
	.byte		N05   , En3
	.byte	W12
	.byte		        Gs2
	.byte		N05   , Bn2
	.byte		N05   , En3
	.byte	W06
	.byte		        Bn2
	.byte		N05   , Dn3
	.byte		N05   , Fs3
	.byte	W06
	.byte		        Cs3
	.byte		N05   , En3
	.byte		N05   , An3
	.byte	W06
	.byte		        Bn3
	.byte		N05   , Dn4
	.byte		N05   , Fs4
	.byte	W06
	.byte		N28   , An3 , v100 , gtp1
	.byte		N28   , Cs4 , v100 , gtp1
	.byte		N28   , En4 , v100 , gtp1
	.byte	W06
	.byte	PATT
	 ptrvalue	mus_qz_644_1_001
	.byte	PATT
	 ptrvalue	mus_qz_644_1_002
	.byte	GOTO
	 ptrvalue	mus_qz_644_1_B1
	.byte	FINE

/********************** Track  2 **********************/

mus_qz_644_2:
	.byte		KEYSH , mus_qz_644_key+0
	.byte		VOICE , 34
	.byte		VOL   , 62*mus_qz_644_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte		PRIO  , 57
	.byte		VOL   , 15*mus_qz_644_mvl/mxv
	.byte	W03
	.byte		N23   , An2 , v100
	.byte		N23   , Cs3
	.byte		N23   , En3
	.byte	W03
	.byte		PAN   , c_v-25
	.byte	W21
	.byte		N05   , An2
	.byte		N05   , Dn3
	.byte		N05   , Fs3
	.byte	W06
	.byte		        An2
	.byte		N05   , Cs3
	.byte		N05   , En3
	.byte	W12
	.byte		N11   , An2
	.byte		N11   , Cs3
	.byte		N11   , En3
	.byte	W12
	.byte		N05   , Gs2
	.byte		N05   , Bn2
	.byte		N05   , Dn3
	.byte	W06
	.byte		        En2
	.byte		N05   , An2
	.byte		N05   , Cs3
	.byte	W24
	.byte		        En2
	.byte		N05   , Gs2
	.byte		N05   , Bn2
	.byte	W06
	.byte		        En2
	.byte		N05   , An2
	.byte		N05   , Cs3
	.byte	W03
mus_qz_644_2_000:
	.byte	W03
	.byte		N05   , En2 , v100
	.byte		N05   , An2
	.byte		N05   , Cs3
	.byte	W06
	.byte		        Fs2
	.byte		N05   , An2
	.byte		N05   , Dn3
	.byte	W06
	.byte		        Fs2
	.byte		N05   , An2
	.byte		N05   , Dn3
	.byte	W12
	.byte		        Fs2
	.byte		N05   , An2
	.byte		N05   , Dn3
	.byte	W12
	.byte		        En2
	.byte		N05   , An2
	.byte		N05   , Cs3
	.byte	W06
	.byte		        Fs2
	.byte		N05   , An2
	.byte		N05   , Dn3
	.byte	W06
	.byte		        En2
	.byte		N05   , An2
	.byte		N05   , Cs3
	.byte	W06
	.byte		        Fs2
	.byte		N05   , An2
	.byte		N05   , Dn3
	.byte	W06
	.byte		        Gs2
	.byte		N05   , Bn2
	.byte		N05   , En3
	.byte	W12
	.byte		        Bn2
	.byte		N05   , Dn3
	.byte		N05   , Fs3
	.byte	W06
	.byte		        Gs2
	.byte		N05   , Bn2
	.byte		N05   , En3
	.byte	W12
	.byte		N28   , An2 , v100 , gtp1
	.byte		N28   , Cs3 , v100 , gtp1
	.byte		N28   , En3 , v100 , gtp1
	.byte	W03
	.byte	PEND
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
mus_qz_644_2_B1:
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W90
	.byte		VOL   , 41*mus_qz_644_mvl/mxv
	.byte	W06
	.byte	W06
	.byte		VOICE , 7
	.byte		PAN   , c_v-20
	.byte		N05   , An2 , v100
	.byte	W18
	.byte		        Gs2
	.byte	W18
	.byte		        An2
	.byte	W24
	.byte		        En2
	.byte	W12
	.byte		        An2
	.byte	W12
	.byte		        Bn2
	.byte	W06
	.byte	W06
	.byte		        Cs3
	.byte	W18
	.byte		        Dn3
	.byte	W18
	.byte		        Fs2
	.byte	W24
	.byte		        An2
	.byte	W12
	.byte		        Gs2
	.byte	W06
	.byte		        An2
	.byte	W06
	.byte		        Bn2
	.byte	W06
	.byte	W06
	.byte		        Cs3
	.byte	W12
	.byte		        Dn3
	.byte	W12
	.byte		        En3
	.byte	W12
	.byte		        Fs3
	.byte	W06
	.byte		N23   , En3
	.byte	W48
	.byte	W06
	.byte		N05   , Cs3
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Dn3
	.byte	W12
	.byte		        En3
	.byte	W12
	.byte		        Fs3
	.byte	W06
	.byte		N23   , En3
	.byte	W48
	.byte	W06
	.byte	N05
	.byte	W12
	.byte		        An2
	.byte	W12
	.byte		        An3
	.byte	W18
	.byte		        Gs3
	.byte	W12
	.byte		        An3
	.byte	W06
	.byte		        Bn3
	.byte	W12
	.byte		        An3
	.byte	W18
	.byte	W06
	.byte	N05
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        En3
	.byte	W12
	.byte		        An3
	.byte	W18
	.byte		        Gs3
	.byte	W12
	.byte		        An3
	.byte	W06
	.byte		        Bn3
	.byte	W12
	.byte		        An3
	.byte	W06
	.byte		        Gs3
	.byte	W06
	.byte		        Fs3
	.byte	W06
	.byte		        Gs3
	.byte	W06
	.byte		        An3
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		N05   , Dn4
	.byte	W06
	.byte		        Cs4
	.byte	W06
	.byte		        Bn3
	.byte	W06
	.byte		N11   , An3
	.byte	W18
	.byte	N11
	.byte	W12
	.byte		N05   , Cs3
	.byte	W06
	.byte		        Dn3
	.byte	W06
	.byte		        En3
	.byte	W06
	.byte		N17   , Fs3
	.byte	W18
	.byte		N11   , An3
	.byte	W12
	.byte		        Dn4
	.byte	W12
	.byte		N05   , Fs3
	.byte	W06
	.byte		N11   , An3
	.byte	W12
	.byte	N05
	.byte	W06
	.byte		N11   , Gs3
	.byte	W12
	.byte		N05   , Fs3
	.byte	W12
	.byte		        Gs3
	.byte	W06
	.byte	W06
	.byte		N44   , An3 , v100 , gtp3
	.byte	W90
	.byte	W96
	.byte	W96
	.byte	W84
	.byte		VOL   , 15*mus_qz_644_mvl/mxv
	.byte	W06
	.byte		PAN   , c_v-25
	.byte	W06
	.byte		VOICE , 34
	.byte	W03
	.byte		N23   , An2
	.byte		N23   , Cs3
	.byte		N23   , En3
	.byte	W24
	.byte		N05   , An2
	.byte		N05   , Dn3
	.byte		N05   , Fs3
	.byte	W06
	.byte		        An2
	.byte		N05   , Cs3
	.byte		N05   , En3
	.byte	W12
	.byte		N11   , An2
	.byte		N11   , Cs3
	.byte		N11   , En3
	.byte	W12
	.byte		N05   , Gs2
	.byte		N05   , Bn2
	.byte		N05   , Dn3
	.byte	W06
	.byte		        En2
	.byte		N05   , An2
	.byte		N05   , Cs3
	.byte	W24
	.byte		        En2
	.byte		N05   , Gs2
	.byte		N05   , Bn2
	.byte	W06
	.byte		        En2
	.byte		N05   , An2
	.byte		N05   , Cs3
	.byte	W03
	.byte	PATT
	 ptrvalue	mus_qz_644_2_000
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_644_2_B1
	.byte	FINE

/********************** Track  3 **********************/

mus_qz_644_3:
	.byte		KEYSH , mus_qz_644_key+0
	.byte		VOICE , 23
	.byte		VOL   , 62*mus_qz_644_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte		        c_v-17
	.byte	W96
	.byte	W96
mus_qz_644_3_000:
	.byte		VOL   , 36*mus_qz_644_mvl/mxv
	.byte		N44   , An4 , v100 , gtp3
	.byte	W48
	.byte		N23   , Cs5
	.byte	W24
	.byte		N44   , En4 , v100 , gtp3
	.byte	W24
	.byte	PEND
mus_qz_644_3_001:
	.byte	W24
	.byte		N23   , Fs4 , v100
	.byte	W24
	.byte		        An4
	.byte	W24
	.byte		        Gs4
	.byte	W24
	.byte	PEND
mus_qz_644_3_002:
	.byte		N44   , An4 , v100 , gtp3
	.byte	W48
	.byte		N23   , Bn4
	.byte	W24
	.byte		N44   , An4 , v100 , gtp3
	.byte	W24
	.byte	PEND
mus_qz_644_3_003:
	.byte	W24
	.byte		N23   , Dn5 , v100
	.byte	W24
	.byte		        Cs5
	.byte	W24
	.byte		        Bn4
	.byte	W24
	.byte	PEND
mus_qz_644_3_B1:
	.byte		N05   , An4 , v100
	.byte	W06
	.byte		        An4 , v064
	.byte	W90
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		VOL   , 30*mus_qz_644_mvl/mxv
	.byte	W36
	.byte		N23   , Fs3 , v100
	.byte		N23   , An3
	.byte		N23   , Dn4
	.byte	W24
	.byte		N05   , En3
	.byte		N05   , An3
	.byte		N05   , Cs4
	.byte	W36
	.byte	W36
	.byte		N23   , An3
	.byte		N23   , Dn4
	.byte		N23   , Fs4
	.byte	W24
	.byte		        Bn3
	.byte		N23   , En4
	.byte		N23   , Gs4
	.byte	W24
	.byte		N11   , Dn4
	.byte		N11   , Fs4
	.byte	W12
	.byte		N32   , Cs4 , v100 , gtp3
	.byte		N32   , En4 , v100 , gtp3
	.byte	W36
	.byte		        Cs3 , v100 , gtp3
	.byte		N32   , An3 , v100 , gtp3
	.byte	W36
	.byte		N23   , En3
	.byte		N23   , Cs4
	.byte	W24
	.byte		N44   , Fs3 , v100 , gtp3
	.byte		N44   , Dn4 , v100 , gtp3
	.byte	W48
	.byte		N23   , Fs3
	.byte		N23   , An3
	.byte	W24
	.byte		        En3
	.byte		N23   , Bn3
	.byte	W24
	.byte		N05   , Cs3
	.byte		N05   , An3
	.byte	W12
	.byte		        An2
	.byte		N05   , En3
	.byte	W84
	.byte		        Fs3
	.byte		N05   , Cs4
	.byte	W12
	.byte		        An2
	.byte		N05   , An3
	.byte	W60
	.byte		        Cs4
	.byte	W06
	.byte		        Bn3
	.byte	W06
	.byte		        An3
	.byte	W06
	.byte		        Bn3
	.byte	W06
	.byte		N23   , Fs3
	.byte		N23   , Dn4
	.byte	W24
	.byte		        Gs3
	.byte		N23   , En4
	.byte	W24
	.byte		        En3
	.byte		N23   , Cs4
	.byte	W24
	.byte		        Cs3
	.byte		N44   , An3 , v100 , gtp3
	.byte	W24
	.byte		N23   , Dn3
	.byte	W24
	.byte		        An2
	.byte		N23   , Fs3
	.byte	W24
	.byte		N44   , Dn3 , v100 , gtp3
	.byte		N23   , An3
	.byte	W24
	.byte		        Bn3
	.byte	W24
	.byte		        Cs3
	.byte		N23   , An3
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		PAN   , c_v-17
	.byte	W96
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_644_3_000
	.byte	PATT
	 ptrvalue	mus_qz_644_3_001
	.byte	PATT
	 ptrvalue	mus_qz_644_3_002
	.byte	PATT
	 ptrvalue	mus_qz_644_3_003
	.byte	GOTO
	 ptrvalue	mus_qz_644_3_B1
	.byte	FINE

/********************** Track  4 **********************/

mus_qz_644_4:
	.byte		KEYSH , mus_qz_644_key+0
	.byte		VOL   , 62*mus_qz_644_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
mus_qz_644_4_B1:
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		VOICE , 34
	.byte		PAN   , c_v-28
	.byte		VOL   , 42*mus_qz_644_mvl/mxv
	.byte	W48
	.byte		N05   , Cs4 , v100
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Dn4
	.byte	W12
	.byte		        En4
	.byte	W12
	.byte		        An4
	.byte	W06
	.byte		N17   , En4
	.byte	W06
	.byte	W12
	.byte		N11   , Fs4
	.byte	W12
	.byte		        An4
	.byte	W12
	.byte		N05   , Cs4
	.byte	W06
	.byte		        Dn4
	.byte	W06
	.byte		        En4
	.byte	W06
	.byte		        Fs4
	.byte	W06
	.byte		N11   , An4
	.byte	W12
	.byte		N05   , Gs4
	.byte	W06
	.byte		        An4
	.byte	W06
	.byte		N11   , Bn4
	.byte	W12
	.byte		N05   , An4
	.byte	W12
	.byte		        En4
	.byte	W60
	.byte		        Cs5
	.byte	W06
	.byte		        Bn4
	.byte	W06
	.byte		        An4
	.byte	W06
	.byte		        Bn4
	.byte	W06
	.byte		        An4
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        En4
	.byte	W48
	.byte		        Cs5
	.byte	W06
	.byte		        Dn5
	.byte	W06
	.byte		        Cs5
	.byte	W06
	.byte		        Bn4
	.byte	W06
	.byte		        An4
	.byte	W06
	.byte		        Bn4
	.byte	W06
	.byte		        Cs5
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		N05   , Fs5
	.byte	W06
	.byte		        En5
	.byte	W06
	.byte		        Dn5
	.byte	W06
	.byte		N11   , Cs5
	.byte	W18
	.byte	N11
	.byte	W12
	.byte		N05   , En4
	.byte	W06
	.byte		        Fs4
	.byte	W06
	.byte		        Gs4
	.byte	W06
	.byte		N17   , An4
	.byte	W06
	.byte	W12
	.byte		N11   , Fs4
	.byte	W12
	.byte		        An4
	.byte	W12
	.byte		N05   , Dn4
	.byte	W06
	.byte		N11   , En4
	.byte	W12
	.byte	N05
	.byte	W06
	.byte		N11   , Dn4
	.byte	W12
	.byte		N05   , Cs4
	.byte	W12
	.byte		        Dn4
	.byte	W12
	.byte		VOL   , 60*mus_qz_644_mvl/mxv
	.byte		PAN   , c_v+20
	.byte		N23   , An3
	.byte		N23   , Cs4
	.byte		N23   , En4
	.byte	W24
	.byte		N05   , An3
	.byte		N05   , Dn4
	.byte		N05   , Fs4
	.byte	W06
	.byte		        An3
	.byte		N05   , Cs4
	.byte		N05   , En4
	.byte	W12
	.byte		        An3
	.byte		N05   , Cs4
	.byte		N05   , En4
	.byte	W12
	.byte		        Gs3
	.byte		N05   , Bn3
	.byte		N05   , Dn4
	.byte	W06
	.byte		        En3
	.byte		N05   , An3
	.byte		N05   , Cs4
	.byte	W18
	.byte		        An3
	.byte		N05   , Cs4
	.byte		N05   , En4
	.byte	W06
	.byte		        Cs4
	.byte		N05   , En4
	.byte		N05   , An4
	.byte	W06
	.byte		        En4
	.byte		N05   , Gs4
	.byte		N05   , Bn4
	.byte	W06
	.byte		        Fs4
	.byte		N05   , An4
	.byte		N05   , Cs5
	.byte	W18
	.byte		        Fs4
	.byte		N05   , An4
	.byte		N05   , Dn5
	.byte	W18
	.byte		        An3
	.byte		N05   , Dn4
	.byte		N05   , Fs4
	.byte	W12
	.byte		        Bn3
	.byte		N05   , En4
	.byte		N05   , An4
	.byte	W06
	.byte		        Bn3
	.byte		N05   , En4
	.byte		N05   , An4
	.byte	W06
	.byte		        Bn3
	.byte		N05   , En4
	.byte		N05   , Gs4
	.byte	W12
	.byte		        An3
	.byte		N05   , Dn4
	.byte		N05   , Fs4
	.byte	W06
	.byte		        Bn3
	.byte		N05   , En4
	.byte		N05   , Gs4
	.byte	W12
	.byte		        Cs4
	.byte		N05   , En4
	.byte		N05   , An4
	.byte	W06
	.byte	W12
	.byte		VOL   , 52*mus_qz_644_mvl/mxv
	.byte		PAN   , c_v-32
	.byte		N05   , An3
	.byte		N05   , Cs4
	.byte		N05   , En4
	.byte	W12
	.byte		        An3
	.byte		N05   , Dn4
	.byte		N05   , Fs4
	.byte	W06
	.byte		        An3
	.byte		N05   , Cs4
	.byte		N05   , En4
	.byte	W12
	.byte		        An3
	.byte		N05   , Cs4
	.byte		N05   , En4
	.byte	W12
	.byte		        Gs3
	.byte		N05   , Bn3
	.byte		N05   , Dn4
	.byte	W06
	.byte		        En3
	.byte		N05   , An3
	.byte		N05   , Cs4
	.byte	W18
	.byte		        An3
	.byte		N05   , Cs4
	.byte		N05   , En4
	.byte	W06
	.byte		        Cs4
	.byte		N05   , En4
	.byte		N05   , An4
	.byte	W06
	.byte		        En4
	.byte		N05   , Gs4
	.byte		N05   , Bn4
	.byte	W06
	.byte		        Fs4
	.byte		N05   , An4
	.byte		N05   , Cs5
	.byte	W18
	.byte		        Fs4
	.byte		N05   , An4
	.byte		N05   , Dn5
	.byte	W18
	.byte		        An3
	.byte		N05   , Dn4
	.byte		N05   , Fs4
	.byte	W12
	.byte		        Bn3
	.byte		N05   , En4
	.byte		N05   , An4
	.byte	W06
	.byte		        Bn3
	.byte		N05   , En4
	.byte		N05   , An4
	.byte	W06
	.byte		        Bn3
	.byte		N05   , En4
	.byte		N05   , Gs4
	.byte	W12
	.byte		        An3
	.byte		N05   , Dn4
	.byte		N05   , Fs4
	.byte	W06
	.byte		        Bn3
	.byte		N05   , En4
	.byte		N05   , Gs4
	.byte	W12
	.byte		N76   , Cs4 , v100 , gtp1
	.byte		N76   , En4 , v100 , gtp1
	.byte		N76   , An4 , v100 , gtp1
	.byte	W06
	.byte	W24
	.byte		VOL   , 49*mus_qz_644_mvl/mxv
	.byte	W05
	.byte		        41*mus_qz_644_mvl/mxv
	.byte	W07
	.byte		        34*mus_qz_644_mvl/mxv
	.byte	W05
	.byte		        23*mus_qz_644_mvl/mxv
	.byte	W07
	.byte		        17*mus_qz_644_mvl/mxv
	.byte	W05
	.byte		        9*mus_qz_644_mvl/mxv
	.byte	W07
	.byte		        6*mus_qz_644_mvl/mxv
	.byte	W05
	.byte		        0*mus_qz_644_mvl/mxv
	.byte	W07
	.byte		        0*mus_qz_644_mvl/mxv
	.byte	W24
	.byte		        62*mus_qz_644_mvl/mxv
	.byte		        21*mus_qz_644_mvl/mxv
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	GOTO
	 ptrvalue	mus_qz_644_4_B1
	.byte	FINE

/********************** Track  5 **********************/

mus_qz_644_5:
	.byte		KEYSH , mus_qz_644_key+0
	.byte		VOICE , 22
	.byte		VOL   , 62*mus_qz_644_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte		VOL   , 79*mus_qz_644_mvl/mxv
	.byte	W96
	.byte	W96
mus_qz_644_5_000:
	.byte		N23   , An1 , v127
	.byte	W48
	.byte		N11   , Cs1
	.byte	W24
	.byte	N05
	.byte	W18
	.byte		        Dn1
	.byte	W06
	.byte	PEND
mus_qz_644_5_001:
	.byte	W12
	.byte		N05   , Dn1 , v127
	.byte	W12
	.byte	N05
	.byte	W12
	.byte		        Fs1
	.byte	W12
	.byte		        En1
	.byte	W24
	.byte	N05
	.byte	W18
	.byte		N28   , An1 , v127 , gtp1
	.byte	W06
	.byte	PEND
mus_qz_644_5_002:
	.byte	W48
	.byte		N11   , Cs1 , v127
	.byte	W24
	.byte	N05
	.byte	W18
	.byte		        Dn1
	.byte	W06
	.byte	PEND
mus_qz_644_5_003:
	.byte	W12
	.byte		N05   , Dn1 , v127
	.byte	W12
	.byte	N05
	.byte	W12
	.byte		        Fs1
	.byte	W12
	.byte		        En1
	.byte	W24
	.byte	N05
	.byte	W24
	.byte	PEND
mus_qz_644_5_B1:
	.byte		N11   , An1 , v127
	.byte	W12
	.byte		N05   , Bn1
	.byte	W06
	.byte		        An1
	.byte	W12
	.byte		N11   , Cs1
	.byte	W12
	.byte		N05   , Dn1
	.byte	W06
	.byte		N11   , En1
	.byte	W12
	.byte		N17   , An0
	.byte	W18
	.byte	N05
	.byte	W06
	.byte		        Bn0
	.byte	W06
	.byte		        Cs1
	.byte	W06
	.byte		N11   , Dn1
	.byte	W12
	.byte		N05   , En1
	.byte	W06
	.byte		N17   , Dn1
	.byte	W18
	.byte		N11   , Fs1
	.byte	W12
	.byte		        En1
	.byte	W12
	.byte	N05
	.byte	W06
	.byte		N11   , Gs1
	.byte	W12
	.byte		N05   , An1
	.byte	W06
	.byte		N11   , Gs1
	.byte	W12
	.byte		        An1
	.byte	W12
	.byte		N05   , Bn1
	.byte	W06
	.byte		N11   , An1
	.byte	W12
	.byte		        Cs1
	.byte	W12
	.byte		N05   , Dn1
	.byte	W06
	.byte		N11   , En1
	.byte	W12
	.byte		        An1
	.byte	W12
	.byte		N05   , An0
	.byte	W06
	.byte		        Bn0
	.byte	W06
	.byte		N11   , Cs1
	.byte	W12
	.byte		        Dn1
	.byte	W12
	.byte		N05   , Cs1
	.byte	W06
	.byte		N11   , Dn1
	.byte	W12
	.byte		N05   , En1
	.byte	W06
	.byte		        Fs1
	.byte	W06
	.byte		        An1
	.byte	W06
	.byte		N11   , En1
	.byte	W12
	.byte		N05   , Fs1
	.byte	W06
	.byte		N11   , En1
	.byte	W12
	.byte	N05
	.byte	W06
	.byte		        Fs1
	.byte	W06
	.byte		        Gs1
	.byte	W06
	.byte		        An1
	.byte	W36
	.byte	N05
	.byte	W48
	.byte		        Cs1
	.byte	W12
	.byte		        Dn1
	.byte	W36
	.byte		        En1
	.byte	W48
	.byte		        Gs1
	.byte	W12
	.byte		N11   , An1
	.byte	W12
	.byte	N05
	.byte	W24
	.byte	N11
	.byte	W24
	.byte		N05   , Gs1
	.byte	W06
	.byte		        An1
	.byte	W12
	.byte		N11   , Cs1
	.byte	W12
	.byte		N05   , En1
	.byte	W06
	.byte		        Dn1
	.byte	W12
	.byte	N11
	.byte	W24
	.byte	N11
	.byte	W18
	.byte		N05   , En1
	.byte	W06
	.byte		        Fs1
	.byte	W06
	.byte		N11   , An1
	.byte	W18
	.byte		N05   , Gs1
	.byte	W06
	.byte		        Bn1
	.byte	W06
	.byte		        An1
	.byte	W12
	.byte		        An0
	.byte	W84
	.byte		        Dn1
	.byte	W12
	.byte		        Dn0
	.byte	W60
	.byte		        An1
	.byte	W06
	.byte		        En1
	.byte	W06
	.byte		        Fs1
	.byte	W06
	.byte		        En1
	.byte	W06
	.byte		N44   , Dn1 , v127 , gtp3
	.byte	W48
	.byte		        Cs1 , v127 , gtp3
	.byte	W48
	.byte		        Dn1 , v127 , gtp3
	.byte	W48
	.byte		        En1 , v127 , gtp3
	.byte	W48
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_644_5_000
	.byte	PATT
	 ptrvalue	mus_qz_644_5_001
	.byte	PATT
	 ptrvalue	mus_qz_644_5_002
	.byte	PATT
	 ptrvalue	mus_qz_644_5_003
	.byte	GOTO
	 ptrvalue	mus_qz_644_5_B1
	.byte	FINE

/********************** Track  6 **********************/

mus_qz_644_6:
	.byte		KEYSH , mus_qz_644_key+0
	.byte		VOICE , 10
	.byte		VOL   , 62*mus_qz_644_mvl/mxv
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte		VOL   , 65*mus_qz_644_mvl/mxv
	.byte	W96
mus_qz_644_6_000:
	.byte	W72
	.byte		N05   , En1 , v088
	.byte	W06
	.byte		        Bn0
	.byte	W06
	.byte		        En1
	.byte	W06
	.byte		        Gs1
	.byte	W06
	.byte	PEND
mus_qz_644_6_001:
	.byte		N23   , An1 , v127
	.byte	W48
	.byte		N11   , Cs1
	.byte	W24
	.byte	N05
	.byte	W18
	.byte		        Dn1
	.byte	W06
	.byte	PEND
mus_qz_644_6_002:
	.byte	W12
	.byte		N05   , Dn1 , v127
	.byte	W12
	.byte	N05
	.byte	W12
	.byte		        Fs1
	.byte	W12
	.byte		        En1
	.byte	W24
	.byte	N05
	.byte	W18
	.byte		N28   , An1 , v127 , gtp1
	.byte	W06
	.byte	PEND
mus_qz_644_6_003:
	.byte	W48
	.byte		N11   , Cs1 , v127
	.byte	W24
	.byte	N05
	.byte	W18
	.byte		        Dn1
	.byte	W06
	.byte	PEND
mus_qz_644_6_004:
	.byte	W12
	.byte		N05   , Dn1 , v127
	.byte	W12
	.byte	N05
	.byte	W12
	.byte		        Fs1
	.byte	W12
	.byte		        En1
	.byte	W24
	.byte	N05
	.byte	W24
	.byte	PEND
mus_qz_644_6_B1:
	.byte		N11   , An1 , v127
	.byte	W12
	.byte		N05   , Bn1
	.byte	W06
	.byte		        An1
	.byte	W12
	.byte		N11   , Cs1
	.byte	W12
	.byte		N05   , Dn1
	.byte	W06
	.byte		N11   , En1
	.byte	W12
	.byte		N17   , An0
	.byte	W18
	.byte	N05
	.byte	W06
	.byte		        Bn0
	.byte	W06
	.byte		        Cs1
	.byte	W06
	.byte		N11   , Dn1
	.byte	W12
	.byte		N05   , En1
	.byte	W06
	.byte		N17   , Dn1
	.byte	W18
	.byte		N11   , Fs1
	.byte	W12
	.byte		        En1
	.byte	W12
	.byte	N05
	.byte	W06
	.byte		N11   , Gs1
	.byte	W12
	.byte		N05   , An1
	.byte	W06
	.byte		N11   , Gs1
	.byte	W12
	.byte		        An1
	.byte	W12
	.byte		N05   , Bn1
	.byte	W06
	.byte		N11   , An1
	.byte	W12
	.byte		        Cs1
	.byte	W12
	.byte		N05   , Dn1
	.byte	W06
	.byte		N11   , En1
	.byte	W12
	.byte		        An1
	.byte	W12
	.byte		N05   , An0
	.byte	W06
	.byte		        Bn0
	.byte	W06
	.byte		N11   , Cs1
	.byte	W12
	.byte		        Dn1
	.byte	W12
	.byte		N05   , Cs1
	.byte	W06
	.byte		N11   , Dn1
	.byte	W12
	.byte		N05   , En1
	.byte	W06
	.byte		        Fs1
	.byte	W06
	.byte		        An1
	.byte	W06
	.byte		N11   , En1
	.byte	W12
	.byte		N05   , Fs1
	.byte	W06
	.byte		N11   , En1
	.byte	W12
	.byte	N05
	.byte	W06
	.byte		        Fs1
	.byte	W06
	.byte		        Gs1
	.byte	W06
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		        An1
	.byte	W12
	.byte		        An0
	.byte	W84
	.byte		        Dn1
	.byte	W12
	.byte		        Dn0
	.byte	W84
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_644_6_000
	.byte	PATT
	 ptrvalue	mus_qz_644_6_001
	.byte	PATT
	 ptrvalue	mus_qz_644_6_002
	.byte	PATT
	 ptrvalue	mus_qz_644_6_003
	.byte	PATT
	 ptrvalue	mus_qz_644_6_004
	.byte	GOTO
	 ptrvalue	mus_qz_644_6_B1
	.byte	FINE

/********************** Track  7 **********************/

mus_qz_644_7:
	.byte		KEYSH , mus_qz_644_key+0
	.byte		VOICE , 39
	.byte		VOL   , 62*mus_qz_644_mvl/mxv
	.byte		        79*mus_qz_644_mvl/mxv
	.byte	W96
mus_qz_644_7_000:
	.byte	W72
	.byte		N05   , Gs2 , v100
	.byte	W06
	.byte	N05
	.byte	W12
	.byte	N05
	.byte	W06
	.byte	PEND
mus_qz_644_7_001:
	.byte		N05   , Cn1 , v100
	.byte	W06
	.byte		        Dn3
	.byte	W06
	.byte		        Ds3
	.byte	W06
	.byte		        Cn1
	.byte		N05   , En3
	.byte	W06
	.byte		        Cn1
	.byte	W06
	.byte		        Ds3
	.byte	W06
	.byte		        Dn3
	.byte	W06
	.byte		        Cn1
	.byte		N05   , En3
	.byte	W06
	.byte		        Cn1
	.byte	W06
	.byte		        Dn3
	.byte	W06
	.byte		        En3
	.byte	W06
	.byte		        Cn1
	.byte		N05   , En3
	.byte	W06
	.byte		        Cn1
	.byte	W06
	.byte		        Ds3
	.byte	W06
	.byte		        En3
	.byte	W06
	.byte		        Cn1
	.byte		N05   , Ds3
	.byte	W06
	.byte	PEND
mus_qz_644_7_002:
	.byte		N05   , Cn1 , v100
	.byte	W06
	.byte		        Dn3
	.byte	W06
	.byte		        Ds3
	.byte	W06
	.byte		        Cn1
	.byte		N05   , En3
	.byte	W06
	.byte		        Cn1
	.byte	W06
	.byte		        Ds3
	.byte	W06
	.byte		        Dn3
	.byte	W06
	.byte		        Cn1
	.byte		N05   , En3
	.byte	W06
	.byte		        Cn1
	.byte	W06
	.byte		        Dn3
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Cn1
	.byte		N05   , En3
	.byte	W06
	.byte		        Cn1
	.byte	W06
	.byte		        Ds3
	.byte	W06
	.byte		        En3
	.byte	W06
	.byte		        Cn1
	.byte		N05   , Ds3
	.byte	W06
	.byte	PEND
mus_qz_644_7_003:
	.byte		N05   , Cn1 , v100
	.byte	W06
	.byte		        Dn3
	.byte	W06
	.byte		        Ds3
	.byte	W06
	.byte		        Cn1
	.byte		N05   , En3
	.byte	W06
	.byte		        Cn1
	.byte	W06
	.byte		        Ds3
	.byte	W06
	.byte		        Dn3
	.byte	W06
	.byte		        Cn1
	.byte		N11   , En3
	.byte	W06
	.byte		N05   , Cn1
	.byte	W06
	.byte		        Dn3
	.byte	W06
	.byte		        En3
	.byte	W06
	.byte		        Cn1
	.byte		N05   , En3
	.byte	W06
	.byte		        Cn1
	.byte	W06
	.byte		        Ds3
	.byte	W06
	.byte		        En3
	.byte	W06
	.byte		        Cn1
	.byte		N05   , Ds3
	.byte	W06
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_644_7_002
mus_qz_644_7_B1:
	.byte		N05   , Cn1 , v100
	.byte	W06
	.byte		        Dn3
	.byte	W06
	.byte		        Ds3
	.byte	W06
	.byte		        Cn1
	.byte		N05   , En3
	.byte	W06
	.byte		N11   , Dn1
	.byte	W06
	.byte		N05   , Ds3
	.byte	W06
	.byte		        Dn3
	.byte	W06
	.byte		        Cn1
	.byte		N05   , En3
	.byte	W06
	.byte		        Cn1
	.byte	W06
	.byte		        Dn3
	.byte	W06
	.byte		        En3
	.byte	W06
	.byte		        Cn1
	.byte		N05   , En3
	.byte	W06
	.byte		N11   , Dn1
	.byte	W06
	.byte		N05   , Ds3
	.byte	W06
	.byte		        En3
	.byte	W06
	.byte		        Cn1
	.byte		N05   , Ds3
	.byte	W06
	.byte	PEND
mus_qz_644_7_004:
	.byte		N05   , Cn1 , v100
	.byte	W06
	.byte		        Dn3
	.byte	W06
	.byte		        Ds3
	.byte	W06
	.byte		        Cn1
	.byte		N05   , En3
	.byte	W06
	.byte		N11   , Dn1
	.byte	W06
	.byte		N05   , Ds3
	.byte	W06
	.byte		        Dn3
	.byte	W06
	.byte		        Cn1
	.byte		N05   , En3
	.byte	W06
	.byte		        Cn1
	.byte	W06
	.byte		        Dn3
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Cn1
	.byte		N05   , En3
	.byte	W06
	.byte		N11   , Dn1
	.byte	W06
	.byte		N05   , Ds3
	.byte	W06
	.byte		        En3
	.byte	W06
	.byte		        Cn1
	.byte		N05   , Ds3
	.byte	W06
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_644_7_B1
	.byte	PATT
	 ptrvalue	mus_qz_644_7_004
mus_qz_644_7_005:
	.byte		N05   , Cn1 , v100
	.byte	W12
	.byte		        Ds3
	.byte	W06
	.byte		        Cn1
	.byte	W06
	.byte		N11   , Dn1
	.byte	W06
	.byte		N05   , En3
	.byte	W06
	.byte		        Cn1
	.byte	W06
	.byte		        En3
	.byte	W06
	.byte		        Cn1
	.byte	W06
	.byte		        Ds3
	.byte	W06
	.byte		        En3
	.byte	W06
	.byte		        Cn1
	.byte	W06
	.byte		        Dn1
	.byte		N05   , Ds3
	.byte	W06
	.byte		        Cn1
	.byte		N05   , En3
	.byte	W12
	.byte		        Cn1
	.byte		N05   , En3
	.byte	W06
	.byte	PEND
mus_qz_644_7_006:
	.byte		N05   , Cn1 , v100
	.byte	W12
	.byte		        En3
	.byte	W06
	.byte		        Cn1
	.byte	W06
	.byte		N11   , Dn1
	.byte	W06
	.byte		N05   , Ds3
	.byte	W06
	.byte		        Cn1
	.byte	W06
	.byte		        En3
	.byte	W06
	.byte		        Cn1
	.byte	W06
	.byte		        Ds3
	.byte	W06
	.byte		        En3
	.byte	W06
	.byte		        Cn1
	.byte	W06
	.byte		        Dn1
	.byte	W06
	.byte		        Cn1
	.byte		N05   , En3
	.byte	W12
	.byte		        Cn1
	.byte		N05   , Ds3
	.byte	W06
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_644_7_005
	.byte	PATT
	 ptrvalue	mus_qz_644_7_006
mus_qz_644_7_007:
	.byte		N05   , Cn1 , v100
	.byte	W06
	.byte		        Dn3
	.byte	W06
	.byte		        Cn1
	.byte	W06
	.byte		        En3
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Ds3
	.byte	W06
	.byte		        Dn3
	.byte	W06
	.byte		        Ds3
	.byte	W06
	.byte		        En3
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Dn3
	.byte	W06
	.byte		        En3
	.byte	W06
	.byte		        Dn3
	.byte	W06
	.byte		        Ds3
	.byte	W06
	.byte		        En3
	.byte	W06
	.byte		        Ds3
	.byte	W06
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_644_7_007
	.byte	PATT
	 ptrvalue	mus_qz_644_7_B1
	.byte	PATT
	 ptrvalue	mus_qz_644_7_004
	.byte	PATT
	 ptrvalue	mus_qz_644_7_001
	.byte	PATT
	 ptrvalue	mus_qz_644_7_002
	.byte	PATT
	 ptrvalue	mus_qz_644_7_001
	.byte	PATT
	 ptrvalue	mus_qz_644_7_002
	.byte	W96
	.byte	PATT
	 ptrvalue	mus_qz_644_7_000
	.byte	PATT
	 ptrvalue	mus_qz_644_7_001
	.byte	PATT
	 ptrvalue	mus_qz_644_7_002
	.byte	PATT
	 ptrvalue	mus_qz_644_7_003
	.byte	PATT
	 ptrvalue	mus_qz_644_7_002
	.byte	GOTO
	 ptrvalue	mus_qz_644_7_B1
	.byte	FINE

/******************************************************/
	.p2align	2

mus_qz_644:
	.byte	7	/* NumTrks */
	.byte	0	/* NumBlks */
	.byte	mus_qz_644_pri	/* Priority */
	.byte	mus_qz_644_rev	/* Reverb. */
space64 4
	ptrvalue	mus_qz_644_grp

	ptrvalue	mus_qz_644_1
	ptrvalue	mus_qz_644_2
	ptrvalue	mus_qz_644_3
	ptrvalue	mus_qz_644_4
	ptrvalue	mus_qz_644_5
	ptrvalue	mus_qz_644_6
	ptrvalue	mus_qz_644_7

	.end
