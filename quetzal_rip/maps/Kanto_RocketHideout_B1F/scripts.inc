Kanto_RocketHideout_B1F_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Kanto_RocketHideout_B1F_EventScript_8410E47
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_RocketHideout_B1F_EventScript_8410E51
	.byte 0

@ Kanto_RocketHideout_B1F object 8 (local id 8, gfx 0x61) at 7,17
Kanto_RocketHideout_B1F_EventScript_840FBDF::
	lock
	faceplayer
	msgbox Quetzal_Text_840FC22, MSGBOX_DEFAULT
	special QzKantoStaticEncounterSetupByLocalId_758
	goto_if_eq VAR_RESULT, 0, Kanto_RocketHideout_B1F_EventScript_840FBFF
	call Quetzal_EventScript_840FC01
	clearflag FLAG_QZ_0x23DC

Kanto_RocketHideout_B1F_EventScript_840FBFF::
	release
	end

Kanto_RocketHideout_B1F_EventScript_8410E47::
	checktrainerflag 0x216A
	call_if_lt Kanto_RocketHideout_B1F_EventScript_8410EDF
	end

Kanto_RocketHideout_B1F_EventScript_8410E51::
	setflag FLAG_QZ_0x224B
	call Quetzal_EventScript_840FBB5
	end

@ Kanto_RocketHideout_B1F object 2 (local id 2, gfx 0x3F3D) at 24,12
Kanto_RocketHideout_B1F_EventScript_8410E5A::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8550, 0, Kanto_RocketHideout_B1F_Text_8425582, Kanto_RocketHideout_B1F_Text_84255A5
	msgbox Kanto_RocketHideout_B1F_Text_84255B2, MSGBOX_AUTOCLOSE
	end

@ Kanto_RocketHideout_B1F object 1 (local id 1, gfx 0x3F3D) at 4,9
Kanto_RocketHideout_B1F_EventScript_8410E71::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8551, 0, Kanto_RocketHideout_B1F_Text_84255E4, Kanto_RocketHideout_B1F_Text_8425618
	msgbox Kanto_RocketHideout_B1F_Text_842561E, MSGBOX_AUTOCLOSE
	end

@ Kanto_RocketHideout_B1F object 4 (local id 4, gfx 0x3F3D) at 10,22
Kanto_RocketHideout_B1F_EventScript_8410E88::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8552, 0, Kanto_RocketHideout_B1F_Text_842564C, Kanto_RocketHideout_B1F_Text_842565C
	msgbox Kanto_RocketHideout_B1F_Text_842566B, MSGBOX_AUTOCLOSE
	end

@ Kanto_RocketHideout_B1F object 3 (local id 3, gfx 0x3F3D) at 6,32
Kanto_RocketHideout_B1F_EventScript_8410E9F::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8553, 0, Kanto_RocketHideout_B1F_Text_8425699, Kanto_RocketHideout_B1F_Text_84256B0
	msgbox Kanto_RocketHideout_B1F_Text_84256BF, MSGBOX_AUTOCLOSE
	end

@ Kanto_RocketHideout_B1F object 5 (local id 5, gfx 0x3F3D) at 21,27
Kanto_RocketHideout_B1F_EventScript_8410EB6::
	trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT, TRAINER_QZ_8554, 0, Kanto_RocketHideout_B1F_Text_84256F2, Kanto_RocketHideout_B1F_Text_8425712, Kanto_RocketHideout_B1F_EventScript_8410ED1
	msgbox Kanto_RocketHideout_B1F_Text_8425718, MSGBOX_AUTOCLOSE
	end

Kanto_RocketHideout_B1F_EventScript_8410ED1::
	call Kanto_RocketHideout_B1F_EventScript_8410F16
	playse SE_UNLOCK
	special DrawWholeMapView
	waitse
	release
	end

Kanto_RocketHideout_B1F_EventScript_8410EDF::
	setmetatile 0x14, 0x13, 0x2CE, 1
	setmetatile 0x15, 0x13, 0x2CF, 1
	setmetatile 0x14, 0x14, 0x2D6, 1
	setmetatile 0x15, 0x14, 0x2D7, 1
	setmetatile 0x14, 0x15, 0x282, 0
	setmetatile 0x15, 0x15, 0x282, 0
	return

Kanto_RocketHideout_B1F_EventScript_8410F16::
	setmetatile 0x14, 0x13, 0x282, 0
	setmetatile 0x15, 0x13, 0x281, 0
	setmetatile 0x14, 0x14, 0x282, 0
	setmetatile 0x15, 0x14, 0x281, 0
	setmetatile 0x14, 0x15, 0x282, 0
	setmetatile 0x15, 0x15, 0x281, 0
	return

@ Kanto_RocketHideout_B1F object 6 (local id 6, gfx 0x3F0E) at 5,16
Kanto_RocketHideout_B1F_EventScript_846BB7C::
	setorcopyvar VAR_0x8000, 0x78
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_RocketHideout_B1F object 7 (local id 7, gfx 0x3F0E) at 1,22
Kanto_RocketHideout_B1F_EventScript_846BB89::
	setorcopyvar VAR_0x8000, 0x1E
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_RocketHideout_B1F_Text_8425582::
	.string "Who are you?\n"
	.string "How did you get here?$"

Kanto_RocketHideout_B1F_Text_84255A5::
	.string "Oww!\n"
	.string "Beaten!$"

Kanto_RocketHideout_B1F_Text_84255B2::
	.string "Blast it… You’re dissing TEAM\n"
	.string "ROCKET, aren’t you?$"

Kanto_RocketHideout_B1F_Text_84255E4::
	.string "You broke into TEAM ROCKET’s\n"
	.string "operation? What nerve!$"

Kanto_RocketHideout_B1F_Text_8425618::
	.string "Boom!$"

Kanto_RocketHideout_B1F_Text_842561E::
	.string "You’re not going to get away with\n"
	.string "this, brat!$"

Kanto_RocketHideout_B1F_Text_842564C::
	.string "Intruder alert!$"

Kanto_RocketHideout_B1F_Text_842565C::
	.string "I can’t do it!$"

Kanto_RocketHideout_B1F_Text_842566B::
	.string "SILPH SCOPE? Humph!\n"
	.string "I don’t know where it is.$"

Kanto_RocketHideout_B1F_Text_8425699::
	.string "Why did you come here?$"

Kanto_RocketHideout_B1F_Text_84256B0::
	.string "This won’t do!$"

Kanto_RocketHideout_B1F_Text_84256BF::
	.string "Okay, I’ll talk…\n"
	.string "Take the elevator to see my BOSS.$"

Kanto_RocketHideout_B1F_Text_84256F2::
	.string "Are you lost, you little mouse?$"

Kanto_RocketHideout_B1F_Text_8425712::
	.string "Why…?$"

Kanto_RocketHideout_B1F_Text_8425718::
	.string "Uh-oh, that ruckus somehow opened\n"
	.string "the door!$"
