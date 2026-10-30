Kanto_FiveIsland_ResortGorgeous_House_MapScripts::
	.byte 0

@ Kanto_FiveIsland_ResortGorgeous_House object 1 (local id 1, gfx 0x3F58) at 4,4
Kanto_FiveIsland_ResortGorgeous_House_EventScript_8421BC0::
	callnative 0x80E5AED
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x2EB4, Kanto_FiveIsland_ResortGorgeous_House_EventScript_8421CF7
	goto_if_set FLAG_QZ_0x2EB2, Kanto_FiveIsland_ResortGorgeous_House_EventScript_8421DA5
	goto_if_ne VAR_QZ_0x5141, 0, Kanto_FiveIsland_ResortGorgeous_House_EventScript_8421C19
	msgbox Kanto_FiveIsland_ResortGorgeous_House_Text_8454849, MSGBOX_YESNO
	goto_if_ne VAR_RESULT, 1, Kanto_FiveIsland_ResortGorgeous_House_EventScript_8421DAD
	goto Kanto_FiveIsland_ResortGorgeous_House_EventScript_8421BFC

Kanto_FiveIsland_ResortGorgeous_House_EventScript_8421BFC::
	specialvar VAR_RESULT, QzSampleResortGorgeousMonAndReward
	goto_if_eq VAR_RESULT, 0, Kanto_FiveIsland_ResortGorgeous_House_EventScript_8421C66
	msgbox Kanto_FiveIsland_ResortGorgeous_House_Text_84548C2, MSGBOX_DEFAULT
	goto Kanto_FiveIsland_ResortGorgeous_House_EventScript_8421DAD

Kanto_FiveIsland_ResortGorgeous_House_EventScript_8421C19::
	specialvar VAR_RESULT, QzSampleResortGorgeousMonAndReward
	goto_if_eq VAR_RESULT, 0, Kanto_FiveIsland_ResortGorgeous_House_EventScript_8421C66
	specialvar VAR_RESULT, QzPartyHasRequestedResortMon
	goto_if_eq VAR_RESULT, 1, Kanto_FiveIsland_ResortGorgeous_House_EventScript_8421C73
	msgbox Kanto_FiveIsland_ResortGorgeous_House_Text_84548C2, MSGBOX_DEFAULT
	msgbox Kanto_FiveIsland_ResortGorgeous_House_Text_8454921, MSGBOX_YESNO
	goto_if_ne VAR_RESULT, 1, Kanto_FiveIsland_ResortGorgeous_House_EventScript_8421DAD
	setvar VAR_QZ_0x5141, 0
	msgbox Kanto_FiveIsland_ResortGorgeous_House_Text_8454948, MSGBOX_DEFAULT
	goto Kanto_FiveIsland_ResortGorgeous_House_EventScript_8421DAD

Kanto_FiveIsland_ResortGorgeous_House_EventScript_8421C66::
	msgbox Kanto_FiveIsland_ResortGorgeous_House_Text_845499B, MSGBOX_DEFAULT
	goto Kanto_FiveIsland_ResortGorgeous_House_EventScript_8421DAD

Kanto_FiveIsland_ResortGorgeous_House_EventScript_8421C73::
	setflag FLAG_QZ_0x2EB4
	msgbox Kanto_FiveIsland_ResortGorgeous_House_Text_84549F2, MSGBOX_DEFAULT
	setobjectxyperm 2, 6, 2
	addobject 2
	applymovement 2, Kanto_FiveIsland_ResortGorgeous_House_Movement_8421DC2
	waitmovement 0
	textcolor 0
	msgbox Kanto_FiveIsland_ResortGorgeous_House_Text_8454D5F, MSGBOX_DEFAULT
	applymovement 1, Quetzal_Movement_8457AAC
	waitmovement 0
	textcolor 1
	msgbox Kanto_FiveIsland_ResortGorgeous_House_Text_8454D75, MSGBOX_DEFAULT
	textcolor 0
	msgbox Kanto_FiveIsland_ResortGorgeous_House_Text_8454DBA, MSGBOX_DEFAULT
	closemessage
	call_if_eq VAR_FACING, 2, Kanto_FiveIsland_ResortGorgeous_House_EventScript_8421D40
	call_if_eq VAR_FACING, 1, Kanto_FiveIsland_ResortGorgeous_House_EventScript_8421D5C
	call_if_eq VAR_FACING, 4, Kanto_FiveIsland_ResortGorgeous_House_EventScript_8421D71
	call_if_eq VAR_FACING, 3, Kanto_FiveIsland_ResortGorgeous_House_EventScript_8421D8D
	removeobject 2
	msgbox Kanto_FiveIsland_ResortGorgeous_House_Text_8454DE2, MSGBOX_DEFAULT
	goto Kanto_FiveIsland_ResortGorgeous_House_EventScript_8421CFF

