
#include <steve/Conv.hpp>
#include <steve/Ast.hpp>
#include <steve/Type.hpp>
#include <steve/Error.hpp>

namespace steve {

// -------------------------------------------------------------------------- //
// Conversion relation
//
// Find a conversion from the expression e to the type t according to
// the following rules:
//
//   convert(e, t) = e when e has type t
//   convert(e, N) = promote(e, N) when e has integral type
//   convert(e, B) = indicate(e, B) when e has boolean or integral type

namespace {

// If e has type t, no conversion is required.
Expr*
identity(Expr* e, Type* t) {
  Type* u = type(e);
  if (is_same(u, t))
    return e;
  return nullptr;
}

// If e has integral type and t is integral, convert by integer
// promotion.
Expr*
promote(Expr* e, Type* t) {
  Type* u = type(e);
  if (is_integral_type(u) and is_integral_type(t))
    return make_expr<Promo>(e->loc, t, e, t);
  return nullptr;
}

// If e has integral or boolean type, and t is boolean,
// convert by predicate evaluation.
Expr*
predicate(Expr* e, Type* t) {
  Type* u = type(e);
  if ((is_boolean_type(u) || is_integral_type(u)) && is_boolean_type(t))
    return make_expr<Pred>(e->loc, t, e, t);
  return nullptr;
}

Expr*
do_convert(Expr* e, Type* t) {
  if (Expr* c = identity(e, t))
    return c;
  if (Expr* c = promote(e, t))
    return c;
  if (Expr* c = predicate(e, t))
    return c;

  error(e->loc) << format("no known conversion from {} to '{}'", 
                          typed(e), 
                          debug(t));

  return nullptr;
}


} // namespace

// Find a rule that allows the conversion of the expression
// `e` to the type `T`. If no conversion is possible, emit a 
// diagnostic.
Expr*
convert(Expr* e, Type* t) { return do_convert(e, t); }

// Find a rule that allows the conversion of the term `e` to
// the type `T`. Emit a diagnostic if no conersion is possible.
Term*
convert(Term* e, Type* t) {
  if (Expr* r = do_convert(e, t)) {
    steve_assert(is<Term>(r), "term converted to non-term");
    return as<Term>(r);
  }
  return nullptr;
}

// Return a conversion to bool, or diagnose the failed conversion.
Term*
convert_to_bool(Term* t) { return convert(t, get_bool_type()); }

// Rank an expression based on its top-level conversions.
Conversion
rank_conversion(Expr* e) { return is<Promo>(e) or is<Pred>(e); }


// Get the ranked list of conversions from the arguments. This
// is a list of integers ranking the "difficulty" of each
// conversion.
//
// Currently, there are only two ranks: either a conversion is
// needed or it is not.
Conversion_list
rank_conversions(Expr_seq* args) {
  Conversion_list convs(args->size());
  for (std::size_t n = 0; n < args->size(); ++n)
    convs[n] = rank_conversion((*args)[n]);
  return convs;
}


} // naemspace steve

