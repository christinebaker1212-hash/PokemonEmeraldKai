Kanto_Route19_MapScripts::
	.byte 0

@ Kanto_Route19 sign / bg event at 13,12
Kanto_Route19_EventScript_8417F2E::
	msgbox Kanto_Route19_Text_8437EFD, MSGBOX_SIGN
	end

@ Kanto_Route19 object 2 (local id 2, gfx 0x3F4E) at 10,9
Kanto_Route19_EventScript_845B7E2::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8427, 0, Kanto_Route19_Text_8437AB1, Kanto_Route19_Text_8437AED
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route19_EventScript_845B809
	msgbox Kanto_Route19_Text_8437AFC, MSGBOX_AUTOCLOSE
	end

Kanto_Route19_EventScript_845B809::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8427, 0, Kanto_Route19_Text_8470194, Kanto_Route19_Text_8437AED
	msgbox Kanto_Route19_Text_8437AFC, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route19 object 1 (local id 1, gfx 0x3F4E) at 15,10
Kanto_Route19_EventScript_845B820::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8428, 0, Kanto_Route19_Text_8437B1F, Kanto_Route19_Text_8437B4C
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route19_EventScript_845B847
	msgbox Kanto_Route19_Text_8437B60, MSGBOX_AUTOCLOSE
	end

Kanto_Route19_EventScript_845B847::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8428, 0, Kanto_Route19_Text_84701BF, Kanto_Route19_Text_8437B4C
	msgbox Kanto_Route19_Text_8437B60, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route19 object 4 (local id 4, gfx 0x3F4F) at 18,33
Kanto_Route19_EventScript_845B85E::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8429, 0, Kanto_Route19_Text_8437B92, Kanto_Route19_Text_8437BB3
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route19_EventScript_845B885
	msgbox Kanto_Route19_Text_8437BBF, MSGBOX_AUTOCLOSE
	end

Kanto_Route19_EventScript_845B885::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8429, 0, Kanto_Route19_Text_84701F1, Kanto_Route19_Text_8437BB3
	msgbox Kanto_Route19_Text_8437BBF, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route19 object 5 (local id 5, gfx 0x3F4F) at 8,27
Kanto_Route19_EventScript_845B89C::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8430, 0, Kanto_Route19_Text_8437BEB, Kanto_Route19_Text_8437C06
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route19_EventScript_845B8C3
	msgbox Kanto_Route19_Text_8437C0C, MSGBOX_AUTOCLOSE
	end

Kanto_Route19_EventScript_845B8C3::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8430, 0, Kanto_Route19_Text_8470223, Kanto_Route19_Text_8437C06
	msgbox Kanto_Route19_Text_8437C0C, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route19 object 6 (local id 6, gfx 0x3F4F) at 16,22
Kanto_Route19_EventScript_845B8DA::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8431, 0, Kanto_Route19_Text_8437C33, Kanto_Route19_Text_8437C63
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route19_EventScript_845B901
	msgbox Kanto_Route19_Text_8437C69, MSGBOX_AUTOCLOSE
	end

Kanto_Route19_EventScript_845B901::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8431, 0, Kanto_Route19_Text_847024D, Kanto_Route19_Text_8437C63
	msgbox Kanto_Route19_Text_8437C69, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route19 object 3 (local id 3, gfx 0x3F4F) at 12,17
Kanto_Route19_EventScript_845B918::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8432, 0, Kanto_Route19_Text_8437C93, Kanto_Route19_Text_8437CD1
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route19_EventScript_845B93F
	msgbox Kanto_Route19_Text_8437CE1, MSGBOX_AUTOCLOSE
	end

Kanto_Route19_EventScript_845B93F::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8432, 0, Kanto_Route19_Text_847028F, Kanto_Route19_Text_8437CD1
	msgbox Kanto_Route19_Text_8437CE1, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route19 object 7 (local id 7, gfx 0x3F4F) at 11,47
Kanto_Route19_EventScript_845B956::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8433, 0, Kanto_Route19_Text_8437E08, Kanto_Route19_Text_8437E33
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route19_EventScript_845B97D
	msgbox Kanto_Route19_Text_8437E43, MSGBOX_AUTOCLOSE
	end

Kanto_Route19_EventScript_845B97D::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8433, 0, Kanto_Route19_Text_8470366, Kanto_Route19_Text_8437E33
	msgbox Kanto_Route19_Text_8437E43, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route19 object 10 (local id 10, gfx 0x3F4D) at 9,48
Kanto_Route19_EventScript_845B994::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8468, 0, Kanto_Route19_Text_8437D1F, Kanto_Route19_Text_8437D52
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route19_EventScript_845B9BB
	msgbox Kanto_Route19_Text_8437D5E, MSGBOX_AUTOCLOSE
	end

Kanto_Route19_EventScript_845B9BB::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8468, 0, Kanto_Route19_Text_84702F0, Kanto_Route19_Text_8437D52
	msgbox Kanto_Route19_Text_8437D5E, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route19 object 8 (local id 8, gfx 0x3F4D) at 14,48
