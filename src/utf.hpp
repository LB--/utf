#ifndef LB_utf_utf_HeaderPlusPlus
#define LB_utf_utf_HeaderPlusPlus

#include <climits>
#include <cstdint>
#include <iterator>
#include <limits>
#include <string>
#include <type_traits>
#include <utility>

namespace LB
{
	namespace utf
	{
		template<typename code_unit_iterator>
		using unsigned_code_unit_t = std::make_unsigned_t<typename std::iterator_traits<code_unit_iterator>::value_type>;

		template<typename code_unit_iterator>
		auto num_code_units(code_unit_iterator it, code_unit_iterator const last, bool verify = false)
		noexcept(noexcept(it == last) && noexcept(*it) && noexcept(++it))
		-> std::size_t
		{
			if(it == last)
			{
				return 0;
			}

			using code_unit_t = unsigned_code_unit_t<code_unit_iterator>;
			static constexpr std::size_t NUM_BITS = sizeof(code_unit_t)*CHAR_BIT;
			static constexpr code_unit_t start = code_unit_t{0b1} << NUM_BITS-1;

			code_unit_t v = static_cast<code_unit_t>(*it);
			code_unit_t test = start;

			if(!(v & test)) //code point made of exactly one code unit
			{
				return 1;
			}
			else
			{
				test >>= 1;
				if(!(v & test)) //unexpected continuation
				{
					return 0;
				}
			}

			//read the header to determine the length
			std::size_t len = 1
			,           skip_bytes = 1;
			while(v & test)
			{
				if(test == 1) //end of current code unit
				{
					test = start;
					if(++it == last || !((v = static_cast<code_unit_t>(*it)) & test)) //unexpected end of sequence
					{
						return 0;
					}
					else
					{
						test >>= 1;
						if(v & test) //should have been a continuation but wasn't
						{
							return 0;
						}
					}
					++len;
					++skip_bytes;
				}
				++len;
				test >>= 1;
			}

			if(verify)
			{
				//verify that the other continuation code units exist
				for(std::size_t i = 0; i < len-skip_bytes; ++i)
				{
					test = start;
					if(++it == last || !((v = static_cast<code_unit_t>(*it)) & test)) //unexpected end of sequence
					{
						return 0;
					}
					else
					{
						test >>= 1;
						if(v & test) //should have been a continuation but wasn't
						{
							return 0;
						}
					}
				}
			}

			return len;
		}

		template<typename code_unit_iterator, typename code_point_t>
		auto read_code_point(code_unit_iterator it, code_unit_iterator const last, code_point_t &cp)
		noexcept(noexcept(num_code_units(it, last)) && noexcept(it == last) && noexcept(*it) && noexcept(++it) && noexcept(cp = *it) && noexcept(cp = {}) && noexcept(cp <<= std::size_t{}) && noexcept(cp |= unsigned_code_unit_t<code_unit_iterator>{}))
		-> std::pair<code_unit_iterator, std::size_t>
		{
			std::size_t const n = num_code_units(it, last);
			if(n == 1)
			{
				cp = *it;
				++it;
			}
			else if(n)
			{
				cp = {};

				using code_unit_t = unsigned_code_unit_t<code_unit_iterator>;
				static constexpr std::size_t NUM_BITS = sizeof(code_unit_t)*CHAR_BIT;
				code_unit_iterator const first = it;

				std::size_t skip_bits = n+1
				,           remaining = n;
				while(skip_bits >= NUM_BITS)
				{
					skip_bits -= NUM_BITS-1;
					--remaining;
					if(++it == last) //unexpected end of sequence
					{
						return {first, 0u};
					}
				}

				code_unit_t v = static_cast<code_unit_t>(*it);
				code_unit_t mask = (std::numeric_limits<code_unit_t>::max() >> skip_bits); //skip initial header
				for(;;)
				{
					cp <<= static_cast<std::size_t>(NUM_BITS-2);
					cp |= static_cast<code_unit_t>(*it & mask);
					mask = (std::numeric_limits<code_unit_t>::max() >> 2); //skip continuation header

					--remaining, ++it;
					if(!remaining)
					{
						break;
					}

					if(it == last || !((v = static_cast<code_unit_t>(*it)) & (code_unit_t{0b1} << NUM_BITS-1))) //unexpected end of sequence
					{
						return {first, 0u};
					}
					else
					{
						if(v & (code_unit_t{0b1} << NUM_BITS-2)) //should have been a continuation but wasn't
						{
							return {first, 0u};
						}
					}
				}
			}
			return {it, n};
		}

