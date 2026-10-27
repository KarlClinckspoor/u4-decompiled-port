# Ultima IV, native port: the game's C from ../SRC (AVATAR.EXE) and ../SRC-TITLE (TITLE.EXE),
# shared with the DOS build, and the port's own C in this folder, on SDL2.
#
#   cmake -S port -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
#   cmake --build build
#
# The game's C must be compiled by clang (MSYS2's CLANG64, or clang on Linux/macOS, or a
# cross build from Linux with port/cmake/mingw64.cmake): it relies on clang evaluating a
# call's arguments left to right, as Microsoft C 4.00's pascal calls did (port/sys/selftest.c
# checks it at start).
cmake_minimum_required(VERSION 3.16)
project(u4port C)

set(CMAKE_C_STANDARD 99)
set(GAME_SRC ${CMAKE_CURRENT_SOURCE_DIR}/../SRC)
set(TITLE_SRC ${CMAKE_CURRENT_SOURCE_DIR}/../SRC-TITLE)
set(INC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(NOT CMAKE_C_COMPILER_ID MATCHES "Clang")
	message(WARNING "the game's C should be built with clang (argument evaluation order); "
		"set CMAKE_C_COMPILER=clang")
endif()
if(MSVC AND NOT CMAKE_C_COMPILER_ID MATCHES "Clang")
	message(FATAL_ERROR "build with clang (MinGW/MSYS2 CLANG64): MSVC's cl does not take the game's C89")
endif()

find_package(SDL2 CONFIG QUIET)
if(NOT SDL2_FOUND)
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)
	set(SDL2_TARGET PkgConfig::SDL2)
else()
	if(TARGET SDL2::SDL2)
		set(SDL2_TARGET SDL2::SDL2)
	else()
		set(SDL2_TARGET ${SDL2_LIBRARIES})
		include_directories(${SDL2_INCLUDE_DIRS})
	endif()
endif()

# the locals the game reads before setting them hold whatever DOS's stack held; the port's
# hold 0, so that every build and every host plays the same game (a recording replays)
set(GAME_FLAGS -std=gnu89 -fsigned-char -fwrapv -fno-strict-aliasing -Wno-everything
	-ftrivial-auto-var-init=zero)
if(NOT CMAKE_C_COMPILER_ID MATCHES "Clang")
	list(APPEND GAME_FLAGS -w -fpermissive)
endif()

# ---- AVATAR.EXE's C, compiled as Microsoft C 4.00 saw it ----
file(GLOB GAME_C ${GAME_SRC}/*.C ${GAME_SRC}/*.c)
# the upper-case .C files are C (not C++), whatever the host's file system says
set_source_files_properties(${GAME_C} PROPERTIES LANGUAGE C COMPILE_OPTIONS "-xc")
add_library(u4game OBJECT ${GAME_C} game/digest.c game/layout.c game/block_u4_combc.c)
target_include_directories(u4game PRIVATE ${INC}/avatar ${INC}/msc ${INC})
target_compile_options(u4game PRIVATE ${GAME_FLAGS} "SHELL:-include ${INC}/compat.h")
foreach(f ${GAME_C})
	get_filename_component(n ${f} NAME_WE)
	string(TOUPPER ${n} N)
	set_property(SOURCE ${f} APPEND PROPERTY COMPILE_DEFINITIONS PORT_TU_${N})
endforeach()
# the files SRC/MAKEFILE compiles with /J (char is unsigned)
foreach(n U4_USE U4_FILE U4_MAIN U4_MAIN2 U4_MAP U4_END U4_ANIM U4_COMBA U4_SPELL U4_Q_N_V U4_COMBB)
	set_property(SOURCE ${GAME_SRC}/${n}.C APPEND PROPERTY COMPILE_OPTIONS -funsigned-char)
endforeach()
# the sections laid out as in DOS (port/tools/gen_block.py)
set_property(SOURCE ${GAME_SRC}/U4_COMBC.C APPEND PROPERTY COMPILE_DEFINITIONS PORT_BLOCK)

# ---- TITLE.EXE's C, compiled the same way, its names renamed T_* (include/title/trename.h)
# so that both programs live in one executable. DATA.C stands for DATA.ASM, which the DOS
# build assembles from the same data. ----
set(TITLE_C ${TITLE_SRC}/TITLE_0.C ${TITLE_SRC}/TITLE_1.C ${TITLE_SRC}/FILE.C ${TITLE_SRC}/DATA.C
	${TITLE_SRC}/ERRMSG.C ${TITLE_SRC}/TITLE_X.C)
set_source_files_properties(${TITLE_C} PROPERTIES LANGUAGE C COMPILE_OPTIONS "-xc")
add_library(u4title OBJECT ${TITLE_C} game/block_data.c)
set_property(SOURCE ${TITLE_SRC}/DATA.C APPEND PROPERTY COMPILE_DEFINITIONS PORT_BLOCK)
target_include_directories(u4title PRIVATE ${INC}/title ${INC}/msc ${INC})
target_compile_options(u4title PRIVATE ${GAME_FLAGS} "SHELL:-include ${INC}/compat.h"
	"SHELL:-include ${INC}/title/trename.h")

# ---- the port's replacements of the assembly modules, built for each program ----
set(RUNTIME_TWICE sys/util2.c sys/low.c)
add_library(u4rt_title OBJECT ${RUNTIME_TWICE} sys/tgraph.c gfx/tgfx.c)
target_include_directories(u4rt_title PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/sys)
target_compile_definitions(u4rt_title PRIVATE PORT_TITLE PORT_TITLE_RUNTIME)
target_compile_options(u4rt_title PRIVATE -fsigned-char -fwrapv -fno-strict-aliasing
	"SHELL:-include ${INC}/title/trename.h")

# ---- the port ----
file(GLOB PORT_C sys/*.c gfx/ega.c replay/*.c ui/*.c platform/hostfs.c platform/sdl2/plat_sdl2.c)
list(REMOVE_ITEM PORT_C ${CMAKE_CURRENT_SOURCE_DIR}/sys/tgraph.c)
add_executable(u4 ${PORT_C} $<TARGET_OBJECTS:u4game> $<TARGET_OBJECTS:u4title> $<TARGET_OBJECTS:u4rt_title>)
target_include_directories(u4 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/platform)
target_compile_options(u4 PRIVATE -fsigned-char -fwrapv -fno-strict-aliasing)
target_link_libraries(u4 PRIVATE ${SDL2_TARGET} m)
if(WIN32)
	target_link_options(u4 PRIVATE -mwindows)
	target_link_libraries(u4 PRIVATE ole32 shell32)
endif()
if(NOT WIN32)
	target_compile_definitions(u4 PRIVATE _POSIX_C_SOURCE=200809L _DEFAULT_SOURCE)
endif()
