# The phone link's protocol core, the game's own C (src/mp/phone_proto.c, Monocypher), for the app through JNI
cmake_minimum_required(VERSION 3.22.1)
project(moemonlink C)
set(GAME_MP ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../src/mp)
add_library(moemonlink SHARED link_jni.c ${GAME_MP}/phone_proto.c ${GAME_MP}/monocypher/monocypher.c)
target_include_directories(moemonlink PRIVATE ${GAME_MP})
target_compile_options(moemonlink PRIVATE -O2 -Wall)
