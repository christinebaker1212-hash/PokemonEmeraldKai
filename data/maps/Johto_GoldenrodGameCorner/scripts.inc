Johto_GoldenrodGameCorner_MapScripts::
	.byte 0

Johto_GoldenrodGameCorner_EventScript_82F5638::
	msgbox Johto_GoldenrodGameCorner_Text_82F6095, MSGBOX_DEFAULT
	release
	end

@ Johto_GoldenrodGameCorner sign / bg event at 5,6
@ Johto_GoldenrodGameCorner sign / bg event at 12,9
Johto_GoldenrodGameCorner_EventScript_82F5F0D::
	lockall
	checkitem ITEM_COIN_CASE, 1
	goto_if_eq VAR_RESULT, 0, Johto_GoldenrodGameCorner_EventScript_82F606D
	setvar VAR_0x8004, 1
	specialvar VAR_RESULT, GetSlotMachineId
	playslotmachine VAR_RESULT
	releaseall
	end

@ Johto_GoldenrodGameCorner sign / bg event at 5,7
@ Johto_GoldenrodGameCorner sign / bg event at 12,10
Johto_GoldenrodGameCorner_EventScript_82F5F2D::
	lockall
	checkitem ITEM_COIN_CASE, 1
	goto_if_eq VAR_RESULT, 0, Johto_GoldenrodGameCorner_EventScript_82F606D
	setvar VAR_0x8004, 2
	specialvar VAR_RESULT, GetSlotMachineId
	playslotmachine VAR_RESULT
	releaseall
	end

@ Johto_GoldenrodGameCorner sign / bg event at 5,8
@ Johto_GoldenrodGameCorner sign / bg event at 13,10
Johto_GoldenrodGameCorner_EventScript_82F5F4D::
	lockall
	checkitem ITEM_COIN_CASE, 1
	goto_if_eq VAR_RESULT, 0, Johto_GoldenrodGameCorner_EventScript_82F606D
	setvar VAR_0x8004, 3
	specialvar VAR_RESULT, GetSlotMachineId
	playslotmachine VAR_RESULT
	releaseall
	end

@ Johto_GoldenrodGameCorner sign / bg event at 5,10
@ Johto_GoldenrodGameCorner sign / bg event at 13,9
Johto_GoldenrodGameCorner_EventScript_82F5F6D::
	lockall
	checkitem ITEM_COIN_CASE, 1
	goto_if_eq VAR_RESULT, 0, Johto_GoldenrodGameCorner_EventScript_82F606D
	setvar VAR_0x8004, 4
	specialvar VAR_RESULT, GetSlotMachineId
	playslotmachine VAR_RESULT
	releaseall
	end

@ Johto_GoldenrodGameCorner sign / bg event at 13,7
@ Johto_GoldenrodGameCorner sign / bg event at 6,10
Johto_GoldenrodGameCorner_EventScript_82F5F8D::
	lockall
	checkitem ITEM_COIN_CASE, 1
	goto_if_eq VAR_RESULT, 0, Johto_GoldenrodGameCorner_EventScript_82F606D
	setvar VAR_0x8004, 5
	specialvar VAR_RESULT, GetSlotMachineId
	playslotmachine VAR_RESULT
	releaseall
	end

@ Johto_GoldenrodGameCorner sign / bg event at 13,6
@ Johto_GoldenrodGameCorner sign / bg event at 6,9
Johto_GoldenrodGameCorner_EventScript_82F5FAD::
	lockall
	checkitem ITEM_COIN_CASE, 1
	goto_if_eq VAR_RESULT, 0, Johto_GoldenrodGameCorner_EventScript_82F606D
	setvar VAR_0x8004, 6
	specialvar VAR_RESULT, GetSlotMachineId
	playslotmachine VAR_RESULT
	releaseall
	end

@ Johto_GoldenrodGameCorner sign / bg event at 6,8
Johto_GoldenrodGameCorner_EventScript_82F5FCD::
	lockall
	checkitem ITEM_COIN_CASE, 1
	goto_if_eq VAR_RESULT, 0, Johto_GoldenrodGameCorner_EventScript_82F606D
	setvar VAR_0x8004, 7
	specialvar VAR_RESULT, GetSlotMachineId
	playslotmachine VAR_RESULT
	releaseall
	end

