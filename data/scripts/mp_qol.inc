@ Moemon Co-op QoL: the Move Tutor's service from START > CO-OP.
@ Same rules as in Fallarbor: one HEART SCALE per move.
EventScript_MpMoveRelearner::
	lockall
	checkitem ITEM_HEART_SCALE
	goto_if_eq VAR_RESULT, FALSE, EventScript_MpMoveRelearner_NoScale
	msgbox Text_MpMoveRelearner_Ask, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EventScript_MpMoveRelearner_End
EventScript_MpMoveRelearner_ChooseMon::
	msgbox FallarborTown_MoveRelearnersHouse_Text_TutorWhichMon, MSGBOX_DEFAULT
	special ChooseMonForMoveRelearner
	goto_if_eq VAR_0x8004, PARTY_NOTHING_CHOSEN, EventScript_MpMoveRelearner_End
	special IsSelectedMonEgg
	goto_if_eq VAR_RESULT, TRUE, EventScript_MpMoveRelearner_Egg
	goto_if_eq VAR_0x8005, 0, EventScript_MpMoveRelearner_NoMove
	msgbox FallarborTown_MoveRelearnersHouse_Text_TeachWhichMove, MSGBOX_DEFAULT
	special TeachMoveRelearnerMove
	goto_if_eq VAR_0x8004, 0, EventScript_MpMoveRelearner_ChooseMon
	removeitem ITEM_HEART_SCALE
	msgbox Text_MpMoveRelearner_UsedScale, MSGBOX_DEFAULT
EventScript_MpMoveRelearner_End::
	releaseall
	end

EventScript_MpMoveRelearner_NoMove::
	msgbox FallarborTown_MoveRelearnersHouse_Text_DontHaveMoveToTeachPokemon, MSGBOX_DEFAULT
	goto EventScript_MpMoveRelearner_ChooseMon

EventScript_MpMoveRelearner_Egg::
	msgbox FallarborTown_MoveRelearnersHouse_Text_CantTeachEgg, MSGBOX_DEFAULT
	goto EventScript_MpMoveRelearner_ChooseMon

EventScript_MpMoveRelearner_NoScale::
	msgbox Text_MpMoveRelearner_NoScale, MSGBOX_DEFAULT
	releaseall
	end

Text_MpMoveRelearner_Ask:
	.string "Use a HEART SCALE to remind a\n"
	.string "MOéMON of a move it forgot?$"

Text_MpMoveRelearner_UsedScale:
	.string "{PLAYER} used up a HEART SCALE.$"

Text_MpMoveRelearner_NoScale:
	.string "The MOVE TUTOR's service needs a\n"
	.string "HEART SCALE. You don't have any…$"

@ Moemon Co-op: walking into a friend's Secret Base from the CO-OP menu
@ (mp_linkx.c has copied it into our list and picked the exit).
EventScript_MpVisitSecretBase::
	lockall
	setvar VAR_INIT_SECRET_BASE, 1
	playse SE_EXIT
	clearflag FLAG_HIDE_SECRET_BASE_TRAINER
	special EnterSecretBase
	special MP_SecretBase_FixExit
	setvar VAR_SECRET_BASE_INITIALIZED, 0
	waitstate
	end

@ Moemon Co-op: BERRY BLENDER with friends (mp_minigame.c sets VAR_0x8004)
EventScript_MpBerryBlender::
	lockall
	fadescreen FADE_TO_BLACK
	special DoBerryBlending
	releaseall
	end

@ Moemon Co-op: BERRY CRUSH with friends (mp_minigame.c)
EventScript_MpBerryCrush::
	lockall
	fadescreen FADE_TO_BLACK
	special MP_StartBerryCrush
	releaseall
	end
