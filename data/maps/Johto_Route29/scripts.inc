Johto_Route29_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_83ECB9A
	.byte 0

@ Johto_Route29 trigger at 4,9 (VAR_TEMP_0 == 0)
Johto_Route29_EventScript_83ECBA0::
	special SpawnCameraObject
	applymovement LOCALID_CAMERA, Quetzal_Movement_8360837
	waitmovement 0
	applymovement LOCALID_CAMERA, Quetzal_Movement_8360837
	waitmovement 0
	applymovement LOCALID_CAMERA, Quetzal_Movement_8360837
	waitmovement 0
	applymovement LOCALID_CAMERA, Quetzal_Movement_8360837
	waitmovement 0
	message Quetzal_Text_83ECC08
	waitbuttonpress
	closemessage
	applymovement LOCALID_CAMERA, Quetzal_Movement_8360839
	waitmovement 0
	applymovement LOCALID_CAMERA, Quetzal_Movement_8360839
	waitmovement 0
	applymovement LOCALID_CAMERA, Quetzal_Movement_8360839
	waitmovement 0
	applymovement LOCALID_CAMERA, Quetzal_Movement_8360839
	waitmovement 0
	special RemoveCameraObject
	applymovement LOCALID_PLAYER, Quetzal_Movement_8360839
	waitmovement 0
	end

@ Johto_Route29 object 1 (local id 1, gfx 0x3EA7) at 61,16
Johto_Route29_EventScript_83ECC26::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x14A0, Johto_Route29_EventScript_83ECC3C
	msgbox Johto_Route29_Text_83ECD92, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_Route29_EventScript_83ECC3C::
	@ Moemon Kai: Crystal's catching demonstration (pokecrystal Route29.asm CatchingTutorialDudeScript; docs/REGIONS_AUDIT.md
	@ 133): the DUDE catches a RATTATA with your POKéMON (battle_setup.c MP_JohtoCatchingTutorial), once
	goto_if_set FLAG_JOHTO_LEARNED_TO_CATCH, Johto_Route29_EventScript_DudeDone
	msgbox Johto_Route29_Text_83ECDCD, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Johto_Route29_EventScript_DudeDeclined
	closemessage
	special SavePlayerParty
	special MP_JohtoCatchingTutorial
	waitstate
	special LoadPlayerParty
	msgbox Johto_Route29_Text_83ECE41, MSGBOX_DEFAULT
	setflag FLAG_JOHTO_LEARNED_TO_CATCH
	release
	end

Johto_Route29_EventScript_DudeDeclined::
	msgbox Johto_Route29_Text_83ECF1E, MSGBOX_DEFAULT
	release
	end

Johto_Route29_EventScript_DudeDone::
	msgbox Johto_Route29_Text_DudeBoxFull, MSGBOX_DEFAULT
	release
	end

Johto_Route29_EventScript_83ECC5B::
	msgbox Johto_Route29_Text_83ECE41, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_Route29 object 7 (local id 7, gfx 0x2F) at 37,14
Johto_Route29_EventScript_83ECC66::
	lock
	faceplayer
	msgbox Johto_Route29_Text_83ED053, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_Route29 object 2 (local id 2, gfx 0x23) at 35,22
Johto_Route29_EventScript_83ECC73::
	lock
	faceplayer
	msgbox Johto_Route29_Text_83ECFF8, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_Route29 object 4 (local id 4, gfx 0x11) at 25,12
Johto_Route29_EventScript_83ECC80::
	@ Moemon Kai: Crystal's WEEKDAY sibling (pokecrystal Route29.asm TuscanyScript; tools/quetzal_port/fix_siblings.py):
	@ only on TUESDAY (FLAG_JOHTO_HIDE_TUSCANY), once met they give the gift
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x1022, Johto_Route29_EventScript_TuscanyDay
	specialvar VAR_RESULT, MP_JohtoWeekday
	goto_if_ne VAR_RESULT, 2, Johto_Route29_EventScript_TuscanyNotDay
	goto_if_set FLAG_JOHTO_MET_TUSCANY, Johto_Route29_EventScript_TuscanyMet
	msgbox Johto_Route29_Text_TuscanyMeet, MSGBOX_DEFAULT
	setflag FLAG_JOHTO_MET_TUSCANY
Johto_Route29_EventScript_TuscanyMet::
	msgbox Johto_Route29_Text_TuscanyGivesGift, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, ITEM_SILK_SCARF
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_83602AA
	setflag FLAG_QZ_0x1022
	msgbox Johto_Route29_Text_TuscanyGaveGift, MSGBOX_DEFAULT
	release
	end

Johto_Route29_EventScript_TuscanyDay::
	msgbox Johto_Route29_Text_TuscanyTuesday, MSGBOX_DEFAULT
	release
	end

Johto_Route29_EventScript_TuscanyNotDay::
	msgbox Johto_Route29_Text_TuscanyNotTuesday, MSGBOX_DEFAULT
	release
	end

