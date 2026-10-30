Kanto_SevenIsland_SevaultCanyon_Entrance_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840FBB9
	.byte 0

@ Kanto_SevenIsland_SevaultCanyon_Entrance sign / bg event at 14,26
Kanto_SevenIsland_SevaultCanyon_Entrance_EventScript_84189F8::
	msgbox Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843CB45, MSGBOX_SIGN
	end

@ Kanto_SevenIsland_SevaultCanyon_Entrance object 1 (local id 1, gfx 0x3F58) at 12,6
Kanto_SevenIsland_SevaultCanyon_Entrance_EventScript_845CC9A::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8780, 0, Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843C6D4, Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843C6FF
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_SevenIsland_SevaultCanyon_Entrance_EventScript_845CCC1
	msgbox Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843C717, MSGBOX_AUTOCLOSE
	end

Kanto_SevenIsland_SevaultCanyon_Entrance_EventScript_845CCC1::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8780, 0, Kanto_SevenIsland_SevaultCanyon_Entrance_Text_84717CD, Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843C6FF
	msgbox Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843C717, MSGBOX_AUTOCLOSE
	end

@ Kanto_SevenIsland_SevaultCanyon_Entrance object 2 (local id 2, gfx 0x3EE8) at 3,34
Kanto_SevenIsland_SevaultCanyon_Entrance_EventScript_845CCD8::
	trainerbattle TRAINER_BATTLE_DOUBLE, TRAINER_QZ_8781, 0, Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843C94A, Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843C97A, Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843C9C3
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_SevenIsland_SevaultCanyon_Entrance_EventScript_845CD03
	msgbox Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843C99B, MSGBOX_AUTOCLOSE
	end

Kanto_SevenIsland_SevaultCanyon_Entrance_EventScript_845CD03::
	trainerbattle TRAINER_BATTLE_REMATCH_DOUBLE, TRAINER_QZ_8781, 0, Kanto_SevenIsland_SevaultCanyon_Entrance_Text_84718BC, Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843C97A, Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843C9C3
	msgbox Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843C99B, MSGBOX_AUTOCLOSE
	end

@ Kanto_SevenIsland_SevaultCanyon_Entrance object 3 (local id 3, gfx 0x3F1E) at 4,34
Kanto_SevenIsland_SevaultCanyon_Entrance_EventScript_845CD1E::
	trainerbattle TRAINER_BATTLE_DOUBLE, TRAINER_QZ_8781, 0, Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843CA18, Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843CA53, Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843CAD5
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_SevenIsland_SevaultCanyon_Entrance_EventScript_845CD49
	msgbox Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843CA8B, MSGBOX_AUTOCLOSE
	end

Kanto_SevenIsland_SevaultCanyon_Entrance_EventScript_845CD49::
	trainerbattle TRAINER_BATTLE_REMATCH_DOUBLE, TRAINER_QZ_8781, 0, Kanto_SevenIsland_SevaultCanyon_Entrance_Text_84718F2, Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843CA53, Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843CAD5
	msgbox Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843CA8B, MSGBOX_AUTOCLOSE
	end

@ Kanto_SevenIsland_SevaultCanyon_Entrance object 6 (local id 6, gfx 0x3F3B) at 11,17
Kanto_SevenIsland_SevaultCanyon_Entrance_EventScript_845CD64::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8782, 0, Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843C763, Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843C78B
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_SevenIsland_SevaultCanyon_Entrance_EventScript_845CD8B
	msgbox Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843C7D2, MSGBOX_AUTOCLOSE
	end

Kanto_SevenIsland_SevaultCanyon_Entrance_EventScript_845CD8B::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8782, 0, Kanto_SevenIsland_SevaultCanyon_Entrance_Text_847181A, Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843C78B
	msgbox Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843C7D2, MSGBOX_AUTOCLOSE
	end

@ Kanto_SevenIsland_SevaultCanyon_Entrance object 4 (local id 4, gfx 0x3EF3) at 11,26
Kanto_SevenIsland_SevaultCanyon_Entrance_EventScript_845CDA2::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8713, 0, Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843C818, Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843C85B
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_SevenIsland_SevaultCanyon_Entrance_EventScript_845CDC9
	msgbox Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843C863, MSGBOX_AUTOCLOSE
	end

Kanto_SevenIsland_SevaultCanyon_Entrance_EventScript_845CDC9::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8713, 0, Kanto_SevenIsland_SevaultCanyon_Entrance_Text_8471853, Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843C85B
	msgbox Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843C863, MSGBOX_AUTOCLOSE
	end

