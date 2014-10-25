
#ifndef STEVE_EVALUATOR_HPP
#define STEVE_EVALUATOR_HPP

#include <steve/Value.hpp>

// This module defines the interface for the partial (compile-time)
// evaluation engine of the Steve programming language. The primary
// interface is through the eval() functions defined in this file.

namespace steve {

// The evaluator is resposible for evaluating an expression as
// as a value.
//
// TODO: Figure out the diagnostic context. Errors could happen
// during elaboration, or during a separate program evaluation.
struct Evaluator {
  Eval operator()(Expr*);
};

Eval eval(Expr*);
Expr* reduce(Expr*);

bool eval_boolean(Term*);
Integer eval_integer(Term*);

} // namespace steve

#endif
