Kanto_ThreeIsland_House2_MapScripts::
	.byte 0

@ Kanto_ThreeIsland_House2 object 2 (local id 2, gfx 0x3F1E) at 7,4
Kanto_ThreeIsland_House2_EventScript_842163C::
	lock
	faceplayer
	setvar VAR_0x8004, 7
	setvar VAR_0x8005, 1
	special QzQuestLogNop
	msgbox Kanto_ThreeIsland_House2_Text_8453862, MSGBOX_DEFAULT
	release
	end

@ Kanto_ThreeIsland_House2 object 1 (local id 1, gfx 0x3F3B) at 4,4
Kanto_ThreeIsland_House2_EventScript_8421655::
	msgbox Kanto_ThreeIsland_House2_Text_84537DD, MSGBOX_NPC
	end

Kanto_ThreeIsland_House2_Text_84537DD::
	.string "Urrrrrrrrgggghhhh…\n"
	.string "Kwaaaaah!\p"
	.string "…It’s useless! I can’t make this\n"
	.string "spoon bend with my mind.\p"
	.string "Maybe I really don’t have any\n"
	.string "psychic powers.$"

Kanto_ThreeIsland_House2_Text_8453862::
	.string "You know about a girl GYM LEADER\n"
	.string "in SAFFRON CITY?\p"
	.string "She uses PSYCHIC-type POKéMON,\n"
	.string "right?\p"
	.string "I admire her, I really do.\p"
	.string "I’ve heard that she’s one cool\n"
	.string "customer and beautiful, too.$"
