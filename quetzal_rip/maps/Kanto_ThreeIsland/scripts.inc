Kanto_ThreeIsland_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Quetzal_EventScript_83C3036
	map_script MAP_SCRIPT_ON_RESUME, Quetzal_EventScript_83C3036
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_ThreeIsland_EventScript_8417447
	.byte 0

Kanto_ThreeIsland_EventScript_8417447::
	special QzKantoTravelingEventCheck_773
	setflag FLAG_QZ_0x225B
	call_if_set FLAG_QZ_0x21EC, Kanto_ThreeIsland_EventScript_8417467
	call_if_eq VAR_QZ_0x5131, 4, Kanto_ThreeIsland_EventScript_841746B
	call Quetzal_EventScript_840FBB5
	end

Kanto_ThreeIsland_EventScript_8417467::
	setflag FLAG_QZ_0x21CE
	return

Kanto_ThreeIsland_EventScript_841746B::
	setobjectmovementtype 1, 1
	setobjectmovementtype 2, 1
	return

@ Kanto_ThreeIsland object 3 (local id 3, gfx 0x3EE9) at 8,24
@ Kanto_ThreeIsland object 4 (local id 4, gfx 0x3EE9) at 9,23
@ Kanto_ThreeIsland object 5 (local id 5, gfx 0x3EE9) at 9,24
@ Kanto_ThreeIsland object 6 (local id 6, gfx 0x3EE9) at 10,22
@ Kanto_ThreeIsland object 7 (local id 7, gfx 0x3EE9) at 10,24
@ Kanto_ThreeIsland object 8 (local id 8, gfx 0x3EE9) at 11,23
Kanto_ThreeIsland_EventScript_8417474::
	lock
	setvar VAR_TEMP_1, 2
	call Kanto_ThreeIsland_EventScript_841750E
	release
	end

@ Kanto_ThreeIsland object 1 (local id 1, gfx 0x3F1E) at 7,24
Kanto_ThreeIsland_EventScript_8417481::
	lock
	goto_if_set FLAG_QZ_0x214A, Kanto_ThreeIsland_EventScript_84174A2
	goto_if_eq VAR_QZ_0x5131, 4, Kanto_ThreeIsland_EventScript_84174B6
	setvar VAR_TEMP_1, 0
	call Kanto_ThreeIsland_EventScript_841750E
	release
	end

Kanto_ThreeIsland_EventScript_84174A2::
	applymovement 1, Quetzal_Movement_8457AA4
	waitmovement 0
	msgbox Kanto_ThreeIsland_Text_8432AB6, MSGBOX_DEFAULT
	release
	end

Kanto_ThreeIsland_EventScript_84174B6::
	applymovement 1, Quetzal_Movement_8457AA4
	waitmovement 0
	msgbox Kanto_ThreeIsland_Text_8432A1F, MSGBOX_DEFAULT
	checkitemspace ITEM_FULL_RESTORE, 1
	goto_if_eq VAR_RESULT, 0, Kanto_ThreeIsland_EventScript_8417504
	loadword 0, Kanto_ThreeIsland_Text_8432A83
	setorcopyvar VAR_0x8000, 0x20
	setorcopyvar VAR_0x8001, 1
	setorcopyvar VAR_0x8002, 0x16F
	call Quetzal_EventScript_845571E
	additem ITEM_FULL_RESTORE, 1
	setflag FLAG_QZ_0x214A
	msgbox Kanto_ThreeIsland_Text_8432AB6, MSGBOX_DEFAULT
	release
	end

Kanto_ThreeIsland_EventScript_8417504::
	msgbox Kanto_ThreeIsland_Text_8432AA0, MSGBOX_DEFAULT
	release
	end

