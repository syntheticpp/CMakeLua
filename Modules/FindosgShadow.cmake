# This is part of the Findosg* suite used to find OpenSceneGraph components.
# Each component is separate and you must opt in to each module. You must 
# also opt into OpenGL and OpenThreads (and Producer if needed) as these 
# modules won't do it for you. This is to allow you control over your own 
# system piece by piece in case you need to opt out of certain components
# or change the Find behavior for a particular module (perhaps because the
# default FindOpenGL.cmake module doesn't work with your system as an
# example).
# If you want to use a more convenient module that includes everything,
# use the FindOpenSceneGraph.cmake instead of the Findosg*.cmake modules.
# 
# Locate osgShadow
# This module defines
# OSGSHADOW_LIBRARY
# OSGSHADOW_FOUND, if false, do not try to link to osgShadow
# OSGSHADOW_INCLUDE_DIR, where to find the headers
#
# $OSGDIR is an environment variable that would
# correspond to the ./configure --prefix=$OSGDIR
# used in building osg.
#
# Created by Eric Wing.

# Header files are presumed to be included like
# #include <osg/PositionAttitudeTransform>
# #include <osgShadow/ShadowTexture>

# Try the user's environment request before anything else.
FIND_PATH(OSGSHADOW_INCLUDE_DIR osgShadow/ShadowTexture
  PATHS
  $ENV{OSGSHADOW_DIR}
  $ENV{OSG_DIR}
  $ENV{OSGDIR}
  NO_DEFAULT_PATH
  PATH_SUFFIXES include
)
FIND_PATH(OSGSHADOW_INCLUDE_DIR osgShadow/ShadowTexture
  PATHS ${CMAKE_PREFIX_PATH} # Unofficial: We are proposing this.
  NO_DEFAULT_PATH
  PATH_SUFFIXES include
)
FIND_PATH(OSGSHADOW_INCLUDE_DIR osgShadow/ShadowTexture
  PATHS
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local
    /usr
    /sw # Fink
    /opt/local # DarwinPorts
    /opt/csw # Blastwave
    /opt
    [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OpenThreads_ROOT]
    [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]
  PATH_SUFFIXES include
)

FIND_LIBRARY(OSGSHADOW_LIBRARY 
  NAMES osgShadow
  PATHS
  $ENV{OSGSHADOW_DIR}
  $ENV{OSG_DIR}
  $ENV{OSGDIR}
  NO_DEFAULT_PATH
    PATH_SUFFIXES lib64 lib
)
FIND_LIBRARY(OSGSHADOW_LIBRARY 
  NAMES osgShadow
  PATHS ${CMAKE_PREFIX_PATH} # Unofficial: We are proposing this.
    NO_DEFAULT_PATH
    PATH_SUFFIXES lib64 lib
)
FIND_LIBRARY(OSGSHADOW_LIBRARY 
  NAMES osgShadow
  PATHS
    ~/Library/Frameworks
    /Library/Frameworks
  /usr/local
  /usr
  /sw
  /opt/local
  /opt/csw
  /opt
    PATH_SUFFIXES lib64 lib
)

SET(OSGSHADOW_FOUND "NO")
IF(OSGSHADOW_LIBRARY AND OSGSHADOW_INCLUDE_DIR)
  SET(OSGSHADOW_FOUND "YES")
ENDIF(OSGSHADOW_LIBRARY AND OSGSHADOW_INCLUDE_DIR)