@ Johto_Route29 object 5 (local id 5, gfx 0x10) at 6,22
Johto_Route29_EventScript_83ECD4B::
	lock
	faceplayer
	msgbox Johto_Route29_Text_83ED0C0, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_Route29 object 3 (local id 3, gfx 0x3E87) at 11,12
Johto_Route29_EventScript_83ECD58::
	lock
	faceplayer
	@ Moemon Kai: Crystal's night check (checktime NITE: 6 PM to 4 AM on the game clock; docs/REGIONS_AUDIT.md 133)
	gettime
	goto_if_ge VAR_0x8000, 18, Johto_Route29_EventScript_83ECD6E
	goto_if_lt VAR_0x8000, 4, Johto_Route29_EventScript_83ECD6E
	msgbox Johto_Route29_Text_83ED0FC, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_Route29_EventScript_83ECD6E::
	msgbox Johto_Route29_Text_83ED137, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_Route29 sign / bg event at 64,15
@ Johto_Route29 sign / bg event at 2,13
Johto_Route29_EventScript_83ECD79::
	lock
	msgbox Johto_Route29_Text_83ED33B, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_Route29 object 8 (local id 8, gfx 0x3A9E) at 55,6
Johto_Route29_EventScript_83ECD85::
	setorcopyvar VAR_0x8000, ITEM_POTION
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Johto_Route29_Text_83ECD92::
	.string "MOéMON hide in the grass.\n"
	.string "Who knows when they’ll pop out…$"

Johto_Route29_Text_83ECDCD::
	.string "Huh? You want me to show you how\n"
	.string "to catch MOéMON?$"

Johto_Route29_Text_83ECE41::
	.string "That’s how you do it.\p"
	.string "If you weaken them first, MOéMON\n"
	.string "are easier to catch.$"

Johto_Route29_Text_83ECF1E::
	.string "Oh. Fine, then.\p"
	.string "Anyway, if you want to catch\n"
	.string "MOéMON, you have to walk a lot.$"

Johto_Route29_Text_83ECFF8::
	.string "Yo. How are your MOéMON?\p"
	.string "If they’re weak and not ready\n"
	.string "for battle, keep out of the grass.$"

Johto_Route29_Text_83ED053::
	.string "See those ledges?\n"
	.string "It’s scary to jump off them.\p"
	.string "But you can go to NEW BARK\n"
	.string "without walking through the grass.$"

Johto_Route29_Text_83ED0C0::
	.string "I wanted to take a break,\n"
	.string "so I saved to record my progress.$"

Johto_Route29_Text_83ED0FC::
	.string "I’m waiting for MOéMON that appear\n"
	.string "only at night.$"

Johto_Route29_Text_83ED137::
	.string "I’m waiting for MOéMON that appear\n"
	.string "only in the morning.$"

Johto_Route29_Text_83ED16A::
	.string "TUSCANY: I do believe that this\n"
	.string "is the first time we’ve met?\p"
	.string "I’m TUSCANY of the WEEKDAY FAMILY.\p"
	.string "If you can guess my favorite day,\n"
	.string "I’ll give you a reward.$"

Johto_Route29_Text_83ED20F::
	.string "Yes! By way of introduction,\n"
	.string "please accept this gift, a SILK\l"
	.string "SCARF.$"

Johto_Route29_Text_83ED253::
	.string "TUSCANY: Wouldn’t you agree\n"
	.string "that it is most adorable?\p"
	.string "It strengthens NORMAL-type\n"
	.string "moves.\p"
	.string "I am certain it will be of use.$"

Johto_Route29_Text_83ED2CB::
	.string "TUSCANY: Have you met MONICA,\n"
	.string "my older sister?\p"
	.string "Or my younger brother, WESLEY?\p"
	.string "I am the third of seven\n"
	.string "children.$"

Johto_Route29_Text_83ED33B::
	.string "ROUTE 29\p"
	.string "CHERRYGROVE CITY - NEW BARK TOWN$"

Johto_Route29_Text_TuscanyMeet::
	.string "TUSCANY: I do believe that this is\n"
	.string "the first time we’ve met?\p"
	.string "Please allow me to introduce\n"
	.string "myself.\p"
	.string "I am TUSCANY of Tuesday.$"

Johto_Route29_Text_TuscanyGaveGift::
	.string "TUSCANY: Wouldn’t you agree that\n"
	.string "it is most adorable?\p"
	.string "It strengthens normal-type moves.\p"
	.string "I am certain it will be of use.$"

Johto_Route29_Text_TuscanyTuesday::
	.string "TUSCANY: Have you met MONICA, my\n"
	.string "older sister?\p"
	.string "Or my younger brother, WESLEY?\p"
	.string "I am the second of seven children.$"

Johto_Route29_Text_TuscanyNotTuesday::
	.string "TUSCANY: Today is not Tuesday.\n"
	.string "That is unfortunate…$"

Johto_Route29_Text_TuscanyGivesGift::
	.string "By way of introduction, please\n"
	.string "accept this gift, a PINK BOW.$"

Johto_Route29_Text_DudeBoxFull::
	.string "MOéMON hide in the grass. Who\n"
	.string "knows when they’ll pop out…$"
