Sinnoh_Route229_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_Route229_EventScript_Person3::
	lock
	faceplayer
	goto_if_set 0x3DD7, Sinnoh_Route229_EventScript_Person3_Given
	msgbox Sinnoh_Route229_Text_Person3, MSGBOX_DEFAULT
	msgbox Sinnoh_Route229_Text_Person3Gift, MSGBOX_DEFAULT
	giveitem ITEM_NUGGET, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3DD7
	release
	end

Sinnoh_Route229_EventScript_Person3_Given::
	msgbox Sinnoh_Route229_Text_Person3, MSGBOX_DEFAULT
	release
	end

Sinnoh_Route229_EventScript_Sign2::
	goto SinnohP_route_229_2
	end

Sinnoh_Route229_Text_Person3:
	.string "The grass on this route is full of BUG\n"
	.string "MOéMON. I'm covered in bites!$"

Sinnoh_Route229_Text_Person3Gift:
	.string "Here's something I found in the grass.\n"
	.string "Shiny, isn't it?$"

@ --- end of SINNOH words ---
