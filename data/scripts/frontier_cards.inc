@ Moemon Kai: the Frontier Brains out by their facilities' doors with a deck, once the player holds that facility's
@ silver symbol. About half of them are out on any day (src/tcg/tcg_field.c Tcg_FrontierSetup). Their decks are the
@ card games' special trainers' (tcg_field.c sSpecials, ygo_field.c sDuelists): the CARD CASE's button starts a duel.
EventScript_FrontierCards_Anabel::
	msgbox FrontierCards_Text_Anabel, MSGBOX_NPC
	end

EventScript_FrontierCards_Tucker::
	msgbox FrontierCards_Text_Tucker, MSGBOX_NPC
	end

EventScript_FrontierCards_Spenser::
	msgbox FrontierCards_Text_Spenser, MSGBOX_NPC
	end

EventScript_FrontierCards_Greta::
	msgbox FrontierCards_Text_Greta, MSGBOX_NPC
	end

EventScript_FrontierCards_Noland::
	msgbox FrontierCards_Text_Noland, MSGBOX_NPC
	end

EventScript_FrontierCards_Lucy::
	msgbox FrontierCards_Text_Lucy, MSGBOX_NPC
	end

FrontierCards_Text_Anabel:
	.string "ANABEL: The TOWER can spare me today.\p"
	.string "A hand of cards tells me as much\n"
	.string "about a TRAINER as a battle does.\p"
	.string "If you carry a CARD CASE,\n"
	.string "show it to me.$"

FrontierCards_Text_Tucker:
	.string "TUCKER: No crowd, no spotlight…\n"
	.string "Just me and my deck out here!\p"
	.string "Still, every duel deserves a show.\n"
	.string "Got your CARD CASE? Let's put one on!$"

FrontierCards_Text_Spenser:
	.string "SPENSER: Hoh! A day away from the\n"
	.string "PALACE does an old man good.\p"
	.string "Cards show a TRAINER's true nature\n"
	.string "too, you know.\p"
	.string "Bring out your CARD CASE, if you dare!$"

FrontierCards_Text_Greta:
	.string "GRETA: Heya! The ARENA's closed\n"
	.string "to me today, so I'm playing cards!\p"
	.string "Guts work at the table, too.\n"
	.string "Show me that CARD CASE of yours!$"

FrontierCards_Text_Noland:
	.string "NOLAND: Building a deck's like\n"
	.string "building a team from rentals.\p"
	.string "You work with what you draw!\n"
	.string "Got a CARD CASE? Let's see it.$"

FrontierCards_Text_Lucy:
	.string "LUCY: …\p"
	.string "…Cards.\n"
	.string "Show me your CARD CASE.$"
