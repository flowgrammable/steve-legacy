
#include <steve/Cli.hpp>
#include <steve/Format.hpp>

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

} // namesapce


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
// Commands

bool 
Steve_command::operator()(int, int, char**) { return true; }

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
    std::cerr << "error: steve help <command>\n";
    return false;
  }
  std::cout << "help on '" << argv[arg] << "'\n";
  return true ; 
}

bool 
Version_command::operator()(int, int, char**) { return true; }

bool 
Extract_command::operator()(int, int, char**) { return true; }


} // namespace cli
} // namespace steve
