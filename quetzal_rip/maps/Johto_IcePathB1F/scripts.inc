Johto_IcePathB1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_IcePathB1F_EventScript_83DB99C
	map_script MAP_SCRIPT_ON_RESUME, Johto_IcePathB1F_EventScript_83DB9A2
	.byte 0

Johto_IcePathB1F_EventScript_83DB99C::
	call Quetzal_EventScript_840EA62
	end

Johto_IcePathB1F_EventScript_83DB9A2::
	setstepcallback 0
	end

@ Johto_IcePathB1F object 1 (local id 1, gfx 0x3A9E) at 23,32
Johto_IcePathB1F_EventScript_83DB9A5::
	setorcopyvar VAR_0x8000, 0x43
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_IcePathB1F object 2 (local id 2, gfx 0x3A9E) at 11,42
Johto_IcePathB1F_EventScript_83DB9B2::
	setorcopyvar VAR_0x8000, 0x21
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_IcePathB1F sign / bg event at 6,13
Johto_IcePathB1F_EventScript_83DB9BF::
	msgbox Johto_IcePathB1F_Text_83DB9C8, MSGBOX_SIGN
	end

Johto_IcePathB1F_Text_83DB9C8::
	.string "DANGER: SLIPPERY ICE\n"
	.string "PROCEED WITH CAUTION.\l"
	.string "The sign is faded...$"
