# - Find Sofia-SIP library
# This module finds if Sofia-SIP is installed or available in its source dir
# and determines where the include files and libraries are.
# This code sets the following variables:
#
#  SOFIA_FOUND           - have the Sofia-SIP libs been found
#  SOFIA_LIBRARIES       - path to the Sofia-SIP library
#  SOFIA_INCLUDE_DIRS    - path to Sofia-SIP include directories
#  SOFIA_DEFINES         - flags to define to compile with Sofia-SIP
#  SOFIA_VERSION_STRING  - version of the Sofia-SIP lib found
#
# The SOFIA_STATIC variable can be used to specify whether to prefer
# static version of Sofia-SIP library.
# The SOFIA_PTW32_STATIC variable can be used to specify whether to prefer
# static version of pthreads-win32 library on Win32 when linkin Sofia-SIP statically.
# You need to set these variables before calling find_package(Sofia).
#
# If you'd like to specify the installation of Sofia-SIP to use, you should modify
# the following cache variables:
#  SOFIA_LIBRARY             - path to the Sofia-SIP library
#  SOFIA_INCLUDE_DIR         - path to where sofia_features.h is found
#  SOFIA_PTW32_LIBRARY       - path to pthreads-win32 library bundled with Sofia-SIP
#                              (only needed when linking statically on Win32)
# If Sofia-SIP not installed, it can be used from the source directory:
#  SOFIA_SOURCE_DIR          - path to compiled Sofia-SIP source directory

#=============================================================================
# Copyright 2014 SpeechTech, s.r.o. http://www.speechtech.cz/en
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# $Id$
#=============================================================================


option (SOFIA_STATIC "Try to find and link static Sofia-SIP library" ${SOFIA_STATIC})
mark_as_advanced (SOFIA_STATIC)
if (SOFIA_STATIC AND WIN32)
	option (SOFIA_PTW32_STATIC "Try to find and link static pthreads-win32 library" ON)
	mark_as_advanced (SOFIA_PTW32_STATIC)
endif (SOFIA_STATIC AND WIN32)

include (SelectLibraryConfigurations)

