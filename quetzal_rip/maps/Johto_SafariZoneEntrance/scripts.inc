Johto_SafariZoneEntrance_MapScripts::
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Johto_SafariZoneEntrance_OnFrame_83F8F17
	.byte 0

Johto_SafariZoneEntrance_OnFrame_83F8F17:
	map_script_2 VAR_QZ_0x500F, 3, Johto_SafariZoneEntrance_EventScript_83F8F76
	map_script_2 VAR_QZ_0x5032, 2, Johto_SafariZoneEntrance_EventScript_83F8F29
	.2byte 0

@ Johto_SafariZoneEntrance object 6 (local id 6, gfx 0x3E87) at 13,6
Johto_SafariZoneEntrance_EventScript_8300732::
	msgbox Johto_SafariZoneEntrance_Text_8382086, MSGBOX_NPC
	end

@ Johto_SafariZoneEntrance object 5 (local id 5, gfx 0x8) at 14,7
Johto_SafariZoneEntrance_EventScript_830073B::
	msgbox Johto_SafariZoneEntrance_Text_83820A2, MSGBOX_NPC
	end

@ Johto_SafariZoneEntrance object 10 (local id 10, gfx 0x3EA7) at 16,7
Johto_SafariZoneEntrance_EventScript_8300762::
	lock
	faceplayer
	msgbox Johto_SafariZoneEntrance_Text_83B2BA4, MSGBOX_DEFAULT
	goto Johto_SafariZoneEntrance_EventScript_83007C3

Johto_SafariZoneEntrance_EventScript_83007C3::
	closemessage
	applymovement VAR_LAST_TALKED, Quetzal_Movement_8360803
	waitmovement 0
	release
	end

Johto_SafariZoneEntrance_EventScript_838180D::
	copyvar VAR_0x8004, VAR_0x8009
	fadescreen 1
	special DoBerryBlending
	waitstate
	releaseall
	end

Johto_SafariZoneEntrance_EventScript_8381845::
	msgbox Johto_SafariZoneEntrance_Text_83820DD, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Johto_SafariZoneEntrance_EventScript_83818A7
	goto Johto_SafariZoneEntrance_EventScript_838185E

Johto_SafariZoneEntrance_EventScript_838185E::
	msgbox Johto_SafariZoneEntrance_Text_8382119, MSGBOX_DEFAULT
	releaseall
	end

Johto_SafariZoneEntrance_EventScript_8381868::
	msgbox Johto_SafariZoneEntrance_Text_8382270, MSGBOX_DEFAULT
	release
	end

Johto_SafariZoneEntrance_EventScript_8381872::
	msgbox Johto_SafariZoneEntrance_Text_8382168, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Johto_SafariZoneEntrance_EventScript_838188B
	goto Johto_SafariZoneEntrance_EventScript_8381899

Johto_SafariZoneEntrance_EventScript_838188B::
	msgbox Johto_SafariZoneEntrance_Text_838219F, MSGBOX_DEFAULT
	goto Johto_SafariZoneEntrance_EventScript_838180D

Johto_SafariZoneEntrance_EventScript_8381899::
	msgbox Johto_SafariZoneEntrance_Text_83821CD, MSGBOX_DEFAULT
	goto Johto_SafariZoneEntrance_EventScript_838188B

Johto_SafariZoneEntrance_EventScript_83818A7::
	specialvar VAR_RESULT, PlayerHasBerries
	goto_if_eq VAR_RESULT, 0, Johto_SafariZoneEntrance_EventScript_8381868
	msgbox Johto_SafariZoneEntrance_Text_8382113, MSGBOX_DEFAULT
	goto Johto_SafariZoneEntrance_EventScript_8381872

Johto_SafariZoneEntrance_EventScript_83818FC::
	msgbox Johto_SafariZoneEntrance_Text_838271E, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Johto_SafariZoneEntrance_EventScript_838195E
	goto Johto_SafariZoneEntrance_EventScript_8381915

Johto_SafariZoneEntrance_EventScript_8381915::
	msgbox Johto_SafariZoneEntrance_Text_8382434, MSGBOX_DEFAULT
	releaseall
	end

