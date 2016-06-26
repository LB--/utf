#include "utf.hpp"

#include <cstdint>
#include <cstdlib>
#include <fstream>

/**
 * Outputs a file with all code points up to 10FFFF
 */
int main()
{
	if(std::ofstream out {"encode_all.txt", std::ios::out|std::ios::binary|std::ios::trunc})
	{
		for(std::uint32_t cp = 0; cp <= 0x10FFFF; ++cp)
		{
			out << cp << ": \"" << LB::utf::encode_code_point<char>(cp) << "\"\n";
		}

		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}
