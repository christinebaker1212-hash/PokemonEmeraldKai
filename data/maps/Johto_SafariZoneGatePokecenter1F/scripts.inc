Johto_SafariZoneGatePokecenter1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_SafariZoneGatePokecenter1F_EventScript_83F8D05
	.byte 0

Johto_SafariZoneGatePokecenter1F_EventScript_83F8D05::
	setrespawn 0x1F
	end

@ Johto_SafariZoneGatePokecenter1F object 3 (local id 3, gfx 0x18) at 14,6
Johto_SafariZoneGatePokecenter1F_EventScript_83F8D09::
	lock
	faceplayer
	msgbox Johto_SafariZoneGatePokecenter1F_Text_83F8D30, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_SafariZoneGatePokecenter1F object 4 (local id 4, gfx 0x37) at 12,5
Johto_SafariZoneGatePokecenter1F_EventScript_83F8D16::
	lock
	faceplayer
	msgbox Johto_SafariZoneGatePokecenter1F_Text_83F8D8E, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_SafariZoneGatePokecenter1F object 5 (local id 5, gfx 0xB) at 3,6
Johto_SafariZoneGatePokecenter1F_EventScript_83F8D23::
	lock
	faceplayer
	msgbox Johto_SafariZoneGatePokecenter1F_Text_83F8E1A, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_SafariZoneGatePokecenter1F_Text_83F8D30::
	.string "Those who come to a POKéMON CENTER\n"
	.string "often take good care of their POKéMON.\p"
	.string "Wouldn’t you agree?$"

Johto_SafariZoneGatePokecenter1F_Text_83F8D8E::
	.string "After a long walk in the mountains to\n"
	.string "find a POKéMON CENTER, when you’re\l"
	.string "just about to fall flat...\p"
	.string "Then it feels like you are in paradise!$"

Johto_SafariZoneGatePokecenter1F_Text_83F8E1A::
	.string "Safari, safari, it is so much fun.\n"
	.string "Safari, safari, so interesting, huh!\p"
	.string "So many POKéMON are waiting for us!\p"
	.string "...That’s a song I made up.\n"
	.string "I sing it when I am in the SAFARI ZONE.$"