Kanto_FiveIsland_ResortGorgeous_House_EventScript_8421CF7::
	msgbox Kanto_FiveIsland_ResortGorgeous_House_Text_8454A3C, MSGBOX_DEFAULT

Kanto_FiveIsland_ResortGorgeous_House_EventScript_8421CFF::
	callnative 0x80E5AED
	copyvar VAR_0x8000, VAR_QZ_0x5142
	setvar VAR_0x8001, 1
	additem VAR_0x8000, VAR_0x8001
	copyvar VAR_0x8007, VAR_RESULT
	goto_if_eq VAR_0x8007, 0, Kanto_FiveIsland_ResortGorgeous_House_EventScript_8421D98
	setflag FLAG_QZ_0x2EB2
	clearflag FLAG_QZ_0x2EB4
	clearflag FLAG_QZ_0x2EB3
	setvar VAR_QZ_0x5141, 0
	setvar VAR_QZ_0x5142, 0
	call Quetzal_EventScript_835FB62
	goto Kanto_FiveIsland_ResortGorgeous_House_EventScript_8421DA5

Kanto_FiveIsland_ResortGorgeous_House_EventScript_8421D40::
	applymovement 1, Quetzal_Movement_8457AB0
	applymovement 2, Kanto_FiveIsland_ResortGorgeous_House_Movement_8421DAF
	waitmovement 0
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AAA
	waitmovement 0
	return

Kanto_FiveIsland_ResortGorgeous_House_EventScript_8421D5C::
	applymovement 2, Kanto_FiveIsland_ResortGorgeous_House_Movement_8421DB7
	waitmovement 0
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AAC
	waitmovement 0
	return

Kanto_FiveIsland_ResortGorgeous_House_EventScript_8421D71::
	applymovement 1, Quetzal_Movement_8457AAA
	applymovement 2, Kanto_FiveIsland_ResortGorgeous_House_Movement_8421DBB
	waitmovement 0
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AAC
	waitmovement 0
	return

Kanto_FiveIsland_ResortGorgeous_House_EventScript_8421D8D::
	applymovement 2, Kanto_FiveIsland_ResortGorgeous_House_Movement_8421DC0
	waitmovement 0
	return

Kanto_FiveIsland_ResortGorgeous_House_EventScript_8421D98::
	msgbox Kanto_FiveIsland_ResortGorgeous_House_Text_8454A72, MSGBOX_DEFAULT
	goto Kanto_FiveIsland_ResortGorgeous_House_EventScript_8421DAD

Kanto_FiveIsland_ResortGorgeous_House_EventScript_8421DA5::
	msgbox Kanto_FiveIsland_ResortGorgeous_House_Text_8454AB2, MSGBOX_DEFAULT

Kanto_FiveIsland_ResortGorgeous_House_EventScript_8421DAD::
	release
	end

@ Kanto_FiveIsland_ResortGorgeous_House object 2 (local id 2, gfx 0x3F09) at 6,2
Kanto_FiveIsland_ResortGorgeous_House_EventScript_8421DC4::
	msgbox Kanto_FiveIsland_ResortGorgeous_House_Text_8454EAB, MSGBOX_NPC
	end

@ Kanto_FiveIsland_ResortGorgeous_House sign / bg event at 9,1
@ Kanto_FiveIsland_ResortGorgeous_House sign / bg event at 8,1
Kanto_FiveIsland_ResortGorgeous_House_EventScript_845D2FD::
	lockall
	setvar VAR_0x8004, 8
	setvar VAR_0x8005, 5
	special QzQuestLogNop
	textcolor 3
	msgbox Kanto_FiveIsland_ResortGorgeous_House_Text_8461BC0, MSGBOX_DEFAULT
	releaseall
	end

