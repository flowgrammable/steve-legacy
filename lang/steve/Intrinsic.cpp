
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
  int_,
  type_
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
  case type_: return get_typename_type();
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


Expr*
eval_net_str_type(Expr* n) {
  Type* type = get_typename_type();
  Term* size = as<Term>(n);
  return make_expr<Net_str_type>(no_location, type, size);
}

Expr*
eval_net_seq_type(Expr* t, Expr* p) {
  Type* type = get_typename_type();
  Type* elem = as<Type>(t);
  Term* pred = as<Term>(p);
  return make_expr<Net_seq_type>(no_location, type, elem, pred);
}

// -------------------------------------------------------------------------- //
// Evaluation support


// Returns the the value of the expression e. Bahavior is undefined if
// e is not fully evaluated.
inline Value
get_value(Expr* e) { return eval(e).as_value(); }

// Returns the boolean value of e.
inline bool
get_bool_value(Expr* e) { return get_value(e).as_bool(); }

// Returns the integer value of e.
inline Integer
get_integer_value(Expr* e) { return get_value(e).as_integer(); }

// Returns the type value of e.
inline Type*
get_type_value(Expr* e) { return get_value(e).as_type(); }


// -------------------------------------------------------------------------- //
// Constant values

// Returns true.
inline Expr*
truth() { return to_expr(Value(true), get_bool_type()); }

// Returns false.
inline Expr*
falsity() { return to_expr(Value(false), get_bool_type()); }


// -------------------------------------------------------------------------- //
// Equality and inequality

// Returns the truth value.
Expr*
unit_equal(Expr*, Expr*) { return truth(); }

// Returns the false value.
Expr*
unit_not_equal(Expr*, Expr*) { return falsity(); }

// Returns true when the boolean values a and b are the same.
Expr*
boolean_equal(Expr* a, Expr* b) {
  Value result = get_bool_value(a) == get_bool_value(b);
  return to_expr(result, get_bool_type()); 
}

// Returns true when the boolean values a and b are different.
Expr*
boolean_not_equal(Expr* a, Expr* b) {
  Value result = get_bool_value(a) != get_bool_value(b);
  return to_expr(result, get_bool_type()); 
}

// Returns true when the integer expressions a and b have the
// same value.
Expr*
integer_equal(Expr* a, Expr* b) {
  Value result = get_integer_value(a) == get_integer_value(b);
  return to_expr(result, get_bool_type()); 
}

// Returns true when the integer expressions a and b have different
// values.
Expr*
integer_not_equal(Expr* a, Expr* b) {
  Value result = get_integer_value(a) != get_integer_value(b);
  return to_expr(result, get_bool_type()); 
}

// Returns true when two types are the same.
Expr*
type_equal(Expr* a, Expr* b) {
  Value result = is_same(get_type(a), get_type(b));
  return to_expr(result, get_bool_type());
}

// Returns true when two types are different.
Expr*
type_not_equal(Expr* a, Expr* b) {
  Value result = not is_same(get_type(a), get_type(b));
  return to_expr(result, get_bool_type());
}

// -------------------------------------------------------------------------- //
// Ordering

// Returns true when the integer expressions a is less than the
// integer expression b.
Expr*
integer_less(Expr* a, Expr* b) {
  Value result = get_integer_value(a) < get_integer_value(b);
  return to_expr(result, get_bool_type()); 
}

// Returns true when the integer expressions a is greater than the
// integer expression b.
Expr*
integer_greater(Expr* a, Expr* b) {
  Value result = get_integer_value(a) > get_integer_value(b);
  return to_expr(result, get_bool_type()); 
}

// Returns true when the integer expressions a is less than or
// equal to the integer expression b.
Expr*
integer_less_equal(Expr* a, Expr* b) {
  Value result = get_integer_value(a) <= get_integer_value(b);
  return to_expr(result, get_bool_type()); 
}

// Returns true when the integer expressions a is greater than or
// equal to the integer expression b.
Expr*
integer_greater_equal(Expr* a, Expr* b) {
  Value result = get_integer_value(a) >= get_integer_value(b);
  return to_expr(result, get_bool_type()); 
}

