Johto_GoldenrodBikeShop_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840EA66
	.byte 0

@ Johto_GoldenrodBikeShop object 1 (local id 1, gfx 0x3E84) at 4,4
Johto_GoldenrodBikeShop_EventScript_83D2E09::
	lock
	faceplayer
	checkitem ITEM_BICYCLE, 1
	goto_if_eq VAR_RESULT, 1, Johto_GoldenrodBikeShop_EventScript_83D2E5B
	msgbox Johto_GoldenrodBikeShop_Text_83D2F09, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Johto_GoldenrodBikeShop_EventScript_83D2E39
	msgbox Johto_GoldenrodBikeShop_Text_83D308C, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_GoldenrodBikeShop_EventScript_83D2E39::
	msgbox Johto_GoldenrodBikeShop_Text_83D2F78, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, ITEM_BICYCLE
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_0x8007, 1, Johto_GoldenrodBikeShop_EventScript_83D2E5B
	closemessage
	release
	end

Johto_GoldenrodBikeShop_EventScript_83D2E5B::
	msgbox Johto_GoldenrodBikeShop_Text_83D2FD5, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodBikeShop object 2 (local id 2, gfx 0x3E8E) at 2,6
Johto_GoldenrodBikeShop_EventScript_83D2E66::
	lock
	faceplayer
	msgbox Johto_GoldenrodBikeShop_Text_83D2E7F, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodBikeShop sign / bg event at 4,3
@ Johto_GoldenrodBikeShop sign / bg event at 2,3
@ Johto_GoldenrodBikeShop sign / bg event at 2,5
@ Johto_GoldenrodBikeShop sign / bg event at 2,7
Johto_GoldenrodBikeShop_EventScript_83D2E73::
	lock
	msgbox Johto_GoldenrodBikeShop_Text_83D30DF, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_GoldenrodBikeShop_Text_83D2E7F::
	.string "You can REGISTER the BICYCLE\n"
	.string "from your BAG.\p"
	.string "Get on your bike and stop.\n"
	.string "Press L to switch modes:\p"
	.string "MACH for speed,\n"
	.string "ACRO for hops and tricks.$"

Johto_GoldenrodBikeShop_Text_83D2F09::
	.string "…sigh… I moved here, but I can’t\n"
	.string "sell my BICYCLES. Why is that?\p"
	.string "Could you ride a BICYCLE and\n"
	.string "advertise for me?$"

Johto_GoldenrodBikeShop_Text_83D2F78::
	.string "Really? Great!\p"
	.string "I’ll loan you a BICYCLE so you\n"
	.string "can advertise the shop.$"

Johto_GoldenrodBikeShop_Text_83D2FD5::
	.string "Thanks for advertising our shop!\p"
	.string "This BICYCLE has two modes:\n"
	.string "MACH and ACRO.\p"
	.string "Get on your bike and stop.\n"
	.string "Press L to switch modes.\p"
	.string "MACH builds up speed.\n"
	.string "ACRO lets you hop and do tricks.$"

Johto_GoldenrodBikeShop_Text_83D308C::
	.string "…sigh… Oh, for the kindness of\n"
	.string "people…$"

Johto_GoldenrodBikeShop_Text_83D30DF::
	.string "It’s a shiny new BICYCLE!$"
