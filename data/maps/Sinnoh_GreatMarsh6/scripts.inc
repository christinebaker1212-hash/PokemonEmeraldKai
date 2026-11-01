Sinnoh_GreatMarsh6_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_GreatMarsh6_EventScript_Person3::
	lock
	faceplayer
	goto_if_set 0x3DB4, Sinnoh_GreatMarsh6_EventScript_Person3_Given
	msgbox Sinnoh_GreatMarsh6_Text_Person3, MSGBOX_DEFAULT
	msgbox Sinnoh_GreatMarsh6_Text_Person3Gift, MSGBOX_DEFAULT
	giveitem ITEM_RED_SHARD, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3DB4
	release
	end

Sinnoh_GreatMarsh6_EventScript_Person3_Given::
	msgbox Sinnoh_GreatMarsh6_Text_Person3, MSGBOX_DEFAULT
	release
	end

Sinnoh_GreatMarsh6_EventScript_Sign2::
	msgbox Sinnoh_GreatMarsh6_Text_Sign2, MSGBOX_SIGN
	end

Sinnoh_GreatMarsh6_Text_Person3:
	.string "The marsh is the best place to train.\n"
	.string "The mud makes your legs strong!$"

Sinnoh_GreatMarsh6_Text_Person3Gift:
	.string "I dug this out of the mud. Odd red\n"
	.string "stone. Maybe you know what it's for?$"

Sinnoh_GreatMarsh6_Text_Sign2:
	.string "A map of this area.$"

@ --- end of SINNOH words ---
