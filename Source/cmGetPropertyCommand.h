/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmGetPropertyCommand.h,v $
  Language:  C++
<<<<<<< cmGetPropertyCommand.h
  Date:      $Date: 2008/01/23 15:27:59 $
  Version:   $Revision: 1.6 $
=======
  Date:      $Date: 2008-04-01 18:22:06 $
  Version:   $Revision: 1.7 $
>>>>>>> 1.7

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
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);

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
        "  get_property(<variable>\n"
        "               <GLOBAL             |\n"
        "                DIRECTORY [dir]    |\n"
        "                TARGET    <target> |\n"
        "                SOURCE    <source> |\n"
        "                TEST      <test>   |\n"
        "                VARIABLE>\n"
        "               PROPERTY <name>\n"
        "               [SET | DEFINED | BRIEF_DOCS | FULL_DOCS])\n"
        "Get one property from one object in a scope.  "
        "The first argument specifies the variable in which to store the "
        "result.  "
        "The second argument determines the scope from which to get the "
        "property.  It must be one of the following:\n"
        "GLOBAL scope is unique and does not accept a name.\n"
        "DIRECTORY scope defaults to the current directory but another "
        "directory (already processed by CMake) may be named by full or "
        "relative path.\n"
        "TARGET scope must name one existing target.\n"
        "SOURCE scope must name one source file.\n"
        "TEST scope must name one existing test.\n"
        "VARIABLE scope is unique and does not accept a name.\n"
        "The required PROPERTY option is immediately followed by the name "
        "of the property to get.  "
        "If the property is not set an empty value is returned.  "
        "If the SET option is given the variable is set to a boolean "
        "value indicating whether the property has been set."
        "If the DEFINED option is given the variable is set to a boolean "
        "value indicating whether the property has been defined "
        "such as with define_property. "
        "If BRIEF_DOCS or FULL_DOCS is given then the variable is set to "
        "a string containing documentation for the requested property.  "
        "If documentation is requested for a property that has not been "
        "defined NOTFOUND is returned.";
    }
  
  cmTypeMacro(cmGetPropertyCommand, cmCommand);
private:
  enum OutType { OutValue, OutDefined, OutBriefDoc, OutFullDoc, OutSet };
  std::string Variable;
  std::string Name;
  std::string PropertyName;
  OutType InfoType;

  // Implementation of result storage.
  bool StoreResult(const char* value);

  // Implementation of each property type.
  bool HandleGlobalMode();
  bool HandleDirectoryMode();
  bool HandleTargetMode();
  bool HandleSourceMode();
  bool HandleTestMode();
  bool HandleVariableMode();
};

#endif
