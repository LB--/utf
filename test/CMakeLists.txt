
macro(simple_test _name)
	add_executable(test-${_name}
		"${_name}.cpp"
	)
	target_link_libraries(test-${_name}
		PUBLIC
			utf
	)
	add_test(
		NAME ${_name}
		COMMAND test-${_name}
	)
endmacro()

simple_test(num_code_units)
simple_test(read_code_point)
set_property(TEST read_code_point PROPERTY DEPENDS "num_code_units")
simple_test(min_code_units)
simple_test(encode_code_point)
set_property(TEST encode_code_point PROPERTY DEPENDS "min_code_units;read_code_point")

if(BUILD_EXAMPLES)
	add_test(
		NAME encode_all-success
		COMMAND example-encode_all
	)
	file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/encode_all-fail-dir/encode_all.txt/")
	add_test(
		NAME encode_all-fail
		COMMAND example-encode_all
		WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/encode_all-fail-dir"
	)
	set_property(TEST encode_all-fail PROPERTY WILL_FAIL ON)

	add_test(
		NAME num_code_points-txt
		COMMAND example-num_code_points "encode_all.txt"
	)
	set_property(TEST num_code_points-txt PROPERTY DEPENDS "encode_all-success")
	add_test(
		NAME num_code_points-self
		COMMAND example-num_code_points $<TARGET_FILE:example-num_code_points>
	)
	add_test(
		NAME num_code_points-fail0
		COMMAND example-num_code_points
	)
	set_property(TEST num_code_points-fail0 PROPERTY WILL_FAIL ON)
	add_test(
		NAME num_code_points-fail2
		COMMAND example-num_code_points a b
	)
	set_property(TEST num_code_points-fail2 PROPERTY WILL_FAIL ON)
endif()
