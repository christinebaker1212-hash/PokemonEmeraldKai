Johto_EarlsPokemonAcademy_MapScripts::
	.byte 0

@ Johto_EarlsPokemonAcademy object 5 (local id 5, gfx 0x3EB7) at 6,3
Johto_EarlsPokemonAcademy_EventScript_84002F6::
	@ Moemon Kai: Crystal's academy (pokecrystal maps/EarlsPokemonAcademy.asm; docs/REGIONS_AUDIT.md 52): EARL's two
	@ lessons, the students, the status blackboard and the notebook
	lock
	faceplayer
	msgbox Johto_EarlsPokemonAcademy_Text_EarlIntro, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Johto_EarlsPokemonAcademy_EventScript_EarlPart1
	msgbox Johto_EarlsPokemonAcademy_Text_EarlHowToWin, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Johto_EarlsPokemonAcademy_EventScript_EarlDone
Johto_EarlsPokemonAcademy_EventScript_EarlPart1::
	msgbox Johto_EarlsPokemonAcademy_Text_EarlTeachMore, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Johto_EarlsPokemonAcademy_EventScript_EarlDone
	msgbox Johto_EarlsPokemonAcademy_Text_EarlRaiseWell, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_EarlsPokemonAcademy_EventScript_EarlDone::
	msgbox Johto_EarlsPokemonAcademy_Text_EarlNoMore, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_EarlsPokemonAcademy_EventScript_Notebook::
	lockall
	msgbox Johto_EarlsPokemonAcademy_Text_Notebook, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Johto_EarlsPokemonAcademy_EventScript_NotebookDone
	msgbox Johto_EarlsPokemonAcademy_Text_Notebook1, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Johto_EarlsPokemonAcademy_EventScript_NotebookDone
	msgbox Johto_EarlsPokemonAcademy_Text_Notebook2, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Johto_EarlsPokemonAcademy_EventScript_NotebookDone
	msgbox Johto_EarlsPokemonAcademy_Text_Notebook3, MSGBOX_DEFAULT
Johto_EarlsPokemonAcademy_EventScript_NotebookDone::
	closemessage
	releaseall
	end

@ Johto_EarlsPokemonAcademy object 6 (local id 6, gfx 0x3EB3) at 4,3
Johto_EarlsPokemonAcademy_EventScript_8400303::
	lock
	faceplayer
	msgbox Johto_EarlsPokemonAcademy_Text_840059E, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_EarlsPokemonAcademy object 1 (local id 1, gfx 0x3E96) at 3,16
Johto_EarlsPokemonAcademy_EventScript_8400310::
	lock
	faceplayer
	msgbox Johto_EarlsPokemonAcademy_Text_8400999, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_EarlsPokemonAcademy object 2 (local id 2, gfx 0x3E96) at 5,16
Johto_EarlsPokemonAcademy_EventScript_840031D::
	lock
	faceplayer
	msgbox Johto_EarlsPokemonAcademy_Text_8400A0C, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_EarlsPokemonAcademy object 4 (local id 4, gfx 0xB) at 3,7
Johto_EarlsPokemonAcademy_EventScript_840032A::
	lock
	faceplayer
	msgbox Johto_EarlsPokemonAcademy_Text_8400882, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_EarlsPokemonAcademy object 3 (local id 3, gfx 0x10) at 6,10
Johto_EarlsPokemonAcademy_EventScript_8400337::
	lock
	faceplayer
	msgbox Johto_EarlsPokemonAcademy_Text_8400AC3, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_EarlsPokemonAcademy object 7 (local id 7, gfx 0x6) at 4,13
Johto_EarlsPokemonAcademy_EventScript_8400344::
	msgbox Johto_EarlsPokemonAcademy_Text_8400359, MSGBOX_NPC
	end

@ Johto_EarlsPokemonAcademy sign / bg event at 4,1
@ Johto_EarlsPokemonAcademy sign / bg event at 5,1
Johto_EarlsPokemonAcademy_EventScript_840034D::
	lockall
