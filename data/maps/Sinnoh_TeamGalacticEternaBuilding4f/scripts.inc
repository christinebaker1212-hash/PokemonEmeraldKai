Sinnoh_TeamGalacticEternaBuilding4f_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_TeamGalacticEternaBuilding4f_OnTransition
	.byte 0

Sinnoh_TeamGalacticEternaBuilding4f_OnTransition:
	special Sinnoh_LegendsRefresh
	end

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_TeamGalacticEternaBuilding4f_EventScript_Bg1::
	goto SinnohP_team_galactic_eterna_building_4f_6
	end

Sinnoh_TeamGalacticEternaBuilding4f_EventScript_Person2::
	msgbox Sinnoh_TeamGalacticEternaBuilding4f_Text_Person2, MSGBOX_NPC
	end

Sinnoh_TeamGalacticEternaBuilding4f_EventScript_Person3::
	msgbox Sinnoh_TeamGalacticEternaBuilding4f_Text_Person3, MSGBOX_NPC
	end

Sinnoh_TeamGalacticEternaBuilding4f_EventScript_Person4::
	msgbox Sinnoh_TeamGalacticEternaBuilding4f_Text_Person4, MSGBOX_NPC
	end

Sinnoh_TeamGalacticEternaBuilding4f_Text_Person2:
	.string "Clefairy looks frightened. It huddles\n"
	.string "close to the old man.$"

Sinnoh_TeamGalacticEternaBuilding4f_Text_Person3:
	.string "They took my CLEFAIRY to study it!\n"
	.string "Please, help us get out of here!$"

Sinnoh_TeamGalacticEternaBuilding4f_Text_Person4:
	.string "BUNEARY is shivering in the corner.$"

@ --- end of SINNOH words ---
