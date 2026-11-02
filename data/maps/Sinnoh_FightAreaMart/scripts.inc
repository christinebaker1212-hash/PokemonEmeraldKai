Sinnoh_FightAreaMart_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_FightAreaMart_EventScript_Person1::
	goto SinnohP_fight_area_mart_4
	end

Sinnoh_FightAreaMart_EventScript_Person2::
	lock
	faceplayer
	goto_if_set 0x3DAB, Sinnoh_FightAreaMart_EventScript_Person2_Given
	msgbox Sinnoh_FightAreaMart_Text_Person2, MSGBOX_DEFAULT
	msgbox Sinnoh_FightAreaMart_Text_Person2Gift, MSGBOX_DEFAULT
	giveitem ITEM_SCOPE_LENS, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3DAB
	release
	end

Sinnoh_FightAreaMart_EventScript_Person2_Given::
	msgbox Sinnoh_FightAreaMart_Text_Person2, MSGBOX_DEFAULT
	release
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

Sinnoh_FightAreaMart_Text_Person2:
	.string "I shop here before every challenge,\n"
	.string "darling. A lady must be prepared.$"

Sinnoh_FightAreaMart_Text_Person2Gift:
	.string "You remind me of myself at your age,\n"
	.string "darling. Take this lens. It helps one\l"
	.string "aim true.$"

@ --- end of SINNOH words ---
