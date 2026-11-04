# The game's own C for the app through JNI: the phone link's protocol core (src/mp/phone_proto.c, Monocypher) and
# TRIPLE TRIAD's engine (src/tt: rules, AI, catalog, hands, ratings) and the POKéMON card game's (src/tcg: rules,
# effects, powers, AI, catalog, deck ratings, the duel board's choices in tcg_flow.c), and YU-GI-OH!'s (EDOPro's ocgcore
# with Lua as C++, the game's bridge src/ygo; the engine's pack is an asset handed over at run time)
cmake_minimum_required(VERSION 3.22.1)
project(moemonlink C CXX)
set(GAME ${CMAKE_CURRENT_SOURCE_DIR}/../../../../..)
set(GAME_MP ${GAME}/src/mp)
set(GAME_TT ${GAME}/src/tt)
set(GAME_TCG ${GAME}/src/tcg)
set(GAME_YGO ${GAME}/src/ygo)
set(EDO ${GAME}/edopro/ocgcore)
# (the game's include/ has its own strings.h: include folders that hold only tt/, and tcg/ with mp_version.h)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/ttinc)
file(CREATE_LINK ${GAME}/include/tt ${CMAKE_BINARY_DIR}/ttinc/tt SYMBOLIC)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/tcginc)
file(CREATE_LINK ${GAME}/include/tcg ${CMAKE_BINARY_DIR}/tcginc/tcg SYMBOLIC)
file(CREATE_LINK ${GAME}/include/mp_version.h ${CMAKE_BINARY_DIR}/tcginc/mp_version.h SYMBOLIC)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/tcginc/constants)
file(CREATE_LINK ${GAME}/include/constants/songs.h ${CMAKE_BINARY_DIR}/tcginc/constants/songs.h SYMBOLIC)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/ygoinc)
file(CREATE_LINK ${GAME}/include/ygo ${CMAKE_BINARY_DIR}/ygoinc/ygo SYMBOLIC)

# EDOPro's engine (the game's Makefile EDO_*): its C++ and its Lua's .c files, all as C++
file(GLOB EDO_CPP ${EDO}/*.cpp)
file(GLOB EDO_LUA ${EDO}/lua/src/*.c)
foreach(skip lbitlib lcorolib ldblib linit loadlib loslib ltests lua luac lutf8lib onelua)
    list(REMOVE_ITEM EDO_LUA ${EDO}/lua/src/${skip}.c)
endforeach()
set_source_files_properties(${EDO_LUA} PROPERTIES LANGUAGE CXX)
add_library(ocgcore STATIC ${EDO_CPP} ${EDO_LUA})
target_include_directories(ocgcore PRIVATE ${EDO}/lua/src)
target_compile_options(ocgcore PRIVATE -std=c++17 -O2 -fno-rtti -w -include ${EDO}/lua/luaconf-customize.h)
target_compile_definitions(ocgcore PRIVATE luaP_opmodes=edo_luaP_opmodes lua_ident=edo_lua_ident
    luaT_typenames_=edo_luaT_typenames_ luai_ctype_=edo_luai_ctype_)
add_library(moemonlink SHARED link_jni.c tt_jni.c tcg_jni.c ygo_jni.c ${GAME_MP}/phone_proto.c ${GAME_MP}/monocypher/monocypher.c
    ${GAME_TT}/tt_duel.c ${GAME_TT}/tt_ai.c ${GAME_TT}/tt_catalog.c ${GAME_TT}/tt_common.c ${GAME_TT}/tt_hands.c
    ${GAME_TT}/tt_pools.c ${GAME_TT}/tt_rating.c ${GAME_TT}/tt_ratings.c ${GAME_TT}/tt_collection.c
    ${GAME_TCG}/tcg_duel.c ${GAME_TCG}/tcg_effects.c ${GAME_TCG}/tcg_trainers.c ${GAME_TCG}/tcg_powers.c ${GAME_TCG}/tcg_ai.c
    ${GAME_TCG}/tcg_catalog.c ${GAME_TCG}/tcg_collection.c ${GAME_TCG}/tcg_decks.c ${GAME_TCG}/tcg_scale.c
    ${GAME_TCG}/tcg_ratings.c ${GAME_TCG}/tcg_levels.c ${GAME_TCG}/tcg_flow.c ${GAME_TCG}/tcg_link.c
    ${GAME_YGO}/ygo_catalog.c ${GAME_YGO}/ygo_fusions.c ${GAME_YGO}/ygo_edo.c ${GAME_YGO}/ygo_edo_data.c
    ${GAME_YGO}/ygo_edo_codes.c ${GAME_YGO}/ygo_ai.c ${GAME_YGO}/ygo_collection.c ${GAME_YGO}/ygo_rating.c
    ${GAME_YGO}/ygo_npc_decks.c ${GAME_YGO}/ygo_card_ratings.c ${GAME}/tools/ygo/stb_zlib.c)
target_include_directories(moemonlink PRIVATE ${GAME_MP} ${CMAKE_BINARY_DIR}/ttinc ${CMAKE_BINARY_DIR}/tcginc
    ${CMAKE_BINARY_DIR}/ygoinc ${GAME_YGO})
target_compile_definitions(moemonlink PRIVATE YGO_EDO_PAK_EXTERNAL)
target_link_libraries(moemonlink ocgcore m)
target_compile_options(moemonlink PRIVATE -O2 -Wall)
