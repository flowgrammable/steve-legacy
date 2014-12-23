
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

void
usage(std::ostream& os) {
  os << "steve [steve-arguments] <command> [command-arguments] [command-inputs]\n";
}

int
usage_error() {
  std::cerr << "error: invalid arguments\n";
  usage(std::cerr);
  return -1;
}

// Top level subcommands
cli::Command_map commands {
  {"help", new cli::Help_command() },
  {"version", new cli::Version_command() },
  {"extract", new cli::Extract_command() }
};

int
main(int argc, char* argv[]) {
  cli::Parameter_map parms { }; // FIXME: Define top-level arguments
  cli::Argument_map args;
  cli::Parser arg_parse(parms, args);
  int last = arg_parse(argc, argv);
  if (last < 0 or last == argc)
    return usage_error();

  std::string cmd = argv[last];
  std::cout << "COMMAND: " << cmd << '\n';


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
