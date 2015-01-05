
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
//    - module search path -- The sequence of paths searched for modules
//      when elaborating an import declaration.
//
//    - input file list -- The sequence of files given an input for
//      some translation command.
//
// TODO: Actually make configuration options!
struct Configuration {
  Configuration();
  ~Configuration();

  Path_list module_path; // The list of paths searched for modules
  Path_list input_files; // The list of files provided as input to a command
};

Configuration& config();

} // namesapce steve

#endif
