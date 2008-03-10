-- a simple test case
cmake.project ("LuaTest");

cmake.add_executable ("LuaTestSub", "simple.cxx");

sources = {
  "simpleLib.cxx",
  "simpleCLib.c", 
  "simpleWe.cpp"
}

cmake.add_library ("simpleLibSub", "STATIC", unpack(sources));
--cmake.add_library ("simpleLibSub", "STATIC", unpack(sources));

cmake.target_link_libraries ("LuaTestSub", "simpleLibSub");

--print("The location of simpleLib is: " .. cmake.get_property("TARGET", "simpleLibSub", "LOCATION"));