// -------------------------------------------------------------------------- //
// Arithmetic operators
//
// TODO: These values need to be adjusted for overflow. Currently,
// they are computed in arbitary precision, which is not correct.
// See Integer.hpp for comments on how to fix this.

// Returns the sum of the integer expressions a and b.
Expr*
integer_addition(Expr* a, Expr* b) {
  Value result = get_integer_value(a) + get_integer_value(b);
  return to_expr(result, get_bool_type()); 
}

// Returns the difference of the integer expressions a and b.
Expr*
integer_subtraction(Expr* a, Expr* b) {
  Value result = get_integer_value(a) - get_integer_value(b);
  return to_expr(result, get_bool_type()); 
}

// Returns the product of the integer expressions a and b.
Expr*
integer_multiplication(Expr* a, Expr* b) {
  Value result = get_integer_value(a) * get_integer_value(b);
  return to_expr(result, get_bool_type()); 
}

// Returns the floor division of the integer expression
// a divided by the integer expression b.
Expr*
integer_division(Expr* a, Expr* b) {
  Value result = get_integer_value(a) / get_integer_value(b);
  return to_expr(result, get_bool_type()); 
}

// Returns the remainder of floor division of the integer expression
// a divided by the integer expression b.
Expr*
integer_remainder(Expr* a, Expr* b) {
  Value result = get_integer_value(a) % get_integer_value(b);
  return to_expr(result, get_bool_type()); 
}

Expr*
integer_negation(Expr* e) {
  Value result = -get_integer_value(e);
  return to_expr(result, get_bool_type());
}

// -------------------------------------------------------------------------- //
// Bitwise operators

Expr*
integer_bitwise_and(Expr* a, Expr* b) {
  Value result = get_integer_value(a) & get_integer_value(b);
  return to_expr(result, get_bool_type()); 
}

Expr*
integer_bitwise_or(Expr* a, Expr* b) {
  Value result = get_integer_value(a) | get_integer_value(b);
  return to_expr(result, get_bool_type()); 
}

Expr*
integer_bitwise_xor(Expr* a, Expr* b) {
  Value result = get_integer_value(a) ^ get_integer_value(b);
  return to_expr(result, get_bool_type()); 
}

Expr*
integer_bitwise_complement(Expr* e) {
  Value result = ~get_integer_value(e);
  return to_expr(result, get_bool_type()); 
}

// -------------------------------------------------------------------------- //
// Left and right shift
//
// Behavior is undefined when the right operand is negative or greater
// than or equal to the number of bits in the representation of the
// left operand.

// In an arithmetic left shift, 0's are inserted into the low
// order bit. Note that this is the same as a logical left shift.
Expr*
arithmetic_left_shift(Expr* a, Expr* b) {
  Value result = get_integer_value(a) << get_integer_value(b);
  return to_expr(result, get_nat_type());
}

// In an arithmetic left shift, the sign bit is preserved.
Expr*
arithmetic_right_shift(Expr* a, Expr* b) {
  Value result = get_integer_value(a) >> get_integer_value(b);
  return to_expr(result, get_nat_type());
}

// In a logical right shift, 0 is inserted into the high-order bit.
Expr*
logical_right_shift(Expr* a, Expr* b) {
  Value result = get_integer_value(a) >> get_integer_value(b);
  return to_expr(result, get_int_type());
}

// -------------------------------------------------------------------------- //
// Logical operators

// Short-circuiting boolean and.
Expr*
bool_and(Expr* a, Expr* b) {
  if (not get_bool_value(a))
    return falsity();
  else
    return reduce(b);
}

// Short-circuiting boolean or.
Expr*
bool_or(Expr* a, Expr* b) {
  if (get_bool_value(a))
    return truth();
  else
    return reduce(b);
}

