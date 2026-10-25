Johto_RadioTower5F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840EA66
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Johto_RadioTower5F_OnFrame_83D8D5B
	map_script MAP_SCRIPT_ON_LOAD, Johto_RadioTower5F_EventScript_83D8D65
	.byte 0

Johto_RadioTower5F_OnFrame_83D8D5B:
	map_script_2 VAR_QZ_0x5076, 1, Johto_RadioTower5F_EventScript_84019DA
	.2byte 0

Johto_RadioTower5F_EventScript_83D8D65::
	goto_if_set FLAG_QZ_0x1318, Johto_RadioTower5F_EventScript_83D8D79
	call_if_eq VAR_QZ_0x500A, 6, Johto_RadioTower5F_EventScript_83D8D7A

Johto_RadioTower5F_EventScript_83D8D79::
	end

Johto_RadioTower5F_EventScript_83D8D7A::
	clearflag FLAG_QZ_0x10A6
	setflag FLAG_QZ_0x10A7
	return

@ Johto_RadioTower5F object 1 (local id 1, gfx 0x30) at 11,9
Johto_RadioTower5F_EventScript_83D8D81::
	lock
	goto_if_ge VAR_QZ_0x500A, 10, Johto_RadioTower5F_EventScript_83D8E01
	goto_if_eq VAR_FACING, 1, Johto_RadioTower5F_EventScript_83D8DF3
	faceplayer
	msgbox Johto_RadioTower5F_Text_83D8E2E, MSGBOX_DEFAULT
	closemessage
	applymovement 1, Johto_RadioTower5F_Movement_83D8E0E
	waitmovement 0
	setflag FLAG_QZ_0x10A6
	removeobject 1
	clearflag FLAG_QZ_0x10A7
	addobject 4
	applymovement 4, Johto_RadioTower5F_Movement_83D8E26
	waitmovement 0
	msgbox Johto_RadioTower5F_Text_83D8E4C, MSGBOX_DEFAULT
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4374, 0, Johto_RadioTower5F_Text_83D8F29
	msgbox Johto_RadioTower5F_Text_83D8F4C, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, 0x2E0
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	setvar VAR_QZ_0x500A, 7
	clearflag FLAG_QZ_0x109B
	closemessage
	release
	end

Johto_RadioTower5F_EventScript_83D8DF3::
	end

@ Johto_RadioTower5F object 4 (local id 4, gfx 0x3EA8) at 11,9
Johto_RadioTower5F_EventScript_83D8DF4::
	lock
	faceplayer
	msgbox Johto_RadioTower5F_Text_83D902A, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_RadioTower5F_EventScript_83D8E01::
	lock
	faceplayer
	msgbox Johto_RadioTower5F_Text_83D9929, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_RadioTower5F object 6 (local id 6, gfx 0x3EBF) at 20,12
Johto_RadioTower5F_EventScript_83D90F3::
	lock
	faceplayer
	msgbox Johto_RadioTower5F_Text_83D99C9, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_RadioTower5F object 3 (local id 3, gfx 0x3E81) at 28,10
Johto_RadioTower5F_EventScript_83D9100::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4228, 0, Johto_RadioTower5F_Text_83D92AE, Johto_RadioTower5F_Text_83D9326
	msgbox Johto_RadioTower5F_Text_83D9377, MSGBOX_AUTOCLOSE
	end

@ Johto_RadioTower5F object 2 (local id 2, gfx 0x3E80) at 25,12
Johto_RadioTower5F_EventScript_83D9117::
	lock
	faceplayer
	msgbox Johto_RadioTower5F_Text_83D93F3, MSGBOX_DEFAULT
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4564, 0, Johto_RadioTower5F_Text_83D9522
	msgbox Johto_RadioTower5F_Text_83D953C, MSGBOX_DEFAULT
	delay 0x37
	closemessage
	fadescreen 1
	setflag FLAG_QZ_0x1097
	playse SE_EXIT
	setvar VAR_QZ_0x500A, 0xA
	removeobject 2
	removeobject 3
	setflag FLAG_QZ_0x10A7
	clearflag FLAG_QZ_0x10A6
	removeobject 4
	addobject 5
	playbgm MUS_QZ_615, 1
	delay 0x78
	fadescreen 0
	goto_if_eq VAR_FACING, 1, Johto_RadioTower5F_EventScript_83D9182
	goto_if_eq VAR_FACING, 3, Johto_RadioTower5F_EventScript_83D91AE
	goto_if_eq VAR_FACING, 2, Johto_RadioTower5F_EventScript_83D91DA
	release
	end

