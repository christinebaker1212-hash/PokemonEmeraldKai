Kanto_Route12_FishingHouse_MapScripts::
	.byte 0

@ Kanto_Route12_FishingHouse object 1 (local id 1, gfx 0x3F06) at 4,4
Kanto_Route12_FishingHouse_EventScript_841FE95::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x2164, Kanto_Route12_FishingHouse_EventScript_841FF16
	msgbox Kanto_Route12_FishingHouse_Text_844F47A, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Kanto_Route12_FishingHouse_EventScript_841FEBD
	msgbox Kanto_Route12_FishingHouse_Text_844F694, MSGBOX_DEFAULT
	release
	end

Kanto_Route12_FishingHouse_EventScript_841FEBD::
	checkitem ITEM_SUPER_ROD, 1
	goto_if_eq VAR_RESULT, 1, Kanto_Route12_FishingHouse_EventScript_841FEFF
	additem ITEM_SUPER_ROD, 1
	goto_if_eq VAR_RESULT, 0, Kanto_Route12_FishingHouse_EventScript_841FF0C
	msgbox Kanto_Route12_FishingHouse_Text_844F4F5, MSGBOX_DEFAULT
	loadword 0, Kanto_Route12_FishingHouse_Text_844F54C
	setorcopyvar VAR_0x8000, ITEM_SUPER_ROD
	setorcopyvar VAR_0x8001, 1
	setorcopyvar VAR_0x8002, 0x16F
	call Quetzal_EventScript_845571E

Kanto_Route12_FishingHouse_EventScript_841FEFF::
	setflag FLAG_QZ_0x2164
	msgbox Kanto_Route12_FishingHouse_Text_IfYouCatchBigMagikarpShowMe, MSGBOX_DEFAULT
	release
	end

Kanto_Route12_FishingHouse_EventScript_841FF0C::
	msgbox Kanto_Route12_FishingHouse_Text_844F92E, MSGBOX_DEFAULT
	release
	end

Kanto_Route12_FishingHouse_EventScript_841FF16::
	@ Moemon Kai: FireRed's MAGIKARP record (pokefirered Route12_FishingHouse): the biggest one shown wins a NET BALL
	@ (FLAG_QZ_0x2160: a record set), measured by Emerald's size-record code (pokemon_size_record.c)
	setvar VAR_0x8004, SPECIES_MAGIKARP
	specialvar VAR_RESULT, QzPartyHasSpecies
	goto_if_eq VAR_RESULT, 0, Kanto_Route12_FishingHouse_EventScript_NoMagikarpInParty
	special GetMagikarpSizeRecordInfo
	msgbox Kanto_Route12_FishingHouse_Text_OhMagikarpAllowMeToSee, MSGBOX_DEFAULT
	special ChoosePartyMon
	waitstate
	copyvar VAR_RESULT, VAR_0x8004
	goto_if_ge VAR_RESULT, PARTY_SIZE, Kanto_Route12_FishingHouse_EventScript_CancelShowMon
	special CompareMagikarpSize
	goto_if_eq VAR_RESULT, 1, Kanto_Route12_FishingHouse_EventScript_NotMagikarp
	goto_if_eq VAR_RESULT, 2, Kanto_Route12_FishingHouse_EventScript_NotRecordMagikarp
	goto_if_eq VAR_RESULT, 3, Kanto_Route12_FishingHouse_EventScript_NewRecordMagikarp
	release
	end

Kanto_Route12_FishingHouse_EventScript_NoMagikarpInParty::
	msgbox Kanto_Route12_FishingHouse_Text_TryFishingBringMeMagikarp, MSGBOX_DEFAULT
	release
	end

Kanto_Route12_FishingHouse_EventScript_CancelShowMon::
	release
	end

Kanto_Route12_FishingHouse_EventScript_NotMagikarp::
	msgbox Kanto_Route12_FishingHouse_Text_DoesntLookLikeMagikarp, MSGBOX_DEFAULT
	release
	end

Kanto_Route12_FishingHouse_EventScript_NotRecordMagikarp::
	goto_if_unset FLAG_QZ_0x2160, Kanto_Route12_FishingHouse_EventScript_NewRecordMagikarp
	msgbox Kanto_Route12_FishingHouse_Text_HmmXInchesDoesntMeasureUp, MSGBOX_DEFAULT
	release
	end

Kanto_Route12_FishingHouse_EventScript_NewRecordMagikarp::
	setflag FLAG_QZ_0x2160
	msgbox Kanto_Route12_FishingHouse_Text_WhoaXInchesTakeThis, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, ITEM_NET_BALL
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Kanto_Route12_FishingHouse_EventScript_841FFDD
	msgbox Kanto_Route12_FishingHouse_Text_LookForwardToGreaterRecords, MSGBOX_DEFAULT
	release
	end

