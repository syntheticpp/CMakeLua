/*=========================================================================

  Copyright (c) 2008 Eric Wing
  Copyright (c) 2008 Peter Kümmel
  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
/**
 * Lua helper utilities.
 * Eric Wing
 */

#include "cmLuaUtils.h"
#include <string.h>

/**
 * This function will create a nested table in Lua based on the 
 * the string you provide.
 * This function will create a nested table in Lua based on the 
 * the string you provide. Among other things, this may be used 
 * for things like namespaces when registering functions.
 * The original code was posted to the Lua mailing list by
 * Malte Thiesen in response to my question, "How to create Lua
 * nested namespaces in C.
 * 
 * @param L The Lua state to operate on.
 * @param table_name The name of the table you want to create. This could
 * be something like "cmake" or "cmake.utils". Each (nested) 
 * table/layer/namespace needs to be separated by exactly one dot (".").
 * @return Returns true on success, false on an error.
 */
bool LuaUtils_CreateNestedTable(lua_State * L, const std::string& table_name)
{
	// The tablename is seperated at the periods and the subtables are
	// created if they don't exist.
	// On success true is returned and the last subtable is on top of
	// the Lua-stack.

	std::string sub_table_name;
	std::string::size_type part_begin = 0;
	while(part_begin <= table_name.size())
	{
		std::string::size_type part_end;
		part_end = table_name.find(".", part_begin);
		if(part_end == std::string::npos)
		{
			part_end = table_name.size();
		}
		sub_table_name = table_name.substr(part_begin, part_end - part_begin);

		// Tables need to have a name (something like "a..b" occured)
		if(sub_table_name.size() == 0)
		{
			return false;
		}
		// Check if a table already exists
		// At the first pass the table is searched in the global
		// namespace. Later the parent-table on the stack is searched.
		if(0 == part_begin)
		{
			lua_pushstring(L, sub_table_name.c_str());
			lua_gettable(L, LUA_GLOBALSINDEX);
		}
		else
		{
			lua_pushstring(L, sub_table_name.c_str());
			lua_gettable(L, -2);
			if(!lua_isnil(L, -1))
			{
				lua_remove(L, -2);
			}
		}

		// If the table wasn't found, it has to be created
		if (lua_isnil(L, -1))
		{
			// Pop nil from the stack
			lua_pop(L, 1);
			// Create new table
			lua_newtable(L);
			lua_pushstring(L, sub_table_name.c_str());
			lua_pushvalue(L, -2);
			if (part_begin == 0)
				lua_settable(L, LUA_GLOBALSINDEX);
			else
			{
				lua_settable(L, -4);
				lua_remove(L, -2);
			}
		}

		// Skip the period
		part_begin = part_end + 1;
	}

	return true;
}



bool LuaUtils_RegisterFunc(lua_State* lua_state,
						   lua_CFunction function_ptr,
						   const char* function_name,
						   const char* library_name,
						   void* user_light_data
)
{
                                                                // stack: 

	/* Validate arguments. */
	if (!lua_state || !function_ptr || !function_name)
	{
		return false;
	}
	if (0 == strlen(function_name))
	{
		return false;
	}
	//fprintf(stderr, "Lua Register (start), end top = %d\n", lua_gettop(lua_state));
	//fprintf(stderr, "Lua Register, start top = %d\n", lua_gettop(script));
	
	/* This will embed the function in a namespace if
		desired */
	if(NULL != library_name)
	{
		LuaUtils_CreateNestedTable(lua_state, library_name);   // stack: table
	}
	else
	{
		lua_pushvalue(lua_state, LUA_GLOBALSINDEX);            // stack: table
	}
	
	/* Register function into lua_state object.
	 * Also passes pointer to a SpaceObjScriptIO instance 
	 * via lightuserdata.
	 */
	// use function_name when calling from Lua
	lua_pushstring( lua_state, function_name );               // stack: table string
	// remember function_name as upvalue
	lua_pushstring( lua_state, function_name );               // stack: table string string
	// push closure on stack
	lua_pushcclosure( lua_state, function_ptr, 1 );           // stack: table string func
	// add function to namespace table
	lua_settable( lua_state, -3);                             // stack: table
	
	/* pop the table that we created/getted */
	lua_pop(lua_state, 1);									  // stack: 

	//fprintf(stderr, "Lua Register (end), end top = %d\n", lua_gettop(lua_state));
	
	return true;
}


bool LuaUtils_RegisterValue(lua_State* script,
							double value,
							const char* literal_name,
							const char* library_name)
{
	/* Validate arguments */
	if (!script || !literal_name)
	{
		return false;
	}
	if (0 == strlen(literal_name))
	{
		return false;
	}
	
	/* This will embed the function in a namespace if
	   desired */
	if(NULL != library_name)
	{
		LuaUtils_CreateNestedTable(script, library_name);
	}
	else
	{
		lua_pushvalue(script, LUA_GLOBALSINDEX);		
	}
	
	/* Register variable into table */
	lua_pushstring( script, literal_name);  /* Add variable name. */
	lua_pushnumber( script, value );  /* Add value */
	lua_settable( script, -3);
	
	/* pop the table that we created/getted */
	lua_pop(script, 1);
	
	return true;
}



