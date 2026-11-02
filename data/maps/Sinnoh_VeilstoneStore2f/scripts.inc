Sinnoh_VeilstoneStore2f_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_VeilstoneStore2f_EventScript_Bg1::
	goto SinnohP_veilstone_store_2f_8
	end

Sinnoh_VeilstoneStore2f_EventScript_Person1::
	goto SinnohP_veilstone_store_2f_2
	end

Sinnoh_VeilstoneStore2f_EventScript_Person2::
	goto SinnohP_veilstone_store_2f_3
	end

Sinnoh_VeilstoneStore2f_EventScript_Person3::
	goto SinnohP_veilstone_store_2f_5
	end

Sinnoh_VeilstoneStore2f_EventScript_Person4::
	goto SinnohP_veilstone_store_2f_1
	end

Sinnoh_VeilstoneStore2f_EventScript_Person5::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart Sinnoh_Mart_Veilstone2fUp
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

Sinnoh_VeilstoneStore2f_EventScript_Person6::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart Sinnoh_Mart_Veilstone2fMid
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

Sinnoh_VeilstoneStore2f_EventScript_Person7::
	goto SinnohP_veilstone_store_2f_4
	end

@ --- end of SINNOH words ---
