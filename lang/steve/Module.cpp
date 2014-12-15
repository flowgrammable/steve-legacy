
#include <steve/Module.hpp>
#include <steve/Config.hpp>
#include <steve/Ast.hpp>
#include <steve/Type.hpp>
#include <steve/Lexer.hpp>
#include <steve/Parser.hpp>
#include <steve/Elaborator.hpp>

#include <fstream>
#include <iostream>
#include <iterator>
#include <map>

#include <unistd.h>

namespace steve {

namespace {

// -------------------------------------------------------------------------- //
// Module map
//
// A module is loaded exactly once. Subsequent imports of the same
// module do not cause it to be reloaded.

using Module_map = std::map<Path, Module*>;

Module_map modules_;

// Returns the module at the given path or nullptr if no such module
// exists.
Module*
lookup_module(const Path& p) {
  auto iter = modules_.find(p);
  if (iter != modules_.end())
    return iter->second;
  return nullptr;
}

// Register the module to the given path. No module shall have been
// previously associated with the given path.
Module*
register_module(Module* m) {
  steve_assert(modules_.count(m->path()) == 0, 
               format("module '{}' already registered", m->path()));
  modules_.insert({m->path(), m});
  return m;
}

Module*
make_module(const Path& p, Decl_seq* ds) {
  return make_expr<Module>(no_location, get_typename_type(), p, ds);
}

// -------------------------------------------------------------------------- //
// Module loading

// Parse the module in the given file, returning its sequence of
// declarations.
//
// FIXME: We should be recurs
Decl_seq*
parse_module(std::ifstream& fs) {
  // Save off the current diagnostics so we don't overwrite them
  // with the lexer, parser, and elaborator.
  //
  // TODO: This is dumb; those components should do that already.
  Diagnostics_guard guard;

  // Get the text.
  using Iter = std::istreambuf_iterator<char>;
  std::string text(Iter{fs}, Iter{});

  // Lex the module.
  Lexer lex;
  Tokens toks = lex(text);

  // Parse the module.
  Parser parse;
  Tree* pt = parse(toks);
  if (not parse.diags.empty()) {
    std::cerr << parse.diags;
    return nullptr;
  }

  // Elaborate the contents.
  Elaborator elab;
  Expr* ast = elab(pt);
  if (not elab.diags.empty()) {
    std::cerr << elab.diags;
    return nullptr;
  }
  return as<Top>(ast)->decls();
}

Module*
load_file_module(Location loc, std::ifstream& fs, const Path& p) {
  if (Decl_seq* ds = parse_module(fs))
    return register_module(make_module(p, ds));
  else
    return nullptr;
}

// Try to load a module from the file with the given name.
Module*
load_file_module(Location loc, const Path& p) {
  if (Module* m = lookup_module(p))
    return m;
  std::ifstream fs(p.c_str());
  if (fs)
    return load_file_module(loc, fs, p);
  error(loc) << format("could not open module '{}'", p.c_str());
  return nullptr;
}

// Create a module corresponding to the given directory. When loaded
// the directory contains a top-declration with an empty sequence
// of declarations.
//
// FIXME: Check directory permissions on the path name.
//
// TODO: It should be possible to load code for a directory
// module. For example, in Python, this code is in the __init__.py
// file. We could have a nested file named __module__.steve
// provide the same benefit.
Module*
load_directory_module(Location loc, const Path& p) {
  if (Module* m = lookup_module(p))
    return m;
  Decl_seq* ds = new Decl_seq();
  return register_module(make_module(p, ds));
}


// Construct a path to a directory from a module-id.
inline Path
steve_dir_name(String id) { return id.str(); }

// Construct a path ot a Steve file from a module-id.
inline Path
steve_file_name(String id) { return id.str() + ".steve"; }

// Try to load a module in the given directory with the
// given name.
Module*
try_load_module(const Location& loc, const Path& root, String id) {
  // Try a directory module.
  Path dir = root / steve_dir_name(id);
  if (fs::exists(dir))
    return load_directory_module(loc, dir);

  // Try a file module.
  Path file = root / steve_file_name(id);
  if (fs::exists(file))
    return load_file_module(loc, file);

  return nullptr;
}

// Load the module from the given file or directory.
Module*
load_module(const Location& loc, const Path& dir, String id) {
  if (Module* m = try_load_module(loc, dir, id))
    return m;
  error(loc) << format("no module named '{}' in '{}'", id, dir.c_str());
  return nullptr;
}

// Search in the module paths for a module with the given name.
// The search order is defined as:
//
// - search starting from the current working directory.
// - search each module in the module path.
Module*
find_module(const Location& loc, String id) {
  // Search locally.
  if (Module* m = try_load_module(loc, fs::current_path(), id))
    return m;

  // Look in the module path.
  for(const Path& search : config().module_path)
    if (Module* m = try_load_module(loc, search, id))
      return m;

  error(loc) << format("no module named '{}' in module path", id);
  return nullptr;
}

} // namespace

// Load a module. The search rules are as follows;
//
// - if parent is null, search each directory in the module path
//   for a module with the name `id` or `id.steve`;
// - otherwise, the parent module is a directory, search for
//   a nested module named `id` or `id.steve`.
//
// TODO: Big project. It would be great if we saved this as in
// a lowered, elaborated form. 
Module*
load_module(Location loc, Module* parent, String id) {
  if (not parent)
    return find_module(loc, id);
  else {
    if (Module* m = load_module(loc, parent->path(), id)) {
      Name* n = new Basic_id(id);
      Decl* d = new Import(n, m);
      parent->decls()->push_back(d);
      return m;
    }
  }
  return nullptr;
}


// Load a module.
Module*
load_module(Module* parent, String id) { 
  return load_module(no_location, parent, id); 
}

} // namesapce steve
