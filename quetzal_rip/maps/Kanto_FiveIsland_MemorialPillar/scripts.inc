Kanto_FiveIsland_MemorialPillar_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840FBB9
	.byte 0

@ Kanto_FiveIsland_MemorialPillar object 1 (local id 1, gfx 0x3F1E) at 8,44
Kanto_FiveIsland_MemorialPillar_EventScript_8418783::
	lock
	goto_if_set FLAG_QZ_0x2174, Kanto_FiveIsland_MemorialPillar_EventScript_84187C8
	goto_if_set FLAG_QZ_0x21E1, Kanto_FiveIsland_MemorialPillar_EventScript_84187D2
	msgbox Kanto_FiveIsland_MemorialPillar_Text_843B317, MSGBOX_DEFAULT
	closemessage
	applymovement 1, Quetzal_Movement_8457AA4
	waitmovement 0
	delay 0x2D
	msgbox Kanto_FiveIsland_MemorialPillar_Text_843B325, MSGBOX_DEFAULT
	applymovement 1, Quetzal_Movement_8457AAC
	waitmovement 0
	msgbox Kanto_FiveIsland_MemorialPillar_Text_843B347, MSGBOX_DEFAULT
	release
	end

Kanto_FiveIsland_MemorialPillar_EventScript_84187C8::
	msgbox Kanto_FiveIsland_MemorialPillar_Text_843B5C3, MSGBOX_DEFAULT
	release
	end

Kanto_FiveIsland_MemorialPillar_EventScript_84187D2::
	msgbox Kanto_FiveIsland_MemorialPillar_Text_843B56B, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, 0xE9
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Kanto_FiveIsland_MemorialPillar_EventScript_84188B0
	call Kanto_FiveIsland_MemorialPillar_EventScript_841889A
	release
	end

@ Kanto_FiveIsland_MemorialPillar sign / bg event at 9,43
Kanto_FiveIsland_MemorialPillar_EventScript_84187F8::
	specialvar VAR_RESULT, QzReturnZero_732
	goto_if_eq VAR_RESULT, 2, Quetzal_EventScript_8457FA3
	special QzQuestLogNop_737
	lockall
	goto_if_set FLAG_QZ_0x2174, Kanto_FiveIsland_MemorialPillar_EventScript_84188BD
	goto_if_set FLAG_QZ_0x21E1, Kanto_FiveIsland_MemorialPillar_EventScript_84188BD
	msgbox Kanto_FiveIsland_MemorialPillar_Text_843B37E, MSGBOX_DEFAULT
	checkitem ITEM_LEMONADE, 1
	goto_if_eq VAR_RESULT, 1, Kanto_FiveIsland_MemorialPillar_EventScript_8418838
	releaseall
	end

Kanto_FiveIsland_MemorialPillar_EventScript_8418838::
	msgbox Kanto_FiveIsland_MemorialPillar_Text_843B3EE, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Kanto_FiveIsland_MemorialPillar_EventScript_841884D
	releaseall
	end

Kanto_FiveIsland_MemorialPillar_EventScript_841884D::
	removeitem ITEM_LEMONADE, 1
	msgbox Kanto_FiveIsland_MemorialPillar_Text_843B427, MSGBOX_DEFAULT
	closemessage
	applymovement 1, Quetzal_Movement_8457AAE
	waitmovement 0
	delay 0x2D
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AAA
	waitmovement 0
	textcolor 0
	msgbox Kanto_FiveIsland_MemorialPillar_Text_843B45A, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, 0xE9
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Kanto_FiveIsland_MemorialPillar_EventScript_84188B0
	call Kanto_FiveIsland_MemorialPillar_EventScript_841889A
	releaseall
	end

Kanto_FiveIsland_MemorialPillar_EventScript_841889A::
	msgbox Kanto_FiveIsland_MemorialPillar_Text_843B502, MSGBOX_DEFAULT
	applymovement 1, Quetzal_Movement_8457AAC
	waitmovement 0
	setflag FLAG_QZ_0x2174
	return

