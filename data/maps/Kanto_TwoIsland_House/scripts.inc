Kanto_TwoIsland_House_MapScripts::
	.byte 0

@ Moemon Kai: FireRed's MOVE MANIAC (pokefirered TwoIsland_House): a move remembered for two TINYMUSHROOMS or a BIG
@ MUSHROOM, on Emerald's MOVE RELEARNER specials (FALLARBOR's); Quetzal's "move training from the team menu" is gone
@ Kanto_TwoIsland_House object 1 (local id 1, gfx 0x3F04) at 7,5
Kanto_TwoIsland_House_EventScript_8421519::
	lock
	faceplayer
	clearflag FLAG_TEMP_2
	clearflag FLAG_TEMP_3
	clearflag FLAG_TEMP_4
	goto_if_set FLAG_TEMP_1, Kanto_TwoIsland_House_EventScript_CheckPlayerHasMushrooms
	msgbox Kanto_TwoIsland_House_Text_TeachMonMoveForMushroom, MSGBOX_DEFAULT
	setflag FLAG_TEMP_1
	goto Kanto_TwoIsland_House_EventScript_CheckPlayerHasMushrooms
	end

Kanto_TwoIsland_House_EventScript_CheckPlayerHasMushrooms::
	checkitem ITEM_BIG_MUSHROOM
	call_if_eq VAR_RESULT, TRUE, Kanto_TwoIsland_House_EventScript_SetHasBigMushroom
	checkitem ITEM_TINY_MUSHROOM, 2
	call_if_eq VAR_RESULT, TRUE, Kanto_TwoIsland_House_EventScript_SetHasTinyMushrooms
	goto_if_set FLAG_TEMP_2, Kanto_TwoIsland_House_EventScript_CheckAlsoHasTinyMushrooms
	goto_if_set FLAG_TEMP_3, Kanto_TwoIsland_House_EventScript_CheckAlsoHasBigMushroom
	goto Kanto_TwoIsland_House_EventScript_EndTutorMove
	end

Kanto_TwoIsland_House_EventScript_CheckAlsoHasTinyMushrooms::
	call_if_set FLAG_TEMP_3, Kanto_TwoIsland_House_EventScript_SetHasBothMushrooms
	goto Kanto_TwoIsland_House_EventScript_AskTutorMon
	end

Kanto_TwoIsland_House_EventScript_CheckAlsoHasBigMushroom::
	call_if_set FLAG_TEMP_2, Kanto_TwoIsland_House_EventScript_SetHasBothMushrooms
	goto Kanto_TwoIsland_House_EventScript_AskTutorMon
	end

Kanto_TwoIsland_House_EventScript_AskTutorMon::
	msgbox Kanto_TwoIsland_House_Text_WantMeToTeachMove, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Kanto_TwoIsland_House_EventScript_EndTutorMove
	goto Kanto_TwoIsland_House_EventScript_ChooseMonToTutor
	end

Kanto_TwoIsland_House_EventScript_ChooseMonToTutor::
	msgbox Kanto_TwoIsland_House_Text_TutorWhichMon, MSGBOX_DEFAULT
	special ChooseMonForMoveRelearner
	goto_if_ge VAR_0x8004, PARTY_SIZE, Kanto_TwoIsland_House_EventScript_EndTutorMove
	special IsSelectedMonEgg
	goto_if_eq VAR_RESULT, TRUE, Kanto_TwoIsland_House_EventScript_CantTutorEgg
	goto_if_eq VAR_0x8005, 0, Kanto_TwoIsland_House_EventScript_NoMoveToTutorMon
	goto Kanto_TwoIsland_House_EventScript_ChooseMoveToTeach
	end

Kanto_TwoIsland_House_EventScript_ChooseMoveToTeach::
	msgbox Kanto_TwoIsland_House_Text_TeachWhichMove, MSGBOX_DEFAULT
	special TeachMoveRelearnerMove
	goto_if_eq VAR_0x8004, 0, Kanto_TwoIsland_House_EventScript_ChooseMonToTutor
	goto_if_set FLAG_TEMP_4, Kanto_TwoIsland_House_EventScript_ChooseMushroom
	goto_if_set FLAG_TEMP_2, Kanto_TwoIsland_House_EventScript_GiveBigMushroom
	goto_if_set FLAG_TEMP_3, Kanto_TwoIsland_House_EventScript_GiveTinyMushrooms
	end

