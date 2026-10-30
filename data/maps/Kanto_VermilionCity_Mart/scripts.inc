Kanto_VermilionCity_Mart_MapScripts::
	.byte 0

@ Kanto_VermilionCity_Mart object 2 (local id 2, gfx 0x3EFC) at 4,2
Kanto_VermilionCity_Mart_EventScript_841B645::
	msgbox Kanto_VermilionCity_Mart_Text_8444BBC, MSGBOX_NPC
	end

@ Kanto_VermilionCity_Mart object 3 (local id 3, gfx 0x3EE6) at 9,4
Kanto_VermilionCity_Mart_EventScript_841B64E::
	msgbox Kanto_VermilionCity_Mart_Text_8444ACE, MSGBOX_NPC
	end

@ Kanto_VermilionCity_Mart object 1 (local id 1, gfx 0x3EFA) at 2,3
Kanto_VermilionCity_Mart_EventScript_841B657::
	specialvar VAR_RESULT, QzReturnZero
	goto_if_eq VAR_RESULT, 2, Quetzal_EventScript_8457FA3
	lock
	faceplayer
	message Quetzal_Text_84563A6
	waitmessage
	pokemart 0x0
	msgbox Quetzal_Text_84565DA, MSGBOX_DEFAULT
	release
	end

Kanto_VermilionCity_Mart_Text_8444ACE::
	.string "There are wicked people who will\n"
	.string "use POKéMON for criminal acts.\p"
	.string "TEAM ROCKET traffics in rare\n"
	.string "POKéMON, for example.\p"
	.string "They also abandon POKéMON that\n"
	.string "they consider unpopular or useless.\p"
	.string "That’s the sort of horrid people\n"
	.string "they are, TEAM ROCKET.$"

Kanto_VermilionCity_Mart_Text_8444BBC::
	.string "I think POKéMON can be good or\n"
	.string "bad. It depends on the TRAINER.$"
