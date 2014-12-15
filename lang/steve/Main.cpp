
#include <iostream>
#include <fstream>
#include <iterator>

#include <steve/Config.hpp>
#include <steve/Language.hpp>
#include <steve/Module.hpp>
#include <steve/Lexer.hpp>
#include <steve/Parser.hpp>
#include <steve/Elaborator.hpp>

using namespace steve;

void
usage(std::ostream& os) {
  os << "steve [options] input-file\n";
}

int
usage_error() {
  std::cerr << "error: invalid arguments\n";
  usage(std::cerr);
  return 01;
}

int 
main(int argc, char* argv[]) {
  Configuration cfg(argc, argv);
  Language lang;

  // FIXME: Actually parse command line arguments.
  if (argc < 2)
    return usage_error();
  String input = argv[argc - 1];

  // Load the program.
  Module* prog = load_file(input);
  if (not prog)
    return -1;

  // Dump output.
  for (Decl* d : *prog->decls())
    std::cout << debug(d) << '\n';

  return 0;
}