# Try to find library specified by ${libnames}
# in ${hints} and put its path to ${var}_LIBRARY and ${var}_LIBRARY_DEBUG,
# and set ${var}_LIBRARIES similarly to CMake's select_library_configurations macro.
# For 32bit configurations, "/x64/" is replaced with "/".
function (find_libs var libnames hints)
	if (NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
		string (REGEX REPLACE "[\\\\/][xX]64[\\\\/]" "/" hints "${hints}")
	endif (NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
	string (REPLACE "/LibR" "/LibD" hints_debug "${hints}")
	string (REPLACE "/Release" "/Debug" hints_debug "${hints_debug}")
	find_library (${var}_LIBRARY
		NAMES ${libnames}
		HINTS ${hints})
	find_library (${var}_LIBRARY_DEBUG
		NAMES ${libnames}
		HINTS ${hints_debug})
	mark_as_advanced (${var}_LIBRARY ${var}_LIBRARY_DEBUG)
	if (${var}_LIBRARY AND ${var}_LIBRARY_DEBUG AND
			NOT (${var}_LIBRARY STREQUAL ${var}_LIBRARY_DEBUG) AND
			(CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE))
		set (${var}_LIBRARIES optimized ${${var}_LIBRARY} debug ${${var}_LIBRARY_DEBUG} PARENT_SCOPE)
	elseif (${var}_LIBRARY)
		set (${var}_LIBRARIES ${${var}_LIBRARY} PARENT_SCOPE)
	elseif (${var}_LIBRARY_DEBUG)
		set (${var}_LIBRARIES ${${var}_LIBRARY_DEBUG} PARENT_SCOPE)
	else ()
		set (${var}_LIBRARIES ${var}_LIBRARY-NOTFOUND PARENT_SCOPE)
	endif ()
endfunction (find_libs)

macro (find_sofia_static)
	set (_sofia_CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES})
	if (WIN32)
		set (CMAKE_FIND_LIBRARY_SUFFIXES .lib .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
	else (WIN32)
		set (CMAKE_FIND_LIBRARY_SUFFIXES .a)
	endif (WIN32)
	set (_sofia_hints)
	if (SOFIA_SOURCE_DIR)
		set (_sofia_hints ${_sofia_hints} "${SOFIA_SOURCE_DIR}/lib"
			"${SOFIA_SOURCE_DIR}/win32/libsofia-sip-ua-static/x64/Release"
			"${SOFIA_SOURCE_DIR}/win32/libsofia-sip-ua/x64/LibR"
			"${SOFIA_SOURCE_DIR}/libsofia-sip-ua/.libs")
		if (WIN32)
			if (SOFIA_PTW32_STATIC)
				set (_sofia_hints ${_sofia_hints} "${SOFIA_SOURCE_DIR}/win32/pthread/x64/LibR")
			else (SOFIA_PTW32_STATIC)
				set (_sofia_hints ${_sofia_hints} "${SOFIA_SOURCE_DIR}/win32/pthread/x64/Release")
			endif (SOFIA_PTW32_STATIC)
		endif (WIN32)
	endif (SOFIA_SOURCE_DIR)
	set (_sofia_hints ${_sofia_hints} /usr/local/lib)
	find_libs (SOFIA "sofia-sip-ua;libsofia_sip_ua_static;libsofia_sip_ua" "${_sofia_hints}")
	if (WIN32)
		find_libs (SOFIA_PTW32 "pthreadVC2" "${_sofia_hints}")
	endif (WIN32)
	set (CMAKE_FIND_LIBRARY_SUFFIXES ${_sofia_CMAKE_FIND_LIBRARY_SUFFIXES})
endmacro (find_sofia_static)

macro (find_sofia_dynamic)
	set (_sofia_hints)
	if (SOFIA_SOURCE_DIR)
		set (_sofia_hints ${_sofia_hints} "${SOFIA_SOURCE_DIR}/lib"
			"${SOFIA_SOURCE_DIR}/win32/libsofia-sip-ua/x64/Release"
			"${SOFIA_SOURCE_DIR}/libsofia-sip-ua/.libs")
	endif (SOFIA_SOURCE_DIR)
	set (_sofia_hints ${_sofia_hints} /usr/local/lib)
	find_libs (SOFIA "sofia-sip-ua;libsofia_sip_ua" "${_sofia_hints}")
endmacro (find_sofia_dynamic)

include (FindPackageMessage)
if (SOFIA_STATIC)
	find_sofia_static ()
	if (NOT SOFIA_LIBRARIES)
		find_package_message (Sofia "Static Sofia-SIP library not found, trying dynamic"
			"[${SOFIA_LIBRARY}][${SOFIA_INCLUDE_DIR}][${SOFIA_STATIC}]")
		find_sofia_dynamic ()
	else (NOT SOFIA_LIBRARIES)
		set (CMAKE_THREAD_PREFER_PTHREAD 1)
		find_package (Threads REQUIRED)
		set (SOFIA_LIBRARIES ${SOFIA_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
		find_package (OpenSSL)
		if (OPENSSL_LIBRARIES)
			set (SOFIA_LIBRARIES ${SOFIA_LIBRARIES} ${OPENSSL_LIBRARIES})
		endif (OPENSSL_LIBRARIES)
	endif (NOT SOFIA_LIBRARIES)
	set (SOFIA_DEFINES -DLIBSOFIA_SIP_UA_STATIC)
	if (WIN32 AND SOFIA_PTW32_STATIC)
		set (SOFIA_DEFINES "${SOFIA_DEFINES} -DPTW32_STATIC_LIB")
	endif (WIN32 AND SOFIA_PTW32_STATIC)
else (SOFIA_STATIC)
	find_sofia_dynamic ()
	if (NOT SOFIA_LIBRARIES)
		find_package_message (Sofia "Dynamic Sofia-SIP library not found, trying static"
			"[${SOFIA_LIBRARY}][${SOFIA_INCLUDE_DIR}][${SOFIA_STATIC}]")
		find_sofia_static ()
	endif (NOT SOFIA_LIBRARIES)
	set (SOFIA_DEFINES)
endif (SOFIA_STATIC)

if (SOFIA_STATIC AND SOFIA_LIBRARIES AND WIN32)
	if (SOFIA_PTW32_LIBRARIES)
		set (SOFIA_LIBRARIES ${SOFIA_LIBRARIES} ${SOFIA_PTW32_LIBRARIES})
	else (SOFIA_PTW32_LIBRARIES)
		message ("Statically linked Sofia-SIP requires pthreads-win32, please set SOFIA_PTW32_LIBRARY")
	endif (SOFIA_PTW32_LIBRARIES)
endif (SOFIA_STATIC AND SOFIA_LIBRARIES AND WIN32)

set (_sofia_dirs)
set (SOFIA_PARTS su nua url sip msg sdp nta nea soa iptsec bnf features tport)
if (WIN32)
	set (SOFIA_PARTS ${SOFIA_PARTS} win32)
endif (WIN32)
set (_sofia_hints /usr/loca/include/sofia-sip-1.12
	/usr/loca/include/sofia-sip-1.1?
	/usr/local/include/sofia-sip-1.*
	/usr/local/include/sofia-sip-*)
if (SOFIA_SOURCE_DIR)
	foreach (part IN LISTS SOFIA_PARTS)
		if (part STREQUAL win32)
			set (_sofia_hints ${_sofia_hints} "${SOFIA_SOURCE_DIR}/${part}")
		else (part STREQUAL win32)
			set (_sofia_hints ${_sofia_hints} "${SOFIA_SOURCE_DIR}/libsofia-sip-ua/${part}")
		endif (part STREQUAL win32)
	endforeach (part)
	if (WIN32 AND SOFIA_STATIC)
		find_path (SOFIA_PTW32_INCLUDE_DIR pthread.h
			HINTS ${_sofia_hints} "${SOFIA_SOURCE_DIR}/win32/pthread")
		if (SOFIA_PTW32_INCLUDE_DIR)
			set (_sofia_dirs ${_sofia_dirs} ${SOFIA_PTW32_INCLUDE_DIR})
		endif (SOFIA_PTW32_INCLUDE_DIR)
	endif (WIN32 AND SOFIA_STATIC)
endif (SOFIA_SOURCE_DIR)
foreach (part IN LISTS SOFIA_PARTS)
	if (part STREQUAL features)
		set (header sofia_features.h)
	elseif (part STREQUAL iptsec)
		set (header auth_client.h)
	elseif (part STREQUAL win32)
		set (header su_configure.h)
	else (part STREQUAL features)
		set (header ${part}.h)
	endif (part STREQUAL features)
	find_path (SOFIA_INC_${part} sofia-sip/${header}
		HINTS ${_sofia_hints})
	mark_as_advanced (SOFIA_INC_${part})
	if (SOFIA_INC_${part})
		list (APPEND _sofia_dirs ${SOFIA_INC_${part}})
	else (SOFIA_INC_${part})
		message ("Sofia warning: ${header} not found")
	endif (SOFIA_INC_${part})
endforeach (part)
if (_sofia_dirs)
	list (REMOVE_DUPLICATES _sofia_dirs)
endif (_sofia_dirs)
set (SOFIA_INCLUDE_DIRS ${_sofia_dirs})
find_path (SOFIA_INCLUDE_DIR sofia-sip/sofia_features.h
	HINTS ${_sofia_hints})

if (SOFIA_INCLUDE_DIR)
	file (STRINGS "${SOFIA_INCLUDE_DIR}/sofia-sip/sofia_features.h" _sofia_ver
		REGEX "^#define[ \t]+SOFIA_SIP_VERSION[ \t]+\"[^\"]+\"")
	string (REGEX REPLACE ".*[ \t]SOFIA_SIP_VERSION[ \t]+\"([^\"]+)\".*" "\\1" SOFIA_VERSION_STRING "${_sofia_ver}")
endif (SOFIA_INCLUDE_DIR)

mark_as_advanced (
	SOFIA_LIBRARY
	SOFIA_LIBRARY_DEBUG
	SOFIA_INCLUDE_DIR
	SOFIA_PTW32_LIBRARY
	SOFIA_PTW32_LIBRARY_DEBUG)

unset (SOFIA_FOUND)
include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (Sofia
	REQUIRED_VARS SOFIA_LIBRARIES SOFIA_INCLUDE_DIRS
	VERSION_VAR SOFIA_VERSION_STRING)
