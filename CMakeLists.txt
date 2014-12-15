
cmake_minimum_required(VERSION 2.8)

project(metal CXX)

# Compiler configuration
set(CMAKE_CXX_FLAGS "-Wall -std=c++11")

# FIXME: Use FindPackage or FindBoost rather than
# configuring manaully.
set(BOOST_INCLUDE_DIR $ENV{HOME}/local/include)
set(BOOST_LIBRARY_DIR $ENV{HOME}/local/lib)

add_subdirectory(lang)
