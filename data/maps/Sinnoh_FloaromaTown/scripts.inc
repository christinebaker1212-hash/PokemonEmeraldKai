Sinnoh_FloaromaTown_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_FloaromaTown_EventScript_Person1::
	msgbox Sinnoh_FloaromaTown_Text_Person1, MSGBOX_NPC
	end

Sinnoh_FloaromaTown_EventScript_Person2::
	msgbox Sinnoh_FloaromaTown_Text_Person2, MSGBOX_NPC
	end

Sinnoh_FloaromaTown_EventScript_Person3::
	msgbox Sinnoh_FloaromaTown_Text_Person3, MSGBOX_NPC
	end

Sinnoh_FloaromaTown_EventScript_Person4::
	lock
	faceplayer
	goto_if_set 0x3DAD, Sinnoh_FloaromaTown_EventScript_Person4_Given
	msgbox Sinnoh_FloaromaTown_Text_Person4, MSGBOX_DEFAULT
	msgbox Sinnoh_FloaromaTown_Text_Person4Gift, MSGBOX_DEFAULT
	giveitem ITEM_GRACIDEA, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3DAD
	release
	end

Sinnoh_FloaromaTown_EventScript_Person4_Given::
	msgbox Sinnoh_FloaromaTown_Text_Person4, MSGBOX_DEFAULT
	release
	end

Sinnoh_FloaromaTown_EventScript_Person5::
	msgbox Sinnoh_FloaromaTown_Text_Person5, MSGBOX_NPC
	end

Sinnoh_FloaromaTown_EventScript_Person8::
	msgbox Sinnoh_FloaromaTown_Text_Person8, MSGBOX_NPC
	end

Sinnoh_FloaromaTown_EventScript_Person9::
	msgbox Sinnoh_FloaromaTown_Text_Person9, MSGBOX_NPC
	end

Sinnoh_FloaromaTown_EventScript_Person13::
	msgbox Sinnoh_FloaromaTown_Text_Person13, MSGBOX_NPC
	end

Sinnoh_FloaromaTown_EventScript_Sign10::
	msgbox Sinnoh_FloaromaTown_Text_Sign10, MSGBOX_SIGN
	end

Sinnoh_FloaromaTown_EventScript_Sign11::
	msgbox Sinnoh_FloaromaTown_Text_Sign11, MSGBOX_SIGN
	end

Sinnoh_FloaromaTown_EventScript_Sign12::
	msgbox Sinnoh_FloaromaTown_Text_Sign12, MSGBOX_SIGN
	end

Sinnoh_FloaromaTown_Text_Person1:
	.string "The flowers in FLOAROMA grow all year\n"
	.string "long. Nobody knows why, and nobody\l"
	.string "minds.$"

Sinnoh_FloaromaTown_Text_Person2:
	.string "I make flower crowns for my MOéMON. It\n"
	.string "wears one every day!$"

Sinnoh_FloaromaTown_Text_Person3:
	.string "The meadow is full of sweet-smelling\n"
	.string "honey trees. Bring HONEY if you have\l"
	.string "some!$"

Sinnoh_FloaromaTown_Text_Person4:
	.string "The flower shop sells little gifts you\n"
	.string "can give your MOéMON to carry. They're\l"
	.string "adorable.$"

Sinnoh_FloaromaTown_Text_Person4Gift:
	.string "Oh, and someone left this flower with\n"
	.string "me. It's meant for a very special\l"
	.string "MOéMON. Take it!$"

Sinnoh_FloaromaTown_Text_Person5:
	.string "I sneezed so many times walking\n"
	.string "through the meadow. Worth it, though!$"

Sinnoh_FloaromaTown_Text_Person8:
	.string "TEAM GALACTIC's taking the sweet\n"
	.string "HONEY from the MEADOW. It's for...\l"
	.string "science!$"

Sinnoh_FloaromaTown_Text_Person9:
	.string "Don't look at me like that. Honey is\n"
	.string "very important to our plans. Very.$"

Sinnoh_FloaromaTown_Text_Person13:
	.string "I set up my tent in the meadow. Waking\n"
	.string "up to flowers is the best.$"

Sinnoh_FloaromaTown_Text_Sign10:
	.string "FLOAROMA TOWN\p"
	.string "The town that blooms all year.$"

Sinnoh_FloaromaTown_Text_Sign11:
	.string "PICK A PECK OF COLORS FLOWER SHOP\p"
	.string "Flowers for every occasion.$"

Sinnoh_FloaromaTown_Text_Sign12:
	.string "FLOAROMA MEADOW\p"
	.string "Please don't pick the flowers!$"

@ --- end of SINNOH words ---
