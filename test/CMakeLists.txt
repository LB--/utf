
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
