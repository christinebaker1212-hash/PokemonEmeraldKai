Kanto_SixIsland_WaterPath_House1_MapScripts::
	.byte 0

@ Moemon Kai: FireRed's HERACROSS record (pokefirered SixIsland_WaterPath_House1): the biggest one shown wins a NEST
@ BALL (FLAG_QZ_0x2158: a record set), measured by Emerald's size-record code (pokemon_size_record.c); Quetzal's
@ "show one, get a gift" is gone
@ Kanto_SixIsland_WaterPath_House1 object 1 (local id 1, gfx 0x3EE8) at 3,4; sign / bg event at 9,1
Kanto_SixIsland_WaterPath_House1_EventScript_8421DCF::
	lock
	faceplayer
	setvar VAR_0x8004, SPECIES_HERACROSS
	specialvar VAR_RESULT, QzPartyHasSpecies
	goto_if_eq VAR_RESULT, FALSE, Kanto_SixIsland_WaterPath_House1_EventScript_NoHeracrossInParty
	special GetHeracrossSizeRecordInfo
	msgbox Kanto_SixIsland_WaterPath_House1_Text_MayIMeasureHeracross, MSGBOX_DEFAULT
	special ChoosePartyMon
	waitstate
	copyvar VAR_RESULT, VAR_0x8004
	goto_if_ge VAR_RESULT, PARTY_SIZE, Kanto_SixIsland_WaterPath_House1_EventScript_DontShowMon
	special CompareHeracrossSize
	goto_if_eq VAR_RESULT, 1, Kanto_SixIsland_WaterPath_House1_EventScript_ShownNonHeracross
	goto_if_eq VAR_RESULT, 2, Kanto_SixIsland_WaterPath_House1_EventScript_ShownSmallHeracross
	goto_if_eq VAR_RESULT, 3, Kanto_SixIsland_WaterPath_House1_EventScript_ShownBigHeracross
	goto_if_eq VAR_RESULT, 4, Kanto_SixIsland_WaterPath_House1_EventScript_ShownTiedHeracross
	release
	end

Kanto_SixIsland_WaterPath_House1_EventScript_NoHeracrossInParty::
	msgbox Kanto_SixIsland_WaterPath_House1_Text_LoveItNeedItHeracross, MSGBOX_DEFAULT
	release
	end

Kanto_SixIsland_WaterPath_House1_EventScript_DontShowMon::
	release
	end

Kanto_SixIsland_WaterPath_House1_EventScript_ShownNonHeracross::
	msgbox Kanto_SixIsland_WaterPath_House1_Text_ThisWontDo, MSGBOX_DEFAULT
	release
	end

Kanto_SixIsland_WaterPath_House1_EventScript_ShownSmallHeracross::
	goto_if_unset FLAG_QZ_0x2158, Kanto_SixIsland_WaterPath_House1_EventScript_ShownBigHeracross
	msgbox Kanto_SixIsland_WaterPath_House1_Text_ItsXInchesYInchesWasBiggest, MSGBOX_DEFAULT
	release
	end

Kanto_SixIsland_WaterPath_House1_EventScript_ShownTiedHeracross::
	goto_if_unset FLAG_QZ_0x2158, Kanto_SixIsland_WaterPath_House1_EventScript_ShownBigHeracross
	msgbox Kanto_SixIsland_WaterPath_House1_Text_ItsXInchesSameAsBefore, MSGBOX_DEFAULT
	release
	end

Kanto_SixIsland_WaterPath_House1_EventScript_ShownBigHeracross::
	setflag FLAG_QZ_0x2158
	msgbox Kanto_SixIsland_WaterPath_House1_Text_ItsXInchesDeserveReward, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, ITEM_NEST_BALL
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, FALSE, Kanto_SixIsland_WaterPath_House1_EventScript_NoRoomForNestBall
	msgbox Kanto_SixIsland_WaterPath_House1_Text_WantToSeeBiggerOne, MSGBOX_DEFAULT
	release
	end

Kanto_SixIsland_WaterPath_House1_EventScript_NoRoomForNestBall::
	msgbox Kanto_SixIsland_WaterPath_House1_Text_YourBagIsFull, MSGBOX_DEFAULT
	release
	end

Kanto_SixIsland_WaterPath_House1_EventScript_8421EA2::
	lockall
	goto_if_set FLAG_QZ_0x2158, Kanto_SixIsland_WaterPath_House1_EventScript_SizeRecordNonEmpty
	msgbox Kanto_SixIsland_WaterPath_House1_Text_BlankChartOfSomeSort, MSGBOX_DEFAULT
	releaseall
	end

Kanto_SixIsland_WaterPath_House1_EventScript_SizeRecordNonEmpty::
	special GetHeracrossSizeRecordInfo
	msgbox Kanto_SixIsland_WaterPath_House1_Text_BiggestHeracrossIsXInches, MSGBOX_DEFAULT
	releaseall
	end

Kanto_SixIsland_WaterPath_House1_Text_BiggestHeracrossIsXInches::
	.string "The biggest HERACROSS that I have\n"
	.string "ever seen measured so far is:\p"
	.string "{STR_VAR_3} inches!$"

Kanto_SixIsland_WaterPath_House1_Text_BlankChartOfSomeSort::
	.string "It’s a blank chart of some sort.\p"
	.string "It has spaces for writing in\n"
	.string "records of some kind.$"

Kanto_SixIsland_WaterPath_House1_Text_ItsXInchesDeserveReward::
	.string "Eeeek, it’s {STR_VAR_2} inches!\n"
	.string "I’ve never seen anything like this!\l"
	.string "You deserve a reward!$"

Kanto_SixIsland_WaterPath_House1_Text_ItsXInchesSameAsBefore::
	.string "Huh? This {STR_VAR_2}-inch measurement…\n"
	.string "Oh, boo! It’s the same as before.$"

Kanto_SixIsland_WaterPath_House1_Text_ItsXInchesYInchesWasBiggest::
	.string "Oh, it’s just {STR_VAR_2} inches.\n"
	.string "The HERACROSS before was bigger.\p"
	.string "It was {STR_VAR_3} inches, the biggest\n"
	.string "HERACROSS you’ve brought me.$"

Kanto_SixIsland_WaterPath_House1_Text_LoveItNeedItHeracross::
	.string "Hera, hera, HERACROSS!\n"
	.string "Big and shiny, it’s the bug boss!\l"
	.string "Love it, need it, HERACROSS!$"

Kanto_SixIsland_WaterPath_House1_Text_MayIMeasureHeracross::
	.string "Eeeeek!\n"
	.string "That’s a HERACROSS!\p"
	.string "Please, please, may I measure how\n"
	.string "big it is?$"

Kanto_SixIsland_WaterPath_House1_Text_ThisWontDo::
	.string "Oh, no! This won’t do!\p"
	.string "HERACROSS looks much more macho\n"
	.string "and cool, and has a lovely horn!$"

Kanto_SixIsland_WaterPath_House1_Text_WantToSeeBiggerOne::
	.string "I want to see a much, much bigger\n"
	.string "HERACROSS than that one.\p"
	.string "Oh, how I adore them, big\n"
	.string "HERACROSS!$"

Kanto_SixIsland_WaterPath_House1_Text_YourBagIsFull::
	.string "Your BAG is full.\n"
	.string "My reward won’t fit.$"
