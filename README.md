steve
=====

The Steve programming language.


## Dependencies

Boost 1.55 or greater. See these
[instructions](https://coderwall.com/p/0atfug/installing-boost-1-55-from-source-on-ubuntu-12-04)
for manually installing newer versions of Boost from source on Ubuntu systems.

## Installing CMake 3.1 on Ubuntu

Building Steve requires at least CMake 3.0, which must be manually installed on Ubuntu. Follow these steps to build version 3.1.0 from source:

	wget http://www.cmake.org/files/v3.1/cmake-3.1.0.tar.gz
	tar xzf cmake-3.1.0.tar.gz
	cd cmake-3.1.0
	./configure --prefix=/opt/cmake
	make
	sudo make install
	
This will install CMake 3.1.0 to /opt/cmake/bin. If you don't have any versions of cmake already installed, you may want to add this directory to your path. However, if you have an older version, you can alias "cmake-3.1.0" to "/opt/cmake/bin/cmake". To do so, add this line to ~/.bash_aliases:
	
	alias cmake-3.1.0="/opt/cmake/bin/cmake"
	

## Build Instructions

1. Clone the current version of the steve repository

		git clone https://github.com/flowgrammable/steve
	
2. Inside the newly created steve directory, create a build directory and use cmake to generate the makefile. Note: if you had created an alias for CMake 3.1.0, "cmake .." wil need to be "cmake-3.1.0 ..".

		cd steve
		mkdir build
		cd build
		cmake ..
		make

## Project layout

The two primary components of the Steve Programming Language are
its compiler, in the `lang` directory, and its Standard Library,
in the `lib` directory.

Note that Steve's runtime library does not exist yet.
