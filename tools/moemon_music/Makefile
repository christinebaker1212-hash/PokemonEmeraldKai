# Moemon Music Studio
#   make            Linux build  -> build/linux/moemon_music_studio   (needs libsdl2-dev, python3)
#   make windows    Windows exe  -> build/windows/MoemonMusicStudio.exe (x86_64-w64-mingw32-gcc;
#                   SDL2_DIR = the SDL2 mingw development package, default ../../SDL2)
#   make test       checks the converter against tools/mid2agb on every game MIDI
# The sound data comes from the game's sound folder at build time (gen_soundfont.py).

ROOT    := ../..
GEN     := build/gen
SRCS    := $(wildcard src/*.c)
HDRS    := $(wildcard src/*.h)
PY      ?= python3
SDL2_DIR ?= $(ROOT)/SDL2

CFLAGS  ?= -O2
CFLAGS  += -std=c99 -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wno-format-truncation -Isrc -I$(GEN)

all: linux
linux: build/linux/moemon_music_studio
windows: build/windows/MoemonMusicStudio.exe

$(GEN)/stamp: gen_soundfont.py $(wildcard $(ROOT)/sound/voicegroups/*.inc) $(ROOT)/sound/voice_groups.inc
	$(PY) gen_soundfont.py $(GEN)
	touch $@
$(GEN)/sf_data.c $(GEN)/sf_counts.h $(GEN)/sf_pcm.bin: $(GEN)/stamp

# Linux
LOBJ := $(SRCS:src/%.c=build/linux/%.o) build/linux/sf_data.o build/linux/sf_pcm.o
build/linux/%.o: src/%.c $(HDRS) $(GEN)/sf_counts.h
	@mkdir -p build/linux
	$(CC) $(CFLAGS) $(shell sdl2-config --cflags 2>/dev/null || echo -I/usr/include/SDL2 -D_REENTRANT) -c $< -o $@
build/linux/sf_data.o: $(GEN)/sf_data.c src/sf.h
	@mkdir -p build/linux
	$(CC) $(CFLAGS) -O0 -c $< -o $@
build/linux/sf_pcm.o: src/sf_pcm.S $(GEN)/sf_pcm.bin
	@mkdir -p build/linux
	$(CC) -c -DSF_PCM_BIN='"$(GEN)/sf_pcm.bin"' $< -o $@
build/linux/moemon_music_studio: $(LOBJ)
	$(CC) -o $@ $^ $(shell sdl2-config --libs 2>/dev/null || echo -lSDL2) -lm

# Windows
WCC  := x86_64-w64-mingw32-gcc
WSDL := $(SDL2_DIR)/x86_64-w64-mingw32
WOBJ := $(SRCS:src/%.c=build/windows/%.o) build/windows/sf_data.o build/windows/sf_pcm.o
build/windows/%.o: src/%.c $(HDRS) $(GEN)/sf_counts.h
	@mkdir -p build/windows
	$(WCC) $(CFLAGS) -I$(WSDL)/include/SDL2 -I$(WSDL)/include -Dmain=SDL_main -c $< -o $@
build/windows/sf_data.o: $(GEN)/sf_data.c src/sf.h
	@mkdir -p build/windows
	$(WCC) $(CFLAGS) -O0 -c $< -o $@
build/windows/sf_pcm.o: src/sf_pcm.S $(GEN)/sf_pcm.bin
	@mkdir -p build/windows
	$(WCC) -c -DSF_PCM_BIN='"$(GEN)/sf_pcm.bin"' $< -o $@
build/windows/MoemonMusicStudio.exe: $(WOBJ)
	$(WCC) -o $@ $^ -L$(WSDL)/lib -lmingw32 -lSDL2main -lSDL2 -lcomdlg32 -lole32 -lshell32 -mwindows -static-libgcc
	cp $(WSDL)/bin/SDL2.dll build/windows/

test: build/linux/moemon_music_studio
	test/run_tests.sh build/linux/moemon_music_studio

clean:
	rm -rf build

.PHONY: all linux windows test clean