Johto_SafariZoneEntrance_EventScript_838191F::
	msgbox Johto_SafariZoneEntrance_Text_8382559, MSGBOX_DEFAULT
	release
	end

Johto_SafariZoneEntrance_EventScript_8381929::
	msgbox Johto_SafariZoneEntrance_Text_838245A, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Johto_SafariZoneEntrance_EventScript_8381942
	goto Johto_SafariZoneEntrance_EventScript_8381950

Johto_SafariZoneEntrance_EventScript_8381942::
	msgbox Johto_SafariZoneEntrance_Text_838248D, MSGBOX_DEFAULT
	goto Johto_SafariZoneEntrance_EventScript_838180D

Johto_SafariZoneEntrance_EventScript_8381950::
	msgbox Johto_SafariZoneEntrance_Text_83824C1, MSGBOX_DEFAULT
	goto Johto_SafariZoneEntrance_EventScript_8381942

Johto_SafariZoneEntrance_EventScript_838195E::
	specialvar VAR_RESULT, PlayerHasBerries
	goto_if_eq VAR_RESULT, 0, Johto_SafariZoneEntrance_EventScript_838191F
	msgbox Johto_SafariZoneEntrance_Text_838242A, MSGBOX_DEFAULT
	goto Johto_SafariZoneEntrance_EventScript_8381929

Johto_SafariZoneEntrance_EventScript_8381990::
	lockall
	setvar VAR_0x8009, 1
	msgbox Johto_SafariZoneEntrance_Text_83B2C1C, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Johto_SafariZoneEntrance_EventScript_83819BD
	msgbox Johto_SafariZoneEntrance_Text_83B2C5A, MSGBOX_DEFAULT
	releaseall
	end

Johto_SafariZoneEntrance_EventScript_83819B3::
	msgbox Johto_SafariZoneEntrance_Text_83B2CC5, MSGBOX_DEFAULT
	releaseall
	end

Johto_SafariZoneEntrance_EventScript_83819BD::
	specialvar VAR_RESULT, PlayerHasBerries
	goto_if_eq VAR_RESULT, 0, Johto_SafariZoneEntrance_EventScript_83819B3
	goto Johto_SafariZoneEntrance_EventScript_83819D3

Johto_SafariZoneEntrance_EventScript_83819D3::
	msgbox Johto_SafariZoneEntrance_Text_83B2D22, MSGBOX_YESNO
	call_if_eq VAR_RESULT, 0, Johto_SafariZoneEntrance_EventScript_83819F4
	msgbox Johto_SafariZoneEntrance_Text_83B2E43, MSGBOX_DEFAULT
	goto Johto_SafariZoneEntrance_EventScript_838180D

Johto_SafariZoneEntrance_EventScript_83819F4::
	msgbox Johto_SafariZoneEntrance_Text_83B2D74, MSGBOX_DEFAULT
	return

@ Johto_SafariZoneEntrance object 8 (local id 8, gfx 0x3E90) at 16,11
Johto_SafariZoneEntrance_EventScript_8381A11::
	msgbox Johto_SafariZoneEntrance_Text_83826D5, MSGBOX_NPC
	end

@ Johto_SafariZoneEntrance object 7 (local id 7, gfx 0x14) at 12,7
Johto_SafariZoneEntrance_EventScript_8381A1A::
	msgbox Johto_SafariZoneEntrance_Text_838271E, MSGBOX_NPC
	end

@ Johto_SafariZoneEntrance object 9 (local id 9, gfx 0x6) at 18,11
Johto_SafariZoneEntrance_EventScript_8381A23::
	msgbox Johto_SafariZoneEntrance_Text_838276B, MSGBOX_NPC
	end

@ Johto_SafariZoneEntrance sign / bg event at 17,7
Johto_SafariZoneEntrance_EventScript_83F8ECA::
	goto Johto_SafariZoneEntrance_EventScript_8381990

