@ Moemon Kai: Crystal's TIN TOWER entrance (pokecrystal maps/EcruteakTinTowerEntrance.asm; audit case 139). Here the
@ sage stands in the doorway to the WISE TRIO's room (the way to the TIN TOWER in this ECRUTEAK): with ECRUTEAK's
@ BADGE he lets the player through before the RADIO TOWER is freed (the trio then turn them back); after it, only
@ the CLEAR BELL moves him (FLAG_JOHTO_RANG_CLEAR_BELL).
Johto_EcruteakTinTowerEntrance_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_EcruteakTinTowerEntrance_EventScript_83CF05E
	.byte 0

Johto_EcruteakTinTowerEntrance_EventScript_83CF05E::
	call Johto_EcruteakTinTowerEntrance_EventScript_WanderingSages
	goto_if_set FLAG_JOHTO_RANG_CLEAR_BELL, Johto_EcruteakTinTowerEntrance_EventScript_83CF068
	goto_if_set FLAG_JOHTO_KOJI_ALLOWS_PASSAGE, Johto_EcruteakTinTowerEntrance_EventScript_83CF068
	goto_if_set FLAG_JOHTO_FOUGHT_SUICUNE, Johto_EcruteakTinTowerEntrance_EventScript_83CF068
	goto_if_ge VAR_QZ_0x500A, 10, Johto_EcruteakTinTowerEntrance_EventScript_Blocked
	goto_if_set FLAG_QZ_0x11AB, Johto_EcruteakTinTowerEntrance_EventScript_83CF068
Johto_EcruteakTinTowerEntrance_EventScript_Blocked::
	end

Johto_EcruteakTinTowerEntrance_EventScript_83CF068::
	setobjectxyperm 3, 15, 2
	end

