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
	lock
	faceplayer
	goto_if_set 0x3DAA, Sinnoh_FightArea_EventScript_Person7_Given
	checkitem ITEM_SUPER_ROD
	goto_if_eq VAR_RESULT, TRUE, Sinnoh_FightArea_EventScript_Person7_Given
	msgbox Sinnoh_FightArea_Text_Person7, MSGBOX_DEFAULT
	msgbox Sinnoh_FightArea_Text_Person7Gift, MSGBOX_DEFAULT
	giveitem ITEM_SUPER_ROD, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3DAA
	release
	end

Sinnoh_FightArea_EventScript_Person7_Given::
	msgbox Sinnoh_FightArea_Text_Person7, MSGBOX_DEFAULT
	release
	end

Sinnoh_FightArea_EventScript_Person9::
	msgbox Sinnoh_FightArea_Text_Person9, MSGBOX_NPC
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

Sinnoh_FightArea_EventScript_Person25::
	msgbox Sinnoh_FightArea_Text_Person25, MSGBOX_NPC
	end

Sinnoh_FightArea_EventScript_Person26::
	msgbox Sinnoh_FightArea_Text_Person26, MSGBOX_NPC
	end

Sinnoh_FightArea_EventScript_Person27::
	msgbox Sinnoh_FightArea_Text_Person27, MSGBOX_NPC
	end

Sinnoh_FightArea_EventScript_Person28::
	msgbox Sinnoh_FightArea_Text_Person28, MSGBOX_NPC
	end

Sinnoh_FightArea_EventScript_Person29::
	msgbox Sinnoh_FightArea_Text_Person29, MSGBOX_NPC
	end

Sinnoh_FightArea_EventScript_Person30::
	msgbox Sinnoh_FightArea_Text_Person30, MSGBOX_NPC
	end

Sinnoh_FightArea_EventScript_Person31::
	msgbox Sinnoh_FightArea_Text_Person31, MSGBOX_NPC
	end

Sinnoh_FightArea_EventScript_Person32::
	msgbox Sinnoh_FightArea_Text_Person32, MSGBOX_NPC
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

Sinnoh_FightArea_Text_Person7Gift:
	.string "Here, take my spare rod. A good one!\n"
	.string "You'll hook the big ones with it.$"

Sinnoh_FightArea_Text_Person9:
	.string "BUCK: Yo! I'm BUCK. I love tough\n"
	.string "battles more than anything.\p"
	.string "STARK MOUNTAIN's where I'm headed\n"
	.string "next. Something's waking up inside it.$"

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

Sinnoh_FightArea_Text_Person25:
	.string "VOLKNER: I came to see if the BATTLE\n"
	.string "ZONE's TRAINERS are as strong as they\l"
	.string "say. So far, so shocking.$"

Sinnoh_FightArea_Text_Person26:
	.string "FLINT: The FRONTIER's got TRAINERS\n"
	.string "who burn hotter than any LEAGUE. I\l"
	.string "love it here!$"

Sinnoh_FightArea_Text_Person27:
	.string "PALMER: I'm PALMER, the TOWER TYCOON!\n"
	.string "My son BARRY never stops talking\l"
	.string "about you.\p"
	.string "Come challenge the BATTLE TOWER. I'll\n"
	.string "be waiting at the top.$"

Sinnoh_FightArea_Text_Person28:
	.string "That battle was incredible! I can't\n"
	.string "stop cheering!$"

Sinnoh_FightArea_Text_Person29:
	.string "I'm studying every move these\n"
	.string "TRAINERS make. My notebook's on fire.$"

Sinnoh_FightArea_Text_Person30:
	.string "Watch closely. The best TRAINERS read\n"
	.string "the battle like a book.$"

Sinnoh_FightArea_Text_Person31:
	.string "I came all the way here just to watch.\n"
	.string "Best decision ever!$"

Sinnoh_FightArea_Text_Person32:
	.string "I sailed in from SNOWPOINT. My boat's\n"
	.string "still frozen, but my heart's warm!$"

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