Kanto_ThreeIsland_EventScript_841750E::
	call_if_eq VAR_TEMP_1, 1, Kanto_ThreeIsland_EventScript_84175E6
	call_if_eq VAR_TEMP_1, 2, Kanto_ThreeIsland_EventScript_84175E6
	applymovement 1, Kanto_ThreeIsland_Movement_84176BA
	waitmovement 0
	msgbox Kanto_ThreeIsland_Text_8432385, MSGBOX_DEFAULT
	call_if_eq VAR_TEMP_1, 0, Kanto_ThreeIsland_EventScript_84175F1
	call_if_eq VAR_TEMP_1, 1, Kanto_ThreeIsland_EventScript_84175F1
	call_if_eq VAR_TEMP_1, 2, Kanto_ThreeIsland_EventScript_84175FC
	applymovement 5, Kanto_ThreeIsland_Movement_84176B8
	waitmovement 0
	msgbox Kanto_ThreeIsland_Text_84323EA, MSGBOX_DEFAULT
	call_if_eq VAR_TEMP_1, 0, Kanto_ThreeIsland_EventScript_84175D0
	call_if_eq VAR_TEMP_1, 1, Kanto_ThreeIsland_EventScript_84175D0
	call_if_eq VAR_TEMP_1, 2, Kanto_ThreeIsland_EventScript_8417612
	applymovement 2, Kanto_ThreeIsland_Movement_84176BA
	waitmovement 0
	msgbox Kanto_ThreeIsland_Text_843246E, MSGBOX_DEFAULT
	call_if_eq VAR_TEMP_1, 0, Kanto_ThreeIsland_EventScript_84175F1
	call_if_eq VAR_TEMP_1, 1, Kanto_ThreeIsland_EventScript_84175F1
	call_if_eq VAR_TEMP_1, 2, Kanto_ThreeIsland_EventScript_84175FC
	applymovement 4, Kanto_ThreeIsland_Movement_84176B8
	waitmovement 0
	msgbox Kanto_ThreeIsland_Text_84324A1, MSGBOX_DEFAULT
	return

Kanto_ThreeIsland_EventScript_84175D0::
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AAC
	waitmovement 0
	return

Kanto_ThreeIsland_EventScript_84175E6::
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AAA
	waitmovement 0
	return

Kanto_ThreeIsland_EventScript_84175F1::
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AAE
	waitmovement 0
	return

Kanto_ThreeIsland_EventScript_84175FC::
	getplayerxy VAR_0x8004, VAR_0x8005
	goto_if_ge VAR_0x8004, 9, Kanto_ThreeIsland_EventScript_84175D0
	goto Kanto_ThreeIsland_EventScript_84175F1

Kanto_ThreeIsland_EventScript_8417612::
	getplayerxy VAR_0x8004, VAR_0x8005
	goto_if_ge VAR_0x8004, 9, Kanto_ThreeIsland_EventScript_84175E6
	goto Kanto_ThreeIsland_EventScript_84175D0

@ Kanto_ThreeIsland object 2 (local id 2, gfx 0x3F1E) at 8,23
Kanto_ThreeIsland_EventScript_8417628::
	lock
	goto_if_eq VAR_QZ_0x5131, 4, Kanto_ThreeIsland_EventScript_8417640
	setvar VAR_TEMP_1, 1
	call Kanto_ThreeIsland_EventScript_841750E
	release
	end

Kanto_ThreeIsland_EventScript_8417640::
	applymovement 2, Quetzal_Movement_8457AA4
	waitmovement 0
	msgbox Kanto_ThreeIsland_Text_8432AE8, MSGBOX_DEFAULT
	release
	end

@ Kanto_ThreeIsland trigger at 7,27 (VAR_QZ_0x5131 == 2)
@ Kanto_ThreeIsland trigger at 8,27 (VAR_QZ_0x5131 == 2)
@ Kanto_ThreeIsland trigger at 9,27 (VAR_QZ_0x5131 == 2)
@ Kanto_ThreeIsland trigger at 10,27 (VAR_QZ_0x5131 == 2)
@ Kanto_ThreeIsland trigger at 11,27 (VAR_QZ_0x5131 == 2)
Kanto_ThreeIsland_EventScript_8417654::
	lockall
	textcolor 0
	applymovement 1, Kanto_ThreeIsland_Movement_84176BA
	waitmovement 0
	msgbox Kanto_ThreeIsland_Text_84324B6, MSGBOX_DEFAULT
	applymovement 3, Kanto_ThreeIsland_Movement_84176B8
	waitmovement 0
	msgbox Kanto_ThreeIsland_Text_84324E4, MSGBOX_DEFAULT
	applymovement 1, Kanto_ThreeIsland_Movement_84176BA
	waitmovement 0
	msgbox Kanto_ThreeIsland_Text_843253A, MSGBOX_DEFAULT
	applymovement 3, Kanto_ThreeIsland_Movement_84176B8
	waitmovement 0
	msgbox Kanto_ThreeIsland_Text_84325C0, MSGBOX_DEFAULT
	applymovement 1, Kanto_ThreeIsland_Movement_84176BA
	waitmovement 0
	msgbox Kanto_ThreeIsland_Text_843268C, MSGBOX_DEFAULT
	setvar VAR_QZ_0x5131, 3
	releaseall
	end

