-- a simple test case
cm_project ("LuaTest");

cm_add_executable ("LuaTestSub", "simple.cxx");

sources = {
  "simpleLib.cxx",
  "simpleCLib.c", 
  "simpleWe.cpp"
}

cm_add_library ("simpleLibSub", "STATIC", unpack(sources));

cm_target_link_libraries ("LuaTestSub", "simpleLibSub");

--print("The location of simpleLib is: " .. 
  cm_get_property("TARGET", "simpleLibSub", "LOCATION"));

