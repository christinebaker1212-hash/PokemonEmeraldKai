Kanto_Route8_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840FBB9
	.byte 0

@ Kanto_Route8 sign / bg event at 16,5
Kanto_Route8_EventScript_8417C8C::
	msgbox Kanto_Route8_Text_843483D, MSGBOX_SIGN
	end

@ Kanto_Route8 object 4 (local id 4, gfx 0x3F16) at 29,3
Kanto_Route8_EventScript_845A4EC::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8320, 0, Kanto_Route8_Text_84344B4, Kanto_Route8_Text_84344D7
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route8_EventScript_845A513
	msgbox Kanto_Route8_Text_84344E2, MSGBOX_AUTOCLOSE
	end

Kanto_Route8_EventScript_845A513::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8320, 0, Kanto_Route8_Text_846ECDA, Kanto_Route8_Text_84344D7
	msgbox Kanto_Route8_Text_84344E2, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route8 object 6 (local id 6, gfx 0x3F16) at 29,5
Kanto_Route8_EventScript_845A52A::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8321, 0, Kanto_Route8_Text_84345E8, Kanto_Route8_Text_843460D
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route8_EventScript_845A551
	msgbox Kanto_Route8_Text_8434617, MSGBOX_AUTOCLOSE
	end

Kanto_Route8_EventScript_845A551::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8321, 0, Kanto_Route8_Text_846ED50, Kanto_Route8_Text_843460D
	msgbox Kanto_Route8_Text_8434617, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route8 object 7 (local id 7, gfx 0x3F16) at 29,6
Kanto_Route8_EventScript_845A568::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8322, 0, Kanto_Route8_Text_8434641, Kanto_Route8_Text_843466D
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route8_EventScript_845A58F
	msgbox Kanto_Route8_Text_8434699, MSGBOX_AUTOCLOSE
	end

Kanto_Route8_EventScript_845A58F::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8322, 0, Kanto_Route8_Text_846ED82, Kanto_Route8_Text_843466D
	msgbox Kanto_Route8_Text_8434699, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route8 object 1 (local id 1, gfx 0x3F16) at 62,14
Kanto_Route8_EventScript_845A5A6::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8323, 0, Kanto_Route8_Text_8434773, Kanto_Route8_Text_843479D
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route8_EventScript_845A5CD
	msgbox Kanto_Route8_Text_84347C4, MSGBOX_AUTOCLOSE
	end

Kanto_Route8_EventScript_845A5CD::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8323, 0, Kanto_Route8_Text_846EDFB, Kanto_Route8_Text_843479D
	msgbox Kanto_Route8_Text_84347C4, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route8 object 9 (local id 9, gfx 0x3F43) at 9,6
Kanto_Route8_EventScript_845A5E4::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8363, 0, Kanto_Route8_Text_843438C, Kanto_Route8_Text_84343C7
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route8_EventScript_845A60B
	msgbox Kanto_Route8_Text_84343D5, MSGBOX_AUTOCLOSE
	end

Kanto_Route8_EventScript_845A60B::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8363, 0, Kanto_Route8_Text_846EC39, Kanto_Route8_Text_84343C7
	msgbox Kanto_Route8_Text_84343D5, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route8 object 3 (local id 3, gfx 0x3F43) at 51,6
Kanto_Route8_EventScript_845A622::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8364, 0, Kanto_Route8_Text_8434443, Kanto_Route8_Text_843447D
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route8_EventScript_845A649
	msgbox Kanto_Route8_Text_843448F, MSGBOX_AUTOCLOSE
	end

Kanto_Route8_EventScript_845A649::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8364, 0, Kanto_Route8_Text_846EC97, Kanto_Route8_Text_843447D
	msgbox Kanto_Route8_Text_843448F, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route8 object 5 (local id 5, gfx 0x3F43) at 29,4
Kanto_Route8_EventScript_845A660::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8365, 0, Kanto_Route8_Text_843451E, Kanto_Route8_Text_8434541
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route8_EventScript_845A687
	msgbox Kanto_Route8_Text_843457A, MSGBOX_AUTOCLOSE
	end

Kanto_Route8_EventScript_845A687::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8365, 0, Kanto_Route8_Text_846ED19, Kanto_Route8_Text_8434541
	msgbox Kanto_Route8_Text_843457A, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route8 object 8 (local id 8, gfx 0x3F2C) at 21,3
Kanto_Route8_EventScript_845A69E::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8454, 0, Kanto_Route8_Text_84343F6, Kanto_Route8_Text_8434414
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route8_EventScript_845A6C5
	msgbox Kanto_Route8_Text_8434429, MSGBOX_AUTOCLOSE
	end

Kanto_Route8_EventScript_845A6C5::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8454, 0, Kanto_Route8_Text_846EC73, Kanto_Route8_Text_8434414
	msgbox Kanto_Route8_Text_8434429, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route8 object 2 (local id 2, gfx 0x3F2C) at 55,15
Kanto_Route8_EventScript_845A6DC::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8456, 0, Kanto_Route8_Text_84346F9, Kanto_Route8_Text_8434716
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route8_EventScript_845A703
	msgbox Kanto_Route8_Text_843472D, MSGBOX_AUTOCLOSE
	end

