Kanto_SaffronCity_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Quetzal_EventScript_83C3036
	map_script MAP_SCRIPT_ON_RESUME, Quetzal_EventScript_83C3036
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_SaffronCity_EventScript_8417048
	.byte 0

Kanto_SaffronCity_EventScript_8417048::
	special QzKantoTravelingEventCheck
	setflag FLAG_QZ_0x2250
	call_if_unset FLAG_QZ_0x21ED, Kanto_SaffronCity_EventScript_841705D
	call Quetzal_EventScript_840FBB5
	end

Kanto_SaffronCity_EventScript_841705D::
	setobjectxyperm 6, 0x21, 0x1F
	return

@ Kanto_SaffronCity object 1 (local id 1, gfx 0x3F3D) at 22,15
Kanto_SaffronCity_EventScript_8417065::
	msgbox Kanto_SaffronCity_Text_84311DF, MSGBOX_NPC
	end

@ Kanto_SaffronCity object 2 (local id 2, gfx 0x3F3D) at 26,16
Kanto_SaffronCity_EventScript_841706E::
	msgbox Kanto_SaffronCity_Text_84311FB, MSGBOX_NPC
	end

@ Kanto_SaffronCity object 3 (local id 3, gfx 0x3F3D) at 46,13
Kanto_SaffronCity_EventScript_8417077::
	msgbox Kanto_SaffronCity_Text_8431236, MSGBOX_NPC
	end

@ Kanto_SaffronCity object 4 (local id 4, gfx 0x3F3D) at 27,22
Kanto_SaffronCity_EventScript_8417080::
	msgbox Kanto_SaffronCity_Text_8431263, MSGBOX_NPC
	end

@ Kanto_SaffronCity object 5 (local id 5, gfx 0x3F3D) at 25,33
Kanto_SaffronCity_EventScript_8417089::
	msgbox Kanto_SaffronCity_Text_8431283, MSGBOX_NPC
	end

@ Kanto_SaffronCity object 8 (local id 8, gfx 0x3F3D) at 48,24
Kanto_SaffronCity_EventScript_8417092::
	msgbox Kanto_SaffronCity_Text_84312B1, MSGBOX_NPC
	end

@ Kanto_SaffronCity object 7 (local id 7, gfx 0x3F3D) at 30,39
Kanto_SaffronCity_EventScript_841709B::
	msgbox Kanto_SaffronCity_Text_84312D1, MSGBOX_NPC
	end

@ Kanto_SaffronCity object 6 (local id 6, gfx 0x3F3D) at 34,31
Kanto_SaffronCity_EventScript_84170A4::
	lock
	goto_if_set FLAG_QZ_0x21ED, Kanto_SaffronCity_EventScript_84170C2
	applymovement 6, Quetzal_Movement_8457AA4
	waitmovement 0
	msgbox Kanto_SaffronCity_Text_84314A6, MSGBOX_DEFAULT
	release
	end

Kanto_SaffronCity_EventScript_84170C2::
	textcolor 3
	msgbox Kanto_SaffronCity_Text_84314DE, MSGBOX_DEFAULT
	release
	end

@ Kanto_SaffronCity object 10 (local id 10, gfx 0x3F5B) at 21,23
Kanto_SaffronCity_EventScript_84170CE::
	msgbox Kanto_SaffronCity_Text_8431335, MSGBOX_NPC
	end

@ Kanto_SaffronCity object 9 (local id 9, gfx 0x3F5C) at 37,32
Kanto_SaffronCity_EventScript_84170D7::
	msgbox Kanto_SaffronCity_Text_8431365, MSGBOX_NPC
	end

@ Kanto_SaffronCity object 14 (local id 14, gfx 0x3F16) at 32,39
Kanto_SaffronCity_EventScript_84170E0::
	msgbox Kanto_SaffronCity_Text_843139B, MSGBOX_NPC
	end

@ Kanto_SaffronCity object 12 (local id 12, gfx 0x3EEE) at 44,22
Kanto_SaffronCity_EventScript_84170E9::
	msgbox Kanto_SaffronCity_Text_8431401, MSGBOX_NPC
	end

@ Kanto_SaffronCity object 13 (local id 13, gfx 0x3F32) at 45,22
Kanto_SaffronCity_EventScript_84170F2::
	lock
	faceplayer
	waitse
	playmoncry SPECIES_PIDGEOT, 0
	msgbox Kanto_SaffronCity_Text_8431462, MSGBOX_DEFAULT
	waitmoncry
	release
	end

@ Kanto_SaffronCity object 11 (local id 11, gfx 0x3F1E) at 39,16
Kanto_SaffronCity_EventScript_8417105::
	msgbox Kanto_SaffronCity_Text_8431475, MSGBOX_NPC
	end

@ Kanto_SaffronCity sign / bg event at 29,14
Kanto_SaffronCity_EventScript_841710E::
	msgbox Kanto_SaffronCity_Text_8431502, MSGBOX_SIGN
	end

@ Kanto_SaffronCity sign / bg event at 42,14
Kanto_SaffronCity_EventScript_8417117::
	msgbox Kanto_SaffronCity_Text_8431530, MSGBOX_SIGN
	end

@ Kanto_SaffronCity sign / bg event at 50,14
Kanto_SaffronCity_EventScript_8417120::
	lockall
	setvar VAR_0x8004, 7
	setvar VAR_0x8005, 0
	special QzQuestLogNop
	msgbox Kanto_SaffronCity_Text_843153E, MSGBOX_DEFAULT
	releaseall
	end

@ Kanto_SaffronCity sign / bg event at 54,25
Kanto_SaffronCity_EventScript_8417138::
	msgbox Kanto_SaffronCity_Text_8431586, MSGBOX_SIGN
	end

