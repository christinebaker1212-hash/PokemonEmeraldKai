Johto_EcruteakTinTowerEntrance_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_EcruteakTinTowerEntrance_EventScript_83CF05E
	.byte 0

Johto_EcruteakTinTowerEntrance_EventScript_83CF05E::
	goto_if_set FLAG_QZ_0x10D8, Johto_EcruteakTinTowerEntrance_EventScript_83CF068
	end

Johto_EcruteakTinTowerEntrance_EventScript_83CF068::
	setobjectxyperm 3, 0xF, 2
	end

@ Johto_EcruteakTinTowerEntrance sign / bg event at 14,1
Johto_EcruteakTinTowerEntrance_EventScript_83CF070::
	msgbox Johto_EcruteakTinTowerEntrance_Text_83CF079, MSGBOX_SIGN
	end

@ Johto_EcruteakTinTowerEntrance object 3 (local id 3, gfx 0x3EB2) at 16,2
Johto_EcruteakTinTowerEntrance_EventScript_83CF0B9::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x10D8, Johto_EcruteakTinTowerEntrance_EventScript_83CF123
	checkitem ITEM_CLEAR_BELL, 1
	goto_if_eq VAR_RESULT, 1, Johto_EcruteakTinTowerEntrance_EventScript_83CF0DF
	msgbox Johto_EcruteakTinTowerEntrance_Text_83CF16C, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_EcruteakTinTowerEntrance_EventScript_83CF0DF::
	msgbox Johto_EcruteakTinTowerEntrance_Text_83CF2D4, MSGBOX_DEFAULT
	waitmessage
	playse SE_M_PETAL_DANCE
	waitse
	playse SE_PIN
	applymovement 3, Quetzal_Movement_83607F9
	delay 0x3C
	msgbox Johto_EcruteakTinTowerEntrance_Text_83CF343, MSGBOX_DEFAULT
	closemessage
	applymovement 3, Johto_EcruteakTinTowerEntrance_Movement_83CF169
	waitmovement 0
	setobjectxyperm 3, 0xF, 2
	setflag FLAG_QZ_0x10D8
	release
	end

Johto_EcruteakTinTowerEntrance_EventScript_83CF123::
	msgbox Johto_EcruteakTinTowerEntrance_Text_83CF432, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_EcruteakTinTowerEntrance object 1 (local id 1, gfx 0x3EB2) at 10,3
Johto_EcruteakTinTowerEntrance_EventScript_83CF12E::
	lock
	faceplayer
	goto_if_eq VAR_QZ_0x500B, 10, Johto_EcruteakTinTowerEntrance_EventScript_83CF151
	goto_if_eq VAR_QZ_0x500B, 11, Johto_EcruteakTinTowerEntrance_EventScript_83CF151
	msgbox Johto_EcruteakTinTowerEntrance_Text_83CF47F, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_EcruteakTinTowerEntrance_EventScript_83CF151::
	msgbox Johto_EcruteakTinTowerEntrance_Text_83CF4E2, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_EcruteakTinTowerEntrance object 2 (local id 2, gfx 0x3EB2) at 3,3
Johto_EcruteakTinTowerEntrance_EventScript_83CF15C::
	lock
	faceplayer
	msgbox Johto_EcruteakTinTowerEntrance_Text_83CF520, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_EcruteakTinTowerEntrance_Movement_83CF169:
	walk_left
	face_down
	step_end

Johto_EcruteakTinTowerEntrance_Text_83CF079::
	.string "The bell of tin rings clearly for\n"
	.string "those under the tower’s wing.$"

Johto_EcruteakTinTowerEntrance_Text_83CF16C::
	.string "TIN TOWER is off limits to the\n"
	.string "unworthy. Sorry but you’ll have\l"
	.string "to leave.$"

Johto_EcruteakTinTowerEntrance_Text_83CF2D4::
	.string "A momentous event has occurred.\p"
	.string "I beg your pardon, but I must ask\n"
	.string "you to leave.\p"
	.string "..............................$"

Johto_EcruteakTinTowerEntrance_Text_83CF343::
	.string "Ah!\p"
	.string "The sound of that CLEAR BELL!\p"
	.string "It… It’s sublime!\p"
	.string "I’ve never heard so beautiful a\n"
	.string "sound before!\p"
	.string "That bell’s chime is indicative of\n"
	.string "the bearer’s soul.\p"
	.string "You…\p"
	.string "You may be able to make it through\n"
	.string "TIN TOWER.\p"
	.string "Please, do go on.$"

Johto_EcruteakTinTowerEntrance_Text_83CF432::
	.string "That bell’s chime is indicative of\n"
	.string "the bearer’s soul.\p"
	.string "You…\n"
	.string "Truly are worthy.$"

Johto_EcruteakTinTowerEntrance_Text_83CF47F::
	.string "The TIN TOWER ahead is a nine-\n"
	.string "tier tower of divine beauty.\p"
	.string "It soothes the soul of all who\n"
	.string "see it.$"

Johto_EcruteakTinTowerEntrance_Text_83CF4E2::
	.string "The TIN TOWER shook! A POKéMON must\n"
	.string "have returned to the top!$"

Johto_EcruteakTinTowerEntrance_Text_83CF520::
	.string "Two towers… Two POKéMON…\p"
	.string "But when one burned down, both\n"
	.string "POKéMON flew away, never to return.$"
