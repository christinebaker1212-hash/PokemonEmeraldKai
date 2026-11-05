@ Moemon Kai: FireRed's POKéMON TRAINER FAN CLUB (pokefirered data/maps/SaffronCity_PokemonTrainerFanClub; audit case 88):
@ its fan record is src/mp/mp_fanclub.c, in KANTO's vars. The POKéMON JOURNAL on the rack is there once KANTO's LEAGUE is beaten.
Kanto_SaffronCity_PokemonTrainerFanClub_MapScripts::
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Kanto_SaffronCity_PokemonTrainerFanClub_OnFrame
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_SaffronCity_PokemonTrainerFanClub_OnTransition
	.byte 0

Kanto_SaffronCity_PokemonTrainerFanClub_OnFrame::
	map_script_2 VAR_KANTO_FANCLUB_SCENE, 1, Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_MeetFirstFans
	.2byte 0

Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_MeetFirstFans::
	lockall
	textcolor 1
	applymovement 1, Kanto_SaffronCity_PokemonTrainerFanClub_Movement_FaceDown
	waitmovement 0
	playse SE_PIN
	applymovement 1, Kanto_SaffronCity_PokemonTrainerFanClub_Movement_ExclamationMark
	waitmovement 0
	applymovement 1, Kanto_SaffronCity_PokemonTrainerFanClub_Movement_Delay48
	waitmovement 0
	msgbox Kanto_SaffronCity_PokemonTrainerFanClub_Text_HuhYou, MSGBOX_DEFAULT
	closemessage
	applymovement 1, Kanto_SaffronCity_PokemonTrainerFanClub_Movement_FanApproachPlayer
	waitmovement 0
	applymovement OBJ_EVENT_ID_PLAYER, Kanto_SaffronCity_PokemonTrainerFanClub_Movement_FaceRight
	waitmovement 0
	msgbox Kanto_SaffronCity_PokemonTrainerFanClub_Text_YourePlayerWereYourFansNow, MSGBOX_DEFAULT
	setvar VAR_KANTO_FANCLUB_SCENE, 2
	releaseall
	end

Kanto_SaffronCity_PokemonTrainerFanClub_Movement_FanApproachPlayer::
	delay_16
	delay_8
	walk_down
	delay_16
	walk_down
	delay_8
	walk_left
	step_end