@ Johto_SafariZoneEntrance sign / bg event at 17,11
Johto_SafariZoneEntrance_EventScript_83F8ED0::
	lockall
	setvar VAR_0x8009, 2
	applymovement 9, Quetzal_Movement_8360803
	applymovement 8, Johto_SafariZoneEntrance_Movement_8381AAD
	waitmovement 0
	goto Johto_SafariZoneEntrance_EventScript_8381845

@ Johto_SafariZoneEntrance sign / bg event at 13,7
Johto_SafariZoneEntrance_EventScript_83F8EED::
	lockall
	setvar VAR_0x8009, 3
	applymovement 6, Quetzal_Movement_8360803
	applymovement 5, Quetzal_Movement_8360803
	applymovement 7, Johto_SafariZoneEntrance_Movement_8381AAD
	waitmovement 0
	goto Johto_SafariZoneEntrance_EventScript_83818FC

Johto_SafariZoneEntrance_EventScript_83F8F29::
	lock
	turnobject LOCALID_PLAYER, 4
	msgbox Johto_SafariZoneEntrance_Text_839360E, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Johto_SafariZoneEntrance_EventScript_83F8F61
	special ExitSafariMode
	setvar VAR_QZ_0x5032, 1
	setflag FLAG_QZ_0x1001
	msgbox Johto_SafariZoneEntrance_Text_8393C62, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, Quetzal_Movement_8360839
	waitmovement 0
	release
	end

Johto_SafariZoneEntrance_EventScript_83F8F61::
	closemessage
	applymovement LOCALID_PLAYER, Johto_SafariZoneEntrance_Movement_83F97DB
	waitmovement 0
	warp MAP_JOHTO_SAFARI_ZONE_ENTRANCE_PATH, 0, -1, -1
	waitstate
	end

Johto_SafariZoneEntrance_EventScript_83F8F76::
	lock
	playse SE_PIN
	applymovement 1, Quetzal_Movement_83607F9
	delay 0x3C
	applymovement 1, Johto_SafariZoneEntrance_Movement_83F97DD
	applymovement LOCALID_PLAYER, Quetzal_Movement_8360837
	waitmovement 1
	msgbox Johto_SafariZoneEntrance_Text_83F97E9, MSGBOX_DEFAULT
	closemessage
	call Johto_SafariZoneEntrance_EventScript_83F9007
	closemessage
	applymovement 1, Johto_SafariZoneEntrance_Movement_83F97E3
	waitmovement 0
	release
	end

@ Johto_SafariZoneEntrance object 1 (local id 1, gfx 0x30) at 5,10
Johto_SafariZoneEntrance_EventScript_83F8FB0::
	lock
	faceplayer
	goto_if_eq VAR_QZ_0x500F, 5, Johto_SafariZoneEntrance_EventScript_83F8FFF
	goto_if_eq VAR_QZ_0x502F, 1, Johto_SafariZoneEntrance_EventScript_83F903B
	goto_if_eq VAR_QZ_0x502F, 2, Johto_SafariZoneEntrance_EventScript_83F909A
	goto_if_eq VAR_QZ_0x502F, 3, Johto_SafariZoneEntrance_EventScript_83F90F9
	goto_if_eq VAR_QZ_0x502F, 4, Johto_SafariZoneEntrance_EventScript_83F9158
	goto_if_ge VAR_QZ_0x502F, 5, Johto_SafariZoneEntrance_EventScript_83F91CC
	msgbox Johto_SafariZoneEntrance_Text_83F999D, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_SafariZoneEntrance_EventScript_83F8FFF::
	call Johto_SafariZoneEntrance_EventScript_83F9007
	closemessage
	release
	end

Johto_SafariZoneEntrance_EventScript_83F9007::
	msgbox Johto_SafariZoneEntrance_Text_83F98D6, MSGBOX_DEFAULT
	closemessage
	setorcopyvar VAR_0x8000, 0xC
	setorcopyvar VAR_0x8001, 0xA
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_0x8007, 0, Johto_SafariZoneEntrance_EventScript_83F902D
	setvar VAR_QZ_0x500F, 4
	return

Johto_SafariZoneEntrance_EventScript_83F902D::
	setvar VAR_QZ_0x500F, 5
	msgbox Johto_SafariZoneEntrance_Text_83F9952, MSGBOX_DEFAULT
	return