@ Johto_GoldenrodGameCorner sign / bg event at 6,7
Johto_GoldenrodGameCorner_EventScript_82F5FED::
	lockall
	checkitem ITEM_COIN_CASE, 1
	goto_if_eq VAR_RESULT, 0, Johto_GoldenrodGameCorner_EventScript_82F606D
	setvar VAR_0x8004, 8
	specialvar VAR_RESULT, GetSlotMachineId
	playslotmachine VAR_RESULT
	releaseall
	end

@ Johto_GoldenrodGameCorner sign / bg event at 6,6
Johto_GoldenrodGameCorner_EventScript_82F600D::
	lockall
	checkitem ITEM_COIN_CASE, 1
	goto_if_eq VAR_RESULT, 0, Johto_GoldenrodGameCorner_EventScript_82F606D
	setvar VAR_0x8004, 9
	specialvar VAR_RESULT, GetSlotMachineId
	playslotmachine VAR_RESULT
	releaseall
	end

@ Johto_GoldenrodGameCorner sign / bg event at 12,6
Johto_GoldenrodGameCorner_EventScript_82F602D::
	lockall
	checkitem ITEM_COIN_CASE, 1
	goto_if_eq VAR_RESULT, 0, Johto_GoldenrodGameCorner_EventScript_82F606D
	setvar VAR_0x8004, 0xA
	specialvar VAR_RESULT, GetSlotMachineId
	playslotmachine VAR_RESULT
	releaseall
	end

@ Johto_GoldenrodGameCorner sign / bg event at 6,6
@ Johto_GoldenrodGameCorner sign / bg event at 12,7
Johto_GoldenrodGameCorner_EventScript_82F604D::
	lockall
	checkitem ITEM_COIN_CASE, 1
	goto_if_eq VAR_RESULT, 0, Johto_GoldenrodGameCorner_EventScript_82F606D
	setvar VAR_0x8004, 0xB
	specialvar VAR_RESULT, GetSlotMachineId
	playslotmachine VAR_RESULT
	releaseall
	end

Johto_GoldenrodGameCorner_EventScript_82F606D::
	msgbox Johto_GoldenrodGameCorner_Text_82F6845, MSGBOX_DEFAULT
	releaseall
	end

Johto_GoldenrodGameCorner_EventScript_8361B1E::
	fadescreen 1
	special ChangeBoxPokemonNickname
	waitstate
	lock
	faceplayer
	return

@ Johto_GoldenrodGameCorner sign / bg event at 19,7
@ Johto_GoldenrodGameCorner sign / bg event at 18,7
@ Johto_GoldenrodGameCorner sign / bg event at 18,8
@ Johto_GoldenrodGameCorner sign / bg event at 19,8
Johto_GoldenrodGameCorner_EventScript_83944F9::
	checkitem ITEM_COIN_CASE, 1
	goto_if_eq VAR_RESULT, 0, Johto_GoldenrodGameCorner_EventScript_82F606D
	setvar VAR_0x8004, 1
	getpokenewsactive 2
	goto_if_eq VAR_RESULT, 0, Johto_GoldenrodGameCorner_EventScript_8394527
	addvar VAR_0x8004, 0x80
	goto Johto_GoldenrodGameCorner_EventScript_8394527

Johto_GoldenrodGameCorner_EventScript_8394527::
	special PlayRoulette
	waitstate
	end

@ Johto_GoldenrodGameCorner object 1 (local id 1, gfx 0x3EA7) at 3,2
Johto_GoldenrodGameCorner_EventScript_83D5162::
	lock
	faceplayer
	msgbox Johto_GoldenrodGameCorner_Text_83D542A, MSGBOX_DEFAULT
	checkitem ITEM_COIN_CASE, 1
	goto_if_eq VAR_RESULT, 0, Johto_GoldenrodGameCorner_EventScript_82F5638
	goto Quetzal_EventScript_841C97A

@ Johto_GoldenrodGameCorner object 2 (local id 2, gfx 0x3E82) at 16,2
Johto_GoldenrodGameCorner_EventScript_83D5181::
	lock
	faceplayer
	msgbox Johto_GoldenrodGameCorner_Text_83D5449, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodGameCorner object 3 (local id 3, gfx 0x3E83) at 18,2