@ Kanto_ThreeIsland trigger at 7,26 (VAR_QZ_0x5131 == 3)
Kanto_ThreeIsland_EventScript_84176BC::
	lockall
	setvar VAR_TEMP_1, 0
	goto Kanto_ThreeIsland_EventScript_84176F8

@ Kanto_ThreeIsland trigger at 8,26 (VAR_QZ_0x5131 == 3)
Kanto_ThreeIsland_EventScript_84176C8::
	lockall
	setvar VAR_TEMP_1, 1
	goto Kanto_ThreeIsland_EventScript_84176F8

@ Kanto_ThreeIsland trigger at 9,26 (VAR_QZ_0x5131 == 3)
Kanto_ThreeIsland_EventScript_84176D4::
	lockall
	setvar VAR_TEMP_1, 2
	goto Kanto_ThreeIsland_EventScript_84176F8

@ Kanto_ThreeIsland trigger at 10,26 (VAR_QZ_0x5131 == 3)
Kanto_ThreeIsland_EventScript_84176E0::
	lockall
	setvar VAR_TEMP_1, 3
	goto Kanto_ThreeIsland_EventScript_84176F8

@ Kanto_ThreeIsland trigger at 11,26 (VAR_QZ_0x5131 == 3)
Kanto_ThreeIsland_EventScript_84176EC::
	lockall
	setvar VAR_TEMP_1, 4
	goto Kanto_ThreeIsland_EventScript_84176F8

Kanto_ThreeIsland_EventScript_84176F8::
	textcolor 0
	applymovement 5, Quetzal_Movement_8457AB0
	waitmovement 0
	playse SE_PIN
	applymovement 5, Quetzal_Movement_8457A9E
	waitmovement 0
	applymovement 5, Quetzal_Movement_8457AA0
	waitmovement 0
	applymovement 5, Kanto_ThreeIsland_Movement_84178F6
	waitmovement 0
	playbgm MUS_RG_ENCOUNTER_BOY, 0
	msgbox Kanto_ThreeIsland_Text_84326B2, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Kanto_ThreeIsland_EventScript_84178D4
	msgbox Kanto_ThreeIsland_Text_8432742, MSGBOX_DEFAULT
	setvar VAR_LAST_TALKED, 5
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_8719, 0, Kanto_ThreeIsland_Text_843277C
	applymovement 5, Kanto_ThreeIsland_Movement_84178FB
	waitmovement 0
	msgbox Kanto_ThreeIsland_Text_8432794, MSGBOX_DEFAULT
	closemessage
	applymovement 7, Kanto_ThreeIsland_Movement_84178F6
	waitmovement 0
	playbgm MUS_RG_ENCOUNTER_BOY, 0
	msgbox Kanto_ThreeIsland_Text_84327B4, MSGBOX_DEFAULT
	setvar VAR_LAST_TALKED, 7
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_8720, 0, Kanto_ThreeIsland_Text_84327E6
	applymovement 7, Kanto_ThreeIsland_Movement_84178FB
	waitmovement 0
	msgbox Kanto_ThreeIsland_Text_84327FB, MSGBOX_DEFAULT
	closemessage
	applymovement 4, Kanto_ThreeIsland_Movement_84178F6
	waitmovement 0
	playbgm MUS_RG_ENCOUNTER_BOY, 0
	msgbox Kanto_ThreeIsland_Text_8432847, MSGBOX_DEFAULT
	setvar VAR_LAST_TALKED, 4
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_8721, 0, Kanto_ThreeIsland_Text_8432899
	applymovement 4, Quetzal_Movement_8457AAA
	waitmovement 0
	applymovement 6, Quetzal_Movement_8457AB0
	applymovement 8, Quetzal_Movement_8457AB0
	waitmovement 0
	msgbox Kanto_ThreeIsland_Text_84328A7, MSGBOX_DEFAULT
	closemessage
	delay 0x2D
	call_if_eq VAR_TEMP_1, 0, Kanto_ThreeIsland_EventScript_8417865
	call_if_eq VAR_TEMP_1, 1, Kanto_ThreeIsland_EventScript_8417877
	call_if_eq VAR_TEMP_1, 2, Kanto_ThreeIsland_EventScript_8417889
	call_if_eq VAR_TEMP_1, 3, Kanto_ThreeIsland_EventScript_84178A2
	call_if_eq VAR_TEMP_1, 4, Kanto_ThreeIsland_EventScript_84178BB
	playbgm MUS_RG_ENCOUNTER_BOY, 0
	msgbox Kanto_ThreeIsland_Text_84328E8, MSGBOX_DEFAULT
	setvar VAR_LAST_TALKED, 3
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_8934, 0, Kanto_ThreeIsland_Text_843297C
	msgbox Kanto_ThreeIsland_Text_84329E2, MSGBOX_DEFAULT
	closemessage
	fadescreen 1
	removeobject 3
	removeobject 5
	removeobject 7
	removeobject 4
	removeobject 6
	removeobject 8
	removeobject 14
	fadescreen 0
	setvar VAR_QZ_0x5131, 4
	releaseall
	end

