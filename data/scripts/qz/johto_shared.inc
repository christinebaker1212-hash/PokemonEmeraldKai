@ Johto_AzaleaPokecenter1F object 1 (local id 1, gfx 0x3A) at 7,2
@ Johto_BlackthornPokecenter1F object 1 (local id 1, gfx 0x3A) at 7,2
@ Johto_CherrygrovePokecenter1F object 1 (local id 1, gfx 0x3A) at 7,2
@ Johto_CianwoodPokecenter1F object 1 (local id 1, gfx 0x3A) at 7,2
@ Johto_EcruteakPokecenter1F object 1 (local id 1, gfx 0x3A) at 7,2
@ Johto_GoldenrodPokecenter1F object 1 (local id 1, gfx 0x3A) at 7,2
@ Johto_IndigoPlateauPokecenter1F object 1 (local id 1, gfx 0x3A) at 19,10
@ Johto_MahoganyPokecenter1F object 1 (local id 1, gfx 0x3A) at 7,2
@ Johto_SilverCavePokecenter1F object 1 (local id 1, gfx 0x3A) at 7,2
@ Johto_OlivinePokecenter1F object 1 (local id 1, gfx 0x3A) at 7,2
@ Johto_Route32Pokecenter1F object 1 (local id 1, gfx 0x3A) at 7,2
@ Johto_SafariZoneGatePokecenter1F object 1 (local id 1, gfx 0x3A) at 7,2
@ Johto_VioletPokecenter1F object 1 (local id 1, gfx 0x3A) at 7,2
Quetzal_EventScript_82E1B64::
	setvar VAR_0x800B, 1
	call Quetzal_EventScript_835F95C
	waitmessage
	waitbuttonpress
	release
	end

@ Johto_GoldenrodDeptStore2F sign / bg event at 14,5
@ Johto_GoldenrodDeptStore2F sign / bg event at 13,5
@ Johto_GoldenrodDeptStore2F sign / bg event at 12,5
@ Johto_GoldenrodDeptStore4F sign / bg event at 15,5
@ Johto_GoldenrodDeptStore4F sign / bg event at 14,5
@ Johto_GoldenrodDeptStore4F sign / bg event at 13,5
@ Johto_GoldenrodDeptStore4F sign / bg event at 12,5
@ Johto_GoldenrodDeptStore6F sign / bg event at 12,5
@ Johto_GoldenrodDeptStore6F sign / bg event at 13,5
@ Johto_GoldenrodDeptStore6F sign / bg event at 14,5
@ Johto_GoldenrodDeptStore6F sign / bg event at 15,5
@ Johto_GoldenrodDeptStoreRoof sign / bg event at 10,5
@ Johto_GoldenrodDeptStoreRoof sign / bg event at 11,5
@ Johto_GoldenrodDeptStoreRoof sign / bg event at 12,5
@ Johto_GoldenrodDeptStoreRoofAlt sign / bg event at 10,5
@ Johto_GoldenrodDeptStoreRoofAlt sign / bg event at 11,5
@ Johto_GoldenrodDeptStoreRoofAlt sign / bg event at 12,5
Quetzal_EventScript_8306A5D::
	lockall
	message Quetzal_Text_8306D36
	waitmessage
	showmoneybox 0, 0, 0
	goto Quetzal_EventScript_8306A6E

Quetzal_EventScript_8306A6E::
	multichoice 0x10, 0, 0x2A, 0
	nop
	copyvar VAR_TEMP_1, VAR_RESULT
	copyvar VAR_0x8000, VAR_TEMP_1
	goto_if_eq VAR_0x8000, 0, Quetzal_EventScript_8306AAD
	goto_if_eq VAR_0x8000, 1, Quetzal_EventScript_8306AB8
	goto_if_eq VAR_0x8000, 2, Quetzal_EventScript_8306AC3
	msgbox Quetzal_Text_8306DCC, MSGBOX_DEFAULT
	goto Quetzal_EventScript_8306C27

Quetzal_EventScript_8306AAD::
	setvar VAR_TEMP_0, 0x23
	goto Quetzal_EventScript_8306AF8

Quetzal_EventScript_8306AB8::
	setvar VAR_TEMP_0, 0x24
	goto Quetzal_EventScript_8306AF8

Quetzal_EventScript_8306AC3::
	setvar VAR_TEMP_0, 0x25
	goto Quetzal_EventScript_8306AF8

Quetzal_EventScript_8306ACE::
	checkmoney 0xC8, 0
	return

Quetzal_EventScript_8306AD5::
	checkmoney 0x12C, 0
	return

Quetzal_EventScript_8306ADC::
	checkmoney 0x15E, 0
	return

Quetzal_EventScript_8306AE3::
	removemoney 0xC8, 0
	return

Quetzal_EventScript_8306AEA::
	removemoney 0x12C, 0
	return

Quetzal_EventScript_8306AF1::
	removemoney 0x15E, 0
	return

Quetzal_EventScript_8306AF8::
	call_if_eq VAR_TEMP_1, 0, Quetzal_EventScript_8306ACE
	call_if_eq VAR_TEMP_1, 1, Quetzal_EventScript_8306AD5
	call_if_eq VAR_TEMP_1, 2, Quetzal_EventScript_8306ADC
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_8306C0B
	checkitemspace VAR_TEMP_0, 1
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_8306C19
	call_if_eq VAR_TEMP_1, 0, Quetzal_EventScript_8306AE3
	call_if_eq VAR_TEMP_1, 1, Quetzal_EventScript_8306AEA
	call_if_eq VAR_TEMP_1, 2, Quetzal_EventScript_8306AF1
	updatemoneybox 0
	bufferitemname 0, VAR_TEMP_0
	playse SE_VEND
	msgbox Quetzal_Text_8306D6A, MSGBOX_DEFAULT
	additem VAR_TEMP_0, 1
	bufferitemname 1, VAR_TEMP_0
	bufferstdstring 2, 0xE
	msgbox Quetzal_Text_8360D7B, MSGBOX_DEFAULT
	random 0x40
	goto_if_ne VAR_RESULT, 0, Quetzal_EventScript_8306BFF
	checkitemspace VAR_TEMP_0, 1
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_8306C19
	playse SE_VEND
	msgbox Quetzal_Text_8306D8B, MSGBOX_DEFAULT
	additem VAR_TEMP_0, 1
	bufferitemname 1, VAR_TEMP_0
	bufferstdstring 2, 0xE
	msgbox Quetzal_Text_8360D7B, MSGBOX_DEFAULT
	random 0x40
	goto_if_ne VAR_RESULT, 0, Quetzal_EventScript_8306BFF
	checkitemspace VAR_TEMP_0, 1
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_8306C19
	playse SE_VEND
	msgbox Quetzal_Text_8306D8B, MSGBOX_DEFAULT
	additem VAR_TEMP_0, 1
	bufferitemname 1, VAR_TEMP_0
	bufferstdstring 2, 0xE
	msgbox Quetzal_Text_8360D7B, MSGBOX_DEFAULT
	goto Quetzal_EventScript_8306BFF

Quetzal_EventScript_8306BFF::
	message Quetzal_Text_8306D36
	waitmessage
	goto Quetzal_EventScript_8306A6E

Quetzal_EventScript_8306C0B::
	msgbox Quetzal_Text_8306DBA, MSGBOX_DEFAULT
	goto Quetzal_EventScript_8306C27

Quetzal_EventScript_8306C19::
	msgbox Quetzal_Text_8360D6A, MSGBOX_DEFAULT
	goto Quetzal_EventScript_8306C27

Quetzal_EventScript_8306C27::
	hidemoneybox
	releaseall
	end

Quetzal_EventScript_835F95C::
	lock
	faceplayer
	setvar VAR_0x8004, 0
	specialvar VAR_RESULT, CountPlayerTrainerStars
	goto_if_eq VAR_RESULT, 4, Quetzal_EventScript_835FABB
	goto Quetzal_EventScript_835F97F

Quetzal_EventScript_835F97F::
	incrementgamestat 0x2B
	call_if_eq VAR_0x8004, 0, Quetzal_EventScript_835F9AC
	call_if_eq VAR_0x8004, 1, Quetzal_EventScript_835F9B2
	waitmessage
	call Quetzal_EventScript_835F9B8
	goto_if_unset FLAG_QZ_0x111, Quetzal_EventScript_835FA96
	goto Quetzal_EventScript_835F9D6

Quetzal_EventScript_835F9AC::
	message Quetzal_Text_83609E1
	return

Quetzal_EventScript_835F9B2::
	message Quetzal_Text_8360C39
	return

Quetzal_EventScript_835F9B8::
	applymovement VAR_0x800B, Quetzal_Movement_8360805
	waitmovement 0
	dofieldeffect 0x19
	waitfieldeffect 0x19
	applymovement VAR_0x800B, Quetzal_Movement_836080B
	waitmovement 0
	special QzHealPlayerPartyEx
	return

Quetzal_EventScript_835F9D6::
	specialvar VAR_RESULT, QzPlayerNotAtTrainerHillEntrance
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_835FA07
	specialvar VAR_RESULT, BufferUnionRoomPlayerName
	copyvar VAR_0x8008, VAR_RESULT
	goto_if_eq VAR_0x8008, 0, Quetzal_EventScript_835FA07
	goto_if_eq VAR_0x8008, 1, Quetzal_EventScript_835FA6C
	end

Quetzal_EventScript_835FA07::
	goto_if_eq VAR_0x8004, 1, Quetzal_EventScript_835FA22
	applymovement VAR_0x800B, Quetzal_Movement_835FB23
	waitmovement 0
	goto Quetzal_EventScript_835FB26

Quetzal_EventScript_835FA22::
	message Quetzal_Text_8360C69
	waitmessage
	applymovement VAR_0x800B, Quetzal_Movement_835FB23
	waitmovement 0
	goto Quetzal_EventScript_835FB26

Quetzal_EventScript_835FA6C::
	goto_if_set FLAG_QZ_0xA98, Quetzal_EventScript_835FA07
	msgbox Quetzal_Text_8360A11, MSGBOX_DEFAULT
	setflag FLAG_QZ_0xA98
	message Quetzal_Text_836713F
	waitmessage
	applymovement VAR_0x800B, Quetzal_Movement_835FB23
	waitmovement 0
	message Quetzal_Text_8360A54
	return

Quetzal_EventScript_835FA96::
	specialvar VAR_RESULT, IsPokerusInParty
	goto_if_eq VAR_RESULT, 1, Quetzal_EventScript_835FAB2
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_835F9D6
	end

Quetzal_EventScript_835FAB2::
	message Quetzal_Text_83611E8
	setflag FLAG_QZ_0x111
	return

Quetzal_EventScript_835FABB::
	goto_if_set FLAG_QZ_0x159, Quetzal_EventScript_835FAFF
	setflag FLAG_QZ_0x159
	msgbox Quetzal_Text_8360A6E, MSGBOX_DEFAULT
	playse SE_PIN
	applymovement VAR_0x800B, Quetzal_Movement_83607F9
	waitmovement 0
	applymovement VAR_0x800B, Quetzal_Movement_83607FB
	waitmovement 0
	msgbox Quetzal_Text_8360AD9, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Quetzal_EventScript_835FB18
	message Quetzal_Text_8360C80
	return

Quetzal_EventScript_835FAFF::
	msgbox Quetzal_Text_8360BFB, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Quetzal_EventScript_835FB18
	message Quetzal_Text_8360C80
	return

Quetzal_EventScript_835FB18::
	setvar VAR_0x8004, 1
	goto Quetzal_EventScript_835F97F

Quetzal_EventScript_835FB26::
	closemessage
	applymovement LOCALID_PLAYER, Quetzal_Movement_8360811
	waitmovement 0
	release
	end

Quetzal_EventScript_83601F0::
	copyvar VAR_0x8000, VAR_0x8008
	goto_if_eq VAR_0x8000, 1, Quetzal_EventScript_836024E
	goto_if_eq VAR_0x8000, 2, Quetzal_EventScript_8360255
	goto_if_eq VAR_0x8000, 3, Quetzal_EventScript_8360262
	goto_if_eq VAR_0x8000, 4, Quetzal_EventScript_836026F
	goto_if_eq VAR_0x8000, 5, Quetzal_EventScript_836027C
	goto_if_eq VAR_0x8000, 6, Quetzal_EventScript_8360289
	goto_if_eq VAR_0x8000, 7, Quetzal_EventScript_836028A
	goto_if_eq VAR_0x8000, 8, Quetzal_EventScript_836029A
	return

Quetzal_EventScript_836024E::
	settrainerflag 0x1009
	settrainerflag 0x1166
	return

Quetzal_EventScript_8360255::
	settrainerflag 0x1168
	settrainerflag 0x1140
	settrainerflag 0x1197
	settrainerflag 0x11E1
	return

Quetzal_EventScript_8360262::
	settrainerflag 0x11F6
	settrainerflag 0x1081
	settrainerflag 0x112B
	settrainerflag 0x10F5
	return

Quetzal_EventScript_836026F::
	settrainerflag 0x1024
	settrainerflag 0x110C
	settrainerflag 0x11D9
	settrainerflag 0x11C2
	return

Quetzal_EventScript_836027C::
	settrainerflag 0x124D
	settrainerflag 0x1031
	settrainerflag 0x12F7
	settrainerflag 0x11A4
	return

Quetzal_EventScript_8360289::
	return

Quetzal_EventScript_836028A::
	settrainerflag 0x115E
	settrainerflag 0x12BD
	settrainerflag 0x1099
	settrainerflag 0x134A
	settrainerflag 0x11B3
	return

Quetzal_EventScript_836029A::
	settrainerflag 0x103A
	settrainerflag 0x1149
	settrainerflag 0x1113
	settrainerflag 0x11EC
	settrainerflag 0x1200
	return

Quetzal_EventScript_83602AA::
	msgbox Quetzal_Text_8360DB1, MSGBOX_DEFAULT
	release
	end

Quetzal_EventScript_83602B4::
	msgbox Quetzal_Text_8360DB1, MSGBOX_DEFAULT
	return

Quetzal_EventScript_83602D4::
	playfanfare MUS_OBTAIN_BADGE
	waitfanfare
	return

Quetzal_EventScript_83602D9::
	fadescreen 1
	special QzHoennPostHealEventCheck
	playfanfare MUS_HEAL
	waitfanfare
	special QzHealPlayerPartyEx
	fadescreen 0
	return

Quetzal_EventScript_836063E::
	fadescreen 1
	special ChangePokemonNickname
	waitstate
	return

@ Johto_KurtsHouse sign / bg event at 2,1
@ Johto_KurtsHouse sign / bg event at 3,1
@ Johto_AzaleaPokecenter1F sign / bg event at 2,1
@ Johto_AzaleaPokecenter1F sign / bg event at 1,1
@ Johto_BlackthornPokecenter1F sign / bg event at 2,1
@ Johto_BlackthornPokecenter1F sign / bg event at 1,1
@ Johto_CherrygrovePokecenter1F sign / bg event at 2,1
@ Johto_CherrygrovePokecenter1F sign / bg event at 1,1
@ Johto_CianwoodPokecenter1F sign / bg event at 2,1
@ Johto_CianwoodPokecenter1F sign / bg event at 1,1
@ Johto_EcruteakPokecenter1F sign / bg event at 2,1
@ Johto_EcruteakPokecenter1F sign / bg event at 1,1
@ Johto_GoldenrodPokecenter1F sign / bg event at 2,1
@ Johto_GoldenrodPokecenter1F sign / bg event at 1,1
@ Johto_MahoganyPokecenter1F sign / bg event at 2,1
@ Johto_MahoganyPokecenter1F sign / bg event at 1,1
@ Johto_SilverCavePokecenter1F sign / bg event at 2,1
@ Johto_SilverCavePokecenter1F sign / bg event at 1,1
@ Johto_PlayersNeighborsHouse sign / bg event at 9,1
@ Johto_PlayersNeighborsHouse sign / bg event at 10,1
@ Johto_ElmsHouse sign / bg event at 0,1
@ Johto_ElmsHouse sign / bg event at 1,1
@ Johto_PlayersHouse2F sign / bg event at 5,1
@ Johto_PlayersHouse2F sign / bg event at 6,1
@ Johto_OlivinePokecenter1F sign / bg event at 2,1
@ Johto_OlivinePokecenter1F sign / bg event at 1,1
@ Johto_Route32Pokecenter1F sign / bg event at 2,1
@ Johto_Route32Pokecenter1F sign / bg event at 1,1
@ Johto_SafariZoneGatePokecenter1F sign / bg event at 2,1
@ Johto_SafariZoneGatePokecenter1F sign / bg event at 1,1
@ Johto_VioletPokecenter1F sign / bg event at 2,1
@ Johto_VioletPokecenter1F sign / bg event at 1,1
@ Johto_EarlsPokemonAcademy sign / bg event at 1,2
@ Johto_EarlsPokemonAcademy sign / bg event at 0,2
Quetzal_EventScript_8360850::
	msgbox Quetzal_Text_8396B35, MSGBOX_SIGN
	end

