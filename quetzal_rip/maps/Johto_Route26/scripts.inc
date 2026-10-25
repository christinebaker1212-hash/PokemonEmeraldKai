Johto_Route26_MapScripts::
	.byte 0

@ Johto_Route26 object 1 (local id 1, gfx 0x3E8F) at 26,24
Johto_Route26_EventScript_83EBCA9::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4541, 0, Johto_Route26_Text_83EBF4B, Johto_Route26_Text_83EBFD9
	msgbox Johto_Route26_Text_83EBFF6, MSGBOX_AUTOCLOSE
	end

@ Johto_Route26 trigger at 12,6 (VAR_TEMP_0 == 0)
Johto_Route26_EventScript_83EC10D::
	setflag FLAG_QZ_0x11C8
	setvar VAR_TEMP_0, 1
	end

Johto_Route26_Text_83EBF4B::
	.string "I lost to a trainer named SILVER.\p"
	.string "He was really strong, but…\p"
	.string "It was as if he absolutely had to\n"
	.string "win at any cost.\p"
	.string "I felt sorry for his POKéMON.$"

Johto_Route26_Text_83EBFD9::
	.string "POKéMON aren’t tools of war.$"

Johto_Route26_Text_83EBFF6::
	.string "POKéMON are in-valuable, lifelong\n"
	.string "partners.$"
