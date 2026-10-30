Kanto_SixIsland_DottedHole_B1F_MapScripts::
	.byte 0

@ Kanto_SixIsland_DottedHole_B1F sign / bg event at 6,5
Kanto_SixIsland_DottedHole_B1F_EventScript_8413F6C::
	lockall
	braillemessage Kanto_SixIsland_DottedHole_B1F_Braille_845956F
	waitbuttonpress
	closebraillemessage
	releaseall
	end

Kanto_SixIsland_DottedHole_B1F_Braille_845956F:
	@ braille
	.string "      ぅá$"
