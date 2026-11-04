Johto_ManiasHouse_MapScripts::
	.byte 0

@ Johto_ManiasHouse object 1 (local id 1, gfx 0x3EBF) at 4,4
Johto_ManiasHouse_EventScript_83CB203::
	lock
	faceplayer
	goto_if_eq VAR_QZ_0x501D, 1, Johto_ManiasHouse_EventScript_83CB254
	goto_if_eq VAR_QZ_0x501D, 3, Johto_ManiasHouse_EventScript_83CB264
	goto_if_eq VAR_QZ_0x501D, 5, Johto_ManiasHouse_EventScript_83CB264
	@ Moemon Kai: Crystal's loan: SHUCKIE is MANIA's, asked back from the next day (docs/REGIONS_AUDIT.md 102)
	goto_if_eq VAR_QZ_0x501D, 4, Johto_ManiasHouse_EventScript_Lent
	msgbox Johto_ManiasHouse_Text_83CB26F, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Johto_ManiasHouse_EventScript_83CB249
	getpartysize
	goto_if_eq VAR_RESULT, PARTY_SIZE, Johto_ManiasHouse_EventScript_PartyFull
	bufferspeciesname STR_VAR_1, SPECIES_SHUCKLE
	qz_givestorymon 2
	msgbox Johto_ManiasHouse_Text_TakeCare, MSGBOX_DEFAULT
	playfanfare MUS_OBTAIN_ITEM
	message Johto_ManiasHouse_Text_GotShuckle
	waitmessage
	waitfanfare
	closemessage
	setvar VAR_QZ_0x501D, 4
	setflag FLAG_DAILY_JOHTO_SHUCKIE
	release
	end

Johto_ManiasHouse_EventScript_PartyFull::
	msgbox Johto_ManiasHouse_Text_PartyFull, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_ManiasHouse_EventScript_Lent::
	goto_if_unset FLAG_DAILY_JOHTO_SHUCKIE, Johto_ManiasHouse_EventScript_AskBack
	msgbox Johto_ManiasHouse_Text_TakeCare, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_ManiasHouse_EventScript_AskBack::
	msgbox Johto_ManiasHouse_Text_CanIHaveItBack, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Johto_ManiasHouse_EventScript_Refused
	specialvar VAR_RESULT, MP_ReturnShuckie
	goto_if_eq VAR_RESULT, 0, Johto_ManiasHouse_EventScript_NotThere
	goto_if_eq VAR_RESULT, 2, Johto_ManiasHouse_EventScript_LastMon
	goto_if_eq VAR_RESULT, 3, Johto_ManiasHouse_EventScript_LikesYou
	msgbox Johto_ManiasHouse_Text_ThankYou, MSGBOX_DEFAULT
	closemessage
	setvar VAR_QZ_0x501D, 5
	release
	end

Johto_ManiasHouse_EventScript_NotThere::
	msgbox Johto_ManiasHouse_Text_NotThere, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_ManiasHouse_EventScript_LastMon::
	msgbox Johto_ManiasHouse_Text_LastMon, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_ManiasHouse_EventScript_LikesYou::
	msgbox Johto_ManiasHouse_Text_LikesYou, MSGBOX_DEFAULT
	closemessage
	setvar VAR_QZ_0x501D, 5
	release
	end

Johto_ManiasHouse_EventScript_Refused::
	msgbox Johto_ManiasHouse_Text_Robbed, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_ManiasHouse_EventScript_83CB249::
	msgbox Johto_ManiasHouse_Text_83CB3BE, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_ManiasHouse_EventScript_83CB254::
	msgbox Johto_ManiasHouse_Text_83CB472, MSGBOX_DEFAULT
	closemessage
	setvar VAR_QZ_0x501D, 3
	release
	end

Johto_ManiasHouse_EventScript_83CB264::
	msgbox Johto_ManiasHouse_Text_83CB521, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_ManiasHouse_Text_83CB26F::
	.string "I, I’m in shock!\p"
	.string "A guy about your age with piercing\n"
	.string "eyes and long hair came in.\p"
	.string "He scared me into giving him my\n"
	.string "prized MOéMON!\p"
	.string "I still have one left, but what if\n"
	.string "he comes back?\p"
	.string "You look strong. Could you look\n"
	.string "after my MOéMON for a while?$"

Johto_ManiasHouse_Text_83CB3BE::
	.string "Oh, no… What’ll I do if he comes\n"
	.string "back?$"

Johto_ManiasHouse_Text_83CB472::
	.string "I want my MOéMON to travel with you.\n"
	.string "You can keep it for good!\p"
	.string "Just promise to take good care of it.$"

Johto_ManiasHouse_Text_83CB521::
	.string "For MOéMON, happiness is being\n"
	.string "with a person who treats them\l"
	.string "well.$"

Johto_ManiasHouse_Text_TakeCare::
	.string "Oh, thank you!\p"
	.string "Take good care of it, please!$"

Johto_ManiasHouse_Text_GotShuckle::
	.string "{PLAYER} received a MOéMON.$"

Johto_ManiasHouse_Text_PartyFull::
	.string "Your MOéMON party is full.$"

Johto_ManiasHouse_Text_CanIHaveItBack::
	.string "Hi! How’s my MOéMON?\p"
	.string "I think I’m safe now, so may I\n"
	.string "have it back?$"

Johto_ManiasHouse_Text_ThankYou::
	.string "Thank you!$"

Johto_ManiasHouse_Text_NotThere::
	.string "Hey, you don’t have my MOéMON with\n"
	.string "you.$"

Johto_ManiasHouse_Text_LikesYou::
	.string "My MOéMON has come to like you.\p"
	.string "All right, you should keep it.\p"
	.string "But promise to be good to it!$"

Johto_ManiasHouse_Text_Robbed::
	.string "Oh, no, no… That’s the same as\n"
	.string "being robbed.$"

Johto_ManiasHouse_Text_LastMon::
	.string "If I take my MOéMON back, what are\n"
	.string "you going to use in battle?$"
