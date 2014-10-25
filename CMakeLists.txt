
cmake_minimum_required(VERSION 2.8)

project(metal CXX)

# Compiler configuration
set(CMAKE_CXX_FLAGS "-Wall -std=c++11")

# Build configuration.
include_directories(.)

# FIXME: Needed for MacPorts install of libgmp. It
# shouldn't be like this, but it is :/
link_directories(/opt/local/lib)

add_subdirectory(steve)
