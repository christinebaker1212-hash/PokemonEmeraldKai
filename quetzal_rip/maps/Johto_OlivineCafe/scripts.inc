Johto_OlivineCafe_MapScripts::
	.byte 0

@ Johto_OlivineCafe object 4 (local id 4, gfx 0x3E9A) at 5,4
Johto_OlivineCafe_EventScript_83E4720::
	lock
	faceplayer
	msgbox Johto_OlivineCafe_Text_83E472D, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_OlivineCafe object 2 (local id 2, gfx 0x31) at 4,3
Johto_OlivineCafe_EventScript_83E4802::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x1002, Johto_OlivineCafe_EventScript_83E4818
	msgbox Johto_OlivineCafe_Text_83E483D, MSGBOX_DEFAULT
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

Johto_OlivineCafe_Text_83E472D::
	.string "Oh... Hello, {PLAYER}.\p"
	.string "These dishes in front of me are\n"
	.string "not all mine...\p"
	.string "Uh...\p"
	.string "Actually, the person who sat here\n"
	.string "before me ate a lot...\p"
	.string "If you’d like to battle again,\n"
	.string "please visit me at the FIGHTING\l"
	.string "DOJO in SAFFRON CITY.$"

Johto_OlivineCafe_Text_83E483D::
	.string "Hah! Your POKéMON sure look like\n"
	.string "lightweights!\p"
	.string "They don’t have the power to\n"
	.string "move boulders aside.\p"
	.string "Here, use this and teach them\n"
	.string "STRENGTH!$"

Johto_OlivineCafe_Text_83E48C6::
	.string "On the sea, the only thing you\n"
	.string "can count on is your own self!\p"
	.string "I’m so proud of my buff bod!\n"
	.string "You should train like me!$"

Johto_OlivineCafe_Text_83E493B::
	.string "OLIVINE CAFE’s menu is packed\n"
	.string "with hearty fare for beefy\l"
	.string "SAILORS like me!$"

Johto_OlivineCafe_Text_83E4985::
	.string "Whenever I roll into this town,\n"
	.string "I always visit the OLIVINE CAFE.\p"
	.string "Everything on the menu makes me\n"
	.string "feel stronger. I can’t stop eating!$"
