Johto_SilverCaveOutside_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_SilverCaveOutside_EventScript_83E0426
	.byte 0

Johto_SilverCaveOutside_EventScript_83E0426::
	goto_if_set FLAG_QZ_0x1318, Johto_SilverCaveOutside_EventScript_83E043B
	goto_if_set FLAG_QZ_0x1425, Johto_SilverCaveOutside_EventScript_83E043B
	setflag FLAG_QZ_0x11C7

Johto_SilverCaveOutside_EventScript_83E043B::
	end

@ Johto_SilverCaveOutside sign / bg event at 15,9
Johto_SilverCaveOutside_EventScript_83E043C::
	msgbox Johto_SilverCaveOutside_Text_83E0445, MSGBOX_SIGN
	end

Johto_SilverCaveOutside_Text_83E0445::
	.string "MT. SILVER$"
