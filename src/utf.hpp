#ifndef LB_utf_utf_HeaderPlusPlus
#define LB_utf_utf_HeaderPlusPlus

#include <climits>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace LB
{
	namespace utf
	{
		template<typename code_unit_iterator>
		auto num_code_units(code_unit_iterator it, code_unit_iterator const last, bool verify = false)
		noexcept(noexcept(it == last) && noexcept(*it) && noexcept(++it))
		-> std::size_t
		{
			if(it == last)
			{
				return 0;
			}

			using code_unit_t = std::make_unsigned_t<std::remove_cv_t<std::remove_reference_t<decltype(*it)>>>;
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
		noexcept(noexcept(num_code_units(it, last)) && noexcept(it == last) && noexcept(*it) && noexcept(++it) && std::is_nothrow_copy_constructible<code_unit_iterator>::value && std::is_nothrow_default_constructible<code_point_t>::value && std::is_nothrow_copy_assignable<code_point_t>::value && noexcept(cp <<= 1) && noexcept(cp |= 1))
		-> code_unit_iterator
		{
			if(std::size_t n = num_code_units(it, last))
			{
				if(n == 1)
				{
					cp = *it;
					return ++it;
				}
				cp = {};

				using code_unit_t = std::make_unsigned_t<std::remove_cv_t<std::remove_reference_t<decltype(*it)>>>;
				static constexpr std::size_t NUM_BITS = sizeof(code_unit_t)*CHAR_BIT;
				code_unit_iterator const first = it;

				std::size_t skip_bits = n+1;
				while(skip_bits >= NUM_BITS)
				{
					skip_bits -= NUM_BITS-1;
					--n;
					if(++it == last) //unexpected end of sequence
					{
						return first;
					}
				}

				code_unit_t v = static_cast<code_unit_t>(*it);
				code_unit_t mask = (std::numeric_limits<code_unit_t>::max() >> skip_bits); //skip initial header
				for(;;)
				{
					cp <<= NUM_BITS-2;
					cp |= (*it & mask);
					mask = (std::numeric_limits<code_unit_t>::max() >> 2); //skip continuation header

					--n, ++it;
					if(!n)
					{
						break;
					}

					if(it == last || !((v = static_cast<code_unit_t>(*it)) & (code_unit_t{0b1} << NUM_BITS-1))) //unexpected end of sequence
					{
						return first;
					}
					else
					{
						if(v & (code_unit_t{0b1} << NUM_BITS-2)) //should have been a continuation but wasn't
						{
							return first;
						}
					}
				}
			}
			return it;
		}

		template<typename code_unit_t, typename code_point_t>
		constexpr auto min_code_units(code_point_t cp)
		noexcept(noexcept(cp < 1) && noexcept(!(cp == 1)) && noexcept(cp >>= 1))
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
			while(!(cp == 0))
			{
				cp >>= 1;
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
	}
}

#endif
