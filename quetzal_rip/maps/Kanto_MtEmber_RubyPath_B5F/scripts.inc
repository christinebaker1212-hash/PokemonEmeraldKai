Kanto_MtEmber_RubyPath_B5F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840FBB9
	.byte 0

@ Kanto_MtEmber_RubyPath_B5F sign / bg event at 7,2
Kanto_MtEmber_RubyPath_B5F_EventScript_8413953::
	lockall
	setvar VAR_0x8005, 0x82
	setvar VAR_0x8006, 0
	braillemessage Kanto_MtEmber_RubyPath_B5F_Braille_8459606
	call Quetzal_EventScript_8459552
	setvar VAR_0x8006, 0
	braillemessage Kanto_MtEmber_RubyPath_B5F_Braille_8459617
	call Quetzal_EventScript_8459552
	setvar VAR_0x8006, 0
	braillemessage Kanto_MtEmber_RubyPath_B5F_Braille_8459629
	call Quetzal_EventScript_8459552
	setvar VAR_0x8006, 0
	braillemessage Kanto_MtEmber_RubyPath_B5F_Braille_8459639
	call Quetzal_EventScript_8459552
	setvar VAR_0x8006, 0
	braillemessage Kanto_MtEmber_RubyPath_B5F_Braille_845964B
	call Quetzal_EventScript_8459552
	setvar VAR_0x8006, 0
	braillemessage Kanto_MtEmber_RubyPath_B5F_Braille_845965D
	call Quetzal_EventScript_8459552
	setvar VAR_0x8006, 0
	braillemessage Kanto_MtEmber_RubyPath_B5F_Braille_845966F
	call Quetzal_EventScript_8459552
	braillemessage Kanto_MtEmber_RubyPath_B5F_Braille_8459681
	waitbuttonpress
	closebraillemessage
	releaseall
	end

@ Kanto_MtEmber_RubyPath_B5F object 1 (local id 1, gfx 0x3F3F) at 7,7
Kanto_MtEmber_RubyPath_B5F_EventScript_84139D4::
	lock
	faceplayer
	setflag FLAG_QZ_0x2161
	removeobject 1
	additem ITEM_RUBY, 1
	loadword 0, Kanto_MtEmber_RubyPath_B5F_Text_842B4CE
	setorcopyvar VAR_0x8000, 0x2F4
	setorcopyvar VAR_0x8001, 1
	setorcopyvar VAR_0x8002, 0x212
	call Quetzal_EventScript_845571E
	release
	end

Kanto_MtEmber_RubyPath_B5F_Braille_8459606:
	@ braille
	.string "      Ì=ÌëごìÒÉéÔ$"

Kanto_MtEmber_RubyPath_B5F_Braille_8459617:
	@ braille
	.string "      ÒÀà ÛÌÀéÉéÔ$"

Kanto_MtEmber_RubyPath_B5F_Braille_8459629:
	@ braille
	.string "      ÌぉÉàìÌéÂÌ$"

Kanto_MtEmber_RubyPath_B5F_Braille_8459639:
	@ braille
	.string "      ÒÀà ÛÌÀéÉéÔ$"

Kanto_MtEmber_RubyPath_B5F_Braille_845964B:
	@ braille
	.string "      ÈÌÉéÔ ÀßÉ=Ì$"

Kanto_MtEmber_RubyPath_B5F_Braille_845965D:
	@ braille
	.string "      ÒÀà ÛÌÀéÉéÔ$"

Kanto_MtEmber_RubyPath_B5F_Braille_845966F:
	@ braille
	.string "      ÒÀ=Ì ÎëÌÀÛà$"

Kanto_MtEmber_RubyPath_B5F_Braille_8459681:
	@ braille
	.string "      ぅàÌ áç+Ìëわ$"

Kanto_MtEmber_RubyPath_B5F_Text_842B4CE::
	.string "{PLAYER} found a RUBY!$"