@ Johto_GoldenrodCity sign / bg event at 50,22
@ Johto_ElmsLab sign / bg event at 11,4
Quetzal_EventScript_836086B::
	msgbox Quetzal_Text_8396BDB, MSGBOX_SIGN
	end

Quetzal_EventScript_8361B12::
	getpartysize
	subvar VAR_RESULT, 1
	copyvar VAR_0x8004, VAR_RESULT
	return

@ Johto_AzaleaTown object 7 (local id 7, gfx 0x3C) at 12,7
@ Johto_Route26South object 5 (local id 5, gfx 0x3C) at 19,9
@ Johto_Route29 object 9 (local id 9, gfx 0x3C) at 15,11
@ Johto_Route30 object 10 (local id 10, gfx 0x3C) at 30,4
@ Johto_Route30 object 11 (local id 11, gfx 0x3C) at 23,38
@ Johto_Route31 object 8 (local id 8, gfx 0x3C) at 16,11
@ Johto_Route33 object 3 (local id 3, gfx 0x3C) at 30,26
@ Johto_Route35 object 14 (local id 14, gfx 0x3C) at 11,25
@ Johto_Route36 object 7 (local id 7, gfx 0x3C) at 20,13
@ Johto_Route37 object 5 (local id 5, gfx 0x3C) at 33,24
@ Johto_Route38 object 10 (local id 10, gfx 0x3C) at 21,24
@ Johto_Route39 object 7 (local id 7, gfx 0x3C) at 12,27
@ Johto_Route42 object 7 (local id 7, gfx 0x3C) at 40,19
@ Johto_Route43 object 6 (local id 6, gfx 0x3C) at 3,27
@ Johto_Route43 object 7 (local id 7, gfx 0x3C) at 4,25
@ Johto_Route44 object 10 (local id 10, gfx 0x3C) at 7,10
@ Johto_Route45 object 13 (local id 13, gfx 0x3C) at 24,87
@ Johto_Route46 object 5 (local id 5, gfx 0x3C) at 9,11
@ Johto_Route47 object 11 (local id 11, gfx 0x3C) at 51,8
@ Johto_VioletCity object 9 (local id 9, gfx 0x3C) at 18,49
Quetzal_EventScript_836266F::
	special ObjectEventInteractionGetBerryTreeData
	copyvar VAR_0x8000, VAR_0x8004
	goto_if_eq VAR_0x8000, 0, Quetzal_EventScript_8362698
	goto_if_eq VAR_0x8000, 1, Quetzal_EventScript_8362746
	end

Quetzal_EventScript_8362698::
	end

Quetzal_EventScript_8362746::
	buffernumberstring 1, VAR_0x8006
	lock
	faceplayer
	special ObjectEventInteractionGetBerryCountString
	msgbox Quetzal_Text_8362911, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Quetzal_EventScript_836276D
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_836279E

Quetzal_EventScript_836276D::
	special ObjectEventInteractionPickBerryTree
	goto_if_eq VAR_0x8004, 0, Quetzal_EventScript_8362795
	special IncrementDailyPickedBerries
	special ObjectEventInteractionRemoveBerryTree
	message Quetzal_Text_836293E
	playfanfare MUS_OBTAIN_BERRY
	waitmessage
	waitfanfare
	waitbuttonpress
	message Quetzal_Text_8362953
	waitmessage
	waitbuttonpress
	release
	end

Quetzal_EventScript_8362795::
	message Quetzal_Text_83629B2
	waitmessage
	waitbuttonpress
	release
	end

Quetzal_EventScript_836279E::
	message Quetzal_Text_83629EE
	waitmessage
	waitbuttonpress
	release
	end

@ Johto_IlexForest object 4 (local id 4, gfx 0x52) at 32,40
@ Johto_NationalPark object 10 (local id 10, gfx 0x52) at 26,3
@ Johto_Route29 object 6 (local id 6, gfx 0x52) at 31,23
@ Johto_Route30 object 5 (local id 5, gfx 0x52) at 30,10
@ Johto_Route31 object 7 (local id 7, gfx 0x52) at 32,15
@ Johto_Route32 object 8 (local id 8, gfx 0x52) at 18,20
@ Johto_Route32 object 18 (local id 18, gfx 0x52) at 22,79
@ Johto_Route35 object 10 (local id 10, gfx 0x52) at 31,6
@ Johto_Route42 object 3 (local id 3, gfx 0x52) at 36,15
@ Johto_VioletCity object 8 (local id 8, gfx 0x52) at 43,38
Quetzal_EventScript_837EEDF::
	lockall
	setvar VAR_0x8004, 0xF
	special QzHasBadgeForFieldMove
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_837EF28
	buffermovename 1, MOVE_CUT
	msgbox Quetzal_Text_837F0F3, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_837EF32
	goto Quetzal_EventScript_837EF17

Quetzal_EventScript_837EF17::
	applymovement VAR_LAST_TALKED, Quetzal_Movement_837EF26
	waitmovement 0
	removeobject VAR_LAST_TALKED
	releaseall
	end

Quetzal_EventScript_837EF28::
	msgbox Quetzal_Text_837F142, MSGBOX_DEFAULT
	releaseall
	end

Quetzal_EventScript_837EF32::
	closemessage
	releaseall
	end

@ Johto_CianwoodCity object 7 (local id 7, gfx 0x56) at 10,26
@ Johto_CianwoodCity object 8 (local id 8, gfx 0x56) at 10,27
@ Johto_CianwoodCity object 9 (local id 9, gfx 0x56) at 11,27
@ Johto_CianwoodCity object 10 (local id 10, gfx 0x56) at 12,31
@ Johto_CianwoodCity object 11 (local id 11, gfx 0x56) at 13,31
@ Johto_CianwoodCity object 12 (local id 12, gfx 0x56) at 12,30
@ Johto_CianwoodGym object 9 (local id 9, gfx 0x56) at 11,2
@ Johto_CianwoodGym object 10 (local id 10, gfx 0x56) at 9,2
@ Johto_CliffCave object 2 (local id 2, gfx 0x56) at 21,25
@ Johto_CliffCave object 3 (local id 3, gfx 0x56) at 21,24
@ Johto_CliffCave object 4 (local id 4, gfx 0x56) at 22,24
@ Johto_CliffCave object 5 (local id 5, gfx 0x56) at 11,8
@ Johto_CliffCave object 6 (local id 6, gfx 0x56) at 31,11
@ Johto_CliffCave object 7 (local id 7, gfx 0x56) at 12,9
@ Johto_CliffCave object 8 (local id 8, gfx 0x56) at 10,7
@ Johto_CliffCave object 9 (local id 9, gfx 0x56) at 21,26
@ Johto_CliffCave object 10 (local id 10, gfx 0x56) at 22,25
@ Johto_CliffCave object 11 (local id 11, gfx 0x56) at 23,24
@ Johto_DarkCaveBlackthornEntrance object 4 (local id 4, gfx 0x56) at 28,5
@ Johto_DarkCaveBlackthornEntrance object 5 (local id 5, gfx 0x56) at 5,6
@ Johto_DarkCaveVioletEntrance object 2 (local id 2, gfx 0x56) at 23,19
@ Johto_DarkCaveVioletEntrance object 3 (local id 3, gfx 0x56) at 18,19
@ Johto_DarkCaveVioletEntrance object 4 (local id 4, gfx 0x56) at 43,12
@ Johto_DarkCaveVioletEntrance object 5 (local id 5, gfx 0x56) at 58,43
@ Johto_IcePathB3F object 2 (local id 2, gfx 0x56) at 12,19
@ Johto_IcePathB3F object 3 (local id 3, gfx 0x56) at 9,20
@ Johto_IcePathB3F object 4 (local id 4, gfx 0x56) at 7,19
@ Johto_IcePathB3F object 5 (local id 5, gfx 0x56) at 13,13
@ Johto_IcePathB3F object 6 (local id 6, gfx 0x56) at 17,13
@ Johto_IcePathB3F object 7 (local id 7, gfx 0x56) at 19,14
@ Johto_IcePathB3F object 8 (local id 8, gfx 0x56) at 21,15
@ Johto_IcePathB3F object 9 (local id 9, gfx 0x56) at 23,17
@ Johto_IcePathB3F object 10 (local id 10, gfx 0x56) at 20,22
@ Johto_IcePathB3F object 11 (local id 11, gfx 0x56) at 14,22
@ Johto_SilverCaveRoom2 object 1 (local id 1, gfx 0x56) at 48,3
@ Johto_SilverCaveRoom2 object 2 (local id 2, gfx 0x56) at 47,2
@ Johto_SilverCaveRoom2 object 3 (local id 3, gfx 0x56) at 3,26
@ Johto_SilverCaveRoom2 object 4 (local id 4, gfx 0x56) at 3,25
@ Johto_SilverCaveRoom2 object 5 (local id 5, gfx 0x56) at 3,24
@ Johto_Route40 object 1 (local id 1, gfx 0x56) at 16,13
@ Johto_Route40 object 2 (local id 2, gfx 0x56) at 9,14
@ Johto_Route40 object 3 (local id 3, gfx 0x56) at 9,16
@ Johto_Route40 object 4 (local id 4, gfx 0x56) at 13,14
@ Johto_TohjoFallsGiovannisRoom object 2 (local id 2, gfx 0x56) at 2,6
@ Johto_TohjoFallsGiovannisRoom object 3 (local id 3, gfx 0x56) at 3,4
@ Johto_TohjoFallsGiovannisRoom object 4 (local id 4, gfx 0x56) at 3,8
@ Johto_TohjoFallsGiovannisRoom object 5 (local id 5, gfx 0x56) at 8,5
@ Johto_TohjoFallsGiovannisRoom object 6 (local id 6, gfx 0x56) at 8,2
@ Johto_TohjoFallsGiovannisRoom object 7 (local id 7, gfx 0x56) at 7,2
@ Johto_TohjoFallsGiovannisRoom object 8 (local id 8, gfx 0x56) at 8,8
@ Johto_VictoryRoad3F object 4 (local id 4, gfx 0x56) at 23,13
@ Johto_VictoryRoad3F object 5 (local id 5, gfx 0x56) at 23,12
@ Johto_VictoryRoad3F object 6 (local id 6, gfx 0x56) at 23,14
@ Johto_VictoryRoad3F object 7 (local id 7, gfx 0x56) at 38,5
@ Johto_VictoryRoad3F object 8 (local id 8, gfx 0x56) at 37,5
Quetzal_EventScript_837EF35::
	lockall
	setvar VAR_0x8004, 0xF9
	special QzHasBadgeForFieldMove
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_837EF9F
	buffermovename 1, MOVE_ROCK_SMASH
	msgbox Quetzal_Text_837F16B, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_837EFA9
	goto Quetzal_EventScript_837EF6D

Quetzal_EventScript_837EF6D::
	applymovement VAR_LAST_TALKED, Quetzal_Movement_837EF9D
	waitmovement 0
	removeobject VAR_LAST_TALKED
	specialvar VAR_RESULT, TryUpdateRusturfTunnelState
	goto_if_eq VAR_RESULT, 1, Quetzal_EventScript_837EF9B
	special RockSmashWildEncounter
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_837EF9B
	waitstate
	releaseall
	end

Quetzal_EventScript_837EF9B::
	releaseall
	end

Quetzal_EventScript_837EF9F::
	msgbox Quetzal_Text_837F1B0, MSGBOX_DEFAULT
	releaseall
	end

Quetzal_EventScript_837EFA9::
	closemessage
	releaseall
	end

@ Johto_BurnedTowerB1F object 6 (local id 6, gfx 0x57) at 6,13
@ Johto_CianwoodGym object 6 (local id 6, gfx 0x57) at 10,8
@ Johto_CianwoodGym object 7 (local id 7, gfx 0x57) at 10,4
@ Johto_CianwoodGym object 8 (local id 8, gfx 0x57) at 10,6
@ Johto_MountMortar1FInside object 10 (local id 10, gfx 0x57) at 22,43
@ Johto_MountMortarB1F object 1 (local id 1, gfx 0x57) at 9,10
@ Johto_SlowpokeWellB1F object 6 (local id 6, gfx 0x57) at 10,3
@ Johto_UnionCaveB1F object 4 (local id 4, gfx 0x57) at 11,11
@ Johto_UnionCaveB1F object 8 (local id 8, gfx 0x57) at 17,5
@ Johto_VictoryRoad2F object 1 (local id 1, gfx 0x57) at 9,11
@ Johto_VictoryRoad2F object 2 (local id 2, gfx 0x57) at 48,12
@ Johto_VictoryRoad1F object 1 (local id 1, gfx 0x57) at 28,42
@ Johto_VictoryRoad1F object 2 (local id 2, gfx 0x57) at 11,11
@ Johto_VictoryRoad1F object 3 (local id 3, gfx 0x57) at 11,13
@ Johto_WhirlIslandB1F object 5 (local id 5, gfx 0x57) at 22,66
Quetzal_EventScript_837EFAC::
	lockall
	setvar VAR_0x8004, 0x46
	special QzHasBadgeForFieldMove
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_837EFEF
	goto_if_set FLAG_QZ_0xAA1, Quetzal_EventScript_837EFF9
	msgbox Quetzal_Text_837F1EB, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_837F003
	closemessage
	goto Quetzal_EventScript_837EFEA

Quetzal_EventScript_837EFEF::
	msgbox Quetzal_Text_837F295, MSGBOX_DEFAULT
	releaseall
	end

Quetzal_EventScript_837EFF9::
	msgbox Quetzal_Text_837F2D5, MSGBOX_DEFAULT
	releaseall
	end

Quetzal_EventScript_837F003::
	closemessage
	releaseall
	end

@ Johto_DragonsDenB1F object 9 (local id 9, gfx 0x3EBB) at 24,22
@ Johto_DragonsDenB1F object 10 (local id 10, gfx 0x3EBB) at 15,39
@ Johto_DragonsDenB1F object 11 (local id 11, gfx 0x3E80) at 15,38
@ Johto_DragonsDenB1F object 12 (local id 12, gfx 0x3E80) at 16,39
@ Johto_DragonsDenB1F object 13 (local id 13, gfx 0x3E80) at 15,39
@ Johto_DragonsDenB1F object 14 (local id 14, gfx 0x3E80) at 16,38
@ Johto_DragonsDenB1F object 15 (local id 15, gfx 0x3E80) at 25,22
@ Johto_DragonsDenB1F object 16 (local id 16, gfx 0x3E80) at 24,22
@ Johto_DragonsDenB1F object 17 (local id 17, gfx 0x3E80) at 24,21
@ Johto_DragonsDenB1F object 18 (local id 18, gfx 0x3E80) at 25,21
@ Johto_Route41 object 15 (local id 15, gfx 0x3EBB) at 27,66
@ Johto_Route41 object 16 (local id 16, gfx 0x3EBB) at 17,42
@ Johto_Route41 object 17 (local id 17, gfx 0x3EBB) at 42,46
@ Johto_Route41 object 18 (local id 18, gfx 0x3EBB) at 52,53
@ Johto_Route41 object 19 (local id 19, gfx 0x3EBB) at 58,76
@ Johto_Route41 object 20 (local id 20, gfx 0x3EBB) at 7,41
@ Johto_Route41 object 21 (local id 21, gfx 0x3E80) at 42,46
@ Johto_Route41 object 22 (local id 22, gfx 0x3E80) at 43,46
@ Johto_Route41 object 23 (local id 23, gfx 0x3E80) at 43,45
@ Johto_Route41 object 24 (local id 24, gfx 0x3E80) at 42,45
@ Johto_Route41 object 25 (local id 25, gfx 0x3E80) at 52,52
@ Johto_Route41 object 26 (local id 26, gfx 0x3E80) at 53,52
@ Johto_Route41 object 27 (local id 27, gfx 0x3E80) at 52,53
@ Johto_Route41 object 28 (local id 28, gfx 0x3E80) at 53,53
@ Johto_Route41 object 29 (local id 29, gfx 0x3E80) at 59,76
@ Johto_Route41 object 30 (local id 30, gfx 0x3E80) at 58,75
@ Johto_Route41 object 31 (local id 31, gfx 0x3E80) at 59,75
@ Johto_Route41 object 32 (local id 32, gfx 0x3E80) at 58,76
@ Johto_Route41 object 33 (local id 33, gfx 0x3E80) at 18,42
@ Johto_Route41 object 34 (local id 34, gfx 0x3E80) at 17,41
@ Johto_Route41 object 35 (local id 35, gfx 0x3E80) at 18,41
@ Johto_Route41 object 36 (local id 36, gfx 0x3E80) at 17,42
@ Johto_Route41 object 37 (local id 37, gfx 0x3E80) at 8,41
@ Johto_Route41 object 38 (local id 38, gfx 0x3E80) at 7,40
@ Johto_Route41 object 39 (local id 39, gfx 0x3E80) at 8,40
@ Johto_Route41 object 40 (local id 40, gfx 0x3E80) at 7,41
@ Johto_Route41 object 41 (local id 41, gfx 0x3E80) at 28,66
@ Johto_Route41 object 42 (local id 42, gfx 0x3E80) at 27,65
@ Johto_Route41 object 43 (local id 43, gfx 0x3E80) at 28,65
@ Johto_Route41 object 44 (local id 44, gfx 0x3E80) at 27,66
Quetzal_EventScript_837F567::
	lockall
	setvar VAR_0x8004, 0xFA
	special QzHasBadgeForFieldMove
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_837F5DC
	playse SE_M_WATERFALL
	goto_if_eq VAR_FACING, 2, Quetzal_EventScript_837F5AC
	goto_if_eq VAR_FACING, 1, Quetzal_EventScript_837F5B8
	goto_if_eq VAR_FACING, 4, Quetzal_EventScript_837F5C4
	goto_if_eq VAR_FACING, 3, Quetzal_EventScript_837F5D0
	releaseall
	end

