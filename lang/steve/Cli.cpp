
#include <steve/Cli.hpp>
#include <steve/Config.hpp>
#include <steve/Ast.hpp>
#include <steve/Error.hpp>
#include <steve/Extract.hpp>
#include <steve/Module.hpp>
#include <steve/String.hpp>

#include <cctype>
#include <cstring>
#include <algorithm>
#include <iostream>

namespace steve {
namespace cli {

namespace {

// Returns true if the character c starts an option.
inline bool
is_option(char c) { return c == '-'; }

bool
parse_long_arg(Parser& p, const char* arg) {
  int n = std::strlen(arg);
  const char* end = arg + n;
  
  // Is there an '=' in the option name?
  const char* eq = std::find(arg, end, '=');

  // Get the name from the argument.
  std::string name;
  if (eq == end)
    name = arg;
  else
    name = std::string(arg, eq);

  // FIXME: Parse the value of the parameter.

  // Find the corresponding parameter.
  if (p.parms.find(name) != p.parms.end()) {
    p.args[name] = Value(true);
  } else {
    std::cerr << format("error: unrecognized option '{}'", name) << '\n';
    return false;
  }
  return true;
}

bool
parse_short_arg(Parser& p, const char* arg) {
  std::cout << format("error: unrecognized option '{}'", arg) << '\n';
  return false;
}

bool
parse_arg(Parser& p, const char* arg) {
  if (std::isalnum(arg[1]))
    return parse_short_arg(p, arg + 1);
  else if (is_option(arg[1]))
    return parse_long_arg(p, arg + 2);
  else
    std::cerr << format("error: unrecognized option '{}'", arg) << '\n';
  return false;
}

} // namespace


int
Parser::operator()(int first, int argc, char** argv) {
  bool err = false;
  char* arg = argv[first];
  while (first != argc and is_option(*arg)) {
    if (not parse_arg(*this, arg))
      err = true;
    ++first;
    arg = argv[first];
  }
  return err ? -1 : first;
}

int
Parser::operator()(int argc, char** argv) {
  return operator()(1, argc, argv);
}


// -------------------------------------------------------------------------- //
// Help command

bool
Help_command::operator()(const Command_map& cmds) {
  std::cout << "steve [steve-arguments] <command> [command-arguments] [command-inputs]\n";
  std::cout << '\n';

  // TODO: Print the list of global steve arguments.
  
  // Print the list of registered commands.
  // TODO: Also print a brief descriptions of those commands.
  std::cout << "Commands\n";
  for (const auto& x : cmds)
    std::cout << "    " << x.first << '\n';
  return true;
}

bool 
Help_command::operator()(int arg, int argc, char** argv) { 
  if (arg == argc) {
    std::cerr << "error: missing help topic\n";
    return false;
  }
  std::cout << "help on '" << argv[arg] << "'\n";
  return true;
}


// -------------------------------------------------------------------------- //
// Version command
// 
// TODO: Abstract the version, copyright, and author data into a
// facility that can be compiled at build time.
//
// TOOD: Version information should be set by configuration, not
// in code.

bool 
Version_command::operator()(int arg, int argc, char** argv) { 
  if (arg != argc) {
    std::cerr << "error: too many arguments\n";
    return false;
  }

  std::cout << "Steve Programming Language v0.1\n";
  std::cout << "Copyright (c) 2013-2015 Flowgrammable.org\n";
  return true; 
}


// -------------------------------------------------------------------------- //
// Extract command
//
// The extract command applies a function (an extractor) to a 
// qualified-id that designates a declaration. For example:
//
//    steve extract doc std.net.ofp.uint
//
// Will extract the associated documentation for the uint function
// in that module.

// TODO: Use command line parameters to provide options for the
// extractor.
bool 
Extract_command::operator()(int arg, int argc, char** argv) { 
  // FIXME: This is totally broken. Also, use the help command to
  // print the actual help for this extractor.
  if (arg + 2 >= argc) {
    error() << "too few arguments";
    error() << "usage: steve extract <extractor> <id>";
  }
  std::string what = argv[arg++];
  std::string from = argv[arg++];

  // Load the name.
  Expr* e = load_name(from);
  if (not e) {
    error() << format("no matching declaration for '{}'", from);
    return false;
  }


  // if (Extractor* e = get_extractor(extractor))
  //   (*e)(mod);
  // else
  //   error() << format("no extractor named '{}'", extractor);

  return true; 
}

// -------------------------------------------------------------------------- //
// Test command
//
// TODO: Support a number of test modes that emit diagnosable
// text for the following activities
//
//  - lexical analysis -- print the list of tokens
//  - syntactic analysis -- print the parse tree
//  - semantic analysis -- print the elaborated AST
//  - other?
//
// TODO: Maybe rename this to 'dump' command... or define several?
//
// TODO: Allow multiple inputs? Maybe, but we need to fork() to
// do that. Otherwise, we end up building a single input module.
bool
Test_command::operator()(int arg, int argc, char** argv) {
  Diagnostics diags;
  Diagnostics_guard dg = diags;

  // Parse arguments, saving input files to the configuration.
  if (arg == argc) {
    error(no_location) << "no input files\n";
    return false;
  }
  if (argc - arg != 1) {
    error(no_location) << "too many input files\n";
  }

  // Parse the input files.
  Module* mod = load_file(argv[arg]);
  if (mod) {
    for (Decl* d : *mod->decls())
      std::cout << debug(d) << '\n';
  } 

  return mod ? true : false; 
}

} // namespace cli
} // namespace steve
