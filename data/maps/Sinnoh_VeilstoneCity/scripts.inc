Sinnoh_VeilstoneCity_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_VeilstoneCity_EventScript_Bg3::
	msgbox Sinnoh_VeilstoneCity_Text_Bg3, MSGBOX_SIGN
	end

Sinnoh_VeilstoneCity_EventScript_Bg4::
	msgbox Sinnoh_VeilstoneCity_Text_Bg4, MSGBOX_SIGN
	end

Sinnoh_VeilstoneCity_EventScript_Bg5::
	msgbox Sinnoh_VeilstoneCity_Text_Bg5, MSGBOX_SIGN
	end

Sinnoh_VeilstoneCity_EventScript_Bg6::
	msgbox Sinnoh_VeilstoneCity_Text_Bg6, MSGBOX_SIGN
	end

Sinnoh_VeilstoneCity_EventScript_Person1::
	msgbox Sinnoh_VeilstoneCity_Text_Person1, MSGBOX_NPC
	end

Sinnoh_VeilstoneCity_EventScript_Person2::
	msgbox Sinnoh_VeilstoneCity_Text_Person2, MSGBOX_NPC
	end

Sinnoh_VeilstoneCity_EventScript_Person3::
	msgbox Sinnoh_VeilstoneCity_Text_Person3, MSGBOX_NPC
	end

Sinnoh_VeilstoneCity_EventScript_Person4::
	msgbox Sinnoh_VeilstoneCity_Text_Person4, MSGBOX_NPC
	end

Sinnoh_VeilstoneCity_EventScript_Person5::
	msgbox Sinnoh_VeilstoneCity_Text_Person5, MSGBOX_NPC
	end

Sinnoh_VeilstoneCity_EventScript_Person6::
	msgbox Sinnoh_VeilstoneCity_Text_Person6, MSGBOX_NPC
	end

Sinnoh_VeilstoneCity_EventScript_Person7::
	msgbox Sinnoh_VeilstoneCity_Text_Person7, MSGBOX_NPC
	end

Sinnoh_VeilstoneCity_EventScript_Person8::
	msgbox Sinnoh_VeilstoneCity_Text_Person8, MSGBOX_NPC
	end

Sinnoh_VeilstoneCity_EventScript_Person21::
	msgbox Sinnoh_VeilstoneCity_Text_Person21, MSGBOX_NPC
	end

Sinnoh_VeilstoneCity_EventScript_Person22::
	msgbox Sinnoh_VeilstoneCity_Text_Person22, MSGBOX_NPC
	end

Sinnoh_VeilstoneCity_EventScript_Person23::
	lock
	faceplayer
	goto_if_set 0x3DE0, Sinnoh_VeilstoneCity_EventScript_Person23_Given
	msgbox Sinnoh_VeilstoneCity_Text_Person23, MSGBOX_DEFAULT
	msgbox Sinnoh_VeilstoneCity_Text_Person23Gift, MSGBOX_DEFAULT
	giveitem ITEM_TM63, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3DE0
	release
	end

Sinnoh_VeilstoneCity_EventScript_Person23_Given::
	msgbox Sinnoh_VeilstoneCity_Text_Person23, MSGBOX_DEFAULT
	release
	end

Sinnoh_VeilstoneCity_EventScript_Person24::
	msgbox Sinnoh_VeilstoneCity_Text_Person24, MSGBOX_NPC
	end

Sinnoh_VeilstoneCity_EventScript_Person25::
	msgbox Sinnoh_VeilstoneCity_Text_Person25, MSGBOX_NPC
	end

Sinnoh_VeilstoneCity_EventScript_Person26::
	msgbox Sinnoh_VeilstoneCity_Text_Person26, MSGBOX_NPC
	end

Sinnoh_VeilstoneCity_EventScript_Person27::
	msgbox Sinnoh_VeilstoneCity_Text_Person27, MSGBOX_NPC
	end

Sinnoh_VeilstoneCity_EventScript_Person29::
	msgbox Sinnoh_VeilstoneCity_Text_Person29, MSGBOX_NPC
	end

Sinnoh_VeilstoneCity_EventScript_Person30::
	msgbox Sinnoh_VeilstoneCity_Text_Person30, MSGBOX_NPC
	end

Sinnoh_VeilstoneCity_EventScript_Sign9::
	msgbox Sinnoh_VeilstoneCity_Text_Sign9, MSGBOX_SIGN
	end

Sinnoh_VeilstoneCity_EventScript_Sign10::
	msgbox Sinnoh_VeilstoneCity_Text_Sign10, MSGBOX_SIGN
	end

Sinnoh_VeilstoneCity_EventScript_Sign11::
	msgbox Sinnoh_VeilstoneCity_Text_Sign11, MSGBOX_SIGN
	end

Sinnoh_VeilstoneCity_EventScript_Sign12::
	msgbox Sinnoh_VeilstoneCity_Text_Sign12, MSGBOX_SIGN
	end

Sinnoh_VeilstoneCity_EventScript_Sign13::
	msgbox Sinnoh_VeilstoneCity_Text_Sign13, MSGBOX_SIGN
	end

Sinnoh_VeilstoneCity_EventScript_Sign14::
	msgbox Sinnoh_VeilstoneCity_Text_Sign14, MSGBOX_SIGN
	end

Sinnoh_VeilstoneCity_EventScript_Sign15::
	msgbox Sinnoh_VeilstoneCity_Text_Sign15, MSGBOX_SIGN
	end

Sinnoh_VeilstoneCity_EventScript_Sign16::
	msgbox Sinnoh_VeilstoneCity_Text_Sign16, MSGBOX_SIGN
	end

