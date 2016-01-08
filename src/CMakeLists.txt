# Set the minimum CMake version
cmake_minimum_required(VERSION 3.2)

# Include globals
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../CMake")

# Declare the project
project(Relight)

# Add multicore build flag when generating for MSVC
if (MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif (MSVC)

# Find Embree
find_package(Embree REQUIRED)

# Add Relight source
add_subdirectory(Relight)