Johto_RadioTower5F_EventScript_83D9182::
	applymovement 5, Johto_RadioTower5F_Movement_83D927B
	waitmovement 0
	turnobject LOCALID_PLAYER, 4
	msgbox Johto_RadioTower5F_Text_83D95E0, MSGBOX_DEFAULT
	closemessage
	call Johto_RadioTower5F_EventScript_83D9206
	applymovement 5, Johto_RadioTower5F_Movement_83D9285
	waitmovement 0
	goto Johto_RadioTower5F_EventScript_83D921D

Johto_RadioTower5F_EventScript_83D91AE::
	applymovement 5, Johto_RadioTower5F_Movement_83D9289
	waitmovement 0
	turnobject LOCALID_PLAYER, 4
	msgbox Johto_RadioTower5F_Text_83D95E0, MSGBOX_DEFAULT
	closemessage
	call Johto_RadioTower5F_EventScript_83D9206
	applymovement 5, Johto_RadioTower5F_Movement_83D9293
	waitmovement 0
	goto Johto_RadioTower5F_EventScript_83D921D

Johto_RadioTower5F_EventScript_83D91DA::
	applymovement 5, Johto_RadioTower5F_Movement_83D9296
	waitmovement 0
	turnobject LOCALID_PLAYER, 4
	msgbox Johto_RadioTower5F_Text_83D95E0, MSGBOX_DEFAULT
	closemessage
	call Johto_RadioTower5F_EventScript_83D9206
	applymovement 5, Johto_RadioTower5F_Movement_83D92A2
	waitmovement 0
	goto Johto_RadioTower5F_EventScript_83D921D

Johto_RadioTower5F_EventScript_83D9206::
	setorcopyvar VAR_0x8000, 0x376
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	msgbox Johto_RadioTower5F_Text_83D965C, MSGBOX_DEFAULT
	closemessage
	release
	return

Johto_RadioTower5F_EventScript_83D921D::
	playse SE_PIN
	applymovement 5, Quetzal_Movement_83607F9
	delay 0x3C
	turnobject 5, 3
	msgbox Johto_RadioTower5F_Text_83D97A9, MSGBOX_DEFAULT
	closemessage
	applymovement 5, Johto_RadioTower5F_Movement_83D92A6
	waitmovement 0
	playse SE_PIN
	applymovement 5, Quetzal_Movement_83607F9
	delay 0x3C
	turnobject 5, 1
	msgbox Johto_RadioTower5F_Text_83D98B5, MSGBOX_DEFAULT
	closemessage
	applymovement 5, Johto_RadioTower5F_Movement_83D92A9
	waitmovement 0
	setflag FLAG_QZ_0x10A8
	removeobject 5
	setvar VAR_QZ_0x500C, 0x11
	release
	end

@ Johto_RadioTower5F object 8 (local id 8, gfx 0x2722) at 26,11
Johto_RadioTower5F_EventScript_84018EA::
	lock
	msgbox Quetzal_Text_8401EDB, MSGBOX_DEFAULT
	release
	end

Johto_RadioTower5F_EventScript_84019DA::
	goto_if_unset FLAG_QZ_0x1318, Quetzal_EventScript_8401C95
	setvar VAR_QZ_0x5076, 0
	goto_if_eq VAR_QZ_0x506C, 5, Johto_RadioTower5F_EventScript_84034C7
	end

