Kanto_CeladonCity_DepartmentStore_4F_MapScripts::
	.byte 0

@ Kanto_CeladonCity_DepartmentStore_4F object 1 (local id 1, gfx 0x3F1E) at 6,6
Kanto_CeladonCity_DepartmentStore_4F_EventScript_841BDB3::
	msgbox Kanto_CeladonCity_DepartmentStore_4F_Text_8445C20, MSGBOX_NPC
	end

@ Kanto_CeladonCity_DepartmentStore_4F object 2 (local id 2, gfx 0x3F5C) at 10,11
Kanto_CeladonCity_DepartmentStore_4F_EventScript_841BDBC::
	msgbox Kanto_CeladonCity_DepartmentStore_4F_Text_8445C61, MSGBOX_NPC
	end

@ Kanto_CeladonCity_DepartmentStore_4F sign / bg event at 4,1
Kanto_CeladonCity_DepartmentStore_4F_EventScript_841BDC5::
	msgbox Kanto_CeladonCity_DepartmentStore_4F_Text_8445CB5, MSGBOX_SIGN
	end

@ Kanto_CeladonCity_DepartmentStore_4F object 3 (local id 3, gfx 0x3EFA) at 3,13
Kanto_CeladonCity_DepartmentStore_4F_EventScript_841BDCE::
	specialvar VAR_RESULT, QzReturnZero_732
	goto_if_eq VAR_RESULT, 2, Quetzal_EventScript_8457FA3
	lock
	faceplayer

Kanto_CeladonCity_DepartmentStore_4F_EventScript_841BDE0::
	message Kanto_CeladonCity_DepartmentStore_4F_Text_8445CF3
	waitmessage
	multichoice 0, 0, 0x8C, 1
	nop
	copyvar VAR_0x8000, VAR_RESULT
	goto_if_eq VAR_0x8000, 0, Kanto_CeladonCity_DepartmentStore_4F_EventScript_841BE27
	goto_if_eq VAR_0x8000, 1, Kanto_CeladonCity_DepartmentStore_4F_EventScript_841BE32
	goto_if_eq VAR_0x8000, 2, Kanto_CeladonCity_DepartmentStore_4F_EventScript_841BE3D
	goto_if_eq VAR_0x8000, 3, Kanto_CeladonCity_DepartmentStore_4F_EventScript_841BE48
	msgbox Quetzal_Text_84565DA, MSGBOX_DEFAULT
	release
	end

Kanto_CeladonCity_DepartmentStore_4F_EventScript_841BE27::
	closemessage
	pokemart Kanto_CeladonCity_DepartmentStore_4F_Mart_841BE54
	goto Kanto_CeladonCity_DepartmentStore_4F_EventScript_841BDE0

Kanto_CeladonCity_DepartmentStore_4F_EventScript_841BE32::
	closemessage
	pokemart Kanto_CeladonCity_DepartmentStore_4F_Mart_841BE94
	goto Kanto_CeladonCity_DepartmentStore_4F_EventScript_841BDE0

Kanto_CeladonCity_DepartmentStore_4F_EventScript_841BE3D::
	closemessage
	pokemart Kanto_CeladonCity_DepartmentStore_4F_Mart_841BEBC
	goto Kanto_CeladonCity_DepartmentStore_4F_EventScript_841BDE0

Kanto_CeladonCity_DepartmentStore_4F_EventScript_841BE48::
	closemessage
	pokemart Kanto_CeladonCity_DepartmentStore_4F_Mart_841BECC
	goto Kanto_CeladonCity_DepartmentStore_4F_EventScript_841BDE0

Kanto_CeladonCity_DepartmentStore_4F_Mart_841BE54:
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
	.2byte ITEM_KEE_BERRY
	.2byte ITEM_MARANGA_BERRY
	.2byte ITEM_GRACIDEA
	.2byte ITEM_NONE

Kanto_CeladonCity_DepartmentStore_4F_Mart_841BE94:
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
	.2byte ITEM_ROSELI_BERRY
	.2byte ITEM_NONE

Kanto_CeladonCity_DepartmentStore_4F_Mart_841BEBC:
	.2byte ITEM_ENERGY_POWDER
	.2byte ITEM_ENERGY_ROOT
	.2byte ITEM_HEAL_POWDER
	.2byte ITEM_REMEDY
	.2byte ITEM_FINE_REMEDY
	.2byte ITEM_SUPERB_REMEDY
	.2byte ITEM_NONE

Kanto_CeladonCity_DepartmentStore_4F_Mart_841BECC:
	.2byte ITEM_HP_UP
	.2byte ITEM_PROTEIN
	.2byte ITEM_IRON
	.2byte ITEM_CALCIUM
	.2byte ITEM_ZINC
	.2byte ITEM_CARBOS
	.2byte ITEM_NONE

Kanto_CeladonCity_DepartmentStore_4F_Text_8445C20::
	.string "I was looking for vitamins.\n"
	.string "They’re by the berries and remedies!$"

Kanto_CeladonCity_DepartmentStore_4F_Text_8445C61::
	.string "A held berry can help your POKéMON\n"
	.string "during battle.\p"
	.string "Pick one that suits your partner!$"

Kanto_CeladonCity_DepartmentStore_4F_Text_8445CB5::
	.string "4F: BERRIES & CARE\p"
	.string "Berries, Gracidea, remedies,\n"
	.string "and vitamins.$"

Kanto_CeladonCity_DepartmentStore_4F_Text_8445CF3::
	.string "Take good care of your POKéMON.\n"
	.string "Which section would you like?$"
