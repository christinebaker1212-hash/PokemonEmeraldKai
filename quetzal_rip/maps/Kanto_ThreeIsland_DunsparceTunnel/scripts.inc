Kanto_ThreeIsland_DunsparceTunnel_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_ThreeIsland_DunsparceTunnel_EventScript_8414B12
	.byte 0

Kanto_ThreeIsland_DunsparceTunnel_EventScript_8414B12::
	setflag FLAG_QZ_0x225D
	call_if_set FLAG_QZ_0x2108, Kanto_ThreeIsland_DunsparceTunnel_EventScript_8414B28
	call_if_unset FLAG_QZ_0x2108, Kanto_ThreeIsland_DunsparceTunnel_EventScript_8414B37
	end

Kanto_ThreeIsland_DunsparceTunnel_EventScript_8414B28::
	setmaplayoutindex 0x3AC
	setobjectxyperm 1, 0x17, 2
	setobjectmovementtype 1, 8
	return

Kanto_ThreeIsland_DunsparceTunnel_EventScript_8414B37::
	setmaplayoutindex 0x3AB
	setobjectxyperm 1, 6, 3
	setobjectmovementtype 1, 0xA
	return

@ Kanto_ThreeIsland_DunsparceTunnel object 1 (local id 1, gfx 0x3F04) at 23,2
Kanto_ThreeIsland_DunsparceTunnel_EventScript_8414B46::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x2159, Kanto_ThreeIsland_DunsparceTunnel_EventScript_8414BA1
	goto_if_set FLAG_QZ_0x2108, Kanto_ThreeIsland_DunsparceTunnel_EventScript_8414B6F
	msgbox Kanto_ThreeIsland_DunsparceTunnel_Text_842D844, MSGBOX_DEFAULT
	closemessage
	applymovement 1, Quetzal_Movement_8457AA8
	waitmovement 0
	release
	end

Kanto_ThreeIsland_DunsparceTunnel_EventScript_8414B6F::
	msgbox Kanto_ThreeIsland_DunsparceTunnel_Text_842D8D2, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, 0x87
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Kanto_ThreeIsland_DunsparceTunnel_EventScript_8414B97
	setflag FLAG_QZ_0x2159
	goto Kanto_ThreeIsland_DunsparceTunnel_EventScript_8414BA1

Kanto_ThreeIsland_DunsparceTunnel_EventScript_8414B97::
	msgbox Kanto_ThreeIsland_DunsparceTunnel_Text_842D9E5, MSGBOX_DEFAULT
	release
	end

Kanto_ThreeIsland_DunsparceTunnel_EventScript_8414BA1::
	msgbox Kanto_ThreeIsland_DunsparceTunnel_Text_842D995, MSGBOX_DEFAULT
	release
	end

Kanto_ThreeIsland_DunsparceTunnel_Text_842D844::
	.string "Hey, there!\n"
	.string "How’s it going?\p"
	.string "What am I doing here, you ask?\n"
	.string "Why, I’m prospecting for gold!\p"
	.string "When I strike it rich, I’m buying\n"
	.string "a house in KANTO.$"

Kanto_ThreeIsland_DunsparceTunnel_Text_842D8D2::
	.string "Hey, there! How’s it going?\n"
	.string "Did you hear? I’ve struck gold!\p"
	.string "You remembered that I was\n"
	.string "prospecting, didn’t you?\p"
	.string "You can’t begin to imagine just\n"
	.string "how happy I am.\p"
	.string "So, I’ll show you!\n"
	.string "This is for you!$"

Kanto_ThreeIsland_DunsparceTunnel_Text_842D995::
	.string "That’s a NUGGET!\p"
	.string "I can’t give you any nuggets of\n"
	.string "wisdom, so that’ll have to do!$"

Kanto_ThreeIsland_DunsparceTunnel_Text_842D9E5::
	.string "Whoops, your BAG is crammed full!\n"
	.string "I’ll give this to you later.$"
