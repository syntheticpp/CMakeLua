SET(CORE lapi.c lcode.c ldebug.c ldo.c ldump.c lfunc.c lgc.c llex.c 
  lmem.c lobject.c lopcodes.c lparser.c lstate.c lstring.c ltable.c 
  ltm.c  lundump.c lvm.c lzio.c)

SET (LIB lauxlib.c lbaselib.c ldblib.c liolib.c lmathlib.c loslib.c 
  ltablib.c lstrlib.c loadlib.c linit.c)

ADD_LIBRARY(lua ${LIB} ${CORE})
ADD_EXECUTABLE(luaexec lua.c)
TARGET_LINK_LIBRARIES(luaexec lua) 
ADD_EXECUTABLE(luac luac.c print.c)
TARGET_LINK_LIBRARIES(luac lua) 
