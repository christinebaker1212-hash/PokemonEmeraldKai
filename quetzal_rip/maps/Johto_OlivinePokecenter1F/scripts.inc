Johto_OlivinePokecenter1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_OlivinePokecenter1F_EventScript_83E67FC
	.byte 0

Johto_OlivinePokecenter1F_EventScript_83E67FC::
	setrespawn 0x1D
	end

@ Johto_OlivinePokecenter1F object 3 (local id 3, gfx 0x11) at 2,6
Johto_OlivinePokecenter1F_EventScript_83E6800::
	lock
	faceplayer
	msgbox Johto_OlivinePokecenter1F_Text_83E681A, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_OlivinePokecenter1F object 4 (local id 4, gfx 0x2F) at 12,5
Johto_OlivinePokecenter1F_EventScript_83E680D::
	lock
	faceplayer
	msgbox Johto_OlivinePokecenter1F_Text_83E68AD, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_OlivinePokecenter1F_Text_83E681A::
	.string "The SAILOR in the OLIVINE CAFE\n"
	.string "next door is really generous.\p"
	.string "He taught my POKéMON STRENGTH.\p"
	.string "Now it can move big boulders\n"
	.string "without breaking a sweat.$"

Johto_OlivinePokecenter1F_Text_83E68AD::
	.string "There’s a person in CIANWOOD\n"
	.string "CITY across the sea.\p"
	.string "I heard him bragging about his\n"
	.string "rare POKéMON all the time.$"
