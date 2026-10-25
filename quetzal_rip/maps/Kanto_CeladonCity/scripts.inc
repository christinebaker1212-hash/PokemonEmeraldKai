Kanto_CeladonCity_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Quetzal_EventScript_83C3036
	map_script MAP_SCRIPT_ON_RESUME, Quetzal_EventScript_83C3036
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_CeladonCity_EventScript_841693F
	.byte 0

Kanto_CeladonCity_EventScript_841693F::
	special QzKantoTravelingEventCheck_773
	setflag FLAG_QZ_0x2234
	call Quetzal_EventScript_840FBB5
	end

@ Kanto_CeladonCity object 4 (local id 4, gfx 0x3F18) at 11,18
Kanto_CeladonCity_EventScript_841694B::
	msgbox Kanto_CeladonCity_Text_843009F, MSGBOX_NPC
	end

@ Kanto_CeladonCity object 9 (local id 9, gfx 0x3F2C) at 9,30
Kanto_CeladonCity_EventScript_8416954::
	lock
	msgbox Kanto_CeladonCity_Text_84300F8, MSGBOX_DEFAULT
	release
	end

@ Kanto_CeladonCity object 5 (local id 5, gfx 0x3F58) at 18,22
Kanto_CeladonCity_EventScript_841695F::
	msgbox Kanto_CeladonCity_Text_8430126, MSGBOX_NPC
	end

@ Kanto_CeladonCity object 6 (local id 6, gfx 0x3F2C) at 30,24
Kanto_CeladonCity_EventScript_8416968::
	msgbox Kanto_CeladonCity_Text_84301A1, MSGBOX_NPC
	end

@ Kanto_CeladonCity object 7 (local id 7, gfx 0x3F2D) at 26,19
Kanto_CeladonCity_EventScript_8416971::
	goto Kanto_CeladonCity_EventScript_8471C93

@ Kanto_CeladonCity object 2 (local id 2, gfx 0x3F04) at 38,14
Kanto_CeladonCity_EventScript_8416977::
	lock
	faceplayer
	msgbox Kanto_CeladonCity_Text_84301F7, MSGBOX_DEFAULT
	closemessage
	applymovement 2, Quetzal_Movement_8457AA8
	waitmovement 0
	release
	end

@ Kanto_CeladonCity object 3 (local id 3, gfx 0x3F37) at 36,14
Kanto_CeladonCity_EventScript_841698E::
	lock
	faceplayer
	waitse
	playmoncry SPECIES_POLIWRATH, 0
	msgbox Kanto_CeladonCity_Text_8430251, MSGBOX_DEFAULT
	closemessage
	waitmoncry
	applymovement 3, Quetzal_Movement_8457AA8
	waitmovement 0
	release
	end

@ Kanto_CeladonCity object 1 (local id 1, gfx 0x3F3D) at 48,15
Kanto_CeladonCity_EventScript_84169AC::
	msgbox Kanto_CeladonCity_Text_843029E, MSGBOX_NPC
	end

@ Kanto_CeladonCity object 8 (local id 8, gfx 0x3F3D) at 38,31
Kanto_CeladonCity_EventScript_84169B5::
	msgbox Kanto_CeladonCity_Text_8430268, MSGBOX_NPC
	end

@ Kanto_CeladonCity object 12 (local id 12, gfx 0x3EEE) at 25,10
Kanto_CeladonCity_EventScript_84169BE::
	msgbox Kanto_CeladonCity_Text_84304DC, MSGBOX_NPC
	end

@ Kanto_CeladonCity object 15 (local id 15, gfx 0x3F43) at 47,24
Kanto_CeladonCity_EventScript_84169C7::
	msgbox Kanto_CeladonCity_Text_84306BD, MSGBOX_NPC
	end

@ Kanto_CeladonCity sign / bg event at 33,16
Kanto_CeladonCity_EventScript_84169D0::
	msgbox Kanto_CeladonCity_Text_84302BD, MSGBOX_SIGN
	end

@ Kanto_CeladonCity sign / bg event at 22,18
Kanto_CeladonCity_EventScript_84169D9::
	msgbox Kanto_CeladonCity_Text_8430356, MSGBOX_SIGN
	end

@ Kanto_CeladonCity sign / bg event at 16,31
Kanto_CeladonCity_EventScript_84169E2::
	lockall
	setvar VAR_0x8004, 5
	setvar VAR_0x8005, 0
	special QzQuestLogNop_737
	msgbox Kanto_CeladonCity_Text_843037E, MSGBOX_DEFAULT
	releaseall
	end

@ Kanto_CeladonCity sign / bg event at 26,11
Kanto_CeladonCity_EventScript_84169FA::
	msgbox Kanto_CeladonCity_Text_84303C1, MSGBOX_SIGN
	end

