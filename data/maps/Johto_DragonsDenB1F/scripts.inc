Johto_DragonsDenB1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_DragonsDenB1F_EventScript_83CBEC5
	.byte 0

Johto_DragonsDenB1F_EventScript_83CBEC5::
	call Quetzal_EventScript_83CC300
	goto Quetzal_EventScript_840EA66

@ Johto_DragonsDenB1F object 21 (local id 21, gfx 0x3E8D) at 31,19
Johto_DragonsDenB1F_EventScript_83CBECF::
	lock
	faceplayer
	msgbox Johto_DragonsDenB1F_Text_83CBEDC, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_DragonsDenB1F object 1 (local id 1, gfx 0x3E90) at 36,13
Johto_DragonsDenB1F_EventScript_83CBF3D::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4566, 0, Johto_DragonsDenB1F_Text_83CC0FB, Johto_DragonsDenB1F_Text_83CC11E
	msgbox Johto_DragonsDenB1F_Text_83CC128, MSGBOX_AUTOCLOSE
	end

@ Johto_DragonsDenB1F object 4 (local id 4, gfx 0x3E8F) at 13,19
Johto_DragonsDenB1F_EventScript_83CBF54::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4808, 0, Johto_DragonsDenB1F_Text_83CC18A, Johto_DragonsDenB1F_Text_83CC1A4
	msgbox Johto_DragonsDenB1F_Text_83CC1B6, MSGBOX_AUTOCLOSE
	end

@ Johto_DragonsDenB1F object 3 (local id 3, gfx 0x6) at 7,31
Johto_DragonsDenB1F_EventScript_83CBF6B::
	trainerbattle TRAINER_BATTLE_DOUBLE, TRAINER_QZ_4146, 0, Johto_DragonsDenB1F_Text_83CC252, Johto_DragonsDenB1F_Text_83CC271, Quetzal_Text_8383BB5
	special GetPlayerBigGuyGirlString
	msgbox Johto_DragonsDenB1F_Text_83CC27A, MSGBOX_DEFAULT
	release
	end

@ Johto_DragonsDenB1F object 2 (local id 2, gfx 0x6) at 7,32
Johto_DragonsDenB1F_EventScript_83CBF8A::
	trainerbattle TRAINER_BATTLE_DOUBLE, TRAINER_QZ_4146, 0, Johto_DragonsDenB1F_Text_83CC29E, Johto_DragonsDenB1F_Text_83CC2AB, Quetzal_Text_8383BB5
	special GetPlayerBigGuyGirlString
	msgbox Johto_DragonsDenB1F_Text_83CC2B3, MSGBOX_DEFAULT
	release
	end

@ Johto_DragonsDenB1F object 8 (local id 8, gfx 0x3A9E) at 54,28
Johto_DragonsDenB1F_EventScript_83CBFA9::
	setorcopyvar VAR_0x8000, ITEM_DRAGON_FANG
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_DragonsDenB1F object 7 (local id 7, gfx 0x3A9E) at 7,35
Johto_DragonsDenB1F_EventScript_83CBFB6::
	setorcopyvar VAR_0x8000, ITEM_MAX_POTION
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_DragonsDenB1F object 6 (local id 6, gfx 0x3A9E) at 54,50
Johto_DragonsDenB1F_EventScript_83CBFC3::
	setorcopyvar VAR_0x8000, ITEM_NUGGET
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Johto_DragonsDenB1F_Text_83CBEDC::
	.string "{PLAYER}… So you’re here.\p"
	.string "If you think you can keep up,\n"
	.string "with me, come to the FIGHTING\l"
	.string "DOJO in SAFFRON.$"

Johto_DragonsDenB1F_Text_83CC0FB::
	.string "You! How dare you enter\n"
	.string "uninvited!$"

Johto_DragonsDenB1F_Text_83CC11E::
	.string "S-strong!$"

Johto_DragonsDenB1F_Text_83CC128::
	.string "The SHRINE ahead is home to the\n"
	.string "MASTER of our dragon-user clan.\p"
	.string "You’re not allowed to just go in!$"

Johto_DragonsDenB1F_Text_83CC18A::
	.string "You shouldn’t be in here!$"

Johto_DragonsDenB1F_Text_83CC1A4::
	.string "Oh yikes, I lost!$"

Johto_DragonsDenB1F_Text_83CC1B6::
	.string "Soon I’m going to get permission\n"
	.string "from our MASTER to use dragons.\p"
	.string "When I do, I’m going to become an\n"
	.string "admirable dragon trainer and gain\l"
	.string "our MASTER’s approval.$"

Johto_DragonsDenB1F_Text_83CC252::
	.string "It’s a stranger we don’t know.$"

Johto_DragonsDenB1F_Text_83CC271::
	.string "Ouchies.$"

Johto_DragonsDenB1F_Text_83CC27A::
	.string "It was like having to battle\n"
	.string "LANCE.$"

Johto_DragonsDenB1F_Text_83CC29E::
	.string "Who are you?$"

Johto_DragonsDenB1F_Text_83CC2AB::
	.string "Meanie.$"

Johto_DragonsDenB1F_Text_83CC2B3::
	.string "We’ll tell on you.\p"
	.string "MASTER will be angry with you.$"
