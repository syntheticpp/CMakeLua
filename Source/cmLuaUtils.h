#ifndef CMLUAUTILS_H
#define CMLUAUTILS_H

#include <stddef.h>
#include <string>

extern "C" {
#include "../Utilities/lua/src/lua.h"
#include "../Utilities/lua/src/lauxlib.h"
#include "../Utilities/lua/src/lualib.h"
}

struct lua_State;
typedef int (*lua_CFunction) (lua_State *L);


bool LuaUtils_CreateNestedTable(lua_State * L, const std::string& table_name);

bool LuaUtils_RegisterFunc(lua_State* script,
						   lua_CFunction function_ptr,
						   const char* function_name,
						   const char* library_name = NULL,
						   void* user_light_data = NULL
);

bool LuaUtils_RegisterValue(lua_State* script,
							double value, 
							const char* literal_name,
							const char* library_name = NULL);

#endif // CMLUAUTILS_H

