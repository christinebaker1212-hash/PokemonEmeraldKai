Sinnoh_PastoriaCityGym_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_PastoriaCityGym_EventScript_Bg1::
	msgbox Sinnoh_PastoriaCityGym_Text_Bg1, MSGBOX_SIGN
	end

Sinnoh_PastoriaCityGym_EventScript_Bg2::
	msgbox Sinnoh_PastoriaCityGym_Text_Bg2, MSGBOX_SIGN
	end

Sinnoh_PastoriaCityGym_EventScript_Person1::
	trainerbattle_single TRAINER_SINNOH_LEADER_WAKE, Sinnoh_PastoriaCityGym_Text_Person1Intro, Sinnoh_PastoriaCityGym_Text_Person1Defeat, Sinnoh_PastoriaCityGym_EventScript_Person1_Defeated, NO_MUSIC
	goto_if_unset 0x3774, Sinnoh_PastoriaCityGym_EventScript_Person1_GiveTm
	msgbox Sinnoh_PastoriaCityGym_Text_Person1After, MSGBOX_DEFAULT
	release
	end

Sinnoh_PastoriaCityGym_EventScript_Person1_Defeated::
	message Sinnoh_PastoriaCityGym_Text_Person1Badge
	waitmessage
	call Common_EventScript_PlayGymBadgeFanfare
	msgbox Sinnoh_PastoriaCityGym_Text_Person1BadgeInfo, MSGBOX_DEFAULT
	setflag 0x3324
	goto Sinnoh_PastoriaCityGym_EventScript_Person1_GiveTm
	end

Sinnoh_PastoriaCityGym_EventScript_Person1_GiveTm::
	giveitem ITEM_TM55
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3774
	msgbox Sinnoh_PastoriaCityGym_Text_Person1Tm, MSGBOX_DEFAULT
	release
	end

Sinnoh_PastoriaCityGym_EventScript_Person2::
	msgbox Sinnoh_PastoriaCityGym_Text_Person2, MSGBOX_NPC
	end

Sinnoh_PastoriaCityGym_Text_Bg1:
	.string "PASTORIA CITY MOéMON GYM\p"
	.string "LEADER: CRASHER WAKE$"

Sinnoh_PastoriaCityGym_Text_Bg2:
	.string "PASTORIA CITY MOéMON GYM\p"
	.string "LEADER: CRASHER WAKE$"

Sinnoh_PastoriaCityGym_Text_Person1Intro:
	.string "I am CRASHER WAKE! The wave that\n"
	.string "crashes over every challenger!\p"
	.string "Can you ride it? Let's find out!\n"
	.string "WAHAHA!$"

Sinnoh_PastoriaCityGym_Text_Person1Defeat:
	.string "The wave crashed over me!$"

Sinnoh_PastoriaCityGym_Text_Person1BadgeInfo:
	.string "The FEN BADGE! Shaped like a raindrop!\n"
	.string "It's slippery, so don't drop it in the\l"
	.string "marsh! WAHAHA!$"

Sinnoh_PastoriaCityGym_Text_Person1Tm:
	.string "And this! BRINE: it hits hardest when\n"
	.string "your foe is already half worn out. Like\l"
	.string "me right now!$"

Sinnoh_PastoriaCityGym_Text_Person1After:
	.string "Wahaha! What a match! I'm going to the\n"
	.string "GREAT MARSH to cool off. Come splash\l"
	.string "around some time!$"

Sinnoh_PastoriaCityGym_Text_Person1Badge:
	.string "{PLAYER} received the FEN BADGE from\n"
	.string "WAKE.$"

Sinnoh_PastoriaCityGym_Text_Person2:
	.string "CRASHER WAKE's WATER MOéMON make big\n"
	.string "waves. GRASS and ELECTRIC moves will\l"
	.string "calm them down!$"

@ --- end of SINNOH words ---