Kanto_FiveIsland_MemorialPillar_EventScript_84188B0::
	setflag FLAG_QZ_0x21E1
	msgbox Kanto_FiveIsland_MemorialPillar_Text_843B527, MSGBOX_DEFAULT
	releaseall
	end

Kanto_FiveIsland_MemorialPillar_EventScript_84188BD::
	msgbox Kanto_FiveIsland_MemorialPillar_Text_843B5E1, MSGBOX_DEFAULT
	releaseall
	end

@ Kanto_FiveIsland_MemorialPillar object 2 (local id 2, gfx 0x3F3B) at 12,6
Kanto_FiveIsland_MemorialPillar_EventScript_845C6E8::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8762, 0, Kanto_FiveIsland_MemorialPillar_Text_843B139, Kanto_FiveIsland_MemorialPillar_Text_843B191
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_FiveIsland_MemorialPillar_EventScript_845C70F
	msgbox Kanto_FiveIsland_MemorialPillar_Text_843B19F, MSGBOX_AUTOCLOSE
	end

Kanto_FiveIsland_MemorialPillar_EventScript_845C70F::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8762, 0, Kanto_FiveIsland_MemorialPillar_Text_8471248, Kanto_FiveIsland_MemorialPillar_Text_843B191
	msgbox Kanto_FiveIsland_MemorialPillar_Text_843B19F, MSGBOX_AUTOCLOSE
	end

@ Kanto_FiveIsland_MemorialPillar object 3 (local id 3, gfx 0x3F3B) at 14,17
Kanto_FiveIsland_MemorialPillar_EventScript_845C726::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8763, 0, Kanto_FiveIsland_MemorialPillar_Text_843B1C5, Kanto_FiveIsland_MemorialPillar_Text_843B222
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_FiveIsland_MemorialPillar_EventScript_845C74D
	msgbox Kanto_FiveIsland_MemorialPillar_Text_843B22E, MSGBOX_AUTOCLOSE
	end

Kanto_FiveIsland_MemorialPillar_EventScript_845C74D::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8763, 0, Kanto_FiveIsland_MemorialPillar_Text_84712B5, Kanto_FiveIsland_MemorialPillar_Text_843B222
	msgbox Kanto_FiveIsland_MemorialPillar_Text_843B22E, MSGBOX_AUTOCLOSE
	end

@ Kanto_FiveIsland_MemorialPillar object 4 (local id 4, gfx 0x3F3B) at 17,31
Kanto_FiveIsland_MemorialPillar_EventScript_845C764::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8764, 0, Kanto_FiveIsland_MemorialPillar_Text_843B25B, Kanto_FiveIsland_MemorialPillar_Text_843B2B5
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_FiveIsland_MemorialPillar_EventScript_845C78B
	msgbox Kanto_FiveIsland_MemorialPillar_Text_843B2DB, MSGBOX_AUTOCLOSE
	end

Kanto_FiveIsland_MemorialPillar_EventScript_845C78B::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8764, 0, Kanto_FiveIsland_MemorialPillar_Text_8471310, Kanto_FiveIsland_MemorialPillar_Text_843B2B5
	msgbox Kanto_FiveIsland_MemorialPillar_Text_843B2DB, MSGBOX_AUTOCLOSE
	end

@ Kanto_FiveIsland_MemorialPillar object 5 (local id 5, gfx 0x3F0E) at 4,47
Kanto_FiveIsland_MemorialPillar_EventScript_846C169::
	setorcopyvar VAR_0x8000, 0x1B9
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_FiveIsland_MemorialPillar_Text_843B139::
	.string "I’m the eldest of the BIRD\n"
	.string "BROTHERS.\p"
	.string "The best thing about birds…\n"
	.string "It has to be the beak.$"

Kanto_FiveIsland_MemorialPillar_Text_843B191::
	.string "Oh, so sharp!$"

Kanto_FiveIsland_MemorialPillar_Text_843B19F::
	.string "You can fly to FOUR ISLAND from\n"
	.string "here.$"

Kanto_FiveIsland_MemorialPillar_Text_843B1C5::
	.string "I’m the middle kid of the BIRD\n"
	.string "BROTHERS.\p"
	.string "The best thing about birds…\n"
	.string "It has to be the wings.$"

