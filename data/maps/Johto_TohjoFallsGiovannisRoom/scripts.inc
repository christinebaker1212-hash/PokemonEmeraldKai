Johto_TohjoFallsGiovannisRoom_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_TohjoFallsGiovannisRoom_EventScript_83FC13A
	.byte 0

Johto_TohjoFallsGiovannisRoom_EventScript_83FC13A::
	special QzGiovanniStoryUpdateFlags
	end

@ Johto_TohjoFallsGiovannisRoom sign / bg event at 5,2
Johto_TohjoFallsGiovannisRoom_EventScript_83FC13E::
	special QzGiovanniStoryTohjoSignCheck
	goto_if_eq VAR_RESULT, 1, Johto_TohjoFallsGiovannisRoom_EventScript_8401997
	msgbox Johto_TohjoFallsGiovannisRoom_Text_83FC155, MSGBOX_SIGN
	end

@ Johto_TohjoFallsGiovannisRoom object 1 (local id 1, gfx 0x276A) at 5,4
Johto_TohjoFallsGiovannisRoom_EventScript_8401902::
	lockall
	special Script_ClearHeldMovement
	faceplayer
	goto_if_set FLAG_QZ_0x1319, Johto_TohjoFallsGiovannisRoom_EventScript_8401978
	special QzGiovanniStoryCanStart
	goto_if_eq VAR_RESULT, 0, Johto_TohjoFallsGiovannisRoom_EventScript_8401949
	msgbox Johto_TohjoFallsGiovannisRoom_Text_840203D, MSGBOX_DEFAULT
	msgbox Johto_TohjoFallsGiovannisRoom_Text_840213B, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_8401C92
	closemessage
	fadescreen 3
	special QzGiovanniStoryStart

Johto_TohjoFallsGiovannisRoom_EventScript_840193F::
	warp MAP_JOHTO_RADIO_TOWER5F, 255, 25, 13
	waitstate
	end

Johto_TohjoFallsGiovannisRoom_EventScript_8401949::
	msgbox Johto_TohjoFallsGiovannisRoom_Text_8402190, MSGBOX_DEFAULT
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4191, 0, Johto_TohjoFallsGiovannisRoom_Text_8401DA1
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, 1, Quetzal_EventScript_8401C92
	msgbox Johto_TohjoFallsGiovannisRoom_Text_84021FB, MSGBOX_DEFAULT
	goto Quetzal_EventScript_8401C92

Johto_TohjoFallsGiovannisRoom_EventScript_8401978::
	msgbox Johto_TohjoFallsGiovannisRoom_Text_8402D71, MSGBOX_DEFAULT
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4191, 0, Johto_TohjoFallsGiovannisRoom_Text_8401DA1
	msgbox Johto_TohjoFallsGiovannisRoom_Text_8402E0D, MSGBOX_DEFAULT
	goto Quetzal_EventScript_8401C92

Johto_TohjoFallsGiovannisRoom_EventScript_8401997::
	lockall
	special QzGiovanniStoryTohjoSignCheck
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_8401C94
	msgbox Johto_TohjoFallsGiovannisRoom_Text_8402324, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_8401C94
	closemessage
	fadescreen 3
	special QzGiovanniStoryStart
	goto_if_eq VAR_QZ_0x506C, 5, Johto_TohjoFallsGiovannisRoom_EventScript_840193F
	goto_if_ge VAR_QZ_0x506C, 2, Johto_TohjoFallsGiovannisRoom_EventScript_8401BA7
	goto Quetzal_EventScript_8401B9D

Johto_TohjoFallsGiovannisRoom_EventScript_8401BA7::
	warp MAP_JOHTO_PLAYERS_HOUSE1F, 255, 9, 7
	waitstate
	end

Johto_TohjoFallsGiovannisRoom_Text_83FC155::
	.string "A radio...\n"
	.string "The battery is dead...$"

Johto_TohjoFallsGiovannisRoom_Text_8401DA1::
	.string "So this is your true power...$"

Johto_TohjoFallsGiovannisRoom_Text_840203D::
	.string "My men called me from the RADIO TOWER.\n"
	.string "I never came... and it all ended.\p"
	.string "CELEBI can take us\n"
	.string "to an alternate past.\p"
	.string "Our present will remain unchanged.\n"
	.string "But that Johto will have a new history.\p"
	.string "There, I will answer their call.\n"
	.string "This time, I will take command.$"

Johto_TohjoFallsGiovannisRoom_Text_840213B::
	.string "You know our colors.\n"
	.string "You could help me reclaim what I lost.\p"
	.string "Will you travel with me?$"

Johto_TohjoFallsGiovannisRoom_Text_8402190::
	.string "Here to challenge me? Very well.\p"
	.string "I will show you that the leader of\n"
	.string "TEAM ROCKET still deserves that title.$"

Johto_TohjoFallsGiovannisRoom_Text_84021FB::
	.string "Another defeat... But none weighs\n"
	.string "on me like the call I left unanswered.\p"
	.string "My men needed me at the RADIO TOWER.\n"
	.string "If only I had gone...\p"
	.string "They say ILEX FOREST’s guardian can\n"
	.string "return to days the rest of us have lost.\p"
	.string "If one of our own brought\n"
	.string "that power before me...\l"
	.string "perhaps we could still change things.$"

Johto_TohjoFallsGiovannisRoom_Text_8402324::
	.string "CELEBI recognizes this place.\n"
	.string "Your journey into the past began here.\p"
	.string "It seems ready to take\n"
	.string "you back to that Johto.\p"
	.string "Travel to the past?$"

Johto_TohjoFallsGiovannisRoom_Text_8402D71::
	.string "I have not forgotten what\n"
	.string "you did at the RADIO TOWER.\p"
	.string "You helped me restore my organization...\n"
	.string "then turned against me.\p"
	.string "Have you come to challenge me again?$"

Johto_TohjoFallsGiovannisRoom_Text_8402E0D::
	.string "You may defeat me again. It will\n"
	.string "not make me forget your betrayal.$"
