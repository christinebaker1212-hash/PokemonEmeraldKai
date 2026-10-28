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
# Moemon Kai: the card game's Game Boy illustrations, the same way
$(C_BUILDDIR)/tcg/tcg_art_data.o: graphics/tcg/original.bin graphics/tcg/printed.bin
$(C_BUILDDIR)/ygo/ygo_art_data.o: graphics/ygo/printed.bin graphics/ygo/sdd_art.bin
