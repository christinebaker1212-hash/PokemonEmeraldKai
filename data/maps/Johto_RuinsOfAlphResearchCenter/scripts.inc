Johto_RuinsOfAlphResearchCenter_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840EA66
	.byte 0

@ Moemon Kai: Crystal's RESEARCH CENTER (pokecrystal maps/RuinsOfAlphResearchCenter.asm; audit case 61): no fossil
@ regenerator and no fossils on show (objects 4-6 wear FLAG_KAI_GONE; the local ids stay, object 7's encounter table
@ names it), the scientists' and books' words Crystal's

@ Johto_RuinsOfAlphResearchCenter object 2 (local id 2, gfx 0x3EB4) at 1,3
Johto_RuinsOfAlphResearchCenter_EventScript_83F6E83::
	lock
	faceplayer
	@ Moemon Kai: Crystal's scientist (pokecrystal RuinsOfAlphResearchCenterScientist1Text); Quetzal's plate shop
	@ is gone (the plates are SINNOH's)
	msgbox Johto_RuinsOfAlphResearchCenter_Text_CrystalRuinsAge, MSGBOX_DEFAULT
	release
	end

Johto_RuinsOfAlphResearchCenter_Text_CrystalRuinsAge:
	.string "The RUINS are about 1500 years old.\p"
	.string "But it's not known why they were\n"
	.string "built--or by whom.$"

@ Johto_RuinsOfAlphResearchCenter object 1 (local id 1, gfx 0x3EB3) at 8,5
Johto_RuinsOfAlphResearchCenter_EventScript_83F6E9C::
	lock
	faceplayer
	msgbox Johto_RuinsOfAlphResearchCenter_Text_83F6F2D, MSGBOX_DEFAULT
	release
	end

Johto_RuinsOfAlphResearchCenter_EventScript_Bookshelf::
	msgbox Johto_RuinsOfAlphResearchCenter_Text_AcademicBooks, MSGBOX_SIGN
	end

@ Johto_RuinsOfAlphResearchCenter object 3 (local id 3, gfx 0x3EB4) at 7,2
Johto_RuinsOfAlphResearchCenter_EventScript_83F6FE3::
	msgbox Johto_RuinsOfAlphResearchCenter_Text_Scientist3, MSGBOX_NPC
	end

Johto_RuinsOfAlphResearchCenter_Text_83F6F2D::
	.string "There are odd patterns drawn on\n"
	.string "the walls of the RUINS.\p"
	.string "They must be the keys for\n"
	.string "unraveling the mystery of the\l"
	.string "RUINS.$"

Johto_RuinsOfAlphResearchCenter_Text_AcademicBooks::
	.string "There are many academic books.\p"
	.string "Ancient Ruins… Mysteries of the\n"
	.string "Ancients…$"

Johto_RuinsOfAlphResearchCenter_Text_Scientist3::
	.string "The UNOWN you catch will all be\n"
	.string "recorded.\p"
	.string "Check to see how many kinds exist.$"
