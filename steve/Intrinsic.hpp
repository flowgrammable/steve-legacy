
#ifndef STEVE_INTRINSIC_HPP
#define STEVE_INTRINSIC_HPP

#include <steve/Ast.hpp>

// The intrinsic module houses the implementations of all steve
// functions built into the language. This includes the definition
// of all builtin type constructors (e.g., bitfield, array, etc),
// and all builtin operators (addition of nats, ints, etc).

namespace steve {

struct Expr;
struct Subst;

// An intrinsic function defines a built-in function. This is
// the base class of all intrinsic implementations.
struct Intrinsic : Fn {
  Intrinsic(Decl_seq* ps, Type* t)
    : Fn(ps, t, nullptr) { }
  
  virtual Expr* subst(const Subst& s) const = 0;
};

// The bitfield function is responsible for the creation of bitfield
// types.
struct Bitfield_fn : Intrinsic { 
  Bitfield_fn();

  Expr* subst(const Subst&) const; 
  
  Decl* type_parm() const { return (*parms())[0]; }
  Decl* width_parm() const { return (*parms())[1]; }
  Decl* order_parm() const { return (*parms())[2]; }
};


// Accessors

Decl* get_bitfield_ctor();


} // namespace steve

#endif
