#include "utf.hpp"

#include <bitset>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <vector>

int main()
{
	int result = EXIT_SUCCESS;
	struct test final
	{
		std::vector<unsigned> const input;
		std::size_t const output;
	};
	std::vector<test> const tests
	{
		{{}, 0},
		{{0b00000000}, 1},
		{{0b10000000}, 0},
		{{0b01000000}, 1},
		{{0b00100000}, 1},
		{{0b00010000}, 1},
		{{0b00001000}, 1},
		{{0b00000100}, 1},
		{{0b00000010}, 1},
		{{0b00000001}, 1},
		{{0b11000000}, 2},
		{{0b11100000}, 3},
		{{0b11110000}, 4},
		{{0b11111000}, 5},
		{{0b11111100}, 6},
		{{0b11111110}, 7},
		{{0b11111111}, 0},
		{{0b11111111, 0b00000000}, 0},
		{{0b11111111, 0b10000000}, 9},
		{{0b11111111, 0b11000000}, 0},
		{{0b11111111, 0b10100000}, 10},
		{{0b11111111, 0b10110000}, 11},
		{{0b11111111, 0b10111000}, 12},
		{{0b11111111, 0b10111100}, 13},
		{{0b11111111, 0b10111110}, 14},
		{{0b11111111, 0b10111111}, 0},
		{{0b11111111, 0b10111111, 0b00000000}, 0},
		{{0b11111111, 0b10111111, 0b10000000}, 16},
		{{0b11111111, 0b10111111, 0b11000000}, 0},
		{{0b11111111, 0b10111111, 0b10100000}, 17},
		{{0b11111111, 0b10111111, 0b10110000}, 18},
		{{0b11111111, 0b10111111, 0b10111000}, 19},
	};
	for(auto const t : tests)
	{
		std::vector<char> input;
		input.reserve(t.input.size());
		for(auto const v : t.input)
		{
			input.push_back(static_cast<char>(v));
		}
		auto output = LB::utf::num_code_units(std::cbegin(input), std::cend(input));
		if(output != t.output)
		{
			result = EXIT_FAILURE;
			std::cout << "Fail: {";
			bool first = true;
			for(auto const v : t.input)
			{
				if(first)
				{
					first = false;
				}
				else
				{
					std::cout << ", ";
				}
				std::cout << "0b" << std::setfill('0') << std::setw(8) << std::bitset<8>{v};
			}
			std::cout << "} -> " << output << " != " << t.output << std::endl;
		}
	}
	return result;
}