@ Johto_RadioTower5F object 7 (local id 7, gfx 0x276A) at 25,12
Johto_RadioTower5F_EventScript_8401BD3::
	lockall
	goto_if_unset FLAG_QZ_0x1318, Quetzal_EventScript_8401C94
	goto_if_set FLAG_QZ_0x1319, Quetzal_EventScript_8401C94
	special Script_ClearHeldMovement
	faceplayer
	goto_if_eq VAR_QZ_0x506C, 3, Johto_RadioTower5F_EventScript_8403AD7
	goto_if_eq VAR_QZ_0x506C, 2, Johto_RadioTower5F_EventScript_8401C0D
	msgbox Johto_RadioTower5F_Text_84024D6, MSGBOX_DEFAULT
	goto Quetzal_EventScript_8401C92

Johto_RadioTower5F_EventScript_8401C0D::
	msgbox Johto_RadioTower5F_Text_8402A39, MSGBOX_DEFAULT
	goto Quetzal_EventScript_8401C92

Johto_RadioTower5F_EventScript_8401C1A::
	goto_if_set FLAG_QZ_0x1320, Johto_RadioTower5F_EventScript_8401C45
	msgbox Johto_RadioTower5F_Text_8404460, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, 0x340
	setorcopyvar VAR_0x8001, 0xA
	callstd STD_OBTAIN_ITEM
	goto_if_ne VAR_0x8007, 1, Johto_RadioTower5F_EventScript_8401C46
	setflag FLAG_QZ_0x1320

Johto_RadioTower5F_EventScript_8401C45::
	return

Johto_RadioTower5F_EventScript_8401C46::
	msgbox Johto_RadioTower5F_Text_840448E, MSGBOX_DEFAULT
	return

Johto_RadioTower5F_EventScript_84034C7::
	lockall
	special QzResetFollowerObjects_572
	applymovement LOCALID_PLAYER, Johto_RadioTower5F_Movement_84035EA
	waitmovement LOCALID_PLAYER
	turnobject 8, 1
	turnobject 7, 2
	msgbox Johto_RadioTower5F_Text_84035F9, MSGBOX_DEFAULT
	msgbox Johto_RadioTower5F_Text_840366B, MSGBOX_DEFAULT
	setvar VAR_0x8004, 7
	setvar VAR_0x8005, 0xFF
	special QzObjectFaceObject_797
	msgbox Johto_RadioTower5F_Text_84036F8, MSGBOX_DEFAULT
	closemessage
	special SavePlayerParty
	fadescreen 1
	special ChooseHalfPartyForBattle
	waitstate
	goto_if_eq VAR_RESULT, 0, Johto_RadioTower5F_EventScript_84035B8
	special ReducePlayerPartyToSelectedMons
	setvar VAR_0x8004, 2
	setvar VAR_0x8005, 4
	special CallFrontierUtilFunc
	turnobject LOCALID_PLAYER, 2
	turnobject 7, 2
	turnobject 8, 1
	setvar VAR_0x8004, 0xD
	setvar VAR_0x8005, 0
	special DoSpecialTrainerBattle
	waitstate
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, 1, Quetzal_EventScript_8401C4F
	turnobject 8, 1
	turnobject 7, 2
	turnobject LOCALID_PLAYER, 2
	msgbox Johto_RadioTower5F_Text_84037BA, MSGBOX_DEFAULT
	closemessage
	applymovement 8, Johto_RadioTower5F_Movement_84035EC
	waitmovement 8
	setflag FLAG_QZ_0x14A9
	removeobject 8
	fadescreen 1
	setvar VAR_QZ_0x506C, 1
	setobjectxy 7, 0x19, 0xC
	setobjectxy LOCALID_PLAYER, 0x19, 0xD
	turnobject 7, 1
	turnobject LOCALID_PLAYER, 2
	special QzHealPlayerPartyEx_0
	delay 0x3C
	fadescreen 0
	msgbox Johto_RadioTower5F_Text_840385F, MSGBOX_DEFAULT
	msgbox Johto_RadioTower5F_Text_84023AC, MSGBOX_DEFAULT
	msgbox Johto_RadioTower5F_Text_8402418, MSGBOX_DEFAULT
	goto Quetzal_EventScript_8401C94

