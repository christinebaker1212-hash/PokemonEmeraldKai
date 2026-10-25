Kanto_SixIsland_DottedHole_B2F_MapScripts::
	.byte 0

@ Kanto_SixIsland_DottedHole_B2F sign / bg event at 6,5
Kanto_SixIsland_DottedHole_B2F_EventScript_8413F8A::
	lockall
	braillemessage Kanto_SixIsland_DottedHole_B2F_Braille_845958F
	waitbuttonpress
	closebraillemessage
	releaseall
	end

Kanto_SixIsland_DottedHole_B2F_Braille_845958F:
	@ braille
	.string "      ßÌÊì$"
