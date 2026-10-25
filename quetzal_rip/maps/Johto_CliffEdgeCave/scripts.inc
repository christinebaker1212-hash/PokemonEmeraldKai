Johto_CliffEdgeCave_MapScripts::
	.byte 0

@ Johto_CliffEdgeCave object 1 (local id 1, gfx 0x18) at 24,11
Johto_CliffEdgeCave_EventScript_83CBB3F::
	lock
	faceplayer
	msgbox Johto_CliffEdgeCave_Text_83CBB59, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_CliffEdgeCave object 2 (local id 2, gfx 0x3E91) at 16,15
@ Johto_CliffEdgeCave object 3 (local id 3, gfx 0x3E91) at 17,15
Johto_CliffEdgeCave_EventScript_83CBB4C::
	lock
	faceplayer
	msgbox Johto_CliffEdgeCave_Text_83CBC2D, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_CliffEdgeCave sign / bg event at 25,11
Johto_CliffEdgeCave_EventScript_83CBCC3::
	lock
	msgbox Johto_CliffEdgeCave_Text_83CBCCF, MSGBOX_SIGN
	closemessage
	release
	end

Johto_CliffEdgeCave_Text_83CBB59::
	.string "Did you know? There’s a SAFARI ZONE\n"
	.string "just around the corner.\p"
	.string "It’s really fun!\n"
	.string "You will see many different kinds of\l"
	.string "POKéMON, and it’s all you can catch!\p"
	.string "There is no reason not to go, right?\n"
	.string "Just around the corner.$"

Johto_CliffEdgeCave_Text_83CBC2D::
	.string "I appreciate that everybody is looking\n"
	.string "forward to it.\p"
	.string "But the lady at the reception seems\n"
	.string "a bit too dramatic...\p"
	.string "Don’t get your expectations too high!$"

Johto_CliffEdgeCave_Text_83CBCCF::
	.string "Everything Is Thrilling!\n"
	.string "Explore to Find Wild POKéMON!\l"
	.string "Warden BAOBA’s SAFARI ZONE\l"
	.string "Up and Running!$"
