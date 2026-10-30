Johto_LakeOfRageMagikarpHouse_MapScripts::
	.byte 0

@ Johto_LakeOfRageMagikarpHouse object 1 (local id 1, gfx 0x3E95) at 5,4
Johto_LakeOfRageMagikarpHouse_EventScript_83DDFD0::
	lock
	faceplayer
	goto_if_ge VAR_QZ_0x500C, 14, Johto_LakeOfRageMagikarpHouse_EventScript_83DE00B
	goto_if_eq VAR_QZ_0x5026, 0, Johto_LakeOfRageMagikarpHouse_EventScript_83DDFF3
	msgbox Johto_LakeOfRageMagikarpHouse_Text_83DE2B3, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_LakeOfRageMagikarpHouse_EventScript_83DDFF3::
	msgbox Johto_LakeOfRageMagikarpHouse_Text_83DE192, MSGBOX_DEFAULT
	msgbox Johto_LakeOfRageMagikarpHouse_Text_83DE2B3, MSGBOX_DEFAULT
	setvar VAR_QZ_0x5026, 1
	closemessage
	release
	end

Johto_LakeOfRageMagikarpHouse_EventScript_83DE00B::
	goto_if_eq VAR_QZ_0x5026, 2, Johto_LakeOfRageMagikarpHouse_EventScript_83DE0F4
	msgbox Johto_LakeOfRageMagikarpHouse_Text_83DE32B, MSGBOX_DEFAULT
	msgbox Johto_LakeOfRageMagikarpHouse_Text_83DE3A6, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Johto_LakeOfRageMagikarpHouse_EventScript_83DE090
	getpartysize
	goto_if_eq VAR_RESULT, 1, Johto_LakeOfRageMagikarpHouse_EventScript_83DE166
	special ChoosePartyMon
	waitstate
	lock
	showmoneybox 0, 0, 0
	copyvar VAR_0x800A, VAR_0x8004
	goto_if_eq VAR_0x8004, 255, Johto_LakeOfRageMagikarpHouse_EventScript_83DE09B
	qz_takepartymonofspecies 0x81
	goto_if_eq VAR_RESULT, 0, Johto_LakeOfRageMagikarpHouse_EventScript_83DE0A9
	goto_if_eq VAR_RESULT, 3, Johto_LakeOfRageMagikarpHouse_EventScript_83DE0B7
	goto_if_eq VAR_RESULT, 4, Johto_LakeOfRageMagikarpHouse_EventScript_83DE0C5
	goto_if_eq VAR_RESULT, 1, Johto_LakeOfRageMagikarpHouse_EventScript_83DE171
	goto_if_eq VAR_RESULT, 2, Johto_LakeOfRageMagikarpHouse_EventScript_83DE0D3

Johto_LakeOfRageMagikarpHouse_EventScript_83DE090::
	msgbox Johto_LakeOfRageMagikarpHouse_Text_83DE53F, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_LakeOfRageMagikarpHouse_EventScript_83DE09B::
	msgbox Johto_LakeOfRageMagikarpHouse_Text_83DE53F, MSGBOX_DEFAULT
	hidemoneybox
	closemessage
	release
	end

Johto_LakeOfRageMagikarpHouse_EventScript_83DE0A9::
	msgbox Johto_LakeOfRageMagikarpHouse_Text_83DE4E5, MSGBOX_DEFAULT
	hidemoneybox
	closemessage
	release
	end

Johto_LakeOfRageMagikarpHouse_EventScript_83DE0B7::
	msgbox Johto_LakeOfRageMagikarpHouse_Text_83DE2F6, MSGBOX_DEFAULT
	hidemoneybox
	closemessage
	release
	end

Johto_LakeOfRageMagikarpHouse_EventScript_83DE0C5::
	msgbox Johto_LakeOfRageMagikarpHouse_Text_83DE4FA, MSGBOX_DEFAULT
	hidemoneybox
	closemessage
	release
	end

Johto_LakeOfRageMagikarpHouse_EventScript_83DE0D3::
	setvar VAR_QZ_0x5026, 2
	msgbox Johto_LakeOfRageMagikarpHouse_Text_83DE4A3, MSGBOX_DEFAULT
	addmoney 0x7D0, 0
	updatemoneybox 0
	playse SE_SHOP
	waitse
	hidemoneybox
	closemessage
	release
	end

