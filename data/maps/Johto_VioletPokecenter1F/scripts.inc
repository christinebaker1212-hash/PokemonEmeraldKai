Johto_VioletPokecenter1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_VioletPokecenter1F_EventScript_83FFD20
	.byte 0

Johto_VioletPokecenter1F_EventScript_83FFD20::
	setrespawn 0x19
	end

@ Johto_VioletPokecenter1F object 6 (local id 6, gfx 0x3EB4) at 9,4
Johto_VioletPokecenter1F_EventScript_83FFD24::
	@ Moemon Kai: Crystal's ELM's aide (pokecrystal maps/VioletPokecenter1F.asm; audit case 59): the party must have room
	@ (not the PC box), and a second visit after a NO asks again
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x1080, Johto_VioletPokecenter1F_EventScript_83FFDD8
	goto_if_set FLAG_JOHTO_REFUSED_TOGEPI_EGG, Johto_VioletPokecenter1F_EventScript_SecondTimeAsking
	msgbox Johto_VioletPokecenter1F_Text_83FFECC, MSGBOX_YESNO
	goto Johto_VioletPokecenter1F_EventScript_AnsweredEgg

Johto_VioletPokecenter1F_EventScript_SecondTimeAsking::
	msgbox Johto_VioletPokecenter1F_Text_AskEgg, MSGBOX_YESNO
Johto_VioletPokecenter1F_EventScript_AnsweredEgg::
	goto_if_eq VAR_RESULT, NO, Johto_VioletPokecenter1F_EventScript_83FFD78
	getpartysize
	goto_if_eq VAR_RESULT, PARTY_SIZE, Johto_VioletPokecenter1F_EventScript_83FFD6D
	goto Johto_VioletPokecenter1F_EventScript_83FFD98

Johto_VioletPokecenter1F_EventScript_83FFD6D::
	msgbox Johto_VioletPokecenter1F_Text_83FFF3F, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_VioletPokecenter1F_EventScript_83FFD78::
	msgbox Johto_VioletPokecenter1F_Text_84000F1, MSGBOX_DEFAULT
	setflag FLAG_JOHTO_REFUSED_TOGEPI_EGG
	release
	end

Johto_VioletPokecenter1F_EventScript_83FFD98::
	giveegg SPECIES_TOGEPI
	goto_if_eq VAR_RESULT, 2, Johto_VioletPokecenter1F_EventScript_83FFD6D
	setflag FLAG_QZ_0x1080
	playfanfare MUS_OBTAIN_ITEM
	message Johto_VioletPokecenter1F_Text_82CF03D
	waitfanfare
	msgbox Johto_VioletPokecenter1F_Text_83FFFE9, MSGBOX_DEFAULT
	closemessage
	call Johto_VioletPokecenter1F_EventScript_83FFDFA
	setflag FLAG_QZ_0x1081
	release
	end

Johto_VioletPokecenter1F_EventScript_83FFDD8::
	call Johto_VioletPokecenter1F_EventScript_83FFDFA
	setflag FLAG_QZ_0x1081
	msgbox Johto_VioletPokecenter1F_Text_83FFFE9, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_VioletPokecenter1F_EventScript_83FFDEB::
	msgbox Johto_VioletPokecenter1F_Text_83FFE11, MSGBOX_DEFAULT
	call Johto_VioletPokecenter1F_EventScript_83FFDFA
	release
	end

Johto_VioletPokecenter1F_EventScript_83FFDFA::
	goto_if_ne VAR_QZ_0x5012, 0, Johto_VioletPokecenter1F_EventScript_83FFE10
	setvar VAR_QZ_0x5012, 1
	clearflag FLAG_QZ_0x108B
	clearflag FLAG_QZ_0x106A

Johto_VioletPokecenter1F_EventScript_83FFE10::
	return

@ Johto_VioletPokecenter1F object 3 (local id 3, gfx 0x30) at 12,5
Johto_VioletPokecenter1F_EventScript_83FFE98::
	lock
	faceplayer
	msgbox Johto_VioletPokecenter1F_Text_84001F8, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_VioletPokecenter1F object 5 (local id 5, gfx 0x3E96) at 12,2
Johto_VioletPokecenter1F_EventScript_83FFEA5::
	lock
	faceplayer
	msgbox Johto_VioletPokecenter1F_Text_84001C3, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_VioletPokecenter1F object 4 (local id 4, gfx 0x3EBF) at 3,6
Johto_VioletPokecenter1F_EventScript_83FFEB2::
	lock
	faceplayer
	msgbox Johto_VioletPokecenter1F_Text_8400274, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_VioletPokecenter1F object 7 (local id 7, gfx 0x2D) at 2,2: Quetzal's reader, gone (FLAG_KAI_GONE; case 59)
Johto_VioletPokecenter1F_EventScript_83FFEBF::
	end

Johto_VioletPokecenter1F_Text_82CF03D::
	.string "{PLAYER} received the EGG.$"

Johto_VioletPokecenter1F_Text_83FFE11::
	.string "I’ll keep the EGG until you finish\n"
	.string "this region’s NUZLOCKE challenge.\p"
	.string "You can continue your journey.\n"
	.string "Come back for it when you’re done!$"

Johto_VioletPokecenter1F_Text_83FFECC::
	.string "{PLAYER}, long time, no see.\p"
	.string "PROF.ELM asked me to find you.\p"
	.string "He has another favor to ask.\p"
	.string "Would you take the MOéMON EGG?$"

Johto_VioletPokecenter1F_Text_83FFF3F::
	.string "Oh, no. You can’t carry any more\n"
	.string "MOéMON with you.\p"
	.string "I’ll wait here while you make room\n"
	.string "for the EGG.$"

Johto_VioletPokecenter1F_Text_83FFFE9::
	.string "We discovered that a MOéMON will\n"
	.string "not hatch until it grows in the\l"
	.string "EGG.\p"
	.string "It also has to be with other\n"
	.string "active MOéMON to hatch.\p"
	.string "{PLAYER}, you’re the only person\n"
	.string "we can rely on.\p"
	.string "Please call PROF. ELM when that\n"
	.string "EGG hatches!$"

Johto_VioletPokecenter1F_Text_84000F1::
	.string "B-but… PROF.ELM asked for you…$"

Johto_VioletPokecenter1F_Text_84001C3::
	.string "A guy named BILL made the\n"
	.string "MOéMON PC storage system.$"

Johto_VioletPokecenter1F_Text_84001F8::
	.string "It was around three years ago.\p"
	.string "TEAM ROCKET was up to no good\n"
	.string "with MOéMON.\p"
	.string "But justice prevailed--a young\n"
	.string "kid broke ’em up.$"

Johto_VioletPokecenter1F_Text_8400274::
	.string "MOéMON are smart. They won’t\n"
	.string "obey a trainer they don’t respect.\p"
	.string "Without the right GYM BADGES,\n"
	.string "they will just do as they please.$"

Johto_VioletPokecenter1F_Text_AskEgg::
	.string "{PLAYER}, will you take the EGG?$"
