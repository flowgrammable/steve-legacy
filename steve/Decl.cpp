
#include <steve/Decl.hpp>
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
  default: break;
  }
  steve_unreachable(format("no declaration name for '{}'", node_name(d)));
}


} // namespace steve
