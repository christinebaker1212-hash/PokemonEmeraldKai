Sinnoh_Route210South_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_Route210South_EventScript_Person22::
	lock
	faceplayer
	goto_if_set 0x3DCD, Sinnoh_Route210South_EventScript_Person22_Given
	msgbox Sinnoh_Route210South_Text_Person22, MSGBOX_DEFAULT
	msgbox Sinnoh_Route210South_Text_Person22Gift, MSGBOX_DEFAULT
	giveitem ITEM_TM51, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3DCD
	release
	end

Sinnoh_Route210South_EventScript_Person22_Given::
	msgbox Sinnoh_Route210South_Text_Person22, MSGBOX_DEFAULT
	release
	end

Sinnoh_Route210South_EventScript_Person31::
	msgbox Sinnoh_Route210South_Text_Person31, MSGBOX_NPC
	end

Sinnoh_Route210South_EventScript_Sign11::
	goto SinnohP_route_210_south_5
	end

Sinnoh_Route210South_EventScript_Sign14::
	goto SinnohP_route_210_south_4
	end

Sinnoh_Route210South_Text_Person22:
	.string "The fog up north gets so thick you\n"
	.string "can't see your own feet. Watch your\l"
	.string "step!$"

Sinnoh_Route210South_Text_Person22Gift:
	.string "If you get lost in the fog, this TM will\n"
	.string "help you fly out of it!$"

Sinnoh_Route210South_Text_Person31:
	.string "I only battle in daylight. At night I\n"
	.string "just run and think about soup.$"

@ --- end of SINNOH words ---
