	.include "asm/macros/bit_width.inc"

	.include "MPlayDef.s"

/* Moemon Quetzal song 616, disassembled from the ROM by tools/quetzal_rip/songs.py.
   "org" comments give the ROM address of the bytes that follow. */

	.equ	mus_qz_616_grp, voicegroup_qz_songs
	.equ	mus_qz_616_pri, 0
	.equ	mus_qz_616_rev, reverb_set+0
	.equ	mus_qz_616_mvl, 127
	.equ	mus_qz_616_key, 0
	.equ	mus_qz_616_tbs, 1
	.equ	mus_qz_616_exg, 1
	.equ	mus_qz_616_cmp, 1

	.section .rodata
	.global	mus_qz_616
	.p2align	2
/* org 0x09672688 */

/********************** Track  1 **********************/

mus_qz_616_1:
	.byte		KEYSH , mus_qz_616_key+0
mus_qz_616_1_B1:
	.byte		TEMPO , 84*mus_qz_616_tbs/2
	.byte		VOICE , 42
	.byte		PAN   , c_v-32
	.byte		        c_v+0
	.byte		BENDR , 6
	.byte		PRIO  , 64
	.byte		LFOS  , 50
	.byte		LFODL , 2
	.byte		PAN   , c_v-37
	.byte		VOL   , 39*mus_qz_616_mvl/mxv
	.byte		N04   , Ds4 , v100
	.byte	W04
	.byte		        As3
	.byte	W04
	.byte		N40   , Gs3
	.byte	W40
	.byte		N04   , As3
	.byte	W04
	.byte		        Gs3
	.byte	W04
	.byte		        Cs3
	.byte	W04
	.byte		        Ds3
	.byte	W04
	.byte		        Cs3
	.byte	W04
	.byte		        Gs2
	.byte	W04
	.byte		        As2
	.byte	W04
	.byte		        Gs2
	.byte	W04
	.byte		        Cs2
	.byte	W04
	.byte		        Ds2
	.byte	W04
	.byte		        Gs2
	.byte	W04
	.byte		        As2
	.byte	W04
	.byte		N07   , Ds3
	.byte	W07
	.byte		N02   , Cs3 , v076
	.byte	W02
	.byte		        Ds3 , v072
	.byte	W03
	.byte		        Cs3 , v060
	.byte	W02
	.byte		        Ds3 , v048
	.byte	W02
	.byte		        Cs3 , v040
	.byte	W03
	.byte		        Ds3 , v036
	.byte	W02
	.byte	N02
	.byte	W03
	.byte		        Ds3 , v044
	.byte	W02
	.byte		        Ds3 , v048
	.byte	W02
	.byte		        Ds3 , v056
	.byte	W03
	.byte		        Ds3 , v064
	.byte	W02
	.byte		        Ds3 , v076
	.byte	W03
	.byte		        Ds3 , v084
	.byte	W02
	.byte		        Ds3 , v100
	.byte	W02
	.byte		        Ds3 , v116
	.byte	W03
	.byte		        Ds3 , v127
	.byte	W02
	.byte	N02
	.byte	W03
	.byte		N04   , En3 , v092
	.byte	W04
	.byte		        Ds3 , v100
	.byte	W04
	.byte		        Gs2
	.byte	W04
	.byte		N12   , As2
	.byte	W12
	.byte		N08   , Gs2
	.byte	W08
	.byte		        As2
	.byte	W08
	.byte		        Cs3
	.byte	W08
	.byte		        Ds3
	.byte	W08
	.byte		        Cs3
	.byte	W08
	.byte		        As2
	.byte	W08
	.byte		N24   , Gs2
	.byte	W12
	.byte		MOD   , 21
	.byte	W12
	.byte		        0
	.byte		N04   , Fs2
	.byte	W04
	.byte		        Gs2
	.byte	W04
	.byte		N16   , As2
	.byte	W16
	.byte		N07   , Gs2
	.byte	W07
	.byte		N02   , Gs2 , v076
	.byte	W02
	.byte		        Gs2 , v072
	.byte	W03
	.byte		        Gs2 , v060
	.byte	W02
	.byte		        Gs2 , v048
	.byte	W02
	.byte		        Gs2 , v040
	.byte	W03
	.byte		        Gs2 , v036
	.byte	W02
	.byte	N02
	.byte	W03
	.byte		N07   , Fs2 , v100
	.byte	W07
	.byte		N02   , Fs2 , v076
	.byte	W02
	.byte		        Fs2 , v072
	.byte	W03
	.byte		        Fs2 , v060
	.byte	W02
	.byte		        Fs2 , v048
	.byte	W02
	.byte		        Fs2 , v040
	.byte	W03
	.byte		        Fs2 , v036
	.byte	W02
	.byte	N02
	.byte	W03
	.byte		N07   , Gs2 , v100
	.byte	W07
	.byte		N02   , Gs2 , v076
	.byte	W02
	.byte		        Gs2 , v072
	.byte	W03
	.byte		        Gs2 , v060
	.byte	W02
	.byte		        Gs2 , v048
	.byte	W02
	.byte		        Gs2 , v040
	.byte	W03
	.byte		        Gs2 , v036
	.byte	W02
	.byte	N02
	.byte	W03
	.byte		N07   , As2 , v100
	.byte	W07
	.byte		N02   , As2 , v076
	.byte	W02
	.byte		        As2 , v072
	.byte	W03
	.byte		        As2 , v060
	.byte	W02
	.byte		        As2 , v048
	.byte	W02
	.byte		        As2 , v040
	.byte	W03
	.byte		        As2 , v036
	.byte	W02
	.byte	N02
	.byte	W03
	.byte		        As2 , v044
	.byte	W02
	.byte		        As2 , v048
	.byte	W02
	.byte		        As2 , v056
	.byte	W03
	.byte		        As2 , v064
	.byte	W02
	.byte		        As2 , v076
	.byte	W03
	.byte		        As2 , v080
	.byte	W02
	.byte		        As2 , v088
	.byte	W04
	.byte		VOL   , 54*mus_qz_616_mvl/mxv
	.byte		N02   , Fn2
	.byte	W03
	.byte		        Fs2 , v084
	.byte	W03
	.byte		BEND  , c_v-16
	.byte		N32   , Cs3 , v127 , gtp3
	.byte	W05
	.byte		BEND  , c_v+0
	.byte	W01
	.byte		        c_v+2
	.byte	W06
	.byte		MOD   , 7
	.byte	W24
	.byte		        0
	.byte		N11   , Ds3 , v080
	.byte	W12
	.byte		N07   , Bn2 , v100
	.byte	W07
	.byte		N02   , Bn2 , v068
	.byte	W02
	.byte		        Bn2 , v056
	.byte	W03
	.byte	N02
	.byte	W02
	.byte	N02
	.byte	W02
	.byte		        Bn2 , v048
	.byte	W03
	.byte	N02
	.byte	W02
	.byte	N02
	.byte	W03
	.byte	N02
	.byte	W02
	.byte	N02
	.byte	W02
	.byte		        Bn2 , v052
	.byte	W03
	.byte		        Bn2 , v056
	.byte	W02
	.byte	N02
	.byte	W03
	.byte		        Bn2 , v060
	.byte	W02
	.byte	N02
	.byte	W02
	.byte		        Bn2 , v068
	.byte	W03
	.byte		        Bn2 , v076
	.byte	W02
	.byte		        Bn2 , v072
	.byte	W03
	.byte		N07   , En3 , v100
	.byte	W07
	.byte		N02   , En3 , v052
	.byte	W02
	.byte		        En3 , v048
	.byte	W03
	.byte		        En3 , v044
	.byte	W02
	.byte		        En3 , v036
	.byte	W02
	.byte		        En3 , v032
	.byte	W03
	.byte	N02
	.byte	W02
	.byte		        En3 , v040
	.byte	W03
	.byte		        En3 , v044
	.byte	W02
	.byte		        En3 , v048
	.byte	W02
	.byte		        En3 , v052
	.byte	W03
	.byte		        En3 , v064
	.byte	W02
	.byte		        En3 , v072
	.byte	W03
	.byte		BEND  , c_v-23
	.byte		N23   , Ds3 , v100
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W21
	.byte		N02   , Ds3 , v072
	.byte	W02
	.byte		        Ds3 , v052
	.byte	W02
	.byte		        Ds3 , v048
	.byte	W03
	.byte		        Ds3 , v044
	.byte	W02
	.byte		        Ds3 , v032
	.byte	W03
	.byte		N11   , En3 , v100
	.byte	W12
	.byte		BEND  , c_v-14
	.byte		N11   , Fs3 , v076
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W09
	.byte		        c_v-16
	.byte		N11   , Ds3 , v100
	.byte	W02
	.byte		BEND  , c_v+0
	.byte	W10
	.byte		N05   , Bn2
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		        Gn2
	.byte	W12
	.byte		N07   , Bn2
	.byte	W07
	.byte		N02   , Bn2 , v052
	.byte	W02
	.byte		        Bn2 , v048
	.byte	W03
	.byte		        Bn2 , v044
	.byte	W02
	.byte		        Bn2 , v036
	.byte	W02
	.byte		        Bn2 , v032
	.byte	W03
	.byte	N02
	.byte	W02
	.byte		        Bn2 , v040
	.byte	W03
	.byte		BEND  , c_v-12
	.byte		N23   , Cs3 , v100
	.byte	W06
	.byte		BEND  , c_v+0
	.byte	W18
	.byte		N11   , Ds3
	.byte	W12
	.byte		BEND  , c_v-21
	.byte		N11   , Fs3
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W09
	.byte		N11   , Ds3
	.byte	W12
	.byte		N07   , Cs3
	.byte	W07
	.byte		N02   , Cs3 , v076
	.byte	W02
	.byte		        Cs3 , v040
	.byte	W03
	.byte		        Cs3 , v052
	.byte	W02
	.byte		        Cs3 , v048
	.byte	W02
	.byte		        Cs3 , v044
	.byte	W03
	.byte	N02
	.byte	W02
	.byte		        Cs3 , v048
	.byte	W03
	.byte	N02
	.byte	W02
	.byte		        Cs3 , v052
	.byte	W02
	.byte		        Cs3 , v056
	.byte	W03
	.byte		        Cs3 , v060
	.byte	W02
	.byte		        Cs3 , v064
	.byte	W03
	.byte		        Dn3 , v072
	.byte	W02
	.byte		        Dn3 , v076
	.byte	W02
	.byte		        Dn3 , v080
	.byte	W03
	.byte		        Dn3 , v084
	.byte	W02
	.byte		        Dn3 , v088
	.byte	W03
	.byte		VOL   , 53*mus_qz_616_mvl/mxv
	.byte		N05   , As3 , v100
	.byte	W06
	.byte		        Bn3
	.byte	W06
	.byte		BEND  , c_v-19
	.byte		N11   , Cs4
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W09
	.byte		N02   , Cs4 , v044
	.byte	W02
	.byte		        Cs4 , v052
	.byte	W02
	.byte		        Cs4 , v056
	.byte	W03
	.byte		        Cs4 , v064
	.byte	W02
	.byte		        Cs4 , v072
	.byte	W03
	.byte		BEND  , c_v-19
	.byte		N11   , Cs4 , v100
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W09
	.byte		N11   , Bn3
	.byte	W12
	.byte		N05   , Cs4
	.byte	W06
	.byte		        Ds4
	.byte	W06
	.byte		N32   , Bn3 , v080 , gtp3
	.byte	W36
	.byte		BEND  , c_v-19
	.byte		N11   , En4 , v100
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W09
	.byte		N11   , Gs3 , v060
	.byte	W12
	.byte		        Fs4 , v100
	.byte	W12
	.byte		N07   , Ds4
	.byte	W07
	.byte		N02   , Ds4 , v072
	.byte	W02
	.byte		        Ds4 , v060
	.byte	W03
	.byte		        Ds4 , v056
	.byte	W02
	.byte		        Ds4 , v048
	.byte	W02
	.byte		        Ds4 , v044
	.byte	W03
	.byte	N02
	.byte	W02
	.byte		        Ds4 , v040
	.byte	W03
	.byte		        Ds4 , v036
	.byte	W02
	.byte	N02
	.byte	W02
	.byte		        Ds4 , v040
	.byte	W03
	.byte		        Ds4 , v044
	.byte	W02
	.byte	N02
	.byte	W03
	.byte		        Ds4 , v052
	.byte	W02
	.byte		        Ds4 , v056
	.byte	W02
	.byte		        Ds4 , v064
	.byte	W03
	.byte		        Ds4 , v072
	.byte	W02
	.byte		        Ds4 , v076
	.byte	W03
	.byte		N03   , Ds4 , v100
	.byte	W04
	.byte		        En4
	.byte	W04
	.byte		        Fs4
	.byte	W04
	.byte		N05   , Gs4
	.byte	W06
	.byte		        Ds4
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Bn3
	.byte	W06
	.byte	N23
	.byte	W24
	.byte		N02   , Bn3 , v016
	.byte	W02
	.byte		        Bn3 , v020
	.byte	W02
	.byte	N02
	.byte	W03
	.byte		        Bn3 , v024
	.byte	W02
	.byte		        Bn3 , v028
	.byte	W03
	.byte		        Bn3 , v032
	.byte	W02
	.byte		        Bn3 , v040
	.byte	W02
	.byte	N02
	.byte	W03
	.byte		        Bn3 , v048
	.byte	W02
	.byte		        Bn3 , v064
	.byte	W03
	.byte		N05   , Fs3 , v084
	.byte	W06
	.byte		        Gs3 , v100
	.byte	W06
	.byte		        Bn3
	.byte	W06
	.byte		        Cs4
	.byte	W06
	.byte		BEND  , c_v-19
	.byte		N11   , Ds4
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W09
	.byte		N02   , Ds4 , v056
	.byte	W02
	.byte		        Ds4 , v048
	.byte	W02
	.byte		        Ds4 , v044
	.byte	W03
	.byte	N02
	.byte	W02
	.byte	N02
	.byte	W03
	.byte		        Ds4 , v052
	.byte	W02
	.byte		        Ds4 , v056
	.byte	W02
	.byte		        Ds4 , v060
	.byte	W03
	.byte		        Ds4 , v068
	.byte	W02
	.byte		        Ds4 , v076
	.byte	W03
	.byte		N05   , Fs4 , v100
	.byte	W06
	.byte		N02   , Fs4 , v056
	.byte	W02
	.byte	N02
	.byte	W02
	.byte		        Fs4 , v052
	.byte	W02
	.byte		BEND  , c_v-19
	.byte		N07   , Gs4 , v100
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W05
	.byte		N02   , Gs4 , v052
	.byte	W02
	.byte		        Gs4 , v064
	.byte	W03
	.byte		        Gs4 , v056
	.byte	W02
	.byte	N02
	.byte	W03
	.byte	N02
	.byte	W02
	.byte	N02
	.byte	W02
	.byte		        Gs4 , v064
	.byte	W02
	.byte		N07   , Fs4 , v100
	.byte	W07
	.byte		N02   , Fs4 , v076
	.byte	W02
	.byte		        Fs4 , v064
	.byte	W03
	.byte		        Fs4 , v060
	.byte	W02
	.byte	N02
	.byte	W02
	.byte		        Fs4 , v064
	.byte	W02
	.byte		VOL   , 43*mus_qz_616_mvl/mxv
	.byte	W01
	.byte		N02   , Fs4 , v072
	.byte	W01
	.byte		VOICE , 34
	.byte	W01
	.byte		PAN   , c_v-16
	.byte		N02   , En4 , v080
	.byte	W03
	.byte		PAN   , c_v-17
	.byte		VOL   , 54*mus_qz_616_mvl/mxv
	.byte		N02   , Fs4 , v108
	.byte	W03
	.byte		N66   , Gs4 , v108 , gtp1
	.byte	W09
	.byte		VOL   , 38*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        33*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        25*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        21*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		MOD   , 6
	.byte		VOL   , 19*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        16*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        15*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        16*mus_qz_616_mvl/mxv
	.byte	W06
	.byte		        19*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        23*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        27*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        32*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        36*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        39*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        45*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        46*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        51*mus_qz_616_mvl/mxv
	.byte	W09
	.byte		MOD   , 0
	.byte		N07   , Gs3 , v127
	.byte	W08
	.byte		        Bn3
	.byte	W08
	.byte		        Ds4
	.byte	W08
	.byte		N15   , Cs4
	.byte	W16
	.byte		N03   , Ds4
	.byte	W04
	.byte		        Cs4
	.byte	W04
	.byte		N23   , Bn3
	.byte	W24
	.byte		N15   , As3
	.byte	W16
	.byte		N07   , Fs3
	.byte	W08
	.byte		        As3
	.byte	W08
	.byte		        Bn3
	.byte	W08
	.byte		        Gs4
	.byte	W08
	.byte		N66   , Fs4 , v127 , gtp1
	.byte	W20
	.byte		VOL   , 31*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		MOD   , 6
	.byte		VOL   , 23*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        17*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        12*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        9*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        12*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        16*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        22*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        29*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        36*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        43*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        49*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        54*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		MOD   , 0
	.byte	N07
	.byte	W08
	.byte		        En4
	.byte	W08
	.byte		        Ds4
	.byte	W08
	.byte		N15   , Cs4
	.byte	W16
	.byte		N03   , Ds4
	.byte	W04
	.byte		        Dn4
	.byte	W04
	.byte		N15   , Cs4
	.byte	W16
	.byte		N07   , Cn4
	.byte	W08
	.byte		N15   , Bn3
	.byte	W16
	.byte		N07   , Gs3
	.byte	W08
	.byte		        As3
	.byte	W08
	.byte		        Bn3
	.byte	W08
	.byte		        Fs3
	.byte	W08
	.byte		N68   , Gs3 , v127 , gtp3
	.byte	W04
	.byte		VOL   , 41*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        30*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        20*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        13*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        13*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		MOD   , 3
	.byte		VOL   , 16*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        20*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        28*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        34*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        43*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        53*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        54*mus_qz_616_mvl/mxv
	.byte	W24
	.byte		MOD   , 0
	.byte		N07   , En4
	.byte	W08
	.byte		        Fn4
	.byte	W08
	.byte		        Fs4
	.byte	W08
	.byte		N15   , Ds4
	.byte	W16
	.byte		N03   , En4
	.byte	W04
	.byte		        Ds4
	.byte	W04
	.byte		N23   , Cs4
	.byte	W24
	.byte		N15   , Bn3
	.byte	W16
	.byte		N07   , As3
	.byte	W08
	.byte		        Bn3
	.byte	W08
	.byte		        Cn4
	.byte	W08
	.byte		        Cs4
	.byte	W08
	.byte	N15
	.byte	W16
	.byte		N78   , Ds4 , v127 , gtp1
	.byte	W08
	.byte		VOL   , 43*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        38*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        33*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        29*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        27*mus_qz_616_mvl/mxv
	.byte	W09
	.byte		MOD   , 3
	.byte	W03
	.byte		VOL   , 29*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        32*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        35*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        38*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        41*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        45*mus_qz_616_mvl/mxv
	.byte	W01
	.byte		MOD   , 6
	.byte	W03
	.byte		VOL   , 49*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        54*mus_qz_616_mvl/mxv
	.byte	W17
	.byte		MOD   , 0
	.byte		N15   , Cs4
	.byte	W16
	.byte		N03   , Ds4
	.byte	W04
	.byte		        Cs4
	.byte	W04
	.byte		N23   , Cn4
	.byte	W24
	.byte		N15   , En4
	.byte	W16
	.byte		N07   , Gs3
	.byte	W08
	.byte		VOICE , 36
	.byte		VOL   , 38*mus_qz_616_mvl/mxv
	.byte		N07   , En4
	.byte	W08
	.byte		        Fn4
	.byte	W08
	.byte		N03   , Fs4
	.byte	W04
	.byte		        Gn4
	.byte	W04
	.byte		BEND  , c_v+2
	.byte		N30   , Gs4 , v127 , gtp1
	.byte	W12
	.byte		MOD   , 3
	.byte	W24
	.byte		        0
	.byte	W04
	.byte		N05   , Ds4
	.byte	W08
	.byte		N02   , Dn4 , v100
	.byte	W03
	.byte		N17   , Ds4
	.byte	W21
	.byte		N07   , Ds4 , v127
	.byte	W08
	.byte		        Gs4
	.byte	W08
	.byte		        Ds4
	.byte	W08
	.byte		N19   , Cs4
	.byte	W12
	.byte		MOD   , 4
	.byte	W08
	.byte		N01   , Ds4
	.byte	W02
	.byte		        Cs4
	.byte	W02
	.byte		MOD   , 0
	.byte		N23   , Bn3
	.byte	W24
	.byte		N15   , As3
	.byte	W16
	.byte		N07   , Gn3
	.byte	W08
	.byte		        As3
	.byte	W08
	.byte		        Cs4
	.byte	W08
	.byte		        Gs4
	.byte	W08
	.byte		N15   , Fs4
	.byte	W16
	.byte		N07   , Cn4
	.byte	W08
	.byte		N44   , Cs4 , v127 , gtp3
	.byte	W12
	.byte		MOD   , 3
	.byte	W36
	.byte		        0
	.byte	N07
	.byte	W08
	.byte		        Fs4
	.byte	W08
	.byte		        An4
	.byte	W08
	.byte		N36   , As4 , v127 , gtp3
	.byte	W16
	.byte		MOD   , 3
	.byte	W24
	.byte		        0
	.byte		N07   , Bn4
	.byte	W08
	.byte		        Gs4
	.byte	W08
	.byte		        Fs4
	.byte	W08
	.byte		        Gn4
	.byte	W08
	.byte		N15   , Gs4
	.byte	W16
	.byte		N07   , Gs3
	.byte	W08
	.byte		N68   , En4 , v127 , gtp3
	.byte	W72
	.byte	N07
	.byte	W08
	.byte		        Fn4
	.byte	W08
	.byte		        Fs4
	.byte	W08
	.byte		N19   , Ds4
	.byte	W20
	.byte		N01   , En4
	.byte	W02
	.byte		        Ds4
	.byte	W02
	.byte		N23   , Cs4
	.byte	W24
	.byte		N15   , Bn3
	.byte	W16
	.byte		N07   , As3
	.byte	W08
	.byte		        Bn3
	.byte	W08
	.byte		        Cn4
	.byte	W08
	.byte		        Cs4
	.byte	W08
	.byte	N15
	.byte	W16
	.byte		TIE   , Ds4
	.byte	W80
	.byte	W23
	.byte	EOT
	.byte	W24
	.byte	W01
	.byte		BEND  , c_v+0
	.byte	W24
	.byte		VOICE , 42
	.byte		VOL   , 42*mus_qz_616_mvl/mxv
	.byte		N03   , As3 , v100
	.byte	W04
	.byte		N02   , As3 , v060
	.byte	W04
	.byte		N03   , Bn3 , v100
	.byte	W04
	.byte		N02   , Bn3 , v060
	.byte	W04
	.byte		N03   , Cs4 , v100
	.byte	W04
	.byte		N02   , Cs4 , v060
	.byte	W04
	.byte	GOTO
	 ptrvalue	mus_qz_616_1_B1
	.byte	FINE

