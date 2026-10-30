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
	message Kanto_CeladonCity_DepartmentStore_5F_Text_8445E1F
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
	message Kanto_CeladonCity_DepartmentStore_5F_Text_8445E46
	waitmessage
	pokemart Kanto_CeladonCity_DepartmentStore_5F_Mart_841BF60
	msgbox Quetzal_Text_84565DA, MSGBOX_DEFAULT
	release
	end

Kanto_CeladonCity_DepartmentStore_5F_Mart_841BF20:
	.2byte ITEM_METEORITE
	.2byte ITEM_NONE

Kanto_CeladonCity_DepartmentStore_5F_Mart_841BF60:
	.2byte ITEM_PEWTER_CRUNCHIES
	.2byte ITEM_RAGE_CANDY_BAR
	.2byte ITEM_LAVA_COOKIE
	.2byte ITEM_OLD_GATEAU
	.2byte ITEM_CASTELIACONE
	.2byte ITEM_LUMIOSE_GALETTE
	.2byte ITEM_SHALOUR_SABLE
	.2byte ITEM_BIG_MALASADA
	.2byte ITEM_NONE

Kanto_CeladonCity_DepartmentStore_5F_Text_8445D31::
	.string "They sell items that change\n"
	.string "the forms of certain POKéMON.\p"
	.string "Each item works with specific\n"
	.string "species.$"

Kanto_CeladonCity_DepartmentStore_5F_Text_8445D92::
	.string "There are treats from many regions!\p"
	.string "They also sell nectars\n"
	.string "for ORICORIO’s forms.$"

Kanto_CeladonCity_DepartmentStore_5F_Text_8445DE3::
	.string "5F: FORMS & TREATS\p"
	.string "Form-changing items.\n"
	.string "Treats and nectars.$"

Kanto_CeladonCity_DepartmentStore_5F_Text_8445E1F::
	.string "Form-changing items.\n"
	.string "What do you need?$"

Kanto_CeladonCity_DepartmentStore_5F_Text_8445E46::
	.string "Treats and nectars.\n"
	.string "What would you like?$"
