Johto_RuinsOfAlphResearchCenter_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840EA66
	.byte 0

@ Johto_RuinsOfAlphResearchCenter object 2 (local id 2, gfx 0x3EB4) at 1,3
Johto_RuinsOfAlphResearchCenter_EventScript_83F6E83::
	lock
	faceplayer
	msgbox Johto_RuinsOfAlphResearchCenter_Text_83F6EB4, MSGBOX_DEFAULT
	pokemart Johto_RuinsOfAlphResearchCenter_Mart_82F6F64
	msgbox Quetzal_Text_8360D20, MSGBOX_DEFAULT
	release
	end

@ Johto_RuinsOfAlphResearchCenter object 1 (local id 1, gfx 0x3EB3) at 8,5
Johto_RuinsOfAlphResearchCenter_EventScript_83F6E9C::
	lock
	faceplayer
	msgbox Johto_RuinsOfAlphResearchCenter_Text_83F6F2D, MSGBOX_DEFAULT
	release
	end

@ Johto_RuinsOfAlphResearchCenter sign / bg event at 5,4
@ Johto_RuinsOfAlphResearchCenter sign / bg event at 6,4
@ Johto_RuinsOfAlphResearchCenter sign / bg event at 7,4
Johto_RuinsOfAlphResearchCenter_EventScript_83F6EA8::
	lock
	msgbox Johto_RuinsOfAlphResearchCenter_Text_83F6FA1, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_RuinsOfAlphResearchCenter object 3 (local id 3, gfx 0x3EB4) at 7,2
Johto_RuinsOfAlphResearchCenter_EventScript_83F6FE3::
	lock
	faceplayer
	call Quetzal_EventScript_8361A4D
	goto_if_ne VAR_QZ_0x5033, 0, Johto_RuinsOfAlphResearchCenter_EventScript_83F7140

Johto_RuinsOfAlphResearchCenter_EventScript_83F6FF5::
	msgbox Johto_RuinsOfAlphResearchCenter_Text_83F75AF, MSGBOX_DEFAULT
	special QzBagHasFossil
	goto_if_eq VAR_RESULT, 1, Johto_RuinsOfAlphResearchCenter_EventScript_83F700D
	release
	end

Johto_RuinsOfAlphResearchCenter_EventScript_83F700D::
	closemessage
	playse SE_PIN
	applymovement 3, Quetzal_Movement_83607F9
	waitmovement 0
	applymovement 3, Quetzal_Movement_83607FB
	waitmovement 0
	msgbox Quetzal_Text_83F760D, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Johto_RuinsOfAlphResearchCenter_EventScript_83F7212
	special QzChooseItemFromBagForFossil
	waitstate
	goto_if_eq VAR_ITEM_ID, 0, Johto_RuinsOfAlphResearchCenter_EventScript_83F7212
	special QzIsSelectedItemFossil
	goto_if_eq VAR_RESULT, 0, Johto_RuinsOfAlphResearchCenter_EventScript_83F7208
	special QzIsSelectedItemFossilHalf
	goto_if_eq VAR_RESULT, 1, Johto_RuinsOfAlphResearchCenter_EventScript_83F7069
	goto Johto_RuinsOfAlphResearchCenter_EventScript_83F70C3

Johto_RuinsOfAlphResearchCenter_EventScript_83F7069::
	msgbox Quetzal_Text_82F8280, MSGBOX_DEFAULT
	special QzChooseItemFromBagForFossil
	waitstate
	goto_if_eq VAR_ITEM_ID, 0, Johto_RuinsOfAlphResearchCenter_EventScript_83F7212
	special QzIsSelectedItemFossil
	goto_if_eq VAR_RESULT, 0, Johto_RuinsOfAlphResearchCenter_EventScript_83F70A2
	special QzAreFossilHalvesCompatible
	goto_if_eq VAR_RESULT, 0, Johto_RuinsOfAlphResearchCenter_EventScript_83F70B0
	goto Johto_RuinsOfAlphResearchCenter_EventScript_83F70E2

Johto_RuinsOfAlphResearchCenter_EventScript_83F70A2::
	msgbox Quetzal_Text_83F721C, MSGBOX_DEFAULT
	goto Johto_RuinsOfAlphResearchCenter_EventScript_83F7069

Johto_RuinsOfAlphResearchCenter_EventScript_83F70B0::
	copyvar VAR_0x800B, VAR_ITEM_ID
	msgbox Quetzal_Text_83F721C, MSGBOX_DEFAULT
	goto Johto_RuinsOfAlphResearchCenter_EventScript_83F7069

