Sinnoh_Cafe_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_Cafe_EventScript_Person1::
	goto SinnohP_cafe_2
	end

Sinnoh_Cafe_EventScript_Person2::
	lock
	faceplayer
	goto_if_set 0x3DA0, Sinnoh_Cafe_EventScript_Person2_Given
	msgbox Sinnoh_Cafe_Text_Person2, MSGBOX_DEFAULT
	msgbox Sinnoh_Cafe_Text_Person2Gift, MSGBOX_DEFAULT
	giveitem ITEM_MOOMOO_MILK, 12
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3DA0
	release
	end

Sinnoh_Cafe_EventScript_Person2_Given::
	msgbox Sinnoh_Cafe_Text_Person2, MSGBOX_DEFAULT
	release
	end

Sinnoh_Cafe_EventScript_Person6::
	goto SinnohP_cafe_4
	end

Sinnoh_Cafe_Text_Person2:
	.string "Welcome to the CAFE CABIN! Today's\n"
	.string "special is MOOMOO MILK. It's always\l"
	.string "MOOMOO MILK.$"

Sinnoh_Cafe_Text_Person2Gift:
	.string "Here, have a few bottles on the house.\n"
	.string "Fresh from the farm!$"

@ --- end of SINNOH words ---
