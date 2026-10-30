Johto_Route41_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_Route41_EventScript_83F43A4
	.byte 0

Johto_Route41_EventScript_83F43A4::
	call Quetzal_EventScript_840EA62
	goto_if_ge VAR_QZ_0x500D, 5, Quetzal_EventScript_83E3F8B
	end

@ Johto_Route41 object 2 (local id 2, gfx 0x3EB9) at 30,18
Johto_Route41_EventScript_83F43B5::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4162, 0, Johto_Route41_Text_83F43CC, Johto_Route41_Text_83F4411
	msgbox Johto_Route41_Text_83F442D, MSGBOX_AUTOCLOSE
	end

@ Johto_Route41 object 6 (local id 6, gfx 0x3EB9) at 38,77
Johto_Route41_EventScript_83F4458::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4169, 0, Johto_Route41_Text_83F446F, Johto_Route41_Text_83F44A0
	msgbox Johto_Route41_Text_83F44AC, MSGBOX_AUTOCLOSE
	end

@ Johto_Route41 object 1 (local id 1, gfx 0x3EB9) at 51,21
Johto_Route41_EventScript_83F4507::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4170, 0, Johto_Route41_Text_83F451E, Johto_Route41_Text_83F456F
	msgbox Johto_Route41_Text_83F4593, MSGBOX_AUTOCLOSE
	end

@ Johto_Route41 object 7 (local id 7, gfx 0x3EB9) at 28,40
Johto_Route41_EventScript_83F4617::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4287, 0, Johto_Route41_Text_83F462E, Johto_Route41_Text_83F4669
	msgbox Johto_Route41_Text_83F4673, MSGBOX_AUTOCLOSE
	end

@ Johto_Route41 object 9 (local id 9, gfx 0x3EB9) at 9,62
Johto_Route41_EventScript_83F46A3::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4442, 0, Johto_Route41_Text_83F46BA, Johto_Route41_Text_83F46E8
	msgbox Johto_Route41_Text_83F4709, MSGBOX_AUTOCLOSE
	end

@ Johto_Route41 object 5 (local id 5, gfx 0x3EB8) at 64,34
Johto_Route41_EventScript_83F4740::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4558, 0, Johto_Route41_Text_83F4757, Johto_Route41_Text_83F4799
	msgbox Johto_Route41_Text_83F47B0, MSGBOX_AUTOCLOSE
	end

@ Johto_Route41 object 8 (local id 8, gfx 0x3EB8) at 63,62
Johto_Route41_EventScript_83F480C::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4552, 0, Johto_Route41_Text_83F4823, Johto_Route41_Text_83F484D
	msgbox Johto_Route41_Text_83F485A, MSGBOX_AUTOCLOSE
	end

@ Johto_Route41 object 3 (local id 3, gfx 0x3EB8) at 15,19
Johto_Route41_EventScript_83F488F::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4540, 0, Johto_Route41_Text_83F48A6, Johto_Route41_Text_83F48D2
	msgbox Johto_Route41_Text_83F48D8, MSGBOX_AUTOCLOSE
	end

@ Johto_Route41 object 4 (local id 4, gfx 0x3EB8) at 34,29
Johto_Route41_EventScript_83F4930::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4553, 0, Johto_Route41_Text_83F4947, Johto_Route41_Text_83F49AA
	msgbox Johto_Route41_Text_83F49CE, MSGBOX_AUTOCLOSE
	end

@ Johto_Route41 object 10 (local id 10, gfx 0x3EB8) at 35,54
Johto_Route41_EventScript_83F49FA::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4188, 0, Johto_Route41_Text_83F4A11, Johto_Route41_Text_83F4A43
	msgbox Johto_Route41_Text_83F4A4D, MSGBOX_AUTOCLOSE
	end

Johto_Route41_Text_83F43CC::
	.string "The water’s warm here.\n"
	.string "I’m loose and limber.\p"
	.string "Sure, I’ll take you on!$"

