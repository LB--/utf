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
	}
}

#endif
