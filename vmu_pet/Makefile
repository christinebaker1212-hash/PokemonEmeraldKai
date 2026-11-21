# Moemon VMU pet. `make` builds every program into build/ (vmu_pet/tools/get_tools.sh first, once).
WATERBEAR := .tools/waterbear-bin
PY        := python3

all: build/LINKTEST.VMS build/LINKTEST.VMI build/linktest.bin build/PET.VMS build/PET.VMI build/pet.bin build/pet_vm2.bin build/pet_old.bin

# The PICTURES file (the colour icons of hatched MOeMON: tools/gen_icons.py), from the PMD portraits
# (tools/fetch_portraits.sh, which needs the network once). Not in `all`: `make pictures`.
pictures: build/PICTURES.VMS build/PICTURES.VMI build/pet_pictures.bin

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
pet/kanto.inc: tools/gen_edition.py tools/eggs.py tools/onebit.py tools/pmd_read.py
	$(PY) tools/gen_edition.py kanto $@

pet/font.inc: build/font.inc
	cp $< $@

build/PET.VMS: pet/pet.s pet/icon.inc pet/battle.inc pet/link.inc pet/update.inc pet/kanto.inc pet/font.inc pet/icon.png inc/*.inc tools/check_layout.py
	@mkdir -p build
	cd pet && ../$(WATERBEAR) assemble --debug pet.s -o ../$@
	$(PY) tools/check_layout.py $@

build/PET.VMI: build/PET.VMS tools/vmufs.py
	$(PY) tools/vmufs.py vmi $< MOEKANTO $@ "MOEMON VMU PET - KANTO EDITION"

build/pet.bin: build/PET.VMS tools/vmufs.py
	$(PY) tools/vmufs.py make $@ --game $< --name MOEKANTO

# An "older version" of it (its version page and ABOUT changed): the UPDATE test's VMU
build/pet_old.bin: build/pet.bin
	$(PY) -c "d=bytearray(open('$<','rb').read()); d[0xBB08]=1; i=d.find(b'MOEMON KANTO\\x00', 0x1000); d[i:i+12]=b'OLD VERSION!'; open('$@','wb').write(d)"

# The same, marked as a VM2's (its LINK menu there): for the VM2's SD card, and the tests
build/pet_vm2.bin: build/pet.bin tools/vm2mark.py
	$(PY) tools/vm2mark.py $< $@

build/PICTURES.VMS: tools/gen_icons.py
	@mkdir -p build
	sh tools/fetch_portraits.sh
	$(PY) tools/gen_icons.py kanto $@

build/PICTURES.VMI: build/PICTURES.VMS tools/vmufs.py
	$(PY) tools/vmufs.py vmi $< MOEKANTO_PIC $@ "MOEMON VMU PET KANTO PICTURES" --data

# The pet and its PICTURES on one VMU image
build/pet_pictures.bin: build/PET.VMS build/PICTURES.VMS tools/vmufs.py
	$(PY) tools/vmufs.py make $@ --game build/PET.VMS --name MOEKANTO --data MOEKANTO_PIC=build/PICTURES.VMS

clean:
	rm -rf build linktest/font.inc pet/font.inc pet/kanto.inc

.PHONY: all clean pictures

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
