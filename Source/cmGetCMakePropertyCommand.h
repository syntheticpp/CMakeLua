/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmGetCMakePropertyCommand.h,v $
  Language:  C++
  Date:      $Date: 2008-01-23 15:27:59 $
  Version:   $Revision: 1.8 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmGetCMakePropertyCommand_h
#define cmGetCMakePropertyCommand_h

#include "cmCommand.h"

class cmGetCMakePropertyCommand : public cmCommand
{
public:
  virtual cmCommand* Clone() 
    {
      return new cmGetCMakePropertyCommand;
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
  virtual const char* GetName() { return "get_cmake_property";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Get a property of the CMake instance.";
    }
  
  /**
   * Longer documentation.
   */
  virtual const char* GetFullDocumentation()
    {
      return
        "  get_cmake_property(VAR property)\n"
        "Get a property from the CMake instance.  The value of the "
        "property is stored in the variable VAR. If the property is "
        "not found, CMake will report an error. Some supported properties "
        "include: VARIABLES, CACHE_VARIABLES, COMMANDS, and MACROS.";
    }
  
  cmTypeMacro(cmGetCMakePropertyCommand, cmCommand);
};



#endif
