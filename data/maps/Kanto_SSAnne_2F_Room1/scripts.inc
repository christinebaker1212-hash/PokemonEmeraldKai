Kanto_SSAnne_2F_Room1_MapScripts::
	.byte 0

@ Kanto_SSAnne_2F_Room1 object 1 (local id 1, gfx 0x3F09) at 1,4
Kanto_SSAnne_2F_Room1_EventScript_8410810::
	lock
	faceplayer
	message Kanto_SSAnne_2F_Room1_Text_8424298
	waitmessage
	setvar VAR_0x8004, SPECIES_SNORLAX
	special QzSetSeenMon
	showmonpic SPECIES_SNORLAX, 0xA, 3
	waitbuttonpress
	hidemonpic
	release
	end

Kanto_SSAnne_2F_Room1_Text_8424298::
	.string "I’ve journeyed far and wide, but in\n"
	.string "all my travels I’ve never seen\l"
	.string "any POKéMON sleep like this one!\p"
	.string "It was something like this!$"