Sinnoh_VeilstoneCity_Text_Bg3:
	.string "A huge rock that fell from the sky\n"
	.string "long ago. Its surface feels strangely\l"
	.string "warm.$"

Sinnoh_VeilstoneCity_Text_Bg4:
	.string "A huge rock that fell from the sky\n"
	.string "long ago. It's covered in tiny\l"
	.string "glittering specks.$"

Sinnoh_VeilstoneCity_Text_Bg5:
	.string "A huge rock that fell from the sky\n"
	.string "long ago. There's a deep crack along\l"
	.string "one side.$"

Sinnoh_VeilstoneCity_Text_Bg6:
	.string "A huge rock that fell from the sky\n"
	.string "long ago. It seems to hum, very\l"
	.string "faintly.$"

Sinnoh_VeilstoneCity_Text_Person1:
	.string "VEILSTONE is carved right out of the\n"
	.string "mountains. The cliffs keep the wind\l"
	.string "out.$"

Sinnoh_VeilstoneCity_Text_Person2:
	.string "The department store has everything.\n"
	.string "I went in for one thing and came out\l"
	.string "with ten.$"

Sinnoh_VeilstoneCity_Text_Person3:
	.string "MAYLENE trains from sunrise to\n"
	.string "sunset. She's the hardest worker in\l"
	.string "the city.$"

Sinnoh_VeilstoneCity_Text_Person4:
	.string "The GAME CORNER is just over there. I\n"
	.string "only go to watch, of course.$"

Sinnoh_VeilstoneCity_Text_Person5:
	.string "Hi-yah! I'm going to challenge MAYLENE\n"
	.string "again tomorrow. And the day after!$"

Sinnoh_VeilstoneCity_Text_Person6:
	.string "See those big rocks? People say they\n"
	.string "fell from the sky a long time ago.$"

Sinnoh_VeilstoneCity_Text_Person7:
	.string "TEAM GALACTIC's headquarters are\n"
	.string "somewhere in this city. Please be\l"
	.string "careful!$"

Sinnoh_VeilstoneCity_Text_Person8:
	.string "This warehouse is TEAM GALACTIC\n"
	.string "property. Hands off!$"

Sinnoh_VeilstoneCity_Text_Person21:
	.string "The warehouse is locked tight. Not\n"
	.string "that I lost the key. Nope.$"

Sinnoh_VeilstoneCity_Text_Person22:
	.string "My guitar sounds great bouncing off\n"
	.string "these cliffs. Listen! ...Okay, maybe\l"
	.string "not that great.$"

Sinnoh_VeilstoneCity_Text_Person23:
	.string "I came to VEILSTONE to get tough. The\n"
	.string "stairs alone are a workout!$"

Sinnoh_VeilstoneCity_Text_Person23Gift:
	.string "A TOUGH TRAINER deserves a TOUGH TM.\n"
	.string "Here!$"

Sinnoh_VeilstoneCity_Text_Person24:
	.string "I'm supposed to be guarding the\n"
	.string "STORAGE KEY. I'm doing great. Totally\l"
	.string "great.$"

Sinnoh_VeilstoneCity_Text_Person25:
	.string "Our HQ is the biggest building in\n"
	.string "VEILSTONE. Can't miss it. Please miss\l"
	.string "it.$"

Sinnoh_VeilstoneCity_Text_Person26:
	.string "WAKE: Hah! TEAM GALACTIC grabbed\n"
	.string "some POKéMON and ran into that\l"
	.string "warehouse!\p"
	.string "I'll be standing guard. Nobody gets\n"
	.string "past CRASHER WAKE!$"

Sinnoh_VeilstoneCity_Text_Person27:
	.string "LOOKER: TEAM GALACTIC's headquarters\n"
	.string "stands right here in VEILSTONE.\p"
	.string "I am gathering clues. Clue number one:\n"
	.string "they really like the color blue.$"

Sinnoh_VeilstoneCity_Text_Person29:
	.string "Training under the stars is the best.\n"
	.string "You can hear your MOéMON breathe.$"

Sinnoh_VeilstoneCity_Text_Person30:
	.string "I play here every evening. The rocks\n"
	.string "are my audience. They never boo.$"

Sinnoh_VeilstoneCity_Text_Sign9:
	.string "VEILSTONE CITY\p"
	.string "The rugged town carved from stone.$"

Sinnoh_VeilstoneCity_Text_Sign10:
	.string "A large warehouse. The doors are\n"
	.string "locked.$"

Sinnoh_VeilstoneCity_Text_Sign11:
	.string "A tall, strange building. Nobody seems\n"
	.string "to come in or out.$"

Sinnoh_VeilstoneCity_Text_Sign12:
	.string "VEILSTONE DEPARTMENT STORE\p"
	.string "Everything a TRAINER needs, all in one\n"
	.string "place.$"

Sinnoh_VeilstoneCity_Text_Sign13:
	.string "VEILSTONE CITY MOéMON GYM\p"
	.string "LEADER: MAYLENE\p"
	.string "The fighter who trains with all her\n"
	.string "heart!$"

Sinnoh_VeilstoneCity_Text_Sign14:
	.string "GAME CORNER\p"
	.string "Try your luck!$"

Sinnoh_VeilstoneCity_Text_Sign15:
	.string "PRIZE EXCHANGE\p"
	.string "Trade your coins for prizes.$"

Sinnoh_VeilstoneCity_Text_Sign16:
	.string "ROUTE 214\p"
	.string "To LAKE VALOR$"

@ --- end of SINNOH words ---
