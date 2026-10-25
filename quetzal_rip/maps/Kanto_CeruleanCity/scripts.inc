Kanto_CeruleanCity_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Quetzal_EventScript_83C3036
	map_script MAP_SCRIPT_ON_RESUME, Quetzal_EventScript_83C3036
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_CeruleanCity_EventScript_84160CC
	.byte 0

Kanto_CeruleanCity_EventScript_84160CC::
	special QzKantoTravelingEventCheck_773
	setflag FLAG_QZ_0x2236
	call_if_unset FLAG_QZ_0x2162, Kanto_CeruleanCity_EventScript_84160E1
	call Quetzal_EventScript_840FBB5
	end

Kanto_CeruleanCity_EventScript_84160E1::
	setobjectxyperm 1, 0x1E, 0xC
	setobjectxyperm 5, 0x1A, 0x1F
	setobjectxyperm 6, 0x1B, 0x1F
	return

@ Kanto_CeruleanCity trigger at 22,6 (VAR_QZ_0x5108 == 0)
Kanto_CeruleanCity_EventScript_84160F7::
	lockall
	setvar VAR_TEMP_1, 0
	goto Kanto_CeruleanCity_EventScript_8416129

@ Kanto_CeruleanCity trigger at 23,6 (VAR_QZ_0x5108 == 0)
Kanto_CeruleanCity_EventScript_8416103::
	lockall
	setvar VAR_TEMP_1, 1
	setobjectxyperm 8, 0x17, 0
	goto Kanto_CeruleanCity_EventScript_8416129

@ Kanto_CeruleanCity trigger at 24,6 (VAR_QZ_0x5108 == 0)
Kanto_CeruleanCity_EventScript_8416116::
	lockall
	setvar VAR_TEMP_1, 2
	setobjectxyperm 8, 0x18, 0
	goto Kanto_CeruleanCity_EventScript_8416129

Kanto_CeruleanCity_EventScript_8416129::
	textcolor 0
	setvar VAR_QZ_0x5125, 2
	playbgm MUS_RG_ENCOUNTER_RIVAL, 0
	addobject 8
	applymovement 8, Kanto_CeruleanCity_Movement_8416214
	waitmovement 0
	msgbox Kanto_CeruleanCity_Text_842EC1A, MSGBOX_DEFAULT
	setvar VAR_LAST_TALKED, 8
	call Kanto_CeruleanCity_EventScript_84161C3
	setvar VAR_0x8004, 0xD
	setvar VAR_0x8005, 0
	special QzQuestLogNop_737
	msgbox Kanto_CeruleanCity_Text_842ECDA, MSGBOX_DEFAULT
	closemessage
	playbgm MUS_RG_RIVAL_EXIT, 0
	call_if_eq VAR_TEMP_1, 0, Kanto_CeruleanCity_EventScript_84161E4
	call_if_eq VAR_TEMP_1, 1, Kanto_CeruleanCity_EventScript_84161E4
	call_if_eq VAR_TEMP_1, 2, Kanto_CeruleanCity_EventScript_84161F6
	msgbox Kanto_CeruleanCity_Text_842EE1B, MSGBOX_DEFAULT
	setvar VAR_QZ_0x5108, 1
	setflag FLAG_QZ_0x2148
	setorcopyvar VAR_0x8000, 0x1D6
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	msgbox Kanto_CeruleanCity_Text_842EE6D, MSGBOX_DEFAULT
	closemessage
	applymovement 8, Kanto_CeruleanCity_Movement_841623C
	waitmovement 0
	fadedefaultbgm
	removeobject 8
	releaseall
	end

Kanto_CeruleanCity_EventScript_84161C3::
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_8524, 0, Kanto_CeruleanCity_Text_842ECB6
	return

Kanto_CeruleanCity_EventScript_84161E4::
	applymovement LOCALID_PLAYER, Kanto_CeruleanCity_Movement_8416208
	applymovement 8, Kanto_CeruleanCity_Movement_841621A
	waitmovement 0
	return

Kanto_CeruleanCity_EventScript_84161F6::
	applymovement LOCALID_PLAYER, Kanto_CeruleanCity_Movement_841620E
	applymovement 8, Kanto_CeruleanCity_Movement_841622B
	waitmovement 0
	return

