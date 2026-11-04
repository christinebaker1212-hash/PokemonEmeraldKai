Kanto_SilphCo_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_SilphCo_1F_EventScript_8411329
	.byte 0

Kanto_SilphCo_1F_EventScript_8411329::
	setflag FLAG_QZ_0x2255
	end

@ Kanto_SilphCo_1F object 1 (local id 1, gfx 0x3F5A) at 3,7
Kanto_SilphCo_1F_EventScript_841132D::
	lock
	faceplayer
	msgbox Kanto_SilphCo_1F_Text_8425BAF, MSGBOX_DEFAULT   @ Moemon Kai: FireRed's line only (Quetzal's empty shop gone)

Kanto_SilphCo_1F_EventScript_8411357::
	release
	end

@ Kanto_SilphCo_1F sign / bg event at 30,1
Kanto_SilphCo_1F_EventScript_8411359::
	msgbox Kanto_SilphCo_1F_Text_8425C2A, MSGBOX_SIGN
	end

Kanto_SilphCo_1F_Mart_8411364:
	.2byte ITEM_NONE

Kanto_SilphCo_1F_Text_8425BAF::
	.string "Welcome.\p"
	.string "The PRESIDENT is in the boardroom\n"
	.string "on 11F.$"

Kanto_SilphCo_1F_Text_8425BE2::
	.string "We also sell supplies\n"
	.string "for your research.$"

Kanto_SilphCo_1F_Text_8425C2A::
	.string "SILPH CO. HEAD OFFICE\n"
	.string "1F$"