Johto_GoldenrodGameCorner_EventScript_83D518E::
	lock
	faceplayer
	call Quetzal_EventScript_8361A4D
	checkitem ITEM_COIN_CASE, 1
	goto_if_eq VAR_RESULT, 0, Johto_GoldenrodGameCorner_EventScript_83D530A
	msgbox Johto_GoldenrodGameCorner_Text_83D54CF, MSGBOX_DEFAULT
	call Quetzal_EventScript_841CE7A
	multichoice 0xC, 0, 0x4C, 1
	nop
	copyvar VAR_0x8000, VAR_RESULT
	goto_if_eq VAR_0x8000, 0, Johto_GoldenrodGameCorner_EventScript_83D5204
	goto_if_eq VAR_0x8000, 1, Johto_GoldenrodGameCorner_EventScript_83D5213
	goto_if_eq VAR_0x8000, 2, Johto_GoldenrodGameCorner_EventScript_83D5222
	goto_if_eq VAR_0x8000, 3, Johto_GoldenrodGameCorner_EventScript_83D5231
	goto_if_eq VAR_0x8000, 4, Johto_GoldenrodGameCorner_EventScript_83D5240
	goto_if_eq VAR_0x8000, 5, Johto_GoldenrodGameCorner_EventScript_83D524F
	goto Johto_GoldenrodGameCorner_EventScript_83D5297

Johto_GoldenrodGameCorner_EventScript_83D5204::
	setvar VAR_TEMP_1, 0x3BA
	setvar VAR_TEMP_2, 0x9C4
	goto Johto_GoldenrodGameCorner_EventScript_83D525E

Johto_GoldenrodGameCorner_EventScript_83D5213::
	setvar VAR_TEMP_1, 0x15D
	setvar VAR_TEMP_2, 0x1388
	goto Johto_GoldenrodGameCorner_EventScript_83D525E

Johto_GoldenrodGameCorner_EventScript_83D5222::
	setvar VAR_TEMP_1, 0x93
	setvar VAR_TEMP_2, 0x157C
	goto Johto_GoldenrodGameCorner_EventScript_83D525E

Johto_GoldenrodGameCorner_EventScript_83D5231::
	setvar VAR_TEMP_1, 0x84
	setvar VAR_TEMP_2, 0x1B39
	goto Johto_GoldenrodGameCorner_EventScript_83D525E

Johto_GoldenrodGameCorner_EventScript_83D5240::
	setvar VAR_TEMP_1, 0x375
	setvar VAR_TEMP_2, 0x1F40
	goto Johto_GoldenrodGameCorner_EventScript_83D525E

Johto_GoldenrodGameCorner_EventScript_83D524F::
	setvar VAR_TEMP_1, 0x30A
	setvar VAR_TEMP_2, 0x2328
	goto Johto_GoldenrodGameCorner_EventScript_83D525E

Johto_GoldenrodGameCorner_EventScript_83D525E::
	bufferspeciesname 0, VAR_TEMP_1
	msgbox Johto_GoldenrodGameCorner_Text_83D54A2, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Johto_GoldenrodGameCorner_EventScript_83D5297
	checkcoins VAR_RESULT
	compare_var_to_var VAR_RESULT, VAR_TEMP_2
	goto_if_lt Johto_GoldenrodGameCorner_EventScript_83D5314
	qz_givemon_ex VAR_TEMP_1, 0x14, ITEM_NONE, 2, 0, 0
	goto Johto_GoldenrodGameCorner_EventScript_83D529A

Johto_GoldenrodGameCorner_EventScript_83D5297::
	closemessage
	release
	end

Johto_GoldenrodGameCorner_EventScript_83D529A::
	bufferspeciesname 0, VAR_TEMP_1
	goto_if_eq VAR_RESULT, 0, Johto_GoldenrodGameCorner_EventScript_83D5322
	goto_if_eq VAR_RESULT, 1, Johto_GoldenrodGameCorner_EventScript_83D535F
	goto_if_eq VAR_RESULT, 2, Johto_GoldenrodGameCorner_EventScript_83D52C4
	goto Johto_GoldenrodGameCorner_EventScript_83D52C4

