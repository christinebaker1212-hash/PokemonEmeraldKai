Sinnoh_OreburghCityPokecenter1f_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_OreburghCityPokecenter1f_OnTransition
	.byte 0

Sinnoh_OreburghCityPokecenter1f_OnTransition:
	setrespawn HEAL_LOCATION_SINNOH_OREBURGH_CITY
	end

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_OreburghCityPokecenter1f_EventScript_Person1::
	msgbox Sinnoh_OreburghCityPokecenter1f_Text_Person1, MSGBOX_NPC
	end

Sinnoh_OreburghCityPokecenter1f_EventScript_Person2::
	msgbox Sinnoh_OreburghCityPokecenter1f_Text_Person2, MSGBOX_NPC
	end

Sinnoh_OreburghCityPokecenter1f_EventScript_Person3::
	msgbox Sinnoh_OreburghCityPokecenter1f_Text_Person3, MSGBOX_NPC
	end

Sinnoh_OreburghCityPokecenter1f_EventScript_Person4::
	setvar VAR_0x800B, 4
	call Common_EventScript_PkmnCenterNurse
	waitmessage
	waitbuttonpress
	release
	end

Sinnoh_OreburghCityPokecenter1f_EventScript_Person5::
	msgbox Sinnoh_OreburghCityPokecenter1f_Text_Person5, MSGBOX_NPC
	end

Sinnoh_OreburghCityPokecenter1f_EventScript_Person8::
	msgbox Sinnoh_OreburghCityPokecenter1f_Text_Person8, MSGBOX_NPC
	end

Sinnoh_OreburghCityPokecenter1f_EventScript_Person9::
	msgbox Sinnoh_OreburghCityPokecenter1f_Text_Person9, MSGBOX_NPC
	end

Sinnoh_OreburghCityPokecenter1f_Text_Person1:
	.string "I can sense... that your MOéMON are\n"
	.string "tired. That's why you're here, right?$"

Sinnoh_OreburghCityPokecenter1f_Text_Person2:
	.string "ROARK uses ROCK MOéMON. My GRASS\n"
	.string "MOéMON had a great time in that\l"
	.string "battle!$"

Sinnoh_OreburghCityPokecenter1f_Text_Person3:
	.string "I'm waiting for my friend to finish in\n"
	.string "the mine. He's been down there all day.$"

Sinnoh_OreburghCityPokecenter1f_Text_Person5:
	.string "I've visited this MOéMON CENTER for\n"
	.string "forty years. The chairs have never\l"
	.string "been comfier.$"

Sinnoh_OreburghCityPokecenter1f_Text_Person8:
	.string "Are you here to trade? I've got a\n"
	.string "GEODUDE, and I'm not afraid to use it!$"

Sinnoh_OreburghCityPokecenter1f_Text_Person9:
	.string "We're linking up to battle! Our MOéMON\n"
	.string "are fighting in a tiny screen world.$"

@ --- end of SINNOH words ---
