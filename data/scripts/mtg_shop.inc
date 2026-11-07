@ Moemon Kai: MAGIC at MOSSDEEP's CARD SHOP (the old GAME CORNER: docs/MTG_DECISIONS.md). The clerk at the counter
@ hands out a CARD CASE if needed and a starter deck (src/mtg/mtg_ui.c), then shows the way to the play room below.
@ The regulars upstairs and down (gMtgDuelists in src/mtg/mtg_decks.c, by VAR_0x8004) and the play room's visiting
@ guests (src/mtg/mtg_club.c: the room changes every half hour) don't let on that they play MAGIC until the player
@ has a deck; then talking to them asks for a duel. MOSSDEEP's CARD CLUB maps are the user's: only this file and the
@ C behind it change (and what the user asked for: the counter's door, the play room's mat, more people).
EventScript_MtgClerk::
	lock
	faceplayer
	special Mtg_ShopState
	goto_if_eq VAR_RESULT, TRUE, EventScript_MtgClerk_Regular
	msgbox MtgClerk_Text_Intro, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EventScript_MtgClerk_No
	checkitem ITEM_CARD_CASE
	goto_if_eq VAR_RESULT, TRUE, EventScript_MtgClerk_HasCase
	msgbox MtgClerk_Text_Case, MSGBOX_DEFAULT
	giveitem ITEM_CARD_CASE
	goto_if_eq VAR_RESULT, FALSE, EventScript_MtgClerk_NoRoom
EventScript_MtgClerk_HasCase::
	msgbox MtgClerk_Text_Pick, MSGBOX_DEFAULT
	closemessage
	special Mtg_ShopStarter
	special Mtg_ShopState
	goto_if_eq VAR_RESULT, FALSE, EventScript_MtgClerk_No
	msgbox MtgClerk_Text_Welcome, MSGBOX_DEFAULT
	release
	end

EventScript_MtgClerk_Regular::
	msgbox MtgClerk_Text_Boosters, MSGBOX_DEFAULT
	closemessage
	special Mtg_ShopBoosters
	msgbox MtgClerk_Text_Downstairs, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EventScript_MtgClerk_Bye
	msgbox MtgClerk_Text_ThisWay, MSGBOX_DEFAULT
	closemessage
	@ the old GAME CORNER's way into its back room: the counter lifts, the clerk shows the way and the player walks
	@ through the door behind it, down to the play room
	call EventScript_OpenMossdeepGameCornerBarrier
	special DrawWholeMapView
	playse SE_CLICK
	delay 30
	applymovement VAR_LAST_TALKED, Movement_MtgClerkFaceGate
	waitmovement 0
	applymovement LOCALID_PLAYER, Movement_MtgEnterPlayRoom
	waitmovement 0
	hideplayer
	warp MAP_MOSSDEEP_CITY_GAME_CORNER_B1F, 0
	waitstate
	release
	end

Movement_MtgClerkFaceGate:
	face_right
	step_end

Movement_MtgEnterPlayRoom:
	walk_right
	walk_up
	walk_up
	walk_up
	step_end

Movement_MtgLeavePlayRoom:
	walk_down
	walk_down
	step_end

@ 1F's map scripts: coming up from the play room (through the door behind the counter), the counter is open; the
@ player walks out in front of it and it closes behind them
MtgClub_1F_OnLoad:
	special Mtg_ClubBackDoor
	goto_if_eq VAR_RESULT, FALSE, CableClub_OnLoad
	setvar VAR_TEMP_1, 1
	call EventScript_OpenMossdeepGameCornerBarrier
	end

MtgClub_1F_OnFrame:
	map_script_2 VAR_TEMP_1, 1, MtgClub_EventScript_BackFromPlayRoom
	map_script_2 VAR_CABLE_CLUB_STATE, USING_MINIGAME, CableClub_EventScript_ExitMinigameRoom
	.short 0

MtgClub_EventScript_BackFromPlayRoom::
	lockall
	applymovement LOCALID_PLAYER, Movement_MtgLeavePlayRoom
	waitmovement 0
	call EventScript_CloseMossdeepGameCornerBarrier
	special DrawWholeMapView
	playse SE_CLICK
	setvar VAR_TEMP_1, 0
	releaseall
	end

@ the play room's map scripts: its players for this half hour (src/mtg/mtg_club.c)
MtgClub_B1F_OnTransition:
	special Mtg_ClubSetUp
	end

EventScript_MtgClerk_Bye::
	msgbox MtgClerk_Text_Bye, MSGBOX_DEFAULT
	release
	end

EventScript_MtgClerk_No::
	msgbox MtgClerk_Text_No, MSGBOX_DEFAULT
	release
	end

