Sinnoh_SurvivalAreaMart_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_SurvivalAreaMart_EventScript_Person1::
	msgbox Sinnoh_SurvivalAreaMart_Text_Person1, MSGBOX_NPC
	end

Sinnoh_SurvivalAreaMart_EventScript_Person2::
	msgbox Sinnoh_SurvivalAreaMart_Text_Person2, MSGBOX_NPC
	end

Sinnoh_SurvivalAreaMart_EventScript_Person3::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart 0x0
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

Sinnoh_SurvivalAreaMart_Text_Person1:
	.string "I buy POTIONS by the dozen out here.\n"
	.string "Every battle counts.$"

Sinnoh_SurvivalAreaMart_Text_Person2:
	.string "The mountain's tough to climb. Stock\n"
	.string "up on ESCAPE ROPES!$"

@ --- end of SINNOH words ---
