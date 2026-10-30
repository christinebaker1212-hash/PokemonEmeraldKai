Johto_TinTower1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_83FAE20
	.byte 0

@ Johto_TinTower1F object 2 (local id 2, gfx 0x3EB2) at 3,4
Johto_TinTower1F_EventScript_83FAE29::
	lock
	faceplayer
	goto_if_eq VAR_QZ_0x500B, 11, Johto_TinTower1F_EventScript_83FAE41
	msgbox Johto_TinTower1F_Text_83FB144, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_TinTower1F_EventScript_83FAE41::
	msgbox Johto_TinTower1F_Text_83FB10A, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_TinTower1F object 1 (local id 1, gfx 0x3EB2) at 7,4
Johto_TinTower1F_EventScript_83FAE4C::
	lock
	faceplayer
	goto_if_eq VAR_QZ_0x500B, 11, Johto_TinTower1F_EventScript_83FAE64
	msgbox Johto_TinTower1F_Text_83FAF47, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_TinTower1F_EventScript_83FAE64::
	msgbox Johto_TinTower1F_Text_83FAE92, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_TinTower1F object 4 (local id 4, gfx 0x3EB2) at 14,5
Johto_TinTower1F_EventScript_83FAE6F::
	lock
	faceplayer
	goto_if_eq VAR_QZ_0x500B, 11, Johto_TinTower1F_EventScript_83FAE87
	msgbox Johto_TinTower1F_Text_83FB1E4, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_TinTower1F_EventScript_83FAE87::
	msgbox Johto_TinTower1F_Text_83FB060, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_TinTower1F_Text_83FAE92::
	.string "According to legend…\n"
	.string "When the souls of POKéMON and humans\p"
	.string "commune, from the heavens descends a\n"
	.string "POKéMON of rainbow colors…\p"
	.string "Could it mean the legendary POKéMON\n"
	.string "are testing us humans?$"

Johto_TinTower1F_Text_83FAF47::
	.string "When the BRASS TOWER burned down,\n"
	.string "three nameless POKéMON were said\l"
	.string "to have perished. It was tragic.\p"
	.string "However…\p"
	.string "A rainbow-colored POKéMON…\n"
	.string "In other words…\p"
	.string "HO-OH descended from the sky and\n"
	.string "gave new life to the three POKéMON.\p"
	.string "They are…\n"
	.string "SUICUNE, ENTEI and RAIKOU.\p"
	.string "That is what they say.$"

Johto_TinTower1F_Text_83FB060::
	.string "The two TOWERS are said to have been\n"
	.string "built to foster friendship and\p"
	.string "hope between POKéMON and people.\n"
	.string "That was 700 years ago, but the ideal\p"
	.string "still remains important today.$"

Johto_TinTower1F_Text_83FB10A::
	.string "HO-OH appears to have descended\n"
	.string "upon this, the TIN TOWER!$"

Johto_TinTower1F_Text_83FB144::
	.string "The legendary POKéMON are said\n"
	.string "to embody three powers…\p"
	.string "The lightning that struck the TOWER.\n"
	.string "The fire that burned the TOWER.\p"
	.string "And the rain that put out the fire…$"

Johto_TinTower1F_Text_83FB1E4::
	.string "Of the legendary POKéMON, SUICUNE\n"
	.string "is said to be the closest to HO-OH.\p"
	.string "I hear there may also be a link to\n"
	.string "POKéMON known as UNOWN.\p"
	.string "The POKéMON UNOWN must be sharing a\n"
	.string "cooperative bond with SUICUNE.$"
