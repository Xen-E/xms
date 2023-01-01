cmake_minimum_required(VERSION 3.5)

project(XMS LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (SMALL)
	message("\n\"SMALL\" option is enabled, the binary will be small and fast.\n")
	if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		set(CMAKE_CXX_FLAGS "-s -Os -ffunction-sections -fdata-sections -Wl,--gc-sections -DNDEBUG")
	elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
		set(CMAKE_CXX_FLAGS "/O2 /DEBUG:NONE /EHsc")
	elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
		set(CMAKE_CXX_FLAGS "-Ofast -ffunction-sections -fdata-sections -Wl,-s -DNDEBUG")
	endif()
endif()

add_executable(XMS main.cpp)
