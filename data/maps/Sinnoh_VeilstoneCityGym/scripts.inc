Sinnoh_VeilstoneCityGym_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_VeilstoneCityGym_EventScript_Person1::
	trainerbattle_single TRAINER_SINNOH_LEADER_MAYLENE, Sinnoh_VeilstoneCityGym_Text_Person1Intro, Sinnoh_VeilstoneCityGym_Text_Person1Defeat, Sinnoh_VeilstoneCityGym_EventScript_Person1_Defeated, NO_MUSIC
	goto_if_unset 0x3773, Sinnoh_VeilstoneCityGym_EventScript_Person1_GiveTm
	msgbox Sinnoh_VeilstoneCityGym_Text_Person1After, MSGBOX_DEFAULT
	release
	end

Sinnoh_VeilstoneCityGym_EventScript_Person1_Defeated::
	message Sinnoh_VeilstoneCityGym_Text_Person1Badge
	waitmessage
	call Common_EventScript_PlayGymBadgeFanfare
	msgbox Sinnoh_VeilstoneCityGym_Text_Person1BadgeInfo, MSGBOX_DEFAULT
	setflag 0x3323
	goto Sinnoh_VeilstoneCityGym_EventScript_Person1_GiveTm
	end

Sinnoh_VeilstoneCityGym_EventScript_Person1_GiveTm::
	giveitem ITEM_TM60
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3773
	msgbox Sinnoh_VeilstoneCityGym_Text_Person1Tm, MSGBOX_DEFAULT
	release
	end

Sinnoh_VeilstoneCityGym_Text_Person1Intro:
	.string "I'm MAYLENE. I've trained here since I\n"
	.string "was small.\p"
	.string "I won't hold anything back. If you're\n"
	.string "ready, so am I!$"

Sinnoh_VeilstoneCityGym_Text_Person1Defeat:
	.string "I'm not strong enough yet...$"

Sinnoh_VeilstoneCityGym_Text_Person1BadgeInfo:
	.string "This is the COBBLE BADGE. Every\n"
	.string "LEADER of this GYM has handed it out\l"
	.string "with pride. Now I have too.$"

Sinnoh_VeilstoneCityGym_Text_Person1Tm:
	.string "Please take this. DRAIN PUNCH takes\n"
	.string "some of the foe's strength for your\l"
	.string "own.$"

Sinnoh_VeilstoneCityGym_Text_Person1After:
	.string "Thank you for that battle. I'm going\n"
	.string "to train even harder. You should too!$"

Sinnoh_VeilstoneCityGym_Text_Person1Badge:
	.string "{PLAYER} received the COBBLE BADGE\n"
	.string "from MAYLENE.$"

@ --- end of SINNOH words ---
