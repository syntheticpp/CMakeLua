-- a simple test case
cm_project ("LuaTest");

cm_add_executable ("LuaTest", "simple.cxx");

sources = {
  "simpleLib.cxx",
  "simpleCLib.c", 
  "simpleWe.cpp"
}

-- New idea: Make API calls in cm. table (namespace) instead of cm_
cm.add_library ("simpleLib", cm.STATIC, sources);

cm_target_link_libraries ("LuaTest", "simpleLib");

--print("The location of simpleLib is: " .. cm_get_property("TARGET", "simpleLib", "LOCATION"));

cm_add_subdirectory("SUBDIR")
