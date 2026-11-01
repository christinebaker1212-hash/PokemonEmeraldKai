Sinnoh_SandgemTownPokecenter1f_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_SandgemTownPokecenter1f_OnTransition
	.byte 0

Sinnoh_SandgemTownPokecenter1f_OnTransition:
	setrespawn HEAL_LOCATION_SINNOH_SANDGEM_TOWN
	end

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_SandgemTownPokecenter1f_EventScript_Person1::
	msgbox Sinnoh_SandgemTownPokecenter1f_Text_Person1, MSGBOX_NPC
	end

Sinnoh_SandgemTownPokecenter1f_EventScript_Person2::
	msgbox Sinnoh_SandgemTownPokecenter1f_Text_Person2, MSGBOX_NPC
	end

Sinnoh_SandgemTownPokecenter1f_EventScript_Person3::
	msgbox Sinnoh_SandgemTownPokecenter1f_Text_Person3, MSGBOX_NPC
	end

Sinnoh_SandgemTownPokecenter1f_EventScript_Person4::
	setvar VAR_0x800B, 4
	call Common_EventScript_PkmnCenterNurse
	waitmessage
	waitbuttonpress
	release
	end

Sinnoh_SandgemTownPokecenter1f_Text_Person1:
	.string "A MOéMON CENTER heals your team for\n"
	.string "free.\p"
	.string "Come back whenever your MOéMON look\n"
	.string "tired. They'd do the same for you.$"

Sinnoh_SandgemTownPokecenter1f_Text_Person2:
	.string "I've traveled every region I can reach\n"
	.string "by boat.\p"
	.string "SINNOH has the best sunsets. Don't\n"
	.string "tell HOENN I said that.$"

Sinnoh_SandgemTownPokecenter1f_Text_Person3:
	.string "The PC here keeps your MOéMON safe in\n"
	.string "boxes.\p"
	.string "I keep my favorites at the front so\n"
	.string "they don't feel lonely.$"

@ --- end of SINNOH words ---
