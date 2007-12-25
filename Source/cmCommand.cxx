/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmCommand.h,v $
  Language:  C++
  Date:      $Date: 2007/07/02 19:43:21 $
  Version:   $Revision: 1.25 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "cmCommand.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

static int cmLuaFunc(lua_State *L) 
{
  // build a list file function 
  cmListFileFunction lff;
  lff.Name = lua_tostring(L, lua_upvalueindex(1));
  
  int i;
  int top = lua_gettop(L);
  for (i = 1; i <= top; i++) 
    { 
    const char *arg = lua_tostring(L, i);
    lff.Arguments.push_back(cmListFileArgument(arg, false,
                                               0, 0));
    }

  // get the current makefile
  lua_pushstring(L,"cmCurrentMakefile");
  lua_gettable(L, LUA_REGISTRYINDEX);
  cmMakefile *mf = static_cast<cmMakefile *>(lua_touserdata(L,-1));

  // pass it to ExecuteCommand
  mf->ExecuteCommand(lff);

  return 0;  /* number of results */
}

cmCommand::cmCommand()
{
  this->Makefile = 0; 
  this->Enabled = true;
  this->ExposeToLua = true;
  this->LuaFunction = cmLuaFunc;
}