Kanto_SaffronCity_PokemonTrainerFanClub_OnTransition::
	special MP_FanClub_Enter
	goto_if_eq VAR_KANTO_FANCLUB_SCENE, 1, Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_SetMemberPosForFirstMeeting
	goto_if_eq VAR_KANTO_FANCLUB_SCENE, 2, Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_UpdateFanMemberPositions
	end

Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_UpdateFanMemberPositions::
	special MP_FanClub_TryLoseFansFromPlayTime
	setvar VAR_0x8004, 0
	specialvar VAR_RESULT, MP_FanClub_IsFan
	call_if_eq VAR_RESULT, FALSE, Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_MoveMember1ToFarTable
	setvar VAR_0x8004, 1
	specialvar VAR_RESULT, MP_FanClub_IsFan
	call_if_eq VAR_RESULT, FALSE, Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_MoveMember2ToFarTable
	setvar VAR_0x8004, 2
	specialvar VAR_RESULT, MP_FanClub_IsFan
	call_if_eq VAR_RESULT, FALSE, Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_MoveMember3ToFarTable
	setvar VAR_0x8004, 3
	specialvar VAR_RESULT, MP_FanClub_IsFan
	call_if_eq VAR_RESULT, FALSE, Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_MoveMember4ToFarTable
	setvar VAR_0x8004, 4
	specialvar VAR_RESULT, MP_FanClub_IsFan
	call_if_eq VAR_RESULT, FALSE, Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_MoveMember5ToFarTable
	setvar VAR_0x8004, 5
	specialvar VAR_RESULT, MP_FanClub_IsFan
	call_if_eq VAR_RESULT, FALSE, Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_MoveMember6ToFarTable
	setvar VAR_0x8004, 6
	specialvar VAR_RESULT, MP_FanClub_IsFan
	call_if_eq VAR_RESULT, FALSE, Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_MoveMember7ToFarTable
	setvar VAR_0x8004, 7
	specialvar VAR_RESULT, MP_FanClub_IsFan
	call_if_eq VAR_RESULT, FALSE, Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_MoveMember8ToFarTable
	end

Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_MoveMember1ToFarTable::
	setobjectxyperm 1, 5, 2
	setobjectmovementtype 1, MOVEMENT_TYPE_FACE_UP
	return

Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_MoveMember2ToFarTable::
	setobjectxyperm 3, 7, 4
	setobjectmovementtype 3, MOVEMENT_TYPE_FACE_LEFT
	return

Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_MoveMember3ToFarTable::
	setobjectxyperm 4, 2, 2
	setobjectmovementtype 4, MOVEMENT_TYPE_FACE_UP
	return

Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_MoveMember4ToFarTable::
	setobjectxyperm 2, 3, 4
	setobjectmovementtype 2, MOVEMENT_TYPE_FACE_RIGHT
	return

Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_MoveMember5ToFarTable::
	setobjectxyperm 6, 4, 6
	setobjectmovementtype 6, MOVEMENT_TYPE_FACE_UP
	return

Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_MoveMember6ToFarTable::
	setobjectxyperm 5, 10, 3
	setobjectmovementtype 5, MOVEMENT_TYPE_FACE_UP
	return

Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_MoveMember7ToFarTable::
	setobjectxyperm 7, 7, 5
	setobjectmovementtype 7, MOVEMENT_TYPE_FACE_LEFT
	return

Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_MoveMember8ToFarTable::
	setobjectxyperm 8, 9, 6
	setobjectmovementtype 8, MOVEMENT_TYPE_FACE_LEFT
	return

Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_SetMemberPosForFirstMeeting::
	call Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_MoveMember4ToFarTable
	call Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_MoveMember5ToFarTable
	call Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_MoveMember6ToFarTable
	call Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_MoveMember7ToFarTable
	call Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_MoveMember8ToFarTable
	end

Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_CrushGirl::
	lock
	faceplayer
	setvar VAR_0x8004, 0
	special MP_FanClub_BufferTrainerName
	goto_if_eq VAR_KANTO_FANCLUB_SCENE, 0, Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_CrushGirlPlayerNotChampion
	specialvar VAR_RESULT, MP_FanClub_IsFan
	goto_if_eq VAR_RESULT, TRUE, Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_CrushGirlPlayersFan
	specialvar VAR_RESULT, MP_FanClub_NumFans
	goto_if_eq VAR_RESULT, 7, Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_CrushGirlOnlyNonFan
	msgbox Kanto_SaffronCity_PokemonTrainerFanClub_Text_WasYourFanNotAnymore, MSGBOX_DEFAULT
	release
	end

Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_CrushGirlPlayersFan::
	specialvar VAR_RESULT, MP_FanClub_NumFans
	goto_if_eq VAR_RESULT, 1, Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_CrushGirlOnlyFan
	msgbox Kanto_SaffronCity_PokemonTrainerFanClub_Text_AlwaysCheerForYou, MSGBOX_DEFAULT
	release
	end

Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_CrushGirlOnlyFan::
	msgbox Kanto_SaffronCity_PokemonTrainerFanClub_Text_IllAlwaysBelieveInYou, MSGBOX_DEFAULT
	release
	end

Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_CrushGirlOnlyNonFan::
	msgbox Kanto_SaffronCity_PokemonTrainerFanClub_Text_EveryonesYourFanButICantBe, MSGBOX_DEFAULT
	release
	end

Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_CrushGirlPlayerNotChampion::
	msgbox Kanto_SaffronCity_PokemonTrainerFanClub_Text_TrainersCoolWhenBattling, MSGBOX_DEFAULT
	release
	end

Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_LittleGirl::
	lock
	faceplayer
	setvar VAR_0x8004, 3
	special MP_FanClub_BufferTrainerName
	goto_if_eq VAR_KANTO_FANCLUB_SCENE, 0, Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_LittleGirlPlayerNotChampion
	specialvar VAR_RESULT, MP_FanClub_IsFan
	goto_if_eq VAR_RESULT, TRUE, Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_LittleGirlPlayersFan
	specialvar VAR_RESULT, MP_FanClub_NumFans
	goto_if_eq VAR_RESULT, 7, Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_LittleGirlOnlyNonFan
	msgbox Kanto_SaffronCity_PokemonTrainerFanClub_Text_WantToBeLikeSabrina, MSGBOX_DEFAULT
	release
	end

Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_LittleGirlPlayersFan::
	specialvar VAR_RESULT, MP_FanClub_NumFans
	goto_if_eq VAR_RESULT, 1, Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_LittleGirlOnlyFan
	msgbox Kanto_SaffronCity_PokemonTrainerFanClub_Text_WantToBeLikeYouOneDay, MSGBOX_DEFAULT
	release
	end

Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_LittleGirlOnlyFan::
	msgbox Kanto_SaffronCity_PokemonTrainerFanClub_Text_EveryoneButMeStoppedBeingYourFan, MSGBOX_DEFAULT
	release
	end

Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_LittleGirlOnlyNonFan::
	msgbox Kanto_SaffronCity_PokemonTrainerFanClub_Text_WontStopBeingSabrinasFan, MSGBOX_DEFAULT
	release
	end

Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_LittleGirlPlayerNotChampion::
	msgbox Kanto_SaffronCity_PokemonTrainerFanClub_Text_CanIBecomeCoolTrainerOneDay, MSGBOX_DEFAULT
	release
	end

Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_Youngster::
	lock
	faceplayer
	setvar VAR_0x8004, 1
	special MP_FanClub_BufferTrainerName
	goto_if_eq VAR_KANTO_FANCLUB_SCENE, 0, Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_YoungsterPlayerNotChampion
	specialvar VAR_RESULT, MP_FanClub_IsFan
	goto_if_eq VAR_RESULT, TRUE, Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_YoungsterPlayersFan
	specialvar VAR_RESULT, MP_FanClub_NumFans
	goto_if_eq VAR_RESULT, 7, Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_YoungsterOnlyNonFan
	msgbox Kanto_SaffronCity_PokemonTrainerFanClub_Text_BrocksMyHero, MSGBOX_DEFAULT
	release
	end

Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_YoungsterPlayersFan::
	specialvar VAR_RESULT, MP_FanClub_NumFans
	goto_if_eq VAR_RESULT, 1, Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_YoungsterOnlyFan
	msgbox Kanto_SaffronCity_PokemonTrainerFanClub_Text_CanYouAutographShorts, MSGBOX_DEFAULT
	release
	end

Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_YoungsterOnlyFan::
	msgbox Kanto_SaffronCity_PokemonTrainerFanClub_Text_CountingOnYou, MSGBOX_DEFAULT
	release
	end

Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_YoungsterOnlyNonFan::
	msgbox Kanto_SaffronCity_PokemonTrainerFanClub_Text_BrocksFanToBitterEnd, MSGBOX_DEFAULT
	release
	end

Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_YoungsterPlayerNotChampion::
	msgbox Kanto_SaffronCity_PokemonTrainerFanClub_Text_BrocksLastWordOnCool, MSGBOX_DEFAULT
	release
	end

Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_Gentleman::
	lock
	faceplayer
	setvar VAR_0x8004, 2
	special MP_FanClub_BufferTrainerName
	goto_if_eq VAR_KANTO_FANCLUB_SCENE, 0, Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_GentlemanPlayerNotChampion
	specialvar VAR_RESULT, MP_FanClub_IsFan
	goto_if_eq VAR_RESULT, TRUE, Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_GentlemanPlayersFan
	specialvar VAR_RESULT, MP_FanClub_NumFans
	goto_if_eq VAR_RESULT, 7, Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_GentlemanOnlyNonFan
	msgbox Kanto_SaffronCity_PokemonTrainerFanClub_Text_HmmAndYouAre, MSGBOX_DEFAULT
	release
	end

Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_GentlemanPlayersFan::
	specialvar VAR_RESULT, MP_FanClub_NumFans
	goto_if_eq VAR_RESULT, 1, Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_GentlemanOnlyFan
	msgbox Kanto_SaffronCity_PokemonTrainerFanClub_Text_HadPleasureOfWatchingYouBattle, MSGBOX_DEFAULT
	release
	end

Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_GentlemanOnlyFan::
	msgbox Kanto_SaffronCity_PokemonTrainerFanClub_Text_NeverSeenTrainerOfYourMagnificence, MSGBOX_DEFAULT
	release
	end

Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_GentlemanOnlyNonFan::
	msgbox Kanto_SaffronCity_PokemonTrainerFanClub_Text_YouveStillAWaysToGo, MSGBOX_DEFAULT
	release
	end

Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_GentlemanPlayerNotChampion::
	msgbox Kanto_SaffronCity_PokemonTrainerFanClub_Text_YoullBeTalkedAboutIfYouKeepWinning, MSGBOX_DEFAULT
	release
	end

Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_Woman::
	lock
	faceplayer
	setvar VAR_0x8004, 5
	special MP_FanClub_BufferTrainerName
	specialvar VAR_RESULT, MP_FanClub_IsFan
	goto_if_eq VAR_RESULT, TRUE, Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_WomanPlayersFan
	specialvar VAR_RESULT, MP_FanClub_NumFans
	goto_if_eq VAR_RESULT, 7, Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_WomanOnlyNonFan
	msgbox Kanto_SaffronCity_PokemonTrainerFanClub_Text_TrainerHasBeenOnFire, MSGBOX_DEFAULT
	release
	end

Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_WomanPlayersFan::
	specialvar VAR_RESULT, MP_FanClub_NumFans
	goto_if_eq VAR_RESULT, 1, Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_WomanOnlyFan
	msgbox Kanto_SaffronCity_PokemonTrainerFanClub_Text_AdoreWayYouBattle, MSGBOX_DEFAULT
	release
	end

Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_WomanOnlyFan::
	msgbox Kanto_SaffronCity_PokemonTrainerFanClub_Text_ImYourNumberOneFan, MSGBOX_DEFAULT
	release
	end

Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_WomanOnlyNonFan::
	msgbox Kanto_SaffronCity_PokemonTrainerFanClub_Text_EveryoneTalksAboutYou, MSGBOX_DEFAULT
	release
	end

Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_Rocker::
	lock
	faceplayer
	setvar VAR_0x8004, 4
	special MP_FanClub_BufferTrainerName
	specialvar VAR_RESULT, MP_FanClub_IsFan
	goto_if_eq VAR_RESULT, TRUE, Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_RockerPlayersFan
	specialvar VAR_RESULT, MP_FanClub_NumFans
	goto_if_eq VAR_RESULT, 7, Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_RockerOnlyNonFan
	msgbox Kanto_SaffronCity_PokemonTrainerFanClub_Text_LoveWayTrainerTalks, MSGBOX_DEFAULT
	release
	end

Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_RockerPlayersFan::
	specialvar VAR_RESULT, MP_FanClub_NumFans
	goto_if_eq VAR_RESULT, 1, Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_RockerOnlyFan
	msgbox Kanto_SaffronCity_PokemonTrainerFanClub_Text_TheWayYouBattleIsCool, MSGBOX_DEFAULT
	release
	end

Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_RockerOnlyFan::
	msgbox Kanto_SaffronCity_PokemonTrainerFanClub_Text_ImOnlyOneLeftOfYourFans, MSGBOX_DEFAULT
	release
	end

Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_RockerOnlyNonFan::
	msgbox Kanto_SaffronCity_PokemonTrainerFanClub_Text_ImLoneTrainerFan, MSGBOX_DEFAULT
	release
	end

Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_Beauty::
	lock
	faceplayer
	setvar VAR_0x8004, 6
	special MP_FanClub_BufferTrainerName
	specialvar VAR_RESULT, MP_FanClub_IsFan
	goto_if_eq VAR_RESULT, TRUE, Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_BeautyPlayersFan
	specialvar VAR_RESULT, MP_FanClub_NumFans
	goto_if_eq VAR_RESULT, 7, Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_BeautyOnlyNonFan
	msgbox Kanto_SaffronCity_PokemonTrainerFanClub_Text_WhyCantOthersSeeMastersDignity, MSGBOX_DEFAULT
	release
	end

Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_BeautyPlayersFan::
	specialvar VAR_RESULT, MP_FanClub_NumFans
	goto_if_eq VAR_RESULT, 1, Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_BeautyOnlyFan
	msgbox Kanto_SaffronCity_PokemonTrainerFanClub_Text_YouReallyAreAmazing, MSGBOX_DEFAULT
	release
	end

Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_BeautyOnlyFan::
	msgbox Kanto_SaffronCity_PokemonTrainerFanClub_Text_ImYourOnlyFan, MSGBOX_DEFAULT
	release
	end

Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_BeautyOnlyNonFan::
	msgbox Kanto_SaffronCity_PokemonTrainerFanClub_Text_BelieveInMasterWithAllMyHeart, MSGBOX_DEFAULT
	release
	end

Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_BlackBelt::
	lock
	faceplayer
	setvar VAR_0x8004, 7
	special MP_FanClub_BufferTrainerName
	specialvar VAR_RESULT, MP_FanClub_IsFan
	goto_if_eq VAR_RESULT, TRUE, Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_BlackBeltPlayersFan
	specialvar VAR_RESULT, MP_FanClub_NumFans
	goto_if_eq VAR_RESULT, 7, Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_BlackBeltOnlyNonFan
	msgbox Kanto_SaffronCity_PokemonTrainerFanClub_Text_OnlyMasterHasMyRespect, MSGBOX_DEFAULT
	release
	end

Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_BlackBeltPlayersFan::
	specialvar VAR_RESULT, MP_FanClub_NumFans
	goto_if_eq VAR_RESULT, 1, Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_BlackBeltOnlyFan
	msgbox Kanto_SaffronCity_PokemonTrainerFanClub_Text_YourBattleStyleIsEducational, MSGBOX_DEFAULT
	release
	end

Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_BlackBeltOnlyFan::
	msgbox Kanto_SaffronCity_PokemonTrainerFanClub_Text_WontStopBeingYourFan, MSGBOX_DEFAULT
	release
	end

Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_BlackBeltOnlyNonFan::
	msgbox Kanto_SaffronCity_PokemonTrainerFanClub_Text_NeverBeFanOfAnyoneButMaster, MSGBOX_DEFAULT
	release
	end

