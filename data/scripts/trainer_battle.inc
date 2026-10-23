EventScript_StartTrainerApproach::
	selectapproachingtrainer
	lockfortrainer
EventScript_TrainerApproach::
	special PlayTrainerEncounterMusic
	special DoTrainerApproach
	@ Moemon Co-op: a trainer a friend is waiting on (for a partner) spotted us: we join their battle
	special MP_Coop_TryJoinHeldTrainer
	goto_if_eq VAR_RESULT, FALSE, EventScript_ShowTrainerIntroMsg
	waitstate
	specialvar VAR_RESULT, MP_Coop_TakeJoinedOther
	goto_if_eq VAR_RESULT, 1, EventScript_MpCoopAfterTrainerBattle
	goto_if_eq VAR_RESULT, 2, EventScript_MpCoopJoinedFriend
	goto EventScript_ShowTrainerIntroMsg

EventScript_TryDoNormalTrainerBattle::
	lock
	faceplayer
	applymovement VAR_LAST_TALKED, Movement_RevealTrainer
	waitmovement 0
	specialvar VAR_RESULT, GetTrainerFlag
	goto_if_ne VAR_RESULT, FALSE, EventScript_NoNormalTrainerBattle
	special PlayTrainerEncounterMusic
	special SetTrainerFacingDirection
	goto EventScript_ShowTrainerIntroMsg

EventScript_NoNormalTrainerBattle::
	gotopostbattlescript

EventScript_TryDoDoubleTrainerBattle::
	lock
	faceplayer
	call EventScript_RevealTrainer
	specialvar VAR_RESULT, GetTrainerFlag
	goto_if_ne VAR_RESULT, FALSE, EventScript_NoDoubleTrainerBattle
	special HasEnoughMonsForDoubleBattle
	goto_if_ne VAR_RESULT, PLAYER_HAS_TWO_USABLE_MONS, EventScript_NotEnoughMonsForDoubleBattle
	special PlayTrainerEncounterMusic
	special SetTrainerFacingDirection
	goto EventScript_ShowTrainerIntroMsg

EventScript_NotEnoughMonsForDoubleBattle::
	special ShowTrainerCantBattleSpeech
	waitmessage
	waitbuttonpress
	release
	end

EventScript_NoDoubleTrainerBattle::
	gotopostbattlescript

EventScript_DoNoIntroTrainerBattle::
	applymovement VAR_LAST_TALKED, Movement_RevealTrainer
	waitmovement 0
	special PlayTrainerEncounterMusic
	dotrainerbattle
	gotopostbattlescript

EventScript_TryDoRematchBattle::
	call EventScript_RevealTrainer
	specialvar VAR_RESULT, IsTrainerReadyForRematch
	goto_if_eq VAR_RESULT, FALSE, EventScript_NoRematchTrainerBattle
	special PlayTrainerEncounterMusic
	special SetTrainerFacingDirection
	special ShowTrainerIntroSpeech
	waitmessage
	waitbuttonpress
	special BattleSetup_StartRematchBattle
	releaseall
	end

EventScript_NoRematchTrainerBattle::
	gotopostbattlescript

EventScript_TryDoDoubleRematchBattle::
	specialvar VAR_RESULT, IsTrainerReadyForRematch
	goto_if_eq VAR_RESULT, FALSE, EventScript_NoDoubleRematchTrainerBattle
	special HasEnoughMonsForDoubleBattle
	goto_if_ne VAR_RESULT, PLAYER_HAS_TWO_USABLE_MONS, EventScript_NotEnoughMonsForDoubleRematchBattle
	special PlayTrainerEncounterMusic
	special SetTrainerFacingDirection
	special ShowTrainerIntroSpeech
	waitmessage
	waitbuttonpress
	special BattleSetup_StartRematchBattle
	releaseall
	end

EventScript_NoDoubleRematchTrainerBattle::
	gotopostbattlescript

EventScript_NotEnoughMonsForDoubleRematchBattle::
	special ShowTrainerCantBattleSpeech
	waitmessage
	waitbuttonpress
	release
	end

EventScript_RevealTrainer::
	applymovement VAR_LAST_TALKED, Movement_RevealTrainer
	waitmovement 0
	return

Movement_RevealTrainer:
	reveal_trainer
	step_end

EventScript_ShowTrainerIntroMsg::
	special ShowTrainerIntroSpeech
	waitmessage
	waitbuttonpress
	@ Moemon Co-op: a player who walked up may ask to join for a Double Battle (or we join a friend waiting on the
	@ same trainer: the battle was theirs, ours is over)
	special MP_Coop_JoinCheck
	waitstate
	specialvar VAR_RESULT, MP_Coop_TakeJoinedOther
	goto_if_eq VAR_RESULT, 1, EventScript_MpCoopAfterTrainerBattle
	goto_if_eq VAR_RESULT, 2, EventScript_MpCoopJoinedFriend
	special TryPrepareSecondApproachingTrainer
	goto_if_eq VAR_RESULT, TRUE, EventScript_TrainerApproach
	goto EventScript_DoTrainerBattle

@ Moemon Co-op: we were in a friend's battle against this trainer and it didn't end in a win (called off)
EventScript_MpCoopJoinedFriend::
	releaseall
	end

EventScript_DoTrainerBattle::
	dotrainerbattle
@ Moemon Co-op: also after a friend's battle against this trainer, won (MP_EndJoinedTrainerBattle)
EventScript_MpCoopAfterTrainerBattle::
	/* Below battle mode check only needed in FRLG*/
	specialvar VAR_RESULT, GetTrainerBattleMode
	goto_if_eq VAR_RESULT, TRAINER_BATTLE_SINGLE, EventScript_EndTrainerBattle
	goto_if_eq VAR_RESULT, TRAINER_BATTLE_CONTINUE_SCRIPT, EventScript_EndTrainerBattle
	goto_if_eq VAR_RESULT, TRAINER_BATTLE_CONTINUE_SCRIPT_NO_MUSIC, EventScript_EndTrainerBattle
	goto_if_eq VAR_RESULT, TRAINER_BATTLE_CONTINUE_SCRIPT_DOUBLE, EventScript_EndTrainerBattle
	goto_if_eq VAR_RESULT, TRAINER_BATTLE_CONTINUE_SCRIPT_DOUBLE_NO_MUSIC, EventScript_EndTrainerBattle
EventScript_EndTrainerBattle::
	gotobeatenscript
	releaseall
	end

Std_MsgboxAutoclose::
	message NULL
	waitmessage
	waitbuttonpress
	release
	return

@ Moemon Co-op: Pokémon that couldn't learn a move during a co-op battle
EventScript_MpCoopLearnMove::
	lockall
EventScript_MpCoopLearnMoveNext::
	special MP_Coop_PrepareLearnMove
	goto_if_eq VAR_RESULT, FALSE, EventScript_MpCoopLearnMoveEnd
	msgbox gStringVar4, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EventScript_MpCoopLearnMoveNext
	special TeachMoveRelearnerMove
	waitstate
	goto EventScript_MpCoopLearnMoveNext
EventScript_MpCoopLearnMoveEnd::
	releaseall
	end