EventScript_MtgClerk_NoRoom::
	msgbox MtgClerk_Text_NoRoom, MSGBOX_DEFAULT
	release
	end

EventScript_MtgShopSign::
	msgbox MtgShop_Text_Sign, MSGBOX_SIGN
	end

@ the regulars: VAR_0x8004 = their place in gMtgDuelists
EventScript_MtgDuelist0::
	setvar VAR_0x8004, 0
	goto EventScript_MtgDuelist
EventScript_MtgDuelist1::
	setvar VAR_0x8004, 1
	goto EventScript_MtgDuelist
@ the play room's six chairs (objects 1-6 there; MtgDuelist2-5 keep their names for the Maker's edits): whoever sits
@ there this half hour (Mtg_ClubSeat: a regular or a visiting guest)
EventScript_MtgDuelist2::
	setvar VAR_0x8005, 0
	goto EventScript_MtgSeat
EventScript_MtgDuelist3::
	setvar VAR_0x8005, 1
	goto EventScript_MtgSeat
EventScript_MtgDuelist4::
	setvar VAR_0x8005, 2
	goto EventScript_MtgSeat
EventScript_MtgDuelist5::
	setvar VAR_0x8005, 3
	goto EventScript_MtgSeat
EventScript_MtgSeat4::
	setvar VAR_0x8005, 4
	goto EventScript_MtgSeat
EventScript_MtgSeat5::
	setvar VAR_0x8005, 5
	goto EventScript_MtgSeat
EventScript_MtgSeat::
	special Mtg_ClubSeat
	goto EventScript_MtgDuelist

@ the club's talkers: VAR_0x8004 which one (two upstairs, two on the play room's carpet); new words each half hour
EventScript_MtgTalker0::
	setvar VAR_0x8004, 0
	goto EventScript_MtgTalker
EventScript_MtgTalker1::
	setvar VAR_0x8004, 1
	goto EventScript_MtgTalker
EventScript_MtgTalker2::
	setvar VAR_0x8004, 2
	goto EventScript_MtgTalker
EventScript_MtgTalker3::
	setvar VAR_0x8004, 3
	goto EventScript_MtgTalker
EventScript_MtgTalker::
	special Mtg_ClubTalk
	msgbox gStringVar4, MSGBOX_NPC
	end

EventScript_MtgDuelist::
	lock
	faceplayer
	special Mtg_DuelistState
	goto_if_eq VAR_RESULT, FALSE, EventScript_MtgDuelist_NoDeck
	special Mtg_DuelistHello
	msgbox gStringVar4, MSGBOX_DEFAULT
	special Mtg_DuelistChallenge
	release
	end

EventScript_MtgDuelist_NoDeck::
	msgbox MtgDuelist_Text_NoDeck, MSGBOX_DEFAULT
	release
	end

MtgClerk_Text_Intro:
	.string "Welcome to the CARD SHOP!\p"
	.string "We play a card game here called\n"
	.string "MAGIC. Not many people know it…\p"
	.string "Would you like to learn? I can give\n"
	.string "you a deck to start with.$"

MtgClerk_Text_Case:
	.string "First, something to keep your cards\n"
	.string "in. Here, take this!$"

MtgClerk_Text_Pick:
	.string "There are five starter decks, one for\n"
	.string "each color of MAGIC. Pick the one\l"
	.string "that suits you!$"

MtgClerk_Text_Welcome:
	.string "Now you're one of us! The regulars\n"
	.string "will be glad of a new face.\p"
	.string "Talk to anyone here to play. There's\n"
	.string "a play room downstairs, too.$"

MtgClerk_Text_Boosters:
	.string "Welcome back! Boosters of every set\n"
	.string "are on the counter, for CARD POINTS.$"

MtgClerk_Text_Downstairs:
	.string "Would you like to go down to the\n"
	.string "play room?$"

MtgClerk_Text_ThisWay:
	.string "Right this way. Have fun!$"

MtgClerk_Text_Bye:
	.string "The regulars up here are always\n"
	.string "ready for a game, too.$"

MtgClerk_Text_No:
	.string "That's all right. Come back if you\n"
	.string "ever change your mind!$"

MtgClerk_Text_NoRoom:
	.string "Oh, your BAG is full. Make some room\n"
	.string "and come back!$"

MtgShop_Text_Sign:
	.string "MOSSDEEP CARD SHOP\n"
	.string "New players welcome at the counter.$"

MtgDuelist_Text_NoDeck:
	.string "Hm? Oh, nothing. We're just talking.\n"
	.string "Don't mind us.$"