/********************** Track  2 **********************/

mus_qz_616_2:
	.byte		KEYSH , mus_qz_616_key+0
mus_qz_616_2_B1:
	.byte		VOICE , 67
	.byte		PAN   , c_v-19
	.byte		        c_v+19
	.byte		        c_v+0
	.byte		BENDR , 6
	.byte		PRIO  , 63
	.byte		VOL   , 34*mus_qz_616_mvl/mxv
	.byte		N12   , Bn0 , v104
	.byte	W12
	.byte		        Fs1
	.byte	W12
	.byte		        Cs2
	.byte	W12
	.byte		N60   , Ds2
	.byte	W03
	.byte		BEND  , c_v+11
	.byte	W01
	.byte		        c_v-2
	.byte	W02
	.byte		        c_v+0
	.byte	W54
	.byte		N12   , Bn0
	.byte	W12
	.byte		        Gs1
	.byte	W12
	.byte		        Ds2
	.byte	W12
	.byte		N56   , En2 , v100 , gtp3
	.byte	W03
	.byte		BEND  , c_v+11
	.byte	W01
	.byte		        c_v-2
	.byte	W02
	.byte		        c_v+0
	.byte	W54
	.byte		N11   , Bn0
	.byte	W12
	.byte		N12   , Fs1 , v104
	.byte	W12
	.byte		        Cs2
	.byte	W12
	.byte		        Ds2
	.byte	W12
	.byte		N24   , Fs2
	.byte	W24
	.byte		N23   , Gs2 , v100
	.byte	W24
	.byte		VOICE , 23
	.byte		N24   , Fs2 , v104
	.byte	W24
	.byte		        Fn2
	.byte	W24
	.byte		        En2
	.byte	W24
	.byte		N12   , As1
	.byte	W12
	.byte		        Fs1 , v080
	.byte	W12
	.byte		PAN   , c_v-32
	.byte		VOL   , 32*mus_qz_616_mvl/mxv
	.byte		PAN   , c_v+56
	.byte		N11   , Ds3 , v100
	.byte	W12
	.byte		        Fs2
	.byte	W12
	.byte		        Ds3
	.byte	W12
	.byte		N23   , Bn2
	.byte	W24
	.byte		        As2
	.byte	W24
	.byte		N11   , Ds3
	.byte	W12
mus_qz_616_2_000:
	.byte		N11   , An3 , v100
	.byte	W12
	.byte		        Cn3
	.byte	W12
	.byte		        An3
	.byte	W12
	.byte		N23   , Cn4
	.byte	W24
	.byte		        Gs3
	.byte	W24
	.byte		N11   , Cn3
	.byte	W12
	.byte	PEND
