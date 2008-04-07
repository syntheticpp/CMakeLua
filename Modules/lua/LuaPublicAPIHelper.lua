--[[=========================================================================

  Copyright (c) 2008 Eric Wing
  Copyright (c) 2008 Peter Kümmel
  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================--]]

-- Eric Wing
-- Little experiment to handle different styles of passing in parameters to
-- CMake functions, plus demonstrations of how to setup some basic things very
-- easily within Lua.

--print("In LuaPublicAPIHelper.lua")

-- Ugly hack to make sure script is only run once.
-- We should probably change this to follow the 5.1 module system conventions
-- but that also requires some C-api for something like luaL_require.
if LuaPublicAPIHelper_HasRun then
--print("Has run")
	return
else
	LuaPublicAPIHelper_HasRun = true
--print("first run")
end


-- create a namespace for public functions using tables
if cmake == nil then
	cmake = {}
end

-- create a namespace for public functions using tables
if cmakelua == nil then
	cmakelua = {}
end

-- Create some global constants (maybe we want this in the namespace too?)
cmake.STATIC = "STATIC"
cmake.DYNAMIC = "DYNAMIC"
cmake.MODULE = "MODULE"
cmake.DEBUG = "Debug"
cmake.RELEASE = "Release"

-- Print a (nested) table (just for debugging, you can ignore this)
function cmakelua.table_print(t, indent, done)
    local my_string
    done = done or {}
    indent = indent or 0
    for key, value in pairs (t) do
        -- Tell (string.rep (" ", indent)) -- indent it
        local my_rep = string.rep (" ", indent) -- indent it
        my_string = my_rep

        if type (value) == "table" and not done [value] then
            done [value] = true
            --      Note (tostring (key), ":");
            local mystring = my_string .. tostring(key) .. ":"
            print(mystring)
             cmakelua.table_print(value, indent + 2, done)
        else
            -- Tell (tostring (key), "=")
            my_string = my_string .. tostring(key) .. "=" .. tostring(value)
            print(my_string)
        end
    end
end








-- private helper function that recursively parses a (nested) table of strings
-- and flattens them into array. (This was adapted from 'rconcat' from
-- Programming in Lua.)
-- The parameter 'big_table' is the current flattened array of items and on completion, this will contain the array of all items. Pass in an empty table when 
-- calling to start things.
-- The parameter 'list' is the list of parameters the user passes in. It can
-- be a string, or table of strings, or nested tables of strings.
-- The function returns partial lists for internal/private use for recursion.
local function recursive_file_concat(big_table, list)
	if type(list) ~= "table" then
--		print("not table:", list)
		table.insert(big_table, list)
		return list
	end
	local res = {}
	for i=1, #list do
--		print("item", i, "is", list[i])
		res[i] = recursive_file_concat(big_table, list[i])
	end
--	print("returning res", res)
--	cmakelua.table_print(res)
	return res
end



-- Here we demonstrate some fancy argument handling for the convenience of the users

--[[
function cm.add_library(library_name, ...)
	local argument_table = {...} -- put the arguments in a table for easier handling
	local number_of_args = #argument_table -- use the # operator to get the size of the table
	-- print("Number of args is", number_of_args)

	-- So now the fun stuff. There are a whole bunch of permutations we can support.
	-- First, the user can use STATIC or SHARED or MODULE or omit the thing entirely.
	-- So lets check if the first argument is any of these.


	-- So for the file list, the user can do several things:
	-- They can pass in a table containing all the files in an array:
	-- list = { file1.c, file2.c, file3.c }
	-- add_library(list)
	--
	-- They can pass in each file as an individual argument.
	-- add_library("file1.c", "file2.c", "file3.c")
	--
	-- They can intersperse tables and individual files
	-- list1 = { file1.c, file2.c, file3.c }
	-- list2 = { file4.c, file5.c, file6.c }
	-- add_library(list1, "filea.c", list2, "fileb.c")
	--

	local flattened_list = {}
	recursive_file_concat(flattened_list, argument_table)


	-- Call the 'real' C/Lua CMake function
	return cmake.add_library(library_name, unpack(flattened_list))
end
--]]

