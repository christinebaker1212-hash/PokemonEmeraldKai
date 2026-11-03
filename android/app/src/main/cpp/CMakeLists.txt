# The game's own C for the app through JNI: the phone link's protocol core (src/mp/phone_proto.c, Monocypher) and
# TRIPLE TRIAD's engine (src/tt: rules, AI, catalog, hands, ratings) and the POKéMON card game's (src/tcg: rules,
# effects, powers, AI, catalog, deck ratings, the duel board's choices in tcg_flow.c)
cmake_minimum_required(VERSION 3.22.1)
project(moemonlink C)
set(GAME ${CMAKE_CURRENT_SOURCE_DIR}/../../../../..)
set(GAME_MP ${GAME}/src/mp)
set(GAME_TT ${GAME}/src/tt)
set(GAME_TCG ${GAME}/src/tcg)
# (the game's include/ has its own strings.h: include folders that hold only tt/, and tcg/ with mp_version.h)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/ttinc)
file(CREATE_LINK ${GAME}/include/tt ${CMAKE_BINARY_DIR}/ttinc/tt SYMBOLIC)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/tcginc)
file(CREATE_LINK ${GAME}/include/tcg ${CMAKE_BINARY_DIR}/tcginc/tcg SYMBOLIC)
file(CREATE_LINK ${GAME}/include/mp_version.h ${CMAKE_BINARY_DIR}/tcginc/mp_version.h SYMBOLIC)
add_library(moemonlink SHARED link_jni.c tt_jni.c tcg_jni.c ${GAME_MP}/phone_proto.c ${GAME_MP}/monocypher/monocypher.c
    ${GAME_TT}/tt_duel.c ${GAME_TT}/tt_ai.c ${GAME_TT}/tt_catalog.c ${GAME_TT}/tt_common.c ${GAME_TT}/tt_hands.c
    ${GAME_TT}/tt_pools.c ${GAME_TT}/tt_rating.c ${GAME_TT}/tt_ratings.c ${GAME_TT}/tt_collection.c
    ${GAME_TCG}/tcg_duel.c ${GAME_TCG}/tcg_effects.c ${GAME_TCG}/tcg_trainers.c ${GAME_TCG}/tcg_powers.c ${GAME_TCG}/tcg_ai.c
    ${GAME_TCG}/tcg_catalog.c ${GAME_TCG}/tcg_collection.c ${GAME_TCG}/tcg_decks.c ${GAME_TCG}/tcg_scale.c
    ${GAME_TCG}/tcg_ratings.c ${GAME_TCG}/tcg_levels.c ${GAME_TCG}/tcg_flow.c ${GAME_TCG}/tcg_link.c)
target_include_directories(moemonlink PRIVATE ${GAME_MP} ${CMAKE_BINARY_DIR}/ttinc ${CMAKE_BINARY_DIR}/tcginc)
target_link_libraries(moemonlink m)
target_compile_options(moemonlink PRIVATE -O2 -Wall)
