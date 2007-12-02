# - Find tclsh
# This module finds if TCL is installed and determines where the
# include files and libraries are. It also determines what the name of
# the library is. This code sets the following variables:
#  TCLSH_FOUND = TRUE if tclsh has been found
#  TCL_TCLSH = the path to the tclsh executable
# In cygwin, look for the cygwin version first.  Don't look for it later to
# avoid finding the cygwin version on a Win32 build.

IF(WIN32 AND UNIX)
  FIND_PROGRAM(TCL_TCLSH NAMES cygtclsh83 cygtclsh80)
ENDIF(WIN32 AND UNIX)

GET_FILENAME_COMPONENT(TCL_LIBRARY_PATH "${TCL_LIBRARY}" PATH)
GET_FILENAME_COMPONENT(TK_LIBRARY_PATH "${TK_LIBRARY}" PATH)

FIND_PROGRAM(TCL_TCLSH
  NAMES tclsh
  tclsh84 tclsh8.4
  tclsh83 tclsh8.3
  tclsh82 tclsh8.2
  tclsh80 tclsh8.0
  PATHS
  "${TCL_INCLUDE_PATH}/../bin"
  "${TK_INCLUDE_PATH}/../bin"
  "${TCL_LIBRARY_PATH}/../bin"
  "${TK_LIBRARY_PATH}/../bin"
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\ActiveState\\ActiveTcl\\8.4.6.0]/bin
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.4;Root]/bin
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.3;Root]/bin
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.2;Root]/bin
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.0;Root]/bin
)

# handle the QUIETLY and REQUIRED arguments and set TIFF_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Tclsh DEFAULT_MSG TCL_TCLSH)

MARK_AS_ADVANCED( TCL_TCLSH  )
