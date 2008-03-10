

set(A a b c)
set(B a2b2c2)

lua("
setmetatable(_G, {
__index = function (_, n) return GetDefinition(n) end,
__newindex = function (_, n1, n2) AddDefinition(n1, n2) end })
")

lua("A = B")


if(A STREQUAL "a2b2c2")
	message(STATUS "Lua test A = B passed")
else()
	message(STATUS "Lua test A = B failed, A = ${A} B = ${B}")
endif()


# move the cmake variables into the cmake table

lua("
setmetatable(_G, nil)
setmetatable(cmake, {
__index = function (_, n) return GetDefinition(n) end,
__newindex = function (_, n1, n2) AddDefinition(n1, n2) end})
")

set(C a b c)
set(D a2b2c2)

lua("cmake.C = cmake.D")


if(C STREQUAL "a2b2c2")
	message(STATUS "Lua test cmake.C = cmake.D passed")
else()
	message(STATUS "Lua test cmake.C = cmake.D failed, cmake.C = ${C} cmake.D = ${d}")
endif()

lua("
t = {}
cmake.t = tostring(t)
")

message(STATUS "Address of Lua table t is ${t}")


# use CMake values as fall back

lua("
CMAKE_VARIABLES = {}

CMakeString = function (n) 
		local val = rawget(_G, n)
		if val ~= nil then
			AddDefinition(n, tostring(val) )    -- init with Lua value
			rawset(_G, n, nil)                  -- overwrite already exising names
		end
		CMAKE_VARIABLES[n] = 1 
	end
	
setmetatable(_G, {
__index = function (_, n) 
		local val = GetDefinition(n) 
		if val == nil then
			return _G[n] 
		else
			return val 
		end
	end,
__newindex = function (_, n1, n2) 
		if CMAKE_VARIABLES[n1] == 1 then 
			AddDefinition(n1, n2) 
		else 
			rawset(_G, n1, n2)
		end
	end 
	})
")

set(str     "value from CMake's str")

lua("
CMakeString \"t1\"
t1 = str                        -- str not known in Lua use CMake value

str = \"value from Lua's str\"  -- now there is str in Lua
CMakeString \"t2\"
t2 = str                        -- use value of Lua's str
")

set(str     "value from CMake's str visibale again")
lua("
str = nil	-- make CMake's str visibile again without using CMakeString
CMakeString \"t3\"
t3 = str
")

lua("
str = \"value from Lua's str overwritten\" 	
CMakeString \"str\"             -- now Lua's str overwrites CMakes's variable str
CMakeString \"t4\"
t4 = str
")


lua("
T = {}
ref = T
CMakeString \"ref\"             -- ref = tostring(T)
")

message(STATUS "")
message(STATUS "t1  : ${t1}")
message(STATUS "t2  : ${t2}")
message(STATUS "t3  : ${t3}")
message(STATUS "t4  : ${t4}")
message(STATUS "ref : ${ref}")
message(STATUS "T   : unknown in cmake. ${T}")


