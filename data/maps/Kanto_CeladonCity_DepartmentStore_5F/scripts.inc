Kanto_CeladonCity_DepartmentStore_5F_MapScripts::
	.byte 0

@ Kanto_CeladonCity_DepartmentStore_5F object 1 (local id 1, gfx 0x3F09) at 11,6
Kanto_CeladonCity_DepartmentStore_5F_EventScript_841BEDB::
	msgbox Kanto_CeladonCity_DepartmentStore_5F_Text_8445D31, MSGBOX_NPC
	end

@ Kanto_CeladonCity_DepartmentStore_5F object 2 (local id 2, gfx 0x3F41) at 6,8
Kanto_CeladonCity_DepartmentStore_5F_EventScript_841BEE4::
	msgbox Kanto_CeladonCity_DepartmentStore_5F_Text_8445D92, MSGBOX_NPC
	end

@ Kanto_CeladonCity_DepartmentStore_5F sign / bg event at 8,1
Kanto_CeladonCity_DepartmentStore_5F_EventScript_841BEED::
	msgbox Kanto_CeladonCity_DepartmentStore_5F_Text_8445DE3, MSGBOX_SIGN
	end

@ Kanto_CeladonCity_DepartmentStore_5F object 3 (local id 3, gfx 0x3EFA) at 1,7
Kanto_CeladonCity_DepartmentStore_5F_EventScript_841BEF6::
	specialvar VAR_RESULT, QzReturnZero
	goto_if_eq VAR_RESULT, 2, Quetzal_EventScript_8457FA3
	lock
	faceplayer
	message Quetzal_Text_84563A6
	waitmessage
	pokemart Kanto_CeladonCity_DepartmentStore_5F_Mart_841BF20
	msgbox Quetzal_Text_84565DA, MSGBOX_DEFAULT
	release
	end

@ Kanto_CeladonCity_DepartmentStore_5F object 4 (local id 4, gfx 0x3EFA) at 1,6
Kanto_CeladonCity_DepartmentStore_5F_EventScript_841BF38::
	specialvar VAR_RESULT, QzReturnZero
	goto_if_eq VAR_RESULT, 2, Quetzal_EventScript_8457FA3
	lock
	faceplayer
	message Quetzal_Text_84563A6
	waitmessage
	pokemart Kanto_CeladonCity_DepartmentStore_5F_Mart_841BF60
	msgbox Quetzal_Text_84565DA, MSGBOX_DEFAULT
	release
	end

Kanto_CeladonCity_DepartmentStore_5F_Mart_841BF20:
	.2byte ITEM_METEORITE
	.2byte ITEM_NONE

Kanto_CeladonCity_DepartmentStore_5F_Mart_841BF60:
	.2byte ITEM_HP_UP   @ (FireRed\'s vitamins clerk: src/shop.c)
	.2byte ITEM_NONE

Kanto_CeladonCity_DepartmentStore_5F_Text_8445D31::
	.string "MOéMON stat enhancers can be\n"
	.string "bought only here.\p"
	.string "HP UP increases the base HP of a\n"
	.string "MOéMON.\p"
	.string "CALCIUM raises the base SP. ATK\n"
	.string "stat of one MOéMON.\p"
	.string "ZINC boosts the base SP. DEF stat\n"
	.string "of one MOéMON.\p"
	.string "CARBOS enhances the base SPEED\n"
	.string "stat.$"

Kanto_CeladonCity_DepartmentStore_5F_Text_8445D92::
	.string "I’m here for MOéMON stat\n"
	.string "enhancers.\p"
	.string "PROTEIN increases the base ATTACK\n"
	.string "power.\p"
	.string "IRON increases the base DEFENSE\n"
	.string "stat.$"

Kanto_CeladonCity_DepartmentStore_5F_Text_8445DE3::
	.string "5F: DRUGSTORE$"

