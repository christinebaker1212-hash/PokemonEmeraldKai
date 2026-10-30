Kanto_Route10_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840FBB9
	.byte 0

@ Kanto_Route10 sign / bg event at 7,21
Kanto_Route10_EventScript_8417CAB::
	msgbox Kanto_Route10_Text_843518C, MSGBOX_SIGN
	end

@ Kanto_Route10 sign / bg event at 15,59
Kanto_Route10_EventScript_8417CB4::
	msgbox Kanto_Route10_Text_84351B0, MSGBOX_SIGN
	end

@ Kanto_Route10 sign / bg event at 6,42
Kanto_Route10_EventScript_8417CBD::
	msgbox Kanto_Route10_Text_84351BC, MSGBOX_SIGN
	end

@ Kanto_Route10 object 6 (local id 6, gfx 0x3F31) at 9,27
Kanto_Route10_EventScript_845A378::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8348, 0, Kanto_Route10_Text_8434FB8, Kanto_Route10_Text_8434FF8
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route10_EventScript_845A39F
	msgbox Kanto_Route10_Text_8435014, MSGBOX_AUTOCLOSE
	end

Kanto_Route10_EventScript_845A39F::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8348, 0, Kanto_Route10_Text_846F140, Kanto_Route10_Text_8434FF8
	msgbox Kanto_Route10_Text_8435014, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route10 object 1 (local id 1, gfx 0x3F31) at 7,60
Kanto_Route10_EventScript_845A3B6::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8349, 0, Kanto_Route10_Text_84350CB, Kanto_Route10_Text_8435102
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route10_EventScript_845A3DD
	msgbox Kanto_Route10_Text_8435125, MSGBOX_AUTOCLOSE
	end

Kanto_Route10_EventScript_845A3DD::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8349, 0, Kanto_Route10_Text_846F1BA, Kanto_Route10_Text_8435102
	msgbox Kanto_Route10_Text_8435125, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route10 object 5 (local id 5, gfx 0x3F4B) at 13,44
Kanto_Route10_EventScript_845A3F4::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8354, 0, Kanto_Route10_Text_8434E51, Kanto_Route10_Text_8434EAC
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route10_EventScript_845A41B
	msgbox Kanto_Route10_Text_8434EC2, MSGBOX_AUTOCLOSE
	end

Kanto_Route10_EventScript_845A41B::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8354, 0, Kanto_Route10_Text_846F0A0, Kanto_Route10_Text_8434EAC
	msgbox Kanto_Route10_Text_8434EC2, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route10 object 3 (local id 3, gfx 0x3F4B) at 15,70
Kanto_Route10_EventScript_845A432::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8355, 0, Kanto_Route10_Text_8434F4D, Kanto_Route10_Text_8434F6E
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route10_EventScript_845A459
	msgbox Kanto_Route10_Text_8434F82, MSGBOX_AUTOCLOSE
	end

Kanto_Route10_EventScript_845A459::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8355, 0, Kanto_Route10_Text_846F11C, Kanto_Route10_Text_8434F6E
	msgbox Kanto_Route10_Text_8434F82, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route10 object 2 (local id 2, gfx 0x3F0C) at 4,62
Kanto_Route10_EventScript_845A470::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8379, 0, Kanto_Route10_Text_8434EE4, Kanto_Route10_Text_8434EF4
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route10_EventScript_845A497
	msgbox Kanto_Route10_Text_8434F27, MSGBOX_AUTOCLOSE
	end

Kanto_Route10_EventScript_845A497::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8379, 0, Kanto_Route10_Text_846F0F5, Kanto_Route10_Text_8434EF4
	msgbox Kanto_Route10_Text_8434F27, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route10 object 4 (local id 4, gfx 0x3F0C) at 4,68
Kanto_Route10_EventScript_845A4AE::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8380, 0, Kanto_Route10_Text_8435071, Kanto_Route10_Text_8435095
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route10_EventScript_845A4D5
	msgbox Kanto_Route10_Text_84350AB, MSGBOX_AUTOCLOSE
	end