@ Kanto_CeruleanCity object 2 (local id 2, gfx 0x3F3D) at 33,6
Kanto_CeruleanCity_EventScript_8416244::
	lock
	faceplayer
	checktrainerflag 0x2163
	goto_if_eq Kanto_CeruleanCity_EventScript_841626F
	message Kanto_CeruleanCity_Text_842EED4
	waitmessage
	playbgm MUS_RG_ENCOUNTER_ROCKET, 0
	waitbuttonpress
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_8547, 0, Kanto_CeruleanCity_Text_842EF36
	setvar VAR_QZ_0x5109, 1
	goto Kanto_CeruleanCity_EventScript_841626F

Kanto_CeruleanCity_EventScript_841626F::
	msgbox Kanto_CeruleanCity_Text_842EF62, MSGBOX_DEFAULT
	checkitemspace ITEM_SUN_STONE, 1
	goto_if_eq VAR_RESULT, 0, Kanto_CeruleanCity_EventScript_84162BB
	additem ITEM_SUN_STONE, 1
	loadword 0, Kanto_CeruleanCity_Text_842EF8C
	setorcopyvar VAR_0x8000, 0xD8
	setorcopyvar VAR_0x8001, 1
	setorcopyvar VAR_0x8002, 0x16F
	call Quetzal_EventScript_845571E
	msgbox Kanto_CeruleanCity_Text_842EFC1, MSGBOX_DEFAULT
	closemessage
	fadescreen 1
	removeobject 2
	fadescreen 0
	setflag FLAG_QZ_0x216E
	release
	end

Kanto_CeruleanCity_EventScript_84162BB::
	textcolor 0
	msgbox Kanto_CeruleanCity_Text_842EFDB, MSGBOX_DEFAULT
	release
	end

@ Kanto_CeruleanCity trigger at 33,5 (VAR_QZ_0x5109 == 0)
Kanto_CeruleanCity_EventScript_84162C7::
	lockall
	setvar VAR_TEMP_1, 0
	applymovement 2, Quetzal_Movement_8457AAC
	waitmovement 0
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AB0
	waitmovement 0
	goto Kanto_CeruleanCity_EventScript_8416307

@ Kanto_CeruleanCity trigger at 33,7 (VAR_QZ_0x5109 == 0)
Kanto_CeruleanCity_EventScript_84162E7::
	lockall
	setvar VAR_TEMP_1, 1
	applymovement 2, Quetzal_Movement_8457AB0
	waitmovement 0
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AAC
	waitmovement 0
	goto Kanto_CeruleanCity_EventScript_8416307

Kanto_CeruleanCity_EventScript_8416307::
	textcolor 0
	msgbox Kanto_CeruleanCity_Text_842EED4, MSGBOX_DEFAULT
	setvar VAR_LAST_TALKED, 2
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_8547, 0, Kanto_CeruleanCity_Text_842EF36
	setvar VAR_QZ_0x5109, 1
	goto Kanto_CeruleanCity_EventScript_841626F

@ Kanto_CeruleanCity object 1 (local id 1, gfx 0x3F36) at 29,12
Kanto_CeruleanCity_EventScript_841632B::
	lock
	faceplayer
	msgbox Kanto_CeruleanCity_Text_842F1C5, MSGBOX_DEFAULT
	closemessage
	applymovement 1, Quetzal_Movement_8457AA8
	waitmovement 0
	release
	end

@ Kanto_CeruleanCity object 3 (local id 3, gfx 0x3F17) at 16,21
Kanto_CeruleanCity_EventScript_8416342::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x2162, Kanto_CeruleanCity_EventScript_8416357
	msgbox Kanto_CeruleanCity_Text_842F0F1, MSGBOX_DEFAULT
	release
	end

Kanto_CeruleanCity_EventScript_8416357::
	msgbox Kanto_CeruleanCity_Text_842F05D, MSGBOX_DEFAULT
	release
	end

@ Kanto_CeruleanCity object 4 (local id 4, gfx 0x3EE6) at 9,23
Kanto_CeruleanCity_EventScript_8416361::
	msgbox Kanto_CeruleanCity_Text_842F186, MSGBOX_NPC
	end

