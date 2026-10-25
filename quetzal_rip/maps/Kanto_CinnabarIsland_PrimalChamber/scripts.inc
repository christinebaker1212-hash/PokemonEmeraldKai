Kanto_CinnabarIsland_PrimalChamber_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Kanto_CinnabarIsland_PrimalChamber_EventScript_8422176
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840FBB9
	.byte 0

Kanto_CinnabarIsland_PrimalChamber_EventScript_8422176::
	setmetatile 0xA, 0x12, 0x207, 0
	end

@ Kanto_CinnabarIsland_PrimalChamber object 1 (local id 1, gfx 0xC4) at 10,14
Kanto_CinnabarIsland_PrimalChamber_EventScript_8422180::
	checktrainerflag 0x22E7
	goto_if_eq Kanto_CinnabarIsland_PrimalChamber_EventScript_84221DE
	trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT, TRAINER_QZ_8935, 0, Kanto_CinnabarIsland_PrimalChamber_Text_8422341, Kanto_CinnabarIsland_PrimalChamber_Text_84223FD, Kanto_CinnabarIsland_PrimalChamber_EventScript_842219D
	release
	end

Kanto_CinnabarIsland_PrimalChamber_EventScript_842219D::
	lockall
	special Script_ClearHeldMovement
	msgbox Kanto_CinnabarIsland_PrimalChamber_Text_842241E, MSGBOX_DEFAULT
	closemessage
	special QzResetFollowerObjects_572
	call Kanto_CinnabarIsland_PrimalChamber_EventScript_8422328
	fadescreen 1
	clearflag FLAG_QZ_0x23A9
	addobject 3
	turnobject 1, 2
	turnobject LOCALID_PLAYER, 2
	delay 0x14
	fadescreen 0
	playmoncry SPECIES_GROUDON, 2
	waitmoncry
	msgbox Kanto_CinnabarIsland_PrimalChamber_Text_8422487, MSGBOX_DEFAULT
	closemessage
	special Script_FacePlayer
	goto Kanto_CinnabarIsland_PrimalChamber_EventScript_8422214

Kanto_CinnabarIsland_PrimalChamber_EventScript_84221DE::
	lockall
	faceplayer
	goto_if_unset FLAG_QZ_0x2EB5, Kanto_CinnabarIsland_PrimalChamber_EventScript_8422214
	msgbox Kanto_CinnabarIsland_PrimalChamber_Text_8422527, MSGBOX_YESNO
	goto_if_ne VAR_RESULT, 1, Kanto_CinnabarIsland_PrimalChamber_EventScript_8422325
	closemessage
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_8935, 0, Kanto_CinnabarIsland_PrimalChamber_Text_84223FD
	msgbox Kanto_CinnabarIsland_PrimalChamber_Text_8422577, MSGBOX_DEFAULT
	goto Kanto_CinnabarIsland_PrimalChamber_EventScript_8422325

Kanto_CinnabarIsland_PrimalChamber_EventScript_8422214::
	goto_if_set FLAG_QZ_0x2EB5, Kanto_CinnabarIsland_PrimalChamber_EventScript_8422325
	msgbox Kanto_CinnabarIsland_PrimalChamber_Text_84224C1, MSGBOX_DEFAULT
	setvar VAR_0x8000, 0x122
	call Kanto_CinnabarIsland_PrimalChamber_EventScript_8422308
	goto_if_eq VAR_RESULT, 0, Kanto_CinnabarIsland_PrimalChamber_EventScript_842231D
	setflag FLAG_QZ_0x2EB5
	goto Kanto_CinnabarIsland_PrimalChamber_EventScript_8422325

@ Kanto_CinnabarIsland_PrimalChamber object 2 (local id 2, gfx 0xC3) at 40,14
Kanto_CinnabarIsland_PrimalChamber_EventScript_8422242::
	checktrainerflag 0x22E8
	goto_if_eq Kanto_CinnabarIsland_PrimalChamber_EventScript_84222A4
	trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT, TRAINER_QZ_8936, 0, Kanto_CinnabarIsland_PrimalChamber_Text_84225A9, Kanto_CinnabarIsland_PrimalChamber_Text_8422657, Kanto_CinnabarIsland_PrimalChamber_EventScript_842225F
	release
	end

Kanto_CinnabarIsland_PrimalChamber_EventScript_842225F::
	lockall
	special Script_ClearHeldMovement
	msgbox Kanto_CinnabarIsland_PrimalChamber_Text_8422679, MSGBOX_DEFAULT
	closemessage
	special QzResetFollowerObjects_572
	call Kanto_CinnabarIsland_PrimalChamber_EventScript_8422328
	playse SE_M_SURF
	waitse
	fadescreen 1
	clearflag FLAG_QZ_0x2E3A
	addobject 4
	turnobject 2, 2
	turnobject LOCALID_PLAYER, 2
	delay 0x14
	fadescreen 0
	playmoncry SPECIES_KYOGRE, 2
	waitmoncry
	msgbox Kanto_CinnabarIsland_PrimalChamber_Text_84226E3, MSGBOX_DEFAULT
	closemessage
	special Script_FacePlayer
	goto Kanto_CinnabarIsland_PrimalChamber_EventScript_84222DA