Kanto_TwoIsland_House_EventScript_GiveBigMushroom::
	removeitem ITEM_BIG_MUSHROOM
	msgbox Kanto_TwoIsland_House_Text_HandedOverOneBigMushroom, MSGBOX_DEFAULT
	goto Kanto_TwoIsland_House_EventScript_EndTutorMove
	end

Kanto_TwoIsland_House_EventScript_GiveTinyMushrooms::
	removeitem ITEM_TINY_MUSHROOM, 2
	msgbox Kanto_TwoIsland_House_Text_HandedOverTwoTinyMushrooms, MSGBOX_DEFAULT
	goto Kanto_TwoIsland_House_EventScript_EndTutorMove
	end

Kanto_TwoIsland_House_EventScript_ChooseMushroom::
	message Kanto_TwoIsland_House_Text_IllNeedMushroomOrTwo
	waitmessage
	multichoice 0, 0, MULTI_QZ_0x6D, TRUE
	switch VAR_RESULT
	case 0, Kanto_TwoIsland_House_EventScript_GiveTinyMushrooms
	case 1, Kanto_TwoIsland_House_EventScript_GiveBigMushroom
	end

Kanto_TwoIsland_House_EventScript_NoMoveToTutorMon::
	msgbox Kanto_TwoIsland_House_Text_NoMoveICanTeachThatMon, MSGBOX_DEFAULT
	goto Kanto_TwoIsland_House_EventScript_ChooseMonToTutor
	end

Kanto_TwoIsland_House_EventScript_CantTutorEgg::
	msgbox Kanto_TwoIsland_House_Text_CantTeachAnEgg, MSGBOX_DEFAULT
	goto Kanto_TwoIsland_House_EventScript_ChooseMonToTutor
	end

Kanto_TwoIsland_House_EventScript_EndTutorMove::
	msgbox Kanto_TwoIsland_House_Text_NeedMushroomOrTwoToLearnMove, MSGBOX_DEFAULT
	release
	end

Kanto_TwoIsland_House_EventScript_SetHasBigMushroom::
	setflag FLAG_TEMP_2
	return

Kanto_TwoIsland_House_EventScript_SetHasTinyMushrooms::
	setflag FLAG_TEMP_3
	return

Kanto_TwoIsland_House_EventScript_SetHasBothMushrooms::
	setflag FLAG_TEMP_4
	return

Kanto_TwoIsland_House_Text_CantTeachAnEgg::
	.string "Hunh? There isn’t a single move\n"
	.string "that I can teach an EGG.$"

Kanto_TwoIsland_House_Text_HandedOverOneBigMushroom::
	.string "{PLAYER} handed over one\n"
	.string "BIG MUSHROOM in exchange.$"

Kanto_TwoIsland_House_Text_HandedOverTwoTinyMushrooms::
	.string "{PLAYER} handed over two\n"
	.string "TINYMUSHROOMS in exchange.$"

Kanto_TwoIsland_House_Text_IllNeedMushroomOrTwo::
	.string "Then, I’ll need a MUSHROOM or two.\n"
	.string "What are you going to give me?$"

Kanto_TwoIsland_House_Text_NeedMushroomOrTwoToLearnMove::
	.string "If your MOéMON need to learn a\n"
	.string "move, I need a MUSHROOM or two.$"

Kanto_TwoIsland_House_Text_NoMoveICanTeachThatMon::
	.string "Sorry…\p"
	.string "It doesn’t appear as if I have a\n"
	.string "move I can teach that MOéMON.$"

Kanto_TwoIsland_House_Text_TeachMonMoveForMushroom::
	.string "I’m the MOéMON MOVE MANIAC.\p"
	.string "I know every single move that\n"
	.string "MOéMON learn growing up.\p"
	.string "I’m also a mushroom maniac.\p"
	.string "Bring me two TINYMUSHROOMS,\n"
	.string "or one BIG MUSHROOM.\p"
	.string "If you do that for me, I’ll teach\n"
	.string "a move to one MOéMON.$"

Kanto_TwoIsland_House_Text_TeachWhichMove::
	.string "Which move should I teach?$"

Kanto_TwoIsland_House_Text_TutorWhichMon::
	.string "Which MOéMON needs tutoring?$"

Kanto_TwoIsland_House_Text_WantMeToTeachMove::
	.string "Sniff, sniff…\n"
	.string "Hm! You smell of MUSHROOMS!\p"
	.string "Do you want me to teach a move\n"
	.string "to a MOéMON of yours?$"

