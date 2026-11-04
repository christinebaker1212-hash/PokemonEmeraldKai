Johto_MoveDeletersHouse_MapScripts::
	.byte 0

@ Moemon Kai: Crystal's MOVE DELETER (pokecrystal MoveDeletersHouse; Quetzal left the house empty), on Emerald's
@ specials as KANTO's FUCHSIA one (docs/REGIONS_AUDIT.md 147)
Johto_MoveDeletersHouse_EventScript_MoveDeleter::
	lock
	faceplayer
	msgbox Johto_MoveDeletersHouse_Text_WouldYouLikeToForgetMove, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, Johto_MoveDeletersHouse_EventScript_ChooseMonForMoveDeleter
	goto Johto_MoveDeletersHouse_EventScript_CancelForgetMove
	end

Johto_MoveDeletersHouse_EventScript_ChooseMonForMoveDeleter::
	msgbox Johto_MoveDeletersHouse_Text_WhichMonShouldForgetMove
	special ChoosePartyMon
	waitstate
	goto_if_ge VAR_0x8004, PARTY_SIZE, Johto_MoveDeletersHouse_EventScript_CancelForgetMove
	special IsSelectedMonEgg
	goto_if_eq VAR_RESULT, TRUE, Johto_MoveDeletersHouse_EventScript_CantForgetMoveEgg
	special GetNumMovesSelectedMonHas
	goto_if_eq VAR_RESULT, 1, Johto_MoveDeletersHouse_EventScript_CantForgetOnlyMove
	msgbox Johto_MoveDeletersHouse_Text_WhichMoveShouldBeForgotten
	fadescreen FADE_TO_BLACK
	special MoveDeleterChooseMoveToForget
	fadescreen FADE_FROM_BLACK
	goto_if_eq VAR_0x8005, MAX_MON_MOVES, Johto_MoveDeletersHouse_EventScript_ChooseMonForMoveDeleter
	special BufferMoveDeleterNicknameAndMove
	msgbox Johto_MoveDeletersHouse_Text_MonsMoveShouldBeForgotten, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, Johto_MoveDeletersHouse_EventScript_ForgetMove
	goto Johto_MoveDeletersHouse_EventScript_CancelForgetMove
	end

Johto_MoveDeletersHouse_EventScript_ForgetMove::
	special MoveDeleterForgetMove
	playfanfare MUS_MOVE_DELETED
	waitfanfare
	msgbox Johto_MoveDeletersHouse_Text_MonHasForgottenMoveCompletely
	release
	end

Johto_MoveDeletersHouse_EventScript_CantForgetOnlyMove::
	special BufferMoveDeleterNicknameAndMove
	msgbox Johto_MoveDeletersHouse_Text_MonOnlyKnowsOneMove
	release
	end

Johto_MoveDeletersHouse_EventScript_CantForgetMoveEgg::
	msgbox Johto_MoveDeletersHouse_Text_NoEggShouldKnowMoves
	release
	end

Johto_MoveDeletersHouse_EventScript_CancelForgetMove::
	msgbox Johto_MoveDeletersHouse_Text_ComeAgainToForgetOtherMoves
	release
	end

Johto_MoveDeletersHouse_Text_ComeAgainToForgetOtherMoves::
	.string "No? Come visit me again.$"

Johto_MoveDeletersHouse_Text_MonHasForgottenMoveCompletely::
	.string "Done! Your MOéMON forgot the move.$"

Johto_MoveDeletersHouse_Text_MonOnlyKnowsOneMove::
	.string "That MOéMON knows only one move.$"

Johto_MoveDeletersHouse_Text_MonsMoveShouldBeForgotten::
	.string "Oh, make it forget {STR_VAR_2}?$"

Johto_MoveDeletersHouse_Text_NoEggShouldKnowMoves::
	.string "An EGG doesn’t know any moves!$"

Johto_MoveDeletersHouse_Text_WhichMonShouldForgetMove::
	.string "Which MOéMON?$"

Johto_MoveDeletersHouse_Text_WhichMoveShouldBeForgotten::
	.string "Which move should it forget, then?$"

Johto_MoveDeletersHouse_Text_WouldYouLikeToForgetMove::
	.string "Um… Oh, yes, I’m the MOVE DELETER.\p"
	.string "I can make MOéMON forget moves.\p"
	.string "Shall I make a MOéMON forget?$"