@ Kanto_CeruleanCity object 7 (local id 7, gfx 0x3F5C) at 34,22
Kanto_CeruleanCity_EventScript_841636A::
	msgbox Kanto_CeruleanCity_Text_842F013, MSGBOX_NPC
	end

@ Kanto_CeruleanCity object 12 (local id 12, gfx 0x3EFD) at 1,13
Kanto_CeruleanCity_EventScript_8416373::
	msgbox Kanto_CeruleanCity_Text_842F3ED, MSGBOX_NPC
	end

@ Kanto_CeruleanCity object 11 (local id 11, gfx 0x3F57) at 12,30
Kanto_CeruleanCity_EventScript_841637C::
	lock
	faceplayer
	msgbox Kanto_CeruleanCity_Text_842F3A3, MSGBOX_DEFAULT
	closemessage
	applymovement 11, Quetzal_Movement_8457AA8
	waitmovement 0
	release
	end

@ Kanto_CeruleanCity object 6 (local id 6, gfx 0x3F16) at 33,29
Kanto_CeruleanCity_EventScript_8416393::
	lock
	random 3
	copyvar VAR_0x8008, VAR_RESULT
	call_if_eq VAR_0x8008, 0, Kanto_CeruleanCity_EventScript_84163F4
	call_if_eq VAR_0x8008, 1, Kanto_CeruleanCity_EventScript_84163FA
	call_if_eq VAR_0x8008, 2, Kanto_CeruleanCity_EventScript_8416400
	waitmessage
	delay 0x28
	playse SE_PIN
	applymovement 5, Quetzal_Movement_8457A9C
	waitmovement 0
	delay 0x1E
	call_if_eq VAR_0x8008, 0, Kanto_CeruleanCity_EventScript_8416406
	call_if_eq VAR_0x8008, 1, Kanto_CeruleanCity_EventScript_841640F
	call_if_eq VAR_0x8008, 2, Kanto_CeruleanCity_EventScript_8416418
	release
	end

Kanto_CeruleanCity_EventScript_84163F4::
	message Kanto_CeruleanCity_Text_842F257
	return

Kanto_CeruleanCity_EventScript_84163FA::
	message Kanto_CeruleanCity_Text_842F296
	return

Kanto_CeruleanCity_EventScript_8416400::
	message Kanto_CeruleanCity_Text_842F2BD
	return

Kanto_CeruleanCity_EventScript_8416406::
	msgbox Kanto_CeruleanCity_Text_842F275, MSGBOX_DEFAULT
	return

Kanto_CeruleanCity_EventScript_841640F::
	msgbox Kanto_CeruleanCity_Text_842F2A6, MSGBOX_DEFAULT
	return

Kanto_CeruleanCity_EventScript_8416418::
	msgbox Kanto_CeruleanCity_Text_842F2D0, MSGBOX_DEFAULT
	return

@ Kanto_CeruleanCity object 5 (local id 5, gfx 0x3F46) at 32,29
Kanto_CeruleanCity_EventScript_8416421::
	lock
	random 4
	copyvar VAR_0x8008, VAR_RESULT
	call_if_eq VAR_0x8008, 0, Kanto_CeruleanCity_EventScript_8416458
	call_if_eq VAR_0x8008, 1, Kanto_CeruleanCity_EventScript_8416461
	call_if_eq VAR_0x8008, 2, Kanto_CeruleanCity_EventScript_841646A
	call_if_eq VAR_0x8008, 3, Kanto_CeruleanCity_EventScript_8416473
	release
	end

Kanto_CeruleanCity_EventScript_8416458::
	msgbox Kanto_CeruleanCity_Text_842F344, MSGBOX_DEFAULT
	return

Kanto_CeruleanCity_EventScript_8416461::
	msgbox Kanto_CeruleanCity_Text_842F35B, MSGBOX_DEFAULT
	return

Kanto_CeruleanCity_EventScript_841646A::
	msgbox Kanto_CeruleanCity_Text_842F376, MSGBOX_DEFAULT
	return

Kanto_CeruleanCity_EventScript_8416473::
	msgbox Kanto_CeruleanCity_Text_842F38B, MSGBOX_DEFAULT
	return

