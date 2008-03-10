

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

