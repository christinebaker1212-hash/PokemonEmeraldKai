Johto_BillsFamilysHouse_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_BillsFamilysHouse_EventScript_83D3112
	map_script MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE, Johto_BillsFamilysHouse_OnWarp_83D3104
	.byte 0

Johto_BillsFamilysHouse_OnWarp_83D3104:
	map_script_2 VAR_QZ_0x5077, 6, Johto_BillsFamilysHouse_EventScript_83D310E
	.2byte 0

Johto_BillsFamilysHouse_EventScript_83D310E::
	special QzTimeTravelHouseFrameCheck
	end

Johto_BillsFamilysHouse_EventScript_83D3112::
	special QzTimeTravelHouseProgress
	end

@ Johto_BillsFamilysHouse object 3 (local id 3, gfx 0x6) at 8,7
Johto_BillsFamilysHouse_EventScript_83D3116::
	lock
	faceplayer
	msgbox Johto_BillsFamilysHouse_Text_83D34D3, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_BillsFamilysHouse object 2 (local id 2, gfx 0x18) at 8,4
Johto_BillsFamilysHouse_EventScript_83D3123::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x11CB, Johto_BillsFamilysHouse_EventScript_83D3139
	msgbox Johto_BillsFamilysHouse_Text_83D33D6, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_BillsFamilysHouse_EventScript_83D3139::
	msgbox Johto_BillsFamilysHouse_Text_83D3481, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_BillsFamilysHouse object 1 (local id 1, gfx 0x3E86) at 5,4
Johto_BillsFamilysHouse_EventScript_83D3144::
	lock
	special Script_ClearHeldMovement
	faceplayer
	special QzTimeTravelHouseStatus
	goto_if_eq VAR_RESULT, 0, Johto_BillsFamilysHouse_EventScript_83D3189
	goto_if_set FLAG_QZ_0x103B, Johto_BillsFamilysHouse_EventScript_8407138
	msgbox Johto_BillsFamilysHouse_Text_84076F4, MSGBOX_DEFAULT
	multichoice 0x14, 0, MULTI_QZ_0x96, 1
	nop
	goto_if_eq VAR_RESULT, 0, Johto_BillsFamilysHouse_EventScript_8407138
	goto_if_eq VAR_RESULT, 1, Johto_BillsFamilysHouse_EventScript_83D319D
	goto Johto_BillsFamilysHouse_EventScript_84073E3

Johto_BillsFamilysHouse_EventScript_83D3189::
	goto_if_unset FLAG_QZ_0x103B, Johto_BillsFamilysHouse_EventScript_83D319D
	msgbox Johto_BillsFamilysHouse_Text_83D336F, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_BillsFamilysHouse_EventScript_83D319D::
	call Quetzal_EventScript_8361A4D
	msgbox Johto_BillsFamilysHouse_Text_83D31E7, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Johto_BillsFamilysHouse_EventScript_83D31DC
	msgbox Johto_BillsFamilysHouse_Text_83D3251, MSGBOX_DEFAULT
	setvar VAR_TEMP_1, 0x1DF
	bufferspeciesname 0, SPECIES_ROTOM
	qz_givestorymon 3
	call Quetzal_EventScript_840181E
	setflag FLAG_QZ_0x103B
	msgbox Johto_BillsFamilysHouse_Text_83D32D5, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_BillsFamilysHouse_EventScript_83D31DC::
	msgbox Johto_BillsFamilysHouse_Text_83D335B, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_BillsFamilysHouse_EventScript_8406DF8::
	lockall
	special QzTimeTravelEventEligibility
	goto_if_eq VAR_RESULT, 0, Johto_BillsFamilysHouse_EventScript_8406EB7
	goto_if_eq VAR_QZ_0x5077, 6, Johto_BillsFamilysHouse_EventScript_8406E40
	msgbox Johto_BillsFamilysHouse_Text_8406F28, MSGBOX_YESNO

Johto_BillsFamilysHouse_EventScript_8406E1A::
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_8406ED7
	closemessage
	fadescreen 3
	special QzTimeTravelEventStart
	goto_if_eq VAR_RESULT, 0, Johto_BillsFamilysHouse_EventScript_8406EB5
	warp MAP_JOHTO_ILEX_FOREST, 255, 32, 37
	waitstate
	end

Johto_BillsFamilysHouse_EventScript_8406E40::
	msgbox Johto_BillsFamilysHouse_Text_840828D, MSGBOX_YESNO
	goto Johto_BillsFamilysHouse_EventScript_8406E1A

