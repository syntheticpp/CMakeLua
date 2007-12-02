# - Locate OpenAL
# This module defines
#  OPENAL_LIBRARY
#  OPENAL_FOUND, if false, do not try to link to OpenAL 
#  OPENAL_INCLUDE_DIR, where to find the headers
#   
# $OPENALDIR is an environment variable that would
# correspond to the ./configure --prefix=$OPENALDIR
# used in building OpenAL.
#   
# Created by Eric Wing. This was influenced by the FindSDL.cmake module.
# On OSX, this will prefer the Framework version (if found) over others.
# People will have to manually change the cache values of 
# OPENAL_LIBRARY to override this selection.
# Tiger will include OpenAL as part of the System.
# But for now, we have to look around.
# Other (Unix) systems should be able to utilize the non-framework paths.
FIND_PATH(OPENAL_INCLUDE_DIR NAMES al.h
  PATH_SUFFIXES AL OpenAL
  PATHS 
  $ENV{OPENALDIR}/include
  ~/Library/Frameworks/OpenAL.framework/Headers
  /Library/Frameworks/OpenAL.framework/Headers
  /System/Library/Frameworks/OpenAL.framework/Headers # Tiger
  )

# I'm not sure if I should do a special casing for Apple. It is 
# unlikely that other Unix systems will find the framework path.
# But if they do ([Next|Open|GNU]Step?), 
# do they want the -framework option also?
IF(${OPENAL_INCLUDE_DIR} MATCHES ".framework")
  STRING(REGEX REPLACE "(.*)/.*\\.framework/.*" "\\1" OPENAL_FRAMEWORK_PATH_TMP ${OPENAL_INCLUDE_DIR})
  IF("${OPENAL_FRAMEWORK_PATH_TMP}" STREQUAL "/Library/Frameworks"
      OR "${OPENAL_FRAMEWORK_PATH_TMP}" STREQUAL "/System/Library/Frameworks"
      )
    # String is in default search path, don't need to use -F
    SET (OPENAL_LIBRARY "-framework OpenAL" CACHE STRING "OpenAL framework for OSX")
  ELSE("${OPENAL_FRAMEWORK_PATH_TMP}" STREQUAL "/Library/Frameworks"
      OR "${OPENAL_FRAMEWORK_PATH_TMP}" STREQUAL "/System/Library/Frameworks"
      )
    # String is not /Library/Frameworks, need to use -F
    SET(OPENAL_LIBRARY "-F${OPENAL_FRAMEWORK_PATH_TMP} -framework OpenAL" CACHE STRING "OpenAL framework for OSX")
  ENDIF("${OPENAL_FRAMEWORK_PATH_TMP}" STREQUAL "/Library/Frameworks"
    OR "${OPENAL_FRAMEWORK_PATH_TMP}" STREQUAL "/System/Library/Frameworks"
    )
  # Clear the temp variable so nobody can see it
  SET(OPENAL_FRAMEWORK_PATH_TMP "" CACHE INTERNAL "")

ELSE(${OPENAL_INCLUDE_DIR} MATCHES ".framework")
  FIND_LIBRARY(OPENAL_LIBRARY 
    NAMES openal al OpenAL32
    PATHS
    $ENV{OPENALDIR}/lib
    $ENV{OPENALDIR}/libs
    )
ENDIF(${OPENAL_INCLUDE_DIR} MATCHES ".framework")


# handle the QUIETLY and REQUIRED arguments and set OPENAL_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(OpenAL DEFAULT_MSG OPENAL_LIBRARY OPENAL_INCLUDE_DIR)
