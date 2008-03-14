SET(CORE lapi.c lcode.c ldebug.c ldo.c ldump.c lfunc.c lgc.c llex.c 
  lmem.c lobject.c lopcodes.c lparser.c lstate.c lstring.c ltable.c 
  ltm.c  lundump.c lvm.c lzio.c)

SET (LIB lauxlib.c lbaselib.c ldblib.c liolib.c lmathlib.c loslib.c 
  ltablib.c lstrlib.c loadlib.c linit.c)

IF(APPLE)
    # Enable dyld for loading external libraries at runtime.
    SET(LUA_C_FLAGS "${LUA_C_FLAGS} -DLUA_USE_MACOSX=1")
ENDIF(APPLE)

IF(CMAKE_SYSTEM MATCHES "Linux.*")
  # enable dlopen() and mkstemp() support, etc.
  SET(LUA_C_FLAGS "${LUA_C_FLAGS} -DLUA_USE_LINUX=1")
  # Lua uses libm's math functions and libdl's dlopen.
  SET(LUA_LINKER_FLAGS "m;dl;${LUA_LINKER_FLAGS}")
ENDIF(CMAKE_SYSTEM MATCHES "Linux.*")

ADD_LIBRARY(lua ${LIB} ${CORE})
SET_TARGET_PROPERTIES(lua PROPERTIES COMPILE_FLAGS ${LUA_C_FLAGS})
ADD_EXECUTABLE(luaexec lua.c)
TARGET_LINK_LIBRARIES(luaexec lua ${LUA_LINKER_FLAGS})
ADD_EXECUTABLE(luac luac.c print.c)
TARGET_LINK_LIBRARIES(luac lua ${LUA_LINKER_FLAGS})

