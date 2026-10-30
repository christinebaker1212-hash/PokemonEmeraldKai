Quetzal_EventScript_835F6E4::
	specialvar VAR_RESULT, QzSaveLeagueVictoryRecord_712
	goto_if_eq VAR_RESULT, 1, Quetzal_EventScript_835F702
	msgbox Quetzal_Text_835F703, MSGBOX_DEFAULT
	closemessage
	goto Quetzal_EventScript_835F6E4

Quetzal_EventScript_835F702::
	return

Quetzal_EventScript_835FB62::
	bufferitemnameplural 1, VAR_0x8000, VAR_0x8001
	checkitemtype VAR_0x8000
	call Quetzal_EventScript_835FB87
	call_if_eq VAR_0x8007, 1, Quetzal_EventScript_835FCC6
	call_if_eq VAR_0x8007, 0, Quetzal_EventScript_835FCDA
	return

Quetzal_EventScript_835FB87::
	copyvar VAR_0x8000, VAR_RESULT
	goto_if_eq VAR_0x8000, 1, Quetzal_EventScript_835FC06
	goto_if_eq VAR_0x8000, 2, Quetzal_EventScript_835FC56
	goto_if_eq VAR_0x8000, 3, Quetzal_EventScript_835FC26
	goto_if_eq VAR_0x8000, 4, Quetzal_EventScript_835FC46
	goto_if_eq VAR_0x8000, 5, Quetzal_EventScript_835FC66
	goto_if_eq VAR_0x8000, 6, Quetzal_EventScript_835FC76
	goto_if_eq VAR_0x8000, 7, Quetzal_EventScript_835FC86
	goto_if_eq VAR_0x8000, 8, Quetzal_EventScript_835FC96
	goto_if_eq VAR_0x8000, 9, Quetzal_EventScript_835FCA6
	goto_if_eq VAR_0x8000, 10, Quetzal_EventScript_835FCB6
	goto_if_eq VAR_0x8000, 11, Quetzal_EventScript_835FC16
	end

Quetzal_EventScript_835FC06::
	bufferstdstring 2, 0xE
	call_if_eq VAR_0x8007, 1, Quetzal_EventScript_835FCE0
	return

Quetzal_EventScript_835FC16::
	bufferstdstring 2, 0xF
	call_if_eq VAR_0x8007, 1, Quetzal_EventScript_835FCE0
	return

Quetzal_EventScript_835FC26::
	bufferstdstring 2, 0x10
	call_if_eq VAR_0x8007, 1, Quetzal_EventScript_835FCE0
	return

Quetzal_EventScript_835FC46::
	bufferstdstring 2, 0x12
	call_if_eq VAR_0x8007, 1, Quetzal_EventScript_835FCE0
	return

Quetzal_EventScript_835FC56::
	bufferstdstring 2, 0x1E
	call_if_eq VAR_0x8007, 1, Quetzal_EventScript_835FCE0
	return

Quetzal_EventScript_835FC66::
	bufferstdstring 2, 0x1F
	call_if_eq VAR_0x8007, 1, Quetzal_EventScript_835FCE0
	return

Quetzal_EventScript_835FC76::
	bufferstdstring 2, 0x20
	call_if_eq VAR_0x8007, 1, Quetzal_EventScript_835FCE0
	return

Quetzal_EventScript_835FC86::
	bufferstdstring 2, 0x21
	call_if_eq VAR_0x8007, 1, Quetzal_EventScript_835FCE0
	return

Quetzal_EventScript_835FC96::
	bufferstdstring 2, 0x22
	call_if_eq VAR_0x8007, 1, Quetzal_EventScript_835FCE0
	return

Quetzal_EventScript_835FCA6::
	bufferstdstring 2, 0x23
	call_if_eq VAR_0x8007, 1, Quetzal_EventScript_835FCE0
	return

Quetzal_EventScript_835FCB6::
	bufferstdstring 2, 0x24
	call_if_eq VAR_0x8007, 1, Quetzal_EventScript_835FCE0
	return

Quetzal_EventScript_835FCC6::
	message Quetzal_Text_8360D59
	waitfanfare
	msgbox Quetzal_Text_8360D7B, MSGBOX_DEFAULT
	setvar VAR_RESULT, 1
	return

Quetzal_EventScript_835FCDA::
	setvar VAR_RESULT, 0
	return

Quetzal_EventScript_835FCE0::
	playfanfare MUS_OBTAIN_ITEM
	return

