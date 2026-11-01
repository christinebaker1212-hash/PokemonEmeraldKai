Sinnoh_FightAreaMart_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_FightAreaMart_EventScript_Person1::
	msgbox Sinnoh_FightAreaMart_Text_Person1, MSGBOX_NPC
	end

Sinnoh_FightAreaMart_EventScript_Person2::
	msgbox Sinnoh_FightAreaMart_Text_Person2, MSGBOX_NPC
	end

Sinnoh_FightAreaMart_EventScript_Person3::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart 0x0
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

Sinnoh_FightAreaMart_Text_Person1:
	.string "Even clowns need POTIONS out here.\n"
	.string "Battles everywhere!$"

Sinnoh_FightAreaMart_Text_Person2:
	.string "I shop here before every challenge,\n"
	.string "darling. A lady must be prepared.$"

@ --- end of SINNOH words ---
