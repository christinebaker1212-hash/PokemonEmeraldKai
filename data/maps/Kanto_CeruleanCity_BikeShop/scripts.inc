Kanto_CeruleanCity_BikeShop_MapScripts::
	.byte 0

@ Kanto_CeruleanCity_BikeShop object 1 (local id 1, gfx 0x3F1E) at 9,3
Kanto_CeruleanCity_BikeShop_EventScript_841ABD5::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x2141, Kanto_CeruleanCity_BikeShop_EventScript_841AC3F
	checkitem ITEM_BICYCLE, 1
	goto_if_eq VAR_RESULT, 1, Kanto_CeruleanCity_BikeShop_EventScript_841AC3F
	goto_if_set FLAG_QZ_0x2142, Kanto_CeruleanCity_BikeShop_EventScript_841AC03
	msgbox Kanto_CeruleanCity_BikeShop_Text_8442A2A, MSGBOX_DEFAULT
	release
	end

Kanto_CeruleanCity_BikeShop_EventScript_841AC03::
	msgbox Kanto_CeruleanCity_BikeShop_Text_8442AB6, MSGBOX_DEFAULT
	additem ITEM_BICYCLE, 1
	goto_if_eq VAR_RESULT, 0, Kanto_CeruleanCity_BikeShop_EventScript_841AC4C
	loadword 0, Kanto_CeruleanCity_BikeShop_Text_8442AE5
	setorcopyvar VAR_0x8000, 0x2C2
	setorcopyvar VAR_0x8001, 1
	setorcopyvar VAR_0x8002, 0x212
	call Quetzal_EventScript_845571E
	removeitem ITEM_BIKE_VOUCHER, 1
	goto Kanto_CeruleanCity_BikeShop_EventScript_841AC3F

Kanto_CeruleanCity_BikeShop_EventScript_841AC3F::
	setflag FLAG_QZ_0x2141
	msgbox Kanto_CeruleanCity_BikeShop_Text_8442B12, MSGBOX_DEFAULT
	release
	end

Kanto_CeruleanCity_BikeShop_EventScript_841AC4C::
	msgbox Kanto_CeruleanCity_BikeShop_Text_8442BA8, MSGBOX_DEFAULT
	release
	end

@ Kanto_CeruleanCity_BikeShop object 3 (local id 3, gfx 0x3F58) at 5,5
Kanto_CeruleanCity_BikeShop_EventScript_841AC56::
	msgbox Kanto_CeruleanCity_BikeShop_Text_8442BCF, MSGBOX_NPC
	end

@ Kanto_CeruleanCity_BikeShop object 2 (local id 2, gfx 0x3F5C) at 9,7
Kanto_CeruleanCity_BikeShop_EventScript_841AC5F::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x2141, Kanto_CeruleanCity_BikeShop_EventScript_841AC74
	msgbox Kanto_CeruleanCity_BikeShop_Text_8442C37, MSGBOX_DEFAULT
	release
	end

Kanto_CeruleanCity_BikeShop_EventScript_841AC74::
	msgbox Kanto_CeruleanCity_BikeShop_Text_8442C93, MSGBOX_DEFAULT
	release
	end

Kanto_CeruleanCity_BikeShop_Text_8442A2A::
	.string "Welcome to our BIKE SHOP!\p"
	.string "We work with the POKéMON FAN CLUB\n"
	.string "in VERMILION CITY.\p"
	.string "Get a BIKE VOUCHER there and\n"
	.string "exchange it here for a BICYCLE!$"

Kanto_CeruleanCity_BikeShop_Text_8442AB6::
	.string "Oh, that’s…\p"
	.string "A BIKE VOUCHER!\p"
	.string "Okay!\n"
	.string "Here you go!$"

Kanto_CeruleanCity_BikeShop_Text_8442AE5::
	.string "{PLAYER} exchanged the\n"
	.string "BIKE VOUCHER for a BICYCLE.$"

Kanto_CeruleanCity_BikeShop_Text_8442B12::
	.string "This BICYCLE has two modes:\n"
	.string "MACH and ACRO.\p"
	.string "Get on your bike and stop.\n"
	.string "Press L to switch modes.\p"
	.string "MACH builds up speed.\n"
	.string "ACRO lets you hop and do tricks.$"

Kanto_CeruleanCity_BikeShop_Text_8442BA8::
	.string "You better make room for your\n"
	.string "BICYCLE!$"

Kanto_CeruleanCity_BikeShop_Text_8442BCF::
	.string "A plain city bike is good enough\n"
	.string "for me.\p"
	.string "After all, you can’t put a shopping\n"
	.string "basket on a mountain bike.$"

Kanto_CeruleanCity_BikeShop_Text_8442C37::
	.string "These BIKES are so cool!\p"
	.string "I’ll visit the POKéMON FAN CLUB\n"
	.string "in VERMILION to ask for a voucher!$"

Kanto_CeruleanCity_BikeShop_Text_8442C93::
	.string "Wow.\n"
	.string "Your BIKE is really cool!$"
