Sinnoh_TrainersSchool_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_TrainersSchool_EventScript_Bg1::
	msgbox Sinnoh_TrainersSchool_Text_Bg1, MSGBOX_SIGN
	end

Sinnoh_TrainersSchool_EventScript_Bg2::
	msgbox Sinnoh_TrainersSchool_Text_Bg2, MSGBOX_SIGN
	end

Sinnoh_TrainersSchool_EventScript_Person1::
	msgbox Sinnoh_TrainersSchool_Text_Person1, MSGBOX_NPC
	end

Sinnoh_TrainersSchool_EventScript_Person3::
	msgbox Sinnoh_TrainersSchool_Text_Person3, MSGBOX_NPC
	end

Sinnoh_TrainersSchool_EventScript_Person4::
	msgbox Sinnoh_TrainersSchool_Text_Person4, MSGBOX_NPC
	end

Sinnoh_TrainersSchool_EventScript_Person5::
	msgbox Sinnoh_TrainersSchool_Text_Person5, MSGBOX_NPC
	end

Sinnoh_TrainersSchool_EventScript_Person7::
	lock
	faceplayer
	goto_if_set 0x3DDE, Sinnoh_TrainersSchool_EventScript_Person7_Given
	msgbox Sinnoh_TrainersSchool_Text_Person7, MSGBOX_DEFAULT
	msgbox Sinnoh_TrainersSchool_Text_Person7Gift, MSGBOX_DEFAULT
	giveitem ITEM_POTION, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3DDE
	release
	end

Sinnoh_TrainersSchool_EventScript_Person7_Given::
	msgbox Sinnoh_TrainersSchool_Text_Person7, MSGBOX_DEFAULT
	release
	end

Sinnoh_TrainersSchool_EventScript_Person8::
	msgbox Sinnoh_TrainersSchool_Text_Person8, MSGBOX_NPC
	end

Sinnoh_TrainersSchool_EventScript_Sign6::
	msgbox Sinnoh_TrainersSchool_Text_Sign6, MSGBOX_SIGN
	end

Sinnoh_TrainersSchool_Text_Bg1:
	.string "Today's lesson: “A MOéMON's type tells\n"
	.string "you what it's strong and weak\l"
	.string "against.”$"

Sinnoh_TrainersSchool_Text_Bg2:
	.string "Today's lesson: “A MOéMON's type tells\n"
	.string "you what it's strong and weak\l"
	.string "against.”$"

Sinnoh_TrainersSchool_Text_Person1:
	.string "Welcome to the TRAINERS' SCHOOL! Feel\n"
	.string "free to look around and read the\l"
	.string "boards.$"

Sinnoh_TrainersSchool_Text_Person3:
	.string "We learned about status today. A\n"
	.string "sleeping MOéMON can't move! Unless it\l"
	.string "snores, I think.$"

Sinnoh_TrainersSchool_Text_Person4:
	.string "The teacher says a good TRAINER\n"
	.string "watches the battle, not just their\l"
	.string "own MOéMON.$"

Sinnoh_TrainersSchool_Text_Person5:
	.string "I'm the best in my class at throwing\n"
	.string "POKé BALLS. Well, the best at hitting\l"
	.string "the wall.$"

Sinnoh_TrainersSchool_Text_Person7:
	.string "My notes are full of type charts. I\n"
	.string "drew little MOéMON in the margins too.$"

Sinnoh_TrainersSchool_Text_Person7Gift:
	.string "Here, I have an extra POTION from\n"
	.string "class. Take it!$"

Sinnoh_TrainersSchool_Text_Person8:
	.string "I want to be a gym leader when I grow\n"
	.string "up! I just have to pick a type.$"

Sinnoh_TrainersSchool_Text_Sign6:
	.string "It's full of handwriting. It doesn't\n"
	.string "seem polite to read it.$"

@ --- end of SINNOH words ---
