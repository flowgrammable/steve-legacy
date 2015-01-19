
cmake_minimum_required(VERSION 3.0)

project(steve CXX)

# CMake configuration
#
# Add local local prefixes to all of the find_* functions
# in order to support custom installations.
set(CMAKE_PREFIX_PATH $ENV{HOME}/local $ENV{HOME}/opt)


# Compiler configuration
set(CMAKE_CXX_FLAGS "-Wall -std=c++11")


# Configure GMP
#
# TODO: Factor this into a find_package module.
find_path(GMP_INCLUDE_DIR NAMES gmp.h)
find_library(GMP_LIBRARIES NAMES gmp libgmp)


# Configure GC
#
# FIXME: This is currently disabled. We should definitely
# re-enable garbage collection.
#
# TODO: Factor this into a find_package module.
set(GC_INCLUDE_DIR NAMES "")
set(GC_LIBRARY "")


# Configure Boost
find_package(Boost 1.55 REQUIRED COMPONENTS system filesystem)


add_subdirectory(lang)
