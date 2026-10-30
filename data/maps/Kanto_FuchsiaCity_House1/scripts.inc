Kanto_FuchsiaCity_House1_MapScripts::
	.byte 0

@ Kanto_FuchsiaCity_House1 object 2 (local id 2, gfx 0x3F58) at 1,2
Kanto_FuchsiaCity_House1_EventScript_841D86B::
	msgbox Kanto_FuchsiaCity_House1_Text_8448B36, MSGBOX_NPC
	end

@ Kanto_FuchsiaCity_House1 object 1 (local id 1, gfx 0x3F2C) at 7,4
Kanto_FuchsiaCity_House1_EventScript_841D874::
	lock
	faceplayer
	setvar VAR_0x8004, 0xD
	setvar VAR_0x8005, 2
	special QzQuestLogNop
	msgbox Kanto_FuchsiaCity_House1_Text_8448B9A, MSGBOX_DEFAULT
	release
	end

@ Kanto_FuchsiaCity_House1 object 3 (local id 3, gfx 0x3F17) at 2,5
Kanto_FuchsiaCity_House1_EventScript_841D88D::
	msgbox Kanto_FuchsiaCity_House1_Text_8448BF6, MSGBOX_NPC
	end

Kanto_FuchsiaCity_House1_Text_8448B36::
	.string "The SAFARI ZONE’s WARDEN is old, \n"
	.string "but he’s still very much active.\p"
	.string "All his teeth are false, though.$"

Kanto_FuchsiaCity_House1_Text_8448B9A::
	.string "Hmm?\n"
	.string "You’ve met BILL?\p"
	.string "He’s my grandson!\p"
	.string "He always liked collecting things,\n"
	.string "even as a child!$"

Kanto_FuchsiaCity_House1_Text_8448BF6::
	.string "BILL files his own POKéMON data on\n"
	.string "his PC.\p"
	.string "Did he show you?$"
