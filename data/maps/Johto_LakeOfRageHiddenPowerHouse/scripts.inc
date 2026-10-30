Johto_LakeOfRageHiddenPowerHouse_MapScripts::
	.byte 0

@ Johto_LakeOfRageHiddenPowerHouse object 1 (local id 1, gfx 0x3E9B) at 4,4
Johto_LakeOfRageHiddenPowerHouse_EventScript_83DDE89::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x1054, Johto_LakeOfRageHiddenPowerHouse_EventScript_83DDEC1
	msgbox Johto_LakeOfRageHiddenPowerHouse_Text_83DDECC, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, ITEM_DAWN_STONE
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_83602AA
	msgbox Johto_LakeOfRageHiddenPowerHouse_Text_83DDF5F, MSGBOX_DEFAULT
	setflag FLAG_QZ_0x1054
	closemessage
	release
	end

Johto_LakeOfRageHiddenPowerHouse_EventScript_83DDEC1::
	msgbox Johto_LakeOfRageHiddenPowerHouse_Text_83DDFBE, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_LakeOfRageHiddenPowerHouse_Text_83DDECC::
	.string "…You have strayed far…\p"
	.string "Here I have meditated. Inside me,\n"
	.string "a new power has been awakened.\p"
	.string "Let me share my power with your\n"
	.string "POKéMON. Take this, child.$"

Johto_LakeOfRageHiddenPowerHouse_Text_83DDF5F::
	.string "This DAWN STONE can make\n"
	.string "certain POKéMON evolve.\p"
	.string "Even a small stone can\n"
	.string "awaken a hidden power.$"

Johto_LakeOfRageHiddenPowerHouse_Text_83DDFBE::
	.string "I am meditating…$"
