
local function add_test_macro(name, command)
	add_test(name, CMAKE_CTEST_COMMAND,
		'--build-and-test',
		CMAKE_SOURCE_DIR .. '/Tests/' .. name,
		CMAKE_BINARY_DIR .. '/Tests/' .. name,
		'--build-two-config',
		'--build-generator ' .. CMAKE_TEST_GENERATOR,
		'--build-makeprogram' .. CMAKE_TEST_MAKEPROGRAM,
		'--build-project ' .. name,
		'--test-command ' .. command
	)
end

-- Testing
if BUILD_TESTING then
	-- Should the long tests be run?
	cmake.option(CMAKE_RUN_LONG_TESTS,
		'Should the long tests be run (such as Bootstrap).', true)
	cmake.mark_as_advanced(CMAKE_RUN_LONG_TESTS)

	if CMAKE_RUN_LONG_TESTS == true then
		cmake.option(CTEST_TEST_CTEST,
			'Should the tests that run a full sub ctest process be run?'
			false)
		cmake.mark_as_advanced(CTEST_TEST_CTEST)
	end

	add_test_macro('Simple', 'Simple')
end -- BUILD_TESTING