Quetzal_EventScript_837F5AC::
	applymovement LOCALID_PLAYER, Quetzal_Movement_837F5E7
	waitmovement 0
	releaseall
	end

Quetzal_EventScript_837F5B8::
	applymovement LOCALID_PLAYER, Quetzal_Movement_837F5EB
	waitmovement 0
	releaseall
	end

Quetzal_EventScript_837F5C4::
	applymovement LOCALID_PLAYER, Quetzal_Movement_837F5EF
	waitmovement 0
	releaseall
	end

Quetzal_EventScript_837F5D0::
	applymovement LOCALID_PLAYER, Quetzal_Movement_837F5F3
	waitmovement 0
	releaseall
	end

Quetzal_EventScript_837F5DC::
	msgbox Quetzal_Text_837F5F7, MSGBOX_DEFAULT
	closemessage
	releaseall
	end

Quetzal_EventScript_83811B3::
	fadescreen 1
	special Special_ViewWallClock
	waitstate
	releaseall
	end

Quetzal_EventScript_83C2D44::
	special QzJohtoTravelingEventCheck
	end

@ Johto_AzaleaTown object 10 (local id 10, gfx 0x2950) at 15,8
@ Johto_AzaleaTown object 11 (local id 11, gfx 0x3A9B) at 16,8
@ Johto_AzaleaTown object 12 (local id 12, gfx 0x3A9C) at 16,7
@ Johto_AzaleaTown object 13 (local id 13, gfx 0x2947) at 17,8
@ Johto_BlackthornCity object 17 (local id 17, gfx 0x2950) at 26,35
@ Johto_BlackthornCity object 18 (local id 18, gfx 0x3A9B) at 27,35
@ Johto_BlackthornCity object 19 (local id 19, gfx 0x3A9C) at 27,34
@ Johto_BlackthornCity object 20 (local id 20, gfx 0x2947) at 28,35
@ Johto_CherrygroveCity object 6 (local id 6, gfx 0x2950) at 52,19
@ Johto_CherrygroveCity object 7 (local id 7, gfx 0x3A9B) at 53,19
@ Johto_CherrygroveCity object 8 (local id 8, gfx 0x3A9C) at 53,18
@ Johto_CherrygroveCity object 9 (local id 9, gfx 0x2947) at 54,19
@ Johto_CianwoodCity object 15 (local id 15, gfx 0x2950) at 27,48
@ Johto_CianwoodCity object 16 (local id 16, gfx 0x3A9B) at 28,48
@ Johto_CianwoodCity object 17 (local id 17, gfx 0x3A9C) at 28,47
@ Johto_CianwoodCity object 18 (local id 18, gfx 0x2947) at 29,48
@ Johto_EcruteakCity object 30 (local id 30, gfx 0x2950) at 38,32
@ Johto_EcruteakCity object 31 (local id 31, gfx 0x3A9B) at 39,32
@ Johto_EcruteakCity object 32 (local id 32, gfx 0x3A9C) at 39,31
@ Johto_EcruteakCity object 33 (local id 33, gfx 0x2947) at 40,32
@ Johto_GoldenrodCity object 27 (local id 27, gfx 0x2950) at 8,22
@ Johto_GoldenrodCity object 28 (local id 28, gfx 0x3A9B) at 9,22
@ Johto_GoldenrodCity object 29 (local id 29, gfx 0x3A9C) at 9,21
@ Johto_GoldenrodCity object 30 (local id 30, gfx 0x2947) at 10,22
@ Johto_IndigoPlateau object 4 (local id 4, gfx 0x2950) at 7,8
@ Johto_IndigoPlateau object 5 (local id 5, gfx 0x3A9B) at 8,8
@ Johto_IndigoPlateau object 6 (local id 6, gfx 0x3A9C) at 8,7
@ Johto_IndigoPlateau object 7 (local id 7, gfx 0x2947) at 9,8
@ Johto_MahoganyTown object 5 (local id 5, gfx 0x2950) at 24,20
@ Johto_MahoganyTown object 6 (local id 6, gfx 0x3A9B) at 25,20
@ Johto_MahoganyTown object 7 (local id 7, gfx 0x3A9C) at 25,19
@ Johto_MahoganyTown object 8 (local id 8, gfx 0x2947) at 26,20
@ Johto_NewBarkTown object 4 (local id 4, gfx 0x2950) at 21,23
@ Johto_NewBarkTown object 5 (local id 5, gfx 0x3A9B) at 22,23
@ Johto_NewBarkTown object 6 (local id 6, gfx 0x3A9C) at 22,22
@ Johto_NewBarkTown object 7 (local id 7, gfx 0x2947) at 23,23
@ Johto_OlivineCity object 23 (local id 23, gfx 0x2950) at 35,25
@ Johto_OlivineCity object 24 (local id 24, gfx 0x3A9B) at 36,25
@ Johto_OlivineCity object 25 (local id 25, gfx 0x3A9C) at 36,24
@ Johto_OlivineCity object 26 (local id 26, gfx 0x2947) at 37,25
@ Johto_SafariZoneGate object 15 (local id 15, gfx 0x2950) at 8,10
@ Johto_SafariZoneGate object 16 (local id 16, gfx 0x3A9B) at 9,10
@ Johto_SafariZoneGate object 17 (local id 17, gfx 0x3A9C) at 9,9
@ Johto_SafariZoneGate object 18 (local id 18, gfx 0x2947) at 10,10
@ Johto_VioletCity object 37 (local id 37, gfx 0x2950) at 27,15
@ Johto_VioletCity object 38 (local id 38, gfx 0x3A9B) at 28,15
@ Johto_VioletCity object 39 (local id 39, gfx 0x3A9C) at 28,14
@ Johto_VioletCity object 40 (local id 40, gfx 0x2947) at 29,15
Quetzal_EventScript_83C2D48::
	lockall
	goto_if_set FLAG_QZ_0x1318, Quetzal_EventScript_83C2DFA
	specialvar VAR_RESULT, QzJohtoTravelingEventCheck
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_83C2DF8
	goto_if_set FLAG_QZ_0x1491, Quetzal_EventScript_83C2D89
	playbgm MUS_ENCOUNTER_CHAMPION, 0
	msgbox Quetzal_Text_83C30E6, MSGBOX_DEFAULT
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_913, 0, Quetzal_Text_83C3288
	goto Quetzal_EventScript_83C2D86

Quetzal_EventScript_83C2D86::
	setflag FLAG_QZ_0x1491

Quetzal_EventScript_83C2D89::
	setvar VAR_TEMP_1, 0x19
	copyvar VAR_0x8000, VAR_QZ_0x5073
	qz_givemon_ex VAR_0x8000, 0x32, ITEM_NONE, 2, 0, 0
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_83C2DC5
	goto_if_eq VAR_RESULT, 1, Quetzal_EventScript_83C2DCF
	msgbox Quetzal_Text_83615FD, MSGBOX_DEFAULT
	goto Quetzal_EventScript_83C2DF8

Quetzal_EventScript_83C2DC5::
	call Quetzal_EventScript_83C2DDE
	goto Quetzal_EventScript_83C2DF4

Quetzal_EventScript_83C2DCF::
	call Quetzal_EventScript_83C2DDE
	call Quetzal_EventScript_8361B27
	goto Quetzal_EventScript_83C2DF4

Quetzal_EventScript_83C2DDE::
	setvar VAR_QZ_0x5072, 0
	setvar VAR_QZ_0x5073, 0
	clearflag FLAG_QZ_0x1491
	setflag FLAG_QZ_0x1490
	call Quetzal_EventScript_83C2D35
	return

Quetzal_EventScript_83C2DF4::
	special QzReloadMapAtPlayerPos
	waitstate

Quetzal_EventScript_83C2DF8::
	releaseall
	end

Quetzal_EventScript_83C2DFA::
	goto_if_set FLAG_QZ_0x1319, Quetzal_EventScript_83C2E28
	goto_if_ge VAR_QZ_0x506C, 2, Quetzal_EventScript_83C2E1B
	msgbox Quetzal_Text_83C2E53, MSGBOX_DEFAULT
	goto Quetzal_EventScript_83C2E30

Quetzal_EventScript_83C2E1B::
	msgbox Quetzal_Text_83C2EF2, MSGBOX_DEFAULT
	goto Quetzal_EventScript_83C2E30

Quetzal_EventScript_83C2E28::
	msgbox Quetzal_Text_83C2FBE, MSGBOX_DEFAULT

Quetzal_EventScript_83C2E30::
	msgbox Quetzal_Text_83C2E3D, MSGBOX_DEFAULT
	goto Quetzal_EventScript_83C2DF8

Quetzal_EventScript_83C6FA5::
	setmetatile 0x13, 0x2F, 0x35A, 0
	setmetatile 0x13, 0x30, 0x35A, 0
	setmetatile 0x13, 0x31, 0x35A, 0
	setmetatile 0x13, 0x32, 0x35A, 0
	setmetatile 0x13, 0x33, 0x35A, 0
	setmetatile 0x13, 0x34, 0x35A, 0
	setmetatile 0x13, 0x35, 0x35A, 0
	setmetatile 0x13, 0x36, 0x35A, 0
	setmetatile 0x14, 0x2F, 0x35A, 0
	setmetatile 0x14, 0x30, 0x35A, 0
	setmetatile 0x14, 0x31, 0x35A, 0
	setmetatile 0x14, 0x32, 0x35A, 0
	setmetatile 0x14, 0x33, 0x35A, 0
	setmetatile 0x14, 0x34, 0x35A, 0
	setmetatile 0x14, 0x35, 0x35A, 0
	setmetatile 0x14, 0x36, 0x35A, 0
	setmetatile 0x15, 0x2F, 0x35A, 0
	setmetatile 0x15, 0x30, 0x35A, 0
	setmetatile 0x15, 0x31, 0x35A, 0
	setmetatile 0x15, 0x32, 0x35A, 0
	setmetatile 0x15, 0x33, 0x35A, 0
	setmetatile 0x15, 0x34, 0x35A, 0
	setmetatile 0x15, 0x35, 0x35A, 0
	setmetatile 0x15, 0x36, 0x35A, 0
	return

Quetzal_EventScript_83C708F::
	setmetatile 0x11, 0x23, 0x35A, 0
	setmetatile 0x11, 0x24, 0x35A, 0
	setmetatile 0x11, 0x25, 0x35A, 0
	setmetatile 0x11, 0x26, 0x35A, 0
	setmetatile 0x11, 0x27, 0x35A, 0
	setmetatile 0x11, 0x28, 0x35A, 0
	setmetatile 0x12, 0x23, 0x35A, 0
	setmetatile 0x12, 0x24, 0x35A, 0
	setmetatile 0x12, 0x25, 0x35A, 0
	setmetatile 0x12, 0x26, 0x35A, 0
	setmetatile 0x12, 0x27, 0x35A, 0
	setmetatile 0x12, 0x28, 0x35A, 0
	setmetatile 0x13, 0x23, 0x35A, 0
	setmetatile 0x13, 0x24, 0x35A, 0
	setmetatile 0x13, 0x25, 0x35A, 0
	setmetatile 0x13, 0x26, 0x35A, 0
	setmetatile 0x13, 0x27, 0x35A, 0
	setmetatile 0x13, 0x28, 0x35A, 0
	setmetatile 0x14, 0x20, 0x359, 0
	setmetatile 0x15, 0x20, 0x359, 0
	setmetatile 0x16, 0x20, 0x359, 0
	setmetatile 0x14, 0x23, 0x361, 1
	setmetatile 0x15, 0x23, 0x361, 1
	setmetatile 0x16, 0x23, 0x361, 1
	setmetatile 0x14, 0x21, 0x35A, 0
	setmetatile 0x14, 0x22, 0x35A, 0
	setmetatile 0x15, 0x21, 0x35A, 0
	setmetatile 0x15, 0x22, 0x35A, 0
	setmetatile 0x16, 0x21, 0x35A, 0
	setmetatile 0x16, 0x22, 0x35A, 0
	return

Quetzal_EventScript_83C71AF::
	setmetatile 0x11, 0x1A, 0x359, 0
	setmetatile 0x12, 0x1A, 0x359, 0
	setmetatile 0x13, 0x1A, 0x359, 0
	setmetatile 0x14, 0x1A, 0x359, 0
	setmetatile 0x15, 0x1A, 0x359, 0
	setmetatile 0x16, 0x1A, 0x359, 0
	setmetatile 0x17, 0x1A, 0x359, 0
	setmetatile 0x11, 0x1D, 0x361, 1
	setmetatile 0x12, 0x1D, 0x361, 1
	setmetatile 0x13, 0x1D, 0x361, 1
	setmetatile 0x14, 0x1D, 0x361, 1
	setmetatile 0x15, 0x1D, 0x361, 1
	setmetatile 0x16, 0x1D, 0x361, 1
	setmetatile 0x17, 0x1D, 0x361, 1
	setmetatile 0x11, 0x1B, 0x35A, 0
	setmetatile 0x11, 0x1C, 0x35A, 0
	setmetatile 0x12, 0x1B, 0x35A, 0
	setmetatile 0x12, 0x1C, 0x35A, 0
	setmetatile 0x13, 0x1B, 0x35A, 0
	setmetatile 0x13, 0x1C, 0x35A, 0
	setmetatile 0x14, 0x1B, 0x35A, 0
	setmetatile 0x14, 0x1C, 0x35A, 0
	setmetatile 0x15, 0x1B, 0x35A, 0
	setmetatile 0x15, 0x1C, 0x35A, 0
	setmetatile 0x16, 0x1B, 0x35A, 0
	setmetatile 0x16, 0x1C, 0x35A, 0
	setmetatile 0x17, 0x1B, 0x35A, 0
	setmetatile 0x17, 0x1C, 0x35A, 0
	return

Quetzal_EventScript_83C72BD::
	setmetatile 0x13, 0xB, 0x35A, 0
	setmetatile 0x13, 0xC, 0x35A, 0
	setmetatile 0x13, 0xD, 0x35A, 0
	setmetatile 0x13, 0xE, 0x35A, 0
	setmetatile 0x13, 0xF, 0x35A, 0
	setmetatile 0x14, 0xB, 0x35A, 0
	setmetatile 0x14, 0xC, 0x35A, 0
	setmetatile 0x14, 0xD, 0x35A, 0
	setmetatile 0x14, 0xE, 0x35A, 0
	setmetatile 0x14, 0xF, 0x35A, 0
	setmetatile 0x15, 0xB, 0x35A, 0
	setmetatile 0x15, 0xC, 0x35A, 0
	setmetatile 0x15, 0xD, 0x35A, 0
	setmetatile 0x15, 0xE, 0x35A, 0
	setmetatile 0x15, 0xF, 0x35A, 0
	return

Quetzal_EventScript_83C83A3::
	setmetatile 0x10, 0xC, 0x37E, 1
	end

@ Johto_AzaleaMart object 3 (local id 3, gfx 0x3E8E) at 2,3
@ Johto_BlackthornMart object 3 (local id 3, gfx 0x3E8E) at 2,3
@ Johto_CherrygroveMart object 1 (local id 1, gfx 0x3E8E) at 2,3
@ Johto_EcruteakMart object 3 (local id 3, gfx 0x3E8E) at 2,3
@ Johto_OlivineMart object 3 (local id 3, gfx 0x3E8E) at 2,3
Quetzal_EventScript_83C9930::
	lock
	faceplayer
	message Quetzal_Text_8360D02
	waitmessage
	pokemart 0x0
	msgbox Quetzal_Text_8360D20, MSGBOX_DEFAULT
	release
	end