Johto_BillsFamilysHouse_EventScript_8406EB5::
	fadescreen 2

Johto_BillsFamilysHouse_EventScript_8406EB7::
	goto_if_eq VAR_QZ_0x5077, 6, Johto_BillsFamilysHouse_EventScript_8406ECF
	msgbox Johto_BillsFamilysHouse_Text_8406F67, MSGBOX_DEFAULT
	goto Quetzal_EventScript_8406ED7

Johto_BillsFamilysHouse_EventScript_8406ECF::
	msgbox Johto_BillsFamilysHouse_Text_8408200, MSGBOX_DEFAULT
	goto Quetzal_EventScript_8406ED7  @ falls through in the ROM

Johto_BillsFamilysHouse_EventScript_8407138::
	lockall
	special QzTimeTravelHouseStatus
	goto_if_eq VAR_RESULT, 5, Johto_BillsFamilysHouse_EventScript_8407178
	goto_if_eq VAR_RESULT, 1, Johto_BillsFamilysHouse_EventScript_8407185
	goto_if_eq VAR_RESULT, 2, Johto_BillsFamilysHouse_EventScript_840721B
	goto_if_eq VAR_RESULT, 3, Johto_BillsFamilysHouse_EventScript_840727F
	goto_if_eq VAR_RESULT, 4, Johto_BillsFamilysHouse_EventScript_84072E0
	goto Johto_BillsFamilysHouse_EventScript_84073E3

Johto_BillsFamilysHouse_EventScript_8407178::
	msgbox Johto_BillsFamilysHouse_Text_8407D70, MSGBOX_DEFAULT
	goto Johto_BillsFamilysHouse_EventScript_84073E3

Johto_BillsFamilysHouse_EventScript_8407185::
	call Quetzal_EventScript_84070F0
	msgbox Johto_BillsFamilysHouse_Text_840771D, MSGBOX_YESNO
	goto_if_ne VAR_RESULT, 1, Johto_BillsFamilysHouse_EventScript_84073E3
	closemessage
	special ChoosePartyMon
	waitstate
	lockall
	special Script_ClearHeldMovement
	faceplayer
	special QzTimeTravelChoosePartyMon
	goto_if_eq VAR_0x8004, 255, Johto_BillsFamilysHouse_EventScript_84073E3
	goto_if_ne VAR_RESULT, 1, Johto_BillsFamilysHouse_EventScript_840722A
	bufferpartymonnick 0, VAR_0x8004
	msgbox Johto_BillsFamilysHouse_Text_8407794, MSGBOX_YESNO
	goto_if_ne VAR_RESULT, 1, Johto_BillsFamilysHouse_EventScript_84073E3
	closemessage
	fadescreen 1
	special QzTimeTravelHandOverPartyMon
	goto_if_ne VAR_RESULT, 1, Johto_BillsFamilysHouse_EventScript_8407228
	addobject 4
	special Script_ClearHeldMovement
	turnobject 1, 3
	fadescreen 0
	msgbox Johto_BillsFamilysHouse_Text_840781D, MSGBOX_DEFAULT
	playse SE_PC_ON
	msgbox Johto_BillsFamilysHouse_Text_8407861, MSGBOX_DEFAULT
	closemessage
	turnobject 4, 2
	playmoncry SPECIES_SKARMORY, 0
	waitmoncry
	msgbox Johto_BillsFamilysHouse_Text_84078E2, MSGBOX_DEFAULT
	faceplayer

Johto_BillsFamilysHouse_EventScript_840721B::
	msgbox Johto_BillsFamilysHouse_Text_8407958, MSGBOX_DEFAULT
	goto Johto_BillsFamilysHouse_EventScript_84073E3

Johto_BillsFamilysHouse_EventScript_8407228::
	fadescreen 0

Johto_BillsFamilysHouse_EventScript_840722A::
	goto_if_eq VAR_RESULT, 2, Johto_BillsFamilysHouse_EventScript_8407258
	goto_if_eq VAR_RESULT, 3, Johto_BillsFamilysHouse_EventScript_8407265
	goto_if_eq VAR_RESULT, 4, Johto_BillsFamilysHouse_EventScript_8407272
	msgbox Johto_BillsFamilysHouse_Text_8407996, MSGBOX_DEFAULT
	goto Johto_BillsFamilysHouse_EventScript_84073E3

Johto_BillsFamilysHouse_EventScript_8407258::
	msgbox Johto_BillsFamilysHouse_Text_84079E5, MSGBOX_DEFAULT
	goto Johto_BillsFamilysHouse_EventScript_84073E3

