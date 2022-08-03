cmake_minimum_required (VERSION 3.0.0)
project (nes-emu)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB srcs src/*.cpp)

add_executable (nes-emu ${srcs})
target_include_directories(nes-emu PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