Johto_EarlsPokemonAcademy_EventScript_BlackboardLoop::
	message Johto_EarlsPokemonAcademy_Text_8400B3A
	waitmessage
	multichoicegrid 1, 1, MULTI_KAI_ACADEMY_BLACKBOARD, 2, FALSE
	switch VAR_RESULT
	case 0, Johto_EarlsPokemonAcademy_EventScript_Poison
	case 1, Johto_EarlsPokemonAcademy_EventScript_Paralysis
	case 2, Johto_EarlsPokemonAcademy_EventScript_Sleep
	case 3, Johto_EarlsPokemonAcademy_EventScript_Burn
	case 4, Johto_EarlsPokemonAcademy_EventScript_Freeze
	closemessage
	releaseall
	end

Johto_EarlsPokemonAcademy_EventScript_Poison::
	msgbox Johto_EarlsPokemonAcademy_Text_Poison, MSGBOX_DEFAULT
	goto Johto_EarlsPokemonAcademy_EventScript_BlackboardLoop

Johto_EarlsPokemonAcademy_EventScript_Paralysis::
	msgbox Johto_EarlsPokemonAcademy_Text_Paralysis, MSGBOX_DEFAULT
	goto Johto_EarlsPokemonAcademy_EventScript_BlackboardLoop

Johto_EarlsPokemonAcademy_EventScript_Sleep::
	msgbox Johto_EarlsPokemonAcademy_Text_Sleep, MSGBOX_DEFAULT
	goto Johto_EarlsPokemonAcademy_EventScript_BlackboardLoop

Johto_EarlsPokemonAcademy_EventScript_Burn::
	msgbox Johto_EarlsPokemonAcademy_Text_Burn, MSGBOX_DEFAULT
	goto Johto_EarlsPokemonAcademy_EventScript_BlackboardLoop

Johto_EarlsPokemonAcademy_EventScript_Freeze::
	msgbox Johto_EarlsPokemonAcademy_Text_Freeze, MSGBOX_DEFAULT
	goto Johto_EarlsPokemonAcademy_EventScript_BlackboardLoop

Johto_EarlsPokemonAcademy_Text_8400359::
	.string "I’m not a GBA kid, but I love\n"
	.string "playing MOéMON games!\p"
	.string "In the old games, you had to battle\n"
	.string "a lot of wild MOéMON to get enough\l"
	.string "EXP.\p"
	.string "But in newer games, the EXP. SHARE\n"
	.string "makes your MOéMON level up too fast.\p"
	.string "In this game, the EXP. SHARE is\n"
	.string "balanced to put your MOéMON at just\l"
	.string "the right level. Or slightly higher,\l"
	.string "depending on how many battles you do.\p"
	.string "If you want a traditional GEN 1-4\n"
	.string "experience, turn EXP. SHARE off.\l"
	.string "If you want a modern experience,\l"
	.string "turn EXP. SHARE ON.\p"
	.string "You choose how you want to play!$"

Johto_EarlsPokemonAcademy_Text_840059E::
	.string "Certain MOéMON start off weak and\n"
	.string "require much more EXP to become\l"
	.string "strong than others do.\p"
	.string "For example, LARVITAR is quite weak\n"
	.string "but will eventually become a\l"
	.string "mighty TYRANITAR, at the cost of\l"
	.string "much EXP.\p"
	.string "Only the most dedicated trainers\n"
	.string "should attempt to train such a\l"
	.string "MOéMON.$"

Johto_EarlsPokemonAcademy_Text_8400882::
	.string "I’m taking notes of the teacher’s\n"
	.string "lecture.\p"
	.string "I’d better copy the stuff on the\n"
	.string "blackboard too.$"

Johto_EarlsPokemonAcademy_Text_8400999::
	.string "I traded my best MOéMON to the guy\n"
	.string "beside me.$"

Johto_EarlsPokemonAcademy_Text_8400A0C::
	.string "Huh? The MOéMON I just got is\n"
	.string "holding something!$"

Johto_EarlsPokemonAcademy_Text_8400AC3::
	.string "A MOéMON holding a BERRY will heal\n"
	.string "itself in battle.\p"
	.string "Many other items can be held by\n"
	.string "MOéMON…\p"
	.string "It sure is tough taking notes…$"

