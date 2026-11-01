Sinnoh_VeilstoneStore1f_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_VeilstoneStore1f_EventScript_Bg1::
	msgbox Sinnoh_VeilstoneStore1f_Text_Bg1, MSGBOX_SIGN
	end

Sinnoh_VeilstoneStore1f_EventScript_Bg2::
	msgbox Sinnoh_VeilstoneStore1f_Text_Bg2, MSGBOX_SIGN
	end

Sinnoh_VeilstoneStore1f_EventScript_Person1::
	msgbox Sinnoh_VeilstoneStore1f_Text_Person1, MSGBOX_NPC
	end

Sinnoh_VeilstoneStore1f_EventScript_Person2::
	msgbox Sinnoh_VeilstoneStore1f_Text_Person2, MSGBOX_NPC
	end

Sinnoh_VeilstoneStore1f_EventScript_Person3::
	msgbox Sinnoh_VeilstoneStore1f_Text_Person3, MSGBOX_NPC
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

Sinnoh_VeilstoneStore1f_Text_Bg1:
	.string "A notice on the wall.$"

Sinnoh_VeilstoneStore1f_Text_Bg2:
	.string "A directory of the floors in this\n"
	.string "building.$"

Sinnoh_VeilstoneStore1f_Text_Person1:
	.string "Welcome to the VEILSTONE DEPARTMENT\n"
	.string "STORE! Every floor has something for\l"
	.string "TRAINERS.$"

Sinnoh_VeilstoneStore1f_Text_Person2:
	.string "I'm here for a gift for my wife. Five\n"
	.string "floors and I still don't know what to\l"
	.string "get.$"

Sinnoh_VeilstoneStore1f_Text_Person3:
	.string "This store has everything. I came for\n"
	.string "one thing and forgot what it was.$"

Sinnoh_VeilstoneStore1f_Text_Person4:
	.string "Shopping is a sport, darling. And I'm a\n"
	.string "champion.$"

@ --- end of SINNOH words ---
