Kanto_SixIsland_DottedHole_B4F_MapScripts::
	.byte 0

@ Kanto_SixIsland_DottedHole_B4F sign / bg event at 6,5
Kanto_SixIsland_DottedHole_B4F_EventScript_8413F76::
	lockall
	braillemessage Kanto_SixIsland_DottedHole_B4F_Braille_8459578
	waitbuttonpress
	closebraillemessage
	releaseall
	end

Kanto_SixIsland_DottedHole_B4F_Braille_8459578:
	@ braille
	.string "      Îç+é$"
