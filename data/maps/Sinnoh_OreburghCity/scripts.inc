Sinnoh_OreburghCity_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_OreburghCity_EventScript_Bg1::
	msgbox Sinnoh_OreburghCity_Text_Bg1, MSGBOX_SIGN
	end

Sinnoh_OreburghCity_EventScript_Person1::
	goto SinnohP_oreburgh_city_5
	end

Sinnoh_OreburghCity_EventScript_Person2::
	goto SinnohP_oreburgh_city_11
	end

Sinnoh_OreburghCity_EventScript_Person3::
	goto SinnohP_oreburgh_city_13
	end

Sinnoh_OreburghCity_EventScript_Person4::
	msgbox Sinnoh_OreburghCity_Text_Person4, MSGBOX_NPC
	end

Sinnoh_OreburghCity_EventScript_Person5::
	msgbox Sinnoh_OreburghCity_Text_Person5, MSGBOX_NPC
	end

Sinnoh_OreburghCity_EventScript_Person16::
	goto SinnohP_oreburgh_city_6
	end

Sinnoh_OreburghCity_EventScript_Person17::
	lock
	faceplayer
	goto_if_set 0x3DBC, Sinnoh_OreburghCity_EventScript_Person17_Given
	msgbox Sinnoh_OreburghCity_Text_Person17, MSGBOX_DEFAULT
	msgbox Sinnoh_OreburghCity_Text_Person17Gift, MSGBOX_DEFAULT
	giveitem ITEM_SUPER_POTION, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3DBC
	release
	end

Sinnoh_OreburghCity_EventScript_Person17_Given::
	msgbox Sinnoh_OreburghCity_Text_Person17, MSGBOX_DEFAULT
	release
	end

Sinnoh_OreburghCity_EventScript_Person18::
	goto SinnohP_oreburgh_city_14
	end

Sinnoh_OreburghCity_EventScript_Person19::
	goto SinnohP_oreburgh_city_8
	end

Sinnoh_OreburghCity_EventScript_Person20::
	goto SinnohP_oreburgh_city_9
	end

Sinnoh_OreburghCity_EventScript_Person21::
	goto SinnohP_oreburgh_city_12
	end

Sinnoh_OreburghCity_EventScript_Person22::
	goto SinnohP_oreburgh_city_10
	end

Sinnoh_OreburghCity_EventScript_Person23::
	msgbox Sinnoh_OreburghCity_Text_Person23, MSGBOX_NPC
	end

Sinnoh_OreburghCity_EventScript_Person26::
	msgbox Sinnoh_OreburghCity_Text_Person26, MSGBOX_NPC
	end

Sinnoh_OreburghCity_EventScript_Person27::
	msgbox Sinnoh_OreburghCity_Text_Person27, MSGBOX_NPC
	end

Sinnoh_OreburghCity_EventScript_Person28::
	goto SinnohP_oreburgh_city_20
	end

Sinnoh_OreburghCity_EventScript_Sign6::
	goto SinnohP_oreburgh_city_15
	end

Sinnoh_OreburghCity_EventScript_Sign7::
	goto SinnohP_oreburgh_city_16
	end

Sinnoh_OreburghCity_EventScript_Sign8::
	goto SinnohP_oreburgh_city_17
	end

Sinnoh_OreburghCity_EventScript_Sign9::
	goto SinnohP_oreburgh_city_18
	end

Sinnoh_OreburghCity_EventScript_Sign10::
	goto SinnohP_oreburgh_city_15
	end

Sinnoh_OreburghCity_Text_Bg1:
	.string "The road to the mine. The ground\n"
	.string "rumbles faintly under your feet.$"

Sinnoh_OreburghCity_Text_Person4:
	.string "BARRY: The GYM LEADER's down in the\n"
	.string "mine. I'm going to beat him the second\l"
	.string "he's back!\p"
	.string "...After I stock up on POTIONS. And\n"
	.string "maybe a snack.$"

Sinnoh_OreburghCity_Text_Person5:
	.string "Have you been to the GYM yet? ROARK\n"
	.string "uses ROCK MOéMON. Bring something\l"
	.string "with water!$"

Sinnoh_OreburghCity_Text_Person17:
	.string "Break time! Ten minutes, then back to\n"
	.string "the mine.$"

Sinnoh_OreburghCity_Text_Person17Gift:
	.string "Here, a little something from the\n"
	.string "first-aid box. Miners share!$"

Sinnoh_OreburghCity_Text_Person23:
	.string "MACHOP: Machop! Ma-chop!\p"
	.string "It's flexing proudly.$"

Sinnoh_OreburghCity_Text_Person26:
	.string "MACHOP: Ma! Machop!\p"
	.string "It's carrying a lump of coal bigger\n"
	.string "than its head.$"

Sinnoh_OreburghCity_Text_Person27:
	.string "MACHOP: Chop chop!\p"
	.string "It's taking a short break.$"

@ --- end of SINNOH words ---
