Kanto_FourIsland_House2_MapScripts::
	.byte 0

@ Kanto_FourIsland_House2 object 1 (local id 1, gfx 0x3F1E) at 4,4
@ Moemon Kai: FireRed's STICKER MAN (pokefirered data/scripts/trainer_card.inc; docs/REGIONS_AUDIT.md 126): a sticker on
@ the TRAINER CARD for a HALL OF FAME, EGGS or VICTORIES story (mp_tcard.c MP_TCard_StickerBrag)
Kanto_FourIsland_House2_EventScript_8462B36::
	lock
	faceplayer
	specialvar VAR_0x8008, MP_TCard_StickerBragFlags
	specialvar VAR_RESULT, MP_TCard_StickerManMet
	goto_if_eq VAR_RESULT, TRUE, Kanto_FourIsland_House2_EventScript_AskForBrag
	msgbox Kanto_FourIsland_House2_Text_WishICouldShowOffStickers
	playse SE_PIN
	applymovement 1, Kanto_FourIsland_House2_Movement_Exclamation
	waitmovement 0
	applymovement 1, Kanto_FourIsland_House2_Movement_Delay48
	waitmovement 0
	goto_if_eq VAR_0x8008, 0, Kanto_FourIsland_House2_EventScript_NothingToBragFirst
	message Kanto_FourIsland_House2_Text_GiveYouStickerIfYouBrag
	waitmessage
	goto Kanto_FourIsland_House2_EventScript_ChooseBrag

Kanto_FourIsland_House2_Movement_Exclamation:
	emote_exclamation_mark
	step_end

Kanto_FourIsland_House2_Movement_Delay48:
	delay_16
	delay_16
	delay_16
	step_end

Kanto_FourIsland_House2_EventScript_NothingToBragFirst::
	msgbox Kanto_FourIsland_House2_Text_GiveYouStickerIfYouBrag
	release
	end

Kanto_FourIsland_House2_EventScript_AskForBrag::
	goto_if_eq VAR_0x8008, 0, Kanto_FourIsland_House2_EventScript_NothingToBragAbout
	message Kanto_FourIsland_House2_Text_BragAboutWhatToday
	waitmessage
	goto Kanto_FourIsland_House2_EventScript_ChooseBrag

Kanto_FourIsland_House2_EventScript_NothingToBragAbout::
	msgbox Kanto_FourIsland_House2_Text_BragAboutSomethingForMe
	release
	end

Kanto_FourIsland_House2_EventScript_ChooseBrag::
	goto_if_eq VAR_0x8008, 1, Kanto_FourIsland_House2_EventScript_Choose1
	goto_if_eq VAR_0x8008, 2, Kanto_FourIsland_House2_EventScript_Choose2
	goto_if_eq VAR_0x8008, 4, Kanto_FourIsland_House2_EventScript_Choose4
	goto_if_eq VAR_0x8008, 3, Kanto_FourIsland_House2_EventScript_Choose3
	goto_if_eq VAR_0x8008, 5, Kanto_FourIsland_House2_EventScript_Choose5
	goto_if_eq VAR_0x8008, 6, Kanto_FourIsland_House2_EventScript_Choose6
	goto_if_eq VAR_0x8008, 7, Kanto_FourIsland_House2_EventScript_Choose7
	release
	end

Kanto_FourIsland_House2_EventScript_Choose1::
	multichoice 15, 8, MULTI_HOF_QUIT, FALSE
	goto_if_eq VAR_RESULT, 0, Kanto_FourIsland_House2_EventScript_BragHoF
	release
	end

Kanto_FourIsland_House2_EventScript_Choose2::
	multichoice 15, 8, MULTI_EGGS_QUIT, FALSE
	goto_if_eq VAR_RESULT, 0, Kanto_FourIsland_House2_EventScript_BragEggs
	release
	end

Kanto_FourIsland_House2_EventScript_Choose4::
	multichoice 15, 8, MULTI_VICTORIES_QUIT, FALSE
	goto_if_eq VAR_RESULT, 0, Kanto_FourIsland_House2_EventScript_BragLinkWins
	release
	end

Kanto_FourIsland_House2_EventScript_Choose3::
	multichoice 15, 6, MULTI_HOF_EGGS_QUIT, FALSE
	goto_if_eq VAR_RESULT, 0, Kanto_FourIsland_House2_EventScript_BragHoF
	goto_if_eq VAR_RESULT, 1, Kanto_FourIsland_House2_EventScript_BragEggs
	release
	end

