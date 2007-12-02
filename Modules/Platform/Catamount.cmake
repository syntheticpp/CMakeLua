#Catamount, which runs on the compute nodes of Cray machines, e.g. RedStorm, doesn't support shared libs 
SET_PROPERTIES(GLOBAL PROPERTIES TARGET_SUPPORTS_SHARED_LIBS FALSE)

SET(CMAKE_SHARED_LIBRARY_C_FLAGS "")            # -pic 
SET(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "")       # -shared
SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")         # +s, flag for exe link to use shared lib
SET(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG "")       # -rpath
SET(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG_SEP "")   # : or empty

SET(CMAKE_LINK_LIBRARY_SUFFIX "")
SET(CMAKE_STATIC_LIBRARY_PREFIX "lib")
SET(CMAKE_STATIC_LIBRARY_SUFFIX ".a")
SET(CMAKE_SHARED_LIBRARY_PREFIX "lib")          # lib
SET(CMAKE_SHARED_LIBRARY_SUFFIX ".a")           # .a
SET(CMAKE_EXECUTABLE_SUFFIX "")          # .exe
SET(CMAKE_DL_LIBS "" )

SET(CMAKE_FIND_LIBRARY_PREFIXES "lib")
SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")

INCLUDE(Platform/UnixPaths)

SET(CMAKE_CXX_LINK_SHARED_LIBRARY)
SET(CMAKE_CXX_LINK_MODULE_LIBRARY)
SET(CMAKE_C_LINK_SHARED_LIBRARY)
SET(CMAKE_C_LINK_MODULE_LIBRARY)
