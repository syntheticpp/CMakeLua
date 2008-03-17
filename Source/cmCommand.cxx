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
#include "cmLuaUtils.h"

static int cmLuaFunc(lua_State *L) 
{
  // build a list file function 
  cmListFileFunction lff;
  cmExecutionStatus status;
  lff.Name = lua_tostring(L, lua_upvalueindex(1));
  
  int i;
  int top = lua_gettop(L);
  /* The old way used variable number of arguments.
   * The problem with this is that the max number of 
   * arguments is capped at 2048. If people are
   * list large number of files, we will get into trouble.
   * So I'm switching the implementation to pass a single
   * table (array) instead which doesn't have the cap.
   */
/*  
  for (i = 1; i <= top; i++) 
    { 
    const char *arg = lua_tostring(L, i);
    lff.Arguments.push_back(cmListFileArgument(arg, false,
                                               0, 0));
    }
*/
 /* New way: uses a table (array) */
  if(lua_istable(L, -1))
    {
    int array_length = luaL_getn(L, -1);	
    for (i = 1; i<= array_length; i++)
      {
      lua_rawgeti(L, 1, i); // assumes table is first (and only) argument
      const char *arg = lua_tostring(L, -1);
      lff.Arguments.push_back(cmListFileArgument(arg, false,
                                                 0, 0));
      lua_pop(L, 1);
      }
	}
  else
    {
		std::cerr << "CMakeLua functions can only take a single table (array) parameter in cmCommand::cmLuaFunc(). It is expected that fancy argument handling is handled in LuaPublicAPIHelper.lua which should prepare all arguments to be formatted for this function. If this message is seen, either the helper failed or invocation bypassed the helper." << std::endl;
    }
  // get the current makefile
  lua_pushstring(L,"cmCurrentMakefile");
  lua_gettable(L, LUA_REGISTRYINDEX);
  cmMakefile *mf = static_cast<cmMakefile *>(lua_touserdata(L,-1));

  // pass it to ExecuteCommand
  mf->ExecuteCommand(lff, status);
  // What should I do with status?

  return 0;  /* number of results */
}

cmCommand::cmCommand()
{
  this->Makefile = 0; 
  this->Enabled = true;
  this->ExposeToLua = true;
  this->LuaFunction = cmLuaFunc;
}