Johto_BillsFamilysHouse_EventScript_8407265::
	msgbox Johto_BillsFamilysHouse_Text_8407A29, MSGBOX_DEFAULT
	goto Johto_BillsFamilysHouse_EventScript_84073E3

Johto_BillsFamilysHouse_EventScript_8407272::
	msgbox Johto_BillsFamilysHouse_Text_8407A93, MSGBOX_DEFAULT
	goto Johto_BillsFamilysHouse_EventScript_84073E3

Johto_BillsFamilysHouse_EventScript_840727F::
	goto_if_ge VAR_QZ_0x5077, 2, Johto_BillsFamilysHouse_EventScript_84072BD
	msgbox Johto_BillsFamilysHouse_Text_8407ACC, MSGBOX_DEFAULT
	msgbox Johto_BillsFamilysHouse_Text_8407B50, MSGBOX_DEFAULT
	msgbox Johto_BillsFamilysHouse_Text_8407C23, MSGBOX_DEFAULT
	special QzTimeTravelEventEligibility
	goto_if_eq VAR_RESULT, 0, Johto_BillsFamilysHouse_EventScript_84072D3
	msgbox Johto_BillsFamilysHouse_Text_8407DC4, MSGBOX_DEFAULT
	goto Johto_BillsFamilysHouse_EventScript_84073DB

Johto_BillsFamilysHouse_EventScript_84072BD::
	msgbox Johto_BillsFamilysHouse_Text_8407E53, MSGBOX_DEFAULT
	special QzTimeTravelEventEligibility
	goto_if_eq VAR_RESULT, 1, Johto_BillsFamilysHouse_EventScript_84073DB

Johto_BillsFamilysHouse_EventScript_84072D3::
	msgbox Johto_BillsFamilysHouse_Text_8407CB2, MSGBOX_DEFAULT
	goto Johto_BillsFamilysHouse_EventScript_84073E3

Johto_BillsFamilysHouse_EventScript_84072E0::
	goto_if_set FLAG_QZ_0x14B5, Johto_BillsFamilysHouse_EventScript_8407387
	call Johto_BillsFamilysHouse_EventScript_840730C
	goto Johto_BillsFamilysHouse_EventScript_840738F

Johto_BillsFamilysHouse_EventScript_840730C::
	goto_if_set FLAG_QZ_0x14B5, Johto_BillsFamilysHouse_EventScript_8407386
	setvar VAR_0x8004, 1
	setvar VAR_0x8005, 0xFF
	special QzObjectFaceObject
	goto_if_unset FLAG_QZ_0x14B3, Johto_BillsFamilysHouse_EventScript_8407333
	turnobject 1, 3
	turnobject 4, 4

Johto_BillsFamilysHouse_EventScript_8407333::
	msgbox Johto_BillsFamilysHouse_Text_8407ED2, MSGBOX_DEFAULT
	playse SE_PC_OFF
	goto_if_unset FLAG_QZ_0x14B3, Johto_BillsFamilysHouse_EventScript_8407366
	closemessage
	msgbox Johto_BillsFamilysHouse_Text_8407F83, MSGBOX_DEFAULT
	closemessage
	delay 0x18
	playmoncry SPECIES_SKARMORY, 0
	waitmoncry
	turnobject 4, 2
	msgbox Johto_BillsFamilysHouse_Text_8407FB5, MSGBOX_DEFAULT

Johto_BillsFamilysHouse_EventScript_8407366::
	setvar VAR_0x8004, 1
	setvar VAR_0x8005, 0xFF
	special QzObjectFaceObject
	msgbox Johto_BillsFamilysHouse_Text_8407FEA, MSGBOX_DEFAULT
	msgbox Johto_BillsFamilysHouse_Text_8408146, MSGBOX_DEFAULT
	setflag FLAG_QZ_0x14B5

Johto_BillsFamilysHouse_EventScript_8407386::
	return

Johto_BillsFamilysHouse_EventScript_8407387::
	msgbox Johto_BillsFamilysHouse_Text_8408146, MSGBOX_DEFAULT

Johto_BillsFamilysHouse_EventScript_840738F::
	special QzTimeTravelEventEligibility
	goto_if_eq VAR_RESULT, 1, Johto_BillsFamilysHouse_EventScript_84073DB
	msgbox Johto_BillsFamilysHouse_Text_8408200, MSGBOX_DEFAULT
	goto Johto_BillsFamilysHouse_EventScript_84073E3