Johto_GoldenrodGameCorner_EventScript_83D52C4::
	msgbox Johto_GoldenrodGameCorner_Text_835E3D6, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_GoldenrodGameCorner_EventScript_83D52CF::
	getpartysize
	subvar VAR_RESULT, 1
	copyvar VAR_0x8004, VAR_RESULT
	call Quetzal_EventScript_8361B12
	call Quetzal_EventScript_836063E
	goto Johto_GoldenrodGameCorner_EventScript_83D5297

Johto_GoldenrodGameCorner_EventScript_83D52EA::
	message Johto_GoldenrodGameCorner_Text_83D54C2
	getpartysize
	subvar VAR_RESULT, 1
	copyvar VAR_0x8004, VAR_RESULT
	call Quetzal_EventScript_8361B12
	call Quetzal_EventScript_836063E
	goto Johto_GoldenrodGameCorner_EventScript_83D5297

Johto_GoldenrodGameCorner_EventScript_83D530A::
	msgbox Johto_GoldenrodGameCorner_Text_83D559A, MSGBOX_DEFAULT
	release
	end

Johto_GoldenrodGameCorner_EventScript_83D5314::
	msgbox Johto_GoldenrodGameCorner_Text_83D5532, MSGBOX_DEFAULT
	goto Johto_GoldenrodGameCorner_EventScript_83D5297

Johto_GoldenrodGameCorner_EventScript_83D5322::
	removecoins VAR_TEMP_2
	playse SE_SHOP
	bufferspeciesname 0, VAR_TEMP_1
	playfanfare MUS_LEVEL_UP
	message Johto_GoldenrodGameCorner_Text_83D54B7
	waitmessage
	waitfanfare
	specialvar VAR_RESULT, QzIsNuzlockeOption2D4Bit6
	goto_if_eq VAR_RESULT, 1, Johto_GoldenrodGameCorner_EventScript_83D52EA
	msgbox Johto_GoldenrodGameCorner_Text_83D54C2, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Johto_GoldenrodGameCorner_EventScript_83D52CF
	goto Johto_GoldenrodGameCorner_EventScript_83D5297

Johto_GoldenrodGameCorner_EventScript_83D535F::
	removecoins VAR_TEMP_2
	playse SE_SHOP
	bufferspeciesname 0, VAR_TEMP_1
	playfanfare MUS_LEVEL_UP
	message Johto_GoldenrodGameCorner_Text_83D54B7
	waitmessage
	waitfanfare
	goto_if_eq VAR_TEMP_1, 147, Johto_GoldenrodGameCorner_EventScript_83D5384
	goto Johto_GoldenrodGameCorner_EventScript_83D539A

Johto_GoldenrodGameCorner_EventScript_83D5384::
	call Quetzal_EventScript_8361B27
	goto Johto_GoldenrodGameCorner_EventScript_83D5297

Johto_GoldenrodGameCorner_EventScript_83D539A::
	msgbox Johto_GoldenrodGameCorner_Text_8361655, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Johto_GoldenrodGameCorner_EventScript_83D53BA
	call Johto_GoldenrodGameCorner_EventScript_8361B1E
	goto Johto_GoldenrodGameCorner_EventScript_83D53BA

Johto_GoldenrodGameCorner_EventScript_83D53BA::
	call Quetzal_EventScript_8361B27
	closemessage
	release
	end

@ Johto_GoldenrodGameCorner object 4 (local id 4, gfx 0x11) at 2,6
Johto_GoldenrodGameCorner_EventScript_83D53C2::
	lock
	faceplayer
	msgbox Johto_GoldenrodGameCorner_Text_83D5618, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodGameCorner object 11 (local id 11, gfx 0x37) at 19,9
Johto_GoldenrodGameCorner_EventScript_83D53CF::
	lock
	faceplayer
	msgbox Johto_GoldenrodGameCorner_Text_83D57FA, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodGameCorner object 10 (local id 10, gfx 0x18) at 18,6
Johto_GoldenrodGameCorner_EventScript_83D53DC::
	lock
	faceplayer
	msgbox Johto_GoldenrodGameCorner_Text_83D5602, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodGameCorner object 7 (local id 7, gfx 0x30) at 4,9
Johto_GoldenrodGameCorner_EventScript_83D53E9::
	lock
	faceplayer
	msgbox Johto_GoldenrodGameCorner_Text_83D5724, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodGameCorner object 5 (local id 5, gfx 0x2F) at 9,4
