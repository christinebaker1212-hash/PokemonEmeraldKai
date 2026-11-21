# Moemon VMU pet. `make` builds every program into build/ (vmu_pet/tools/get_tools.sh first, once).
WATERBEAR := .tools/waterbear-bin
PY        := python3

all: build/LINKTEST.VMS build/LINKTEST.VMI build/linktest.bin build/PET.VMS build/PET.VMI build/pet.bin

build/font.inc: tools/gen_font.py
	@mkdir -p build
	$(PY) tools/gen_font.py $@

linktest/font.inc: build/font.inc
	cp $< $@

build/LINKTEST.VMS: linktest/linktest.s linktest/icon.png linktest/font.inc inc/*.inc
	@mkdir -p build
	cd linktest && ../$(WATERBEAR) assemble linktest.s -o ../$@

build/LINKTEST.VMI: build/LINKTEST.VMS tools/vmufs.py
	$(PY) tools/vmufs.py vmi $< LINKTEST $@ "MOEMON VMU LINK TEST"

# A whole VMU image holding only LINK TEST: the VM2's SD card takes it as is.
build/linktest.bin: build/LINKTEST.VMS tools/vmufs.py
	$(PY) tools/vmufs.py make $@ --game $< --name LINKTEST

# The pet, KANTO edition: its data from Kai's tables and PMD sprites (tools/gen_edition.py)
pet/kanto.inc: tools/gen_edition.py tools/onebit.py tools/pmd_read.py
	$(PY) tools/gen_edition.py kanto $@

pet/font.inc: build/font.inc
	cp $< $@

build/PET.VMS: pet/pet.s pet/kanto.inc pet/font.inc pet/icon.png inc/*.inc
	@mkdir -p build
	cd pet && ../$(WATERBEAR) assemble pet.s -o ../$@

build/PET.VMI: build/PET.VMS tools/vmufs.py
	$(PY) tools/vmufs.py vmi $< MOEKANTO $@ "MOEMON VMU PET - KANTO EDITION"

build/pet.bin: build/PET.VMS tools/vmufs.py
	$(PY) tools/vmufs.py make $@ --game $< --name MOEKANTO

clean:
	rm -rf build linktest/font.inc pet/font.inc pet/kanto.inc

.PHONY: all clean

# The test harness and the tests
build/font_table.h: tools/gen_font.py
	@mkdir -p build
	$(PY) tools/gen_font.py $@

build/vmutest: test/vmutest.c test/CMakeLists.txt build/font_table.h
	cmake -S test -B build/harness -DCMAKE_BUILD_TYPE=Release -DFONT_DIR=$(CURDIR)/build >/dev/null
	cmake --build build/harness -j4 --target vmutest >/dev/null
	cp build/harness/vmutest $@

test: all build/vmutest
	sh test/run.sh

.PHONY: test
