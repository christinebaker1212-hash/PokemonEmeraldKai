Sinnoh_Cafe_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_Cafe_EventScript_Person1::
	msgbox Sinnoh_Cafe_Text_Person1, MSGBOX_NPC
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
	msgbox Sinnoh_Cafe_Text_Person6, MSGBOX_NPC
	end

Sinnoh_Cafe_Text_Person1:
	.string "The MOOMOO MILK here comes straight\n"
	.string "from the ranch. It's the freshest\l"
	.string "you'll ever taste.$"

Sinnoh_Cafe_Text_Person2:
	.string "Welcome to the CAFE CABIN! Today's\n"
	.string "special is MOOMOO MILK. It's always\l"
	.string "MOOMOO MILK.$"

Sinnoh_Cafe_Text_Person2Gift:
	.string "Here, have a few bottles on the house.\n"
	.string "Fresh from the farm!$"

Sinnoh_Cafe_Text_Person6:
	.string "I come here every day for a glass of\n"
	.string "milk. Keeps me young!$"

@ --- end of SINNOH words ---
