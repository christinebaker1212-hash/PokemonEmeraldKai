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
	goto SinnohP_trainers_school_2
	end

Sinnoh_TrainersSchool_EventScript_Person2::
	msgbox Sinnoh_TrainersSchool_Text_Person2, MSGBOX_NPC
	end

Sinnoh_TrainersSchool_EventScript_Person3::
	goto SinnohP_trainers_school_4
	end

Sinnoh_TrainersSchool_EventScript_Person4::
	goto SinnohP_trainers_school_6
	end

Sinnoh_TrainersSchool_EventScript_Person5::
	goto SinnohP_trainers_school_5
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
	goto SinnohP_trainers_school_12
	end

Sinnoh_TrainersSchool_Text_Bg1:
	.string "Today's lesson: “A MOéMON's type tells\n"
	.string "you what it's strong and weak\l"
	.string "against.”$"

Sinnoh_TrainersSchool_Text_Bg2:
	.string "Today's lesson: “A MOéMON's type tells\n"
	.string "you what it's strong and weak\l"
	.string "against.”$"

Sinnoh_TrainersSchool_Text_Person2:
	.string "BARRY: Oh! It's you! I was teaching\n"
	.string "these kids how to battle.\p"
	.string "Well, they were teaching me, kind of.\n"
	.string "Anyway! I'm off to OREBURGH! Last one\l"
	.string "there pays a fine!$"

Sinnoh_TrainersSchool_Text_Person7:
	.string "My notes are full of type charts. I\n"
	.string "drew little MOéMON in the margins too.$"

Sinnoh_TrainersSchool_Text_Person7Gift:
	.string "Here, I have an extra POTION from\n"
	.string "class. Take it!$"

Sinnoh_TrainersSchool_Text_Person8:
	.string "I want to be a gym leader when I grow\n"
	.string "up! I just have to pick a type.$"

@ --- end of SINNOH words ---
