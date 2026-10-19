@ Moemon Co-op: team rematches with gym leaders (src/mp/mp_rematch.c).
@ Offered when you talk to a leader you have already beaten while a friend is
@ on the same map. Your friend joins by standing next to you, facing you.

MpRematch_EventScript_Roxanne::
	lock
	faceplayer
	msgbox MpRematch_Text_Offer, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, MpRematch_EventScript_Declined
	specialvar VAR_RESULT, MP_Rematch_IsEndgame
	goto_if_eq VAR_RESULT, TRUE, MpRematch_EventScript_RoxanneEndgame
	cleartrainerflag TRAINER_ROXANNE_2
	trainerbattle_single TRAINER_ROXANNE_2, MpRematch_Text_Intro, MpRematch_Text_Defeat, MpRematch_EventScript_Won, FALSE
	release
	end
MpRematch_EventScript_RoxanneEndgame::
	cleartrainerflag TRAINER_ROXANNE_5
	trainerbattle_single TRAINER_ROXANNE_5, MpRematch_Text_Intro, MpRematch_Text_Defeat, MpRematch_EventScript_Won, FALSE
	release
	end

MpRematch_EventScript_Brawly::
	lock
	faceplayer
	msgbox MpRematch_Text_Offer, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, MpRematch_EventScript_Declined
	specialvar VAR_RESULT, MP_Rematch_IsEndgame
	goto_if_eq VAR_RESULT, TRUE, MpRematch_EventScript_BrawlyEndgame
	cleartrainerflag TRAINER_BRAWLY_2
	trainerbattle_single TRAINER_BRAWLY_2, MpRematch_Text_Intro, MpRematch_Text_Defeat, MpRematch_EventScript_Won, FALSE
	release
	end
MpRematch_EventScript_BrawlyEndgame::
	cleartrainerflag TRAINER_BRAWLY_5
	trainerbattle_single TRAINER_BRAWLY_5, MpRematch_Text_Intro, MpRematch_Text_Defeat, MpRematch_EventScript_Won, FALSE
	release
	end

MpRematch_EventScript_Wattson::
	lock
	faceplayer
	msgbox MpRematch_Text_Offer, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, MpRematch_EventScript_Declined
	specialvar VAR_RESULT, MP_Rematch_IsEndgame
	goto_if_eq VAR_RESULT, TRUE, MpRematch_EventScript_WattsonEndgame
	cleartrainerflag TRAINER_WATTSON_2
	trainerbattle_single TRAINER_WATTSON_2, MpRematch_Text_Intro, MpRematch_Text_Defeat, MpRematch_EventScript_Won, FALSE
	release
	end
MpRematch_EventScript_WattsonEndgame::
	cleartrainerflag TRAINER_WATTSON_5
	trainerbattle_single TRAINER_WATTSON_5, MpRematch_Text_Intro, MpRematch_Text_Defeat, MpRematch_EventScript_Won, FALSE
	release
	end

MpRematch_EventScript_Flannery::
	lock
	faceplayer
	msgbox MpRematch_Text_Offer, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, MpRematch_EventScript_Declined
	specialvar VAR_RESULT, MP_Rematch_IsEndgame
	goto_if_eq VAR_RESULT, TRUE, MpRematch_EventScript_FlanneryEndgame
	cleartrainerflag TRAINER_FLANNERY_2
	trainerbattle_single TRAINER_FLANNERY_2, MpRematch_Text_Intro, MpRematch_Text_Defeat, MpRematch_EventScript_Won, FALSE
	release
	end
MpRematch_EventScript_FlanneryEndgame::
	cleartrainerflag TRAINER_FLANNERY_5
	trainerbattle_single TRAINER_FLANNERY_5, MpRematch_Text_Intro, MpRematch_Text_Defeat, MpRematch_EventScript_Won, FALSE
	release
	end

MpRematch_EventScript_Norman::
	lock
	faceplayer
	msgbox MpRematch_Text_Offer, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, MpRematch_EventScript_Declined
	specialvar VAR_RESULT, MP_Rematch_IsEndgame
	goto_if_eq VAR_RESULT, TRUE, MpRematch_EventScript_NormanEndgame
	cleartrainerflag TRAINER_NORMAN_2
	trainerbattle_single TRAINER_NORMAN_2, MpRematch_Text_Intro, MpRematch_Text_Defeat, MpRematch_EventScript_Won, FALSE
	release
	end
