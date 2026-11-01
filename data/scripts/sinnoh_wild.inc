@ Moemon Kai: SINNOH's swarms and HONEY trees (src/sinnoh/s3d_wild.c). The words are this game's own.

@ The twin in SANDGEM's house by the LAB: once SINNOH's LEAGUE is beaten, the day's swarm
Sinnoh_EventScript_SwarmTwin::
	lock
	faceplayer
	specialvar VAR_RESULT, Sinnoh_LeagueDone
	goto_if_eq VAR_RESULT, FALSE, Sinnoh_EventScript_SwarmTwinBefore
	special Sinnoh_SwarmNews
	goto_if_eq VAR_RESULT, FALSE, Sinnoh_EventScript_SwarmTwinNone
	msgbox Sinnoh_Text_SwarmTwinNews, MSGBOX_DEFAULT
	release
	end

Sinnoh_EventScript_SwarmTwinBefore::
	msgbox Sinnoh_Text_SwarmTwinBefore, MSGBOX_DEFAULT
	release
	end

Sinnoh_EventScript_SwarmTwinNone::
	msgbox Sinnoh_Text_SwarmTwinNone, MSGBOX_DEFAULT
	release
	end

Sinnoh_Text_SwarmTwinBefore:
	.string "My big sister knows the name of every\n"
	.string "MOéMON!\p"
	.string "Well, almost every one. She says there\n"
	.string "are always more.$"

Sinnoh_Text_SwarmTwinNews:
	.string "{PLAYER}! You beat the LEAGUE? Then\n"
	.string "you get my sister's big news!\p"
	.string "She keeps track of where MOéMON\n"
	.string "bunch up. Today there are loads of\l"
	.string "{STR_VAR_2} on {STR_VAR_1}!\p"
	.string "It changes every day. Come back\n"
	.string "tomorrow and I'll tell you the next!$"

Sinnoh_Text_SwarmTwinNone:
	.string "My sister says the MOéMON are all\n"
	.string "spread out today. No big crowds\l"
	.string "anywhere. Boring!$"

@ A HONEY tree, faced from below (tools/sinnoh/gen_maps.py honey_tree)
Sinnoh_EventScript_HoneyTree::
	lockall
	specialvar VAR_RESULT, Sinnoh_HoneyTreeState
	goto_if_eq VAR_RESULT, 2, Sinnoh_EventScript_HoneyTreeMon
	goto_if_eq VAR_RESULT, 1, Sinnoh_EventScript_HoneyTreeWaiting
	goto_if_eq VAR_RESULT, 3, Sinnoh_EventScript_HoneyTreeNothing
	checkitem ITEM_HONEY
	goto_if_eq VAR_RESULT, FALSE, Sinnoh_EventScript_HoneyTreeNoHoney
	msgbox Sinnoh_Text_HoneyTreeAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Sinnoh_EventScript_HoneyTreeEnd
	removeitem ITEM_HONEY
	special Sinnoh_HoneySlather
	msgbox Sinnoh_Text_HoneyTreeSlathered, MSGBOX_DEFAULT
	releaseall
	end

Sinnoh_EventScript_HoneyTreeNoHoney::
	msgbox Sinnoh_Text_HoneyTree, MSGBOX_DEFAULT
Sinnoh_EventScript_HoneyTreeEnd::
	releaseall
	end

Sinnoh_EventScript_HoneyTreeWaiting::
	msgbox Sinnoh_Text_HoneyTreeWaiting, MSGBOX_DEFAULT
	releaseall
	end

Sinnoh_EventScript_HoneyTreeNothing::
	msgbox Sinnoh_Text_HoneyTreeNothing, MSGBOX_DEFAULT
	releaseall
	end

Sinnoh_EventScript_HoneyTreeMon::
	msgbox Sinnoh_Text_HoneyTreeMon, MSGBOX_DEFAULT
	closemessage
	special Sinnoh_HoneyTreeMon
	dowildbattle
	releaseall
	end

Sinnoh_Text_HoneyTree:
	.string "It's a big tree with soft, golden bark.\n"
	.string "It smells faintly sweet.$"

Sinnoh_Text_HoneyTreeAsk:
	.string "It's a big tree with soft, golden bark.\n"
	.string "It smells faintly sweet.\p"
	.string "Spread some HONEY on it?$"

Sinnoh_Text_HoneyTreeSlathered:
	.string "{PLAYER} spread HONEY on the tree.\p"
	.string "Something may come for it if you\n"
	.string "leave it for a while...$"

Sinnoh_Text_HoneyTreeWaiting:
	.string "The HONEY glistens on the bark.\n"
	.string "Nothing has come for it yet.$"

