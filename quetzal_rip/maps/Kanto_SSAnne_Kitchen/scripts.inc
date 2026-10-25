Kanto_SSAnne_Kitchen_MapScripts::
	.byte 0

@ Kanto_SSAnne_Kitchen object 1 (local id 1, gfx 0x3EF8) at 1,5
Kanto_SSAnne_Kitchen_EventScript_8410629::
	msgbox Kanto_SSAnne_Kitchen_Text_84239E3, MSGBOX_NPC
	end

@ Kanto_SSAnne_Kitchen object 2 (local id 2, gfx 0x3EF8) at 6,6
Kanto_SSAnne_Kitchen_EventScript_8410632::
	msgbox Kanto_SSAnne_Kitchen_Text_8423A14, MSGBOX_NPC
	end

@ Kanto_SSAnne_Kitchen object 3 (local id 3, gfx 0x3EF8) at 10,4
Kanto_SSAnne_Kitchen_EventScript_841063B::
	msgbox Kanto_SSAnne_Kitchen_Text_8423A4D, MSGBOX_NPC
	end

@ Kanto_SSAnne_Kitchen object 4 (local id 4, gfx 0x3EF8) at 8,9
Kanto_SSAnne_Kitchen_EventScript_8410644::
	lock
	faceplayer
	msgbox Kanto_SSAnne_Kitchen_Text_8423B5B, MSGBOX_DEFAULT
	random 3
	copyvar VAR_0x8008, VAR_RESULT
	call_if_eq VAR_0x8008, 0, Kanto_SSAnne_Kitchen_EventScript_8410679
	call_if_eq VAR_0x8008, 1, Kanto_SSAnne_Kitchen_EventScript_8410682
	call_if_eq VAR_0x8008, 2, Kanto_SSAnne_Kitchen_EventScript_841068B
	release
	end

Kanto_SSAnne_Kitchen_EventScript_8410679::
	msgbox Kanto_SSAnne_Kitchen_Text_8423B8A, MSGBOX_DEFAULT
	return

Kanto_SSAnne_Kitchen_EventScript_8410682::
	msgbox Kanto_SSAnne_Kitchen_Text_8423BCA, MSGBOX_DEFAULT
	return

Kanto_SSAnne_Kitchen_EventScript_841068B::
	msgbox Kanto_SSAnne_Kitchen_Text_8423BFC, MSGBOX_DEFAULT
	return

@ Kanto_SSAnne_Kitchen object 5 (local id 5, gfx 0x3EF8) at 14,5
Kanto_SSAnne_Kitchen_EventScript_8410694::
	msgbox Kanto_SSAnne_Kitchen_Text_8423A8B, MSGBOX_NPC
	end

@ Kanto_SSAnne_Kitchen object 6 (local id 6, gfx 0x3EF8) at 14,7
Kanto_SSAnne_Kitchen_EventScript_841069D::
	msgbox Kanto_SSAnne_Kitchen_Text_8423ABE, MSGBOX_NPC
	end

@ Kanto_SSAnne_Kitchen object 7 (local id 7, gfx 0x3EF8) at 14,9
Kanto_SSAnne_Kitchen_EventScript_84106A6::
	msgbox Kanto_SSAnne_Kitchen_Text_8423B29, MSGBOX_NPC
	end

@ Kanto_SSAnne_Kitchen object 8 (local id 8, gfx 0x3F0E) at 1,10
Kanto_SSAnne_Kitchen_EventScript_846BB62::
	setorcopyvar VAR_0x8000, 2
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_SSAnne_Kitchen_Text_84239E3::
	.string "You, mon petit!\n"
	.string "We’re busy here!\l"
	.string "Out of the way!$"

Kanto_SSAnne_Kitchen_Text_8423A14::
	.string "I saw an odd BERRY in the trash.\n"
	.string "I wonder what that was?$"

Kanto_SSAnne_Kitchen_Text_8423A4D::
	.string "I’m so busy I’m getting dizzy!\n"
	.string "You have to give me room here!$"

Kanto_SSAnne_Kitchen_Text_8423A8B::
	.string "Hum-de-hum-de-ho…\p"
	.string "I peel spuds every day!\n"
	.string "Hum-hum…$"

Kanto_SSAnne_Kitchen_Text_8423ABE::
	.string "Did you hear about SNORLAX?\n"
	.string "It’s a glutton.\p"
	.string "No other POKéMON eats and sleeps\n"
	.string "the way SNORLAX can and does!$"

Kanto_SSAnne_Kitchen_Text_8423B29::
	.string "Snivel… Sniff…\p"
	.string "I only get to peel onions…\n"
	.string "Snivel…$"

Kanto_SSAnne_Kitchen_Text_8423B5B::
	.string "Er-hem!\n"
	.string "Indeed I am le CHEF!\p"
	.string "Le main course is$"

Kanto_SSAnne_Kitchen_Text_8423B8A::
	.string "Salmon du Salad!\p"
	.string "Les guests may gripe it’s fish\n"
	.string "again, however!$"

Kanto_SSAnne_Kitchen_Text_8423BCA::
	.string "Eels au Barbecue!\p"
	.string "Les guests will mutiny, I fear.$"

Kanto_SSAnne_Kitchen_Text_8423BFC::
	.string "Prime Beefsteak!\p"
	.string "But, have I enough fillets du\n"
	.string "beef?$"
