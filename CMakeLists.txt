
cmake_minimum_required(VERSION 2.8)

project(steve CXX)

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
find_package(Boost REQUIRED COMPONENTS system filesystem)


add_subdirectory(lang)