Johto_RuinsOfAlphResearchCenter_EventScript_83F70C3::
	special QzGiveRevivedFossilMon
	goto_if_eq VAR_RESULT, 0, Johto_RuinsOfAlphResearchCenter_EventScript_83F7101
	goto_if_eq VAR_RESULT, 1, Johto_RuinsOfAlphResearchCenter_EventScript_83F7101
	goto Johto_RuinsOfAlphResearchCenter_EventScript_83F7202

Johto_RuinsOfAlphResearchCenter_EventScript_83F70E2::
	special QzGiveRevivedFossilMon
	goto_if_eq VAR_RESULT, 0, Johto_RuinsOfAlphResearchCenter_EventScript_83F7118
	goto_if_eq VAR_RESULT, 1, Johto_RuinsOfAlphResearchCenter_EventScript_83F7118
	goto Johto_RuinsOfAlphResearchCenter_EventScript_83F7202

Johto_RuinsOfAlphResearchCenter_EventScript_83F7101::
	bufferitemname 0, VAR_ITEM_ID
	msgbox Johto_RuinsOfAlphResearchCenter_Text_83F77E2, MSGBOX_DEFAULT
	removeitem VAR_ITEM_ID, 1
	goto Johto_RuinsOfAlphResearchCenter_EventScript_83F71DD

Johto_RuinsOfAlphResearchCenter_EventScript_83F7118::
	bufferitemname 0, VAR_ITEM_ID
	msgbox Johto_RuinsOfAlphResearchCenter_Text_83F77E2, MSGBOX_DEFAULT
	removeitem VAR_ITEM_ID, 1
	bufferitemname 0, VAR_0x800B
	msgbox Johto_RuinsOfAlphResearchCenter_Text_83F77E2, MSGBOX_DEFAULT
	removeitem VAR_0x800B, 1
	goto Johto_RuinsOfAlphResearchCenter_EventScript_83F71DD

Johto_RuinsOfAlphResearchCenter_EventScript_83F7140::
	goto_if_eq VAR_QZ_0x5034, 1, Johto_RuinsOfAlphResearchCenter_EventScript_83F7187
	goto_if_eq VAR_QZ_0x5034, 2, Johto_RuinsOfAlphResearchCenter_EventScript_83F7192
	goto_if_eq VAR_QZ_0x5034, 3, Johto_RuinsOfAlphResearchCenter_EventScript_83F719D
	goto_if_eq VAR_QZ_0x5034, 4, Johto_RuinsOfAlphResearchCenter_EventScript_83F71A8
	goto_if_eq VAR_QZ_0x5034, 5, Johto_RuinsOfAlphResearchCenter_EventScript_83F71B3
	setvar VAR_QZ_0x5033, 0
	setvar VAR_QZ_0x5034, 0
	goto Johto_RuinsOfAlphResearchCenter_EventScript_83F6FF5

Johto_RuinsOfAlphResearchCenter_EventScript_83F7187::
	setvar VAR_ITEM_ID, ITEM_ROOT_FOSSIL
	goto Johto_RuinsOfAlphResearchCenter_EventScript_83F71BE

Johto_RuinsOfAlphResearchCenter_EventScript_83F7192::
	setvar VAR_ITEM_ID, ITEM_CLAW_FOSSIL
	goto Johto_RuinsOfAlphResearchCenter_EventScript_83F71BE

Johto_RuinsOfAlphResearchCenter_EventScript_83F719D::
	setvar VAR_ITEM_ID, ITEM_HELIX_FOSSIL
	goto Johto_RuinsOfAlphResearchCenter_EventScript_83F71BE

Johto_RuinsOfAlphResearchCenter_EventScript_83F71A8::
	setvar VAR_ITEM_ID, ITEM_DOME_FOSSIL
	goto Johto_RuinsOfAlphResearchCenter_EventScript_83F71BE

Johto_RuinsOfAlphResearchCenter_EventScript_83F71B3::
	setvar VAR_ITEM_ID, ITEM_OLD_AMBER
	goto Johto_RuinsOfAlphResearchCenter_EventScript_83F71BE

