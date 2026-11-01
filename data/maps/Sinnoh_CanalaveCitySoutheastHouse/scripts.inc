Sinnoh_CanalaveCitySoutheastHouse_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_CanalaveCitySoutheastHouse_EventScript_Person1::
	msgbox Sinnoh_CanalaveCitySoutheastHouse_Text_Person1, MSGBOX_NPC
	end

Sinnoh_CanalaveCitySoutheastHouse_EventScript_Person2::
	lock
	faceplayer
	goto_if_set 0x3DA2, Sinnoh_CanalaveCitySoutheastHouse_EventScript_Person2_Given
	msgbox Sinnoh_CanalaveCitySoutheastHouse_Text_Person2, MSGBOX_DEFAULT
	msgbox Sinnoh_CanalaveCitySoutheastHouse_Text_Person2Gift, MSGBOX_DEFAULT
	giveitem ITEM_TM48, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3DA2
	release
	end

Sinnoh_CanalaveCitySoutheastHouse_EventScript_Person2_Given::
	msgbox Sinnoh_CanalaveCitySoutheastHouse_Text_Person2, MSGBOX_DEFAULT
	release
	end

Sinnoh_CanalaveCitySoutheastHouse_Text_Person1:
	.string "My dad's a sailor. He brings me a shell\n"
	.string "from every place he goes.$"

Sinnoh_CanalaveCitySoutheastHouse_Text_Person2:
	.string "I train on the docks at sunrise. The\n"
	.string "ships' horns are my alarm clock.$"

Sinnoh_CanalaveCitySoutheastHouse_Text_Person2Gift:
	.string "You look like you train hard too. Take\n"
	.string "this TM. It's served me well.$"

@ --- end of SINNOH words ---
