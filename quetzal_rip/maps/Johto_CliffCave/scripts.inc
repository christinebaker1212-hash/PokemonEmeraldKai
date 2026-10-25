Johto_CliffCave_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840EA66
	.byte 0

@ Johto_CliffCave object 1 (local id 1, gfx 0x3E84) at 34,8
Johto_CliffCave_EventScript_83CBAA1::
	lock
	faceplayer
	msgbox Johto_CliffCave_Text_83CBAAE, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_CliffCave_Text_83CBAAE::
	.string "If you see a fork in the road\n"
	.string "inside a cave, most likely one\l"
	.string "way is right and the other leads\l"
	.string "to a dead end.\p"
	.string "but people can’t help trying both.$"