Johto_RuinsOfAlphResearchCenter_EventScript_83F71BE::
	special QzGiveRevivedFossilMon
	goto_if_eq VAR_RESULT, 0, Johto_RuinsOfAlphResearchCenter_EventScript_83F71DD
	goto_if_eq VAR_RESULT, 1, Johto_RuinsOfAlphResearchCenter_EventScript_83F71DD
	goto Johto_RuinsOfAlphResearchCenter_EventScript_83F7202

Johto_RuinsOfAlphResearchCenter_EventScript_83F71DD::
	setvar VAR_QZ_0x5033, 0
	setvar VAR_QZ_0x5034, 0
	setflag FLAG_QZ_0x10B
	bufferspeciesname 1, VAR_0x800A
	msgbox Johto_RuinsOfAlphResearchCenter_Text_83F78E0, MSGBOX_DEFAULT
	playfanfare MUS_OBTAIN_ITEM
	message Johto_RuinsOfAlphResearchCenter_Text_83F795A
	waitmessage
	waitfanfare
	release
	end

Johto_RuinsOfAlphResearchCenter_EventScript_83F7202::
	goto Quetzal_EventScript_8361BCF

Johto_RuinsOfAlphResearchCenter_EventScript_83F7208::
	msgbox Johto_RuinsOfAlphResearchCenter_Text_83F76F4, MSGBOX_DEFAULT
	release
	end

Johto_RuinsOfAlphResearchCenter_EventScript_83F7212::
	msgbox Johto_RuinsOfAlphResearchCenter_Text_83F76AD, MSGBOX_DEFAULT
	release
	end

Johto_RuinsOfAlphResearchCenter_Mart_82F6F64:
	.2byte ITEM_FLAME_PLATE
	.2byte ITEM_SPLASH_PLATE
	.2byte ITEM_ZAP_PLATE
	.2byte ITEM_MEADOW_PLATE
	.2byte ITEM_ICICLE_PLATE
	.2byte ITEM_FIST_PLATE
	.2byte ITEM_TOXIC_PLATE
	.2byte ITEM_EARTH_PLATE
	.2byte ITEM_SKY_PLATE
	.2byte ITEM_MIND_PLATE
	.2byte ITEM_INSECT_PLATE
	.2byte ITEM_STONE_PLATE
	.2byte ITEM_SPOOKY_PLATE
	.2byte ITEM_DRACO_PLATE
	.2byte ITEM_DREAD_PLATE
	.2byte ITEM_IRON_PLATE
	.2byte ITEM_NONE

Johto_RuinsOfAlphResearchCenter_Text_83F6EB4::
	.string "These RUINS are about 1500 years old.\n"
	.string "Their origins remain a mystery.\p"
	.string "I also sell research supplies.\n"
	.string "Please take a look!$"

Johto_RuinsOfAlphResearchCenter_Text_83F6F2D::
	.string "There are odd patterns drawn on\n"
	.string "on the walls of the RUINS.\p"
	.string "They must be the keys for\n"
	.string "unraveling the RUINS’ mystery.$"

Johto_RuinsOfAlphResearchCenter_Text_83F6FA1::
	.string "Fossils found within the RUINS.\n"
	.string "Maybe there are more somewhere...$"

Johto_RuinsOfAlphResearchCenter_Text_83F75AF::
	.string "I’ve been trying to develop a device\n"
	.string "that resurrects POKéMON from fossils…\p"
	.string "And, it’s working!$"

Johto_RuinsOfAlphResearchCenter_Text_83F76AD::
	.string "Oh, is that so?\p"
	.string "My technological expertise\n"
	.string "is outstanding, I tell you.$"

Johto_RuinsOfAlphResearchCenter_Text_83F76F4::
	.string "Oh, you don’t have this fossil.\n"
	.string "Come back if you manage to find it.$"

Johto_RuinsOfAlphResearchCenter_Text_83F77E2::
	.string "Excellent!\n"
	.string "Let’s do this right away.\p"
	.string "{PLAYER} handed the {STR_VAR_1} to\n"
	.string "the RUINS RESEARCHER.$"

Johto_RuinsOfAlphResearchCenter_Text_83F78E0::
	.string "Thanks for waiting!\p"
	.string "Your fossilized POKéMON has been\n"
	.string "brought back to life!\p"
	.string "The fossil was an ancient POKéMON.\n"
	.string "{STR_VAR_2}, it was!$"

Johto_RuinsOfAlphResearchCenter_Text_83F795A::
	.string "{PLAYER} received {STR_VAR_2} from\n"
	.string "the RESEARCHER.$"
