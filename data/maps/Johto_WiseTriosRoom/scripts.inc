@ Moemon Kai: Crystal's WISE TRIO (pokecrystal maps/WiseTriosRoom.asm; audit case 139). In this ECRUTEAK the room
@ joins the TIN TOWER's entrance to the way north to the TIN TOWER (Quetzal's), so the trio guard that way: without
@ the CLEAR BELL the sage at the way out turns the player back; with it, KOJI (after GAKU and MASA) battles, then
@ lets the player pass. Gone once SUICUNE is met at the TIN TOWER.
Johto_WiseTriosRoom_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_WiseTriosRoom_OnTransition
	.byte 0

Johto_WiseTriosRoom_OnTransition::
	setvar VAR_TEMP_1, 0
	goto_if_set FLAG_JOHTO_FOUGHT_SUICUNE, Johto_WiseTriosRoom_EventScript_NoWiseTrio
	goto_if_set FLAG_JOHTO_KOJI_ALLOWS_PASSAGE, Johto_WiseTriosRoom_EventScript_WiseTrio2
	checkitem ITEM_CLEAR_BELL
	goto_if_eq VAR_RESULT, TRUE, Johto_WiseTriosRoom_EventScript_WiseTrio2
	setflag FLAG_TEMP_12
	setvar VAR_TEMP_1, 1
	end

Johto_WiseTriosRoom_EventScript_WiseTrio2::
	setflag FLAG_TEMP_11
	goto_if_set FLAG_JOHTO_KOJI_ALLOWS_PASSAGE, Johto_WiseTriosRoom_EventScript_KojiAside
	setvar VAR_TEMP_1, 2
	end

Johto_WiseTriosRoom_EventScript_KojiAside::
	setobjectxyperm 6, 7, 3
	end

Johto_WiseTriosRoom_EventScript_NoWiseTrio::
	setflag FLAG_TEMP_11
	setflag FLAG_TEMP_12
	end

Johto_WiseTriosRoom_EventScript_Sage1::
	msgbox Johto_WiseTriosRoom_Text_Sage1, MSGBOX_NPC
	end

Johto_WiseTriosRoom_EventScript_Sage2::
	msgbox Johto_WiseTriosRoom_Text_Sage2, MSGBOX_NPC
	end

Johto_WiseTriosRoom_EventScript_Sage3::
	msgbox Johto_WiseTriosRoom_Text_Sage3, MSGBOX_NPC
	end

