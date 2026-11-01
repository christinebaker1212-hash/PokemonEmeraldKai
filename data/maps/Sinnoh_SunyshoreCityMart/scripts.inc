Sinnoh_SunyshoreCityMart_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_SunyshoreCityMart_EventScript_Person1::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart 0x0
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

Sinnoh_SunyshoreCityMart_EventScript_Person2::
	msgbox Sinnoh_SunyshoreCityMart_Text_Person2, MSGBOX_NPC
	end

Sinnoh_SunyshoreCityMart_EventScript_Person3::
	msgbox Sinnoh_SunyshoreCityMart_Text_Person3, MSGBOX_NPC
	end

Sinnoh_SunyshoreCityMart_EventScript_Person5::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart 0x0
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

Sinnoh_SunyshoreCityMart_Text_Person2:
	.string "I'm going to explore the ruins north\n"
	.string "of here. Bring ESCAPE ROPES, I say!$"

Sinnoh_SunyshoreCityMart_Text_Person3:
	.string "A sailor always stocks up before a\n"
	.string "long trip. Even if it's just to the next\l"
	.string "town!$"

@ --- end of SINNOH words ---
