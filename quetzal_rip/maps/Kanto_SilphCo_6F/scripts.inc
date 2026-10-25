Kanto_SilphCo_6F_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Kanto_SilphCo_6F_EventScript_84115DD
	.byte 0

Kanto_SilphCo_6F_EventScript_84115DD::
	call_if_unset FLAG_QZ_0x21FF, Kanto_SilphCo_6F_EventScript_84587D7
	end

@ Kanto_SilphCo_6F object 4 (local id 4, gfx 0x3F5B) at 22,13
Kanto_SilphCo_6F_EventScript_84115E7::
	lock
	faceplayer
	goto_if_ge VAR_QZ_0x512D, 1, Kanto_SilphCo_6F_EventScript_84115FE
	msgbox Kanto_SilphCo_6F_Text_842675F, MSGBOX_DEFAULT
	release
	end

Kanto_SilphCo_6F_EventScript_84115FE::
	msgbox Kanto_SilphCo_6F_Text_8426797, MSGBOX_DEFAULT
	release
	end

@ Kanto_SilphCo_6F object 5 (local id 5, gfx 0x3F5B) at 14,9
Kanto_SilphCo_6F_EventScript_8411608::
	lock
	faceplayer
	goto_if_ge VAR_QZ_0x512D, 1, Kanto_SilphCo_6F_EventScript_841161F
	msgbox Kanto_SilphCo_6F_Text_8426605, MSGBOX_DEFAULT
	release
	end

Kanto_SilphCo_6F_EventScript_841161F::
	msgbox Kanto_SilphCo_6F_Text_8426632, MSGBOX_DEFAULT
	release
	end

@ Kanto_SilphCo_6F object 2 (local id 2, gfx 0x3F5B) at 23,9
Kanto_SilphCo_6F_EventScript_8411629::
	lock
	faceplayer
	goto_if_ge VAR_QZ_0x512D, 1, Kanto_SilphCo_6F_EventScript_8411640
	msgbox Kanto_SilphCo_6F_Text_8426651, MSGBOX_DEFAULT
	release
	end

Kanto_SilphCo_6F_EventScript_8411640::
	msgbox Kanto_SilphCo_6F_Text_8426674, MSGBOX_DEFAULT
	release
	end

@ Kanto_SilphCo_6F object 3 (local id 3, gfx 0x3F5A) at 24,9
Kanto_SilphCo_6F_EventScript_841164A::
	lock
	faceplayer
	goto_if_ge VAR_QZ_0x512D, 1, Kanto_SilphCo_6F_EventScript_8411661
	msgbox Kanto_SilphCo_6F_Text_842668B, MSGBOX_DEFAULT
	release
	end

Kanto_SilphCo_6F_EventScript_8411661::
	msgbox Kanto_SilphCo_6F_Text_84266B4, MSGBOX_DEFAULT
	release
	end

@ Kanto_SilphCo_6F object 6 (local id 6, gfx 0x3F5A) at 16,12
Kanto_SilphCo_6F_EventScript_841166B::
	lock
	faceplayer
	goto_if_ge VAR_QZ_0x512D, 1, Kanto_SilphCo_6F_EventScript_8411682
	msgbox Kanto_SilphCo_6F_Text_84266F3, MSGBOX_DEFAULT
	release
	end

Kanto_SilphCo_6F_EventScript_8411682::
	msgbox Kanto_SilphCo_6F_Text_842672C, MSGBOX_DEFAULT
	release
	end

@ Kanto_SilphCo_6F sign / bg event at 25,1
Kanto_SilphCo_6F_EventScript_841168C::
	msgbox Kanto_SilphCo_6F_Text_842692E, MSGBOX_SIGN
	end

@ Kanto_SilphCo_6F object 7 (local id 7, gfx 0x3F43) at 9,11
Kanto_SilphCo_6F_EventScript_8411695::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8533, 0, Kanto_SilphCo_6F_Text_8426817, Kanto_SilphCo_6F_Text_842689E
	msgbox Kanto_SilphCo_6F_Text_84268A5, MSGBOX_AUTOCLOSE
	end

@ Kanto_SilphCo_6F object 1 (local id 1, gfx 0x3F3D) at 21,5
Kanto_SilphCo_6F_EventScript_84116AC::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8572, 0, Kanto_SilphCo_6F_Text_84267BF, Kanto_SilphCo_6F_Text_84267E5
	msgbox Kanto_SilphCo_6F_Text_84267F0, MSGBOX_AUTOCLOSE
	end

