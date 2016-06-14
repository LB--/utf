cmake_minimum_required(VERSION 3.4)

set(CMAKE_CXX_STANDARD 14 CACHE STRING "Must be at least 14")
option(CMAKE_CXX_EXTENSIONS "C++ language extensions" OFF)

project(LB/utf
	VERSION 0.0.0
	LANGUAGES CXX
)
set(CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}/${PROJECT_VERSION}")

add_library(utf
	"src/utf.cpp"
)
target_include_directories(utf
	PUBLIC
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
		$<INSTALL_INTERFACE:include>
)

install(
	TARGETS
		utf
	EXPORT utf
	ARCHIVE DESTINATION lib/${PROJECT_NAME}
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