mus_qz_616_2_001:
	.byte		N11   , En3 , v100
	.byte	W12
	.byte		        Gs2
	.byte	W12
	.byte		N05   , En3
	.byte	W12
	.byte	N11
	.byte	W12
	.byte	N23
	.byte	W24
	.byte		        Fs3
	.byte	W24
	.byte	PEND
	.byte	PATT
	 ptrvalue	mus_qz_616_2_001
	.byte		N11   , Ds3 , v100
	.byte	W12
	.byte		        Fs2
	.byte	W12
	.byte		        Ds3
	.byte	W12
	.byte		N23   , Bn2
	.byte	W24
	.byte		        As2
	.byte	W24
	.byte		N11   , Ds3
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_616_2_000
	.byte		N11   , En3 , v100
	.byte	W12
	.byte		        Gs2
	.byte	W12
	.byte		N05   , En3
	.byte	W12
	.byte	N11
	.byte	W12
	.byte	N23
	.byte	W24
	.byte	N23
	.byte	W24
	.byte	PATT
	 ptrvalue	mus_qz_616_2_001
	.byte		VOL   , 38*mus_qz_616_mvl/mxv
	.byte		N07   , Gs3 , v100
	.byte	W08
	.byte		N03   , Bn2
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , Gs3
	.byte	W16
	.byte		        Ds4 , v076
	.byte	W08
	.byte		N03   , Bn2 , v100
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , Gs3
	.byte	W08
	.byte		N03   , Bn2
	.byte	W08
	.byte		N07   , Gs3
	.byte	W08
	.byte		N03   , As2
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , Gs3
	.byte	W16
	.byte		        As3
	.byte	W08
	.byte		N03   , As2
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , Gs3
	.byte	W08
	.byte		        Gn3
	.byte	W08
	.byte		        Fs3
	.byte	W08
	.byte		N03   , As2
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , Fs3
	.byte	W08
	.byte		N03   , As2
	.byte	W08
	.byte		N07   , Cs4 , v076
	.byte	W08
	.byte		N03   , As2 , v100
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , Fs3
	.byte	W08
	.byte		        Gn3
	.byte	W08
	.byte		        Gs3
	.byte	W08
	.byte		N03   , Bn2
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , Gs3
	.byte	W08
	.byte		N03   , Bn2
	.byte	W08
	.byte		N07   , Bn3 , v076
	.byte	W08
	.byte		N03   , Bn2 , v100
	.byte	W08
	.byte		N07   , Ds3
	.byte	W08
	.byte		N03   , Bn2
	.byte	W08
	.byte		N07   , Gs3
	.byte	W08
	.byte		        Bn3
	.byte	W08
	.byte		        En3
	.byte	W08
	.byte		N03   , Gs2
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , En3
	.byte	W08
	.byte		N03   , Gs2
	.byte	W08
	.byte		N07   , Bn3 , v076
	.byte	W08
	.byte		N03   , Gs2 , v100
	.byte	W08
	.byte		N07   , En3
	.byte	W08
	.byte		N03   , Gs2
	.byte	W08
	.byte		N07   , En3
	.byte	W08
	.byte		N03   , Gs2
	.byte		N07   , Bn3
	.byte	W08
	.byte		        En3
	.byte	W08
	.byte		N03   , Fs2
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , En3
	.byte	W08
	.byte		N03   , Fs2
	.byte	W08
	.byte		N07   , As3 , v076
	.byte	W08
	.byte		N03   , Fs2 , v100
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , En3
	.byte	W08
	.byte		        Dn3
	.byte	W08
	.byte		        Ds3
	.byte	W08
	.byte		N03   , Fs2
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , Ds3
	.byte	W08
	.byte		N03   , Fs2
	.byte	W08
	.byte		N07   , Cs4 , v076
	.byte	W08
	.byte		N03   , Fs2 , v100
	.byte	W08
	.byte		N07   , Ds3
	.byte	W08
	.byte		N03   , Fs2
	.byte	W08
	.byte		N07   , Fs3
	.byte	W08
	.byte		        Cs4
	.byte	W08
	.byte		N23   , Ds3
	.byte	W24
	.byte		        Cn3
	.byte	W24
	.byte		        En3
	.byte	W24
	.byte		        Fs3
	.byte	W24
	.byte		VOL   , 41*mus_qz_616_mvl/mxv
	.byte		N07   , Gs3
	.byte	W08
	.byte		N03   , Bn2
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , Gs3
	.byte	W16
	.byte		        Ds4 , v076
	.byte	W08
	.byte		N03   , Bn2 , v100
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , Gs3
	.byte	W08
	.byte		        Fs3
	.byte	W08
	.byte		        Gn3
	.byte	W08
	.byte		N03   , As2
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , Gn3
	.byte	W16
	.byte		        As3
	.byte	W08
	.byte		N03   , As2
	.byte	W08
	.byte		N07   , En3
	.byte	W08
	.byte		N03   , As2
	.byte	W08
	.byte		N07   , Gs3
	.byte	W08
	.byte		        Gn3
	.byte	W08
	.byte		        Fs3
	.byte	W08
	.byte		N03   , As2
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , Fs3
	.byte	W08
	.byte		N03   , As2
	.byte	W08
	.byte		N07   , Cs4 , v076
	.byte	W08
	.byte		N03   , As2 , v100
	.byte	W08
	.byte		N07   , Ds3
	.byte	W08
	.byte		N03   , As2
	.byte	W08
	.byte		N07   , Fs3
	.byte	W08
	.byte		        Gn3
	.byte	W08
	.byte		        Gs3
	.byte	W08
	.byte		N03   , Bn2
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , Gs3
	.byte	W08
	.byte		N03   , Bn2
	.byte	W08
	.byte		N07   , Bn3 , v076
	.byte	W08
	.byte		N03   , Bn2 , v100
	.byte	W08
	.byte		N07   , Ds3
	.byte	W08
	.byte		        Bn2
	.byte	W08
	.byte		        Gs3
	.byte	W08
	.byte		        Gn3
	.byte	W08
	.byte		        En3
	.byte	W08
	.byte		N03   , Gs2
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , En3
	.byte	W08
	.byte		N03   , Gs2
	.byte	W08
	.byte		N07   , Bn3 , v076
	.byte	W08
	.byte		N03   , Gs2 , v100
	.byte	W08
	.byte		N07   , En3
	.byte	W08
	.byte		N03   , Gs2
	.byte	W08
	.byte		N07   , En3
	.byte	W08
	.byte		        Ds3
	.byte	W08
	.byte		        En3
	.byte	W08
	.byte		N03   , Fs2
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , En3
	.byte	W08
	.byte		N03   , Fs2
	.byte	W08
	.byte		N07   , As3 , v076
	.byte	W08
	.byte		N03   , Fs2 , v100
	.byte	W08
	.byte		N07   , Fs3
	.byte	W08
	.byte		N03   , Fs2
	.byte	W08
	.byte		N07   , En3
	.byte	W08
	.byte		        Dn3
	.byte	W08
	.byte		        Bn2
	.byte	W08
	.byte		N03   , Fs2
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , Ds3
	.byte	W08
	.byte		N03   , Fs2
	.byte	W08
	.byte		N07   , Cs4 , v076
	.byte	W08
	.byte		N03   , Fs2 , v100
	.byte	W08
	.byte		N07   , Fs3
	.byte	W08
	.byte		N03   , Fs2
	.byte	W08
	.byte		N07   , Fs3
	.byte	W08
	.byte		        Fn3
	.byte	W08
	.byte		N15   , En3
	.byte	W16
	.byte	N07
	.byte	W08
	.byte		N68   , En3 , v100 , gtp3
	.byte	W48
	.byte		VOL   , 27*mus_qz_616_mvl/mxv
	.byte	W01
	.byte		        22*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        21*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        19*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        16*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        14*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        13*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        12*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        9*mus_qz_616_mvl/mxv
	.byte	W05
	.byte	GOTO
	 ptrvalue	mus_qz_616_2_B1
	.byte	FINE

/********************** Track  3 **********************/

mus_qz_616_3:
	.byte		KEYSH , mus_qz_616_key+0
mus_qz_616_3_B1:
	.byte		VOICE , 23
	.byte		PAN   , c_v-50
	.byte		        c_v+0
	.byte		PRIO  , 63
	.byte		LFOS  , 40
	.byte		VOL   , 2*mus_qz_616_mvl/mxv
	.byte		N44   , Fs2 , v100 , gtp3
	.byte	W02
	.byte		VOL   , 3*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        4*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        5*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        8*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        10*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        12*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        15*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        19*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        25*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        32*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        36*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        38*mus_qz_616_mvl/mxv
	.byte	W06
	.byte		        41*mus_qz_616_mvl/mxv
	.byte	W06
	.byte		        1*mus_qz_616_mvl/mxv
	.byte		N48   , As3 , v104
	.byte	W04
	.byte		VOL   , 2*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        4*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        5*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        7*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        9*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        13*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        18*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        25*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        32*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        46*mus_qz_616_mvl/mxv
	.byte	W08
	.byte		        5*mus_qz_616_mvl/mxv
	.byte		N44   , Gs2 , v100 , gtp3
	.byte	W02
	.byte		VOL   , 6*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        8*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        10*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        12*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        15*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        19*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        23*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        25*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        33*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        36*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        39*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        41*mus_qz_616_mvl/mxv
	.byte	W12
	.byte		        1*mus_qz_616_mvl/mxv
	.byte		N48   , En3 , v104
	.byte	W04
	.byte		VOL   , 2*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        4*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        5*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        7*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        9*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        13*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        18*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        25*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        32*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        46*mus_qz_616_mvl/mxv
	.byte	W08
	.byte		PAN   , c_v-50
	.byte		N44   , Ds2 , v100 , gtp3
	.byte	W48
	.byte		VOL   , 12*mus_qz_616_mvl/mxv
	.byte		N24   , Fs2 , v104
	.byte	W04
	.byte		VOL   , 16*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        19*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        22*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        25*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        27*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        13*mus_qz_616_mvl/mxv
	.byte		N23   , Fn2 , v100
	.byte	W04
	.byte		VOL   , 19*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        23*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        27*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        29*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        31*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        28*mus_qz_616_mvl/mxv
	.byte		N24   , Fs2 , v104
	.byte	W24
	.byte		        Fn2
	.byte	W24
	.byte		        En2
	.byte	W24
	.byte		N23   , Cs2 , v100
	.byte	W24
	.byte		VOL   , 34*mus_qz_616_mvl/mxv
	.byte		PAN   , c_v-51
	.byte		N11   , Bn2
	.byte	W12
	.byte		        Fs2
	.byte	W12
	.byte		        Bn2
	.byte	W12
	.byte		N23   , Fs2
	.byte	W24
	.byte		        Ds2
	.byte	W24
	.byte		N11   , Fs2
	.byte	W12
mus_qz_616_3_000:
	.byte		N11   , Cn3 , v100
	.byte	W12
	.byte		        Fs2
	.byte	W12
	.byte		        Cn3
	.byte	W12
	.byte		N23   , Ds3
	.byte	W24
	.byte		        Cn3
	.byte	W24
	.byte		N11   , Gs2
	.byte	W12
	.byte	PEND
	.byte	N11
	.byte	W12
	.byte		        Cs2
	.byte	W12
	.byte		N05   , Gs2
	.byte	W12
	.byte		N11   , Gn2
	.byte	W12
	.byte	N23
	.byte	W24
	.byte		        En2
	.byte	W24
mus_qz_616_3_001:
	.byte		N11   , Gs2 , v100
	.byte	W12
	.byte		        Cs2
	.byte	W12
	.byte		N05   , Gs2
	.byte	W12
	.byte		N11   , Fs2
	.byte	W12
	.byte		N23   , As2
	.byte	W24
	.byte	N23
	.byte	W24
	.byte	PEND
	.byte		N11   , Bn2
	.byte	W12
	.byte		        Fs2
	.byte	W12
	.byte		        Bn2
	.byte	W12
	.byte		N23   , Fs2
	.byte	W24
	.byte		        Ds2
	.byte	W24
	.byte		N11   , Fs2
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_616_3_000
	.byte		N11   , Gs2 , v100
	.byte	W12
	.byte		        Cs2
	.byte	W12
	.byte		N05   , Gs2
	.byte	W12
	.byte		N11   , Gn2
	.byte	W12
	.byte		N23   , Bn2
	.byte	W24
	.byte		        Cs3
	.byte	W24
	.byte	PATT
	 ptrvalue	mus_qz_616_3_001
	.byte		VOL   , 38*mus_qz_616_mvl/mxv
	.byte		PAN   , c_v-54
	.byte		N07   , Bn2 , v100
	.byte	W08
	.byte		N03   , Gs2
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		MOD   , 1
	.byte	N03
	.byte	W08
	.byte		N07   , Bn2
	.byte	W08
	.byte		N03   , Gs2
	.byte	W08
	.byte		N07   , En3 , v076
	.byte	W08
	.byte		N03   , Gs2 , v100
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		MOD   , 0
	.byte	N03
	.byte	W08
	.byte		N07   , Bn2
	.byte	W08
	.byte		N03   , Gs2
	.byte	W08
	.byte		N07   , As2
	.byte	W08
	.byte		N03   , Gs2
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , As2
	.byte	W08
	.byte		N03   , Gs2
	.byte	W08
	.byte		N07   , En3
	.byte	W08
	.byte		N03   , Gs2
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , Bn2
	.byte	W08
	.byte		        En2
	.byte	W08
	.byte		        As2
	.byte	W08
	.byte		N03   , Fs2
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , As2
	.byte	W08
	.byte		N03   , Fs2
	.byte	W08
	.byte		N07   , Ds3 , v076
	.byte	W08
	.byte		N03   , Fs2 , v100
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , As2
	.byte	W08
	.byte		N03   , Fs2
	.byte	W08
	.byte		N07   , Bn2
	.byte	W08
	.byte		N03   , Fs2
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , Bn2
	.byte	W08
	.byte		N03   , Fs2
	.byte	W08
	.byte		N07   , Fs3 , v076
	.byte	W08
	.byte		N03   , Fs2 , v100
	.byte	W08
	.byte		N07   , Bn2
	.byte	W08
	.byte		N03   , Fs2
	.byte	W08
	.byte		N07   , Ds3
	.byte	W08
	.byte		        Dn3
	.byte	W08
	.byte		        Gs2
	.byte	W08
	.byte		N03   , En2
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , Gs2
	.byte	W08
	.byte		N03   , En2
	.byte	W08
	.byte		N07   , Cs3 , v076
	.byte	W08
	.byte		N03   , En2 , v100
	.byte	W08
	.byte		N07   , Gs2
	.byte	W08
	.byte		N03   , En2
	.byte	W08
	.byte		N07   , Gs2
	.byte	W08
	.byte		N03   , En2
	.byte	W08
	.byte		N07   , Fs2
	.byte	W08
	.byte		N03   , En2
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , Fs2
	.byte	W08
	.byte		N03   , En2
	.byte	W08
	.byte		N07   , En3 , v076
	.byte	W08
	.byte		N03   , En2 , v100
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , Fs2
	.byte	W08
	.byte		        Fn2
	.byte	W08
	.byte		        Fs2
	.byte	W08
	.byte		N03   , Ds2
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , Fs2
	.byte	W08
	.byte		N03   , Ds2
	.byte	W08
	.byte		N07   , Ds3 , v076
	.byte	W08
	.byte		N03   , Ds2 , v100
	.byte	W08
	.byte		N07   , Bn2
	.byte	W08
	.byte		N03   , Ds2
	.byte	W08
	.byte		N07   , Bn2
	.byte	W08
	.byte		        Fs3
	.byte	W08
	.byte		N23   , Fs2
	.byte	W24
	.byte		        Ds2
	.byte	W24
	.byte		        Gs2
	.byte	W24
	.byte		        As2
	.byte	W24
	.byte		VOL   , 37*mus_qz_616_mvl/mxv
	.byte		N07   , Bn2
	.byte	W08
	.byte		N03   , Gs2
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , Bn2
	.byte	W08
	.byte		N03   , Gs2
	.byte	W08
	.byte		N07   , En3 , v076
	.byte	W08
	.byte		N03   , Gs2 , v100
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , Bn2
	.byte	W08
	.byte		        An2
	.byte	W08
	.byte		        As2
	.byte	W08
	.byte		N03   , Gn2
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , As2
	.byte	W08
	.byte		N03   , Gn2
	.byte	W08
	.byte		N07   , En3
	.byte	W08
	.byte		N03   , Gn2
	.byte	W08
	.byte		N07   , As2
	.byte	W08
	.byte		N03   , Gn2
	.byte	W08
	.byte		N07   , Bn2
	.byte	W08
	.byte		        An2
	.byte	W08
	.byte		        As2
	.byte	W08
	.byte		N03   , Fs2
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , As2
	.byte	W08
	.byte		N03   , Fs2
	.byte	W08
	.byte		N07   , Ds3 , v076
	.byte	W08
	.byte		N03   , Fs2 , v100
	.byte	W08
	.byte		N07   , As2
	.byte	W08
	.byte		N03   , Fs2
	.byte	W08
	.byte		N07   , As2
	.byte	W08
	.byte		        An2
	.byte	W08
	.byte		        Gs2
	.byte	W08
	.byte		N03   , Fs2
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , Bn2
	.byte	W08
	.byte		N03   , Fs2
	.byte	W08
	.byte		N07   , Fs3 , v076
	.byte	W08
	.byte		N03   , Fs2 , v100
	.byte	W08
	.byte		N07   , Bn2
	.byte	W08
	.byte		        Gs2
	.byte	W08
	.byte		        Bn2
	.byte	W08
	.byte		        Gn2
	.byte	W08
	.byte		        Gs2
	.byte	W08
	.byte		N03   , En2
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , Gs2
	.byte	W08
	.byte		N03   , En2
	.byte	W08
	.byte		N07   , Gs3 , v076
	.byte	W08
	.byte		N03   , En2 , v100
	.byte	W08
	.byte		N07   , Cs3
	.byte	W08
	.byte		N03   , En2
	.byte	W08
	.byte		N07   , Gs2
	.byte	W08
	.byte		        Fn2
	.byte	W08
	.byte		        Fs2
	.byte	W08
	.byte		N03   , En2
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , Fs2
	.byte	W08
	.byte		N03   , En2
	.byte	W08
	.byte		N07   , En3 , v076
	.byte	W08
	.byte		N03   , En2 , v100
	.byte	W08
	.byte		N07   , Cs3
	.byte	W08
	.byte		N03   , En2
	.byte	W08
	.byte		N07   , Fs2
	.byte	W08
	.byte		        Fn2
	.byte	W08
	.byte		        Fs2
	.byte	W08
	.byte		N03   , Ds2
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , Fs2
	.byte	W08
	.byte		N03   , Ds2
	.byte	W08
	.byte		N07   , Ds3 , v076
	.byte	W08
	.byte		N03   , Ds2 , v100
	.byte	W08
	.byte		N07   , Bn2
	.byte	W08
	.byte		N03   , Ds2
	.byte	W08
	.byte		N07   , Fs2
	.byte	W08
	.byte		        Gn2
	.byte	W08
	.byte		N15   , Gs2
	.byte	W16
	.byte	N07
	.byte	W08
	.byte		N68   , Gs2 , v100 , gtp3
	.byte	W48
	.byte		VOL   , 28*mus_qz_616_mvl/mxv
	.byte	W01
	.byte		        24*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        22*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        21*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        20*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        17*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        14*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        13*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        12*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        8*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        8*mus_qz_616_mvl/mxv
	.byte	W03
	.byte	GOTO
	 ptrvalue	mus_qz_616_3_B1
	.byte	FINE

