Kanto_SSAnne_1F_Room6_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_SSAnne_1F_Room6_EventScript_8410976
	map_script MAP_SCRIPT_ON_RESUME, Kanto_SSAnne_1F_Room6_EventScript_8410976
	.byte 0

Kanto_SSAnne_1F_Room6_EventScript_8410976::
	special QzSSAnneRoom6OnResume
	end

@ Kanto_SSAnne_1F_Room6 object 1 (local id 1, gfx 0x3F58) at 3,5
Kanto_SSAnne_1F_Room6_EventScript_841097A::
	lock
	faceplayer
	msgbox Kanto_SSAnne_1F_Room6_Text_8424AD7, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Kanto_SSAnne_1F_Room6_EventScript_84109F0
	closemessage
	special QzSSAnneDreamEventEligibility
	goto_if_eq VAR_RESULT, 1, Kanto_SSAnne_1F_Room6_EventScript_8410A5B
	call Quetzal_EventScript_84570E0
	msgbox Kanto_SSAnne_1F_Room6_Text_8424B0C, MSGBOX_DEFAULT
	goto_if_unset FLAG_QZ_0x214E, Kanto_SSAnne_1F_Room6_EventScript_84109EE
	msgbox Kanto_SSAnne_1F_Room6_Text_8424B4C, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Kanto_SSAnne_1F_Room6_EventScript_84109EE
	closemessage
	goto_if_eq VAR_QZ_0x5135, 1, Kanto_SSAnne_1F_Room6_EventScript_84109E3
	setvar VAR_QZ_0x5145, 0
	warp MAP_KANTO_VERMILION_CITY, 255, 23, 31
	waitstate
	release
	end

Kanto_SSAnne_1F_Room6_EventScript_84109E3::
	warp MAP_KANTO_SSANNE_EXTERIOR, 255, 32, 14
	waitstate
	release
	end

Kanto_SSAnne_1F_Room6_EventScript_84109EE::
	release
	end

Kanto_SSAnne_1F_Room6_EventScript_84109F0::
	checkplayergender
	goto_if_eq VAR_RESULT, 0, Kanto_SSAnne_1F_Room6_EventScript_8410A06
	msgbox Kanto_SSAnne_1F_Room6_Text_8424BCB, MSGBOX_DEFAULT
	release
	end

Kanto_SSAnne_1F_Room6_EventScript_8410A06::
	msgbox Kanto_SSAnne_1F_Room6_Text_8424B86, MSGBOX_DEFAULT
	release
	end

@ Kanto_SSAnne_1F_Room6 sign / bg event at 4,5
@ Kanto_SSAnne_1F_Room6 sign / bg event at 4,6
Kanto_SSAnne_1F_Room6_EventScript_8410A10::
	lockall
	goto_if_set FLAG_QZ_0x292B, Kanto_SSAnne_1F_Room6_EventScript_8410A50
	msgbox Kanto_SSAnne_1F_Room6_Text_8424A1D, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Kanto_SSAnne_1F_Room6_EventScript_8410A58
	closemessage
	special QzSSAnneDreamEventEligibility
	goto_if_eq VAR_RESULT, 1, Kanto_SSAnne_1F_Room6_EventScript_8410A5B
	fadescreen 1
	special QzHealPlayerPartyEx
	fadescreen 0
	msgbox Kanto_SSAnne_1F_Room6_Text_8424A51, MSGBOX_DEFAULT
	goto Kanto_SSAnne_1F_Room6_EventScript_8410A58

Kanto_SSAnne_1F_Room6_EventScript_8410A50::
	msgbox Kanto_SSAnne_1F_Room6_Text_8424A68, MSGBOX_DEFAULT

Kanto_SSAnne_1F_Room6_EventScript_8410A58::
	closemessage
	releaseall
	end

Kanto_SSAnne_1F_Room6_EventScript_8410A5B::
	lockall
	fadescreen 1
	special QzSSAnneDreamEventStart
	warpsilent MAP_KANTO_SSANNE_1F_ROOM6, 255, 4, 4
	waitstate
	end

@ Kanto_SSAnne_1F_Room6 object 2 (local id 2, gfx 0x2EB) at 3,3
Kanto_SSAnne_1F_Room6_EventScript_8410A6B::
	lock
	faceplayer
	special QzKantoStaticEncounterSetupByLocalId
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_840FC18
	call Quetzal_EventScript_840FC01
	goto_if_eq VAR_RESULT, 1, Quetzal_EventScript_840FC13
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, 2, Kanto_SSAnne_1F_Room6_EventScript_8410AA8
	goto_if_eq VAR_RESULT, 3, Kanto_SSAnne_1F_Room6_EventScript_8410AA8
	release
	end

Kanto_SSAnne_1F_Room6_EventScript_8410AA8::
	fadescreen 1
	special QzHealPlayerPartyEx
	special QzSSAnneDreamEventEnd
	warpsilent MAP_KANTO_SSANNE_1F_ROOM6, 255, 5, 5
	waitstate
	end

Kanto_SSAnne_1F_Room6_Text_8424A1D::
	.string "The bed looks comfortable.\n"
	.string "Would you like to sleep?$"

Kanto_SSAnne_1F_Room6_Text_8424A51::
	.string "Your party has rested.$"

Kanto_SSAnne_1F_Room6_Text_8424A68::
	.string "This bed doesn’t feel real...\n"
	.string "The door is still there.$"

Kanto_SSAnne_1F_Room6_Text_8424AD7::
	.string "You look tired. Would you like to\n"
	.string "take a short rest?$"

Kanto_SSAnne_1F_Room6_Text_8424B0C::
	.string "I’m glad to see that everyone is\n"
	.string "looking refreshed and chipper.$"

Kanto_SSAnne_1F_Room6_Text_8424B4C::
	.string "Would you like to leave the ship\n"
	.string "and return to VERMILION?$"

Kanto_SSAnne_1F_Room6_Text_8424B86::
	.string "Oh, I’m sorry for fussing over you.\n"
	.string "You look like my little brother…$"

Kanto_SSAnne_1F_Room6_Text_8424BCB::
	.string "Oh, I’m sorry for fussing over you.\n"
	.string "You look like my little sister…$"
