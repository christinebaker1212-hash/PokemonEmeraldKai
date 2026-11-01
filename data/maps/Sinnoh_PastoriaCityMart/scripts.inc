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

Sinnoh_PastoriaCityMart_EventScript_Person2::
	msgbox Sinnoh_PastoriaCityMart_Text_Person2, MSGBOX_NPC
	end

Sinnoh_PastoriaCityMart_EventScript_Person3::
	msgbox Sinnoh_PastoriaCityMart_Text_Person3, MSGBOX_NPC
	end

Sinnoh_PastoriaCityMart_EventScript_Person4::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart Sinnoh_Mart_Pastoria
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

Sinnoh_PastoriaCityMart_Text_Bg1:
	.string "A carving of a grinning MOéMON. Its\n"
	.string "cheeks look puffed up.$"

Sinnoh_PastoriaCityMart_Text_Person2:
	.string "I'm buying POKé BALLS for the GREAT\n"
	.string "MARSH. Wait, you can't use your own\l"
	.string "there? Oh no...$"

Sinnoh_PastoriaCityMart_Text_Person3:
	.string "My mom sent me for groceries. I'm\n"
	.string "buying MOéMON food first, though.$"

@ --- end of SINNOH words ---
