Kanto_SilphCo_5F_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Kanto_SilphCo_5F_EventScript_841151A
	.byte 0

Kanto_SilphCo_5F_EventScript_841151A::
	call_if_unset FLAG_QZ_0x21FC, Kanto_SilphCo_5F_EventScript_8458732
	call_if_unset FLAG_QZ_0x21FD, Kanto_SilphCo_5F_EventScript_8458769
	call_if_unset FLAG_QZ_0x21FE, Kanto_SilphCo_5F_EventScript_84587A0
	end

@ Kanto_SilphCo_5F object 3 (local id 3, gfx 0x3F5B) at 16,13
Kanto_SilphCo_5F_EventScript_8411536::
	lock
	faceplayer
	goto_if_ge VAR_QZ_0x512D, 1, Kanto_SilphCo_5F_EventScript_841154D
	msgbox Kanto_SilphCo_5F_Text_8426251, MSGBOX_DEFAULT
	release
	end

Kanto_SilphCo_5F_EventScript_841154D::
	msgbox Kanto_SilphCo_5F_Text_8426294, MSGBOX_DEFAULT
	release
	end

@ Kanto_SilphCo_5F sign / bg event at 25,15
Kanto_SilphCo_5F_EventScript_8411557::
	msgbox Kanto_SilphCo_5F_Text_842646A, MSGBOX_SIGN
	end

@ Kanto_SilphCo_5F sign / bg event at 30,13
Kanto_SilphCo_5F_EventScript_8411560::
	msgbox Kanto_SilphCo_5F_Text_84264C1, MSGBOX_SIGN
	end

@ Kanto_SilphCo_5F sign / bg event at 28,11
Kanto_SilphCo_5F_EventScript_8411569::
	msgbox Kanto_SilphCo_5F_Text_8426509, MSGBOX_SIGN
	end

@ Kanto_SilphCo_5F sign / bg event at 29,1
Kanto_SilphCo_5F_EventScript_8411572::
	msgbox Kanto_SilphCo_5F_Text_8426561, MSGBOX_SIGN
	end

@ Kanto_SilphCo_5F object 4 (local id 4, gfx 0x3F43) at 11,6
Kanto_SilphCo_5F_EventScript_841157B::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8532, 0, Kanto_SilphCo_5F_Text_8426321, Kanto_SilphCo_5F_Text_842634E
	msgbox Kanto_SilphCo_5F_Text_842635E, MSGBOX_AUTOCLOSE
	end

@ Kanto_SilphCo_5F object 5 (local id 5, gfx 0x3F3D) at 9,21
Kanto_SilphCo_5F_EventScript_8411592::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8570, 0, Kanto_SilphCo_5F_Text_84262C6, Kanto_SilphCo_5F_Text_84262EA
	msgbox Kanto_SilphCo_5F_Text_84262F0, MSGBOX_AUTOCLOSE
	end

@ Kanto_SilphCo_5F object 1 (local id 1, gfx 0x3F3D) at 35,7
Kanto_SilphCo_5F_EventScript_84115A9::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8571, 0, Kanto_SilphCo_5F_Text_8426409, Kanto_SilphCo_5F_Text_842642C
	msgbox Kanto_SilphCo_5F_Text_842643A, MSGBOX_AUTOCLOSE
	end

@ Kanto_SilphCo_5F object 2 (local id 2, gfx 0x3F3B) at 23,13
Kanto_SilphCo_5F_EventScript_84115C0::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8478, 0, Kanto_SilphCo_5F_Text_842639E, Kanto_SilphCo_5F_Text_84263CC
	msgbox Kanto_SilphCo_5F_Text_84263DA, MSGBOX_AUTOCLOSE
	end

Kanto_SilphCo_5F_EventScript_8458732::
	setmetatile 7, 8, 0x3C0, 1
	setmetatile 8, 8, 0x3C1, 1
	setmetatile 7, 9, 0x3C2, 1
	setmetatile 8, 9, 0x3C3, 1
	setmetatile 7, 0xA, 0x3C4, 1
	setmetatile 8, 0xA, 0x3C5, 1
	return

Kanto_SilphCo_5F_EventScript_8458769::
	setmetatile 7, 0x11, 0x3C0, 1
	setmetatile 8, 0x11, 0x3C1, 1
	setmetatile 7, 0x12, 0x3C2, 1
	setmetatile 8, 0x12, 0x3C3, 1
	setmetatile 7, 0x13, 0x3C4, 1
	setmetatile 8, 0x13, 0x3C5, 1
	return

Kanto_SilphCo_5F_EventScript_84587A0::
	setmetatile 0x12, 0xC, 0x3C0, 1
	setmetatile 0x13, 0xC, 0x3C1, 1
	setmetatile 0x12, 0xD, 0x3C2, 1
	setmetatile 0x13, 0xD, 0x3C3, 1
	setmetatile 0x12, 0xE, 0x3C4, 1
	setmetatile 0x13, 0xE, 0x3C5, 1
	return

@ Kanto_SilphCo_5F sign / bg event at 8,9
@ Kanto_SilphCo_5F sign / bg event at 8,10
@ Kanto_SilphCo_5F sign / bg event at 7,9
@ Kanto_SilphCo_5F sign / bg event at 7,10
Kanto_SilphCo_5F_EventScript_8458E7A::
	lockall
	setvar VAR_TEMP_1, 7
	setvar VAR_0x8004, 0x21FC
	goto_if_set FLAG_QZ_0x21FC, Quetzal_EventScript_8458602
	goto Quetzal_EventScript_8458FE6

