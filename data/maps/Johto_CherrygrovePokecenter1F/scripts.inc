Johto_CherrygrovePokecenter1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_CherrygrovePokecenter1F_EventScript_83C9A97
	.byte 0

Johto_CherrygrovePokecenter1F_EventScript_83C9A97::
	setrespawn 0x18
	end

@ Johto_CherrygrovePokecenter1F object 4 (local id 4, gfx 0x3E95) at 12,5
Johto_CherrygrovePokecenter1F_EventScript_83C9A9B::
	lock
	faceplayer
	msgbox Johto_CherrygrovePokecenter1F_Text_83C9AD8, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_CherrygrovePokecenter1F object 5 (local id 5, gfx 0x30) at 14,6
Johto_CherrygrovePokecenter1F_EventScript_83C9AA8::
	lock
	faceplayer
	msgbox Johto_CherrygrovePokecenter1F_Text_83C9B1A, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_CherrygrovePokecenter1F object 3 (local id 3, gfx 0x2F) at 2,7
Johto_CherrygrovePokecenter1F_EventScript_83C9AB5::
	lock
	faceplayer
	msgbox Johto_CherrygrovePokecenter1F_Text_83C9B42, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_CherrygrovePokecenter1F_Text_83C9AD8::
	.string "It’s great. I can store any number\n"
	.string "of POKéMON, and it’s all free.$"

Johto_CherrygrovePokecenter1F_Text_83C9B1A::
	.string "That PC is free for any\n"
	.string "trainer to use.$"

Johto_CherrygrovePokecenter1F_Text_83C9B42::
	.string "They say a watched pot never\n"
	.string "boils. But watching the clock\l"
	.string "in the POKéCENTER makes time fly!$"
