Johto_SafariZoneGate_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Quetzal_EventScript_83C2D44
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_83C2D44
	map_script MAP_SCRIPT_ON_RESUME, Quetzal_EventScript_83C2D44
	.byte 0

@ Johto_SafariZoneGate object 5 (local id 5, gfx 0x3EBF) at 12,23
Johto_SafariZoneGate_EventScript_83F8822::
	lock
	faceplayer
	msgbox Johto_SafariZoneGate_Text_83F8863, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_SafariZoneGate object 7 (local id 7, gfx 0x3E91) at 22,11
Johto_SafariZoneGate_EventScript_83F883C::
	lock
	faceplayer
	msgbox Johto_SafariZoneGate_Text_83F890A, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_SafariZoneGate object 6 (local id 6, gfx 0x6) at 18,17
Johto_SafariZoneGate_EventScript_83F8849::
	lock
	faceplayer
	msgbox Johto_SafariZoneGate_Text_83F896C, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_SafariZoneGate object 4 (local id 4, gfx 0x2D) at 21,23
Johto_SafariZoneGate_EventScript_83F8856::
	lock
	faceplayer
	msgbox Johto_SafariZoneGate_Text_83F89AF, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_SafariZoneGate object 2 (local id 2, gfx 0x14) at 11,21
Johto_SafariZoneGate_EventScript_83F8BC0::
	lock
	faceplayer
	message Johto_SafariZoneGate_Text_83F8C1C
	waitmessage
	pokemart Johto_SafariZoneGate_Mart_834C8A0
	msgbox Quetzal_Text_8360D20, MSGBOX_DEFAULT
	release
	end

@ Johto_SafariZoneGate object 3 (local id 3, gfx 0x3EA7) at 20,21
Johto_SafariZoneGate_EventScript_83F8BD7::
	lock
	faceplayer
	message Johto_SafariZoneGate_Text_83F8C50
	waitmessage
	pokemart Johto_SafariZoneGate_Mart_834CADC
	msgbox Quetzal_Text_8360D20, MSGBOX_DEFAULT
	release
	end

@ Johto_SafariZoneGate object 1 (local id 1, gfx 0x18) at 20,15
Johto_SafariZoneGate_EventScript_83F8BEE::
	lock
	faceplayer
	message Johto_SafariZoneGate_Text_83F8C94
	waitmessage
	pokemart Johto_SafariZoneGate_Mart_834CC2C
	msgbox Quetzal_Text_8360D20, MSGBOX_DEFAULT
	release
	end

@ Johto_SafariZoneGate object 14 (local id 14, gfx 0x30) at 21,15
Johto_SafariZoneGate_EventScript_83F8C05::
	lock
	faceplayer
	message Johto_SafariZoneGate_Text_83F8CCA
	waitmessage
	pokemart Johto_SafariZoneGate_Mart_834CDC8
	msgbox Quetzal_Text_8360D20, MSGBOX_DEFAULT
	release
	end

Johto_SafariZoneGate_Mart_834C8A0:
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

Johto_SafariZoneGate_Mart_834CADC:
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

Johto_SafariZoneGate_Mart_834CC2C:
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
	.2byte ITEM_NONE

Johto_SafariZoneGate_Mart_834CDC8:
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

Johto_SafariZoneGate_Text_83F8863::
	.string "SAFARI ZONE...\n"
	.string "It makes me feel like I am in a\l"
	.string "foreign country.\p"
	.string "It brings up a strange feeling.$"

Johto_SafariZoneGate_Text_83F890A::
	.string "This SAFARI ZONE is great!\n"
	.string "I just kept going after POKéMON!\p"
	.string "That made my JIGGLYPUFF upset though.$"

Johto_SafariZoneGate_Text_83F896C::
	.string "I have always wanted to catch my first\n"
	.string "POKéMON at the SAFARI ZONE.$"

Johto_SafariZoneGate_Text_83F89AF::
	.string "There are many POKéMON to choose from,\n"
	.string "but nothing beats the cuteness of\l"
	.string "SLOWPOKE.\p"
	.string "People often say that I look like one!$"

Johto_SafariZoneGate_Text_83F8C1C::
	.string "We stock items that strengthen\n"
	.string "moves of every type.$"

Johto_SafariZoneGate_Text_83F8C50::
	.string "We stock weather and terrain items\n"
	.string "for different battle strategies.$"

Johto_SafariZoneGate_Text_83F8C94::
	.string "We stock classic held items for\n"
	.string "all kinds of battles.$"

Johto_SafariZoneGate_Text_83F8CCA::
	.string "We stock modern held items for\n"
	.string "all kinds of battles.$"
