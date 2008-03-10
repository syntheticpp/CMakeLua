-- a simple test case
cmake.project ("LuaTest");

cmake.add_executable ("LuaTest", "simple.cxx");

sources = {
  "simpleLib.cxx",
  "simpleCLib.c", 
  "simpleWe.cpp"
}

--  parameters as list of strings
cmake.add_library ("simpleLibList", cmake.STATIC, unpack(sources));

-- paramaters are keys in ONE table
cmake.add_library {"simpleLibTable", cmake.STATIC, unpack(sources)};

cmake.target_link_libraries ("LuaTest", "simpleLibList", "simpleLibTable");

print("\n\n---------------------------------------------------------------------------");
compiler = GetDefinition("CMAKE_CXX_COMPILER")
print("Compiler: " .. compiler .."\n");

setmetatable(_G, {__index = function (_, n)
 --print("__index: couldn't find value, try as CMake variable " .. n .. "\n"); 
 return GetDefinition(n) end})
 
print(CMAKE_CXX_COMPILER .."\n");
print(CMAKE_BINARY_DIR .."\n");
print(compiler .."\n");

print("\nSet value in Lua without a call of AddDefinition")
print("AddDefinition(\"CMAKE_FROM_LUA_2\", \"AddDefinition called\"")
AddDefinition("CMAKE_FROM_LUA_2", "AddDefinition called")
print("value of CMAKE_FROM_LUA_2:" .. CMAKE_FROM_LUA_2);

setmetatable(_G, {
__index = function (_, n) 
--print("__index: couldn't find value, try as CMake variable " .. n .. "\n"); 
return GetDefinition(n) end,
__newindex = function (_, n1, n2) 
--print("__newindex: set as CMake variable " .. n1 .. "\n"); 
AddDefinition(n1, n2) end
})

print("\nSet value in Lua without a explicit function call")
print("\nCMAKE_FROM_LUA_2 = \"quod erat demonstrandum\"")
CMAKE_FROM_LUA_2 = "quod erat demonstrandum"
print("value of CMAKE_FROM_LUA_2: " .. CMAKE_FROM_LUA_2);

print("\nCreate CMake variable and asign value of CMAKE_SOURCE_DIR");
print("CMAKE_FROM_LUA_3 = CMAKE_SOURCE_DIR")
CMAKE_FROM_LUA_3 = CMAKE_SOURCE_DIR 
print("value of CMAKE_FROM_LUA_3:" .. CMAKE_FROM_LUA_3)

 
print("\n\nReplace CMake variable with value of another cmake variable");
print("original value of CMAKE_SOURCE_DIR  : " .. CMAKE_SOURCE_DIR);
print("original value of CMAKE_CXX_COMPILER: " .. CMAKE_CXX_COMPILER);
print("CMAKE_SOURCE_DIR = CMAKE_CXX_COMPILER")
CMAKE_SOURCE_DIR = CMAKE_CXX_COMPILER
print("new value of CMAKE_SOURCE_DIR: " .. CMAKE_SOURCE_DIR);


print("---------------------------------------------------------------------------\n\n");


--print("\n\DefinitionStackToString from Lua\n:  " .. DefinitionStackToString("back") .. "\n\n");

--print("The location of simpleLib is: " .. cmake.get_property("TARGET", "simpleLib", "LOCATION"));

--cmake.add_subdirectory("SUBDIR")