Kanto_FiveIsland_MemorialPillar_Text_843B222::
	.string "Flap, flap!$"

Kanto_FiveIsland_MemorialPillar_Text_843B22E::
	.string "There was a really somber guy\n"
	.string "over that way.$"

Kanto_FiveIsland_MemorialPillar_Text_843B25B::
	.string "I’m the youngest of the BIRD\n"
	.string "BROTHERS.\p"
	.string "The best thing about birds…\n"
	.string "It has to be the down.$"

Kanto_FiveIsland_MemorialPillar_Text_843B2B5::
	.string "Down is so comfy…\n"
	.string "It makes me drowsy…$"

Kanto_FiveIsland_MemorialPillar_Text_843B2DB::
	.string "It fills me with happiness when\n"
	.string "bird POKéMON beg playfully…$"

Kanto_FiveIsland_MemorialPillar_Text_843B317::
	.string "Scrub, scrub…$"

Kanto_FiveIsland_MemorialPillar_Text_843B325::
	.string "Hello…\n"
	.string "Your POKéMON look healthy…$"

Kanto_FiveIsland_MemorialPillar_Text_843B347::
	.string "This is where I buried my ONIX…\n"
	.string "It was named TECTONIX…$"

Kanto_FiveIsland_MemorialPillar_Text_843B37E::
	.string "“Here lies TECTONIX” is engraved\n"
	.string "on a boulder.\p"
	.string "There is a LEMONADE placed as an\n"
	.string "offering in front of the grave.$"

Kanto_FiveIsland_MemorialPillar_Text_843B3EE::
	.string "Would you like to leave another\n"
	.string "LEMONADE as an offering?$"

Kanto_FiveIsland_MemorialPillar_Text_843B427::
	.string "{PLAYER} placed a can of LEMONADE\n"
	.string "in front of the grave.$"

Kanto_FiveIsland_MemorialPillar_Text_843B45A::
	.string "Th-thank you…\p"
	.string "That was my TECTONIX’s favorite\n"
	.string "drink…\p"
	.string "I don’t even know you, but your\n"
	.string "act of kindness…\p"
	.string "It’s uplifted me a little.\p"
	.string "Can you please take this as my\n"
	.string "thanks?$"

Kanto_FiveIsland_MemorialPillar_Text_843B502::
	.string "Please be good to your POKéMON,\n"
	.string "too.$"

Kanto_FiveIsland_MemorialPillar_Text_843B527::
	.string "If you don’t have room for it,\n"
	.string "I’ll hold it for you till next time.$"

Kanto_FiveIsland_MemorialPillar_Text_843B56B::
	.string "Oh, it’s you from that time…\p"
	.string "I still have that thing I wanted\n"
	.string "you to have as my thanks.$"

Kanto_FiveIsland_MemorialPillar_Text_843B5C3::
	.string "Scrub, scrub…\p"
	.string "… … …\n"
	.string "TECTONIX…$"

Kanto_FiveIsland_MemorialPillar_Text_843B5E1::
	.string "Boulders and rocks are neatly piled\n"
	.string "up as a cairn.\p"
	.string "“Here lies TECTONIX” is engraved\n"
	.string "on a boulder.$"

Kanto_FiveIsland_MemorialPillar_Text_8471248::
	.string "I’m the eldest of the BIRD\n"
	.string "BROTHERS. Remember me?\p"
	.string "That’s right, I’m the one who loves\n"
	.string "birds for their beaks!$"

Kanto_FiveIsland_MemorialPillar_Text_84712B5::
	.string "I’m the middle kid of the BIRD\n"
	.string "BROTHERS.\p"
	.string "I’m the one that loves wings.\n"
	.string "Let’s battle again!$"

Kanto_FiveIsland_MemorialPillar_Text_8471310::
	.string "I’m the youngest of the BIRD\n"
	.string "BROTHERS.\p"
	.string "I love birds for their down.\n"
	.string "I didn’t think I’d see you again!$"
