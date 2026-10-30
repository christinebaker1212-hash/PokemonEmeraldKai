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
	msgbox Johto_Route29_Text_83ECDCD, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Johto_Route29_EventScript_83ECC5B
	waitmessage
	msgbox Johto_Route29_Text_83ECF1E, MSGBOX_DEFAULT
	closemessage
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
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x1022, Johto_Route29_EventScript_83ECD40
	msgbox Johto_Route29_Text_83ED16A, MSGBOX_DEFAULT
	closemessage
	multichoice 0x14, 0, MULTI_QZ_0x4A, 1
	nop
	copyvar VAR_0x8000, VAR_RESULT
	goto_if_eq VAR_0x8000, 0, Johto_Route29_EventScript_83ECD32
	goto_if_eq VAR_0x8000, 1, Johto_Route29_EventScript_83ECD32
	goto_if_eq VAR_0x8000, 2, Johto_Route29_EventScript_83ECD05
	goto_if_eq VAR_0x8000, 3, Johto_Route29_EventScript_83ECD32
	goto_if_eq VAR_0x8000, 4, Johto_Route29_EventScript_83ECD32
	goto_if_eq VAR_0x8000, 5, Johto_Route29_EventScript_83ECD32
	goto_if_eq VAR_0x8000, 6, Johto_Route29_EventScript_83ECD32
	goto_if_eq VAR_0x8000, 7, Johto_Route29_EventScript_83ECD3D
	goto_if_eq VAR_0x8000, 127, Johto_Route29_EventScript_83ECD3D
	closemessage
	release
	end

Johto_Route29_EventScript_83ECD05::
	msgbox Johto_Route29_Text_83ED20F, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, ITEM_SILK_SCARF
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_83602AA
	setflag FLAG_QZ_0x1022
	msgbox Johto_Route29_Text_83ED253, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_Route29_EventScript_83ECD32::
	msgbox Quetzal_Text_83ED204, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_Route29_EventScript_83ECD3D::
	closemessage
	release
	end

Johto_Route29_EventScript_83ECD40::
	msgbox Johto_Route29_Text_83ED2CB, MSGBOX_DEFAULT
	closemessage
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
	goto_if_set FLAG_QZ_0x1015, Johto_Route29_EventScript_83ECD6E
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
	.string "POKéMON hide in the grass.\n"
	.string "Who knows when they’ll pop out…$"

Johto_Route29_Text_83ECDCD::
	.string "I’ve seen you a couple times.\n"
	.string "How many POKéMON have you caught?\p"
	.string "Would you like me to tell you\n"
	.string "how to catch POKéMON?$"

Johto_Route29_Text_83ECE41::
	.string "If you weaken them first,\n"
	.string "POKéMON are easier to catch.\p"
	.string "Then, throw a ball at them from\n"
	.string "your bag! Any ball will do.\p"
	.string "If you want to learn more,\n"
	.string "go to the ACADEMY in VIOLET CITY!\p"
	.string "The students there know a lot\n"
	.string "about POKéMON.$"

Johto_Route29_Text_83ECF1E::
	.string "Oh. Fine, then.\p"
	.string "Check every route to find\n"
	.string "different types of POKéMON.\p"
	.string "Your POKéDEX shows where you can\n"
	.string "find them too!\p"
	.string "If you want to learn more,\n"
	.string "go to the ACADEMY in VIOLET CITY!\p"
	.string "The students know a lot about\n"
	.string "POKéMON.$"

Johto_Route29_Text_83ECFF8::
	.string "Yo. How are your POKéMON?\p"
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
	.string "I’m waiting for POKéMON that\n"
	.string "appear only later in the day.$"

Johto_Route29_Text_83ED137::
	.string "I’m waiting for POKéMON that\n"
	.string "appear only at night.$"

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
