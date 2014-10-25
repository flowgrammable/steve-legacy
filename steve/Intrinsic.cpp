
#include <steve/Intrinsic.hpp>
#include <steve/Language.hpp>
#include <steve/Type.hpp>
#include <steve/Subst.hpp>
#include <steve/Debug.hpp>

#include <algorithm>
#include <iterator>

namespace steve {

namespace {

using Type_desc = Type* (*)();
using Parm_desc = std::pair<const char*, Type_desc>;

// Create a name.
inline Name*
make_name(const char* n) { return new Basic_id(n); }

// Create a parameter
Parm*
make_parm(const char* name, Type_desc type) {
  Name* n = make_name(name);
  Type* t = type();
  Parm* p = make_expr<Parm>(no_location, t, n, t, nullptr);
  return p;
}

Decl_seq*
make_parms(std::initializer_list<Parm_desc> list) {
  Decl_seq* decls = new Decl_seq;
  for (const Parm_desc& d : list)
    decls->push_back(make_parm(d.first, d.second));
  return decls;
}

// Define an intrinsic function with the given name.
template<typename T>
  inline Decl* 
  define_fn(const char* s) {
    Fn* f = new T();
    Type* t =  get_fn_type(f);
    f->tr = t;
    Name* n = make_name(s);
    return make_expr<Def>(no_location, t, n, t, f);
  }

// Defines the bitfield constructor.
Decl*
make_bitfield_ctor() { return define_fn<Bitfield_fn>("bitfield"); }

// Intrinsic functions
Decl* bitfield_ctor_;

} // namespace


// -------------------------------------------------------------------------- //
// Intrinsic initialization and access

void
init_intrinsics() {
  bitfield_ctor_ = make_bitfield_ctor();
}

// Returns the bitfield function definition.
Decl*
get_bitfield_ctor() { return bitfield_ctor_; }


// -------------------------------------------------------------------------- //
// Type constructors


Bitfield_fn::Bitfield_fn()
  : Intrinsic(make_parms({ 
        {"t", get_typename_type},
        {"n", get_nat_type},
        {"o", get_nat_type}
      }),
      get_typename_type()) 
{ }

// FIXME: These arguments should be type checked prior to the
// substitution.
Expr*
Bitfield_fn::subst(const Subst& s) const {
  // Get the underlying type of the bitfield. 
  // TODO: It must be one of bool, nat, int, or char.
  Type* t = as<Type>(s.get(type_parm()));
  steve_assert(t, format("'{}' is not a valid type for a bitfield", debug(t)));

  // Get the width of the bitfield.
  // TODO: Is n partially evaluated?
  // TODO: Ensure that n is a valid selection for the
  // given type.
  Int* n = as<Int>(s.get(width_parm()));
  steve_assert(n, format("invalid bitifield witdh '{}'", debug(n)));

  // Get the optional boolean value. If omitted, the selector
  // chooses the natural byte order of the host architecture.
  //
  // TODO: Is b partially evaluated?
  // TODO: Ensure that the byte order selector is valid for the
  // type and witdth (e.g., byte order selectors on sub-byte
  // bitfields don't make a lot of sense).
  Int* o = as<Int>(s.get(order_parm()));
  steve_assert(o, format("invalid byte order selector '{}'", debug(o)));

  return make_expr<Bitfield_type>(loc, get_typename_type(), t, n, o);
}

} // namespace steve
