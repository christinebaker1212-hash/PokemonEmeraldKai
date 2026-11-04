Kanto_TwoIsland_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Quetzal_EventScript_83C3036
	map_script MAP_SCRIPT_ON_RESUME, Quetzal_EventScript_83C3036
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_TwoIsland_EventScript_841726C
	.byte 0

Kanto_TwoIsland_EventScript_841726C::
	special QzKantoTravelingEventCheck
	setflag FLAG_QZ_0x225F
	setflag FLAG_QZ_0x2231
	call Kanto_TwoIsland_EventScript_8417280
	call Quetzal_EventScript_840FBB5
	end

Kanto_TwoIsland_EventScript_8417280::
	goto_if_set FLAG_QZ_0x220A, Kanto_TwoIsland_EventScript_84172A1
	goto_if_set FLAG_QZ_0x2108, Kanto_TwoIsland_EventScript_84172C2
	goto_if_set FLAG_QZ_0x21EC, Kanto_TwoIsland_EventScript_84172E3
	goto Kanto_TwoIsland_EventScript_84172F2

Kanto_TwoIsland_EventScript_84172A1::
	goto_if_unset FLAG_QZ_0x222D, Kanto_TwoIsland_EventScript_84172F8
	goto_if_unset FLAG_QZ_0x222A, Kanto_TwoIsland_EventScript_84172FE
	goto_if_unset FLAG_QZ_0x222B, Kanto_TwoIsland_EventScript_8417307
	goto Kanto_TwoIsland_EventScript_8417310

Kanto_TwoIsland_EventScript_84172C2::
	goto_if_unset FLAG_QZ_0x222D, Kanto_TwoIsland_EventScript_84172F8
	goto_if_unset FLAG_QZ_0x21EC, Kanto_TwoIsland_EventScript_84172F8
	goto_if_unset FLAG_QZ_0x222A, Kanto_TwoIsland_EventScript_84172FE
	goto Kanto_TwoIsland_EventScript_8417307

Kanto_TwoIsland_EventScript_84172E3::
	goto_if_unset FLAG_QZ_0x222D, Kanto_TwoIsland_EventScript_84172F8
	goto Kanto_TwoIsland_EventScript_84172FE

Kanto_TwoIsland_EventScript_84172F2::
	goto Kanto_TwoIsland_EventScript_84172F8

Kanto_TwoIsland_EventScript_84172F8::
	setvar VAR_QZ_0x5133, 1
	return

Kanto_TwoIsland_EventScript_84172FE::
	setvar VAR_QZ_0x5133, 2
	clearflag FLAG_QZ_0x21D6
	return

Kanto_TwoIsland_EventScript_8417307::
	setvar VAR_QZ_0x5133, 3
	clearflag FLAG_QZ_0x21D2
	return

Kanto_TwoIsland_EventScript_8417310::
	setvar VAR_QZ_0x5133, 4
	clearflag FLAG_QZ_0x21D5
	return

@ Kanto_TwoIsland object 1 (local id 1, gfx 0x3EFA) at 28,4
Kanto_TwoIsland_EventScript_8417319::
	specialvar VAR_RESULT, QzReturnZero
	goto_if_eq VAR_RESULT, 2, Quetzal_EventScript_8457FA3
	lock
	faceplayer
	goto_if_eq VAR_QZ_0x5133, 4, Kanto_TwoIsland_EventScript_8417352
	goto_if_eq VAR_QZ_0x5133, 3, Kanto_TwoIsland_EventScript_8417364
	goto_if_eq VAR_QZ_0x5133, 2, Kanto_TwoIsland_EventScript_8417376
	goto Kanto_TwoIsland_EventScript_8417388

