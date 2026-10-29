# Moemon Music Studio
#   make            Linux build  -> build/linux/moemon_music_studio   (needs libsdl2-dev, python3)
#   make windows    Windows exe  -> build/windows/MoemonMusicStudio.exe (x86_64-w64-mingw32-gcc;
#                   SDL2_DIR = the SDL2 mingw development package, default ../../SDL2)
#   make test       checks the converter against tools/mid2agb on every game MIDI
# The sound data comes from the game's sound folder at build time (gen_soundfont.py).

ROOT    := ../..
GEN     := build/gen
SRCS    := $(wildcard src/*.c)
CXXSRCS := $(wildcard src/*.cpp)
HDRS    := $(wildcard src/*.h)
TP      := third_party
PY      ?= python3
SDL2_DIR ?= $(ROOT)/SDL2

CFLAGS  ?= -O2
CFLAGS  += -std=c99 -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wno-format-truncation -Isrc -I$(GEN)
# the consoles' emulators (third_party/README.md): include paths for the files that talk to them
TPINC   := -I$(TP)/he -I$(TP)/psflib -I$(TP)/lazyusf2 -I$(TP)/zlib -DEMU_COMPILE -DEMU_LITTLE_ENDIAN
CXXINC  := -I$(TP)/desmume_2sf -I$(TP)/desmume_2sf/desmume -I$(TP)/desmume_2sf/spu -I$(TP) -I$(TP)/gme_gbs -I$(TP)/vbam_gsf -I$(TP)/compat \
           -I$(TP)/psflib -I$(TP)/zlib -DBLARGG_LITTLE_ENDIAN=1
CXXFLAGS ?= -O2
CXXFLAGS += -std=c++20 -w -Isrc

TP_HE   := bios iop ioptimer psx r3000 spu spucore vfs mkhebios
TP_USF  := ai/ai_controller api/callbacks debugger/dbg_decoder main/main main/rom main/savestates main/util \
           memory/memory pi/cart_rom pi/pi_controller r4300/cached_interp r4300/cp0 r4300/cp1 r4300/exception \
           r4300/interupt r4300/mi_controller r4300/pure_interp r4300/r4300 r4300/r4300_core r4300/recomp r4300/reset \
           r4300/tlb rdp/rdp_core ri/rdram ri/rdram_detection_hack ri/ri_controller rsp/rsp_core rsp_hle/alist \
           rsp_hle/alist_audio rsp_hle/alist_naudio rsp_hle/alist_nead rsp_hle/audio rsp_hle/cicx105 rsp_hle/hle \
           rsp_hle/hvqm rsp_hle/jpeg rsp_hle/memory rsp_hle/mp3 rsp_hle/musyx rsp_hle/plugin rsp_hle/re2 rsp_lle/rsp \
           si/cic si/game_controller si/n64_cic_nus_6105 si/pif si/si_controller usf/usf usf/barray usf/resampler \
           vi/vi_controller
TP_ZLIB := adler32 crc32 inffast inflate inftrees uncompr zutil
TP_GME  := Gbs_Emu Gb_Cpu Gb_Apu Gb_Oscs Blip_Buffer Classic_Emu Multi_Buffer Music_Emu Gme_File Data_Reader M3u_Playlist Effects_Buffer
TP_C    := $(TP_HE:%=he/%) psflib/psflib psflib/psf2fs $(TP_USF:%=lazyusf2/%) $(TP_ZLIB:%=zlib/%)
TP_CXX  := $(TP_GME:%=gme_gbs/%) \
           $(patsubst $(TP)/%.cpp,%,$(shell find $(TP)/desmume_2sf $(TP)/vbam_gsf -name '*.cpp'))

all: linux
linux: build/linux/moemon_music_studio
windows: build/windows/MoemonMusicStudio.exe

$(GEN)/stamp: gen_soundfont.py $(wildcard $(ROOT)/sound/voicegroups/*.inc) $(ROOT)/sound/voice_groups.inc
	$(PY) gen_soundfont.py $(GEN)
	touch $@
$(GEN)/sf_data.c $(GEN)/sf_counts.h $(GEN)/sf_pcm.bin: $(GEN)/stamp

# Linux
LOBJ := $(SRCS:src/%.c=build/linux/%.o) $(CXXSRCS:src/%.cpp=build/linux/%.o) build/linux/sf_data.o build/linux/sf_pcm.o \
        $(TP_C:%=build/linux/tp/%.o) $(TP_CXX:%=build/linux/tp/%.o)
build/linux/%.o: src/%.c $(HDRS) $(GEN)/sf_counts.h
	@mkdir -p build/linux
	$(CC) $(CFLAGS) $(TPINC) $(shell sdl2-config --cflags 2>/dev/null || echo -I/usr/include/SDL2 -D_REENTRANT) -c $< -o $@
build/linux/%.o: src/%.cpp $(HDRS)
	@mkdir -p build/linux
	$(CXX) $(CXXFLAGS) $(CXXINC) -c $< -o $@
build/linux/tp/%.o: $(TP)/%.c
	@mkdir -p $(dir $@)
	$(CC) -O2 -w $(TPINC) -I$(dir $<) -c $< -o $@
build/linux/tp/%.o: $(TP)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CXXINC) -I$(dir $<) -c $< -o $@
build/linux/tp/gme_gbs/%.o: $(TP)/gme_gbs/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CXXINC) -include $(TP)/gme_gbs/gme_rename.h -c $< -o $@
build/linux/rip_gbs.o: src/rip_gbs.cpp $(HDRS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CXXINC) -include $(TP)/gme_gbs/gme_rename.h -c $< -o $@
build/linux/sf_data.o: $(GEN)/sf_data.c src/sf.h
	@mkdir -p build/linux
	$(CC) $(CFLAGS) -O0 -c $< -o $@
build/linux/sf_pcm.o: src/sf_pcm.S $(GEN)/sf_pcm.bin src/fonts.bin
	@mkdir -p build/linux
	$(CC) -c -DSF_PCM_BIN='"$(GEN)/sf_pcm.bin"' -DFONTS_BIN='"src/fonts.bin"' $< -o $@
build/linux/moemon_music_studio: $(LOBJ)
	$(CXX) -o $@ $^ $(shell sdl2-config --libs 2>/dev/null || echo -lSDL2) -lm -lpthread

# Windows
WCC  := x86_64-w64-mingw32-gcc
WCXX := x86_64-w64-mingw32-g++
WSDL := $(SDL2_DIR)/x86_64-w64-mingw32
WOBJ := $(SRCS:src/%.c=build/windows/%.o) $(CXXSRCS:src/%.cpp=build/windows/%.o) build/windows/sf_data.o build/windows/sf_pcm.o \
        $(TP_C:%=build/windows/tp/%.o) $(TP_CXX:%=build/windows/tp/%.o)
build/windows/%.o: src/%.c $(HDRS) $(GEN)/sf_counts.h
	@mkdir -p build/windows
	$(WCC) $(CFLAGS) $(TPINC) -I$(WSDL)/include/SDL2 -I$(WSDL)/include -Dmain=SDL_main -c $< -o $@
build/windows/%.o: src/%.cpp $(HDRS)
	@mkdir -p build/windows
	$(WCXX) $(CXXFLAGS) $(CXXINC) -c $< -o $@
build/windows/tp/%.o: $(TP)/%.c
	@mkdir -p $(dir $@)
	$(WCC) -O2 -w $(TPINC) -I$(dir $<) -c $< -o $@
build/windows/tp/%.o: $(TP)/%.cpp
	@mkdir -p $(dir $@)
	$(WCXX) $(CXXFLAGS) $(CXXINC) -I$(dir $<) -c $< -o $@
build/windows/tp/gme_gbs/%.o: $(TP)/gme_gbs/%.cpp
	@mkdir -p $(dir $@)
	$(WCXX) $(CXXFLAGS) $(CXXINC) -include $(TP)/gme_gbs/gme_rename.h -c $< -o $@
build/windows/rip_gbs.o: src/rip_gbs.cpp $(HDRS)
	@mkdir -p $(dir $@)
	$(WCXX) $(CXXFLAGS) $(CXXINC) -include $(TP)/gme_gbs/gme_rename.h -c $< -o $@
build/windows/sf_data.o: $(GEN)/sf_data.c src/sf.h
	@mkdir -p build/windows
	$(WCC) $(CFLAGS) -O0 -c $< -o $@
build/windows/sf_pcm.o: src/sf_pcm.S $(GEN)/sf_pcm.bin src/fonts.bin
	@mkdir -p build/windows
	$(WCC) -c -DSF_PCM_BIN='"$(GEN)/sf_pcm.bin"' -DFONTS_BIN='"src/fonts.bin"' $< -o $@
build/windows/MoemonMusicStudio.exe: $(WOBJ)
	$(WCXX) -o $@ $^ -L$(WSDL)/lib -lmingw32 -lSDL2main -lSDL2 -lcomdlg32 -lole32 -lshell32 -mwindows -static-libgcc -static-libstdc++ -Wl,-Bstatic -lwinpthread -Wl,-Bdynamic
	cp $(WSDL)/bin/SDL2.dll build/windows/
	cp $$(dirname $$($(WCC) -print-file-name=libwinpthread.a))/../bin/libwinpthread-1.dll build/windows/ 2>/dev/null || \
	    cp /usr/x86_64-w64-mingw32/lib/libwinpthread-1.dll build/windows/

test: build/linux/moemon_music_studio
	test/run_tests.sh build/linux/moemon_music_studio

clean:
	rm -rf build

.PHONY: all linux windows test clean
