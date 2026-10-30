Johto_Route39Barn_MapScripts::
	.byte 0

@ Johto_Route39Barn object 1 (local id 1, gfx 0x1F1) at 13,5
Johto_Route39Barn_EventScript_83F3580::
	lock
	faceplayer
	waitse
	playmoncry SPECIES_MILTANK, 0
	goto_if_eq VAR_QZ_0x501B, 0, Johto_Route39Barn_EventScript_83F35CA
	goto_if_eq VAR_QZ_0x501B, 1, Johto_Route39Barn_EventScript_83F361B
	goto_if_eq VAR_QZ_0x501B, 2, Johto_Route39Barn_EventScript_83F366C
	goto_if_eq VAR_QZ_0x501B, 3, Johto_Route39Barn_EventScript_83F36BD
	goto_if_eq VAR_QZ_0x501B, 4, Johto_Route39Barn_EventScript_83F370E
	msgbox Johto_Route39Barn_Text_83F382B, MSGBOX_DEFAULT
	waitmoncry
	release
	end

Johto_Route39Barn_EventScript_83F35CA::
	msgbox Johto_Route39Barn_Text_83F37EC, MSGBOX_DEFAULT
	waitmoncry
	closemessage
	delay 0xA
	msgbox Johto_Route39Barn_Text_83F37FB, MSGBOX_YESNO
	call_if_eq VAR_RESULT, 0, Johto_Route39Barn_EventScript_83F3763
	checkitem ITEM_ORAN_BERRY, 1
	goto_if_eq VAR_RESULT, 0, Johto_Route39Barn_EventScript_83F376E
	removeitem ITEM_ORAN_BERRY, 1
	playse SE_USE_ITEM
	msgbox Johto_Route39Barn_Text_83F383A, MSGBOX_AUTOCLOSE
	waitse
	msgbox Johto_Route39Barn_Text_83F385C, MSGBOX_DEFAULT
	closemessage
	setvar VAR_QZ_0x501B, 1
	release
	end

Johto_Route39Barn_EventScript_83F361B::
	msgbox Johto_Route39Barn_Text_83F37EC, MSGBOX_DEFAULT
	waitmoncry
	closemessage
	delay 0xA
	msgbox Johto_Route39Barn_Text_83F37FB, MSGBOX_YESNO
	call_if_eq VAR_RESULT, 0, Johto_Route39Barn_EventScript_83F3763
	checkitem ITEM_ORAN_BERRY, 1
	goto_if_eq VAR_RESULT, 0, Johto_Route39Barn_EventScript_83F376E
	removeitem ITEM_ORAN_BERRY, 1
	playse SE_USE_ITEM
	msgbox Johto_Route39Barn_Text_83F383A, MSGBOX_AUTOCLOSE
	waitse
	msgbox Johto_Route39Barn_Text_83F385C, MSGBOX_DEFAULT
	closemessage
	setvar VAR_QZ_0x501B, 2
	release
	end

Johto_Route39Barn_EventScript_83F366C::
	msgbox Johto_Route39Barn_Text_83F37EC, MSGBOX_DEFAULT
	waitmoncry
	closemessage
	delay 0xA
	msgbox Johto_Route39Barn_Text_83F37FB, MSGBOX_YESNO
	call_if_eq VAR_RESULT, 0, Johto_Route39Barn_EventScript_83F3763
	checkitem ITEM_ORAN_BERRY, 1
	goto_if_eq VAR_RESULT, 0, Johto_Route39Barn_EventScript_83F376E
	removeitem ITEM_ORAN_BERRY, 1
	playse SE_USE_ITEM
	msgbox Johto_Route39Barn_Text_83F383A, MSGBOX_AUTOCLOSE
	waitse
	msgbox Johto_Route39Barn_Text_83F387F, MSGBOX_DEFAULT
	closemessage
	setvar VAR_QZ_0x501B, 3
	release
	end

