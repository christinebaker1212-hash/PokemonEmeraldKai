Johto_PlayersHouse1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_PlayersHouse1F_EventScript_83E3AEE
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Johto_PlayersHouse1F_OnFrame_83E3ADC
	.byte 0

Johto_PlayersHouse1F_OnFrame_83E3ADC:
	map_script_2 VAR_QZ_0x5076, 2, Johto_PlayersHouse1F_EventScript_84019F4
	map_script_2 VAR_QZ_0x5076, 3, Johto_PlayersHouse1F_EventScript_8401C72
	.2byte 0

Johto_PlayersHouse1F_EventScript_83813E8::
	msgbox Johto_PlayersHouse1F_Text_82DD80A, MSGBOX_DEFAULT
	goto Johto_PlayersHouse1F_EventScript_8381400

Johto_PlayersHouse1F_EventScript_8381400::
	closemessage
	call Quetzal_EventScript_83602D9
	msgbox Johto_PlayersHouse1F_Text_82DD85E, MSGBOX_DEFAULT
	release
	end

Johto_PlayersHouse1F_EventScript_83E3AEE::
	special QzGiovanniStoryNewBarkHouseFlags_697
	goto_if_set FLAG_QZ_0x1318, Johto_PlayersHouse1F_EventScript_83E3B32
	goto_if_eq VAR_QZ_0x5076, 3, Johto_PlayersHouse1F_EventScript_83E3B27
	call_if_eq VAR_QZ_0x5003, 1, Johto_PlayersHouse1F_EventScript_83E3B33
	call_if_eq VAR_QZ_0x5003, 2, Johto_PlayersHouse1F_EventScript_83E3B33
	call_if_eq VAR_QZ_0x5003, 3, Johto_PlayersHouse1F_EventScript_83E3B33
	end

Johto_PlayersHouse1F_EventScript_83E3B27::
	setobjectxyperm 1, 8, 4
	setobjectmovementtype 1, 8

Johto_PlayersHouse1F_EventScript_83E3B32::
	end

Johto_PlayersHouse1F_EventScript_83E3B33::
	setobjectxyperm 1, 8, 4
	setobjectmovementtype 1, 9
	clearflag FLAG_QZ_0x106C
	clearflag FLAG_QZ_0x106D
	return

@ Johto_PlayersHouse1F object 1 (local id 1, gfx 0xD7) at 2,2
Johto_PlayersHouse1F_EventScript_83E3B45::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x106D, Johto_PlayersHouse1F_EventScript_83E3B98
	goto_if_set FLAG_QZ_0x14A0, Johto_PlayersHouse1F_EventScript_83E3B83
	msgbox Johto_PlayersHouse1F_Text_83E3D12, MSGBOX_DEFAULT
	applymovement 1, Johto_PlayersHouse1F_Movement_83E3C23
	waitmovement 0
	release
	end

@ Johto_PlayersHouse1F object 2 (local id 2, gfx 0x14) at 5,4
Johto_PlayersHouse1F_EventScript_83E3B6D::
	lock
	faceplayer
	msgbox Johto_PlayersHouse1F_Text_83E3D44, MSGBOX_DEFAULT
	applymovement 2, Johto_PlayersHouse1F_Movement_83E3C21
	waitmovement 0
	release
	end

Johto_PlayersHouse1F_EventScript_83E3B83::
	msgbox Johto_PlayersHouse1F_Text_83E3DC0, MSGBOX_DEFAULT
	setflag FLAG_QZ_0x106D
	clearflag FLAG_QZ_0x107D
	setvar VAR_QZ_0x5003, 5
	release
	end

Johto_PlayersHouse1F_EventScript_83E3B98::
	call Johto_PlayersHouse1F_EventScript_83813E8
	release
	end

@ Johto_PlayersHouse1F trigger at 9,2 (VAR_QZ_0x5003 == 1)
Johto_PlayersHouse1F_EventScript_83E3B9F::
	lock
	applymovement LOCALID_PLAYER, Johto_PlayersHouse1F_Movement_83E3C0E
	waitmovement 0
	goto Johto_PlayersHouse1F_EventScript_83E3BC1

