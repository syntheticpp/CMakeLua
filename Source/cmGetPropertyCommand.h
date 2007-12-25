/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmGetPropertyCommand.h,v $
  Language:  C++
  Date:      $Date: 2007/10/24 18:43:10 $
  Version:   $Revision: 1.4 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmGetPropertyCommand_h
#define cmGetPropertyCommand_h

#include "cmCommand.h"

class cmGetPropertyCommand : public cmCommand
{
public:
  cmGetPropertyCommand();

  virtual cmCommand* Clone() 
    {
      return new cmGetPropertyCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the input file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * This determines if the command is invoked when in script mode.
   */
  virtual bool IsScriptable() { return true; }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "get_property";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Get a property.";
    }
  
  /**
   * Longer documentation.
   */
  virtual const char* GetFullDocumentation()
    {
      return
        "  get_property(VAR scope_value property)\n"
        "  get_property(VAR scope_value property \n"
        "               BRIEF_DOCS)\n"
        "  get_property(VAR scope_value property \n"
        "               FULL_DOCS)\n"
        "Get a property from cmake.  The scope_value is either GLOBAL, "
        "DIRECTORY dir_name, TARGET tgt_name, SOURCE_FILE src_name, "
        "TEST test_name or VARIABLE var_name. The resulting value is "
        "stored in the variable VAR. If the property is not found, "
        "CMake will report an error. The second and third signatures "
        "return the documentation for a property or variable instead of "
        "its value.";
    }
  
  cmTypeMacro(cmGetPropertyCommand, cmCommand);
};



#endif
