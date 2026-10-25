Kanto_CeladonCity_DepartmentStore_3F_MapScripts::
	.byte 0

@ Kanto_CeladonCity_DepartmentStore_3F object 1 (local id 1, gfx 0x3EFA) at 10,9
Kanto_CeladonCity_DepartmentStore_3F_EventScript_841BBC1::
	specialvar VAR_RESULT, QzReturnZero_732
	goto_if_eq VAR_RESULT, 2, Quetzal_EventScript_8457FA3
	lock
	faceplayer

Kanto_CeladonCity_DepartmentStore_3F_EventScript_841BBD3::
	message Kanto_CeladonCity_DepartmentStore_3F_Text_8445A81
	waitmessage
	multichoice 0, 0, 0x8B, 1
	nop
	copyvar VAR_0x8000, VAR_RESULT
	goto_if_eq VAR_0x8000, 0, Kanto_CeladonCity_DepartmentStore_3F_EventScript_841BC25
	goto_if_eq VAR_0x8000, 1, Kanto_CeladonCity_DepartmentStore_3F_EventScript_841BC30
	goto_if_eq VAR_0x8000, 2, Kanto_CeladonCity_DepartmentStore_3F_EventScript_841BC3B
	goto_if_eq VAR_0x8000, 3, Kanto_CeladonCity_DepartmentStore_3F_EventScript_841BC46
	goto_if_eq VAR_0x8000, 4, Kanto_CeladonCity_DepartmentStore_3F_EventScript_841BC51
	msgbox Quetzal_Text_84565DA, MSGBOX_DEFAULT
	release
	end

Kanto_CeladonCity_DepartmentStore_3F_EventScript_841BC25::
	closemessage
	pokemart Kanto_CeladonCity_DepartmentStore_3F_Mart_841BCC0
	goto Kanto_CeladonCity_DepartmentStore_3F_EventScript_841BBD3

Kanto_CeladonCity_DepartmentStore_3F_EventScript_841BC30::
	closemessage
	pokemart Kanto_CeladonCity_DepartmentStore_3F_Mart_841BD34
	goto Kanto_CeladonCity_DepartmentStore_3F_EventScript_841BBD3

Kanto_CeladonCity_DepartmentStore_3F_EventScript_841BC3B::
	closemessage
	pokemart Kanto_CeladonCity_DepartmentStore_3F_Mart_841BD58
	goto Kanto_CeladonCity_DepartmentStore_3F_EventScript_841BBD3

Kanto_CeladonCity_DepartmentStore_3F_EventScript_841BC46::
	closemessage
	pokemart Kanto_CeladonCity_DepartmentStore_3F_Mart_841BD78
	goto Kanto_CeladonCity_DepartmentStore_3F_EventScript_841BBD3

Kanto_CeladonCity_DepartmentStore_3F_EventScript_841BC51::
	closemessage
	pokemart Kanto_CeladonCity_DepartmentStore_3F_Mart_841BDA0
	goto Kanto_CeladonCity_DepartmentStore_3F_EventScript_841BBD3

@ Kanto_CeladonCity_DepartmentStore_3F object 2 (local id 2, gfx 0x3F08) at 11,6
Kanto_CeladonCity_DepartmentStore_3F_EventScript_841BC5C::
	msgbox Kanto_CeladonCity_DepartmentStore_3F_Text_844582E, MSGBOX_NPC
	end

@ Kanto_CeladonCity_DepartmentStore_3F object 4 (local id 4, gfx 0x3F08) at 1,9
Kanto_CeladonCity_DepartmentStore_3F_EventScript_841BC65::
	msgbox Kanto_CeladonCity_DepartmentStore_3F_Text_84458AE, MSGBOX_NPC
	end

@ Kanto_CeladonCity_DepartmentStore_3F object 3 (local id 3, gfx 0x3F08) at 4,9
Kanto_CeladonCity_DepartmentStore_3F_EventScript_841BC6E::
	msgbox Kanto_CeladonCity_DepartmentStore_3F_Text_84458F5, MSGBOX_NPC
	end

@ Kanto_CeladonCity_DepartmentStore_3F object 5 (local id 5, gfx 0x3F18) at 9,14
Kanto_CeladonCity_DepartmentStore_3F_EventScript_841BC77::
	msgbox Kanto_CeladonCity_DepartmentStore_3F_Text_8445968, MSGBOX_NPC
	end

@ Kanto_CeladonCity_DepartmentStore_3F sign / bg event at 1,8
@ Kanto_CeladonCity_DepartmentStore_3F sign / bg event at 1,11
@ Kanto_CeladonCity_DepartmentStore_3F sign / bg event at 4,8
@ Kanto_CeladonCity_DepartmentStore_3F sign / bg event at 4,11
Kanto_CeladonCity_DepartmentStore_3F_EventScript_841BC80::
	msgbox Kanto_CeladonCity_DepartmentStore_3F_Text_84459A8, MSGBOX_SIGN
	end

@ Kanto_CeladonCity_DepartmentStore_3F sign / bg event at 2,8
Kanto_CeladonCity_DepartmentStore_3F_EventScript_841BC89::
	msgbox Kanto_CeladonCity_DepartmentStore_3F_Text_84459BA, MSGBOX_SIGN
	end

