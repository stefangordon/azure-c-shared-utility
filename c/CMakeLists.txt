#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.0)
project(azure_c_shared_utility)

#the following variables are project-wide and can be used with cmake-gui
option(skip_unittests "set skip_unittests to ON to skip unittests (default is OFF)[if possible, they are always build]" OFF)
option(use_http "set use_http to ON if http is to be used, set to OFF to not use http" ON)

if(WIN32)
	option(use_schannel "set use_schannel to ON if schannel is to be used, set to OFF to not use schannel" ON)
	option(use_openssl "set use_openssl to ON if openssl is to be used, set to OFF to not use openssl" OFF)
	option(use_wolfssl "set use_wolfssl to ON if wolfssl is to be used, set to OFF to not use wolfssl" OFF)
else()
	option(use_schannel "set use_schannel to ON if schannel is to be used, set to OFF to not use schannel" OFF)
	option(use_openssl "set use_openssl to ON if openssl is to be used, set to OFF to not use openssl" ON)
	option(use_wolfssl "set use_wolfssl to ON if wolfssl is to be used, set to OFF to not use wolfssl" OFF)
endif()

option(compileOption_C "passes a string to the command line of the C compiler" OFF)
option(compileOption_CXX "passes a string to the command line of the C++ compiler" OFF)

include_directories(inc)

# Start of variables used during install
set (INCLUDE_INSTALL_DIR "/usr/include" CACHE PATH "Include file directory")
set (LIB_INSTALL_DIR lib CACHE PATH "Library object file directory")

#Use solution folders. 
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

enable_testing()

#if any compiler has a command line switch called "OFF" then it will need special care
if(NOT "${compileOption_C}" STREQUAL "OFF")
	set(CMAKE_C_FLAGS "${compileOption_C} ${CMAKE_C_FLAGS}")
endif()

if(NOT "${compileOption_CXX}" STREQUAL "OFF")
	set(CMAKE_CXX_FLAGS "${compileOption_CXX} ${CMAKE_CXX_FLAGS}")
endif()

#this project uses several other projects that are build not by these CMakeFiles
#this project also targets several OSes

#this function takes care of three things:
#1. copying some shared libraries(.dll or .so) to the location of the output executable

macro(compileAsC99)
  if (CMAKE_VERSION VERSION_LESS "3.1")
	if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
	  set (CMAKE_C_FLAGS "--std=c99 ${CMAKE_C_FLAGS}")
	  set (CMAKE_CXX_FLAGS "--std=c++11 ${CMAKE_CXX_FLAGS}")
	endif()
  else()
	set (CMAKE_C_STANDARD 99)
	set (CMAKE_CXX_STANDARD 11)
  endif()
endmacro(compileAsC99)

macro(compileAsC11)
  if (CMAKE_VERSION VERSION_LESS "3.1")
	if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
	  set (CMAKE_C_FLAGS "--std=c11 ${CMAKE_C_FLAGS}")
	  set (CMAKE_C_FLAGS "-D_POSIX_C_SOURCE=200112L ${CMAKE_C_FLAGS}")
	  set (CMAKE_CXX_FLAGS "--std=c++11 ${CMAKE_CXX_FLAGS}")
	endif()
  else()
	set (CMAKE_C_STANDARD 11)
	set (CMAKE_CXX_STANDARD 11)
  endif()
endmacro(compileAsC11)

function(add_files_to_install filesToBeInstalled)
	set(INSTALL_H_FILES ${INSTALL_H_FILES} ${filesToBeInstalled} CACHE INTERNAL "Files that will be installed on the system")
endfunction()

function(windows_unittests_add_dll whatIsBuilding)
	link_directories(${whatIsBuilding}_dll $ENV{VCInstallDir}UnitTest/lib)
	
	add_library(${whatIsBuilding}_dll SHARED 
		${${whatIsBuilding}_cpp_files} 
		${${whatIsBuilding}_h_files} 
		${${whatIsBuilding}_c_files}
	)

	set_target_properties(${whatIsBuilding}_dll
			   PROPERTIES
			   FOLDER "UnitTests") 
	
	target_include_directories(${whatIsBuilding}_dll PUBLIC ${sharedutil_include_directories} $ENV{VCInstallDir}UnitTest/include)
	target_compile_definitions(${whatIsBuilding}_dll PUBLIC -DCPP_UNITTEST)
	target_link_libraries(${whatIsBuilding}_dll micromock_cpp_unittest ctest ${ARGN})
endfunction()

function(windows_unittests_add_lib whatIsBuilding)
	link_directories(${whatIsBuilding}_lib $ENV{VCInstallDir}UnitTest/lib)
	
	add_library(${whatIsBuilding}_lib STATIC 
		${${whatIsBuilding}_cpp_files} 
		${${whatIsBuilding}_h_files} 
		${${whatIsBuilding}_c_files}
	)
	
	target_include_directories(${whatIsBuilding}_lib PUBLIC ${sharedutil_include_directories})
	target_compile_definitions(${whatIsBuilding}_lib PUBLIC -DUSE_CTEST)
	target_link_libraries(${whatIsBuilding}_lib micromock_ctest ctest ${ARGN})
