Sinnoh_SnowpointCityGym_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_SnowpointCityGym_EventScript_Person1::
	trainerbattle_single TRAINER_SINNOH_LEADER_CANDICE, Sinnoh_SnowpointCityGym_Text_Person1Intro, Sinnoh_SnowpointCityGym_Text_Person1Defeat, Sinnoh_SnowpointCityGym_EventScript_Person1_Defeated, NO_MUSIC
	goto_if_unset 0x3776, Sinnoh_SnowpointCityGym_EventScript_Person1_GiveTm
	msgbox Sinnoh_SnowpointCityGym_Text_Person1After, MSGBOX_DEFAULT
	release
	end

Sinnoh_SnowpointCityGym_EventScript_Person1_Defeated::
	message Sinnoh_SnowpointCityGym_Text_Person1Badge
	waitmessage
	call Common_EventScript_PlayGymBadgeFanfare
	msgbox Sinnoh_SnowpointCityGym_Text_Person1BadgeInfo, MSGBOX_DEFAULT
	setflag 0x3326
	goto Sinnoh_SnowpointCityGym_EventScript_Person1_GiveTm
	end

Sinnoh_SnowpointCityGym_EventScript_Person1_GiveTm::
	giveitem ITEM_TM72
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3776
	msgbox Sinnoh_SnowpointCityGym_Text_Person1Tm, MSGBOX_DEFAULT
	release
	end

Sinnoh_SnowpointCityGym_Text_Person1Intro:
	.string "I'm CANDICE! In SNOWPOINT we say it's\n"
	.string "all about focus. Diamond-hard focus!\p"
	.string "And right now I'm focused on beating\n"
	.string "you!$"

Sinnoh_SnowpointCityGym_Text_Person1Defeat:
	.string "Brr... I lost my focus!$"

Sinnoh_SnowpointCityGym_Text_Person1BadgeInfo:
	.string "Here's the ICICLE BADGE! Don't worry,\n"
	.string "it never melts.$"

Sinnoh_SnowpointCityGym_Text_Person1Tm:
	.string "AVALANCHE is yours too. Get hit first,\n"
	.string "then hit back twice as hard. Cool,\l"
	.string "right?$"

Sinnoh_SnowpointCityGym_Text_Person1After:
	.string "Your focus is amazing! Come back when\n"
	.string "it's snowing. That's when SNOWPOINT is\l"
	.string "prettiest.$"

Sinnoh_SnowpointCityGym_Text_Person1Badge:
	.string "{PLAYER} received the ICICLE BADGE\n"
	.string "from CANDICE.$"

@ --- end of SINNOH words ---
