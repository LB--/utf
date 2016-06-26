#include "utf.hpp"

#include <cstdint>
#include <cstdlib>
#include <functional>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <utility>

/**
 * Give a UTF-8 filename for a count of valid code points and invalid code units
 */
int main(int nargs, char const *const *args)
{
	if(nargs != 2)
	{
		std::cerr << "Please pass the filename as an argument" << std::endl;
		return EXIT_FAILURE;
	}
	std::string const utf_string = [&]
	{
		if(std::ifstream in {args[1], std::ios::in|std::ios::binary})
		{
			return std::string(std::istreambuf_iterator<char>(in), {});
		}
		std::cerr << "Cannot find file: " << args[1] << std::endl;
		std::exit(EXIT_FAILURE);
	}();

	std::size_t valid = 0
	,           invalid = 0;
	for(auto it = std::cbegin(utf_string), end = std::cend(utf_string); it != end; )
	{
		std::uint32_t cp {};
		std::size_t num_code_units {};
		auto r = std::make_pair(std::ref(it), std::ref(num_code_units));
		r = LB::utf::read_code_point(it, end, cp);

		if(num_code_units)
		{
			++valid;
		}
		else
		{
			++invalid;
			++it;
		}
	}

	std::cout
		<< "Number of original code units: " << utf_string.size()
		<< '\n'
		<< "Number of valid code points: " << valid
		<< '\n'
		<< "Number of invalid code units: " << invalid
		<< std::endl;
	return EXIT_SUCCESS;
}