Kanto_Route12_FishingHouse_EventScript_841FFDD::
	msgbox Kanto_Route12_FishingHouse_Text_844F92E, MSGBOX_DEFAULT
	release
	end

@ Kanto_Route12_FishingHouse sign / bg event at 9,1
Kanto_Route12_FishingHouse_EventScript_841FFE7::
	lockall
	goto_if_set FLAG_QZ_0x2160, Kanto_Route12_FishingHouse_EventScript_RecordSignSet
	msgbox Kanto_Route12_FishingHouse_Text_BlankChartOfSomeSort, MSGBOX_DEFAULT
	releaseall
	end

Kanto_Route12_FishingHouse_EventScript_RecordSignSet::
	special GetMagikarpSizeRecordInfo
	msgbox Kanto_Route12_FishingHouse_Text_MostGiganticMagikarpXInches, MSGBOX_DEFAULT
	releaseall
	end

Kanto_Route12_FishingHouse_Text_844F47A::
	.string "I’m the FISHING GURU’s younger\n"
	.string "brother.\p"
	.string "I simply looove fishing!\n"
	.string "I can’t bear to go without.\p"
	.string "Tell me, do you like to fish?$"

Kanto_Route12_FishingHouse_Text_844F4F5::
	.string "Grand! I like your style.\n"
	.string "I think we can be friends.\p"
	.string "Take this and fish, young friend!$"

Kanto_Route12_FishingHouse_Text_844F54C::
	.string "{PLAYER} received a SUPER ROD from\n"
	.string "the FISHING GURU’s brother.$"

Kanto_Route12_FishingHouse_Text_844F694::
	.string "Oh…\n"
	.string "That’s so disappointing…$"

Kanto_Route12_FishingHouse_Text_844F92E::
	.string "Oh, no!\p"
	.string "I had a gift for you, but you have\n"
	.string "no room for it.$"

Kanto_Route12_FishingHouse_Text_OhMagikarpAllowMeToSee::
	.string "Oh? {PLAYER}?\n"
	.string "Why, if it isn’t a MAGIKARP!\p"
	.string "Allow me to see it, quick!$"

Kanto_Route12_FishingHouse_Text_TryFishingBringMeMagikarp::
	.string "Hello there, {PLAYER}!\n"
	.string "Have you been fishing?\p"
	.string "Try fishing with the SUPER ROD in\n"
	.string "any body of water.\p"
	.string "You’ll find different MOéMON in\n"
	.string "different places.\p"
	.string "Oh, and don’t forget to bring me\n"
	.string "gigantic MAGIKARP.$"

Kanto_Route12_FishingHouse_Text_DoesntLookLikeMagikarp::
	.string "Uh… That doesn’t look much like\n"
	.string "a MAGIKARP.$"

Kanto_Route12_FishingHouse_Text_HmmXInchesDoesntMeasureUp::
	.string "Hmm…\n"
	.string "This one is {STR_VAR_2} inches long.\p"
	.string "It doesn’t measure up to the\n"
	.string "{STR_VAR_3}-inch one you brought before.$"

Kanto_Route12_FishingHouse_Text_WhoaXInchesTakeThis::
	.string "… … …Whoa!\n"
	.string "{STR_VAR_2} inches!\p"
	.string "You have a rare appreciation for\n"
	.string "the fine, poetic aspects of fishing!\p"
	.string "You must take this.\n"
	.string "I insist!$"

Kanto_Route12_FishingHouse_Text_LookForwardToGreaterRecords::
	.string "I’ll look forward to seeing greater\n"
	.string "records from you!$"

Kanto_Route12_FishingHouse_Text_BlankChartOfSomeSort::
	.string "It’s a blank chart of some sort.\p"
	.string "It has spaces for writing in\n"
	.string "records of some kind.$"

Kanto_Route12_FishingHouse_Text_MostGiganticMagikarpXInches::
	.string "The most gigantic MAGIKARP\n"
	.string "I have ever witnessed…\p"
	.string "{STR_VAR_3} inches!$"

Kanto_Route12_FishingHouse_Text_IfYouCatchBigMagikarpShowMe::
	.string "Fishing is a way of life!\n"
	.string "It is like the finest poetry.\p"
	.string "From the seas to rivers, go out\n"
	.string "and land the big one, my friend.\p"
	.string "Now, I have a request.\p"
	.string "If you catch a big MAGIKARP \n"
	.string "with that ROD, I want to see it.\p"
	.string "As much as I love to fish, I also\n"
	.string "love seeing gigantic MAGIKARP.$"