Johto_SafariZoneEntrance_EventScript_83F903B::
	msgbox Johto_SafariZoneEntrance_Text_83F9442, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Johto_SafariZoneEntrance_EventScript_83F91C1
	special ChoosePartyMon
	waitstate
	lockall
	copyvar VAR_0x800A, VAR_0x8004
	goto_if_eq VAR_0x8004, 255, Johto_SafariZoneEntrance_EventScript_83F91C1
	qz_checkpartymoninlist 1
	goto_if_eq VAR_RESULT, 0, Johto_SafariZoneEntrance_EventScript_83F91B7
	msgbox Johto_SafariZoneEntrance_Text_83F9214, MSGBOX_DEFAULT
	showmoneybox 0, 0, 0
	addmoney 0x30D40, 0
	delay 0x3C
	playse SE_SHOP
	updatemoneybox 0
	delay 0x3C
	closemessage
	hidemoneybox
	setvar VAR_QZ_0x502F, 2
	release
	end

Johto_SafariZoneEntrance_EventScript_83F909A::
	msgbox Johto_SafariZoneEntrance_Text_83F94A5, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Johto_SafariZoneEntrance_EventScript_83F91C1
	special ChoosePartyMon
	waitstate
	lockall
	copyvar VAR_0x800A, VAR_0x8004
	goto_if_eq VAR_0x8004, 255, Johto_SafariZoneEntrance_EventScript_83F91C1
	qz_checkpartymoninlist 2
	goto_if_eq VAR_RESULT, 0, Johto_SafariZoneEntrance_EventScript_83F91B7
	msgbox Johto_SafariZoneEntrance_Text_83F928E, MSGBOX_DEFAULT
	showmoneybox 0, 0, 0
	addmoney 0x493E0, 0
	delay 0x3C
	playse SE_SHOP
	updatemoneybox 0
	delay 0x3C
	closemessage
	hidemoneybox
	setvar VAR_QZ_0x502F, 3
	release
	end

Johto_SafariZoneEntrance_EventScript_83F90F9::
	msgbox Johto_SafariZoneEntrance_Text_83F9508, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Johto_SafariZoneEntrance_EventScript_83F91C1
	special ChoosePartyMon
	waitstate
	lockall
	copyvar VAR_0x800A, VAR_0x8004
	goto_if_eq VAR_0x8004, 255, Johto_SafariZoneEntrance_EventScript_83F91C1
	qz_checkpartymoninlist 3
	goto_if_eq VAR_RESULT, 0, Johto_SafariZoneEntrance_EventScript_83F91B7
	msgbox Johto_SafariZoneEntrance_Text_83F930B, MSGBOX_DEFAULT
	showmoneybox 0, 0, 0
	addmoney 0x61A80, 0
	delay 0x3C
	playse SE_SHOP
	updatemoneybox 0
	delay 0x3C
	closemessage
	hidemoneybox
	setvar VAR_QZ_0x502F, 4
	release
	end

Johto_SafariZoneEntrance_EventScript_83F9158::
	msgbox Johto_SafariZoneEntrance_Text_83F956E, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Johto_SafariZoneEntrance_EventScript_83F91C1
	special ChoosePartyMon
	waitstate
	lockall
	copyvar VAR_0x800A, VAR_0x8004
	goto_if_eq VAR_0x8004, 255, Johto_SafariZoneEntrance_EventScript_83F91C1
	qz_checkpartymoninlist 4
	goto_if_eq VAR_RESULT, 0, Johto_SafariZoneEntrance_EventScript_83F91B7
	msgbox Johto_SafariZoneEntrance_Text_83F9384, MSGBOX_DEFAULT
	showmoneybox 0, 0, 0
	addmoney 0x61A80, 0
	delay 0x14
	playse SE_SHOP
	updatemoneybox 0
	delay 0x14
	closemessage
	hidemoneybox
	setvar VAR_QZ_0x502F, 5
	release
	end

