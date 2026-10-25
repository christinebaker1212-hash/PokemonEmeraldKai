Kanto_VermilionCity_PokemonFanClub_MapScripts::
	.byte 0

@ Kanto_VermilionCity_PokemonFanClub object 1 (local id 1, gfx 0x3F09) at 5,4
Kanto_VermilionCity_PokemonFanClub_EventScript_841B457::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x2142, Kanto_VermilionCity_PokemonFanClub_EventScript_841B47F
	msgbox Kanto_VermilionCity_PokemonFanClub_Text_84445D5, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Kanto_VermilionCity_PokemonFanClub_EventScript_841B489
	msgbox Kanto_VermilionCity_PokemonFanClub_Text_84448B7, MSGBOX_DEFAULT
	release
	end

Kanto_VermilionCity_PokemonFanClub_EventScript_841B47F::
	msgbox Kanto_VermilionCity_PokemonFanClub_Text_84448E5, MSGBOX_DEFAULT
	release
	end

Kanto_VermilionCity_PokemonFanClub_EventScript_841B489::
	msgbox Kanto_VermilionCity_PokemonFanClub_Text_844467F, MSGBOX_DEFAULT
	checkitem ITEM_BIKE_VOUCHER, 1
	goto_if_eq VAR_RESULT, 1, Kanto_VermilionCity_PokemonFanClub_EventScript_841B4CB
	additem ITEM_BIKE_VOUCHER, 1
	goto_if_eq VAR_RESULT, 0, Kanto_VermilionCity_PokemonFanClub_EventScript_841B4D8
	loadword 0, Kanto_VermilionCity_PokemonFanClub_Text_84447A5
	setorcopyvar VAR_0x8000, 0x2EC
	setorcopyvar VAR_0x8001, 1
	setorcopyvar VAR_0x8002, 0x212
	call Quetzal_EventScript_845571E

Kanto_VermilionCity_PokemonFanClub_EventScript_841B4CB::
	setflag FLAG_QZ_0x2142
	msgbox Kanto_VermilionCity_PokemonFanClub_Text_84447D3, MSGBOX_DEFAULT
	release
	end

Kanto_VermilionCity_PokemonFanClub_EventScript_841B4D8::
	msgbox Kanto_VermilionCity_PokemonFanClub_Text_8444929, MSGBOX_DEFAULT
	release
	end

@ Kanto_VermilionCity_PokemonFanClub object 2 (local id 2, gfx 0x3F5A) at 6,4
Kanto_VermilionCity_PokemonFanClub_EventScript_841B4E2::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x2108, Kanto_VermilionCity_PokemonFanClub_EventScript_841B4F7
	msgbox Kanto_VermilionCity_PokemonFanClub_Text_844493D, MSGBOX_DEFAULT
	release
	end

Kanto_VermilionCity_PokemonFanClub_EventScript_841B4F7::
	setvar VAR_0x8004, 1
	setvar VAR_0x8005, 1
	special QzQuestLogNop_737
	msgbox Kanto_VermilionCity_PokemonFanClub_Text_84449B7, MSGBOX_DEFAULT
	release
	end

@ Kanto_VermilionCity_PokemonFanClub object 5 (local id 5, gfx 0x3F57) at 4,5
Kanto_VermilionCity_PokemonFanClub_EventScript_841B50E::
	lock
	faceplayer
	goto_if_set FLAG_TEMP_3, Kanto_VermilionCity_PokemonFanClub_EventScript_841B531
	msgbox Kanto_VermilionCity_PokemonFanClub_Text_8444523, MSGBOX_DEFAULT
	closemessage
	applymovement 5, Quetzal_Movement_8457AA8
	waitmovement 0
	setflag FLAG_TEMP_2
	release
	end

Kanto_VermilionCity_PokemonFanClub_EventScript_841B531::
	msgbox Kanto_VermilionCity_PokemonFanClub_Text_844456F, MSGBOX_DEFAULT
	closemessage
	applymovement 5, Quetzal_Movement_8457AA8
	waitmovement 0
	clearflag FLAG_TEMP_3
	release
	end

@ Kanto_VermilionCity_PokemonFanClub object 6 (local id 6, gfx 0x3F04) at 7,5
Kanto_VermilionCity_PokemonFanClub_EventScript_841B549::
	lock
	faceplayer
	goto_if_set FLAG_TEMP_2, Kanto_VermilionCity_PokemonFanClub_EventScript_841B56C
	msgbox Kanto_VermilionCity_PokemonFanClub_Text_84444C6, MSGBOX_DEFAULT
	closemessage
	applymovement 6, Quetzal_Movement_8457AA8
	waitmovement 0
	setflag FLAG_TEMP_3
	release
	end

Kanto_VermilionCity_PokemonFanClub_EventScript_841B56C::
	msgbox Kanto_VermilionCity_PokemonFanClub_Text_84444F3, MSGBOX_DEFAULT
	closemessage
	applymovement 6, Quetzal_Movement_8457AA8
	waitmovement 0
	clearflag FLAG_TEMP_2
	release
	end

@ Kanto_VermilionCity_PokemonFanClub object 3 (local id 3, gfx 0x3F34) at 7,6
Kanto_VermilionCity_PokemonFanClub_EventScript_841B584::
	lock
	faceplayer
	waitse
	playmoncry SPECIES_PIKACHU, 0
	msgbox Kanto_VermilionCity_PokemonFanClub_Text_84445B1, MSGBOX_DEFAULT
	waitmoncry
	release
	end

