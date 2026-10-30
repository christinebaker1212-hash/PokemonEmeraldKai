Johto_WhirlIslandB2F_MapScripts::
	.byte 0

@ Johto_WhirlIslandB2F object 3 (local id 3, gfx 0x3A9E) at 27,10
Johto_WhirlIslandB2F_EventScript_8400E81::
	setorcopyvar VAR_0x8000, ITEM_MAX_REVIVE
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_WhirlIslandB2F object 2 (local id 2, gfx 0x3A9E) at 47,29
Johto_WhirlIslandB2F_EventScript_8400E8E::
	setorcopyvar VAR_0x8000, ITEM_FULL_RESTORE
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_WhirlIslandB2F object 1 (local id 1, gfx 0x3A9E) at 59,55
Johto_WhirlIslandB2F_EventScript_8400E9B::
	setorcopyvar VAR_0x8000, ITEM_NUGGET
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_WhirlIslandB2F trigger at 41,69 (0 == 0)
@ Johto_WhirlIslandB2F trigger at 35,86 (0 == 0)
@ Johto_WhirlIslandB2F trigger at 36,86 (0 == 0)
@ Johto_WhirlIslandB2F trigger at 34,86 (0 == 0)
@ Johto_WhirlIslandB2F trigger at 35,87 (0 == 0)
@ Johto_WhirlIslandB2F trigger at 57,92 (0 == 0)
@ Johto_WhirlIslandB2F trigger at 58,93 (0 == 0)
@ Johto_WhirlIslandB2F trigger at 56,93 (0 == 0)
@ Johto_WhirlIslandB2F trigger at 57,94 (0 == 0)
@ Johto_WhirlIslandB2F trigger at 57,93 (0 == 0)
Johto_WhirlIslandB2F_EventScript_8400EA8::
	setweather 6
	doweather
	end
