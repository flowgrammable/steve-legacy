
#ifndef CLI_HPP
#define CLI_HPP

#include <map>
#include <string>
#include <vector>

namespace steve {
namespace cli {

// A CLI value is a simple union of different value kinds.
//
// TODO: Improve encapsulation.
struct Value {
  Value() : kind(None) { }
  Value(bool b) : kind(Bool) { value.b = b; }
  Value(int n) : kind(Int) { value.n = n; }
  Value(const char* s) : kind(Str) { value.s = s; }

  enum { 
    None, Bool, Int, Str 
  } kind;
  union { 
    bool b; int n; const char* s; 
  } value;
};

// A parameter specifies information about a command line argument
// that can be parsed. In particular, the parameter has a name,
// a default value, and a description.
struct Parameter {
  std::string name;
  Value def;
  std::string desc;
};

// A mapping of names to parameters.
//
// TODO: Allow aliases in the parameter map to accommodate shorthand
// arguments (e.g., -x versus --exclude).
using Parameter_map = std::map<std::string, Parameter>;

// A mapping of parameter names to values.
using Argument_map = std::map<std::string, Value>;
using Argument_list = std::vector<std::string>;

// The command line parser is responsible for parsing command line
// arguments into a set of named values. 
//
//  The parser consumes an argument-list and then stops, allowing
// a second parser to resume parsing with a different set of options.
//
//    argument-list ::= named-arguments*
//    named-argument ::= short-arguments | long-arguments
//    short-argument ::= '-' alphanumeric ['=' value]
//    long-argument  ::=  '--' name ['=' value]
//    name ::= alphabetic-character alphanumeric-character*
//    alphanumeric-character ::= [a-zA-Z0-9_]
//    alphabetic-character ::= [a-zA-Z_]
struct Parser {
  Parser(const Parameter_map& p, Argument_map& a)
    : parms(p), args(a) { } 

  int operator()(int, char**);
  int operator()(int, int, char**);

  const Parameter_map& parms;
  Argument_map& args;
};


// A command defines an action invokable by the Steve Programming
// language environment. 
struct Command {
  virtual ~Command() { }

  virtual bool operator()(int, int, char**) = 0;

  std::string name;
  Parameter_map parms;
  Argument_map args;
};

// A set of named commands.
using Command_map = std::map<std::string, Command*>;

// Commands

struct Steve_command : Command {
  bool operator()(int, int, char**);
};

// The help command displays help about available commands and
// their options.
struct Help_command : Command {
  bool operator()(int, int, char**);
};

// The version command displays the current version of the compiler
// and available versions of the language and runtime.
struct Version_command : Command {
  bool operator()(int, int, char**);
};

// The extract command extracts code from a Steve program.
struct Extract_command : Command {
  bool operator()(int, int, char**);
};

} // namespace cli
} // namespace steve

#endif
