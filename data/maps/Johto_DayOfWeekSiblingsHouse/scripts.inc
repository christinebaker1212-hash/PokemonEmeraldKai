Johto_DayOfWeekSiblingsHouse_MapScripts::
	.byte 0

@ Johto_DayOfWeekSiblingsHouse sign / bg event at 6,4
@ Johto_DayOfWeekSiblingsHouse sign / bg event at 7,4
Johto_DayOfWeekSiblingsHouse_EventScript_83EC1A4::
	lock
	msgbox Johto_DayOfWeekSiblingsHouse_Text_83EC1DC, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Johto_DayOfWeekSiblingsHouse_EventScript_83EC1BB
	closemessage
	release
	end

Johto_DayOfWeekSiblingsHouse_EventScript_83EC1BB::
	msgbox Johto_DayOfWeekSiblingsHouse_Text_83EC205, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Johto_DayOfWeekSiblingsHouse_EventScript_83EC1D1
	closemessage
	release
	end

Johto_DayOfWeekSiblingsHouse_EventScript_83EC1D1::
	msgbox Johto_DayOfWeekSiblingsHouse_Text_83EC29E, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_DayOfWeekSiblingsHouse_Text_83EC1DC::
	.string "There’s something written here.\p"
	.string "Read it?$"

Johto_DayOfWeekSiblingsHouse_Text_83EC205::
	.string "To my brothers and sisters:\p"
	.string "We take pride in serving POKéMON\n"
	.string "trainers.\p"
	.string "When trainers talk to you, give\n"
	.string "them something useful.\p"
	.string "Love,\n"
	.string "MONICA\p"
	.string "Keep reading?$"

Johto_DayOfWeekSiblingsHouse_Text_83EC29E::
	.string "Monday, MONICA\n"
	.string "ROUTE 40\p"
	.string "Tuesday, TUSCANY\n"
	.string "ROUTE 29\p"
	.string "Wednesday, WESLEY\n"
	.string "LAKE OF RAGE\p"
	.string "Thursday, ARTHUR\n"
	.string "ROUTE 36\p"
	.string "Friday, FRIEDA\n"
	.string "ROUTE 32\p"
	.string "Saturday, SANTOS\n"
	.string "BLACKTHORN CITY\p"
	.string "Sunday, SUNNY\n"
	.string "ROUTE 37$"
