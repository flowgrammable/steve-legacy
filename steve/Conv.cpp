
#include <steve/Conv.hpp>
#include <steve/Ast.hpp>
#include <steve/Type.hpp>

namespace steve {

// -------------------------------------------------------------------------- //
// Conversion relattion
//
// Find a conversion from the expression e to the type t according to
// the following rules:
//
//   convert(e, t) = e when e has type t
//   convert(e, N) = promote(e, N) when e has integral type
//   convert(e, B) = indicate(e, B) when e has boolean or integral type

namespace {

// If e has type t, no conversion is reuqired.
Expr*
identity(Expr* e, Type* t) {
  Type* u = get_type(e);
  if (is_same(u, t))
    return e;
  return nullptr;
}

// If e has integral type and t is integral, convert by integer
// promotion.
Expr*
promote(Expr* e, Type* t) {
  Type* u = get_type(e);
  if (is_integral_type(u) and is_integral_type(t))
    return make_expr<Promo>(e->loc, t, e, t);
  return nullptr;
}

// If e has integral or boolean type, and t is boolean,
// convert by predicate evaluation.
Expr*
predicate(Expr* e, Type* t) {
  Type* u = get_type(e);
  if ((is_boolean_type(u) || is_integral_type(u)) && is_boolean_type(t))
    return make_expr<Pred>(e->loc, t, e, t);
  return nullptr;
}

} // namespace

// Find a rule that allows the conversion of e to T.
Expr*
convert(Expr* e, Type* t) {
  // Identity conversions
  if (Expr* c = identity(e, t))
    return c;

  // Integral promotion, boolean predication
  if (Expr* c = promote(e, t))
    return c;
  if (Expr* c = predicate(e, t))
    return c;
  return nullptr;
}

} // naemspace steve

