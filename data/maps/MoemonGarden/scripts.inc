@ Moemon Co-op: the MOeMON GARDEN (src/mp/mp_garden.c does the rest: growth marks at load, the Pokemon, the board's menu)
MoemonGarden_MapScripts::
	.byte 0

MoemonGarden_EventScript_Board::
	lockall
	special MP_Garden_OpenBoard
	waitstate
	goto_if_eq VAR_RESULT, 1, MoemonGarden_EventScript_Decorate
	releaseall
	end

@ DECORATE: the SECRET BASE's decorating (decoration.c, its MOeMON GARDEN mode), still locked; it ends with DecorateDone
MoemonGarden_EventScript_Decorate::
	special MP_Garden_ShowDecorationMenu
	end

MoemonGarden_EventScript_DecorateDone::
	releaseall
	end
