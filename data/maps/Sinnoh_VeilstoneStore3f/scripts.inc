Sinnoh_VeilstoneStore3f_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_VeilstoneStore3f_EventScript_Bg1::
	msgbox Sinnoh_VeilstoneStore3f_Text_Bg1, MSGBOX_SIGN
	end

Sinnoh_VeilstoneStore3f_EventScript_Person1::
	msgbox Sinnoh_VeilstoneStore3f_Text_Person1, MSGBOX_NPC
	end

Sinnoh_VeilstoneStore3f_EventScript_Person2::
	msgbox Sinnoh_VeilstoneStore3f_Text_Person2, MSGBOX_NPC
	end

Sinnoh_VeilstoneStore3f_EventScript_Person3::
	msgbox Sinnoh_VeilstoneStore3f_Text_Person3, MSGBOX_NPC
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
	msgbox Sinnoh_VeilstoneStore3f_Text_Person6, MSGBOX_NPC
	end

Sinnoh_VeilstoneStore3f_Text_Bg1:
	.string "A directory of the floors in this\n"
	.string "building.$"

Sinnoh_VeilstoneStore3f_Text_Person1:
	.string "Welcome to the third floor! TMs to\n"
	.string "teach your MOéMON new tricks.$"

Sinnoh_VeilstoneStore3f_Text_Person2:
	.string "I want every TM! My piggy bank says\n"
	.string "otherwise.$"

Sinnoh_VeilstoneStore3f_Text_Person3:
	.string "I'm looking for a TM that teaches\n"
	.string "SING. My MOéMON has the voice for it!$"

Sinnoh_VeilstoneStore3f_Text_Person6:
	.string "Teaching your MOéMON a new move is\n"
	.string "like giving it a new toy. They love it!$"

@ --- end of SINNOH words ---
