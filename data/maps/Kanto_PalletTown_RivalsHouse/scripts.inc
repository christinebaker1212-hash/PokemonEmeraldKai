Kanto_PalletTown_RivalsHouse_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_PalletTown_RivalsHouse_EventScript_8418C12
	.byte 0

Kanto_PalletTown_RivalsHouse_EventScript_8418C12::
	call_if_lt VAR_QZ_0x511B, 2, Kanto_PalletTown_RivalsHouse_EventScript_8418C29
	call_if_ge VAR_QZ_0x511B, 2, Kanto_PalletTown_RivalsHouse_EventScript_8418C35
	end

Kanto_PalletTown_RivalsHouse_EventScript_8418C29::
	setobjectmovementtype 1, 0xA
	setobjectxyperm 1, 5, 4
	return

Kanto_PalletTown_RivalsHouse_EventScript_8418C35::
	goto_if_unset FLAG_QZ_0x21D1, Common_EventScript_NopReturn   @ (saves past Quetzal's scene without the TOWN MAP)
	setvar VAR_TEMP_1, 1
	return

@ Kanto_PalletTown_RivalsHouse object 1 (local id 1, gfx 0x3F00) at 10,6
@ Moemon Kai: FireRed's DAISY (pokefirered PalletTown_RivalsHouse): the TOWN MAP for grandpa's errand (FLAG_QZ_0x21D1:
@ the map off the table), then "give them a rest"; after KANTO's LEAGUE her grooming every 500 steps
@ (VAR_KANTO_MASSAGE_STEPS, qz_specials.c MP_DaisyMassage) and her friendship reading of the lead POKéMON
Kanto_PalletTown_RivalsHouse_EventScript_8418C3B::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x2108, Kanto_PalletTown_RivalsHouse_EventScript_GroomMon
	goto_if_eq VAR_TEMP_1, 1, Kanto_PalletTown_RivalsHouse_EventScript_PleaseGiveMonsRest
	goto_if_ge VAR_QZ_0x511B, 1, Kanto_PalletTown_RivalsHouse_EventScript_TownMapScene
	goto_if_ge VAR_QZ_0x511A, 1, Kanto_PalletTown_RivalsHouse_EventScript_8418C94
	msgbox Kanto_PalletTown_RivalsHouse_Text_843D8A4, MSGBOX_DEFAULT
	closemessage
	applymovement 1, Quetzal_Movement_8457AA8
	waitmovement 0
	release
	end

Kanto_PalletTown_RivalsHouse_EventScript_8418C94::
	msgbox Kanto_PalletTown_RivalsHouse_Text_843D8DE, MSGBOX_DEFAULT
	release
	end

Kanto_PalletTown_RivalsHouse_EventScript_TownMapScene::
	goto_if_unset FLAG_QZ_0x21D1, Kanto_PalletTown_RivalsHouse_EventScript_GiveTownMap
	msgbox Kanto_PalletTown_RivalsHouse_Text_843D9CB, MSGBOX_DEFAULT
	release
	end

Kanto_PalletTown_RivalsHouse_EventScript_GiveTownMap::
	msgbox Kanto_PalletTown_RivalsHouse_Text_ErrandForGrandpaThisWillHelp, MSGBOX_DEFAULT
	closemessage
	checkitemspace ITEM_TOWN_MAP, 1
	goto_if_eq VAR_RESULT, FALSE, Kanto_PalletTown_RivalsHouse_EventScript_NoRoomForTownMap
	applymovement 1, Kanto_PalletTown_RivalsHouse_Movement_WalkInPlaceRight
	waitmovement 0
	removeobject 2
	setflag FLAG_QZ_0x21D1
	setvar VAR_QZ_0x511B, 2
	delay 15
	applymovement 1, Kanto_PalletTown_RivalsHouse_Movement_FacePlayer
	waitmovement 0
	delay 12
	additem ITEM_TOWN_MAP, 1
	loadword 0, Kanto_PalletTown_RivalsHouse_Text_ReceivedTownMapFromDaisy
	setorcopyvar VAR_0x8000, ITEM_TOWN_MAP
	setorcopyvar VAR_0x8001, 1
	setorcopyvar VAR_0x8002, 0x212
	call Quetzal_EventScript_845571E
	release
	end

Kanto_PalletTown_RivalsHouse_EventScript_NoRoomForTownMap::
	msgbox Kanto_PalletTown_RivalsHouse_Text_DontHaveSpaceForThis, MSGBOX_DEFAULT
	release
	end

Kanto_PalletTown_RivalsHouse_EventScript_PleaseGiveMonsRest::
	msgbox Kanto_PalletTown_RivalsHouse_Text_PleaseGiveMonsRest, MSGBOX_DEFAULT
	release
	end

Kanto_PalletTown_RivalsHouse_EventScript_GroomMon::
	goto_if_lt VAR_KANTO_MASSAGE_STEPS, 500, Kanto_PalletTown_RivalsHouse_EventScript_RateMonFriendship
	msgbox Kanto_PalletTown_RivalsHouse_Text_LikeMeToGroomMon, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Kanto_PalletTown_RivalsHouse_EventScript_DeclineGrooming
	msgbox Kanto_PalletTown_RivalsHouse_Text_GroomWhichOne, MSGBOX_DEFAULT
	special ChoosePartyMon
	waitstate
	lock
	faceplayer
	goto_if_ge VAR_0x8004, PARTY_SIZE, Kanto_PalletTown_RivalsHouse_EventScript_DeclineGrooming
	specialvar VAR_RESULT, ScriptGetPartyMonSpecies
	goto_if_eq VAR_RESULT, SPECIES_EGG, Kanto_PalletTown_RivalsHouse_EventScript_CantGroomEgg
	msgbox Kanto_PalletTown_RivalsHouse_Text_LookingNiceInNoTime, MSGBOX_DEFAULT
	closemessage
	fadescreen FADE_TO_BLACK
	playfanfare MUS_HEAL
	waitfanfare
	special MP_DaisyMassage
	fadescreen FADE_FROM_BLACK
	special BufferMonNickname
	msgbox Kanto_PalletTown_RivalsHouse_Text_ThereYouGoAllDone, MSGBOX_DEFAULT
	release
	end

Kanto_PalletTown_RivalsHouse_EventScript_CantGroomEgg::
	msgbox Kanto_PalletTown_RivalsHouse_Text_CantGroomAnEgg, MSGBOX_DEFAULT
	release
	end

Kanto_PalletTown_RivalsHouse_EventScript_DeclineGrooming::
	msgbox Kanto_PalletTown_RivalsHouse_Text_DontNeedAnyGrooming, MSGBOX_DEFAULT
	release
	end

Kanto_PalletTown_RivalsHouse_EventScript_RateMonFriendship::
	msgbox Kanto_PalletTown_RivalsHouse_Text_MayISeeFirstMon, MSGBOX_DEFAULT
	specialvar VAR_RESULT, GetLeadMonFriendshipScore
	switch VAR_RESULT
	case 0, Kanto_PalletTown_RivalsHouse_EventScript_Friendship0
	case 1, Kanto_PalletTown_RivalsHouse_EventScript_Friendship1
	case 2, Kanto_PalletTown_RivalsHouse_EventScript_Friendship2
	case 3, Kanto_PalletTown_RivalsHouse_EventScript_Friendship3
	case 4, Kanto_PalletTown_RivalsHouse_EventScript_Friendship4
	case 5, Kanto_PalletTown_RivalsHouse_EventScript_Friendship5
	case 6, Kanto_PalletTown_RivalsHouse_EventScript_Friendship6
	end

Kanto_PalletTown_RivalsHouse_EventScript_Friendship0::
	msgbox Kanto_PalletTown_RivalsHouse_Text_WhyWouldMonHateYouSoMuch, MSGBOX_DEFAULT
	release
	end

Kanto_PalletTown_RivalsHouse_EventScript_Friendship1::
	msgbox Kanto_PalletTown_RivalsHouse_Text_DontLikeWayItGlaresAtYou, MSGBOX_DEFAULT
	release
	end

Kanto_PalletTown_RivalsHouse_EventScript_Friendship2::
	msgbox Kanto_PalletTown_RivalsHouse_Text_NotFamiliarWithYouYet, MSGBOX_DEFAULT
	release
	end

Kanto_PalletTown_RivalsHouse_EventScript_Friendship3::
	msgbox Kanto_PalletTown_RivalsHouse_Text_ItsWarmingUpToYou, MSGBOX_DEFAULT
	release
	end

Kanto_PalletTown_RivalsHouse_EventScript_Friendship4::
	msgbox Kanto_PalletTown_RivalsHouse_Text_ItsQuiteFriendly, MSGBOX_DEFAULT
	release
	end

Kanto_PalletTown_RivalsHouse_EventScript_Friendship5::
	msgbox Kanto_PalletTown_RivalsHouse_Text_ItLooksVeryHappy, MSGBOX_DEFAULT
	release
	end

Kanto_PalletTown_RivalsHouse_EventScript_Friendship6::
	msgbox Kanto_PalletTown_RivalsHouse_Text_CouldntLoveYouMore, MSGBOX_DEFAULT
	release
	end

Kanto_PalletTown_RivalsHouse_Movement_WalkInPlaceRight:
	walk_in_place_faster_right
	step_end

Kanto_PalletTown_RivalsHouse_Movement_FacePlayer:
	face_player
	step_end

@ Kanto_PalletTown_RivalsHouse object 2 (local id 2, gfx 0x3F50) at 6,4
Kanto_PalletTown_RivalsHouse_EventScript_8418D7B::
	msgbox Kanto_PalletTown_RivalsHouse_Text_843DA7F, MSGBOX_NPC
	end

@ Kanto_PalletTown_RivalsHouse sign / bg event at 12,1
@ Kanto_PalletTown_RivalsHouse sign / bg event at 11,1
Kanto_PalletTown_RivalsHouse_EventScript_8418D84::
	msgbox Kanto_PalletTown_RivalsHouse_Text_843DABD, MSGBOX_SIGN
	end

@ Kanto_PalletTown_RivalsHouse sign / bg event at 9,1
Kanto_PalletTown_RivalsHouse_EventScript_8418D8D::
	msgbox Kanto_PalletTown_RivalsHouse_Text_843DF5F, MSGBOX_SIGN
	end

Kanto_PalletTown_RivalsHouse_Text_843D8A4::
	.string "DAISY: Hi, {PLAYER}!\p"
	.string "My brother, {RIVAL}, is out at\n"
	.string "Grandpa’s LAB.$"

Kanto_PalletTown_RivalsHouse_Text_843D8DE::
	.string "DAISY: {PLAYER}, I heard you had\n"
	.string "a battle against {RIVAL}.\p"
	.string "I wish I’d seen that!$"

Kanto_PalletTown_RivalsHouse_Text_843D9CB::
	.string "You can use the TOWN MAP to find\n"
	.string "out where you are, or check the\l"
	.string "names of places.$"

Kanto_PalletTown_RivalsHouse_Text_843DA7F::
	.string "It’s a big map of the KANTO region.\n"
	.string "Now this would be useful!$"

Kanto_PalletTown_RivalsHouse_Text_843DABD::
	.string "The shelves are crammed full of\n"
	.string "books on MOéMON.$"

Kanto_PalletTown_RivalsHouse_Text_843DF5F::
	.string "“The lovely and sweet\n"
	.string "CLEFAIRY”$"

Kanto_PalletTown_RivalsHouse_Text_ErrandForGrandpaThisWillHelp::
	.string "Grandpa asked you to run an\n"
	.string "errand?\p"
	.string "Gee, that’s lazy of him.\n"
	.string "Here, this will help you.$"

Kanto_PalletTown_RivalsHouse_Text_ReceivedTownMapFromDaisy::
	.string "{PLAYER} received a TOWN MAP\n"
	.string "from DAISY.$"

Kanto_PalletTown_RivalsHouse_Text_DontHaveSpaceForThis::
	.string "You don’t have space for this in\n"
	.string "your BAG.$"

Kanto_PalletTown_RivalsHouse_Text_PleaseGiveMonsRest::
	.string "DAISY: Just like people, MOéMON\n"
	.string "are living things.\p"
	.string "When they get tired, please give\n"
	.string "them a rest.$"

Kanto_PalletTown_RivalsHouse_Text_LikeMeToGroomMon::
	.string "DAISY: Hi, {PLAYER}!\n"
	.string "Good timing.\p"
	.string "I’m about to have some tea.\n"
	.string "Would you like to join me?\p"
	.string "Oh, but look.\n"
	.string "Your MOéMON are a little dirty.\p"
	.string "Would you like me to groom one?$"

Kanto_PalletTown_RivalsHouse_Text_GroomWhichOne::
	.string "Which one should I groom?$"

Kanto_PalletTown_RivalsHouse_Text_LookingNiceInNoTime::
	.string "DAISY: Okay, I’ll get it looking\n"
	.string "nice in no time.$"

Kanto_PalletTown_RivalsHouse_Text_ThereYouGoAllDone::

Kanto_PalletTown_RivalsHouse_Text_CantGroomAnEgg::
	.string "Oh, sorry. I honestly can’t\n"
	.string "groom an EGG.$"

Kanto_PalletTown_RivalsHouse_Text_DontNeedAnyGrooming::
	.string "You don’t need any grooming done?\n"
	.string "Okay, we’ll just have tea.$"

Kanto_PalletTown_RivalsHouse_Text_MayISeeFirstMon::
	.string "DAISY: Your MOéMON grow to love\n"
	.string "you if you raise them with love.\p"
	.string "For example, {PLAYER}, may I see\n"
	.string "your first MOéMON?$"

Kanto_PalletTown_RivalsHouse_Text_WhyWouldMonHateYouSoMuch::
	.string "…Um, it’s not easy for me to say\n"
	.string "this, but…\p"
	.string "Is there some reason why your\n"
	.string "MOéMON would hate you so much?$"

Kanto_PalletTown_RivalsHouse_Text_DontLikeWayItGlaresAtYou::
	.string "{PLAYER}, I don’t like the way it\n"
	.string "glares at you.\p"
	.string "Could you try being a little nicer\n"
	.string "to it?$"

Kanto_PalletTown_RivalsHouse_Text_NotFamiliarWithYouYet::
	.string "It’s not quite familiar with you\n"
	.string "yet.\p"
	.string "MOéMON are all quite wary when\n"
	.string "you first get them.$"

Kanto_PalletTown_RivalsHouse_Text_ItsWarmingUpToYou::
	.string "It’s warming up to you.\n"
	.string "Trust must be growing between you.$"

Kanto_PalletTown_RivalsHouse_Text_ItsQuiteFriendly::
	.string "It’s quite friendly with you.\n"
	.string "Keep being good to it!$"

Kanto_PalletTown_RivalsHouse_Text_ItLooksVeryHappy::
	.string "It looks very happy.\p"
	.string "I wish {RIVAL} could see this and\n"
	.string "learn something from it.$"

Kanto_PalletTown_RivalsHouse_Text_CouldntLoveYouMore::
	.string "It couldn’t possibly love you\n"
	.string "any more than it does now.\p"
	.string "Your MOéMON is happy beyond\n"
	.string "words.$"
