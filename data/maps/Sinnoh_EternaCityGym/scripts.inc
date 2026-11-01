Sinnoh_EternaCityGym_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_EternaCityGym_EventScript_Bg1::
	msgbox Sinnoh_EternaCityGym_Text_Bg1, MSGBOX_SIGN
	end

Sinnoh_EternaCityGym_EventScript_Bg2::
	msgbox Sinnoh_EternaCityGym_Text_Bg2, MSGBOX_SIGN
	end

Sinnoh_EternaCityGym_EventScript_Person1::
	msgbox Sinnoh_EternaCityGym_Text_Person1, MSGBOX_NPC
	end

Sinnoh_EternaCityGym_EventScript_Person2::
	trainerbattle_single TRAINER_SINNOH_LEADER_GARDENIA, Sinnoh_EternaCityGym_Text_Person2Intro, Sinnoh_EternaCityGym_Text_Person2Defeat, Sinnoh_EternaCityGym_EventScript_Person2_Defeated, NO_MUSIC
	goto_if_unset 0x3771, Sinnoh_EternaCityGym_EventScript_Person2_GiveTm
	msgbox Sinnoh_EternaCityGym_Text_Person2After, MSGBOX_DEFAULT
	release
	end

Sinnoh_EternaCityGym_EventScript_Person2_Defeated::
	message Sinnoh_EternaCityGym_Text_Person2Badge
	waitmessage
	call Common_EventScript_PlayGymBadgeFanfare
	msgbox Sinnoh_EternaCityGym_Text_Person2BadgeInfo, MSGBOX_DEFAULT
	setflag 0x3321
	goto Sinnoh_EternaCityGym_EventScript_Person2_GiveTm
	end

Sinnoh_EternaCityGym_EventScript_Person2_GiveTm::
	giveitem ITEM_TM86
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3771
	msgbox Sinnoh_EternaCityGym_Text_Person2Tm, MSGBOX_DEFAULT
	release
	end

Sinnoh_EternaCityGym_Text_Bg1:
	.string "ETERNA CITY MOéMON GYM\p"
	.string "LEADER: GARDENIA$"

Sinnoh_EternaCityGym_Text_Bg2:
	.string "ETERNA CITY MOéMON GYM\p"
	.string "LEADER: GARDENIA$"

Sinnoh_EternaCityGym_Text_Person1:
	.string "GARDENIA's MOéMON love the sun. FIRE,\n"
	.string "FLYING, ICE and BUG moves will help\l"
	.string "you thin out her garden!$"

Sinnoh_EternaCityGym_Text_Person2Intro:
	.string "Hi there! I'm GARDENIA. I grow MOéMON\n"
	.string "the way I grow flowers: with sunshine\l"
	.string "and a little stubbornness!\p"
	.string "Ready to see what's blooming?$"

Sinnoh_EternaCityGym_Text_Person2Defeat:
	.string "Wilted... but in a nice way.$"

Sinnoh_EternaCityGym_Text_Person2BadgeInfo:
	.string "The FOREST BADGE! It's shaped like a\n"
	.string "leaf from the oldest tree in ETERNA\l"
	.string "FOREST.$"

Sinnoh_EternaCityGym_Text_Person2Tm:
	.string "And this is GRASS KNOT. The heavier\n"
	.string "your foe, the harder it trips. Big\l"
	.string "MOéMON hate it!$"

Sinnoh_EternaCityGym_Text_Person2After:
	.string "Have you walked through ETERNA\n"
	.string "FOREST? It's spooky, but I love it. The\l"
	.string "trees there are older than the city.$"

Sinnoh_EternaCityGym_Text_Person2Badge:
	.string "{PLAYER} received the FOREST BADGE\n"
	.string "from GARDENIA.$"

@ --- end of SINNOH words ---