endfunction()

function(windows_unittests_add_exe whatIsBuilding)
	add_executable(${whatIsBuilding}_exe
		${${whatIsBuilding}_cpp_files} 
		${${whatIsBuilding}_h_files} 
		${${whatIsBuilding}_c_files}
		${CMAKE_CURRENT_LIST_DIR}/main.c
	)
	set_target_properties(${whatIsBuilding}_exe
			   PROPERTIES
			   FOLDER "UnitTests")
			    
	target_compile_definitions(${whatIsBuilding}_exe PUBLIC -DUSE_CTEST)
	target_include_directories(${whatIsBuilding}_exe PUBLIC ${sharedutil_include_directories})
	target_link_libraries(${whatIsBuilding}_exe micromock_ctest ctest ${ARGN})
	add_test(NAME ${whatIsBuilding} COMMAND ${whatIsBuilding}_exe)
endfunction()

function(build_test_artifacts whatIsBuilding use_gballoc)
	
	#the first argument is what is building
	#the second argument is whether the tests should be build with gballoc #defines or not
	#the following arguments are a list of libraries to link with
	
	if(${use_gballoc})
		add_definitions(-DGB_MEASURE_MEMORY_FOR_THIS -DGB_DEBUG_ALLOC)
	else()	
	endif()
	
	#setting #defines
	if(WIN32)
		add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	else()
	endif()
	
	#setting includes
	set(sharedutil_include_directories ${MICROMOCK_INC_FOLDER} ${TESTRUNNERSWITCHER_INC_FOLDER} ${CTEST_INC_FOLDER} ${SAL_INC_FOLDER} ${SHARED_UTIL_INC_FOLDER} ${SHARED_UTIL_SRC_FOLDER})
	if(WIN32)
	else()
		include_directories(${sharedutil_include_directories})
	endif()

	#setting output type
	if(WIN32)
		#to disable running e2e or longhaul or unittests tests for windows, we build the the same thing as "static library" so it is not picked up by visual studio
		if(
			(("${whatIsBuilding}" MATCHES ".*unittests.*") AND ${skip_unittests})
		)
			windows_unittests_add_lib(${whatIsBuilding} ${ARGN})
		else()
			windows_unittests_add_exe(${whatIsBuilding} ${ARGN})
			windows_unittests_add_dll(${whatIsBuilding} ${ARGN})
		endif()
	else()
		if(
			(("${whatIsBuilding}" MATCHES ".*unittests.*") AND ${skip_unittests})
		)
			windows_unittests_add_lib(${whatIsBuilding} ${ARGN})
		else()
			windows_unittests_add_exe(${whatIsBuilding} ${ARGN})
		endif()
	endif()
endfunction(build_test_artifacts)

function(set_platform_files)
	if(WIN32)
		set(CONDITION_C_FILE ${CMAKE_CURRENT_LIST_DIR}/adapters/condition_c11.c PARENT_SCOPE)
		set(HTTP_C_FILE ${CMAKE_CURRENT_LIST_DIR}/adapters/httpapi_winhttp.c PARENT_SCOPE)
		set(LOCK_C_FILE ${CMAKE_CURRENT_LIST_DIR}/adapters/lock_c11.c PARENT_SCOPE)
		set(PLATFORM_C_FILE ${CMAKE_CURRENT_LIST_DIR}/adapters/platform_win32.c PARENT_SCOPE)
		set(SOCKETIO_C_FILE ${CMAKE_CURRENT_LIST_DIR}/adapters/socketio_win32.c PARENT_SCOPE)
		set(THREAD_C_FILE ${CMAKE_CURRENT_LIST_DIR}/adapters/threadapi_c11.c PARENT_SCOPE)
	else()
		set(CONDITION_C_FILE ${CMAKE_CURRENT_LIST_DIR}/adapters/condition_pthreads.c PARENT_SCOPE)
		set(HTTP_C_FILE ${CMAKE_CURRENT_LIST_DIR}/adapters/httpapi_curl.c PARENT_SCOPE)
		set(LOCK_C_FILE ${CMAKE_CURRENT_LIST_DIR}/adapters/lock_pthreads.c PARENT_SCOPE)
		set(PLATFORM_C_FILE ${CMAKE_CURRENT_LIST_DIR}/adapters/platform_linux.c PARENT_SCOPE)
		set(SOCKETIO_C_FILE ${CMAKE_CURRENT_LIST_DIR}/adapters/socketio_berkeley.c PARENT_SCOPE)
		set(THREAD_C_FILE ${CMAKE_CURRENT_LIST_DIR}/adapters/threadapi_pthreads.c PARENT_SCOPE)
	endif()
endfunction(set_platform_files)

#Setup the platform files
set_platform_files()