Kanto_FiveIsland_ResortGorgeous_House_Movement_8421DAF:
	walk_left
	walk_left
	walk_left
	walk_down
	walk_down
	walk_down
	walk_in_place_faster_right
	step_end

Kanto_FiveIsland_ResortGorgeous_House_Movement_8421DB7:
	walk_left
	walk_left
	walk_in_place_faster_down
	step_end

Kanto_FiveIsland_ResortGorgeous_House_Movement_8421DBB:
	walk_left
	walk_left
	walk_left
	walk_down
	step_end

Kanto_FiveIsland_ResortGorgeous_House_Movement_8421DC0:
	walk_up
	step_end

Kanto_FiveIsland_ResortGorgeous_House_Movement_8421DC2:
	walk_in_place_down
	step_end

Kanto_FiveIsland_ResortGorgeous_House_Text_8454849::
	.string "SELPHY: Will you do me a favor?\p"
	.string "I’ll reward you for showing me\n"
	.string "a Pokémon. One reward per day!\p"
	.string "Will you accept a request?$"

Kanto_FiveIsland_ResortGorgeous_House_Text_84548C2::
	.string "I wish to see a Pokémon:\n"
	.string "{STR_VAR_1}.\p"
	.string "Bring one in your party anytime.\n"
	.string "You don’t need to catch another.$"

Kanto_FiveIsland_ResortGorgeous_House_Text_8454921::
	.string "Would you like to cancel this request?$"

Kanto_FiveIsland_ResortGorgeous_House_Text_8454948::
	.string "I’ll think of another Pokémon, then!\p"
	.string "Talk to me again if you’d like\n"
	.string "a new request.$"

Kanto_FiveIsland_ResortGorgeous_House_Text_845499B::
	.string "I can’t think of a request yet.\p"
	.string "Register more captures in Kanto\n"
	.string "and come see me again.$"

Kanto_FiveIsland_ResortGorgeous_House_Text_84549F2::
	.string "Oh, how darling!\n"
	.string "It’s {STR_VAR_1}!\p"
	.string "Thank you for showing me.\n"
	.string "SEBASTIAN! Come here!$"

Kanto_FiveIsland_ResortGorgeous_House_Text_8454A3C::
	.string "Your reward is still here!\n"
	.string "Let’s try handing it over.$"

Kanto_FiveIsland_ResortGorgeous_House_Text_8454A72::
	.string "There’s no room for your reward.\n"
	.string "I’ll keep it until you return.$"

Kanto_FiveIsland_ResortGorgeous_House_Text_8454AB2::
	.string "You’ve received today’s reward!\n"
	.string "Come back tomorrow for a request.$"

Kanto_FiveIsland_ResortGorgeous_House_Text_8454D5F::
	.string "Butler: Yes, my lady.$"

Kanto_FiveIsland_ResortGorgeous_House_Text_8454D75::
	.string "SELPHY: See to it that this person\n"
	.string "is given a token of appreciation.$"

Kanto_FiveIsland_ResortGorgeous_House_Text_8454DBA::
	.string "Butler: I shall do as you bid,\n"
	.string "my lady.$"

Kanto_FiveIsland_ResortGorgeous_House_Text_8454DE2::
	.string "Butler: I sincerely thank you for\n"
	.string "pleasing Lady SELPHY.\p"
	.string "Please accept this as her thanks…$"

Kanto_FiveIsland_ResortGorgeous_House_Text_8454EAB::
	.string "Butler: The smile of Lady SELPHY\n"
	.string "has the brilliance of the sun.\p"
	.string "Please, I beg you to bring joy\n"
	.string "to Lady SELPHY.$"

Kanto_FiveIsland_ResortGorgeous_House_Text_8461BC0::
	.string "POKéMON JOURNAL\p"
	.string "Special Feature: CINNABAR GYM\n"
	.string "LEADER BLAINE!\p"
	.string "BLAINE is said to remove his dark\n"
	.string "shades only when he is thinking up\l"
	.string "new quiz questions.$"
