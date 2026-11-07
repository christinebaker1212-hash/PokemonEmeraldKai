@ Moemon Kai: ORRE's map scripts (docs/ORRE_DECISIONS.md; the maps from tools/orre/gen_maps.py)
#include "constants/global.h"
#include "constants/flags.h"
#include "constants/vars.h"
#include "constants/comparison_operators.h"
	.include "asm/macros.inc"
	.include "constants/constants.inc"
	.include "asm/macros/event.inc"
	.section script_data, "aw"
	.include "data/scripts/orre.inc"
	.include "data/maps/Orre_OutskirtStand/scripts.inc"
	.include "data/maps/Orre_OutskirtStandDiner/scripts.inc"

	@ Moemon Maker M4b: the file-local labels (texts, movements) made global, for the Maker's raw lines (generated)
	.include "data/maker_globals.inc"
