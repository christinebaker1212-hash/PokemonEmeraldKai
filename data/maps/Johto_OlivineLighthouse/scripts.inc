Johto_OlivineLighthouse_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840EA66
	.byte 0

@ Johto_OlivineLighthouse object 18 (local id 18, gfx 0x3A9E) at 66,3
Johto_OlivineLighthouse_EventScript_83E59A9::
	setorcopyvar VAR_0x8000, ITEM_ETHER
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_OlivineLighthouse object 17 (local id 17, gfx 0x3A9E) at 119,13
Johto_OlivineLighthouse_EventScript_83E59B6::
	setorcopyvar VAR_0x8000, ITEM_RARE_CANDY
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_OlivineLighthouse object 15 (local id 15, gfx 0x3A9E) at 135,14
Johto_OlivineLighthouse_EventScript_83E59D0::
	setorcopyvar VAR_0x8000, ITEM_HYPER_POTION
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_OlivineLighthouse object 14 (local id 14, gfx 0x3A9E) at 149,3
Johto_OlivineLighthouse_EventScript_83E59DD::
	setorcopyvar VAR_0x8000, ITEM_PROTEIN
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_OlivineLighthouse object 2 (local id 2, gfx 0x31) at 5,3
Johto_OlivineLighthouse_EventScript_83E59EA::
	lock
	faceplayer
	msgbox Johto_OlivineLighthouse_Text_83E5A04, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_OlivineLighthouse object 1 (local id 1, gfx 0x18) at 19,9
Johto_OlivineLighthouse_EventScript_83E59F7::
	lock
	faceplayer
	msgbox Johto_OlivineLighthouse_Text_83E5A62, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_OlivineLighthouse object 3 (local id 3, gfx 0x31) at 47,3
Johto_OlivineLighthouse_EventScript_83E5AE0::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4586, 0, Johto_OlivineLighthouse_Text_83E5B0E, Johto_OlivineLighthouse_Text_83E5B43
	msgbox Johto_OlivineLighthouse_Text_83E5B50, MSGBOX_AUTOCLOSE
	end

@ Johto_OlivineLighthouse object 4 (local id 4, gfx 0x30) at 43,13
Johto_OlivineLighthouse_EventScript_83E5AF7::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4457, 0, Johto_OlivineLighthouse_Text_83E5B89, Johto_OlivineLighthouse_Text_83E5BAB
	msgbox Johto_OlivineLighthouse_Text_83E5BCE, MSGBOX_AUTOCLOSE
	end

@ Johto_OlivineLighthouse object 6 (local id 6, gfx 0x3EAE) at 61,9
Johto_OlivineLighthouse_EventScript_83E5C3D::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4675, 0, Johto_OlivineLighthouse_Text_83E5C82, Johto_OlivineLighthouse_Text_83E5CC5
	msgbox Johto_OlivineLighthouse_Text_83E5CF6, MSGBOX_AUTOCLOSE
	end

@ Johto_OlivineLighthouse object 5 (local id 5, gfx 0x31) at 67,3
Johto_OlivineLighthouse_EventScript_83E5C54::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4666, 0, Johto_OlivineLighthouse_Text_83E5D4D, Johto_OlivineLighthouse_Text_83E5D7E
	msgbox Johto_OlivineLighthouse_Text_83E5D9C, MSGBOX_AUTOCLOSE
	end

@ Johto_OlivineLighthouse object 7 (local id 7, gfx 0x30) at 78,7
Johto_OlivineLighthouse_EventScript_83E5C6B::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4329, 0, Johto_OlivineLighthouse_Text_83E5E08, Johto_OlivineLighthouse_Text_83E5E46
	msgbox Johto_OlivineLighthouse_Text_83E5E65, MSGBOX_AUTOCLOSE
	end

@ Johto_OlivineLighthouse object 8 (local id 8, gfx 0x31) at 101,4
Johto_OlivineLighthouse_EventScript_83E5E96::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4716, 0, Johto_OlivineLighthouse_Text_83E5EC4, Johto_OlivineLighthouse_Text_83E5F1A
	msgbox Johto_OlivineLighthouse_Text_83E5F39, MSGBOX_AUTOCLOSE
	end

