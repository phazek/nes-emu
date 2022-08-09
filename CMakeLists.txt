cmake_minimum_required (VERSION 3.0.0)
project (nes-emu)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB srcs src/*.cpp)
file(GLOB nes_srcs src/nes/*.cpp src/nes/mappers/*.cpp)

add_executable (nes-emu ${nes_srcs} ${srcs})
target_include_directories(nes-emu PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
