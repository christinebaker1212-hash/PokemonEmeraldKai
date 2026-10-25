Johto_Route38_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_Route38_EventScript_83F2755
	.byte 0

Johto_Route38_EventScript_83F2755::
	end

@ Johto_Route38 object 4 (local id 4, gfx 0x3EAE) at 13,28
Johto_Route38_EventScript_83F2756::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4665, 0, Johto_Route38_Text_83F2805, Johto_Route38_Text_83F2835
	msgbox Johto_Route38_Text_83F2857, MSGBOX_AUTOCLOSE
	end

@ Johto_Route38 object 5 (local id 5, gfx 0xB) at 2,7
Johto_Route38_EventScript_83F276D::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4270, 0, Johto_Route38_Text_83F2898, Johto_Route38_Text_83F28BE
	msgbox Johto_Route38_Text_83F28DE, MSGBOX_AUTOCLOSE
	end

@ Johto_Route38 object 2 (local id 2, gfx 0x2F) at 28,12
Johto_Route38_EventScript_83F2784::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4554, 0, Johto_Route38_Text_83F2950, Johto_Route38_Text_83F299D
	msgbox Johto_Route38_Text_83F29C0, MSGBOX_AUTOCLOSE
	end

@ Johto_Route38 object 6 (local id 6, gfx 0x2D) at 38,29
Johto_Route38_EventScript_83F279B::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4110, 0, Johto_Route38_Text_83F2A04, Johto_Route38_Text_83F2A3B
	msgbox Johto_Route38_Text_83F2A5F, MSGBOX_AUTOCLOSE
	end

@ Johto_Route38 object 1 (local id 1, gfx 0x31) at 39,7
Johto_Route38_EventScript_83F27B2::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4710, 0, Johto_Route38_Text_83F2A91, Johto_Route38_Text_83F2ACB
	msgbox Johto_Route38_Text_83F2AE6, MSGBOX_AUTOCLOSE
	end

@ Johto_Route38 object 3 (local id 3, gfx 0x14) at 15,6
Johto_Route38_EventScript_83F27C9::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4226, 0, Johto_Route38_Text_83F2B26, Johto_Route38_Text_83F2B56
	msgbox Johto_Route38_Text_83F2B76, MSGBOX_AUTOCLOSE
	end

@ Johto_Route38 sign / bg event at 42,14
Johto_Route38_EventScript_83F27E0::
	lock
	msgbox Johto_Route38_Text_83F2BDA, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_Route38 sign / bg event at 15,27
Johto_Route38_EventScript_83F27EC::
	lock
	msgbox Johto_Route38_Text_83F2CA5, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_Route38 object 9 (local id 9, gfx 0x3A9E) at 1,30
Johto_Route38_EventScript_83F27F8::
	setorcopyvar VAR_0x8000, 0x195
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Johto_Route38_Text_83F2805::
	.string "Fly high into the sky,\n"
	.string "my beloved bird POKéMON!$"

Johto_Route38_Text_83F2835::
	.string "I feel like just flying\n"
	.string "away now.$"

Johto_Route38_Text_83F2857::
	.string "I plan to train in CIANWOOD CITY\n"
	.string "to teach my POKéMON how to FLY.$"

Johto_Route38_Text_83F2898::
	.string "Let me try something\n"
	.string "I learned today.$"

Johto_Route38_Text_83F28BE::
	.string "I didn’t study enough,\n"
	.string "I guess.$"

Johto_Route38_Text_83F28DE::
	.string "I have to take so many\n"
	.string "tests, I don’t have much\l"
	.string "time for POKéMON.\p"
	.string "So when I do get to play,\n"
	.string "I really concentrate.$"

Johto_Route38_Text_83F2950::
	.string "You seem to be good at\n"
	.string "POKéMON.\p"
	.string "If you are, how about\n"
	.string "giving me some advice?$"

Johto_Route38_Text_83F299D::
	.string "I see. So you can battle\n"
	.string "that way.$"

Johto_Route38_Text_83F29C0::
	.string "I know something good!\p"
	.string "MOOMOO FARM’s milk is\n"
	.string "famous for its flavor.$"

Johto_Route38_Text_83F2A04::
	.string "Hi! Aren’t you a cute\n"
	.string "trainer!\p"
	.string "May I see your POKéMON?$"

Johto_Route38_Text_83F2A3B::
	.string "I’m glad I got to see\n"
	.string "your POKéMON!$"

Johto_Route38_Text_83F2A5F::
	.string "When I see POKéMON, it\n"
	.string "seems to soothe my nerves.$"

Johto_Route38_Text_83F2A91::
	.string "I’ve been overseas, so I\n"
	.string "know about all sorts of\l"
	.string "POKéMON!$"

Johto_Route38_Text_83F2ACB::
	.string "Your skill is world class!$"

Johto_Route38_Text_83F2AE6::
	.string "All kinds of people around\n"
	.string "the world live happily with\l"
	.string "POKéMON.$"

Johto_Route38_Text_83F2B26::
	.string "Don’t you think my POKéMON\n"
	.string "and I are beautiful?$"

Johto_Route38_Text_83F2B56::
	.string "We drink MOOMOO MILK every\n"
	.string "day.$"

Johto_Route38_Text_83F2B76::
	.string "MOOMOO MILK is good for\n"
	.string "beauty and health,\p"
	.string "but inconveniently, they\n"
	.string "only sell one bottle at a time.$"

Johto_Route38_Text_83F2BDA::
	.string "ROUTE 38\p"
	.string "OLIVINE CITY - ECRUTEAK CITY$"

Johto_Route38_Text_83F2CA5::
	.string "NO SWIMMING OR FISHING.\n"
	.string "WATER FOR FARMING ONLY.\p"
	.string "-ECRUTEAK COUNCIL$"
