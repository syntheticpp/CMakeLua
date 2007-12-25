-- a simple test case
cm_project ("LuaTest");

cm_add_executable ("LuaTest", "simple.cxx");

sources = {
  "simpleLib.cxx",
  "simpleCLib.c", 
  "simpleWe.cpp"
}

cm_add_library ("simpleLib", "STATIC", unpack(sources));

cm_target_link_libraries ("LuaTest", "simpleLib");

print("The location of simpleLib is: " .. 
  cm_get_property("TARGET", "simpleLib", "LOCATION"));