Johto_Route41_Text_83F4411::
	.string "Yikes! I’ve got prune skin!$"

Johto_Route41_Text_83F442D::
	.string "Isn’t it relaxing just floating\n"
	.string "like this?$"

Johto_Route41_Text_83F446F::
	.string "I’m a bit tired.\n"
	.string "If I win, lend me your POKéMON.$"

Johto_Route41_Text_83F44A0::
	.string "Pant, pant…$"

Johto_Route41_Text_83F44AC::
	.string "It’s so far to CIANWOOD.\p"
	.string "But it’s no easy return trip\n"
	.string "to OLIVINE either.\p"
	.string "What should I do?$"

Johto_Route41_Text_83F451E::
	.string "See those islands that are\n"
	.string "blocked by whirlpools?\p"
	.string "There just has to be a secret!$"

Johto_Route41_Text_83F456F::
	.string "What’s the secret to your\n"
	.string "strength?$"

Johto_Route41_Text_83F4593::
	.string "It was a dark and stormy night…\p"
	.string "I saw this giant POKéMON flying\n"
	.string "from the islands.\p"
	.string "It was scattering feathers\n"
	.string "from its silver wings.$"

Johto_Route41_Text_83F462E::
	.string "The waves are wild here.\p"
	.string "They tire you out while\n"
	.string "you swim.$"

Johto_Route41_Text_83F4669::
	.string "I’m beat!$"

Johto_Route41_Text_83F4673::
	.string "The currents keep me from\n"
	.string "reaching that island.$"

Johto_Route41_Text_83F46BA::
	.string "Are you seeking the secrets\n"
	.string "of WHIRL ISLANDS?$"

Johto_Route41_Text_83F46E8::
	.string "Ooh, you’ve got great endurance!$"

Johto_Route41_Text_83F4709::
	.string "A secret about WHIRL ISLANDS…\p"
	.string "It’s pitch-black inside!$"

Johto_Route41_Text_83F4757::
	.string "I’m on my way to WHIRL ISLANDS.\p"
	.string "I’m going exploring with friends.$"

Johto_Route41_Text_83F4799::
	.string "Is that how you do it?$"

Johto_Route41_Text_83F47B0::
	.string "There’s supposed to be a big\n"
	.string "POKéMON deep beneath WHIRL ISLANDS.\p"
	.string "I wonder what it could be?$"

Johto_Route41_Text_83F4823::
	.string "You look so elegant,\n"
	.string "riding your POKéMON.$"

Johto_Route41_Text_83F484D::
	.string "I’m crushed…$"

Johto_Route41_Text_83F485A::
	.string "Wasn’t there a hit song about\n"
	.string "a boy riding a LAPRAS?$"

Johto_Route41_Text_83F48A6::
	.string "The weather is so beautiful,\n"
	.string "I’m in a daze!$"

Johto_Route41_Text_83F48D2::
	.string "Ohhh!$"

Johto_Route41_Text_83F48D8::
	.string "A sunburn is the worst for skin.\p"
	.string "But I don’t use a sunblock.\p"
	.string "I won’t pollute the water.$"

Johto_Route41_Text_83F4947::
	.string "If you need to rest,\n"
	.string "just tread water.\p"
	.string "You’ll get your wind back,\n"
	.string "so you can keep on going strong.$"

Johto_Route41_Text_83F49AA::
	.string "Oh! You have more energy than I do.$"

Johto_Route41_Text_83F49CE::
	.string "I heard roars from deep inside\n"
	.string "the ISLANDS.$"

Johto_Route41_Text_83F4A11::
	.string "At night, STARYU gather near\n"
	.string "the water’s surface.$"

Johto_Route41_Text_83F4A43::
	.string "Oh, dear…$"

Johto_Route41_Text_83F4A4D::
	.string "The clusters of STARYU light up\n"
	.string "at the same time.\p"
	.string "It’s so beautiful, it’s scary.$"
