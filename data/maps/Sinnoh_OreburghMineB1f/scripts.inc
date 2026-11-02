Sinnoh_OreburghMineB1f_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_OreburghMineB1f_EventScript_Person1::
	goto SinnohP_oreburgh_mine_b1f_2
	end

Sinnoh_OreburghMineB1f_EventScript_Person3::
	goto SinnohP_oreburgh_mine_b1f_3
	end

Sinnoh_OreburghMineB1f_EventScript_Person4::
	lock
	faceplayer
	goto_if_set 0x3DC1, Sinnoh_OreburghMineB1f_EventScript_Person4_Given
	msgbox Sinnoh_OreburghMineB1f_Text_Person4, MSGBOX_DEFAULT
	msgbox Sinnoh_OreburghMineB1f_Text_Person4Gift, MSGBOX_DEFAULT
	giveitem ITEM_FLAME_PLATE, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3DC1
	release
	end

Sinnoh_OreburghMineB1f_EventScript_Person4_Given::
	msgbox Sinnoh_OreburghMineB1f_Text_Person4, MSGBOX_DEFAULT
	release
	end

Sinnoh_OreburghMineB1f_Text_Person4:
	.string "The mine's tunnels go deeper every\n"
	.string "year. ROARK says the fossils down\l"
	.string "here are older than the hills.$"

Sinnoh_OreburghMineB1f_Text_Person4Gift:
	.string "I dug up this warm plate. It's yours if\n"
	.string "you want it!$"

@ --- end of SINNOH words ---