Kanto_FourIsland_House2_EventScript_Choose5::
	multichoice 15, 6, MULTI_HOF_VICTORIES_QUIT, FALSE
	goto_if_eq VAR_RESULT, 0, Kanto_FourIsland_House2_EventScript_BragHoF
	goto_if_eq VAR_RESULT, 1, Kanto_FourIsland_House2_EventScript_BragLinkWins
	release
	end

Kanto_FourIsland_House2_EventScript_Choose6::
	multichoice 15, 6, MULTI_EGGS_VICTORIES_QUIT, FALSE
	goto_if_eq VAR_RESULT, 0, Kanto_FourIsland_House2_EventScript_BragEggs
	goto_if_eq VAR_RESULT, 1, Kanto_FourIsland_House2_EventScript_BragLinkWins
	release
	end

Kanto_FourIsland_House2_EventScript_Choose7::
	multichoice 15, 5, MULTI_HOF_EGGS_VICTORIES_QUIT, FALSE
	goto_if_eq VAR_RESULT, 0, Kanto_FourIsland_House2_EventScript_BragHoF
	goto_if_eq VAR_RESULT, 1, Kanto_FourIsland_House2_EventScript_BragEggs
	goto_if_eq VAR_RESULT, 2, Kanto_FourIsland_House2_EventScript_BragLinkWins
	release
	end

Kanto_FourIsland_House2_EventScript_BragHoF::
	setvar VAR_0x8009, 0
	specialvar VAR_RESULT, MP_TCard_StickerBrag
	goto_if_eq VAR_RESULT, 0, Kanto_FourIsland_House2_EventScript_AlreadyBraggedHoF
	goto_if_eq VAR_RESULT, 5, Kanto_FourIsland_House2_EventScript_AlreadyBraggedHoFMax
	call_if_eq VAR_RESULT, 1, Kanto_FourIsland_House2_EventScript_WowHoFLowest
	call_if_eq VAR_RESULT, 2, Kanto_FourIsland_House2_EventScript_WowHoFLow
	call_if_eq VAR_RESULT, 3, Kanto_FourIsland_House2_EventScript_WowHoFHigh
	call_if_eq VAR_RESULT, 4, Kanto_FourIsland_House2_EventScript_WowHoFHighest
	playfanfare MUS_LEVEL_UP
	message Kanto_FourIsland_House2_Text_HoFStickerApplied
	waitmessage
	waitfanfare
	delay 90
	release
	end

Kanto_FourIsland_House2_EventScript_WowHoFLowest::
	msgbox Kanto_FourIsland_House2_Text_WowHoFLowest
	return

Kanto_FourIsland_House2_EventScript_WowHoFLow::
	msgbox Kanto_FourIsland_House2_Text_WowHoFLow
	return

Kanto_FourIsland_House2_EventScript_WowHoFHigh::
	msgbox Kanto_FourIsland_House2_Text_WowHoFHigh
	return

Kanto_FourIsland_House2_EventScript_WowHoFHighest::
	msgbox Kanto_FourIsland_House2_Text_WowHoFHighest
	return

Kanto_FourIsland_House2_EventScript_AlreadyBraggedHoF::
	msgbox Kanto_FourIsland_House2_Text_ComeBackWithBetterStoryHoF
	release
	end

Kanto_FourIsland_House2_EventScript_AlreadyBraggedHoFMax::
	msgbox Kanto_FourIsland_House2_Text_LivingLegendHoF
	release
	end

Kanto_FourIsland_House2_EventScript_BragEggs::
	setvar VAR_0x8009, 1
	specialvar VAR_RESULT, MP_TCard_StickerBrag
	goto_if_eq VAR_RESULT, 0, Kanto_FourIsland_House2_EventScript_AlreadyBraggedEggs
	goto_if_eq VAR_RESULT, 5, Kanto_FourIsland_House2_EventScript_AlreadyBraggedEggsMax
	call_if_eq VAR_RESULT, 1, Kanto_FourIsland_House2_EventScript_WowEggsLowest
	call_if_eq VAR_RESULT, 2, Kanto_FourIsland_House2_EventScript_WowEggsLow
	call_if_eq VAR_RESULT, 3, Kanto_FourIsland_House2_EventScript_WowEggsHigh
	call_if_eq VAR_RESULT, 4, Kanto_FourIsland_House2_EventScript_WowEggsHighest
	playfanfare MUS_LEVEL_UP
	message Kanto_FourIsland_House2_Text_EggStickerApplied
	waitmessage
	waitfanfare
	delay 90
	release
	end

Kanto_FourIsland_House2_EventScript_WowEggsLowest::
	msgbox Kanto_FourIsland_House2_Text_WowEggsLowest
	return