local function setup_fancy_argument_handling(original_function)
	local old_function = original_function

	return function(...)
		local argument_table = {...} -- put the arguments in a table for easier handling
		local number_of_args = #argument_table -- use the # operator to get the size of the table

		local flattened_list = {}
		recursive_file_concat(flattened_list, argument_table)

		--return old_function(unpack(flattened_list))
		return old_function(flattened_list)
	end
end

	


-- For every registered (C++) CMake API function (in cmake. table)
-- provide fancy argument handling
--for key, value in pairs(cmake) do
--[[
for key, value in pairs(_G) do
	if type(value) == "function" then
		local func_name = string.match(key, '^cm_(.*)$')
		if func_name then
--			print("Found function:", key, "func_name", func_name, ".")
			--cmake[key] = setup_fancy_argument_handling(value)
			--cmake[func_name] = value
			cmake[func_name] = setup_fancy_argument_handling(value)
			_G[key] = nil
		end
	else
--		print("Type:", type(value), "for key:", key)
	end
end
--]]

for key, value in pairs(cmake) do
	if type(value) == "function" then
--			print("Found function:", key, ".")
			cmake[key] = setup_fancy_argument_handling(value)
			
			-- also make them global
			--_G[key] = cmake[key] 
			
			--cmake[func_name] = value
			--cmake[func_name] = setup_fancy_argument_handling(value)
			--_G[key] = nil
	else
--		print("Type:", type(value), "for key:", key)
	end
end
--print("end of helper")



-- from CMake we only get lists so we have evtl to make a simple string
cmakelua.tostring = function (x) 
	if x == nil then
		return ""
	else
		return string.join(";", x)
	end
end

cmakelua.totable = function(x)
	if x == nil then
		return {}
	elseif type(x) == 'table' then
		return x
	else
		return { x }
	end
end

-- converts single CMake value to boolean.
-- empty, 0, N, NO, OFF, FALSE, NOTFOUND, or <variable>-NOTFOUND.
-- Will create compile error if table is passed.
cmakelua.toboolean = function(x)
	if x == nil then
		return false
	elseif type(x) == 'boolean' then
		return x
	elseif type(x) == 'number' then
		if x == 0 then -- contrary to lua, 0 is false in CMake
			return false
		else
			return true
		end
	elseif type(x) == 'string' then
		local lower_case_string = string.lower(x)
		if lower_case_string == 'false' then
			return false
		elseif lower_case_string == 'no' then
			return false
		elseif lower_case_string == 'off' then
			return false
		elseif lower_case_string == 'n' then
			return false
		elseif lower_case_string == 'notfound' then
			return false
		elseif lower_case_string == 'true' then
			return true
		elseif lower_case_string == 'yes' then
			return true
		elseif lower_case_string == 'on' then
			return true
		elseif lower_case_string == 'y' then
			return true
		elseif lower_case_string == '' then
			return false
		-- handle <variable>-NOTFOUND
		elseif string.find(lower_case_string, '-notfound') then
			return false
		-- return true for all other strings
		else
			return true
		end
	elseif type(x) == 'table' then
		-- if table is empty, return false
		if #x == 0 then
			return false
		-- return true for all other tables
		else
			return true
		end
	-- catch all, return true for anything else
	else
		return true
	end
end

