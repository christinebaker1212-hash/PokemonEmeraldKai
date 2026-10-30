Johto_TrainerHillOutside_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_TrainerHillOutside_EventScript_83FC17D
	.byte 0

Johto_TrainerHillOutside_EventScript_83FC17D::
	setvar VAR_QZ_0x40D6, 0
	end

@ Johto_TrainerHillOutside sign / bg event at 15,29
Johto_TrainerHillOutside_EventScript_83FC183::
	lock
	msgbox Johto_TrainerHillOutside_Text_83FC306, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_TrainerHillOutside sign / bg event at 32,29
Johto_TrainerHillOutside_EventScript_83FC18F::
	lock
	msgbox Johto_TrainerHillOutside_Text_83FC35D, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_TrainerHillOutside sign / bg event at 22,29
Johto_TrainerHillOutside_EventScript_83FC19B::
	lock
	msgbox Johto_TrainerHillOutside_Text_83FC393, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_TrainerHillOutside sign / bg event at 25,20
Johto_TrainerHillOutside_EventScript_83FC1A7::
	lock
	msgbox Johto_TrainerHillOutside_Text_83FC3C5, MSGBOX_SIGN
	closemessage
	release
	end

Johto_TrainerHillOutside_Text_83FC306::
	.string "ル TRAINER HILL ENTRANCE\p"
	.string "’Scale the heights, you hot-blooded\n"
	.string "TRAINERS!’\p"
	.string "ワ BATTLE TENTS.$"

Johto_TrainerHillOutside_Text_83FC35D::
	.string "BATTLE TENT: FACTORY\n"
	.string "’Find it! The ultimate POKéMON!’$"

Johto_TrainerHillOutside_Text_83FC393::
	.string "BATTLE TENT: PALACE\n"
	.string "’Feast Your Eyes on Battles!’$"

Johto_TrainerHillOutside_Text_83FC3C5::
	.string "BATTLE TENT: ARENA\n"
	.string "’May the Greatest Teams Gather!’$"
