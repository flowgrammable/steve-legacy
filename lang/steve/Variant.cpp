
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
// compare values of the argument type.
Term*
make_discriminator_eq(Decl* parm, Expr* arg) {
  Type* type = get_type(parm);
  Term* fake = make_expr<Default>(no_location, type);
  Name* name = new Operator_id("==");
  Overload* ovl = lookup(name);
  Resolution res = resolve_binary(arg->loc, *ovl, arg, fake);
  if (!res.is_unique()) {
    // TODO: Diagnose candidates.
    error(arg->loc) << format("no matching call to operator == for '{}' "
                              "and a descriminator '{}'",
                              typed(arg), typed(parm));
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
find_alternative(Dep_variant_type* type, Expr* arg) {
  Term* eq = make_discriminator_eq(type->arg(), arg);
  if (not eq)
    return nullptr;

  Alt* def = nullptr;
  for (Decl* d : *type->alts()) {
    Alt* alt = as<Alt>(d);
    Expr* tag = alt->tag();

    // Matching depends on the kind of discriminator.    
    if (is<Default>(tag)) {
      // Match everything, but look for a better match.
      def = alt;
    } else if (is<Range>(tag)) {
      // Test to see if tag is in the given range.
      sorry(arg->loc) << "ranges not implemented";
      return nullptr;
    } else {
      // Determine if arg == tag.
      if (compare_alternative(tag, arg, eq))
        return alt;
    }
  }
  return def;
}

// Instantiate the variant over the concrete argument. This is done
// by searching in the variant for an alternative that matches
// the argument.
Type*
instantiate_variant(Dep_variant_type* type, Expr* arg) {
  if (Alt* alt = find_alternative(type, arg))
    return alt->type();
  error(arg->loc) << format("no alternative matching '{}'", debug(arg));
  return nullptr;
}

} // namespace

// Instantiate the dependent variant type. The argument to the variant
// shall be a value. That is, the argument must match one of the
// descriminators within the variant.
Type*
instantiate_variant(Type* type, Expr* arg) {
  steve_assert(is<Dep_variant_type>(type), "type is not a dependent variant");
  steve_assert(is_value(arg), "argument is not a value");  
  return instantiate_variant(static_cast<Dep_variant_type*>(type), arg);
}


} // namesapace steve


