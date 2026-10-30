@ Moemon Co-op: the GAME CENTER's ROOFTOP (the SKY LOUNGE): people taking a break, and drinks.
RustboroCity_GameCenter_Rooftop_MapScripts::
	.byte 0

RustboroCity_GameCenter_Rooftop_EventScript_VendingMachine::
	goto LilycoveCity_DepartmentStoreRooftop_EventScript_VendingMachine
	end

RustboroCity_GameCenter_Rooftop_EventScript_Sign::
	msgbox RustboroCity_GameCenter_Rooftop_Text_Sign, MSGBOX_SIGN
	end

RustboroCity_GameCenter_Rooftop_EventScript_View::
	msgbox RustboroCity_GameCenter_Rooftop_Text_View, MSGBOX_SIGN
	end

RustboroCity_GameCenter_Rooftop_EventScript_OldMan::
	msgbox RustboroCity_GameCenter_Rooftop_Text_OldMan, MSGBOX_NPC
	end

RustboroCity_GameCenter_Rooftop_EventScript_Cook::
	msgbox RustboroCity_GameCenter_Rooftop_Text_Cook, MSGBOX_NPC
	end

RustboroCity_GameCenter_Rooftop_EventScript_Reporter::
	msgbox RustboroCity_GameCenter_Rooftop_Text_Reporter, MSGBOX_NPC
	end

RustboroCity_GameCenter_Rooftop_EventScript_Cameraman::
	msgbox RustboroCity_GameCenter_Rooftop_Text_Cameraman, MSGBOX_NPC
	end

RustboroCity_GameCenter_Rooftop_EventScript_Expert::
	msgbox RustboroCity_GameCenter_Rooftop_Text_Expert, MSGBOX_NPC
	end

RustboroCity_GameCenter_Rooftop_Text_Sign:
	.string "GAME CENTER SKY LOUNGE\n"
	.string "Please take a break before you play\l"
	.string "again!$"

RustboroCity_GameCenter_Rooftop_Text_View:
	.string "All of RUSTBORO CITY is down there.\n"
	.string "DEVON CORPORATION looks tiny!$"

RustboroCity_GameCenter_Rooftop_Text_OldMan:
	.string "Back in my day, a game was a stick\n"
	.string "and a ball. Now look at this place!\p"
	.string "...I won three card duels downstairs\n"
	.string "today, though. Hohoho!$"

RustboroCity_GameCenter_Rooftop_Text_Cook:
	.string "I come up here on my break. The wind\n"
	.string "smells like the sea!$"

RustboroCity_GameCenter_Rooftop_Text_Reporter:
	.string "We're doing a story on the GAME\n"
	.string "CENTER. It's the talk of RUSTBORO!$"

RustboroCity_GameCenter_Rooftop_Text_Cameraman:
	.string "From up here I can get the whole\n"
	.string "city in one shot!$"

RustboroCity_GameCenter_Rooftop_Text_Expert:
	.string "On a clear day, you can see all the\n"
	.string "way to PETALBURG WOODS.$"