@ Kanto_SilphCo_5F sign / bg event at 8,18
@ Kanto_SilphCo_5F sign / bg event at 8,19
@ Kanto_SilphCo_5F sign / bg event at 7,19
@ Kanto_SilphCo_5F sign / bg event at 7,18
Kanto_SilphCo_5F_EventScript_8458E94::
	lockall
	setvar VAR_TEMP_1, 8
	setvar VAR_0x8004, 0x21FD
	goto_if_set FLAG_QZ_0x21FD, Quetzal_EventScript_8458602
	goto Quetzal_EventScript_8458FE6

@ Kanto_SilphCo_5F sign / bg event at 19,13
@ Kanto_SilphCo_5F sign / bg event at 19,14
@ Kanto_SilphCo_5F sign / bg event at 18,13
@ Kanto_SilphCo_5F sign / bg event at 18,14
Kanto_SilphCo_5F_EventScript_8458EAE::
	lockall
	setvar VAR_TEMP_1, 9
	setvar VAR_0x8004, 0x21FE
	goto_if_set FLAG_QZ_0x21FE, Quetzal_EventScript_8458602
	goto Quetzal_EventScript_8458FE6

@ Kanto_SilphCo_5F object 9 (local id 9, gfx 0x3F43) at 2,7
Kanto_SilphCo_5F_EventScript_845D4A5::
	lock
	faceplayer
	setvar VAR_0x8004, 0xF
	setvar VAR_0x8005, 1
	special QzQuestLogNop
	msgbox Kanto_SilphCo_5F_Text_842657A, MSGBOX_DEFAULT
	release
	end

@ Kanto_SilphCo_5F object 6 (local id 6, gfx 0x3F0E) at 4,9
Kanto_SilphCo_5F_EventScript_846BD60::
	setorcopyvar VAR_0x8000, ITEM_PROTEIN
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_SilphCo_5F object 7 (local id 7, gfx 0x3F0E) at 1,18
Kanto_SilphCo_5F_EventScript_846BD6D::
	setorcopyvar VAR_0x8000, ITEM_BLACK_BELT
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_SilphCo_5F object 8 (local id 8, gfx 0x3F0E) at 22,21
Kanto_SilphCo_5F_EventScript_846BD7A::
	checkitem ITEM_CARD_KEY, 1
	goto_if_eq VAR_RESULT, 1, Quetzal_EventScript_835FD2E
	setorcopyvar VAR_0x8000, ITEM_CARD_KEY
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_SilphCo_5F_Text_8426251::
	.string "TEAM ROCKET is in an uproar over\n"
	.string "some intruder.\p"
	.string "That’s you, right?$"

Kanto_SilphCo_5F_Text_8426294::
	.string "TEAM ROCKET took off!\n"
	.string "You’re our hero! Thank you!$"

Kanto_SilphCo_5F_Text_84262C6::
	.string "I heard a kid was wandering\n"
	.string "around.$"

Kanto_SilphCo_5F_Text_84262EA::
	.string "Boom!$"

Kanto_SilphCo_5F_Text_84262F0::
	.string "It’s not smart to pick a fight with\n"
	.string "TEAM ROCKET!$"

Kanto_SilphCo_5F_Text_8426321::
	.string "We study POKé BALL technology on\n"
	.string "this floor.$"

Kanto_SilphCo_5F_Text_842634E::
	.string "Dang!\n"
	.string "Blast it!$"

Kanto_SilphCo_5F_Text_842635E::
	.string "We worked on the ultimate POKé\n"
	.string "BALL which would catch anything.$"

Kanto_SilphCo_5F_Text_842639E::
	.string "Whaaat?\p"
	.string "There shouldn’t be any children\n"
	.string "here!$"

Kanto_SilphCo_5F_Text_84263CC::
	.string "Oh, goodness!$"

Kanto_SilphCo_5F_Text_84263DA::
	.string "You’re only on 5F.\n"
	.string "It’s a long way to my BOSS!$"

Kanto_SilphCo_5F_Text_8426409::
	.string "Show TEAM ROCKET a little respect!$"

Kanto_SilphCo_5F_Text_842642C::
	.string "Cough…\n"
	.string "Cough…$"

Kanto_SilphCo_5F_Text_842643A::
	.string "Which reminds me.\p"
	.string "KOFFING evolves into WEEZING!$"

Kanto_SilphCo_5F_Text_842646A::
	.string "It’s a POKéMON REPORT!\p"
	.string "POKéMON LAB created PORYGON,\n"
	.string "the first virtual-reality POKéMON.$"

Kanto_SilphCo_5F_Text_84264C1::
	.string "It’s a POKéMON REPORT!\p"
	.string "Over 350 POKéMON techniques have\n"
	.string "been confirmed.$"

Kanto_SilphCo_5F_Text_8426509::
	.string "It’s a POKéMON REPORT!\p"
	.string "Some POKéMON have been confirmed\n"
	.string "to evolve when they are traded.$"

Kanto_SilphCo_5F_Text_8426561::
	.string "SILPH CO. HEAD OFFICE\n"
	.string "5F$"

Kanto_SilphCo_5F_Text_842657A::
	.string "Those thugs that took over our\n"
	.string "building…\p"
	.string "Their BOSS said he was looking for\n"
	.string "strong POKéMON.\p"
	.string "I hope our PRESIDENT managed to\n"
	.string "avoid trouble…$"