@ Kanto_CeladonCity_DepartmentStore_3F sign / bg event at 5,8
Kanto_CeladonCity_DepartmentStore_3F_EventScript_841BC92::
	msgbox Kanto_CeladonCity_DepartmentStore_3F_Text_84459DC, MSGBOX_SIGN
	end

@ Kanto_CeladonCity_DepartmentStore_3F sign / bg event at 2,11
Kanto_CeladonCity_DepartmentStore_3F_EventScript_841BC9B::
	msgbox Kanto_CeladonCity_DepartmentStore_3F_Text_84459FD, MSGBOX_SIGN
	end

@ Kanto_CeladonCity_DepartmentStore_3F sign / bg event at 5,11
Kanto_CeladonCity_DepartmentStore_3F_EventScript_841BCA4::
	msgbox Kanto_CeladonCity_DepartmentStore_3F_Text_8445A1D, MSGBOX_SIGN
	end

@ Kanto_CeladonCity_DepartmentStore_3F sign / bg event at 8,1
Kanto_CeladonCity_DepartmentStore_3F_EventScript_841BCAD::
	msgbox Kanto_CeladonCity_DepartmentStore_3F_Text_8445A3B, MSGBOX_SIGN
	end

@ Kanto_CeladonCity_DepartmentStore_3F sign / bg event at 2,4
@ Kanto_CeladonCity_DepartmentStore_3F sign / bg event at 0,4
Kanto_CeladonCity_DepartmentStore_3F_EventScript_841BCB6::
	msgbox Kanto_CeladonCity_DepartmentStore_3F_Text_8445ABF, MSGBOX_SIGN
	end

Kanto_CeladonCity_DepartmentStore_3F_Mart_841BCC0:
	.2byte ITEM_LEFTOVERS
	.2byte ITEM_BRIGHT_POWDER
	.2byte ITEM_WHITE_HERB
	.2byte ITEM_QUICK_CLAW
	.2byte ITEM_MENTAL_HERB
	.2byte ITEM_FOCUS_BAND
	.2byte ITEM_SCOPE_LENS
	.2byte ITEM_CHOICE_BAND
	.2byte ITEM_CHOICE_SPECS
	.2byte ITEM_CHOICE_SCARF
	.2byte ITEM_SHELL_BELL
	.2byte ITEM_WIDE_LENS
	.2byte ITEM_MUSCLE_BAND
	.2byte ITEM_WISE_GLASSES
	.2byte ITEM_EXPERT_BELT
	.2byte ITEM_LIGHT_CLAY
	.2byte ITEM_LIFE_ORB
	.2byte ITEM_POWER_HERB
	.2byte ITEM_FOCUS_SASH
	.2byte ITEM_ZOOM_LENS
	.2byte ITEM_METRONOME
	.2byte ITEM_IRON_BALL
	.2byte ITEM_LAGGING_TAIL
	.2byte ITEM_DESTINY_KNOT
	.2byte ITEM_BLACK_SLUDGE
	.2byte ITEM_GRIP_CLAW
	.2byte ITEM_STICKY_BARB
	.2byte ITEM_SHED_SHELL
	.2byte ITEM_BIG_ROOT
	.2byte ITEM_EVIOLITE
	.2byte ITEM_FLOAT_STONE
	.2byte ITEM_ROCKY_HELMET
	.2byte ITEM_AIR_BALLOON
	.2byte ITEM_RED_CARD
	.2byte ITEM_RING_TARGET
	.2byte ITEM_BINDING_BAND
	.2byte ITEM_EJECT_BUTTON
	.2byte ITEM_WEAKNESS_POLICY
	.2byte ITEM_ASSAULT_VEST
	.2byte ITEM_SAFETY_GOGGLES
	.2byte ITEM_ADRENALINE_ORB
	.2byte ITEM_TERRAIN_EXTENDER
	.2byte ITEM_PROTECTIVE_PADS
	.2byte ITEM_THROAT_SPRAY
	.2byte ITEM_EJECT_PACK
	.2byte ITEM_HEAVY_DUTY_BOOTS
	.2byte ITEM_BLUNDER_POLICY
	.2byte ITEM_ROOM_SERVICE
	.2byte ITEM_UTILITY_UMBRELLA
	.2byte ITEM_ABILITY_SHIELD
	.2byte ITEM_CLEAR_AMULET
	.2byte ITEM_PUNCHING_GLOVE
	.2byte ITEM_COVERT_CLOAK
	.2byte ITEM_LOADED_DICE
	.2byte ITEM_BOOSTER_ENERGY
	.2byte ITEM_MIRROR_HERB
	.2byte ITEM_NONE

