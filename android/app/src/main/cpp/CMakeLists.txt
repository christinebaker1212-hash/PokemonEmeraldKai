# The game's own C for the app through JNI: the phone link's protocol core (src/mp/phone_proto.c, Monocypher) and
# TRIPLE TRIAD's engine (src/tt: rules, AI, catalog, hands, ratings)
cmake_minimum_required(VERSION 3.22.1)
project(moemonlink C)
set(GAME ${CMAKE_CURRENT_SOURCE_DIR}/../../../../..)
set(GAME_MP ${GAME}/src/mp)
set(GAME_TT ${GAME}/src/tt)
# (the game's include/ has its own strings.h: an include folder that holds only tt/)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/ttinc)
file(CREATE_LINK ${GAME}/include/tt ${CMAKE_BINARY_DIR}/ttinc/tt SYMBOLIC)
add_library(moemonlink SHARED link_jni.c tt_jni.c ${GAME_MP}/phone_proto.c ${GAME_MP}/monocypher/monocypher.c
    ${GAME_TT}/tt_duel.c ${GAME_TT}/tt_ai.c ${GAME_TT}/tt_catalog.c ${GAME_TT}/tt_common.c ${GAME_TT}/tt_hands.c
    ${GAME_TT}/tt_pools.c ${GAME_TT}/tt_rating.c ${GAME_TT}/tt_ratings.c ${GAME_TT}/tt_collection.c)
target_include_directories(moemonlink PRIVATE ${GAME_MP} ${CMAKE_BINARY_DIR}/ttinc)
target_compile_options(moemonlink PRIVATE -O2 -Wall)
