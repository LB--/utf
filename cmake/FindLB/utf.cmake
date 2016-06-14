########
# FindMyPackage.cmake
# This is a generic FindPackage script that doesn't require any modification to be used in your
# own project - just change the filename and you're good to go! More information available at
# https://github.com/LB--/FindMyPackage.cmake
########

# Debug reporting macro
macro(_fmp_report_debug _msg)
	if(LB_FIND_MY_PACKAGE_DEBUG)
		message("<> ${_msg}")
	endif()
endmacro()

# Figure out which package we are finding
string(FIND "${CMAKE_CURRENT_LIST_FILE}" "/Find" _start REVERSE)
math(EXPR _start "${_start} + 5")
string(SUBSTRING "${CMAKE_CURRENT_LIST_FILE}" ${_start} -1 _package) # strip "/Find"
string(LENGTH "${_package}" _length)
math(EXPR _end "${_length} - 6")
string(SUBSTRING "${_package}" 0 ${_end} _package) # strip ".cmake"
string(REPLACE "/" "::" _package_name "${_package}")
_fmp_report_debug("${_package} | ${_package_name}")

# Avoid double execution
if(${_package}_FOUND)
	return()
endif()

# Error reporting macro
macro(_fmp_report_error _msg)
	if(${_package}_FIND_REQUIRED)
		message(FATAL_ERROR "Find ${_package_name}: ${_msg}")
	elseif(NOT ${_package}_FIND_QUIETLY)
		message(STATUS "Find ${_package_name}: ${_msg}")
	endif()
	set(${_package}_NOT_FOUND_MESSAGE "${_msg}")
endmacro()

# Info reporting macro
macro(_fmp_report_info _msg)
	if(NOT ${_package}_FIND_QUIETLY)
		message(STATUS "Find ${_package_name}: ${_msg}")
	endif()
endmacro()

# Debugging spam
_fmp_report_debug("########")
_fmp_report_debug("${${_package}_FIND_VERSION}")
_fmp_report_debug("${${_package}_FIND_VERSION_EXACT}")
_fmp_report_debug("${${_package}_FIND_VERSION_MAJOR}")
_fmp_report_debug("${${_package}_FIND_VERSION_MINOR}")
_fmp_report_debug("${${_package}_FIND_VERSION_PATCH}")
_fmp_report_debug("${${_package}_FIND_VERSION_TWEAK}")
_fmp_report_debug("${${_package}_FIND_QUIETLY}")
_fmp_report_debug("${${_package}_FIND_REQUIRED}")
_fmp_report_debug("${${_package}_FIND_COMPONENTS}")
foreach(_component ${${_package}_FIND_COMPONENTS})
	if(${_package}_FIND_REQUIRED_${_component})
		_fmp_report_debug("Component ${_component} is REQUIRED")
	else()
		_fmp_report_debug("Component ${_component} is OPTIONAL")
	endif()
endforeach()
_fmp_report_debug("########")

# For CMake GUI
option(${_package}_PREFER_HIGHEST
	"Prefer the highest available version even if a lower version has more requested optional components"
)

# Find where all versions are installed
find_path(${_package}_ROOT
	NAMES
		${_package}
	DOC "This should be the directory that contains ${_package}"
)

# Error out if we could not find the package directory
if("${${_package}_ROOT}" STREQUAL "${_package}_ROOT-NOTFOUND")
	_fmp_report_error("Could not find where versions are stored - please set ${_package}_ROOT")
	return()
endif()

_fmp_report_debug("${_package}_ROOT=${${_package}_ROOT}")

set(${_package}_VERSIONS_DIRECTORY ${${_package}_ROOT}/${_package})

