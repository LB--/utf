#include "utf.hpp"

#include <algorithm>
#include <bitset>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <tuple>
#include <vector>

struct test final
{
	using input_t = std::vector<std::uintmax_t>;
	input_t input;
	using output_t = std::tuple<std::size_t, std::uintmax_t>;
	output_t output;
};

int result = EXIT_SUCCESS;
template<typename code_unit_t>
void validate(test const &t, test::output_t output)
{
	static constexpr std::size_t NUM_BITS = sizeof(code_unit_t)*CHAR_BIT;
	static constexpr std::size_t MAX_NUM_BITS = sizeof(std::uintmax_t)*CHAR_BIT;
	if(output != t.output && std::get<0>(t.output) != 0)
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
			std::cout << "0b" << std::bitset<NUM_BITS>{v};
		}
		auto const a = std::bitset<MAX_NUM_BITS>{std::get<1>(output)}.to_string();
		auto const b = std::bitset<MAX_NUM_BITS>{std::get<1>(t.output)}.to_string();
		auto trim = std::min(a.find('1'), b.find('1'));
		if(trim == std::string::npos)
		{
			trim = MAX_NUM_BITS-1;
		}
		std::cout
			<< "} -> <" << std::get<0>(output) << ", 0b" << a.substr(trim)
			<< "> != <" << std::get<0>(t.output) << ", 0b" << b.substr(trim)
			<< ">" << std::endl;
	}
}

template<typename code_unit_t>
auto cast(test::input_t const &input)
-> std::vector<code_unit_t>
{
	std::vector<code_unit_t> r;
	for(auto const v : input)
	{
		r.push_back(static_cast<code_unit_t>(v));
	}
	return r;
}

template<typename code_unit_t>
void run_tests(std::vector<test> const &tests)
{
	std::cout << "Valid sequences - just right" << std::endl;
	for(auto const &t : tests)
	{
		auto const input = cast<code_unit_t>(t.input);
		std::uintmax_t cp {};
		auto it = LB::utf::read_code_point(std::cbegin(input), std::cend(input), cp).first;
		validate<code_unit_t>(t, {it-std::cbegin(input), cp});
	}
	std::cout << "Valid sequences - with extra" << std::endl;
	for(auto const &t : tests)
	{
		auto input = cast<code_unit_t>(t.input);
		input.push_back(0);
		std::uintmax_t cp {};
		auto it = LB::utf::read_code_point(std::cbegin(input), std::cend(input), cp).first;
		validate<code_unit_t>(t, {it-std::cbegin(input), cp});
	}
	std::cout << "Invalid sequences - too short" << std::endl;
	for(auto t : tests)
	{
		auto input = cast<code_unit_t>(t.input);
		if(input.size() > 0)
		{
			input.pop_back();
			t.output = {0, 0};
		}
		std::uintmax_t cp {};
		auto it = LB::utf::read_code_point(std::cbegin(input), std::cend(input), cp).first;
		validate<code_unit_t>(t, {it-std::cbegin(input), cp});
	}
	std::cout << "Invalid sequences - WAY too short" << std::endl;
	for(auto t : tests)
	{
		auto input = cast<code_unit_t>(t.input);
		if(input.size() > 1)
		{
			input.resize(1);
			t.output = {0, 0};
		}
		std::uintmax_t cp {};
		auto it = LB::utf::read_code_point(std::cbegin(input), std::cend(input), cp).first;
		validate<code_unit_t>(t, {it-std::cbegin(input), cp});
	}
	std::cout << "Invalid sequences - unexpected 0b11 header" << std::endl;
	for(auto t : tests)
	{
		auto input = cast<code_unit_t>(t.input);
		if(input.size() > 0)
		{
			input.back() = (std::uintmax_t{0b11} << (sizeof(code_unit_t)*CHAR_BIT - 2));
			t.output = {0, 0};
		}
		std::uintmax_t cp {};
		auto it = LB::utf::read_code_point(std::cbegin(input), std::cend(input), cp).first;
		validate<code_unit_t>(t, {it-std::cbegin(input), cp});
	}
	std::cout << "Invalid sequences - unexpected 0b0 header" << std::endl;
	for(auto t : tests)
	{
		auto input = cast<code_unit_t>(t.input);
		if(input.size() > 0)
		{
			input.back() = 0;
			t.output = {0, 0};
		}
		std::uintmax_t cp {};
		auto it = LB::utf::read_code_point(std::cbegin(input), std::cend(input), cp).first;
		validate<code_unit_t>(t, {it-std::cbegin(input), cp});
	}
}

