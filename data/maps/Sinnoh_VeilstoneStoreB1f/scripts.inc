Sinnoh_VeilstoneStoreB1f_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_VeilstoneStoreB1f_EventScript_Bg1::
	goto SinnohP_veilstone_store_b1f_12
	end

Sinnoh_VeilstoneStoreB1f_EventScript_Person1::
	goto SinnohP_veilstone_store_b1f_4
	end

Sinnoh_VeilstoneStoreB1f_EventScript_Person3::
	goto SinnohP_veilstone_store_b1f_9
	end

Sinnoh_VeilstoneStoreB1f_EventScript_Person4::
	goto SinnohP_veilstone_store_b1f_2
	end

Sinnoh_VeilstoneStoreB1f_EventScript_Person5::
	goto SinnohP_veilstone_store_b1f_5
	end

Sinnoh_VeilstoneStoreB1f_EventScript_Person6::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart Sinnoh_Mart_VeilstoneB1f
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

Sinnoh_VeilstoneStoreB1f_EventScript_Person7::
	goto SinnohP_veilstone_store_b1f_6
	end

Sinnoh_VeilstoneStoreB1f_EventScript_Person8::
	goto SinnohP_veilstone_store_b1f_3
	end

Sinnoh_VeilstoneStoreB1f_EventScript_Person9::
	goto SinnohP_veilstone_store_b1f_1
	end

Sinnoh_VeilstoneStoreB1f_EventScript_Person10::
	msgbox Sinnoh_VeilstoneStoreB1f_Text_Person10, MSGBOX_NPC
	end

Sinnoh_VeilstoneStoreB1f_Text_Person10:
	.string "PROF. ROWAN: Even a PROFESSOR needs\n"
	.string "to restock. These POTIONS are for the\l"
	.string "lab's POKéMON.\p"
	.string "Keep traveling. Your notes on\n"
	.string "SINNOH's POKéMON have been most\l"
	.string "useful.$"

@ --- end of SINNOH words ---