add_subdirectory(testtools)

compileAsC11()

#these are the C source files
set(source_c_files
./src/base64.c
./src/buffer.c
./src/crt_abstractions.c
./src/doublylinkedlist.c
./src/gballoc.c
./src/hmac.c
./src/hmacsha256.c
./src/xio.c
./src/list.c
./src/map.c
./src/sastoken.c
./src/sha1.c
./src/sha224.c
./src/sha384-512.c
./src/strings.c
./src/string_tokenizer.c
./src/tickcounter.c
./src/urlencode.c
./src/usha.c
./src/vector.c
./adapters/agenttime.c
${CONDITION_C_FILE}
${PLATFORM_C_FILE}
${SOCKETIO_C_FILE}
${LOCK_C_FILE}
${THREAD_C_FILE}
)

if(${use_http})
	set(source_c_files ${source_c_files}
		./src/httpapiex.c
		./src/httpapiexsas.c
		./src/httpheaders.c
		${HTTP_C_FILE}		
	)	
endif()

if(${use_schannel})
	set(source_c_files ${source_c_files}
		./src/tlsio_schannel.c
	)
endif()
if(${use_wolfssl})
	set(source_c_files ${source_c_files}
		./src/tlsio_wolfssl.c
	)
endif()
if(${use_openssl})
	set(source_c_files ${source_c_files}
		./src/tlsio_openssl.c
	)
endif()

#these are the C headers
set(source_h_files
./inc/agenttime.h
./inc/base64.h
./inc/buffer_.h
./inc/crt_abstractions.h
./inc/condition.h
./inc/doublylinkedlist.h
./inc/gballoc.h
./inc/hmac.h
./inc/hmacsha256.h
./inc/iot_logging.h
./inc/list.h
./inc/lock.h
./inc/macro_utils.h
./inc/map.h
./inc/mqttapi.h
./inc/platform.h
./inc/refcount.h
./inc/sastoken.h
./inc/sha-private.h
./inc/sha.h
./inc/socketio.h
./inc/stdint_ce6.h
./inc/strings.h
./inc/string_tokenizer.h
./inc/tickcounter.h
./inc/threadapi.h
./inc/xio.h
./inc/urlencode.h
./inc/vector.h
./inc/xlogging.h
)

if(${use_http})
	set(source_h_files ${source_h_files}
		./inc/httpapi.h
		./inc/httpapiex.h
		./inc/httpapiexsas.h
		./inc/httpheaders.h
	)
endif()

if(${use_schannel})
	set(source_h_files ${source_h_files}
		./inc/tlsio_schannel.h
	)
endif()
if(${use_wolfssl})
	set(source_h_files ${source_h_files}
		./inc/tlsio_wolfssl.h
	)
endif()
if(${use_openssl})
	set(source_h_files ${source_h_files}
		./inc/tlsio_openssl.h
	)
if(WIN32)
include_directories($ENV{OpenSSLDir}/include)
endif()
endif()

if(WIN32)
else()
	add_files_to_install("${source_h_files}")
	add_files_to_install("./src/buffer.c")
	add_files_to_install("./src/crt_abstractions.c")
	add_files_to_install("./src/doublylinkedlist.c")
	add_files_to_install("./src/gballoc.c")
	add_files_to_install("./src/strings.c")
	add_files_to_install("./src/vector.c")
endif()

#these are the include folders
#the following "set" statetement exports across the project a global variable called COMMON_INC_FOLDER that expands to whatever needs to included when using COMMON library
set(SHARED_UTIL_INC_FOLDER ${CMAKE_CURRENT_LIST_DIR}/inc CACHE INTERNAL "this is what needs to be included if using sharedLib lib" FORCE)
set(SHARED_UTIL_SRC_FOLDER ${CMAKE_CURRENT_LIST_DIR}/src CACHE INTERNAL "this is what needs to be included when doing include sources" FORCE)
include_directories(${SHARED_UTIL_INC_FOLDER})

IF(WIN32)
	#windows needs this define
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
ENDIF(WIN32)

#this is the product (a library)
add_library(aziotsharedutil ${source_c_files} ${source_h_files})

if(${use_http})
	if(WIN32)
		target_link_libraries(aziotsharedutil winhttp.lib)
	else()
		target_link_libraries(aziotsharedutil curl)
	endif()
endif()

if(WIN32)
else()
	target_link_libraries(aziotsharedutil pthread)
endif()

add_subdirectory(tests)

if(WIN32)
else()
	install (TARGETS aziotsharedutil DESTINATION lib)
	install (FILES ${INSTALL_H_FILES} 
		${LOCK_C_FILE} 
		./src/buffer.c
		./src/crt_abstractions.c
		./src/doublylinkedlist.c
		./src/gballoc.c
		${HTTP_C_FILE}
		./src/strings.c
		./src/vector.c		
		DESTINATION ${INCLUDE_INSTALL_DIR}/azuresharedutil)
endif (WIN32)