Johto_LakeOfRageMagikarpHouse_EventScript_83DE0F4::
	msgbox Johto_LakeOfRageMagikarpHouse_Text_83DE3A6, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Johto_LakeOfRageMagikarpHouse_EventScript_83DE090
	getpartysize
	goto_if_eq VAR_RESULT, 1, Johto_LakeOfRageMagikarpHouse_EventScript_83DE166
	special ChoosePartyMon
	waitstate
	lock
	showmoneybox 0, 0, 0
	copyvar VAR_0x800A, VAR_0x8004
	goto_if_eq VAR_0x8004, 255, Johto_LakeOfRageMagikarpHouse_EventScript_83DE09B
	qz_takepartymonofspecies 0x81
	goto_if_eq VAR_RESULT, 0, Johto_LakeOfRageMagikarpHouse_EventScript_83DE0A9
	goto_if_eq VAR_RESULT, 3, Johto_LakeOfRageMagikarpHouse_EventScript_83DE0B7
	goto_if_eq VAR_RESULT, 4, Johto_LakeOfRageMagikarpHouse_EventScript_83DE0C5
	goto_if_eq VAR_RESULT, 1, Johto_LakeOfRageMagikarpHouse_EventScript_83DE171
	goto_if_eq VAR_RESULT, 2, Johto_LakeOfRageMagikarpHouse_EventScript_83DE0D3

Johto_LakeOfRageMagikarpHouse_EventScript_83DE166::
	msgbox Johto_LakeOfRageMagikarpHouse_Text_83DE2F6, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_LakeOfRageMagikarpHouse_EventScript_83DE171::
	setvar VAR_QZ_0x5026, 2
	msgbox Johto_LakeOfRageMagikarpHouse_Text_83DE43A, MSGBOX_DEFAULT
	addmoney 0x186A0, 0
	updatemoneybox 0
	playse SE_SHOP
	waitse
	hidemoneybox
	closemessage
	release
	end

Johto_LakeOfRageMagikarpHouse_Text_83DE192::
	.string "LAKE OF RAGE is actually a crater\n"
	.string "made by rampaging GYARADOS.\p"
	.string "The crater filled up with rainwater\n"
	.string "and the LAKE was formed.\p"
	.string "That’s the story passed on from my\n"
	.string "Grandpa’s great-great-grandpa.\p"
	.string "It used to be that you could catch\n"
	.string "lively MAGIKARP there, but…\p"
	.string "I don’t understand what’s happening.$"

Johto_LakeOfRageMagikarpHouse_Text_83DE2B3::
	.string "The LAKE hasn’t been normal since\n"
	.string "those men wearing black arrived.$"

Johto_LakeOfRageMagikarpHouse_Text_83DE2F6::
	.string "I can’t leave you without a POKéMON\n"
	.string "that can battle!$"

Johto_LakeOfRageMagikarpHouse_Text_83DE32B::
	.string "LAKE OF RAGE is back to normal.\p"
	.string "The MAGIKARP have returned.\p"
	.string "I’d like to raise more MAGIKARP.\n"
	.string "I’ll pay for any you sell me!$"

Johto_LakeOfRageMagikarpHouse_Text_83DE3A6::
	.string "I pay ¥2,000 for each MAGIKARP,\n"
	.string "or ¥100,000 if it is level 100.\p"
	.string "Would you like to sell me one?$"

Johto_LakeOfRageMagikarpHouse_Text_83DE43A::
	.string "A level 100 MAGIKARP!\n"
	.string "Here is ¥100,000. I’ll take care of it!$"

Johto_LakeOfRageMagikarpHouse_Text_83DE4A3::
	.string "Thank you for the MAGIKARP!\n"
	.string "Here is ¥2,000. I’ll take care of it!$"

Johto_LakeOfRageMagikarpHouse_Text_83DE4E5::
	.string "I only buy MAGIKARP.$"

Johto_LakeOfRageMagikarpHouse_Text_83DE4FA::
	.string "Your MAGIKARP is holding an item.\n"
	.string "Remove it before selling it to me.$"

Johto_LakeOfRageMagikarpHouse_Text_83DE53F::
	.string "That’s all right. Come back anytime!$"
