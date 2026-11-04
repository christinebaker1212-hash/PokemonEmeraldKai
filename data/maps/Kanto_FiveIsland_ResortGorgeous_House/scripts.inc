.equ SHOWN_REQUESTED_MON, FLAG_TEMP_2

Kanto_FiveIsland_ResortGorgeous_House_MapScripts::
	.byte 0

@ Moemon Kai: FireRed's SELPHY and her butler, as pokefirered data/maps/FiveIsland_ResortGorgeous_House (Quetzal's
@ "one reward per day" version never reset: docs/REGIONS_AUDIT.md case 30). Her wish lasts 250 steps
@ (field_control_avatar.c -> mp_region.c MP_ResortGorgeousStep, FireRed's IncrementResortGorgeousStepCounter).

Kanto_FiveIsland_ResortGorgeous_House_EventScript_8421BC0::
	lock
	faceplayer
	goto_if_set SHOWN_REQUESTED_MON, Kanto_FiveIsland_ResortGorgeous_House_EventScript_JustFulfilledRequest
	goto_if_eq VAR_QZ_0x5141, 0xFFFF, Kanto_FiveIsland_ResortGorgeous_House_EventScript_RequestTookTooLong
	goto_if_ne VAR_QZ_0x5141, SPECIES_NONE, Kanto_FiveIsland_ResortGorgeous_House_EventScript_CheckForRequestedMon
	msgbox Kanto_FiveIsland_ResortGorgeous_House_Text_PleaseHearMyWish
	goto Kanto_FiveIsland_ResortGorgeous_House_EventScript_RequestMon
	end

Kanto_FiveIsland_ResortGorgeous_House_EventScript_RequestMon::
	special QzSampleResortGorgeousMonAndReward
	msgbox Kanto_FiveIsland_ResortGorgeous_House_Text_WishToSeeMonPlease
	release
	end

Kanto_FiveIsland_ResortGorgeous_House_EventScript_RequestTookTooLong::
	msgbox Kanto_FiveIsland_ResortGorgeous_House_Text_OhImTiredOfWaiting
	goto Kanto_FiveIsland_ResortGorgeous_House_EventScript_RequestMon
	end

Kanto_FiveIsland_ResortGorgeous_House_EventScript_CheckForRequestedMon::
	specialvar VAR_RESULT, QzPartyHasRequestedResortMon
	goto_if_eq VAR_RESULT, TRUE, Kanto_FiveIsland_ResortGorgeous_House_EventScript_ShowRequestedMon
	goto Kanto_FiveIsland_ResortGorgeous_House_EventScript_RequestMon
	end

Kanto_FiveIsland_ResortGorgeous_House_EventScript_ShowRequestedMon::
	special QzSampleResortGorgeousMonAndReward
	msgbox Kanto_FiveIsland_ResortGorgeous_House_Text_MonYouCaughtForMeThankYou
	addobject 2
	applymovement 2, Kanto_FiveIsland_ResortGorgeous_House_Movement_ButlerEnter
	waitmovement 0
	textcolor 0
	msgbox Kanto_FiveIsland_ResortGorgeous_House_Text_ButlerYesMyLady
	applymovement 1, Kanto_FiveIsland_ResortGorgeous_House_Movement_FaceUp
	waitmovement 0
	textcolor 1
	msgbox Kanto_FiveIsland_ResortGorgeous_House_Text_SelphyGiveTokenOfAppreciation
	textcolor 0
	msgbox Kanto_FiveIsland_ResortGorgeous_House_Text_ButlerIShallDoAsYouBid
	closemessage
	call_if_eq VAR_FACING, DIR_NORTH, Kanto_FiveIsland_ResortGorgeous_House_EventScript_ButlerApproachPlayerNorth
	call_if_eq VAR_FACING, DIR_SOUTH, Kanto_FiveIsland_ResortGorgeous_House_EventScript_ButlerApproachPlayerSouth
	call_if_eq VAR_FACING, DIR_EAST, Kanto_FiveIsland_ResortGorgeous_House_EventScript_ButlerApproachPlayerEast
	call_if_eq VAR_FACING, DIR_WEST, Kanto_FiveIsland_ResortGorgeous_House_EventScript_ButlerApproachPlayerWest
	copyobjectxytoperm 2
	msgbox Kanto_FiveIsland_ResortGorgeous_House_Text_ButlerPleaseAcceptThisAsHerThanks
	giveitem 0x5142
	goto_if_eq VAR_RESULT, FALSE, Kanto_FiveIsland_ResortGorgeous_House_EventScript_NoRoomForReward
	setflag SHOWN_REQUESTED_MON
	setvar VAR_QZ_0x5141, SPECIES_NONE
	release
	end

Kanto_FiveIsland_ResortGorgeous_House_EventScript_ButlerApproachPlayerNorth::
	applymovement 1, Kanto_FiveIsland_ResortGorgeous_House_Movement_FaceDown
	applymovement 2, Kanto_FiveIsland_ResortGorgeous_House_Movement_ButlerApproachPlayerNorth
	waitmovement 0
	applymovement LOCALID_PLAYER, Kanto_FiveIsland_ResortGorgeous_House_Movement_FaceLeft
	waitmovement 0
	return

Kanto_FiveIsland_ResortGorgeous_House_EventScript_ButlerApproachPlayerSouth::
	applymovement 2, Kanto_FiveIsland_ResortGorgeous_House_Movement_ButlerApproachPlayerSouth
	waitmovement 0
	applymovement LOCALID_PLAYER, Kanto_FiveIsland_ResortGorgeous_House_Movement_FaceUp
	waitmovement 0
	return

Kanto_FiveIsland_ResortGorgeous_House_EventScript_ButlerApproachPlayerEast::
	applymovement 1, Kanto_FiveIsland_ResortGorgeous_House_Movement_FaceLeft
	applymovement 2, Kanto_FiveIsland_ResortGorgeous_House_Movement_ButlerApproachPlayerEast
	waitmovement 0
	applymovement LOCALID_PLAYER, Kanto_FiveIsland_ResortGorgeous_House_Movement_FaceUp
	waitmovement 0
	return

Kanto_FiveIsland_ResortGorgeous_House_EventScript_ButlerApproachPlayerWest::
	applymovement 2, Kanto_FiveIsland_ResortGorgeous_House_Movement_ButlerApproachPlayerWest
	waitmovement 0
	return

Kanto_FiveIsland_ResortGorgeous_House_EventScript_NoRoomForReward::
	textcolor 0
	msgbox Kanto_FiveIsland_ResortGorgeous_House_Text_ButlerBagAppearsToBeFull
	textcolor 1
	msgbox Kanto_FiveIsland_ResortGorgeous_House_Text_OhHowDisappointing
	setflag SHOWN_REQUESTED_MON
	setvar VAR_QZ_0x5141, SPECIES_NONE
	release
	end

Kanto_FiveIsland_ResortGorgeous_House_EventScript_JustFulfilledRequest::
	msgbox Kanto_FiveIsland_ResortGorgeous_House_Text_SelphyWishYouBestOfLuck
	release
	end

Kanto_FiveIsland_ResortGorgeous_House_Movement_ButlerApproachPlayerNorth::
	walk_left
	walk_left
	walk_left
	walk_down
	walk_down
	walk_down
	walk_in_place_faster_right
	step_end

Kanto_FiveIsland_ResortGorgeous_House_Movement_ButlerApproachPlayerSouth::
	walk_left
	walk_left
	walk_in_place_faster_down
	step_end

Kanto_FiveIsland_ResortGorgeous_House_Movement_ButlerApproachPlayerEast::
	walk_left
	walk_left
	walk_left
	walk_down
	step_end

@ Very odd. If the player is talking to Selphy facing West, then the player has clipped through the table
@ In this case rather than approach the player the Butler walks up, which clips the Butler into a wall
Kanto_FiveIsland_ResortGorgeous_House_Movement_ButlerApproachPlayerWest::
	walk_up
	step_end

Kanto_FiveIsland_ResortGorgeous_House_Movement_ButlerEnter::
	walk_in_place_down
	step_end

@ Addressed as "Sebastian" but speaks only as "Butler"
Kanto_FiveIsland_ResortGorgeous_House_EventScript_8421DC4::
	msgbox Kanto_FiveIsland_ResortGorgeous_House_Text_LadySelphySmileHasBrillianceOfSun, MSGBOX_NPC
	end

Kanto_FiveIsland_ResortGorgeous_House_Text_PleaseHearMyWish::
	.string "SELPHY: Oh?\n"
	.string "Do I know you from somewhere?\p"
	.string "Oh, but that doesn’t matter.\p"
	.string "I am exquisitely bored.\n"
	.string "Please, hear my wish.$"

Kanto_FiveIsland_ResortGorgeous_House_Text_WishToSeeMonPlease::
	.string "I wish to see a MOéMON.\n"
	.string "A darling {STR_VAR_1}.\p"
	.string "I want to see one right away.\n"
	.string "Please, make my wish come true.$"

Kanto_FiveIsland_ResortGorgeous_House_Text_OhImTiredOfWaiting::
	.string "SELPHY: Oh, I’m tired of this.\n"
	.string "I’m tired of waiting for you.\p"
	.string "I’ll forgive you this time, but\n"
	.string "please don’t fail me next time.$"

Kanto_FiveIsland_ResortGorgeous_House_Text_MonYouCaughtForMeThankYou::
	.string "SELPHY: Oh, hello, there.\p"
	.string "Oh, my gracious. That must be the\n"
	.string "{STR_VAR_1} you caught for me.\p"
	.string "Giggle…\n"
	.string "I see a resemblance to you.\p"
	.string "Thank you so kindly.\p"
	.string "… …  … …  … …\n"
	.string "Is something the matter?\p"
	.string "You may leave.\p"
	.string "… …  … …  … …\n"
	.string "Oh, fine. I understand now.\p"
	.string "SEBASTIAN!\n"
	.string "I need you here this instant!$"

Kanto_FiveIsland_ResortGorgeous_House_Text_ButlerYesMyLady::
	.string "Butler: Yes, my lady.$"

Kanto_FiveIsland_ResortGorgeous_House_Text_SelphyGiveTokenOfAppreciation::
	.string "SELPHY: See to it that this person\n"
	.string "is given a token of appreciation.$"

Kanto_FiveIsland_ResortGorgeous_House_Text_ButlerIShallDoAsYouBid::
	.string "Butler: I shall do as you bid,\n"
	.string "my lady.$"

Kanto_FiveIsland_ResortGorgeous_House_Text_ButlerPleaseAcceptThisAsHerThanks::
	.string "Butler: I sincerely thank you for\n"
	.string "pleasing Lady SELPHY.\p"
	.string "Please accept this as her thanks…$"

Kanto_FiveIsland_ResortGorgeous_House_Text_ButlerBagAppearsToBeFull::
	.string "Butler: Your BAG appears to be\n"
	.string "unfortunately full.$"

Kanto_FiveIsland_ResortGorgeous_House_Text_OhHowDisappointing::
	.string "Oh…\n"
	.string "How disappointing.$"

Kanto_FiveIsland_ResortGorgeous_House_Text_SelphyWishYouBestOfLuck::
	.string "SELPHY: I wish you the best of\n"
	.string "luck.$"

Kanto_FiveIsland_ResortGorgeous_House_Text_LadySelphySmileHasBrillianceOfSun::
	.string "Butler: The smile of Lady SELPHY\n"
	.string "has the brilliance of the sun.\p"
	.string "Please, I beg you to bring joy\n"
	.string "to Lady SELPHY.$"

@ the POKéMON JOURNAL on the shelf
Kanto_FiveIsland_ResortGorgeous_House_EventScript_845D2FD::
	lockall
	setvar VAR_0x8004, 8
	setvar VAR_0x8005, 5
	special QzQuestLogNop
	textcolor 3
	msgbox Kanto_FiveIsland_ResortGorgeous_House_Text_8461BC0, MSGBOX_DEFAULT
	releaseall
	end

Kanto_FiveIsland_ResortGorgeous_House_Text_8461BC0::
	.string "MOéMON JOURNAL\p"
	.string "Special Feature: CINNABAR GYM\n"
	.string "LEADER BLAINE!\p"
	.string "BLAINE is said to remove his dark\n"
	.string "shades only when he is thinking up\l"
	.string "new quiz questions.$"

Kanto_FiveIsland_ResortGorgeous_House_Movement_FaceUp:
	walk_in_place_faster_up
	step_end

Kanto_FiveIsland_ResortGorgeous_House_Movement_FaceDown:
	walk_in_place_faster_down
	step_end

Kanto_FiveIsland_ResortGorgeous_House_Movement_FaceLeft:
	walk_in_place_faster_left
	step_end
