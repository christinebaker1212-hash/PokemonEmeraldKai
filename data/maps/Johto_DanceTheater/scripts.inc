@ Moemon Kai: the DANCE THEATER as Crystal has it (pokecrystal maps/DanceTheater.asm; docs/REGIONS_AUDIT.md 65): five
@ KIMONO GIRLS to battle, and the old man's HM03 SURF once all five are beaten. Quetzal's ROCKET, the girls' story chain
@ and its Lv 48 rematches are gone.
Johto_DanceTheater_MapScripts::
	.byte 0

Johto_DanceTheater_EventScript_Naoko::
	trainerbattle_single TRAINER_QZ_4312, Johto_DanceTheater_Text_NaokoSeen, Johto_DanceTheater_Text_NaokoBeaten
	msgbox Johto_DanceTheater_Text_NaokoAfter, MSGBOX_AUTOCLOSE
	end

Johto_DanceTheater_EventScript_Sayo::
	trainerbattle_single TRAINER_QZ_4098, Johto_DanceTheater_Text_SayoSeen, Johto_DanceTheater_Text_SayoBeaten
	msgbox Johto_DanceTheater_Text_SayoAfter, MSGBOX_AUTOCLOSE
	end

Johto_DanceTheater_EventScript_Zuki::
	trainerbattle_single TRAINER_QZ_4114, Johto_DanceTheater_Text_ZukiSeen, Johto_DanceTheater_Text_ZukiBeaten
	msgbox Johto_DanceTheater_Text_ZukiAfter, MSGBOX_AUTOCLOSE
	end

Johto_DanceTheater_EventScript_Kuni::
	trainerbattle_single TRAINER_QZ_4143, Johto_DanceTheater_Text_KuniSeen, Johto_DanceTheater_Text_KuniBeaten
	msgbox Johto_DanceTheater_Text_KuniAfter, MSGBOX_AUTOCLOSE
	end

Johto_DanceTheater_EventScript_Miki::
	trainerbattle_single TRAINER_QZ_4594, Johto_DanceTheater_Text_MikiSeen, Johto_DanceTheater_Text_MikiBeaten
	msgbox Johto_DanceTheater_Text_MikiAfter, MSGBOX_AUTOCLOSE
	end

Johto_DanceTheater_EventScript_SurfGuy::
	lock
	faceplayer
	msgbox Johto_DanceTheater_Text_SurfGuyNeverLeftAScratch, MSGBOX_DEFAULT
	goto_if_set FLAG_QZ_0x1006, Johto_DanceTheater_EventScript_SurfGuyAlready

	goto_if_not_defeated TRAINER_QZ_4312, Johto_DanceTheater_EventScript_SurfGuyUndefeated

	goto_if_not_defeated TRAINER_QZ_4098, Johto_DanceTheater_EventScript_SurfGuyUndefeated

	goto_if_not_defeated TRAINER_QZ_4114, Johto_DanceTheater_EventScript_SurfGuyUndefeated

	goto_if_not_defeated TRAINER_QZ_4143, Johto_DanceTheater_EventScript_SurfGuyUndefeated

	goto_if_not_defeated TRAINER_QZ_4594, Johto_DanceTheater_EventScript_SurfGuyUndefeated

	msgbox Johto_DanceTheater_Text_SurfGuyLikeADance, MSGBOX_DEFAULT
	setvar VAR_0x8004, ITEM_HM03
	call EventScript_GiveHmOnce
	setflag FLAG_QZ_0x1006
	msgbox Johto_DanceTheater_Text_SurfGuySurfExplanation, MSGBOX_DEFAULT
	release
	end

Johto_DanceTheater_EventScript_SurfGuyUndefeated::
	checkplayergender
	goto_if_eq VAR_RESULT, FEMALE, Johto_DanceTheater_EventScript_SurfGuyLassie
	msgbox Johto_DanceTheater_Text_SurfGuyLadGift, MSGBOX_DEFAULT
	release
	end

Johto_DanceTheater_EventScript_SurfGuyLassie::
	msgbox Johto_DanceTheater_Text_SurfGuyLassieGift, MSGBOX_DEFAULT
	release
	end

Johto_DanceTheater_EventScript_SurfGuyAlready::
	msgbox Johto_DanceTheater_Text_SurfGuyElegantKimonoGirls, MSGBOX_DEFAULT
	release
	end

Johto_DanceTheater_EventScript_CooltrainerM::
	msgbox Johto_DanceTheater_Text_CooltrainerM, MSGBOX_NPC
	end