constexpr std::uintmax_t ones(std::size_t n) noexcept
{
	std::uintmax_t v {};
	while(n--)
	{
		v <<= 1;
		v |= 1;
	}
	return v;
}

int main()
{
	{
		static constexpr std::uintmax_t _0 = 0b10000000;
		static constexpr std::uintmax_t _1 = 0b10111111;
		run_tests<std::int8_t>(
		{
			{{}, {0, {}}},
			{{0b00000000}, {1, 0b0000000}},
			{{0b10000000}, {0, {}}},
			{{0b01111111}, {1, 0b1111111}},
			{{0b11000000, _0}, {2, 0}},
			{{0b11011111, _1}, {2, ones(1 + 5*2)}},
			{{0b11100000, _0, _0}, {3, 0}},
			{{0b11101111, _1, _1}, {3, ones(1 + 5*3)}},
			{{0b11110000, _0, _0, _0}, {4, 0}},
			{{0b11110111, _1, _1, _1}, {4, ones(1 + 5*4)}},
			{{0b11111000, _0, _0, _0, _0}, {5, 0}},
			{{0b11111011, _1, _1, _1, _1}, {5, ones(1 + 5*5)}},
			{{0b11111100, _0, _0, _0, _0, _0}, {6, 0}},
			{{0b11111101, _1, _1, _1, _1, _1}, {6, ones(1 + 5*6)}},
			{{0b11111110, _0, _0, _0, _0, _0, _0}, {7, 0}},
			{{0b11111110, _1, _1, _1, _1, _1, _1}, {7, ones(1 + 5*7)}},
			{{0b11111111, 0b10000000, _0, _0, _0, _0, _0, _0, _0}, {9, 0}},
			{{0b11111111, 0b10011111, _1, _1, _1, _1, _1, _1, _1}, {9, ones(2 + 5*9)}},
			{{0b11111111, 0b10100000, _0, _0, _0, _0, _0, _0, _0, _0}, {10, 0}},
			{{0b11111111, 0b10101111, _1, _1, _1, _1, _1, _1, _1, _1}, {10, ones(2 + 5*10)}},
			{{0b11111111, 0b10110000, _0, _0, _0, _0, _0, _0, _0, _0, _0}, {11, 0}},
			{{0b11111111, 0b10110111, _1, _1, _1, _1, _1, _1, _1, _1, _1}, {11, ones(2 + 5*11)}},
			{{0b11111111, 0b10111000, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0}, {12, 0}},
			{{0b11111111, 0b10111011, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1}, {12, ones(2 + 5*12)}},
			//more than 64-bits
			{{0b11111111, 0b10111100, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0}, {13, 0}},
			{{0b11111111, 0b10111101, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1}, {13, ones(2 + 5*13)}},
			{{0b11111111, 0b10111110, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0}, {14, 0}},
			{{0b11111111, 0b10111110, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1}, {14, ones(2 + 5*14)}},
			{{0b11111111, 0b10111111, 0b10000000, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0}, {16, 0}},
			{{0b11111111, 0b10111111, 0b10011111, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1}, {16, ones(3 + 5*16)}},
			{{0b11111111, 0b10111111, 0b10100000, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0}, {17, 0}},
			{{0b11111111, 0b10111111, 0b10101111, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1}, {17, ones(3 + 5*17)}},
			{{0b11111111, 0b10111111, 0b10110000, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0}, {18, 0}},
			{{0b11111111, 0b10111111, 0b10110111, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1}, {18, ones(3 + 5*18)}},
			{{0b11111111, 0b10111111, 0b10111000, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0}, {19, 0}},
			{{0b11111111, 0b10111111, 0b10111011, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1}, {19, ones(3 + 5*19)}},
			{{0b11111111, 0b10111111, 0b10111100, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0}, {20, 0}},
			{{0b11111111, 0b10111111, 0b10111101, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1}, {20, ones(3 + 5*20)}},
			{{0b11111111, 0b10111111, 0b10111110, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0}, {21, 0}},
			{{0b11111111, 0b10111111, 0b10111110, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1}, {21, ones(3 + 5*21)}},
		});
	}
	{
		static constexpr std::uintmax_t _0 = 0b10000000'00000000;
		static constexpr std::uintmax_t _1 = 0b10111111'11111111;
		run_tests<std::int16_t>(
		{
			{{}, {0, {}}},
			{{0b00000000'00000000}, {1, 0b0000000'00000000}},
			{{0b10000000'00000000}, {0, {}}},
			{{0b01111111'11111111}, {1, 0b1111111'11111111}},
			{{0b11000000'00000000, _0}, {2, 0}},
			{{0b11011111'11111111, _1}, {2, ones(1 + 13*2)}},
			{{0b11100000'00000000, _0, _0}, {3, 0}},
			{{0b11101111'11111111, _1, _1}, {3, ones(1 + 13*3)}},
			{{0b11110000'00000000, _0, _0, _0}, {4, 0}},
			{{0b11110111'11111111, _1, _1, _1}, {4, ones(1 + 13*4)}},
			//more than 64-bits
			{{0b11111000'00000000, _0, _0, _0, _0}, {5, 0}},
			{{0b11111011'11111111, _1, _1, _1, _1}, {5, ones(1 + 13*5)}},
			{{0b11111100'00000000, _0, _0, _0, _0, _0}, {6, 0}},
			{{0b11111101'11111111, _1, _1, _1, _1, _1}, {6, ones(1 + 13*6)}},
			{{0b11111110'00000000, _0, _0, _0, _0, _0, _0}, {7, 0}},
			{{0b11111110'11111111, _1, _1, _1, _1, _1, _1}, {7, ones(1 + 13*7)}},
			{{0b11111111'00000000, _0, _0, _0, _0, _0, _0, _0}, {8, 0}},
			{{0b11111111'01111111, _1, _1, _1, _1, _1, _1, _1}, {8, ones(1 + 13*8)}},
			{{0b11111111'10000000, _0, _0, _0, _0, _0, _0, _0, _0}, {9, 0}},
			{{0b11111111'10111111, _1, _1, _1, _1, _1, _1, _1, _1}, {9, ones(1 + 13*9)}},
			{{0b11111111'11000000, _0, _0, _0, _0, _0, _0, _0, _0, _0}, {10, 0}},
			{{0b11111111'11011111, _1, _1, _1, _1, _1, _1, _1, _1, _1}, {10, ones(1 + 13*10)}},
			{{0b11111111'11100000, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0}, {11, 0}},
			{{0b11111111'11101111, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1}, {11, ones(1 + 13*11)}},
			{{0b11111111'11110000, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0}, {12, 0}},
			{{0b11111111'11110111, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1}, {12, ones(1 + 13*12)}},
			{{0b11111111'11111000, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0}, {13, 0}},
			{{0b11111111'11111011, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1}, {13, ones(1 + 13*13)}},
			{{0b11111111'11111100, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0}, {14, 0}},
			{{0b11111111'11111101, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1}, {14, ones(1 + 13*14)}},
			{{0b11111111'11111110, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0}, {15, 0}},
			{{0b11111111'11111110, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1}, {15, ones(1 + 13*15)}},
			{{0b11111111'11111111, 0b10000000'00000000, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0}, {17, 0}},
			{{0b11111111'11111111, 0b10011111'11111111, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1}, {17, ones(2 + 13*17)}},
			{{0b11111111'11111111, 0b10100000'00000000, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0}, {18, 0}},
			{{0b11111111'11111111, 0b10101111'11111111, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1}, {18, ones(2 + 13*18)}},
			{{0b11111111'11111111, 0b10110000'00000000, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0}, {19, 0}},
			{{0b11111111'11111111, 0b10110111'11111111, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1}, {19, ones(2 + 13*19)}},
			{{0b11111111'11111111, 0b10111000'00000000, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0}, {20, 0}},
			{{0b11111111'11111111, 0b10111011'11111111, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1}, {20, ones(2 + 13*20)}},
			{{0b11111111'11111111, 0b10111100'00000000, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0}, {21, 0}},
			{{0b11111111'11111111, 0b10111101'11111111, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1}, {21, ones(2 + 13*21)}},
		});
	}

	return result;
}