Quetzal_EventScript_835FD2E::
	lock
	faceplayer
	waitse
	msgbox Quetzal_Text_835FF09, MSGBOX_DEFAULT
	removeobject VAR_LAST_TALKED
	release
	end

Quetzal_EventScript_835FF25::
	setvar VAR_0x8000, ITEM_POKE_BALL   @ Moemon Kai: Quetzal's item 1 (a number here: not translated by the import)
	setvar VAR_0x8001, 5
	additem VAR_0x8000, VAR_0x8001
	copyvar VAR_0x8007, VAR_RESULT
	call Quetzal_EventScript_835FB62
	return

Quetzal_EventScript_835FF3F::
	special QzIncrementChallengeCounterVar_778
	return

Quetzal_EventScript_8361A4D::
	specialvar VAR_RESULT, QzIsNuzlockeActiveInRegion_779
	goto_if_eq VAR_RESULT, 1, Quetzal_EventScript_8361A5E
	return

Quetzal_EventScript_8361A5E::
	msgbox Quetzal_Text_8361A68, MSGBOX_DEFAULT
	releaseall
	end

Quetzal_EventScript_8361ADB::
	special QzBufferLastReceivedMonForNickname_831
	goto_if_ne VAR_0x8004, 6, Quetzal_EventScript_8361AF1
	msgbox Quetzal_Text_8361AF9, MSGBOX_DEFAULT

Quetzal_EventScript_8361AF1::
	return

Quetzal_EventScript_8361AF2::
	fadescreen 1
	special QzNicknameLastReceivedMon_832
	waitstate
	return

Quetzal_EventScript_8361B27::
	bufferboxname 0, VAR_QZ_0x4036
	bufferspeciesname 1, VAR_TEMP_1
	call_if_unset FLAG_QZ_0xAC3, Quetzal_EventScript_8361B42
	call_if_set FLAG_QZ_0xAC3, Quetzal_EventScript_8361B6D
	return

Quetzal_EventScript_8361B42::
	specialvar VAR_RESULT, ShouldShowBoxWasFullMessage
	goto_if_eq VAR_RESULT, 1, Quetzal_EventScript_8361B5B
	msgbox Quetzal_Text_83614F7, MSGBOX_DEFAULT
	return

Quetzal_EventScript_8361B5B::
	specialvar VAR_RESULT, GetPCBoxToSendMon
	bufferboxname 2, VAR_RESULT
	msgbox Quetzal_Text_8361577, MSGBOX_DEFAULT
	return

Quetzal_EventScript_8361B6D::
	specialvar VAR_RESULT, ShouldShowBoxWasFullMessage
	goto_if_eq VAR_RESULT, 1, Quetzal_EventScript_8361B86
	msgbox Quetzal_Text_8361537, MSGBOX_DEFAULT
	return

Quetzal_EventScript_8361B86::
	specialvar VAR_RESULT, GetPCBoxToSendMon
	bufferboxname 2, VAR_RESULT
	msgbox Quetzal_Text_83615BA, MSGBOX_DEFAULT
	return

Quetzal_EventScript_8361B98::
	msgbox Quetzal_Text_8361BA2, MSGBOX_DEFAULT
	release
	end

Quetzal_EventScript_8361BCF::
	msgbox Quetzal_Text_83615FD, MSGBOX_DEFAULT
	release
	end

Quetzal_EventScript_8365E4C::
	special InitUnionRoom
	end

Quetzal_EventScript_837EFEA::
	setflag FLAG_QZ_0xAA1
	releaseall
	end

Quetzal_EventScript_83C2D35::
	playfanfare MUS_OBTAIN_ITEM
	message Quetzal_Text_83C32AA
	waitmessage
	waitfanfare
	bufferspeciesname 0, SPECIES_PIKACHU
	return

@ Johto_Route34 object 12 (local id 12, gfx 0x3EA6) at 31,34
Quetzal_EventScript_83F08A1::
	lock
	faceplayer
	msgbox Quetzal_Text_83F0C34, MSGBOX_DEFAULT
	release
	end

@ Johto_DayCare object 1 (local id 1, gfx 0x1E) at 3,8
Quetzal_EventScript_83F0976::
	lock
	faceplayer
	message Quetzal_Text_83F09A4
	waitmessage
	pokemart Quetzal_Mart_83F0990
	msgbox Quetzal_Text_8360D20, MSGBOX_DEFAULT
	release
	end

