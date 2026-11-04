Kanto_CeruleanCity_BikeShop_MapScripts::
	.byte 0

@ Kanto_CeruleanCity_BikeShop object 1 (local id 1, gfx 0x3F1E) at 9,3
Kanto_CeruleanCity_BikeShop_EventScript_841ABD5::
	@ Moemon Kai: FireRed's clerk (pokefirered CeruleanCity_BikeShop): the ¥1,000,000 BICYCLE on show, the voucher
	@ exchange, "How do you like your new BICYCLE?" (Quetzal's MACH / ACRO words described a bike this game hasn't)
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x2141, Kanto_CeruleanCity_BikeShop_EventScript_841AC3F
	checkitem ITEM_BICYCLE, 1
	goto_if_eq VAR_RESULT, 1, Kanto_CeruleanCity_BikeShop_EventScript_841AC3F
	goto_if_set FLAG_QZ_0x2142, Kanto_CeruleanCity_BikeShop_EventScript_841AC03
	showmoneybox 0, 0
	message Kanto_CeruleanCity_BikeShop_Text_8442A2A
	waitmessage
	multichoice 11, 0, MULTI_QZ_0x57, FALSE
	switch VAR_RESULT
	case 0, Kanto_CeruleanCity_BikeShop_EventScript_TryPurchaseBicycle
	case 1, Kanto_CeruleanCity_BikeShop_EventScript_ClerkGoodbye
	case MULTI_B_PRESSED, Kanto_CeruleanCity_BikeShop_EventScript_ClerkGoodbye
	end

@ (FireRed never checks the price either)
Kanto_CeruleanCity_BikeShop_EventScript_TryPurchaseBicycle::
	msgbox Kanto_CeruleanCity_BikeShop_Text_SorryYouCantAffordIt, MSGBOX_DEFAULT
	goto Kanto_CeruleanCity_BikeShop_EventScript_ClerkGoodbye
	end

Kanto_CeruleanCity_BikeShop_EventScript_ClerkGoodbye::
	msgbox Kanto_CeruleanCity_BikeShop_Text_ThankYouComeAgain, MSGBOX_DEFAULT
	hidemoneybox
	release
	end

Kanto_CeruleanCity_BikeShop_EventScript_841AC03::
	msgbox Kanto_CeruleanCity_BikeShop_Text_8442AB6, MSGBOX_DEFAULT
	additem ITEM_BICYCLE, 1
	goto_if_eq VAR_RESULT, 0, Kanto_CeruleanCity_BikeShop_EventScript_841AC4C
	loadword 0, Kanto_CeruleanCity_BikeShop_Text_8442AE5
	setorcopyvar VAR_0x8000, ITEM_BICYCLE
	setorcopyvar VAR_0x8001, 1
	setorcopyvar VAR_0x8002, 0x212
	call Quetzal_EventScript_845571E
	removeitem ITEM_BIKE_VOUCHER, 1
	setflag FLAG_QZ_0x2141
	msgbox Kanto_CeruleanCity_BikeShop_Text_ThankYouComeAgain, MSGBOX_DEFAULT
	release
	end

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
	.string "Hi!\n"
	.string "Welcome to our BIKE SHOP.\p"
	.string "Have we got just the BIKE for\n"
	.string "you!$"

Kanto_CeruleanCity_BikeShop_Text_8442AB6::
	.string "Oh, that’s…\p"
	.string "A BIKE VOUCHER!\p"
	.string "Okay!\n"
	.string "Here you go!$"

Kanto_CeruleanCity_BikeShop_Text_8442AE5::
	.string "{PLAYER} exchanged the\n"
	.string "BIKE VOUCHER for a BICYCLE.$"

Kanto_CeruleanCity_BikeShop_Text_8442B12::
	.string "How do you like your new BICYCLE?\n"
	.string "Do you like how it rides?\p"
	.string "You can take it out on CYCLING\n"
	.string "ROAD and even into caves!$"

Kanto_CeruleanCity_BikeShop_Text_8442BA8::
	.string "You better make room for your\n"
	.string "BICYCLE!$"

Kanto_CeruleanCity_BikeShop_Text_8442BCF::
	.string "A plain city bike is good enough\n"
	.string "for me.\p"
	.string "After all, you can’t put a shopping\n"
	.string "basket on a mountain bike.$"

Kanto_CeruleanCity_BikeShop_Text_8442C37::
	.string "These bikes are cool, but they’re\n"
	.string "way expensive!$"

Kanto_CeruleanCity_BikeShop_Text_8442C93::
	.string "Wow.\n"
	.string "Your BIKE is really cool!$"

Kanto_CeruleanCity_BikeShop_Text_SorryYouCantAffordIt::
	.string "Sorry!\n"
	.string "You can’t afford it!$"

Kanto_CeruleanCity_BikeShop_Text_ThankYouComeAgain::
	.string "Thank you!\n"
	.string "Come back again sometime!$"
