
cmake.project("LuaTest")


sources = {
  "simpleLib.cxx",
  "simpleCLib.c", 
  "simpleWe.cpp"
}

SomeLuaVariable = "Some Lua Variable"

--compiler = GetDefinition("CMAKE_CXX_COMPILER")
--print("HELLO Compiler: " .. compiler .."\n");
print("HELLO")

print("CMAKE_HOST_UNIX:", CMAKE_HOST_UNIX)
print("CMAKE_FILES_DIRECTORY:", CMAKE_FILES_DIRECTORY)
print("CMAKE_C_COMPILER_ID:", CMAKE_C_COMPILER_ID)
print("CMAKE_C_PLATFORM_ID:", CMAKE_C_PLATFORM_ID)
print("APPLE:", APPLE)
print("CMAKE_CXX_COMPILER_ID:", CMAKE_CXX_COMPILER_ID)
print("CMAKE_CXX_PLATFORM_ID:", CMAKE_CXX_PLATFORM_ID)
print("CMAKE_SIZEOF_VOID_P:", CMAKE_SIZEOF_VOID_P)


print(CMAKE_CXX_COMPILER .."\n");
print(CMAKE_BINARY_DIR .."\n");
--print(compiler .."\n");

cmake.find_package("OpenAL")


print("OPENAL_INCLUDE_DIR: ", OPENAL_INCLUDE_DIR)
print("OPENAL_LIBRARY: ", OPENAL_LIBRARY)
print("OPENAL_FOUND: ", OPENAL_FOUND)
OPENAL_FOUND = false

path_to_cmaketest_file = PROJECT_SOURCE_DIR .. "/CMakeScriptTest.txt"
print("path_to_cmaketest_file:", path_to_cmaketest_file)

cmake.include(path_to_cmaketest_file)

print("A:", a)
print("B:", b)
if a==b then
	print("a == b")
else
	print("a == b test failed")
end


