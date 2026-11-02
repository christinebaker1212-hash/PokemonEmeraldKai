Sinnoh_VeilstoneStore3f_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_VeilstoneStore3f_EventScript_Bg1::
	goto SinnohP_veilstone_store_3f_7
	end

Sinnoh_VeilstoneStore3f_EventScript_Person1::
	msgbox Sinnoh_VeilstoneStore3f_Text_Person1, MSGBOX_NPC
	end

Sinnoh_VeilstoneStore3f_EventScript_Person2::
	goto SinnohP_veilstone_store_3f_2
	end

Sinnoh_VeilstoneStore3f_EventScript_Person3::
	goto SinnohP_veilstone_store_3f_3
	end

Sinnoh_VeilstoneStore3f_EventScript_Person4::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart Sinnoh_Mart_Veilstone3fUp
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

Sinnoh_VeilstoneStore3f_EventScript_Person5::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart Sinnoh_Mart_Veilstone3fDown
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

Sinnoh_VeilstoneStore3f_EventScript_Person6::
	goto SinnohP_veilstone_store_3f_4
	end

Sinnoh_VeilstoneStore3f_Text_Person1:
	.string "Welcome to the third floor! TMs to\n"
	.string "teach your MOéMON new tricks.$"

@ --- end of SINNOH words ---