@ Johto_PlayersHouse1F trigger at 10,3 (VAR_QZ_0x5003 == 1)
Johto_PlayersHouse1F_EventScript_83E3BB0::
	lock
	applymovement LOCALID_PLAYER, Johto_PlayersHouse1F_Movement_83E3C15
	waitmovement 0
	goto Johto_PlayersHouse1F_EventScript_83E3BC1

Johto_PlayersHouse1F_EventScript_83E3BC1::
	playse SE_PIN
	applymovement 1, Quetzal_Movement_83607F9
	delay 0x3C
	applymovement 1, Johto_PlayersHouse1F_Movement_83E3C1A
	waitmovement 0
	msgbox Johto_PlayersHouse1F_Text_83E3C25, MSGBOX_DEFAULT
	call Johto_PlayersHouse1F_EventScript_83E3C01
	applymovement 1, Johto_PlayersHouse1F_Movement_83E3C1D
	waitmovement 0
	setvar VAR_QZ_0x5003, 2
	setvar VAR_QZ_0x5004, 0
	setflag FLAG_QZ_0x1074
	setflag FLAG_QZ_0x11BA
	releaseall
	end

Johto_PlayersHouse1F_EventScript_83E3C01::
	setflag FLAG_QZ_0xAD8
	msgbox Johto_PlayersHouse1F_Text_83E3C8A, MSGBOX_DEFAULT
	closemessage
	return

@ Johto_PlayersHouse1F object 3 (local id 3, gfx 0x3EB0) at 9,6
Johto_PlayersHouse1F_EventScript_84019F4::
	setvar VAR_QZ_0x5076, 0
	goto_if_unset FLAG_QZ_0x1318, Quetzal_EventScript_8401C95
	goto_if_ne VAR_QZ_0x506C, 1, Quetzal_EventScript_8401C95
	lockall
	special QzResetFollowerObjects_572
	getplayerxy VAR_0x8004, VAR_0x8005
	goto_if_ne VAR_0x8004, 9, Johto_PlayersHouse1F_EventScript_8401A2C
	goto_if_eq VAR_0x8005, 7, Johto_PlayersHouse1F_EventScript_8401A37

Johto_PlayersHouse1F_EventScript_8401A2C::
	fadescreen 1
	setobjectxy LOCALID_PLAYER, 9, 7
	fadescreen 0

Johto_PlayersHouse1F_EventScript_8401A37::
	turnobject LOCALID_PLAYER, 2
	closemessage
	fadenewbgm MUS_QZ_661
	applymovement 3, Johto_PlayersHouse1F_Movement_8404BF8
	waitmovement 3
	applymovement 1, Johto_PlayersHouse1F_Movement_8404BFB
	waitmovement 1
	msgbox Johto_PlayersHouse1F_Text_8405554, MSGBOX_DEFAULT
	closemessage
	applymovement 3, Johto_PlayersHouse1F_Movement_8404BFE
	playse SE_WALL_HIT
	applymovement 1, Johto_PlayersHouse1F_Movement_8404C01
	waitmovement 1
	waitmovement 3
	playmoncry SPECIES_EEVEE, 0
	waitmoncry
	setvar VAR_0x8004, 3
	setvar VAR_0x8005, 0xFF
	special QzObjectFaceObject_797
	msgbox Johto_PlayersHouse1F_Text_8402553, MSGBOX_DEFAULT
	setvar VAR_0x8004, 1
	setvar VAR_0x8005, 0xFF
	special QzObjectFaceObject_797
	msgbox Johto_PlayersHouse1F_Text_84025F8, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, Johto_PlayersHouse1F_Movement_8404C05
	waitmovement LOCALID_PLAYER
	turnobject 3, 3
	msgbox Johto_PlayersHouse1F_Text_84026AE, MSGBOX_DEFAULT
	setvar VAR_QZ_0x5076, 4
	cleartrainerflag 0x137B
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4987, 0, Johto_PlayersHouse1F_Text_840270C
	fadedefaultbgm
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, 1, Quetzal_EventScript_8401C4F
	setvar VAR_QZ_0x506C, 2
	setvar VAR_0x8004, 3
	setvar VAR_0x8005, 0xFF
	special QzObjectFaceObject_797
	msgbox Johto_PlayersHouse1F_Text_8402725, MSGBOX_DEFAULT
	closemessage
	applymovement 3, Johto_PlayersHouse1F_Movement_8404C09
	waitmovement 3
	playse SE_DOOR
	setflag FLAG_QZ_0x14A1
	removeobject 3
	applymovement LOCALID_PLAYER, Johto_PlayersHouse1F_Movement_8404C0E
	waitmovement LOCALID_PLAYER
	applymovement 1, Johto_PlayersHouse1F_Movement_8404C11
	waitmovement 1
	turnobject 4, 1
	playmoncry SPECIES_EEVEE, 0
	waitmoncry
	setvar VAR_QZ_0x5076, 0
	setvar VAR_0x8004, 1
	setvar VAR_0x8005, 0xFF
	special QzObjectFaceObject_797
	msgbox Johto_PlayersHouse1F_Text_8402770, MSGBOX_DEFAULT
	msgbox Johto_PlayersHouse1F_Text_8402837, MSGBOX_DEFAULT
	closemessage
	special QzHealPlayerPartyEx_0
	playfanfare MUS_HEAL
	waitfanfare
	goto Quetzal_EventScript_8401C94

