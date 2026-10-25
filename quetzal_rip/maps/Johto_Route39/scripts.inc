Johto_Route39_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_Route39_EventScript_83F2CED
	.byte 0

@ Johto_Route39 trigger at 24,53 (VAR_QZ_0x500D == 0)
@ Johto_Route39 trigger at 25,53 (VAR_QZ_0x500D == 0)
@ Johto_Route39 trigger at 26,53 (VAR_QZ_0x500D == 0)
@ Johto_Route39 trigger at 27,53 (VAR_QZ_0x500D == 0)
Johto_Route39_EventScript_83E401D::
	setflag FLAG_QZ_0x11C0
	setvar VAR_QZ_0x500D, 1
	end

Johto_Route39_EventScript_83F2CED::
	end

@ Johto_Route39 object 6 (local id 6, gfx 0x3A9E) at 11,13
Johto_Route39_EventScript_83F2CEE::
	setorcopyvar VAR_0x8000, 0x1AF
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_Route39 object 10 (local id 10, gfx 0x3A9E) at 16,32
Johto_Route39_EventScript_83F2CFB::
	setorcopyvar VAR_0x8000, 0x88
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_Route39 trigger at 35,20 (VAR_QZ_0x5017 == 0)
@ Johto_Route39 trigger at 35,21 (VAR_QZ_0x5017 == 0)
@ Johto_Route39 trigger at 35,22 (VAR_QZ_0x5017 == 0)
@ Johto_Route39 trigger at 35,23 (VAR_QZ_0x5017 == 0)
Johto_Route39_EventScript_83F2D08::
	lock
	clearflag FLAG_QZ_0x10B4
	addobject 8
	playse SE_M_RAZOR_WIND
	applymovement 8, Johto_Route39_Movement_83F3025
	waitmovement 0
	setflag FLAG_QZ_0x10B4
	removeobject 8
	playse SE_PIN
	applymovement 2, Quetzal_Movement_83607F7
	applymovement LOCALID_PLAYER, Quetzal_Movement_83607F7
	turnobject 2, 1
	turnobject LOCALID_PLAYER, 1
	delay 0x3C
	getplayerxy VAR_TEMP_0, VAR_TEMP_1
	call_if_eq VAR_TEMP_1, 20, Johto_Route39_EventScript_83F2D89
	call_if_eq VAR_TEMP_1, 21, Johto_Route39_EventScript_83F2D94
	call_if_eq VAR_TEMP_1, 22, Johto_Route39_EventScript_83F2D9F
	call_if_eq VAR_TEMP_1, 23, Johto_Route39_EventScript_83F2DAA
	turnobject 2, 4
	turnobject LOCALID_PLAYER, 3
	msgbox Johto_Route39_Text_83F2DF6, MSGBOX_DEFAULT
	setvar VAR_QZ_0x5017, 2
	goto Johto_Route39_EventScript_83F2DDB

Johto_Route39_EventScript_83F2D89::
	applymovement 2, Johto_Route39_Movement_83F2DB5
	waitmovement 0
	return

Johto_Route39_EventScript_83F2D94::
	applymovement 2, Johto_Route39_Movement_83F2DC0
	waitmovement 0
	return

Johto_Route39_EventScript_83F2D9F::
	applymovement 2, Johto_Route39_Movement_83F2DCA
	waitmovement 0
	return

Johto_Route39_EventScript_83F2DAA::
	applymovement 2, Johto_Route39_Movement_83F2DD3
	waitmovement 0
	return

Johto_Route39_EventScript_83F2DDB::
	msgbox Johto_Route39_Text_83F2E6C, MSGBOX_DEFAULT
	closemessage
	applymovement 2, Johto_Route39_Movement_83F3017
	waitmovement 0
	setflag FLAG_QZ_0x1049
	removeobject 2
	release
	end

@ Johto_Route39 object 11 (local id 11, gfx 0x18) at 37,30
Johto_Route39_EventScript_83F303A::
	lock
	faceplayer
	msgbox Johto_Route39_Text_83F3047, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_Route39 object 5 (local id 5, gfx 0x31) at 22,42
Johto_Route39_EventScript_83F3082::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4414, 0, Johto_Route39_Text_83F3099, Johto_Route39_Text_83F30D4
	msgbox Johto_Route39_Text_83F30DE, MSGBOX_AUTOCLOSE
	end

@ Johto_Route39 object 4 (local id 4, gfx 0x3E84) at 29,40
Johto_Route39_EventScript_83F3137::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4323, 0, Johto_Route39_Text_83F314E, Johto_Route39_Text_83F317C
	msgbox Johto_Route39_Text_83F319F, MSGBOX_AUTOCLOSE
	end

@ Johto_Route39 object 3 (local id 3, gfx 0x10) at 22,38
Johto_Route39_EventScript_83F31FC::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4579, 0, Johto_Route39_Text_83F3213, Johto_Route39_Text_83F325B
	msgbox Johto_Route39_Text_83F3270, MSGBOX_AUTOCLOSE
	end

@ Johto_Route39 object 1 (local id 1, gfx 0x25) at 29,17
Johto_Route39_EventScript_83F32AD::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4134, 0, Johto_Route39_Text_83F32C4, Johto_Route39_Text_83F32F1
	msgbox Johto_Route39_Text_83F3313, MSGBOX_AUTOCLOSE
	end

@ Johto_Route39 object 9 (local id 9, gfx 0x14) at 16,45
Johto_Route39_EventScript_83F337E::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4706, 0, Johto_Route39_Text_83F3395, Johto_Route39_Text_83F33C4
	msgbox Johto_Route39_Text_83F33DB, MSGBOX_AUTOCLOSE
	end