Kanto_SaffronCity_PokemonTrainerFanClub_Movement_FaceDown:
	walk_in_place_faster_down
	step_end

Kanto_SaffronCity_PokemonTrainerFanClub_Movement_FaceRight:
	walk_in_place_faster_right
	step_end

Kanto_SaffronCity_PokemonTrainerFanClub_Movement_ExclamationMark:
	emote_exclamation_mark
	step_end

Kanto_SaffronCity_PokemonTrainerFanClub_Movement_Delay48:
	delay_16
	delay_16
	delay_16
	step_end

Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_PokemonJournal::
	goto_if_unset FLAG_QZ_0x2108, Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_JournalNotYet
	lockall
	textcolor 3
	msgbox Kanto_SaffronCity_PokemonTrainerFanClub_Text_SpecialFeatureBruno, MSGBOX_DEFAULT
	releaseall
Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_JournalNotYet::
	end

Kanto_SaffronCity_PokemonTrainerFanClub_Text_HuhYou::
	.string "Huh?\n"
	.string "You…$"

Kanto_SaffronCity_PokemonTrainerFanClub_Text_YourePlayerWereYourFansNow::
	.string "Are you maybe…\n"
	.string "… …\p"
	.string "You are!\n"
	.string "You are {PLAYER}!\p"
	.string "We were just talking about you!\p"
	.string "We were talking about how an\n"
	.string "incredibly good TRAINER appeared.\p"
	.string "And that someone is you!\n"
	.string "You’re so cool!\p"
	.string "So, we decided that we’ll become\n"
	.string "your FAN CLUB!\p"
	.string "We all hope you’ll keep battling\n"
	.string "and show us how cool you are.\p"
	.string "That way, maybe other people will\n"
	.string "become fans just like me.$"

