Sinnoh_FightArea_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_FightArea_OnTransition
	.byte 0

Sinnoh_FightArea_OnTransition:
	special Sinnoh_LegendsRefresh
	end

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_FightArea_EventScript_Person4::
	goto SinnohP_common_28
	end

Sinnoh_FightArea_EventScript_Person5::
	goto SinnohP_common_28
	end

Sinnoh_FightArea_EventScript_Person6::
	lock
	faceplayer
	msgbox Sinnoh_FightArea_Text_Person6Ask0, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, Sinnoh_FightArea_EventScript_Person6_Sail0
	msgbox Sinnoh_FightArea_Text_Person6Bye, MSGBOX_DEFAULT
	release
	end

Sinnoh_FightArea_EventScript_Person6_Sail0::
	msgbox Sinnoh_FightArea_Text_Person6Off, MSGBOX_DEFAULT
	closemessage
	warp MAP_SINNOH_SNOWPOINT_CITY, 21, 37
	waitstate
	releaseall
	end

Sinnoh_FightArea_EventScript_Person7::
	goto SinnohP_fight_area_8
	end

Sinnoh_FightArea_EventScript_Person9::
	goto SinnohP_fight_area_25
	end

Sinnoh_FightArea_EventScript_Person14::
	goto SinnohP_fight_area_4
	end

Sinnoh_FightArea_EventScript_Person15::
	goto SinnohP_fight_area_2
	end

Sinnoh_FightArea_EventScript_Person16::
	goto SinnohP_fight_area_3
	end

Sinnoh_FightArea_EventScript_Person17::
	goto SinnohP_fight_area_5
	end

Sinnoh_FightArea_EventScript_Person18::
	goto SinnohP_fight_area_6
	end

Sinnoh_FightArea_EventScript_Person19::
	lock
	faceplayer
	msgbox Sinnoh_FightArea_Text_Person19Ask0, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, Sinnoh_FightArea_EventScript_Person19_Sail0
	msgbox Sinnoh_FightArea_Text_Person19Bye, MSGBOX_DEFAULT
	release
	end

Sinnoh_FightArea_EventScript_Person19_Sail0::
	msgbox Sinnoh_FightArea_Text_Person19Off, MSGBOX_DEFAULT
	closemessage
	warp MAP_SINNOH_SNOWPOINT_CITY, 21, 37
	waitstate
	releaseall
	end

Sinnoh_FightArea_EventScript_Person20::
	goto SinnohP_fight_area_13
	end

Sinnoh_FightArea_EventScript_Person23::
	goto SinnohP_fight_area_16
	end

Sinnoh_FightArea_EventScript_Person24::
	goto SinnohP_fight_area_17
	end

Sinnoh_FightArea_EventScript_Person25::
	goto SinnohP_fight_area_19
	end

Sinnoh_FightArea_EventScript_Person26::
	goto SinnohP_fight_area_18
	end

Sinnoh_FightArea_EventScript_Person27::
	msgbox Sinnoh_FightArea_Text_Person27, MSGBOX_NPC
	end

Sinnoh_FightArea_EventScript_Person28::
	goto SinnohP_fight_area_22
	end

Sinnoh_FightArea_EventScript_Person29::
	goto SinnohP_fight_area_21
	end

Sinnoh_FightArea_EventScript_Person30::
	goto SinnohP_fight_area_20
	end

Sinnoh_FightArea_EventScript_Person31::
	goto SinnohP_fight_area_23
	end

Sinnoh_FightArea_EventScript_Person32::
	goto SinnohP_fight_area_24
	end

Sinnoh_FightArea_EventScript_Sign1::
	goto SinnohP_fight_area_9
	end

Sinnoh_FightArea_EventScript_Sign2::
	goto SinnohP_fight_area_11
	end

Sinnoh_FightArea_EventScript_Sign3::
	goto SinnohP_fight_area_10
	end

Sinnoh_FightArea_Text_Person6Ask0:
	.string "Shall I take you to SNOWPOINT CITY?$"

Sinnoh_FightArea_Text_Person6Bye:
	.string "Come back any time. The sea's always\n"
	.string "here.$"

Sinnoh_FightArea_Text_Person6Off:
	.string "All aboard! Hold on tight!$"

Sinnoh_FightArea_Text_Person19Ask0:
	.string "Shall I take you to SNOWPOINT CITY?$"

Sinnoh_FightArea_Text_Person19Bye:
	.string "Come back any time. The sea's always\n"
	.string "here.$"

Sinnoh_FightArea_Text_Person19Off:
	.string "All aboard! Hold on tight!$"

Sinnoh_FightArea_Text_Person27:
	.string "PALMER: I'm PALMER, the TOWER TYCOON!\n"
	.string "My son BARRY never stops talking\l"
	.string "about you.\p"
	.string "Come challenge the BATTLE TOWER. I'll\n"
	.string "be waiting at the top.$"

@ --- end of SINNOH words ---