Johto_RadioTower5F_EventScript_84035B8::
	special LoadPlayerParty
	msgbox Johto_RadioTower5F_Text_8403910, MSGBOX_DEFAULT
	closemessage
	fadescreen 3
	special QzGiovanniStoryEnterPast_699
	setescapewarp MAP_JOHTO_ROUTE27, 255, 45, 17
	warp MAP_JOHTO_TOHJO_FALLS, 255, 46, 17
	waitstate
	end

Johto_RadioTower5F_EventScript_8403AD7::
	goto_if_unset FLAG_QZ_0x1318, Quetzal_EventScript_8401C94
	goto_if_set FLAG_QZ_0x1319, Quetzal_EventScript_8401C94
	goto_if_ne VAR_QZ_0x506C, 3, Quetzal_EventScript_8401C94
	lockall
	special QzResetFollowerObjects_572
	closemessage
	getplayerxy VAR_0x8004, VAR_0x8005
	goto_if_ne VAR_0x8004, 25, Johto_RadioTower5F_EventScript_8403B2E
	goto_if_eq VAR_0x8005, 13, Johto_RadioTower5F_EventScript_8403B5E
	goto_if_ne VAR_0x8005, 11, Johto_RadioTower5F_EventScript_8403B53
	applymovement LOCALID_PLAYER, Johto_RadioTower5F_Movement_8403CD6
	waitmovement LOCALID_PLAYER
	goto Johto_RadioTower5F_EventScript_8403B5E

Johto_RadioTower5F_EventScript_8403B2E::
	goto_if_ne VAR_0x8005, 12, Johto_RadioTower5F_EventScript_8403B53
	goto_if_ne VAR_0x8004, 26, Johto_RadioTower5F_EventScript_8403B53
	applymovement LOCALID_PLAYER, Johto_RadioTower5F_Movement_8403CDB
	waitmovement LOCALID_PLAYER
	goto Johto_RadioTower5F_EventScript_8403B5E

Johto_RadioTower5F_EventScript_8403B53::
	fadescreen 1
	setobjectxy LOCALID_PLAYER, 0x19, 0xD
	fadescreen 0