Johto_DanceTheater_EventScript_Granny::
	msgbox Johto_DanceTheater_Text_Granny, MSGBOX_NPC
	end

Johto_DanceTheater_EventScript_FancyPanel::
	msgbox Johto_DanceTheater_Text_FancyPanel, MSGBOX_SIGN
	end

Johto_DanceTheater_Text_NaokoSeen::
	.string "You have lovely MOéMON. May I see\n"
	.string "them in battle?$"

Johto_DanceTheater_Text_NaokoBeaten::
	.string "Oh, you are very strong.$"

Johto_DanceTheater_Text_NaokoAfter::
	.string "I enjoyed that bout. I would like\n"
	.string "to see you again.$"

Johto_DanceTheater_Text_SayoSeen::
	.string "I always dance with my MOéMON.\p"
	.string "Of course, I also train them.$"

Johto_DanceTheater_Text_SayoBeaten::
	.string "Oh, so close! I almost had you.$"

Johto_DanceTheater_Text_SayoAfter::
	.string "Rhythm is important for both\n"
	.string "dancing and MOéMON.$"

Johto_DanceTheater_Text_ZukiSeen::
	.string "Isn’t my barrette pretty?\p"
	.string "Oh. A MOéMON battle?$"

Johto_DanceTheater_Text_ZukiBeaten::
	.string "I don’t have any MOéMON left…$"

Johto_DanceTheater_Text_ZukiAfter::
	.string "I put a different flower in my\n"
	.string "barrette every month.$"

Johto_DanceTheater_Text_KuniSeen::
	.string "Oh, you’re a cute trainer. Would\n"
	.string "you like to battle?$"

Johto_DanceTheater_Text_KuniBeaten::
	.string "You’re stronger than you look.$"

Johto_DanceTheater_Text_KuniAfter::
	.string "I trained a lot, so I thought I\n"
	.string "was a capable trainer. I guess I’m\l"
	.string "not.$"

Johto_DanceTheater_Text_MikiSeen::
	.string "Do you like my dancing? I’m good\n"
	.string "at MOéMON too.$"

Johto_DanceTheater_Text_MikiBeaten::
	.string "Ooh, you’re good at MOéMON too.$"

Johto_DanceTheater_Text_MikiAfter::
	.string "I can keep dancing because there\n"
	.string "are people who enjoy what I do.\p"
	.string "My MOéMON keep my spirits up too.$"

Johto_DanceTheater_Text_SurfGuyNeverLeftAScratch::
	.string "Not only are the KIMONO GIRLS\n"
	.string "great dancers, they’re also\l"
	.string "skilled at MOéMON.\p"
	.string "I always challenge them, but I’ve\n"
	.string "never even left a scratch…$"

Johto_DanceTheater_Text_SurfGuyLadGift::
	.string "Lad! If you can defeat all the\n"
	.string "KIMONO GIRLS, I’ll give you a\l"
	.string "gift.$"

Johto_DanceTheater_Text_SurfGuyLassieGift::
	.string "Lassie, if you can defeat all the\n"
	.string "KIMONO GIRLS, I’ll give you a\l"
	.string "gift.$"

Johto_DanceTheater_Text_SurfGuyLikeADance::
	.string "The way you battled, it was like\n"
	.string "watching a dance.\p"
	.string "It was a rare treat to see!\p"
	.string "I want you to have this. Don’t\n"
	.string "worry --take it!$"

Johto_DanceTheater_Text_SurfGuySurfExplanation::
	.string "That’s SURF.\p"
	.string "It’s a move that lets MOéMON swim\n"
	.string "across water.$"

Johto_DanceTheater_Text_SurfGuyElegantKimonoGirls::
	.string "I wish my MOéMON were as elegant\n"
	.string "as the KIMONO GIRLS…$"

Johto_DanceTheater_Text_CooltrainerM::
	.string "That man’s always with his RHYDON.\p"
	.string "Says he wants a MOéMON that can\n"
	.string "SURF and dance.\p"
	.string "Is he trying to make a\n"
	.string "synchronized swimming MOéMON?$"

Johto_DanceTheater_Text_Granny::
	.string "The KIMONO GIRLS are so beautiful…\p"
	.string "But they have to go through\n"
	.string "rigorous training.\p"
	.string "And they have to learn to follow\n"
	.string "customs before appearing in\l"
	.string "public.\p"
	.string "But if you love something,\n"
	.string "anything is possible.$"

Johto_DanceTheater_Text_FancyPanel::
	.string "It’s a fancy panel that’s\n"
	.string "decorated with flowers.$"