// Logical not.
Expr*
bool_not(Expr* e) {
  Value result = not get_bool_value(e);
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
  Diagnostics diags;
  Diagnostics_guard guard(diags);

  // FIXME: Re-enable and finish all of the various builtin operators.
  Spec specs[] __attribute__ ((unused)) = {
    // equal to (a == b)
    { equal_equal_tok,    {unit_, unit_}, bool_, unit_equal},
    { equal_equal_tok,    {bool_, bool_}, bool_, boolean_equal},
    { equal_equal_tok,    {nat_, nat_}, bool_, integer_equal},
    { equal_equal_tok,    {int_, int_}, bool_, integer_equal},
    { equal_equal_tok,    {type_, type_}, bool_, type_equal},
    // not equal to (a != b)
    { bang_equal_tok,     {unit_, unit_}, bool_, unit_not_equal},
    { bang_equal_tok,     {bool_, bool_}, bool_, boolean_not_equal},
    { bang_equal_tok,     {nat_, nat_}, bool_, integer_not_equal},
    { bang_equal_tok,     {int_, int_}, bool_, integer_not_equal},
    { bang_equal_tok,     {type_, type_}, bool_, type_not_equal},
    // Less than (a < b)
    // { langle_tok,         {nat_, nat_}, bool_, integer_less},
    { langle_tok,         {int_, int_}, bool_, integer_less},
    // Greater than (a > b)
    // { rangle_tok,         {nat_, nat_}, bool_, integer_greater},
    { rangle_tok,         {int_, int_}, bool_, integer_greater},
    // Less than or equal to (a <= b)
    // { langle_equal_tok,   {nat_, nat_}, bool_, integer_less_equal},
    { langle_equal_tok,   {int_, int_}, bool_, integer_less_equal},
    // Greater than or equal to (a >= b)
    // { rangle_equal_tok,   {nat_, nat_}, bool_, integer_greater_equal},
    { rangle_equal_tok,   {int_, int_}, bool_, integer_greater_equal},
    // Addition
    // { plus_tok,           {nat_, nat_}, nat_, integer_addition},
    { plus_tok,           {int_, int_}, int_, integer_addition},
    // Subtraction
    // { minus_tok,          {nat_, nat_}, nat_, integer_subtraction},
    { minus_tok,          {int_, int_}, int_, integer_subtraction},
    // Multiplication
    // { star_tok,           {nat_, nat_}, nat_, integer_multiplication},
    { star_tok,           {int_, int_}, int_, integer_multiplication},
    // Division
    // { slash_tok,          {nat_, nat_}, nat_, integer_division},
    { slash_tok,          {int_, int_}, int_, integer_division},
    // Remainder
    // { percent_tok,        {nat_, nat_}, nat_, integer_remainder},
    { percent_tok,        {int_, int_}, int_, integer_remainder},
    // Negation
    { minus_tok,          {nat_}, nat_, integer_negation},
    // Bitwise operations
    { ampersand_tok,      {nat_, nat_}, nat_, integer_bitwise_and},
    { pipe_tok,           {nat_, nat_}, nat_, integer_bitwise_or},
    { caret_tok,          {nat_, nat_}, nat_, integer_bitwise_xor},
    { tilde_tok,          {nat_, nat_}, nat_, integer_bitwise_complement},
    // Left shift
    { langle_langle_tok,  {nat_, nat_}, nat_, arithmetic_left_shift},
    { langle_langle_tok,  {int_, nat_}, int_, arithmetic_left_shift},
    // Right shift
    { rangle_rangle_tok,  {nat_, nat_}, nat_, logical_right_shift},
    { rangle_rangle_tok,  {int_, nat_}, int_, arithmetic_right_shift},
    // Logical operators
    { and_tok,            {bool_, bool_}, bool_, bool_and},
    { or_tok,             {bool_, bool_}, bool_, bool_or},
    { not_tok,            {bool_, bool_}, bool_, bool_not},

    { "bitfield",         {typename_, nat_, nat_}, typename_, eval_bitfield},
    { "__net_str",        {nat_}, typename_,                  eval_net_str_type},

    // FIXME: the second argument tyoe should be (t)->bool 
    // where t is the type given as the first argument.
    { "__net_seq",        {typename_, bool_}, typename_,       eval_net_seq_type},
  };

  // Extract the declared features
  // TODO: Actually do this, or just allow lookups in the usual way?
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