@ Johto_AzaleaPokecenter1F object 2 (local id 2, gfx 0x3EA5) at 8,2
@ Johto_BlackthornPokecenter1F object 2 (local id 2, gfx 0x3EA5) at 8,2
@ Johto_CherrygrovePokecenter1F object 2 (local id 2, gfx 0x3EA5) at 8,2
@ Johto_CianwoodPokecenter1F object 2 (local id 2, gfx 0x3EA5) at 8,2
@ Johto_EcruteakPokecenter1F object 2 (local id 2, gfx 0x3EA5) at 8,2
@ Johto_GoldenrodPokecenter1F object 2 (local id 2, gfx 0x3EA5) at 8,2
@ Johto_IndigoPlateauPokecenter1F object 2 (local id 2, gfx 0x3EA5) at 20,10
@ Johto_MahoganyPokecenter1F object 2 (local id 2, gfx 0x3EA5) at 8,2
@ Johto_SilverCavePokecenter1F object 2 (local id 2, gfx 0x3EA5) at 8,2
@ Johto_OlivinePokecenter1F object 2 (local id 2, gfx 0x3EA5) at 8,2
@ Johto_Route32Pokecenter1F object 2 (local id 2, gfx 0x3EA5) at 8,2
@ Johto_SafariZoneGatePokecenter1F object 2 (local id 2, gfx 0x3EA5) at 8,2
@ Johto_VioletPokecenter1F object 2 (local id 2, gfx 0x3EA5) at 8,2
Quetzal_EventScript_83C9AC2::
	lock
	faceplayer
	playmoncry SPECIES_CHANSEY, 0
	msgbox Quetzal_Text_83C9B9F, MSGBOX_DEFAULT
	delay 0x28
	waitmoncry
	closemessage
	release
	end

Quetzal_EventScript_83CC300::
	goto_if_ne VAR_QZ_0x5010, 3, Quetzal_EventScript_83CC33D
	goto_if_set FLAG_QZ_0x1318, Quetzal_EventScript_83CC33D
	goto_if_set FLAG_QZ_0x14AA, Quetzal_EventScript_83CC33D
	goto_if_set FLAG_QZ_0x1425, Quetzal_EventScript_83CC33D
	goto_if_unset FLAG_QZ_0x11B9, Quetzal_EventScript_83CC33D
	setflag FLAG_QZ_0x1057
	setflag FLAG_QZ_0x1059
	setflag FLAG_QZ_0x14B9
	setvar VAR_QZ_0x5010, 4

Quetzal_EventScript_83CC33D::
	return

Quetzal_EventScript_83D3844::
	callnative 0x80E2C95
	end

Quetzal_EventScript_83D384A::
	lockall
	hideobjectat LOCALID_PLAYER, MAP_PETALBURG_CITY
	special QzResetFollowerObjects
	getplayerxy VAR_0x8005, VAR_0x8006
	addvar VAR_0x8006, 1
	opendoor VAR_0x8005, VAR_0x8006
	waitdooranim
	applymovement LOCALID_PLAYER, Quetzal_Movement_83D39C4
	waitmovement 0
	closedoor VAR_0x8005, VAR_0x8006
	waitdooranim
	setvar VAR_QZ_0x5015, 0
	callnative 0x82672E5
	gotonative 0x80E2CB1
	releaseall
	end

@ Johto_GoldenrodDeptStoreRoof object 2 (local id 2, gfx 0x14) at 19,8
@ Johto_GoldenrodDeptStoreRoofAlt object 4 (local id 4, gfx 0x14) at 19,8
Quetzal_EventScript_83D4938::
	lock
	faceplayer
	msgbox Quetzal_Text_83D49A0, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodDeptStoreRoof object 3 (local id 3, gfx 0x11) at 11,6
@ Johto_GoldenrodDeptStoreRoofAlt object 2 (local id 2, gfx 0x11) at 11,6
Quetzal_EventScript_83D4945::
	lock
	faceplayer
	msgbox Quetzal_Text_83D4AAD, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodDeptStoreRoof object 1 (local id 1, gfx 0x3EB7) at 22,11
@ Johto_GoldenrodDeptStoreRoofAlt object 1 (local id 1, gfx 0x3EB7) at 22,11
Quetzal_EventScript_83D495F::
	lock
	msgbox Quetzal_Text_83D4A88, MSGBOX_DEFAULT
	closemessage
	playse SE_PIN
	applymovement 1, Quetzal_Movement_83607F9
	delay 0x14
	faceplayer
	msgbox Quetzal_Text_83D4A91, MSGBOX_DEFAULT
	closemessage
	turnobject 1, 1
	release
	end

@ Johto_GoldenrodDeptStoreRoof object 4 (local id 4, gfx 0x24) at 12,10
@ Johto_GoldenrodDeptStoreRoofAlt object 3 (local id 3, gfx 0x24) at 12,10
Quetzal_EventScript_83D4993::
	lock
	faceplayer
	msgbox Quetzal_Text_83D4B64, MSGBOX_DEFAULT
	closemessage
	release
	end

Quetzal_EventScript_83D801D::
	setmetatile 0x19, 8, 0x335, 0
	setmetatile 0x19, 9, 0x335, 0
	setmetatile 0x1A, 8, 0x334, 0
	setmetatile 0x1A, 9, 0x334, 0
	setmetatile 0x1B, 8, 0x334, 0
	setmetatile 0x1B, 9, 0x334, 0
	setmetatile 0x1C, 8, 0x334, 0
	setmetatile 0x1C, 9, 0x334, 0
	setmetatile 0x1D, 8, 0x334, 0
	setmetatile 0x1D, 9, 0x334, 0
	setmetatile 0x1E, 8, 0x334, 0
	setmetatile 0x1E, 9, 0x334, 0
	setmetatile 0x1F, 8, 0x334, 0
	setmetatile 0x1F, 9, 0x334, 0
	return

Quetzal_EventScript_83DA173::
	call Quetzal_EventScript_83DA3A2
	call Quetzal_EventScript_83DA523
	call Quetzal_EventScript_83DA55A
	call Quetzal_EventScript_83DA447
	call Quetzal_EventScript_83DA5C8
	call Quetzal_EventScript_83DA4B5
	return

Quetzal_EventScript_83DA3A2::
	setmetatile 5, 7, 0x399, 0
	setmetatile 6, 7, 0x39A, 0
	setmetatile 7, 7, 0x39B, 0
	setmetatile 5, 8, 0x3A1, 0
	setmetatile 6, 8, 0x3A2, 0
	setmetatile 7, 8, 0x3A3, 0
	return

Quetzal_EventScript_83DA447::
	setmetatile 5, 0xB, 0x399, 0
	setmetatile 6, 0xB, 0x39A, 0
	setmetatile 7, 0xB, 0x39B, 0
	setmetatile 5, 0xC, 0x3A1, 0
	setmetatile 6, 0xC, 0x3A2, 0
	setmetatile 7, 0xC, 0x3A3, 0
	return

Quetzal_EventScript_83DA4B5::
	setmetatile 0x17, 0xB, 0x399, 0
	setmetatile 0x18, 0xB, 0x39A, 0
	setmetatile 0x19, 0xB, 0x39B, 0
	setmetatile 0x17, 0xC, 0x3A1, 0
	setmetatile 0x18, 0xC, 0x3A2, 0
	setmetatile 0x19, 0xC, 0x3A3, 0
	return

Quetzal_EventScript_83DA523::
	setmetatile 0xE, 7, 0x378, 0
	setmetatile 0xF, 7, 0x379, 0
	setmetatile 0x10, 7, 0x37A, 0
	setmetatile 0xE, 8, 0x380, 0
	setmetatile 0xF, 8, 0x381, 0
	setmetatile 0x10, 8, 0x382, 0
	return

Quetzal_EventScript_83DA55A::
	setmetatile 0x17, 7, 0x378, 0
	setmetatile 0x18, 7, 0x379, 0
	setmetatile 0x19, 7, 0x37A, 0
	setmetatile 0x17, 8, 0x380, 0
	setmetatile 0x18, 8, 0x381, 0
	setmetatile 0x19, 8, 0x382, 0
	return

Quetzal_EventScript_83DA5C8::
	setmetatile 0xE, 0xB, 0x378, 0
	setmetatile 0xF, 0xB, 0x379, 0
	setmetatile 0x10, 0xB, 0x37A, 0
	setmetatile 0xE, 0xC, 0x380, 0
	setmetatile 0xF, 0xC, 0x381, 0
	setmetatile 0x10, 0xC, 0x382, 0
	return

Quetzal_EventScript_83DB29A::
	setmetatile 0xC, 0xA, 0x2DD, 0
	end

@ Johto_IlexForest object 6 (local id 6, gfx 0x11) at 42,26
@ Johto_IlexForest object 18 (local id 18, gfx 0xB) at 24,14
@ Johto_OlivineCity object 8 (local id 8, gfx 0x6) at 28,41
Quetzal_EventScript_83DC26C::
	lock
	faceplayer
	msgbox Quetzal_Text_83DC904, MSGBOX_DEFAULT
	release
	end

Quetzal_EventScript_83DF288::
	setmetatile 8, 4, 0x28D, 0
	end

Quetzal_EventScript_83DF68E::
	goto_if_lt VAR_QZ_0x502E, 7, Quetzal_EventScript_83DF6A9
	goto_if_ge VAR_QZ_0x500C, 15, Quetzal_EventScript_83DF6A9
	setvar VAR_QZ_0x500C, 0xF

Quetzal_EventScript_83DF6A9::
	return

Quetzal_EventScript_83DF6B4::
	lock
	msgbox Quetzal_Text_83DFBD4, MSGBOX_DEFAULT
	setvar VAR_QZ_0x500A, 6
	clearflag FLAG_QZ_0x1097
	setflag FLAG_QZ_0x1096
	setflag FLAG_QZ_0x1098
	clearflag FLAG_QZ_0x10A5
	setvar VAR_QZ_0x500C, 0x10
	delay 0xA
	release
	end

@ Johto_SilverCaveSummit object 1 (local id 1, gfx 0x272B) at 10,6
@ Johto_SilverCaveSummitAlt object 1 (local id 1, gfx 0x272B) at 10,6
Quetzal_EventScript_83E05E8::
	lock
	faceplayer
	msgbox Quetzal_Text_83E0706, MSGBOX_DEFAULT
	closemessage
	goto_if_unset FLAG_QZ_0x11B0, Quetzal_EventScript_83E0605
	goto_if_unset FLAG_QZ_0x149E, Quetzal_EventScript_83E0644

Quetzal_EventScript_83E0605::
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4327, 0, Quetzal_Text_83E073A
	setflag FLAG_QZ_0x11B0
	call_if_unset FLAG_QZ_0x149E, Quetzal_EventScript_83E064C
	closemessage
	playse SE_M_BLIZZARD
	setrespawn 0x17
	msgbox Quetzal_Text_83E0706, MSGBOX_DEFAULT
	closemessage
	fadescreenswapbuffers 1
	setflag FLAG_QZ_0x1063
	removeobject 1
	fadescreenswapbuffers 0
	delay 0xB4
	fadescreenspeed 1, 0x18
	clearflag FLAG_QZ_0x1063
	special QzAfterCreditsReset
	waitstate
	releaseall
	end

Quetzal_EventScript_83E0644::
	call Quetzal_EventScript_83E064C
	closemessage
	releaseall
	end

Quetzal_EventScript_83E064C::
	msgbox Quetzal_Text_83E0685, MSGBOX_DEFAULT
	setvar VAR_0x8000, ITEM_LIGHT_BALL   @ Moemon Kai: Quetzal's PIKASHUNIUM Z (0x186, a number: not translated)
	setvar VAR_0x8001, 1
	additem VAR_0x8000, VAR_0x8001
	copyvar VAR_0x8007, VAR_RESULT
	call Quetzal_EventScript_835FB62
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_83E067C
	setflag FLAG_QZ_0x149E
	return

Quetzal_EventScript_83E067C::
	msgbox Quetzal_Text_83E06B6, MSGBOX_DEFAULT
	return

@ Johto_NationalParkBugContest sign / bg event at 16,45
@ Johto_NationalPark sign / bg event at 16,45
Quetzal_EventScript_83E07FC::
	lock
	msgbox Quetzal_Text_83E0B86, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_NationalParkBugContest sign / bg event at 31,33
@ Johto_NationalPark sign / bg event at 31,33
Quetzal_EventScript_83E0808::
	lock
	msgbox Quetzal_Text_83E0BA6, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_NationalParkBugContest sign / bg event at 14,4
@ Johto_NationalPark sign / bg event at 14,4
Quetzal_EventScript_83E0814::
	lock
	msgbox Quetzal_Text_83E0BFA, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_NationalParkBugContest sign / bg event at 14,42
@ Johto_NationalPark sign / bg event at 14,42
@ Johto_NationalPark sign / bg event at 34,42
Quetzal_EventScript_83E0820::
	lock
	msgbox Quetzal_Text_83E091E, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_ElmsLab sign / bg event at 2,2
@ Johto_ElmsLab sign / bg event at 1,2
@ Johto_RuinsOfAlphResearchCenter sign / bg event at 8,2
@ Johto_RuinsOfAlphResearchCenter sign / bg event at 9,2
Quetzal_EventScript_83E2032::
	lock
	goto_if_set FLAG_QZ_0x11D1, Quetzal_EventScript_83E2047
	msgbox Quetzal_Text_82E0BEC, MSGBOX_SIGN
	closemessage
	release
	end

Quetzal_EventScript_83E2047::
	call Quetzal_EventScript_83602D9
	release
	end

Quetzal_EventScript_83E3F77::
	special QzJohtoTravelingEventCheck
	call Quetzal_EventScript_840EA62
	goto_if_ge VAR_QZ_0x500D, 5, Quetzal_EventScript_83E3F8B
	end

Quetzal_EventScript_83E3F8B::
	setweather 0
	end

@ Johto_OlivineCity object 6 (local id 6, gfx 0x3B) at 53,47
@ Johto_OlivineLighthouse object 16 (local id 16, gfx 0x3A9E) at 125,15
Quetzal_EventScript_83E42D0::
	setorcopyvar VAR_0x8000, ITEM_THUNDER_STONE
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Quetzal_EventScript_83E894C::
	call_if_eq VAR_TEMP_0, 1, Quetzal_EventScript_83E896E
	end

Quetzal_EventScript_83E8958::
	setmetatile 6, 1, 0x28E, 0
	setmetatile 6, 2, 0x296, 0
	setflag FLAG_TEMP_4
	return

Quetzal_EventScript_83E896E::
	setmetatile 5, 0xB, 0x29D, 1
	setmetatile 6, 0xB, 0x29E, 1
	setmetatile 7, 0xB, 0x29F, 1
	setmetatile 5, 0xC, 0x2A5, 1
	setmetatile 6, 0xC, 0x2A6, 1
	setmetatile 7, 0xC, 0x2A7, 1
	return

Quetzal_EventScript_83E89B8::
	lock
	setflag FLAG_TEMP_2
	applymovement LOCALID_PLAYER, Quetzal_Movement_8360837
	waitmovement 0
	applymovement LOCALID_PLAYER, Quetzal_Movement_8360837
	waitmovement 0
	setmetatile 5, 0xB, 0x29D, 1
	setmetatile 6, 0xB, 0x29E, 1
	setmetatile 7, 0xB, 0x29F, 1
	setmetatile 5, 0xC, 0x2A5, 1
	setmetatile 6, 0xC, 0x2A6, 1
	setmetatile 7, 0xC, 0x2A7, 1
	playse SE_BANG
	setvar VAR_0x8004, 1
	setvar VAR_0x8005, 1
	setvar VAR_0x8006, 4
	setvar VAR_0x8007, 4
	special ShakeCamera
	waitstate
	special DrawWholeMapView
	setvar VAR_TEMP_0, 1
	release
	end

Quetzal_EventScript_83E9A0F::
	setmetatile 0xE, 0x13, 0x378, 0
	setmetatile 0xE, 0x14, 0x380, 0
	setmetatile 0xF, 0x13, 0x379, 0
	setmetatile 0xF, 0x14, 0x379, 0
	setmetatile 0x10, 0x13, 0x379, 0
	setmetatile 0x10, 0x14, 0x379, 0
	return

Quetzal_EventScript_83EAC6A::
	setmetatile 0xB, 0xB, 0x378, 0
	setmetatile 0xB, 0xC, 0x380, 0
	setmetatile 0xC, 0xB, 0x379, 0
	setmetatile 0xC, 0xC, 0x379, 0
	setmetatile 0xD, 0xB, 0x379, 0
	setmetatile 0xD, 0xC, 0x379, 0
	return

