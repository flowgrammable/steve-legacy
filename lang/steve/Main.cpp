
#include <iostream>
#include <fstream>
#include <iterator>

#include <steve/Cli.hpp>
#include <steve/Config.hpp>
#include <steve/Language.hpp>
#include <steve/Module.hpp>
#include <steve/Lexer.hpp>
#include <steve/Parser.hpp>
#include <steve/Elaborator.hpp>

using namespace steve;

// Commands
//
// TODO: Is there any reason why these are defined here and not
// in Cli.cpp? What's the purpose of putting them here...
cli::Help_command    help_cmd;
cli::Version_command version_cmd;
cli::Extract_command extract_cmd;
cli::Test_command    test_cmd;

// Populate the command map
cli::Command_map commands {
  {"help",    &help_cmd},
  {"version", &version_cmd},
  {"extract", &extract_cmd},
  {"test",    &test_cmd}
};

// FIXME: Move these into the help function.
int
usage_error() {
  std::cerr << "error: invalid arguments\n";
  help_cmd(commands);
  return -1;
}

int
command_error(const char* cmd) {
  std::cerr << format("error: unrecognized command '{}'\n", cmd);
  help_cmd(commands);
  return -1;
}

int
main(int argc, char* argv[]) {
  // Initialize the configuration
  Configuration cfg;

  // Initialize the language environment.
  Language lang;

  // Initialize the root diagnostics.  
  Diagnostics diags;
  Diagnostics_guard dg = diags;

  // FIXME: Refactor the top-level parser into a command
  cli::Parameter_map parms { }; // FIXME: Define top-level arguments
  cli::Argument_map args;
  cli::Parser arg_parse(parms, args);
  int last = arg_parse(argc, argv);
  if (last < 0 or last == argc)
    return usage_error();

  // tOIDO

  // Get the command.
  auto iter = commands.find(argv[last]);
  if (iter == commands.end())
    return command_error(argv[last]);
  cli::Command& cmd = *iter->second;

  // Run the command.
  if (not cmd(++last, argc, argv)) {
    std::cerr << diags;
    return -1;
  }
  
  return 0;
}