/********************** Track  4 **********************/

mus_qz_616_4:
	.byte		KEYSH , mus_qz_616_key+0
mus_qz_616_4_B1:
	.byte		VOICE , 35
	.byte		PAN   , c_v-32
	.byte		        c_v+0
	.byte		BENDR , 6
	.byte		PRIO  , 60
	.byte		LFOS  , 40
	.byte		VOL   , 5*mus_qz_616_mvl/mxv
	.byte		N44   , Bn2 , v127 , gtp3
	.byte	W03
	.byte		VOL   , 10*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        11*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        13*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        14*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        18*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        21*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        22*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        25*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        29*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        34*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        35*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        38*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        39*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        40*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        42*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        5*mus_qz_616_mvl/mxv
	.byte		N48   , Fs4
	.byte	W03
	.byte		VOL   , 10*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        11*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        13*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        14*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        18*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        21*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        22*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        25*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        29*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        34*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        35*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        38*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        39*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        40*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        42*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        5*mus_qz_616_mvl/mxv
	.byte		N44   , Bn2 , v127 , gtp3
	.byte	W03
	.byte		VOL   , 10*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        11*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        13*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        14*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        18*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        21*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        22*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        25*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        29*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        34*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        35*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        38*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        39*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        40*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        42*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        5*mus_qz_616_mvl/mxv
	.byte		N48   , As4
	.byte	W03
	.byte		VOL   , 10*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        11*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        13*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        14*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        18*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        21*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        22*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        25*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        29*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        34*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        35*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        38*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        39*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        40*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        42*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        5*mus_qz_616_mvl/mxv
	.byte		PAN   , c_v+20
	.byte		N44   , As2 , v100 , gtp3
	.byte	W02
	.byte		VOL   , 8*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        10*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        12*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        13*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        15*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        18*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        21*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        25*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        32*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        36*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        41*mus_qz_616_mvl/mxv
	.byte	W16
	.byte		PAN   , c_v-34
	.byte		VOL   , 13*mus_qz_616_mvl/mxv
	.byte		N23   , Cs4 , v076
	.byte	W02
	.byte		VOL   , 19*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        21*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        29*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        34*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        41*mus_qz_616_mvl/mxv
	.byte	W01
	.byte		        38*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        41*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        41*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        15*mus_qz_616_mvl/mxv
	.byte		N23   , Bn3 , v068
	.byte	W02
	.byte		VOL   , 19*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        25*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        36*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        38*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        42*mus_qz_616_mvl/mxv
	.byte	W09
	.byte		PAN   , c_v+21
	.byte		VOL   , 39*mus_qz_616_mvl/mxv
	.byte		N23   , Fs3 , v100
	.byte	W24
	.byte		        Fn3
	.byte	W24
	.byte		        En3
	.byte	W24
	.byte		N17   , Cs3
	.byte	W24
	.byte		VOICE , 42
	.byte		VOL   , 34*mus_qz_616_mvl/mxv
	.byte		PAN   , c_v-48
	.byte		MOD   , 1
	.byte		BEND  , c_v+2
	.byte	N11
	.byte	W12
	.byte		PAN   , c_v+47
	.byte		N11   , Cs3 , v080
	.byte	W12
	.byte		PAN   , c_v-48
	.byte		N11   , Cs3 , v064
	.byte	W12
	.byte		PAN   , c_v+48
	.byte		N11   , Cs3 , v036
	.byte	W12
	.byte		PAN   , c_v-48
	.byte		N11   , Ds3 , v088
	.byte	W12
	.byte		        Bn2 , v096
	.byte	W12
	.byte		PAN   , c_v+48
	.byte		N11   , Bn2 , v080
	.byte	W12
	.byte		PAN   , c_v-48
	.byte	N11
	.byte	W12
	.byte		PAN   , c_v-48
	.byte		N11   , En3 , v100
	.byte	W12
	.byte		PAN   , c_v+47
	.byte		N11   , En3 , v104
	.byte	W12
	.byte		PAN   , c_v-48
	.byte		N11   , En3 , v088
	.byte	W12
	.byte		PAN   , c_v+48
	.byte		N11   , En3 , v032
	.byte	W12
	.byte		PAN   , c_v-48
	.byte		N11   , Ds3 , v068
	.byte	W12
	.byte		PAN   , c_v+48
	.byte		N11   , Ds3 , v056
	.byte	W12
	.byte		PAN   , c_v-48
	.byte		N11   , Ds3 , v028
	.byte	W12
	.byte		PAN   , c_v+48
	.byte		N11   , En3 , v056
	.byte	W12
	.byte		PAN   , c_v-48
	.byte	N11
	.byte	W12
	.byte		PAN   , c_v+48
	.byte		N11   , Ds3 , v080
	.byte	W12
	.byte		        Bn2
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		        Gn2 , v064
	.byte	W12
	.byte		        Bn2
	.byte	W12
	.byte		PAN   , c_v-48
	.byte	N11
	.byte	W12
	.byte		        Cs3
	.byte	W12
	.byte		PAN   , c_v+48
	.byte	N11
	.byte	W12
	.byte		        Ds3
	.byte	W12
	.byte		        Fs3
	.byte	W12
	.byte		        Ds3
	.byte	W12
	.byte		        Cs3
	.byte	W12
	.byte		        Cs3 , v044
	.byte	W03
	.byte		PAN   , c_v-48
	.byte	W09
	.byte		        c_v+48
	.byte	N11
	.byte	W12
	.byte		        Dn3 , v064
	.byte	W12
	.byte		VOL   , 34*mus_qz_616_mvl/mxv
	.byte		N05   , As3 , v100
	.byte	W06
	.byte		        Bn3
	.byte	W06
	.byte		N11   , Cs4 , v080
	.byte	W12
	.byte		        Cs4 , v072
	.byte	W12
	.byte		        Cs4 , v060
	.byte	W12
	.byte		        Bn3 , v076
	.byte	W12
	.byte		N05   , Cs4 , v044
	.byte	W06
	.byte		        Ds4 , v040
	.byte	W06
	.byte		N11   , Bn3 , v044
	.byte	W12
	.byte		        Bn3 , v036
	.byte	W12
	.byte	W12
	.byte		        En4 , v056
	.byte	W12
	.byte		        Gs3 , v060
	.byte	W12
	.byte		        Fs4 , v056
	.byte	W12
	.byte		        Ds4
	.byte	W12
	.byte		        Ds4 , v032
	.byte	W12
	.byte		        Ds4 , v020
	.byte	W24
	.byte		N03   , Ds4 , v052
	.byte	W04
	.byte		        En4
	.byte	W04
	.byte		        Fs4
	.byte	W04
	.byte		N05   , Gs4
	.byte	W06
	.byte		        Ds4 , v056
	.byte	W06
	.byte	N05
	.byte	W12
	.byte		N11   , Bn3 , v092
	.byte	W12
	.byte		        Bn3 , v080
	.byte	W12
	.byte		        Bn3 , v056
	.byte	W24
	.byte		N05   , Fs3 , v080
	.byte	W06
	.byte		        Gs3 , v076
	.byte	W06
	.byte		VOICE , 68
	.byte		PAN   , c_v-53
	.byte		N07   , En3 , v100
	.byte	W08
	.byte		PAN   , c_v+45
	.byte		N07   , En3 , v036
	.byte	W08
	.byte		PAN   , c_v-51
	.byte		N07   , En3 , v020
	.byte	W08
	.byte		PAN   , c_v+37
	.byte		N07   , En3 , v016
	.byte	W08
	.byte		PAN   , c_v-51
	.byte		N07   , En3 , v004
	.byte	W08
	.byte		PAN   , c_v+32
	.byte	N07
	.byte	W08
	.byte		VOICE , 69
	.byte		PAN   , c_v-53
	.byte		VOL   , 51*mus_qz_616_mvl/mxv
	.byte		N15   , Cn3 , v104
	.byte	W36
	.byte		MOD   , 0
	.byte	W12
	.byte		BEND  , c_v+0
	.byte	W24
	.byte		VOICE , 27
	.byte	W24
	.byte		PAN   , c_v+44
	.byte		VOL   , 34*mus_qz_616_mvl/mxv
	.byte	W24
	.byte		PAN   , c_v+15
	.byte	W08
	.byte		        c_v-32
	.byte	W16
	.byte	W04
	.byte		MOD   , 3
	.byte	W44
	.byte		        0
	.byte	W48
	.byte	W96
	.byte	W60
	.byte	W01
	.byte		        4
	.byte	W28
	.byte	W01
	.byte		        0
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
	.byte	GOTO
	 ptrvalue	mus_qz_616_4_B1
	.byte	FINE

/********************** Track  5 **********************/

mus_qz_616_5:
	.byte		KEYSH , mus_qz_616_key+0
