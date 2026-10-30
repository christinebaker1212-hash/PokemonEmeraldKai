Johto_SproutTower3F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_SproutTower3F_EventScript_83FA777
	.byte 0

Johto_SproutTower3F_EventScript_83FA777::
	call Quetzal_EventScript_840EA62
	end

@ Johto_SproutTower3F object 2 (local id 2, gfx 0x3EB2) at 8,14
Johto_SproutTower3F_EventScript_83FA77D::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4899, 0, Johto_SproutTower3F_Text_83FAC6C, Johto_SproutTower3F_Text_83FAC96
	msgbox Johto_SproutTower3F_Text_83FACB1, MSGBOX_AUTOCLOSE
	end

@ Johto_SproutTower3F object 1 (local id 1, gfx 0x3EB2) at 14,13
Johto_SproutTower3F_EventScript_83FA794::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4316, 0, Johto_SproutTower3F_Text_83FAD7D, Johto_SproutTower3F_Text_83FADAE
	msgbox Johto_SproutTower3F_Text_83FADCC, MSGBOX_AUTOCLOSE
	end

@ Johto_SproutTower3F object 3 (local id 3, gfx 0x3EB2) at 11,11
Johto_SproutTower3F_EventScript_83FA7AB::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4102, 0, Johto_SproutTower3F_Text_83FAD1C, Johto_SproutTower3F_Text_83FAD48
	msgbox Johto_SproutTower3F_Text_83FAD61, MSGBOX_AUTOCLOSE
	end

@ Johto_SproutTower3F sign / bg event at 11,1
Johto_SproutTower3F_EventScript_83FA7C2::
	lock
	msgbox Johto_SproutTower3F_Text_83FADF0, MSGBOX_AUTOCLOSE
	release
	end

@ Johto_SproutTower3F object 7 (local id 7, gfx 0x3A9E) at 7,16
Johto_SproutTower3F_EventScript_83FA7CD::
	setorcopyvar VAR_0x8000, ITEM_POTION
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_SproutTower3F object 6 (local id 6, gfx 0x3A9E) at 16,3
Johto_SproutTower3F_EventScript_83FA7DA::
	setorcopyvar VAR_0x8000, ITEM_ESCAPE_ROPE
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_SproutTower3F trigger at 13,7 (VAR_QZ_0x5011 == 0)
Johto_SproutTower3F_EventScript_83FA7E7::
	lock
	playbgm MUS_QZ_648, 1
	applymovement 5, Johto_SproutTower3F_Movement_83FA8F6
	waitmovement 0
	msgbox Johto_SproutTower3F_Text_83FA915, MSGBOX_DEFAULT
	closemessage
	playse SE_PIN
	applymovement 5, Quetzal_Movement_83607F9
	delay 0x3C
	applymovement 5, Johto_SproutTower3F_Movement_83FA8F9
	waitmovement 0
	msgbox Johto_SproutTower3F_Text_83FA9DD, MSGBOX_DEFAULT
	delay 0x1E
	msgbox Johto_SproutTower3F_Text_83FAAC4, MSGBOX_DEFAULT
	applymovement 5, Johto_SproutTower3F_Movement_83FA908
	waitmovement 0
	closemessage
	fadescreen 1
	playse SE_EXIT
	setflag FLAG_QZ_0x1082
	removeobject 5
	delay 0x37
	fadescreen 0
	special DrawWholeMapView
	fadedefaultbgm
	setvar VAR_QZ_0x5011, 1
	release
	end

@ Johto_SproutTower3F trigger at 14,7 (VAR_QZ_0x5011 == 0)
Johto_SproutTower3F_EventScript_83FA84F::
	lock
	playbgm MUS_QZ_648, 1
	applymovement 5, Johto_SproutTower3F_Movement_83FA8F6
	waitmovement 0
	msgbox Johto_SproutTower3F_Text_83FA915, MSGBOX_DEFAULT
	closemessage
	playse SE_PIN
	applymovement 5, Quetzal_Movement_83607F9
	delay 0x3C
	applymovement 5, Johto_SproutTower3F_Movement_83FA900
	waitmovement 0
	waitmovement 0
	msgbox Johto_SproutTower3F_Text_83FA9DD, MSGBOX_DEFAULT
	msgbox Johto_SproutTower3F_Text_83FAAC4, MSGBOX_DEFAULT
	applymovement 5, Johto_SproutTower3F_Movement_83FA908
	waitmovement 0
	closemessage
	fadescreen 1
	playse SE_EXIT
	setflag FLAG_QZ_0x1082
	removeobject 5
	delay 0x37
	fadescreen 0
	special DrawWholeMapView
	fadedefaultbgm
	setvar VAR_QZ_0x5011, 1
	release
	end

