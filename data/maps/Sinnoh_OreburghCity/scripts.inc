Sinnoh_OreburghCity_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_OreburghCity_EventScript_Bg1::
	msgbox Sinnoh_OreburghCity_Text_Bg1, MSGBOX_SIGN
	end

Sinnoh_OreburghCity_EventScript_Person1::
	msgbox Sinnoh_OreburghCity_Text_Person1, MSGBOX_NPC
	end

Sinnoh_OreburghCity_EventScript_Person2::
	msgbox Sinnoh_OreburghCity_Text_Person2, MSGBOX_NPC
	end

Sinnoh_OreburghCity_EventScript_Person3::
	msgbox Sinnoh_OreburghCity_Text_Person3, MSGBOX_NPC
	end

Sinnoh_OreburghCity_EventScript_Person4::
	msgbox Sinnoh_OreburghCity_Text_Person4, MSGBOX_NPC
	end

Sinnoh_OreburghCity_EventScript_Person5::
	msgbox Sinnoh_OreburghCity_Text_Person5, MSGBOX_NPC
	end

Sinnoh_OreburghCity_EventScript_Person16::
	msgbox Sinnoh_OreburghCity_Text_Person16, MSGBOX_NPC
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
	msgbox Sinnoh_OreburghCity_Text_Person18, MSGBOX_NPC
	end

Sinnoh_OreburghCity_EventScript_Person19::
	msgbox Sinnoh_OreburghCity_Text_Person19, MSGBOX_NPC
	end

Sinnoh_OreburghCity_EventScript_Person20::
	msgbox Sinnoh_OreburghCity_Text_Person20, MSGBOX_NPC
	end

Sinnoh_OreburghCity_EventScript_Person21::
	msgbox Sinnoh_OreburghCity_Text_Person21, MSGBOX_NPC
	end

Sinnoh_OreburghCity_EventScript_Person22::
	msgbox Sinnoh_OreburghCity_Text_Person22, MSGBOX_NPC
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
	msgbox Sinnoh_OreburghCity_Text_Person28, MSGBOX_NPC
	end

Sinnoh_OreburghCity_EventScript_Sign6::
	msgbox Sinnoh_OreburghCity_Text_Sign6, MSGBOX_SIGN
	end

Sinnoh_OreburghCity_EventScript_Sign7::
	msgbox Sinnoh_OreburghCity_Text_Sign7, MSGBOX_SIGN
	end

Sinnoh_OreburghCity_EventScript_Sign8::
	msgbox Sinnoh_OreburghCity_Text_Sign8, MSGBOX_SIGN
	end

Sinnoh_OreburghCity_EventScript_Sign9::
	msgbox Sinnoh_OreburghCity_Text_Sign9, MSGBOX_SIGN
	end

Sinnoh_OreburghCity_EventScript_Sign10::
	msgbox Sinnoh_OreburghCity_Text_Sign10, MSGBOX_SIGN
	end

Sinnoh_OreburghCity_Text_Bg1:
	.string "The road to the mine. The ground\n"
	.string "rumbles faintly under your feet.$"

Sinnoh_OreburghCity_Text_Person1:
	.string "OREBURGH runs on coal. The mine's been\n"
	.string "here longer than the city has!$"

Sinnoh_OreburghCity_Text_Person2:
	.string "My dad works in the mine. He comes\n"
	.string "home covered in soot, but he's always\l"
	.string "smiling.$"

Sinnoh_OreburghCity_Text_Person3:
	.string "The MINING MUSEUM tells the whole\n"
	.string "story of OREBURGH. Even the fossils\l"
	.string "have a corner.$"

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

Sinnoh_OreburghCity_Text_Person16:
	.string "Heave-ho! This coal won't move itself.\n"
	.string "Well, my MACHOP helps.$"

Sinnoh_OreburghCity_Text_Person17:
	.string "Break time! Ten minutes, then back to\n"
	.string "the mine.$"

Sinnoh_OreburghCity_Text_Person17Gift:
	.string "Here, a little something from the\n"
	.string "first-aid box. Miners share!$"

Sinnoh_OreburghCity_Text_Person18:
	.string "I came here to camp, but there's coal\n"
	.string "dust on everything! Even my\l"
	.string "marshmallows.$"

Sinnoh_OreburghCity_Text_Person19:
	.string "The coal from this mine heats half the\n"
	.string "homes in SINNOH. Pretty cool, huh?\l"
	.string "Well, warm.$"

Sinnoh_OreburghCity_Text_Person20:
	.string "Mind the carts! They roll by faster\n"
	.string "than you'd think.$"

Sinnoh_OreburghCity_Text_Person21:
	.string "I wrote a song about coal. It's got a\n"
	.string "lot of rumbling in it.$"

Sinnoh_OreburghCity_Text_Person22:
	.string "My MACHOP and I train by carrying\n"
	.string "coal. Two birds with one stone!$"

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

Sinnoh_OreburghCity_Text_Person28:
	.string "Hard work makes strong MOéMON. Look\n"
	.string "at those arms!$"

Sinnoh_OreburghCity_Text_Sign6:
	.string "OREBURGH CITY\p"
	.string "The energetic town that runs on coal.$"

Sinnoh_OreburghCity_Text_Sign7:
	.string "OREBURGH CITY MOéMON GYM\p"
	.string "LEADER: ROARK\p"
	.string "The rock-hard miner!$"

Sinnoh_OreburghCity_Text_Sign8:
	.string "OREBURGH MINING MUSEUM\p"
	.string "The history of OREBURGH's coal, and\n"
	.string "the fossils found with it.$"

Sinnoh_OreburghCity_Text_Sign9:
	.string "OREBURGH MINE\p"
	.string "Watch your step!$"

Sinnoh_OreburghCity_Text_Sign10:
	.string "OREBURGH CITY\p"
	.string "The energetic town that runs on coal.$"

@ --- end of SINNOH words ---