@ Johto_BillsFamilysHouse object 4 (local id 4, gfx 0xC59) at 3,4
Johto_BillsFamilysHouse_EventScript_84073AA::
	lock
	special QzTimeTravelHouseStatus
	goto_if_eq VAR_RESULT, 0, Johto_BillsFamilysHouse_EventScript_84073E3
	goto_if_eq VAR_RESULT, 5, Johto_BillsFamilysHouse_EventScript_84073E3
	special Script_ClearHeldMovement
	faceplayer
	playmoncry SPECIES_SKARMORY, 0
	waitmoncry
	msgbox Johto_BillsFamilysHouse_Text_8408256, MSGBOX_DEFAULT
	goto Johto_BillsFamilysHouse_EventScript_84073E3

Johto_BillsFamilysHouse_EventScript_84073DB::
	special Script_ClearHeldMovement
	goto Johto_BillsFamilysHouse_EventScript_8406DF8

Johto_BillsFamilysHouse_EventScript_84073E3::
	special Script_ClearHeldMovement
	release
	end

Johto_BillsFamilysHouse_Text_83D31E7::
	.string "BILL: Hi, {PLAYER}! This ROTOM\n"
	.string "moved into my computer.\p"
	.string "It’s curious about the world.\n"
	.string "Will you take it with you?$"

Johto_BillsFamilysHouse_Text_83D3251::
	.string "BILL: I knew you’d come through!\p"
	.string "Way to go! You’re the real deal!\p"
	.string "OK, I’m counting on you.\p"
	.string "Take good care of it!$"

Johto_BillsFamilysHouse_Text_83D32D5::
	.string "BILL: ROTOM can enter appliances\n"
	.string "and change its form.\p"
	.string "You’ll have lots of fun together!$"

Johto_BillsFamilysHouse_Text_83D335B::
	.string "Oh… Now what to do?$"

Johto_BillsFamilysHouse_Text_83D336F::
	.string "BILL: My pop, he won’t work. All he\n"
	.string "does is goof off all day long.\p"
	.string "He’s getting to be a real headache…$"

Johto_BillsFamilysHouse_Text_83D33D6::
	.string "Oh, you collect POKéMON? My son\n"
	.string "BILL is an expert.\p"
	.string "He just got called to the POKéMON\n"
	.string "CENTER in ECRUTEAK CITY.\p"
	.string "My husband went off to the GAME\n"
	.string "CORNER without being called…$"

Johto_BillsFamilysHouse_Text_83D3481::
	.string "My husband was once known as a\n"
	.string "POKéMANIAC.\p"
	.string "BILL must have taken after his\n"
	.string "father.$"

Johto_BillsFamilysHouse_Text_83D34D3::
	.string "My big brother BILL made the PC\n"
	.string "POKéMON storage system.$"

Johto_BillsFamilysHouse_Text_8406F28::
	.string "Travel with BILL to that\n"
	.string "possible future?\p"
	.string "Use START to return.$"

Johto_BillsFamilysHouse_Text_8406F67::
	.string "Speak to BILL at his Goldenrod\n"
	.string "home after his research.\p"
	.string "Use CELEBI as your\n"
	.string "playable character.$"

Johto_BillsFamilysHouse_Text_84076F4::
	.string "BILL: What would you like\n"
	.string "to talk about?$"

Johto_BillsFamilysHouse_Text_840771D::
	.string "Magearna can sense emotions.\n"
	.string "I want to learn how it does that.\p"
	.string "Would you choose an original-color\n"
	.string "Magearna to give me?$"

Johto_BillsFamilysHouse_Text_8407794::
	.string "Give {STR_VAR_1} to BILL?\p"
	.string "It will stay with me. You won’t\n"
	.string "be able to take this Pokémon back.\p"
	.string "Any item it is holding will\n"
	.string "be returned to your Bag.$"

Johto_BillsFamilysHouse_Text_840781D::
	.string "BILL: Thank you! I’ll care for it.\n"
	.string "I had this device ready already.$"

Johto_BillsFamilysHouse_Text_8407861::
	.string "BILL: The device tries to translate\n"
	.string "the signals from its heart.\p"
	.string "Look: it says Magearna is happy.\n"
	.string "I think it’s enjoying the test!$"

Johto_BillsFamilysHouse_Text_84078E2::
	.string "BILL: You don’t want to continue?\n"
	.string "But the reading was so clear…\p"
	.string "It must need more precision.\n"
	.string "Get some rest, Magearna.$"

Johto_BillsFamilysHouse_Text_8407958::
	.string "BILL: Let me study the data.\n"
	.string "Take a walk and come back later.$"