-- converts single CMake value to boolean.
-- empty, 0, N, NO, OFF, FALSE, NOTFOUND, or <variable>-NOTFOUND.
-- Will create compile error if table is passed.
cmakelua.isboolean = function(x)
	if x == nil then
		return false
	elseif type(x) == 'boolean' then
		return true
	elseif type(x) == 'number' then
		if x == 0 then -- contrary to lua, 0 is false in CMake
			return true
		elseif x == 1 then
			return true
		-- any other number is not a boolean in my opinion
		else
			return false
		end
	elseif type(x) == 'string' then
		local lower_case_string = string.lower(x)
		if lower_case_string == 'false' then
			return true
		elseif lower_case_string == 'no' then
			return true
		elseif lower_case_string == 'off' then
			return true
		elseif lower_case_string == 'n' then
			return true
		elseif lower_case_string == 'notfound' then
			return true
		elseif lower_case_string == 'true' then
			return true
		elseif lower_case_string == 'yes' then
			return true
		elseif lower_case_string == 'on' then
			return true
		elseif lower_case_string == 'y' then
			return true
		elseif lower_case_string == '' then
			return true
		-- handle <variable>-NOTFOUND
		elseif string.find(lower_case_string, '-notfound') then
			return true
		-- return false for all other strings
		else
			return false
		end
	elseif type(x) == 'table' then
		-- how about: the only way that a table will be a boolean 
		-- is if it contains a single value that happens to be a boolean?
		-- if table is empty, return false
		if #x == 1 then
			return cmakelua.isboolean(x[1])
		else
			return false
		end
	-- catch all, return false for anything else
	else
		return false
	end
end

cmakelua.tonumber = tonumber
cmakelua.isnumber = tonumber

cmakelua.convert_to_native_lua_types = function(x)
	if x == nil then
		return x
	elseif type(x) == 'string' then
		if cmakelua.isboolean(x) then
			return cmakelua.toboolean(x)
		elseif cmakelua.isnumber(x) then
			return cmakelua.tonumber(x)
		else
			return x
		end
	elseif type(x) == 'table' then
		for key,value in pairs(x) do
			x[key] = cmakelua.convert_to_native_lua_types(x[key])
		end
		return x
	else
		return x
	end
end

-- in CMakeLua assumes all args for CMake are string lists 
cmakelua.is_string_list = function(x) 
					if x == nil then
						return false
					elseif type(x) == "function" then
						return false
					elseif type(x) == "table" then
						-- TODO check for array only
						return true
					end
				end




-- clean namespace
list = nil

lua_module_path = GetDefinition("CMAKE_MODULE_PATH")
if lua_module_path == nil then
	lua_module_path = GetDefinition("CMAKE_ROOT")
	if lua_module_path ~= nil then
		lua_module_path = lua_module_path .. "/Modules"
	end
end
if lua_module_path ~= nil then	
	lua_module_path = ";" .. lua_module_path .. "/lua"
	-- set search paths
	package.path = lua_module_path.."/?.lua" ..lua_module_path.."/stdlib/modules/?.lua"
	print("package.path is", package.path)
	lua_module_path = nil
	--print(package.path)
	
	-- load parts of stdlib
	require "list"
	require "string_ext"
	require "table_ext"
	require "base"
end



	
setmetatable(_G, 
{
__index = function (_, n) 
		local val = GetDefinition(n) 
		if val == nil then
			val = cmake[n]
			if val == nil then
				return rawget(_G, n)
			else
				return val
			end
		else
			-- return the CMake list as indexed table
			local t = string.split(";", val) 
			if #t == 1 then
				return cmakelua.convert_to_native_lua_types(rawget(t, 1))
			else
				return cmakelua.convert_to_native_lua_types(t)
			end
		end		
	end,
__newindex = function (_, n1, n2)
		if type(n2) == "boolean" then
			AddDefinition(n1, tostring(n2))
		elseif type(n2) == "number" then
			AddDefinition(n1, tostring(n2))
		elseif type(n2) == "string" then
			AddDefinition(n1, n2)
		elseif cmakelua.is_string_list(n2) then 
			-- only string lists are visible in CMake
			AddDefinition(n1, string.join(";", n2)) 
		else 
			rawset(_G, n1, n2)
		end
	end
})







