Sinnoh_VeilstoneStore4f_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_VeilstoneStore4f_EventScript_Bg1::
	goto SinnohP_veilstone_store_4f_7
	end

Sinnoh_VeilstoneStore4f_EventScript_Person1::
	goto SinnohP_veilstone_store_4f_2
	end

Sinnoh_VeilstoneStore4f_EventScript_Person2::
	goto SinnohP_veilstone_store_4f_3
	end

Sinnoh_VeilstoneStore4f_EventScript_Person3::
	msgbox Sinnoh_VeilstoneStore4f_Text_Person3, MSGBOX_NPC
	end

Sinnoh_VeilstoneStore4f_EventScript_Person6::
	goto SinnohP_veilstone_store_4f_4
	end

Sinnoh_VeilstoneStore4f_Text_Person3:
	.string "Welcome to the fourth floor!\n"
	.string "Decorations and dolls for your room.$"

@ --- end of SINNOH words ---
