

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

set(cml1 a1 a2 a3)
set(cml2 "b 1" "b 2" "b 3")


lua("
CMakeString \"cml1\" 
CMakeString \"num\" 
l1 = string.split(\";\", cml1)
num = #l1
")

lua("
CMakeString \"joined_list1\" 
joined_list1 = string.join(\"-\", l1)
")

lua("
CMakeString \"joined_list2\" 
l2 = string.split(\";\", cml2)
joined_list2 = string.join(\"  ,  \", l2)
")

lua("
CMakeString \"joined_list3\" 
joined_list3 = string.join(\";\", list.reverse(list.concat(l1, l2)) )
")

message(STATUS "")
message(STATUS "list examples")
message(STATUS "-------------")
message(STATUS "number of entries        : ${num}")
message(STATUS "joined with \"-\"          : ${joined_list1}")
message(STATUS "joined with \"  ,  \"      : ${joined_list2}")
message(STATUS "join(\";\", reverse(concat): ${joined_list3}")
message(STATUS "")


# list metafunctions

# use CMake values as fall back

lua("
CMAKE_VARIABLES = {}

isStringList = function(x) 
					-- TODO
					return true
				end
	
setmetatable(_G, {
__index = function (_, n) 
		local val = GetDefinition(n) 
		if val == nil then
			return _G[n] 
		else
			-- return the CMake list as indexed table
			return string.split(\";\", val) 
		end
	end,
__newindex = function (_, n1, n2) 
		if isStringList(n1) then 
			-- only string lists are visible in CMake
			AddDefinition(n1, string.join(\";\", n2)) 
		else 
			rawset(_G, n1, n2)
		end
	end
	})
")


message(STATUS "")
message(STATUS "list metatable examples")
message(STATUS "-------------")
set(cmlist x1 x2 x3 x4)

lua("
print(\"-- table in lua after  set(cmlist x1 x2 x3 x4): \")
print(cmlist)
print()
items = {tostring(#cmlist)}
lualist = {\"l1\", \"l2\", \"l3\", \"l4\", \"l5\"} 
itemslua =  {tostring(#lualist)}
")

lua("
lualist2 = lualist
lualist3 = cmlist
") 
 
message(STATUS "items in cmlist    : ${items}")
message(STATUS "items in lualist   : ${itemslua}")
message(STATUS "set(cmlist ...)    : ${cmlist}")
message(STATUS "lualist  = {...}   : ${lualist}")
message(STATUS "lualist2 = lualist : ${lualist2}")
message(STATUS "lualist3 = cmlist  : ${lualist3}")
message(STATUS "")

