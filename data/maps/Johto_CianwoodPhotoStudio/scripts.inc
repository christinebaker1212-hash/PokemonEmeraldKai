@ Moemon Kai: Crystal's CIANWOOD PHOTO STUDIO (pret/pokecrystal maps/CianwoodPhotoStudio.asm; audit cases 116 / 146).
@ Its door had become Quetzal's CLIFF EDGE CAVE to HGSS's ROUTE 47 / 48 and the JOHTO SAFARI ZONE. The photo goes on
@ the TRAINER CARD's TEAM page, as CELADON's printer does (mp_tcard.c MP_TCard_PrintPhoto), and costs nothing.
Johto_CianwoodPhotoStudio_MapScripts::
	.byte 0

Johto_CianwoodPhotoStudio_EventScript_FishingGuru::
	lock
	faceplayer
	msgbox Johto_CianwoodPhotoStudio_Text_Question, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Johto_CianwoodPhotoStudio_EventScript_Refused
	message Johto_CianwoodPhotoStudio_Text_Yes
	waitmessage
	playse SE_CLICK
	fadescreenspeed FADE_TO_WHITE, 0
	fadescreenspeed FADE_FROM_WHITE, 0
	delay 30
	setvar VAR_0x8004, 0
	special MP_TCard_PrintPhoto
	waitbuttonpress
	closemessage
	release
	end

Johto_CianwoodPhotoStudio_EventScript_Refused::
	msgbox Johto_CianwoodPhotoStudio_Text_No, MSGBOX_DEFAULT
	release
	end

Johto_CianwoodPhotoStudio_Text_Question::
	.string "You have magnificent MOéMON with\n"
	.string "you.\p"
	.string "How about a photo for a souvenir?$"

Johto_CianwoodPhotoStudio_Text_Yes::
	.string "OK! Big smile now!$"

Johto_CianwoodPhotoStudio_Text_No::
	.string "Oh, that’s too bad. I thought it\n"
	.string "would be a great memento…$"