@ Johto_PlayersHouse1F object 4 (local id 4, gfx 0x185) at 9,4
Johto_PlayersHouse1F_EventScript_8401BBE::
	lock
	playmoncry SPECIES_EEVEE, 0
	waitmoncry
	msgbox Johto_PlayersHouse1F_Text_8402A08, MSGBOX_DEFAULT
	goto Quetzal_EventScript_8401C94

Johto_PlayersHouse1F_EventScript_8401C72::
	lockall
	setvar VAR_0x8004, 1
	setvar VAR_0x8005, 0xFF
	special QzObjectFaceObject_797
	msgbox Johto_PlayersHouse1F_Text_8402CC2, MSGBOX_DEFAULT
	setvar VAR_QZ_0x5076, 0
	goto Quetzal_EventScript_8401C94

Johto_PlayersHouse1F_Movement_83E3C0E:
	delay_8
	walk_down
	walk_right
	walk_down
	face_left
	delay_8
	step_end

Johto_PlayersHouse1F_Movement_83E3C15:
	delay_8
	walk_down
	face_left
	delay_8
	step_end

Johto_PlayersHouse1F_Movement_83E3C1A:
	face_right
	walk_right
	step_end

Johto_PlayersHouse1F_Movement_83E3C1D:
	face_left
	walk_left
	delay_8
	step_end

Johto_PlayersHouse1F_Movement_83E3C21:
	face_right
	step_end

Johto_PlayersHouse1F_Movement_83E3C23:
	face_left
	step_end

Johto_PlayersHouse1F_Movement_8404BF8:
	walk_up
	face_left
	step_end

Johto_PlayersHouse1F_Movement_8404BFB:
	walk_down
	face_right
	step_end

Johto_PlayersHouse1F_Movement_8404BFE:
	walk_in_place_fast_left
	delay_16
	step_end

Johto_PlayersHouse1F_Movement_8404C01:
	walk_fast_up
	face_down
	delay_16
	step_end

Johto_PlayersHouse1F_Movement_8404C05:
	walk_up
	walk_up
	face_right
	step_end

Johto_PlayersHouse1F_Movement_8404C09:
	walk_down
	walk_down
	walk_left
	walk_down
	step_end

Johto_PlayersHouse1F_Movement_8404C0E:
	walk_right
	face_left
	step_end

Johto_PlayersHouse1F_Movement_8404C11:
	walk_down
	walk_left
	face_up
	step_end

