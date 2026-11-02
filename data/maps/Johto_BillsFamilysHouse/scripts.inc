Johto_BillsFamilysHouse_MapScripts::
	.byte 0

@ Johto_BillsFamilysHouse object 3 (local id 3, gfx 0x6) at 8,7
Johto_BillsFamilysHouse_EventScript_83D3116::
	lock
	faceplayer
	msgbox Johto_BillsFamilysHouse_Text_83D34D3, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_BillsFamilysHouse object 2 (local id 2, gfx 0x18) at 8,4
Johto_BillsFamilysHouse_EventScript_83D3123::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x11CB, Johto_BillsFamilysHouse_EventScript_83D3139
	msgbox Johto_BillsFamilysHouse_Text_83D33D6, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_BillsFamilysHouse_EventScript_83D3139::
	msgbox Johto_BillsFamilysHouse_Text_83D3481, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_BillsFamilysHouse object 1 (local id 1, gfx 0x3E86) at 5,4
Johto_BillsFamilysHouse_EventScript_83D3144::
	lock
	special Script_ClearHeldMovement
	faceplayer

Johto_BillsFamilysHouse_EventScript_83D3189::
	goto_if_unset FLAG_QZ_0x103B, Johto_BillsFamilysHouse_EventScript_83D319D
	msgbox Johto_BillsFamilysHouse_Text_83D336F, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_BillsFamilysHouse_EventScript_83D319D::
	@ Moemon Kai: Crystal's gift (tools/quetzal_port/fix_johto_gifts.py) (Quetzal: ROTOM)
	msgbox Johto_BillsFamilysHouse_Text_83D31E7, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Johto_BillsFamilysHouse_EventScript_83D31DC
	msgbox Johto_BillsFamilysHouse_Text_83D3251, MSGBOX_DEFAULT
	setvar VAR_TEMP_1, SPECIES_EEVEE
	call Quetzal_EventScript_8361A4D
	bufferspeciesname 0, SPECIES_EEVEE
	qz_givemon_ex SPECIES_EEVEE, 20, ITEM_NONE, 1, 2, 3
	call Quetzal_EventScript_840181E
	setflag FLAG_QZ_0x103B
	msgbox Johto_BillsFamilysHouse_Text_83D32D5, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_BillsFamilysHouse_EventScript_83D31DC::
	msgbox Johto_BillsFamilysHouse_Text_83D335B, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_BillsFamilysHouse_Text_83D31E7::
	.string "BILL: Hi, {PLAYER}! Do us a favor\n"
	.string "and take this EEVEE.\p"
	.string "It came over when I was adjusting\n"
	.string "the TIME CAPSULE.\p"
	.string "Someone has to take care of it,\n"
	.string "but I don’t like being outside.\p"
	.string "Can I count on you to play with\n"
	.string "it, {PLAYER}?$"

Johto_BillsFamilysHouse_Text_83D3251::
	.string "BILL: I knew you’d come through!\p"
	.string "Way to go! You’re the real deal!\p"
	.string "OK, I’m counting on you.\p"
	.string "Take good care of it!$"

Johto_BillsFamilysHouse_Text_83D32D5::
	.string "BILL: PROF.ELM claims EEVEE may\n"
	.string "evolve in new and unknown ways.$"

Johto_BillsFamilysHouse_Text_83D335B::
	.string "Oh… Now what to do?$"

Johto_BillsFamilysHouse_Text_83D336F::
	.string "BILL: My pop, he won’t work. All\n"
	.string "he does is goof off all day long.\p"
	.string "He’s getting to be a real\n"
	.string "headache…$"

Johto_BillsFamilysHouse_Text_83D33D6::
	.string "Oh, you collect MOéMON? My son\n"
	.string "BILL is an expert.\p"
	.string "He just got called to the MOéMON\n"
	.string "CENTER in ECRUTEAK CITY.\p"
	.string "My husband went off to the GAME\n"
	.string "CORNER without being called…$"

Johto_BillsFamilysHouse_Text_83D3481::
	.string "My husband was once known as a\n"
	.string "MOéMANIAC.\p"
	.string "BILL must have taken after his\n"
	.string "father.$"

Johto_BillsFamilysHouse_Text_83D34D3::
	.string "My big brother BILL made the PC\n"
	.string "MOéMON storage system.$"