@ Kanto_SilphCo_6F object 10 (local id 10, gfx 0x3F3D) at 18,17
Kanto_SilphCo_6F_EventScript_84116C3::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8573, 0, Kanto_SilphCo_6F_Text_84268D2, Kanto_SilphCo_6F_Text_84268EF
	msgbox Kanto_SilphCo_6F_Text_84268FC, MSGBOX_AUTOCLOSE
	end

Kanto_SilphCo_6F_EventScript_84587D7::
	setmetatile 5, 0xE, 0x3C0, 1
	setmetatile 6, 0xE, 0x3C1, 1
	setmetatile 5, 0xF, 0x3C2, 1
	setmetatile 6, 0xF, 0x3C3, 1
	setmetatile 5, 0x10, 0x3C4, 1
	setmetatile 6, 0x10, 0x3C5, 1
	return

@ Kanto_SilphCo_6F sign / bg event at 6,15
@ Kanto_SilphCo_6F sign / bg event at 6,16
@ Kanto_SilphCo_6F sign / bg event at 5,15
@ Kanto_SilphCo_6F sign / bg event at 5,16
Kanto_SilphCo_6F_EventScript_8458EC8::
	lockall
	setvar VAR_TEMP_1, 0xA
	setvar VAR_0x8004, 0x21FF
	goto_if_set FLAG_QZ_0x21FF, Quetzal_EventScript_8458602
	goto Quetzal_EventScript_8458FE6

@ Kanto_SilphCo_6F object 8 (local id 8, gfx 0x3F0E) at 2,14
Kanto_SilphCo_6F_EventScript_846BD97::
	setorcopyvar VAR_0x8000, 0x41
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_SilphCo_6F object 9 (local id 9, gfx 0x3F0E) at 1,17
Kanto_SilphCo_6F_EventScript_846BDA4::
	setorcopyvar VAR_0x8000, 0x7B
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_SilphCo_6F_Text_8426605::
	.string "The ROCKETS came and took over\n"
	.string "the building!$"

Kanto_SilphCo_6F_Text_8426632::
	.string "Well, better get back to work.$"

Kanto_SilphCo_6F_Text_8426651::
	.string "Oh dear, oh dear.\n"
	.string "Help me, please!$"

Kanto_SilphCo_6F_Text_8426674::
	.string "We got engaged.\n"
	.string "Heheh!$"

Kanto_SilphCo_6F_Text_842668B::
	.string "That man next to me…\n"
	.string "He’s such a coward!$"

Kanto_SilphCo_6F_Text_84266B4::
	.string "He’s so helpless, he needed\n"
	.string "someone like me to look after him.$"

Kanto_SilphCo_6F_Text_84266F3::
	.string "TEAM ROCKET is trying to conquer\n"
	.string "the world with POKéMON.$"

Kanto_SilphCo_6F_Text_842672C::
	.string "TEAM ROCKET ran away, and it’s all\n"
	.string "because of you!$"

Kanto_SilphCo_6F_Text_842675F::
	.string "They must have targeted SILPH for\n"
	.string "our POKéMON products.$"

Kanto_SilphCo_6F_Text_8426797::
	.string "Come work for SILPH when you\n"
	.string "get older.$"

Kanto_SilphCo_6F_Text_84267BF::
	.string "I am one of the four ROCKET\n"
	.string "BROTHERS!$"

Kanto_SilphCo_6F_Text_84267E5::
	.string "Flame out!$"

Kanto_SilphCo_6F_Text_84267F0::
	.string "No matter!\n"
	.string "My brothers will avenge me!$"

Kanto_SilphCo_6F_Text_8426817::
	.string "That rotten PRESIDENT!\p"
	.string "Serves him right for shipping me\n"
	.string "off to the TIKSI BRANCH!\p"
	.string "That’s why TEAM ROCKET came after\n"
	.string "us, I’m sure of it!$"

Kanto_SilphCo_6F_Text_842689E::
	.string "Shoot!$"

Kanto_SilphCo_6F_Text_84268A5::
	.string "TIKSI BRANCH?\n"
	.string "It’s in Russian no-man’s-land!$"

Kanto_SilphCo_6F_Text_84268D2::
	.string "You dare betray TEAM ROCKET?$"

Kanto_SilphCo_6F_Text_84268EF::
	.string "You traitor!$"

Kanto_SilphCo_6F_Text_84268FC::
	.string "If you stand for justice, you\n"
	.string "betray us bad guys!$"

Kanto_SilphCo_6F_Text_842692E::
	.string "SILPH CO. HEAD OFFICE\n"
	.string "6F$"
