Johto_MahoganyPokecenter1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_MahoganyPokecenter1F_EventScript_83DF143
	.byte 0

Johto_MahoganyPokecenter1F_EventScript_83DF143::
	setrespawn 0x20
	end

@ Johto_MahoganyPokecenter1F object 3 (local id 3, gfx 0x37) at 13,3
Johto_MahoganyPokecenter1F_EventScript_83DF147::
	msgbox Johto_MahoganyPokecenter1F_Text_83DF162, MSGBOX_NPC
	end

@ Johto_MahoganyPokecenter1F object 4 (local id 4, gfx 0x3E87) at 4,4
Johto_MahoganyPokecenter1F_EventScript_83DF150::
	msgbox Johto_MahoganyPokecenter1F_Text_83DF1B3, MSGBOX_NPC
	end

@ Johto_MahoganyPokecenter1F object 5 (local id 5, gfx 0x14) at 5,4
Johto_MahoganyPokecenter1F_EventScript_83DF159::
	msgbox Johto_MahoganyPokecenter1F_Text_83DF218, MSGBOX_NPC
	end

Johto_MahoganyPokecenter1F_Text_83DF162::
	.string "What’s this? TEAM ROCKET has come\n"
	.string "back?\p"
	.string "I saw some men in black at LAKE OF\n"
	.string "RAGE…$"

Johto_MahoganyPokecenter1F_Text_83DF1B3::
	.string "I stop my POKéMON from evolving too\n"
	.string "early.\p"
	.string "I make them learn certain moves\n"
	.string "before I let them evolve.$"

Johto_MahoganyPokecenter1F_Text_83DF218::
	.string "POKéMON do become stronger when they\n"
	.string "evolve,\p"
	.string "but they also learn moves more\n"
	.string "slowly.$"
