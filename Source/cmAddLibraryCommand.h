/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmAddLibraryCommand.h,v $
  Language:  C++
  Date:      $Date: 2008/01/23 15:27:59 $
  Version:   $Revision: 1.20 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmLibrarysCommand_h
#define cmLibrarysCommand_h

#include "cmCommand.h"

/** \class cmLibrarysCommand
 * \brief Defines a list of executables to build.
 *
 * cmLibrarysCommand defines a list of executable (i.e., test)
 * programs to create.
 */
class cmAddLibraryCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmAddLibraryCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "add_library";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Add a library to the project using the specified source files.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  add_library(libname [SHARED | STATIC | MODULE] [EXCLUDE_FROM_ALL]\n"
      "              source1 source2 ... sourceN)\n"
      "Adds a library target.  SHARED, STATIC or MODULE keywords are used "
      "to set the library type.  If the keyword MODULE appears, the library "
      "type is set to MH_BUNDLE on systems which use dyld.  On systems "
      "without dyld, MODULE is treated like SHARED.  If no keywords appear "
      " as the second argument, the type defaults to the current value of "
      "BUILD_SHARED_LIBS.  If this variable is not set, the type defaults "
      "to STATIC.\n"
      "If EXCLUDE_FROM_ALL is given the target will not be built by default. "
      "It will be built only if the user explicitly builds the target or "
      "another target that requires the target depends on it.";
    }
  
  cmTypeMacro(cmAddLibraryCommand, cmCommand);
};


#endif