@ Kanto_SaffronCity sign / bg event at 18,30
Kanto_SaffronCity_EventScript_8417141::
	msgbox Kanto_SaffronCity_Text_843162E, MSGBOX_SIGN
	end

@ Kanto_SaffronCity sign / bg event at 29,31
Kanto_SaffronCity_EventScript_841714A::
	msgbox Kanto_SaffronCity_Text_8431695, MSGBOX_SIGN
	end

@ Kanto_SaffronCity sign / bg event at 40,38
Kanto_SaffronCity_EventScript_8417153::
	msgbox Kanto_SaffronCity_Text_84316AF, MSGBOX_SIGN
	end

@ Kanto_SaffronCity sign / bg event at 13,25
Kanto_SaffronCity_EventScript_841715C::
	msgbox Kanto_SaffronCity_Text_84316C3, MSGBOX_SIGN
	end

@ Kanto_SaffronCity sign / bg event at 49,22
Kanto_SaffronCity_EventScript_8417165::
	msgbox Kanto_SaffronCity_Text_84316F5, MSGBOX_SIGN
	end

@ Kanto_SaffronCity object 15 (local id 15, gfx 0x3EE7) at 47,24
Kanto_SaffronCity_EventScript_845D40B::
	lock
	faceplayer
	setvar VAR_0x8004, 0xC
	setvar VAR_0x8005, 2
	special QzQuestLogNop
	msgbox Kanto_SaffronCity_Text_8431748, MSGBOX_DEFAULT
	release
	end

Kanto_SaffronCity_Text_84311DF::
	.string "What do you want?\n"
	.string "Get lost!$"

Kanto_SaffronCity_Text_84311FB::
	.string "BOSS said he’ll take this town\n"
	.string "in the name of TEAM ROCKET!$"

Kanto_SaffronCity_Text_8431236::
	.string "Don’t get defiant!\n"
	.string "Or I’ll have to hurt you!$"

Kanto_SaffronCity_Text_8431263::
	.string "SAFFRON belongs to TEAM ROCKET!$"

Kanto_SaffronCity_Text_8431283::
	.string "My life as a criminal makes me feel\n"
	.string "so alive!$"

Kanto_SaffronCity_Text_84312B1::
	.string "Ow!\n"
	.string "Watch where you’re walking!$"

Kanto_SaffronCity_Text_84312D1::
	.string "With SILPH under control, we can\n"
	.string "exploit POKéMON around the world!\p"
	.string "We’ll get stinking rich, yeahah!$"

Kanto_SaffronCity_Text_8431335::
	.string "You beat TEAM ROCKET all alone?\n"
	.string "That’s amazing!$"

Kanto_SaffronCity_Text_8431365::
	.string "Yeah! TEAM ROCKET is gone!\n"
	.string "It’s safe to go out again!$"

Kanto_SaffronCity_Text_843139B::
	.string "People fled from here in droves\n"
	.string "when those ROCKETS came.\p"
	.string "They should be flocking back to\n"
	.string "SAFFRON now.$"

Kanto_SaffronCity_Text_8431401::
	.string "I flew here on my PIDGEOT when\n"
	.string "I read about SILPH.\p"
	.string "It’s already over?\n"
	.string "I missed the media action…$"

Kanto_SaffronCity_Text_8431462::
	.string "PIDGEOT: Bi bibii!$"

Kanto_SaffronCity_Text_8431475::
	.string "I saw the ROCKET BOSS escaping\n"
	.string "SILPH’s building.$"

Kanto_SaffronCity_Text_84314A6::
	.string "I’m a security guard.\p"
	.string "Suspicious kids I don’t allow in!$"

Kanto_SaffronCity_Text_84314DE::
	.string "…\n"
	.string "Snore…\p"
	.string "Hah! He’s taking a snooze!$"

Kanto_SaffronCity_Text_8431502::
	.string "SAFFRON CITY\n"
	.string "Shining, Golden Land of Commerce$"

Kanto_SaffronCity_Text_8431530::
	.string "FIGHTING DOJO$"

Kanto_SaffronCity_Text_843153E::
	.string "SAFFRON CITY POKéMON GYM\n"
	.string "LEADER: SABRINA\l"
	.string "The Master of PSYCHIC POKéMON!$"

Kanto_SaffronCity_Text_8431586::
	.string "TRAINER TIPS\p"
	.string "FULL HEAL cures all ailments like\n"
	.string "burns, paralysis, poisoning,\l"
	.string "freezing, and sleep.\p"
	.string "It costs a bit more, but it’s more\n"
	.string "convenient than buying other items.$"

Kanto_SaffronCity_Text_843162E::
	.string "TRAINER TIPS\p"
	.string "The new GREAT BALL offers improved\n"
	.string "catching rates.\p"
	.string "Try it on those hard-to-catch\n"
	.string "POKéMON.$"

Kanto_SaffronCity_Text_8431695::
	.string "SILPH CO. OFFICE BUILDING$"

Kanto_SaffronCity_Text_84316AF::
	.string "MR. PSYCHIC’S HOUSE$"

Kanto_SaffronCity_Text_84316C3::
	.string "SILPH’s latest product!\n"
	.string "Release to be determined…$"

Kanto_SaffronCity_Text_84316F5::
	.string "KANTO EXPLORERS’ CLUB\p"
	.string "Sharing tips, discoveries and\n"
	.string "rumors from across our region.$"

Kanto_SaffronCity_Text_8431748::
	.string "This FAN CLUB…\n"
	.string "No one here has a clue!\p"
	.string "How could they not recognize\n"
	.string "the brilliance that is LANCE?\p"
	.string "He stands for justice!\n"
	.string "He’s cool, and yet passionate!\l"
	.string "He’s the greatest, LANCE!$"
