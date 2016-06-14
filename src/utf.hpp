#ifndef LB_utf_utf_HeaderPlusPlus
#define LB_utf_utf_HeaderPlusPlus

#include <climits>
#include <cstdint>
#include <type_traits>

namespace LB
{
	namespace utf
	{
		template<typename code_unit_iterator>
		std::size_t num_code_units(code_unit_iterator it, code_unit_iterator const last)
		{
			if(it == last)
			{
				return 0;
			}

			using code_unit_t = std::make_unsigned_t<std::remove_cv_t<std::remove_reference_t<decltype(*it)>>>;
			static constexpr std::size_t NUM_BITS = sizeof(code_unit_t)*CHAR_BIT;
			code_unit_t v = static_cast<code_unit_t>(*it);
			static constexpr code_unit_t start = code_unit_t{1} << NUM_BITS-1;
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
			std::size_t len = 1;
			while(v & test)
			{
				++len;
				if(test == 1) //end of current code unit
				{
					test = start;
					if(++it == last || !((v = static_cast<code_unit_t>(*it)) & test)) //unexpected end of sequence
					{
						return 0;
					}
					++len;
				}
				test >>= 1;
			}
			return len;
		}
	}
}

#endif
