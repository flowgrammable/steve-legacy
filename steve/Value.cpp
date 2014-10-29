
#include <steve/Value.hpp>
#include <steve/Type.hpp>

#include <algorithm>

namespace steve {

namespace {

// Reconstitute an expression from a value. The source expression
// e is provides the source location and type of of the reconstituted
// expression.
Expr* 
to_expr(const Location& l, const Value& v, Type* t) {
  switch (v.kind) {
  case unit_value: return make_expr<Unit>(l, t);
  case bool_value: return make_expr<Bool>(l, t, v.as_bool());
  case integer_value: return make_expr<Int>(l, t, v.as_integer());
  case function_value: return v.as_function();
  case type_value: return v.as_type();
  default: break;
  }
  steve_unreachable("unhandled value kind");
}

// Reconstitute an expression from a value. The source expression
// e is provides the source location and type of of the reconstituted
// expression.
Expr* 
to_expr(const Value& v, Expr* e) {
  return to_expr(e->loc, v, get_type(e));
}

} // namespace

// Returns true if all evaluations have resulted in values.
bool
all_values(const Eval_seq& evals) {
  return std::all_of(evals.begin(), evals.end(), is_value);
}

// Return an expression representing the result of the evaluation
// and give it type t.
Expr*
to_expr(const Eval& eval, Type* t) {
  if (is_partial(eval))
    return eval.as_expr();
  else
    return to_expr(no_location, eval.as_value(), t);
}


// Return an expression representing the result of the evaluation.
// The expression e provides typing and context for the result when
// the evaluation is not partial.
Expr*
to_expr(const Eval& eval, Expr* e) {
  if (is_partial(eval))
    return eval.as_expr();
  else
    return to_expr(eval.as_value(), e);
}

Expr_seq*
to_exprs(const Eval_seq& evals, Expr_seq* exprs) {
  Expr_seq* result = new Expr_seq();
  auto first1 = evals.begin();
  auto first2 = exprs->begin();
  while (first1 != evals.end()) {
    result->push_back(to_expr(*first1, *first2));
    ++first1;
    ++first2;
  }
  return result;
}

} // namesapce steve