@ Kanto_CeruleanCity sign / bg event at 20,25
Kanto_CeruleanCity_EventScript_841647C::
	msgbox Kanto_CeruleanCity_Text_842F4F4, MSGBOX_SIGN
	end

@ Kanto_CeruleanCity sign / bg event at 19,32
Kanto_CeruleanCity_EventScript_8416485::
	msgbox Kanto_CeruleanCity_Text_842F527, MSGBOX_SIGN
	end

@ Kanto_CeruleanCity sign / bg event at 11,28
Kanto_CeruleanCity_EventScript_841648E::
	msgbox Kanto_CeruleanCity_Text_842F599, MSGBOX_SIGN
	end

@ Kanto_CeruleanCity sign / bg event at 27,21
Kanto_CeruleanCity_EventScript_8416497::
	lockall
	setvar VAR_0x8004, 3
	setvar VAR_0x8005, 0
	special QzQuestLogNop_737
	msgbox Kanto_CeruleanCity_Text_842F5C3, MSGBOX_DEFAULT
	releaseall
	end

Kanto_CeruleanCity_Movement_8416208:
	delay_16
	delay_8
	walk_in_place_faster_right
	delay_16
	walk_in_place_faster_down
	step_end

Kanto_CeruleanCity_Movement_841620E:
	delay_16
	delay_8
	walk_in_place_faster_left
	delay_16
	walk_in_place_faster_down
	step_end

Kanto_CeruleanCity_Movement_8416214:
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	step_end

Kanto_CeruleanCity_Movement_841621A:
	walk_right
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	delay_16
	delay_16
	walk_left
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

Kanto_CeruleanCity_Movement_841622B:
	walk_left
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	delay_16
	delay_16
	walk_right
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

Kanto_CeruleanCity_Movement_841623C:
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	step_end

Kanto_CeruleanCity_Text_842EC1A::
	.string "BLUE: Yo! {PLAYER}!\p"
	.string "You’re still struggling along back\n"
	.string "here?\p"
	.string "I’m doing great! I caught a bunch\n"
	.string "of strong and smart POKéMON!\p"
	.string "Here, let me see what you caught,\n"
	.string "{PLAYER}!$"

Kanto_CeruleanCity_Text_842ECB6::
	.string "Hey!\n"
	.string "Take it easy!\l"
	.string "You won already!$"

Kanto_CeruleanCity_Text_842ECDA::
	.string "BLUE: Hey, guess what?\p"
	.string "I went to BILL’s and got him to\n"
	.string "show me his rare POKéMON.\p"
	.string "That added a lot of pages to my\n"
	.string "POKéDEX!\p"
	.string "After all, BILL’s world famous as a\n"
	.string "POKéMANIAC.\p"
	.string "He invented the POKéMON Storage\n"
	.string "System on PC, too.\p"
	.string "Since you’re using his system,\n"
	.string "you should go thank him.\p"
	.string "Well, I better get rolling!\n"
	.string "Smell ya later!$"

Kanto_CeruleanCity_Text_842EE1B::
	.string "Oh, yeah, right.\p"
	.string "I picked up an extra LUCKY EGG.\n"
	.string "You can have it--try to keep up!$"

Kanto_CeruleanCity_Text_842EE6D::
	.string "Have a POKéMON hold it to gain\n"
	.string "more EXP. Don’t fall behind!\p"
	.string "All right, smell ya!$"

Kanto_CeruleanCity_Text_842EED4::
	.string "Hey! Stay out!\n"
	.string "It’s not your yard!\p"
	.string "…Huh?\n"
	.string "Me?\p"
	.string "I’m an innocent bystander!\n"
	.string "Don’t you believe me?{PLAY_BGM 0x01EF}$"

Kanto_CeruleanCity_Text_842EF36::
	.string "GRUNT: Stop! I give up!\n"
	.string "I’ll leave quietly!$"

Kanto_CeruleanCity_Text_842EF62::
	.string "…Okay.\n"
	.string "I’ll return the SUN STONE I stole.$"

Kanto_CeruleanCity_Text_842EF8C::
	.string "{PLAYER} recovered a SUN STONE\n"
	.string "from the TEAM ROCKET GRUNT.$"

