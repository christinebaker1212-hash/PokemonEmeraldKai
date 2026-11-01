Sinnoh_EternaCityCondominiums2f_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_EternaCityCondominiums2f_EventScript_Person1::
	lock
	faceplayer
	goto_if_set 0x3DA7, Sinnoh_EternaCityCondominiums2f_EventScript_Person1_Given
	msgbox Sinnoh_EternaCityCondominiums2f_Text_Person1, MSGBOX_DEFAULT
	msgbox Sinnoh_EternaCityCondominiums2f_Text_Person1Gift, MSGBOX_DEFAULT
	giveitem ITEM_TM67, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3DA7
	release
	end

Sinnoh_EternaCityCondominiums2f_EventScript_Person1_Given::
	msgbox Sinnoh_EternaCityCondominiums2f_Text_Person1, MSGBOX_DEFAULT
	release
	end

Sinnoh_EternaCityCondominiums2f_EventScript_Person2::
	msgbox Sinnoh_EternaCityCondominiums2f_Text_Person2, MSGBOX_NPC
	end

Sinnoh_EternaCityCondominiums2f_EventScript_Person3::
	msgbox Sinnoh_EternaCityCondominiums2f_Text_Person3, MSGBOX_NPC
	end

Sinnoh_EternaCityCondominiums2f_Text_Person1:
	.string "My grandson reads me the old stories\n"
	.string "every night. I know them all by heart\l"
	.string "now.$"

Sinnoh_EternaCityCondominiums2f_Text_Person1Gift:
	.string "Grandpa says the old stories hold old\n"
	.string "moves too. He wants you to have this\l"
	.string "TM.$"

Sinnoh_EternaCityCondominiums2f_Text_Person2:
	.string "I'm playing hide and seek with my\n"
	.string "MOéMON! Shh! Don't tell it where I am!$"

Sinnoh_EternaCityCondominiums2f_Text_Person3:
	.string "They say the statue in town was put\n"
	.string "up when ETERNA was founded. That's a\l"
	.string "long time.$"

@ --- end of SINNOH words ---