Kanto_Route8_EventScript_845A703::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8456, 0, Kanto_Route8_Text_846EDC5, Kanto_Route8_Text_8434716
	msgbox Kanto_Route8_Text_843472D, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route8 object 12 (local id 12, gfx 0x3F18) at 40,3
Kanto_Route8_EventScript_845A71A::
	trainerbattle TRAINER_BATTLE_DOUBLE, TRAINER_QZ_8676, 0, Kanto_Route8_Text_843486B, Kanto_Route8_Text_8434897, Kanto_Route8_Text_84348DD
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route8_EventScript_845A745
	msgbox Kanto_Route8_Text_84348B9, MSGBOX_AUTOCLOSE
	end

Kanto_Route8_EventScript_845A745::
	trainerbattle TRAINER_BATTLE_REMATCH_DOUBLE, TRAINER_QZ_8676, 0, Kanto_Route8_Text_846EE8B, Kanto_Route8_Text_8434897, Kanto_Route8_Text_84348DD
	msgbox Kanto_Route8_Text_84348B9, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route8 object 13 (local id 13, gfx 0x3F18) at 41,3
Kanto_Route8_EventScript_845A760::
	trainerbattle TRAINER_BATTLE_DOUBLE, TRAINER_QZ_8676, 0, Kanto_Route8_Text_8434911, Kanto_Route8_Text_843493C, Kanto_Route8_Text_8434976
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route8_EventScript_845A78B
	msgbox Kanto_Route8_Text_8434952, MSGBOX_AUTOCLOSE
	end

Kanto_Route8_EventScript_845A78B::
	trainerbattle TRAINER_BATTLE_REMATCH_DOUBLE, TRAINER_QZ_8676, 0, Kanto_Route8_Text_846EEB3, Kanto_Route8_Text_843493C, Kanto_Route8_Text_8434976
	msgbox Kanto_Route8_Text_8434952, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route8 object 14 (local id 14, gfx 0x3EE9) at 14,16
Kanto_Route8_EventScript_845A7A6::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8727, 0, Kanto_Route8_Text_84349A9, Kanto_Route8_Text_84349C3
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route8_EventScript_845A7CD
	msgbox Kanto_Route8_Text_84349E0, MSGBOX_AUTOCLOSE
	end

Kanto_Route8_EventScript_845A7CD::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8727, 0, Kanto_Route8_Text_846EE42, Kanto_Route8_Text_84349C3
	msgbox Kanto_Route8_Text_84349E0, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route8 object 15 (local id 15, gfx 0x3EE9) at 24,17
Kanto_Route8_EventScript_845A7E4::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8728, 0, Kanto_Route8_Text_8434A09, Kanto_Route8_Text_8434A2E
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route8_EventScript_845A80B
	msgbox Kanto_Route8_Text_8434A41, MSGBOX_AUTOCLOSE
	end

Kanto_Route8_EventScript_845A80B::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8728, 0, Kanto_Route8_Text_846EE62, Kanto_Route8_Text_8434A2E
	msgbox Kanto_Route8_Text_8434A41, MSGBOX_AUTOCLOSE
	end

Kanto_Route8_Text_843438C::
	.string "You look good at POKéMON, but how\n"
	.string "is your chemistry grade?$"

Kanto_Route8_Text_84343C7::
	.string "Ow!\n"
	.string "Meltdown!$"

Kanto_Route8_Text_84343D5::
	.string "I am better at school than this!$"

Kanto_Route8_Text_84343F6::
	.string "All right!\n"
	.string "Let’s play a game!$"

Kanto_Route8_Text_8434414::
	.string "Drat!\n"
	.string "Came up short!$"

Kanto_Route8_Text_8434429::
	.string "Things didn’t work today.$"

Kanto_Route8_Text_8434443::
	.string "You should be informed.\n"
	.string "You need strategy to win at this!$"

Kanto_Route8_Text_843447D::
	.string "It’s not logical!$"

Kanto_Route8_Text_843448F::
	.string "Go with GRIMER first…and…\n"
	.string "…and…then…$"

Kanto_Route8_Text_84344B4::
	.string "I like NIDORAN, so I collect them!$"

Kanto_Route8_Text_84344D7::
	.string "Why?\n"
	.string "Why??$"

Kanto_Route8_Text_84344E2::
	.string "When POKéMON grow up, they get\n"
	.string "ugly! They shouldn’t evolve.$"

Kanto_Route8_Text_843451E::
	.string "School is fun, but so are POKéMON.$"

Kanto_Route8_Text_8434541::
	.string "You made it clear to me that\n"
	.string "I should stick with school.$"

Kanto_Route8_Text_843457A::
	.string "I was planning on getting some\n"
	.string "fresh air for a change, but…\p"
	.string "We’re stuck here because of the\n"
	.string "gates at SAFFRON.$"

Kanto_Route8_Text_84345E8::
	.string "MEOWTH is so cute, meow, meow,\n"
	.string "meow!$"