mus_qz_616_5_B1:
	.byte		VOICE , 42
	.byte		PAN   , c_v+0
	.byte		BENDR , 6
	.byte		PRIO  , 64
	.byte		LFOS  , 35
	.byte		VOL   , 29*mus_qz_616_mvl/mxv
	.byte		PAN   , c_v+15
	.byte	W01
	.byte		N04   , Ds5 , v100
	.byte	W04
	.byte		        As4
	.byte	W04
	.byte		N40   , Gs4
	.byte	W40
	.byte		N04   , As4
	.byte	W04
	.byte		        Gs4
	.byte	W04
	.byte		        Cs4
	.byte	W04
	.byte		        Ds4
	.byte	W04
	.byte		        Cs4
	.byte	W04
	.byte		        Gs3
	.byte	W04
	.byte		        As3
	.byte	W04
	.byte		        Gs3
	.byte	W04
	.byte		        Cs3
	.byte	W04
	.byte		        Ds3
	.byte	W04
	.byte		        Gs3
	.byte	W04
	.byte		        As3
	.byte	W03
	.byte	W01
	.byte		N07   , Ds4
	.byte	W07
	.byte		N02   , Cs4 , v076
	.byte	W02
	.byte		        Ds4 , v072
	.byte	W03
	.byte		        Cs4 , v060
	.byte	W02
	.byte		        Ds4 , v048
	.byte	W03
	.byte		        Ds4 , v040
	.byte	W02
	.byte		        Ds4 , v036
	.byte	W02
	.byte	N02
	.byte	W03
	.byte		        Ds4 , v044
	.byte	W02
	.byte		        Ds4 , v048
	.byte	W03
	.byte		        Ds4 , v056
	.byte	W02
	.byte		        Ds4 , v064
	.byte	W02
	.byte		        Ds4 , v076
	.byte	W03
	.byte		        Ds4 , v084
	.byte	W02
	.byte		        Ds4 , v100
	.byte	W03
	.byte		        Ds4 , v116
	.byte	W02
	.byte		        Ds4 , v127
	.byte	W02
	.byte	N02
	.byte	W03
	.byte		N04   , En4 , v092
	.byte	W04
	.byte		        Ds4 , v100
	.byte	W04
	.byte		        Gs3
	.byte	W04
	.byte		N12   , As3
	.byte	W12
	.byte		N08   , Gs3
	.byte	W08
	.byte		        As3
	.byte	W08
	.byte		        Cs4
	.byte	W07
	.byte		BEND  , c_v-7
	.byte	W01
	.byte		N08   , Ds4
	.byte	W02
	.byte		BEND  , c_v+0
	.byte	W06
	.byte		N08   , Cs4
	.byte	W08
	.byte		        As3
	.byte	W08
	.byte		N24   , Gs3
	.byte	W24
	.byte		N04   , Fs3
	.byte	W04
	.byte		        Gs3
	.byte	W04
	.byte		N16   , As3
	.byte	W16
	.byte		N07   , Gs3
	.byte	W07
	.byte		N02   , Gs3 , v076
	.byte	W02
	.byte		        Gs3 , v072
	.byte	W03
	.byte		        Gs3 , v060
	.byte	W02
	.byte		        Gs3 , v048
	.byte	W03
	.byte		        Gs3 , v040
	.byte	W02
	.byte		        Gs3 , v036
	.byte	W02
	.byte	N02
	.byte	W02
	.byte	W01
	.byte		N07   , Fs3 , v100
	.byte	W07
	.byte		N02   , Fs3 , v076
	.byte	W02
	.byte		        Fs3 , v072
	.byte	W03
	.byte		        Fs3 , v060
	.byte	W02
	.byte		        Fs3 , v048
	.byte	W03
	.byte		        Fs3 , v040
	.byte	W02
	.byte		        Fs3 , v036
	.byte	W02
	.byte	N02
	.byte	W03
	.byte		N07   , Gs3 , v100
	.byte	W07
	.byte		N02   , Gs3 , v076
	.byte	W02
	.byte		        Gs3 , v072
	.byte	W03
	.byte		        Gs3 , v060
	.byte	W02
	.byte		        Gs3 , v048
	.byte	W03
	.byte		        Gs3 , v040
	.byte	W02
	.byte		        Gs3 , v036
	.byte	W02
	.byte	N02
	.byte	W03
	.byte		N07   , As3 , v100
	.byte	W07
	.byte		N02   , As3 , v076
	.byte	W02
	.byte		        As3 , v072
	.byte	W03
	.byte		        As3 , v060
	.byte	W02
	.byte		        As3 , v048
	.byte	W03
	.byte		        As3 , v040
	.byte	W02
	.byte		        As3 , v036
	.byte	W02
	.byte	N02
	.byte	W03
	.byte		        As3 , v044
	.byte	W02
	.byte		        As3 , v048
	.byte	W03
	.byte		        As3 , v056
	.byte	W02
	.byte		        As3 , v064
	.byte	W02
	.byte		        As3 , v076
	.byte	W03
	.byte		        As3 , v084
	.byte	W02
	.byte		        As3 , v100
	.byte	W03
	.byte		MOD   , 2
	.byte		VOL   , 45*mus_qz_616_mvl/mxv
	.byte		N02   , Fn2 , v088
	.byte	W03
	.byte		        Fs2 , v084
	.byte	W03
	.byte		BEND  , c_v-16
	.byte	W01
	.byte		N36   , Cs3 , v092 , gtp1
	.byte	W04
	.byte		BEND  , c_v+0
	.byte	W01
	.byte		        c_v+1
	.byte	W30
	.byte	W01
	.byte		N11   , Ds3 , v080
	.byte	W12
	.byte		N07   , Bn2 , v100
	.byte	W07
	.byte		N02   , Bn2 , v068
	.byte	W02
	.byte		        Bn2 , v056
	.byte	W03
	.byte	N02
	.byte	W02
	.byte	N02
	.byte	W03
	.byte		        Bn2 , v048
	.byte	W02
	.byte	N02
	.byte	W02
	.byte	N02
	.byte	W03
	.byte	N02
	.byte	W02
	.byte	N02
	.byte	W03
	.byte		        Bn2 , v052
	.byte	W02
	.byte		        Bn2 , v056
	.byte	W02
	.byte	N02
	.byte	W03
	.byte		        Bn2 , v060
	.byte	W02
	.byte	N02
	.byte	W03
	.byte		        Bn2 , v068
	.byte	W02
	.byte		        Bn2 , v076
	.byte	W02
	.byte		        Bn2 , v072
	.byte	W02
	.byte	W01
	.byte		N07   , En3 , v100
	.byte	W07
	.byte		N02   , En3 , v052
	.byte	W02
	.byte		        En3 , v048
	.byte	W03
	.byte		        En3 , v044
	.byte	W02
	.byte		        En3 , v036
	.byte	W03
	.byte		        En3 , v032
	.byte	W02
	.byte	N02
	.byte	W02
	.byte		        En3 , v040
	.byte	W03
	.byte		        En3 , v044
	.byte	W02
	.byte		        En3 , v048
	.byte	W03
	.byte		        En3 , v052
	.byte	W02
	.byte		        En3 , v064
	.byte	W02
	.byte		        En3 , v072
	.byte	W02
	.byte		BEND  , c_v-23
	.byte	W01
	.byte		N23   , Ds3 , v100
	.byte	W05
	.byte		BEND  , c_v+1
	.byte		        c_v+0
	.byte	W19
	.byte		N02   , Ds3 , v072
	.byte	W02
	.byte		        Ds3 , v052
	.byte	W03
	.byte		        Ds3 , v048
	.byte	W02
	.byte		        Ds3 , v044
	.byte	W02
	.byte		        Ds3 , v032
	.byte	W03
	.byte		N11   , En3 , v100
	.byte	W11
	.byte		BEND  , c_v-14
	.byte	W01
	.byte		N11   , Fs3 , v076
	.byte	W01
	.byte		BEND  , c_v+4
	.byte	W10
	.byte		        c_v-16
	.byte	W01
	.byte		N11   , Ds3 , v100
	.byte	W01
	.byte		BEND  , c_v+4
	.byte		        c_v+0
	.byte	W04
	.byte		        c_v+1
	.byte		        c_v+0
	.byte	W07
	.byte		N05   , Bn2
	.byte	W12
	.byte	N11
	.byte	W12
	.byte		        Gn2
	.byte	W12
	.byte		N07   , Bn2
	.byte	W07
	.byte		N02   , Bn2 , v052
	.byte	W02
	.byte		        Bn2 , v048
	.byte	W03
	.byte		        Bn2 , v044
	.byte	W02
	.byte		        Bn2 , v036
	.byte	W03
	.byte		        Bn2 , v032
	.byte	W02
	.byte	N02
	.byte	W02
	.byte		        Bn2 , v040
	.byte	W02
	.byte		BEND  , c_v-12
	.byte	W01
	.byte		N23   , Cs3 , v100
	.byte	W05
	.byte		BEND  , c_v+1
	.byte		        c_v+0
	.byte	W18
	.byte	W01
	.byte		N11   , Ds3
	.byte	W11
	.byte		BEND  , c_v-21
	.byte	W01
	.byte		N11   , Fs3
	.byte	W01
	.byte		BEND  , c_v+1
	.byte	W11
	.byte		N11   , Ds3
	.byte	W12
	.byte		N07   , Cs3
	.byte	W07
	.byte		N02   , Cs3 , v076
	.byte	W02
	.byte		        Cs3 , v056
	.byte	W03
	.byte		        Cs3 , v048
	.byte	W02
	.byte		        Cs3 , v044
	.byte	W03
	.byte	N02
	.byte	W02
	.byte		        Cs3 , v048
	.byte	W02
	.byte	N02
	.byte	W03
	.byte		        Cs3 , v052
	.byte	W02
	.byte		        Cs3 , v056
	.byte	W03
	.byte	N02
	.byte	W02
	.byte		        Cs3 , v064
	.byte	W02
	.byte		        Cs3 , v068
	.byte	W03
	.byte		        Dn3 , v076
	.byte	W02
	.byte		        Dn3 , v080
	.byte	W03
	.byte		        Dn3 , v084
	.byte	W02
	.byte		        Dn3 , v088
	.byte	W02
	.byte		        Dn3 , v048
	.byte	W02
	.byte		VOL   , 36*mus_qz_616_mvl/mxv
	.byte	W01
	.byte		N05   , As3 , v100
	.byte	W06
	.byte		        Bn3
	.byte	W05
	.byte		BEND  , c_v-19
	.byte	W01
	.byte		N11   , Cs4
	.byte	W05
	.byte		BEND  , c_v+0
	.byte	W07
	.byte		N02   , Cs4 , v056
	.byte	W02
	.byte		        Cs4 , v048
	.byte	W03
	.byte		        Cs4 , v044
	.byte	W02
	.byte	N02
	.byte	W02
	.byte	N02
	.byte	W03
	.byte		        Cs4 , v052
	.byte	W02
	.byte		        Cs4 , v060
	.byte	W03
	.byte		        Cs4 , v068
	.byte	W02
	.byte		        Cs4 , v076
	.byte	W02
	.byte		        Cs4 , v084
	.byte	W03
	.byte		N11   , Bn3 , v100
	.byte	W12
	.byte		N05   , Cs4
	.byte	W06
	.byte		        Ds4
	.byte	W06
	.byte		N32   , Bn3 , v080 , gtp3
	.byte	W32
	.byte	W03
	.byte		BEND  , c_v-19
	.byte	W01
	.byte		N11   , En4 , v100
	.byte	W02
	.byte		BEND  , c_v+0
	.byte	W10
	.byte		N11   , Gs3 , v060
	.byte	W12
	.byte		        Fs4 , v100
	.byte	W12
	.byte		N07   , Ds4
	.byte	W07
	.byte		N02   , Ds4 , v052
	.byte	W02
	.byte		        Ds4 , v048
	.byte	W03
	.byte		        Ds4 , v044
	.byte	W02
	.byte		        Ds4 , v036
	.byte	W03
	.byte		        Ds4 , v032
	.byte	W02
	.byte		        Ds4 , v028
	.byte	W02
	.byte		        Ds4 , v024
	.byte	W03
	.byte	N02
	.byte	W02
	.byte	N02
	.byte	W03
	.byte		        Ds4 , v028
	.byte	W02
	.byte	N02
	.byte	W02
	.byte		        Ds4 , v032
	.byte	W03
	.byte		        Ds4 , v024
	.byte	W02
	.byte	N02
	.byte	W03
	.byte		        Ds4 , v028
	.byte	W02
	.byte	N02
	.byte	W02
	.byte		        Ds4 , v032
	.byte	W03
	.byte		N03   , Ds4 , v100
	.byte	W04
	.byte		        En4
	.byte	W04
	.byte		        Fs4
	.byte	W03
	.byte	W01
	.byte		N05   , Gs4
	.byte	W06
	.byte		        Ds4
	.byte	W06
	.byte	N05
	.byte	W06
	.byte		        Bn3
	.byte	W06
	.byte	N23
	.byte	W24
	.byte		N02   , Bn3 , v016
	.byte	W02
	.byte		        Bn3 , v020
	.byte	W03
	.byte		        Bn3 , v024
	.byte	W02
	.byte	N02
	.byte	W02
	.byte		        Bn3 , v028
	.byte	W03
	.byte		        Bn3 , v032
	.byte	W02
	.byte		        Bn3 , v040
	.byte	W03
	.byte		        Bn3 , v048
	.byte	W02
	.byte	N02
	.byte	W02
	.byte		        Bn3 , v064
	.byte	W03
	.byte		N05   , Fs3 , v084
	.byte	W06
	.byte		        Gs3 , v100
	.byte	W06
	.byte		        Bn3
	.byte	W06
	.byte		        Cs4
	.byte	W05
	.byte		BEND  , c_v-19
	.byte	W01
	.byte		N11   , Ds4
	.byte	W05
	.byte		BEND  , c_v+0
	.byte	W07
	.byte		N02   , Ds4 , v048
	.byte	W02
	.byte		        Ds4 , v044
	.byte	W03
	.byte	N02
	.byte	W02
	.byte	N02
	.byte	W02
	.byte		        Ds4 , v052
	.byte	W03
	.byte		        Ds4 , v056
	.byte	W02
	.byte		        Ds4 , v060
	.byte	W03
	.byte		        Ds4 , v068
	.byte	W02
	.byte		        Ds4 , v072
	.byte	W02
	.byte		        Ds4 , v048
	.byte	W03
	.byte		N05   , Fs4 , v100
	.byte	W06
	.byte		N02   , Fs4 , v056
	.byte	W02
	.byte		        Fs4 , v052
	.byte	W03
	.byte		BEND  , c_v-19
	.byte		N22   , Gs4 , v127
	.byte	W03
	.byte		BEND  , c_v+0
	.byte	W19
	.byte		N07   , Fs4 , v100
	.byte	W07
	.byte		N02   , Fs4 , v060
	.byte	W02
	.byte		        Fs4 , v076
	.byte	W03
	.byte		        Fs4 , v064
	.byte	W02
	.byte		        Fs4 , v060
	.byte	W03
	.byte	N02
	.byte	W02
	.byte		        Fs4 , v064
	.byte	W03
	.byte		VOICE , 34
	.byte	W01
	.byte		N02   , En4 , v080
	.byte	W01
	.byte		        Fs4 , v060
	.byte	W02
	.byte		PAN   , c_v+39
	.byte		MOD   , 0
	.byte		VOL   , 38*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		N66   , Gs4 , v108 , gtp1
	.byte	W09
	.byte		VOL   , 38*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        34*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        25*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        19*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        15*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        12*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        12*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        10*mus_qz_616_mvl/mxv
	.byte	W06
	.byte		        13*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        16*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        21*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		MOD   , 2
	.byte		VOL   , 25*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        33*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        36*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        43*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        49*mus_qz_616_mvl/mxv
	.byte	W12
	.byte		        34*mus_qz_616_mvl/mxv
	.byte		MOD   , 0
	.byte		BEND  , c_v+2
	.byte	W03
	.byte		N07   , Gs3 , v116
	.byte	W08
	.byte		        Bn3
	.byte	W08
	.byte		        Ds4
	.byte	W05
	.byte	W03
	.byte		N15   , Cs4
	.byte	W16
	.byte		N03   , Ds4
	.byte	W04
	.byte		        Cs4
	.byte	W04
	.byte		N23   , Bn3
	.byte	W24
	.byte		N15   , As3
	.byte	W16
	.byte		N07   , Fs3
	.byte	W08
	.byte		        As3
	.byte	W08
	.byte		        Bn3
	.byte	W08
	.byte		        Gs4
	.byte	W05
	.byte	W03
	.byte		N66   , Fs4 , v116 , gtp1
	.byte	W17
	.byte		VOL   , 22*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        13*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        8*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        5*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        7*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        9*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        13*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        15*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        19*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        24*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        29*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        36*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        36*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        34*mus_qz_616_mvl/mxv
	.byte	W03
	.byte	N07
	.byte	W08
	.byte		        En4
	.byte	W08
	.byte		        Ds4
	.byte	W05
	.byte	W03
	.byte		N15   , Cs4
	.byte	W16
	.byte		N03   , Ds4
	.byte	W04
	.byte		        Dn4
	.byte	W04
	.byte		N15   , Cs4
	.byte	W16
	.byte		N07   , Cn4
	.byte	W08
	.byte		N15   , Bn3
	.byte	W16
	.byte		N07   , Gs3
	.byte	W08
	.byte		        As3
	.byte	W08
	.byte		        Bn3
	.byte	W08
	.byte		        Fs3
	.byte	W05
	.byte	W03
	.byte		N66   , Gs3 , v116 , gtp1
	.byte	W05
	.byte		VOL   , 24*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        16*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        13*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        8*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		MOD   , 2
	.byte	W20
	.byte		VOL   , 13*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        16*mus_qz_616_mvl/mxv
	.byte	W01
	.byte		MOD   , 6
	.byte	W03
	.byte		VOL   , 21*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        28*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        32*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        33*mus_qz_616_mvl/mxv
	.byte	W05
	.byte		        39*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		MOD   , 0
	.byte		VOL   , 34*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		N07   , En4
	.byte	W08
	.byte		        Fn4
	.byte	W08
	.byte		        Fs4
	.byte	W05
	.byte	W03
	.byte		N15   , Ds4
	.byte	W16
	.byte		N03   , En4
	.byte	W04
	.byte		        Ds4
	.byte	W04
	.byte		N23   , Cs4
	.byte	W24
	.byte		N15   , Bn3
	.byte	W16
	.byte		N07   , As3
	.byte	W08
	.byte		        Bn3
	.byte	W08
	.byte		        Cn4
	.byte	W08
	.byte		        Cs4
	.byte	W05
	.byte	W03
	.byte	N15
	.byte	W16
	.byte		N78   , Ds4 , v116 , gtp1
	.byte	W09
	.byte		VOL   , 24*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        17*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        13*mus_qz_616_mvl/mxv
	.byte	W10
	.byte		        13*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        14*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        17*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        19*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        21*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        24*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        25*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        29*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        31*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		MOD   , 4
	.byte		VOL   , 34*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        37*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        39*mus_qz_616_mvl/mxv
	.byte	W19
	.byte		MOD   , 0
	.byte		VOL   , 33*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		N15   , Cs4
	.byte	W16
	.byte		N03   , Ds4
	.byte	W04
	.byte		        Cs4
	.byte	W04
	.byte		N23   , Cn4
	.byte	W24
	.byte		N15   , En4
	.byte	W16
	.byte		N07   , Gs3 , v104
	.byte	W05
	.byte		VOL   , 29*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		VOICE , 36
	.byte		N07   , En4 , v127
	.byte	W08
	.byte		        Fn4
	.byte	W08
	.byte		N03   , Fs4
	.byte	W04
	.byte		        Gn4
	.byte	W01
	.byte	W03
	.byte		N30   , Gs4 , v127 , gtp1
	.byte	W13
	.byte		MOD   , 2
	.byte	W24
	.byte		        0
	.byte	W03
	.byte		N05   , Ds4
	.byte	W08
	.byte		N02   , Dn4 , v100
	.byte	W03
	.byte		N17   , Ds4
	.byte	W21
	.byte		N07   , Ds4 , v127
	.byte	W08
	.byte		        Gs4
	.byte	W08
	.byte		        Ds4
	.byte	W05
	.byte	W03
	.byte		N19   , Cs4
	.byte	W13
	.byte		MOD   , 3
	.byte	W07
	.byte		N01   , Ds4
	.byte	W02
	.byte		        Cs4
	.byte	W02
	.byte		N23   , Bn3
	.byte	W01
	.byte		MOD   , 0
	.byte	W23
	.byte		N15   , As3
	.byte	W16
	.byte		N07   , Gn3
	.byte	W08
	.byte		        As3
	.byte	W08
	.byte		        Cs4
	.byte	W08
	.byte		        Gs4
	.byte	W05
	.byte	W03
	.byte		N15   , Fs4
	.byte	W16
	.byte		N07   , Cn4
	.byte	W08
	.byte		N44   , Cs4 , v127 , gtp3
	.byte	W13
	.byte		MOD   , 2
	.byte	W32
	.byte	W03
	.byte	N07
	.byte	W01
	.byte		MOD   , 0
	.byte	W07
	.byte		N07   , Fs4
	.byte	W08
	.byte		        An4
	.byte	W05
	.byte	W03
	.byte		N36   , As4 , v127 , gtp3
	.byte	W17
	.byte		MOD   , 2
	.byte	W23
	.byte		N07   , Bn4
	.byte	W01
	.byte		MOD   , 0
	.byte	W07
	.byte		N07   , Gs4
	.byte	W08
	.byte		        Fs4
	.byte	W08
	.byte		        Gn4
	.byte	W08
	.byte		N15   , Gs4
	.byte	W16
	.byte		N07   , Gs3
	.byte	W05
	.byte	W03
	.byte		N68   , En4 , v127 , gtp3
	.byte	W72
	.byte	N07
	.byte	W08
	.byte		        Fn4
	.byte	W08
	.byte		        Fs4
	.byte	W05
	.byte	W03
	.byte		N19   , Ds4
	.byte	W20
	.byte		N01   , En4
	.byte	W02
	.byte		        Ds4
	.byte	W02
	.byte		N23   , Cs4
	.byte	W24
	.byte		N15   , Bn3
	.byte	W16
	.byte		N07   , As3
	.byte	W08
	.byte		        Bn3
	.byte	W08
	.byte		        Cn4
	.byte	W08
	.byte		        Cs4
	.byte	W05
	.byte	W03
	.byte	N15
	.byte	W16
	.byte		TIE   , Ds4
	.byte	W76
	.byte	W01
	.byte	W24
	.byte	W02
	.byte	EOT
	.byte	W22
	.byte		BEND  , c_v+0
	.byte	W24
	.byte		VOICE , 42
	.byte		N03   , As4 , v100
	.byte	W04
	.byte		N02   , As4 , v060
	.byte	W04
	.byte		N03   , Bn4 , v100
	.byte	W04
	.byte		N02   , Bn4 , v060
	.byte	W04
	.byte		N03   , Cs5 , v100
	.byte	W04
	.byte		N02   , Cs5 , v060
	.byte	W04
	.byte	GOTO
	 ptrvalue	mus_qz_616_5_B1
	.byte	FINE

