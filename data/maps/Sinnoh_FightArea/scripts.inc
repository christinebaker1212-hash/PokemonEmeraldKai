Sinnoh_FightArea_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_FightArea_OnTransition
	.byte 0

Sinnoh_FightArea_OnTransition:
	special Sinnoh_LegendsRefresh
	end

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
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
	msgbox Sinnoh_FightArea_Text_Person7, MSGBOX_NPC
	end

Sinnoh_FightArea_EventScript_Person14::
	msgbox Sinnoh_FightArea_Text_Person14, MSGBOX_NPC
	end

Sinnoh_FightArea_EventScript_Person15::
	msgbox Sinnoh_FightArea_Text_Person15, MSGBOX_NPC
	end

Sinnoh_FightArea_EventScript_Person16::
	msgbox Sinnoh_FightArea_Text_Person16, MSGBOX_NPC
	end

Sinnoh_FightArea_EventScript_Person17::
	msgbox Sinnoh_FightArea_Text_Person17, MSGBOX_NPC
	end

Sinnoh_FightArea_EventScript_Person18::
	msgbox Sinnoh_FightArea_Text_Person18, MSGBOX_NPC
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
	msgbox Sinnoh_FightArea_Text_Person20, MSGBOX_NPC
	end

Sinnoh_FightArea_EventScript_Sign1::
	msgbox Sinnoh_FightArea_Text_Sign1, MSGBOX_SIGN
	end

Sinnoh_FightArea_EventScript_Sign2::
	msgbox Sinnoh_FightArea_Text_Sign2, MSGBOX_SIGN
	end

Sinnoh_FightArea_EventScript_Sign3::
	msgbox Sinnoh_FightArea_Text_Sign3, MSGBOX_SIGN
	end

Sinnoh_FightArea_Text_Person6Ask0:
	.string "Shall I take you to SNOWPOINT CITY?$"

Sinnoh_FightArea_Text_Person6Bye:
	.string "Come back any time. The sea's always\n"
	.string "here.$"

Sinnoh_FightArea_Text_Person6Off:
	.string "All aboard! Hold on tight!$"

Sinnoh_FightArea_Text_Person7:
	.string "The fish out here are as tough as the\n"
	.string "TRAINERS. I've lost three rods today!$"

Sinnoh_FightArea_Text_Person14:
	.string "Everyone here came to battle. Even\n"
	.string "the MOéMON look like they're warming\l"
	.string "up.$"

Sinnoh_FightArea_Text_Person15:
	.string "The BATTLE FRONTIER is where the real\n"
	.string "challenge is. Are you ready?$"

Sinnoh_FightArea_Text_Person16:
	.string "I've been training in the BATTLE ZONE\n"
	.string "for a year. I'm almost ready for the\l"
	.string "FRONTIER.$"

Sinnoh_FightArea_Text_Person17:
	.string "My big brother is going to win\n"
	.string "everything at the FRONTIER!\l"
	.string "...Someday!$"

Sinnoh_FightArea_Text_Person18:
	.string "Hah! Nobody comes here to rest. Even\n"
	.string "the cafe has a battle menu!$"

Sinnoh_FightArea_Text_Person19Ask0:
	.string "Shall I take you to SNOWPOINT CITY?$"

Sinnoh_FightArea_Text_Person19Bye:
	.string "Come back any time. The sea's always\n"
	.string "here.$"

Sinnoh_FightArea_Text_Person19Off:
	.string "All aboard! Hold on tight!$"

Sinnoh_FightArea_Text_Person20:
	.string "Feel that? The air out here crackles\n"
	.string "with fighting spirit!$"

Sinnoh_FightArea_Text_Sign1:
	.string "FIGHT AREA\p"
	.string "The gateway to the BATTLE ZONE.$"

Sinnoh_FightArea_Text_Sign2:
	.string "ROUTE 225\p"
	.string "To the SURVIVAL AREA$"

Sinnoh_FightArea_Text_Sign3:
	.string "BATTLE FRONTIER\p"
	.string "For the strongest TRAINERS.$"

@ --- end of SINNOH words ---
