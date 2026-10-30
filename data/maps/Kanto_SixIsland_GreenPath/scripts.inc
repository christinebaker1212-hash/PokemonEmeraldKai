Kanto_SixIsland_GreenPath_MapScripts::
	.byte 0

@ Kanto_SixIsland_GreenPath sign / bg event at 69,10
Kanto_SixIsland_GreenPath_EventScript_84188E0::
	msgbox Kanto_SixIsland_GreenPath_Text_843BB36, MSGBOX_SIGN
	end

@ Kanto_SixIsland_GreenPath sign / bg event at 40,10
Kanto_SixIsland_GreenPath_EventScript_84188E9::
	msgbox Kanto_SixIsland_GreenPath_Text_843BB54, MSGBOX_SIGN
	end

@ Kanto_SixIsland_GreenPath object 1 (local id 1, gfx 0x3F57) at 11,8
Kanto_SixIsland_GreenPath_EventScript_845C8E8::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8709, 0, Kanto_SixIsland_GreenPath_Text_843BA99, Kanto_SixIsland_GreenPath_Text_843BAD9
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_SixIsland_GreenPath_EventScript_845C90F
	msgbox Kanto_SixIsland_GreenPath_Text_843BAE6, MSGBOX_AUTOCLOSE
	end

Kanto_SixIsland_GreenPath_EventScript_845C90F::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8709, 0, Kanto_SixIsland_GreenPath_Text_847148F, Kanto_SixIsland_GreenPath_Text_843BAD9
	msgbox Kanto_SixIsland_GreenPath_Text_843BAE6, MSGBOX_AUTOCLOSE
	end

Kanto_SixIsland_GreenPath_Text_843BA99::
	.string "…Huh?\p"
	.string "I was envisioning my house, but\n"
	.string "this is where I TELEPORT?$"

Kanto_SixIsland_GreenPath_Text_843BAD9::
	.string "Big boo-boo!$"

Kanto_SixIsland_GreenPath_Text_843BAE6::
	.string "I am so scatterbrained!\p"
	.string "I need to focus so I can TELEPORT\n"
	.string "properly to my house!$"

Kanto_SixIsland_GreenPath_Text_843BB36::
	.string "GREEN PATH\n"
	.string "PATTERN BUSH AHEAD$"

Kanto_SixIsland_GreenPath_Text_843BB54::
	.string "GREEN PATH\n"
	.string "Think Green, Keep Nature Clean!$"

Kanto_SixIsland_GreenPath_Text_847148F::
	.string "…Huh?\p"
	.string "I envision my house, but TELEPORT\n"
	.string "always brings me here!$"