Johto_SafariZoneEntrance_EventScript_83F91B7::
	msgbox Johto_SafariZoneEntrance_Text_83F942D, MSGBOX_DEFAULT
	release
	end

Johto_SafariZoneEntrance_EventScript_83F91C1::
	msgbox Johto_SafariZoneEntrance_Text_83F95D0, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_SafariZoneEntrance_EventScript_83F91CC::
	msgbox Johto_SafariZoneEntrance_Text_83F9624, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_SafariZoneEntrance object 2 (local id 2, gfx 0xDF) at 15,3
Johto_SafariZoneEntrance_EventScript_83F97AD::
	msgbox Johto_SafariZoneEntrance_Text_83F9A44, MSGBOX_NPC
	end

@ Johto_SafariZoneEntrance object 3 (local id 3, gfx 0xDF) at 12,2
Johto_SafariZoneEntrance_EventScript_83F97B6::
	msgbox Johto_SafariZoneEntrance_Text_83F9AAD, MSGBOX_NPC
	end

@ Johto_SafariZoneEntrance object 4 (local id 4, gfx 0xB) at 2,5
Johto_SafariZoneEntrance_EventScript_83F97BF::
	lock
	faceplayer
	msgbox Johto_SafariZoneEntrance_Text_83F99CA, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_SafariZoneEntrance_Movement_8381AAD:
	walk_in_place_faster_right
	step_end

Johto_SafariZoneEntrance_Movement_83F97DB:
	walk_up
	step_end

Johto_SafariZoneEntrance_Movement_83F97DD:
	walk_right
	walk_right
	walk_right
	walk_right
	walk_down
	step_end

Johto_SafariZoneEntrance_Movement_83F97E3:
	walk_up
	walk_left
	walk_left
	walk_left
	walk_left
	step_end

Johto_SafariZoneEntrance_Text_8382086::
	.string "Let’s get blending already!$"

Johto_SafariZoneEntrance_Text_83820A2::
	.string "I wonder what kind of オカキクケ I’ll get?\n"
	.string "This is so exciting!$"

Johto_SafariZoneEntrance_Text_83820DD::
	.string "Hi, there! Did you want to blend some\n"
	.string "オカキクケS with us?$"

Johto_SafariZoneEntrance_Text_8382113::
	.string "Okay!$"

Johto_SafariZoneEntrance_Text_8382119::
	.string "That’s too bad…\p"
	.string "But we’ll always be around whenever\n"
	.string "you get the urge to blend!$"

Johto_SafariZoneEntrance_Text_8382168::
	.string "Of course, you do know how to\n"
	.string "blend オカキクケS, don’t you?$"

Johto_SafariZoneEntrance_Text_838219F::
	.string "Let’s get started, then!\p"
	.string "Let’s BERRY BLENDER!$"

Johto_SafariZoneEntrance_Text_83821CD::
	.string "Okay!\n"
	.string "Let me explain it to you!\p"
	.string "When the spinning BLENDER’s arrow\n"
	.string "reaches your marker, just press\l"
	.string "the A Button.\p"
	.string "That’s all it takes.\n"
	.string "Pretty easy, don’t you think?$"

Johto_SafariZoneEntrance_Text_8382270::
	.string "Oh, but wait a second here…\n"
	.string "You don’t have any BERRIES.\p"
	.string "You can’t make any オカキクケS without\n"
	.string "BERRIES…\p"
	.string "We’ll always be around whenever you\n"
	.string "get hold of some BERRIES to blend.$"

Johto_SafariZoneEntrance_Text_838242A::
	.string "Oh, dear!$"

Johto_SafariZoneEntrance_Text_8382434::
	.string "Oh, dear me…\p"
	.string "You’ve left us in shock!$"

Johto_SafariZoneEntrance_Text_838245A::
	.string "Naturally, you know how to make\n"
	.string "オカキクケS, don’t you?$"

Johto_SafariZoneEntrance_Text_838248D::
	.string "Okay, dear!\n"
	.string "Let’s get started!\p"
	.string "Let’s BERRY BLENDER!$"