Johto_RadioTower5F_EventScript_8403B5E::
	turnobject LOCALID_PLAYER, 2
	turnobject 7, 1
	msgbox Johto_RadioTower5F_Text_8402A8F, MSGBOX_DEFAULT
	closemessage
	fadenewbgm MUS_QZ_648
	addobject 2
	applymovement 2, Johto_RadioTower5F_Movement_8403CDE
	waitmovement 2
	setvar VAR_0x8004, 2
	setvar VAR_0x8005, 7
	special QzObjectFaceObject_797
	msgbox Johto_RadioTower5F_Text_840412D, MSGBOX_DEFAULT
	msgbox Johto_RadioTower5F_Text_8404155, MSGBOX_DEFAULT
	msgbox Johto_RadioTower5F_Text_84041CB, MSGBOX_DEFAULT
	msgbox Johto_RadioTower5F_Text_840425F, MSGBOX_DEFAULT
	closemessage
	applymovement 2, Johto_RadioTower5F_Movement_8403CE5
	waitmovement 2
	setvar VAR_0x8004, 2
	setvar VAR_0x8005, 7
	special QzObjectFaceObject_797
	turnobject LOCALID_PLAYER, 2
	msgbox Johto_RadioTower5F_Text_840428D, MSGBOX_DEFAULT
	msgbox Johto_RadioTower5F_Text_84042DB, MSGBOX_DEFAULT
	msgbox Johto_RadioTower5F_Text_8404303, MSGBOX_DEFAULT
	closemessage
	setvar VAR_0x8004, 2
	setvar VAR_0x8005, 0xFF
	special QzObjectFaceObject_797
	msgbox Johto_RadioTower5F_Text_840433A, MSGBOX_DEFAULT
	closemessage
	setvar VAR_QZ_0x5076, 5
	special SavePlayerParty
	fadescreen 1
	special ChooseHalfPartyForBattle
	waitstate
	goto_if_eq VAR_RESULT, 0, Johto_RadioTower5F_EventScript_8403CBE
	special ReducePlayerPartyToSelectedMons
	setvar VAR_0x8004, 2
	setvar VAR_0x8005, 4
	special CallFrontierUtilFunc
	turnobject LOCALID_PLAYER, 2
	turnobject 2, 2
	turnobject 7, 1
	cleartrainerflag 0x1362
	setvar VAR_0x8004, 0xE
	setvar VAR_0x8005, 0
	special DoSpecialTrainerBattle
	waitstate
	fadedefaultbgm
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, 1, Quetzal_EventScript_8401C4F
	setvar VAR_0x8004, 2
	setvar VAR_0x8005, 7
	special QzObjectFaceObject_797
	turnobject LOCALID_PLAYER, 2
	msgbox Johto_RadioTower5F_Text_8402BB7, MSGBOX_DEFAULT
	msgbox Johto_RadioTower5F_Text_840437F, MSGBOX_DEFAULT
	setvar VAR_0x8004, 7
	setvar VAR_0x8005, 0xFF
	special QzObjectFaceObject_797
	msgbox Johto_RadioTower5F_Text_8402C25, MSGBOX_DEFAULT
	closemessage
	fadescreen 3
	removeobject 7
	fadescreen 2
	setvar VAR_0x8004, 2
	setvar VAR_0x8005, 0xFF
	special QzObjectFaceObject_797
	call Johto_RadioTower5F_EventScript_8401C1A
	msgbox Johto_RadioTower5F_Text_84044CD, MSGBOX_DEFAULT
	closemessage
	fadescreen 3
	special QzGiovanniStoryEnd_698
	setescapewarp MAP_JOHTO_ROUTE27, 255, 45, 17
	warp MAP_JOHTO_PLAYERS_HOUSE1F, 255, 9, 7
	waitstate
	end

Johto_RadioTower5F_EventScript_8403CBE::
	fadedefaultbgm
	special LoadPlayerParty
	goto Quetzal_EventScript_8401C4F

Johto_RadioTower5F_Movement_83D8E0E:
	face_right
	delay_8
	face_up
	delay_8
	face_left
	delay_8
	face_down
	delay_8
	face_right
	delay_4
	face_up
	delay_4
	face_left
	delay_4
	face_down
	delay_4
	face_right
	delay_2
	face_up
	delay_2
	face_left
	delay_2
	face_down
	step_end

Johto_RadioTower5F_Movement_83D8E26:
	face_right
	delay_2
	face_up
	delay_2
	face_left
	delay_2
	face_down
	step_end

Johto_RadioTower5F_Movement_83D927B:
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_left
	walk_left
	walk_left
	step_end

Johto_RadioTower5F_Movement_83D9285:
	walk_right
	walk_right
	walk_right
	step_end

Johto_RadioTower5F_Movement_83D9289:
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_left
	walk_left
	step_end

Johto_RadioTower5F_Movement_83D9293:
	walk_right
	walk_right
	step_end

Johto_RadioTower5F_Movement_83D9296:
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_left
	walk_left
	walk_left
	step_end

Johto_RadioTower5F_Movement_83D92A2:
	walk_right
	walk_right
	walk_right
	step_end

Johto_RadioTower5F_Movement_83D92A6:
	walk_up
	walk_up
	step_end

Johto_RadioTower5F_Movement_83D92A9:
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

Johto_RadioTower5F_Movement_84035EA:
	walk_up
	step_end

Johto_RadioTower5F_Movement_84035EC:
	walk_right
	walk_right
	walk_up
	walk_up
	walk_up
	walk_up
	walk_left
	walk_left
	walk_left
	walk_left
	walk_up
	walk_up
	step_end