Quetzal_EventScript_840F954::
	specialvar VAR_RESULT, QzGetReachableRegionsMask_782
	copyvar VAR_0x8000, VAR_RESULT
	goto_if_eq VAR_0x8000, 1, Quetzal_EventScript_840F9AD
	goto_if_eq VAR_0x8000, 2, Quetzal_EventScript_840F9C9
	goto_if_eq VAR_0x8000, 3, Quetzal_EventScript_840F9E5
	goto_if_eq VAR_0x8000, 4, Quetzal_EventScript_840FA11
	goto_if_eq VAR_0x8000, 5, Quetzal_EventScript_840FA2D
	goto_if_eq VAR_0x8000, 6, Quetzal_EventScript_840FA59
	msgbox Quetzal_Text_840FB2D, MSGBOX_DEFAULT
	goto Quetzal_EventScript_840FA97

Quetzal_EventScript_840F9AD::
	message Quetzal_Text_840FAE5
	waitmessage
	multichoice 0x10, 2, 0x8E, 1
	nop
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_840FA85
	goto Quetzal_EventScript_840FA97

Quetzal_EventScript_840F9C9::
	message Quetzal_Text_840FAE5
	waitmessage
	multichoice 0x10, 2, 0x8F, 1
	nop
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_840FA8B
	goto Quetzal_EventScript_840FA97

Quetzal_EventScript_840F9E5::
	message Quetzal_Text_840FAE5
	waitmessage
	multichoice 0x10, 2, 0x90, 1
	nop
	copyvar VAR_0x8000, VAR_RESULT
	goto_if_eq VAR_0x8000, 0, Quetzal_EventScript_840FA8B
	goto_if_eq VAR_0x8000, 1, Quetzal_EventScript_840FA85
	goto Quetzal_EventScript_840FA97

Quetzal_EventScript_840FA11::
	message Quetzal_Text_840FAE5
	waitmessage
	multichoice 0x10, 2, 0x91, 1
	nop
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_840FA91
	goto Quetzal_EventScript_840FA97

Quetzal_EventScript_840FA2D::
	message Quetzal_Text_840FAE5
	waitmessage
	multichoice 0x10, 2, 0x92, 1
	nop
	copyvar VAR_0x8000, VAR_RESULT
	goto_if_eq VAR_0x8000, 0, Quetzal_EventScript_840FA91
	goto_if_eq VAR_0x8000, 1, Quetzal_EventScript_840FA85
	goto Quetzal_EventScript_840FA97

Quetzal_EventScript_840FA59::
	message Quetzal_Text_840FAE5
	waitmessage
	multichoice 0x10, 2, 0x93, 1
	nop
	copyvar VAR_0x8000, VAR_RESULT
	goto_if_eq VAR_0x8000, 0, Quetzal_EventScript_840FA91
	goto_if_eq VAR_0x8000, 1, Quetzal_EventScript_840FA8B
	goto Quetzal_EventScript_840FA97

Quetzal_EventScript_840FA85::
	setvar VAR_RESULT, 0
	return

Quetzal_EventScript_840FA8B::
	setvar VAR_RESULT, 1
	return

Quetzal_EventScript_840FA91::
	setvar VAR_RESULT, 2
	return

Quetzal_EventScript_840FA97::
	setvar VAR_RESULT, 0x7F
	return

Quetzal_EventScript_840FA9D::
	copyvar VAR_0x8004, VAR_RESULT
	msgbox Quetzal_Text_840FB09, MSGBOX_DEFAULT
	closemessage
	specialvar VAR_RESULT, QzTravelToRegion_783
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_840FABD
	waitstate
	end

Quetzal_EventScript_840FABD::
	msgbox Quetzal_Text_840FB2D, MSGBOX_DEFAULT
	release
	end

Quetzal_EventScript_841C97A::
	showmoneybox 0, 0, 0
	checkcoins VAR_RESULT
	buffernumberstring 0, VAR_RESULT
	message Quetzal_Text_8446AB2
	waitmessage
	multichoice 0xD, 0, 0x60, 1
	nop
	copyvar VAR_0x8009, VAR_RESULT
	copyvar VAR_0x8000, VAR_RESULT
	goto_if_eq VAR_0x8000, 0, Quetzal_EventScript_841C9C1
	goto_if_eq VAR_0x8000, 1, Quetzal_EventScript_841C9C1
	goto_if_eq VAR_0x8000, 127, Quetzal_EventScript_841CA6A
	goto Quetzal_EventScript_841CA6A

