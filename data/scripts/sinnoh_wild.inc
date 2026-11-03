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

@ SANDGEM's LAB: PROF. ROWAN's aide gives the POKé RADAR once SINNOH's LEAGUE is beaten (flag 0x3D33)
Sinnoh_EventScript_RadarAide::
	lock
	faceplayer
	specialvar VAR_RESULT, Sinnoh_LeagueDone
	goto_if_eq VAR_RESULT, FALSE, Sinnoh_EventScript_RadarAideBefore
	goto_if_set 0x3D33, Sinnoh_EventScript_RadarAideGiven
	msgbox Sinnoh_Text_RadarAideGive, MSGBOX_DEFAULT
	giveitem ITEM_POKE_RADAR
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3D33
	msgbox Sinnoh_Text_RadarAideHow, MSGBOX_DEFAULT
	release
	end

Sinnoh_EventScript_RadarAideBefore::
	msgbox Sinnoh_Text_RadarAideBefore, MSGBOX_DEFAULT
	release
	end

Sinnoh_EventScript_RadarAideGiven::
	msgbox Sinnoh_Text_RadarAideHow, MSGBOX_DEFAULT
	release
	end

Sinnoh_Text_RadarAideBefore:
	.string "The professor is out in the field\n"
	.string "again.\p"
	.string "He says you can't study MOéMON from\n"
	.string "behind a desk. He's right, but\l"
	.string "someone has to file all this!$"

Sinnoh_Text_RadarAideGive:
	.string "Oh! The new CHAMPION! The professor\n"
	.string "left something here for you.\p"
	.string "It's a device he's been testing. He\n"
	.string "says it's wasted on a desk!$"

Sinnoh_Text_RadarAideHow:
	.string "Use the POKé RADAR in tall grass, and\n"
	.string "patches nearby start to shake.\p"
	.string "Step into one to meet a MOéMON. Beat\n"
	.string "or catch it, and the grass shakes\l"
	.string "again, often with the same kind.\p"
	.string "Keep the chain going and some patches\n"
	.string "may even sparkle!$"