Kanto_CeladonCity_DepartmentStore_3F_Mart_841BD34:
	.2byte ITEM_SILK_SCARF
	.2byte ITEM_CHARCOAL
	.2byte ITEM_MYSTIC_WATER
	.2byte ITEM_MAGNET
	.2byte ITEM_MIRACLE_SEED
	.2byte ITEM_NEVER_MELT_ICE
	.2byte ITEM_BLACK_BELT
	.2byte ITEM_POISON_BARB
	.2byte ITEM_SOFT_SAND
	.2byte ITEM_SHARP_BEAK
	.2byte ITEM_TWISTED_SPOON
	.2byte ITEM_SILVER_POWDER
	.2byte ITEM_HARD_STONE
	.2byte ITEM_SPELL_TAG
	.2byte ITEM_DRAGON_FANG
	.2byte ITEM_BLACK_GLASSES
	.2byte ITEM_FAIRY_FEATHER
	.2byte ITEM_NONE

Kanto_CeladonCity_DepartmentStore_3F_Mart_841BD58:
	.2byte ITEM_FLAME_ORB
	.2byte ITEM_TOXIC_ORB
	.2byte ITEM_DAMP_ROCK
	.2byte ITEM_HEAT_ROCK
	.2byte ITEM_SMOOTH_ROCK
	.2byte ITEM_ICY_ROCK
	.2byte ITEM_ELECTRIC_SEED
	.2byte ITEM_PSYCHIC_SEED
	.2byte ITEM_MISTY_SEED
	.2byte ITEM_GRASSY_SEED
	.2byte ITEM_ABSORB_BULB
	.2byte ITEM_CELL_BATTERY
	.2byte ITEM_LUMINOUS_MOSS
	.2byte ITEM_SNOWBALL
	.2byte ITEM_NONE

Kanto_CeladonCity_DepartmentStore_3F_Mart_841BD78:
	.2byte ITEM_NORMAL_GEM
	.2byte ITEM_FIRE_GEM
	.2byte ITEM_WATER_GEM
	.2byte ITEM_ELECTRIC_GEM
	.2byte ITEM_GRASS_GEM
	.2byte ITEM_ICE_GEM
	.2byte ITEM_FIGHTING_GEM
	.2byte ITEM_POISON_GEM
	.2byte ITEM_GROUND_GEM
	.2byte ITEM_FLYING_GEM
	.2byte ITEM_PSYCHIC_GEM
	.2byte ITEM_BUG_GEM
	.2byte ITEM_ROCK_GEM
	.2byte ITEM_GHOST_GEM
	.2byte ITEM_DRAGON_GEM
	.2byte ITEM_DARK_GEM
	.2byte ITEM_STEEL_GEM
	.2byte ITEM_FAIRY_GEM
	.2byte ITEM_NONE

Kanto_CeladonCity_DepartmentStore_3F_Mart_841BDA0:
	.2byte ITEM_X_SPEED
	.2byte ITEM_X_SP_ATK
	.2byte ITEM_X_ATTACK
	.2byte ITEM_X_DEFENSE
	.2byte ITEM_X_SP_DEF
	.2byte ITEM_DIRE_HIT
	.2byte ITEM_GUARD_SPEC
	.2byte ITEM_X_ACCURACY
	.2byte ITEM_NONE

Kanto_CeladonCity_DepartmentStore_3F_Text_844582E::
	.string "Captured POKéMON are registered\n"
	.string "with an ID No. and the OT.\p"
	.string "OT stands for “Original TRAINER” -\n"
	.string "the TRAINER that caught it first.$"

Kanto_CeladonCity_DepartmentStore_3F_Text_84458AE::
	.string "All right!\p"
	.string "My buddy’s going to trade me his\n"
	.string "KANGASKHAN for my HAUNTER!$"

Kanto_CeladonCity_DepartmentStore_3F_Text_84458F5::
	.string "Come on, HAUNTER!\p"
	.string "I love HAUNTER!\n"
	.string "I collect them!\p"
	.string "Huh?\p"
	.string "HAUNTER turned into a different\n"
	.string "POKéMON while being traded!$"

Kanto_CeladonCity_DepartmentStore_3F_Text_8445968::
	.string "You can identify POKéMON you get\n"
	.string "in trades by their ID Numbers.$"

Kanto_CeladonCity_DepartmentStore_3F_Text_84459A8::
	.string "It’s a Super NES.$"

Kanto_CeladonCity_DepartmentStore_3F_Text_84459BA::
	.string "An RPG!\n"
	.string "There’s no time for that!$"

Kanto_CeladonCity_DepartmentStore_3F_Text_84459DC::
	.string "A sports game!\n"
	.string "Dad’ll like that!$"

Kanto_CeladonCity_DepartmentStore_3F_Text_84459FD::
	.string "A puzzle game!\n"
	.string "Looks addictive!$"

Kanto_CeladonCity_DepartmentStore_3F_Text_8445A1D::
	.string "A fighting game!\n"
	.string "Looks tough!$"

Kanto_CeladonCity_DepartmentStore_3F_Text_8445A3B::
	.string "3F: BATTLE SUPPLIES\p"
	.string "Held, type, and weather items,\n"
	.string "gems, and X items.$"

Kanto_CeladonCity_DepartmentStore_3F_Text_8445A81::
	.string "Get your team ready for battle.\n"
	.string "Which section would you like?$"

Kanto_CeladonCity_DepartmentStore_3F_Text_8445ABF::
	.string "Red and Green!\n"
	.string "Both are POKéMON!$"
