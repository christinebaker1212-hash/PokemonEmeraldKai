Sinnoh_Route201_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_Route201_OnTransition
	.byte 0

Sinnoh_Route201_OnTransition:
	special Sinnoh_LegendsRefresh
	end

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_Route201_EventScript_Person1::
	msgbox Sinnoh_Route201_Text_Person1, MSGBOX_NPC
	end

Sinnoh_Route201_EventScript_Person2::
	lock
	faceplayer
	goto_if_set 0x3DC6, Sinnoh_Route201_EventScript_Person2_Given
	msgbox Sinnoh_Route201_Text_Person2, MSGBOX_DEFAULT
	msgbox Sinnoh_Route201_Text_Person2Gift, MSGBOX_DEFAULT
	giveitem ITEM_POTION, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3DC6
	release
	end

Sinnoh_Route201_EventScript_Person2_Given::
	msgbox Sinnoh_Route201_Text_Person2, MSGBOX_DEFAULT
	release
	end

Sinnoh_Route201_EventScript_Person10::
	msgbox Sinnoh_Route201_Text_Person10, MSGBOX_NPC
	end

Sinnoh_Route201_EventScript_Person12::
	msgbox Sinnoh_Route201_Text_Person12, MSGBOX_NPC
	end

Sinnoh_Route201_EventScript_Person13::
	goto Sinnoh_EventScript_Briefcase
	end

Sinnoh_Route201_EventScript_Sign4::
	msgbox Sinnoh_Route201_Text_Sign4, MSGBOX_SIGN
	end

Sinnoh_Route201_EventScript_Sign5::
	msgbox Sinnoh_Route201_Text_Sign5, MSGBOX_SIGN
	end

Sinnoh_Route201_EventScript_Sign11::
	msgbox Sinnoh_Route201_Text_Sign11, MSGBOX_SIGN
	end

Sinnoh_Route201_Text_Person1:
	.string "Wild MOéMON hide in the tall grass.\n"
	.string "Stay on the path if your team is tired.\p"
	.string "Or don't! Running into them is half the\n"
	.string "fun.$"

Sinnoh_Route201_Text_Person2:
	.string "I'm handing out samples from the POKé\n"
	.string "MART in SANDGEM! It's just east of\l"
	.string "here.$"

Sinnoh_Route201_Text_Person2Gift:
	.string "Here, have a free POTION!$"

Sinnoh_Route201_Text_Person10:
	.string "I'm practicing my throws. POKé BALLS\n"
	.string "fly better when your MOéMON has worn\l"
	.string "the wild one down a little first.$"

Sinnoh_Route201_Text_Person12:
	.string "STARLY travel in flocks around here.\p"
	.string "When one gets tired, the others slow\n"
	.string "down and wait. Isn't that sweet?$"

Sinnoh_Route201_Text_Sign4:
	.string "ROUTE 201\p"
	.string "West: TWINLEAF TOWN$"

Sinnoh_Route201_Text_Sign5:
	.string "ROUTE 201\p"
	.string "East: SANDGEM TOWN$"

Sinnoh_Route201_Text_Sign11:
	.string "TRAINER TIPS\p"
	.string "Talk to people everywhere you go.\n"
	.string "Most of them have something to say,\l"
	.string "and some of them have something to\l"
	.string "give.$"

@ --- end of SINNOH words ---
