Sinnoh_IronIsland_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_IronIsland_EventScript_Person1::
	goto SinnohP_iron_island_2
	end

Sinnoh_IronIsland_EventScript_Person2::
	lock
	faceplayer
	goto_if_set 0x3DBB, Sinnoh_IronIsland_EventScript_Person2_Given
	msgbox Sinnoh_IronIsland_Text_Person2, MSGBOX_DEFAULT
	msgbox Sinnoh_IronIsland_Text_Person2Gift, MSGBOX_DEFAULT
	giveitem ITEM_HM04, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3DBB
	release
	end

Sinnoh_IronIsland_EventScript_Person2_Given::
	msgbox Sinnoh_IronIsland_Text_Person2, MSGBOX_DEFAULT
	release
	end

Sinnoh_IronIsland_Text_Person2:
	.string "RILEY: I'm RILEY. I train on IRON\n"
	.string "ISLAND with my LUCARIO.\p"
	.string "Something's not right in the mine\n"
	.string "today. Wild POKéMON are fleeing the\l"
	.string "deep tunnels.$"

Sinnoh_IronIsland_Text_Person2Gift:
	.string "Take this HM. STRENGTH will move the\n"
	.string "boulders down in the mine.$"

@ --- end of SINNOH words ---
