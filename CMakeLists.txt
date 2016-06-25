cmake_minimum_required(VERSION 3.4)

set(CMAKE_CXX_STANDARD 14 CACHE STRING "Must be at least 14")
option(CMAKE_CXX_EXTENSIONS "C++ language extensions" OFF)

project(LB/utf
	VERSION 0.0.0
	LANGUAGES CXX
)
set(CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}/${PROJECT_VERSION}")

add_library(utf INTERFACE)
set_property(TARGET utf PROPERTY INTERFACE_CXX_STANDARD ${CMAKE_CXX_STANDARD})
set_property(TARGET utf PROPERTY INTERFACE_CXX_EXTENSIONS ${CMAKE_CXX_EXTENSIONS})
target_include_directories(utf
	INTERFACE
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
		$<INSTALL_INTERFACE:include>
)

install(
	TARGETS
		utf
	EXPORT utf
)
install(
	EXPORT utf
	DESTINATION cmake/LB
	NAMESPACE LB::
)
install(
	FILES
		"src/utf.hpp"
	DESTINATION include/${PROJECT_NAME}
)

option(BUILD_TESTS "Whether to build the tests" ON)
if(BUILD_TESTS)
	enable_testing()
	add_subdirectory(test)
endif()
