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
#include <sstream>


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
	 * Also passes a pointer for user data (which could be 
	 * a class instance for example) via lightuserdata.
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

/**
 * Returns an string containing the function, filename, and line number.
 * Returns an string containing the function, filename, and line number.
 * It may look like "SomeFunction:/home/foo/MyProj/src/CMakeLua.lua:151
 * @return Returns a std::string containing these items separated by colons.
 */
std::string LuaUtils_GetLocationString(lua_State* lua_script, int level)
{
	std::string function_name;
	std::string file_name;
	std::string ret_string;
	std::ostringstream format_stream;
	int error;
	lua_Debug ar;
	
	
	// Typically, the level is 1 to 
	// go one up the stack to find information about 
	// who called this function so we can find the name
	// and line number.
	// This function only fills the private parts of the structure.
	// lua_getinfo must be called afterwards.
	error = lua_getstack(lua_script, level, &ar);
	// If there was an error, return
	if(1 != error)
	{
		return ret_string;
	}
	/* Fill up the structure with requested information
	 * n - gets the "name" of the function we're looking at
	 * n also gets "namewhat" which states 'global', 'local',
	 * 'field', or 'method'.
	 * S - Provides the "source" which seems to be 
	 * the full path and filename (via chunkname) associated
	 * with this lua state.
	 * S also provides "short_src" which seems to be a 60 max character
	 * truncated version of source.
	 * S also provides "what" which is Lua function, C function, 
	 * Lua main.
	 * And S provides the line number ("linedefined") that the 
	 * current function begins its definition.
	 * l - Provides the current line number ("currentline").
	 * u - provides the number of upvalues ("nups").
	 */
	//	error = lua_getinfo(script_ptr, "nSlu", &ar);
	error = lua_getinfo(lua_script, "nSl", &ar);
	
	// If there was an error, print out what we have
	if(0 == error)
	{
		return ret_string;
	}

#if 0
	fprintf(stderr, "Name: %s\n", ar.name);
	
	fprintf(stderr, "namewhat: %s\n", ar.namewhat);
	fprintf(stderr, "what: %s\n", ar.what);
	fprintf(stderr, "short_src: %s\n", ar.short_src);
	
	fprintf(stderr, "source: %s\n", ar.source);
	fprintf(stderr, "currentline: %d\n", ar.currentline);
	fprintf(stderr, "linedefined: %d\n", ar.linedefined);
	fprintf(stderr, "upvalues: %d\n", ar.nups);
#endif
	
	// Clear the buffer 
//	format_stream.str("");
#if 0
	// May crash program if any fields are NULL
	format_stream << ar.name
		<< ":"
		<< ar.source
		<< ":"
		<< ar.currentline;
	subkeyword = format_stream.str();
#endif
	
	format_stream << ar.currentline;
	
#if 1
	if(NULL != ar.source)
	{
		file_name = ar.source;
	}
#else
	if(NULL != ar.short_src)
	{
		file_name = ar.short_src;
	}
#endif
	if(NULL != ar.name)
	{
		function_name = ar.name;
	}
	else
	{
		/* Ugh. Lua 5.1 doesn't report function names at 
		 * the global level when called from C.
		 * Mike Pall offered me some code that works from within Lua
		 * to restore the 5.0 behavior.
		 */
//		fprintf(stderr, "top (start) =%d\n", lua_gettop(lua_script));
		error = lua_getinfo(lua_script, "f", &ar);
		if(0 == error)
		{
			fprintf(stderr, "Unexpected error calling lua_getinfo");
		}
		/* There should be a function pushed on top by calling get_info with "f". 
		 * If not, I don't know what's going on. Hopefully something was pushed.
		 * otherwise my pop is going to be unbalanced.
		 */
		if(lua_isfunction(lua_script, -1))
		{
			/* table is in the stack at index 't' */
			lua_pushnil(lua_script);  /* dummy key because each call to lua_next pops 1 */
			while(lua_next(lua_script, LUA_GLOBALSINDEX) != 0)
			{
				/* uses 'key' (at index -2) and 'value' (at index -1) */
				/*
				printf("%s - %s\n",
					   lua_typename(lua_script, lua_type(lua_script, -2)),
					   lua_typename(lua_script, lua_type(lua_script, -1)));
					   
				if(lua_type(lua_script, -2) == LUA_TSTRING)
				{
					printf("\tkey  : %s\n", lua_tostring(lua_script, -2));
				}
				else
				{
			//		printf("\tkey  : %d\n", lua_tointeger(lua_script, -2));
				}
				if(lua_type(lua_script, -1) == LUA_TSTRING)
				{
					printf("\tvalue: %s\n", lua_tostring(lua_script, -1));
				}
				else
				{
			//		printf("\tvalue: %d\n", lua_tointeger(lua_script, -1));
				}
				*/
				
				/* Experimental results are coming out as:
				 * string - function (key - value)
				 * The current function is at index: -1, and the function we want
				 * to compare against is at -3 (put there by lua_getinfo(L, "f", &ar)).
				 * If we hit, the function name is the string at index: -2
				 */
				if(lua_type(lua_script, -1) == LUA_TFUNCTION)
				{
					if(lua_rawequal(lua_script, -1, -3))
					{
						//printf("We have a match!!!\n");
						if(lua_type(lua_script, -2) == LUA_TSTRING)
						{
							function_name = lua_tostring(lua_script, -2);
						//	printf("\tkey  : %s\n", lua_tostring(lua_script, -2));
						}
						/* else, we don't have a global name? */
						
						/* We're going to break early. We need to make sure
						 * the stack is popped correctly. Normally,
						 * the end of the loop pops 1, and the next call to
						 * lua_next pops another 1. Since we are bypassing
						 * both, we need to pop 2.
						 */
						 lua_pop(lua_script, 2);
						 break;
					}
				}
						   
				/* removes 'value'; keeps 'key' for next iteration where 
				 * the next call to lua_next will pop the key and start all over again
				 */
				lua_pop(lua_script, 1);
			}
//			fprintf(stderr, "top (out) =%d\n", lua_gettop(lua_script));

			/* We need to pop the function on the stack placed by lua_getinfo(L, "f", &ar) */
		}
		/* pop the function placed by get_info */
		lua_pop(lua_script, 1);
//		fprintf(stderr, "top (end) =%d\n", lua_gettop(lua_script));

	}
	// Adjust the memory size of ret_string to 
	// try to keep memory allocations to a minimum
	ret_string.reserve(function_name.size() 
					   + file_name.size()
					   + format_stream.str().size()
					   + 2  // add the size of the colons
					   );

	ret_string = 
		function_name
		+ std::string(":") 
		+ file_name
		+ std::string(":") 
		+ format_stream.str()
		;

	return ret_string;
}