@ (Crystal's wandering sage and old man are out except while the tower is closed for the beasts)
Johto_EcruteakTinTowerEntrance_EventScript_WanderingSages::
	goto_if_set FLAG_JOHTO_FOUGHT_SUICUNE, Johto_EcruteakTinTowerEntrance_EventScript_SagesOut
	goto_if_set FLAG_JOHTO_KOJI_ALLOWS_PASSAGE, Johto_EcruteakTinTowerEntrance_EventScript_SagesOut
	goto_if_lt VAR_QZ_0x500A, 10, Johto_EcruteakTinTowerEntrance_EventScript_SagesOut
	setflag FLAG_TEMP_11
Johto_EcruteakTinTowerEntrance_EventScript_SagesOut::
	return

@ Johto_EcruteakTinTowerEntrance sign / bg event at 14,1
Johto_EcruteakTinTowerEntrance_EventScript_83CF070::
	msgbox Johto_EcruteakTinTowerEntrance_Text_83CF079, MSGBOX_SIGN
	end

@ Johto_EcruteakTinTowerEntrance object 3 at 16,2: the sage in the doorway (Crystal's EcruteakTinTowerEntranceSageScript)
Johto_EcruteakTinTowerEntrance_EventScript_83CF0B9::
	lock
	faceplayer
	goto_if_ge VAR_QZ_0x500A, 10, Johto_EcruteakTinTowerEntrance_EventScript_CheckForClearBell
	goto_if_set FLAG_QZ_0x11AB, Johto_EcruteakTinTowerEntrance_EventScript_GotFogBadge
	msgbox Johto_EcruteakTinTowerEntrance_Text_Sage, MSGBOX_DEFAULT
	release
	end

Johto_EcruteakTinTowerEntrance_EventScript_GotFogBadge::
	msgbox Johto_EcruteakTinTowerEntrance_Text_SageGotFogBadge, MSGBOX_DEFAULT
	call Johto_EcruteakTinTowerEntrance_EventScript_StepAside
	release
	end

Johto_EcruteakTinTowerEntrance_EventScript_CheckForClearBell::
	goto_if_set FLAG_JOHTO_KOJI_ALLOWS_PASSAGE, Johto_EcruteakTinTowerEntrance_EventScript_AllowedThrough
	goto_if_set FLAG_JOHTO_RANG_CLEAR_BELL, Johto_EcruteakTinTowerEntrance_EventScript_RangClearBell
	checkitem ITEM_CLEAR_BELL
	goto_if_eq VAR_RESULT, TRUE, Johto_EcruteakTinTowerEntrance_EventScript_GotClearBell
	msgbox Johto_EcruteakTinTowerEntrance_Text_SageNoClearBell, MSGBOX_DEFAULT
	release
	end

Johto_EcruteakTinTowerEntrance_EventScript_GotClearBell::
	msgbox Johto_EcruteakTinTowerEntrance_Text_SageHearsClearBell, MSGBOX_DEFAULT
	setflag FLAG_JOHTO_RANG_CLEAR_BELL
	call Johto_EcruteakTinTowerEntrance_EventScript_StepAside
	release
	end

Johto_EcruteakTinTowerEntrance_EventScript_StepAside::
	closemessage
	goto_if_eq VAR_FACING, DIR_EAST, Johto_EcruteakTinTowerEntrance_EventScript_StepAsideRight
	applymovement 3, Johto_EcruteakTinTowerEntrance_Movement_83CF169
	waitmovement 0
	setobjectxyperm 3, 15, 2
	return

Johto_EcruteakTinTowerEntrance_EventScript_StepAsideRight::
	applymovement 3, Johto_EcruteakTinTowerEntrance_Movement_StepRight
	waitmovement 0
	setobjectxyperm 3, 17, 2
	return

Johto_EcruteakTinTowerEntrance_EventScript_AllowedThrough::
	msgbox Johto_EcruteakTinTowerEntrance_Text_SagePleaseDoGoOn, MSGBOX_DEFAULT
	release
	end

Johto_EcruteakTinTowerEntrance_EventScript_RangClearBell::
	msgbox Johto_EcruteakTinTowerEntrance_Text_SageHeardClearBell, MSGBOX_DEFAULT
	release
	end

@ Johto_EcruteakTinTowerEntrance object 1 at 10,3 (Crystal's wandering sage)
Johto_EcruteakTinTowerEntrance_EventScript_83CF12E::
	lock
	faceplayer
	checkitem ITEM_CLEAR_BELL
	goto_if_eq VAR_RESULT, TRUE, Johto_EcruteakTinTowerEntrance_EventScript_83CF151
	msgbox Johto_EcruteakTinTowerEntrance_Text_WanderingSage, MSGBOX_DEFAULT
	release
	end

Johto_EcruteakTinTowerEntrance_EventScript_83CF151::
	msgbox Johto_EcruteakTinTowerEntrance_Text_WanderingSageGotClearBell, MSGBOX_DEFAULT
	release
	end

@ Johto_EcruteakTinTowerEntrance object 2 at 3,3 (Crystal's old man)
Johto_EcruteakTinTowerEntrance_EventScript_83CF15C::
	msgbox Johto_EcruteakTinTowerEntrance_Text_Gramps, MSGBOX_NPC
	end

Johto_EcruteakTinTowerEntrance_Movement_StepRight:
	walk_right
	face_down
	step_end

Johto_EcruteakTinTowerEntrance_Movement_83CF169:
	walk_left
	face_down
	step_end

Johto_EcruteakTinTowerEntrance_Text_83CF079::
	.string "The bell of tin rings clearly for\n"
	.string "those under the tower’s wing.$"

Johto_EcruteakTinTowerEntrance_Text_Sage::
	.string "TIN TOWER is off limits to anyone\n"
	.string "without ECRUTEAK GYM’s BADGE.\p"
	.string "Sorry, but you’ll have to leave.$"

Johto_EcruteakTinTowerEntrance_Text_SageGotFogBadge::
	.string "TIN TOWER is off limits to anyone\n"
	.string "without ECRUTEAK GYM’s BADGE.\p"
	.string "Ah!\p"
	.string "ECRUTEAK’s GYM BADGE! Please, go\n"
	.string "right through.$"

Johto_EcruteakTinTowerEntrance_Text_SageNoClearBell::
	.string "A momentous event has occurred.\p"
	.string "I beg your pardon, but I must ask\n"
	.string "you to leave.\p"
	.string "…What soothes the soul…\p"
	.string "The WISE TRIO say things that are\n"
	.string "so very difficult to understand…$"

Johto_EcruteakTinTowerEntrance_Text_SageHearsClearBell::
	.string "A momentous event has occurred.\p"
	.string "I beg your pardon, but I must ask\n"
	.string "you to leave.\p"
	.string "………………\p"
	.string "Ah!\p"
	.string "The sound of that CLEAR BELL!\p"
	.string "It… It’s sublime!\p"
	.string "I’ve never heard so beautiful a\n"
	.string "sound before!\p"
	.string "That bell’s chime is indicative of\n"
	.string "the bearer’s soul.\p"
	.string "You…\p"
	.string "You may be able to make it through\n"
	.string "TIN TOWER.\p"
	.string "Please, do go on.$"

Johto_EcruteakTinTowerEntrance_Text_SagePleaseDoGoOn::
	.string "Please, do go on.$"

Johto_EcruteakTinTowerEntrance_Text_SageHeardClearBell::
	.string "That bell’s chime is indicative of\n"
	.string "the bearer’s soul.\p"
	.string "You…\p"
	.string "You may be able to make it through\n"
	.string "TIN TOWER.\p"
	.string "Please, do go on.$"

Johto_EcruteakTinTowerEntrance_Text_WanderingSage::
	.string "The TIN TOWER ahead is a ninetier\n"
	.string "tower of divine beauty.\p"
	.string "It soothes the soul of all who see\n"
	.string "it.$"

Johto_EcruteakTinTowerEntrance_Text_WanderingSageGotClearBell::
	.string "The TIN TOWER shook! A MOéMON must\n"
	.string "have returned to the top!$"

Johto_EcruteakTinTowerEntrance_Text_Gramps::
	.string "Two towers… Two MOéMON…\p"
	.string "But when one burned down, both\n"
	.string "MOéMON flew away, never to return.$"
