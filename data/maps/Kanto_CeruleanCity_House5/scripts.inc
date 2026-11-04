Kanto_CeruleanCity_House5_MapScripts::
	.byte 0

@ Kanto_CeruleanCity_House5 object 1 (local id 1, gfx 0x3F2D) at 7,4
Kanto_CeruleanCity_House5_EventScript_841AE49::
	@ Moemon Kai: FireRed's BERRY POWDER man without his POWDER JAR (which this game hasn't got): his own idle line;
	@ Quetzal's "BERRY SWEET" that was a NUGGET is gone
	lock
	faceplayer
	msgbox Kanto_CeruleanCity_House5_Text_AnyInterestInBerries, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, Kanto_CeruleanCity_House5_EventScript_InterestInBerries
	msgbox Kanto_CeruleanCity_House5_Text_TakeInterestInAllSortsOfThings, MSGBOX_DEFAULT
Kanto_CeruleanCity_House5_EventScript_InterestInBerries::
	release
	end

@ Kanto_CeruleanCity_House5 sign / bg event at 3,1
Kanto_CeruleanCity_House5_EventScript_841B0B7::
	lockall
	specialvar VAR_RESULT, QzReturnZero
	goto_if_eq VAR_RESULT, 2, Quetzal_EventScript_8457FA3
	msgbox Kanto_CeruleanCity_House5_Text_84435A6, MSGBOX_DEFAULT
	releaseall
	end

Kanto_CeruleanCity_House5_Text_8443292::
	.string "I study rare sweets made from\n"
	.string "BERRIES.\p"
	.string "This one belongs with a promising\n"
	.string "TRAINER like you.$"

Kanto_CeruleanCity_House5_Text_84432ED::
	.string "Please, take this BERRY SWEET.$"

Kanto_CeruleanCity_House5_Text_844330C::
	.string "A BERRY SWEET can help a certain\n"
	.string "MOéMON evolve.\p"
	.string "I hope you put it to good use.$"

Kanto_CeruleanCity_House5_Text_8443404::
	.string "I hope that BERRY SWEET helps\n"
	.string "your MOéMON grow.$"

Kanto_CeruleanCity_House5_Text_84435A6::
	.string "No records have been registered.$"

Kanto_CeruleanCity_House5_Text_TakeInterestInAllSortsOfThings::
	.string "You have no interest in BERRIES?\p"
	.string "Young one, it’s important to take an\n"
	.string "interest in all sorts of things.$"

Kanto_CeruleanCity_House5_Text_AnyInterestInBerries::
	.string "I concoct a variety of medicine\n"
	.string "from BERRY POWDER.\p"
	.string "Using good BERRY POWDER, I can\n"
	.string "make any kind of medicine.\p"
	.string "Now tell me, have you any interest\n"
	.string "in BERRIES?$"
