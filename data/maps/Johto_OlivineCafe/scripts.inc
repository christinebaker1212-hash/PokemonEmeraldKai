Johto_OlivineCafe_MapScripts::
	.byte 0

@ Johto_OlivineCafe object 2 (local id 2, gfx 0x31) at 4,3
Johto_OlivineCafe_EventScript_83E4802::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x1002, Johto_OlivineCafe_EventScript_83E4818
	msgbox Johto_OlivineCafe_Text_83E483D, MSGBOX_DEFAULT
	setvar VAR_0x8004, ITEM_HM04   @ Moemon Kai: Crystal's HM (Quetzal said it, never gave it)
	call EventScript_GiveHmOnce
	setflag FLAG_QZ_0x1002

Johto_OlivineCafe_EventScript_83E4818::
	msgbox Johto_OlivineCafe_Text_83E48C6, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_OlivineCafe object 1 (local id 1, gfx 0x3E8E) at 7,4
Johto_OlivineCafe_EventScript_83E4823::
	lock
	faceplayer
	msgbox Johto_OlivineCafe_Text_83E493B, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_OlivineCafe object 3 (local id 3, gfx 0x31) at 6,7
Johto_OlivineCafe_EventScript_83E4830::
	lock
	faceplayer
	msgbox Johto_OlivineCafe_Text_83E4985, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_OlivineCafe_Text_83E483D::
	.string "Hah! Your MOéMON sure look like\n"
	.string "lightweights!\p"
	.string "They don’t have the power to\n"
	.string "move boulders aside.\p"
	.string "Here, use this and teach them\n"
	.string "STRENGTH!$"

Johto_OlivineCafe_Text_83E48C6::
	.string "On the sea, the only thing you can\n"
	.string "count on is your own good self!\p"
	.string "I’m so proud of my buff bod!$"

Johto_OlivineCafe_Text_83E493B::
	.string "OLIVINE CAFE’s menu is chock full\n"
	.string "of hearty fare for beefy SAILORS!$"

Johto_OlivineCafe_Text_83E4985::
	.string "Whenever I roll into this town,\n"
	.string "I always visit the OLIVINE CAFE.\p"
	.string "Everything on the menu makes me\n"
	.string "feel stronger. I can’t stop eating!$"
