Sinnoh_VeilstoneStore1f_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_VeilstoneStore1f_EventScript_Bg1::
	goto SinnohP_veilstone_store_1f_6
	end

Sinnoh_VeilstoneStore1f_EventScript_Bg2::
	goto SinnohP_veilstone_store_1f_7
	end

Sinnoh_VeilstoneStore1f_EventScript_Person1::
	goto SinnohP_veilstone_store_1f_1
	end

Sinnoh_VeilstoneStore1f_EventScript_Person2::
	goto SinnohP_veilstone_store_1f_2
	end

Sinnoh_VeilstoneStore1f_EventScript_Person3::
	goto SinnohP_veilstone_store_1f_3
	end

Sinnoh_VeilstoneStore1f_EventScript_Person4::
	msgbox Sinnoh_VeilstoneStore1f_Text_Person4, MSGBOX_NPC
	end

Sinnoh_VeilstoneStore1f_EventScript_Person5::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart Sinnoh_Mart_Veilstone1fRight
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

Sinnoh_VeilstoneStore1f_EventScript_Person6::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart Sinnoh_Mart_Veilstone1fLeft
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

Sinnoh_VeilstoneStore1f_Text_Person4:
	.string "Shopping is a sport, darling. And I'm a\n"
	.string "champion.$"

@ --- end of SINNOH words ---
