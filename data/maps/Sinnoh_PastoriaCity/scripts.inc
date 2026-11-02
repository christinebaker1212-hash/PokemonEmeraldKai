Sinnoh_PastoriaCity_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_PastoriaCity_OnTransition
	.byte 0

Sinnoh_PastoriaCity_OnTransition:
	special Sinnoh_LegendsRefresh
	end

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_PastoriaCity_EventScript_Person10::
	goto SinnohP_pastoria_city_4
	end

Sinnoh_PastoriaCity_EventScript_Person11::
	msgbox Sinnoh_PastoriaCity_Text_Person11, MSGBOX_NPC
	end

Sinnoh_PastoriaCity_EventScript_Person12::
	msgbox Sinnoh_PastoriaCity_Text_Person12, MSGBOX_NPC
	end

Sinnoh_PastoriaCity_EventScript_Person13::
	goto SinnohP_pastoria_city_7
	end

Sinnoh_PastoriaCity_EventScript_Person18::
	goto SinnohP_pastoria_city_2
	end

Sinnoh_PastoriaCity_EventScript_Person19::
	goto SinnohP_pastoria_city_3
	end

Sinnoh_PastoriaCity_EventScript_Person20::
	goto SinnohP_pastoria_city_5
	end

Sinnoh_PastoriaCity_EventScript_Person22::
	msgbox Sinnoh_PastoriaCity_Text_Person22, MSGBOX_NPC
	end

Sinnoh_PastoriaCity_EventScript_Person24::
	msgbox Sinnoh_PastoriaCity_Text_Person24, MSGBOX_NPC
	end

Sinnoh_PastoriaCity_EventScript_Person25::
	goto SinnohP_pastoria_city_11
	end

Sinnoh_PastoriaCity_EventScript_Person26::
	goto SinnohP_pastoria_city_12
	end

Sinnoh_PastoriaCity_EventScript_Person28::
	msgbox Sinnoh_PastoriaCity_Text_Person28, MSGBOX_NPC
	end

Sinnoh_PastoriaCity_EventScript_Person29::
	msgbox Sinnoh_PastoriaCity_Text_Person29, MSGBOX_NPC
	end

Sinnoh_PastoriaCity_EventScript_Sign5::
	goto SinnohP_pastoria_city_13
	end

Sinnoh_PastoriaCity_EventScript_Sign6::
	goto SinnohP_pastoria_city_14
	end

Sinnoh_PastoriaCity_EventScript_Sign7::
	goto SinnohP_pastoria_city_15
	end

Sinnoh_PastoriaCity_EventScript_Sign9::
	goto SinnohP_pastoria_city_16
	end

Sinnoh_PastoriaCity_Text_Person11:
	.string "Hyah! I train by walking through the\n"
	.string "marsh. The mud makes your legs strong!$"

Sinnoh_PastoriaCity_Text_Person12:
	.string "CLEFAIRY: Pippi!\p"
	.string "It's waving its little arms.$"

Sinnoh_PastoriaCity_Text_Person22:
	.string "I'm totally not hiding from TEAM\n"
	.string "GALACTIC's bosses. I'm resting. In a\l"
	.string "bush.$"

Sinnoh_PastoriaCity_Text_Person24:
	.string "My parasol keeps the marsh mist off.\n"
	.string "My MOéMON hides under it too.$"

Sinnoh_PastoriaCity_Text_Person28:
	.string "WAKE: You're the TRAINER everyone's\n"
	.string "talking about! I'm CRASHER WAKE,\l"
	.string "PASTORIA's GYM LEADER!\p"
	.string "Come splash around in my GYM anytime!$"

Sinnoh_PastoriaCity_Text_Person29:
	.string "CROAGUNK puffs out its cheeks and\n"
	.string "croaks.$"

@ --- end of SINNOH words ---
