Sinnoh_SolaceonTown_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_SolaceonTown_EventScript_Person1::
	goto SinnohP_solaceon_town_2
	end

Sinnoh_SolaceonTown_EventScript_Person2::
	goto SinnohP_solaceon_town_3
	end

Sinnoh_SolaceonTown_EventScript_Person3::
	goto SinnohP_solaceon_town_1
	end

Sinnoh_SolaceonTown_EventScript_Person8::
	goto SinnohP_solaceon_town_4
	end

Sinnoh_SolaceonTown_EventScript_Person9::
	msgbox Sinnoh_SolaceonTown_Text_Person9, MSGBOX_NPC
	end

Sinnoh_SolaceonTown_EventScript_Person15::
	goto SinnohP_solaceon_town_5
	end

Sinnoh_SolaceonTown_EventScript_Person19::
	msgbox Sinnoh_SolaceonTown_Text_Person19, MSGBOX_NPC
	end

Sinnoh_SolaceonTown_EventScript_Sign5::
	goto SinnohP_solaceon_town_7
	end

Sinnoh_SolaceonTown_EventScript_Sign6::
	goto SinnohP_solaceon_town_8
	end

Sinnoh_SolaceonTown_EventScript_Sign7::
	goto SinnohP_solaceon_town_9
	end

Sinnoh_SolaceonTown_Text_Person9:
	.string "The ruins east of town are full of\n"
	.string "strange symbols. I've been studying\l"
	.string "them for years.$"

Sinnoh_SolaceonTown_Text_Person19:
	.string "BARRY: SOLACEON's ruins have UNOWN all\n"
	.string "over the walls! I tried to read them.\l"
	.string "They said 'GO AWAY'. Probably.$"

@ --- end of SINNOH words ---
