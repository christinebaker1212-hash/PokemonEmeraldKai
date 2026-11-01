Sinnoh_Route209LostTower5f_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_Route209LostTower5f_EventScript_Person1::
	lock
	faceplayer
	goto_if_set 0x3DCB, Sinnoh_Route209LostTower5f_EventScript_Person1_Given
	msgbox Sinnoh_Route209LostTower5f_Text_Person1, MSGBOX_DEFAULT
	msgbox Sinnoh_Route209LostTower5f_Text_Person1Gift, MSGBOX_DEFAULT
	giveitem ITEM_SPELL_TAG, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3DCB
	release
	end

Sinnoh_Route209LostTower5f_EventScript_Person1_Given::
	msgbox Sinnoh_Route209LostTower5f_Text_Person1, MSGBOX_DEFAULT
	release
	end

Sinnoh_Route209LostTower5f_EventScript_Person2::
	lock
	faceplayer
	goto_if_set 0x3DCC, Sinnoh_Route209LostTower5f_EventScript_Person2_Given
	msgbox Sinnoh_Route209LostTower5f_Text_Person2, MSGBOX_DEFAULT
	msgbox Sinnoh_Route209LostTower5f_Text_Person2Gift, MSGBOX_DEFAULT
	giveitem ITEM_CLEANSE_TAG, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3DCC
	release
	end

Sinnoh_Route209LostTower5f_EventScript_Person2_Given::
	msgbox Sinnoh_Route209LostTower5f_Text_Person2, MSGBOX_DEFAULT
	release
	end

Sinnoh_Route209LostTower5f_Text_Person1:
	.string "We've looked after this tower for many\n"
	.string "years. Every grave here was someone's\l"
	.string "best friend.$"

Sinnoh_Route209LostTower5f_Text_Person1Gift:
	.string "Please take this. It keeps the memory\n"
	.string "of a MOéMON's spirit.$"

Sinnoh_Route209LostTower5f_Text_Person2:
	.string "Thank you for climbing all this way.\n"
	.string "The MOéMON here would be glad of a\l"
	.string "visitor.$"

Sinnoh_Route209LostTower5f_Text_Person2Gift:
	.string "Please take this tag. It keeps\n"
	.string "restless spirits away from you.$"

@ --- end of SINNOH words ---