Kanto_SaffronCity_PokemonTrainerFanClub_Text_WasYourFanNotAnymore::
	.string "I’m sorry.\n"
	.string "I was your fan before.\p"
	.string "But now, you seem to have lost\n"
	.string "the drive you had before.\p"
	.string "Lately, I think {STR_VAR_1} has been\n"
	.string "a lot cooler.$"

Kanto_SaffronCity_PokemonTrainerFanClub_Text_AlwaysCheerForYou::
	.string "I’ll always be cheering for you!\p"
	.string "Go, {PLAYER}, go!$"

Kanto_SaffronCity_PokemonTrainerFanClub_Text_IllAlwaysBelieveInYou::
	.string "I’ll always believe in you,\n"
	.string "{PLAYER}.\p"
	.string "Please show the others what you’re\n"
	.string "like when you battle seriously.$"

Kanto_SaffronCity_PokemonTrainerFanClub_Text_EveryonesYourFanButICantBe::
	.string "Oh!\n"
	.string "{PLAYER}!\p"
	.string "Everyone’s been all excited about\n"
	.string "you recently.\p"
	.string "I can understand why.\n"
	.string "I know that you’re strong.\p"
	.string "But, do you know what?\p"
	.string "Once I became {STR_VAR_1}’s fan,\n"
	.string "I can’t just stop being a fan.$"

