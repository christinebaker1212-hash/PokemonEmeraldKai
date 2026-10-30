Johto_GoldenrodPokecenter1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_GoldenrodPokecenter1F_EventScript_83D6887
	.byte 0

Johto_GoldenrodPokecenter1F_EventScript_83D6887::
	setrespawn 0x1B
	end

@ Johto_GoldenrodPokecenter1F object 4 (local id 4, gfx 0x3E96) at 2,5
Johto_GoldenrodPokecenter1F_EventScript_83D688B::
	lock
	faceplayer
	msgbox Johto_GoldenrodPokecenter1F_Text_83D6916, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodPokecenter1F object 6 (local id 6, gfx 0x3E96) at 1,5
Johto_GoldenrodPokecenter1F_EventScript_83D6898::
	lock
	faceplayer
	msgbox Johto_GoldenrodPokecenter1F_Text_83D6952, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodPokecenter1F object 5 (local id 5, gfx 0x1E) at 14,6
Johto_GoldenrodPokecenter1F_EventScript_83D68A5::
	lock
	faceplayer
	msgbox Johto_GoldenrodPokecenter1F_Text_83D68BF, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodPokecenter1F object 3 (local id 3, gfx 0x2F) at 11,5
Johto_GoldenrodPokecenter1F_EventScript_83D68B2::
	lock
	faceplayer
	msgbox Johto_GoldenrodPokecenter1F_Text_83D69A0, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_GoldenrodPokecenter1F_Text_83D68BF::
	.string "I just love living in the big city!\n"
	.string "I moved here when I was a young\l"
	.string "trainer, like you!$"

Johto_GoldenrodPokecenter1F_Text_83D6916::
	.string "This is a special POKéMON CENTER.\n"
	.string "The roof is made of gold!$"

Johto_GoldenrodPokecenter1F_Text_83D6952::
	.string "I told my brother that the roof is\n"
	.string "made of real gold. He’ll believe\l"
	.string "anything!$"

Johto_GoldenrodPokecenter1F_Text_83D69A0::
	.string "A higher level POKéMON doesn’t\n"
	.string "always win.\p"
	.string "After all, it may have a type\n"
	.string "disadvantage.\p"
	.string "I don’t think there is a single\n"
	.string "POKéMON that is the toughest.$"