Johto_PlayersHouse1F_Text_82DD80A::
	.string "MOM: How are you doing, {PLAYER}?\n"
	.string "You look a little tired.\p"
	.string "I think you should rest a bit.$"

Johto_PlayersHouse1F_Text_82DD85E::
	.string "MOM: Take care, honey!$"

Johto_PlayersHouse1F_Text_83E3C25::
	.string "Oh, {PLAYER}! Our neighbor,\n"
	.string "PROF. ELM, was looking for you.\p"
	.string "He said he wanted you to do\n"
	.string "something for him.$"

Johto_PlayersHouse1F_Text_83E3C8A::
	.string "MOM: With your RUNNING SHOES,\n"
	.string "hold B to run.\p"
	.string "While on foot, press L to turn\n"
	.string "automatic running on or off.\p"
	.string "When it is on, hold B to walk.$"

Johto_PlayersHouse1F_Text_83E3D12::
	.string "MOM: PROF.ELM is waiting for you.\p"
	.string "Hurry up, baby!$"

Johto_PlayersHouse1F_Text_83E3D44::
	.string "Hi, {PLAYER}!\p"
	.string "I’m visiting!\p"
	.string "Have you heard?\p"
	.string "My daughter is adamant about\n"
	.string "becoming PROF.ELM’s assistant.\p"
	.string "She really loves POKéMON!$"

Johto_PlayersHouse1F_Text_83E3DC0::
	.string "MOM: Wow, that’s a cute POKéMON.\n"
	.string "Where did you get it?\p"
	.string "…\p"
	.string "So, you’re leaving on an adventure...\n"
	.string "That does sound challenging...\p"
	.string "Just be careful.\p"
	.string "POKéMON are your friends.\n"
	.string "You need to work as a team.\p"
	.string "Now, go on!\n"
	.string "I’m behind you all the way!$"

Johto_PlayersHouse1F_Text_8402553::
	.string "ROCKET: Perfect.\n"
	.string "You can make her see reason.\p"
	.string "The boss demands EEVEE and her labor.\n"
	.string "But she refuses to obey.\p"
	.string "If she keeps refusing,\n"
	.string "our orders are to take her away.$"

Johto_PlayersHouse1F_Text_84025F8::
	.string "MOM: They said you helped\n"
	.string "bring GIOVANNI here...\p"
	.string "I told them it had to be a mistake.\n"
	.string "They want to force me to work for them.\p"
	.string "EEVEE has done nothing to them.\n"
	.string "I will not hand it over.$"

Johto_PlayersHouse1F_Text_84026AE::
	.string "ROCKET: What are you doing? Move!\p"
	.string "These are the boss’s orders!\n"
	.string "I won’t let you get in my way!$"

Johto_PlayersHouse1F_Text_840270C::
	.string "This is insubordination!$"

Johto_PlayersHouse1F_Text_8402725::
	.string "ROCKET: GIOVANNI will hear of this!\n"
	.string "I will tell him who turned against us!$"

Johto_PlayersHouse1F_Text_8402770::
	.string "MOM: Thank you for protecting us.\n"
	.string "I knew I could still trust you.\p"
	.string "But we are not the only ones. So many\n"
	.string "families cannot defend themselves.\p"
	.string "If you can stop him... please do.\n"
	.string "EEVEE and I will be here.$"

Johto_PlayersHouse1F_Text_8402837::
	.string "MOM: Before you go, let me\n"
	.string "take care of your Pokémon.\p"
	.string "They fought for us too.$"

Johto_PlayersHouse1F_Text_8402A08::
	.string "EEVEE stays close to your mother.\n"
	.string "It trusts her.$"

Johto_PlayersHouse1F_Text_8402CC2::
	.string "MOM: You’re back! I was making\n"
	.string "something to eat. Will you stay a while?\p"
	.string "What’s wrong? You look as though\n"
	.string "you haven’t seen me in ages.\p"
	.string "Come, sit down. Tell me\n"
	.string "about your trip.$"

Johto_PlayersHouse1F_Text_8405554::
	.string "MOM: Stay back!\n"
	.string "EEVEE is not going with you.$"
