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
	goto SinnohP_eterna_city_condominiums_2f_2
	end

Sinnoh_EternaCityCondominiums2f_EventScript_Person3::
	goto SinnohP_eterna_city_condominiums_2f_3
	end

Sinnoh_EternaCityCondominiums2f_Text_Person1:
	.string "My grandson reads me the old stories\n"
	.string "every night. I know them all by heart\l"
	.string "now.$"

Sinnoh_EternaCityCondominiums2f_Text_Person1Gift:
	.string "Grandpa says the old stories hold old\n"
	.string "moves too. He wants you to have this\l"
	.string "TM.$"

@ --- end of SINNOH words ---
