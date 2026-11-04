Johto_GoldenrodHappinessRater_MapScripts::
	.byte 0

@ Johto_GoldenrodHappinessRater object 1 (local id 1, gfx 0x2F) at 4,5
Johto_GoldenrodHappinessRater_EventScript_82E8C5C::
	@ Moemon Kai: Crystal's HAPPINESS RATER (GoldenrodHappinessRater.asm): the lead POKéMON's friendship in six words
	@ (Quetzal's rules text about RETURN and evolution was false here)
	lock
	faceplayer
	specialvar VAR_0x8004, MP_LeadFriendship
	msgbox Johto_GoldenrodHappinessRater_Text_Teacher, MSGBOX_DEFAULT
	goto_if_ge VAR_0x8004, 250, Johto_GoldenrodHappinessRater_EventScript_LovesYouALot
	goto_if_ge VAR_0x8004, 200, Johto_GoldenrodHappinessRater_EventScript_ReallyTrustsYou
	goto_if_ge VAR_0x8004, 150, Johto_GoldenrodHappinessRater_EventScript_SortOfHappy
	goto_if_ge VAR_0x8004, 100, Johto_GoldenrodHappinessRater_EventScript_QuiteCute
	goto_if_ge VAR_0x8004, 50, Johto_GoldenrodHappinessRater_EventScript_NotUsedToYou
	goto Johto_GoldenrodHappinessRater_EventScript_LooksMean

Johto_GoldenrodHappinessRater_EventScript_LovesYouALot::
	msgbox Johto_GoldenrodHappinessRater_Text_LovesYouALot, MSGBOX_DEFAULT
	release
	end

Johto_GoldenrodHappinessRater_EventScript_ReallyTrustsYou::
	msgbox Johto_GoldenrodHappinessRater_Text_ReallyTrustsYou, MSGBOX_DEFAULT
	release
	end

Johto_GoldenrodHappinessRater_EventScript_SortOfHappy::
	msgbox Johto_GoldenrodHappinessRater_Text_SortOfHappy, MSGBOX_DEFAULT
	release
	end

Johto_GoldenrodHappinessRater_EventScript_QuiteCute::
	msgbox Johto_GoldenrodHappinessRater_Text_QuiteCute, MSGBOX_DEFAULT
	release
	end

Johto_GoldenrodHappinessRater_EventScript_NotUsedToYou::
	msgbox Johto_GoldenrodHappinessRater_Text_NotUsedToYou, MSGBOX_DEFAULT
	release
	end

Johto_GoldenrodHappinessRater_EventScript_LooksMean::
	msgbox Johto_GoldenrodHappinessRater_Text_LooksMean, MSGBOX_DEFAULT
	release
	end

@ Johto_GoldenrodHappinessRater object 2 (local id 2, gfx 0x11) at 7,4
Johto_GoldenrodHappinessRater_EventScript_83D67C6::
	lock
	faceplayer
	msgbox Johto_GoldenrodHappinessRater_Text_83D67E0, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodHappinessRater object 3 (local id 3, gfx 0x6) at 7,7
Johto_GoldenrodHappinessRater_EventScript_83D67D3::
	lock
	faceplayer
	msgbox Johto_GoldenrodHappinessRater_Text_83D684A, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_GoldenrodHappinessRater_Text_83D67E0::
	.string "I keep losing in battles, and my\n"
	.string "MOéMON end up fainting…\p"
	.string "Maybe that’s why my MOéMON don’t\n"
	.string "like me much…$"

Johto_GoldenrodHappinessRater_Text_83D684A::
	.string "When I use an item on my MOéMON,\n"
	.string "it acts really glad!$"

Johto_GoldenrodHappinessRater_Text_Teacher::
	.string "If you treat your MOéMON nicely,\n"
	.string "they will love you in return.\p"
	.string "Oh? Let me see your {STR_VAR_1}…$"

Johto_GoldenrodHappinessRater_Text_LovesYouALot::
	.string "It looks really happy! It must\n"
	.string "love you a lot.$"

Johto_GoldenrodHappinessRater_Text_ReallyTrustsYou::
	.string "I get the feeling that it really\n"
	.string "trusts you.$"

Johto_GoldenrodHappinessRater_Text_SortOfHappy::
	.string "It’s friendly toward you. It looks\n"
	.string "sort of happy.$"

Johto_GoldenrodHappinessRater_Text_QuiteCute::
	.string "It’s quite cute.$"

Johto_GoldenrodHappinessRater_Text_NotUsedToYou::
	.string "You should treat it better. It’s\n"
	.string "not used to you.$"

Johto_GoldenrodHappinessRater_Text_LooksMean::
	.string "It doesn’t seem to like you at\n"
	.string "all. It looks mean.$"
