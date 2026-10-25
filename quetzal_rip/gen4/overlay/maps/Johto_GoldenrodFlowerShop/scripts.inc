Johto_GoldenrodFlowerShop_MapScripts::
	.byte 0

Johto_GoldenrodFlowerShop_EventScript_8310F82::
	msgbox Johto_GoldenrodFlowerShop_Text_83963E6, MSGBOX_DEFAULT
	release
	end

@ Johto_GoldenrodFlowerShop object 2 (local id 2, gfx 0x14) at 10,8
Johto_GoldenrodFlowerShop_EventScript_83D4BEA::
	lock
	faceplayer
	dotimebasedevents
	goto_if_set FLAG_QZ_0x149C, Johto_GoldenrodFlowerShop_EventScript_8310F82
	msgbox Johto_GoldenrodFlowerShop_Text_839638B, MSGBOX_DEFAULT
	random 8
	addvar VAR_RESULT, 0x202
	setorcopyvar VAR_0x8000, VAR_RESULT
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_83602AA
	setflag FLAG_QZ_0x149C
	goto Johto_GoldenrodFlowerShop_EventScript_8310F82

@ Johto_GoldenrodFlowerShop object 1 (local id 1, gfx 0x8) at 2,4
Johto_GoldenrodFlowerShop_EventScript_83D4C26::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x102E, Johto_GoldenrodFlowerShop_EventScript_83D4C77
	goto_if_set FLAG_QZ_0x1099, Johto_GoldenrodFlowerShop_EventScript_83D4C6A
	goto_if_set FLAG_QZ_0x11B4, Johto_GoldenrodFlowerShop_EventScript_83D4C4E
	msgbox Johto_GoldenrodFlowerShop_Text_83D4D3C, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_GoldenrodFlowerShop_EventScript_83D4C4E::
	lock
	faceplayer
	msgbox Johto_GoldenrodFlowerShop_Text_83D4E58, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, 0x373
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	setflag FLAG_QZ_0x1099
	closemessage
	release
	end

Johto_GoldenrodFlowerShop_EventScript_83D4C6A::
	lock
	faceplayer
	msgbox Johto_GoldenrodFlowerShop_Text_83D4F7E, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_GoldenrodFlowerShop_EventScript_83D4C77::
	lock
	faceplayer
	msgbox Johto_GoldenrodFlowerShop_Text_83D4FAE, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodFlowerShop object 4 (local id 4, gfx 0x6) at 11,5
Johto_GoldenrodFlowerShop_EventScript_83D4C84::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x102E, Johto_GoldenrodFlowerShop_EventScript_83D4CAE
	goto_if_set FLAG_QZ_0x11B4, Johto_GoldenrodFlowerShop_EventScript_83D4CA3
	msgbox Johto_GoldenrodFlowerShop_Text_83D4CB9, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_GoldenrodFlowerShop_EventScript_83D4CA3::
	msgbox Johto_GoldenrodFlowerShop_Text_83D4FE0, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_GoldenrodFlowerShop_EventScript_83D4CAE::
	msgbox Johto_GoldenrodFlowerShop_Text_83D4FFD, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodFlowerShop object 5 (local id 5, gfx 0x3E82) at 6,5
Johto_GoldenrodFlowerShop_EventScript_83D5019::
	lock
	faceplayer
	message Johto_GoldenrodFlowerShop_Text_83D5056
	waitmessage
	pokemart Johto_GoldenrodFlowerShop_Mart_83D5030
	msgbox Quetzal_Text_8360D20, MSGBOX_DEFAULT
	release
	end

@ Johto_GoldenrodFlowerShop object 3 (local id 3, gfx 0x3E82) at 8,4
Johto_GoldenrodFlowerShop_EventScript_83D50E0::
	lock
	faceplayer
	message Johto_GoldenrodFlowerShop_Text_83D5136
	waitmessage
	pokemart Johto_GoldenrodFlowerShop_Mart_83D50F8
	msgbox Quetzal_Text_8360D20, MSGBOX_DEFAULT
	release
	end

Johto_GoldenrodFlowerShop_Mart_83D5030:
	.2byte ITEM_CHILAN_BERRY
	.2byte ITEM_OCCA_BERRY
	.2byte ITEM_PASSHO_BERRY
	.2byte ITEM_WACAN_BERRY
	.2byte ITEM_RINDO_BERRY
	.2byte ITEM_YACHE_BERRY
	.2byte ITEM_CHOPLE_BERRY
	.2byte ITEM_KEBIA_BERRY
	.2byte ITEM_SHUCA_BERRY
	.2byte ITEM_COBA_BERRY
	.2byte ITEM_PAYAPA_BERRY
	.2byte ITEM_TANGA_BERRY
	.2byte ITEM_CHARTI_BERRY
	.2byte ITEM_KASIB_BERRY
	.2byte ITEM_HABAN_BERRY
	.2byte ITEM_COLBUR_BERRY
	.2byte ITEM_BABIRI_BERRY
	.2byte ITEM_NONE