/********************** Track  6 **********************/

mus_qz_616_6:
	.byte		KEYSH , mus_qz_616_key+0
mus_qz_616_6_B1:
	.byte		VOICE , 23
	.byte		PAN   , c_v-57
	.byte		        c_v+0
	.byte		PRIO  , 62
	.byte		        c_v-32
	.byte		VOL   , 5*mus_qz_616_mvl/mxv
	.byte		N44   , Ds3 , v096 , gtp3
	.byte	W03
	.byte		VOL   , 10*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        11*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        13*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        14*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        18*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        21*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        22*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        25*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        29*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        34*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        35*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        38*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        39*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        40*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        42*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        5*mus_qz_616_mvl/mxv
	.byte		N48   , Ds4 , v076
	.byte	W03
	.byte		VOL   , 10*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        11*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        13*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        14*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        18*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        21*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        22*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        25*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        29*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        34*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        35*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        38*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        39*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        40*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        42*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        5*mus_qz_616_mvl/mxv
	.byte		N44   , En3 , v108 , gtp3
	.byte	W03
	.byte		VOL   , 10*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        11*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        13*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        14*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        18*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        21*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        22*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        25*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        29*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        34*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        35*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        38*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        39*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        40*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        42*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        5*mus_qz_616_mvl/mxv
	.byte		N48   , Gs4 , v072
	.byte	W03
	.byte		VOL   , 10*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        11*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        13*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        14*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        18*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        21*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        22*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        25*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        29*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        34*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        35*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        38*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        39*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        40*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        42*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        5*mus_qz_616_mvl/mxv
	.byte		N44   , Ds3 , v127 , gtp3
	.byte	W03
	.byte		VOL   , 10*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        11*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        13*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        14*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        18*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        21*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        22*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        25*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        29*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        34*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        35*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        38*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        39*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        40*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        42*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		N23   , Fs3 , v100
	.byte	W24
	.byte		        Gs3
	.byte	W24
	.byte		N24   , As2 , v127
	.byte	W24
	.byte		        Bn2
	.byte	W24
	.byte		        Cs3
	.byte	W24
	.byte		N23   , En3 , v112
	.byte	W24
	.byte		PAN   , c_v-57
	.byte		VOL   , 29*mus_qz_616_mvl/mxv
	.byte		PAN   , c_v-28
	.byte		N11   , As3 , v100
	.byte	W12
	.byte		        Bn2
	.byte	W12
	.byte		        As3
	.byte	W12
	.byte		N23   , Gs3
	.byte	W24
	.byte		        As3
	.byte	W24
	.byte		N11   , Bn3
	.byte	W12
mus_qz_616_6_000:
	.byte		N11   , En4 , v100
	.byte	W12
	.byte		        Fs3
	.byte	W12
	.byte		        En4
	.byte	W12
	.byte		N23   , Fs4
	.byte	W24
	.byte		        Ds4
	.byte	W24
	.byte		N11   , Cn4
	.byte	W12
	.byte	PEND
	.byte		        Bn3
	.byte	W12
	.byte		        Cs3 , v056
	.byte	W12
	.byte		N05   , Bn3 , v100
	.byte	W12
	.byte	N11
	.byte	W12
	.byte	N23
	.byte	W24
	.byte		        As3
	.byte	W24
	.byte		N11   , Bn3
	.byte	W12
	.byte		        Cs3 , v056
	.byte	W12
	.byte		N05   , Bn3 , v100
	.byte	W12
	.byte		N11   , Cs4
	.byte	W12
	.byte	N23
	.byte	W24
	.byte		        Dn4
	.byte	W24
	.byte		N11   , As3
	.byte	W12
	.byte		        Bn2
	.byte	W12
	.byte		        As3
	.byte	W12
	.byte		N23   , Gs3
	.byte	W24
	.byte		        As3
	.byte	W24
	.byte		N11   , Bn3
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_616_6_000
	.byte		N11   , Bn3 , v100
	.byte	W12
	.byte		        Cs3
	.byte	W12
	.byte		N05   , Bn3
	.byte	W12
	.byte	N11
	.byte	W12
	.byte	N23
	.byte	W24
	.byte		        As3
	.byte	W24
	.byte		N11   , Bn3
	.byte	W12
	.byte		        Cs3
	.byte	W12
	.byte		N05   , Bn3
	.byte	W12
	.byte		N11   , Cs4
	.byte	W12
	.byte	N23
	.byte	W24
	.byte		        Dn4
	.byte	W24
	.byte		PAN   , c_v-41
	.byte		VOL   , 36*mus_qz_616_mvl/mxv
	.byte		N07   , Ds4
	.byte	W08
	.byte		N03   , En3
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , Ds4
	.byte	W08
	.byte		N03   , En3
	.byte	W08
	.byte		N07   , Gs4 , v076
	.byte	W08
	.byte		N03   , En3 , v100
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , Ds4
	.byte	W08
	.byte		N03   , En3
	.byte	W08
	.byte		N07   , Cs4
	.byte	W08
	.byte		N03   , En3
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , Cs4
	.byte	W08
	.byte		N03   , En3
	.byte	W08
	.byte		N07   , Fs4
	.byte	W08
	.byte		N03   , En3
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , Ds4
	.byte	W08
	.byte		        Dn4
	.byte	W08
	.byte		        Cs4
	.byte	W08
	.byte		N03   , Ds3
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , Cs4
	.byte	W08
	.byte		N03   , Ds3
	.byte	W08
	.byte		N07   , Fs4 , v076
	.byte	W08
	.byte		N03   , Ds3 , v100
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , Cs4
	.byte	W08
	.byte		        Dn4
	.byte	W08
	.byte		        Ds4
	.byte	W08
	.byte		N03   , Ds3
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , Ds4
	.byte	W08
	.byte		N03   , Ds3
	.byte	W08
	.byte		N07   , Gs4 , v076
	.byte	W08
	.byte		N03   , Ds3 , v100
	.byte	W08
	.byte		N07   , Bn3
	.byte	W08
	.byte		N03   , Ds3
	.byte	W08
	.byte		N07   , Ds4
	.byte	W08
	.byte		        Gs4
	.byte	W08
	.byte		        Cs4
	.byte	W08
	.byte		N03   , Cs3
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , Bn3
	.byte	W08
	.byte		N03   , Cs3
	.byte	W08
	.byte		N07   , En4 , v076
	.byte	W08
	.byte		N03   , Cs3 , v100
	.byte	W08
	.byte		N07   , Bn3 , v076
	.byte	W08
	.byte		N03   , Cs3 , v100
	.byte	W08
	.byte		N07   , Bn3
	.byte	W08
	.byte		N03   , Cs3
	.byte		N07   , En4
	.byte	W08
	.byte		        As3
	.byte	W08
	.byte		N03   , Cs3
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , As3
	.byte	W08
	.byte		N03   , Cs3
	.byte	W08
	.byte		N07   , Fs4 , v076
	.byte	W08
	.byte		N03   , Cs3 , v100
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , As3
	.byte	W08
	.byte		        An3
	.byte	W08
	.byte		        As3
	.byte	W08
	.byte		N03   , Bn2
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , As3
	.byte	W08
	.byte		N03   , Bn2
	.byte	W08
	.byte		N07   , Fs4 , v076
	.byte	W08
	.byte		N03   , Bn2 , v100
	.byte	W08
	.byte		N07   , Fs3
	.byte	W08
	.byte		N03   , Bn2
	.byte	W08
	.byte		N07   , As3
	.byte	W08
	.byte		        Fs4
	.byte	W08
	.byte		N23   , An3
	.byte	W24
	.byte		        Fs3
	.byte	W24
	.byte		        Cs4
	.byte	W24
	.byte		        Dn4
	.byte	W24
	.byte		VOL   , 40*mus_qz_616_mvl/mxv
	.byte		N07   , Ds4
	.byte	W08
	.byte		N03   , En3
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , Ds4
	.byte	W08
	.byte		N03   , En3
	.byte	W08
	.byte		N07   , Gs4 , v076
	.byte	W08
	.byte		N03   , En3 , v100
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , Ds4
	.byte	W08
	.byte		        Cn4
	.byte	W08
	.byte		        Cs4
	.byte	W08
	.byte		N03   , En3
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , Cs4
	.byte	W08
	.byte		N03   , En3
	.byte	W08
	.byte		N07   , Gn4
	.byte	W08
	.byte		N03   , En3
	.byte	W08
	.byte		N07   , As3
	.byte	W08
	.byte		N03   , En3
	.byte	W08
	.byte		N07   , En4
	.byte	W08
	.byte		        Ds4
	.byte	W08
	.byte		        Cs4
	.byte	W08
	.byte		N03   , Ds3
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , Cs4
	.byte	W08
	.byte		N03   , Ds3
	.byte	W08
	.byte		N07   , Fs4 , v076
	.byte	W08
	.byte		N03   , Ds3 , v100
	.byte	W08
	.byte		N07   , As3
	.byte	W08
	.byte		N03   , Ds3
	.byte	W08
	.byte		N07   , Cs4
	.byte	W08
	.byte		        Dn4
	.byte	W08
	.byte		        Ds4
	.byte	W08
	.byte		N03   , Ds3
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , Ds4
	.byte	W08
	.byte		N03   , Ds3
	.byte	W08
	.byte		N07   , Gs4 , v076
	.byte	W08
	.byte		N03   , Ds3 , v100
	.byte	W08
	.byte		N07   , Gs3
	.byte	W08
	.byte		        Ds3
	.byte	W08
	.byte		        Ds4
	.byte	W08
	.byte		        Dn4
	.byte	W08
	.byte		        Cs4
	.byte	W08
	.byte		N03   , Cs3
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , Bn3
	.byte	W08
	.byte		N03   , Cs3
	.byte	W08
	.byte		N07   , En4 , v076
	.byte	W08
	.byte		N03   , Cs3 , v100
	.byte	W08
	.byte		N07   , Gs3
	.byte	W08
	.byte		N03   , Cs3
	.byte	W08
	.byte		N07   , Bn3
	.byte	W08
	.byte		        An3
	.byte	W08
	.byte		        As3
	.byte	W08
	.byte		N03   , Cs3
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , As3
	.byte	W08
	.byte		N03   , Cs3
	.byte	W08
	.byte		N07   , Fs4 , v076
	.byte	W08
	.byte		N03   , Cs3 , v100
	.byte	W08
	.byte		N07   , As3
	.byte	W08
	.byte		N03   , Cs3
	.byte	W08
	.byte		N07   , As3
	.byte	W08
	.byte		        An3
	.byte	W08
	.byte		        Gs3
	.byte	W08
	.byte		N03   , Bn2
	.byte	W08
	.byte	N03
	.byte	W08
	.byte	N03
	.byte	W08
	.byte		N07   , As3
	.byte	W08
	.byte		N03   , Bn2
	.byte	W08
	.byte		N07   , Fs4 , v076
	.byte	W08
	.byte		N03   , Bn2 , v100
	.byte	W08
	.byte		N07   , As3
	.byte	W08
	.byte		N03   , Bn2
	.byte	W08
	.byte		N07   , Cs4
	.byte	W08
	.byte		        Dn4
	.byte	W08
	.byte		N15   , Ds4
	.byte	W16
	.byte		N03   , En4
	.byte	W04
	.byte		        Ds4
	.byte	W04
	.byte		N23   , Bn3
	.byte	W24
	.byte	N15
	.byte	W16
	.byte		N07   , Gs3
	.byte	W08
	.byte		        As3
	.byte	W08
	.byte		        Bn3
	.byte	W08
	.byte		        Cs4
	.byte	W08
	.byte	GOTO
	 ptrvalue	mus_qz_616_6_B1
	.byte	FINE