@ Johto_TeamRocketBaseB2F object 15 (local id 15, gfx 0x1C6) at 10,2
@ Johto_TeamRocketBaseB2F object 16 (local id 16, gfx 0x1C6) at 15,22
@ Johto_TeamRocketBaseB3F object 14 (local id 14, gfx 0x1C6) at 8,2
@ Johto_TeamRocketBaseB3F object 17 (local id 17, gfx 0x1C6) at 29,3
Quetzal_EventScript_83EB7DA::
	lock
	faceplayer
	waitse
	playmoncry SPECIES_MURKROW, 0
	msgbox Quetzal_Text_83EB7ED, MSGBOX_DEFAULT
	waitmoncry
	release
	end

@ Johto_Route26South object 2 (local id 2, gfx 0x3E90) at 16,-10
@ Johto_Route26 object 2 (local id 2, gfx 0x3E90) at 16,20
Quetzal_EventScript_83EBC64::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4526, 0, Quetzal_Text_83EBCD7, Quetzal_Text_83EBD10
	msgbox Quetzal_Text_83EBD1B, MSGBOX_AUTOCLOSE
	end

@ Johto_Route26South object 3 (local id 3, gfx 0x3E8F) at 9,-19
@ Johto_Route26 object 3 (local id 3, gfx 0x3E8F) at 9,11
Quetzal_EventScript_83EBC92::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4111, 0, Quetzal_Text_83EBE70, Quetzal_Text_83EBED1
	msgbox Quetzal_Text_83EBEEB, MSGBOX_AUTOCLOSE
	end

Quetzal_EventScript_83ECB9A::
	call Quetzal_EventScript_840EA62
	end

@ Johto_Route41 object 11 (local id 11, gfx 0x149) at 0,88
@ Johto_Route41 object 12 (local id 12, gfx 0x149) at 2,88
@ Johto_Route41 object 13 (local id 13, gfx 0x149) at 1,88
@ Johto_Route41 object 14 (local id 14, gfx 0x149) at 3,88
@ Johto_WhirlIslandB2F object 4 (local id 4, gfx 0x149) at 35,86
Quetzal_EventScript_83F4A9E::
	lock
	faceplayer
	waitse
	playmoncry SPECIES_TENTACRUEL, 0
	msgbox Quetzal_Text_83F4AB1, MSGBOX_DEFAULT
	waitmoncry
	release
	end

Quetzal_EventScript_83F4AD7::
	setvar VAR_TEMP_0, 1
	return

Quetzal_EventScript_83F7F66::
	call_if_set FLAG_QZ_0x1076, Quetzal_EventScript_83F7FAF
	call_if_set FLAG_QZ_0x1078, Quetzal_EventScript_83F7FC2
	call_if_set FLAG_QZ_0x1079, Quetzal_EventScript_83F7FD5
	call_if_set FLAG_QZ_0x1077, Quetzal_EventScript_83F7FE8
	call_if_set FLAG_QZ_0x102A, Quetzal_EventScript_83F7FFB
	call_if_set FLAG_QZ_0x102B, Quetzal_EventScript_83F8005
	call_if_set FLAG_QZ_0x1029, Quetzal_EventScript_83F800F
	call_if_set FLAG_QZ_0x1028, Quetzal_EventScript_83F8019
	end

Quetzal_EventScript_83F7FAF::
	setmetatile 0x14, 5, 0x302, 1
	setmetatile 0x15, 5, 0x303, 1
	return

Quetzal_EventScript_83F7FC2::
	setmetatile 4, 0x16, 0x302, 1
	setmetatile 5, 0x16, 0x303, 1
	return

Quetzal_EventScript_83F7FD5::
	setmetatile 0x14, 0x16, 0x302, 1
	setmetatile 0x15, 0x16, 0x303, 1
	return

Quetzal_EventScript_83F7FE8::
	setmetatile 4, 5, 0x302, 1
	setmetatile 5, 5, 0x303, 1
	return

Quetzal_EventScript_83F7FFB::
	setmetatile 0x15, 1, 0x308, 0
	return

Quetzal_EventScript_83F8005::
	setmetatile 5, 0x12, 0x308, 0
	return

Quetzal_EventScript_83F800F::
	setmetatile 0x15, 0x12, 0x308, 0
	return

Quetzal_EventScript_83F8019::
	setmetatile 5, 1, 0x308, 0
	return

@ Johto_BurnedTower1F sign / bg event at 11,22
@ Johto_BurnedTower1F sign / bg event at 19,22
@ Johto_DragonsDenB1F sign / bg event at 27,46
@ Johto_DragonsDenB1F sign / bg event at 35,46
@ Johto_DragonsDenB1F sign / bg event at 35,36
@ Johto_DragonsDenB1F sign / bg event at 27,36
@ Johto_DragonsDenB1F sign / bg event at 29,4
@ Johto_DragonsDenB1F sign / bg event at 33,4
@ Johto_DragonShrine sign / bg event at 4,15
@ Johto_DragonShrine sign / bg event at 8,15
@ Johto_WiseTriosRoom sign / bg event at 7,10
@ Johto_WiseTriosRoom sign / bg event at 7,14
@ Johto_WiseTriosRoom sign / bg event at 1,14
@ Johto_WiseTriosRoom sign / bg event at 1,10
@ Johto_WiseTriosRoom sign / bg event at 1,6
@ Johto_IndigoPlateau sign / bg event at 8,10
@ Johto_IndigoPlateau sign / bg event at 14,10
@ Johto_IndigoPlateau sign / bg event at 14,7
@ Johto_IndigoPlateau sign / bg event at 8,7
@ Johto_IndigoPlateau sign / bg event at 8,13
@ Johto_IndigoPlateau sign / bg event at 14,13
@ Johto_IndigoPlateauPokecenter1F sign / bg event at 5,12
@ Johto_IndigoPlateauPokecenter1F sign / bg event at 5,15
@ Johto_IndigoPlateauPokecenter1F sign / bg event at 30,9
@ Johto_IndigoPlateauPokecenter1F sign / bg event at 30,6
@ Johto_IndigoPlateauPokecenter1F sign / bg event at 34,6
@ Johto_IndigoPlateauPokecenter1F sign / bg event at 34,9
@ Johto_Route23 sign / bg event at 5,8
@ Johto_Route23 sign / bg event at 2,8
@ Johto_Route23 sign / bg event at 18,8
@ Johto_Route23 sign / bg event at 21,8
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 22,22
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 19,22
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 16,22
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 13,22
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 10,22
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 7,22
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 7,16
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 22,16
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 22,10
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 19,10
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 16,10
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 13,10
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 10,10
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 7,10
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 7,4
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 10,4
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 13,4
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 16,4
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 19,4
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 22,4
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 7,28
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 10,28
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 13,28
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 16,28
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 19,28
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 22,28
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 4,4
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 25,4
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 25,10
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 4,10
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 4,16
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 25,16
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 25,22
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 25,28
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 4,28
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 4,22
@ Johto_RuinsOfAlphChambers sign / bg event at 19,5
@ Johto_RuinsOfAlphChambers sign / bg event at 22,5
@ Johto_RuinsOfAlphChambers sign / bg event at 22,22
@ Johto_RuinsOfAlphChambers sign / bg event at 19,22
@ Johto_RuinsOfAlphChambers sign / bg event at 6,22
@ Johto_RuinsOfAlphChambers sign / bg event at 3,22
@ Johto_RuinsOfAlphChambers sign / bg event at 3,5
@ Johto_RuinsOfAlphChambers sign / bg event at 6,5
@ Johto_RuinsOfAlphChambers sign / bg event at 19,36
@ Johto_RuinsOfAlphChambers sign / bg event at 22,36
@ Johto_RuinsOfAlphChambers sign / bg event at 6,36
@ Johto_RuinsOfAlphChambers sign / bg event at 3,36
@ Johto_RuinsOfAlphChambers sign / bg event at 3,53
@ Johto_RuinsOfAlphChambers sign / bg event at 6,53
@ Johto_RuinsOfAlphChambers sign / bg event at 19,53
@ Johto_RuinsOfAlphChambers sign / bg event at 22,53
@ Johto_TinTower1F sign / bg event at 11,17
@ Johto_TinTower1F sign / bg event at 15,13
@ Johto_TinTower1F sign / bg event at 15,9
@ Johto_TinTower1F sign / bg event at 7,17
@ Johto_TinTower1F sign / bg event at 3,13
@ Johto_TinTower1F sign / bg event at 3,9
@ Johto_TinTower1F sign / bg event at 7,3
@ Johto_TinTower1F sign / bg event at 11,3
@ Johto_TinTower2F sign / bg event at 12,3
@ Johto_TinTower2F sign / bg event at 6,3
@ Johto_TinTower3F sign / bg event at 17,3
@ Johto_TinTower3F sign / bg event at 2,3
@ Johto_TinTower4F sign / bg event at 11,3
@ Johto_TinTower4F sign / bg event at 16,3
@ Johto_TinTower4F sign / bg event at 6,3
@ Johto_TinTower4F sign / bg event at 1,3
@ Johto_TinTower7F sign / bg event at 16,3
@ Johto_TinTower7F sign / bg event at 1,3
@ Johto_TinTower9F sign / bg event at 14,3
@ Johto_TinTower9F sign / bg event at 10,3
@ Johto_TinTower9F sign / bg event at 7,3
@ Johto_TinTower9F sign / bg event at 3,3
@ Johto_TinTower8F sign / bg event at 16,3
@ Johto_TinTower8F sign / bg event at 9,3
@ Johto_TinTower8F sign / bg event at 1,3
Quetzal_EventScript_83F804F::
	lock
	msgbox Quetzal_Text_83F863F, MSGBOX_DEFAULT
	release
	end

@ Johto_SproutTower1F sign / bg event at 12,16
@ Johto_SproutTower1F sign / bg event at 8,16
@ Johto_SproutTower2F sign / bg event at 12,16
@ Johto_SproutTower3F sign / bg event at 17,17
@ Johto_SproutTower3F sign / bg event at 5,17
@ Johto_SproutTower3F sign / bg event at 8,2
@ Johto_SproutTower3F sign / bg event at 14,2
Quetzal_EventScript_83FA60F::
	lock
	msgbox Quetzal_Text_83FA742, MSGBOX_AUTOCLOSE
	release
	end

Quetzal_EventScript_83FAE20::
	setescapewarp MAP_JOHTO_ECRUTEAK_CITY_NORTH, 255, 35, 42
	end

@ Johto_TinTowerRoof trigger at 10,13 (VAR_QZ_0x5007 == 2)
@ Johto_TinTowerRoofAlt trigger at 10,13 (VAR_QZ_0x5007 == 2)
Quetzal_EventScript_83FB64A::
	lockall
	playbgm MUS_QZ_654, 1
	msgbox Quetzal_Text_83FBCB8, MSGBOX_DEFAULT
	closemessage
	applymovement 4, Quetzal_Movement_83FBB94
	waitmovement 0
	applymovement LOCALID_PLAYER, Quetzal_Movement_83FBB9A
	waitmovement 0
	applymovement 4, Quetzal_Movement_83FBBA0
	applymovement 3, Quetzal_Movement_83FBBA4
	applymovement 5, Quetzal_Movement_83FBBB1
	waitmovement 5
	playse SE_M_PETAL_DANCE
	applymovement 4, Quetzal_Movement_83FBBBE
	waitmovement 0
	playse SE_M_PETAL_DANCE
	applymovement 3, Quetzal_Movement_83FBBBE
	applymovement 6, Quetzal_Movement_83FBBBE
	applymovement 7, Quetzal_Movement_83FBBBE
	applymovement 5, Quetzal_Movement_83FBBBE
	applymovement 4, Quetzal_Movement_83FBBBE
	waitmovement 0
	playse SE_M_PETAL_DANCE
	setweather 0xC
	doweather
	applymovement 3, Quetzal_Movement_83FBBD8
	applymovement 6, Quetzal_Movement_83FBBDF
	applymovement 7, Quetzal_Movement_83FBBE6
	applymovement 5, Quetzal_Movement_83FBBEF
	waitmovement 5
	delay 0x1E
	applymovement 3, Quetzal_Movement_83FBBBE
	applymovement 6, Quetzal_Movement_83FBBBE
	applymovement 7, Quetzal_Movement_83FBBBE
	applymovement 5, Quetzal_Movement_83FBBBE
	applymovement 4, Quetzal_Movement_83FBBBE
	waitmovement 0
	playse SE_M_PETAL_DANCE
	applymovement 3, Quetzal_Movement_83FBBD8
	applymovement 6, Quetzal_Movement_83FBBFA
	applymovement 7, Quetzal_Movement_83FBBE6
	applymovement 5, Quetzal_Movement_83FBC02
	waitmovement 5
	delay 0x1E
	applymovement 3, Quetzal_Movement_83FBBBE
	applymovement 6, Quetzal_Movement_83FBBBE
	applymovement 7, Quetzal_Movement_83FBBBE
	applymovement 5, Quetzal_Movement_83FBBBE
	applymovement 4, Quetzal_Movement_83FBBBE
	waitmovement 0
	playse SE_M_PETAL_DANCE
	applymovement 3, Quetzal_Movement_83FBBDF
	applymovement 6, Quetzal_Movement_83FBBE6
	applymovement 7, Quetzal_Movement_83FBBEF
	applymovement 5, Quetzal_Movement_83FBBD8
	waitmovement 7
	delay 0x1E
	applymovement 3, Quetzal_Movement_83FBBBE
	applymovement 6, Quetzal_Movement_83FBBBE
	applymovement 7, Quetzal_Movement_83FBBBE
	applymovement 5, Quetzal_Movement_83FBBBE
	applymovement 4, Quetzal_Movement_83FBBBE
	waitmovement 0
	playse SE_M_PETAL_DANCE
	applymovement 3, Quetzal_Movement_83FBBFA
	applymovement 6, Quetzal_Movement_83FBBE6
	applymovement 7, Quetzal_Movement_83FBC02
	applymovement 5, Quetzal_Movement_83FBBD8
	waitmovement 7
	delay 0x1E
	applymovement 3, Quetzal_Movement_83FBBBE
	applymovement 6, Quetzal_Movement_83FBBBE
	applymovement 7, Quetzal_Movement_83FBBBE
	applymovement 5, Quetzal_Movement_83FBBBE
	applymovement 4, Quetzal_Movement_83FBBBE
	waitmovement 0
	playse SE_M_PETAL_DANCE
	applymovement 3, Quetzal_Movement_83FBC0E
	applymovement 5, Quetzal_Movement_83FBC1F
	applymovement 4, Quetzal_Movement_83FBC31
	waitmovement 3
	delay 0x1E
	applymovement 3, Quetzal_Movement_83FBBBE
	applymovement 6, Quetzal_Movement_83FBBBE
	applymovement 7, Quetzal_Movement_83FBBBE
	applymovement 5, Quetzal_Movement_83FBBBE
	applymovement 4, Quetzal_Movement_83FBBBE
	waitmovement 0
	playse SE_M_PETAL_DANCE
	waitse
	setvar VAR_TEMP_1, 1
	special SpawnCameraObject
	setvar VAR_0x8004, 3
	setvar VAR_0x8005, 0x23
	special LoopWingFlapSE
	applymovement 8, Quetzal_Movement_83FBB8D
	applymovement LOCALID_CAMERA, Quetzal_Movement_83FBB74
	waitmovement 0
	delay 0x32
	waitse
	playmoncry SPECIES_HO_OH, 2
	delay 0x1E
	waitmoncry
	delay 0x3C
	setvar VAR_0x8004, 3
	setvar VAR_0x8005, 0x23
	special LoopWingFlapSE
	applymovement 8, Quetzal_Movement_83FBB84
	waitmovement 0
	applymovement LOCALID_CAMERA, Quetzal_Movement_83FBB7B
	waitmovement 0
	special RemoveCameraObject
	applymovement 4, Quetzal_Movement_83FBC2C
	applymovement 3, Quetzal_Movement_83FBC2C
	applymovement 6, Quetzal_Movement_83FBC2C
	applymovement 7, Quetzal_Movement_83FBC2C
	applymovement 5, Quetzal_Movement_83FBC2C
	waitmovement 0
	msgbox Quetzal_Text_83FBDA2, MSGBOX_DEFAULT
	waitmessage
	closemessage
	releaseall
	end

