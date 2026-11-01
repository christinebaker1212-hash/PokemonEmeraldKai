Sinnoh_CanalaveCityGym_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_CanalaveCityGym_EventScript_Person5::
	trainerbattle_single TRAINER_SINNOH_LEADER_BYRON, Sinnoh_CanalaveCityGym_Text_Person5Intro, Sinnoh_CanalaveCityGym_Text_Person5Defeat, Sinnoh_CanalaveCityGym_EventScript_Person5_Defeated, NO_MUSIC
	goto_if_unset 0x3775, Sinnoh_CanalaveCityGym_EventScript_Person5_GiveTm
	msgbox Sinnoh_CanalaveCityGym_Text_Person5After, MSGBOX_DEFAULT
	release
	end

Sinnoh_CanalaveCityGym_EventScript_Person5_Defeated::
	message Sinnoh_CanalaveCityGym_Text_Person5Badge
	waitmessage
	call Common_EventScript_PlayGymBadgeFanfare
	msgbox Sinnoh_CanalaveCityGym_Text_Person5BadgeInfo, MSGBOX_DEFAULT
	setflag 0x3325
	goto Sinnoh_CanalaveCityGym_EventScript_Person5_GiveTm
	end

Sinnoh_CanalaveCityGym_EventScript_Person5_GiveTm::
	giveitem ITEM_TM91
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3775
	msgbox Sinnoh_CanalaveCityGym_Text_Person5Tm, MSGBOX_DEFAULT
	release
	end

Sinnoh_CanalaveCityGym_Text_Person5Intro:
	.string "So you're the one who's been climbing\n"
	.string "through SINNOH! I'm BYRON. My STEEL\l"
	.string "MOéMON are as hard as the mountains I\l"
	.string "dig in.\p"
	.string "Show me what you're made of!$"

Sinnoh_CanalaveCityGym_Text_Person5Defeat:
	.string "Cracked my shell, you did!$"

Sinnoh_CanalaveCityGym_Text_Person5BadgeInfo:
	.string "The MINE BADGE. Forged from iron dug\n"
	.string "out of the hills behind CANALAVE.$"

Sinnoh_CanalaveCityGym_Text_Person5Tm:
	.string "And FLASH CANNON: a burst of light\n"
	.string "from a STEEL MOéMON's body. It may\l"
	.string "lower the foe's SP. DEF.$"

Sinnoh_CanalaveCityGym_Text_Person5After:
	.string "My son runs the gym in OREBURGH. Tell\n"
	.string "him his old man got beaten fair and\l"
	.string "square. He'll laugh about it.$"

Sinnoh_CanalaveCityGym_Text_Person5Badge:
	.string "{PLAYER} received the MINE BADGE\n"
	.string "from BYRON.$"

@ --- end of SINNOH words ---