/********************** Track  7 **********************/

mus_qz_616_7:
	.byte		KEYSH , mus_qz_616_key+0
mus_qz_616_7_B1:
	.byte		VOICE , 19
	.byte		VOL   , 50*mus_qz_616_mvl/mxv
	.byte		PAN   , c_v-30
	.byte		        c_v+0
	.byte		BENDR , 6
	.byte		PRIO  , 64
	.byte		PAN   , c_v-48
	.byte		N12   , Bn0 , v104
	.byte	W12
	.byte		        Fs1
	.byte	W12
	.byte		        Cs2
	.byte	W12
	.byte		N60   , Ds2
	.byte	W03
	.byte		BEND  , c_v+11
	.byte	W01
	.byte		        c_v-2
	.byte	W02
	.byte		        c_v+0
	.byte	W54
	.byte		N12   , Bn0
	.byte	W12
	.byte		        Gs1
	.byte	W12
	.byte		        Ds2
	.byte	W12
	.byte		N60   , En2
	.byte	W03
	.byte		BEND  , c_v+11
	.byte	W01
	.byte		        c_v-2
	.byte	W02
	.byte		        c_v+0
	.byte	W54
	.byte		N12   , Bn0
	.byte	W12
	.byte		        Fs1
	.byte	W12
	.byte		        Cs2
	.byte	W12
	.byte		        Ds2
	.byte	W12
	.byte		N24   , Fs2
	.byte	W24
	.byte		N23   , Gs2
	.byte	W24
	.byte		PAN   , c_v-16
	.byte		N24   , Fs1
	.byte	W24
	.byte		        Fn1
	.byte	W24
	.byte		        En1
	.byte	W24
	.byte		        Cs1
	.byte	W24
	.byte		PAN   , c_v-16
	.byte		VOL   , 54*mus_qz_616_mvl/mxv
	.byte		PAN   , c_v-14
	.byte		N32   , Bn0 , v127 , gtp3
	.byte	W36
	.byte	N05
	.byte	W12
	.byte	N23
	.byte	W36
	.byte		N11   , Fs1
	.byte	W12
mus_qz_616_7_000:
	.byte		N32   , Cn1 , v127 , gtp3
	.byte	W36
	.byte		N23   , Gs1
	.byte	W24
	.byte		        Ds1
	.byte	W24
	.byte		N11   , Cn1
	.byte	W12
	.byte	PEND
mus_qz_616_7_001:
	.byte		N32   , Cs1 , v127 , gtp3
	.byte	W36
	.byte		N11   , Gn0
	.byte	W12
	.byte		N23   , Fs1
	.byte	W24
	.byte		N11   , En1
	.byte	W12
	.byte		        Cs1
	.byte	W12
	.byte	PEND
	.byte		N23   , Fs0
	.byte	W24
	.byte		N11   , Fs1
	.byte	W12
	.byte		N23   , As0
	.byte	W24
	.byte		N11   , En1
	.byte	W12
	.byte		N23   , Fs0
	.byte	W24
	.byte		N32   , Bn0 , v127 , gtp3
	.byte	W36
	.byte	N05
	.byte	W12
	.byte	N23
	.byte	W36
	.byte		N11   , Fs1
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_616_7_000
	.byte	PATT
	 ptrvalue	mus_qz_616_7_001
	.byte		N23   , Fs0 , v127
	.byte	W24
	.byte		N11   , Fs1
	.byte	W12
	.byte		N23   , Fs0
	.byte	W24
	.byte		N11   , Fs1
	.byte	W12
	.byte		N09   , Fs0
	.byte	W12
	.byte		N11   , Fn1
	.byte	W12
mus_qz_616_7_002:
	.byte		N68   , En1 , v127 , gtp3
	.byte	W72
	.byte	N07
	.byte	W08
	.byte		        Ds2
	.byte	W08
	.byte		        Bn1
	.byte	W08
	.byte	PEND
	.byte		N44   , En1 , v127 , gtp3
	.byte	W48
	.byte		N07   , Fs2
	.byte	W08
	.byte		        En2
	.byte	W08
	.byte		        Ds2
	.byte	W08
	.byte		        En1
	.byte	W08
	.byte		        Fn1
	.byte	W08
	.byte		        Fs1
	.byte	W08
	.byte		        As1
	.byte	W08
	.byte		        Fn1
	.byte	W08
	.byte		N54   , Fs1 , v127 , gtp1
	.byte	W56
	.byte	N07
	.byte	W08
	.byte		        Gs1
	.byte	W08
	.byte		        Bn1
	.byte	W08
	.byte		        Ds2
	.byte	W08
	.byte		        Gs1
	.byte	W08
	.byte		N23   , Bn1
	.byte	W24
	.byte		N07   , Ds1
	.byte	W08
	.byte		N15   , Ds2
	.byte	W16
	.byte		N07   , Gs1
	.byte	W08
	.byte		        As1
	.byte	W08
	.byte		        Bn1
	.byte	W08
	.byte		        As1
	.byte	W08
mus_qz_616_7_003:
	.byte		N23   , Cs1 , v127
	.byte	W24
	.byte		N07   , En1
	.byte	W08
	.byte		        Ds1
	.byte	W08
	.byte		        Dn1
	.byte	W08
	.byte		N15   , Cs1
	.byte	W16
	.byte		        Gs0
	.byte	W16
	.byte		N07   , Fs1
	.byte	W08
	.byte		        Fn1
	.byte	W08
	.byte	PEND
mus_qz_616_7_004:
	.byte		N15   , En1 , v127
	.byte	W16
	.byte		        As0
	.byte	W16
	.byte		        Fs1
	.byte	W16
	.byte		N07   , Fn1
	.byte	W08
	.byte		        En1
	.byte	W08
	.byte		        Ds1
	.byte	W08
	.byte		N15   , Cs1
	.byte	W16
	.byte		N07   , Cn1
	.byte	W08
	.byte	PEND
mus_qz_616_7_005:
	.byte		N15   , Bn0 , v127
	.byte	W16
	.byte		        Fs0
	.byte	W16
	.byte		N23   , Fs1
	.byte	W24
	.byte		N07   , En1
	.byte	W08
	.byte		        Ds1
	.byte	W08
	.byte		        Cs1
	.byte	W08
	.byte		        Bn0
	.byte	W08
	.byte		        Gs0
	.byte	W08
	.byte	PEND
	.byte		        Fs0
	.byte	W08
	.byte		        Fs1
	.byte	W08
	.byte		N03   , Fs0
	.byte	W08
	.byte	N23
	.byte	W24
	.byte		        En1
	.byte	W24
	.byte		N15   , Fs1
	.byte	W16
	.byte		N07   , Fn1
	.byte	W08
	.byte	PATT
	 ptrvalue	mus_qz_616_7_002
	.byte		N44   , En1 , v127 , gtp3
	.byte	W48
	.byte		N07   , Gn2
	.byte	W08
	.byte		        En2
	.byte	W08
	.byte		        Bn1
	.byte	W08
	.byte		        As1
	.byte	W08
	.byte		        Gn1
	.byte	W08
	.byte		        En1
	.byte	W08
	.byte		        Ds1
	.byte	W08
	.byte		        As1
	.byte	W08
	.byte		        Cs1
	.byte	W08
	.byte		N44   , Ds1 , v127 , gtp3
	.byte	W48
	.byte		N07   , Fs1
	.byte	W08
	.byte		        Gs1
	.byte	W08
	.byte		        Bn1
	.byte	W08
	.byte		        Ds2
	.byte	W08
	.byte		        Gs1
	.byte	W08
	.byte		N23   , Bn1
	.byte	W24
	.byte		N07   , Ds1
	.byte	W08
	.byte		N15   , Ds2
	.byte	W16
	.byte		N07   , Dn2 , v100
	.byte	W08
	.byte		        Bn1 , v127
	.byte	W08
	.byte		        Fs1
	.byte	W08
	.byte		        Gs1
	.byte	W08
	.byte	PATT
	 ptrvalue	mus_qz_616_7_003
	.byte	PATT
	 ptrvalue	mus_qz_616_7_004
	.byte	PATT
	 ptrvalue	mus_qz_616_7_005
	.byte		N07   , En1 , v100
	.byte	W08
	.byte		        Bn0
	.byte	W08
	.byte		        Gs0
	.byte	W08
	.byte		N68   , Bn0 , v100 , gtp1
	.byte	W72
	.byte	GOTO
	 ptrvalue	mus_qz_616_7_B1
	.byte	FINE

/********************** Track  8 **********************/

mus_qz_616_8:
	.byte		KEYSH , mus_qz_616_key+0
mus_qz_616_8_B1:
	.byte		VOICE , 33
	.byte		PAN   , c_v+0
	.byte		PRIO  , 64
	.byte		VOL   , 2*mus_qz_616_mvl/mxv
	.byte		PAN   , c_v+57
	.byte		N44   , As3 , v056 , gtp3
	.byte	W02
	.byte		VOL   , 3*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        5*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        6*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        8*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        10*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        12*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        12*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        14*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        16*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        19*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        25*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        29*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        34*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        38*mus_qz_616_mvl/mxv
	.byte	W07
	.byte		        1*mus_qz_616_mvl/mxv
	.byte		N48   , Cs5 , v052
	.byte	W04
	.byte		VOL   , 2*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        4*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        5*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        7*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        9*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        13*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        18*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        25*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        32*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        46*mus_qz_616_mvl/mxv
	.byte	W08
	.byte		        2*mus_qz_616_mvl/mxv
	.byte		N44   , Gs3 , v052 , gtp3
	.byte	W02
	.byte		VOL   , 3*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        5*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        6*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        8*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        12*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        13*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        16*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        19*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        25*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        27*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        32*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        34*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        38*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        45*mus_qz_616_mvl/mxv
	.byte	W06
	.byte		        1*mus_qz_616_mvl/mxv
	.byte		N48   , Ds5 , v056
	.byte	W04
	.byte		VOL   , 2*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        4*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        5*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        7*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        9*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        13*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        18*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        25*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        32*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        46*mus_qz_616_mvl/mxv
	.byte	W08
	.byte		        6*mus_qz_616_mvl/mxv
	.byte		N44   , Fs3 , v084 , gtp3
	.byte	W02
	.byte		VOL   , 8*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        10*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        12*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        13*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        16*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        19*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        23*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        25*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        32*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        36*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        41*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        43*mus_qz_616_mvl/mxv
	.byte	W12
	.byte		        15*mus_qz_616_mvl/mxv
	.byte		N23   , Cs3 , v076
	.byte	W02
	.byte		VOL   , 20*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        25*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        33*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        38*mus_qz_616_mvl/mxv
	.byte	W02
	.byte		        41*mus_qz_616_mvl/mxv
	.byte	W10
	.byte		        18*mus_qz_616_mvl/mxv
	.byte		N23   , Bn2 , v072
	.byte	W02
	.byte		VOL   , 23*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        29*mus_qz_616_mvl/mxv
	.byte	W03
	.byte		        36*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		        39*mus_qz_616_mvl/mxv
	.byte	W12
	.byte		        46*mus_qz_616_mvl/mxv
	.byte	W96
	.byte		VOICE , 0
	.byte		PAN   , c_v+38
	.byte		VOL   , 32*mus_qz_616_mvl/mxv
	.byte		PAN   , c_v+45
	.byte		N32   , Bn1 , v127 , gtp3
	.byte	W36
	.byte	N05
	.byte	W12
	.byte	N23
	.byte	W36
	.byte		N11   , Fs2
	.byte	W12
