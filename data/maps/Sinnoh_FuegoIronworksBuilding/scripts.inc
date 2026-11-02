Sinnoh_FuegoIronworksBuilding_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_FuegoIronworksBuilding_EventScript_Person5::
	lock
	faceplayer
	goto_if_set 0x3DB1, Sinnoh_FuegoIronworksBuilding_EventScript_Person5_Given
	msgbox Sinnoh_FuegoIronworksBuilding_Text_Person5, MSGBOX_DEFAULT
	msgbox Sinnoh_FuegoIronworksBuilding_Text_Person5Gift, MSGBOX_DEFAULT
	giveitem ITEM_STAR_PIECE, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3DB1
	release
	end

Sinnoh_FuegoIronworksBuilding_EventScript_Person5_Given::
	msgbox Sinnoh_FuegoIronworksBuilding_Text_Person5, MSGBOX_DEFAULT
	release
	end

Sinnoh_FuegoIronworksBuilding_Text_Person5:
	.string "Welcome to FUEGO IRONWORKS! My\n"
	.string "MOéMON and I make iron here, the old\l"
	.string "way.$"

Sinnoh_FuegoIronworksBuilding_Text_Person5Gift:
	.string "Found this in the furnace ash. Pretty,\n"
	.string "isn't it? It's yours!$"

@ --- end of SINNOH words ---
