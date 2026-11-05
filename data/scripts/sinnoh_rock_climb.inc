@ Moemon Kai: SINNOH's rock walls, Platinum's FieldMoves_RockyWall with Platinum's own lines (field_moves bank 142,
@ messages 18-20): src/sinnoh/s3d_misc.c S3D_RockClimbSlot / S3D_RockClimbStart, field_control_avatar.c
EventScript_SinnohRockClimb::
	lockall
	special S3D_RockClimbSlot
	goto_if_eq VAR_RESULT, PARTY_SIZE, EventScript_SinnohRockClimb_Cant
	message SinnohP_T_142_18
	waitmessage
	yesnobox 20, 8
	goto_if_eq VAR_RESULT, NO, EventScript_SinnohRockClimb_End
	special S3D_RockClimbSlot
	message SinnohP_T_142_19
	waitmessage
	closemessage
	special S3D_RockClimbStart
	waitstate
EventScript_SinnohRockClimb_End:
	closemessage
	releaseall
	end

EventScript_SinnohRockClimb_Cant:
	message SinnohP_T_142_20
	waitmessage
	waitbuttonpress
	closemessage
	releaseall
	end

@ ROCK CLIMB and DEFOG from the party menu (the POKeMON's nickname already in gStringVar1)
EventScript_SinnohRockClimbFromMenu::
	lockall
	message SinnohP_T_142_19
	waitmessage
	closemessage
	special S3D_RockClimbStart
	waitstate
	releaseall
	end

EventScript_SinnohDefog::
	lockall
	message SinnohP_T_142_16
	waitmessage
	closemessage
	playse SE_M_GUST
	special S3D_DefogClear
	releaseall
	end
