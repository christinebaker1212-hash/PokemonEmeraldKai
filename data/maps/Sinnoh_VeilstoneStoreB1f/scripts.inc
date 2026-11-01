Sinnoh_VeilstoneStoreB1f_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_VeilstoneStoreB1f_EventScript_Bg1::
	msgbox Sinnoh_VeilstoneStoreB1f_Text_Bg1, MSGBOX_SIGN
	end

Sinnoh_VeilstoneStoreB1f_EventScript_Person1::
	msgbox Sinnoh_VeilstoneStoreB1f_Text_Person1, MSGBOX_NPC
	end

Sinnoh_VeilstoneStoreB1f_EventScript_Person4::
	msgbox Sinnoh_VeilstoneStoreB1f_Text_Person4, MSGBOX_NPC
	end

Sinnoh_VeilstoneStoreB1f_EventScript_Person5::
	msgbox Sinnoh_VeilstoneStoreB1f_Text_Person5, MSGBOX_NPC
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

Sinnoh_VeilstoneStoreB1f_EventScript_Person8::
	msgbox Sinnoh_VeilstoneStoreB1f_Text_Person8, MSGBOX_NPC
	end

Sinnoh_VeilstoneStoreB1f_EventScript_Person9::
	msgbox Sinnoh_VeilstoneStoreB1f_Text_Person9, MSGBOX_NPC
	end

Sinnoh_VeilstoneStoreB1f_Text_Bg1:
	.string "A directory of the floors in this\n"
	.string "building.$"

Sinnoh_VeilstoneStoreB1f_Text_Person1:
	.string "Welcome to the basement! We have\n"
	.string "special goods down here.$"

Sinnoh_VeilstoneStoreB1f_Text_Person4:
	.string "The basement has the rarest items,\n"
	.string "darling. I come here first.$"

Sinnoh_VeilstoneStoreB1f_Text_Person5:
	.string "Welcome to the basement! We have\n"
	.string "special goods down here.$"

Sinnoh_VeilstoneStoreB1f_Text_Person8:
	.string "I'm buying gifts for my MOéMON. They\n"
	.string "deserve the very best.$"

Sinnoh_VeilstoneStoreB1f_Text_Person9:
	.string "A gentleman never shops in a hurry. I\n"
	.string "take my time down here.$"

@ --- end of SINNOH words ---
