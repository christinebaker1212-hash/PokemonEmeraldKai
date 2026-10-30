Kanto_SixIsland_DottedHole_B3F_MapScripts::
	.byte 0

@ Kanto_SixIsland_DottedHole_B3F sign / bg event at 6,5
Kanto_SixIsland_DottedHole_B3F_EventScript_8413F80::
	lockall
	braillemessage Kanto_SixIsland_DottedHole_B3F_Braille_8459583
	waitbuttonpress
	closebraillemessage
	releaseall
	end

Kanto_SixIsland_DottedHole_B3F_Braille_8459583:
	@ braille
	.string "      ëÉÔÒì$"
