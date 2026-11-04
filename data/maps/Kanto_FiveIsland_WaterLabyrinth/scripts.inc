Kanto_FiveIsland_WaterLabyrinth_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840FBB9
	.byte 0

@ Kanto_FiveIsland_WaterLabyrinth object 1 (local id 1, gfx 0x3F09) at 14,11
Kanto_FiveIsland_WaterLabyrinth_EventScript_8418552::
	@ Moemon Kai: FireRed's egg gentleman (pokefirered FiveIsland_WaterLabyrinth): a lead POKéMON at full friendship
	@ earns a TOGEPI EGG (FLAG_QZ_0x2175 got it; 0x221E: no room last time), then his DAISY line for a TOGEPI or
	@ TOGETIC in the party; Quetzal's hatched Lv. 5 TOGEPI is gone
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x2175, Kanto_FiveIsland_WaterLabyrinth_EventScript_PostEggComment
	goto_if_set 0x221E, Kanto_FiveIsland_WaterLabyrinth_EventScript_TryGiveEgg
	msgbox Kanto_FiveIsland_WaterLabyrinth_Text_LetMeTakeLookAtMons, MSGBOX_DEFAULT
	specialvar VAR_RESULT, GetLeadMonFriendshipScore
	goto_if_eq VAR_RESULT, 6, Kanto_FiveIsland_WaterLabyrinth_EventScript_LeadMonMaxFriendship
	msgbox Kanto_FiveIsland_WaterLabyrinth_Text_HmmISeeIsee, MSGBOX_DEFAULT
	release
	end

Kanto_FiveIsland_WaterLabyrinth_EventScript_MonDaisyComment::
	msgbox Kanto_FiveIsland_WaterLabyrinth_Text_CuteMonRemindsMeOfDaisy, MSGBOX_DEFAULT
	release
	end

Kanto_FiveIsland_WaterLabyrinth_EventScript_LeadMonMaxFriendship::
	msgbox Kanto_FiveIsland_WaterLabyrinth_Text_TreatMonRightHaveThis, MSGBOX_DEFAULT
	goto Kanto_FiveIsland_WaterLabyrinth_EventScript_TryGiveEgg
	end

Kanto_FiveIsland_WaterLabyrinth_EventScript_TryGiveEgg::
	getpartysize
	goto_if_eq VAR_RESULT, PARTY_SIZE, Kanto_FiveIsland_WaterLabyrinth_EventScript_NoRoomForEgg
	setflag FLAG_QZ_0x2175
	clearflag FLAG_QZ_0x21E2
	giveegg SPECIES_TOGEPI
	playfanfare MUS_LEVEL_UP
	message Kanto_FiveIsland_WaterLabyrinth_Text_ReceivedEggFromMan
	waitfanfare
	waitmessage
	waitbuttonpress
	goto Kanto_FiveIsland_WaterLabyrinth_EventScript_PostEggComment
	end

Kanto_FiveIsland_WaterLabyrinth_EventScript_PostEggComment::
	bufferspeciesname STR_VAR_2, SPECIES_TOGEPI
	setvar VAR_0x8004, SPECIES_TOGEPI
	specialvar VAR_RESULT, QzPartyHasSpecies
	goto_if_ne VAR_RESULT, 0, Kanto_FiveIsland_WaterLabyrinth_EventScript_MonDaisyComment
	bufferspeciesname STR_VAR_2, SPECIES_TOGETIC
	setvar VAR_0x8004, SPECIES_TOGETIC
	specialvar VAR_RESULT, QzPartyHasSpecies
	goto_if_ne VAR_RESULT, 0, Kanto_FiveIsland_WaterLabyrinth_EventScript_MonDaisyComment
	msgbox Kanto_FiveIsland_WaterLabyrinth_Text_GladIMetSomeoneLikeYou, MSGBOX_DEFAULT
	release
	end

