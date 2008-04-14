/*=========================================================================

  Copyright (c) 2008 Peter Kümmel.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmLuaCommand.h"

#include "cmake.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}


// cmLuaCommand
bool cmLuaCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  // assume there is only one string
  if(args.size() < 1)
  {
    SetError("lua command called with incorrect number of arguments");
    return false;
  }

  int error = 0;
  if (args.size() == 2 && args[0] == "FILE") 
    {
    std::string luaFile = Makefile->GetCurrentDirectory();
    luaFile += "/";
    luaFile += args[1];
    error = Makefile->RunLuaFile(luaFile);
    }
  else
    {
    // join args
    std::string str;
    for(std::vector<std::string>::const_iterator it = args.begin(); it != args.end(); ++it)
      {
      str += *it;
      }

    error = luaL_dostring(Makefile->GetCMakeInstance()->GetLuaState(), str.c_str());
    }

  if (error != 0) 
    {
    SetError("Error when processing Lua code");
    return false;
    }

  return true;
}

