Johto_CianwoodPokecenter1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_CianwoodPokecenter1F_EventScript_83CB5C0
	.byte 0

Johto_CianwoodPokecenter1F_EventScript_83CB5C0::
	setrespawn 0x1E
	end

@ Johto_CianwoodPokecenter1F object 6 (local id 6, gfx 0x3E95) at 14,6
Johto_CianwoodPokecenter1F_EventScript_83CB5C4::
	lock
	faceplayer
	msgbox Johto_CianwoodPokecenter1F_Text_83CB5FC, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_CianwoodPokecenter1F object 5 (local id 5, gfx 0x2F) at 3,6
Johto_CianwoodPokecenter1F_EventScript_83CB68E::
	lock
	faceplayer
	msgbox Johto_CianwoodPokecenter1F_Text_83CB69B, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_CianwoodPokecenter1F object 4 (local id 4, gfx 0x3E98) at 9,4
Johto_CianwoodPokecenter1F_EventScript_83CB6E5::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x11AC, Johto_CianwoodPokecenter1F_EventScript_83CB6FB
	msgbox Johto_CianwoodPokecenter1F_Text_83CB706, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_CianwoodPokecenter1F_EventScript_83CB6FB::
	msgbox Johto_CianwoodPokecenter1F_Text_83CB89F, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_CianwoodPokecenter1F object 3 (local id 3, gfx 0x3EB7) at 12,5
Johto_CianwoodPokecenter1F_EventScript_83CB8CC::
	lock
	faceplayer
	msgbox Johto_CianwoodPokecenter1F_Text_83CB8D9, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_CianwoodPokecenter1F_Text_83CB5FC::
	.string "As far as water POKéMON go,\n"
	.string "TENTACOOL is the best by a\l"
	.string "nautical mile!$"

Johto_CianwoodPokecenter1F_Text_83CB69B::
	.string "Did you meet the POKéMANIAC?\p"
	.string "He’s always bragging about his\n"
	.string "rare POKéMON.$"

Johto_CianwoodPokecenter1F_Text_83CB706::
	.string "The POKéMON GYM trainers here are\n"
	.string "macho bullies.\p"
	.string "If I stick around, they might\n"
	.string "come after me.\p"
	.string "Here’s some advice: the GYM LEADER\n"
	.string "uses the FIGHTING-type.\p"
	.string "So you should confound him with\n"
	.string "psychic POKéMON.\p"
	.string "Wipe out his POKéMON before they\n"
	.string "can use their physical strength.\p"
	.string "And those boulders in the middle\n"
	.string "of the GYM?\p"
	.string "If you don’t move them correctly,\n"
	.string "you won’t reach the GYM LEADER.\p"
	.string "If you get stuck, go outside.$"

Johto_CianwoodPokecenter1F_Text_83CB89F::
	.string "{PLAYER}! You won!\n"
	.string "I could tell by looking at you!$"

Johto_CianwoodPokecenter1F_Text_83CB8D9::
	.string "I love showing off the POKéMON\n"
	.string "that I’ve raised.\p"
	.string "Don’t you?\p"
	.string "I’m going to get into a bunch of\n"
	.string "battles, and show off my POKéMON!$"
