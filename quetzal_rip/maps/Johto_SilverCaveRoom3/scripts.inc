Johto_SilverCaveRoom3_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_SilverCaveRoom3_EventScript_83E034B
	map_script MAP_SCRIPT_ON_RETURN_TO_FIELD, Johto_SilverCaveRoom3_EventScript_83E03A8
	.byte 0

Johto_SilverCaveRoom3_EventScript_83E034B::
	call Quetzal_EventScript_840EA62
	setvar VAR_QZ_0x5070, 0
	setvar VAR_QZ_0x5071, 0
	setobjectxyperm 1, 0x1E, 0xE
	setobjectxyperm 2, 0x10, 0xE
	goto_if_set FLAG_QZ_0x1218, Johto_SilverCaveRoom3_EventScript_83E0399
	goto_if_set FLAG_QZ_0x1219, Johto_SilverCaveRoom3_EventScript_83E0399
	goto_if_set FLAG_QZ_0x1424, Johto_SilverCaveRoom3_EventScript_83E038E
	setflag FLAG_QZ_0x1217
	clearflag FLAG_QZ_0x1423
	goto Johto_SilverCaveRoom3_EventScript_83E039F

Johto_SilverCaveRoom3_EventScript_83E038E::
	setflag FLAG_QZ_0x1423
	clearflag FLAG_QZ_0x1217
	goto Johto_SilverCaveRoom3_EventScript_83E039F

Johto_SilverCaveRoom3_EventScript_83E0399::
	setflag FLAG_QZ_0x1423
	setflag FLAG_QZ_0x1217

Johto_SilverCaveRoom3_EventScript_83E039F::
	setescapewarp MAP_JOHTO_SILVER_CAVE_OUTSIDE, 255, 17, 8
	end

Johto_SilverCaveRoom3_EventScript_83E03A8::
	special QzSetDeoxysRockPaletteVar5071_492
	end

@ Johto_SilverCaveRoom3 object 1 (local id 1, gfx 0xE9) at 30,14
Johto_SilverCaveRoom3_EventScript_83E03AC::
	lock
	faceplayer
	special QzDoDeoxysRockInteractionAlt_491
	waitstate
	copyvar VAR_0x8000, VAR_RESULT
	goto_if_eq VAR_0x8000, 0, Johto_SilverCaveRoom3_EventScript_83E03E4
	goto_if_eq VAR_0x8000, 1, Johto_SilverCaveRoom3_EventScript_83E03E4
	goto_if_eq VAR_0x8000, 2, Johto_SilverCaveRoom3_EventScript_83E03E6
	goto_if_eq VAR_0x8000, 3, Johto_SilverCaveRoom3_EventScript_83E03E4
	end

Johto_SilverCaveRoom3_EventScript_83E03E4::
	release
	end

Johto_SilverCaveRoom3_EventScript_83E03E6::
	waitse
	setfieldeffectargument 0, 1
	setfieldeffectargument 1, 0x72
	setfieldeffectargument 2, 0x22
	dofieldeffect 0x41
	playbgm MUS_RG_ENCOUNTER_DEOXYS, 0
	waitfieldeffect 0x41
	setflag FLAG_QZ_0x1423
	clearflag FLAG_QZ_0x1217
	addobject 2
	delay 0x1E
	setvar VAR_LAST_TALKED, 2
	release
	goto Quetzal_EventScript_840EA6A
