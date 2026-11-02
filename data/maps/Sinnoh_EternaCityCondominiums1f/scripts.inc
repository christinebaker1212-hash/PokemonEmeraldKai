Sinnoh_EternaCityCondominiums1f_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_EternaCityCondominiums1f_EventScript_Person1::
	msgbox Sinnoh_EternaCityCondominiums1f_Text_Person1, MSGBOX_NPC
	end

Sinnoh_EternaCityCondominiums1f_EventScript_Person2::
	goto SinnohP_eterna_city_condominiums_1f_3
	end

Sinnoh_EternaCityCondominiums1f_EventScript_Person3::
	msgbox Sinnoh_EternaCityCondominiums1f_Text_Person3, MSGBOX_NPC
	end

Sinnoh_EternaCityCondominiums1f_Text_Person1:
	.string "There's an old man upstairs who knows\n"
	.string "everything about ETERNA's legends.$"

Sinnoh_EternaCityCondominiums1f_Text_Person3:
	.string "Every family in this building has a\n"
	.string "MOéMON. Some have five!$"

@ --- end of SINNOH words ---
