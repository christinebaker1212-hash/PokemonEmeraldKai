Sinnoh_Route215_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_Route215_EventScript_Person24::
	goto SinnohP_route_215_2
	end

Sinnoh_Route215_EventScript_Person26::
	msgbox Sinnoh_Route215_Text_Person26, MSGBOX_NPC
	end

Sinnoh_Route215_EventScript_Person27::
	msgbox Sinnoh_Route215_Text_Person27, MSGBOX_NPC
	end

Sinnoh_Route215_EventScript_Sign9::
	goto SinnohP_route_215_3
	end

Sinnoh_Route215_EventScript_Sign10::
	goto SinnohP_route_215_4
	end

Sinnoh_Route215_EventScript_Sign28::
	goto SinnohP_route_215_8
	end

Sinnoh_Route215_Text_Person26:
	.string "I'm running laps till sunrise. Battles?\n"
	.string "Not till morning!$"

Sinnoh_Route215_Text_Person27:
	.string "Night runs are my favorite. No\n"
	.string "battling though. Too sleepy.$"

@ --- end of SINNOH words ---
