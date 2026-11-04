Johto_Route27SandstormHouse_MapScripts::
	.byte 0

@ Johto_Route27SandstormHouse object 1 (local id 1, gfx 0x1E) at 4,4
Johto_Route27SandstormHouse_EventScript_83EC99A::
	lock
	faceplayer
	goto_if_unset FLAG_QZ_0x105C, Johto_Route27SandstormHouse_EventScript_83EC9B0
	msgbox Johto_Route27SandstormHouse_Text_83ECA2A, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_Route27SandstormHouse_EventScript_83EC9B0::
	@ Moemon Kai: Crystal's loyalty check (docs/REGIONS_AUDIT.md 135)
	specialvar VAR_RESULT, MP_LeadFriendship
	msgbox Johto_Route27SandstormHouse_Text_Loyal, MSGBOX_DEFAULT
	goto_if_lt VAR_RESULT, 150, Johto_Route27SandstormHouse_EventScript_Disloyal
	msgbox Johto_Route27SandstormHouse_Text_83EC9DD, MSGBOX_DEFAULT
	@ Moemon Kai: Crystal's TM (tools/quetzal_port/fix_johto_tms.py) (Quetzal gave SMOOTH_ROCK)
	setorcopyvar VAR_0x8000, ITEM_TM37
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_83602AA
	setflag FLAG_QZ_0x105C
	msgbox Johto_Route27SandstormHouse_Text_83ECA2A, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_Route27SandstormHouse_EventScript_Disloyal::
	msgbox Johto_Route27SandstormHouse_Text_Disloyal, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_Route27SandstormHouse_Text_Loyal::
	.string "Where are you off to with MOéMON?\p"
	.string "MOéMON LEAGUE?\p"
	.string "Are your MOéMON loyal enough for\n"
	.string "you to win?\p"
	.string "Let me see…$"

Johto_Route27SandstormHouse_Text_Disloyal::
	.string "If it doesn’t come to trust you\n"
	.string "some more, it could be tough\l"
	.string "going.\p"
	.string "Trust is the tie that binds MOéMON\n"
	.string "and trainers.$"

Johto_Route27SandstormHouse_Text_83EC9DD::
	.string "Ah! Your MOéMON trusts you very\n"
	.string "much.\p"
	.string "It’s nice to see a good trainer.\p"
	.string "Here. A gift for your journey.$"

Johto_Route27SandstormHouse_Text_83ECA2A::
	.string "TM37 happens to be SANDSTORM.\p"
	.string "It’s a move that inflicts damage\n"
	.string "on both battlers.\p"
	.string "It’s for advanced trainers only.\p"
	.string "Use it if you dare. Good luck!$"
