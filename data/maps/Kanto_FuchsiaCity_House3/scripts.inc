Kanto_FuchsiaCity_House3_MapScripts::
	.byte 0

@ Moemon Kai: FireRed's MOVE DELETER (pokefirered FuchsiaCity_House3), on Emerald's specials as LILYCOVE's; Quetzal's
@ "delete moves on your summary screen" (which this game can't) is gone
@ Kanto_FuchsiaCity_House3 object 1 (local id 1, gfx 0x3EE6) at 4,4
Kanto_FuchsiaCity_House3_EventScript_841DA41::
	lock
	faceplayer
	msgbox Kanto_FuchsiaCity_House3_Text_WouldYouLikeToForgetMove, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, Kanto_FuchsiaCity_House3_EventScript_ChooseMonForMoveDeleter
	goto Kanto_FuchsiaCity_House3_EventScript_CancelForgetMove
	end

Kanto_FuchsiaCity_House3_EventScript_ChooseMonForMoveDeleter::
	msgbox Kanto_FuchsiaCity_House3_Text_WhichMonShouldForgetMove
	special ChoosePartyMon
	waitstate
	goto_if_ge VAR_0x8004, PARTY_SIZE, Kanto_FuchsiaCity_House3_EventScript_CancelForgetMove
	special IsSelectedMonEgg
	goto_if_eq VAR_RESULT, TRUE, Kanto_FuchsiaCity_House3_EventScript_CantForgetMoveEgg
	special GetNumMovesSelectedMonHas
	goto_if_eq VAR_RESULT, 1, Kanto_FuchsiaCity_House3_EventScript_CantForgetOnlyMove
	msgbox Kanto_FuchsiaCity_House3_Text_WhichMoveShouldBeForgotten
	fadescreen FADE_TO_BLACK
	special MoveDeleterChooseMoveToForget
	fadescreen FADE_FROM_BLACK
	goto_if_eq VAR_0x8005, MAX_MON_MOVES, Kanto_FuchsiaCity_House3_EventScript_ChooseMonForMoveDeleter
	special BufferMoveDeleterNicknameAndMove
	msgbox Kanto_FuchsiaCity_House3_Text_MonsMoveShouldBeForgotten, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, Kanto_FuchsiaCity_House3_EventScript_ForgetMove
	goto Kanto_FuchsiaCity_House3_EventScript_CancelForgetMove
	end

Kanto_FuchsiaCity_House3_EventScript_ForgetMove::
	special MoveDeleterForgetMove
	playfanfare MUS_MOVE_DELETED
	waitfanfare
	msgbox Kanto_FuchsiaCity_House3_Text_MonHasForgottenMoveCompletely
	release
	end

Kanto_FuchsiaCity_House3_EventScript_CantForgetOnlyMove::
	special BufferMoveDeleterNicknameAndMove
	msgbox Kanto_FuchsiaCity_House3_Text_MonOnlyKnowsOneMove
	release
	end

Kanto_FuchsiaCity_House3_EventScript_CantForgetMoveEgg::
	msgbox Kanto_FuchsiaCity_House3_Text_NoEggShouldKnowMoves
	release
	end

Kanto_FuchsiaCity_House3_EventScript_CancelForgetMove::
	msgbox Kanto_FuchsiaCity_House3_Text_ComeAgainToForgetOtherMoves
	release
	end

Kanto_FuchsiaCity_House3_Text_ComeAgainToForgetOtherMoves::
	.string "Come again if there are other\n"
	.string "moves to be forgotten.$"

Kanto_FuchsiaCity_House3_Text_MonHasForgottenMoveCompletely::
	.string "It worked to perfection!\p"
	.string "{STR_VAR_1} has forgotten\n"
	.string "{STR_VAR_2} completely.$"

Kanto_FuchsiaCity_House3_Text_MonOnlyKnowsOneMove::
	.string "{STR_VAR_1} seems to know only one\n"
	.string "move…$"

Kanto_FuchsiaCity_House3_Text_MonsMoveShouldBeForgotten::
	.string "Hm! {STR_VAR_1}’s {STR_VAR_2}?\n"
	.string "That move should be forgotten?$"

Kanto_FuchsiaCity_House3_Text_NoEggShouldKnowMoves::
	.string "What?\n"
	.string "No EGG should know any moves.$"

Kanto_FuchsiaCity_House3_Text_WhichMonShouldForgetMove::
	.string "Which MOéMON should forget a\n"
	.string "move?$"

Kanto_FuchsiaCity_House3_Text_WhichMoveShouldBeForgotten::
	.string "Which move should be forgotten?$"

Kanto_FuchsiaCity_House3_Text_WouldYouLikeToForgetMove::
	.string "Uh…\n"
	.string "Oh, yes, I’m the MOVE DELETER.\p"
	.string "I can make MOéMON forget their\n"
	.string "moves.\p"
	.string "Would you like me to do that?$"

