# Map JSON data

# Inputs
MAPS_DIR = $(DATA_ASM_SUBDIR)/maps
LAYOUTS_DIR = $(DATA_ASM_SUBDIR)/layouts

# Outputs
MAPS_OUTDIR := $(MAPS_DIR)
LAYOUTS_OUTDIR := $(LAYOUTS_DIR)
INCLUDECONSTS_OUTDIR := include/constants

AUTO_GEN_TARGETS += $(INCLUDECONSTS_OUTDIR)/map_groups.h
AUTO_GEN_TARGETS += $(INCLUDECONSTS_OUTDIR)/layouts.h
AUTO_GEN_TARGETS += $(INCLUDECONSTS_OUTDIR)/map_event_ids.h
# Moemon Maker: the editor's names (maps, songs, people's looks) from this build's own constants (tools/maker/gen_names.py)
AUTO_GEN_TARGETS += $(INCLUDECONSTS_OUTDIR)/maker_names.h $(INCLUDECONSTS_OUTDIR)/maker_tilesets.h $(INCLUDECONSTS_OUTDIR)/maker_ops.h
AUTO_GEN_TARGETS += $(INCLUDECONSTS_OUTDIR)/maker_macros.h data/maker_globals.inc

MAP_DIRS := $(dir $(wildcard $(MAPS_DIR)/*/map.json))
MAP_CONNECTIONS := $(patsubst $(MAPS_DIR)/%/,$(MAPS_DIR)/%/connections.inc,$(MAP_DIRS))
MAP_EVENTS := $(patsubst $(MAPS_DIR)/%/,$(MAPS_DIR)/%/events.inc,$(MAP_DIRS))
MAP_HEADERS := $(patsubst $(MAPS_DIR)/%/,$(MAPS_DIR)/%/header.inc,$(MAP_DIRS))
MAP_JSONS := $(patsubst $(MAPS_DIR)/%/,$(MAPS_DIR)/%/map.json,$(MAP_DIRS))

$(DATA_ASM_BUILDDIR)/maps.o: $(DATA_ASM_SUBDIR)/maps.s $(LAYOUTS_DIR)/layouts.inc $(LAYOUTS_DIR)/layouts_table.inc $(MAPS_DIR)/headers.inc $(MAPS_DIR)/groups.inc $(MAPS_DIR)/connections.inc $(MAP_CONNECTIONS) $(MAP_HEADERS)
	$(PREPROC) $< charmap.txt | $(CPP) -I include - | $(PREPROC) -ie $< charmap.txt | $(ASM_PSEUDO_OP_CONV) - | $(AS) $(ASFLAGS) -o $@
	$(FIX_UNDERSCORE) $@
$(DATA_ASM_BUILDDIR)/map_events.o: $(DATA_ASM_SUBDIR)/map_events.s $(MAPS_DIR)/events.inc $(MAP_EVENTS)
	$(PREPROC) $< charmap.txt | $(CPP) -I include - | $(PREPROC) -ie $< charmap.txt | $(ASM_PSEUDO_OP_CONV) - | $(AS) $(ASFLAGS) -o $@
	$(FIX_UNDERSCORE) $@


# Moemon Kai: also remade when mapjson changes (the u16 map sections: stale headers read section 0)
$(MAPS_OUTDIR)/%/header.inc $(MAPS_OUTDIR)/%/events.inc $(MAPS_OUTDIR)/%/connections.inc: $(MAPS_DIR)/%/map.json $(MAPJSON)
	$(MAPJSON) map emerald $< $(LAYOUTS_DIR)/layouts.json $(@D)

$(MAPS_OUTDIR)/connections.inc $(MAPS_OUTDIR)/groups.inc $(MAPS_OUTDIR)/events.inc $(MAPS_OUTDIR)/headers.inc $(INCLUDECONSTS_OUTDIR)/map_groups.h: $(MAPS_DIR)/map_groups.json
	$(MAPJSON) groups emerald $< $(MAPS_OUTDIR) $(INCLUDECONSTS_OUTDIR)

$(LAYOUTS_OUTDIR)/layouts.inc $(LAYOUTS_OUTDIR)/layouts_table.inc $(INCLUDECONSTS_OUTDIR)/layouts.h: $(LAYOUTS_DIR)/layouts.json
	$(MAPJSON) layouts emerald $< $(LAYOUTS_OUTDIR) $(INCLUDECONSTS_OUTDIR)

# Generate constants for map events, which depend on data that's distributed across the map.json files.
# There's a lot of map.json files, so we print an abbreviated output with echo.
$(INCLUDECONSTS_OUTDIR)/map_event_ids.h: $(MAP_JSONS)
	@$(MAPJSON) event_constants emerald $^ $(INCLUDECONSTS_OUTDIR)/map_event_ids.h
	@echo "$(MAPJSON) event_constants emerald <MAP_JSONS> $(INCLUDECONSTS_OUTDIR)/map_event_ids.h"

# Moemon Maker: the editor's names, made again whenever the constants they come from change
$(INCLUDECONSTS_OUTDIR)/maker_names.h: tools/maker/gen_names.py $(INCLUDECONSTS_OUTDIR)/map_groups.h include/constants/songs.h include/constants/event_objects.h include/constants/qz_port.h include/constants/sinnoh_gfx.h include/mp.h include/constants/event_object_movement.h include/constants/trainer_types.h include/constants/items.h include/constants/species.h include/constants/flags.h include/constants/vars.h include/constants/opponents.h include/constants/region_map_sections.h include/constants/moves.h include/constants/pokemon.h include/constants/abilities.h data/specials.inc data/qz_specials.inc $(MAP_JSONS)
	python3 tools/maker/gen_names.py $@
	@touch $@
$(INCLUDECONSTS_OUTDIR)/maker_ops.h: tools/maker/gen_names.py data/script_cmd_table.inc
	python3 tools/maker/gen_names.py --ops $@
	@touch $@
# (M4b) the script macros, constants and global labels for the Maker's raw lines (src/maker/maker_raw.c)
$(INCLUDECONSTS_OUTDIR)/maker_macros.h: tools/maker/gen_macros.py tools/maker/gen_names.py asm/macros/event.inc asm/macros/qz_event.inc asm/macros/movement.inc asm/macros/map.inc asm/macros/battle_tent.inc asm/macros/trainer_hill.inc $(wildcard asm/macros/battle_frontier/*.inc) data/script_cmd_table.inc data/specials.inc $(filter-out data/maker_%,$(wildcard data/event_scripts*.s)) $(INCLUDECONSTS_OUTDIR)/map_groups.h $(INCLUDECONSTS_OUTDIR)/map_event_ids.h $(filter-out include/constants/maker_%.h,$(wildcard include/constants/*.h)) $(wildcard data/scripts/*.inc) $(wildcard data/scripts/*/*.inc) $(wildcard data/maps/*/scripts.inc)
	python3 tools/maker/gen_macros.py $@ $(INCLUDECONSTS_OUTDIR)/maker_labels.h data/maker_globals.inc
	@touch $@   # (maker_labels.h and data/maker_globals.inc are only rewritten when they change: no needless rebuilds)
$(INCLUDECONSTS_OUTDIR)/maker_labels.h: $(INCLUDECONSTS_OUTDIR)/maker_macros.h
data/maker_globals.inc: $(INCLUDECONSTS_OUTDIR)/maker_macros.h
$(INCLUDECONSTS_OUTDIR)/maker_tilesets.h: tools/maker/gen_names.py src/data/tilesets/headers.h src/data/tilesets/qz_headers.h
	python3 tools/maker/gen_names.py --tilesets $@
	@touch $@