Kanto_ThreeIsland_EventScript_8417865::
	applymovement 4, Quetzal_Movement_8457AB0
	applymovement 3, Kanto_ThreeIsland_Movement_84178FD
	waitmovement 0
	return

Kanto_ThreeIsland_EventScript_8417877::
	applymovement 4, Quetzal_Movement_8457AB0
	applymovement 3, Kanto_ThreeIsland_Movement_8417901
	waitmovement 0
	return

Kanto_ThreeIsland_EventScript_8417889::
	applymovement 4, Quetzal_Movement_8457AB0
	applymovement 3, Kanto_ThreeIsland_Movement_8417903
	applymovement LOCALID_PLAYER, Kanto_ThreeIsland_Movement_8417907
	waitmovement 0
	return

Kanto_ThreeIsland_EventScript_84178A2::
	applymovement 4, Quetzal_Movement_8457AB0
	applymovement LOCALID_PLAYER, Kanto_ThreeIsland_Movement_8417907
	applymovement 3, Kanto_ThreeIsland_Movement_841790B
	waitmovement 0
	return

Kanto_ThreeIsland_EventScript_84178BB::
	applymovement 4, Quetzal_Movement_8457AB0
	applymovement LOCALID_PLAYER, Kanto_ThreeIsland_Movement_8417907
	applymovement 3, Kanto_ThreeIsland_Movement_8417910
	waitmovement 0
	return

Kanto_ThreeIsland_EventScript_84178D4::
	msgbox Kanto_ThreeIsland_Text_8432718, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, Kanto_ThreeIsland_Movement_84178F4
	waitmovement 0
	applymovement 5, Kanto_ThreeIsland_Movement_84178F8
	waitmovement 0
	fadedefaultbgm
	releaseall
	end

@ Kanto_ThreeIsland object 11 (local id 11, gfx 0x3F57) at 15,7
Kanto_ThreeIsland_EventScript_8417916::
	msgbox Kanto_ThreeIsland_Text_8432BB0, MSGBOX_NPC
	end

@ Kanto_ThreeIsland object 12 (local id 12, gfx 0x3F17) at 14,14
Kanto_ThreeIsland_EventScript_841791F::
	msgbox Kanto_ThreeIsland_Text_8432C4F, MSGBOX_NPC
	end

@ Kanto_ThreeIsland object 13 (local id 13, gfx 0x3F02) at 15,14
Kanto_ThreeIsland_EventScript_8417928::
	lock
	faceplayer
	waitse
	playmoncry SPECIES_DODUO, 0
	msgbox Kanto_ThreeIsland_Text_8432CB9, MSGBOX_DEFAULT
	waitmoncry
	release
	end