@ Kanto_SevenIsland_SevaultCanyon_Entrance object 5 (local id 5, gfx 0x3F31) at 10,26
Kanto_SevenIsland_SevaultCanyon_Entrance_EventScript_845CDE0::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8714, 0, Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843C8C2, Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843C8E5
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_SevenIsland_SevaultCanyon_Entrance_EventScript_845CE07
	msgbox Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843C90D, MSGBOX_AUTOCLOSE
	end

Kanto_SevenIsland_SevaultCanyon_Entrance_EventScript_845CE07::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8714, 0, Kanto_SevenIsland_SevaultCanyon_Entrance_Text_8471892, Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843C8E5
	msgbox Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843C90D, MSGBOX_AUTOCLOSE
	end

Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843C6D4::
	.string "Kyahaha!\n"
	.string "I’ll flick you away effortlessly!$"

Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843C6FF::
	.string "Tch!\n"
	.string "Too strong for me!$"

Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843C717::
	.string "What’s that?\n"
	.string "I don’t act the way I look?\p"
	.string "Heh, that’s a part of my strategy!$"

Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843C763::
	.string "Howdy!\n"
	.string "Are you a member of my fan club?$"

Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843C78B::
	.string "Oh, so you’re not a fan…\p"
	.string "Well, I can change that.\n"
	.string "Let me sing for you!$"

Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843C7D2::
	.string "Lalalah…\n"
	.string "My POKéMON, I send them out,\l"
	.string "all the girls scream and shout!$"

Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843C818::
	.string "This island is too spread out…\n"
	.string "It’s not easy patrolling the place.$"

Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843C85B::
	.string "Uh-huh…$"

Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843C863::
	.string "TRAINERS like you from cities,\n"
	.string "they sure are tough.\p"
	.string "You’ll be headed for the TOWER,\n"
	.string "won’t you?$"

Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843C8C2::
	.string "I punish people who abuse\n"
	.string "POKéMON!$"

Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843C8E5::
	.string "You don’t seem to be a problem\n"
	.string "TRAINER.$"

Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843C90D::
	.string "If you treat your POKéMON with\n"
	.string "kindness, they’ll understand.$"

Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843C94A::
	.string "EVE: I’ll team up with JON and\n"
	.string "battle together.$"

Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843C97A::
	.string "EVE: Me and JON, we lost.\n"
	.string "Ehehe.$"

Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843C99B::
	.string "EVE: I’m going to work harder with\n"
	.string "JON.$"

Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843C9C3::
	.string "EVE: I want to team up with JON\n"
	.string "when I battle you.\p"
	.string "Come back with two POKéMON, okay?$"

Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843CA18::
	.string "JON: When I’m with EVE, it feels\n"
	.string "like we could never lose.$"

Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843CA53::
	.string "JON: When I’m with EVE,\n"
	.string "it doesn’t feel like I’ve lost!$"

Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843CA8B::
	.string "JON: When I’m with EVE, I feel\n"
	.string "giddy whether I win or lose.\p"
	.string "It’s magical!$"

Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843CAD5::
	.string "JON: If it’s with EVE, I’ll be\n"
	.string "happy to battle with you.\p"
	.string "So, I’d like to make it a\n"
	.string "two-on-two battle, if I may.$"

Kanto_SevenIsland_SevaultCanyon_Entrance_Text_843CB45::
	.string "SEVAULT CANYON ENTRANCE\p"
	.string "TRAINERS are asked to refrain from\n"
	.string "damaging plants in the CANYON.$"

Kanto_SevenIsland_SevaultCanyon_Entrance_Text_84717CD::
	.string "Kyahaha!\p"
	.string "Coming back won’t change a thing.\n"
	.string "I’ll flick you away effortlessly!$"

Kanto_SevenIsland_SevaultCanyon_Entrance_Text_847181A::
	.string "Howdy! You’re a member of my\n"
	.string "fan club, isn’t that right?$"

Kanto_SevenIsland_SevaultCanyon_Entrance_Text_8471853::
	.string "This island is too spread out…\n"
	.string "Patrolling the place is a drag…$"

Kanto_SevenIsland_SevaultCanyon_Entrance_Text_8471892::
	.string "I don’t forgive people who abuse\n"
	.string "POKéMON!$"

Kanto_SevenIsland_SevaultCanyon_Entrance_Text_84718BC::
	.string "EVE: I’ll team up with JON and\n"
	.string "battle together again.$"

Kanto_SevenIsland_SevaultCanyon_Entrance_Text_84718F2::
	.string "JON: When I’m with EVE, it feels\n"
	.string "like we could never lose, not ever.$"
