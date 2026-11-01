Sinnoh_TurnbackCaveGiratinaRoom_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_TurnbackCaveGiratinaRoom_OnTransition
	.byte 0

Sinnoh_TurnbackCaveGiratinaRoom_OnTransition:
	special Sinnoh_LegendsRefresh
	end

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_TurnbackCaveGiratinaRoom_EventScript_Bg1::
	msgbox Sinnoh_TurnbackCaveGiratinaRoom_Text_Bg1, MSGBOX_SIGN
	end

Sinnoh_TurnbackCaveGiratinaRoom_EventScript_Bg2::
	msgbox Sinnoh_TurnbackCaveGiratinaRoom_Text_Bg2, MSGBOX_SIGN
	end

Sinnoh_TurnbackCaveGiratinaRoom_EventScript_Bg3::
	msgbox Sinnoh_TurnbackCaveGiratinaRoom_Text_Bg3, MSGBOX_SIGN
	end

Sinnoh_TurnbackCaveGiratinaRoom_EventScript_Bg4::
	msgbox Sinnoh_TurnbackCaveGiratinaRoom_Text_Bg4, MSGBOX_SIGN
	end

Sinnoh_TurnbackCaveGiratinaRoom_Text_Bg1:
	.string "Old letters are carved into the stone.\n"
	.string "They seem to spell out a word, but its\l"
	.string "meaning has been lost.$"

Sinnoh_TurnbackCaveGiratinaRoom_Text_Bg2:
	.string "Old letters are carved into the stone.\n"
	.string "They seem to spell out a word, but its\l"
	.string "meaning has been lost.$"

Sinnoh_TurnbackCaveGiratinaRoom_Text_Bg3:
	.string "Old letters are carved into the stone.\n"
	.string "They seem to spell out a word, but its\l"
	.string "meaning has been lost.$"

Sinnoh_TurnbackCaveGiratinaRoom_Text_Bg4:
	.string "A strange glowing panel in the floor.$"

@ --- end of SINNOH words ---
