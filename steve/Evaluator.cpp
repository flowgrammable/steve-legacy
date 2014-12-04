
#include <steve/Evaluator.hpp>
#include <steve/Ast.hpp>
#include <steve/Type.hpp>
#include <steve/Subst.hpp>
#include <steve/Error.hpp>
#include <steve/Debug.hpp>

namespace steve {

// Declarations
namespace {

Eval eval_expr(Expr*);

} // namespace



// -------------------------------------------------------------------------- //
// Evaluator class

// TODO: See notes about diagnostics on the class definition.
Eval
Evaluator::operator()(Expr* e) { return eval_expr(e); }

namespace {

// -------------------------------------------------------------------------- //
// Partial evaluation helpers
//
// The following functions provide explicit construction of partial
// and normal forms. These are used to clearly indicate whether the
// result of evaluation is a value or an expression.

inline Eval 
value(unit_t) { return Value(unit); }

inline Eval
value(bool b) { return Value(b); }

inline Eval
value(Integer n) { return Value(n); }

inline Eval
value(Fn* f) { return Value(f); }

inline Eval
value(Type* t) { return Value(t); }

inline Eval 
partial(Expr* e) { return e; }


// -------------------------------------------------------------------------- //
// Evaluation rules

// Evaluate a declaration id. If the declaration's initializer is
// a value (fully reduced), then substitute the declaration reference
// with that value. Otherwise, preserve the reference for code
// generation.
//
// Note that a definitions' initializer was previously evaluated. 
// However, we need to re-evaluate to get into eval form. This is
// effectively just a conversion.
//
// Note that a reference to a parameter is not expanded. It is
// preserved as a declaration id. These are generally removed
// by substitution.
//
// TODO: The semantics of this evaluation need to be specified
// more thoroughly. When do we actually need to collapse a
// reference and when can we not do it. 
Eval
eval_decl_id(Decl_id* t) {
  Decl* decl = t->decl();
  if (Def* def = as<Def>(decl))
    return eval_expr(def->init());
  else if (is<Parm>(decl))
    return partial(t);
  else
    return partial(decl);
}


// Evaluate a literal by simply extracting its value.
template<typename T>
  inline Eval
  eval_literal(T* e) { return value(e->value()); }


// Evaluate a sequence of arguments.
Eval_seq
eval_args(Expr_seq* args) {
  Eval_seq result;
  for (Expr* a : *args)
    result.push_back(eval_expr(a));
  return result;
}

Eval
eval_fn_call(Fn* fn, Expr_seq* args) {
  Subst s {fn->parms(), args};
  Expr* r = subst(fn, s);
  return eval_expr(r);
}


Eval
eval_builtin_call(Builtin* b, Expr_seq* args) {
  Expr_seq& a = *args;
  if (b->arity() == 1)
    return eval_expr(b->fn().f1(a[0]));
  if (b->arity() == 2)
    return eval_expr(b->fn().f2(a[0], a[1]));
  if (b->arity() == 3)
    return eval_expr(b->fn().f3(a[0], a[1], a[2]));
  steve_unreachable(format("{}: evaluation failure: invalid arity", b->loc));
}

// Evaluate the call of a function declaration by applying the arguments
// to its definition.
Eval
eval_call(Call* e) {
  // Reduce the function.
  std::cout << "HERE: " << e->loc << ":" << debug(e->fn()) << '\n';
  Term* tgt = as<Term>(reduce(e->fn()));

  // Evaluate the function arguments first.
  Eval_seq evals = eval_args(e->args());
  Expr_seq* args = to_exprs(evals, e->args());

  // If all of the arguments are fully reduced, then evaluate the
  // function call. Otherwise, replace the call's arguments with
  // the partially evaluated results.
  if (all_values(evals)) {
    if (Fn* fn = as<Fn>(tgt))
      return eval_fn_call(fn, args);
    if (Builtin* b = as<Builtin>(tgt))
      return eval_builtin_call(b, args);
    steve_unreachable(format("{}: evalutaion failure: invalid call target"));
  } else {
    // Rewrite the call expression with the reduced function and
    // arguments.
    e->first = tgt;
    e->second = args;
    return partial(e);
  }

  steve_unreachable(format("{}: evaluation failure: '{}' is not a function", 
                    e->loc,
                    debug(e->fn())));
}

// Check that the value of n can fit in the representation of t. Here,
// e is the expression inducing the check (e.g., a promotion). It
// provides diagnostic context.
void
check_overflow(const Integer& n, Type* t, Expr* e) {
  if (n.bits() > size_in_bits(t))
    error(e->loc) << format("promotion of '{}' exceeds size of '{}'",
                             n, debug(t));
}

// Evlauate the term promote(e, N) where e has integral type and
// N is an integral type.
Eval
eval_promo(Promo* e) {
  // Partially evalutae the expression. If its not fully reduced,
  // reconstitute the expression and return it.
  Eval v = eval_expr(e->expr());
  
  // Otherwise, make sure that the result can fit into the bits
  // provided by the target. Emit an error, but continue evaluating.
  //
  // FIXME: Am I sure that the v is an integral value?
  if (is_value(v)) {
    const Value& val = v.as_value();
    check_overflow(val.as_integer(), e->type(), e);
  }
  return v;
}

// Evaluate the term predicate(e, B) where e has integral or bolean
// type. The expression results in 'true' when e is true or non-zero,
// and 'false' otherwise.
Eval
eval_pred(Pred* e) {
  // Partially evalutae the argument. If not fully reduced, return
  // the reconstituted value as a reconstituted expression.
  Eval v = eval_expr(e->expr());
  if (is_partial(v))
    return v;
  const Value& val = v.as_value();

  // Evalutae the predicate. 
  bool test;
  if (val.kind == integer_value)
    test = (val.as_integer() != 0);
  else if (val.kind == bool_value)
    test = val.as_bool();
  else
    steve_unreachable("unhanded boolean value");

  return value(test);
}

// FIXME: Unify this with the basic function call mechanism.
Eval
eval_binary(Binary* e) {
  // Reduce the function.
  Term* tgt = as<Term>(reduce(e->fn()));

  // Evaluate the function arguments first.
  Expr_seq init {e->left(), e->right()};
  Eval_seq evals = eval_args(&init);
  Expr_seq* args = to_exprs(evals, &init);

  // If all of the arguments are fully reduced, then evaluate the
  // function call. Otherwise, replace the call's arguments with
  // the partially evaluated results.
  if (all_values(evals)) {
    if (Fn* fn = as<Fn>(tgt))
      return eval_fn_call(fn, args);
    if (Builtin* b = as<Builtin>(tgt))
      return eval_builtin_call(b, args);
    steve_unreachable(format("{}: evalutaion failure: invalid call target"));
  } else {
    // Build a new partially evalutaed term.
    e->first = tgt;
    e->second = (*args)[0];
    e->third = (*args)[1];
    return partial(e);
  }

  steve_unreachable(format("{}: evaluation failure: '{}' is not a function", 
                    e->loc,
                    debug(e->fn())));
}


Eval
eval_expr(Expr* e) {
  // All types are values.
  if (Type* t = as<Type>(e))
    return value(t);

  // Declarations are not values.
  if (Decl* d = as<Decl>(e))
    return partial(d);

  switch (e->kind) {
  // Names
  case decl_id: return eval_decl_id(as<Decl_id>(e));
  // Terms
  case unit_term: return eval_literal(as<Unit>(e));
  case bool_term: return eval_literal(as<Bool>(e));
  case int_term: return eval_literal(as<Int>(e));
  // Misc terms
  case block_term: return e;
  case fn_term: return value(as<Fn>(e));
  case call_term: return eval_call(as<Call>(e));
  case promo_term: return eval_promo(as<Promo>(e));
  case pred_term: return eval_pred(as<Pred>(e));
  case range_term: return e;
  case unary_term: return e;
  case binary_term: return eval_binary(as<Binary>(e));
  case builtin_term: return e;
  default:
    break;
  }
  steve_unreachable(format("evaluation of unknown expression '{}'", debug(e)));
}

} // namespace

// Partially evaluate the given expression, returning it evaluation.
// The result of evaluating can be either a value in normal form
// or a reduced but partially evaluated function.
Eval
eval(Expr* e) {
  Evaluator ev;
  return ev(e);
}

// Partially evaluate an expression, returning a new expression.
Expr*
reduce(Expr* e) {
  Eval v = eval(e);
  return to_expr(v, e);
}

// Evaluate t, expecting a boolean value. Behavior is undefined if
// t does not result in a boolean value.
bool
eval_boolean(Term* t) {
  Evaluator ev;
  Eval v = ev(t);
  return v.as_value().as_bool();
}

// Evaluate t, expecting an integer value. Behavior is undefined if
// t does not reduce an integer value.
Integer
eval_integer(Term* t) {
  Evaluator ev;
  Eval v = ev(t);
  return v.as_value().as_integer();
}

} // namespace steve