@ Johto_OlivineLighthouse object 9 (local id 9, gfx 0x3E8F) at 101,14
Johto_OlivineLighthouse_EventScript_83E5EAD::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4224, 0, Johto_OlivineLighthouse_Text_83E5F7B, Johto_OlivineLighthouse_Text_83E5FE1
	msgbox Johto_OlivineLighthouse_Text_83E5FF4, MSGBOX_AUTOCLOSE
	end

@ Johto_OlivineLighthouse object 11 (local id 11, gfx 0x31) at 126,12
Johto_OlivineLighthouse_EventScript_83E607E::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4200, 0, Johto_OlivineLighthouse_Text_83E60AC, Johto_OlivineLighthouse_Text_83E60F7
	msgbox Johto_OlivineLighthouse_Text_83E6118, MSGBOX_AUTOCLOSE
	end

@ Johto_OlivineLighthouse object 10 (local id 10, gfx 0x3EAE) at 127,3
Johto_OlivineLighthouse_EventScript_83E6095::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4305, 0, Johto_OlivineLighthouse_Text_83E616D, Johto_OlivineLighthouse_Text_83E61AB
	msgbox Johto_OlivineLighthouse_Text_83E61BF, MSGBOX_AUTOCLOSE
	end

@ Johto_OlivineLighthouse object 12 (local id 12, gfx 0x3E9A) at 156,9
Johto_OlivineLighthouse_EventScript_83E6220::
	lock
	faceplayer
	goto_if_ge VAR_QZ_0x500D, 5, Johto_OlivineLighthouse_EventScript_83E626A
	checkitem ITEM_SECRET_POTION, 1
	goto_if_eq VAR_RESULT, 1, Johto_OlivineLighthouse_EventScript_83E6498
	goto_if_ge VAR_QZ_0x500D, 3, Johto_OlivineLighthouse_EventScript_83E6260
	msgbox Johto_OlivineLighthouse_Text_83E62B6, MSGBOX_DEFAULT
	msgbox Johto_OlivineLighthouse_Text_83E639D, MSGBOX_DEFAULT
	closemessage
	setvar VAR_QZ_0x500D, 3
	release
	end

Johto_OlivineLighthouse_EventScript_83E6260::
	msgbox Johto_OlivineLighthouse_Text_83E63D1, MSGBOX_DEFAULT
	release
	end

Johto_OlivineLighthouse_EventScript_83E626A::
	msgbox Johto_OlivineLighthouse_Text_83E65F8, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_OlivineLighthouse object 13 (local id 13, gfx 0x1B5) at 156,8
Johto_OlivineLighthouse_EventScript_83E6275::
	lock
	faceplayer
	goto_if_lt VAR_QZ_0x500D, 5, Johto_OlivineLighthouse_EventScript_83E629D
	fadescreen 3
	playse SE_M_THUNDERBOLT2
	waitse
	fadescreen 2
	playmoncry SPECIES_AMPHAROS, 0
	msgbox Johto_OlivineLighthouse_Text_83E66F3, MSGBOX_DEFAULT
	waitmoncry
	setflag FLAG_QZ_0x10AA
	release
	end

Johto_OlivineLighthouse_EventScript_83E629D::
	playmoncry SPECIES_AMPHAROS, 0
	msgbox Johto_OlivineLighthouse_Text_83E66BB, MSGBOX_DEFAULT
	waitmoncry
	msgbox Johto_OlivineLighthouse_Text_83E66D0, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_OlivineLighthouse_EventScript_83E6498::
	msgbox Johto_OlivineLighthouse_Text_83E646F, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Johto_OlivineLighthouse_EventScript_83E6544
	removeitem ITEM_SECRET_POTION, 1
	goto_if_eq VAR_RESULT, 0, Johto_OlivineLighthouse_EventScript_83E6260
	playse SE_USE_ITEM
	msgbox Johto_OlivineLighthouse_Text_83E654F, MSGBOX_DEFAULT
	waitse
	msgbox Johto_OlivineLighthouse_Text_83E6577, MSGBOX_DEFAULT
	closemessage
	turnobject 12, 2
	delay 0x14
	turnobject 13, 1
	playse MUS_HEAL
	waitse
	msgbox Johto_OlivineLighthouse_Text_83E65D1, MSGBOX_DEFAULT
	fadescreen 3
	playse SE_M_THUNDERBOLT2
	waitse
	fadescreen 2
	playmoncry SPECIES_AMPHAROS, 0
	msgbox Johto_OlivineLighthouse_Text_83E66F3, MSGBOX_DEFAULT
	waitmoncry
	closemessage
	applymovement 13, Johto_OlivineLighthouse_Movement_83E653D
	waitmovement 0
	applymovement 12, Johto_OlivineLighthouse_Movement_83E653B
	msgbox Johto_OlivineLighthouse_Text_83E65F8, MSGBOX_DEFAULT
	closemessage
	fadescreen 1
	playse SE_EXIT
	setflag FLAG_QZ_0x10B7
	removeobject 12
	waitse
	delay 0x32
	fadescreen 0
	setvar VAR_QZ_0x500F, 1
	setvar VAR_QZ_0x500D, 5
	setflag FLAG_QZ_0x10AA
	clearflag FLAG_QZ_0x10B6
	release
	end