Johto_BillsFamilysHouse_Text_8407996::
	.string "I need an original-color\n"
	.string "Magearna.\p"
	.string "First, resolve its encounter\n"
	.string "here in Johto.$"

Johto_BillsFamilysHouse_Text_84079E5::
	.string "You must have another Pokémon\n"
	.string "able to battle before giving it away.$"

Johto_BillsFamilysHouse_Text_8407A29::
	.string "There is no room for its held item.\n"
	.string "Make room in your Bag and try again.\p"
	.string "Magearna is still in your party.$"

Johto_BillsFamilysHouse_Text_8407A93::
	.string "I can’t accept this Pokémon\n"
	.string "with the item it is holding.$"

Johto_BillsFamilysHouse_Text_8407ACC::
	.string "BILL: Understanding Pokémon could\n"
	.string "prevent misunderstandings.\p"
	.string "They could tell us what they need.\n"
	.string "We could live together more easily.$"

Johto_BillsFamilysHouse_Text_8407B50::
	.string "The device will take years to finish.\n"
	.string "But perhaps we don’t have to wait!\p"
	.string "We could visit a future where humans\n"
	.string "and Pokémon can talk to each other.\p"
	.string "If we bring back that technology,\n"
	.string "we could skip all these tests.$"

Johto_BillsFamilysHouse_Text_8407C23::
	.string "It would be an alternate future:\n"
	.string "a possibility, not a certainty.\p"
	.string "We don’t know what that world is like.\n"
	.string "But I want to learn from its advances.$"

Johto_BillsFamilysHouse_Text_8407CB2::
	.string "The guardian of Ilex Forest can\n"
	.string "travel through time: Celebi.\p"
	.string "Return to this house using Celebi\n"
	.string "as your playable character.\p"
	.string "Having it in your party isn’t enough.\n"
	.string "It must open the way itself.$"

Johto_BillsFamilysHouse_Text_8407D70::
	.string "BILL: To travel to the future,\n"
	.string "disconnect from multiplayer.\p"
	.string "Then speak to me again.$"

Johto_BillsFamilysHouse_Text_8407DC4::
	.string "BILL: Celebi! Will you help us?\p"
	.string "Take us to a possible future where\n"
	.string "we can talk to Pokémon.\p"
	.string "We’ll open the passage from here.\n"
	.string "I’ll go with you.$"

Johto_BillsFamilysHouse_Text_8407E53::
	.string "BILL: The link to that future\n"
	.string "is still open from this house.\p"
	.string "Celebi can take us there again.\n"
	.string "We’ll carry on where we left off.$"

Johto_BillsFamilysHouse_Text_8407ED2::
	.string "BILL: Magearna… The device\n"
	.string "said you were happy.\p"
	.string "You wanted to stop.\n"
	.string "I should have trusted you first.\p"
	.string "I am turning this prototype off.\n"
	.string "I won’t bring back one I don’t\l"
	.string "understand.$"

Johto_BillsFamilysHouse_Text_8407F83::
	.string "BILL: Would you like to\n"
	.string "keep researching with me?$"

Johto_BillsFamilysHouse_Text_8407FB5::
	.string "BILL: All right. Not today.\n"
	.string "Get some rest, Magearna.$"

Johto_BillsFamilysHouse_Text_8407FEA::
	.string "BILL: I’ll share the medical research\n"
	.string "with doctors and other researchers.\p"
	.string "Some advances need machines\n"
	.string "we don’t have yet.\p"
	.string "There’s a lot of work ahead…\n"
	.string "but now we know where to start.\p"
	.string "I thought technology from the future\n"
	.string "would save me having to understand it.\p"
	.string "Now we have their research.\p"
	.string "We need to understand it, test it,\n"
	.string "and keep working together.$"

Johto_BillsFamilysHouse_Text_8408146::
	.string "BILL: I will keep trying\n"
	.string "to understand Pokémon.\p"
	.string "Without skipping the tests\n"
	.string "or deciding their answers.\p"
	.string "I won’t return to that future.\p"
	.string "With Celebi in your party,\n"
	.string "you can visit without me.$"

Johto_BillsFamilysHouse_Text_8408200::
	.string "Keep Celebi in your party\n"
	.string "to revisit that future.\p"
	.string "You can use any\n"
	.string "playable character.$"

Johto_BillsFamilysHouse_Text_8408256::
	.string "Magearna answers with the soft\n"
	.string "sound of turning gears.$"

Johto_BillsFamilysHouse_Text_840828D::
	.string "Return to that future without BILL?\p"
	.string "Use START to come back.$"
