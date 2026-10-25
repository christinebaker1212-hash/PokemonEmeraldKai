Kanto_ThreeIsland_House1_MapScripts::
	.byte 0

@ Kanto_ThreeIsland_House1 object 1 (local id 1, gfx 0x3F18) at 3,4
Kanto_ThreeIsland_House1_EventScript_842157E::
	lock
	faceplayer
	special GetPlayerBigGuyGirlString
	msgbox Kanto_ThreeIsland_House1_Text_84534D6, MSGBOX_DEFAULT
	release
	end

@ Kanto_ThreeIsland_House1 sign / bg event at 2,1
Kanto_ThreeIsland_House1_EventScript_842158D::
	msgbox Kanto_ThreeIsland_House1_Text_845352A, MSGBOX_SIGN
	end

Kanto_ThreeIsland_House1_Text_84534D6::
	.string "LOSTELLE: Hi, {PLAYER}!\n"
	.string "You’re always so cool!\p"
	.string "I’ll make lunch for you, too, one\n"
	.string "day, {PLAYER}.$"

Kanto_ThreeIsland_House1_Text_845352A::
	.string "Rare rocks and gems are displayed.$"
