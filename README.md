utf [![travis](https://travis-ci.org/LB--/utf.svg?branch=0)](https://travis-ci.org/LB--/utf)
===

My personal C++14 UTF-8/16/32/64 library in the public domain.
It supports arbitrary header length (and therefore supports code points that require more than four code units in UTF-8).
Interaction is via iterators - as long as `operator*()` returns values with native endianness, everything works.

## Usage
### CMake
From the `cmake` directory, copy the `FindLB` directory to a place in your `CMAKE_MODULE_PATH`.
Then, add `find_package(LB/utf REQUIRED)` to your CMake script.
You may need to set the CMake variable `LB/utf_ROOT` if you installed to a nonstandard location.
Finally, link to the `LB::utf` imported target with `target_link_libraries()`.

### C++
`#include <LB/utf/utf.hpp>`

TODO
