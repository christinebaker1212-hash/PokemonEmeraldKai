Sinnoh_HearthomeCityGymLeaderRoom_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_HearthomeCityGymLeaderRoom_EventScript_Person1::
	trainerbattle_single TRAINER_SINNOH_LEADER_FANTINA, Sinnoh_HearthomeCityGymLeaderRoom_Text_Person1Intro, Sinnoh_HearthomeCityGymLeaderRoom_Text_Person1Defeat, Sinnoh_HearthomeCityGymLeaderRoom_EventScript_Person1_Defeated, NO_MUSIC
	goto_if_unset 0x3774, Sinnoh_HearthomeCityGymLeaderRoom_EventScript_Person1_GiveTm
	msgbox Sinnoh_HearthomeCityGymLeaderRoom_Text_Person1After, MSGBOX_DEFAULT
	release
	end

Sinnoh_HearthomeCityGymLeaderRoom_EventScript_Person1_Defeated::
	message Sinnoh_HearthomeCityGymLeaderRoom_Text_Person1Badge
	waitmessage
	call Common_EventScript_PlayGymBadgeFanfare
	msgbox Sinnoh_HearthomeCityGymLeaderRoom_Text_Person1BadgeInfo, MSGBOX_DEFAULT
	setflag 0x3324
	goto Sinnoh_HearthomeCityGymLeaderRoom_EventScript_Person1_GiveTm
	end

Sinnoh_HearthomeCityGymLeaderRoom_EventScript_Person1_GiveTm::
	giveitem ITEM_TM65
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3774
	msgbox Sinnoh_HearthomeCityGymLeaderRoom_Text_Person1Tm, MSGBOX_DEFAULT
	release
	end

Sinnoh_HearthomeCityGymLeaderRoom_Text_Person1Intro:
	.string "Bonjour! I am FANTINA. My GHOST\n"
	.string "MOéMON and I, we dance in the dark!\p"
	.string "Tonight you will dance with us. Allez!$"

Sinnoh_HearthomeCityGymLeaderRoom_Text_Person1Defeat:
	.string "Ah... the music stops.$"

Sinnoh_HearthomeCityGymLeaderRoom_Text_Person1BadgeInfo:
	.string "The RELIC BADGE is yours. It is very\n"
	.string "old, and very lovely. Like my GHOSTS.$"

Sinnoh_HearthomeCityGymLeaderRoom_Text_Person1Tm:
	.string "Take this as well. SHADOW CLAW: a quick\n"
	.string "slash from the shadows that finds the\l"
	.string "weak spot.$"

Sinnoh_HearthomeCityGymLeaderRoom_Text_Person1After:
	.string "You dance well in battle. Come to the\n"
	.string "CONTEST HALL some time, yes? We dance\l"
	.string "there too.$"

Sinnoh_HearthomeCityGymLeaderRoom_Text_Person1Badge:
	.string "{PLAYER} received the RELIC BADGE\n"
	.string "from FANTINA.$"

@ --- end of SINNOH words ---