@ Johto_TinTowerRoof object 8 (local id 8, gfx 0x1FA) at 10,1
@ Johto_TinTowerRoofAlt object 8 (local id 8, gfx 0x1FA) at 10,1
Quetzal_EventScript_83FB8BE::
	lockall
	waitse
	playmoncry SPECIES_HO_OH, 2
	delay 0x1E
	waitmoncry
	playbgm MUS_QZ_652, 0
	setwildbattle SPECIES_HO_OH, 0x32, ITEM_NONE
	nop
	nop
	nop
	nop
	nop
	setflag FLAG_QZ_0xAD9
	special BattleSetup_StartLegendaryBattle
	waitstate
	clearflag FLAG_QZ_0xAD9
	setvar VAR_LAST_TALKED, 8
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, 4, Quetzal_EventScript_83FB9A9
	goto_if_eq VAR_RESULT, 5, Quetzal_EventScript_83FB9A9
	call_if_eq VAR_RESULT, 1, Quetzal_EventScript_83FB97C
	call_if_eq VAR_RESULT, 7, Quetzal_EventScript_83FB997
	setflag FLAG_QZ_0x1014
	applymovement 4, Quetzal_Movement_83FBC3C
	waitmovement 0
	msgbox Quetzal_Text_83FBEE7, MSGBOX_DEFAULT
	checkitem ITEM_SILVER_WING, 1
	goto_if_eq VAR_RESULT, 1, Quetzal_EventScript_83FB949
	setorcopyvar VAR_0x8000, ITEM_SILVER_WING
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM

Quetzal_EventScript_83FB949::
	checkitem ITEM_TIDAL_BELL, 1
	goto_if_eq VAR_RESULT, 1, Quetzal_EventScript_83FB965
	setorcopyvar VAR_0x8000, ITEM_TIDAL_BELL
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM

Quetzal_EventScript_83FB965::
	clearflag FLAG_QZ_0x1013
	setflag FLAG_QZ_0x1048
	setvar VAR_QZ_0x5006, 2
	setvar VAR_QZ_0x501A, 7
	setvar VAR_QZ_0x5007, 3
	releaseall
	end

Quetzal_EventScript_83FB97C::
	fadescreenswapbuffers 1
	removeobject 8
	setweather 0
	doweather
	setflag FLAG_QZ_0x1060
	fadescreenswapbuffers 0
	bufferspeciesname 0, SPECIES_HO_OH
	msgbox Quetzal_Text_83614E5, MSGBOX_DEFAULT
	return

Quetzal_EventScript_83FB997::
	setflag FLAG_QZ_0x100A
	fadescreenswapbuffers 1
	removeobject 8
	setweather 0
	doweather
	setflag FLAG_QZ_0x1060
	fadescreenswapbuffers 0
	return

Quetzal_EventScript_83FB9A9::
	releaseall
	end

@ Johto_TinTowerRoof object 4 (local id 4, gfx 0x3E9D) at 10,12
@ Johto_TinTowerRoofAlt object 4 (local id 4, gfx 0x3E9D) at 10,12
Quetzal_EventScript_83FB9AB::
	goto_if_eq VAR_QZ_0x5007, 3, Quetzal_EventScript_83FB9BF
	msgbox Quetzal_Text_83FBC44, MSGBOX_NPC
	end

Quetzal_EventScript_83FB9BF::
	goto_if_unset FLAG_QZ_0x100A, Quetzal_EventScript_83FB9D1
	msgbox Quetzal_Text_83FBAE2, MSGBOX_NPC
	end

Quetzal_EventScript_83FB9D1::
	msgbox Quetzal_Text_83FBA4E, MSGBOX_NPC
	end

@ Johto_TinTowerRoof object 5 (local id 5, gfx 0x3E9D) at 11,11
@ Johto_TinTowerRoofAlt object 5 (local id 5, gfx 0x3E9D) at 11,11
Quetzal_EventScript_83FB9DA::
	goto_if_eq VAR_QZ_0x5007, 3, Quetzal_EventScript_83FB9EE
	msgbox Quetzal_Text_83FBC5B, MSGBOX_NPC
	end

Quetzal_EventScript_83FB9EE::
	msgbox Quetzal_Text_83FBF97, MSGBOX_NPC
	end

@ Johto_TinTowerRoof object 3 (local id 3, gfx 0x3E9D) at 9,11
@ Johto_TinTowerRoofAlt object 2 (local id 2, gfx 0x3E9D) at -5,18
@ Johto_TinTowerRoofAlt object 6 (local id 6, gfx 0x3E9D) at 8,11
Quetzal_EventScript_83FB9F7::
	goto_if_eq VAR_QZ_0x5007, 3, Quetzal_EventScript_83FBA0B
	msgbox Quetzal_Text_83FBC73, MSGBOX_NPC
	end

Quetzal_EventScript_83FBA0B::
	msgbox Quetzal_Text_83FBF97, MSGBOX_NPC
	end

@ Johto_TinTowerRoof object 2 (local id 2, gfx 0x3E9D) at -5,18
@ Johto_TinTowerRoof object 6 (local id 6, gfx 0x3E9D) at 8,11
@ Johto_TinTowerRoofAlt object 3 (local id 3, gfx 0x3E9D) at 9,11
Quetzal_EventScript_83FBA14::
	goto_if_eq VAR_QZ_0x5007, 3, Quetzal_EventScript_83FBA28
	msgbox Quetzal_Text_83FBC8A, MSGBOX_NPC
	end

Quetzal_EventScript_83FBA28::
	msgbox Quetzal_Text_83FBF97, MSGBOX_NPC
	end

@ Johto_TinTowerRoof object 7 (local id 7, gfx 0x3E9D) at 12,11
@ Johto_TinTowerRoofAlt object 7 (local id 7, gfx 0x3E9D) at 12,11
Quetzal_EventScript_83FBA31::
	goto_if_eq VAR_QZ_0x5007, 3, Quetzal_EventScript_83FBA45
	msgbox Quetzal_Text_83FBCA1, MSGBOX_NPC
	end

Quetzal_EventScript_83FBA45::
	msgbox Quetzal_Text_83FBF97, MSGBOX_NPC
	end

Quetzal_EventScript_83FDB17::
	setmetatile 0xA, 0x12, 0x287, 0
	end

@ Johto_WhirlIslandB1F trigger at 59,42 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 61,38 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 72,52 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 73,51 (0 == 0)
@ Johto_WhirlIslandB1F trigger at 71,51 (0 == 0)
@ Johto_WhirlIslandB2F trigger at 41,68 (0 == 0)
Quetzal_EventScript_8400E2D::
	setweather 0xB
	doweather
	end

@ Johto_KurtsHouse sign / bg event at 11,1
@ Johto_AzaleaPokecenter1F sign / bg event at 3,1
@ Johto_BlackthornPokecenter1F sign / bg event at 3,1
@ Johto_CherrygrovePokecenter1F sign / bg event at 3,1
@ Johto_CianwoodPokecenter1F sign / bg event at 3,1
@ Johto_EcruteakPokecenter1F sign / bg event at 3,1
@ Johto_GoldenrodPokecenter1F sign / bg event at 3,1
@ Johto_IndigoPlateauPokecenter1F sign / bg event at 14,9
@ Johto_LakeOfRageHiddenPowerHouse sign / bg event at 5,1
@ Johto_MahoganyPokecenter1F sign / bg event at 3,1
@ Johto_SilverCavePokecenter1F sign / bg event at 3,1
@ Johto_PlayersHouse2F sign / bg event at 10,1
@ Johto_OlivinePokecenter1F sign / bg event at 3,1
@ Johto_Route32Pokecenter1F sign / bg event at 3,1
@ Johto_SafariZoneGatePokecenter1F sign / bg event at 3,1
@ Johto_VioletPokecenter1F sign / bg event at 3,1
Quetzal_EventScript_84017BA::
	lockall
	setvar VAR_0x8004, 0
	goto Quetzal_EventScript_83811B3

Quetzal_EventScript_840180E::
	playfanfare MUS_OBTAIN_ITEM
	message Quetzal_Text_840184F
	waitmessage
	waitfanfare
	call Quetzal_EventScript_8361B27
	return

Quetzal_EventScript_840181E::
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_8401839
	goto_if_eq VAR_RESULT, 1, Quetzal_EventScript_840180E
	goto Quetzal_EventScript_8401844

Quetzal_EventScript_8401839::
	playfanfare MUS_OBTAIN_ITEM
	message Quetzal_Text_840184F
	waitmessage
	waitfanfare
	return

Quetzal_EventScript_8401844::
	msgbox Quetzal_Text_8401892, MSGBOX_DEFAULT
	closemessage
	releaseall
	end

@ Johto_EcruteakCityNorth sign / bg event at 54,58
@ Johto_Route34 sign / bg event at 23,50
Quetzal_EventScript_84018CD::
	msgbox Quetzal_Text_84018D6, MSGBOX_SIGN
	end

Quetzal_EventScript_8401B9D::
	warp MAP_JOHTO_GOLDENROD_CITY, 255, 16, 23
	waitstate
	end

Quetzal_EventScript_8401C4F::
	special QzHealPlayerPartyEx
	msgbox Quetzal_Text_8402C9D, MSGBOX_DEFAULT
	closemessage
	fadescreen 3
	special QzGiovanniStoryEnterPast
	setescapewarp MAP_JOHTO_ROUTE27, 255, 45, 17
	warp MAP_JOHTO_TOHJO_FALLS, 255, 46, 17
	waitstate
	end

Quetzal_EventScript_8401C92::
	release
	end

Quetzal_EventScript_8401C94::
	releaseall

Quetzal_EventScript_8401C95::
	end

Quetzal_EventScript_8401EDF::
	lock
	special Script_ClearHeldMovement
	faceplayer
	goto_if_set FLAG_QZ_0x1319, Quetzal_EventScript_8401EF7
	msgbox Quetzal_Text_8402E50, MSGBOX_DEFAULT
	release
	end

Quetzal_EventScript_8401EF7::
	msgbox Quetzal_Text_8402EFB, MSGBOX_DEFAULT
	release
	end

Quetzal_EventScript_8401F01::
	lock
	special Script_ClearHeldMovement
	faceplayer
	goto_if_set FLAG_QZ_0x1319, Quetzal_EventScript_8401F1C
	msgbox Quetzal_Text_8402F7A, MSGBOX_DEFAULT
	goto Quetzal_EventScript_8401C92

Quetzal_EventScript_8401F1C::
	msgbox Quetzal_Text_8403019, MSGBOX_DEFAULT
	goto Quetzal_EventScript_8401C92

@ Johto_AzaleaTown object 14 (local id 14, gfx 0x3EB0) at 22,17
@ Johto_AzaleaTown object 15 (local id 15, gfx 0x10) at 21,15
@ Johto_CherrygroveCity object 10 (local id 10, gfx 0x3EB0) at 52,9
@ Johto_CherrygroveCity object 11 (local id 11, gfx 0x13A) at 54,10
@ Johto_GoldenrodCity object 31 (local id 31, gfx 0x3EB0) at 23,23
@ Johto_Route35 object 17 (local id 17, gfx 0x3EB0) at 18,8
@ Johto_Route35 object 18 (local id 18, gfx 0x1B3) at 18,9
Quetzal_EventScript_8404959::
	lock
	special Script_ClearHeldMovement
	faceplayer
	special QzPickNpcFlavourLine
	goto_if_eq VAR_0x8006, 1, Quetzal_EventScript_84049B8
	goto_if_eq VAR_0x8005, 1, Quetzal_EventScript_8404A54
	goto_if_eq VAR_0x8005, 2, Quetzal_EventScript_8404A61
	copyvar VAR_0x8000, VAR_0x8004
	goto_if_eq VAR_0x8000, 1, Quetzal_EventScript_8401EDF
	goto_if_eq VAR_0x8000, 2, Quetzal_EventScript_8404ADA
	goto_if_eq VAR_0x8000, 3, Quetzal_EventScript_8404AF9
	goto_if_eq VAR_0x8000, 4, Quetzal_EventScript_8404B32
	goto Quetzal_EventScript_8401C92

Quetzal_EventScript_84049B8::
	goto_if_eq VAR_0x8005, 2, Quetzal_EventScript_84049F9
	copyvar VAR_0x8000, VAR_0x8004
	goto_if_eq VAR_0x8000, 1, Quetzal_EventScript_8404A09
	goto_if_eq VAR_0x8000, 2, Quetzal_EventScript_8404A20
	goto_if_eq VAR_0x8000, 3, Quetzal_EventScript_8404A3A
	goto_if_eq VAR_0x8000, 4, Quetzal_EventScript_8404A47
	goto Quetzal_EventScript_8401C92

Quetzal_EventScript_84049F9::
	goto_if_eq VAR_0x8004, 2, Quetzal_EventScript_8404A2D
	goto Quetzal_EventScript_8404A61

Quetzal_EventScript_8404A09::
	goto_if_set FLAG_QZ_0x1319, Quetzal_EventScript_8401EDF
	special Script_ClearHeldMovement
	faceplayer
	msgbox Quetzal_Text_8404C15, MSGBOX_DEFAULT
	release
	end

Quetzal_EventScript_8404A20::
	msgbox Quetzal_Text_8404C95, MSGBOX_DEFAULT
	goto Quetzal_EventScript_8401C92

Quetzal_EventScript_8404A2D::
	msgbox Quetzal_Text_8404CFC, MSGBOX_DEFAULT
	goto Quetzal_EventScript_8401C92

Quetzal_EventScript_8404A3A::
	msgbox Quetzal_Text_8404D29, MSGBOX_DEFAULT
	goto Quetzal_EventScript_8401C92

Quetzal_EventScript_8404A47::
	msgbox Quetzal_Text_8404D5F, MSGBOX_DEFAULT
	goto Quetzal_EventScript_8401C92

Quetzal_EventScript_8404A54::
	msgbox Quetzal_Text_8405319, MSGBOX_DEFAULT
	goto Quetzal_EventScript_8401C92

Quetzal_EventScript_8404A61::
	goto_if_eq VAR_0x8004, 2, Quetzal_EventScript_8404AC4
	goto_if_eq VAR_0x8004, 3, Quetzal_EventScript_8404A8E
	playmoncry SPECIES_GROWLITHE, 0
	setvar VAR_0x8004, 0xB
	setvar VAR_0x8005, 3
	special QzObjectFaceObject
	goto Quetzal_EventScript_8404AA0

Quetzal_EventScript_8404A8E::
	playmoncry SPECIES_MAREEP, 0
	setvar VAR_0x8004, 0x12
	setvar VAR_0x8005, 1
	special QzObjectFaceObject

Quetzal_EventScript_8404AA0::
	waitmoncry
	goto_if_set FLAG_QZ_0x1319, Quetzal_EventScript_8404AB7
	msgbox Quetzal_Text_840534C, MSGBOX_DEFAULT
	goto Quetzal_EventScript_8401C92

Quetzal_EventScript_8404AB7::
	msgbox Quetzal_Text_840537E, MSGBOX_DEFAULT
	goto Quetzal_EventScript_8401C92

Quetzal_EventScript_8404AC4::
	goto_if_set FLAG_QZ_0x14A6, Quetzal_EventScript_8401F01
	msgbox Quetzal_Text_8404F87, MSGBOX_DEFAULT
	goto Quetzal_EventScript_8401C92

Quetzal_EventScript_8404ADA::
	msgbox Quetzal_Text_84053B7, MSGBOX_DEFAULT
	goto Quetzal_EventScript_8401C92

Quetzal_EventScript_8404AF9::
	goto_if_set FLAG_QZ_0x1319, Quetzal_EventScript_8404B25
	goto_if_unset FLAG_QZ_0x14A7, Quetzal_EventScript_8404B18
	msgbox Quetzal_Text_8405199, MSGBOX_DEFAULT
	goto Quetzal_EventScript_8401C92

Quetzal_EventScript_8404B18::
	msgbox Quetzal_Text_84050CC, MSGBOX_DEFAULT
	goto Quetzal_EventScript_8401C92

Quetzal_EventScript_8404B25::
	msgbox Quetzal_Text_8405466, MSGBOX_DEFAULT
	goto Quetzal_EventScript_8401C92

Quetzal_EventScript_8404B32::
	goto_if_set FLAG_QZ_0x1319, Quetzal_EventScript_8404B5E
	goto_if_unset FLAG_QZ_0x14A8, Quetzal_EventScript_8404B51
	msgbox Quetzal_Text_84052B8, MSGBOX_DEFAULT
	goto Quetzal_EventScript_8401C92

Quetzal_EventScript_8404B51::
	msgbox Quetzal_Text_840524F, MSGBOX_DEFAULT
	goto Quetzal_EventScript_8401C92

Quetzal_EventScript_8404B5E::
	msgbox Quetzal_Text_84054E0, MSGBOX_DEFAULT
	goto Quetzal_EventScript_8401C92

Quetzal_EventScript_8406ED7::
	closemessage
	releaseall
	end

Quetzal_EventScript_840EA62::
	special QzJohtoStaticEncountersRefresh
	return

Quetzal_EventScript_840EA66::
	special QzJohtoStaticEncountersRefresh
	end

