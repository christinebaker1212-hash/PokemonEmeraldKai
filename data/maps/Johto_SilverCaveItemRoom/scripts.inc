Johto_SilverCaveItemRoom_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840EA66
	.byte 0

@ Johto_SilverCaveItemRoom object 1 (local id 1, gfx 0x3B) at 10,7
Johto_SilverCaveItemRoom_EventScript_83E032C::
	setorcopyvar VAR_0x8000, ITEM_SPELL_TAG
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_SilverCaveItemRoom object 2 (local id 2, gfx 0x61) at 8,8
Johto_SilverCaveItemRoom_EventScript_840EA8C::
	lock
	faceplayer
	msgbox Quetzal_Text_840F0C0, MSGBOX_DEFAULT
	special QzJohtoStaticEncounterSetup
	goto_if_eq VAR_RESULT, 0, Johto_SilverCaveItemRoom_EventScript_840EAA9
	call Quetzal_EventScript_840EAAB

Johto_SilverCaveItemRoom_EventScript_840EAA9::
	release
	end
