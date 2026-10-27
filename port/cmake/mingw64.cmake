# Cross build for 64-bit Windows from Linux with clang and MinGW-w64's headers and libraries
# (Debian/Ubuntu: apt install clang lld mingw-w64), and SDL2's "SDL2-devel-2.x-mingw.tar.gz"
# from its GitHub releases:
#   cmake -S port -B build-win -G Ninja -DCMAKE_TOOLCHAIN_FILE=$PWD/port/cmake/mingw64.cmake \
#         -DSDL2_DIR=/path/to/SDL2-2.x/x86_64-w64-mingw32/lib/cmake/SDL2
#   cmake --build build-win
# Put SDL2.dll (SDL2-2.x/x86_64-w64-mingw32/bin) next to u4.exe.
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)
set(CMAKE_C_COMPILER clang)
set(CMAKE_C_COMPILER_TARGET x86_64-w64-mingw32)
set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)
set(CMAKE_SYSROOT /usr/x86_64-w64-mingw32)
# MinGW's libgcc (clang's driver asks for it): where its gcc keeps it
execute_process(COMMAND x86_64-w64-mingw32-gcc -print-libgcc-file-name
	OUTPUT_VARIABLE MINGW_LIBGCC OUTPUT_STRIP_TRAILING_WHITESPACE)
get_filename_component(MINGW_LIBGCC_DIR "${MINGW_LIBGCC}" DIRECTORY)
set(CMAKE_EXE_LINKER_FLAGS_INIT "-fuse-ld=lld -L${MINGW_LIBGCC_DIR}")
set(CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)
