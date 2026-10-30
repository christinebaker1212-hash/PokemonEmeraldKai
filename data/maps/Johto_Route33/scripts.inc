Johto_Route33_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_Route33_EventScript_83EFA86
	.byte 0

@ Johto_Route33 trigger at 0,23 (VAR_QZ_0x5009 == 0)
@ Johto_Route33 trigger at 0,22 (VAR_QZ_0x5009 == 0)
@ Johto_Route33 trigger at 0,24 (VAR_QZ_0x5009 == 0)
@ Johto_Route33 trigger at 0,25 (VAR_QZ_0x5009 == 0)
Johto_Route33_EventScript_83C3B76::
	setflag FLAG_QZ_0x11BD
	setvar VAR_QZ_0x5009, 1
	end

Johto_Route33_EventScript_83EFA86::
	call Quetzal_EventScript_840EA62
	goto_if_unset FLAG_QZ_0x12B3, Johto_Route33_EventScript_83EFA98
	setweather 3
	end

Johto_Route33_EventScript_83EFA98::
	setweather 5
	end

@ Johto_Route33 object 1 (local id 1, gfx 0x37) at 17,20
Johto_Route33_EventScript_83EFA9C::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4448, 0, Johto_Route33_Text_83EFAC0, Johto_Route33_Text_83EFB03
	msgbox Johto_Route33_Text_83EFB26, MSGBOX_AUTOCLOSE
	end

@ Johto_Route33 object 2 (local id 2, gfx 0x10) at 25,27
Johto_Route33_EventScript_83EFAB3::
	lock
	faceplayer
	msgbox Johto_Route33_Text_83EFBB9, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_Route33_Text_83EFAC0::
	.string "I came through the tunnel,\n"
	.string "but I still have plenty\l"
	.string "of energy left.$"

Johto_Route33_Text_83EFB03::
	.string "Whoa! You’ve got more zip\n"
	.string "than me!$"

Johto_Route33_Text_83EFB26::
	.string "We HIKERS are at our best\n"
	.string "in the mountains.$"

Johto_Route33_Text_83EFBB9::
	.string "Pant, pant…\p"
	.string "That cave was bigger than\n"
	.string "I’d expected.\p"
	.string "I got too tired exploring\n"
	.string "for MARILL, so I came outside.$"
