
#include <steve/Subst.hpp>
#include <steve/Ast.hpp>
#include <steve/Intrinsic.hpp>
#include <steve/Type.hpp>
#include <steve/Variant.hpp>
#include <steve/Evaluator.hpp>
#include <steve/Debug.hpp>

namespace steve {

template<typename C, typename T>
  std::basic_ostream<C, T>&
  operator<<(std::basic_ostream<C, T>& os, const Subst& s) {
    for (auto&& p : s)
      os << debug(p.first) << " ~> " << debug(p.second) << '\n';
    return os;
  }


// -------------------------------------------------------------------------- //
// Substitution

// Initialize the substitution with the mapping of a sequence of
// arguments to parameters.
Subst::Subst(Decl_seq* parms, Expr_seq* args) {
  steve_assert(parms->size() == args->size(), "parameter/argument mismatch");
  auto first1 = parms->begin();
  auto first2 = args->begin();
  while (first1 != parms->end()) {
    insert({*first1, *first2});
    ++first1;
    ++first2;
  }
}

Expr*
Subst::get(Decl* d) const {
  auto iter = find(d);
  if (iter != end())
    return iter->second;
  else
    return iter->first;
}

// -------------------------------------------------------------------------- //
// Substitution rules

namespace {


// Substitute for a declaration-id. When a declaration id refers
// to a parameter, substitute according to the following rules:
//
//    [x->s]x = s
//    [y->s]y = y when y != x
//
// Basically, when we find x in the substitution, replace it with
// its mapped expression. Otherwise, preserve the id.
//
// When the declaration-id refers to a non-parameter definition,
// preserve it. That is:
//
//    [x->s]d = d
//
// where d refers to declaration of the form 'def n ...' and
// probably some others.
Expr*
subst_decl_id(Decl_id* e, const Subst& sub) {
  Decl *decl = e->decl();
  if (Parm* parm = as<Parm>(decl))
    if (Expr* s = sub.get(parm))
      return s;
  return e;
}

// Evaluate the intrinsic function.
Expr*
subst_builtin(Builtin* e, const Subst& s) {
  return e;
}

// Substitute into a function of the form \(p*).e.
//
//    [s]\(p*).e = \(p*).[s]e
//
// FIXME: Do I need to substitute into the paramter list? Probably.
Expr*
subst_fn(Fn* e, const Subst& s) {
  return subst(e->body(), s);
}

// Substitute into a call expression.
//
//    [s]f(a*) = [s](f) ([s]ai)
//
// TODO: Do I really need to substitute into the target? Probably.
Expr*
subst_call(Call* e, const Subst& sub) {
  // Get the target function. 
  Term* fn = e->fn();

  // Substitute through all arguments.
  Expr_seq* args = new Expr_seq();
  for (Expr* a : *e->args())
    args->push_back(subst(a, sub));

  // Rebuild the call expression. Note that you can't substitute
  // directly into the body. We need to build a new substutition
  // that covers the complete set of arguments.
  return make_expr<Call>(e->loc, type(e), fn, args);
}

Expr*
subst_variant(Dep_variant_type* e, const Subst& sub) {
  steve_assert(sub.get(e->arg()), "missing argument for dependent variant type");
  Expr* arg = sub.get(e->arg());
  return instantiate_variant(e, arg);
}

Expr*
subst_dependent(Dep_type* e, const Subst& sub) {
  // Substitute into the arguments.
  Expr_seq* args = new Expr_seq();
  for (Expr* arg : *e->args())
    args->push_back(subst(arg, sub));

  // Re-evaluate the type function.
  Expr* call = make_expr<Call>(no_location, type(e), e->fn(), args);
  return reduce(call);
}

} // namespace


// Replace, in the expression e, all those declarations in the
// the substitution s that are mapped to terms.
Expr* 
subst(Expr* e, const Subst& sub) {
  switch (e->kind) {
  case decl_id: return subst_decl_id(as<Decl_id>(e), sub);
  case unit_term: return e;
  case bool_term: return e;
  case int_term: return e;

  case fn_term: return subst_fn(as<Fn>(e), sub);
  case call_term: return subst_call(as<Call>(e), sub);
  case builtin_term: return subst_builtin(as<Builtin>(e), sub);

  case typename_type: return e;
  case unit_type: return e;
  case bool_type: return e;
  case nat_type: return e;
  case int_type: return e;
  case dep_variant_type: return subst_variant(as<Dep_variant_type>(e), sub);
  case dep_type: return subst_dependent(as<Dep_type>(e), sub);
  default:
    steve_unreachable(format("at {}, undefined substitution '{}'", 
                             e->loc, node_name(e)));
  }
  return e;
}

} // namespace steve
