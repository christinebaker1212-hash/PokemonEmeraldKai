Johto_IcePath1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_IcePath1F_EventScript_83DB627
	.byte 0

Johto_IcePath1F_EventScript_83DB627::
	@ Moemon Kai: no KIMONO GIRL stuck on the ice (HGSS's SAYO, Crystal has none: audit case 116)
	end

@ Johto_IcePath1F object 1 (local id 1, gfx 0x3B) at 48,14
Johto_IcePath1F_EventScript_83DB628::
	setvar VAR_0x8004, ITEM_HM07   @ Moemon Kai: Crystal's HM07 (Quetzal's NUGGET)
	goto EventScript_FindHmOnce
	end

@ Johto_IcePath1F object 2 (local id 2, gfx 0x3B) at 47,29
Johto_IcePath1F_EventScript_83DB635::
	setorcopyvar VAR_0x8000, ITEM_PROTEIN
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

