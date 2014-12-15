
#include <steve/Decl.hpp>
#include <steve/Ast.hpp>
#include <steve/Debug.hpp>

namespace steve {

namespace {

// Return the name of a declaration.
template<typename T>
  inline Name*
  decl_name(T* t) { return t->first; }

} // namespace

// Returns the name of the declaration.
Name* 
get_name(Decl* d) {
  switch (d->kind) {
  case def_decl: return decl_name(as<Def>(d));
  case parm_decl: return decl_name(as<Parm>(d));
  case field_decl: return decl_name(as<Field>(d));
  case enum_decl: return decl_name(as<Enum>(d));
  case import_decl: return decl_name(as<Import>(d));
  default: break;
  }
  steve_unreachable(format("no declaration name for '{}'", node_name(d)));
}

// When 'd' declares a function, return that. Otherwise,
// return nullptr.
Fn*
get_declared_fn(Decl* d) {
  if (Def* def = as<Def>(d))
    return as<Fn>(def->init());
  return nullptr;
}

} // namespace steve