		template<typename code_unit_t, typename code_point_t>
		constexpr auto min_code_units(code_point_t cp)
		noexcept(noexcept(cp < std::make_unsigned_t<code_unit_t>{}) && noexcept(!(cp == 0u)) && noexcept(cp >>= std::size_t{}))
		-> std::size_t
		{
			using code_unit_ty = std::make_unsigned_t<code_unit_t>;
			constexpr std::size_t NUM_BITS = sizeof(code_unit_ty)*CHAR_BIT;

			//no-op if we can fit the code point in a single code unit
			if(cp < static_cast<code_unit_ty>(code_unit_ty{1} << NUM_BITS-1))
			{
				return 1;
			}

			//count the number of bits we have to store
			std::size_t bits = 0;
			while(!(cp == 0u))
			{
				cp >>= std::size_t{1u};
				++bits;
			}

			//calculate how many code units are needed to store these bits plus the header bits
			std::size_t units = 2;
			while(bits + (units+1 - (units + (units-1)/NUM_BITS)/NUM_BITS) + (units-1)*2 > units*NUM_BITS)
			{
				++units;
			}

			//add extra to avoid situations which are impossible to represent
			//e.g. with 8 bit code units, we cannot represent 8, 15, 22, 29, etc.
			if((units + (units-1)/NUM_BITS)%NUM_BITS == 0)
			{
				++units;
			}

			//TODO: explain the expression (units + (units-1)/NUM_BITS)

			return units;
		}

		template<typename code_unit_t, typename code_point_t>
		auto encode_code_point(code_point_t cp)
		-> std::basic_string<code_unit_t>
		{
			using code_unit_ty = std::make_unsigned_t<std::remove_cv_t<std::remove_reference_t<code_unit_t>>>;
			static constexpr std::size_t NUM_BITS = sizeof(code_unit_ty)*CHAR_BIT;

			std::size_t const units = min_code_units<code_unit_t>(cp);
			if(units == 1)
			{
				return {static_cast<code_unit_t>(cp)};
			}

			std::basic_string<code_unit_t> code_units (units, code_unit_t{});

			//set continuation headers
			for(std::size_t i = 1; i < code_units.size(); ++i)
			{
				code_units[i] = static_cast<code_unit_t>(code_unit_ty{0b1u} << NUM_BITS-1);
			}

			//fill header bits
			if(code_units.size() < NUM_BITS)
			{
				code_units[0] = static_cast<code_unit_t>(std::numeric_limits<code_unit_ty>::max() << (NUM_BITS - code_units.size()));
			}
			else
			{
				code_units[0] = static_cast<code_unit_t>(std::numeric_limits<code_unit_ty>::max());
				std::size_t bits = code_units.size()-NUM_BITS;
				bits -= (bits-1)/(NUM_BITS-2) + 1; //include continuation header bits
				for(std::size_t i = 0; i < bits; ++i)
				{
					code_unit_ty &cu = reinterpret_cast<code_unit_ty &>(code_units[1 + i/(NUM_BITS-2)]);
					cu |= (code_unit_ty{0b1u} << ((NUM_BITS-3) - i%(NUM_BITS-2)));
				}
			}

			//store code point
			auto it = std::rbegin(code_units);
			std::size_t shift = 0;
			while(!(cp == 0u))
			{
				code_unit_ty &cu = reinterpret_cast<code_unit_ty &>(*it);
				if(cp & 0b1u)
				{
					cu |= static_cast<code_unit_ty>(code_unit_ty{0b1u} << shift);
				}
				cp >>= std::size_t{1u};

				if(++shift >= NUM_BITS-2)
				{
					++it;
					shift = 0;
				}
			}

			return code_units;
		}
	}
}

#endif