Kanto_CinnabarIsland_PrimalChamber_EventScript_84222A4::
	lockall
	faceplayer
	goto_if_unset FLAG_QZ_0x2EB6, Kanto_CinnabarIsland_PrimalChamber_EventScript_84222DA
	msgbox Kanto_CinnabarIsland_PrimalChamber_Text_842277F, MSGBOX_YESNO
	goto_if_ne VAR_RESULT, 1, Kanto_CinnabarIsland_PrimalChamber_EventScript_8422325
	closemessage
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_8936, 0, Kanto_CinnabarIsland_PrimalChamber_Text_8422657
	msgbox Kanto_CinnabarIsland_PrimalChamber_Text_84227BE, MSGBOX_DEFAULT
	goto Kanto_CinnabarIsland_PrimalChamber_EventScript_8422325

Kanto_CinnabarIsland_PrimalChamber_EventScript_84222DA::
	goto_if_set FLAG_QZ_0x2EB6, Kanto_CinnabarIsland_PrimalChamber_EventScript_8422325
	msgbox Kanto_CinnabarIsland_PrimalChamber_Text_8422727, MSGBOX_DEFAULT
	setvar VAR_0x8000, 0x123
	call Kanto_CinnabarIsland_PrimalChamber_EventScript_8422308
	goto_if_eq VAR_RESULT, 0, Kanto_CinnabarIsland_PrimalChamber_EventScript_842231D
	setflag FLAG_QZ_0x2EB6
	goto Kanto_CinnabarIsland_PrimalChamber_EventScript_8422325

Kanto_CinnabarIsland_PrimalChamber_EventScript_8422308::
	setvar VAR_0x8001, 1
	additem VAR_0x8000, VAR_0x8001
	copyvar VAR_0x8007, VAR_RESULT
	call Quetzal_EventScript_835FB62
	return

Kanto_CinnabarIsland_PrimalChamber_EventScript_842231D::
	msgbox Kanto_CinnabarIsland_PrimalChamber_Text_84227F9, MSGBOX_DEFAULT

Kanto_CinnabarIsland_PrimalChamber_EventScript_8422325::
	closemessage
	releaseall
	end

Kanto_CinnabarIsland_PrimalChamber_EventScript_8422328::
	setvar VAR_0x8004, 1
	setvar VAR_0x8005, 2
	setvar VAR_0x8006, 0xC
	setvar VAR_0x8007, 3
	special ShakeCamera
	waitstate
	return

Kanto_CinnabarIsland_PrimalChamber_Text_8422341::
	.string "MAXIE: Beneath this island sleeps\n"
	.string "the power that raised continents.\p"
	.string "GROUDON has sensed your arrival.\n"
	.string "First, prove your strength to me.\p"
	.string "My team will burn with all\n"
	.string "the fury of this volcano!$"

Kanto_CinnabarIsland_PrimalChamber_Text_84223FD::
	.string "Your will surpasses the volcano!$"

Kanto_CinnabarIsland_PrimalChamber_Text_842241E::
	.string "MAXIE: I cannot hold you back.\n"
	.string "The earth knows it, too...\p"
	.string "Do you feel that tremor?\n"
	.string "GROUDON is awakening!$"

Kanto_CinnabarIsland_PrimalChamber_Text_8422487::
	.string "MAXIE: So this is the power\n"
	.string "that raised the continents...$"

Kanto_CinnabarIsland_PrimalChamber_Text_84224C1::
	.string "If GROUDON chooses to join you,\n"
	.string "this orb will awaken its true power.\p"
	.string "I want you to have this Red Orb.$"

Kanto_CinnabarIsland_PrimalChamber_Text_8422527::
	.string "There is still much to learn\n"
	.string "from this place.\p"
	.string "Shall we test our strength again?$"

Kanto_CinnabarIsland_PrimalChamber_Text_8422577::
	.string "A battle always reveals\n"
	.string "something we can improve.$"

Kanto_CinnabarIsland_PrimalChamber_Text_84225A9::
	.string "ARCHIE: These waters reach\n"
	.string "the depths of the ocean.\p"
	.string "KYOGRE has sensed your arrival.\n"
	.string "I won’t let just anyone pass!\p"
	.string "Withstand the might of my team\n"
	.string "and show me what you can do!$"

Kanto_CinnabarIsland_PrimalChamber_Text_8422657::
	.string "You have overcome the great tide!$"

Kanto_CinnabarIsland_PrimalChamber_Text_8422679::
	.string "ARCHIE: Ha! What a battle!\n"
	.string "Even the ocean answers you...\p"
	.string "Look at the churning water!\n"
	.string "KYOGRE is awakening!$"

Kanto_CinnabarIsland_PrimalChamber_Text_84226E3::
	.string "ARCHIE: What a presence!\p"
	.string "Now I understand the legends\n"
	.string "of the ocean.$"

Kanto_CinnabarIsland_PrimalChamber_Text_8422727::
	.string "If KYOGRE chooses to join you,\n"
	.string "this orb will awaken its true power.\p"
	.string "Take this Blue Orb!$"

Kanto_CinnabarIsland_PrimalChamber_Text_842277F::
	.string "I’ve still got strength\n"
	.string "for another battle!\p"
	.string "Are you up for it?$"

Kanto_CinnabarIsland_PrimalChamber_Text_84227BE::
	.string "What strength you’ve got!\n"
	.string "I’ll be here for the next round!$"

Kanto_CinnabarIsland_PrimalChamber_Text_84227F9::
	.string "Make room in your Bag and return.\n"
	.string "I’ll keep the orb for you.$"
