Kanto_TwoIsland_CapeBrink_House_MapScripts::
	.byte 0

@ Moemon Kai: FireRed's starter tutor (pokefirered data/scripts/move_tutors.inc TwoIsland_CapeBrink_House_EventScript_
@ StarterTutor): FRENZY PLANT / BLAST BURN / HYDRO CANNON for a lead VENUSAUR / CHARIZARD / BLASTOISE at full
@ friendship, on Kai's move tutors (qz_specials.c MP_CapeBrinkLeadMove); Quetzal's stand-in line is gone
@ Kanto_TwoIsland_CapeBrink_House object 1 (local id 1, gfx 0x3EF6) at 4,4
Kanto_TwoIsland_CapeBrink_House_EventScript_8471CDB::
	lock
	faceplayer
	goto_if_set 0x2229, Kanto_TwoIsland_CapeBrink_House_EventScript_TaughtAllMoves
	goto_if_set FLAG_TEMP_2, Kanto_TwoIsland_CapeBrink_House_EventScript_MoveJustTaught
	bufferleadmonspeciesname STR_VAR_1
	msgbox Kanto_TwoIsland_CapeBrink_House_Text_UltimateMoveThatMon, MSGBOX_DEFAULT
	specialvar VAR_RESULT, MP_CapeBrinkLeadMove
	goto_if_eq VAR_RESULT, FALSE, Kanto_TwoIsland_CapeBrink_House_EventScript_NoLeadStarter
	copyvar VAR_0x8009, VAR_0x8005
	call_if_eq VAR_FACING, DIR_NORTH, Kanto_TwoIsland_CapeBrink_House_EventScript_JumpInPlaceDown
	call_if_eq VAR_FACING, DIR_SOUTH, Kanto_TwoIsland_CapeBrink_House_EventScript_JumpInPlaceUp
	call_if_eq VAR_FACING, DIR_EAST, Kanto_TwoIsland_CapeBrink_House_EventScript_JumpInPlaceLeft
	call_if_eq VAR_FACING, DIR_WEST, Kanto_TwoIsland_CapeBrink_House_EventScript_JumpInPlaceRight
	msgbox Kanto_TwoIsland_CapeBrink_House_Text_AllowMeToTeachMonUltimateMove, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Kanto_TwoIsland_CapeBrink_House_EventScript_DeclineMove
	call MoveTutor_EventScript_CanOnlyBeLearnedOnce
	goto_if_eq VAR_RESULT, NO, Kanto_TwoIsland_CapeBrink_House_EventScript_DeclineMove
	msgbox Kanto_TwoIsland_CapeBrink_House_Text_LetMeConferUltimateMove, MSGBOX_DEFAULT
	closemessage
	fadescreen FADE_TO_BLACK
	goto Kanto_TwoIsland_CapeBrink_House_EventScript_ChooseMon

Kanto_TwoIsland_CapeBrink_House_EventScript_MoveJustTaught::
	msgbox Kanto_TwoIsland_CapeBrink_House_Text_MakeBestUseOfMove, MSGBOX_DEFAULT
	release
	end

Kanto_TwoIsland_CapeBrink_House_EventScript_TaughtAllMoves::
	msgbox Kanto_TwoIsland_CapeBrink_House_Text_FeelRejuvenated, MSGBOX_DEFAULT
	release
	end

Kanto_TwoIsland_CapeBrink_House_EventScript_TaughtMove::
	copyvar VAR_0x8005, VAR_0x8009
	specialvar VAR_RESULT, MP_CapeBrinkTaught
	goto_if_eq VAR_RESULT, TRUE, Kanto_TwoIsland_CapeBrink_House_EventScript_LearnedAllMoves
	msgbox Kanto_TwoIsland_CapeBrink_House_Text_TaughtMove, MSGBOX_DEFAULT
	setflag FLAG_TEMP_2
	release
	end

Kanto_TwoIsland_CapeBrink_House_EventScript_LearnedAllMoves::
	msgbox Kanto_TwoIsland_CapeBrink_House_Text_PassedOnEverythingIKnow, MSGBOX_DEFAULT
	setflag 0x2229
	release
	end

Kanto_TwoIsland_CapeBrink_House_EventScript_ChooseMon::
	call MoveTutor_EventScript_OpenPartyMenu
	goto_if_eq VAR_RESULT, FALSE, Kanto_TwoIsland_CapeBrink_House_EventScript_DeclineMove
	goto Kanto_TwoIsland_CapeBrink_House_EventScript_TaughtMove
	end