@ Moemon Kai: FireRed's shop that grows (pokefirered TwoIsland/scripts.inc): its first words for each new stock, then
@ "May I help you?"; the AURORA / MYSTIC TICKETs (Quetzal's, the way to NAVEL ROCK / BIRTH ISLAND) in the last list
Kanto_TwoIsland_EventScript_8417352::
	goto_if_set FLAG_QZ_0x222C, Kanto_TwoIsland_EventScript_ShopSkipIntro
	setflag FLAG_QZ_0x222C
	message Kanto_TwoIsland_Text_BringingItemsFromDistantLands
	waitmessage
	goto Kanto_TwoIsland_EventScript_Shop4

Kanto_TwoIsland_EventScript_8417364::
	goto_if_set FLAG_QZ_0x222B, Kanto_TwoIsland_EventScript_ShopSkipIntro
	setflag FLAG_QZ_0x222B
	message Kanto_TwoIsland_Text_HopeYouGiveItYourBest
	waitmessage
	goto Kanto_TwoIsland_EventScript_Shop3

Kanto_TwoIsland_EventScript_8417376::
	goto_if_set FLAG_QZ_0x222A, Kanto_TwoIsland_EventScript_ShopSkipIntro
	setflag FLAG_QZ_0x222A
	message Kanto_TwoIsland_Text_AddedMerchandiseForLostelle
	waitmessage
	goto Kanto_TwoIsland_EventScript_Shop2

Kanto_TwoIsland_EventScript_8417388::
	goto_if_set FLAG_QZ_0x222D, Kanto_TwoIsland_EventScript_ShopSkipIntro
	setflag FLAG_QZ_0x222D
	message Kanto_TwoIsland_Text_WelcomeToShopMerchandiseLimited
	waitmessage
	goto Kanto_TwoIsland_EventScript_Shop1

Kanto_TwoIsland_EventScript_ShopSkipIntro::
	message Quetzal_Text_84563A6
	waitmessage
	goto_if_eq VAR_QZ_0x5133, 4, Kanto_TwoIsland_EventScript_Shop4
	goto_if_eq VAR_QZ_0x5133, 3, Kanto_TwoIsland_EventScript_Shop3
	goto_if_eq VAR_QZ_0x5133, 2, Kanto_TwoIsland_EventScript_Shop2
	goto Kanto_TwoIsland_EventScript_Shop1

Kanto_TwoIsland_EventScript_Shop1::
	pokemart Kanto_TwoIsland_Mart_1
	msgbox Quetzal_Text_84565DA, MSGBOX_DEFAULT
	release
	end

	.align 2
Kanto_TwoIsland_Mart_1:
	.2byte ITEM_GREAT_BALL
	.2byte ITEM_FRESH_WATER
	.2byte ITEM_NONE

Kanto_TwoIsland_EventScript_Shop2::
	pokemart Kanto_TwoIsland_Mart_2
	msgbox Quetzal_Text_84565DA, MSGBOX_DEFAULT
	release
	end

	.align 2
Kanto_TwoIsland_Mart_2:
	.2byte ITEM_ULTRA_BALL
	.2byte ITEM_GREAT_BALL
	.2byte ITEM_SODA_POP
	.2byte ITEM_FRESH_WATER
	.2byte ITEM_NONE

Kanto_TwoIsland_EventScript_Shop3::
	pokemart Kanto_TwoIsland_Mart_3
	msgbox Quetzal_Text_84565DA, MSGBOX_DEFAULT
	release
	end

	.align 2
Kanto_TwoIsland_Mart_3:
	.2byte ITEM_ULTRA_BALL
	.2byte ITEM_GREAT_BALL
	.2byte ITEM_LEMONADE
	.2byte ITEM_SODA_POP
	.2byte ITEM_FRESH_WATER
	.2byte ITEM_MOOMOO_MILK
	.2byte ITEM_NONE

Kanto_TwoIsland_EventScript_Shop4::
	pokemart Kanto_TwoIsland_Mart_4
	msgbox Quetzal_Text_84565DA, MSGBOX_DEFAULT
	release
	end

	.align 2
Kanto_TwoIsland_Mart_4:
	.2byte ITEM_ULTRA_BALL
	.2byte ITEM_REPEAT_BALL
	.2byte ITEM_TIMER_BALL
	.2byte ITEM_LEMONADE
	.2byte ITEM_SODA_POP
	.2byte ITEM_FRESH_WATER
	.2byte ITEM_MOOMOO_MILK
	.2byte ITEM_LAVA_COOKIE
	.2byte ITEM_AURORA_TICKET
	.2byte ITEM_MYSTIC_TICKET
	.2byte ITEM_NONE

@ Kanto_TwoIsland object 4 (local id 4, gfx 0x3F41) at 26,5
Kanto_TwoIsland_EventScript_84173D6::
	msgbox Kanto_TwoIsland_Text_8431FF4, MSGBOX_NPC
	end

@ Kanto_TwoIsland object 2 (local id 2, gfx 0x3F57) at 31,6
Kanto_TwoIsland_EventScript_84173DF::
	msgbox Kanto_TwoIsland_Text_843208F, MSGBOX_NPC
	end

@ Kanto_TwoIsland object 3 (local id 3, gfx 0x3EE8) at 30,11
Kanto_TwoIsland_EventScript_84173E8::
	lock
	faceplayer
	setvar VAR_0x8004, 0xA
	setvar VAR_0x8005, 4
	special QzQuestLogNop
	msgbox Kanto_TwoIsland_Text_843210C, MSGBOX_DEFAULT
	release
	end

@ Kanto_TwoIsland object 5 (local id 5, gfx 0x3F4B) at 27,6
Kanto_TwoIsland_EventScript_8417401::
	msgbox Kanto_TwoIsland_Text_84321ED, MSGBOX_NPC
	end

@ Kanto_TwoIsland object 9 (local id 9, gfx 0x3EEE) at 24,12
Kanto_TwoIsland_EventScript_841740A::
	msgbox Kanto_TwoIsland_Text_8432222, MSGBOX_NPC
	end

@ Kanto_TwoIsland object 8 (local id 8, gfx 0x3F17) at 37,12
Kanto_TwoIsland_EventScript_8417413::
	msgbox Kanto_TwoIsland_Text_8432293, MSGBOX_NPC
	end

@ Kanto_TwoIsland sign / bg event at 29,8
Kanto_TwoIsland_EventScript_841741C::
	msgbox Kanto_TwoIsland_Text_8431D4D, MSGBOX_SIGN
	end

@ Kanto_TwoIsland sign / bg event at 40,10
Kanto_TwoIsland_EventScript_8417425::
	msgbox Kanto_TwoIsland_Text_8431D6C, MSGBOX_SIGN
	end

@ Kanto_TwoIsland sign / bg event at 8,3
Kanto_TwoIsland_EventScript_841742E::
	msgbox Kanto_TwoIsland_Text_8431D8D, MSGBOX_SIGN
	end

@ Kanto_TwoIsland object 7 (local id 7, gfx 0x3F0E) at 39,16
Kanto_TwoIsland_EventScript_846BF95::
	setorcopyvar VAR_0x8000, ITEM_REVIVE
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_TwoIsland_Text_8431D4D::
	.string "TWO ISLAND\n"
	.string "Boon Island for Two$"

Kanto_TwoIsland_Text_8431D6C::
	.string "Skip & Chomp!\n"
	.string "Joyful GAME CORNER$"

Kanto_TwoIsland_Text_8431D8D::
	.string "DANGER! FAST CURRENT!\n"
	.string "Fishing and surfing prohibited!$"

Kanto_TwoIsland_Text_8431FF4::
	.string "That fellow there, he moved here\n"
	.string "from CELADON CITY.\p"
	.string "He started up a shop with his\n"
	.string "younger brother.\p"
	.string "You have to wonder if he knew what\n"
	.string "he was getting into.$"

Kanto_TwoIsland_Text_843208F::
	.string "The shopkeeper’s brother took\n"
	.string "a job at the GAME CORNER.\p"
	.string "He’s trying to pull together money\n"
	.string "so they can buy more merchandise.$"

Kanto_TwoIsland_Text_843210C::
	.string "Listen, listen.\n"
	.string "Did you know?\p"
	.string "A famous TRAINER came to the\n"
	.string "ISLAND a little while back.\p"
	.string "He’s one of the ELITE FOUR.\n"
	.string "His name is BRUNO.\p"
	.string "He went away disappointed when he\n"
	.string "found out that they were all sold\l"
	.string "out of Rage Candybars.$"

Kanto_TwoIsland_Text_84321ED::
	.string "The discerning TRAINER knows to\n"
	.string "buy rare items here!$"

Kanto_TwoIsland_Text_8432222::
	.string "There’s an old woman who lives in\n"
	.string "a cottage out on the cape.\p"
	.string "She gives the eye to MOéMON.\n"
	.string "It’s a bit unnerving.$"

Kanto_TwoIsland_Text_8432293::
	.string "Boing! Boing!\n"
	.string "Have you tried the jumping game?\p"
	.string "The MOéMON you can enter…\n"
	.string "Boing!\p"
	.string "Some go boing!\p"
	.string "And some go boooooooing!\p"
	.string "Did you know that?\n"
	.string "Boing!$"

Kanto_TwoIsland_Text_BringingItemsFromDistantLands::
	.string "Oh, hello, {PLAYER}!\n"
	.string "Welcome!\p"
	.string "I’ve started bringing in items from\n"
	.string "distant lands.\p"
	.string "I’ve got some rare items in, too.\n"
	.string "Please have a look!$"

Kanto_TwoIsland_Text_HopeYouGiveItYourBest::
	.string "Hi! I’m giving it my best here.\n"
	.string "I hope you do too, {PLAYER}.$"

Kanto_TwoIsland_Text_AddedMerchandiseForLostelle::
	.string "I can’t tell you how grateful I am\n"
	.string "for your rescue of LOSTELLE.\p"
	.string "Thanks to your feat, the people of\n"
	.string "THREE ISLAND have changed their\l"
	.string "attitudes about KANTO people.\p"
	.string "And, I’m from KANTO, you see.\p"
	.string "The people of THREE ISLAND\n"
	.string "helped me add to my merchandise.$"

Kanto_TwoIsland_Text_WelcomeToShopMerchandiseLimited::
	.string "Oh!\n"
	.string "Hello, welcome to my shop!\p"
	.string "We’ve only opened up recently.\p"
	.string "Our merchandise is limited, but I\n"
	.string "hope we can serve your needs.$"
