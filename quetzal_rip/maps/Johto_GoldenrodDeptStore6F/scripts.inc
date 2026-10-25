Johto_GoldenrodDeptStore6F_MapScripts::
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Johto_GoldenrodDeptStore6F_OnFrame_83D3830
	map_script MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE, Johto_GoldenrodDeptStore6F_OnWarp_83D383A
	.byte 0

Johto_GoldenrodDeptStore6F_OnFrame_83D3830:
	map_script_2 VAR_QZ_0x5015, 1, Quetzal_EventScript_83D384A
	.2byte 0

Johto_GoldenrodDeptStore6F_OnWarp_83D383A:
	map_script_2 VAR_QZ_0x5015, 1, Quetzal_EventScript_83D3844
	.2byte 0

@ Johto_GoldenrodDeptStore6F object 4 (local id 4, gfx 0x3EBC) at 20,9
Johto_GoldenrodDeptStore6F_EventScript_83D4596::
	lock
	faceplayer
	msgbox Johto_GoldenrodDeptStore6F_Text_83D45A3, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodDeptStore6F object 3 (local id 3, gfx 0x11) at 13,10
Johto_GoldenrodDeptStore6F_EventScript_83D461E::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x103F, Johto_GoldenrodDeptStore6F_EventScript_83D46C4
	setvar VAR_0x8008, 5
	copyvar VAR_0x8004, VAR_0x8008
	specialvar VAR_RESULT, GetInGameTradeSpeciesInfo
	goto_if_eq VAR_RESULT, 0, Johto_GoldenrodDeptStore6F_EventScript_83D46AC
	copyvar VAR_0x8009, VAR_RESULT
	msgbox Johto_GoldenrodDeptStore6F_Text_83D46CE, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Johto_GoldenrodDeptStore6F_EventScript_83D46AC
	special ChoosePartyMon
	waitstate
	copyvar VAR_0x800A, VAR_0x8004
	goto_if_eq VAR_0x8004, 255, Johto_GoldenrodDeptStore6F_EventScript_83D46AC
	copyvar VAR_0x8005, VAR_0x800A
	specialvar VAR_RESULT, GetTradeSpecies
	copyvar VAR_0x800B, VAR_RESULT
	compare_var_to_var VAR_RESULT, VAR_0x8009
	goto_if_ne Johto_GoldenrodDeptStore6F_EventScript_83D46B6
	copyvar VAR_0x8004, VAR_0x8008
	copyvar VAR_0x8005, VAR_0x800A
	special CreateInGameTradePokemon
	special DoInGameTradeScene
	waitstate
	bufferspeciesname 0, VAR_0x8009
	msgbox Johto_GoldenrodDeptStore6F_Text_83D470F, MSGBOX_DEFAULT
	closemessage
	setflag FLAG_QZ_0x103F
	release
	end

Johto_GoldenrodDeptStore6F_EventScript_83D46AC::
	msgbox Johto_GoldenrodDeptStore6F_Text_83D4763, MSGBOX_DEFAULT
	release
	end

Johto_GoldenrodDeptStore6F_EventScript_83D46B6::
	bufferspeciesname 0, VAR_0x8009
	msgbox Johto_GoldenrodDeptStore6F_Text_83D4738, MSGBOX_DEFAULT
	release
	end

Johto_GoldenrodDeptStore6F_EventScript_83D46C4::
	msgbox Johto_GoldenrodDeptStore6F_Text_83D4783, MSGBOX_DEFAULT
	release
	end

@ Johto_GoldenrodDeptStore6F object 2 (local id 2, gfx 0x3E85) at 14,6
Johto_GoldenrodDeptStore6F_EventScript_83D479F::
	lock
	faceplayer
	msgbox Johto_GoldenrodDeptStore6F_Text_83D47C6, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodDeptStore6F object 1 (local id 1, gfx 0x24) at 11,8
Johto_GoldenrodDeptStore6F_EventScript_83D47AC::
	lock
	faceplayer
	msgbox Johto_GoldenrodDeptStore6F_Text_83D4894, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodDeptStore6F sign / bg event at 24,4
Johto_GoldenrodDeptStore6F_EventScript_83D47B9::
	lock
	faceplayer
	msgbox Johto_GoldenrodDeptStore6F_Text_83D48E8, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_GoldenrodDeptStore6F_Text_83D45A3::
	.string "{PLAYER}! Fancy seeing you here!\p"
	.string "I’m picking up something for\n"
	.string "my MILTANK.\p"
	.string "If you want another battle, come to\n"
	.string "the FIGHTING DOJO!$"

Johto_GoldenrodDeptStore6F_Text_83D46CE::
	.string "I collect POKéMON!\n"
	.string "Do you have a {STR_VAR_1}?\p"
	.string "Want to trade it for my\n"
	.string "{STR_VAR_2}?$"

Johto_GoldenrodDeptStore6F_Text_83D470F::
	.string "Yay! I got myself {STR_VAR_1}!\p"
	.string "{STR_VAR_2}, you take care!$"

Johto_GoldenrodDeptStore6F_Text_83D4738::
	.string "Uh, no, I don’t think so.\n"
	.string "That isn’t a {STR_VAR_1}.$"

Johto_GoldenrodDeptStore6F_Text_83D4763::
	.string "You don’t want to trade? Aww...$"

Johto_GoldenrodDeptStore6F_Text_83D4783::
	.string "Hi, how’s my PANCHAM doing?$"

Johto_GoldenrodDeptStore6F_Text_83D47C6::
	.string "Do you listen to LUCKY CHANNEL?\p"
	.string "They say if you want to win,\n"
	.string "you should trade POKéMON with as\l"
	.string "many people as possible to get\l"
	.string "different ID numbers.\p"
	.string "However, trading isn’t as widely\n"
	.string "available as it once was.$"

Johto_GoldenrodDeptStore6F_Text_83D4894::
	.string "If you’re tired, try the vending\n"
	.string "machine’s drinks.\p"
	.string "Your POKéMON will love them too.$"

Johto_GoldenrodDeptStore6F_Text_83D48E8::
	.string "Take a Break from Shopping!\p"
	.string "6F TRANQUIL SQUARE$"
