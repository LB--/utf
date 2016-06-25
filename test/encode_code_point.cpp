#include "utf.hpp"

#include <bitset>
#include <cstdint>
#include <cstdlib>
#include <iostream>

int result = EXIT_SUCCESS;

template<typename code_unit_t>
void run_tests()
{
	static constexpr std::size_t NUM_BITS = sizeof(code_unit_t)*CHAR_BIT;
	static constexpr std::size_t MAX_NUM_BITS = sizeof(std::uintmax_t)*CHAR_BIT;
	for(std::uintmax_t cp = 0; ; (cp <<= 1) |= 0b1)
	{
		auto const str = LB::utf::encode_code_point<code_unit_t>(cp);
		std::uintmax_t cp2 = 0;
		LB::utf::read_code_point(std::cbegin(str), std::cend(str), cp2);
		if(cp != cp2)
		{
			result = EXIT_FAILURE;
			auto const v = std::bitset<MAX_NUM_BITS>{cp}.to_string();
			auto trim = v.find('1');
			if(trim == std::string::npos)
			{
				trim = MAX_NUM_BITS-1;
			}
			std::cout << "Fail: 0b" << v.substr(trim) << " (" << cp << ") encoded as {";
			bool first = true;
			for(auto const cu : str)
			{
				if(first)
				{
					first = false;
				}
				else
				{
					std::cout << ", ";
				}
				std::cout << "0b" << std::bitset<NUM_BITS>{static_cast<std::uintmax_t>(cu)};
			}
			std::cout << "} (" << cp2 << ")" << std::endl;
		}

		if(cp & (std::uintmax_t{0b1} << MAX_NUM_BITS-1))
		{
			break;
		}
	}
}

int main()
{
	run_tests<std::int8_t>();
	run_tests<std::int16_t>();
	run_tests<std::int32_t>();

	return result;
}
