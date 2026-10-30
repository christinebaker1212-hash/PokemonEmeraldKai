Johto_GoldenrodDeptStore1F_MapScripts::
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Johto_GoldenrodDeptStore1F_OnFrame_83D3830
	map_script MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE, Johto_GoldenrodDeptStore1F_OnWarp_83D383A
	.byte 0

Johto_GoldenrodDeptStore1F_OnFrame_83D3830:
	map_script_2 VAR_QZ_0x5015, 1, Quetzal_EventScript_83D384A
	.2byte 0

Johto_GoldenrodDeptStore1F_OnWarp_83D383A:
	map_script_2 VAR_QZ_0x5015, 1, Quetzal_EventScript_83D3844
	.2byte 0

@ Johto_GoldenrodDeptStore1F object 4 (local id 4, gfx 0x3EBE) at 15,5
Johto_GoldenrodDeptStore1F_EventScript_83D39D4::
	lock
	faceplayer
	msgbox Johto_GoldenrodDeptStore1F_Text_83D3A14, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodDeptStore1F object 3 (local id 3, gfx 0x30) at 21,9
Johto_GoldenrodDeptStore1F_EventScript_83D39E1::
	lock
	faceplayer
	msgbox Johto_GoldenrodDeptStore1F_Text_83D3A35, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodDeptStore1F object 1 (local id 1, gfx 0x18) at 11,9
Johto_GoldenrodDeptStore1F_EventScript_83D39EE::
	lock
	faceplayer
	msgbox Johto_GoldenrodDeptStore1F_Text_83D3A95, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodDeptStore1F object 2 (local id 2, gfx 0xB) at 10,9
Johto_GoldenrodDeptStore1F_EventScript_83D39FB::
	lock
	faceplayer
	msgbox Johto_GoldenrodDeptStore1F_Text_83D3AB5, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodDeptStore1F sign / bg event at 21,4
Johto_GoldenrodDeptStore1F_EventScript_83D3A08::
	lock
	msgbox Johto_GoldenrodDeptStore1F_Text_83D3AFB, MSGBOX_SIGN
	closemessage
	release
	end

Johto_GoldenrodDeptStore1F_Text_83D3A14::
	.string "Welcome to GOLDENROD DEPT.STORE.$"

Johto_GoldenrodDeptStore1F_Text_83D3A35::
	.string "The DEPT.STORE has a decent\n"
	.string "selection.\p"
	.string "But some items are only available\n"
	.string "as GAME CORNER prizes.$"

Johto_GoldenrodDeptStore1F_Text_83D3A95::
	.string "I’m raring to shop again today!$"

Johto_GoldenrodDeptStore1F_Text_83D3AB5::
	.string "Mom’s good at bargain hunting.\p"
	.string "She always buys stuff at lower\n"
	.string "prices.$"

Johto_GoldenrodDeptStore1F_Text_83D3AFB::
	.string "1F SERVICE COUNTER\n"
	.string "YOU ARE HERE.\p"
	.string "2F ITEMS & BATTLE TOOLS\p"
	.string "3F TRAINING & MINTS\p"
	.string "4F EVOLUTION & FORMS\p"
	.string "5F MEGA, Z & TERA\p"
	.string "6F TRANQUIL SQUARE\p"
	.string "ROOFTOP LOOKOUT$"
