Sinnoh_SunyshoreCityGymRoom3_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_SunyshoreCityGymRoom3_EventScript_Person1::
	trainerbattle_single TRAINER_SINNOH_LEADER_VOLKNER, Sinnoh_SunyshoreCityGymRoom3_Text_Person1Intro, Sinnoh_SunyshoreCityGymRoom3_Text_Person1Defeat, Sinnoh_SunyshoreCityGymRoom3_EventScript_Person1_Defeated, NO_MUSIC
	goto_if_unset 0x3777, Sinnoh_SunyshoreCityGymRoom3_EventScript_Person1_GiveTm
	msgbox Sinnoh_SunyshoreCityGymRoom3_Text_Person1After, MSGBOX_DEFAULT
	release
	end

Sinnoh_SunyshoreCityGymRoom3_EventScript_Person1_Defeated::
	message Sinnoh_SunyshoreCityGymRoom3_Text_Person1Badge
	waitmessage
	call Common_EventScript_PlayGymBadgeFanfare
	msgbox Sinnoh_SunyshoreCityGymRoom3_Text_Person1BadgeInfo, MSGBOX_DEFAULT
	setflag 0x3327
	goto Sinnoh_SunyshoreCityGymRoom3_EventScript_Person1_GiveTm
	end

Sinnoh_SunyshoreCityGymRoom3_EventScript_Person1_GiveTm::
	giveitem ITEM_TM57
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3777
	msgbox Sinnoh_SunyshoreCityGymRoom3_Text_Person1Tm, MSGBOX_DEFAULT
	release
	end

Sinnoh_SunyshoreCityGymRoom3_Text_Person1Intro:
	.string "I'm VOLKNER. I've been bored a long\n"
	.string "time. Nobody gives me a real battle\l"
	.string "any more.\p"
	.string "Maybe you're different. Light me up.$"

Sinnoh_SunyshoreCityGymRoom3_Text_Person1Defeat:
	.string "...Now that was a battle.$"

Sinnoh_SunyshoreCityGymRoom3_Text_Person1BadgeInfo:
	.string "The BEACON BADGE. It's the last badge\n"
	.string "in SINNOH. Shine bright with it.$"

Sinnoh_SunyshoreCityGymRoom3_Text_Person1Tm:
	.string "Take CHARGE BEAM too. It may raise\n"
	.string "your SP. ATK as it hits.$"

Sinnoh_SunyshoreCityGymRoom3_Text_Person1After:
	.string "Thanks. I remember why I liked\n"
	.string "battling. The MOéMON LEAGUE is past\l"
	.string "VICTORY ROAD. Go show them that\l"
	.string "spark.$"

Sinnoh_SunyshoreCityGymRoom3_Text_Person1Badge:
	.string "{PLAYER} received the BEACON BADGE\n"
	.string "from VOLKNER.$"

@ --- end of SINNOH words ---
