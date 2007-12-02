set(PROCESSORS 1)
set(HOST magrathea)
set(MAKE_PROGRAM "make")
set(CC gcc332s)
set(CXX c++332s)
set(INITIAL_CACHE "
CURSES_LIBRARY:FILEPATH=/usr/i686-gcc-332s/lib/libncurses.a
CURSES_INCLUDE_PATH:PATH=/usr/i686-gcc-332s/include/ncurses
FORM_LIBRARY:FILEPATH=/usr/i686-gcc-332s/lib/libform.a
CPACK_SYSTEM_NAME:STRING=Linux-i386
")
get_filename_component(path "${CMAKE_CURRENT_LIST_FILE}" PATH)
include(${path}/release_cmake.cmake)