Johto_SafariZoneEntrance_Text_83824C1::
	.string "Oh, dear!\p"
	.string "Then, I’ll explain it to you nicely.\p"
	.string "When the BLENDER’s arrow spins to\n"
	.string "your marker, press the A Button.\p"
	.string "That’s all it takes.\n"
	.string "Isn’t it simple?$"

Johto_SafariZoneEntrance_Text_8382559::
	.string "You don’t have any BERRIES,\n"
	.string "do you?\p"
	.string "If you don’t have any BERRIES,\n"
	.string "you can’t make any オカキクケS.\p"
	.string "We’ll always be making オカキクケS here,\n"
	.string "so let’s make some together when\l"
	.string "you get a BERRY or two.$"

Johto_SafariZoneEntrance_Text_83826D5::
	.string "Okay! Today’s going to be the day that\n"
	.string "I set a new BLENDER speed record!$"

Johto_SafariZoneEntrance_Text_838271E::
	.string "Oh, dear!\n"
	.string "You look as if you’re good at blending.\l"
	.string "Would you like to join us?$"

Johto_SafariZoneEntrance_Text_838276B::
	.string "I’m going to make delicious オカキクケS\n"
	.string "and make my POKéMON cuter.$"

Johto_SafariZoneEntrance_Text_839360E::
	.string "Would you like to exit the SAFARI\n"
	.string "ZONE right now?$"

Johto_SafariZoneEntrance_Text_8393C62::
	.string "Okay.\p"
	.string "I’ll take back your remaining SAFARI\n"
	.string "BALLS.\p"
	.string "Thank you for playing.\n"
	.string "We hope to see you again.$"

Johto_SafariZoneEntrance_Text_83B2BA4::
	.string "BLEND MASTER: Indeed I am!\n"
	.string "The BLEND MASTER am I!\p"
	.string "Blend with me, and you shall witness\n"
	.string "the mastery I bring to blending!$"

Johto_SafariZoneEntrance_Text_83B2C1C::
	.string "BLEND MASTER: Hmmm! So, you wish to\n"
	.string "see my mastery in action?$"

Johto_SafariZoneEntrance_Text_83B2C5A::
	.string "Hmmm!\p"
	.string "So, you are too busy now, I see!\p"
	.string "But fear not!\n"
	.string "I shall be here all day!\l"
	.string "Hurry back from your errand!$"

Johto_SafariZoneEntrance_Text_83B2CC5::
	.string "Hmmm!\p"
	.string "You haven’t got a single BERRY!\p"
	.string "I shall be here all day!\n"
	.string "Hurry back with some BERRIES!$"

Johto_SafariZoneEntrance_Text_83B2D22::
	.string "Of course!\n"
	.string "Of course!\p"
	.string "Incidentally…\n"
	.string "You do know how to blend オカキクケS\l"
	.string "from BERRIES?$"

Johto_SafariZoneEntrance_Text_83B2D74::
	.string "Hmmm!\p"
	.string "Ah, but it is a simple process!\p"
	.string "When the BLENDER’s arrow comes to\n"
	.string "your marker, just press the A Button.\p"
	.string "That’s all you have to do.\p"
	.string "When you see how precisely I press\n"
	.string "the A Button, you will understand.$"

Johto_SafariZoneEntrance_Text_83B2E43::
	.string "Fine!\p"
	.string "Let’s get started, then!\p"
	.string "All together with the BLEND MASTER,\n"
	.string "let’s BERRY BLENDER!$"

Johto_SafariZoneEntrance_Text_83F9214::
	.string "Thank you, {PLAYER}!\n"
	.string "Here, take this money to upgrade\l"
	.string "your SAFARI LEVEL and show me an\l"
	.string "exotic POKéMON from the BRUSH area\l"
	.string "next!$"

Johto_SafariZoneEntrance_Text_83F928E::
	.string "Thank you, {PLAYER}!\n"
	.string "Here, take this money to upgrade\l"
	.string "your SAFARI LEVEL and show me an\l"
	.string "exotic POKéMON from the MOUNTAIN\l"
	.string "area next!$"