@ Johto_AzaleaTown object 9 (local id 9, gfx 0x413) at 50,14
@ Johto_EcruteakCityNorth object 19 (local id 19, gfx 0x4FE) at 35,43
@ Johto_BurnedTower1F object 8 (local id 8, gfx 0xDC8) at 16,7
@ Johto_BurnedTowerB1F object 8 (local id 8, gfx 0x501) at 9,17
@ Johto_BurnedTowerB1F object 9 (local id 9, gfx 0x3CD) at 16,3
@ Johto_CianwoodCity object 14 (local id 14, gfx 0x414) at 22,26
@ Johto_CliffCave object 12 (local id 12, gfx 0x3CF) at 25,27
@ Johto_DarkCaveVioletEntrance object 9 (local id 9, gfx 0x2E7) at 36,19
@ Johto_DragonsDenB1F object 22 (local id 22, gfx 0x2E2) at 24,32
@ Johto_DragonShrine object 5 (local id 5, gfx 0x383) at 8,11
@ Johto_DragonShrine object 6 (local id 6, gfx 0x384) at 4,11
@ Johto_EcruteakCity object 27 (local id 27, gfx 0x412) at 31,21
@ Johto_EcruteakCity object 28 (local id 28, gfx 0x1F4) at 25,18
@ Johto_EcruteakCity object 29 (local id 29, gfx 0x532) at 25,18
@ Johto_DanceTheater object 12 (local id 12, gfx 0xC01) at 12,4
@ Johto_EmbeddedTower object 1 (local id 1, gfx 0x27E) at 14,13
@ Johto_EmbeddedTower object 2 (local id 2, gfx 0x27F) at 14,13
@ Johto_EmbeddedTower object 3 (local id 3, gfx 0x280) at 14,13
@ Johto_GoldenrodCity object 26 (local id 26, gfx 0x411) at 9,24
@ Johto_GoldenrodBikeShop object 3 (local id 3, gfx 0x428) at 7,7
@ Johto_GoldenrodMagnetTrainStation object 10 (local id 10, gfx 0x505) at 15,24
@ Johto_GoldenrodUndergroundWarehouse object 8 (local id 8, gfx 0x41F) at 19,14
@ Johto_GoldenrodUndergroundSwitchRoomEntrances object 11 (local id 11, gfx 0xC59) at 4,4
@ Johto_IcePathB1F object 3 (local id 3, gfx 0x4FF) at 15,16
@ Johto_IcePathB1F object 4 (local id 4, gfx 0x480) at 17,32
@ Johto_IcePathB2FBlackthornSide object 2 (local id 2, gfx 0x190) at 14,10
@ Johto_IcePathB3F object 12 (local id 12, gfx 0x386) at 18,18
@ Johto_IlexForest object 19 (local id 19, gfx 0x197) at 74,42
@ Johto_IlexForest object 20 (local id 20, gfx 0x3CC) at 29,25
@ Johto_IlexForest object 21 (local id 21, gfx 0x5B6) at 10,24
@ Johto_IndigoPlateau object 1 (local id 1, gfx 0x478) at 9,11
@ Johto_IndigoPlateau object 2 (local id 2, gfx 0x479) at 13,11
@ Johto_IndigoPlateau object 3 (local id 3, gfx 0x47A) at 11,11
@ Johto_LakeOfRage object 16 (local id 16, gfx 0x1F5) at 32,23
@ Johto_LakeOfRage object 17 (local id 17, gfx 0x506) at 32,23
@ Johto_LakeOfRage object 18 (local id 18, gfx 0x2E1) at 40,20
@ Johto_MahoganyMart1F object 5 (local id 5, gfx 0x537) at 2,8
@ Johto_MountMortar1FInside object 11 (local id 11, gfx 0x500) at 29,13
@ Johto_MountMortar1FOutside object 3 (local id 3, gfx 0x37E) at 19,42
@ Johto_MountMortar1FOutside object 4 (local id 4, gfx 0x535) at 19,42
@ Johto_MountMortarB1F object 8 (local id 8, gfx 0x3D1) at 24,27
@ Johto_MountMortarB1F object 9 (local id 9, gfx 0x387) at 31,11
@ Johto_MountMortarB1F object 10 (local id 10, gfx 0x47B) at 17,4
@ Johto_SilverCaveItemRoom object 3 (local id 3, gfx 0x420) at 11,8
@ Johto_SilverCaveFireRoom object 1 (local id 1, gfx 0x192) at 17,20
@ Johto_SilverCaveFireRoom object 2 (local id 2, gfx 0x2E5) at 17,6
@ Johto_SilverCaveRoom3 object 2 (local id 2, gfx 0x282) at 16,14
@ Johto_SilverCaveMountainside object 1 (local id 1, gfx 0x281) at 43,20
@ Johto_SilverCaveSnowySlope object 3 (local id 3, gfx 0x482) at 23,41
@ Johto_SilverCaveSnowySlope object 4 (local id 4, gfx 0xDC6) at 20,53
@ Johto_NationalPark object 35 (local id 35, gfx 0x2EC) at 24,28
@ Johto_NationalPark object 36 (local id 36, gfx 0x489) at 34,47
@ Johto_ElmsLab object 9 (local id 9, gfx 0x423) at 11,6
@ Johto_OlivineCity object 21 (local id 21, gfx 0x1F3) at 40,51
@ Johto_OlivineCity object 22 (local id 22, gfx 0x533) at 40,51
@ Johto_OlivineLighthouse object 19 (local id 19, gfx 0x191) at 156,5
@ Johto_OlivineLighthouse object 20 (local id 20, gfx 0x2EE) at 19,14
@ Johto_TeamRocketBaseB2F object 17 (local id 17, gfx 0x41C) at 15,18
@ Johto_TeamRocketBaseB3F object 18 (local id 18, gfx 0x196) at 12,6
@ Johto_TeamRocketBaseB3F object 19 (local id 19, gfx 0x404) at 28,10
@ Johto_Route26South object 6 (local id 6, gfx 0x27D) at 7,63
@ Johto_Route27 object 13 (local id 13, gfx 0x27C) at 33,25
@ Johto_Route29 object 10 (local id 10, gfx 0x51C) at 60,6
@ Johto_Route33 object 4 (local id 4, gfx 0x427) at 28,28
@ Johto_Route35 object 15 (local id 15, gfx 0x51A) at 12,24
@ Johto_Route35 object 16 (local id 16, gfx 0x426) at 17,13
@ Johto_Route41 object 45 (local id 45, gfx 0x382) at 41,20
@ Johto_Route44 object 12 (local id 12, gfx 0x380) at 37,14
@ Johto_Route44 object 13 (local id 13, gfx 0x507) at 37,14
@ Johto_Route45 object 16 (local id 16, gfx 0xDC7) at 28,50
@ Johto_Route45 object 17 (local id 17, gfx 0x519) at 33,9
@ Johto_Route47 object 13 (local id 13, gfx 0x381) at 52,12
@ Johto_Route47 object 14 (local id 14, gfx 0x51B) at 85,14
@ Johto_Route48 object 1 (local id 1, gfx 0x385) at 25,15
@ Johto_RuinsOfAlphInnerChamber object 8 (local id 8, gfx 0x2E6) at 13,15
@ Johto_RuinsOfAlphResearchCenter object 7 (local id 7, gfx 0x389) at 4,3
@ Johto_RuinsOfAlphOutside object 19 (local id 19, gfx 0x504) at 20,12
@ Johto_RuinsOfAlphOutside object 20 (local id 20, gfx 0x2E0) at 29,35
@ Johto_RuinsOfAlphChambers object 18 (local id 18, gfx 0x279) at 20,7
@ Johto_RuinsOfAlphChambers object 19 (local id 19, gfx 0x27A) at 4,24
@ Johto_RuinsOfAlphChambers object 20 (local id 20, gfx 0x27B) at 20,24
@ Johto_RuinsOfAlphChambers object 21 (local id 21, gfx 0x47E) at 3,7
@ Johto_RuinsOfAlphChambers object 22 (local id 22, gfx 0x47F) at 6,7
@ Johto_SafariZone_Desert object 1 (local id 1, gfx 0x41B) at 20,15
@ Johto_SafariZone_Peak object 1 (local id 1, gfx 0x2E8) at 7,21
@ Johto_SafariZone_Forest object 1 (local id 1, gfx 0x41A) at 18,12
@ Johto_SproutTower1F object 8 (local id 8, gfx 0x422) at 5,12
@ Johto_SproutTower2F object 5 (local id 5, gfx 0x481) at 17,6
@ Johto_SproutTower3F object 9 (local id 9, gfx 0x41E) at 6,5
@ Johto_TinTower8F object 4 (local id 4, gfx 0x41D) at 12,5
@ Johto_TohjoFalls object 2 (local id 2, gfx 0x2E4) at 39,19
@ Johto_TohjoFalls object 3 (local id 3, gfx 0x2E3) at 53,19
@ Johto_UnionCaveB1F object 9 (local id 9, gfx 0x536) at 21,12
@ Johto_UnionCaveB2F object 7 (local id 7, gfx 0x3CE) at 3,33
@ Johto_VictoryRoad2F object 6 (local id 6, gfx 0x37F) at 23,13
@ Johto_VictoryRoad2F object 7 (local id 7, gfx 0x534) at 23,13
@ Johto_VictoryRoad1F object 7 (local id 7, gfx 0x425) at 20,18
@ Johto_WhirlIslandTunnelEast object 1 (local id 1, gfx 0x2EA) at 35,14
@ Johto_WhirlIslandTunnelEast object 2 (local id 2, gfx 0x419) at 45,14
Quetzal_EventScript_840EA6A::
	lock
	faceplayer
	special QzJohtoStaticEncounterSetup
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_840EBAE
	call Quetzal_EventScript_840EAAB
	goto_if_eq VAR_RESULT, 1, Quetzal_EventScript_840EBA9
	release
	end

Quetzal_EventScript_840EAAB::
	waitmoncry
	special CreateEnemyEventMon
	setflag FLAG_QZ_0xAD9
	special BattleSetup_StartLegendaryBattle
	waitstate
	clearflag FLAG_QZ_0xAD9
	special QzJohtoStaticEncounterAfterBattle
	return

Quetzal_EventScript_840EBA9::
	removeobject VAR_LAST_TALKED
	release
	end

@ Johto_ElmsLab object 8 (local id 8, gfx 0x2ED) at 6,6
Quetzal_EventScript_840EBAE::
	msgbox Quetzal_Text_840F0C0, MSGBOX_DEFAULT
	release
	end

Quetzal_EventScript_840ED15::
	closemessage
	release
	end

Quetzal_EventScript_840ED18::
	msgbox Quetzal_Text_840F091, MSGBOX_DEFAULT
	release
	end

Quetzal_Movement_835FB23:
	nurse_joy_bow
	delay_4
	step_end

Quetzal_Movement_83607F7:
	emote_question_mark
	step_end

Quetzal_Movement_8360803:
	face_original_direction
	step_end

Quetzal_Movement_8360805:
	walk_in_place_faster_left
	step_end

Quetzal_Movement_836080B:
	walk_in_place_faster_down
	step_end

Quetzal_Movement_8360811:
	face_down
	step_end

Quetzal_Movement_8360831:
	walk_in_place_faster_up
	step_end

Quetzal_Movement_8360837:
	walk_up
	step_end

Quetzal_Movement_8360839:
	walk_down
	step_end

Quetzal_Movement_836083B:
	walk_right
	step_end

Quetzal_Movement_836083D:
	walk_left
	step_end

Quetzal_Movement_837EF26:
	cut_tree
	step_end

Quetzal_Movement_837EF9D:
	rock_smash_break
	step_end

Quetzal_Movement_837F5E7:
	slide_up
	slide_up
	slide_up
	step_end

Quetzal_Movement_837F5EB:
	slide_down
	slide_down
	slide_down
	step_end

Quetzal_Movement_837F5EF:
	slide_right
	slide_right
	slide_right
	step_end

Quetzal_Movement_837F5F3:
	slide_left
	slide_left
	slide_left
	step_end

Quetzal_Movement_83D39C4:
	walk_down
	set_visible
	walk_down
	walk_down
	step_end

Quetzal_Movement_83FBB74:
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

Quetzal_Movement_83FBB7B:
	delay_16
	delay_16
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	step_end

Quetzal_Movement_83FBB84:
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_in_place_down
	delay_16
	delay_16
	step_end

Quetzal_Movement_83FBB8D:
	delay_16
	delay_16
	walk_in_place_down
	walk_in_place_down
	walk_in_place_down
	walk_in_place_down
	step_end

Quetzal_Movement_83FBB94:
	face_right
	delay_16
	walk_right
	delay_16
	face_left
	step_end

Quetzal_Movement_83FBB9A:
	walk_up
	walk_up
	walk_up
	delay_16
	face_down
	step_end

Quetzal_Movement_83FBBA0:
	walk_left
	delay_16
	face_down
	step_end

Quetzal_Movement_83FBBA4:
	face_right
	delay_16
	face_up
	delay_16
	walk_up
	walk_up
	delay_16
	face_left
	delay_16
	walk_left
	delay_16
	face_down
	step_end

Quetzal_Movement_83FBBB1:
	face_right
	delay_16
	face_up
	delay_16
	walk_up
	walk_up
	delay_16
	face_right
	delay_16
	walk_right
	delay_16
	face_down
	step_end

Quetzal_Movement_83FBBBE:
	face_down
	delay_8
	face_right
	delay_8
	face_up
	delay_8
	face_left
	delay_8
	face_down
	delay_8
	delay_8
	delay_8
	delay_8
	delay_8
	delay_16
	face_down
	delay_8
	face_right
	delay_8
	face_up
	delay_8
	face_left
	delay_8
	face_down
	delay_16
	step_end

Quetzal_Movement_83FBBD8:
	face_down
	delay_8
	walk_down
	delay_8
	face_down
	delay_16
	step_end

Quetzal_Movement_83FBBDF:
	face_right
	delay_8
	walk_right
	delay_8
	face_down
	delay_16
	step_end

Quetzal_Movement_83FBBE6:
	face_right
	delay_8
	face_up
	delay_8
	walk_up
	delay_8
	face_down
	delay_16
	step_end

Quetzal_Movement_83FBBEF:
	face_right
	delay_8
	face_up
	delay_8
	face_left
	delay_8
	walk_left
	delay_8
	face_down
	delay_16
	step_end

Quetzal_Movement_83FBBFA:
	face_right
	delay_8
	walk_right
	walk_right
	walk_right
	face_down
	delay_16
	step_end

Quetzal_Movement_83FBC02:
	face_right
	delay_8
	face_up
	delay_8
	face_left
	delay_8
	walk_left
	walk_left
	walk_left
	face_down
	delay_16
	step_end

Quetzal_Movement_83FBC0E:
	face_right
	delay_8
	face_up
	delay_8
	face_left
	delay_8
	walk_left
	delay_8
	face_up
	delay_8
	walk_up
	delay_8
	face_left
	delay_8
	face_down
	delay_16
	step_end

Quetzal_Movement_83FBC1F:
	face_right
	delay_8
	walk_right
	delay_8
	face_up
	delay_8
	walk_up
	delay_8
	face_right
	delay_8
	face_down
	delay_16
	step_end

Quetzal_Movement_83FBC2C:
	face_right
	delay_8
	face_up
	delay_16
	step_end

Quetzal_Movement_83FBC31:
	face_right
	delay_8
	face_up
	delay_8
	walk_up
	delay_8
	face_left
	delay_8
	face_down
	delay_16
	step_end

Quetzal_Movement_83FBC3C:
	face_right
	delay_8
	walk_right
	delay_8
	face_down
	delay_16
	face_left
	step_end

Quetzal_Text_82E0BEC::
	.string "It’s a serious-looking machine.\n"
	.string "The PROF must use this for research.$"

Quetzal_Text_8306D36::
	.string "It’s a VENDING MACHINE.\n"
	.string "Which drink would you like?$"

Quetzal_Text_8306D6A::
	.string "Clang!\p"
	.string "A can of {STR_VAR_1} dropped down.$"

Quetzal_Text_8306D8B::
	.string "Clang!\p"
	.string "Score! An extra can of {STR_VAR_1}\n"
	.string "dropped down!$"

Quetzal_Text_8306DBA::
	.string "Not enough money…$"

Quetzal_Text_8306DCC::
	.string "Decided against buying a drink.$"

Quetzal_Text_83609E1::
	.string "Okay, I’ll take your POKéMON\n"
	.string "for a few seconds.$"

Quetzal_Text_8360A11::
	.string "Thank you for waiting.\p"
	.string "We’ve restored your POKéMON\n"
	.string "to full health.$"

Quetzal_Text_8360A54::
	.string "We hope to see you again!$"