mus_qz_616_8_000:
	.byte		N32   , Cn2 , v127 , gtp3
	.byte	W36
	.byte		N23   , Gs2
	.byte	W24
	.byte		        Ds2
	.byte	W24
	.byte		N11   , Cn2
	.byte	W12
	.byte	PEND
mus_qz_616_8_001:
	.byte		N32   , Cs2 , v127 , gtp3
	.byte	W36
	.byte		N11   , Gn1
	.byte	W12
	.byte		N23   , Fs2
	.byte	W24
	.byte		N11   , En2
	.byte	W12
	.byte		        Cs2
	.byte	W12
	.byte	PEND
	.byte		N23   , Fs1
	.byte	W24
	.byte		N11   , Fs2
	.byte	W12
	.byte		N23   , As1
	.byte	W24
	.byte		N11   , En2
	.byte	W12
	.byte		N23   , Fs1
	.byte	W24
	.byte		N32   , Bn1 , v127 , gtp3
	.byte	W36
	.byte	N05
	.byte	W12
	.byte	N23
	.byte	W36
	.byte		N11   , Fs2
	.byte	W12
	.byte	PATT
	 ptrvalue	mus_qz_616_8_000
	.byte	PATT
	 ptrvalue	mus_qz_616_8_001
	.byte		N23   , Fs1 , v127
	.byte	W24
	.byte		N11   , Fs2
	.byte	W12
	.byte		N23   , Fs1
	.byte	W24
	.byte		N11   , Fs2
	.byte	W12
	.byte		VOL   , 54*mus_qz_616_mvl/mxv
	.byte		N11   , Fs1
	.byte	W12
	.byte		        Fn1
	.byte	W12
	.byte		VOICE , 36
	.byte		PAN   , c_v+42
	.byte		VOL   , 42*mus_qz_616_mvl/mxv
	.byte		N68   , En1 , v088 , gtp3
	.byte	W12
	.byte		VOL   , 40*mus_qz_616_mvl/mxv
	.byte	W05
	.byte		        28*mus_qz_616_mvl/mxv
	.byte	W07
	.byte		        18*mus_qz_616_mvl/mxv
	.byte	W05
	.byte		        10*mus_qz_616_mvl/mxv
	.byte	W07
	.byte		        13*mus_qz_616_mvl/mxv
	.byte	W06
	.byte		        20*mus_qz_616_mvl/mxv
	.byte	W06
	.byte		        29*mus_qz_616_mvl/mxv
	.byte	W05
	.byte		        32*mus_qz_616_mvl/mxv
	.byte	W01
	.byte		        40*mus_qz_616_mvl/mxv
	.byte	W06
	.byte		        48*mus_qz_616_mvl/mxv
	.byte	W05
	.byte		        43*mus_qz_616_mvl/mxv
	.byte	W01
	.byte		        54*mus_qz_616_mvl/mxv
	.byte	W06
	.byte		        43*mus_qz_616_mvl/mxv
	.byte	N07
	.byte	W08
	.byte		        Ds2
	.byte	W08
	.byte		        Bn1
	.byte	W08
	.byte		N44   , En1 , v088 , gtp3
	.byte	W06
	.byte		VOL   , 35*mus_qz_616_mvl/mxv
	.byte	W06
	.byte		        29*mus_qz_616_mvl/mxv
	.byte	W05
	.byte		        25*mus_qz_616_mvl/mxv
	.byte	W07
	.byte		        32*mus_qz_616_mvl/mxv
	.byte	W05
	.byte		        48*mus_qz_616_mvl/mxv
	.byte	W07
	.byte		        54*mus_qz_616_mvl/mxv
	.byte	W12
	.byte		        43*mus_qz_616_mvl/mxv
	.byte		N07   , Fs2
	.byte	W08
	.byte		        En2
	.byte	W08
	.byte		        Ds2
	.byte	W08
	.byte		        En1
	.byte	W08
	.byte		        Fn1
	.byte	W08
	.byte		        Fs1
	.byte	W08
	.byte		        As1
	.byte	W08
	.byte		        Fn1
	.byte	W08
	.byte		N54   , Fs1 , v088 , gtp1
	.byte	W14
	.byte		VOL   , 32*mus_qz_616_mvl/mxv
	.byte	W06
	.byte		        20*mus_qz_616_mvl/mxv
	.byte	W05
	.byte		        12*mus_qz_616_mvl/mxv
	.byte	W07
	.byte		        27*mus_qz_616_mvl/mxv
	.byte	W05
	.byte		        38*mus_qz_616_mvl/mxv
	.byte	W07
	.byte		        48*mus_qz_616_mvl/mxv
	.byte	W06
	.byte		        54*mus_qz_616_mvl/mxv
	.byte	W06
	.byte		        43*mus_qz_616_mvl/mxv
	.byte	N07
	.byte	W08
	.byte		        Gs1
	.byte	W08
	.byte		        Bn1
	.byte	W08
	.byte		        Ds2
	.byte	W08
	.byte		        Gs1
	.byte	W08
	.byte		N23   , Bn1
	.byte	W08
	.byte		VOL   , 37*mus_qz_616_mvl/mxv
	.byte	W05
	.byte		        29*mus_qz_616_mvl/mxv
	.byte	W07
	.byte		        25*mus_qz_616_mvl/mxv
	.byte	W04
	.byte		N07   , Ds1
	.byte	W02
	.byte		VOL   , 43*mus_qz_616_mvl/mxv
	.byte	W06
	.byte		        54*mus_qz_616_mvl/mxv
	.byte		N15   , Ds2
	.byte	W16
	.byte		N07   , Gs1
	.byte	W08
	.byte		VOL   , 43*mus_qz_616_mvl/mxv
	.byte		N07   , As1
	.byte	W08
	.byte		        Bn1
	.byte	W08
	.byte		        As1
	.byte	W08
mus_qz_616_8_002:
	.byte		N23   , Cs1 , v088
	.byte	W24
	.byte		N07   , En1
	.byte	W08
	.byte		        Ds1
	.byte	W08
	.byte		        Dn1
	.byte	W08
	.byte		N15   , Cs1
	.byte	W16
	.byte		        Gs0
	.byte	W16
	.byte		N07   , Fs1
	.byte	W08
	.byte		        Fn1
	.byte	W08
	.byte	PEND
mus_qz_616_8_003:
	.byte		N15   , En1 , v088
	.byte	W16
	.byte		        As0
	.byte	W16
	.byte		        Fs1
	.byte	W16
	.byte		N07   , Fn1
	.byte	W08
	.byte		        En1
	.byte	W08
	.byte		        Ds1
	.byte	W08
	.byte		N15   , Cs1
	.byte	W16
	.byte		N07   , Cn1
	.byte	W08
	.byte	PEND
mus_qz_616_8_004:
	.byte		N15   , Bn0 , v088
	.byte	W16
	.byte		        Fs0
	.byte	W16
	.byte		N23   , Fs1
	.byte	W24
	.byte		N07   , En1
	.byte	W08
	.byte		        Ds1
	.byte	W08
	.byte		        Cs1
	.byte	W08
	.byte		        Bn0
	.byte	W08
	.byte		        Gs0
	.byte	W08
	.byte	PEND
	.byte		        Fs0
	.byte	W08
	.byte		        Fs1
	.byte	W08
	.byte		N03   , Fs0
	.byte	W08
	.byte	N23
	.byte	W24
	.byte		        En1
	.byte	W24
	.byte		VOICE , 0
	.byte		N15   , Fs1
	.byte	W16
	.byte		N07   , Fn1
	.byte	W08
	.byte		VOL   , 42*mus_qz_616_mvl/mxv
	.byte		N68   , En1 , v088 , gtp3
	.byte	W36
	.byte		VOL   , 42*mus_qz_616_mvl/mxv
	.byte	W05
	.byte		        34*mus_qz_616_mvl/mxv
	.byte	W07
	.byte		        27*mus_qz_616_mvl/mxv
	.byte	W05
	.byte		        20*mus_qz_616_mvl/mxv
	.byte	W07
	.byte		        16*mus_qz_616_mvl/mxv
	.byte	W05
	.byte		        10*mus_qz_616_mvl/mxv
	.byte	W07
	.byte		        43*mus_qz_616_mvl/mxv
	.byte	N07
	.byte	W08
	.byte		        Ds2
	.byte	W08
	.byte		        Bn1
	.byte	W08
	.byte		N44   , En1 , v088 , gtp3
	.byte	W30
	.byte		VOL   , 39*mus_qz_616_mvl/mxv
	.byte	W06
	.byte		        34*mus_qz_616_mvl/mxv
	.byte	W05
	.byte		        27*mus_qz_616_mvl/mxv
	.byte	W07
	.byte		        43*mus_qz_616_mvl/mxv
	.byte		N07   , Gn2
	.byte	W08
	.byte		        En2
	.byte	W08
	.byte		        Bn1
	.byte	W08
	.byte		        As1
	.byte	W08
	.byte		        Gn1
	.byte	W08
	.byte		        En1
	.byte	W08
	.byte		        Ds1
	.byte	W08
	.byte		        As1
	.byte	W08
	.byte		        Cs1
	.byte	W08
	.byte		N44   , Ds1 , v088 , gtp3
	.byte	W24
	.byte		VOL   , 39*mus_qz_616_mvl/mxv
	.byte	W05
	.byte		        33*mus_qz_616_mvl/mxv
	.byte	W07
	.byte		        25*mus_qz_616_mvl/mxv
	.byte	W05
	.byte		        20*mus_qz_616_mvl/mxv
	.byte	W07
	.byte		        43*mus_qz_616_mvl/mxv
	.byte		N07   , Fs1
	.byte	W08
	.byte		        Gs1
	.byte	W08
	.byte		        Bn1
	.byte	W08
	.byte		        Ds2
	.byte	W08
	.byte		        Gs1
	.byte	W08
	.byte		N23   , Bn1
	.byte	W24
	.byte		N07   , Ds1
	.byte	W08
	.byte		N15   , Ds2
	.byte	W16
	.byte		N07   , Dn2
	.byte	W08
	.byte		        Bn1
	.byte	W08
	.byte		        Fs1
	.byte	W08
	.byte		        Gs1
	.byte	W08
	.byte	PATT
	 ptrvalue	mus_qz_616_8_002
	.byte	PATT
	 ptrvalue	mus_qz_616_8_003
	.byte	PATT
	 ptrvalue	mus_qz_616_8_004
	.byte		N07   , En1 , v088
	.byte	W08
	.byte		        Bn0
	.byte	W08
	.byte		        Gs0
	.byte	W08
	.byte		VOL   , 42*mus_qz_616_mvl/mxv
	.byte		N68   , Bn0 , v088 , gtp1
	.byte	W36
	.byte		VOL   , 42*mus_qz_616_mvl/mxv
	.byte	W05
	.byte		        34*mus_qz_616_mvl/mxv
	.byte	W07
	.byte		        27*mus_qz_616_mvl/mxv
	.byte	W05
	.byte		        20*mus_qz_616_mvl/mxv
	.byte	W07
	.byte		        16*mus_qz_616_mvl/mxv
	.byte	W05
	.byte		        10*mus_qz_616_mvl/mxv
	.byte	W07
	.byte	GOTO
	 ptrvalue	mus_qz_616_8_B1
	.byte	FINE

/********************** Track  9 **********************/

mus_qz_616_9:
	.byte		KEYSH , mus_qz_616_key+0
mus_qz_616_9_B1:
	.byte		VOICE , 40
	.byte		VOL   , 54*mus_qz_616_mvl/mxv
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte		PAN   , c_v+0
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W96
	.byte	W48
	.byte		VOICE , 70
	.byte		PAN   , c_v+40
	.byte	W18
	.byte		N02   , Cn3 , v096
	.byte	W03
	.byte		        Cn3 , v092
	.byte	W03
	.byte		PAN   , c_v+42
	.byte		N11   , Cn3 , v116
	.byte	W12
	.byte		        Cn2 , v080
	.byte	W12
	.byte		VOICE , 71
	.byte		PAN   , c_v-20
	.byte		VOL   , 43*mus_qz_616_mvl/mxv
	.byte		N07   , Cn3 , v060
	.byte	W08
	.byte		PAN   , c_v+37
	.byte		N07   , Cn3 , v024
	.byte	W08
	.byte		PAN   , c_v-51
	.byte		N07   , Cn3 , v008
	.byte	W80
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
	 ptrvalue	mus_qz_616_9_B1
	.byte	FINE

/******************************************************/
	.p2align	2

mus_qz_616:
	.byte	9	/* NumTrks */
	.byte	0	/* NumBlks */
	.byte	mus_qz_616_pri	/* Priority */
	.byte	mus_qz_616_rev	/* Reverb. */
space64 4
	ptrvalue	mus_qz_616_grp

	ptrvalue	mus_qz_616_1
	ptrvalue	mus_qz_616_2
	ptrvalue	mus_qz_616_3
	ptrvalue	mus_qz_616_4
	ptrvalue	mus_qz_616_5
	ptrvalue	mus_qz_616_6
	ptrvalue	mus_qz_616_7
	ptrvalue	mus_qz_616_8
	ptrvalue	mus_qz_616_9

	.end