Kanto_CeruleanCity_Text_842EFC1::
	.string "I better get moving!\n"
	.string "Bye!$"

Kanto_CeruleanCity_Text_842EFDB::
	.string "Make room for this!\n"
	.string "I can’t run until I give it to you!$"

Kanto_CeruleanCity_Text_842F013::
	.string "You’re a TRAINER, too?\p"
	.string "Collecting, battling…\n"
	.string "It’s a tough life, isn’t it?$"

Kanto_CeruleanCity_Text_842F05D::
	.string "Did you know that you can CUT down\n"
	.string "small trees?\p"
	.string "Even that small tree in front of\n"
	.string "the shop can be CUT down.\p"
	.string "I think there’s a way around it,\n"
	.string "though.$"

Kanto_CeruleanCity_Text_842F0F1::
	.string "If that SLOWBRO wasn’t there, you\n"
	.string "could CUT down the small tree.\p"
	.string "That way, you could get to the\n"
	.string "other side.\p"
	.string "I think there’s a way around it,\n"
	.string "though.$"

Kanto_CeruleanCity_Text_842F186::
	.string "You’re making an encyclopedia on\n"
	.string "POKéMON? That sounds amusing.$"

Kanto_CeruleanCity_Text_842F1C5::
	.string "The people here were robbed.\p"
	.string "It’s obvious that TEAM ROCKET is\n"
	.string "behind this most heinous crime!\p"
	.string "Even our POLICE FORCE has\n"
	.string "trouble with the ROCKETS!$"

Kanto_CeruleanCity_Text_842F257::
	.string "Okay! SLOWBRO!\n"
	.string "Use SONICBOOM!$"

Kanto_CeruleanCity_Text_842F275::
	.string "Come on, SLOWBRO, pay attention!$"

Kanto_CeruleanCity_Text_842F296::
	.string "SLOWBRO, punch!$"

Kanto_CeruleanCity_Text_842F2A6::
	.string "No!\n"
	.string "You blew it again!$"

Kanto_CeruleanCity_Text_842F2BD::
	.string "SLOWBRO, WITHDRAW!$"

Kanto_CeruleanCity_Text_842F2D0::
	.string "No! That’s wrong!\n"
	.string "It’s so hard to control POKéMON!\p"
	.string "Your POKéMON’s obedience depends\n"
	.string "on your abilities as a TRAINER.$"

Kanto_CeruleanCity_Text_842F344::
	.string "SLOWBRO took a snooze…$"

Kanto_CeruleanCity_Text_842F35B::
	.string "SLOWBRO is loafing around…$"

Kanto_CeruleanCity_Text_842F376::
	.string "SLOWBRO turned away…$"

Kanto_CeruleanCity_Text_842F38B::
	.string "SLOWBRO ignored orders…$"

Kanto_CeruleanCity_Text_842F3A3::
	.string "I want a bright red bicycle.\p"
	.string "I’ll keep it at home, so it won’t\n"
	.string "get dirty.$"

Kanto_CeruleanCity_Text_842F3ED::
	.string "This is CERULEAN CAVE.\p"
	.string "Horribly strong POKéMON live inside\n"
	.string "there.\p"
	.string "It takes a very special TRAINER to\n"
	.string "be allowed inside there.\p"
	.string "You’d have to be strong enough to\n"
	.string "become the POKéMON LEAGUE\l"
	.string "CHAMPION for starters.\p"
	.string "And, you would have to have\n"
	.string "made a great achievement.$"

Kanto_CeruleanCity_Text_842F4F4::
	.string "CERULEAN CITY\n"
	.string "A Mysterious, Blue Aura Surrounds\l"
	.string "It$"

Kanto_CeruleanCity_Text_842F527::
	.string "TRAINER TIPS\p"
	.string "A POKéMON can be made to hold\n"
	.string "an item.\p"
	.string "Some items can even be used by\n"
	.string "the holding POKéMON in battle.$"

Kanto_CeruleanCity_Text_842F599::
	.string "Grass and caves handled easily!\n"
	.string "BIKE SHOP$"

Kanto_CeruleanCity_Text_842F5C3::
	.string "CERULEAN CITY POKéMON GYM\n"
	.string "LEADER: MISTY\l"
	.string "The Tomboyish Mermaid!$"
