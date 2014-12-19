
#include <steve/Type.hpp>
#include <steve/Decl.hpp>
#include <steve/Evaluator.hpp>
#include <steve/Debug.hpp>

#include <climits>
#include <algorithm>

namespace steve {

// -------------------------------------------------------------------------- //
// Type accessors

namespace {

Type* typename_;
Type* unit_;
Type* bool_;
Type* nat_;
Type* int_;
Type* char_;

} // namespace

void 
init_types() {
  typename_ = new Typename_type();
  unit_ = make_expr<Unit_type>(no_location, typename_);
  bool_ = make_expr<Bool_type>(no_location, typename_);
  nat_ = make_expr<Nat_type>(no_location, typename_);
  int_ = make_expr<Int_type>(no_location, typename_);
  char_ = make_expr<Char_type>(no_location, typename_);
}

// Returns the builtin typename type.
Type* 
get_typename_type() { return typename_; }

// Returns the unit type.
Type*
get_unit_type() { return unit_; }

// Returns the builtin bool type.
Type*
get_bool_type() { return bool_; }

// Returns the builtin nat type.
Type*
get_nat_type() { return nat_; }

// Returns the builtin int type.
Type*
get_int_type() { return int_; }

// Returns the builtin char type.
Type*
get_char_type() { return char_; }

// Create a type expression for type typename.
Type*
make_typename_type(const Location& l) { 
  return make_expr<Typename_type>(l, typename_);
}

// Create a type expression for type unit.
Type*
make_unit_type(const Location& l) { 
  return make_expr<Unit_type>(l, typename_);
}

// Create a type expression for type bool.
Type*
make_bool_type(const Location& l) { 
  return make_expr<Bool_type>(l, typename_);
}

// Create a type expression for type nat.
Type*
make_nat_type(const Location& l) { 
  return make_expr<Nat_type>(l, typename_);
}

// Create a type expression for type int.
Type*
make_int_type(const Location& l) { 
  return make_expr<Int_type>(l, typename_);
}

// Create a type expression for type char.
Type*
make_char_type(const Location& l) { 
  return make_expr<Char_type>(l, typename_);
}

// -------------------------------------------------------------------------- //
// Type categories

// Returns ture if t is the typename type.
bool
is_typename_type(Type* t) { return t->kind == typename_type; }

// Returns true if t is a boolean type. The boolean types are
// bool and bitfield(bool, _, _).
bool
is_boolean_type(Type* t) {
  if (is<Bool_type>(t))
    return true;
  if (Bitfield_type* b = as<Bitfield_type>(t))
    return is_boolean_type(b->type());
  return false;
}

// Returns true if t is an intgral type. The integral types are
// nat, int, bitfield(nat, _, _), and bitfield(int, _, _).
bool
is_integral_type(Type* t) {
  if (is<Nat_type>(t) or is<Int_type>(t))
    return true;
  if (Bitfield_type* b = as<Bitfield_type>(t))
    return is_integral_type(b->type());
  return false;
}

// Returns true if t is the type of a type function. That is,
// the result type of t is `typename`.
bool
is_type_constructor_type(Type* t) {
  if (Fn_type* f = as<Fn_type>(t))
    return is_typename_type(f->result());
  return false;
}

// -------------------------------------------------------------------------- //
// Type queries

namespace {

Type*
parm_type(Decl* p) { return as<Parm>(p)->type(); }

Type_seq*
get_parm_type_list(Decl_seq* ps) {
  Type_seq* ts = new Type_seq(ps->size(), nullptr);
  std::transform(ps->begin(), ps->end(), ts->begin(), parm_type);
  return ts;
}

} // namespace

// Given a sequence of paramters and a return type, create a
// function type.
Type*
make_fn_type(Decl_seq* ps, Type* e) {
  Type_seq* ts = get_parm_type_list(ps);
  return new Fn_type(ts, e);
}


namespace {

// Returns the size of an integer on the target architecture.
//
// FIXME: This is currently totally broken. Its returning the size
// on the host architecture; we need legit architecture support. 
constexpr int
default_int_size() { return sizeof(int); }

} // namespace

Integer
size_in_bits(Type* t) {
  steve_assert(is_integral_type(t), format("'{}' is not an integral type", debug(t)));
  if (is<Nat_type>(t) || is<Int_type>(t))
    return default_int_size() * CHAR_BIT;
  if (Bitfield_type* b = as<Bitfield_type>(t))
    return eval_integer(b->width());
  steve_unreachable("unknown integral type");
}


} // namespace steve