Kanto_SaffronCity_PokemonTrainerFanClub_Text_TrainersCoolWhenBattling::
	.string "I think TRAINERS are so cool when\n"
	.string "they’re battling.\p"
	.string "Don’t you think so, too?$"

Kanto_SaffronCity_PokemonTrainerFanClub_Text_WantToBeLikeSabrina::
	.string "Do you know what?\n"
	.string "I want to be like SABRINA!\p"
	.string "I want to be a psychic girl just\n"
	.string "like her!$"

Kanto_SaffronCity_PokemonTrainerFanClub_Text_WantToBeLikeYouOneDay::
	.string "It’s {PLAYER}!\n"
	.string "It’s {PLAYER}!\p"
	.string "I want to be like {PLAYER} one day!$"

Kanto_SaffronCity_PokemonTrainerFanClub_Text_EveryoneButMeStoppedBeingYourFan::
	.string "Everyone stopped being {PLAYER}’s\n"
	.string "fan…\p"
	.string "But that means I can have {PLAYER}\n"
	.string "all to myself!\p"
	.string "Yay! Go for it!\n"
	.string "My one and only {PLAYER}!$"

Kanto_SaffronCity_PokemonTrainerFanClub_Text_WontStopBeingSabrinasFan::
	.string "Even if I’m the only one, I won’t\n"
	.string "stop being SABRINA’s fan.\p"
	.string "My dream is to be like SABRINA,\n"
	.string "cute, but cool.\p"
	.string "I want to be a psychic girl just\n"
	.string "like her!$"

