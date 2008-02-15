
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



-- Print a (nested) table (just for debugging, you can ignore this)
function TablePrint(t, indent, done)
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
            TablePrint(value, indent + 2, done)
        else
            -- Tell (tostring (key), "=")
            my_string = my_string .. tostring(key) .. "=" .. tostring(value)
            print(my_string)
        end
    end
end



-- create a namespace for public functions using tables
cm = {}

-- Create some global constants (maybe we want this in the namespace too?)
cm.STATIC = "STATIC"





-- private helper function that recursively parses a (nested) table of strings
-- and flattens them into array. (This was adapted from 'rconcat' from
-- Programming in Lua.)
-- The parameter 'big_table' is the current flattened array of items and on completion, this will contain the array of all items. Pass in an empty table when 
-- calling to start things.
-- The parameter 'list' is the list of parameters the user passes in. It can
-- be a string, or table of strings, or nested tables of strings.
-- The function returns partial lists for internal/private use for recursion.
function recursive_file_concat(big_table, list)
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
--	TablePrint(res)
	return res
end



-- Here we demonstrate some fancy argument handling for the convenience of the users

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
	return cm_add_library(library_name, unpack(flattened_list))
end




