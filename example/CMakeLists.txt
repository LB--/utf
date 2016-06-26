
macro(simple_example _name)
	add_executable(example-${_name}
		"${_name}.cpp"
	)
	target_link_libraries(example-${_name}
		PUBLIC
			utf
	)
endmacro()

simple_example(num_code_points)
simple_example(encode_all)
