Johto_OlivineCityPier_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_83E3F77
	.byte 0

@ Johto_OlivineCityPier object 1 (local id 1, gfx 0x3E95) at 12,5
Johto_OlivineCityPier_EventScript_83E6F8B::
	lock
	faceplayer
	msgbox Johto_OlivineCityPier_Text_83E6FA5, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_OlivineCityPier object 2 (local id 2, gfx 0x3E95) at 21,6
Johto_OlivineCityPier_EventScript_83E6F98::
	lock
	faceplayer
	msgbox Johto_OlivineCityPier_Text_83E7022, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_OlivineCityPier_Text_83E6FA5::
	.string "From OLIVINE, you can visit\n"
	.string "VERMILION and LILYCOVE any day.\p"
	.string "You need an S.S. Ticket and must\n"
	.string "have won both regions’ Leagues.$"

Johto_OlivineCityPier_Text_83E7022::
	.string "Aw yeah, I’m fishin’.$"
