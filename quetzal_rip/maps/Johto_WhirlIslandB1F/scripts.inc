Johto_WhirlIslandB1F_MapScripts::
	.byte 0

@ Johto_WhirlIslandB1F trigger at 21,67 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 20,66 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 27,67 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 28,68 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 29,67 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 28,66 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 42,41 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 43,41 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 44,41 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 58,42 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 60,38 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 56,33 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 57,33 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 58,33 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 59,33 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 60,33 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 61,33 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 26,32 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 27,32 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 28,32 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 33,57 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 34,56 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 33,55 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 32,56 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 20,35 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 21,35 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 22,35 (0 == 0)
Johto_WhirlIslandB1F_EventScript_8400E28::
	setweather 2
	doweather
	end

@ Johto_WhirlIslandB1F trigger at 16,7 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 17,6 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 15,6 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 31,15 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 31,17 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 30,16 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 31,23 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 32,24 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 42,39 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 43,39 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 44,39 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 75,6 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 75,4 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 76,5 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 74,5 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 71,8 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 70,7 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 71,6 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 42,37 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 43,38 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 44,37 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 55,15 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 55,17 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 56,16 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 64,32 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 65,33 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 64,34 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 63,33 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 20,34 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 21,34 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 22,34 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 54,24 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 55,23 (0 == 0)
Johto_WhirlIslandB1F_EventScript_8400E32::
	setweather 3
	doweather
	end

@ Johto_WhirlIslandB1F object 1 (local id 1, gfx 0x3B) at 65,62
Johto_WhirlIslandB1F_EventScript_8400E37::
	setorcopyvar VAR_0x8000, 0x88
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_WhirlIslandB1F object 2 (local id 2, gfx 0x3B) at 29,16
Johto_WhirlIslandB1F_EventScript_8400E44::
	setorcopyvar VAR_0x8000, 0x20
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_WhirlIslandB1F object 3 (local id 3, gfx 0x3B) at 43,27
Johto_WhirlIslandB1F_EventScript_8400E51::
	setorcopyvar VAR_0x8000, 0x46
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_WhirlIslandB1F object 6 (local id 6, gfx 0x3B) at 12,64
Johto_WhirlIslandB1F_EventScript_8400E5E::
	setorcopyvar VAR_0x8000, 0x44
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_WhirlIslandB1F object 4 (local id 4, gfx 0x3B) at 57,9
Johto_WhirlIslandB1F_EventScript_8400E6B::
	setorcopyvar VAR_0x8000, 0x78
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end
