Sinnoh_OreburghCityGym_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_OreburghCityGym_EventScript_Bg1::
	goto SinnohP_oreburgh_city_gym_3
	end

Sinnoh_OreburghCityGym_EventScript_Bg2::
	goto SinnohP_oreburgh_city_gym_3
	end

Sinnoh_OreburghCityGym_EventScript_Person1::
	trainerbattle_single TRAINER_SINNOH_LEADER_ROARK, Sinnoh_OreburghCityGym_Text_Person1Intro, Sinnoh_OreburghCityGym_Text_Person1Defeat, Sinnoh_OreburghCityGym_EventScript_Person1_Defeated, NO_MUSIC
	goto_if_unset 0x3770, Sinnoh_OreburghCityGym_EventScript_Person1_GiveTm
	msgbox Sinnoh_OreburghCityGym_Text_Person1After, MSGBOX_DEFAULT
	release
	end

Sinnoh_OreburghCityGym_EventScript_Person1_Defeated::
	message Sinnoh_OreburghCityGym_Text_Person1Badge
	waitmessage
	call Common_EventScript_PlayGymBadgeFanfare
	msgbox Sinnoh_OreburghCityGym_Text_Person1BadgeInfo, MSGBOX_DEFAULT
	setflag 0x3320
	goto Sinnoh_OreburghCityGym_EventScript_Person1_GiveTm
	end

Sinnoh_OreburghCityGym_EventScript_Person1_GiveTm::
	giveitem ITEM_TM76
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3770
	msgbox Sinnoh_OreburghCityGym_Text_Person1Tm, MSGBOX_DEFAULT
	release
	end

Sinnoh_OreburghCityGym_EventScript_Person2::
	goto SinnohP_oreburgh_city_gym_2
	end

Sinnoh_OreburghCityGym_Text_Person1Intro:
	.string "Welcome to the OREBURGH GYM! I'm\n"
	.string "ROARK, the LEADER here.\p"
	.string "I spend my mornings in the mine and my\n"
	.string "afternoons in here. Both make you\l"
	.string "tough. Let's see how tough you are!$"

Sinnoh_OreburghCityGym_Text_Person1Defeat:
	.string "Crushed... like gravel!$"

Sinnoh_OreburghCityGym_Text_Person1BadgeInfo:
	.string "That's the COAL BADGE. It's made from\n"
	.string "OREBURGH coal, pressed hard. Like you\l"
	.string "pressed me!$"

Sinnoh_OreburghCityGym_Text_Person1Tm:
	.string "Take this too. STEALTH ROCK scatters\n"
	.string "sharp stones that hurt every MOéMON\l"
	.string "your foe sends out.$"

Sinnoh_OreburghCityGym_Text_Person1After:
	.string "The mine runs right under the city.\n"
	.string "Every rock down there has a story.\l"
	.string "Your battle's one of them now.$"

Sinnoh_OreburghCityGym_Text_Person1Badge:
	.string "{PLAYER} received the COAL BADGE\n"
	.string "from ROARK.$"

@ --- end of SINNOH words ---