@ Kanto_CeladonCity sign / bg event at 18,14
Kanto_CeladonCity_EventScript_8416A03::
	msgbox Kanto_CeladonCity_Text_84303D1, MSGBOX_SIGN
	end

@ Kanto_CeladonCity sign / bg event at 45,23
Kanto_CeladonCity_EventScript_8416A0C::
	msgbox Kanto_CeladonCity_Text_8430400, MSGBOX_SIGN
	end

@ Kanto_CeladonCity sign / bg event at 38,23
Kanto_CeladonCity_EventScript_8416A15::
	msgbox Kanto_CeladonCity_Text_8430480, MSGBOX_SIGN
	end

@ Kanto_CeladonCity sign / bg event at 33,23
Kanto_CeladonCity_EventScript_8416A1E::
	msgbox Kanto_CeladonCity_Text_84304AB, MSGBOX_SIGN
	end

@ Kanto_CeladonCity object 14 (local id 14, gfx 0x3F0E) at 5,3
Kanto_CeladonCity_EventScript_846C0A6::
	setorcopyvar VAR_0x8000, 0x31
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_CeladonCity_EventScript_8471C93::
	msgbox Quetzal_Text_8471CE4, MSGBOX_NPC
	end

Kanto_CeladonCity_Text_843009F::
	.string "I got my KOFFING in CINNABAR.\p"
	.string "It’s usually nice, but it breathes\n"
	.string "poison when it’s angry.$"

Kanto_CeladonCity_Text_84300F8::
	.string "Heheh! This GYM is great!\n"
	.string "It’s full of women!$"

Kanto_CeladonCity_Text_8430126::
	.string "CELADON takes pride in its efforts\n"
	.string "to keep the city beautiful.\p"
	.string "That’s why the new GAME CORNER is\n"
	.string "bad for our city’s image.$"

Kanto_CeladonCity_Text_84301A1::
	.string "Moan…\n"
	.string "I blew it all at the slots!\p"
	.string "I knew I should have cashed in my\n"
	.string "COINS for prizes!$"

Kanto_CeladonCity_Text_84301F7::
	.string "This is my trusted pal, POLIWRATH.\p"
	.string "It evolved from a POLIWHIRL when\n"
	.string "I used a WATER STONE.$"

Kanto_CeladonCity_Text_8430251::
	.string "POLIWRATH: Ribi ribit!$"

Kanto_CeladonCity_Text_8430268::
	.string "What are you staring at?\n"
	.string "Get lost, or I’ll punch you.$"

Kanto_CeladonCity_Text_843029E::
	.string "Keep out of TEAM ROCKET’s way!$"

Kanto_CeladonCity_Text_84302BD::
	.string "TRAINER TIPS\p"
	.string "X ACCURACY boosts the accuracy of\n"
	.string "techniques.\p"
	.string "DIRE HIT jacks up the likelihood\n"
	.string "of critical hits.\p"
	.string "Get your items at the CELADON\n"
	.string "DEPT. STORE!$"

Kanto_CeladonCity_Text_8430356::
	.string "CELADON CITY\n"
	.string "The City of Rainbow Dreams$"

Kanto_CeladonCity_Text_843037E::
	.string "CELADON CITY POKéMON GYM\n"
	.string "LEADER: ERIKA\l"
	.string "The Nature-Loving Princess!$"

Kanto_CeladonCity_Text_84303C1::
	.string "CELADON MANSION$"

Kanto_CeladonCity_Text_84303D1::
	.string "Find what you need at the\n"
	.string "CELADON DEPT. STORE!$"

Kanto_CeladonCity_Text_8430400::
	.string "TRAINER TIPS\p"
	.string "GUARD SPEC. protects POKéMON\n"
	.string "from status-reduction moves during\l"
	.string "battle.\p"
	.string "Get your items at the\n"
	.string "CELADON DEPT. STORE!$"

Kanto_CeladonCity_Text_8430480::
	.string "COINS exchanged for prizes!\n"
	.string "PRIZE EXCHANGE$"

Kanto_CeladonCity_Text_84304AB::
	.string "ROCKET GAME CORNER\n"
	.string "The playground for grown-ups!$"

Kanto_CeladonCity_Text_84304DC::
	.string "Aaaagh, ow…\n"
	.string "I scalded my tongue!\p"
	.string "This nice old lady in the MANSION\n"
	.string "gave me some TEA.\p"
	.string "But it was boiling hot!\n"
	.string "Gotta cool it to drink it.$"

Kanto_CeladonCity_Text_84306BD::
	.string "Oh, what am I to do…\p"
	.string "Someone stole our SILPH SCOPE.\p"
	.string "The thief came running this way,\n"
	.string "I’m sure of it.\p"
	.string "But I lost sight of him!\n"
	.string "Where’d he go?$"
