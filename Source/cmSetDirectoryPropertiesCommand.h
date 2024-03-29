/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmSetDirectoryPropertiesCommand.h,v $
  Language:  C++
  Date:      $Date: 2008/01/23 15:27:59 $
  Version:   $Revision: 1.7 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmSetDirectoryPropertiesCommand_h
#define cmSetDirectoryPropertiesCommand_h

#include "cmCommand.h"

class cmSetDirectoryPropertiesCommand : public cmCommand
{
public:
  virtual cmCommand* Clone() 
    {
      return new cmSetDirectoryPropertiesCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the input file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);

  /**
   * This determines if the command is invoked when in script mode.
   */
  virtual bool IsScriptable() { return true; }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "set_directory_properties";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Set a property of the directory.";
    }
  
  /**
   * Static entry point for use by other commands
   */
  static bool RunCommand(cmMakefile *mf,
                         std::vector<std::string>::const_iterator ait,
                         std::vector<std::string>::const_iterator aitend,
                         std::string &errors);

  /**
   * Longer documentation.
   */
  virtual const char* GetFullDocumentation()
    {
      return
        "  set_directory_properties(PROPERTIES prop1 value1 prop2 value2)\n"
        "Set a property for the current directory and subdirectories. If the "
        "property is not found, CMake will report an error. The properties "
        "include: INCLUDE_DIRECTORIES, LINK_DIRECTORIES, "
        "INCLUDE_REGULAR_EXPRESSION, and ADDITIONAL_MAKE_CLEAN_FILES.\n"
        "ADDITIONAL_MAKE_CLEAN_FILES is a list of files that will be cleaned "
        "as a part of \"make clean\" stage.";
    }
  
  cmTypeMacro(cmSetDirectoryPropertiesCommand, cmCommand);
};



#endif
