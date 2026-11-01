Sinnoh_VeilstoneCityGym_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_VeilstoneCityGym_EventScript_Bg1::
	msgbox Sinnoh_VeilstoneCityGym_Text_Bg1, MSGBOX_SIGN
	end

Sinnoh_VeilstoneCityGym_EventScript_Bg2::
	msgbox Sinnoh_VeilstoneCityGym_Text_Bg2, MSGBOX_SIGN
	end

Sinnoh_VeilstoneCityGym_EventScript_Bg3::
	msgbox Sinnoh_VeilstoneCityGym_Text_Bg3, MSGBOX_SIGN
	end

Sinnoh_VeilstoneCityGym_EventScript_Bg4::
	msgbox Sinnoh_VeilstoneCityGym_Text_Bg4, MSGBOX_SIGN
	end

Sinnoh_VeilstoneCityGym_EventScript_Bg5::
	msgbox Sinnoh_VeilstoneCityGym_Text_Bg5, MSGBOX_SIGN
	end

Sinnoh_VeilstoneCityGym_EventScript_Person1::
	trainerbattle_single TRAINER_SINNOH_LEADER_MAYLENE, Sinnoh_VeilstoneCityGym_Text_Person1Intro, Sinnoh_VeilstoneCityGym_Text_Person1Defeat, Sinnoh_VeilstoneCityGym_EventScript_Person1_Defeated, NO_MUSIC
	goto_if_unset 0x3772, Sinnoh_VeilstoneCityGym_EventScript_Person1_GiveTm
	msgbox Sinnoh_VeilstoneCityGym_Text_Person1After, MSGBOX_DEFAULT
	release
	end

Sinnoh_VeilstoneCityGym_EventScript_Person1_Defeated::
	message Sinnoh_VeilstoneCityGym_Text_Person1Badge
	waitmessage
	call Common_EventScript_PlayGymBadgeFanfare
	msgbox Sinnoh_VeilstoneCityGym_Text_Person1BadgeInfo, MSGBOX_DEFAULT
	setflag 0x3322
	goto Sinnoh_VeilstoneCityGym_EventScript_Person1_GiveTm
	end

Sinnoh_VeilstoneCityGym_EventScript_Person1_GiveTm::
	giveitem ITEM_TM60
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3772
	msgbox Sinnoh_VeilstoneCityGym_Text_Person1Tm, MSGBOX_DEFAULT
	release
	end

Sinnoh_VeilstoneCityGym_EventScript_Person2::
	msgbox Sinnoh_VeilstoneCityGym_Text_Person2, MSGBOX_NPC
	end

Sinnoh_VeilstoneCityGym_Text_Bg1:
	.string "VEILSTONE CITY MOéMON GYM\p"
	.string "LEADER: MAYLENE$"

Sinnoh_VeilstoneCityGym_Text_Bg2:
	.string "VEILSTONE CITY MOéMON GYM\p"
	.string "LEADER: MAYLENE$"

Sinnoh_VeilstoneCityGym_Text_Bg3:
	.string "A poster of a TRAINER striking a pose.\n"
	.string "It's signed!$"

Sinnoh_VeilstoneCityGym_Text_Bg4:
	.string "A poster of a TRAINER striking a pose.\n"
	.string "It's signed!$"

Sinnoh_VeilstoneCityGym_Text_Bg5:
	.string "A poster of a TRAINER striking a pose.\n"
	.string "It's signed!$"

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

Sinnoh_VeilstoneCityGym_Text_Person2:
	.string "MAYLENE's FIGHTING MOéMON hit hard.\n"
	.string "FLYING and PSYCHIC moves will keep\l"
	.string "you on your feet!$"

@ --- end of SINNOH words ---