Kanto_FourIsland_House2_EventScript_WowEggsLow::
	msgbox Kanto_FourIsland_House2_Text_WowEggsLow
	return

Kanto_FourIsland_House2_EventScript_WowEggsHigh::
	msgbox Kanto_FourIsland_House2_Text_WowEggsHigh
	return

Kanto_FourIsland_House2_EventScript_WowEggsHighest::
	msgbox Kanto_FourIsland_House2_Text_WowEggsHighest
	return

Kanto_FourIsland_House2_EventScript_AlreadyBraggedEggs::
	msgbox Kanto_FourIsland_House2_Text_ComeBackWithBetterStoryEggs
	release
	end

Kanto_FourIsland_House2_EventScript_AlreadyBraggedEggsMax::
	msgbox Kanto_FourIsland_House2_Text_LivingLegendEggs
	release
	end

Kanto_FourIsland_House2_EventScript_BragLinkWins::
	setvar VAR_0x8009, 2
	specialvar VAR_RESULT, MP_TCard_StickerBrag
	goto_if_eq VAR_RESULT, 0, Kanto_FourIsland_House2_EventScript_AlreadyBraggedLinkWins
	goto_if_eq VAR_RESULT, 5, Kanto_FourIsland_House2_EventScript_AlreadyBraggedLinkWinsMax
	call_if_eq VAR_RESULT, 1, Kanto_FourIsland_House2_EventScript_WowLinkWinsLowest
	call_if_eq VAR_RESULT, 2, Kanto_FourIsland_House2_EventScript_WowLinkWinsLow
	call_if_eq VAR_RESULT, 3, Kanto_FourIsland_House2_EventScript_WowLinkWinsHigh
	call_if_eq VAR_RESULT, 4, Kanto_FourIsland_House2_EventScript_WowLinkWinsHighest
	playfanfare MUS_LEVEL_UP
	message Kanto_FourIsland_House2_Text_VictoryStickerApplied
	waitmessage
	waitfanfare
	delay 90
	release
	end

Kanto_FourIsland_House2_EventScript_WowLinkWinsLowest::
	msgbox Kanto_FourIsland_House2_Text_WowLinkWinsLowest
	return

Kanto_FourIsland_House2_EventScript_WowLinkWinsLow::
	msgbox Kanto_FourIsland_House2_Text_WowLinkWinsLow
	return

Kanto_FourIsland_House2_EventScript_WowLinkWinsHigh::
	msgbox Kanto_FourIsland_House2_Text_WowLinkWinsHigh
	return

Kanto_FourIsland_House2_EventScript_WowLinkWinsHighest::
	msgbox Kanto_FourIsland_House2_Text_WowLinkWinsHighest
	return

Kanto_FourIsland_House2_EventScript_AlreadyBraggedLinkWins::
	msgbox Kanto_FourIsland_House2_Text_ComeBackWithBetterStoryLinkWins
	release
	end

Kanto_FourIsland_House2_EventScript_AlreadyBraggedLinkWinsMax::
	msgbox Kanto_FourIsland_House2_Text_LivingLegendLinkWins
	release
	end

Kanto_FourIsland_House2_Text_WishICouldShowOffStickers::
	.string "Giggle…\n"
	.string "I collected a ton of STICKERS.\l"
	.string "I wish I could show them off…$"

Kanto_FourIsland_House2_Text_GiveYouStickerIfYouBrag::
	.string "Oh, excellent!\n"
	.string "You’ve come to the right place!\p"
	.string "Look, look! See? See?\n"
	.string "These are my STICKERS!\l"
	.string "Look how many I got!\p"
	.string "I bet you want some.\n"
	.string "I bet you do!\p"
	.string "I’ll give a STICKER if you can tell\n"
	.string "me something awesome about\l"
	.string "yourself.\p"
	.string "What will you brag about?$"

Kanto_FourIsland_House2_Text_BragAboutSomethingForMe::
	.string "Brag about something for me.\n"
	.string "I’ll give you a STICKER.$"

Kanto_FourIsland_House2_Text_BragAboutWhatToday::
	.string "Oh, hi!\n"
	.string "Here comes the braggart.\p"
	.string "What are you going to brag about\n"
	.string "today?$"

Kanto_FourIsland_House2_Text_WowHoFLowest::
	.string "Oh, wow, you made it into the\n"
	.string "HALL OF FAME.\p"
	.string "That’s pretty good, yup!\n"
	.string "I’ll give you one of these.$"

Kanto_FourIsland_House2_Text_WowHoFLow::
	.string "Oh, wow, you’ve entered the\n"
	.string "HALL OF FAME often!\p"
	.string "That’s impressive, yup!\n"
	.string "I’ll give you one of these.$"