@ Kanto_ThreeIsland sign / bg event at 12,31
Kanto_ThreeIsland_EventScript_841793B::
	msgbox Kanto_ThreeIsland_Text_8432326, MSGBOX_SIGN
	end

@ Kanto_ThreeIsland object 14 (local id 14, gfx 0x3EE9) at 15,31
Kanto_ThreeIsland_EventScript_8417944::
	msgbox Kanto_ThreeIsland_Text_8432350, MSGBOX_NPC
	end

@ Kanto_ThreeIsland object 10 (local id 10, gfx 0x3F0E) at 18,17
Kanto_ThreeIsland_EventScript_846BFA2::
	setorcopyvar VAR_0x8000, 0x45
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_ThreeIsland_Movement_84176B8:
	walk_in_place_left
	step_end

Kanto_ThreeIsland_Movement_84176BA:
	walk_in_place_right
	step_end

Kanto_ThreeIsland_Movement_84178F4:
	walk_down
	step_end

Kanto_ThreeIsland_Movement_84178F6:
	walk_down
	step_end

Kanto_ThreeIsland_Movement_84178F8:
	walk_up
	walk_in_place_faster_left
	step_end

Kanto_ThreeIsland_Movement_84178FB:
	walk_in_place_down
	step_end

Kanto_ThreeIsland_Movement_84178FD:
	walk_down
	walk_left
	walk_in_place_faster_down
	step_end

Kanto_ThreeIsland_Movement_8417901:
	walk_down
	step_end

Kanto_ThreeIsland_Movement_8417903:
	walk_down
	walk_down
	walk_in_place_faster_right
	step_end

Kanto_ThreeIsland_Movement_8417907:
	delay_16
	delay_16
	walk_in_place_faster_left
	step_end

Kanto_ThreeIsland_Movement_841790B:
	walk_down
	walk_down
	walk_right
	walk_in_place_faster_right
	step_end

Kanto_ThreeIsland_Movement_8417910:
	walk_down
	walk_down
	walk_right
	walk_right
	walk_in_place_faster_right
	step_end

Kanto_ThreeIsland_Text_8432326::
	.string "THREE ISLAND\n"
	.string "Kin Island of Family Bonding$"

Kanto_ThreeIsland_Text_8432350::
	.string "Hyahoo!\p"
	.string "From this day on, this island\n"
	.string "belongs to us!$"

Kanto_ThreeIsland_Text_8432385::
	.string "We don’t need you people bringing\n"
	.string "your noise and trouble here!\p"
	.string "We’re asking you to go back to\n"
	.string "KANTO!$"

Kanto_ThreeIsland_Text_84323EA::
	.string "Hey, go cry somewhere else.\n"
	.string "Our boss is on his way.\p"
	.string "When he gets here, we’ll give you\n"
	.string "a k-rad motorbike show you won’t\l"
	.string "soon forget!$"

Kanto_ThreeIsland_Text_843246E::
	.string "W-what!? Not on your life!\n"
	.string "Get off the island now!$"

Kanto_ThreeIsland_Text_84324A1::
	.string "Who’s gonna make me?$"

Kanto_ThreeIsland_Text_84324B6::
	.string "Are you the boss?\n"
	.string "Go back to KANTO right now!$"

Kanto_ThreeIsland_Text_84324E4::
	.string "Hah?\p"
	.string "I just got here, pal.\p"
	.string "What’s with the hostile attitude?\n"
	.string "It’s mighty cold of you!$"

Kanto_ThreeIsland_Text_843253A::
	.string "Your gang of followers have been\n"
	.string "raising havoc on their bikes.\p"
	.string "Do you have any idea how much\n"
	.string "trouble they’ve caused us on the\l"
	.string "island?$"

Kanto_ThreeIsland_Text_84325C0::
	.string "No, man, I don’t get it at all.\p"
	.string "Look at this place.\n"
	.string "What do you do for entertainment?\p"
	.string "You ought to be thanking us for\n"
	.string "livening up this sleepy village.\p"
	.string "But hey, if you insist, you can try\n"
	.string "making us leave.$"