Sinnoh_Text_HoneyTreeNothing:
	.string "The HONEY has dried away...\p"
	.string "Nothing came for it this time.$"

Sinnoh_Text_HoneyTreeMon:
	.string "Something is clinging to the tree,\n"
	.string "licking the HONEY!$"

@ FLOAROMA MEADOW's HONEY seller
Sinnoh_EventScript_HoneySeller::
	lock
	faceplayer
	message Sinnoh_Text_HoneySeller
	waitmessage
	pokemart Sinnoh_Mart_Honey
	msgbox Sinnoh_Text_HoneySellerBye, MSGBOX_DEFAULT
	release
	end

	.align 2
Sinnoh_Mart_Honey:
	.short ITEM_HONEY
	pokemartlistend

Sinnoh_Text_HoneySeller:
	.string "The trees here love HONEY, and so do\n"
	.string "the MOéMON that come to lick it off.\p"
	.string "Fresh from the meadow! Want some?$"

Sinnoh_Text_HoneySellerBye:
	.string "Spread it on a sweet tree and come\n"
	.string "back later. You'll see!$"

@ MR. BACKLOT, the POKéMON MANSION's owner: after SINNOH's LEAGUE a daily POKéMON for his TROPHY GARDEN
Sinnoh_EventScript_MrBacklot::
	lock
	faceplayer
	specialvar VAR_RESULT, Sinnoh_TrophyGardenToday
	goto_if_eq VAR_RESULT, 0, Sinnoh_EventScript_MrBacklotWelcome
	goto_if_eq VAR_RESULT, 1, Sinnoh_EventScript_MrBacklotToday
	msgbox Sinnoh_Text_MrBacklotAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Sinnoh_EventScript_MrBacklotNo
	special Sinnoh_TrophyGardenAdd
	msgbox Sinnoh_Text_MrBacklotBoast, MSGBOX_DEFAULT
	release
	end

Sinnoh_EventScript_MrBacklotWelcome::
	msgbox Sinnoh_Text_MrBacklotWelcome, MSGBOX_DEFAULT
	release
	end

Sinnoh_EventScript_MrBacklotToday::
	msgbox Sinnoh_Text_MrBacklotToday, MSGBOX_DEFAULT
	release
	end

Sinnoh_EventScript_MrBacklotNo::
	msgbox Sinnoh_Text_MrBacklotNo, MSGBOX_DEFAULT
	release
	end

Sinnoh_Text_MrBacklotWelcome:
	.string "Welcome! This mansion is my pride and\n"
	.string "joy. Look around, but don't break\l"
	.string "anything!$"

Sinnoh_Text_MrBacklotAsk:
	.string "Ah, the LEAGUE CHAMPION! You must\n"
	.string "have seen every MOéMON there is.\p"
	.string "But have you seen my TROPHY GARDEN?\n"
	.string "Shall I tell you what lives there?$"

Sinnoh_Text_MrBacklotBoast:
	.string "Why, {STR_VAR_1} has made a home in my\n"
	.string "garden! Charming little thing.\p"
	.string "Go and see for yourself. Out the\n"
	.string "back, past the statue!$"

Sinnoh_Text_MrBacklotToday:
	.string "Have you met the {STR_VAR_1} in my\n"
	.string "TROPHY GARDEN yet? Come back\l"
	.string "tomorrow and I may have another!$"

Sinnoh_Text_MrBacklotNo:
	.string "Hmph! Your loss. The finest garden\n"
	.string "in SINNOH, and you won't hear of it!$"

@ ROUTE 208's BLACK BELT: the ODD KEYSTONE (once; flag 0x3D31)
Sinnoh_EventScript_OddKeystoneGiver::
	lock
	faceplayer
	goto_if_set 0x3D31, Sinnoh_EventScript_OddKeystoneGiven
	msgbox Sinnoh_Text_OddKeystoneGive, MSGBOX_DEFAULT
	giveitem ITEM_ODD_KEYSTONE
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3D31
	msgbox Sinnoh_Text_OddKeystoneGiven, MSGBOX_DEFAULT
	release
	end

Sinnoh_EventScript_OddKeystoneGiven::
	msgbox Sinnoh_Text_OddKeystoneGiven, MSGBOX_DEFAULT
	release
	end

Sinnoh_Text_OddKeystoneGive:
	.string "Hyah! I train on this bridge. The river\n"
	.string "below keeps me focused.\p"
	.string "I pulled this odd stone out of the\n"
	.string "water. It's no use to me. Take it!$"