Kanto_FourIsland_House2_Text_WowHoFHigh::
	.string "Whoa! You’ve made it into the\n"
	.string "HALL OF FAME that often?\l"
	.string "That’s seriously incredible, yup!\p"
	.string "You own the MOéMON LEAGUE!\n"
	.string "I’ll give you one of these.$"

Kanto_FourIsland_House2_Text_WowHoFHighest::
	.string "No way! You’ve gone into the\n"
	.string "HALL OF FAME that many times?\p"
	.string "You’re beyond incredible, yup!\n"
	.string "That’s it, I have to give you this.$"

Kanto_FourIsland_House2_Text_HoFStickerApplied::
	.string "The HALL OF FAME STICKER was\n"
	.string "applied to the TRAINER CARD.$"

Kanto_FourIsland_House2_Text_ComeBackWithBetterStoryHoF::
	.string "Hmm…\p"
	.string "Come back with a better story next\n"
	.string "time, okay?$"

Kanto_FourIsland_House2_Text_LivingLegendHoF::
	.string "Oh, no, no can do.\p"
	.string "You’re practically a living legend.\n"
	.string "I have no SITCKERS left to give.$"

Kanto_FourIsland_House2_Text_WowEggsLowest::
	.string "Oh, wow, there are MOéMON EGGS?\n"
	.string "I didn’t know that!\p"
	.string "That’s pretty good, yup!\n"
	.string "I’ll give you one of these.$"

Kanto_FourIsland_House2_Text_WowEggsLow::
	.string "You’ve hatched that many EGGS?\n"
	.string "You really must like them!\p"
	.string "That’s impressive, yup!\n"
	.string "I’ll give you one of these.$"

Kanto_FourIsland_House2_Text_WowEggsHigh::
	.string "Whoa! You’ve hatched a whole\n"
	.string "bunch of EGGS!\p"
	.string "You’re an EGG-hatching machine!\n"
	.string "I’ll give you one of these.$"

Kanto_FourIsland_House2_Text_WowEggsHighest::
	.string "Wh… You hatched that many EGGS?\p"
	.string "What’s behind your love of EGGS?\n"
	.string "It’s beyond incredible, yup!\p"
	.string "You’re too awesome, I tell you.\n"
	.string "That’s it, I have to give you this.$"

Kanto_FourIsland_House2_Text_EggStickerApplied::
	.string "The EGG STICKER was applied\n"
	.string "to the TRAINER CARD.$"

Kanto_FourIsland_House2_Text_ComeBackWithBetterStoryEggs::
	.string "Hmm…\p"
	.string "Come back with a better story next\n"
	.string "time, okay?$"

Kanto_FourIsland_House2_Text_LivingLegendEggs::
	.string "Oh, no, no can do.\p"
	.string "You’re practically a living legend.\n"
	.string "I have no SITCKERS left to give.$"

Kanto_FourIsland_House2_Text_WowLinkWinsLowest::
	.string "Oh, wow, you’ve had success\n"
	.string "link battling?\p"
	.string "You’re pretty strong, yup!\n"
	.string "I’ll give you one of these.$"

Kanto_FourIsland_House2_Text_WowLinkWinsLow::
	.string "You’ve beaten your friends a lot\n"
	.string "link battling, huh?\p"
	.string "You’re impressively strong, yup!\n"
	.string "I’ll give you one of these.$"

Kanto_FourIsland_House2_Text_WowLinkWinsHigh::
	.string "Whoa! You’ve beaten your friends\n"
	.string "a frightful number of times.\p"
	.string "Have you lost friends over this?\n"
	.string "I’ll give you one of these.$"

Kanto_FourIsland_House2_Text_WowLinkWinsHighest::
	.string "Wh… Wickedly whoa!\n"
	.string "You’ve won mind-blowingly often!\p"
	.string "It just knocks me out thinking\n"
	.string "about how tough you are.\p"
	.string "You’re the stuff of nightmares!\n"
	.string "That’s it, I have to give you this.$"

Kanto_FourIsland_House2_Text_VictoryStickerApplied::
	.string "The VICTORY STICKER was applied\n"
	.string "to the TRAINER CARD.$"

Kanto_FourIsland_House2_Text_ComeBackWithBetterStoryLinkWins::
	.string "Hmm…\p"
	.string "Come back with a better story next\n"
	.string "time, okay?$"

Kanto_FourIsland_House2_Text_LivingLegendLinkWins::
	.string "Oh, no, no can do.\p"
	.string "You’re practically a living legend.\n"
	.string "I have no SITCKERS left to give.$"

