
cmake_minimum_required(VERSION 2.8)

project(metal CXX)

# Compiler configuration
set(CMAKE_CXX_FLAGS "-Wall -std=c++11")

set(BOOST_INCLUDE_DIR $ENV{HOME}/local/include)
set(BOOST_LIBRARY_DIR $ENV{HOME}/local/lib)

# Build configuration.
#
# FIXME: We need to find Boost 1.55. I have it installed
# here, but that's not an ideal place for it.
include_directories(. ${BOOST_INCLUDE_DIR})
link_directories(${BOOST_LIBRARY_DIR})

add_subdirectory(steve)