MpRematch_EventScript_NormanEndgame::
	cleartrainerflag TRAINER_NORMAN_5
	trainerbattle_single TRAINER_NORMAN_5, MpRematch_Text_Intro, MpRematch_Text_Defeat, MpRematch_EventScript_Won, FALSE
	release
	end

MpRematch_EventScript_Winona::
	lock
	faceplayer
	msgbox MpRematch_Text_Offer, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, MpRematch_EventScript_Declined
	specialvar VAR_RESULT, MP_Rematch_IsEndgame
	goto_if_eq VAR_RESULT, TRUE, MpRematch_EventScript_WinonaEndgame
	cleartrainerflag TRAINER_WINONA_2
	trainerbattle_single TRAINER_WINONA_2, MpRematch_Text_Intro, MpRematch_Text_Defeat, MpRematch_EventScript_Won, FALSE
	release
	end
MpRematch_EventScript_WinonaEndgame::
	cleartrainerflag TRAINER_WINONA_5
	trainerbattle_single TRAINER_WINONA_5, MpRematch_Text_Intro, MpRematch_Text_Defeat, MpRematch_EventScript_Won, FALSE
	release
	end

MpRematch_EventScript_TateAndLiza::
	lock
	faceplayer
	msgbox MpRematch_Text_Offer, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, MpRematch_EventScript_Declined
	specialvar VAR_RESULT, MP_Rematch_IsEndgame
	goto_if_eq VAR_RESULT, TRUE, MpRematch_EventScript_TateAndLizaEndgame
	cleartrainerflag TRAINER_TATE_AND_LIZA_2
	trainerbattle_single TRAINER_TATE_AND_LIZA_2, MpRematch_Text_Intro, MpRematch_Text_Defeat, MpRematch_EventScript_Won, FALSE
	release
	end
MpRematch_EventScript_TateAndLizaEndgame::
	cleartrainerflag TRAINER_TATE_AND_LIZA_5
	trainerbattle_single TRAINER_TATE_AND_LIZA_5, MpRematch_Text_Intro, MpRematch_Text_Defeat, MpRematch_EventScript_Won, FALSE
	release
	end

MpRematch_EventScript_Juan::
	lock
	faceplayer
	msgbox MpRematch_Text_Offer, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, MpRematch_EventScript_Declined
	specialvar VAR_RESULT, MP_Rematch_IsEndgame
	goto_if_eq VAR_RESULT, TRUE, MpRematch_EventScript_JuanEndgame
	cleartrainerflag TRAINER_JUAN_2
	trainerbattle_single TRAINER_JUAN_2, MpRematch_Text_Intro, MpRematch_Text_Defeat, MpRematch_EventScript_Won, FALSE
	release
	end
MpRematch_EventScript_JuanEndgame::
	cleartrainerflag TRAINER_JUAN_5
	trainerbattle_single TRAINER_JUAN_5, MpRematch_Text_Intro, MpRematch_Text_Defeat, MpRematch_EventScript_Won, FALSE
	release
	end

MpRematch_EventScript_Declined::
	msgbox MpRematch_Text_Declined, MSGBOX_DEFAULT
	release
	end

MpRematch_EventScript_Won::
	special MP_Rematch_Reward
	msgbox MpRematch_Text_Won, MSGBOX_DEFAULT
	release
	end

MpRematch_Text_Offer:
	.string "You're back, and with a friend!\n"
	.string "How about a team rematch?\p"
	.string "I'll use my strongest team. Your friend\n"
	.string "joins by standing next to you, facing you.$"

MpRematch_Text_Intro:
	.string "Then show me your teamwork!$"

MpRematch_Text_Defeat:
	.string "Incredible teamwork!\n"
	.string "You two are a real team.$"

MpRematch_Text_Won:
	.string "Take these as a reward for both of you.\n"
	.string "Come back for another rematch anytime!$"

MpRematch_Text_Declined:
	.string "Come back with your friend anytime!$"