Quetzal_EventScript_841C9C1::
	checkitem ITEM_COIN_CASE, 1
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_841CA78
	goto_if_eq VAR_0x8009, 0, Quetzal_EventScript_841CA1A
	goto_if_eq VAR_0x8009, 1, Quetzal_EventScript_841C9EC
	goto Quetzal_EventScript_841CA6A

Quetzal_EventScript_841C9EC::
	checkcoins VAR_TEMP_1
	goto_if_ge VAR_TEMP_1, 9500, Quetzal_EventScript_841CA86
	checkmoney 0x2710, 0
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_841CA94
	addcoins 0x1F4
	removemoney 0x2710, 0
	goto Quetzal_EventScript_841CA48

Quetzal_EventScript_841CA1A::
	checkcoins VAR_TEMP_1
	goto_if_ge VAR_TEMP_1, 9950, Quetzal_EventScript_841CA86
	checkmoney 0x3E8, 0
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_841CA94
	addcoins 0x32
	removemoney 0x3E8, 0
	goto Quetzal_EventScript_841CA48

Quetzal_EventScript_841CA48::
	updatemoneybox 0
	checkcoins VAR_RESULT
	buffernumberstring 0, VAR_RESULT
	playse SE_SHOP
	msgbox Quetzal_Text_8446C00, MSGBOX_DEFAULT
	goto Quetzal_EventScript_841CA64

Quetzal_EventScript_841CA64::
	hidemoneybox
	closemessage
	release
	end

Quetzal_EventScript_841CA6A::
	msgbox Quetzal_Text_8446B7A, MSGBOX_DEFAULT
	goto Quetzal_EventScript_841CA64

Quetzal_EventScript_841CA78::
	msgbox Quetzal_Text_8446B99, MSGBOX_DEFAULT
	goto Quetzal_EventScript_841CA64

Quetzal_EventScript_841CA86::
	msgbox Quetzal_Text_8446BC4, MSGBOX_DEFAULT
	goto Quetzal_EventScript_841CA64

Quetzal_EventScript_841CA94::
	msgbox Quetzal_Text_8446BE4, MSGBOX_DEFAULT
	goto Quetzal_EventScript_841CA64

Quetzal_EventScript_841CE7A::
	checkcoins VAR_RESULT
	buffernumberstring 0, VAR_RESULT
	message Quetzal_Text_8446A8F
	waitmessage
	return

Quetzal_Movement_83607F9:
	emote_exclamation_mark
	step_end

Quetzal_Movement_83607FB:
	delay_16
	delay_16
	delay_16
	step_end

Quetzal_Mart_83F0990:
	.2byte ITEM_SEA_INCENSE
	.2byte ITEM_LAX_INCENSE
	.2byte ITEM_ODD_INCENSE
	.2byte ITEM_ROCK_INCENSE
	.2byte ITEM_FULL_INCENSE
	.2byte ITEM_WAVE_INCENSE
	.2byte ITEM_ROSE_INCENSE
	.2byte ITEM_LUCK_INCENSE
	.2byte ITEM_PURE_INCENSE
	.2byte ITEM_NONE

Quetzal_Text_82F8280::
	.string "Oh, now that’s a surprise!\n"
	.string "You need one more fossil!\p"
	.string "Please select another fossil\n"
	.string "to combine.$"

Quetzal_Text_835F703::
	.string "The League could not be recorded.\n"
	.string "The game has not been saved.\p"
	.string "Press A or B to try again.\n"
	.string "If this persists, restart the game.$"

Quetzal_Text_835FF09::
	.string "You already have this item.$"

Quetzal_Text_8360D02::
	.string "Welcome!\p"
	.string "How may I serve you?$"

Quetzal_Text_8360D20::
	.string "Please come again!$"

Quetzal_Text_8360D59::
	.string "Obtained the {STR_VAR_2}!$"

Quetzal_Text_8360D7B::
	.string "{PLAYER} put away the {STR_VAR_2}\n"
	.string "in the {STR_VAR_3} POCKET.$"

Quetzal_Text_8360DB1::
	.string "Too bad!\n"
	.string "The BAG is full…$"

Quetzal_Text_83614F7::
	.string "{STR_VAR_2} was transferred to\n"
	.string "SOMEONE’S PC.\p"
	.string "It was placed in \n"
	.string "BOX “{STR_VAR_1}.”$"

Quetzal_Text_8361537::
	.string "{STR_VAR_2} was transferred to\n"
	.string "LANETTE’S PC.\p"
	.string "It was placed in \n"
	.string "BOX “{STR_VAR_1}.”$"

