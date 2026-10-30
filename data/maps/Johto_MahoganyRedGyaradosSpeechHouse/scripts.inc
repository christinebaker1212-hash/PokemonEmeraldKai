Johto_MahoganyRedGyaradosSpeechHouse_MapScripts::
	.byte 0

@ Johto_MahoganyRedGyaradosSpeechHouse object 2 (local id 2, gfx 0x2C) at 8,7
Johto_MahoganyRedGyaradosSpeechHouse_EventScript_83DF032::
	msgbox Johto_MahoganyRedGyaradosSpeechHouse_Text_83DF063, MSGBOX_NPC
	end

@ Johto_MahoganyRedGyaradosSpeechHouse object 1 (local id 1, gfx 0x3E85) at 3,5
Johto_MahoganyRedGyaradosSpeechHouse_EventScript_83DF03B::
	goto_if_lt VAR_QZ_0x500A, 6, Johto_MahoganyRedGyaradosSpeechHouse_EventScript_83DF051
	goto_if_lt VAR_QZ_0x500A, 10, Johto_MahoganyRedGyaradosSpeechHouse_EventScript_83DF05A

Johto_MahoganyRedGyaradosSpeechHouse_EventScript_83DF051::
	msgbox Johto_MahoganyRedGyaradosSpeechHouse_Text_83DF0D5, MSGBOX_NPC
	end

Johto_MahoganyRedGyaradosSpeechHouse_EventScript_83DF05A::
	msgbox Johto_MahoganyRedGyaradosSpeechHouse_Text_83DF107, MSGBOX_NPC
	end

Johto_MahoganyRedGyaradosSpeechHouse_Text_83DF063::
	.string "I heard that a red GYARADOS appeared\n"
	.string "at the LAKE.\p"
	.string "That’s odd, since even ordinary\n"
	.string "GYARADOS are rare in that lake…$"

Johto_MahoganyRedGyaradosSpeechHouse_Text_83DF0D5::
	.string "My favorite radio program? I’d say\n"
	.string "POKéMON MUSIC.$"

Johto_MahoganyRedGyaradosSpeechHouse_Text_83DF107::
	.string "I’ve been hearing laughter on the\n"
	.string "radio…\p"
	.string "It’s creepy.$"