Kanto_SaffronCity_PokemonTrainerFanClub_Text_CanIBecomeCoolTrainerOneDay::
	.string "I wonder if I can become a cool\n"
	.string "TRAINER one day?$"

Kanto_SaffronCity_PokemonTrainerFanClub_Text_BrocksMyHero::
	.string "BROCK’s my hero!\n"
	.string "He’s a man among men!\p"
	.string "Heheh, I got my mom to do\n"
	.string "something for me.\p"
	.string "I got her to embroider “BROCK”\n"
	.string "on my shorts!$"

Kanto_SaffronCity_PokemonTrainerFanClub_Text_CanYouAutographShorts::
	.string "Oh! It’s {PLAYER}!\n"
	.string "Too cool!\p"
	.string "Oh!\n"
	.string "Oh, yeah!\p"
	.string "Can you autograph something?\n"
	.string "How about my shorts?$"

Kanto_SaffronCity_PokemonTrainerFanClub_Text_CountingOnYou::
	.string "However hot it’s been…\p"
	.string "And however cold it may get…\p"
	.string "I’ve always believed in, and worn,\n"
	.string "shorts!\p"
	.string "I’m counting on you to keep\n"
	.string "winning!\p"
	.string "I believe in you just as much as\n"
	.string "I believe in shorts!$"

Kanto_SaffronCity_PokemonTrainerFanClub_Text_BrocksFanToBitterEnd::
	.string "…  …\p"
	.string "…  …\n"
	.string "I’m BROCK’s fan!\p"
	.string "Even if I were his only fan,\n"
	.string "I swear it on my rock-hard will,\l"
	.string "and on my shorts!\p"
	.string "I’ll be his fan to the bitter end!$"

