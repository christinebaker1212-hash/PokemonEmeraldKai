Johto_Route32Pokecenter1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_Route32Pokecenter1F_EventScript_83EF894
	.byte 0

Johto_Route32Pokecenter1F_EventScript_83EF894::
	setrespawn 0x25
	end

@ Johto_Route32Pokecenter1F object 4 (local id 4, gfx 0x2F) at 11,5
Johto_Route32Pokecenter1F_EventScript_83EF898::
	lock
	faceplayer
	msgbox Johto_Route32Pokecenter1F_Text_83EFA31, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_Route32Pokecenter1F object 3 (local id 3, gfx 0x3E95) at 2,5
Johto_Route32Pokecenter1F_EventScript_83EF8A5::
	lock
	faceplayer
	checkitem ITEM_OLD_ROD, 1
	goto_if_eq VAR_RESULT, 1, Johto_Route32Pokecenter1F_EventScript_83EF8FF
	msgbox Johto_Route32Pokecenter1F_Text_83EF909, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Johto_Route32Pokecenter1F_EventScript_83EF8D5
	msgbox Johto_Route32Pokecenter1F_Text_83EF9F2, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_Route32Pokecenter1F_EventScript_83EF8D5::
	msgbox Johto_Route32Pokecenter1F_Text_83EF96D, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, 0x2C5
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_83602AA
	msgbox Johto_Route32Pokecenter1F_Text_83EF9A1, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_Route32Pokecenter1F_EventScript_83EF8FF::
	msgbox Johto_Route32Pokecenter1F_Text_83EFA13, MSGBOX_DEFAULT
	release
	end

Johto_Route32Pokecenter1F_Text_83EF909::
	.string "This is a great fishing spot.\p"
	.string "You saw people fishing?\n"
	.string "How about you?\p"
	.string "Would you like one of my RODS?$"

Johto_Route32Pokecenter1F_Text_83EF96D::
	.string "Heh, that’s good to hear.\p"
	.string "Now you’re an angler too!$"

Johto_Route32Pokecenter1F_Text_83EF9A1::
	.string "Fishing is great!\p"
	.string "If there’s water, be it the sea or\n"
	.string "a stream, try out your ROD.$"

Johto_Route32Pokecenter1F_Text_83EF9F2::
	.string "Oh. That’s rather disappointing…$"

Johto_Route32Pokecenter1F_Text_83EFA13::
	.string "Yo, kid. How are they biting?$"

Johto_Route32Pokecenter1F_Text_83EFA31::
	.string "What should I make my POKéMON hold?\p"
	.string "Maybe an item that increases\n"
	.string "ATTACK power…$"
