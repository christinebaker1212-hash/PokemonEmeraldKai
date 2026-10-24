@ Moemon Co-op: the MOeMON GARDEN (src/mp/mp_garden.c does the rest: growth marks at load, the Pokemon, the board's menu)
MoemonGarden_MapScripts::
	.byte 0

MoemonGarden_EventScript_Board::
	lockall
	special MP_Garden_OpenBoard
	waitstate
	releaseall
	end