Kanto_ThreeIsland_Text_843268C::
	.string "Grr… You cowards…\n"
	.string "So tough in a pack…$"

Kanto_ThreeIsland_Text_84326B2::
	.string "You, what are you staring at?\n"
	.string "Don’t you know it’s not polite?\p"
	.string "You wanna make something of it\n"
	.string "or what?$"

Kanto_ThreeIsland_Text_8432718::
	.string "That’s smart.\n"
	.string "Keep your nose out of this.$"

Kanto_ThreeIsland_Text_8432742::
	.string "Heh, I like your guts.\n"
	.string "You’ll be losing money to me, but…$"

Kanto_ThreeIsland_Text_843277C::
	.string "Wha…\n"
	.string "What is this kid?!$"

Kanto_ThreeIsland_Text_8432794::
	.string "Aww, man…\n"
	.string "Don’t you dare laugh!$"

Kanto_ThreeIsland_Text_84327B4::
	.string "Aren’t you from KANTO?\n"
	.string "You should be on our side!$"

Kanto_ThreeIsland_Text_84327E6::
	.string "Stop fooling around!$"

Kanto_ThreeIsland_Text_84327FB::
	.string "What’s the matter with you,\n"
	.string "getting all hot like that?\p"
	.string "Totally uncool, man!$"

Kanto_ThreeIsland_Text_8432847::
	.string "We invited the boss out here,\n"
	.string "but you had to mess it up!\p"
	.string "You embarrassed us, man!$"

Kanto_ThreeIsland_Text_8432899::
	.string "… … …   … … …$"

Kanto_ThreeIsland_Text_84328A7::
	.string "Boss, I’m telling you, you’ve gotta\n"
	.string "do something about this kid!$"

Kanto_ThreeIsland_Text_84328E8::
	.string "I’ve been watching you, and I’d say\n"
	.string "you’ve done enough.\p"
	.string "What are you, their friend or\n"
	.string "something?\p"
	.string "Then I guess you’ll be battling me\n"
	.string "in their place.$"

Kanto_ThreeIsland_Text_843297C::
	.string "All right, enough!\n"
	.string "We’ll leave like you wanted!\p"
	.string "We’ll be happy to see the last of\n"
	.string "this boring island!$"

Kanto_ThreeIsland_Text_84329E2::
	.string "Humph! Yeah, go right on hanging\n"
	.string "around with these hayseeds!$"

Kanto_ThreeIsland_Text_8432A1F::
	.string "Thank you! Those goons were\n"
	.string "nothing but bad trouble.\p"
	.string "Oh, look, your POKéMON got hurt\n"
	.string "on my account.$"

Kanto_ThreeIsland_Text_8432A83::
	.string "{PLAYER} was given\n"
	.string "a FULL RESTORE.$"

Kanto_ThreeIsland_Text_8432AA0::
	.string "Oh?\n"
	.string "Your BAG is full.$"

Kanto_ThreeIsland_Text_8432AB6::
	.string "You’ve got seriously impressive\n"
	.string "POKéMON with you.$"

Kanto_ThreeIsland_Text_8432AE8::
	.string "It’d be fantastic if someone as\n"
	.string "strong as you lived here.\p"
	.string "I hope you’ll at least stay here\n"
	.string "a while.\p"
	.string "…I beg your pardon?\n"
	.string "You’re looking for LOSTELLE?\p"
	.string "LOSTELLE went off towards BOND\n"
	.string "BRIDGE a while ago.$"

Kanto_ThreeIsland_Text_8432BB0::
	.string "THREE ISLAND is actually the most\n"
	.string "populous of the islands here.\p"
	.string "Still, it could be less sleepy.\p"
	.string "But I wouldn’t want to see goons\n"
	.string "like those BIKERS here again.$"

Kanto_ThreeIsland_Text_8432C4F::
	.string "I’m going to train my DODUO in the\n"
	.string "BERRY FOREST.\p"
	.string "When it evolves, I’m going to play\n"
	.string "a game on TWO ISLAND.$"

Kanto_ThreeIsland_Text_8432CB9::
	.string "DODUO: Gigiih!$"