Kanto_TwoIsland_CapeBrink_House_EventScript_JumpInPlaceDown::
	applymovement 1, Kanto_TwoIsland_CapeBrink_House_Movement_JumpInPlaceDown
	waitmovement 0
	return

Kanto_TwoIsland_CapeBrink_House_EventScript_JumpInPlaceUp::
	applymovement 1, Kanto_TwoIsland_CapeBrink_House_Movement_JumpInPlaceUp
	waitmovement 0
	return

Kanto_TwoIsland_CapeBrink_House_EventScript_JumpInPlaceLeft::
	applymovement 1, Kanto_TwoIsland_CapeBrink_House_Movement_JumpInPlaceLeft
	waitmovement 0
	return

Kanto_TwoIsland_CapeBrink_House_EventScript_JumpInPlaceRight::
	applymovement 1, Kanto_TwoIsland_CapeBrink_House_Movement_JumpInPlaceRight
	waitmovement 0
	return

Kanto_TwoIsland_CapeBrink_House_EventScript_DeclineMove::
	msgbox Kanto_TwoIsland_CapeBrink_House_Text_YouRejectIt, MSGBOX_DEFAULT
	release
	end

Kanto_TwoIsland_CapeBrink_House_EventScript_NoLeadStarter::
	msgbox Kanto_TwoIsland_CapeBrink_House_Text_JustMistaken, MSGBOX_DEFAULT
	release
	end

Kanto_TwoIsland_CapeBrink_House_Movement_JumpInPlaceDown:
	jump_in_place_down
	step_end

Kanto_TwoIsland_CapeBrink_House_Movement_JumpInPlaceUp:
	jump_in_place_up
	step_end

Kanto_TwoIsland_CapeBrink_House_Movement_JumpInPlaceLeft:
	jump_in_place_left
	step_end

Kanto_TwoIsland_CapeBrink_House_Movement_JumpInPlaceRight:
	jump_in_place_right
	step_end

Kanto_TwoIsland_CapeBrink_House_Text_AllowMeToTeachMonUltimateMove::
	.string "Oh! This is the one!\n"
	.string "This is the MOéMON!\p"
	.string "This {STR_VAR_1} is worthy of\n"
	.string "learning my ultimate move!\p"
	.string "Will you allow it?\p"
	.string "Will you allow your {STR_VAR_1} to\n"
	.string "learn my {STR_VAR_2}?$"

Kanto_TwoIsland_CapeBrink_House_Text_FeelRejuvenated::
	.string "After I passed on my knowledge to\n"
	.string "you, I felt so much relief.\p"
	.string "In fact, I feel rejuvenated,\n"
	.string "as if I were younger again!$"

Kanto_TwoIsland_CapeBrink_House_Text_JustMistaken::
	.string "…No…\n"
	.string "I was just mistaken.$"

Kanto_TwoIsland_CapeBrink_House_Text_LetMeConferUltimateMove::
	.string "You will allow it?\p"
	.string "Then, let me confer my ultimate\n"
	.string "{STR_VAR_2} on your {STR_VAR_1}.\p"
	.string "Ggggrah-awooo!$"

Kanto_TwoIsland_CapeBrink_House_Text_MakeBestUseOfMove::
	.string "I implore you to make the best\n"
	.string "use of that move and gain power!$"

Kanto_TwoIsland_CapeBrink_House_Text_PassedOnEverythingIKnow::
	.string "Gasp, gasp, gasp…\p"
	.string "I have no regrets now.\n"
	.string "I’ve passed on everything I know.\p"
	.string "Now I can live out my life knowing\n"
	.string "my work is done.$"

Kanto_TwoIsland_CapeBrink_House_Text_TaughtMove::
	.string "Gasp, gasp, gasp…\p"
	.string "I didn’t think I could teach that\n"
	.string "move while I still lived…$"

Kanto_TwoIsland_CapeBrink_House_Text_UltimateMoveThatMon::
	.string "I perfected the ultimate move of\n"
	.string "its type…\p"
	.string "But will no one take it for\n"
	.string "future use?\p"
	.string "…Hm? Hmmm!\p"
	.string "Y-you…\n"
	.string "Th-that {STR_VAR_1}…$"

Kanto_TwoIsland_CapeBrink_House_Text_YouRejectIt::
	.string "Gaah! You reject it?\n"
	.string "No, I won’t be dissuaded!$"

