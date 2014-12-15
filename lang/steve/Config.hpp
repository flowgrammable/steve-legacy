
#ifndef STEVE_CONFIG_HPP
#define STEVE_CONFIG_HPP

#include <steve/File.hpp>

#include <vector>

// This module includes facilities for defining the runtime configuration 
// of the Steve Programming Language. The configuration is maintained
// as part of the global state of the program.

namespace steve {

// The configuration class maintains configuration data for the
// Steve Programming Language compiler. In particular, the following
// information is maintained:
//
//    - module search path
//
// TODO: Flesh this out a little more.
struct Configuration {
  Configuration(int argc, char* argv[]);

  Path_list module_path;
};

const Configuration& config();

} // namesapce steve

#endif
