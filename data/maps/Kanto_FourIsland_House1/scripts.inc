Kanto_FourIsland_House1_MapScripts::
	.byte 0

@ Kanto_FourIsland_House1 object 2 (local id 2, gfx 0x3F04) at 9,3
Kanto_FourIsland_House1_EventScript_8421958::
	msgbox Kanto_FourIsland_House1_Text_8453D9A, MSGBOX_NPC
	end

@ Kanto_FourIsland_House1 object 1 (local id 1, gfx 0x3F04) at 8,3
Kanto_FourIsland_House1_EventScript_8471CD2::
	@ Moemon Kai: FireRed's BODY SLAM tutor (tools/quetzal_port/fix_move_tutors.py),
	@ now gives its TM (tools/quetzal_port/tutors_to_tms.py)
	lock
	faceplayer
	goto_if_set FLAG_KANTO_GOT_TM_BODY_SLAM, Kanto_FourIsland_House1_EventScript_8471CD2_Taught
	msgbox Kanto_FourIsland_House1_EventScript_8471CD2_Teach, MSGBOX_DEFAULT
	giveitem ITEM_TM103
	goto_if_eq VAR_RESULT, FALSE, Kanto_FourIsland_House1_EventScript_8471CD2_Declined
	setflag FLAG_KANTO_GOT_TM_BODY_SLAM
	goto Kanto_FourIsland_House1_EventScript_8471CD2_Taught
	end

Kanto_FourIsland_House1_EventScript_8471CD2_Declined::
	release
	end

Kanto_FourIsland_House1_EventScript_8471CD2_Taught:
	msgbox Kanto_FourIsland_House1_EventScript_8471CD2_TaughtText, MSGBOX_DEFAULT
	release
	end

Kanto_FourIsland_House1_EventScript_8471CD2_Teach:
	.string "Ready?\n"
	.string "Boing!\p"
	.string "We're having a BODY SLAM match\n"
	.string "to see who wimps out first.\p"
	.string "If you were to join us, you'd be\n"
	.string "squashed like a bug, though…\p"
	.string "So take this TM instead, and\n"
	.string "teach BODY SLAM to a MOéMON!$"

Kanto_FourIsland_House1_EventScript_8471CD2_TaughtText:
	.string "Boioing!$"

Kanto_FourIsland_House1_Text_8453D9A::
	.string "There! Boing!\n"
	.string "You’re awfully heavy!$"
