
#include <steve/Variant.hpp>
#include <steve/Ast.hpp>
#include <steve/Scope.hpp>
#include <steve/Type.hpp>
#include <steve/Conv.hpp>
#include <steve/Overload.hpp>
#include <steve/Evaluator.hpp>

namespace steve {

namespace {

// Get the function that will be used to compare the argument to
// the descriminator type.
Term*
make_discriminator_eq(Type* desc, Expr* arg) {
  Term* fake = make_expr<Default>(arg->loc, desc);
  Name* name = new Operator_id("==");
  Overload* ovl = lookup(name);
  Resolution res = resolve_binary(arg->loc, *ovl, arg, fake);
  if (!res.is_unique()) {
    // TODO: Diagnose candidates.
    error(arg->loc) << format("no matching call to operator == for '{}' "
                              "and a descriminator of type '{}'",
                              typed(arg), debug(desc));
    return nullptr;
  }
  Decl* decl = res.solution();
  Term* fn = as<Term>(as<Def>(decl)->init()); // FIXME: Gross
  // FIXME: Make sure that the result type can be converted to bool.
  return fn;
} 

// Returns true if the cmp(left, right) evaluates to true. Note
// Note that cmp is known to be a function that can be called with
// left and right arguments and whose result is convertible to bool.
bool
compare_alternative(Expr* left, Expr* right, Term* cmp) {
  Type* result = as<Fn_type>(get_type(cmp))->result();
  Term* comp = make_expr<Binary>({}, result, cmp, left, right);
  Term* expr = as<Term>(convert(comp, get_bool_type()));
  return eval_boolean(expr);
}

// Search for an alternative that matches the given value.
Alt*
find_alternative(Desc_variant_type* type, Expr* arg) {
  Term* eq = make_discriminator_eq(type->desc(), arg);
  if (not eq)
    return nullptr;

  Alt* def = nullptr;
  for (Decl* d : *type->alts()) {
    Alt* alt = as<Alt>(d);
    Expr* tag = alt->tag();
    
    // If we find the default tag, remmber that we've seen it.
    if (is<Default>(tag))
      def = alt;

    // If we find a range, determine if arg is within that range
    // of values.
    if (is<Range>(tag)) {
      sorry(arg->loc) << "ranges not implemented";
      return nullptr;
    }

    // Otherwise, see if this matches!
    if (compare_alternative(tag, arg, eq))
      return alt;
  }
  return def;
}

// Instantiate the variant over the concrete argument. This is done
// by searching in the variant for an alternative that matches
// the argument.
Type*
instantiate_concrete_variant(Desc_variant_type* type, Expr* arg) {
  if (Alt* alt = find_alternative(type, arg))
    return alt->type();
  error(arg->loc) << format("no alternative matching '{}'", debug(arg));
  return nullptr;
}

// Instantiate a dependent 
Type*
instantiate_dependent_variant(Desc_variant_type* type, Expr* expr, Decl* decl) {
  Type* kind = get_typename_type();
  return make_expr<Dep_variant_type>(type->loc, kind, type, decl);
}

// Search through the variant for an alternative that corresponds
// to the given value.
//
// TODO: Should we allow partially evaluated expressions as arguments
// to a dependent variant? For example, if V is a discriminated
// variant and d is a field or parameter:
//
//    v : V(d + 3);
//
// I haven't seen strong motivation for this yet.
Type*
instantiate_variant(Desc_variant_type* type, Expr* arg) {
  if (Decl* decl = as<Decl>(arg))
    return instantiate_dependent_variant(type, arg, decl);
  else
    return instantiate_concrete_variant(type, arg);
}


} // namespace

// Instantiate the discriminated variant type. If the argument is
// a value, then the result of instantiation is the type indexed by
// that value. However, if the argument is a field, then instantiate
// a dependent variant type.
Type*
instantiate_variant(Type* type, Expr* arg) {
  steve_assert(is<Desc_variant_type>(type), "type is not a discriminated variant");
  return instantiate_variant(static_cast<Desc_variant_type*>(type), arg);
}


} // namesapace steve