Johto_OlivineLighthouse_EventScript_83E6544::
	msgbox Johto_OlivineLighthouse_Text_83E6699, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_OlivineLighthouse_Movement_83E653B:
	face_player
	step_end

Johto_OlivineLighthouse_Movement_83E653D:
	walk_up
	jump_in_place_down
	jump_in_place_left
	jump_in_place_right
	jump_in_place_up
	face_down
	step_end

Johto_OlivineLighthouse_Text_83E5A04::
	.string "People train at this LIGHTHOUSE.\p"
	.string "It’s not easy to climb with all\n"
	.string "the trainers waiting inside.$"

Johto_OlivineLighthouse_Text_83E5A62::
	.string "In the past, POKéMON used to light\n"
	.string "the sea around OLIVINE at night.\p"
	.string "The LIGHTHOUSE was built in honor\n"
	.string "of those noble POKéMON.$"

Johto_OlivineLighthouse_Text_83E5B0E::
	.string "Men of the sea are always\n"
	.string "spoiling for a good fight!$"

Johto_OlivineLighthouse_Text_83E5B43::
	.string "Urf!\n"
	.string "I lose!$"

Johto_OlivineLighthouse_Text_83E5B50::
	.string "What power! How would you like\n"
	.string "to sail the seas with me?$"

Johto_OlivineLighthouse_Text_83E5B89::
	.string "Hm? This is no place\n"
	.string "for playing.$"

Johto_OlivineLighthouse_Text_83E5BAB::
	.string "Ah! I can see that you’re\n"
	.string "serious.$"

Johto_OlivineLighthouse_Text_83E5BCE::
	.string "Up top is a POKéMON that keeps\n"
	.string "the LIGHTHOUSE lit.\p"
	.string "But I hear that it’s sick now,\n"
	.string "and can’t be cured normally.$"

Johto_OlivineLighthouse_Text_83E5C82::
	.string "Why are you here? Are you just\n"
	.string "going to gawk? I suggest you leave!$"

Johto_OlivineLighthouse_Text_83E5CC5::
	.string "You really are concerned about\n"
	.string "the sick POKéMON…$"

Johto_OlivineLighthouse_Text_83E5CF6::
	.string "How the heck do you go up?\p"
	.string "I want to visit the sick POKéMON,\n"
	.string "but I can’t get up there…$"

Johto_OlivineLighthouse_Text_83E5D4D::
	.string "Sailors are both kind and strong.\n"
	.string "How about you?$"

Johto_OlivineLighthouse_Text_83E5D7E::
	.string "You are both kind and strong…$"

Johto_OlivineLighthouse_Text_83E5D9C::
	.string "Every time I return to OLIVINE,\n"
	.string "I visit the GYM.\p"
	.string "The GYM LEADER’s POKéMON type\n"
	.string "changed without me noticing.$"

Johto_OlivineLighthouse_Text_83E5E08::
	.string "I travel the world to train my\n"
	.string "POKéMON. I wish to battle you.$"

Johto_OlivineLighthouse_Text_83E5E46::
	.string "…sigh… I must train some more…$"

Johto_OlivineLighthouse_Text_83E5E65::
	.string "JASMINE used to use ROCK-type\n"
	.string "POKéMON like ONIX.$"

Johto_OlivineLighthouse_Text_83E5EC4::
	.string "JASMINE must be worried sick about\n"
	.string "the POKéMON here.\p"
	.string "She won’t even smile these days.$"

Johto_OlivineLighthouse_Text_83E5F1A::
	.string "I can’t manage a smile either…$"

