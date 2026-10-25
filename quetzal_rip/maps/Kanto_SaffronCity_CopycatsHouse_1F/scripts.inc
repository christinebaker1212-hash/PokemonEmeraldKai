Kanto_SaffronCity_CopycatsHouse_1F_MapScripts::
	.byte 0

@ Kanto_SaffronCity_CopycatsHouse_1F object 2 (local id 2, gfx 0x3F59) at 3,3
Kanto_SaffronCity_CopycatsHouse_1F_EventScript_841EBB9::
	msgbox Kanto_SaffronCity_CopycatsHouse_1F_Text_844AAF6, MSGBOX_NPC
	end

@ Kanto_SaffronCity_CopycatsHouse_1F object 1 (local id 1, gfx 0x3EE6) at 8,4
Kanto_SaffronCity_CopycatsHouse_1F_EventScript_841EBC2::
	msgbox Kanto_SaffronCity_CopycatsHouse_1F_Text_844AB32, MSGBOX_NPC
	end

@ Kanto_SaffronCity_CopycatsHouse_1F object 3 (local id 3, gfx 0x3EF7) at 4,4
Kanto_SaffronCity_CopycatsHouse_1F_EventScript_841EBCB::
	lock
	faceplayer
	waitse
	playmoncry SPECIES_CHANSEY, 0
	msgbox Kanto_SaffronCity_CopycatsHouse_1F_Text_844ABA6, MSGBOX_DEFAULT
	waitmoncry
	release
	end

Kanto_SaffronCity_CopycatsHouse_1F_Text_844AAF6::
	.string "My daughter is so self-centered.\n"
	.string "She has but a few friends.$"

Kanto_SaffronCity_CopycatsHouse_1F_Text_844AB32::
	.string "My daughter likes to mimic people.\n"
	.string "It’s rather vexing.\p"
	.string "Her mimicry has earned her the\n"
	.string "nickname COPYCAT around here.$"

Kanto_SaffronCity_CopycatsHouse_1F_Text_844ABA6::
	.string "CHANSEY: Chaan! Sii!$"
