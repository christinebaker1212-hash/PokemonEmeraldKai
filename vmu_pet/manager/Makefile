# Moemon VMU manager. `make` (Linux: libsdl2-dev), `make windows` (x86_64-w64-mingw32-g++ and SDL2's MinGW package:
# SDL2_MINGW, by default ../.tools/sdl2-mingw/SDL2-2.30.9/x86_64-w64-mingw32, the same SDL2 the game's release uses),
# `make test` (the self test and the VM2 protocol against a pretend VM2).
B        := ../build/manager
IMGUI    := ../../maker/imgui
SRCS     := main.cpp vmu.cpp vm2link.cpp pngio.cpp filedlg.cpp
IMGUI_SRCS := $(IMGUI)/imgui.cpp $(IMGUI)/imgui_draw.cpp $(IMGUI)/imgui_tables.cpp $(IMGUI)/imgui_widgets.cpp \
              $(IMGUI)/backends/imgui_impl_sdl2.cpp $(IMGUI)/backends/imgui_impl_sdlrenderer2.cpp $(IMGUI)/misc/imgui_stdlib.cpp
CXXFLAGS := -std=c++17 -O2 -Wall -Wno-unused-function -I$(B) -I$(IMGUI) -I$(IMGUI)/backends -I../../src/mp/third_party
FONT     := ../../maker/fonts/LiberationSans-Regular.ttf

SDL2_MINGW ?= ../.tools/sdl2-mingw/SDL2-2.30.9/x86_64-w64-mingw32
WINCXX   := x86_64-w64-mingw32-g++-posix

all: $(B)/vmu_manager $(B)/vm2cli

$(B)/font_data.h: $(FONT)
	@mkdir -p $(B)
	python3 -c "import sys;d=open(sys.argv[1],'rb').read();open(sys.argv[2],'w').write('// Liberation Sans (SIL OFL, maker/fonts/OFL.txt). Generated.\nstatic const unsigned char kFontData[]={'+','.join(map(str,d))+'};\n')" $< $@

$(B)/font_bold.h: ../../maker/fonts/LiberationSans-Bold.ttf
	@mkdir -p $(B)
	python3 -c "import sys;d=open(sys.argv[1],'rb').read();open(sys.argv[2],'w').write('// Liberation Sans Bold (SIL OFL). Generated.\nstatic const unsigned char kFontBold[]={'+','.join(map(str,d))+'};\n')" $< $@

$(B)/vmu_manager: $(SRCS) *.h $(B)/font_data.h $(B)/font_bold.h
	$(CXX) $(CXXFLAGS) $(shell pkg-config --cflags sdl2) $(SRCS) $(IMGUI_SRCS) -o $@ $(shell pkg-config --libs sdl2) -lpthread

$(B)/vm2cli: vm2cli.cpp vm2link.cpp vm2link.h
	@mkdir -p $(B)
	$(CXX) -std=c++17 -O2 -Wall vm2cli.cpp vm2link.cpp -o $@

# Windows: one folder with the .exe, SDL2.dll, the font's licence (zip it for the user)
windows: $(B)/win/vmu_manager.exe $(B)/win/vm2cli.exe

$(B)/win/vmu_manager.exe: $(SRCS) *.h $(B)/font_data.h $(B)/font_bold.h
	@mkdir -p $(B)/win
	$(WINCXX) $(CXXFLAGS) -I$(SDL2_MINGW)/include/SDL2 -Dmain=SDL_main $(SRCS) $(IMGUI_SRCS) -o $@ \
	  -L$(SDL2_MINGW)/lib -lmingw32 -lSDL2main -lSDL2 -lsetupapi -ladvapi32 -lole32 -luuid -lshell32 -mwindows -static-libgcc -static-libstdc++ -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive,-Bdynamic
	cp $(SDL2_MINGW)/bin/SDL2.dll $(B)/win/
	cp ../../maker/fonts/OFL.txt $(B)/win/FONT_LICENSE.txt
	cp README.txt $(B)/win/

$(B)/win/vm2cli.exe: vm2cli.cpp vm2link.cpp vm2link.h
	@mkdir -p $(B)/win
	$(WINCXX) -std=c++17 -O2 -Wall vm2cli.cpp vm2link.cpp -o $@ -lsetupapi -static -static-libgcc -static-libstdc++

test: all
	$(B)/vmu_manager --selftest
	sh test/vm2_test.sh

.PHONY: all windows test
