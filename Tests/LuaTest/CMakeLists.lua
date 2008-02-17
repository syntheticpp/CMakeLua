-- a simple test case
cmake.project ("LuaTest");

cmake.add_executable ("LuaTest", "simple.cxx");

sources = {
  "simpleLib.cxx",
  "simpleCLib.c", 
  "simpleWe.cpp"
}

-- New idea: Make API calls in cm. table (namespace) instead of cmake.
cmake.add_library ("simpleLib", cmake.STATIC, sources);

cmake.target_link_libraries ("LuaTest", "simpleLib");

--print("The location of simpleLib is: " .. cmake.get_property("TARGET", "simpleLib", "LOCATION"));

cmake.add_subdirectory("SUBDIR")