Kanto_SaffronCity_PokemonTrainerFanClub_Text_BrocksLastWordOnCool::
	.string "BROCK’s the last word on cool,\n"
	.string "end of story!\p"
	.string "Now there’s a guy who I want to\n"
	.string "be like!$"

Kanto_SaffronCity_PokemonTrainerFanClub_Text_HmmAndYouAre::
	.string "Hmm…\n"
	.string "And you are?\p"
	.string "Were you hoping to sign up for\n"
	.string "a membership in our FAN CLUB?$"

Kanto_SaffronCity_PokemonTrainerFanClub_Text_HadPleasureOfWatchingYouBattle::
	.string "{PLAYER}, I’ve had the pleasure of\n"
	.string "watching you battle.\p"
	.string "It brings back sweet memories of\n"
	.string "my own long-lost youth.\p"
	.string "May you continue to find success.\n"
	.string "You have a fan in me!$"

Kanto_SaffronCity_PokemonTrainerFanClub_Text_NeverSeenTrainerOfYourMagnificence::
	.string "I have journeyed far and wide\n"
	.string "across the lands.\p"
	.string "But never have I seen a TRAINER\n"
	.string "of your magnificence.\p"
	.string "Take courage in your abilities\n"
	.string "and keep battling on!\p"
	.string "Make that a promise to me,\n"
	.string "my young idol!$"

Kanto_SaffronCity_PokemonTrainerFanClub_Text_YouveStillAWaysToGo::
	.string "Ah, so you are the much-\n"
	.string "talked-about {PLAYER}.\p"
	.string "You certainly do have the look of\n"
	.string "a winner.\p"
	.string "But for me to accept you as\n"
	.string "a true MOéMON great…\p"
	.string "… …\p"
	.string "You’ve still a ways to go!$"

Kanto_SaffronCity_PokemonTrainerFanClub_Text_YoullBeTalkedAboutIfYouKeepWinning::
	.string "All of us gathered here are fans\n"
	.string "of MOéMON TRAINERS.\p"
	.string "The way they look in battle…\n"
	.string "They are the picture of coolness.\p"
	.string "Perhaps you will be talked about\n"
	.string "here, too, if you keep winning.$"

Kanto_SaffronCity_PokemonTrainerFanClub_Text_TrainerHasBeenOnFire::
	.string "Recently, {STR_VAR_1} has been on\n"
	.string "fire.\p"
	.string "The way he battles… It’s just \n"
	.string "the way that I like it!$"

Kanto_SaffronCity_PokemonTrainerFanClub_Text_AdoreWayYouBattle::
	.string "Oh, my goodness!\n"
	.string "Is it really you, {PLAYER}?\p"
	.string "I have to tell you, I adore the\n"
	.string "way you battle.\p"
	.string "I hope you’ll keep at it.\n"
	.string "I’ll be your number one fan!$"

Kanto_SaffronCity_PokemonTrainerFanClub_Text_ImYourNumberOneFan::
	.string "Oh, my goodness!\n"
	.string "If it isn’t {PLAYER}!\p"
	.string "It’s so disappointing… I realized\n"
	.string "I’m your number-one-and-only fan!\p"
	.string "Well, it can’t be helped!\n"
	.string "Let me cheer you on, at least!\p"
	.string "Hurray, hurray, {PLAYER}!$"

Kanto_SaffronCity_PokemonTrainerFanClub_Text_EveryoneTalksAboutYou::
	.string "Oh, hello!\n"
	.string "If it isn’t {PLAYER}!\p"
	.string "Lately, all everyone talks about\n"
	.string "is you.\p"
	.string "I’m feeling lonesome because I\n"
	.string "prefer {STR_VAR_1}.$"

Kanto_SaffronCity_PokemonTrainerFanClub_Text_LoveWayTrainerTalks::
	.string "{STR_VAR_1}’s cool, huh?\n"
	.string "I love the way he talks.\p"
	.string "I’d really relish a chance to meet\n"
	.string "him in person once.$"

Kanto_SaffronCity_PokemonTrainerFanClub_Text_TheWayYouBattleIsCool::
	.string "Oh, man, oh, man!\n"
	.string "Cool! Too cool!\p"
	.string "…Huh?\n"
	.string "No, not you.\p"
	.string "Just the way you battle.\n"
	.string "That’s cool.$"

