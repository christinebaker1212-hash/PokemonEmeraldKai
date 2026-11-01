Sinnoh_SolaceonRuinsRoom2_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_SolaceonRuinsRoom2_EventScript_Person1::
	lock
	faceplayer
	goto_if_set 0x3DD9, Sinnoh_SolaceonRuinsRoom2_EventScript_Person1_Given
	msgbox Sinnoh_SolaceonRuinsRoom2_Text_Person1, MSGBOX_DEFAULT
	msgbox Sinnoh_SolaceonRuinsRoom2_Text_Person1Gift, MSGBOX_DEFAULT
	giveitem ITEM_GREEN_SHARD, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3DD9
	release
	end

Sinnoh_SolaceonRuinsRoom2_EventScript_Person1_Given::
	msgbox Sinnoh_SolaceonRuinsRoom2_Text_Person1, MSGBOX_DEFAULT
	release
	end

Sinnoh_SolaceonRuinsRoom2_Text_Person1:
	.string "These ruins go on forever! I keep\n"
	.string "finding new rooms.$"

Sinnoh_SolaceonRuinsRoom2_Text_Person1Gift:
	.string "I found this green stone in one of the\n"
	.string "rooms. You can have it!$"

@ --- end of SINNOH words ---