Kanto_Route19_EventScript_845B9D2::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8469, 0, Kanto_Route19_Text_8437DAC, Kanto_Route19_Text_8437DCF
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route19_EventScript_845B9F9
	msgbox Kanto_Route19_Text_8437DD8, MSGBOX_AUTOCLOSE
	end

Kanto_Route19_EventScript_845B9F9::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8469, 0, Kanto_Route19_Text_8470343, Kanto_Route19_Text_8437DCF
	msgbox Kanto_Route19_Text_8437DD8, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route19 object 9 (local id 9, gfx 0x3F4D) at 12,49
Kanto_Route19_EventScript_845BA10::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8470, 0, Kanto_Route19_Text_8437E6B, Kanto_Route19_Text_8437E93
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route19_EventScript_845BA37
	msgbox Kanto_Route19_Text_8437EA2, MSGBOX_AUTOCLOSE
	end

Kanto_Route19_EventScript_845BA37::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8470, 0, Kanto_Route19_Text_84703A2, Kanto_Route19_Text_8437E93
	msgbox Kanto_Route19_Text_8437EA2, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route19 object 11 (local id 11, gfx 0x3F4D) at 8,41
Kanto_Route19_EventScript_845BA4E::
	trainerbattle TRAINER_BATTLE_DOUBLE, TRAINER_QZ_8682, 0, Kanto_Route19_Text_8437F29, Kanto_Route19_Text_8437F66, Kanto_Route19_Text_8437FE9
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route19_EventScript_845BA79
	msgbox Kanto_Route19_Text_8437F95, MSGBOX_AUTOCLOSE
	end

Kanto_Route19_EventScript_845BA79::
	trainerbattle TRAINER_BATTLE_REMATCH_DOUBLE, TRAINER_QZ_8682, 0, Kanto_Route19_Text_84703E7, Kanto_Route19_Text_8437F66, Kanto_Route19_Text_8437FE9
	msgbox Kanto_Route19_Text_8437F95, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route19 object 12 (local id 12, gfx 0x3F53) at 9,41
Kanto_Route19_EventScript_845BA94::
	trainerbattle TRAINER_BATTLE_DOUBLE, TRAINER_QZ_8682, 0, Kanto_Route19_Text_843803C, Kanto_Route19_Text_8438075, Kanto_Route19_Text_84380DF
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route19_EventScript_845BABF
	msgbox Kanto_Route19_Text_84380A4, MSGBOX_AUTOCLOSE
	end

Kanto_Route19_EventScript_845BABF::
	trainerbattle TRAINER_BATTLE_REMATCH_DOUBLE, TRAINER_QZ_8682, 0, Kanto_Route19_Text_8470454, Kanto_Route19_Text_8438075, Kanto_Route19_Text_84380DF
	msgbox Kanto_Route19_Text_84380A4, MSGBOX_AUTOCLOSE
	end

Kanto_Route19_Text_8437AB1::
	.string "I have to exercise and warm up\n"
	.string "before going out for a swim.$"

Kanto_Route19_Text_8437AED::
	.string "All warmed up!$"

Kanto_Route19_Text_8437AFC::
	.string "Thanks, kid!\n"
	.string "I’m ready for a swim.$"

Kanto_Route19_Text_8437B1F::
	.string "Wait! Slow down!\n"
	.string "You’ll have a heart attack!$"

Kanto_Route19_Text_8437B4C::
	.string "Ooh!\n"
	.string "That’s chilly!$"

Kanto_Route19_Text_8437B60::
	.string "Watch out for TENTACOOL.\n"
	.string "They sting so painfully.$"

Kanto_Route19_Text_8437B92::
	.string "I love swimming!\n"
	.string "What about you?$"

Kanto_Route19_Text_8437BB3::
	.string "Belly flop!$"

Kanto_Route19_Text_8437BBF::
	.string "I can beat even marine POKéMON\n"
	.string "at swimming.$"

Kanto_Route19_Text_8437BEB::
	.string "What’s beyond the horizon?$"

Kanto_Route19_Text_8437C06::
	.string "Glub!$"

Kanto_Route19_Text_8437C0C::
	.string "I see a couple of islands over\n"
	.string "yonder!$"

Kanto_Route19_Text_8437C33::
	.string "I tried diving for POKéMON, but it\n"
	.string "was a no-go.$"

Kanto_Route19_Text_8437C63::
	.string "Help!$"

Kanto_Route19_Text_8437C69::
	.string "I guess you have to fish for sea\n"
	.string "POKéMON.$"

Kanto_Route19_Text_8437C93::
	.string "I look at the sea to forget all\n"
	.string "the bad things that happened.$"

Kanto_Route19_Text_8437CD1::
	.string "Ooh!\n"
	.string "Traumatic!$"

