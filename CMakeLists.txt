cmake_minimum_required(VERSION 3.5)

project(music_sync LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (SMALL)
	message("\n\"SMALL\" option is enabled, the binary will be very small and fast.\n")
	set(CMAKE_CXX_FLAGS "-s -Os -ffunction-sections -fdata-sections -Wl,--gc-sections -DNDEBUG")
endif()

add_executable(music_sync main.cpp)