Johto_RadioTower5F_Movement_8403CD6:
	walk_right
	walk_down
	walk_down
	walk_left
	step_end

Johto_RadioTower5F_Movement_8403CDB:
	walk_down
	walk_left
	step_end

Johto_RadioTower5F_Movement_8403CDE:
	walk_down
	walk_down
	walk_down
	walk_left
	walk_left
	walk_down
	step_end

Johto_RadioTower5F_Movement_8403CE5:
	walk_down
	face_left
	step_end

Johto_RadioTower5F_Text_83D8E2E::
	.string "Y-you! You came to rescue me?$"

Johto_RadioTower5F_Text_83D8E4C::
	.string "Is that what you were expecting?\p"
	.string "Wrong! I’m an imposter!\p"
	.string "I pretended to be the real thing to\n"
	.string "prepare for our takeover.\p"
	.string "Do you want to know where we hid\n"
	.string "the real DIRECTOR?\p"
	.string "Sure, I’ll tell you. But only if you\n"
	.string "can beat me!$"

Johto_RadioTower5F_Text_83D8F29::
	.string "OK, OK. I’ll tell you where he is.$"

Johto_RadioTower5F_Text_83D8F4C::
	.string "The boss stashed the DIRECTOR in\n"
	.string "the UNDERGROUND WAREHOUSE.\p"
	.string "It’s at the far end of the UNDER-\n"
	.string "GROUND.\p"
	.string "I am a nice guy, I will give you\n"
	.string "the BASEMENT KEY to get to the\l"
	.string "UNDERGROUND WAREHOUSE.\p"
	.string "But I doubt you’ll get that far.$"

Johto_RadioTower5F_Text_83D902A::
	.string "The boss stashed the DIRECTOR in\n"
	.string "the UNDERGROUND WAREHOUSE.\p"
	.string "It’s at the far end of the UNDER-\n"
	.string "GROUND.\p"
	.string "You can use the BASEMENT KEY to\n"
	.string "get to the UNDERGROUND WAREHOUSE.\p"
	.string "But I doubt you’ll get that far.$"

Johto_RadioTower5F_Text_83D92AE::
	.string "Remember me from the HIDEOUT in\n"
	.string "MAHOGANY TOWN?\p"
	.string "My boss scolded me but I’m stronger.\p"
	.string "I lost then, but I won’t this time.$"

Johto_RadioTower5F_Text_83D9326::
	.string "This can’t be happening!\p"
	.string "I fought hard, but I still lost…\p"
	.string "I’m so sorry GIOVANNI…$"

Johto_RadioTower5F_Text_83D9377::
	.string "{PLAYER}, isn’t it?\p"
	.string "A brat like you won’t appreciate\n"
	.string "the magnificence of TEAM ROCKET.\p"
	.string "That’s too bad. I really admire\n"
	.string "your power.$"

Johto_RadioTower5F_Text_83D93F3::
	.string "Oh? You managed to get this far?\p"
	.string "You must be quite the trainer.\p"
	.string "We intend to take over this RADIO\n"
	.string "STATION and announce our comeback.\p"
	.string "That should bring our boss GIOVANNI\n"
	.string "back from his solitary training.\p"
	.string "We are going to regain our former\n"
	.string "glory...\p"
	.string "I won’t allow you to interfere with\n"
	.string "our meticulous plans!$"

Johto_RadioTower5F_Text_83D9522::
	.string "No! Forgive me, GIOVANNI!$"

Johto_RadioTower5F_Text_83D953C::
	.string "How could this be?\p"
	.string "Our dreams have come to naught.\p"
	.string "I wasn’t up to the task after all.\p"
	.string "Like GIOVANNI did before me, I will\n"
	.string "disband TEAM ROCKET here today.\p"
	.string "Farewell.$"

Johto_RadioTower5F_Text_83D95E0::
	.string "DIRECTOR: {PLAYER}, thank you!\p"
	.string "Your courageous actions have saved\n"
	.string "POKéMON nationwide.\p"
	.string "I know it’s not much, but please\n"
	.string "take this.$"