Quetzal_Text_8361577::
	.string "BOX “{STR_VAR_3}” on\n"
	.string "SOMEONE’S PC was full.\p"
	.string "{STR_VAR_2} was transferred to\n"
	.string "BOX “{STR_VAR_1}.”$"

Quetzal_Text_83615BA::
	.string "BOX “{STR_VAR_3}” on\n"
	.string "LANETTE’S PC was full.\p"
	.string "{STR_VAR_2} was transferred to\n"
	.string "BOX “{STR_VAR_1}.”$"

Quetzal_Text_83615FD::
	.string "There’s no more room for POKéMON!\p"
	.string "The POKéMON BOXES are full and\n"
	.string "can’t accept any more!$"

Quetzal_Text_8361A68::
	.string "You cannot receive gift POKéMON\n"
	.string "during the NUZLOCKE challenge.\p"
	.string "Come back after completing\n"
	.string "this region’s challenge.$"

Quetzal_Text_8361AF9::
	.string "{STR_VAR_1} was sent\n"
	.string "to a PC BOX.$"

Quetzal_Text_8361BA2::
	.string "Couldn’t choose a starter.\n"
	.string "Please try again.$"

Quetzal_Text_83C30E6::
	.string "Jessie: Prepare for trouble!\p"
	.string "James: Make it double!\p"
	.string "Jessie: To protect the world from\n"
	.string "devastation!\p"
	.string "James: To unite all peoples within\n"
	.string "our nation!\p"
	.string "Jessie: To denounce the evils of truth\n"
	.string "and love!\p"
	.string "James: To extend our reach to the stars\n"
	.string "above!\p"
	.string "Jessie: Jessie!\p"
	.string "James: James!\p"
	.string "Jessie: Team Rocket blast off at the\n"
	.string "speed of light!\p"
	.string "James: Surrender now or prepare to\n"
	.string "fight!\p"
	.string "Meowth: Meowth! That’s right!\p"
	.string "Wobuffet: Wobbuffet!$"

Quetzal_Text_83C3288::
	.string "Team Rocket’s blasting off again!$"

Quetzal_Text_83C32AA::
	.string "{PLAYER} received Pikachu!$"

Quetzal_Text_83E90EF::
	.string "This passage links JOHTO and KANTO.\p"
	.string "Only the champions of both POKéMON\n"
	.string "LEAGUES may cross.$"

Quetzal_Text_83F09A4::
	.string "The DAY CARE is now managed\n"
	.string "through the PC.\p"
	.string "We sell special INCENSES here\n"
	.string "for breeding.$"

Quetzal_Text_83F0C34::
	.string "The DAY-CARE service is now\n"
	.string "available from any PC.$"

Quetzal_Text_83F721C::
	.string "Those fossils aren’t compatible.\p"
	.string "Please choose one of the other type.$"

Quetzal_Text_83F760D::
	.string "Wait! That thing you have there…\n"
	.string "Is that a POKéMON fossil?\p"
	.string "Would you like to bring that POKéMON\n"
	.string "back to life?\p"
	.string "I can with my newly developed\n"
	.string "FOSSIL REGENERATOR.$"

Quetzal_Text_840FAE5::
	.string "Which port would you like to visit?$"

Quetzal_Text_840FB09::
	.string "All aboard!\n"
	.string "Have a pleasant voyage!$"

Quetzal_Text_840FB2D::
	.string "To visit another region, you need\n"
	.string "an S.S. Ticket and must have won\l"
	.string "both Leagues.\p"
	.string "Please finish any pending events\n"
	.string "before boarding, too.$"

Quetzal_Text_8446A8F::
	.string "Coins: {STR_VAR_1}\n"
	.string "Choose a prize. B: Exit.$"

Quetzal_Text_8446AB2::
	.string "Coins: {STR_VAR_1}\n"
	.string "How many would you like? B: Exit.$"

Quetzal_Text_8446B7A::
	.string "No?\n"
	.string "Please come play sometime!$"

Quetzal_Text_8446B99::
	.string "Oh, I’m sorry.\n"
	.string "You don’t have a COIN CASE.$"

Quetzal_Text_8446BC4::
	.string "Whoops!\n"
	.string "Your COIN CASE is full.$"

Quetzal_Text_8446BE4::
	.string "You can’t afford the COINS.$"

Quetzal_Text_8446C00::
	.string "Thank you.\n"
	.string "You now have {STR_VAR_1} coins!$"
