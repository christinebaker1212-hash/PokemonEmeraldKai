Sinnoh_GalacticHqControlRoom_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_GalacticHqControlRoom_EventScript_Bg1::
	goto SinnohP_galactic_hq_control_room_4
	end

Sinnoh_GalacticHqControlRoom_EventScript_Bg2::
	goto SinnohP_galactic_hq_control_room_5
	end

Sinnoh_GalacticHqControlRoom_EventScript_Bg3::
	goto SinnohP_galactic_hq_control_room_6
	end

Sinnoh_GalacticHqControlRoom_EventScript_Bg4::
	goto SinnohP_galactic_hq_control_room_10
	end

Sinnoh_GalacticHqControlRoom_EventScript_Person1::
	msgbox Sinnoh_GalacticHqControlRoom_Text_Person1, MSGBOX_NPC
	end

Sinnoh_GalacticHqControlRoom_EventScript_Person2::
	msgbox Sinnoh_GalacticHqControlRoom_Text_Person2, MSGBOX_NPC
	end

Sinnoh_GalacticHqControlRoom_EventScript_Person3::
	msgbox Sinnoh_GalacticHqControlRoom_Text_Person3, MSGBOX_NPC
	end

Sinnoh_GalacticHqControlRoom_EventScript_Person4::
	msgbox Sinnoh_GalacticHqControlRoom_Text_Person4, MSGBOX_NPC
	end

Sinnoh_GalacticHqControlRoom_EventScript_Person5::
	msgbox Sinnoh_GalacticHqControlRoom_Text_Person5, MSGBOX_NPC
	end

Sinnoh_GalacticHqControlRoom_Text_Person1:
	.string "SATURN: The lake POKéMON are in those\n"
	.string "tubes. Free them if you can. I won't\l"
	.string "stop you.\p"
	.string "I'm starting to think our BOSS doesn't\n"
	.string "care what happens to any of us.$"

Sinnoh_GalacticHqControlRoom_Text_Person2:
	.string "AZELF floats behind the glass, eyes\n"
	.string "closed.$"

Sinnoh_GalacticHqControlRoom_Text_Person3:
	.string "MESPRIT floats behind the glass, eyes\n"
	.string "closed.$"

Sinnoh_GalacticHqControlRoom_Text_Person4:
	.string "UXIE floats behind the glass, eyes\n"
	.string "closed.$"

Sinnoh_GalacticHqControlRoom_Text_Person5:
	.string "CHARON: Heh heh. The red chains are\n"
	.string "made. CYRUS has gone to SPEAR PILLAR.\p"
	.string "Whatever happens next, I'll be\n"
	.string "watching from a safe distance.$"

@ --- end of SINNOH words ---
