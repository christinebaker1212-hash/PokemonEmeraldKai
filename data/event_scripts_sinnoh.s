@ Moemon Kai: SINNOH's map scripts (docs/SINNOH_HANDOFF.md; the maps from tools/sinnoh/gen_maps.py)
#include "config.h"
#include "constants/global.h"
#include "constants/event_objects.h"
#include "constants/flags.h"
#include "constants/vars.h"
	.include "asm/macros.inc"
	.include "constants/constants.inc"

	.include "asm/macros/event.inc"

	.section script_data, "aw"

	.include "data/maps/Sinnoh_TwinleafTown/scripts.inc"
	.include "data/maps/Sinnoh_Route201/scripts.inc"