@ Johto_SproutTower3F object 4 (local id 4, gfx 0x3EA4) at 11,3
Johto_SproutTower3F_EventScript_83FA8B7::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x1004, Johto_SproutTower3F_EventScript_83FA8EB
	msgbox Johto_SproutTower3F_Text_83FAAE0, MSGBOX_DEFAULT
	closemessage
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4461, 0, Johto_SproutTower3F_Text_83FABB7
	msgbox Johto_SproutTower3F_Text_83FABC6, MSGBOX_DEFAULT
	setflag FLAG_QZ_0x1004
	msgbox Johto_SproutTower3F_Text_83FABF9, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_SproutTower3F_EventScript_83FA8EB::
	msgbox Johto_SproutTower3F_Text_83FAC3F, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_SproutTower3F_Movement_83FA8F6:
	walk_fast_up
	walk_fast_up
	step_end

Johto_SproutTower3F_Movement_83FA8F9:
	face_down
	delay_16
	walk_right
	walk_right
	walk_down
	walk_down
	step_end

Johto_SproutTower3F_Movement_83FA900:
	face_down
	delay_16
	walk_right
	walk_right
	walk_right
	walk_down
	walk_down
	step_end

Johto_SproutTower3F_Movement_83FA908:
	face_right
	face_up
	face_left
	face_down
	face_right
	face_up
	face_left
	face_down
	face_right
	face_up
	face_left
	face_down
	step_end

Johto_SproutTower3F_Text_83FA915::
	.string "ELDER: You are indeed skilled\n"
	.string "as a trainer.\p"
	.string "As promised, here is your HM.\p"
	.string "But let me say this: You should\n"
	.string "treat your POKéMON better.\p"
	.string "The way you battle is far too\n"
	.string "harsh.\p"
	.string "POKéMON are not tools of war…$"

Johto_SproutTower3F_Text_83FA9DD::
	.string "…\n"
	.string "…Humph!\p"
	.string "He claims to be the ELDER but\n"
	.string "he’s weak.\p"
	.string "It stands to reason.\p"
	.string "I’d never lose to fools who babble\n"
	.string "about being nice to POKéMON.\p"
	.string "I only care about strong\n"
	.string "POKéMON that can win.\p"
	.string "I really couldn’t care less\n"
	.string "about weak POKéMON.$"

Johto_SproutTower3F_Text_83FAAC4::
	.string "SILVER used an ESCAPE ROPE!$"

Johto_SproutTower3F_Text_83FAAE0::
	.string "So good of you to come here!\p"
	.string "SPROUT TOWER is a place of\n"
	.string "training.\p"
	.string "People and POKéMON test their\n"
	.string "bonds to build a bright future\l"
	.string "together.\p"
	.string "I am the final test.\p"
	.string "Allow me to check the ties\n"
	.string "between your POKéMON and you!$"

Johto_SproutTower3F_Text_83FABB7::
	.string "Ah, excellent!$"

Johto_SproutTower3F_Text_83FABC6::
	.string "You and your POKéMON have passed\n"
	.string "the tower’s test.$"

Johto_SproutTower3F_Text_83FABF9::
	.string "Trust in that bond to face\n"
	.string "the challenges ahead.\p"
	.string "Have a safe journey!$"

Johto_SproutTower3F_Text_83FAC3F::
	.string "I hope you learn and grow from\n"
	.string "your journey.$"

Johto_SproutTower3F_Text_83FAC6C::
	.string "I train to find enlightenment\n"
	.string "in POKéMON!$"

Johto_SproutTower3F_Text_83FAC96::
	.string "My training is incomplete…$"

Johto_SproutTower3F_Text_83FACB1::
	.string "As POKéMON grow stronger,\n"
	.string "so does the trainer.\p"
	.string "No, wait. As the trainer\n"
	.string "grows stronger, so do the\l"
	.string "POKéMON.$"

Johto_SproutTower3F_Text_83FAD1C::
	.string "Let me see how much you trust\n"
	.string "your POKéMON.$"

Johto_SproutTower3F_Text_83FAD48::
	.string "Yes, your trust is real!$"

Johto_SproutTower3F_Text_83FAD61::
	.string "It is not far to the ELDER.$"

Johto_SproutTower3F_Text_83FAD7D::
	.string "The ELDER’s HM lights even\n"
	.string "pitch-black darkness.$"

Johto_SproutTower3F_Text_83FADAE::
	.string "It is my head that is bright!$"

Johto_SproutTower3F_Text_83FADCC::
	.string "Let there be light on your\n"
	.string "journey.$"

Johto_SproutTower3F_Text_83FADF0::
	.string "It’s a powerful painting of a\n"
	.string "BELLSPROUT.$"
