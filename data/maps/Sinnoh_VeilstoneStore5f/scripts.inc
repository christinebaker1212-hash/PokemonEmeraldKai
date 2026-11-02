Sinnoh_VeilstoneStore5f_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_VeilstoneStore5f_EventScript_Bg1::
	msgbox Sinnoh_VeilstoneStore5f_Text_Bg1, MSGBOX_SIGN
	end

Sinnoh_VeilstoneStore5f_EventScript_Bg2::
	msgbox Sinnoh_VeilstoneStore5f_Text_Bg2, MSGBOX_SIGN
	end

Sinnoh_VeilstoneStore5f_EventScript_Bg3::
	goto SinnohP_veilstone_store_5f_5
	end

Sinnoh_VeilstoneStore5f_EventScript_Person1::
	goto SinnohP_veilstone_store_5f_1
	end

Sinnoh_VeilstoneStore5f_EventScript_Person2::
	goto SinnohP_veilstone_store_5f_2
	end

Sinnoh_VeilstoneStore5f_EventScript_Person3::
	goto SinnohP_veilstone_store_5f_3
	end

Sinnoh_VeilstoneStore5f_EventScript_Person4::
	goto SinnohP_veilstone_store_5f_4
	end

Sinnoh_VeilstoneStore5f_Text_Bg1:
	.string "A vending machine. It's out of order.$"

Sinnoh_VeilstoneStore5f_Text_Bg2:
	.string "A vending machine. It's out of order.$"

@ --- end of SINNOH words ---