Johto_RadioTower5F_Text_83D965C::
	.string "There used to be a tower right here\n"
	.string "in GOLDENROD CITY.\p"
	.string "But it was old and creaky.\p"
	.string "So we replaced it with our RADIO\n"
	.string "TOWER.\p"
	.string "We dug up that wing during\n"
	.string "construction.\p"
	.string "I heard that all sorts of POKéMON\n"
	.string "lived in GOLDENROD in the past.\p"
	.string "Perhaps…\p"
	.string "That wing has some connection to the\n"
	.string "TIN TOWER in ECRUTEAK CITY…\p"
	.string "OK, I better go to my OFFICE.$"

Johto_RadioTower5F_Text_83D97A9::
	.string "Ah!\p"
	.string "That reminds me…\p"
	.string "That wing I gave you... I think\n"
	.string "it’s related to the LEGENDARY\l"
	.string "POKéMON.\p"
	.string "But! The LEGENDARY POKéMON won’t\n"
	.string "appear with just that wing...\p"
	.string "Apparently there is something else\n"
	.string "you need... Or something like that.\p"
	.string "I wonder what it was...\p"
	.string "I can’t remember.$"

Johto_RadioTower5F_Text_83D98B5::
	.string "Are you expecting something?\n"
	.string "This is what they call FAKE OUT.\p"
	.string "I really must go back to my OFFICE\n"
	.string "this time. See ya!$"

Johto_RadioTower5F_Text_83D9929::
	.string "DIRECTOR: Hello, {PLAYER}!\p"
	.string "You know, I love POKéMON.\p"
	.string "I built this RADIO TOWER so I could\n"
	.string "express my love of POKéMON.\p"
	.string "It would be nice if people enjoyed\n"
	.string "our programs.$"

Johto_RadioTower5F_Text_83D99C9::
	.string "BEN: Do you listen to our music?$"

Johto_RadioTower5F_Text_84023AC::
	.string "Without you, that call\n"
	.string "would have been the end.\p"
	.string "Now TEAM ROCKET has a future.\n"
	.string "And you have earned my trust.$"

Johto_RadioTower5F_Text_8402418::
	.string "I have a mission for you.\n"
	.string "Go to NEW BARK TOWN and oversee\l"
	.string "contributions from the south.\p"
	.string "Some families still resist us.\n"
	.string "We need their Pokémon and their labor.\l"
	.string "I trust you will persuade them.$"

Johto_RadioTower5F_Text_84024D6::
	.string "Have you been to NEW BARK TOWN?\n"
	.string "Do not forget your assignment.\p"
	.string "My men await your supervision\n"
	.string "of the families’ contributions.$"

Johto_RadioTower5F_Text_8402A39::
	.string "GIOVANNI: You have saved your mother.\n"
	.string "Go back to NEW BARK\l"
	.string "and stop getting in my way.$"

Johto_RadioTower5F_Text_8402A8F::
	.string "GIOVANNI: I gave you\n"
	.string "authority over my men.\l"
	.string "And you used it to turn against them.\p"
	.string "While it was other families,\n"
	.string "you obeyed my orders.\p"
	.string "All it took was for those orders\n"
	.string "to affect yours for you to betray me.\p"
	.string "Your mother is free.\n"
	.string "You have what you wanted.\l"
	.string "Why do the others matter to you?$"

Johto_RadioTower5F_Text_8402BB7::
	.string "GIOVANNI: My ally... and my son.\n"
	.string "Both against me.\p"
	.string "SILVER, if you had understood\n"
	.string "what I was trying to build...$"

Johto_RadioTower5F_Text_8402C25::
	.string "GIOVANNI: Taking me back\n"
	.string "to that cave, CELEBI?\p"
	.string "Very well. I will leave this Johto.\n"
	.string "But I will not forget your betrayal.$"