Kanto_Route10_EventScript_845A4D5::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8380, 0, Kanto_Route10_Text_846F179, Kanto_Route10_Text_8435095
	msgbox Kanto_Route10_Text_84350AB, MSGBOX_AUTOCLOSE
	end

Kanto_Route10_Text_8434E51::
	.string "Wow, you came all the way here?\n"
	.string "Maybe you’re a POKéMANIAC, too?\l"
	.string "Want to see my collection?$"

Kanto_Route10_Text_8434EAC::
	.string "Humph.\n"
	.string "I’m not angry!$"

Kanto_Route10_Text_8434EC2::
	.string "I have more rare POKéMON at home!$"

Kanto_Route10_Text_8434EE4::
	.string "Ha-hahah-ah-ha!$"

Kanto_Route10_Text_8434EF4::
	.string "Ha-haha!\n"
	.string "Not laughing!\l"
	.string "Ha-hay fever! Haha-ha-choo!$"

Kanto_Route10_Text_8434F27::
	.string "Haha-ha-choo!\n"
	.string "Ha-choo!\l"
	.string "Snort! Snivel!$"

Kanto_Route10_Text_8434F4D::
	.string "Hi, kid!\n"
	.string "Want to see my POKéMON?$"

Kanto_Route10_Text_8434F6E::
	.string "Oh, no!\n"
	.string "My POKéMON!$"

Kanto_Route10_Text_8434F82::
	.string "I don’t like you.\n"
	.string "I don’t like anyone better than me!$"

Kanto_Route10_Text_8434FB8::
	.string "I’ve been out at a POKéMON GYM\n"
	.string "a few times.\p"
	.string "…But I always lose.$"

Kanto_Route10_Text_8434FF8::
	.string "Ohh!\n"
	.string "After all my training!$"

Kanto_Route10_Text_8435014::
	.string "I noticed some POKéMANIACS\n"
	.string "prowling around.\p"
	.string "Can you imagine? Them?\n"
	.string "Up here in the mountains?$"

Kanto_Route10_Text_8435071::
	.string "Ah!\n"
	.string "This mountain air is delicious!$"

Kanto_Route10_Text_8435095::
	.string "That cleared my head!$"

Kanto_Route10_Text_84350AB::
	.string "I feel bloated on mountain air!$"

Kanto_Route10_Text_84350CB::
	.string "I’m feeling a bit faint.\n"
	.string "I haven’t hiked in some time.$"

Kanto_Route10_Text_8435102::
	.string "I’m too tired.\n"
	.string "I wasn’t up for it.$"

Kanto_Route10_Text_8435125::
	.string "The POKéMON here in the mountains\n"
	.string "are so chunky…\p"
	.string "I wish there were pink POKéMON\n"
	.string "with a floral pattern!$"

Kanto_Route10_Text_843518C::
	.string "ROCK TUNNEL\n"
	.string "Detour to LAVENDER TOWN$"

Kanto_Route10_Text_84351B0::
	.string "ROCK TUNNEL$"

Kanto_Route10_Text_84351BC::
	.string "POWER PLANT$"

Kanto_Route10_Text_846F0A0::
	.string "Wow, you came here again?\n"
	.string "Maybe you’re a POKéMANIAC, too?\l"
	.string "Want to see my collection?$"

Kanto_Route10_Text_846F0F5::
	.string "Ha-hahah-ah-ha!\n"
	.string "I can’t stop sneezing!$"

Kanto_Route10_Text_846F11C::
	.string "Hi, kid!\n"
	.string "Did I show you my POKéMON?$"

Kanto_Route10_Text_846F140::
	.string "I went out to a POKéMON GYM again.\p"
	.string "…But I lost as usual.$"

Kanto_Route10_Text_846F179::
	.string "Ah!\n"
	.string "This mountain air is delicious!\l"
	.string "It’s so good, I can’t leave!$"

Kanto_Route10_Text_846F1BA::
	.string "My head is starting to spin.\n"
	.string "I’ve been hiking for too long…$"
