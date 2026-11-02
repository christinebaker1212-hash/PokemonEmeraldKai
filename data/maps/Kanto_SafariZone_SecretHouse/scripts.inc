Kanto_SafariZone_SecretHouse_MapScripts::
	.byte 0

@ Kanto_SafariZone_SecretHouse object 1 (local id 1, gfx 0x3F5B) at 6,5
Kanto_SafariZone_SecretHouse_EventScript_84121A9::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x2150, Kanto_SafariZone_SecretHouse_EventScript_8412202
	msgbox Kanto_SafariZone_SecretHouse_Text_84284C2, MSGBOX_DEFAULT
	setvar VAR_0x8004, ITEM_HM03   @ Moemon Kai: FireRed's HM (tools/quetzal_port/fix_hm_givers.py)
	call EventScript_GiveHmOnce
	msgbox Kanto_SafariZone_SecretHouse_Text_84285C1, MSGBOX_DEFAULT
	setflag FLAG_QZ_0x2150
	release
	end

Kanto_SafariZone_SecretHouse_EventScript_84121F8::
	msgbox Kanto_SafariZone_SecretHouse_Text_8428629, MSGBOX_DEFAULT
	release
	end

Kanto_SafariZone_SecretHouse_EventScript_8412202::
	msgbox Kanto_SafariZone_SecretHouse_Text_84285C1, MSGBOX_DEFAULT
	release
	end

Kanto_SafariZone_SecretHouse_Text_84284C2::
	.string "Ah! Finally!\p"
	.string "You're the first person to reach\n"
	.string "the SECRET HOUSE!\p"
	.string "Although I made a campaign\n"
	.string "for our grand opening,\p"
	.string "I was getting worried that no one\n"
	.string "would win our campaign prize.\p"
	.string "Congratulations!\n"
	.string "You have won!$"

Kanto_SafariZone_SecretHouse_Text_8428593::
	.string "{PLAYER} received a PRISM SCALE\n"
	.string "from the attendant!$"

Kanto_SafariZone_SecretHouse_Text_84285C1::
	.string "HM03 is SURF.\p"
	.string "MOéMON will be able to ferry you\n"
	.string "across water using it.\p"
	.string "And, this HM isn't disposable, so\n"
	.string "you can use it over and over.\p"
	.string "You're super lucky for winning this\n"
	.string "fabulous prize!$"

Kanto_SafariZone_SecretHouse_Text_8428629::
	.string "You don't have room for this\n"
	.string "fabulous prize!$"
