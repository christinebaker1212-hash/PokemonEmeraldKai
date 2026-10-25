Johto_BlackthornPokecenter1F_MapScripts::
	map_script MAP_SCRIPT_ON_RESUME, Quetzal_EventScript_8365E4C
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_BlackthornPokecenter1F_EventScript_83C8286
	.byte 0

Johto_BlackthornPokecenter1F_EventScript_83C8286::
	setrespawn 0x22
	end

@ Johto_BlackthornPokecenter1F object 3 (local id 3, gfx 0x30) at 12,5
Johto_BlackthornPokecenter1F_EventScript_83C828A::
	msgbox Johto_BlackthornPokecenter1F_Text_83C829C, MSGBOX_NPC
	end

@ Johto_BlackthornPokecenter1F object 4 (local id 4, gfx 0x6) at 2,6
Johto_BlackthornPokecenter1F_EventScript_83C8293::
	msgbox Johto_BlackthornPokecenter1F_Text_83C8315, MSGBOX_NPC
	end

Johto_BlackthornPokecenter1F_Text_83C829C::
	.string "Deep inside far-off INDIGO\n"
	.string "PLATEAU is the POKéMON LEAGUE.\p"
	.string "I hear the best trainers gather\n"
	.string "there from around the country.$"

Johto_BlackthornPokecenter1F_Text_83C8315::
	.string "There was this move I just had\n"
	.string "to teach my POKéMON.\p"
	.string "So I got the MOVE DELETER to make\n"
	.string "it forget an HM move.$"
