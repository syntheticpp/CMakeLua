/*=========================================================================

  Copyright (c) 2008 Peter Kümmel.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmLuaCommand_h
#define cmLuaCommand_h

#include "cmCommand.h"

/** \class cmLuaCommand
 * \brief Execute a string as Lua command sequence
 *
 * cmLuaCommand executes a strring as Lua program
 */
class cmLuaCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmLuaCommand;
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
  virtual const char* GetName() {return "lua";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation()
    {
    return "Execute a string as Lua program.";
    }

  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  "
      ;
    }

  cmTypeMacro(cmLuaCommand, cmCommand);
private:
  bool ParseDefinition(std::string const& def);
};



#endif
