#ifndef CMLUAUTILS_H
#define CMLUAUTILS_H


#include <stddef.h>
#include <string>

extern "C"
{
//struct lua_State;
//typedef int (*lua_CFunction) (lua_State *L);
	#include "lua.h"
}


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




template <typename Host, typename Func>
int callMemberFunction_char_char(lua_State* L)
{
	Host& h = **static_cast<Host**>(lua_touserdata(L, lua_upvalueindex(1)));
	Func f = *static_cast<Func*>(lua_touserdata(L, lua_upvalueindex(2)));
	const char* str = lua_tostring(L, 1);
	const char* r = (h.*f)(str);
	lua_pushstring(L, r);
	return 1;
}

template <typename Host, typename Func>
int callMemberFunction_void_char_char(lua_State* L)
{
	Host& h = **static_cast<Host**>(lua_touserdata(L, lua_upvalueindex(1)));
	Func f = *static_cast<Func*>(lua_touserdata(L, lua_upvalueindex(2)));
	const char* str_1 = lua_tostring(L, 1);
  const char* str_2 = lua_tostring(L, 2);
	(h.*f)(str_1, str_2);
	return 0;
}

template<class Host, class Func>
struct Helper_registerMemberFunction
{
  lua_State*const  L;
  Helper_registerMemberFunction(lua_State*  L, Host* host, Func func, const char* name) : L(L)
  {
    lua_pushvalue(L, LUA_GLOBALSINDEX);
    lua_pushstring(L, name);
    *static_cast<Host**>(lua_newuserdata(L, sizeof(Host*))) = host;
    *static_cast<Func*>(lua_newuserdata(L, sizeof(Func)))   = func;
  }

  ~Helper_registerMemberFunction()
  {
    lua_settable(L, -3);
    lua_pop(L, 1);
  }
};

template<class Host>
void registerMemberFunction(lua_State* L, Host* host, const char* (Host::* func)(const char*) const, const char* name)
{
  typedef const char* (Host::* Func)(const char*) const;
  Helper_registerMemberFunction<Host, Func> helper(L, host, func, name);
  lua_pushcclosure(L, callMemberFunction_char_char<Host, Func>, 2);
}

template<class Host>
void registerMemberFunction(lua_State* L, Host* host, void (Host::* func)(const char*, const char*), const char* name)
{
  typedef void (Host::* Func)(const char*, const char*);
  Helper_registerMemberFunction<Host, Func> helper(L, host, func, name);
  lua_pushcclosure(L, callMemberFunction_void_char_char<Host, Func>, 2);
}

#endif // CMLUAUTILS_H

