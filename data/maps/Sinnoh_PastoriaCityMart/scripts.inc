Sinnoh_PastoriaCityMart_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_PastoriaCityMart_EventScript_Bg1::
	msgbox Sinnoh_PastoriaCityMart_Text_Bg1, MSGBOX_SIGN
	end

Sinnoh_PastoriaCityMart_EventScript_Person1::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart 0x0
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

Sinnoh_PastoriaCityMart_EventScript_Person4::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart 0x0
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

Sinnoh_PastoriaCityMart_Text_Bg1:
	.string "A carving of a grinning MOéMON. Its\n"
	.string "cheeks look puffed up.$"

@ --- end of SINNOH words ---
