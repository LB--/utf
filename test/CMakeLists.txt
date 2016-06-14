
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
