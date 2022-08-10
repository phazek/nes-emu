cmake_minimum_required (VERSION 3.0.0)
project (nes-emu)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PNG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

file(GLOB srcs src/*.cpp)
file(GLOB nes_srcs src/nes/*.cpp src/nes/mappers/*.cpp)

add_executable (nes-emu ${nes_srcs} ${srcs})
target_link_libraries(nes-emu ${PNG_LIBRARIES} ${GLUT_LIBRARIES} ${OPENGL_LIBRARIES} )
target_include_directories(nes-emu PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include ${PNG_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS})
