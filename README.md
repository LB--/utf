utf [![travis](https://travis-ci.org/LB--/utf.svg?branch=0)](https://travis-ci.org/LB--/utf)
===

My personal C++14 UTF-8/16/32/64 library in the public domain.
It supports arbitrary header length (and therefore supports code points that require more than four code units in UTF-8).
Interaction is via iterators - as long as `operator*()` returns values with native endianness, everything works.

This library is fully compatible with UTF-8, but supports an extension that allows storing code points of any arbitrary value (even huge values of over 16 bytes).
The same basic rules of UTF-8 apply:
* Code units that start with `0b0` are always 7-bit ASCII
* Code units that start with `0b11` are always the start of a multi-code-unit sequence
* Code units that start with `0b10` should always be a continuation of a multi-code-unit sequence

However, the official UTF-8 standard limits code points to at most U+10FFFF, which uses four 8-bit code units.
Even the original proposal was limited to six 8-bit code units.
In order to understand what happens when we want to store really large values in a code point, we need to talk about header overflow.

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

## Usage
### CMake
From the `cmake` directory, copy the `FindLB` directory to a place in your `CMAKE_MODULE_PATH`.
Then, add `find_package(LB/utf REQUIRED)` to your CMake script.
You may need to set the CMake variable `LB/utf_ROOT` if you installed to a nonstandard location.
Finally, link to the `LB::utf` imported target with `target_link_libraries()`.

### C++
`#include <LB/utf/utf.hpp>`

TODO