Johto_SafariZoneEntrance_Text_83F930B::
	.string "Thank you, {PLAYER}!\n"
	.string "Here, take this money to upgrade\l"
	.string "your SAFARI LEVEL and show me an\l"
	.string "exotic POKéMON from the CAVE area\l"
	.string "next!$"

Johto_SafariZoneEntrance_Text_83F9384::
	.string "Thank you, {PLAYER}!\n"
	.string "That’s all I need to see.\p"
	.string "Now that I know what DEVON is up\n"
	.string "to, I’ll be able to keep my SAFARI\l"
	.string "competitive. Here, take this as a\l"
	.string "reward for your troubles.$"

Johto_SafariZoneEntrance_Text_83F942D::
	.string "That can’t be right!$"

Johto_SafariZoneEntrance_Text_83F9442::
	.string "Ah, {PLAYER}!\p"
	.string "Did you bring an exotic POKéMON\n"
	.string "from the BEACH area of the FUCHSIA\l"
	.string "SAFARI ZONE to show me?$"

Johto_SafariZoneEntrance_Text_83F94A5::
	.string "Ah, {PLAYER}!\p"
	.string "Did you bring an exotic POKéMON\n"
	.string "from the BRUSH area of the FUCHSIA\l"
	.string "SAFARI ZONE to show me?$"

Johto_SafariZoneEntrance_Text_83F9508::
	.string "Ah, {PLAYER}!\p"
	.string "Did you bring an exotic POKéMON\n"
	.string "from the MOUNTAIN area of the\l"
	.string "FUCHSIA SAFARI ZONE to show me?$"

Johto_SafariZoneEntrance_Text_83F956E::
	.string "Ah, {PLAYER}!\p"
	.string "Did you bring an exotic POKéMON\n"
	.string "from the CAVE area of the FUCHSIA\l"
	.string "SAFARI ZONE to show me?$"

Johto_SafariZoneEntrance_Text_83F95D0::
	.string "What will I do now?\n"
	.string "We’ll never keep up with DEVON!$"

Johto_SafariZoneEntrance_Text_83F9624::
	.string "Ah, {PLAYER}!\p"
	.string "Thanks to you, the SAFARI ZONE is\n"
	.string "thriving!\p"
	.string "Maybe I should name you OWNER of\n"
	.string "the safari zone...\p"
	.string "No, that would be ridiculous.\n"
	.string "Ahahahaha.$"

Johto_SafariZoneEntrance_Text_83F97E9::
	.string "Hi {PLAYER}!\n"
	.string "I’ve been waiting for you.\n"
	.string "What do you think?\p"
	.string "This is a brand new SAFARI ZONE,\n"
	.string "with all six areas ready to explore!\p"
	.string "Every area has different wild\n"
	.string "POKéMON waiting to be discovered.\p"
	.string "Enjoy your visit and explore\n"
	.string "as much as you like!$"

Johto_SafariZoneEntrance_Text_83F98D6::
	.string "Some Poké Balls to start your visit!\p"
	.string "Use them at the start of a battle.\n"
	.string "They’ll help you meet the\l"
	.string "residents of our reserve!$"

Johto_SafariZoneEntrance_Text_83F9952::
	.string "Make room in your BAG\n"
	.string "and come talk to me again.\p"
	.string "I’ll keep your gift safe!$"

Johto_SafariZoneEntrance_Text_83F999D::
	.string "Hi {PLAYER}!\p"
	.string "How are you enjoying the SAFARI\n"
	.string "ZONE?$"

Johto_SafariZoneEntrance_Text_83F99CA::
	.string "I heard one of the areas has\n"
	.string "rare dragon POKéMON!$"

Johto_SafariZoneEntrance_Text_83F9A44::
	.string "Welcome to JOHTO’S SAFARI ZONE!\p"
	.string "Entry is free. You can explore all six\n"
	.string "areas with no time or step limit.$"

Johto_SafariZoneEntrance_Text_83F9AAD::
	.string "Wild POKéMON here are battled and\n"
	.string "caught normally, just like on any\l"
	.string "other route.\p"
	.string "Each area has different wildlife.\n"
	.string "Bring your own POKé BALLS!$"