Johto_EarlsPokemonAcademy_Text_8400B3A::
	.string "The blackboard describes MOéMON\n"
	.string "status changes in battle.$"

Johto_EarlsPokemonAcademy_Text_EarlIntro::
	.string "EARL, I am!\p"
	.string "Wonderful are MOéMON, yes!\p"
	.string "Teach you I will to be a better\n"
	.string "trainer!\p"
	.string "What you want to know? Want to be\n"
	.string "a winner is you?$"

Johto_EarlsPokemonAcademy_Text_EarlHowToWin::
	.string "Good! Teach you, I will!\p"
	.string "In battle, MOéMON top on list jump\n"
	.string "out first!\p"
	.string "Change order in list, make battle\n"
	.string "easy, maybe!\p"
	.string "More from me you want to hear?$"

Johto_EarlsPokemonAcademy_Text_EarlTeachMore::
	.string "So, want to know how to raise\n"
	.string "MOéMON well?$"

Johto_EarlsPokemonAcademy_Text_EarlRaiseWell::
	.string "Fine! Teach you, I will!\p"
	.string "If MOéMON come out in battle even\n"
	.string "briefly, some EXP. Points it gets.\p"
	.string "At top of list put weak MOéMON.\p"
	.string "Switch in battle quick!\p"
	.string "This way, weak MOéMON strong\n"
	.string "become!$"

Johto_EarlsPokemonAcademy_Text_EarlNoMore::
	.string "Oh! Smart student you are! Nothing\n"
	.string "more do I teach!\p"
	.string "Good to MOéMON you must be!$"

Johto_EarlsPokemonAcademy_Text_Notebook::
	.string "It’s this kid’s notebook…\p"
	.string "Catch MOéMON using POKé BALLS.\p"
	.string "Up to six can be in your party.\p"
	.string "Keep reading?$"

Johto_EarlsPokemonAcademy_Text_Notebook1::
	.string "Before throwing a POKé BALL,\n"
	.string "weaken the target first.\p"
	.string "A poisoned or burned MOéMON is\n"
	.string "easier to catch.\p"
	.string "Keep reading?$"

Johto_EarlsPokemonAcademy_Text_Notebook2::
	.string "Some moves may cause confusion.\p"
	.string "Confusion may make a MOéMON attack\n"
	.string "itself.\p"
	.string "Leaving battle clears up any\n"
	.string "confusion.\p"
	.string "Keep reading?$"

Johto_EarlsPokemonAcademy_Text_Notebook3::
	.string "People who catch and use MOéMON in\n"
	.string "battle are MOéMON trainers.\p"
	.string "They are expected to visit MOéMON\n"
	.string "GYMS and defeat other trainers.\p"
	.string "The next page is… Blank!\p"
	.string "Boy: E-he-he…\p"
	.string "I haven’t written anymore…$"

Johto_EarlsPokemonAcademy_Text_Poison::
	.string "If poisoned, a MOéMON steadily\n"
	.string "loses HP.\p"
	.string "Poison lingers after the battle,\n"
	.string "and HP is lost as you walk.\p"
	.string "To cure it, use an ANTIDOTE.$"

Johto_EarlsPokemonAcademy_Text_Paralysis::
	.string "Paralysis reduces speed and may\n"
	.string "prevent movement.\p"
	.string "It remains after battle, so use a\n"
	.string "PARLYZ HEAL.$"

Johto_EarlsPokemonAcademy_Text_Sleep::
	.string "If asleep, your MOéMON can’t make\n"
	.string "a move.\p"
	.string "A sleeping MOéMON doesn’t wake up\n"
	.string "after battle.\p"
	.string "Wake it up with an AWAKENING.$"

Johto_EarlsPokemonAcademy_Text_Burn::
	.string "A burn steadily consumes HP.\p"
	.string "It also reduces attack power.\p"
	.string "A burn lingers after battle.\p"
	.string "Use a BURN HEAL as the cure.$"

Johto_EarlsPokemonAcademy_Text_Freeze::
	.string "If your MOéMON is frozen, it can’t\n"
	.string "do a thing.\p"
	.string "It remains frozen after battle.\p"
	.string "Thaw it out with an ICE HEAL.$"
