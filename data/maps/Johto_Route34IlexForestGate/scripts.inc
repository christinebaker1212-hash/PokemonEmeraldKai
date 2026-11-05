Johto_Route34IlexForestGate_MapScripts::
	.byte 0

@ Johto_Route34IlexForestGate object 1 (local id 1, gfx 0x14) at 9,6
Johto_Route34IlexForestGate_EventScript_83D12A9::
	lock
	faceplayer
	msgbox Johto_Route34IlexForestGate_Text_83D12B6, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_Route34IlexForestGate_Text_83D12B6::
	.string "Did you see the shrine honoring\n"
	.string "the protector?\p"
	.string "It watches over the FOREST from\n"
	.string "across time.\p"
	.string "I think that it must be a\n"
	.string "GRASS-type MOéMON.$"

@ Moemon Kai: Crystal's teacher (T11b: TM108 SWEET SCENT)
Johto_Route34IlexForestGate_EventScript_Teacher::
	lock
	faceplayer
	goto_if_set FLAG_JOHTO_GOT_TM_SWEET_SCENT, Johto_Route34IlexForestGate_EventScript_GotSweetScent
	msgbox Johto_Route34IlexForestGate_Text_Teacher, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, ITEM_TM108
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_83602AA
	setflag FLAG_JOHTO_GOT_TM_SWEET_SCENT
Johto_Route34IlexForestGate_EventScript_GotSweetScent::
	msgbox Johto_Route34IlexForestGate_Text_GotSweetScent, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_Route34IlexForestGate_Text_Teacher::
	.string "Oh, honey. You’re making a\n"
	.string "POKéDEX?\p"
	.string "It must be hard if MOéMON won’t\n"
	.string "appear. Try using this TM.$"

Johto_Route34IlexForestGate_Text_GotSweetScent::
	.string "It’s SWEET SCENT.\p"
	.string "Use it wherever MOéMON appear.\p"
	.string "MOéMON will be enticed by it.$"
