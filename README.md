utf [![travis](https://travis-ci.org/LB--/utf.svg?branch=0)](https://travis-ci.org/LB--/utf)
===

My personal C++14 UTF-8 library in the public domain.
It supports arbitrary header length (and therefore supports code points that require more than four code units in UTF-8).
Interaction is via iterators - as long as `operator*()` returns values with native endianness, everything works.

### UTF-8
This library is fully compatible with UTF-8, but supports an extension that allows storing code points of any arbitrary value (even huge values of over 16 bytes).
The same basic rules of UTF-8 apply:
* Code units that start with `0b0` are always 7-bit ASCII
* Code units that start with `0b11` are always the start of a multi-code-unit sequence
* Code units that start with `0b10` should always be a continuation of a multi-code-unit sequence

However, the official UTF-8 standard limits code points to at most U+10FFFF, which uses four 8-bit code units.
Even the original proposal was limited to six 8-bit code units.
In order to understand what happens when we want to store really large values in a code point, we need to talk about header overflow.

### Arbitrary Code Point Storage
The header of a multi-code-unit sequence starts with `0b11`, meaning two code units, and appends `0b1` for each additional code unit that makes up the code point.
The header ends with `0b0` and the remaining bits make up the actual code point.
With 8-bits, you can have up to `0b11111110` (7 code units) before the header can no longer fit in a single code unit.
To store larger values, the header needs to overflow into the next code unit, but we also want to keep the property that code units starting with `0b10` are a continuation, since this allows backtracking in a UTF-8 sequence.
Thus, the first header size that spills over into multiple 8-bit code units looks like `0b11111111, 0b100xxxxx` and means there will be 9 code units.
Note that it is not possible for there to be 8 code units - Either the header doesn't spill over and we have 7 code units, or the header does spill over and we have 9.
This is intentional, for it retains the ability to visually inspect the binary and know how many code points there will be by simply counting the `1`s until the terminating `0` (ignoring the `0` in the second most significant bit).

Using this technique we retain full compatibility with UTF-8 while also supporting excessively large code points.
Although the Unicode standard doesn't currently have enough code points to warrant such support, this library will be ready for the day it does (assuming the problem is still relevant by then and is solved the same way then as it is here).
Just be aware that currently if you actually store any code points that require more than four or six 8-bit code units, the resulting encoded code points will likely not be recognized or supported by 99% of software that exists today.

### UTF-16
**This library is NOT compatible with traditional UTF-16 encodings!**
If you use a 16-bit code unit type, it is treated as if UTF-8 simply had more bits, which unfortunately is not how UTF-16 works.
UTF-16 uses a more complicated encoding which does not share the same properties as UTF-8.
[The only time you should use traditional UTF-16 is when interacting with the Windows API.](http://utf8everywhere.org/#windows)
If you have UTF-16 strings that you want to use with this library, first [convert them to UTF-8](https://gist.github.com/LB--/735a911302ee9891a431514f6978e0a6) and then pass them to this library. 


### Other uses
UTF-style encoding isn't just useful for storage of Unicode code points - you can store anything you want with this library.
UTF-style encoding is useful when the values can normally fit in a single code unit but may occasionally need to occupy multiple code units.
Text storage is just one such example of a good use case.
Additionally, there is relatively low overhead - on average just 3 bits per additional code unit.

### UTF-32
Sure!
After 31 bits, you'll start to use more than one code unit per code point.

### Endianness
If you are working with single-byte code units (e.g. UTF-8), you don't need to worry about endianness.
Otherwise, always ensure that the code units you pass in have _native_ endianness.
This library makes no attempt to detect or otherwise acknowledge endianness, and leaves it up to the user to convert to or from native endianness as necessary.

### UTF-64
Sure!
After 63 bits, you'll start to use more than one code unit per code point.
To really take advantage of this, though, you will want to use a custom unsigned numeric type, most likely from the C++ bignum library of your choice.
Just make sure the type you use overloads the proper C++ operators.

### UTF-128, UTF-256, UTF-512, UTF-1024, etc
Sure, I guess?
Does your compiler even have any primitive integer types that large?
Code point types have to be primitive integer types so that they can be passed to [`std::make_unsigned`](http://en.cppreference.com/w/cpp/types/make_unsigned), though I guess you could defenestrate conformance and specialize `std::make_unsigned` on your own types.
If your compiler has primitive integer types that large or you want to break conformance, go for it.
It _should_ work as long as `sizeof(your_type)` is correct.
I think.

### UTF-not-a-power-of-2 (e.g. UTF-24)
If you have a system with one or more primitive integer types that are not powers of two but are still multiples of `CHAR_BIT`, this library will still work.
(For example, I have actually worked on a system with a 24-bit `short long` primitive integer type).
If you have a weird system where `CHAR_BIT` is not `8`, this library should still work.
You only need at least 3 bits per code unit to use UTF-style encoding, though at that point the headers would take up over half of the code points.
If you have a weirder system where some primitive integer types are not multiples of `CHAR_BIT`, then sorry, you won't be able to use this library, or a C++ compiler for that matter.

## Usage
Known good compilers are GCC 6 and Visual Studio 2015 Update 2.
For Visual Studio, you may need to remove the `constexpr` declarations from some functions if you get errors related to that.

### CMake
Building, installation, and linking to your own project are done with [CMake](https://cmake.org/).
You need at least CMake 3.4.

This is a header-only library, so if you don't want to build, install, and link with CMake, you don't need to - just copy `utf.hpp` wherever you want.

#### Configuring and Building
Create an empty directory to use with CMake, and run CMake there with the path to this repository.
You can simply create a `build` subfolder within this repository and run `cmake ..` or the CMake GUI.
Specify the installation path of your choice with [`-DCMAKE_INSTALL_PREFIX=path`](https://cmake.org/cmake/help/latest/variable/CMAKE_INSTALL_PREFIX.html) if the default on your system is not to your liking.
The tests are enabled by default, but if you aren't interested in those, set `-DBUILD_TESTS=OFF`.
Also, be sure to set [`CMAKE_BUILD_TYPE`](https://cmake.org/cmake/help/latest/variable/CMAKE_BUILD_TYPE) appropriately.

Then, simply build with the generator of your choice, or ask CMake to do it for you with `cmake --build .`.
Since this is a header-only library, the build step is only really for building the tests.

#### Installation and Linking
First, use CMake's install step to install the project (e.g. `cmake --build . --target install`).
From the `cmake` directory, copy the `FindLB` directory to a place in your [`CMAKE_MODULE_PATH`](https://cmake.org/cmake/help/latest/variable/CMAKE_MODULE_PATH.html).
Then, add `find_package(LB/utf REQUIRED)` to your CMake script.
You may need to set the CMake variable `LB/utf_ROOT` if you installed to a nonstandard location.
Finally, link to the `LB::utf` imported target with [`target_link_libraries()`](https://cmake.org/cmake/help/latest/command/target_link_libraries.html).

### C++
`#include <LB/utf/utf.hpp>`  
All names are in the `LB::utf::` namespace.

#### `num_code_units`
Reads the header of a UTF sequence and returns the number of code units that make up the sequence, or 0 if the sequence is invalid.
```cpp
template<typename code_unit_iterator>
auto num_code_units(code_unit_iterator it, code_unit_iterator const last, bool verify = false)
noexcept(noexcept(it == last) && noexcept(*it) && noexcept(++it))
-> std::size_t
```
`code_unit_iterator` must be at least an input iterator, and `*it` must return an integral type whose value has native endianness.
`it` should refer to the first code unit that makes up the sequence, and `last` should be the one-past-the-end iterator for the sequence or container (e.g. `std::cend(utf_string)`).
If `verify` is true, the function examines additional code units after the ones that contain the header to ensure the sequence is valid.

#### `read_code_point`
Decodes a UTF sequence to yield the original code point, and returns both an iterator to the start of the next sequence and the number of read code units if successful.
```cpp
template<typename code_unit_iterator, typename code_point_t>
auto read_code_point(code_unit_iterator it, code_unit_iterator const last, code_point_t &cp)
noexcept(noexcept(num_code_units(it, last)) && noexcept(it == last) && noexcept(*it) && noexcept(++it) && std::is_nothrow_copy_constructible<code_unit_iterator>::value && noexcept(cp = *it) && noexcept(cp = {}) && noexcept(cp <<= std::size_t{}) && noexcept(cp |= unsigned_code_unit_t<code_unit_iterator>{}))
-> std::pair<code_unit_iterator, std::size_t>
```
`code_unit_iterator` must be at least an input iterator with multi-pass support, and `*it` must return an integral type whose value has native endianness.
`it` should refer to the first code unit that makes up the sequence, and `last` should be the one-past-the-end iterator for the sequence or container (e.g. `std::cend(utf_string)`).
`cp` is an output parameter for the code point to be stored in, and for obvious reasons must be large enough to contain any Unicode code point, and must be unsigned, though does not necessarily have to be a primitive type.
The operations `cp` must support are those shown in the `noexcept` specification.
If the sequence is invalid, the function returns the original value of `it` and `0`, and the value of `cp` is undefined.

#### `min_code_units`
Calculates the minimum number of code units required to store a code point.
```cpp
template<typename code_unit_t, typename code_point_t>
constexpr auto min_code_units(code_point_t cp)
noexcept(noexcept(cp < std::make_unsigned_t<code_unit_t>{}) && noexcept(!(cp == 0u)) && noexcept(cp >>= std::size_t{}))
-> std::size_t
```
`code_unit_t` must be an integral type.
`code_point_t` must be an unsigned integral type, or an unsigned-integer-like type which must support the operations shown in the `noexcept` specification.
The return value is guaranteed to be non-zero.

#### `encode_code_point`
Encodes a code point as a UTF sequence and returns it in the form of a string.
```cpp
template<typename code_unit_t, typename code_point_t>
auto encode_code_point(code_point_t cp)
-> std::basic_string<code_unit_t>
```
`code_unit_t` must be an integral type.
`code_point_t` must be an unsigned integral type, or and unsigned-integer-like type that must support , `operator==(unsigned)`, `operator>>=(std::size_t)`, `operator&(unsigned)`, must be convertible to `code_unit_t`, and must be able to be passed to `min_code_units`.
This function does not have any `noexcept` specification because `std::basic_string` does not - the memory allocation could throw an exception.