Johto_RadioTower5F_Text_84035F9::
	.string "ETHAN: GIOVANNI? Then that call...\n"
	.string "You actually came!\p"
	.string "Your men have hurt enough people.\n"
	.string "I won’t let you carry on!$"

Johto_RadioTower5F_Text_840366B::
	.string "GIOVANNI: So you’re the one\n"
	.string "who cornered my men.\p"
	.string "You’ve come far. But this time,\n"
	.string "you won’t be facing a grunt.\l"
	.string "And I won’t be fighting alone.$"

Johto_RadioTower5F_Text_84036F8::
	.string "GIOVANNI: Stand beside me.\n"
	.string "You brought me here.\l"
	.string "Let’s finish what we started.\p"
	.string "Choose up to three Pokémon. I’ll bring\n"
	.string "three of my own. That boy will\l"
	.string "have to face us both.$"

Johto_RadioTower5F_Text_84037BA::
	.string "ETHAN: My Pokémon can’t take any more.\n"
	.string "I have to get them out of here.\p"
	.string "I don’t know what he promised you.\n"
	.string "But look at who you’re helping.\p"
	.string "This isn’t over, GIOVANNI.$"

Johto_RadioTower5F_Text_840385F::
	.string "GIOVANNI: We have achieved so much\n"
	.string "in these past few weeks.\p"
	.string "The radio broadcasts our orders.\n"
	.string "New recruits arrive every day.\p"
	.string "Johto recognizes TEAM ROCKET’s\n"
	.string "authority once again.$"

Johto_RadioTower5F_Text_8403910::
	.string "CELEBI surrounds you with its light.$"

Johto_RadioTower5F_Text_840412D::
	.string "SILVER: You never cared about your own.$"

Johto_RadioTower5F_Text_8404155::
	.string "GIOVANNI: SILVER...\n"
	.string "Another past, and you are just as weak.\p"
	.string "Still waiting for your father\n"
	.string "to come and look after you.$"

Johto_RadioTower5F_Text_84041CB::
	.string "SILVER: When you left, I thought it was\n"
	.string "my fault. That I was not strong enough.\p"
	.string "That is why I wanted to surpass you.\n"
	.string "So you would look at me again.$"

Johto_RadioTower5F_Text_840425F::
	.string "GIOVANNI: Then prove it.\n"
	.string "Beat me on your own.$"

Johto_RadioTower5F_Text_840428D::
	.string "SILVER: No. I will not let\n"
	.string "you keep hurting people\l"
	.string "just to earn your respect.$"

Johto_RadioTower5F_Text_84042DB::
	.string "GIOVANNI: And you need help to stop me?$"

Johto_RadioTower5F_Text_8404303::
	.string "SILVER: Stopping you matters\n"
	.string "more than doing it alone.$"

Johto_RadioTower5F_Text_840433A::
	.string "SILVER: Choose up to three Pokémon.\n"
	.string "I will fight with three of mine.$"

Johto_RadioTower5F_Text_840437F::
	.string "SILVER: For a long time, I thought I had\n"
	.string "to be stronger for you to come back.\p"
	.string "But leaving was your choice.\n"
	.string "Not my fault.\p"
	.string "I have mistreated those\n"
	.string "who stood by me too.\p"
	.string "My Pokémon deserve better.\n"
	.string "That is something I can change.$"

Johto_RadioTower5F_Text_8404460::
	.string "SILVER: Here. Your Pokémon\n"
	.string "have earned these.$"

Johto_RadioTower5F_Text_840448E::
	.string "SILVER: I’ll hold on to them.\n"
	.string "Come back with room in your BAG.$"

Johto_RadioTower5F_Text_84044CD::
	.string "SILVER: Go back to your mother.\n"
	.string "I will stay and help here.\p"
	.string "Then I will head to the shrine\n"
	.string "in DRAGON’S DEN, in BLACKTHORN.\p"
	.string "You fought well. And do not think\n"
	.string "I have forgotten our battle...\l"
	.string "You owe me a rematch.$"