Johto_Route39Barn_EventScript_83F36BD::
	msgbox Johto_Route39Barn_Text_83F37EC, MSGBOX_DEFAULT
	waitmoncry
	closemessage
	delay 0xA
	msgbox Johto_Route39Barn_Text_83F37FB, MSGBOX_YESNO
	call_if_eq VAR_RESULT, 0, Johto_Route39Barn_EventScript_83F3763
	checkitem ITEM_ORAN_BERRY, 1
	goto_if_eq VAR_RESULT, 0, Johto_Route39Barn_EventScript_83F376E
	removeitem ITEM_ORAN_BERRY, 1
	playse SE_USE_ITEM
	msgbox Johto_Route39Barn_Text_83F383A, MSGBOX_AUTOCLOSE
	waitse
	msgbox Johto_Route39Barn_Text_83F387F, MSGBOX_DEFAULT
	closemessage
	setvar VAR_QZ_0x501B, 4
	release
	end

Johto_Route39Barn_EventScript_83F370E::
	msgbox Johto_Route39Barn_Text_83F37EC, MSGBOX_DEFAULT
	waitmoncry
	closemessage
	delay 0xA
	msgbox Johto_Route39Barn_Text_83F37FB, MSGBOX_YESNO
	call_if_eq VAR_RESULT, 0, Johto_Route39Barn_EventScript_83F3763
	checkitem ITEM_ORAN_BERRY, 1
	goto_if_eq VAR_RESULT, 0, Johto_Route39Barn_EventScript_83F376E
	removeitem ITEM_ORAN_BERRY, 1
	playse SE_USE_ITEM
	msgbox Johto_Route39Barn_Text_83F383A, MSGBOX_AUTOCLOSE
	waitse
	msgbox Johto_Route39Barn_Text_83F389D, MSGBOX_DEFAULT
	playfanfare MUS_OBTAIN_BADGE
	waitfanfare
	closemessage
	setvar VAR_QZ_0x501B, 5
	release
	end

Johto_Route39Barn_EventScript_83F3763::
	msgbox Johto_Route39Barn_Text_83F38D5, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_Route39Barn_EventScript_83F376E::
	msgbox Johto_Route39Barn_Text_83F38BD, MSGBOX_DEFAULT
	release
	end

@ Johto_Route39Barn object 2 (local id 2, gfx 0x6) at 12,5
@ Johto_Route39Barn object 3 (local id 3, gfx 0x6) at 14,5
Johto_Route39Barn_EventScript_83F3778::
	lock
	faceplayer
	goto_if_eq VAR_QZ_0x501B, 5, Johto_Route39Barn_EventScript_83F3790
	msgbox Johto_Route39Barn_Text_83F379B, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_Route39Barn_EventScript_83F3790::
	msgbox Johto_Route39Barn_Text_83F37CB, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_Route39Barn_Text_83F379B::
	.string "MOOMOO is sick…\p"
	.string "She needs lots of ORAN BERRIES.$"

Johto_Route39Barn_Text_83F37CB::
	.string "MOOMOO is all better!\n"
	.string "Thank you!$"

Johto_Route39Barn_Text_83F37EC::
	.string "MILTANK: …Moo…$"

Johto_Route39Barn_Text_83F37FB::
	.string "Its cry is weak…\n"
	.string "Give an ORAN BERRY to MILTANK?$"

Johto_Route39Barn_Text_83F382B::
	.string "MILTANK: Mooo!$"

Johto_Route39Barn_Text_83F383A::
	.string "{PLAYER} gave an ORAN BERRY to\n"
	.string "MILTANK.$"

Johto_Route39Barn_Text_83F385C::
	.string "MILTANK became a little\n"
	.string "healthier!$"

Johto_Route39Barn_Text_83F387F::
	.string "MILTANK became quite\n"
	.string "healthy!$"

Johto_Route39Barn_Text_83F389D::
	.string "MILTANK became totally\n"
	.string "healthy!$"

Johto_Route39Barn_Text_83F38BD::
	.string "{PLAYER} has no ORAN BERRIES…$"

Johto_Route39Barn_Text_83F38D5::
	.string "{PLAYER} wouldn’t give an\n"
	.string "ORAN BERRY.\p"
	.string "MILTANK looks sad.$"
