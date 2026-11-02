Sinnoh_Route201_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_Route201_OnTransition
	.byte 0

Sinnoh_Route201_OnTransition:
	special Sinnoh_LegendsRefresh
	end

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_Route201_EventScript_Person1::
	goto SinnohP_route_201_9
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
	goto SinnohP_route_201_11
	end

Sinnoh_Route201_EventScript_Person12::
	goto SinnohP_route_201_12
	end

Sinnoh_Route201_EventScript_Person13::
	goto Sinnoh_EventScript_Briefcase
	end

Sinnoh_Route201_EventScript_Sign4::
	goto SinnohP_route_201_6
	end

Sinnoh_Route201_EventScript_Sign5::
	goto SinnohP_route_201_7
	end

Sinnoh_Route201_EventScript_Sign11::
	goto SinnohP_route_201_8
	end

Sinnoh_Route201_Text_Person2:
	.string "I'm handing out samples from the POKé\n"
	.string "MART in SANDGEM! It's just east of\l"
	.string "here.$"

Sinnoh_Route201_Text_Person2Gift:
	.string "Here, have a free POTION!$"

@ --- end of SINNOH words ---