@ (Crystal's WiseTriosRoom_CannotEnterTinTowerScript)
Johto_WiseTriosRoom_EventScript_CannotEnterTinTower::
	lockall
	turnobject 3, DIR_NORTH
	playse SE_PIN
	applymovement 3, Quetzal_Movement_83607F9
	waitmovement 0
	msgbox Johto_WiseTriosRoom_Text_Sage3BlocksExit, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, Johto_WiseTriosRoom_Movement_TurnedBack
	waitmovement 0
	releaseall
	end

Johto_WiseTriosRoom_EventScript_Gaku::
	trainerbattle_single TRAINER_JOHTO_SAGE_GAKU, Johto_WiseTriosRoom_Text_GakuSeen, Johto_WiseTriosRoom_Text_GakuBeaten
	msgbox Johto_WiseTriosRoom_Text_GakuAfterBattle, MSGBOX_AUTOCLOSE
	end

Johto_WiseTriosRoom_EventScript_Masa::
	trainerbattle_single TRAINER_JOHTO_SAGE_MASA, Johto_WiseTriosRoom_Text_MasaSeen, Johto_WiseTriosRoom_Text_MasaBeaten
	msgbox Johto_WiseTriosRoom_Text_MasaAfterBattle, MSGBOX_AUTOCLOSE
	end

@ KOJI at the way out: the player stepping toward it is challenged (Crystal's sight-2 sage beside the door)
Johto_WiseTriosRoom_EventScript_KojiBlocks::
	lockall
	applymovement 6, Johto_WiseTriosRoom_Movement_FaceLeft
	waitmovement 0
Johto_WiseTriosRoom_EventScript_Koji::
	trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT, TRAINER_JOHTO_SAGE_KOJI, 0, Johto_WiseTriosRoom_Text_KojiSeen, Johto_WiseTriosRoom_Text_KojiBeaten, Johto_WiseTriosRoom_EventScript_KojiBeaten
	goto_if_set FLAG_JOHTO_KOJI_ALLOWS_PASSAGE, Johto_WiseTriosRoom_EventScript_KojiFinal
Johto_WiseTriosRoom_EventScript_KojiBeaten::
	goto_if_set FLAG_JOHTO_KOJI_ALLOWS_PASSAGE, Johto_WiseTriosRoom_EventScript_KojiFinal
	delay 10
	playse SE_PIN
	applymovement 6, Quetzal_Movement_83607F9
	waitmovement 0
	msgbox Johto_WiseTriosRoom_Text_KojiAfterBattleQuestion, MSGBOX_DEFAULT
	msgbox Johto_WiseTriosRoom_Text_KojiAfterBattleSpeech, MSGBOX_DEFAULT
	closemessage
	applymovement 6, Johto_WiseTriosRoom_Movement_KojiAside
	waitmovement 0
	setflag FLAG_JOHTO_KOJI_ALLOWS_PASSAGE
	setvar VAR_TEMP_1, 0
	releaseall
	end

Johto_WiseTriosRoom_EventScript_KojiFinal::
	msgbox Johto_WiseTriosRoom_Text_KojiAfterBattleFinal, MSGBOX_DEFAULT
	releaseall
	end

Johto_WiseTriosRoom_Movement_TurnedBack:
	walk_left
	walk_left
	step_end

Johto_WiseTriosRoom_Movement_FaceLeft:
	face_left
	step_end

Johto_WiseTriosRoom_Movement_KojiAside:
	walk_left
	walk_up
	face_down
	step_end

Johto_WiseTriosRoom_Text_Sage1::
	.string "Astounding…\p"
	.string "SUICUNE, ENTEI and RAIKOU have\n"
	.string "arisen from their sleep…\p"
	.string "Is the legend coming true?$"

Johto_WiseTriosRoom_Text_Sage2::
	.string "We train at the BURNED TOWER, but\n"
	.string "we’ve never heard of a hole\l"
	.string "opening up there before. It must\l"
	.string "have been deliberately made by\l"
	.string "someone.$"

Johto_WiseTriosRoom_Text_Sage3BlocksExit::
	.string "TIN TOWER may be entered by those\n"
	.string "bearing ECRUTEAK’s GYM BADGE.\p"
	.string "However, now that SUICUNE, RAIKOU\n"
	.string "and ENTEI have arisen, I ask you\l"
	.string "to refrain from entering!$"

Johto_WiseTriosRoom_Text_Sage3::
	.string "We, the WISE TRIO, are the\n"
	.string "protectors of the legendary\l"
	.string "MOéMON.$"

Johto_WiseTriosRoom_Text_GakuSeen::
	.string "Legend has it that upon the\n"
	.string "emergence of a trainer who has the\l"
	.string "ability to touch the souls of\l"
	.string "MOéMON, a MOéMON will come forth\l"
	.string "to put that trainer to test at the\l"
	.string "TIN TOWER.\p"
	.string "The legend has come true!\p"
	.string "The legendary MOéMON SUICUNE has\n"
	.string "arrived!\p"
	.string "We, the WISE TRIO, shall test your\n"
	.string "worthiness to go inside!$"

Johto_WiseTriosRoom_Text_GakuBeaten::
	.string "Stronger than we thought? Perhaps…$"

Johto_WiseTriosRoom_Text_GakuAfterBattle::
	.string "Ah, so it is you who claim to have\n"
	.string "seen SUICUNE, ENTEI and RAIKOU\l"
	.string "while they slept?\p"
	.string "Unbelievable!\p"
	.string "Legend has it that they can’t be\n"
	.string "seen while they sleep…$"

Johto_WiseTriosRoom_Text_MasaSeen::
	.string "Can you be trusted with the truth?\p"
	.string "I must ascertain your worthiness.$"

Johto_WiseTriosRoom_Text_MasaBeaten::
	.string "…I will tell you the truth…$"

Johto_WiseTriosRoom_Text_MasaAfterBattle::
	.string "In the past, there were two\n"
	.string "nine-tier towers here.\p"
	.string "The BRASS TOWER, which was said to\n"
	.string "awaken MOéMON, and the TIN TOWER,\l"
	.string "where MOéMON were said to rest.\p"
	.string "The view from the tops of the\n"
	.string "TOWERS must have been magnificent.\p"
	.string "At the time, an immense,\n"
	.string "silvercolored MOéMON was said to\l"
	.string "make its roost atop the BRASS\l"
	.string "TOWER.\p"
	.string "However…\p"
	.string "About 150 years ago, a lightning\n"
	.string "bolt struck one of the TOWERS.\p"
	.string "It was engulfed in flames that\n"
	.string "raged for three days.\p"
	.string "A sudden downpour finally put out\n"
	.string "the blaze.\p"
	.string "And that is how the BURNED TOWER\n"
	.string "came to be.$"

Johto_WiseTriosRoom_Text_KojiSeen::
	.string "Let me see your power!$"

Johto_WiseTriosRoom_Text_KojiBeaten::
	.string "Too strong! Why?$"

Johto_WiseTriosRoom_Text_KojiAfterBattleQuestion::
	.string "You… Are you the trainer who is\n"
	.string "awaited by the legendary MOéMON?$"

Johto_WiseTriosRoom_Text_KojiAfterBattleSpeech::
	.string "I see…\p"
	.string "We, the WISE TRIO, have been given\n"
	.string "the responsibility of protecting\l"
	.string "the legendary MOéMON. We are to\l"
	.string "allow passage only to those people\l"
	.string "who possess the power and soul of\l"
	.string "truth.\p"
	.string "Please, do go on and enter the TIN\n"
	.string "TOWER ahead.\p"
	.string "SUICUNE will put you to the test.$"

Johto_WiseTriosRoom_Text_KojiAfterBattleFinal::
	.string "Please, do go on.\p"
	.string "SUICUNE will put you to the test.$"
