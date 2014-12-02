
#ifndef STEVE_VALUE_HPP
#define STEVE_VALUE_HPP

#include <steve/String.hpp>
#include <steve/Integer.hpp>
#include <steve/Ast.hpp>
#include <steve/Debug.hpp>

namespace steve { 

// -------------------------------------------------------------------------- //
// Values

// Enumerates the different kinds of values.
enum Value_kind {
  unit_value,
  bool_value,
  integer_value,
  function_value,
  type_value
};

// Internal representation of the a value.
union Value_data {
  Value_data() { }
  Value_data(unit_t u) : u(u) { }
  Value_data(bool b) : b(b) { }
  Value_data(Integer n) : n(n) { }
  Value_data(Fn* f) : f(f) { }
  Value_data(Type* t) : t(t) { }
  ~Value_data() { }

  unit_t  u; // The unit value
  bool    b; // A boolean value
  Integer n; // An integer value
  Fn*     f; // An function
  Type*   t; // A type
};


// The value class denotes the value of an elaborated expression
// that can be computed at compile time. Values are represented as a 
// variant: one of the different kinds of values that can be the 
// result of an expression.
//
// TODO: Expand this to cover other built in types.
struct Value {
  // Copy semantics
  Value(const Value&);
  Value& operator=(const Value&);

  // Move semantics
  Value(Value&&);
  Value& operator=(Value&&);

  Value(unit_t);
  Value(bool);
  Value(Integer);
  Value(Fn*);
  Value(Type*);
  ~Value();

  // Extraction
  unit_t as_unit() const;
  bool as_bool() const;
  const Integer& as_integer() const;
  Fn* as_function() const;
  Type* as_type() const;

  Value_kind kind;
  Value_data data;
};

bool is_unit(const Value&);
bool is_bool(const Value&);
bool is_integer(const Value&);
bool is_function(const Value&);
bool is_type(const Value&);

// -------------------------------------------------------------------------- //
// Evaluations

// The union representation of value-expressions.
union Eval_data {
  Eval_data() { }
  Eval_data(const Value& v) : v(v) { }
  Eval_data(Value&& v) : v(std::move(v)) { }
  Eval_data(Expr* e) : e(e) { }
  ~Eval_data() { }
  Value v;
  Expr* e;
};

// The evaluation class embodies the result of partially evaluating
// an expression. It allows the evaluator to opportunistically evaluate
// some parts of an expression while leaving other aspects unevaluated.
struct Eval {
  using Data = Eval_data;

  // Copy semantics
  Eval(const Eval&);
  Eval& operator=(const Eval&);

  // Move semantics
  Eval(Eval&&);
  Eval& operator=(Eval&&);

  // Initialization
  Eval(const Value& v);
  Eval(Expr* e);

  // Extraction
  const Value& as_value() const;
  Expr* as_expr() const;

  bool partial;
  Data data;
};

// A sequence of evaluations. This is used, for example to store
// the results of evaluating function arguments.
using Eval_seq = std::vector<Eval>;

bool is_value(const Eval&);
bool is_partial(const Eval&);
bool all_values(const Eval_seq&);

Expr* to_expr(const Eval&, Type*);
Expr* to_expr(const Eval&, Expr*);
Expr_seq* to_exprs(const Eval_seq&, Expr_seq*);

} // namespace steve

#include <steve/Value.ipp>

#endif
