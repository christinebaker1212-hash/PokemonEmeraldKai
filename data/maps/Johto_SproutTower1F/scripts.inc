Johto_SproutTower1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_SproutTower1F_EventScript_83FA398
	.byte 0

Johto_SproutTower1F_EventScript_83FA398::
	call Quetzal_EventScript_840EA62
	end

@ Johto_SproutTower1F object 4 (local id 4, gfx 0x3EB2) at 1,2
Johto_SproutTower1F_EventScript_83FA39E::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4673, 0, Johto_SproutTower1F_Text_83FA3F6, Johto_SproutTower1F_Text_83FA449
	msgbox Johto_SproutTower1F_Text_83FA457, MSGBOX_AUTOCLOSE
	end

@ Johto_SproutTower1F object 3 (local id 3, gfx 0x3EB2) at 6,8
Johto_SproutTower1F_EventScript_83FA3B5::
	lock
	faceplayer
	msgbox Johto_SproutTower1F_Text_83FA4AB, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_SproutTower1F object 2 (local id 2, gfx 0x3EB2) at 10,4
Johto_SproutTower1F_EventScript_83FA3C2::
	lock
	faceplayer
	msgbox Johto_SproutTower1F_Text_83FA4DC, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_SproutTower1F object 5 (local id 5, gfx 0x1E) at 16,4
Johto_SproutTower1F_EventScript_83FA3CF::
	lock
	faceplayer
	msgbox Johto_SproutTower1F_Text_83FA51D, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_SproutTower1F object 1 (local id 1, gfx 0x2F) at 15,9
Johto_SproutTower1F_EventScript_83FA3DC::
	lock
	faceplayer
	msgbox Johto_SproutTower1F_Text_83FA570, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_SproutTower1F object 6 (local id 6, gfx 0x3A9E) at 19,7
Johto_SproutTower1F_EventScript_83FA3E9::
	setorcopyvar VAR_0x8000, ITEM_PARALYZE_HEAL
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Johto_SproutTower1F_Text_83FA3F6::
	.string "We stand guard in this tower.\p"
	.string "Here, we express our gratitude\n"
	.string "to honor all POKéMON.$"

Johto_SproutTower1F_Text_83FA449::
	.string "Th-Thank you!$"

Johto_SproutTower1F_Text_83FA457::
	.string "All living beings coexist\n"
	.string "through cooperation.\p"
	.string "We must always be thankful\n"
	.string "for this.$"

Johto_SproutTower1F_Text_83FA4AB::
	.string "Only if you reach the top will\n"
	.string "you obtain an HM.$"

Johto_SproutTower1F_Text_83FA4DC::
	.string "SPROUT TOWER was built long ago\n"
	.string "as a place for POKéMON training.$"

Johto_SproutTower1F_Text_83FA51D::
	.string "A BELLSPROUT over 100 feet\n"
	.string "tall…\p"
	.string "People say that it became the\n"
	.string "center pillar here.$"

Johto_SproutTower1F_Text_83FA570::
	.string "See the pillar shaking?\p"
	.string "People are training upstairs.$"