Kanto_SaffronCity_PokemonTrainerFanClub_Text_ImOnlyOneLeftOfYourFans::
	.string "Yo, {PLAYER}!\p"
	.string "Looks like I’m the only one left\n"
	.string "out of your fans.\p"
	.string "But, you know, I like being an only\n"
	.string "fan… It’s less stressful.\p"
	.string "Keep up and at it!$"

Kanto_SaffronCity_PokemonTrainerFanClub_Text_ImLoneTrainerFan::
	.string "Huh, can you believe it?\n"
	.string "Everyone turned into your fans.\p"
	.string "So after all we’ve been through,\n"
	.string "I’m the lone {STR_VAR_1} fan?\p"
	.string "Eh, being the one and only\n"
	.string "{STR_VAR_1} fan’s my style, anyway.$"

Kanto_SaffronCity_PokemonTrainerFanClub_Text_WhyCantOthersSeeMastersDignity::
	.string "Master {STR_VAR_1} is so dignified…\n"
	.string "He is so my type.\p"
	.string "Why can’t the others see the\n"
	.string "dignity he simply oozes?$"

Kanto_SaffronCity_PokemonTrainerFanClub_Text_YouReallyAreAmazing::
	.string "Amazing!\n"
	.string "{PLAYER}, you really are amazing!\p"
	.string "Instead of just watching, maybe\n"
	.string "I should become a TRAINER, too.$"

Kanto_SaffronCity_PokemonTrainerFanClub_Text_ImYourOnlyFan::
	.string "I’m your only fan…\n"
	.string "Sigh… This is lonely…\p"
	.string "Can’t you try harder and make the\n"
	.string "others take notice of you?$"

Kanto_SaffronCity_PokemonTrainerFanClub_Text_BelieveInMasterWithAllMyHeart::
	.string "I believe in Master {STR_VAR_1}\n"
	.string "with all my heart.\p"
	.string "Even if I am the only one to\n"
	.string "believe in him.\p"
	.string "I won’t ever feel lonesome, though.\p"
	.string "Not if I keep thinking about Master\n"
	.string "{STR_VAR_1}.$"

Kanto_SaffronCity_PokemonTrainerFanClub_Text_OnlyMasterHasMyRespect::
	.string "Hiyah! There is but one TRAINER\n"
	.string "who has my respect.\p"
	.string "And that is none other than my\n"
	.string "MASTER at the FIGHTING DOJO.\p"
	.string "You should become a fan of my\n"
	.string "MASTER like me! Hiyah!$"

Kanto_SaffronCity_PokemonTrainerFanClub_Text_YourBattleStyleIsEducational::
	.string "Hiyah!\p"
	.string "Your battling style is most\n"
	.string "educational.\p"
	.string "I hope that you will keep plugging\n"
	.string "away at success, hiyah!$"

Kanto_SaffronCity_PokemonTrainerFanClub_Text_WontStopBeingYourFan::
	.string "Instead of just watching others\n"
	.string "battle, battle yourself!\p"
	.string "…My DOJO’s MASTER chewed me out\n"
	.string "like that…\p"
	.string "But I won’t let that stop me from\n"
	.string "being your fan!\p"
	.string "I can’t stop being your fan!$"

Kanto_SaffronCity_PokemonTrainerFanClub_Text_NeverBeFanOfAnyoneButMaster::
	.string "Hiyah! I respect my MASTER at the\n"
	.string "FIGHTING DOJO.\p"
	.string "I will never become a fan of\n"
	.string "anyone other than my MASTER!\p"
	.string "I will even sneak out of the DOJO\n"
	.string "to spread word of his greatness.\p"
	.string "Hiyah!$"

Kanto_SaffronCity_PokemonTrainerFanClub_Text_SpecialFeatureBruno::
	.string "MOéMON JOURNAL\p"
	.string "Special Feature:\n"
	.string "ELITE FOUR’s BRUNO!\p"
	.string "BRUNO apparently joined the ELITE\n"
	.string "FOUR out of his burning ambition to\l"
	.string "battle the best TRAINERS.$"