Johto_GoldenrodFlowerShop_Mart_83D50F8:
	.2byte ITEM_CHERI_BERRY
	.2byte ITEM_CHESTO_BERRY
	.2byte ITEM_PECHA_BERRY
	.2byte ITEM_RAWST_BERRY
	.2byte ITEM_ASPEAR_BERRY
	.2byte ITEM_LEPPA_BERRY
	.2byte ITEM_ORAN_BERRY
	.2byte ITEM_PERSIM_BERRY
	.2byte ITEM_LUM_BERRY
	.2byte ITEM_SITRUS_BERRY
	.2byte ITEM_FIGY_BERRY
	.2byte ITEM_WIKI_BERRY
	.2byte ITEM_MAGO_BERRY
	.2byte ITEM_AGUAV_BERRY
	.2byte ITEM_IAPAPA_BERRY
	.2byte ITEM_LIECHI_BERRY
	.2byte ITEM_GANLON_BERRY
	.2byte ITEM_SALAC_BERRY
	.2byte ITEM_PETAYA_BERRY
	.2byte ITEM_APICOT_BERRY
	.2byte ITEM_LANSAT_BERRY
	.2byte ITEM_STARF_BERRY
	.2byte ITEM_ENIGMA_BERRY
	.2byte ITEM_MICLE_BERRY
	.2byte ITEM_CUSTAP_BERRY
	.2byte ITEM_JABOCA_BERRY
	.2byte ITEM_ROWAP_BERRY
	.2byte ITEM_GRACIDEA
	.2byte ITEM_NONE

Johto_GoldenrodFlowerShop_Text_839638B::
	.string "I’m trying to be like my big sisters.\n"
	.string "I’m growing flowers, too!\p"
	.string "Here you go!\n"
	.string "It’s for you!$"

Johto_GoldenrodFlowerShop_Text_83963E6::
	.string "You can use a BERRY or give it\n"
	.string "to a POKéMON to hold.\p"
	.string "But now they have a machine that mixes\n"
	.string "up different BERRIES and makes candies\l"
	.string "for POKéMON.\p"
	.string "I want some candy, too.$"

Johto_GoldenrodFlowerShop_Text_83D4CB9::
	.string "When I told my sis about the jiggly\n"
	.string "tree, she said it’s dangerous.\p"
	.string "If I beat WHITNEY, I wonder if\n"
	.string "she’ll lend me her water bottle…$"

Johto_GoldenrodFlowerShop_Text_83D4D3C::
	.string "Apparently, there is an odd tree\n"
	.string "on ROUTE 36.\p"
	.string "They say it dances when you water\n"
	.string "it...\p"
	.string "Oh, of course not!\p"
	.string "This is a flower shop, so we do\n"
	.string "have watering cans...\p"
	.string "But I cannot let you have one!\n"
	.string "It’s too dangerous!\p"
	.string "Well, maybe if you manage to\n"
	.string "defeat gym leader WHITNEY, it\l"
	.string "might be okay.$"

Johto_GoldenrodFlowerShop_Text_83D4E58::
	.string "Apparently, there is an odd tree\n"
	.string "on ROUTE 36.\p"
	.string "They say it dances when you water\n"
	.string "it...\p"
	.string "Oh, that’s a PLAINBADGE from\n"
	.string "The GOLDENROD GYM!\p"
	.string "Did you defeat WHITNEY?!\p"
	.string "Well, then there’s nothing to worry\n"
	.string "about, is there?\p"
	.string "I’ll let you use this SQUIRTBOTTLE\n"
	.string "for a while then.\l"
	.string "...\p"
	.string "Oh, never mind. Keep it!$"

Johto_GoldenrodFlowerShop_Text_83D4F7E::
	.string "The odd tree is on ROUTE 36.\n"
	.string "Please be careful!$"

Johto_GoldenrodFlowerShop_Text_83D4FAE::
	.string "Lalala lalalala. Have plenty of\n"
	.string "water, my lovely!$"

Johto_GoldenrodFlowerShop_Text_83D4FE0::
	.string "Wow, you beat WHITNEY? Cool!$"

Johto_GoldenrodFlowerShop_Text_83D4FFD::
	.string "So it really was a POKéMON!$"

Johto_GoldenrodFlowerShop_Text_83D5056::
	.string "These BERRIES weaken super-effective\n"
	.string "attacks of their matching type.\p"
	.string "The NORMAL-type one reduces damage\n"
	.string "even if it isn’t super effective.$"

Johto_GoldenrodFlowerShop_Text_83D5136::
	.string "We sell all kinds of BERRIES.\n"
	.string "Have a look.$"