Johto_OlivineLighthouse_Text_83E5F39::
	.string "Speaking of sick, I’ve heard there’s\n"
	.string "a good PHARMACY in CIANWOOD.$"

Johto_OlivineLighthouse_Text_83E5F7B::
	.string "JASMINE is this city’s GYM LEADER.\p"
	.string "I mean to bring her back with me.\p"
	.string "Nobody had better get in my way!$"

Johto_OlivineLighthouse_Text_83E5FE1::
	.string "Aaack! My POKéMON!$"

Johto_OlivineLighthouse_Text_83E5FF4::
	.string "Right. Anybody would be worried if\n"
	.string "his or her own POKéMON were hurt.\p"
	.string "I’ll pray for the POKéMON here, so\n"
	.string "JASMINE can come back to the GYM.$"

Johto_OlivineLighthouse_Text_83E60AC::
	.string "I wanted to battle JASMINE, but she’s\n"
	.string "not up to it now.\p"
	.string "So, how about you?$"

Johto_OlivineLighthouse_Text_83E60F7::
	.string "Whoa, whoa. You’re overwhelming!$"

Johto_OlivineLighthouse_Text_83E6118::
	.string "A GYM LEADER isn’t just about being\n"
	.string "strong.\p"
	.string "A LEADER also needs to be\n"
	.string "compassionate.$"

Johto_OlivineLighthouse_Text_83E616D::
	.string "We’re pretty high up here. My bird\n"
	.string "POKéMON are in prime form.$"

Johto_OlivineLighthouse_Text_83E61AB::
	.string "Oops… They crashed…$"

Johto_OlivineLighthouse_Text_83E61BF::
	.string "My POKéMON learned how to use FLY in\n"
	.string "CIANWOOD.\p"
	.string "Well, since I’m a loser, I’ll FLY\n"
	.string "across the sea…$"

Johto_OlivineLighthouse_Text_83E62B6::
	.string "JASMINE: …This POKéMON always kept\n"
	.string "the sea lit at night.\p"
	.string "…But it suddenly got sick… It’s\n"
	.string "gasping for air…\p"
	.string "…I understand that there is a\n"
	.string "wonderful PHARMACY in CIANWOOD…\p"
	.string "But that’s across the sea…\n"
	.string "And I can’t leave AMPHY unattended…$"

Johto_OlivineLighthouse_Text_83E639D::
	.string "…May I ask you to get some medicine\n"
	.string "for me? Please?$"

Johto_OlivineLighthouse_Text_83E63D1::
	.string "There is a wonderful PHARMACY\n"
	.string "in CIANWOOD…\p"
	.string "But that’s across the sea…\n"
	.string "And I can’t leave AMPHY unattended…\p"
	.string "…May I ask you to get some medicine\n"
	.string "for me? Please?$"

Johto_OlivineLighthouse_Text_83E646F::
	.string "JASMINE: …Will that medicine cure\n"
	.string "AMPHY?$"

Johto_OlivineLighthouse_Text_83E654F::
	.string "{PLAYER} handed the SECRET POTION to\n"
	.string "JASMINE.$"

Johto_OlivineLighthouse_Text_83E6577::
	.string "JASMINE: …Um, please don’t be\n"
	.string "offended…\p"
	.string "…AMPHY will not take anything from\n"
	.string "anyone but me…$"

Johto_OlivineLighthouse_Text_83E65D1::
	.string "JASMINE: …\p"
	.string "AMPHY, how are you feeling?$"

Johto_OlivineLighthouse_Text_83E65F8::
	.string "JASMINE: …Oh, I’m so relieved…\p"
	.string "This is just so wonderful…\p"
	.string "Thank you so very, very much.\p"
	.string "…I will return to the GYM…\p"
	.string "I’ll also warn you, STEEL types\n"
	.string "are powerful.$"

Johto_OlivineLighthouse_Text_83E6699::
	.string "JASMINE: …I see…\n"
	.string "…AMPHY, hang on!$"

Johto_OlivineLighthouse_Text_83E66BB::
	.string "AMPHY: …\n"
	.string "…Pa… paloo…$"

Johto_OlivineLighthouse_Text_83E66D0::
	.string "Its breathing is terribly\n"
	.string "labored…$"

Johto_OlivineLighthouse_Text_83E66F3::
	.string "AMPHY: Palu! Palulu!$"
