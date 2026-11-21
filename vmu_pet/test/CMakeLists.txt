# Moemon VMU pet: the two-VMU test harness on libevmu (vmu_pet/tools/get_tools.sh fetches libevmu).
cmake_minimum_required(VERSION 3.10)
project(vmutest C CXX)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../.tools/libevmu ${CMAKE_BINARY_DIR}/libevmu EXCLUDE_FROM_ALL)
add_executable(vmutest vmutest.c)
target_include_directories(vmutest PRIVATE ${FONT_DIR})
target_link_libraries(vmutest libLibElysianVMU m pthread)