Kanto_Route19_Text_8437CE1::
	.string "I’m looking at the sea to forget\n"
	.string "the bad thing that happened!$"

Kanto_Route19_Text_8437D1F::
	.string "Oh, I just love your ride!\n"
	.string "Can I have it if I win?$"

Kanto_Route19_Text_8437D52::
	.string "Oh! I lost!$"

Kanto_Route19_Text_8437D5E::
	.string "It’s still a long way to go to\n"
	.string "SEAFOAM ISLANDS…\p"
	.string "I want to go back to FUCHSIA…$"

Kanto_Route19_Text_8437DAC::
	.string "Swimming’s great!\n"
	.string "Sunburns aren’t!$"

Kanto_Route19_Text_8437DCF::
	.string "Shocker!$"

Kanto_Route19_Text_8437DD8::
	.string "My boyfriend wanted to swim to\n"
	.string "SEAFOAM ISLANDS.$"

Kanto_Route19_Text_8437E08::
	.string "Ahoy, there!\n"
	.string "These waters are treacherous!$"

Kanto_Route19_Text_8437E33::
	.string "Ooh!\n"
	.string "Dangerous!$"

Kanto_Route19_Text_8437E43::
	.string "M-my legs! They cramped up!\n"
	.string "Glub, glub…$"

Kanto_Route19_Text_8437E6B::
	.string "I swam here with my friends…\n"
	.string "I’m tired…$"

Kanto_Route19_Text_8437E93::
	.string "I’m exhausted…$"

Kanto_Route19_Text_8437EA2::
	.string "When I get tired, I ride LAPRAS.\p"
	.string "It’s so big that I can cross the\n"
	.string "sea without getting wet.$"

Kanto_Route19_Text_8437EFD::
	.string "SEA ROUTE 19\n"
	.string "FUCHSIA CITY - SEAFOAM ISLANDS$"

Kanto_Route19_Text_8437F29::
	.string "LIA: I’m looking after my brother.\n"
	.string "He just became a TRAINER.$"

Kanto_Route19_Text_8437F66::
	.string "LIA: That’s no way to treat my\n"
	.string "little brother!$"

Kanto_Route19_Text_8437F95::
	.string "LIA: Do you have a younger\n"
	.string "brother?\p"
	.string "I hope you’re teaching him all\n"
	.string "sorts of things.$"

Kanto_Route19_Text_8437FE9::
	.string "LIA: I want to battle together\n"
	.string "with my little brother.\p"
	.string "Don’t you have two POKéMON?$"

Kanto_Route19_Text_843803C::
	.string "LUC: My big sis taught me how\n"
	.string "to swim and train POKéMON.$"

Kanto_Route19_Text_8438075::
	.string "LUC: Oh, wow!\n"
	.string "Someone tougher than my big sis!$"

Kanto_Route19_Text_84380A4::
	.string "LUC: My big sis is strong and nice.\n"
	.string "I think she’s awesome!$"

Kanto_Route19_Text_84380DF::
	.string "LUC: I don’t want to if I can’t\n"
	.string "battle you with my big sis.\p"
	.string "Don’t you have two POKéMON?$"

Kanto_Route19_Text_8470194::
	.string "I’m almost warmed up to go\n"
	.string "out for a swim.$"

Kanto_Route19_Text_84701BF::
	.string "Wait! Slow down!\n"
	.string "What if you have a heart attack?$"

Kanto_Route19_Text_84701F1::
	.string "I love swimming!\n"
	.string "I guess you’re the surfing type.$"

Kanto_Route19_Text_8470223::
	.string "What’s beyond the horizon?\n"
	.string "Have you seen?$"

Kanto_Route19_Text_847024D::
	.string "I tried diving for POKéMON again,\n"
	.string "but it was a no-go like before.$"

Kanto_Route19_Text_847028F::
	.string "I look at the sea to forget all\n"
	.string "the bad things that happened.\p"
	.string "…Like losing to you the last time!$"

Kanto_Route19_Text_84702F0::
	.string "You always get to ride your\n"
	.string "POKéMON…\p"
	.string "It looks so relaxing.\n"
	.string "Can I have it if I win?$"

Kanto_Route19_Text_8470343::
	.string "Swimming’s great!\n"
	.string "Sunburns aren’t!$"

Kanto_Route19_Text_8470366::
	.string "These waters are treacherous!\n"
	.string "You shouldn’t come back here!$"

Kanto_Route19_Text_84703A2::
	.string "I swam here with my friends…\n"
	.string "I’m tired…\l"
	.string "Must we really battle again?$"

Kanto_Route19_Text_84703E7::
	.string "LIA: You know my brother just\n"
	.string "became a TRAINER, right?\p"
	.string "I want to make him better, so I\n"
	.string "need your help again.$"

Kanto_Route19_Text_8470454::
	.string "LUC: My big sis taught me all\n"
	.string "about POKéMON.\p"
	.string "I wonder if I’m better?$"
