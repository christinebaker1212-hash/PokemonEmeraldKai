Kanto_ThreeIsland_House5_MapScripts::
	.byte 0

@ Kanto_ThreeIsland_House5 object 1 (local id 1, gfx 0x3F18) at 2,5
Kanto_ThreeIsland_House5_EventScript_842167C::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x21EC, Kanto_ThreeIsland_House5_EventScript_8421691
	msgbox Kanto_ThreeIsland_House5_Text_8453A01, MSGBOX_DEFAULT
	release
	end

Kanto_ThreeIsland_House5_EventScript_8421691::
	msgbox Kanto_ThreeIsland_House5_Text_8453A73, MSGBOX_DEFAULT
	release
	end

Kanto_ThreeIsland_House5_Text_8453A01::
	.string "Me?\n"
	.string "I’m not LOSTELLE.\p"
	.string "She’s probably gone to the\n"
	.string "BERRY FOREST, I think.\p"
	.string "She picks BERRIES there for making\n"
	.string "lunch.$"

Kanto_ThreeIsland_House5_Text_8453A73::
	.string "I think I’ll go play with LOSTELLE\n"
	.string "today.$"