Quetzal_Text_8360A6E::
	.string "Hello, and welcome to\n"
	.string "the POKéMON CENTER.\p"
	.string "We restore your tired POKéMON\n"
	.string "to full health.\p"
	.string "Would you like to…$"

Quetzal_Text_8360AD9::
	.string "Th-that card…\n"
	.string "Could it be… The GOLD CARD?!\p"
	.string "Oh, the gold color is brilliant!\n"
	.string "The four stars seem to sparkle!\p"
	.string "I’ve seen several TRAINERS with\n"
	.string "a SILVER CARD before, but, {PLAYER},\l"
	.string "you’re the first TRAINER I’ve ever\l"
	.string "seen with a GOLD CARD!\p"
	.string "Okay, {PLAYER}, please allow me\n"
	.string "the honor of resting your POKéMON!$"

Quetzal_Text_8360BFB::
	.string "I’m delighted to see you, {PLAYER}!\n"
	.string "You want the usual, am I right?$"

Quetzal_Text_8360C39::
	.string "Okay, I’ll take your POKéMON\n"
	.string "for a few seconds.$"

Quetzal_Text_8360C69::
	.string "Thank you for waiting.$"

Quetzal_Text_8360C80::
	.string "We hope to see you again!$"

Quetzal_Text_8360D6A::
	.string "The BAG is full…$"

Quetzal_Text_83611E8::
	.string "Your POKéMON may be infected with\n"
	.string "POKéRUS.\p"
	.string "Little is known about the POKéRUS\n"
	.string "except that they are microscopic life-\l"
	.string "forms that attach to POKéMON.\p"
	.string "While infected, POKéMON are said to\n"
	.string "grow exceptionally well.$"

Quetzal_Text_83614E5::
	.string "The {STR_VAR_1} flew away!$"

Quetzal_Text_8362911::
	.string "You found {STR_VAR_2} {STR_VAR_1}!\p"
	.string "Do you want to pick the\n"
	.string "{STR_VAR_1}?$"

Quetzal_Text_836293E::
	.string "{PLAYER} picked the {STR_VAR_2} {STR_VAR_1}.$"

Quetzal_Text_8362953::
	.string "{PLAYER} put away the {STR_VAR_1}\n"
	.string "in the BAG’s BERRIES POCKET.\p"
	.string "The soil returned to its soft and\n"
	.string "loamy state.$"

Quetzal_Text_83629B2::
	.string "The BAG’s BERRIES POCKET is full.\p"
	.string "The {STR_VAR_1} couldn’t be taken.$"

Quetzal_Text_83629EE::
	.string "{PLAYER} left the {STR_VAR_1}\n"
	.string "unpicked.$"

Quetzal_Text_836713F::
	.string "It appears as if {STR_VAR_1} is playing\n"
	.string "right now.\l"
	.string "Go for it!$"

Quetzal_Text_837F0F3::
	.string "This tree looks like it can be\n"
	.string "CUT down!\p"
	.string "Would you like to CUT it?$"

Quetzal_Text_837F142::
	.string "This tree looks like it can be\n"
	.string "CUT down!$"

Quetzal_Text_837F16B::
	.string "This rock appears to be breakable.\n"
	.string "Would you like to use ROCK SMASH?$"

Quetzal_Text_837F1B0::
	.string "It’s a rugged rock, but a POKéMON\n"
	.string "may be able to smash it.$"

Quetzal_Text_837F1EB::
	.string "It’s a big boulder, but a POKéMON\n"
	.string "may be able to push it aside.\p"
	.string "Would you like to use STRENGTH?$"

Quetzal_Text_837F295::
	.string "It’s a big boulder, but a POKéMON\n"
	.string "may be able to push it aside.$"

Quetzal_Text_837F2D5::
	.string "STRENGTH made it possible to move\n"
	.string "boulders around.$"

Quetzal_Text_837F5F7::
	.string "The water is swirling with a mighty roar.\n"
	.string "You cannot cross it yet.$"

Quetzal_Text_8383BB5::
	.string "GINA: Oh? Only one POKéMON?\n"
	.string "Then, we don’t battle with you.\p"
	.string "If there’s only one POKéMON, it will\n"
	.string "be lonesome. That’s not nice.$"

Quetzal_Text_8396B35::
	.string "It’s filled with all sorts of books.$"

Quetzal_Text_8396BDB::
	.string "It’s empty.$"

Quetzal_Text_83C2E3D::
	.string "WOBBUFFET: Wobbuffet!$"

Quetzal_Text_83C2E53::
	.string "JESSIE: Finally, some recognition!\n"
	.string "All of JOHTO wears our uniform!\p"
	.string "JAMES: But nobody looks happy...\p"
	.string "MEOWTH: And the boss still hasn’t\n"
	.string "paid to fix our balloon!$"

Quetzal_Text_83C2EF2::
	.string "JESSIE: They say you defied the boss.\n"
	.string "I just see someone passing through.\p"
	.string "JAMES: Picking on someone’s mother...\n"
	.string "That’s got no class at all.\p"
	.string "MEOWTH: Keep going. If anyone asks,\n"
	.string "we were fixing the balloon!$"

Quetzal_Text_83C2FBE::
	.string "JESSIE: The boss is gone?\n"
	.string "This is our big chance!\p"
	.string "JAMES: To rule JOHTO?\p"
	.string "MEOWTH: To go on vacation\n"
	.string "before he comes back!$"

Quetzal_Text_83C36A1::
	.string "You cannot enter the Bug-Catching\n"
	.string "Contest while your Nuzlocke\l"
	.string "challenge is active.\p"
	.string "Come back when your challenge ends!$"

Quetzal_Text_83C9B9F::
	.string "CHANSEY: Chaan! Sii!$"

Quetzal_Text_83D49A0::
	.string "Whew, I’m tired.\p"
	.string "I sometimes come up to the rooftop\n"
	.string "to take a break from shopping.$"

Quetzal_Text_83D4A88::
	.string "Oh, wow!$"

Quetzal_Text_83D4A91::
	.string "Will you quit bothering me?$"

Quetzal_Text_83D4AAD::
	.string "There’s something I really want,\n"
	.string "but I don’t have the necessary cash…\p"
	.string "Maybe I’ll sell off the BERRIES I’ve\n"
	.string "collected…$"

Quetzal_Text_83D4B64::
	.string "My POKéMON always get paralyzed or\n"
	.string "poisoned when the chips are down…\p"
	.string "So I came to buy some FULL HEAL.\p"
	.string "I wonder if there’s any left?$"

Quetzal_Text_83DC904::
	.string "Moves can be managed from the\n"
	.string "POKEMON summary screen.$"

Quetzal_Text_83DFBD4::
	.string "A public RADIO alert interrupts\n"
	.string "the usual broadcast:\p"
	.string "TEAM ROCKET has taken over the\n"
	.string "GOLDENROD RADIO TOWER!\p"
	.string "Someone must stop them!$"

Quetzal_Text_83E0685::
	.string "RED offers you a LIGHT BALL\n"
	.string "as a sign of respect.$"

Quetzal_Text_83E06B6::
	.string "There is no room in the BAG.\p"
	.string "RED will keep the LIGHT BALL\n"
	.string "until you return for it.$"

Quetzal_Text_83E0706::
	.string ".........................\n"
	.string ".........................$"

Quetzal_Text_83E073A::
	.string "...!$"

Quetzal_Text_83E091E::
	.string "Nothing here.$"

Quetzal_Text_83E0B86::
	.string "RELAXATION SQUARE\n"
	.string "NATIONAL PARK$"

Quetzal_Text_83E0BA6::
	.string "What is this notice?\p"
	.string "Please battle only in the grass.\p"
	.string "NATIONAL PARK WARDEN’S OFFICE$"

Quetzal_Text_83E0BFA::
	.string "The NATIONAL PARK has different\n"
	.string "POKéMON during the BUG-CATCHING\l"
	.string "CONTEST.$"

Quetzal_Text_83EB7ED::
	.string "MURKROW: Hail GIOVANNI.$"

Quetzal_Text_83EBCD7::
	.string "I’m making my final preparations\n"
	.string "for the POKéMON LEAGUE.$"

Quetzal_Text_83EBD10::
	.string "I blew it!$"

Quetzal_Text_83EBD1B::
	.string "It’s going to be tough to win at\n"
	.string "the LEAGUE.\p"
	.string "I need to do some more training.\p"
	.string "I hear that the LEAGUE’s ELITE\n"
	.string "FOUR are tougher than GYM LEADERS.$"

Quetzal_Text_83EBE70::
	.string "Since you’ve come this far,\n"
	.string "you must be good.\p"
	.string "I’m going to give this battle\n"
	.string "everything I’ve got!$"

Quetzal_Text_83EBED1::
	.string "No! I don’t believe this!$"

Quetzal_Text_83EBEEB::
	.string "I’ve defeated eight GYM LEADERS,\n"
	.string "so I was feeling confident.\p"
	.string "I’ll have to try harder next time.$"

Quetzal_Text_83ECC08::
	.string "Press A or B to stop viewing.$"

Quetzal_Text_83ED204::
	.string "Not quite!$"

Quetzal_Text_83F4AB1::
	.string "TENTACRUEL: Cruooo...$"

Quetzal_Text_83F863F::
	.string "It’s a replica of an ancient\n"
	.string "POKéMON.$"

Quetzal_Text_83FA742::
	.string "A POKéMON statue…\p"
	.string "It looks very distinguished.$"

Quetzal_Text_83FBA4E::
	.string "You have not only made HO-OH\n"
	.string "appear, but you have also defeated\l"
	.string "it...\p"
	.string "There is nothing more for us to\n"
	.string "say. Take care of yourself in your\l"
	.string "journey...$"

Quetzal_Text_83FBAE2::
	.string "You have not only made HO-OH\n"
	.string "appear, but you have also caught\l"
	.string "it...\p"
	.string "There is nothing more for us to\n"
	.string "say. Take care of yourself in your\l"
	.string "journey...$"

Quetzal_Text_83FBC44::
	.string "ZUKI: Go, face HO-OH. $"

Quetzal_Text_83FBC5B::
	.string "NAOKO: Go, face HO-OH. $"

Quetzal_Text_83FBC73::
	.string "MIKI: Go, face HO-OH. $"

Quetzal_Text_83FBC8A::
	.string "SAYO: Go, face HO-OH. $"

Quetzal_Text_83FBCA1::
	.string "KUNI: Go, face HO-OH. $"

Quetzal_Text_83FBCB8::
	.string "KIMONO GIRL: That’s right.\n"
	.string "This is where we welcome HO-OH.\p"
	.string "When the dance we practiced many\n"
	.string "days becomes one with the sound of the\p"
	.string "CLEAR BELL we entrusted to you, then\n"
	.string "HO-OH shall come down from the great\l"
	.string "arch of the sky once again. $"

Quetzal_Text_83FBDA2::
	.string "That is indeed HO-OH...\n"
	.string "The guardian of the sky from\p"
	.string "ancient times... \p"
	.string "So many have tried again and again,\n"
	.string "only to fail...\p"
	.string "{PLAYER}, your heart in complete\n"
	.string "harmony with the CLEAR BELL has finally\p"
	.string "allowed it to appear...\n"
	.string "{PLAYER}, don’t you see?\p"
	.string "HO-OH... It must have been waiting for\n"
	.string "someone like you all this time.\p"
	.string "Wouldn’t you agree? $"

Quetzal_Text_83FBEE7::
	.string "KIMONO GIRL: HO-OH has acknowledged\n"
	.string "your strength, but one guardian remains.\p"
	.string "Take the SILVER WING as well.\p"
	.string "Bring the TIDAL BELL to the WHIRL\n"
	.string "ISLANDS.\p"
	.string "We will be waiting there.$"

Quetzal_Text_83FBF97::
	.string "You have done a great job.\n"
	.string "Say hello to PROF.ELM for us.$"

Quetzal_Text_840184F::
	.string "{PLAYER} received {STR_VAR_1}!$"

Quetzal_Text_8401892::
	.string "Whoa, wait. You can’t carry any\n"
	.string "more POKéMON.$"

Quetzal_Text_84018D6::
	.string "The door is locked.$"

Quetzal_Text_8401EDB::
	.string "...$"

Quetzal_Text_8402C9D::
	.string "CELEBI surrounds you with its light.$"

Quetzal_Text_8402E50::
	.string "They demand part of what we earn.\n"
	.string "If we refuse, they threaten to take\l"
	.string "someone from our family away.\p"
	.string "Working for them does not mean I\n"
	.string "support them. They gave us no choice.$"

Quetzal_Text_8402EFB::
	.string "Today we can work without handing\n"
	.string "our earnings over to the Rockets.\p"
	.string "At last, we can use that money\n"
	.string "to take care of our family.$"

Quetzal_Text_8402F7A::
	.string "Grandpa said this had happened before\n"
	.string "and he would not let it happen again.\p"
	.string "They have not let him come home since.\n"
	.string "They force him to make\l"
	.string "Poké Balls for them.$"

Quetzal_Text_8403019::
	.string "Grandpa is back! He says he will never\n"
	.string "make another Poké Ball out of fear.\p"
	.string "Tonight we will eat together.$"

Quetzal_Text_8404C15::
	.string "The Rocket left for the tower.\n"
	.string "They say a rebel is on the loose.\p"
	.string "For once, I can go home\n"
	.string "without him demanding another payment.$"

Quetzal_Text_8404C95::
	.string "KURT: My guard was called away.\n"
	.string "He has left me a moment of peace.\p"
	.string "I’ll spend it with my granddaughter.$"

Quetzal_Text_8404CFC::
	.string "The guard ran off.\n"
	.string "I’m staying with Grandpa.$"

Quetzal_Text_8404D29::
	.string "MAREEP is still with me.\n"
	.string "I won’t let us be separated.$"

Quetzal_Text_8404D5F::
	.string "GROWLITHE is by my side.\n"
	.string "That’s what matters to me.$"

Quetzal_Text_8404F87::
	.string "Grandpa! You said you\n"
	.string "would come home today...$"

Quetzal_Text_84050CC::
	.string "TRAINER: It is valid!\n"
	.string "You issued it right here yesterday.$"

Quetzal_Text_8405199::
	.string "I have the permit.\n"
	.string "It has my name and their seal...\p"
	.string "But I no longer have MAREEP.$"

Quetzal_Text_840524F::
	.string "It does not understand your orders.\n"
	.string "It thinks we are going home together.\p"
	.string "Please... let me take it home.$"

Quetzal_Text_84052B8::
	.string "They said I would get\n"
	.string "another if I cooperated.\p"
	.string "But I do not want another.\n"
	.string "I want my friend back.$"

Quetzal_Text_8405319::
	.string "We have orders from the boss.\n"
	.string "Stay out of the way.$"

Quetzal_Text_840534C::
	.string "It looks toward its trainer.\n"
	.string "It seems frightened.$"

Quetzal_Text_840537E::
	.string "It stays close to its trainer.\n"
	.string "It is not afraid anymore.$"

Quetzal_Text_84053B7::
	.string "KURT: I will not call this right\n"
	.string "just because their orders say so.$"

Quetzal_Text_8405466::
	.string "MAREEP is back! It ran to me\n"
	.string "as if nothing had happened.\p"
	.string "I tore up that permit. We do not\n"
	.string "need permission to be together.$"

Quetzal_Text_84054E0::
	.string "This time we did go home together.\p"
	.string "GROWLITHE still looks back\n"
	.string "when it hears footsteps.\l"
	.string "But it is not alone anymore.$"

Quetzal_Text_840745D::
	.string "Old notes describe a sleeping\n"
	.string "artificial Pokémon.\p"
	.string "It’s in the switch room in\n"
	.string "Goldenrod’s Underground.\p"
	.string "They say it responds to a party\n"
	.string "of six Steel-type Pokémon.\p"
	.string "The first must be holding\n"
	.string "a Heart Scale.$"

Quetzal_Text_8407527::
	.string "I don’t know where to find that\n"
	.string "scale, so I haven’t tried it.\p"
	.string "If you find a Magearna with its\n"
	.string "original colors, visit my house.$"

Quetzal_Text_84075A6::
	.string "I found references to an\n"
	.string "artificial Pokémon.\p"
	.string "It’s in the switch room in\n"
	.string "Goldenrod’s Underground.\p"
	.string "If you find a Magearna with its\n"
	.string "original colors, visit my house.\p"
	.string "I’d like to study how it\n"
	.string "senses emotions.$"

Quetzal_Text_8407672::
	.string "I’m interested in the Magearna\n"
	.string "you found in the Underground.\p"
	.string "If you have one with its original\n"
	.string "colors in your party, I need help.$"

Quetzal_Text_840F091::
	.string "Make room in your Bag,\n"
	.string "then speak to me again.$"

Quetzal_Text_840F0C0::
	.string "...$"