Kanto_Route8_Text_843460D::
	.string "Meeeeyow!$"

Kanto_Route8_Text_8434617::
	.string "I think PIDGEY and RATTATA are\n"
	.string "cute, too!$"

Kanto_Route8_Text_8434641::
	.string "We must look silly standing here\n"
	.string "like this.$"

Kanto_Route8_Text_843466D::
	.string "Look what you did to my poor, cute\n"
	.string "POKéMON!$"

Kanto_Route8_Text_8434699::
	.string "SAFFRON’s gatekeeper won’t let us\n"
	.string "go through.\p"
	.string "I know he’s doing his job, but I\n"
	.string "think he’s mean.$"

Kanto_Route8_Text_84346F9::
	.string "I’m a rambling, gaming dude!$"

Kanto_Route8_Text_8434716::
	.string "Missed the big chance!$"

Kanto_Route8_Text_843472D::
	.string "Playing games and POKéMON are like\n"
	.string "eating peanuts! I just can’t stop!$"

Kanto_Route8_Text_8434773::
	.string "What’s a cute, round, and fluffy\n"
	.string "POKéMON?$"

Kanto_Route8_Text_843479D::
	.string "Stop!\p"
	.string "Don’t be so mean to my CLEFAIRY!$"

Kanto_Route8_Text_84347C4::
	.string "I read that CLEFAIRY evolves when\n"
	.string "it’s exposed to a MOON STONE.\p"
	.string "I read that in a POKéMON JOURNAL.\n"
	.string "I wonder if it’s true?$"

Kanto_Route8_Text_843483D::
	.string "UNDERGROUND PATH\n"
	.string "CELADON CITY - LAVENDER TOWN$"

Kanto_Route8_Text_843486B::
	.string "ELI: Twin power is fantastic.\n"
	.string "Did you know?$"

Kanto_Route8_Text_8434897::
	.string "ELI: But…\n"
	.string "We used our twin power…$"

Kanto_Route8_Text_84348B9::
	.string "ELI: I caught my POKéMON with\n"
	.string "ANNE!$"

Kanto_Route8_Text_84348DD::
	.string "ELI: We can’t battle if you don’t\n"
	.string "have two POKéMON.$"

Kanto_Route8_Text_8434911::
	.string "ANNE: We’ll shock you with our twin\n"
	.string "power!$"

Kanto_Route8_Text_843493C::
	.string "ANNE: Our twin power…$"

Kanto_Route8_Text_8434952::
	.string "ANNE: I’m raising POKéMON with\n"
	.string "ELI.$"

Kanto_Route8_Text_8434976::
	.string "ANNE: Hi, hi! Let’s battle!\n"
	.string "But bring two POKéMON.$"

Kanto_Route8_Text_84349A9::
	.string "My bike’s acting up, man.$"

Kanto_Route8_Text_84349C3::
	.string "Aww, man.\n"
	.string "I’m not into this.$"

Kanto_Route8_Text_84349E0::
	.string "I got grass caught up in my\n"
	.string "spokes, man.$"

Kanto_Route8_Text_8434A09::
	.string "Clear the way, or I’ll run you\n"
	.string "down!$"

Kanto_Route8_Text_8434A2E::
	.string "You for real, kid?$"

Kanto_Route8_Text_8434A41::
	.string "Don’t think you’re all special and\n"
	.string "all just because of this.$"

Kanto_Route8_Text_846EC39::
	.string "You are good at POKéMON, but how\n"
	.string "is your chemistry grade?$"

Kanto_Route8_Text_846EC73::
	.string "All right!\n"
	.string "Let’s play another game!$"

Kanto_Route8_Text_846EC97::
	.string "You need strategy to win at\n"
	.string "battling.\p"
	.string "Are you following my advice?$"

Kanto_Route8_Text_846ECDA::
	.string "I’ve collected many NIDORAN.\n"
	.string "I don’t want them to evolve, but…$"

Kanto_Route8_Text_846ED19::
	.string "School is fun, but I still think\n"
	.string "POKéMON are fun, too.$"

Kanto_Route8_Text_846ED50::
	.string "MEOWTH and PERSIAN are so cute,\n"
	.string "meow, meow, meow!$"

Kanto_Route8_Text_846ED82::
	.string "We may look silly standing here\n"
	.string "like this, but I can still battle.$"

Kanto_Route8_Text_846EDC5::
	.string "I’m a rambling, gaming dude!\n"
	.string "I’m on a winning streak!$"

Kanto_Route8_Text_846EDFB::
	.string "What’s a cute, round, and fluffy\n"
	.string "POKéMON?\p"
	.string "You already know, don’t you?$"

Kanto_Route8_Text_846EE42::
	.string "My bike’s still acting up, man.$"

Kanto_Route8_Text_846EE62::
	.string "Okay, kid!\n"
	.string "Don’t expect mercy this time!$"

Kanto_Route8_Text_846EE8B::
	.string "ELI: Our twin power became even\n"
	.string "better!$"

Kanto_Route8_Text_846EEB3::
	.string "ANNE: Our twin power powered up!$"
