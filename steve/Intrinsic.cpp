
#include <steve/Intrinsic.hpp>
#include <steve/Token.hpp>
#include <steve/Language.hpp>
#include <steve/Scope.hpp>
#include <steve/Type.hpp>
#include <steve/Value.hpp>
#include <steve/Evaluator.hpp>
#include <steve/Error.hpp>
#include <steve/Debug.hpp>

#include <algorithm>
#include <iterator>

namespace steve {

namespace {

// Builtin functions
Expr* eval_bitfield(Expr*, Expr*, Expr*);

// -------------------------------------------------------------------------- //
// Construction framework
//
// The following facilities support a declarative style of creating
// and resolving builtin facilities.

enum Typename {
  typename_,
  unit_,
  bool_,
  nat_,
  int_
};

using Typenames = std::initializer_list<Typename>;

using Builtin_fn = Builtin::Fn;
using Arity1 = Builtin::Unary;
using Arity2 = Builtin::Binary;
using Arity3 = Builtin::Ternary;

// Create a name.
inline Name*
make_name(const char* n) { return new Basic_id(n); }

inline Name*
make_name(Token_kind k) { return new Operator_id(token_name(k)); }

// Make a tpye corresponding to the type selector.
Type*
make_type(Typename t) {
  switch (t) {
  case typename_: return get_typename_type();
  case unit_: return get_unit_type();
  case bool_: return get_bool_type();
  case nat_: return get_nat_type();
  case int_: return get_int_type();
  default: steve_unreachable("unspecified builtin type.");
  }
}

// Make a sequence of types.
Type_seq*
make_parms(Typenames ts) {
  Type_seq* rs = new Type_seq();
  for (Typename t : ts)
    rs->push_back(make_type(t));
  return rs;
}

inline Builtin*
make_builtin(Arity1 f) { return new Builtin(1, f); }

inline Builtin*
make_builtin(Arity2 f) { return new Builtin(2, f); }

inline Builtin*
make_builtin(Arity3 f) { return new Builtin(3, f); }

// The Spec class provides a partial constructor for a builtin
// function. That is, it partially allocates many of the resources
// based on simplified inputs, and these are used to complete
// the declaration in a subsequent pass.
//
// FIXME: Factor common initializations?
struct Spec {
  template<typename N, typename F>
    Spec(N n, Typenames ps, Typename r, F f)
      : name(make_name(n))
      , parms(make_parms(ps))
      , result(make_type(r))
      , fn(make_builtin(f))
      , def()
      , ovl()
     { complete(); }

  void complete();
  
  Name*     name;
  Type_seq* parms;
  Type*     result;
  Builtin*  fn;
  Decl*     def;
  Overload* ovl;
};

void
Spec::complete() {
  // Finish the function typoe
  Type* type = new Fn_type(parms, result);
  fn->tr = type;

  // Build the declaration.
  def = new Def(name, type, fn);
  def->tr = type;

  // Declare it, saving the overload set.
  ovl = declare(def);
}

// -------------------------------------------------------------------------- //
// Builtin implementations

Expr*
eval_bitfield(Expr* t, Expr* n, Expr* b) {
  // Get the underlying type of the bitfield. 
  // TODO: It must be one of bool, nat, int, or char.
  Type* t1 = as<Type>(t);
  steve_assert(t, format("'{}' is not a valid type for a bitfield", debug(t)));

  // Get the width of the bitfield.
  // TODO: Is n partially evaluated?
  // TODO: Ensure that n is a valid selection for the
  // given type.
  Int* n1 = as<Int>(n);
  steve_assert(n, format("invalid bitifield witdh '{}'", debug(n)));

  // Get the optional boolean value. If omitted, the selector
  // chooses the natural byte order of the host architecture.
  //
  // TODO: Is b partially evaluated?
  // TODO: Ensure that the byte order selector is valid for the
  // type and witdth (e.g., byte order selectors on sub-byte
  // bitfields don't make a lot of sense).
  Int* b1 = as<Int>(b);
  steve_assert(b1, format("invalid byte order selector '{}'", debug(b)));

  // FIXME: Do I need a source location or not?
  return make_expr<Bitfield_type>(no_location, get_typename_type(), t1, n1, b1);
}

inline Value
get_value(Expr* e) { return eval(e).as_value(); }

Expr*
eq_unit(Expr* a, Expr* b) {
  Value result = true;
  return to_expr(result, get_bool_type()); 
}

Expr*
eq_bool(Expr* a, Expr* b) {
  Value result = get_value(a).as_bool() == get_value(b).as_bool();
  return to_expr(result, get_bool_type()); 
}

Expr*
eq_nat(Expr* a, Expr* b) {
  Value result = get_value(a).as_integer() == get_value(b).as_integer();
  return to_expr(result, get_bool_type()); 
}

Expr*
eq_int(Expr* a, Expr* b) {
  Value result = get_value(a).as_integer() == get_value(b).as_integer();
  return to_expr(result, get_bool_type()); 
}


// -------------------------------------------------------------------------- //
// Globals

// Intrinsic functions
Decl* bitfield_;

} // namespace


// -------------------------------------------------------------------------- //
// Builtin accessors

void
init_intrinsics() {
  Diagnostics_guard diags;
  Scope_guard scope(module_scope);

  // FIXME: Re-enable and finish all of the various builtin operators.
  Spec specs[] = {
    // Equality comparison: a == b
    { equal_equal_tok,    {unit_, unit_}, bool_, eq_unit },
    { equal_equal_tok,    {bool_, bool_}, bool_, eq_bool },
    { equal_equal_tok,    {nat_, nat_}, bool_, eq_nat },
    { equal_equal_tok,    {int_, int_}, bool_, eq_int },

    { "bitfield",         {typename_, nat_, nat_}, typename_, eval_bitfield }
  };
  (void)specs; // Suppress unused variable

  // Extract the declared features
  bitfield_ = lookup_single(make_name("bitfield"));

  // FIXME: This could be improved.
  if (not current_diagnostics()->empty()) {
    std::cerr << "internal compiler error\n";
    std::cerr << *current_diagnostics();
    steve_unreachable("");
  }
}



// -------------------------------------------------------------------------- //
// Builtin accessors

// Returns the bitfield function definition.
Decl*
get_bitfield() { return bitfield_; }

} // namespace steve
