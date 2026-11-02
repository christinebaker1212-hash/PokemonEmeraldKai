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
	goto SinnohP_veilstone_city_2
	end

Sinnoh_VeilstoneCity_EventScript_Person2::
	goto SinnohP_veilstone_city_4
	end

Sinnoh_VeilstoneCity_EventScript_Person3::
	goto SinnohP_veilstone_city_5
	end

Sinnoh_VeilstoneCity_EventScript_Person4::
	goto SinnohP_veilstone_city_8
	end

Sinnoh_VeilstoneCity_EventScript_Person5::
	goto SinnohP_veilstone_city_9
	end

Sinnoh_VeilstoneCity_EventScript_Person6::
	goto SinnohP_veilstone_city_6
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
	goto SinnohP_veilstone_city_3
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
	goto SinnohP_veilstone_city_32
	end

Sinnoh_VeilstoneCity_EventScript_Person30::
	goto SinnohP_veilstone_city_31
	end

Sinnoh_VeilstoneCity_EventScript_Sign9::
	goto SinnohP_veilstone_city_17
	end

Sinnoh_VeilstoneCity_EventScript_Sign10::
	goto SinnohP_veilstone_city_19
	end

Sinnoh_VeilstoneCity_EventScript_Sign11::
	goto SinnohP_veilstone_city_20
	end

Sinnoh_VeilstoneCity_EventScript_Sign12::
	goto SinnohP_veilstone_city_21
	end

Sinnoh_VeilstoneCity_EventScript_Sign13::
	goto SinnohP_veilstone_city_18
	end

Sinnoh_VeilstoneCity_EventScript_Sign14::
	goto SinnohP_veilstone_city_22
	end

Sinnoh_VeilstoneCity_EventScript_Sign15::
	goto SinnohP_veilstone_city_23
	end

Sinnoh_VeilstoneCity_EventScript_Sign16::
	goto SinnohP_veilstone_city_24
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

@ --- end of SINNOH words ---