@ Johto_Route39 sign / bg event at 22,46
Johto_Route39_EventScript_83F3481::
	lock
	msgbox Johto_Route39_Text_83F348D, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_Route39 sign / bg event at 25,17
Johto_Route39_EventScript_83F34B3::
	lock
	msgbox Johto_Route39_Text_83F34BF, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_Route39 sign / bg event at 25,31
Johto_Route39_EventScript_83F34EA::
	lock
	msgbox Johto_Route39_Text_83F34F6, MSGBOX_SIGN
	closemessage
	release
	end

Johto_Route39_Movement_83F2DB5:
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_right
	walk_right
	walk_right
	step_end

Johto_Route39_Movement_83F2DC0:
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_right
	walk_right
	walk_right
	step_end

Johto_Route39_Movement_83F2DCA:
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_right
	walk_right
	walk_right
	step_end

Johto_Route39_Movement_83F2DD3:
	walk_up
	walk_up
	walk_up
	walk_up
	walk_right
	walk_right
	walk_right
	step_end

Johto_Route39_Movement_83F3017:
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	step_end

Johto_Route39_Movement_83F3025:
	slide_down
	slide_down
	slide_down
	slide_down
	slide_down
	slide_down
	slide_down
	slide_down
	slide_down
	slide_down
	slide_down
	slide_down
	slide_down
	slide_down
	slide_down
	slide_down
	slide_down
	slide_down
	slide_down
	slide_down
	step_end

Johto_Route39_Text_83F2DF6::
	.string "BAOBA: GOOD HEAVENS! Did that\n"
	.string "shadow belong to a POKéMON?\p"
	.string "Those golden feathers... No doubt\n"
	.string "about it--that was HO-OH!$"

Johto_Route39_Text_83F2E6C::
	.string "How strange...\p"
	.string "...Oh!\n"
	.string "What you are holding is...\l"
	.string "what they call... a POKéDEX!\p"
	.string "Haven’t seen one of those in ages.\p"
	.string "My name is BAOBA!\n"
	.string "I used to run a SAFARI ZONE in\l"
	.string "FUCHSIA CITY, but it was becoming\l"
	.string "out of date with new entertainment\l"
	.string "options coming out.\p"
	.string "I decided to go overseas to learn\n"
	.string "the newest technology and open a\l"
	.string "state-of-the-art SAFARI ZONE\l"
	.string "near CIANWOOD.\p"
	.string "Please visit once it opens!\n"
	.string "I’ll make sure the news reaches you.$"

Johto_Route39_Text_83F3047::
	.string "That shadow... Maybe it was a giant\n"
	.string "PIDGEY?$"

Johto_Route39_Text_83F3099::
	.string "I just got back to OLIVINE.\p"
	.string "So how about a POKéMON battle?$"

Johto_Route39_Text_83F30D4::
	.string "Awaaargh!$"

Johto_Route39_Text_83F30DE::
	.string "My POKéMON were caught and\n"
	.string "raised overseas.\p"
	.string "They’re my companions on\n"
	.string "those long voyages.$"

Johto_Route39_Text_83F314E::
	.string "This is a good time to brag\n"
	.string "about my PIKACHU!$"

Johto_Route39_Text_83F317C::
	.string "I had no time to show off\n"
	.string "PIKACHU…$"

Johto_Route39_Text_83F319F::
	.string "I’m not listening to your\n"
	.string "bragging!\p"
	.string "We POKé FANS have a policy\n"
	.string "of not listening to bragging!$"

Johto_Route39_Text_83F3213::
	.string "Such darling POKéMON.\p"
	.string "Let’s show our POKéMON\n"
	.string "together at the same time.$"

Johto_Route39_Text_83F325B::
	.string "I don’t mind losing.$"

Johto_Route39_Text_83F3270::
	.string "Do you know about baby\n"
	.string "POKéMON?\p"
	.string "I bet they’re just adorable!$"

Johto_Route39_Text_83F32C4::
	.string "Let me see what your\n"
	.string "POKéMON are capable of.$"

Johto_Route39_Text_83F32F1::
	.string "Ooh, your POKéMON have\n"
	.string "potential.$"

Johto_Route39_Text_83F3313::
	.string "You know how POKéMON have\n"
	.string "different abilities?\p"
	.string "People are like that too.\n"
	.string "Everyone has different\l"
	.string "potential.$"

Johto_Route39_Text_83F3395::
	.string "You came at just the right\n"
	.string "time.\p"
	.string "Let’s battle.$"

Johto_Route39_Text_83F33C4::
	.string "Oh, how disappointing…$"

Johto_Route39_Text_83F33DB::
	.string "I met UMBREON right here\n"
	.string "on ROUTE 39.\p"
	.string "I’m not sure why, but it\n"
	.string "seems to like it when I\l"
	.string "train here.\p"
	.string "It seems to become friendlier\n"
	.string "by training here than\l"
	.string "anywhere else.$"

Johto_Route39_Text_83F348D::
	.string "ROUTE 39\p"
	.string "OLIVINE CITY - ECRUTEAK CITY$"

Johto_Route39_Text_83F34BF::
	.string "MOOMOO FARM\p"
	.string "Enjoy Our Fresh and Tasty Milk$"

Johto_Route39_Text_83F34F6::
	.string "TRAINER TIPS\p"
	.string "Use HEADBUTT on trees to shake\n"
	.string "POKéMON out.\p"
	.string "Different kinds of POKéMON\n"
	.string "drop out of trees.\p"
	.string "Use HEADBUTT on any tree\n"
	.string "you see!$"