@ Kanto_VermilionCity_PokemonFanClub object 4 (local id 4, gfx 0x3F45) at 4,6
Kanto_VermilionCity_PokemonFanClub_EventScript_841B597::
	lock
	faceplayer
	waitse
	playmoncry SPECIES_SEEL, 0
	msgbox Kanto_VermilionCity_PokemonFanClub_Text_84445C8, MSGBOX_DEFAULT
	waitmoncry
	release
	end

@ Kanto_VermilionCity_PokemonFanClub sign / bg event at 2,1
Kanto_VermilionCity_PokemonFanClub_EventScript_841B5AA::
	msgbox Kanto_VermilionCity_PokemonFanClub_Text_8444967, MSGBOX_SIGN
	end

@ Kanto_VermilionCity_PokemonFanClub sign / bg event at 9,1
Kanto_VermilionCity_PokemonFanClub_EventScript_841B5B3::
	msgbox Kanto_VermilionCity_PokemonFanClub_Text_8444994, MSGBOX_SIGN
	end

Kanto_VermilionCity_PokemonFanClub_Text_84444C6::
	.string "Won’t you admire my PIKACHU’s\n"
	.string "adorable tail?$"

Kanto_VermilionCity_PokemonFanClub_Text_84444F3::
	.string "Humph!\p"
	.string "My PIKACHU is twice as cute as\n"
	.string "that one!$"

Kanto_VermilionCity_PokemonFanClub_Text_8444523::
	.string "I just adore my SEEL!\n"
	.string "It’s so lovable!\p"
	.string "It squeals, “Kyuuuh,” when I\n"
	.string "hug it!$"

Kanto_VermilionCity_PokemonFanClub_Text_844456F::
	.string "Oh, dear!\p"
	.string "My SEEL is far more attractive.\n"
	.string "By double, I would say.$"

Kanto_VermilionCity_PokemonFanClub_Text_84445B1::
	.string "PIKACHU: Chu! Pikachu!$"

Kanto_VermilionCity_PokemonFanClub_Text_84445C8::
	.string "SEEL: Kyuoo!$"

Kanto_VermilionCity_PokemonFanClub_Text_84445D5::
	.string "I chair the POKéMON Fan Club!\p"
	.string "I raise more than a hundred\n"
	.string "POKéMON!\p"
	.string "I’m very fussy when it comes to\n"
	.string "POKéMON! I surely am!\p"
	.string "So…\p"
	.string "Did you come visit to hear about\n"
	.string "my POKéMON?$"

Kanto_VermilionCity_PokemonFanClub_Text_844467F::
	.string "Good!\n"
	.string "Then listen up!\p"
	.string "My favorite RAPIDASH…\p"
	.string "It’s…cute…lovely…smart… \n"
	.string "plus…amazing…you think so?…\l"
	.string "oh yes…it’s…stunning…\l"
	.string "kindly…love it!\p"
	.string "Hug it…when…sleeping…warm\n"
	.string "and cuddly…spectacular…\l"
	.string "ravishing…\l"
	.string "…Oops! Look at the time!\l"
	.string "I kept you too long!\p"
	.string "Thanks for hearing me out!\n"
	.string "I want you to have this!$"

Kanto_VermilionCity_PokemonFanClub_Text_84447A5::
	.string "{PLAYER} received a BIKE VOUCHER\n"
	.string "from the CHAIRMAN.$"

Kanto_VermilionCity_PokemonFanClub_Text_84447D3::
	.string "Take that BIKE VOUCHER to the\n"
	.string "BIKE SHOP in CERULEAN CITY.\p"
	.string "Exchange that for a BICYCLE\n"
	.string "free of charge!\p"
	.string "Don’t worry, my favorite FEAROW\n"
	.string "will FLY me anywhere I need to go.\p"
	.string "So, I have no need for a BICYCLE.\p"
	.string "I hope you like cycling!$"

Kanto_VermilionCity_PokemonFanClub_Text_84448B7::
	.string "Oh. Come back when you want to\n"
	.string "hear my story!$"

Kanto_VermilionCity_PokemonFanClub_Text_84448E5::
	.string "Hello, {PLAYER}!\p"
	.string "Did you come see me about my\n"
	.string "POKéMON again?\p"
	.string "No?\n"
	.string "Too bad!$"

Kanto_VermilionCity_PokemonFanClub_Text_8444929::
	.string "Make room for this!$"

Kanto_VermilionCity_PokemonFanClub_Text_844493D::
	.string "Our CHAIRMAN is very vocal about\n"
	.string "POKéMON.$"

Kanto_VermilionCity_PokemonFanClub_Text_8444967::
	.string "Let’s all listen politely to other\n"
	.string "TRAINERS!$"

Kanto_VermilionCity_PokemonFanClub_Text_8444994::
	.string "If someone brags, brag right back!$"

Kanto_VermilionCity_PokemonFanClub_Text_84449B7::
	.string "Our CHAIRMAN really does adore his\n"
	.string "POKéMON.\p"
	.string "But the person who is most liked by\n"
	.string "POKéMON is DAISY, I think.$"