Sinnoh_Text_OddKeystoneGiven:
	.string "That stone came from somewhere old.\n"
	.string "There's a crumbling tower on ROUTE 209,\l"
	.string "south of SOLACEON. Maybe it fits there?$"

@ ROUTE 209's HALLOWED TOWER: the ODD KEYSTONE on top, then the spirits gather as the player battles (32
@ battles: src/sinnoh/s3d_wild.c Sinnoh_SpiritombTower), and SPIRITOMB comes out (Lv. 25)
Sinnoh_EventScript_HallowedTower::
	lockall
	specialvar VAR_RESULT, Sinnoh_SpiritombTower
	goto_if_eq VAR_RESULT, 6, Sinnoh_EventScript_HallowedTowerSpiritomb
	goto_if_eq VAR_RESULT, 5, Sinnoh_EventScript_HallowedTower5
	goto_if_eq VAR_RESULT, 4, Sinnoh_EventScript_HallowedTower4
	goto_if_eq VAR_RESULT, 3, Sinnoh_EventScript_HallowedTower3
	goto_if_eq VAR_RESULT, 2, Sinnoh_EventScript_HallowedTower2
	goto_if_eq VAR_RESULT, 1, Sinnoh_EventScript_HallowedTower1
	checkitem ITEM_ODD_KEYSTONE
	goto_if_eq VAR_RESULT, FALSE, Sinnoh_EventScript_HallowedTowerBroken
	msgbox Sinnoh_Text_HallowedTowerAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Sinnoh_EventScript_HallowedTowerEnd
	removeitem ITEM_ODD_KEYSTONE
	special Sinnoh_SpiritombPlace
	msgbox Sinnoh_Text_HallowedTowerPlaced, MSGBOX_DEFAULT
Sinnoh_EventScript_HallowedTowerEnd::
	releaseall
	end

Sinnoh_EventScript_HallowedTowerBroken::
	msgbox Sinnoh_Text_HallowedTowerBroken, MSGBOX_DEFAULT
	releaseall
	end

Sinnoh_EventScript_HallowedTower1::
	msgbox Sinnoh_Text_HallowedTower1, MSGBOX_DEFAULT
	releaseall
	end

Sinnoh_EventScript_HallowedTower2::
	msgbox Sinnoh_Text_HallowedTower2, MSGBOX_DEFAULT
	releaseall
	end

Sinnoh_EventScript_HallowedTower3::
	msgbox Sinnoh_Text_HallowedTower3, MSGBOX_DEFAULT
	releaseall
	end

Sinnoh_EventScript_HallowedTower4::
	msgbox Sinnoh_Text_HallowedTower4, MSGBOX_DEFAULT
	releaseall
	end

Sinnoh_EventScript_HallowedTower5::
	msgbox Sinnoh_Text_HallowedTower5, MSGBOX_DEFAULT
	releaseall
	end

Sinnoh_EventScript_HallowedTowerSpiritomb::
	waitse
	playmoncry SPECIES_SPIRITOMB, CRY_MODE_ENCOUNTER
	msgbox Sinnoh_Text_HallowedTowerCry, MSGBOX_DEFAULT
	waitmoncry
	closemessage
	setwildbattle SPECIES_SPIRITOMB, 25
	special BattleSetup_StartLegendaryBattle
	waitstate
	special Sinnoh_SpiritombAfter
	releaseall
	end

Sinnoh_Text_HallowedTowerBroken:
	.string "A small tower of stacked stones, very\n"
	.string "old. The stone on top is missing.$"

Sinnoh_Text_HallowedTowerAsk:
	.string "A small tower of stacked stones. The\n"
	.string "stone on top is missing.\p"
	.string "Put the ODD KEYSTONE on top?$"

Sinnoh_Text_HallowedTowerPlaced:
	.string "{PLAYER} set the ODD KEYSTONE on the\n"
	.string "tower. It fits as if it always had.\p"
	.string "The air around the tower feels a\n"
	.string "little colder...$"

Sinnoh_Text_HallowedTower1:
	.string "The tower stands quiet with its\n"
	.string "keystone on top.$"

Sinnoh_Text_HallowedTower2:
	.string "The stones of the tower seem to have\n"
	.string "moved, just a little...$"

Sinnoh_Text_HallowedTower3:
	.string "A faint sound, like crying, comes from\n"
	.string "inside the tower...$"

Sinnoh_Text_HallowedTower4:
	.string "The tower is trembling!$"

Sinnoh_Text_HallowedTower5:
	.string "Something is inside the tower. You can\n"
	.string "feel it watching you...$"

Sinnoh_Text_HallowedTowerCry:
	.string "Bwaaaaagh!$"