# Get a list of the package versions
file(GLOB ${_package}_VERSIONS
	LIST_DIRECTORIES true
	RELATIVE ${${_package}_VERSIONS_DIRECTORY}
	${${_package}_VERSIONS_DIRECTORY}/*
)

_fmp_report_debug("${_package}_VERSIONS=${${_package}_VERSIONS}")

# Error out if there aren't any versions (unusual scenario, but still possible)
if(NOT ${_package}_VERSIONS)
	_fmp_report_error("Could not find package - no versions were detected in ${${_package}_VERSIONS_DIRECTORY}")
	return()
endif()

# Find candidate versions
set(_candidate_versions "")
foreach(_version ${${_package}_VERSIONS})
	# Reset find version vars
	set(_find_version ${${_package}_FIND_VERSION})
	string(REPLACE "." ";" _find_version_list "${_find_version}")
	list(LENGTH _find_version_list _find_version_depth)

	# Reset candidate version vars
	string(REPLACE "." ";" _version_list "${_version}")
	list(LENGTH _version_list _version_depth)

	# Determine if this version is a candidate
	if(_find_version_depth GREATER _version_depth)
		if(${_package}_FIND_VERSION_EXACT)
			continue()
		endif()
	else()
		# Pad find version with components from candidate
		while(_find_version_depth LESS _version_depth)
			list(GET _version_list ${_find_version_depth} _component)
			list(APPEND _find_version_list ${_component})
			list(LENGTH _find_version_list _find_version_depth)
		endwhile()
	endif()
	if(_find_version_list STREQUAL _version_list)
		list(APPEND _candidate_versions ${_version})
		continue()
	elseif(NOT ${_package}_FIND_VERSION_EXACT)
		string(REPLACE ";" "." _find_version "${_find_version_list}")
		if(_find_version VERSION_LESS _version)
			list(APPEND _candidate_versions ${_version})
			continue()
		endif()
	endif()
endforeach()
_fmp_report_debug("_candidate_versions=${_candidate_versions}")

# Error out if we didn't find any candidates
list(LENGTH _candidate_versions _num_candidates)
if(_num_candidates EQUAL 0)
	if(${_package}_FIND_VERSION_EXACT)
		set(_matching "equivalent to")
	else()
		set(_matching "at least")
	endif()
	_fmp_report_error("Could not find any version that was ${_matching} ${${_package}_FIND_VERSION} - available versions: ${${_package}_VERSIONS}")
	return()
endif()

# Filter out versions that don't have all required components
set(_index 0)
while(_index LESS _num_candidates)
	list(GET _candidate_versions ${_index} _version)
	foreach(_component ${${_package}_FIND_COMPONENTS})
		if(${_package}_FIND_REQUIRED_${_component})
			if(NOT EXISTS "${${_package}_VERSIONS_DIRECTORY}/${_version}/cmake/${_package}/${_component}.cmake")
				list(REMOVE_AT _candidate_versions ${_index})
				math(EXPR _index "${_index} - 1")
				_fmp_report_info("Ignoring version ${_version} because it does not have the required component ${_component}")
				break()
			endif()
		endif()
	endforeach()
	math(EXPR _index "0${_index} + 1") # CMake math() cannot handle negative numbers!!
	list(LENGTH _candidate_versions _num_candidates)
endwhile()
_fmp_report_debug("_candidate_versions=${_candidate_versions}")

# Error out if none of the versions had all the required components
list(LENGTH _candidate_versions _num_candidates)
if(_num_candidates EQUAL 0)
	_fmp_report_error("No applicable version has all the required components")
	return()
endif()

if(${_package}_PREFER_HIGHEST)
	set(_preferred_versions "${_candidate_versions}")
else()
	# Prefer versions with the most requested optional components
	set(_preferred_versions "")
	set(_preferred_versions_compnum 0)
	foreach(_version ${_candidate_versions})
		set(_compnum 0)
		# Count present requested optional components
		foreach(_component ${${_package}_FIND_COMPONENTS})
			if(NOT ${_package}_FIND_REQUIRED_${_component})
				if(EXISTS "${${_package}_VERSIONS_DIRECTORY}/${_version}/cmake/${_package}/${_component}.cmake")
					math(EXPR _compnum "${_compnum} + 1")
				endif()
			endif()
		endforeach()
		# Update preferred versions
		if(_compnum GREATER _preferred_versions_compnum)
			set(_preferred_versions ${_version})
			set(_preferred_versions_compnum ${_compnum})
		elseif(_compnum EQUAL _preferred_versions_compnum)
			list(APPEND _preferred_versions ${_version})
		endif()
	endforeach()
	_fmp_report_debug("_preferred_versions=${_preferred_versions}")

	# Error out if we made a mistake in our logic above
	# (worst case should be that _preferred_versions is the same as _candidate_versions)
	list(LENGTH _preferred_versions _num_preferred)
	if(_num_preferred EQUAL 0)
		message(FATAL_ERROR "Something has gone wrong the code in Find${_package}.cmake")
		return()
	endif()
endif()

# Finally, select the highest version
list(GET _preferred_versions 0 _preferred_version)
foreach(_version ${_preferred_versions})
	if(_version VERSION_GREATER _preferred_version)
		set(_preferred_version ${_version})
	endif()
endforeach()
_fmp_report_debug("_preferred_version=${_preferred_version}")

# Load the version and report findings
include("${${_package}_VERSIONS_DIRECTORY}/${_preferred_version}/cmake/${_package}.cmake")
set(_components "")
foreach(_component ${${_package}_FIND_COMPONENTS})
	set(_comp_path "${${_package}_VERSIONS_DIRECTORY}/${_preferred_version}/cmake/${_package}/${_component}.cmake")
	if(EXISTS "${_comp_path}")
		include("${_comp_path}")
		list(APPEND _components ${_component})
		set(${_package}_${_component}_FOUND 1)
	endif()
endforeach()
set(${_package}_FOUND 1)
set(${_package}_VERSION "${_preferred_version}")
set(${_package}_VERSION_STRING "${_preferred_version}")
string(REPLACE "." ";" _preferred_version_list "${_preferred_version}")
list(LENGTH _preferred_version_list _preferred_version_depth)
list(GET _preferred_version_list 0 ${_package}_VERSION_MAJOR)
if(_preferred_version_depth GREATER 1)
	list(GET _preferred_version_list 1 ${_package}_VERSION_MINOR)
	if(_preferred_version_depth GREATER 2)
		list(GET _preferred_version_list 2 ${_package}_VERSION_PATCH)
		if(_preferred_version_depth GREATER 3)
			list(GET _preferred_version_list 3 ${_package}_VERSION_TWEAK)
		endif()
	endif()
endif()
set(${_package}_ROOT_DIR "${${_package}_VERSIONS_DIRECTORY}/${_preferred_version}")
string(REPLACE ";" ", " _components "${_components}")
if(_components STREQUAL "")
	_fmp_report_info("Found version ${_preferred_version} with no components")
else()
	_fmp_report_info("Found version ${_preferred_version} with components (${_components})")
endif()
return()

# Done!