Johto_GoldenrodGameCorner_EventScript_83D53F6::
	lock
	faceplayer
	msgbox Johto_GoldenrodGameCorner_Text_83D569F, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodGameCorner object 6 (local id 6, gfx 0x3E96) at 11,2
Johto_GoldenrodGameCorner_EventScript_83D5403::
	lock
	faceplayer
	msgbox Johto_GoldenrodGameCorner_Text_83D5782, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodGameCorner object 9 (local id 9, gfx 0x3EAE) at 14,8
Johto_GoldenrodGameCorner_EventScript_83D5410::
	lock
	faceplayer
	msgbox Johto_GoldenrodGameCorner_Text_83D566E, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodGameCorner object 8 (local id 8, gfx 0x3E84) at 11,8
Johto_GoldenrodGameCorner_EventScript_83D541D::
	lock
	faceplayer
	msgbox Johto_GoldenrodGameCorner_Text_83D55BA, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_GoldenrodGameCorner_Text_82F6095::
	.string "Okay, you wanted some COINS for\n"
	.string "the games?\p"
	.string "But you don’t have a COIN CASE for\n"
	.string "stowing the COINS.$"

Johto_GoldenrodGameCorner_Text_82F6845::
	.string "You can’t play if you don’t have\n"
	.string "a COIN CASE.$"

Johto_GoldenrodGameCorner_Text_835E3D6::
	.string "Hm? You don’t seem to have any room\n"
	.string "for this POKéMON.$"

Johto_GoldenrodGameCorner_Text_8361655::
	.string "Do you want to give a nickname to\n"
	.string "this {STR_VAR_1}?$"

Johto_GoldenrodGameCorner_Text_83D542A::
	.string "This is GOLDENROD GAME CORNER.$"

Johto_GoldenrodGameCorner_Text_83D5449::
	.string "All POKéMON prizes are at the\n"
	.string "counter to the right.\p"
	.string "My colleague will show you the list!$"

Johto_GoldenrodGameCorner_Text_83D54A2::
	.string "{STR_VAR_1} is what you want?$"

Johto_GoldenrodGameCorner_Text_83D54B7::
	.string "{PLAYER} got {STR_VAR_1}!$"

Johto_GoldenrodGameCorner_Text_83D54C2::
	.string "Nickname {STR_VAR_1}?$"

Johto_GoldenrodGameCorner_Text_83D54CF::
	.string "Welcome!\p"
	.string "We exchange your game coins for\n"
	.string "fabulous prizes!$"

Johto_GoldenrodGameCorner_Text_83D5532::
	.string "Sorry! You need more coins.$"

Johto_GoldenrodGameCorner_Text_83D559A::
	.string "Oh? You don’t have a COIN CASE.$"

Johto_GoldenrodGameCorner_Text_83D55BA::
	.string "I always play this slot machine.\n"
	.string "It pays out more than others,\l"
	.string "I think.$"

Johto_GoldenrodGameCorner_Text_83D5602::
	.string "I can’t stop winning!$"

Johto_GoldenrodGameCorner_Text_83D5618::
	.string "I just love this new slot machine.\p"
	.string "It’s more of a challenge than the\n"
	.string "ones in CELADON.$"

Johto_GoldenrodGameCorner_Text_83D566E::
	.string "Life is a gamble. I’m going to pull\n"
	.string "till I drop!$"

Johto_GoldenrodGameCorner_Text_83D569F::
	.string "Roulette…\p"
	.string "I prefer it over the slots because\n"
	.string "it’s easier to figure the odds.\p"
	.string "But the payout is much lower.$"

Johto_GoldenrodGameCorner_Text_83D5724::
	.string "I taught ICE BEAM to my POKéMON.\p"
	.string "It was hard to get enough coins for\n"
	.string "it, but it was worth it.$"

Johto_GoldenrodGameCorner_Text_83D5782::
	.string "I couldn’t win at the slots, and I\n"
	.string "blew it on roulette.\p"
	.string "I got so furious, I tossed out my\n"
	.string "COIN CASE in the UNDERGROUND.$"

Johto_GoldenrodGameCorner_Text_83D57FA::
	.string "Wahahah! The coins keep rolling in!$"
