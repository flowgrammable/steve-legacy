
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
cli::Help_command    help_cmd;
cli::Version_command version_cmd;
cli::Extract_command extract_cmd;

// Populate the command map
cli::Command_map commands {
  {"help",    &help_cmd },
  {"version", &version_cmd },
  {"extract", &extract_cmd }
};

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

  // FIXME: Refactor the top-level parser into a command
  cli::Parameter_map parms { }; // FIXME: Define top-level arguments
  cli::Argument_map args;
  cli::Parser arg_parse(parms, args);
  int last = arg_parse(argc, argv);
  if (last < 0 or last == argc)
    return usage_error();

  // Get the command.
  auto iter = commands.find(argv[last]);
  if (iter == commands.end())
    return command_error(argv[last]);
  cli::Command& cmd = *iter->second;

  // Run the command.
  if (not cmd(++last, argc, argv))
    return 01;

  // Configuration cfg(argc, argv);
  // Language lang;

  // // FIXME: Actually parse command line arguments.
  // if (argc < 2)
  //   return usage_error();
  // String input = argv[argc - 1];

  // // Load the program.
  // Module* prog = load_file(input);
  // if (not prog)
  //   return -1;

  // // Dump output.
  // for (Decl* d : *prog->decls())
  //   std::cout << debug(d) << '\n';

  return 0;
}
