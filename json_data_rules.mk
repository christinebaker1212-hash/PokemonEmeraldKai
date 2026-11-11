# JSON files are run through jsonproc, which is a tool that converts JSON data to an output file
# based on an Inja template. https://github.com/pantor/inja

AUTO_GEN_TARGETS += $(DATA_SRC_SUBDIR)/wild_encounters.h
$(DATA_SRC_SUBDIR)/wild_encounters.h: $(DATA_SRC_SUBDIR)/wild_encounters.json $(DATA_SRC_SUBDIR)/wild_encounters.json.txt
	$(JSONPROC) $^ $@

$(C_BUILDDIR)/wild_encounter.o: c_dep += $(DATA_SRC_SUBDIR)/wild_encounters.h

AUTO_GEN_TARGETS += include/constants/wild_encounter.h
include/constants/wild_encounter.h: $(DATA_SRC_SUBDIR)/wild_encounters.json $(DATA_SRC_SUBDIR)/wild_encounters.constants.json.txt
	$(JSONPROC) $^ $@

AUTO_GEN_TARGETS += $(DATA_SRC_SUBDIR)/region_map/region_map_entries.h
$(DATA_SRC_SUBDIR)/region_map/region_map_entries.h: $(DATA_SRC_SUBDIR)/region_map/region_map_sections.json $(DATA_SRC_SUBDIR)/region_map/region_map_sections.json.txt
	$(JSONPROC) $^ $@

$(C_BUILDDIR)/region_map.o: c_dep += $(DATA_SRC_SUBDIR)/region_map/region_map_entries.h

AUTO_GEN_TARGETS += include/constants/region_map_sections.h
include/constants/region_map_sections.h: $(DATA_SRC_SUBDIR)/region_map/region_map_sections.json $(DATA_SRC_SUBDIR)/region_map/region_map_sections.constants.json.txt
	$(JSONPROC) $^ $@

AUTO_GEN_TARGETS += $(DATA_SRC_SUBDIR)/heal_locations.h
$(DATA_SRC_SUBDIR)/heal_locations.h: $(DATA_SRC_SUBDIR)/heal_locations.json $(DATA_SRC_SUBDIR)/heal_locations.json.txt
	$(JSONPROC) $^ $@

$(C_BUILDDIR)/heal_location.o: c_dep += $(DATA_SRC_SUBDIR)/heal_locations.h

AUTO_GEN_TARGETS += include/constants/heal_locations.h
include/constants/heal_locations.h: $(DATA_SRC_SUBDIR)/heal_locations.json $(DATA_SRC_SUBDIR)/heal_locations.constants.json.txt
	$(JSONPROC) $^ $@

# Moemon Co-op: the PATCH NOTES on the CO-OP setup screen, from the release notes (tools/release/notes/v<ver>.md)
PATCH_NOTES := $(wildcard tools/release/notes/v*.md)
AUTO_GEN_TARGETS += $(DATA_SRC_SUBDIR)/mp_patchnotes.h
$(DATA_SRC_SUBDIR)/mp_patchnotes.h: tools/release/gen_patchnotes.py $(PATCH_NOTES)
	python3 tools/release/gen_patchnotes.py $@ $(PATCH_NOTES)

$(C_BUILDDIR)/mp/mp_patchnotes.o: c_dep += $(DATA_SRC_SUBDIR)/mp_patchnotes.h

# Moemon Co-op: the PMD Collab sheets go in with .incbin (scaninc doesn't see it)
$(C_BUILDDIR)/mp/mp_pmd_data.o: graphics/moemon/pmd/pmd.bin
# Moemon Co-op: the BADGE CASE's badges, the same way
$(C_BUILDDIR)/mp/mp_badge_data.o: graphics/moemon/badges/badges.bin
$(C_BUILDDIR)/mp/mp_pass_data.o: graphics/moemon/cards/passes.bin
# Moemon Kai: the card game's Game Boy illustrations, the same way
$(C_BUILDDIR)/tcg/tcg_art_data.o: graphics/tcg/original.bin graphics/tcg/printed.bin
$(C_BUILDDIR)/ygo/ygo_art_data.o: graphics/ygo/printed.bin graphics/ygo/sdd_art.bin
$(C_BUILDDIR)/ygo/ygo_edo_pak.o: graphics/ygo/edo.pak
$(C_BUILDDIR)/mtg/mtg_pak.o: graphics/mtg/forge.pak
$(C_BUILDDIR)/mtg/mtg_art_data.o: graphics/mtg/printed.bin
$(C_BUILDDIR)/tt/tt_art_data.o: graphics/tt/cards.bin
# Moemon Rhythm: the DANCE machine's art and sounds
$(C_BUILDDIR)/rhythm/rh_assets.o: graphics/rhythm/rhythm.bin

# Moemon Co-op: how many maps each map group holds, for the phone's live map (src/mp/mp_phone_dev.c)
AUTO_GEN_TARGETS += $(DATA_SRC_SUBDIR)/mp_map_group_sizes.h
$(DATA_SRC_SUBDIR)/mp_map_group_sizes.h: tools/phone/gen_map_sizes.py data/maps/map_groups.json
	python3 tools/phone/gen_map_sizes.py $@ data/maps/map_groups.json
$(C_BUILDDIR)/mp/mp_phone_dev.o: c_dep += $(DATA_SRC_SUBDIR)/mp_map_group_sizes.h
$(C_BUILDDIR)/mp/mp_friends.o: c_dep += $(DATA_SRC_SUBDIR)/mp_map_group_sizes.h
