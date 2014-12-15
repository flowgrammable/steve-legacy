
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
inline Module*
lookup_module(const Path& p) {
  auto iter = modules_.find(p);
  if (iter != modules_.end())
    return iter->second;
  return nullptr;
}

// Register the module to the given path. No module shall have been
// previously associated with the given path.
inline Module*
register_module(Module* m) {
  steve_assert(modules_.count(m->path()) == 0, 
               format("module '{}' already registered", m->path().c_str()));
  modules_.insert({m->path(), m});
  return m;
}

// Create an initial module. This allocates the module type, but leaves
// the declaration sequence uninitialized.
inline Module*
init_module(const Path& p, Name* n) {
  return make_expr<Module>(no_location, get_typename_type(), p, n, nullptr);
}

// Create an initial module. This allocates the module type, but leaves
// the declaration sequence uninitialized.
inline Module*
make_module(const Path& p, Name* n, Decl_seq* ds) {
  return make_expr<Module>(no_location, get_typename_type(), p, n, ds);
}

// Complete the construction of the module by assigning it a sequence
// of nested declarations (possibly empty).
inline Module*
finish_module(Module* m, Decl_seq* ds) {
  m->second = ds;
  return m;
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
  // HACK: There's a GCC bug that causes exceptions to be thrown
  // when they aren't expected (e.g., underflow errors). 
  std::string text;
  try {
    using Iter = std::istreambuf_iterator<char>;
    text = std::string(Iter{fs}, Iter{});
  } catch (...) { 
    // Don't do anything here...
  }

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

// Load the file module.
Module*
load_file_module(const Location& loc, std::ifstream& fs, const Path& p, Name* n) {
  Module* m = register_module(init_module(p, n));
  if (Decl_seq* ds = parse_module(fs))
    return finish_module(m, ds);
  else
    return nullptr;
}

// Try to load a module from the file with the given name. A module 
// shall not import itself.
Module*
load_file_module(const Location& loc, const Path& p, Name* n) {
  if (Module* m = lookup_module(p)) {
    if (m->path() == p) {
      error(loc) << format("module '{}' imports itself", debug(n));
      return nullptr;
    }
    return m;
  }
  std::ifstream fs(p.c_str());
  if (fs)
    return load_file_module(loc, fs, p, n);
  error(loc) << format("could not open module '{}'", p.c_str());
  return nullptr;
}

// Create a module corresponding to the given directory. When loaded
// the directory contains a top-declaration with an empty sequence
// of declarations.
//
// FIXME: Check directory permissions on the path name.
//
// TODO: It should be possible to load code for a directory
// module. For example, in Python, this code is in the __init__.py
// file. We could have a nested file named __module__.steve
// provide the same benefit.
Module*
load_directory_module(const Location& loc, const Path& p, Name* n) {
  if (Module* m = lookup_module(p))
    return m;
  return register_module(make_module(p, n, new Decl_seq()));
}

// Try to load a directory module at path p.
Module*
try_load_dir(const Location& loc, const Path& p, Name* n) {
  if (fs::exists(p))
    return load_directory_module(loc, p, n);
  return nullptr;
}

// Try to load a file module at path p.
Module*
try_load_file(const Location& loc, const Path& p, Name* n) {
  if (fs::exists(p))
    return load_file_module(loc, p, n);
  return nullptr;
}

// Construct a path to a directory from a module-id.
inline Path
steve_dir_name(String id) { return id.str(); }

// Construct a path ot a Steve file from a module-id.
inline Path
steve_file_name(String id) { return id.str() + ".steve"; }

// Try to load a module in the given directory with the
// given name. Note that id could name a file module or
// a directory module.
Module*
try_load_module(const Location& loc, const Path& dir, String id, Name* n) {
  if (Module* m = try_load_dir(loc, dir / steve_dir_name(id), n))
    return m;
  if (Module* m = try_load_file(loc, dir / steve_file_name(id), n))
    return m;
  return nullptr;
}

// Load the module from the given file or directory.
Module*
load_module(const Location& loc, const Path& dir, String id, Name* n) {
  if (Module* m = try_load_module(loc, dir, id, n))
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
find_module(const Location& loc, String id, Name* n) {
  // Search locally.
  if (Module* m = try_load_module(loc, fs::current_path(), id, n))
    return m;

  // Look in the module path.
  for(const Path& search : config().module_path)
    if (Module* m = try_load_module(loc, search, id, n))
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
  Name* n = new Basic_id(id);
  if (not parent)
    return find_module(loc, id, n);
  else {
    if (Module* m = load_module(loc, parent->path(), id, n)) {
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

// Load the input file as a module.
//
// FIXME: The diagnostics are a bit wonky here... There should be a
// uniform way of managing diagnostics for module loading.
Module*
load_file(String id) {
  Diagnostics diags;
  Diagnostics_guard guard(diags);

  Path p = fs::current_path() / id.str();
  if (Module* m = try_load_file(no_location, p, new Basic_id(id)))
    return m;

  error(no_location) << format("error loading file '{}'", id);

  if (not diags.empty())
    std::cerr << diags;
  return nullptr;
}

} // namesapce steve