Kanto_FiveIsland_WaterLabyrinth_EventScript_NoRoomForEgg::
	setflag 0x221E
	msgbox Kanto_FiveIsland_WaterLabyrinth_Text_DontHaveSpaceInYourParty, MSGBOX_DEFAULT
	release
	end

@ Kanto_FiveIsland_WaterLabyrinth object 2 (local id 2, gfx 0x3F58) at 9,7
Kanto_FiveIsland_WaterLabyrinth_EventScript_845C6AA::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8712, 0, Kanto_FiveIsland_WaterLabyrinth_Text_843ADBE, Kanto_FiveIsland_WaterLabyrinth_Text_843ADF4
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_FiveIsland_WaterLabyrinth_EventScript_845C6D1
	msgbox Kanto_FiveIsland_WaterLabyrinth_Text_843AE20, MSGBOX_AUTOCLOSE
	end

Kanto_FiveIsland_WaterLabyrinth_EventScript_845C6D1::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8712, 0, Kanto_FiveIsland_WaterLabyrinth_Text_847105D, Kanto_FiveIsland_WaterLabyrinth_Text_843ADF4
	msgbox Kanto_FiveIsland_WaterLabyrinth_Text_843AE20, MSGBOX_AUTOCLOSE
	end

Kanto_FiveIsland_WaterLabyrinth_Text_843ADBE::
	.string "This is an excellent environment\n"
	.string "for raising MOéMON.$"

Kanto_FiveIsland_WaterLabyrinth_Text_843ADF4::
	.string "Your MOéMON are growing up\n"
	.string "admirably well!$"

Kanto_FiveIsland_WaterLabyrinth_Text_843AE20::
	.string "You know, I think you have the\n"
	.string "talent to be a good BREEDER.$"

Kanto_FiveIsland_WaterLabyrinth_Text_847105D::
	.string "Oh, hello!\p"
	.string "Are you raising your MOéMON in a\n"
	.string "good environment?$"

Kanto_FiveIsland_WaterLabyrinth_Text_LetMeTakeLookAtMons::
	.string "You travel all over the place,\n"
	.string "don’t you?\p"
	.string "You’re not driving your MOéMON\n"
	.string "too harshly, are you?\p"
	.string "Let me take a look.\n"
	.string "… … …   … … …$"

Kanto_FiveIsland_WaterLabyrinth_Text_HmmISeeIsee::
	.string "Ah, hmm…\n"
	.string "I see, I see…$"

Kanto_FiveIsland_WaterLabyrinth_Text_CuteMonRemindsMeOfDaisy::
	.string "Oh, hello.\n"
	.string "That’s a cute {STR_VAR_2}.\p"
	.string "Seeing it reminds me of a sweet \n"
	.string "little girl I met while traveling.\p"
	.string "She was gently grooming MOéMON…\n"
	.string "She was a little angel.\p"
	.string "That little girl’s name…\n"
	.string "I think it was DAISY.$"

Kanto_FiveIsland_WaterLabyrinth_Text_TreatMonRightHaveThis::
	.string "Oh, impressive.\n"
	.string "You treat your MOéMON right.\p"
	.string "I think you can be entrusted with\n"
	.string "this.\p"
	.string "Please, I’d like you to have this.$"

Kanto_FiveIsland_WaterLabyrinth_Text_ReceivedEggFromMan::
	.string "{PLAYER} received an EGG\n"
	.string "from the man!$"

Kanto_FiveIsland_WaterLabyrinth_Text_GladIMetSomeoneLikeYou::
	.string "I received that EGG while I was\n"
	.string "traveling.\p"
	.string "I’m glad I met someone like you.$"

Kanto_FiveIsland_WaterLabyrinth_Text_DontHaveSpaceInYourParty::
	.string "…Unfortunately, you don’t have\n"
	.string "space for this in your party.\p"
	.string "You’d better come back for it\n"
	.string "another time.$"
